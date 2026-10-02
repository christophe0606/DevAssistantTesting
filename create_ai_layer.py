"""CMSIS-Executorch MLOps flow adapted for the recurrent SDF renderer.

Reads Toolbox-generated target options; emits a multi-method .pte, C array,
selected runtime/operator components, IO descriptors and delegation report.
No downloaded project or source tree is required.
"""
import argparse
from copy import deepcopy
import importlib.metadata
import json
import logging
import os
from pathlib import Path
import re
import shlex
import sys
import warnings
import torch
import yaml

ROOT = Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT/'model'))
warnings.filterwarnings('ignore',message='erase_node.*already erased node')


def compile_spec(mlops,root):
    from executorch.backends.arm.ethosu import EthosUCompileSpec
    options = shlex.split(mlops.get('vela',{}).get('options',''))
    spec,extra = {},[]
    while options:
        token = options.pop(0)
        key,eq,value = token.removeprefix('--').partition('=')
        if not eq and options and not options[0].startswith('--'):
            value=options.pop(0)
        if key in ('accelerator-config','system-config','memory-mode'):
            spec[key]=value
        else:
            extra.append('--'+key+('='+value if value else ''))
    kwargs = dict(target=spec.get('accelerator-config',mlops['npu']['type'].lower()+'-'+str(mlops['npu'].get('macs',256))),system_config=spec.get('system-config'),memory_mode=spec.get('memory-mode'),extra_flags=extra)
    if mlops.get('vela',{}).get('ini'):
        kwargs['config_ini']=str(root/mlops['vela']['ini'])
    print('Vela configuration:',kwargs,flush=True)
    return EthosUCompileSpec(**kwargs).dump_intermediate_artifacts_to(str(ROOT/'out/export/intermediates'))


def quantize_method(spec,method):
    from executorch.backends.arm.quantizer import EthosUQuantizer, QuantizationConfig, get_symmetric_a16w8_quantization_config, get_symmetric_quantization_config
    from torchao.quantization.pt2e import MinMaxObserver
    from torchao.quantization.pt2e.quantizer import QuantizationSpec
    from torchao.quantization.pt2e.quantize_pt2e import prepare_pt2e,convert_pt2e
    bits=method.activation_bits
    base=(get_symmetric_a16w8_quantization_config(epsilon=2**-24) if bits==16 else get_symmetric_quantization_config())
    dtype=torch.int16 if bits==16 else torch.int8
    # Histograms can clip rare near-surface and mask states. Min/max retains
    # the representative trajectory range; held-out render checks it later.
    act=QuantizationSpec(dtype=dtype,quant_min=-32767 if bits==16 else -128,quant_max=32767 if bits==16 else 127,qscheme=torch.per_tensor_symmetric,observer_or_fake_quant_ctr=MinMaxObserver.with_args(eps=2**-24))
    class ShaderQuantizer(EthosUQuantizer):
        def annotate(self,graph):
            graph=super().annotate(graph)
            # Recurrent state slices can share a source while a comparison
            # explicitly ties two slice scales together. TorchAO 0.18's
            # implicit unions then create a self-referencing shared qspec.
            # Retain explicit Arm sharing; disable only inferred sharing.
            for node in graph.graph.nodes:
                annotation=node.meta.get('quantization_annotation')
                if annotation is not None:
                    annotation.allow_implicit_sharing=False
            return graph
    q=ShaderQuantizer(spec).set_global(QuantizationConfig(act,act,base.weight,base.bias))
    graph=torch.export.export(method.module,method.example).module()
    from scripts.prepare_shader_graph import prepare_shader_graph
    prepare_shader_graph(graph)
    # PyTorch 2.13 may attach a shape-guard module; it is a host export
    # assertion, not part of the tensor model. Arm's passes cannot visit it.
    for node in list(graph.graph.nodes):
        if node.op=='call_module' and node.target=='_guards_fn' and not node.users:
            graph.graph.erase_node(node)
    graph.recompile()
    prepared=prepare_pt2e(graph,q)
    with torch.no_grad():
        for sample in method.samples:
            prepared(*sample)
    return convert_pt2e(prepared)


def components(pte,pack):
    import xml.etree.ElementTree as ET
    available={e.attrib.get('Csub') for e in ET.parse(next(pack.glob('*.pdsc'))).iter('component')}
    selected=set()
    for ns,op in sorted(set(re.findall(rb'(aten|dim_order_ops|quantized_decomposed|cortex_m)::(\w+)',pte))):
        ns,op=ns.decode(),op.decode()
        family={'aten':'Portable','dim_order_ops':'Portable','quantized_decomposed':'Quantized','cortex_m':'Cortex-M'}[ns]
        candidates=[family+' '+op,family+' '+op.lstrip('_'),family+' '+re.sub(r'_(per_tensor|per_channel|byte|copy)$','',op)]
        match=next((x for x in candidates if x in available),None)
        if not match:
            raise RuntimeError('No pack component for '+ns+'::'+op)
        selected.add(match)
    return sorted(selected)


def io_desc(shape,dtype=torch.float32,scale=1.,zp=0):
    return dict(shape=list(shape),dtype=str(dtype).replace('torch.',''),scale=float(scale),zero_point=int(zp))


def export_method(method,spec):
    from executorch.backends.arm.ethosu import EthosUPartitioner
    from executorch.exir import EdgeCompileConfig,to_edge_transform_and_lower
    q=quantize_method(spec,method)
    # Arm logs the offending node's inputs, quantization metadata and source
    # trace on a lowering failure. Retain that context outside console output.
    debug_logger=logging.getLogger('executorch.backends.arm.common.debug')
    debug_logger.setLevel(logging.INFO)
    debug_logger.propagate=False
    debug_file=ROOT/'out/export'/f'{method.name}-lowering.log'
    debug_handler=logging.FileHandler(debug_file,mode='w',encoding='utf-8')
    debug_logger.addHandler(debug_handler)
    error=[]
    with torch.no_grad():
        for sample in method.samples:
            delta=(q(*sample)-method.module(*sample)).abs()
            error.append(float(delta.max()))
    print(method.name,'calibration max error:',max(error),flush=True)
    # Export separately so reports identify CPU fallbacks per method. The
    # final program is assembled from these exported edge graphs below.
    try:
        edge=to_edge_transform_and_lower({method.name:torch.export.export(q,method.example)},partitioner={method.name:[EthosUPartitioner(spec)]},compile_config=EdgeCompileConfig(_check_ir_validity=False))
    finally:
        debug_logger.removeHandler(debug_handler)
        debug_handler.close()
    ep=edge.exported_program(method.name)
    delegates=[n for n in ep.graph.nodes if n.op=='call_function' and 'executorch_call_delegate' in str(n.target)]
    fallback=[str(n.target) for n in ep.graph.nodes if n.op=='call_function' and 'getitem' not in str(n.target) and 'executorch_call_delegate' not in str(n.target)]
    # Tensor computation must execute on Ethos. Float application IO permits
    # only boundary Q/DQ on M55_HP; never silently publish math/mask fallbacks.
    cpu_math=[op for op in fallback if not any(name in op for name in ('quantized_decomposed.quantize_per_tensor','quantized_decomposed.dequantize_per_tensor'))]
    if cpu_math or len(delegates)!=1:
        raise RuntimeError(f'{method.name}: expected one complete Ethos delegate and IO Q/DQ only; got {len(delegates)} delegates, CPU math {cpu_math}')
    return ep,dict(delegates=len(delegates),edge_operators=fallback,calibration_max_error=max(error))


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('mlops',type=Path)
    ap.add_argument('--methods',nargs='*',help='Probe selected methods; final layer requires all methods')
    ap.add_argument('--probe',action='store_true',help='Compile and report without replacing the AI layer')
    args=ap.parse_args()
    torch.set_num_threads(4)
    if importlib.metadata.version('executorch')!='1.4.1':
        raise RuntimeError('Python exporter must match PyTorch::ExecuTorch@1.4.1')
    mlops_file=args.mlops.resolve()
    mlops=yaml.safe_load(mlops_file.read_text())['cbuild-mlops']
    spec=compile_spec(mlops,mlops_file.parent)
    from model import get_methods,RenderConfig
    config=RenderConfig.load()
    methods=get_methods()
    if args.methods:
        methods=[m for m in methods if m.name in args.methods]
        if not args.probe:
            raise ValueError('--methods requires --probe to prevent an incomplete runtime layer')
    from executorch.exir import EdgeProgramManager,ExecutorchBackendConfig
    from executorch.exir.passes import MemoryPlanningPass
    from scripts.inspect_program import inspect_schema,arena_sizes,check_banks
    def backend_config():
        return ExecutorchBackendConfig(extract_delegate_segments=False,memory_planning_pass=MemoryPlanningPass(alloc_graph_input=False))
    programs,report={},{}
    report_dir=ROOT/'out/export';report_dir.mkdir(parents=True,exist_ok=True)
    for method in methods:
        print('Exporting',method.name,flush=True)
        try:
            programs[method.name],report[method.name]=export_method(method,spec)
            # Check each method immediately: a large packed-normal network
            # must not waste the rest of an export before failing bank limits.
            # to_executorch mutates its edge program; keep the original for
            # final multi-method serialization and use a fresh memory pass.
            probe=EdgeProgramManager({method.name:deepcopy(programs[method.name])}).to_executorch(backend_config())
            method_memory=inspect_schema(probe.executorch_program)
            planned,temp=arena_sizes(method_memory)
            check_banks(planned,temp,config.display_width,config.display_height)
            report[method.name]['memory']=method_memory[method.name]
            print(method.name,'planned bytes',planned,'temporary bytes',temp,flush=True)
        except Exception as error:
            report[method.name]={'export_error':str(error)}
            (report_dir/'delegation.json').write_text(json.dumps(report,indent=2))
            raise
        (report_dir/'delegation.json').write_text(json.dumps(report,indent=2))
    # Float IO is intentional for the first accuracy/performance experiment.
    # Only application IO Q/DQ remains on the CPU. Each tensor stage has one
    # complete Ethos delegate, enforced before publishing the AI layer.
    edge=EdgeProgramManager(programs)
    program=edge.to_executorch(backend_config())
    pte=bytes(program.buffer)
    memory_report=inspect_schema(program.executorch_program)
    planned,temp=arena_sizes(memory_report)
    check_banks(planned,temp,config.display_width,config.display_height)
    report['pte_bytes']=len(pte)
    (report_dir/'delegation.json').write_text(json.dumps(report,indent=2))
    if args.probe:
        (report_dir/'probe.pte').write_bytes(pte)
        print(json.dumps(report,indent=2),flush=True)
        return
    packroot=Path(os.environ.get('CMSIS_PACK_ROOT',str(Path.home()/'AppData/Local/Arm/Packs')))
    selected=components(pte,packroot/'PyTorch/ExecuTorch/1.4.1')
    layer_file=mlops_file.parent/mlops['model']['clayer']
    layer_file.parent.mkdir(parents=True,exist_ok=True)
    # Publish complete files only after all methods compile successfully.
    (layer_file.parent/'model.pte').write_bytes(pte)
    data=',\n'.join('  '+', '.join('0x%02x'%b for b in pte[i:i+16]) for i in range(0,len(pte),16))
    (layer_file.parent/'model_pte.c').write_text('#include "model_pte.h"\n__attribute__((aligned(32))) const unsigned char model_pte[] = {\n'+data+'\n};\nconst unsigned long model_pte_size=sizeof(model_pte);\n')
    (layer_file.parent/'model_pte.h').write_text('#pragma once\n#ifdef __cplusplus\nextern "C" {\n#endif\nextern const unsigned char model_pte[];\nextern const unsigned long model_pte_size;\n#ifdef __cplusplus\n}\n#endif\n')
    shape_lines=['#pragma once', '#define SDF_MODEL_READY 1',f'#define SDF_PLANNED_ARENA_BYTES {planned}',f'#define SDF_TEMP_ARENA_BYTES {temp}']
    for name,value in vars(config).items():
        shape_lines.append(f'#define SDF_{name.upper()} {value}')
    for half in (False,True):
        h,w=config.shape(half)
        suffix='HALF' if half else 'FULL'
        shape_lines.extend([f'#define SDF_{suffix}_HEIGHT {h}',f'#define SDF_{suffix}_WIDTH {w}'])
    for m in methods:
        shape_lines.append(f'#define SDF_{m.name.upper()}_INPUT_CHANNELS {m.example[0].shape[-1]}')
    (layer_file.parent/'model_io.h').write_text('\n'.join(shape_lines)+'\n')
    layer=dict(type='AI',description='Image-wide SDF stages for Ethos-U85; float IO boundaries',packs=[{'pack':'PyTorch::ExecuTorch@1.4.1'}],define=[{'ET_LOG_ENABLED':0}],**{'add-path':['.']},components=[{'component':'Machine Learning:ExecuTorch:'+x} for x in ['Runtime','Kernel Utils','Kernel Registration','Backend EthosU']]+[{'component':'Machine Learning:ExecuTorch Operators:'+x} for x in selected],groups=[{'group':'SDF program','files':[{'file':'./model_pte.c'},{'file':'./model_pte.h'},{'file':'./model_io.h'}]}])
    layer_file.write_text('# Generated by create_ai_layer.py; do not edit.\n'+yaml.safe_dump({'layer':layer},sort_keys=False))
    (layer_file.parent/'delegation.json').write_text(json.dumps(report,indent=2))
    (layer_file.parent/'memory.json').write_text(json.dumps(dict(methods=memory_report,planned_arena_bytes=planned,temp_arena_bytes=temp),indent=2))
    print('Wrote',layer_file,'program bytes',len(pte),'operators',selected,flush=True)


if __name__=='__main__':
    main()

"""Inspect a generated PTE's operator selection and bare-metal arena sizes."""
import argparse
from collections import Counter
import json
from pathlib import Path
import re
import struct
import math


def align32(value):
    return (value+31)&~31


def vela_blocks(data):
    """Read the block stream emitted by ExecuTorch's arm_vela.py."""
    offset=0
    blocks={}
    while offset+32<=len(data):
        name=data[offset:offset+16].split(b'\0',1)[0]
        size=struct.unpack_from('<I',data,offset+16)[0]
        if offset+32+size>len(data):
            raise ValueError('Truncated Vela block stream')
        if name in blocks:
            raise ValueError('Duplicate Vela block '+repr(name))
        blocks[name]=data[offset+32:offset+32+size]
        offset+=32+((size+15)&~15)
    if offset!=len(data):
        raise ValueError('Truncated Vela block header/padding')
    return blocks


def scratch_size(data):
    blocks=vela_blocks(data)
    if b'scratch_size' in blocks:
        return struct.unpack('<I',blocks[b'scratch_size'])[0]
    raise ValueError('Ethos delegate has no scratch_size block')


def validate_delegate_io(method,payloads):
    """Reject mismatched PTE/Vela IO before embedding an unusable program.

    Ethos-U85's IO row is six dimensions, element size, offset and region.
    An integer delegate cannot write a Float destination: the CPU must retain
    the boundary dequantize operator and its scale/zero point.
    """
    from executorch.exir.schema import DelegateCall, Tensor
    widths={'BYTE':1,'CHAR':1,'SHORT':2,'INT':4,'LONG':8,'BOOL':1}
    checked=0
    for chain in method.chains:
        for instruction in chain.instructions:
            call=instruction.instr_args
            if not isinstance(call,DelegateCall):
                continue
            blocks=vela_blocks(payloads[call.delegate_index])
            slot=0
            for direction in (b'inputs',b'outputs'):
                desc=blocks[direction]
                if len(desc)<4:
                    raise ValueError('Truncated Vela IO descriptor')
                count=struct.unpack_from('<i',desc)[0]
                if count<0 or len(desc)!=4+count*36:
                    raise ValueError('Invalid Vela IO descriptor size')
                for row in range(count):
                    shape=struct.unpack_from('<9i',desc,4+row*36)
                    if slot>=len(call.args):
                        raise ValueError('Delegate call has too few IO arguments')
                    tensor=method.values[call.args[slot]].val
                    context=f'{method.name} delegate {call.delegate_index} {direction.decode()}[{row}]'
                    if not isinstance(tensor,Tensor):
                        raise ValueError(context+': expected a tensor')
                    dtype=tensor.scalar_type.name
                    if widths.get(dtype)!=shape[6]:
                        raise ValueError(f'{context}: PTE {dtype} incompatible with Vela {shape[6]}-byte integer IO')
                    if any(d<=0 for d in shape[:6]) or math.prod(tensor.sizes)!=math.prod(shape[:6]):
                        raise ValueError(f'{context}: PTE shape {tensor.sizes} incompatible with Vela {shape[:6]}')
                    slot+=1
                    checked+=1
    return checked


def inspect_program(pte):
    from executorch.exir._serialize._program import deserialize_pte_binary
    return inspect_schema(deserialize_pte_binary(pte).program)


def inspect_schema(program):
    from executorch.exir.schema import DataLocation
    report={}
    for method in program.execution_plan:
        planned=sum(align32(size) for size in method.non_const_buffer_sizes[1:])
        scratch=[]
        payloads=[]
        for delegate in method.delegates:
            if delegate.id!='EthosUBackend':
                raise ValueError('Unexpected backend '+delegate.id)
            if delegate.processed.location!=DataLocation.INLINE:
                raise ValueError('Expected inline delegate segments')
            payload=program.backend_delegate_data[delegate.processed.index].data
            payloads.append(bytes(payload))
            scratch.append(scratch_size(payloads[-1]))
        checked=validate_delegate_io(method,payloads)
        operators=Counter(op.name+'.'+op.overload for op in method.operators)
        report[method.name]=dict(planned_bytes=planned,delegate_scratch_bytes=scratch,validated_delegate_io=checked,cpu_operator_types=dict(sorted(operators.items())))
    return report


def arena_sizes(report):
    planned=align32(max(method['planned_bytes'] for method in report.values()))
    # Temp memory is reused between operators. Leave room for CPU kernels'
    # temporary allocations and the delegate buffer's alignment padding.
    scratch=max((size for method in report.values() for size in method['delegate_scratch_bytes']),default=0)
    return planned,align32(scratch+512*1024)


def check_banks(planned,temp,width,height):
    if planned+2*width*height*2>4*1024*1024:
        raise ValueError('Planned arena plus two RGB565 frames exceed E8 SRAM1; reduce tile dimensions')
    if temp>4*1024*1024:
        raise ValueError('Temporary arena exceeds E8 SRAM0; reduce tile dimensions')


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('pte',type=Path)
    ap.add_argument('--update-header',action='store_true')
    args=ap.parse_args()
    report=inspect_program(args.pte.read_bytes())
    planned,temp=arena_sizes(report)
    result=dict(methods=report,planned_arena_bytes=planned,temp_arena_bytes=temp)
    path=args.pte.with_name('memory.json')
    path.write_text(json.dumps(result,indent=2)+'\n')
    if args.update_header:
        header=args.pte.with_name('model_io.h')
        content=header.read_text()
        width=int(re.search(r'^#define SDF_DISPLAY_WIDTH (\d+)',content,re.M)[1])
        height=int(re.search(r'^#define SDF_DISPLAY_HEIGHT (\d+)',content,re.M)[1])
        check_banks(planned,temp,width,height)
        for name,value in [('PLANNED_ARENA_BYTES',planned),('TEMP_ARENA_BYTES',temp)]:
            content=re.sub(r'^#define SDF_'+name+r' .*\n','',content,flags=re.M)
            content+=f'#define SDF_{name} {value}\n'
        header.write_text(content)
    print(json.dumps(result,indent=2))


if __name__=='__main__':
    main()

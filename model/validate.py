"""Preview and measure the actual PT2E quantized recurrent image layers."""
import argparse
import json
import sys
from pathlib import Path
import torch
from PIL import Image

sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from model import get_methods, render, March, STRIP_ROWS, RenderConfig
from primitives import box, capsule, scene


def save(path,rgb):
    data = (rgb.squeeze(0).clamp(0,1)*255).round().byte().numpy()
    Image.fromarray(data).save(path)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--width',type=int,default=96)
    ap.add_argument('--time',type=float,default=0)
    ap.add_argument('--quantized',action='store_true')
    ap.add_argument('--output-dir',type=Path,default=Path('out/validation'))
    args = ap.parse_args()
    torch.set_num_threads(4)
    out = args.output_dir; out.mkdir(parents=True,exist_ok=True)
    p = torch.tensor([[[[0.,0.,0.],[2.,0.,0.]]]])
    torch.testing.assert_close(box(p,(1,1,1)),torch.tensor([[[[-1.],[1.]]]]),atol=2e-6,rtol=0)
    torch.testing.assert_close(capsule(p,(-1,0,0),(1,0,0),.25),torch.tensor([[[[-.25],[.75]]]]),atol=2e-6,rtol=0)
    # Frozen rays must remain bit-identical across invocations.
    frozen = torch.zeros(1,2,4,10)
    frozen[...,0:3] = torch.tensor([0.,.5,0.])
    frozen[...,6:8] = torch.tensor([2.,4.])
    torch.testing.assert_close(March()(frozen),frozen)
    config=RenderConfig.load()
    width,height = args.width,max(1,args.width*config.image_height//config.image_width)
    with torch.no_grad():
        reference = render(width,height,args.time)
        assert torch.isfinite(reference).all()
        save(out/'float.png',reference)
        save(out/'float_display.png',config.to_display(reference))
    report = {'width':width,'height':height,'rotation_degrees':config.rotation_degrees,'time':args.time,'float_finite':True}
    if args.quantized:
        from create_ai_layer import quantize_method
        from executorch.backends.arm.ethosu import EthosUCompileSpec
        spec = EthosUCompileSpec(target='ethos-u85-256',system_config='Ethos_U85_SYS_DRAM_Mid',memory_mode='Shared_Sram')
        methods = get_methods(widths=(width,),rows=STRIP_ROWS)
        quantized = {}
        for method in methods:
            if method.name=='upscale':
                continue
            print('quantizing',method.name,flush=True)
            quantized[method.name.rsplit('_',1)[0]] = quantize_method(spec,method)
        def invoke(name,x):
            return quantized[name](x)
        images = []
        with torch.no_grad():
            for row in range(0,height,STRIP_ROWS):
                rgb = render(width,height,args.time,invoke=invoke,row=row,rows=STRIP_ROWS)
                images.append(rgb[:,:min(STRIP_ROWS,height-row)])
            actual = torch.cat(images,dim=1)
            assert torch.isfinite(actual).all()
            save(out/'pt2e.png',actual)
            save(out/'pt2e_display.png',config.to_display(actual))
            error = (reference-actual).abs()
            report.update(mae=float(error.mean()),max_error=float(error.max()),psnr=float(-10*torch.log10(((reference-actual)**2).mean().clamp_min(1e-12))))
    (out/'quality.json').write_text(json.dumps(report,indent=2))
    print(json.dumps(report,indent=2),flush=True)


if __name__=='__main__':
    main()

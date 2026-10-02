"""Change display orientation without quantizing or compiling the Ethos model.

All settings baked into tensor shapes, calibration or memory must still match
the generated layer. Only rotation is updated; re-export for other changes.
"""
import re
import sys
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'model'))
from model import RenderConfig


def update_orientation(config,header):
    for name,value in vars(config).items():
        if name=='rotation_degrees':
            continue
        macro='SDF_'+name.upper()
        match=re.search(r'^#define '+macro+r' (\S+)$',header,re.M)
        if match is None or float(match[1])!=value:
            raise ValueError(f'{macro} changed; re-export the AI layer')
    line=f'#define SDF_ROTATION_DEGREES {config.rotation_degrees}'
    if re.search(r'^#define SDF_ROTATION_DEGREES .*$',header,re.M):
        return re.sub(r'^#define SDF_ROTATION_DEGREES .*$',line,header,flags=re.M)
    return header+line+'\n'


if __name__=='__main__':
    path=ROOT/'ai_layer/model_io.h'
    config=RenderConfig.load()
    path.write_text(update_orientation(config,path.read_text()))
    print(f'Logical image {config.image_width} x {config.image_height}; '
          f'{config.rotation_degrees} degrees clockwise; existing PTE retained')

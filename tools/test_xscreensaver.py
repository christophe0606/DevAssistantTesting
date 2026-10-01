"""Exercise every saver and repeated init/draw/free; render a contact sheet."""
import json, subprocess, sys
from pathlib import Path
from PIL import Image, ImageDraw
root = Path(__file__).resolve().parents[1]
exe = root/'out/xscreensaver-port/native/xs_host.exe'
out = root/'out/xscreensaver-port/frames'
out.mkdir(exist_ok=True)
count = int(subprocess.check_output([str(exe)], text=True))
results = []
indices = [int(x) for x in sys.argv[1:]] or range(count)
for i in indices:
    try:
        r = subprocess.run([str(exe), str(i), str(out/f'{i:03}.ppm')],capture_output=True,text=True,timeout=45)
        entry = {'index':i,'code':r.returncode,'output':r.stdout.strip(),'error':r.stderr.strip()}
    except subprocess.TimeoutExpired:
        entry = {'index':i,'code':'timeout','output':'','error':'45-second host timeout'}
    results.append(entry)
    print(json.dumps(entry), flush=True)
(out.parent/('targeted-results.json' if len(sys.argv)>1 else 'results.json')).write_text(json.dumps(results,indent=2))
cols=8;tw=150;th=275
sheet=Image.new('RGB',(cols*tw,((len(results)+cols-1)//cols)*th),'#202020')
d=ImageDraw.Draw(sheet)
for j,r in enumerate(results):
    x=(j%cols)*tw;y=(j//cols)*th
    p=out/f'{r["index"]:03}.ppm'
    if p.exists():
        im=Image.open(p);im.thumbnail((144,240));sheet.paste(im,(x,y+22))
    d.text((x+2,y+3),str(r['index'])+' '+r['output'].split(' varied_')[0],fill='white' if r['code']==0 else 'red')
sheet.save(out.parent/('targeted-contact-sheet.png' if len(sys.argv)>1 else 'contact-sheet.png'))
print(f'{sum(r["code"]==0 for r in results)}/{len(results)} passed',flush=True)
sys.exit(any(r['code']!=0 for r in results))

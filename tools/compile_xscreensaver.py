"""Native compiler checks for all ports; diagnostics stay in the ignored out tree."""
import hashlib, json, subprocess, sys
from pathlib import Path
root = Path(__file__).resolve().parents[1]
manifest = json.loads((root/'third_party/xscreensaver/inventory.json').read_text())
includes = ['M55_HP/xs_port', 'third_party/xscreensaver/hacks', 'third_party/xscreensaver/utils', 'third_party/xscreensaver/jwxyz']
flags = ['-std=gnu11','-O2','-Wno-everything','-DXS_HOST','-D__STDC__=1','-D_CRT_SECURE_NO_WARNINGS','-DSTANDALONE','-DHAVE_CONFIG_H']
flags += [x for p in includes for x in ['-I',p]]
out = root/'out/xscreensaver-port/native'
out.mkdir(parents=True, exist_ok=True)
files = [Path('third_party/xscreensaver') / e['source'] for e in manifest['cpu']]
files=list(dict.fromkeys(files))
if '--all' in sys.argv:
    files += [Path('third_party/xscreensaver/hacks') / (s+'.c') for s in
              ['xlockmore','analogtv','apple2','pacman_ai','pacman_level','asm6502','delaunay','ansi-tty','bubbles-default']]
    files += [Path('third_party/xscreensaver/utils') / (s+'.c') for s in
              ['hsv','colors','spline','erase','xshm','xdbe','aligned_malloc','thread_util','pow2','xft','xftwrap','utf8wc','font-retry','easing','doubletime']]
    files += sorted(Path('M55_HP/xs_port').glob('*.c'))
failed = 0
common = hashlib.sha256(json.dumps(flags).encode())
for directory in includes:
    for header in sorted((root/directory).rglob('*.h')):
        common.update(header.as_posix().encode())
        common.update(header.read_bytes())
objects = []
for p in files:
    obj = out/(p.stem+'.obj')
    objects.append(obj)
    stamp = out/(p.stem+'.sha256')
    digest = hashlib.sha256(common.digest() + (root/p).read_bytes()).hexdigest()
    if '--incremental' in sys.argv and obj.exists() and stamp.exists() and stamp.read_text() == digest:
        continue
    command = ['clang',*flags, '-c',str(p),'-o',str(out/(p.stem+'.obj'))] if '--objects' in sys.argv else ['clang',*flags,'-fsyntax-only',str(p)]
    r = subprocess.run(command, cwd=root, capture_output=True, text=True)
    (out/(p.stem+'.log')).write_text(r.stderr)
    if r.returncode:
        failed += 1
        print(p.stem + ': ' + next((s for s in r.stderr.splitlines() if 'error:' in s),r.stderr[:150]))
    elif '--objects' in sys.argv:
        stamp.write_text(digest)
print(f'{failed}/{len(files)} failed')
if not failed and '--link' in sys.argv:
    r = subprocess.run(['clang',*flags,'-c','tools/xs_host.c','-o',str(out/'xs_host.obj')], cwd=root,capture_output=True,text=True)
    print(r.stderr)
    if r.returncode:
        sys.exit(1)
    rsp = out/'link.rsp'
    rsp.write_text('\n'.join('"'+p.as_posix()+'"' for p in objects+[out/'xs_host.obj']) + '\n-o "'+(out/'xs_host.exe').as_posix()+'"\n')
    r = subprocess.run(['clang','@'+str(rsp)],cwd=root,capture_output=True,text=True)
    (out/'link.log').write_text(r.stderr)
    print(r.stderr)
    failed = r.returncode
sys.exit(bool(failed))

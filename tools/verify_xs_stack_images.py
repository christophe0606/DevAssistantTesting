"""Compare seeded Maze/Pacman renderings with the pre-fix captures."""
import hashlib
import re
import subprocess
from pathlib import Path

root = Path(__file__).resolve().parents[1]
out = root/'out/xscreensaver-port'
records = []
for record in (out/'grid-walks-reference.log').read_text().splitlines():
    match = re.fullmatch(r'index=(\d+) seed=(\d+) (\w+) varied_pixels=(\d+) sha256=(\w+)', record)
    if not match:
        raise SystemExit(f'Invalid reference: {record}')
    index, seed, name, varied, digest = match.groups()
    ppm = out/f'stack-fixed-{index}-{seed}.ppm'
    run = subprocess.run([str(out/'native/xs_host.exe'), index, str(ppm), '0', seed],
                         capture_output=True, text=True, cwd=root)
    actual = hashlib.sha256(ppm.read_bytes()).hexdigest().upper() if ppm.exists() else ''
    if run.returncode or f'varied_pixels={varied} ' not in run.stdout or actual != digest:
        raise SystemExit(f'{index} seed={seed}: output differs: {run.returncode} {run.stdout} {run.stderr}')
    records.append(record)
(out/'grid-walks-fixed.log').write_text('\n'.join(records)+'\n')
print(f'{len(records)} seeded renderings exactly match the pre-fix captures')

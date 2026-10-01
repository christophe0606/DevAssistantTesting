"""Run native stress cases with captured output and explicit process status."""
import subprocess
import sys
from pathlib import Path

root = Path(__file__).resolve().parents[1]
out = root / 'out/xscreensaver-port'
args = sys.argv[1:]
if len(args) < 2:
    raise SystemExit('usage: test_xs_stress.py LOG_NAME HOST_ARGUMENTS...')
label, *command = args
try:
    result = subprocess.run([str(out/'native/xs_host.exe'), *command], cwd=root,
                            capture_output=True, timeout=300)
except subprocess.TimeoutExpired as error:
    (out/f'{label}.log').write_bytes(error.stdout or b'')
    (out/f'{label}-stderr.log').write_bytes(error.stderr or b'')
    raise SystemExit(f'{label}: timed out after 300 seconds')
(out/f'{label}.log').write_bytes(result.stdout)
(out/f'{label}-stderr.log').write_bytes(result.stderr)
print(f'{label}: exit={result.returncode} records={len(result.stdout.splitlines())}', flush=True)
if result.returncode:
    print(result.stderr.decode(errors='replace')[-2000:], flush=True)
raise SystemExit(bool(result.returncode))

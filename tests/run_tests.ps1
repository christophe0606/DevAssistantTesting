$ErrorActionPreference = 'Stop'
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    & uv run python tools/compile_xscreensaver.py --all --objects --link --incremental
    if ($LASTEXITCODE -ne 0) { throw 'Host compilation failed' }
    & uv run --with pillow python tools/test_xscreensaver.py
    if ($LASTEXITCODE -ne 0) { throw 'Screensaver checks failed' }
    & ./out/xscreensaver-port/native/xs_host.exe --rotation
    if ($LASTEXITCODE -ne 0) { throw 'Rotation checks failed' }
} finally {
    Pop-Location
}

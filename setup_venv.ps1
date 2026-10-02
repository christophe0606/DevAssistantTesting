$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    uv venv --python 3.12
    if ($LASTEXITCODE) { throw 'uv venv failed' }
    uv pip install --python .venv/Scripts/python.exe -r requirements.txt
    if ($LASTEXITCODE) { throw 'Dependency installation failed' }
    uv pip install --python .venv/Scripts/python.exe --no-deps -r requirements-arm-tosa.txt
    if ($LASTEXITCODE) { throw 'TOSA installation failed' }
    uv run --active --no-project python scripts/check_environment.py
    if ($LASTEXITCODE) { throw 'ExecuTorch import/version check failed' }
} finally { Pop-Location }

$ErrorActionPreference = 'Stop'
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    & uv run python tools/compile_xscreensaver.py --all --objects --link --incremental
    if ($LASTEXITCODE -ne 0) { throw 'Host compilation failed' }
    & uv run python tools/test_xs_stress.py display-checks --display-checks
    if ($LASTEXITCODE -ne 0) { throw 'Display rotation or slot timing checks failed' }
    & uv run python tools/test_xs_stress.py pacman-small-stack --pacman-small-stack 4096
    if ($LASTEXITCODE -ne 0) { throw 'Pacman level generation failed on a 64 KiB stack' }
    & uv run python tools/test_xs_stress.py pacman-paths-small-stack --pacman-paths-small-stack 4096
    if ($LASTEXITCODE -ne 0) { throw 'Pacman ghost path checks failed on a 64 KiB stack' }
    & uv run --with pillow python tools/test_xscreensaver.py
    if ($LASTEXITCODE -ne 0) { throw 'Screensaver checks failed' }
    foreach ($seed in @(1, 2, 1741900050)) {
        & uv run python tools/test_xs_stress.py "rotation-small-stack-$seed" --rotation-small-stack $seed
        if ($LASTEXITCODE -ne 0) { throw "Rotation checks failed for seed $seed" }
    }
} finally {
    Pop-Location
}

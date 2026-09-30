$ErrorActionPreference = 'Stop'
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    New-Item -ItemType Directory -Force out | Out-Null
    & clang -std=c11 -Wall -Wextra -Werror -D_CRT_SECURE_NO_WARNINGS -O3 -ffast-math -I M55_HP tests/test_tetris.c M55_HP/tetris.c M55_HP/tetris_ui.c -o out/test_tetris.exe
    if ($LASTEXITCODE -ne 0) { throw 'Host compilation failed' }
    & ./out/test_tetris.exe
    if ($LASTEXITCODE -ne 0) { throw 'Tetris checks failed' }
} finally {
    Pop-Location
}

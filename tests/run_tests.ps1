$ErrorActionPreference = 'Stop'
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    New-Item -ItemType Directory -Force out | Out-Null
    & clang -std=c11 -Wall -Wextra -Werror -D_CRT_SECURE_NO_WARNINGS -O3 -ffast-math -I M55_HP tests/test_pacman.c M55_HP/pacman.c M55_HP/pacman_ui.c -o out/test_pacman.exe
    if ($LASTEXITCODE -ne 0) { throw 'Host compilation failed' }
    & ./out/test_pacman.exe
    if ($LASTEXITCODE -ne 0) { throw 'Pac-Man checks failed' }
} finally {
    Pop-Location
}

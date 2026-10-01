$ErrorActionPreference = 'Stop'
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    New-Item -ItemType Directory -Force out | Out-Null
    & clang -std=c11 -Wall -Wextra -Werror -D_CRT_SECURE_NO_WARNINGS -O3 -ffast-math -I M55_HP tests/test_maze.c M55_HP/maze.c M55_HP/maze_ui.c -o out/test_maze.exe
    if ($LASTEXITCODE -ne 0) { throw 'Host compilation failed' }
    & ./out/test_maze.exe
    if ($LASTEXITCODE -ne 0) { throw 'Maze checks failed' }
} finally {
    Pop-Location
}

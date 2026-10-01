# Growing mazes for Alif DevKit-E8

An automatic random maze animation for the standard 480 x 800 LCD on the
Cortex-M55 HP. Eight scattered seeds grow at the same time until the full screen
contains a single connected maze with an entrance on the top edge and an exit on
the bottom edge. The full-screen display shows white walls on black and an animated
red solution, without text or growth markers. No joystick input is needed.

Generation takes four seconds. The finished maze stays visible for three seconds,
then a red trajectory draws from the entrance to the exit over four seconds.
The entire trajectory remains visible for three seconds before a new random maze
starts automatically.

## Algorithm

The generator uses multi-source randomized Prim generation, specifically the
random-cell growing-tree variant. Each source has a random frontier and takes
turns opening passages. All sources and walls use the same white color.
Union-find joins regions when they meet, opening a passage only between
different connected components. This produces a perfect maze: all 1,500 cells
are connected by 1,499 passages, with exactly one route between any two cells.

Each maze has randomly positioned openings on opposite edges. After generation,
breadth-first search finds the unique entrance-to-exit route. It reuses two retired
frontier buffers for scratch memory. The route animates at a constant speed in
pixels, including partial cell segments, to finish in four seconds regardless of
route length. A four-pixel-wide red line stays inside the corridors and reaches
the screen edges through both openings. Internal passage bits never point outside
the maze; the renderer draws the exterior openings separately.

Seeds are randomly placed in eight regions of the screen so every animation
starts across the display. A persistent xorshift generator provides new seeds
and passages each cycle. Startup timer jitter seeds the sequence on the board;
this is visual pseudorandomness rather than a hardware entropy source.

## Build and run

Open `Blinky.csolution.yml` in CMSIS Solution. Select **DevKit-E8@Release**, then
**Build** and **Load and Run**. Release retains `-O3 -ffast-math`. The original
solution name remains for compatibility.

## Implementation

- `M55_HP/maze.c`: incremental generation, region merging, route finding, and cycling.
- `M55_HP/maze.h`: hardware-independent state, dimensions, and timing constants.
- `M55_HP/maze_ui.c`: full-screen RGB565 white walls and an animated red trajectory.
- `M55_HP/main.c`: LCD setup, cache cleaning, and VSYNC double buffering.

The maze is 30 columns by 50 rows, drawn with 16-pixel cells across all 480 x 800
pixels. Set `MAZE_GROW_MS`, `MAZE_HOLD_MS`, `MAZE_ROUTE_MS`, and
`MAZE_ROUTE_HOLD_MS` in `maze.h` to change the timing. The eight-source placement
uses two columns of four seed regions. Rendering targets approximately 30 FPS;
generation is driven by elapsed milliseconds and stays independent of frame rate.
Two LCD framebuffers occupy 1,536,000 bytes in SRAM0. Maze state uses about 29 KiB
of normal data memory, without heap allocation or recursion.

## Verification

Run `./tests/run_tests.ps1` with native Clang installed. Tests cover 131 seeds,
simultaneous growth, connected acyclic mazes, reciprocal passages, opposite-edge
openings, valid unique solution paths, red route progression without wall overlap,
all four phase timings, different successive mazes, timer rollover during drawing
and holds, long scheduler stalls, reproducibility, and framebuffer bounds.
Previews are emitted as `out/maze-*.ppm`.

The DevKit-E8 Release firmware build and host tests passed. Growing and finished
maze previews and partial and completed red route previews were visually inspected.
The latest Release firmware was
flashed with CMSIS Load and Run, and CMSIS Run remains active. Physical LCD
appearance and maze cycling still need visual confirmation on the board.
To check counters in VS Code, attach the debugger,
pause, and inspect `frames_presented` and `maze.number` individually between runs.

For live inspection, `app_stage = 8` identifies the main loop; `app_error`,
`app_service_error`, and `display_events` should be zero, and `frames_presented`
should advance. `maze` contains the generator state, including the current maze
number, visited cells, carved passages, and generation phase.

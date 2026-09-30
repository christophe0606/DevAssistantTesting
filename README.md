# Pac-Man for Alif DevKit-E8

A joystick-controlled Pac-Man-style maze game for the standard 480 x 800 LCD on the Cortex-M55 HP. This branch replaces the previous Tetris application with an original maze and code-drawn sprites.

## Controls

- Press center to start, pause, resume, or restart after game over.
- Move the joystick left, right, up, or down to steer. Movement continues after release; requested turns are buffered until a legal intersection. Reversing direction is immediate.
- Eat all pellets to advance to the next level. The side tunnel wraps around the maze.
- Small pellets award 10 points; the four large power pellets award 50 points and make ghosts vulnerable for 6.5 seconds. Eating successive ghosts during one power period awards 200, 400, 800, then 1600 points.
- You have three lives. Ghosts speed up over the first five levels. Best score survives restarts in RAM, but not power cycles.

The four ghosts alternate between chasing and scattering. Red follows the player, pink and cyan aim ahead, and orange retreats when close. They choose routes through the maze using breadth-first distances; frightened ghosts choose random turns. Eaten ghosts return to the center with a short respawn delay. This is a compact adaptation, not an exact reproduction of the arcade rules.

## Build and run

Open `Blinky.csolution.yml` in CMSIS Solution. Select **DevKit-E8@Release**, then **Build** and **Load and Run**. Release retains `-O3 -ffast-math`. The original solution name remains for compatibility.

## Implementation

- `M55_HP/pacman.c`: maze, movement, ghost routing, scoring, lives, levels, and controls.
- `M55_HP/pacman.h`: hardware-independent state and API.
- `M55_HP/pacman_ui.c`: RGB565 maze, animated sprites, HUD, and overlays.
- `M55_HP/main.c`: existing LCD setup, GPIO joystick, debounce, cache cleaning, and VSYNC double buffering.

The editable maze in `pacman.c` is 19 columns by 25 rows: `#` walls, `.` pellets, `o` power pellets, and spaces empty corridors. Keep spawn positions traversable and row 12 open at both edges for the wrap tunnel. The native connectivity check catches isolated corridors and pellets.

Joystick GPIO15 pins are unchanged from the previously verified board setup: 0 left, 1 up, 2 down, 3 right, 4 center, active low with pull-ups. Inputs are sampled every 5 ms and debounced for 20 ms. Fixed 20 ms game ticks give smooth movement at three pixels per tick; rendering targets approximately 30 FPS. Two LCD framebuffers occupy 1,536,000 bytes in SRAM0.

## Verification

Run `./tests/run_tests.ps1` with native Clang installed. Checks cover maze connectivity, pellets, buffered turns, walls, tunnel wrapping, power and ghost eating, life loss, pause, restart, level advancement, timer rollover, 30,000 simulated input updates, and framebuffer bounds. Screenshots are emitted as `out/pacman-*.ppm`.

The release build and host checks passed; ready and gameplay previews were visually inspected. Live Release inspection confirmed the ready screen render loop at app_stage 8, 1,128 presented frames, and zero application, service, or display errors. The debugger was detached with CMSIS Run left active. Physical play testing of this new game is still needed.

For live inspection, `app_stage = 8` identifies the main loop; `app_error`, `app_service_error`, and `display_events` should be zero, and `frames_presented` should advance. `joystick_raw` and `joystick_keys` bits 0 through 4 represent left, right, up, down, center. `game` contains the maze and actors.

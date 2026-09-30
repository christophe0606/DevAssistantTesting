# Tetris for Alif DevKit-E8

A joystick-controlled falling-block game on the standard 480 x 800 ILI9806E LCD, running on the Cortex-M55 HP core. It replaces the rotating square demo.

## Play

Press the joystick center to start.

| Control | Action |
| --- | --- |
| Left / right | Move; hold to repeat |
| Up | Rotate clockwise, once per press |
| Down | Soft drop; hold to repeat |
| Short center press and release | Hard drop |
| Hold center for 600 ms | Pause / resume |
| Center while paused | Resume |
| Center after game over | Restart |

The outlined piece shows the landing position. The sidebar shows score, level, cleared lines, next piece, and best score for the current powered session.

Seven tetrominoes are shuffled in bags of seven. The well has 10 columns and 20 visible rows, with two hidden spawn rows. Rotation tries small wall/floor adjustments. A grounded piece locks after 400 ms; up to 12 successful movement/rotation adjustments can reset that delay. Line clears award 100 / 300 / 500 / 800 points times the current level. Soft drops award one point per cell and hard drops two. Every ten cleared lines raises the level and fall speed. Best score is kept in RAM across restarts, not across power cycles.

## Build and run

Open `Blinky.csolution.yml` in CMSIS Solution. Select **DevKit-E8@Release**, then **Build** and **Load and Run**. Release retains `-O3 -ffast-math`. The solution's original Blinky name is retained for compatibility. The default target set references `M55_HP.Debug`.

## Implementation

- `M55_HP/tetris.c` and `tetris.h`: hardware-independent game rules and controls.
- `M55_HP/tetris_ui.c`: RGB565 board, text, next-piece and landing previews.
- `M55_HP/main.c`: Alif LCD initialization, joystick GPIO, 20 ms debounce, and double buffering.

Joystick mapping follows Ensemble 2.2.1 `Boards/DevKit-e8/Drivers/vio_DevKit-E8.c`: GPIO15 pin 0 = left, pin 1 = up, pin 2 = down, pin 3 = right, pin 4 = center. Switches are active low with pull-ups. Only joystick pins are configured; the LCD reset on GPIO15 pin 5 is left to the panel driver.

The LCD uses two frame buffers totaling 1,536,000 bytes in SRAM0. Cache cleaning and vertical-blank swaps prevent stale frames and tearing. Inputs and game timers are serviced every 5 ms while waiting for display refresh. Rendering targets approximately 30 FPS.

For live inspection, `app_stage = 8` indicates the main loop, `app_error` and `display_events` should be zero, and `frames_presented` should advance. `joystick_raw` and `joystick_keys` use bits 0 through 4 for left, right, up, down, center. `game` contains the board, active piece, score, and state.

## Host checks

With a native Clang toolchain installed, run `./tests/run_tests.ps1` from the solution directory. The tests cover all piece rotations, seven-bag distribution, wall/floor kicks, collision, line compaction/scoring, lock delay, input repeat, pause/resume, restart, timer wrap, and 50,000 simulated input updates. They also render ready, gameplay, and game-over previews into `out/tetris-*.ppm` with buffer guard checks. These checks do not replace a physical joystick test.

Validation: Release build and host checks pass. Live board inspection confirmed the render loop with no LCD/service errors; joystick start, movement, rotation, and soft drop were confirmed on the physical board.

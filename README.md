# Arkanoid for Alif DevKit-E8

A joystick-controlled brick-breaker for the standard 480 x 800 ILI9806E LCD, running on the Cortex-M55 HP core. This branch replaces Tetris with Arkanoid.

## Controls

| Control | Action |
| --- | --- |
| Left / right | Move the paddle |
| Down + left / right | Move slowly for precise aiming |
| Center on the title screen | Start and launch |
| Center during play | Pause / resume |
| Center after losing a ball | Launch the next ball |
| Center after clearing a level | Start the next level |
| Center after game over | Start a new game |

You start with three lives. Deflect the ball into the bricks and catch it with the paddle. Hits near the paddle edges send the ball sideways; center hits send it mostly upward. Moving the paddle adds a little spin.

Bricks with two white studs take two hits. Higher rows award more points; clearing a level earns a bonus and advances to a faster wall. Every seventh destroyed brick can release a teal W capsule: catch it to widen the paddle for 12 seconds. Its duration bar appears below the paddle. Pause freezes the game and the power-up timer.

Score, level, and remaining lives appear above the arena. Best score persists across restarts in RAM, but resets when firmware is reloaded or power is removed.

## Build and run

Open `Blinky.csolution.yml` in CMSIS Solution, select **DevKit-E8@Release**, then **Build** and **Load and Run**. Release keeps `-O3 -ffast-math`. The default target set references the Debug configuration. The solution's original Blinky name remains for compatibility with the board setup.

## Implementation

- `M55_HP/arkanoid.c` and `arkanoid.h`: portable game state, controls, and physics.
- `M55_HP/arkanoid_ui.c` and `arkanoid_ui.h`: RGB565 graphics, text, brick armor, ball trail, paddle, and overlays.
- `M55_HP/main.c`: existing LCD initialization, joystick GPIO/debounce, cache maintenance, and vertical-blank frame swaps.

Physics advances in fixed 5 ms steps independently of LCD rendering. Swept collision checks find the earliest contact with bricks, walls, or the paddle, avoiding tunneling through thin bricks. Brick corners use a rectangular ball envelope for forgiving arcade collisions. Long execution stalls are capped at 100 ms of simulation time.

Joystick mapping follows the Alif Ensemble 2.2.1 E8 board driver: GPIO15 pin 0 = left, 1 = up, 2 = down, 3 = right, 4 = center. Inputs are active low, sampled every 5 ms and debounced for 20 ms. Up is unused in this game.

Two RGB565 buffers occupy 1,536,000 bytes in SRAM0. The renderer targets approximately 30 FPS; the game continues servicing input and physics while waiting for the display.

For live inspection, `app_stage = 8` means the main loop; `app_error`, `app_service_error`, and `display_events` should be zero. `frames_presented` advances. `game` holds the ball, paddle, bricks, score, lives, and state. Joystick bits 0 through 4 are left, right, up, down, center.

## Host verification

Run `./tests/run_tests.ps1` with native Clang installed. Tests exercise launch/pause, precision movement and bounds, walls, paddle angle, armored bricks, side and high-speed collisions, scoring, level progression, lives/restart, power-up collection/expiry, fixed-step consistency, timer wrap, and 100,000 input updates. Rendering checks buffer guards and writes previews to `out/arkanoid-*.ppm`.

Validation: Release build and host tests pass. Live inspection confirmed the render loop, 48 initial bricks, three lives, and no LCD/service errors. Paddle control and ball/brick bounces were confirmed on the physical board.

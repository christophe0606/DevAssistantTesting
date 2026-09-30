# Matrix digital rain - Alif DevKit-E8

An automatic Matrix-style screensaver for the standard 480 x 800 ILI9806E LCD. Green code streams fall over a black background, with pale green leading characters, fading trails, and subtle glyph glow. No joystick input or start button is required.

The screen has 30 columns of 16-pixel cells. Two layers of independently timed streams create depth: brighter, faster foreground rain and dimmer background rain. Each stream has a randomized speed, length, brightness, and restart delay. Symbols change as the heads pass and occasionally flicker within trails. The original bitmap alphabet combines digits and abstract angular symbols, some mirrored.

## Build and run

Open `Blinky.csolution.yml` in CMSIS Solution, select **DevKit-E8@Release**, then **Build** and **Load and Run**. The optimized build retains `-O3 -ffast-math`. The original solution name is kept for compatibility with the existing board configuration.

## Source

- `M55_HP/matrix.c` and `matrix.h`: portable rain state, timing, glyphs, and RGB565 renderer.
- `M55_HP/main.c`: LCD/clock initialization, animation loop, cache maintenance, and vertical-blank frame swaps.

The previous game source, tests, menus, scores, and joystick polling have been removed. GPIO support remains because the LCD driver uses it for reset and backlight.

The existing two frame buffers occupy 1,536,000 bytes in SRAM0. Animation timing uses elapsed milliseconds and retains fractional stream movement; rendering targets approximately 30 FPS. Each frame is redrawn from animation state, so alternating buffers never retain stale trails. Long pauses are capped to 250 ms of animation catch-up.

Live inspection: `app_stage = 8` indicates the animation loop. `app_error`, `app_service_error`, and `display_events` should remain zero; `frames_presented`, `rain.updates`, and eventually `rain.respawns` should advance.

## Host checks

Run `./tests/run_tests.ps1` with native Clang installed. Tests cover deterministic initialization, changing frames, fractional timing, timer wrap, 20,000 animation updates, stream recycling, glyph indices, green color bounds, and framebuffer guards. Preview images are written as `out/matrix-initial.ppm` and `out/matrix-rain.ppm`.

Validation: Release build and host checks pass. Live board inspection confirmed 560 presented frames, 115 recycled streams, and zero LCD/service errors. The screensaver is left running.

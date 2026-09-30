# Star Wars-style crawl - Alif DevKit-E8

An automatic gold perspective text crawl over a black starfield on the standard 480 x 800 ILI9806E LCD. The text rises from the bottom, narrows into the distance, and fades away. Once the final text is completely invisible, the screen shows stars alone for 1.4 seconds and starts the crawl again. No controls are required.

## Customize the text

Edit **`M55_HP/crawl_text.h`**, rebuild, and load the application.

- Change the `CRAWL_TEXT` string to your own text.
- Words automatically wrap at 28 columns.
- Use `\n` to force a new line and `\n\n` for a paragraph break.
- Prefix a heading with `#` to center it.
- Lowercase letters display as uppercase. ASCII letters, digits, spaces, and `. , ! ? - ' : *` are supported; other characters appear as question marks.
- Up to 96 wrapped lines are accepted. Overlong text is rejected at initialization rather than silently cut off.
- `CRAWL_SPEED` controls travel speed; keep it positive.
- `CRAWL_RESTART_MS` controls the starfield pause after the crawl; keep it nonzero.

Example:

```c
static const char CRAWL_TEXT[] =
    "# EPISODE II\n"
    "\n"
    "# YOUR TITLE\n"
    "\n"
    "Your story begins here. Words wrap automatically.\n"
    "\n"
    "Add another paragraph to continue the adventure.\n";
```

Loop duration is calculated from the last rendered text row, the perspective fade cutoff, and the configured speed. Longer text therefore gets a longer cycle. The default original sample story repeats approximately every 79 seconds, including the final pause.

## Build and run

Open `Blinky.csolution.yml` in CMSIS Solution, choose **DevKit-E8@Release**, then **Build** and **Load and Run**. Release retains `-O3 -ffast-math`. The solution's original Blinky name remains for compatibility with the board setup.

## Implementation

- `M55_HP/crawl_text.h`: editable story, crawl speed, and restart pause.
- `M55_HP/starwars.c` and `starwars.h`: word wrapping, bitmap text, perspective projection, starfield, fade, and looping timer.
- `M55_HP/main.c`: LCD initialization, cache cleaning, and vertical-blank buffer swaps.

The renderer maps LCD pixels onto a receding text plane, with four subpixel samples to smooth distant glyphs. A fixed starfield remains visible behind the crawl. There is no dynamic memory allocation. Two RGB565 frame buffers use 1,536,000 bytes in SRAM0, and the text mask uses approximately 194 KB of normal RAM.

Live inspection: `app_stage = 8` means the render loop; `app_error`, `app_service_error`, and `display_events` should be zero. `frames_presented` advances. `crawl.phase_ms`, `crawl.crawl_ms`, `crawl.cycle_ms`, and `crawl.cycles` describe progress through the repeating animation.

## Host verification

Run `./tests/run_tests.ps1` with native Clang installed. Tests cover word wrapping, headings, case conversion, long words, maximum text length, empty text, perspective clipping, final fade, exact cycle boundaries, 100 repeat cycles, timer wrap, and framebuffer guards. Preview images are written to `out/starwars-*.ppm`.

Validation: Release build and host tests pass. Live inspection confirmed rendering with zero LCD/service errors. Advancing the animation clock near the end verified the on-board transition through the final fade/pause into a new cycle. The screensaver is left running.

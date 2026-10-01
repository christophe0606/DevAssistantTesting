# XScreenSaver gallery for Alif DevKit-E8

The Cortex-M55 HP runs a full-screen rotation of 146 CPU screensavers from XScreenSaver 6.16. Each gets ten seconds starting from its first presented frame, then the gallery wraps around. The previous joystick demo has been removed.

The catalog contains 121 current upstream C savers, 22 retired CPU savers, Mismunch (the Munch variant), and local WebCollage and VidWhacker equivalents. The 172 OpenGL/GPU entries are excluded, along with the upstream `testx11` drawing diagnostic. The upstream autonomous Pacman screensaver remains one gallery entry.

## Build and run

Open `Blinky.csolution.yml` in CMSIS Solution, select **DevKit-E8@Release**, then **Build** and **Load and Run**. Release uses Arm Compiler 6 with `-O3 -ffast-math`. The existing solution name is retained.

Programming uses the combined HEX, which contains code and read-only assets in separate MRAM regions. Raw BIN export is disabled because multiple load regions require multiple output files; an existing single BIN file prevents that export and leaves the HEX stale.

## Display and memory

Original saver callbacks draw through a CPU implementation of the Xlib/JWXYZ API. A 240×400 ARGB canvas is enlarged 2× to fill the 480×800 RGB565 LCD. There are no gallery headers or footers. Font and sprite atlases retain their required dimensions; ordinary pictures are scaled to at most 200×320 before compact encoding. Desktop screenshots, external programs and network content use bundled images and text.

| Allocation | Bytes |
| --- | ---: |
| LCD double buffers | 1,536,000 |
| Logical canvas | 384,000 |
| Reclaimed saver heap | 6,291,456 |
| Total bulk SRAM | 8,211,456 / 8,388,608 |
| Shared image assets in MRAM | 1,553,824 / 1,572,864 |
| HP stack in DTCM | 65,536 |

SRAM0 (`0x02000000`) and SRAM1 (`0x08000000`) are separate banks requested through the existing secure-enclave service setup. The saver heap has a 2 MiB pool in SRAM0 and a 4 MiB pool in SRAM1; allocations never span the gap between banks. Bulk buffers are excluded from startup zeroing because SRAM1 is enabled in `main`; the gallery initializes the canvas, framebuffers and allocations after that power request. Only one saver is active at a time; its memory is reclaimed at each transition. Small resource limits and a few intermediate image sizes are reduced for the board. See [porting notes](third_party/xscreensaver/PORTING.md) for adaptations and attribution.

## Validation and maintenance

`tests/run_tests.ps1` uses native Clang and `uv` to compile the gallery, exercise every entry for three ten-second simulations, check for nonblank drawing and allocator damage, and run two complete rotations in a shared process across the 32-bit timer rollover. Results and a visual contact sheet are generated under `out/xscreensaver-port/`.

The host checks pass all 146 entries and 292 rotation slots, including timer rollover and a first frame delayed beyond ten seconds. Release builds and its linker memory checks pass. All 146 entries also passed individual checks on the DevKit-E8 M55 HP in Release: multiple frame presentations, zero application/display/saver errors, and clear fault registers. These results span successive builds; earlier passes were retained as fixes were made. The per-entry build and measurements are saved in [board-test-progress.json](third_party/xscreensaver/board-test-progress.json).

Live checks confirmed that Phosphor's slot starts at its first presented frame, after 3.661 seconds of initialization and first-frame work in the measured run. Four automatic transitions occurred after 10.010–10.025 seconds of visible-slot time, including the last-to-first wraparound. The final Release image was loaded through CMSIS Run with cycling enabled, breakpoints cleared and the debugger detached.

Frame presentations can repeat a canvas; these checks do not establish distinct animation-frame rates or visual quality for every entry. Long blocking callbacks can delay a transition beyond ten seconds. Per-entry performance tuning and precomputed Phosphor glyphs remain future work in [WORK_TO_DO.md](WORK_TO_DO.md).

For live inspection, `app_stage = 8` identifies the main loop. `app_error`, `app_service_error`, `display_events`, `xs_failures` and `xs_last_error` should be zero; `frames_presented` and `xs_current_saver` should advance. `xs_error_text` describes a recovered saver error.

To test entries individually, set `xs_cycle_enabled = 0` and `xs_requested_saver` to the desired zero-based index while stopped at the main loop. Resume to initialize and draw that saver through the normal execution path. The request resets to `UINT32_MAX` once consumed. Set `xs_cycle_enabled = 1` to restore ten-second cycling; normal startup enables cycling by default.

The pinned source inventory and exclusion reasons are in `third_party/xscreensaver/inventory.json`. After changing that inventory or the asset policy, regenerate with `tools/prepare_xscreensaver.py` (Pillow) and `tools/integrate_xscreensaver.py`. Native incremental compilation fingerprints configuration and headers as well as each source file.

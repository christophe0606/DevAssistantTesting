# XScreenSaver gallery for Alif DevKit-E8

The Cortex-M55 HP runs a full-screen landscape rotation of 146 CPU screensavers from XScreenSaver 6.16. Each gets 30 seconds starting from its first presented frame, then the gallery wraps around. The previous joystick demo has been removed.

The catalog contains 121 current upstream C savers, 22 retired CPU savers, Mismunch (the Munch variant), and local WebCollage and VidWhacker equivalents. The 172 OpenGL/GPU entries are excluded, along with the upstream `testx11` drawing diagnostic. The upstream autonomous Pacman screensaver remains one gallery entry.

## Build and run

Open `Blinky.csolution.yml` in CMSIS Solution, select **DevKit-E8@Release**, then **Build** and **Load and Run**. Release uses Arm Compiler 6 with `-O3 -ffast-math`. The existing solution name is retained.

Programming uses the combined HEX, which contains code and read-only assets in separate MRAM regions. Raw BIN export is disabled because multiple load regions require multiple output files; an existing single BIN file prevents that export and leaves the HEX stale.

## Display and memory

Original saver callbacks draw through a CPU implementation of the Xlib/JWXYZ API. All savers receive a 400×240 landscape canvas. The shared renderer rotates it clockwise by 90° and enlarges it 2× to fill the physical 480×800 RGB565 LCD; with the board held in landscape, the visible dimensions are 800×480. There are no gallery headers or footers. Font and sprite atlases retain their required dimensions; bundled pictures keep their existing asset scaling. Desktop screenshots, external programs and network content use bundled images and text.

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

`tests/run_tests.ps1` uses native Clang and `uv` to compile the gallery, exercise every entry for three configured-duration simulations, and check for nonblank drawing and allocator damage. It also runs 12,288 Pacman level generations, 16,384 connected/disconnected ghost path cases and six full rotations across three seeds on Windows threads with a 64 KiB stack. Focused display checks verify the rotation, scaling, buffer guards and 30-second boundary, including timer rollover and a delayed first presentation. Logs and a visual contact sheet are generated under `out/xscreensaver-port/`.

All 146 entries passed historical individual host and Release board checks. These sampled short runs and one random history; the initial host tests used the desktop's larger stack. Normal board cycling later exposed a Pacman maze-generator stack overflow. Expanded 64 KiB host tests found a second overflow in Polyominoes. Both walks now use bounded heap state; Maze's union-find and Pacman's ghost path search also use iterative walks, and Pacman rejects out-of-range coordinates. Seeded maze/path/image comparisons preserve the reference behavior.

The stack-fixed portrait image passed ten targeted board checks and 876 small-stack host rotation slots across three seeds. Natural board cycling reached the last-to-first wrap with clear errors/faults; the planned second rotation was stopped at the user's request to change timing and orientation. Historical evidence remains in [board-test-progress.json](third_party/xscreensaver/board-test-progress.json) and [runtime-stress-progress.json](third_party/xscreensaver/runtime-stress-progress.json).

The 30-second landscape Release passed focused host display/timing checks and Phosphor, Polyominoes and Pacman host/board checks. Pacman-to-FuzzyFlakes first presentations were 30.084 seconds apart. The full gallery was not retested after the orientation change, as requested. Current evidence is in [landscape-release-checks.json](third_party/xscreensaver/landscape-release-checks.json).

Live checks confirmed that Phosphor's slot starts at its first presented frame, after 3.661 seconds of initialization and first-frame work in the measured run. Historical automatic transitions occurred after 10.010–10.025 seconds of visible-slot time, including the last-to-first wraparound. Pacman's procedural case also transitioned successfully after the generator fix.

Frame presentations can repeat a canvas; these checks do not establish distinct animation-frame rates or visual quality for every entry. Long blocking callbacks can delay a transition beyond its configured slot. Per-entry performance tuning and precomputed Phosphor glyphs remain future work in [WORK_TO_DO.md](WORK_TO_DO.md).

For live inspection, `app_stage = 8` identifies the main loop. `app_error`, `app_service_error`, `display_events`, `xs_failures` and `xs_last_error` should be zero; `frames_presented` and `xs_current_saver` should advance. `xs_error_text` describes a recovered saver error.

To test entries individually, set `xs_cycle_enabled = 0` and `xs_requested_saver` to the desired zero-based index while stopped at the main loop. Resume to initialize and draw that saver through the normal execution path. The request resets to `UINT32_MAX` once consumed. Set `xs_cycle_enabled = 1` to restore 30-second cycling; normal startup enables cycling by default.

The pinned source inventory and exclusion reasons are in `third_party/xscreensaver/inventory.json`. After changing that inventory or the asset policy, regenerate with `tools/prepare_xscreensaver.py` (Pillow) and `tools/integrate_xscreensaver.py`. Native incremental compilation fingerprints configuration and headers as well as each source file.

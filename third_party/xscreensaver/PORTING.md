# Alif CPU port of XScreenSaver 6.16

Source: <https://www.jwz.org/xscreensaver/xscreensaver-6.16.tar.gz>.
The upstream C files, source images, attribution and license notices remain in this directory. XScreenSaver modules have individual notices: retain those notices and their permissions when redistributing this port or its generated assets. This is an embedded adaptation, not an upstream-supported XScreenSaver platform.

## Runtime

`M55_HP/xs_port` provides resource lookup, deterministic PRNG, a millisecond clock, a bounded allocator, bundled text and image input, a bitmap font, and CPU drawing through the existing JWXYZ function table. There is no X server, window manager, network, filesystem image scanning, subprocess execution, shared-memory extension, or OpenGL renderer. Desktop input and those dependencies are replaced by local behavior where applicable.

All modules receive a 400x240 landscape drawable. The shared renderer rotates it clockwise by 90 degrees and scales it 2x into the physical 480x800 LCD. Slots last 30 seconds from first presentation. The canvas still occupies 384,000 bytes and the LCD buffer sizes are unchanged.

Bulk SRAM uses separate `UNINIT` execution regions at `0x02000000` and `0x08000000`: startup must not clear SRAM1 before the secure-enclave power request in `main`. The allocator links a 2 MiB pool in SRAM0 and a 4 MiB pool in SRAM1 and never coalesces across the bank boundary. The native checks use the same two-pool layout and validate both block chains. The canvas and LCD buffers are fully drawn before display startup, and allocated saver state is explicitly initialized. Read-only assets have their own MRAM load region; combined HEX export is used to program both regions.

`board_io.c` retargets AC6 standard streams to a local sink/EOF and disables semihosting. The original modules can retain their C-library formatting calls without stopping standalone firmware at a debugger I/O trap. C-library assertions are routed through the gallery's recovery path.

Depth-1 pixmaps use packed bits. Graphics contexts retain their referenced clip, tile and stipple pixmaps until replacement or destruction, matching the lifetimes upstream modules expect. Encoded pictures live once in a shared MRAM pool; headers retain explicit array sizes for upstream `sizeof` uses. Initialization and drawing callbacks recover from allocation or explicit-abort errors and count them in `xs_failures`.

Compact image signatures, fields and masks are read with explicit byte accesses because the pack maps the MRAM user region as Device memory; even short `memcmp` calls can become unaligned loads and fault on oddly aligned assets.

WebCollage composes random crops from bundled content. VidWhacker cycles local image filters. XSublim's blocking desktop overlay becomes a scheduled text callback. GlitchPEG corrupts the bundled compact image encoding rather than a filesystem JPEG. Desktop font selection uses the bundled bitmap font.

## Source adaptations

- `jwxyz.h`: load the platform configuration before macro decisions; board abort/exit wrappers; expose fill/stipple GC fields.
- Maze: bound its desktop million-cell arrays to 64×128, enforce compatible cell sizes, and use iterative union-find path compression.
- Pacman: move maze-generation backtracking and ghost-path DFS to bounded heap work stacks; reject maze coordinates outside the fixed grid.
- Polyominoes: replace the recursive blank-region flood with a queue bounded by puzzle cell count.
- Rocks: cap intermediate rock sprites at 160 pixels.
- FilmLeader: reduce its intermediate width to 400 pixels.
- Droste: reduce input oversampling from 2.5 to 2 so its two image copies fit the separate SRAM banks.
- XAnalogTV: three stations, smaller source inputs, and a terminating test-pattern sentinel.
- SpeedMine: correct the upstream assertion that dereferenced the integer `ncolors` field.
- XSublim and GlitchPEG: local callbacks/input as described above.
- Resource defaults: lower Flow particle/color counts, Deluxe thickness, and Tessellimage resolution/depth; disable desktop SHM/DBE/FPS/anaglyph options.
- Phosphor: use scale 2 for readable text and faster glyph preparation on the small display.
- Generated assembly programs and image headers: bundled M6502 strings and compact RLE ARGB or RGB565-plus-mask images.

All 146 rotation entries fit the configured memory budgets; no picture-dependent entry was excluded for memory. The inventory records GPU exclusions separately. Reduced resolution and default counts preserve the algorithms but do not guarantee desktop-identical appearance or desktop frame rates.

## Regeneration

`tools/inventory_xscreensaver.ps1` reads active and retired executable lists and the XML catalog. `tools/prepare_xscreensaver.py` regenerates assets, bitmap fonts, assembly strings and callback registry. `tools/integrate_xscreensaver.py` regenerates the CMSIS source group. The native build and tests are run through `tests/run_tests.ps1`.

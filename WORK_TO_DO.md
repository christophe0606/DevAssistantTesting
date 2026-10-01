# XScreenSaver port: future work and board-validation notes

## Active validation: runtime stack fixes and natural rotations

- Normal cycling faulted in recursive creatlevelblock/nextstep; live UFSR=0x0010 confirmed STKOF against the 64 KiB CPU stack. The generator has a 1312-byte local maze snapshot per recursive frame.
- Pacman generation is fixed with bounded heap backtracking: 768 exact reference matches, 12,288 generations on a 64 KiB host stack, seeded board passes and an automatic transition. No reboot was needed.
- Expanded small-stack rotations also found Polyominoes' region-walk overflow. Its queue replacement preserves 32 seeded images. Maze's union-find and Pacman's ghost DFS now use iterative walks; 32 Maze/Pacman images and 1,024 ghost path references match. The bounds guard fixes an access violation triggered by a direct test from outside blank cells. 16,384 connected/disconnected path cases pass on a 64 KiB stack.
- Release b-7 passed ten targeted board checks and six seeded host rotations on a 64 KiB stack. **Two sustained natural board rotations are in progress and still pending.** Record them separately in `third_party/xscreensaver/runtime-stress-progress.json`; preserve the 146 historical individual passes.
- Finish natural rotations, update the evidence and documentation, clear breakpoints and leave final Release through CMSIS Run with the debugger detached. Do not claim every random state or distinct animation frame has been validated.

## Precompute glyph bitmaps before building

- Phosphor currently prepares all Latin and DEC glyphs, including normal and inverted variants, at runtime. Its desktop scale of 6 took about 12 seconds to initialize on the E8. Scale 2 passed the individual Release board check; a separate timing check measured 3.661 seconds from selection to first presentation. Precomputation remains future work.
- Generate scaled glyph masks and any fuzzy borders during asset preparation, then keep the immutable atlas in MRAM. Build the runtime character descriptors from that atlas instead of drawing hundreds of bitmaps at startup.
- Keep the original 6x10 font and DEC mappings. Compare normal, inverse-video, cursor and symbol rendering with the current implementation before replacing it.
- Check shared atlas lifetimes: graphics contexts must retain referenced masks, and a saver must not free static MRAM data through the dynamic allocator.
- Measure startup time and MRAM cost. The shared asset region currently uses 1,553,824 of 1,572,864 bytes, so avoid adding duplicate expanded atlases without a new memory budget.

## Completed individual Release board validation

- All 146 entries passed on 2026-10-01. The 42 earlier passes for indices 20 through 61 were retained; the resumed run checked 62 through 145, then 0 through 19.
- Every saved result records multiple frame presentations, zero application/display/saver errors and clear Cortex-M fault registers. Presentations can repeat a canvas; they are not necessarily distinct animation updates. Results span successive Release builds rather than one complete replay on the final image.
- Historical automatic transitions passed at 10.010–10.025 seconds, including 145-to-0 wraparound. Current runtime stress status and image are recorded above and in CHECKPOINT.md.
- Results and timing evidence are in `third_party/xscreensaver/board-test-progress.json`; inventory and README reflect this evidence. `CHECKPOINT.md` records the completed state, and `tools/resume_board_tests.js` retains the MCP helpers with all passes loaded.
- For future targeted checks, use `xs_cycle_enabled = 0` and `xs_requested_saver = index` through the normal main loop. Allow slow initialization to finish before judging animation. Injecting `xs_gallery_select()` previously stalled the debugger at Coral; the requested-index control passed.

## Keep ten seconds of visible display time

- `xs_gallery_presented()` starts each slot at its first presented frame. Host and Release board checks passed, including Phosphor's delayed first frame.
- Keep a host regression for a first frame delayed beyond ten seconds and for timer rollover.
- Measure the slowest callbacks. A long blocking draw can still delay the next transition even when the visible slot timer is correct.

## Memory-map and allocator findings

- The working board exposes SRAM0 at `0x02000000` and SRAM1 at `0x08000000`. A live read at `0x02400000` failed. The pack's contiguous 8 MiB application configuration disagreed with its device memory map and caused Triangle to BusFault while zeroing a 2.3 MiB allocation.
- Keep the separate linker and MPU regions. Both bulk regions are `UNINIT`; the secure-enclave power request enables the banks before the gallery initializes their contents.
- Keep a 2 MiB saver pool in SRAM0 and a 4 MiB pool in SRAM1. Allocations and free-block coalescing must never span the gap between banks. Native allocator checks exercise the same two-pool layout.
- Budget both total memory and the largest contiguous allocation. Droste's two 2.5x oversampled image copies did not fit the split pools. Reducing oversampling to 2x made its host test pass at about 3.48 MiB peak, instead of 5.21 MiB.
- Ordinary bundled images remain capped at 200x320; preserve required font/sprite atlas dimensions. Skip a future picture-heavy entry only when scaling or shared assets cannot make it fit, as authorized by the user.

## Asset access and standalone startup

- Read compact image signatures, fields and masks with explicit byte accesses. Asset arrays can be oddly aligned, and MRAM assets have Device memory attributes. AC6 optimized short `memcmp` calls and byte expressions into unaligned loads that faulted in BlitSpin and NoseGuy.
- Keep the no-semihosting standard-I/O retargets so standalone execution does not stop at a debugger I/O trap.
- Program the combined HEX containing code and the separate MRAM asset region. Keep raw BIN export disabled unless multi-region output handling is fixed; the old single BIN file previously prevented HEX regeneration.

## Performance and visual review

- Benchmark each CPU saver on the E8. Excluding OpenGL avoids GPU dependencies but does not guarantee that every CPU algorithm runs smoothly.
- Tune particle counts, temporary image sizes and font scales where needed, preserving recognizable behavior. Compare board output with the native contact sheet, especially clipping, transparency, fonts and sprite atlases.
- The renderer uses a 240x400 logical canvas enlarged to the 480x800 LCD. Consider improving expensive drawing paths before increasing logical resolution.
- Describe WebCollage, VidWhacker and desktop-dependent input as bundled/local equivalents, and retain upstream licenses and source-adaptation notes.

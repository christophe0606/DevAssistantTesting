# Completed XScreenSaver port validation

Updated on 2026-10-01. The resumed board-validation work is complete. Do not restart the individual tests.

## Workspace and requested outcome

- Workspace: `C:\Users\chrfav01\benchresults\TEMP\testassistant\Blinky_M55_HP`
- Branch: `xscreensaver`. No commits have been made; all port changes remain in the working tree.
- The Cortex-M55 HP on Alif DevKit-E8 runs all 146 CPU/non-OpenGL rotation entries in Release, full screen, with ten seconds starting from the first presented frame.
- The user approved bundled/local equivalents for photos, text, network input and external programs, and scaled pictures. No saver was excluded for memory.
- The user's validation strategy was followed: retain successful individual checks and resume at the failing entry after fixes. No delegation was authorized.
- Precomputed Phosphor glyph bitmaps remain future work in [WORK_TO_DO.md](WORK_TO_DO.md).

## Validation evidence

- **All 146 individual Release board checks passed.** Earlier indices 20 through 61 retain their original passing evidence from successive builds. The resumed run checked 62 through 145, then 0 through 19 on build b-41.
- Each passing check observed initialization and multiple frame presentations, zero application/service/display/saver errors, and clear Cortex-M fault registers. Presentations can repeat a canvas; this is not a benchmark of distinct animation frames or a visual review of every saver.
- Authoritative results: [third_party/xscreensaver/board-test-progress.json](third_party/xscreensaver/board-test-progress.json), including per-entry build, time, presentation count, allocator peak and faults.
- Scale-2 Phosphor passed. A separate first-presentation check measured `started` changing from 731422 to 735083 at the presentation callback: **3661 ms** of initialization and first-frame work are excluded from its visible slot.
- Automatic transitions passed: 62-to-63 after 10013 ms, 63-to-64 after 10010 ms, **145-to-0 wraparound after 10025 ms**, and 0-to-1 after 10021 ms. All fault/error flags remained clear.
- Droste passed on the board at 3,481,152 bytes peak allocation, confirming that the reduced oversampling fits separate SRAM banks.
- One CMSIS `continue_execution` worker timeout interrupted the runner at Anemone (83). Session status, recent problems, a pause and a stack/status inspection showed healthy firmware; restoring the source breakpoint allowed that entry and the rest to pass. No shell workaround was used.
- Existing host evidence: all 146 entries and 292 rotation slots passed, including timer rollover and a first frame delayed beyond ten seconds. The scale-2 Phosphor targeted result is merged into the saved host results/contact sheet.
- README and inventory reflect completed board checks and the limits of that evidence. Broad host or board tests were not replayed without a new code change.

## Final build and hardware state

- Solution: `Blinky.csolution.yml`; target **DevKit-E8@Release**, context `M55_HP.Release+DevKit-E8`; AC6 6.24, `-O3 -ffast-math`.
- Firmware implementation is still build **b-41** from the earlier session. No firmware source changes were needed during resumed validation.
- Program: `out/M55_HP/DevKit-E8/Release/M55_HP.axf`; program its combined HEX containing code and assets.
- Resumed CMSIS window: PID **29888**. Re-discover and pin the window after a VS Code restart.
- Resumed load/debug job **d-1** passed; final **load_and_run job r-2** passed.
- **CMSIS Run is active; debugger detached; all breakpoints cleared; normal cycling enabled by the firmware startup default.**
- Do not stop or reflash the running image solely to repeat completed checks.

## Implementation and memory findings to retain

- Official pinned XScreenSaver 6.16 sources and notices are under `third_party/xscreensaver`; archive at `out/xscreensaver-port/xscreensaver-6.16.tar.gz`, source URL https://www.jwz.org/xscreensaver/xscreensaver-6.16.tar.gz.
- Catalog: 121 current CPU C savers, 22 retired CPU savers, Mismunch (Munch variant), and local WebCollage/VidWhacker equivalents. Excludes 172 GL/GPU entries and `testx11`. The old local Pacman demo is removed; upstream autonomous Pacman remains.
- `M55_HP/xs_port` supplies CPU Xlib/JWXYZ drawing, bounded allocation, resources, fonts, images, local text, lifecycle, scheduling and recovery.
- A 240x400 ARGB canvas is enlarged 2x to the 480x800 RGB565 LCD. Ordinary pictures are capped at 200x320; required font/sprite atlas dimensions are preserved.
- Shared assets use a separate MRAM region. Raw BIN export remains disabled: the old single BIN prevented multi-region export and left stale HEX output.
- Startup requests SRAM0/SRAM1 power through SE services before use, excludes bulk SRAM from startup zeroing, and disables semihosting with local standard-I/O retargets.
- GCs retain referenced clip/stipple/tile pixmaps, allowing upstream masks to be freed while referenced. This fixed Maze's host crash.
- Compact asset fields/signatures/masks use volatile byte reads: AC6 O3 unaligned loads in Device-memory MRAM caused BlitSpin and NoseGuy faults.
- SRAM0 is **0x02000000 / 4 MiB**; SRAM1 is **0x08000000 / 4 MiB**. Live reads rejected 0x02400000. The pack's contiguous map caused Triangle's BusFault. Keep separate linker/MPU regions and 2 MiB + 4 MiB allocator pools; no block or coalescing may cross the gap.
- Droste input oversampling is 2, reduced from 2.5. Phosphor scale is 2.
- `xs_gallery_presented(now)` starts the ten-second slot at first presentation; long blocking callbacks can still delay a transition.
- Budget: SRAM0 4,017,152 / 4,194,304 bytes; SRAM1 4,194,304 / 4,194,304 bytes; total bulk 8,211,456 / 8,388,608 bytes. MRAM assets 1,553,824 / 1,572,864 bytes; stack 64 KiB in DTCM. Allocator peak measures payload usage.

## Future targeted checks

1. Apply `cmsis-debug-live` and the user's AGENTS.md rules. Use `list_debug_windows` / `select_debug_window`, then `get_session_status`.
2. CMSIS Run holds the probe: use `cmsis_action attach` to inspect the running firmware; pause before reading. Use `stop_run` before a fresh load.
3. Read `tools/resume_board_tests.js` and evaluate its source inside `functions.exec`. Loading restores all 146 passes and helpers without accessing the board. Never run it through Node or a shell against hardware.
4. At a stopped source frame, set `xs_cycle_enabled = 0` and `xs_requested_saver = index`. Use assignments, not injected calls to `xs_gallery_select()`. A request resets to UINT32_MAX when consumed.
5. Main-loop breakpoint remains `M55_HP/main.c:158`. After a rebuild, use `lookup_symbol` to refresh addresses and verify block layouts.
6. Restore cycling, clear breakpoints and detach when done. Benchmark callbacks and review visuals separately; see WORK_TO_DO.md.

## Build b-41 raw addresses

| Symbol | Address |
| --- | --- |
| ms_ticks | `0x2000a258` |
| app_stage | `0x2000a25c` |
| display_events | `0x2000a260` |
| app_error | `0x2000a264` |
| app_service_error | `0x2000a268` |
| frames_presented | `0x2000a26c` |
| xs_memory_peak | `0x2000a40c` |
| xs_last_error | `0x2000a4d8` |
| xs_failures | `0x2000a4e0` |
| xs_current_saver | `0x2000a4ec` |
| next_frame | `0x2000a4f0` |
| started | `0x2000a4f4` |
| xs_cycle_enabled | `0x2000001c` |
| xs_requested_saver | `0x20000020` |

Read 24 bytes at ms_ticks, 32 bytes at xs_last_error, and 4 at xs_memory_peak; decode little endian. Healthy main loop: app_stage 8, zero errors and failures, advancing frames, clear faults. Generic fault diagnosis labels code PCs in 0x802... as external RAM; these are valid Alif MRAM code.

## Tool restrictions

- Board/build/run/debug/serial/docs/symbols/memory usage through CMSIS MCP only. No shell cbuild, pyOCD, GDB, J-Link/OpenOCD, serial scripts, internal control-server or registry access.
- If a needed tool fails twice, call get_session_status and get_recent_problems, stop and request the corresponding VS Code action. Wait for the user's answer.
- Native-only commands remain available: `uv run python tools/compile_xscreensaver.py --all --objects --link --incremental`, `uv run --with pillow python tools/test_xscreensaver.py 62`, `out/xscreensaver-port/native/xs_host.exe --rotation`. Do not replay broad checks without a new change or failure.
- Sandbox Git needs `git -c safe.directory=C:/Users/chrfav01/benchresults/TEMP/testassistant/Blinky_M55_HP ...`. Do not commit, discard or overwrite the user's work without instructions.

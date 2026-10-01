# Runtime stack review, 2026-10-01

The original per-saver checks sampled short runs and one random history, while
the original native harness used the desktop stack. These checks missed the
procedural Pacman generator failure seen during normal board cycling. They did
not justify a general runtime-stability claim.

Live board inspection confirmed Pacman's generator exceeded the 64 KiB CPU
stack: UFSR.STKOF was set and each recursive frame held a 1,312-byte maze copy.
Expanded 64 KiB native rotation threads reproduced another overflow in
Polyominoes' recursive blank-region flood for seeds 1 and 2.

Both walks now use bounded heap work storage. Maze's union-find and Pacman's
ghost DFS also use iterative walks to remove depth proportional to grid size.
The ghost search preserves direction order and reversed return-path storage.
Pacman's position check rejects out-of-range coordinates. A direct test had
included blank cells outside the playable connected component and crashed with
an access violation; the corrected test independently floods the jail component
and checks both reachable and disconnected starting cells.

Seeded comparisons preserve 768 maze/dot/RNG records, 1,024 ghost path records,
32 Polyominoes images and 32 Maze/Pacman images. The Windows harness now uses
64 KiB thread reservations for generation, paths and rotations, suppresses
Windows crash-dialog UI, captures process exit status and bounds test duration.

Selected remaining recursion candidates were manually checked: Flame's configured
depth is 25, Julia clamps depth to 10, Forest reduces thickness by 0.68 from at
most 12, Lightning decreases an iteration count starting at 4, and the assembly
parser stops after at most 25 byte parameters. Bouboule's secondary oscillator
disables further recursion, NerveRot subdivisions terminate at small blot counts,
WhirlwindWarp has a terminal secondary field, and XMatrix's failure fallback
switches to a mode that does not call the trace initializer. T3D sorts at most
100 fixed-array elements; bundled Bubbles sprite lists contain fewer than 50
elements. Disabled debugging code and function-like macros accounted for several
heuristic call-graph candidates.

This is a targeted source review and runtime stress campaign. It is not a formal
whole-program stack proof or exhaustive coverage of all random states, callbacks,
resource settings or visuals. Frame presentations can repeat the same canvas.
Historical individual results and new stress/board-rotation evidence are retained
separately in board-test-progress.json and runtime-stress-progress.json.

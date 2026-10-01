#include "maze.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static Maze maze, repeat;
static uint16_t pixels[MAZE_LCD_WIDTH * MAZE_LCD_HEIGHT + 2];
static uint16_t walls[MAZE_LCD_WIDTH * MAZE_LCD_HEIGHT];

/* Independent graph traversal: verify reciprocal internal passages,
 * no cycles or isolated visited cells, and accurate generation counters. */
static void check_forest(const Maze *m)
{
    bool seen[MAZE_CELLS] = {false};
    unsigned queue[MAZE_CELLS], components = 0, visited = 0, ends = 0;
    unsigned territories[MAZE_SOURCES] = {0};
    const int offsets[4] = {-1, 1, -MAZE_COLS, MAZE_COLS};
    for (unsigned cell = 0; cell < MAZE_CELLS; ++cell) {
        assert((m->passages[cell] & ~15U) == 0);
        if (m->owner[cell] == MAZE_UNVISITED) {
            assert(m->passages[cell] == 0);
            continue;
        }
        assert(m->owner[cell] < MAZE_SOURCES);
        ++territories[m->owner[cell]];
        ++visited;
        for (unsigned direction = 0; direction < 4; ++direction) {
            if (!(m->passages[cell] & (1U << direction))) continue;
            assert(direction != MAZE_LEFT || cell % MAZE_COLS != 0);
            assert(direction != MAZE_RIGHT || cell % MAZE_COLS != MAZE_COLS - 1);
            int next = (int)cell + offsets[direction];
            assert(next >= 0 && next < MAZE_CELLS);
            assert(m->owner[next] != MAZE_UNVISITED);
            assert(m->passages[next] & (1U << (direction ^ 1U)));
            ++ends;
        }
        if (seen[cell]) continue;
        ++components;
        unsigned head = 0, tail = 0;
        queue[tail++] = cell;
        seen[cell] = true;
        while (head < tail) {
            unsigned current = queue[head++];
            for (unsigned d = 0; d < 4; ++d) {
                if (!(m->passages[current] & (1U << d))) continue;
                int next = (int)current + offsets[d];
                assert(next >= 0 && next < MAZE_CELLS);
                if (!seen[next]) { seen[next] = true; queue[tail++] = (unsigned)next; }
            }
        }
    }
    assert(visited == m->visited && ends == m->carved * 2U);
    assert(m->carved == visited - components); /* A forest has V - C edges. */
    for (unsigned source = 0; source < MAZE_SOURCES; ++source) {
        assert(territories[source] > 0);
        assert(m->owner[m->seeds[source]] == source);
        assert(m->frontier_count[source] <= territories[source]);
        for (unsigned i = 0; i < m->frontier_count[source]; ++i)
            assert(m->owner[m->frontier[source][i]] == source);
    }
    assert(m->entry > 0 && m->entry < MAZE_COLS - 1);
    assert(m->exit / MAZE_COLS == MAZE_ROWS - 1);
    assert(m->exit % MAZE_COLS > 0 && m->exit % MAZE_COLS < MAZE_COLS - 1);
    if (m->state != MAZE_GROWING) {
        assert(visited == MAZE_CELLS && components == 1);
        assert(m->carved == MAZE_CELLS - 1);
    }
}

static void check_path(const Maze *m)
{
    bool seen[MAZE_CELLS] = {false};
    assert(m->path_count >= MAZE_ROWS && m->path_count <= MAZE_CELLS);
    assert(m->path[0] == m->entry && m->path[m->path_count - 1] == m->exit);
    for (unsigned i = 0; i < m->path_count; ++i) {
        unsigned cell = m->path[i];
        assert(cell < MAZE_CELLS && !seen[cell]);
        seen[cell] = true;
        if (i == 0) continue;
        unsigned before = m->path[i - 1], direction;
        if (cell / MAZE_COLS == before / MAZE_COLS) {
            assert(cell + 1 == before || cell == before + 1);
            direction = cell < before ? MAZE_LEFT : MAZE_RIGHT;
        } else {
            assert(cell + MAZE_COLS == before || cell == before + MAZE_COLS);
            direction = cell < before ? MAZE_UP : MAZE_DOWN;
        }
        assert(m->passages[before] & (1U << direction));
        assert(m->passages[cell] & (1U << (direction ^ 1U)));
    }
}

static void preview(const char *path)
{
    pixels[0] = 0x1234;
    pixels[MAZE_LCD_WIDTH * MAZE_LCD_HEIGHT + 1] = 0xABCD;
    maze_render(pixels + 1, &maze);
    assert(pixels[0] == 0x1234);
    assert(pixels[MAZE_LCD_WIDTH * MAZE_LCD_HEIGHT + 1] == 0xABCD);
    if (!path) return;
    FILE *file = fopen(path, "wb");
    assert(file);
    fprintf(file, "P6\n%d %d\n255\n", MAZE_LCD_WIDTH, MAZE_LCD_HEIGHT);
    for (unsigned i = 1; i <= MAZE_LCD_WIDTH * MAZE_LCD_HEIGHT; ++i) {
        uint16_t p = pixels[i];
        fputc(((p >> 11) & 31) * 255 / 31, file);
        fputc(((p >> 5) & 63) * 255 / 63, file);
        fputc((p & 31) * 255 / 31, file);
    }
    assert(fclose(file) == 0);
}

static void remember_walls(void)
{
    preview(NULL);
    unsigned top_gap = 0, bottom_gap = 0;
    for (unsigned x = 0; x < MAZE_LCD_WIDTH; ++x) {
        if (pixels[1 + x] == 0) ++top_gap;
        if (pixels[1 + (MAZE_LCD_HEIGHT - 1) * MAZE_LCD_WIDTH + x] == 0) ++bottom_gap;
    }
    assert(top_gap == MAZE_CELL_PIXELS - 2 && bottom_gap == MAZE_CELL_PIXELS - 2);
    for (unsigned y = 0; y < MAZE_LCD_HEIGHT; ++y) {
        assert(pixels[1 + y * MAZE_LCD_WIDTH] == UINT16_MAX);
        assert(pixels[1 + y * MAZE_LCD_WIDTH + MAZE_LCD_WIDTH - 1] == UINT16_MAX);
    }
    for (unsigned i = 0; i < MAZE_LCD_WIDTH * MAZE_LCD_HEIGHT; ++i)
        assert(pixels[i + 1] == 0 || pixels[i + 1] == UINT16_MAX);
    memcpy(walls, pixels + 1, sizeof(walls));
}

static unsigned check_route_image(bool complete)
{
    preview(NULL);
    unsigned red = 0;
    for (unsigned i = 0; i < MAZE_LCD_WIDTH * MAZE_LCD_HEIGHT; ++i) {
        if (walls[i] == UINT16_MAX) assert(pixels[i + 1] == UINT16_MAX);
        else if (pixels[i + 1] != 0) {
            uint16_t p = pixels[i + 1];
            /* The route must be predominantly red and stay off all white walls. */
            assert(((p >> 11) & 31) > (p & 31));
            assert(((p >> 11) & 31) * 2 > ((p >> 5) & 63));
            ++red;
        }
    }
    unsigned entrance_pixel = (maze.entry % MAZE_COLS) * MAZE_CELL_PIXELS + MAZE_CELL_PIXELS / 2;
    unsigned exit_pixel = (MAZE_LCD_HEIGHT - 1) * MAZE_LCD_WIDTH +
                          (maze.exit % MAZE_COLS) * MAZE_CELL_PIXELS + MAZE_CELL_PIXELS / 2;
    assert(pixels[1 + entrance_pixel] != 0);
    if (complete) assert(pixels[1 + exit_pixel] != 0);
    else assert(pixels[1 + exit_pixel] == 0);
    return red;
}

static void growth_and_restart(uint32_t seed, uint32_t start)
{
    maze_init(&maze, start, seed);
    assert(maze.visited == MAZE_SOURCES && maze.carved == 0 && maze.number == 1);
    check_forest(&maze);
    for (unsigned elapsed = 33; elapsed < MAZE_GROW_MS; elapsed += 33) {
        unsigned before = maze.carved;
        maze_update(&maze, start + elapsed);
        assert(maze.state == MAZE_GROWING && maze.number == 1);
        assert(maze.carved >= before);
        assert(maze.carved - before <= (MAZE_CELLS - 1U) * 33U / MAZE_GROW_MS + 1U);
        if (elapsed % 330 == 0) check_forest(&maze);
        if (elapsed == 330) {
            unsigned territories[MAZE_SOURCES] = {0};
            for (unsigned i = 0; i < MAZE_CELLS; ++i)
                if (maze.owner[i] != MAZE_UNVISITED) ++territories[maze.owner[i]];
            for (unsigned s = 0; s < MAZE_SOURCES; ++s) assert(territories[s] > 1);
            preview(NULL);
        }
    }
    maze_update(&maze, start + MAZE_GROW_MS);
    assert(maze.state == MAZE_COMPLETE);
    check_forest(&maze);
    check_path(&maze);
    remember_walls();
    uint8_t finished[MAZE_CELLS];
    memcpy(finished, maze.passages, sizeof(finished));
    maze_update(&maze, start + MAZE_GROW_MS + MAZE_HOLD_MS - 1);
    assert(maze.state == MAZE_COMPLETE && maze.number == 1);
    assert(memcmp(finished, maze.passages, sizeof(finished)) == 0);
    preview(NULL);
    maze_update(&maze, start + MAZE_GROW_MS + MAZE_HOLD_MS);
    assert(maze.state == MAZE_SOLVING && maze.number == 1 && maze.route_progress == 0);
    unsigned previous_red = 0;
    uint32_t route_start = start + MAZE_GROW_MS + MAZE_HOLD_MS;
    for (unsigned elapsed = 1000; elapsed < MAZE_ROUTE_MS; elapsed += 1000) {
        maze_update(&maze, route_start + elapsed);
        assert(maze.state == MAZE_SOLVING && maze.route_progress > 0);
        unsigned red = check_route_image(false);
        assert(red > previous_red);
        previous_red = red;
    }
    maze_update(&maze, route_start + MAZE_ROUTE_MS);
    assert(maze.state == MAZE_SOLVED && maze.number == 1);
    assert(maze.route_progress == (uint32_t)maze.path_count * MAZE_CELL_PIXELS - 1U);
    assert(check_route_image(true) > previous_red);
    assert(memcmp(finished, maze.passages, sizeof(finished)) == 0);
    maze_update(&maze, route_start + MAZE_ROUTE_MS + MAZE_ROUTE_HOLD_MS - 1);
    assert(maze.state == MAZE_SOLVED && maze.number == 1);
    check_forest(&maze);
    check_path(&maze);
    check_route_image(true);
    uint32_t restart = route_start + MAZE_ROUTE_MS + MAZE_ROUTE_HOLD_MS;
    maze_update(&maze, restart);
    assert(maze.state == MAZE_GROWING && maze.number == 2 && maze.carved == 0);
    assert(maze.path_count == 0 && maze.route_progress == 0);
    check_forest(&maze);
    maze_update(&maze, restart + MAZE_GROW_MS);
    assert(maze.state == MAZE_COMPLETE);
    check_forest(&maze);
    check_path(&maze);
    assert(memcmp(finished, maze.passages, sizeof(finished)) != 0);
}

int main(void)
{
    /* Many topologies, including zero-seed fallback and timer rollover. */
    for (uint32_t seed = 0; seed < 128; ++seed)
        growth_and_restart(seed, seed & 1U ? UINT32_MAX - 2000U : 100U);
    growth_and_restart(UINT32_MAX, UINT32_MAX - 5500U);
    growth_and_restart(0x80000000U, UINT32_MAX - 8500U); /* Rollover during drawing. */
    growth_and_restart(0xC001D00DU, UINT32_MAX - 12000U); /* Rollover during solved hold. */

    /* Scheduler cadence must not change the random sequence or final maze. */
    maze_init(&maze, 0, 42);
    maze_init(&repeat, 0, 42);
    for (unsigned t = 1; t <= MAZE_GROW_MS; ++t) maze_update(&repeat, t);
    maze_update(&maze, MAZE_GROW_MS);
    assert(memcmp(maze.passages, repeat.passages, sizeof(maze.passages)) == 0);
    assert(memcmp(maze.owner, repeat.owner, sizeof(maze.owner)) == 0);
    assert(maze.path_count == repeat.path_count);
    assert(memcmp(maze.path, repeat.path, maze.path_count * sizeof(maze.path[0])) == 0);

    /* Long stalls still show each phase, including a full hold after arrival. */
    maze_init(&maze, 0, 42);
    maze_update(&maze, 1000000U);
    assert(maze.state == MAZE_COMPLETE && maze.completed_ms == 1000000U);
    maze_update(&maze, 1000000U + MAZE_HOLD_MS - 1);
    assert(maze.state == MAZE_COMPLETE);
    maze_update(&maze, 1000000U + MAZE_HOLD_MS);
    assert(maze.state == MAZE_SOLVING && maze.route_progress == 0);
    maze_update(&maze, 2000000U);
    assert(maze.state == MAZE_SOLVED && maze.solved_ms == 2000000U);
    maze_update(&maze, 2000000U + MAZE_ROUTE_HOLD_MS - 1);
    assert(maze.state == MAZE_SOLVED);
    maze_update(&maze, 2000000U + MAZE_ROUTE_HOLD_MS);
    assert(maze.state == MAZE_GROWING && maze.number == 2);

    maze_init(&maze, 0, 42);
    preview("out/maze-seeds.ppm");
    maze_update(&maze, 1000);
    preview("out/maze-growing.ppm");
    maze_update(&maze, 2000);
    preview("out/maze-half.ppm");
    maze_update(&maze, MAZE_GROW_MS);
    preview("out/maze-complete.ppm");
    maze_update(&maze, MAZE_GROW_MS + MAZE_HOLD_MS);
    maze_update(&maze, MAZE_GROW_MS + MAZE_HOLD_MS + MAZE_ROUTE_MS / 2);
    preview("out/maze-route-half.ppm");
    maze_update(&maze, MAZE_GROW_MS + MAZE_HOLD_MS + MAZE_ROUTE_MS);
    preview("out/maze-route-complete.ppm");
    maze_update(&maze, MAZE_GROW_MS + MAZE_HOLD_MS + MAZE_ROUTE_MS + MAZE_ROUTE_HOLD_MS);
    maze_update(&maze, 2U * MAZE_GROW_MS + MAZE_HOLD_MS + MAZE_ROUTE_MS + MAZE_ROUTE_HOLD_MS);
    preview("out/maze-next.ppm");
    puts("PASS: 131 seeds, concurrent growth, connected acyclic mazes, opposite openings, valid unique routes, red animation without wall overlap, phase timing, new mazes, clock rollover, long stalls, reproducibility, framebuffer guards.");
    return 0;
}

#include "maze.h"
#include <stdbool.h>
#include <string.h>

_Static_assert(MAZE_CELLS <= UINT16_MAX, "Cell indices must fit in uint16_t");
_Static_assert(MAZE_COLS >= 12 && MAZE_ROWS >= 24 && MAZE_SOURCES == 8,
               "Seed placement uses two columns of four regions");

static uint32_t random_below(Maze *maze, uint32_t limit)
{
    /* Xorshift32; avoid the all-zero state in maze_init. */
    uint32_t value = maze->random;
    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    maze->random = value;
    /* Scale rather than using the generator's low bits. */
    return (uint32_t)(((uint64_t)value * limit) >> 32);
}

static uint8_t root(Maze *maze, uint8_t source)
{
    while (maze->parent[source] != source) {
        maze->parent[source] = maze->parent[maze->parent[source]];
        source = maze->parent[source];
    }
    return source;
}

static int neighbor(unsigned cell, unsigned direction)
{
    unsigned x = cell % MAZE_COLS, y = cell / MAZE_COLS;
    switch (direction) {
    case MAZE_LEFT:  return x > 0 ? (int)cell - 1 : -1;
    case MAZE_RIGHT: return x + 1 < MAZE_COLS ? (int)cell + 1 : -1;
    case MAZE_UP:    return y > 0 ? (int)cell - MAZE_COLS : -1;
    default:        return y + 1 < MAZE_ROWS ? (int)cell + MAZE_COLS : -1;
    }
}

static void begin(Maze *maze, uint32_t now)
{
    memset(maze->passages, 0, sizeof(maze->passages));
    memset(maze->owner, MAZE_UNVISITED, sizeof(maze->owner));
    maze->visited = MAZE_SOURCES;
    maze->carved = 0;
    maze->next_source = 0;
    maze->started_ms = maze->clock_ms = now;
    maze->completed_ms = 0;
    maze->route_started_ms = maze->solved_ms = 0;
    maze->path_count = 0;
    maze->route_progress = 0;
    /* Top entrance and bottom exit, away from the corners. Exterior openings
     * are rendered separately; passage bits describe only internal neighbors. */
    maze->entry = (uint16_t)(1 + random_below(maze, MAZE_COLS - 2));
    maze->exit = (uint16_t)((MAZE_ROWS - 1) * MAZE_COLS + 1 +
                          random_below(maze, MAZE_COLS - 2));
    maze->state = MAZE_GROWING;
    ++maze->number;

    /* Stratified random seeds keep growth visible across the whole display. */
    for (unsigned source = 0; source < MAZE_SOURCES; ++source) {
        unsigned left = (source % 2) * MAZE_COLS / 2;
        unsigned right = (source % 2 + 1) * MAZE_COLS / 2;
        unsigned top = (source / 2) * MAZE_ROWS / 4;
        unsigned bottom = (source / 2 + 1) * MAZE_ROWS / 4;
        unsigned x = left + 2 + random_below(maze, right - left - 4);
        unsigned y = top + 2 + random_below(maze, bottom - top - 4);
        uint16_t cell = (uint16_t)(y * MAZE_COLS + x);
        maze->owner[cell] = (uint8_t)source;
        maze->parent[source] = (uint8_t)source;
        maze->seeds[source] = cell;
        maze->frontier[source][0] = cell;
        maze->frontier_count[source] = 1;
    }
}

void maze_init(Maze *maze, uint32_t now, uint32_t seed)
{
    memset(maze, 0, sizeof(*maze));
    maze->random = seed ? seed : 0xA11F1234U;
    begin(maze, now);
}

/* Multi-source randomized Prim (the random-cell growing-tree variant).
 * Each successful turn opens one passage. Union-find allows separate trees
 * to meet without closing a cycle; source ownership stays separate from
 * connected-component membership. */
static bool grow_source(Maze *maze, unsigned source)
{
    while (maze->frontier_count[source] != 0) {
        unsigned index = random_below(maze, maze->frontier_count[source]);
        unsigned cell = maze->frontier[source][index];
        uint8_t directions[4];
        unsigned count = 0;
        uint8_t component = root(maze, (uint8_t)source);
        for (unsigned direction = 0; direction < 4; ++direction) {
            int next = neighbor(cell, direction);
            if (next < 0) continue;
            uint8_t owner = maze->owner[next];
            if (owner == MAZE_UNVISITED || root(maze, owner) != component)
                directions[count++] = (uint8_t)direction;
        }
        if (count == 0) {
            /* Exhausted cells cannot become eligible again: components only merge. */
            maze->frontier[source][index] =
                maze->frontier[source][--maze->frontier_count[source]];
            continue;
        }

        unsigned direction = directions[random_below(maze, count)];
        unsigned next = (unsigned)neighbor(cell, direction);
        if (maze->owner[next] == MAZE_UNVISITED) {
            maze->owner[next] = (uint8_t)source;
            maze->frontier[source][maze->frontier_count[source]++] = (uint16_t)next;
            ++maze->visited;
        } else {
            maze->parent[root(maze, maze->owner[next])] = component;
        }
        maze->passages[cell] |= (uint8_t)(1U << direction);
        maze->passages[next] |= (uint8_t)(1U << (direction ^ 1U));
        ++maze->carved;
        return true;
    }
    return false;
}

static void prepare_route(Maze *maze)
{
    /* Generation is finished, so reuse two frontier buffers for BFS scratch.
     * Keep large arrays out of the Cortex-M stack. */
    uint16_t *previous = maze->frontier[0];
    uint16_t *queue = maze->frontier[1];
    memset(maze->frontier_count, 0, sizeof(maze->frontier_count));
    memset(previous, 0xFF, sizeof(maze->frontier[0]));
    unsigned head = 0, tail = 0;
    previous[maze->entry] = maze->entry;
    queue[tail++] = maze->entry;
    while (head < tail) {
        unsigned cell = queue[head++];
        if (cell == maze->exit) break;
        for (unsigned direction = 0; direction < 4; ++direction) {
            if (!(maze->passages[cell] & (1U << direction))) continue;
            int next = neighbor(cell, direction);
            if (next < 0 || previous[next] != UINT16_MAX) continue;
            previous[next] = (uint16_t)cell;
            queue[tail++] = (uint16_t)next;
        }
    }
    /* A completed perfect maze always reaches the exit; guard against corrupt
     * state rather than following an invalid predecessor outside the arrays. */
    if (previous[maze->exit] == UINT16_MAX) return;
    uint16_t cell = maze->exit;
    for (;;) {
        maze->path[maze->path_count++] = cell;
        if (cell == maze->entry) break;
        cell = previous[cell];
    }
    for (unsigned i = 0; i < maze->path_count / 2U; ++i) {
        uint16_t swap = maze->path[i];
        maze->path[i] = maze->path[maze->path_count - 1U - i];
        maze->path[maze->path_count - 1U - i] = swap;
    }
}

void maze_update(Maze *maze, uint32_t now)
{
    maze->clock_ms = now;
    if (maze->state == MAZE_COMPLETE) {
        if ((uint32_t)(now - maze->completed_ms) >= MAZE_HOLD_MS) {
            if (maze->path_count == 0) { begin(maze, now); return; }
            maze->state = MAZE_SOLVING;
            maze->route_started_ms = now;
        }
        return;
    }
    if (maze->state == MAZE_SOLVING) {
        uint32_t elapsed = now - maze->route_started_ms;
        if (elapsed > MAZE_ROUTE_MS) elapsed = MAZE_ROUTE_MS;
        uint32_t distance = MAZE_CELL_PIXELS * (uint32_t)maze->path_count - 1U;
        maze->route_progress = distance * elapsed / MAZE_ROUTE_MS;
        if (elapsed == MAZE_ROUTE_MS) {
            maze->state = MAZE_SOLVED;
            maze->solved_ms = now;
        }
        return;
    }
    if (maze->state == MAZE_SOLVED) {
        if ((uint32_t)(now - maze->solved_ms) >= MAZE_ROUTE_HOLD_MS)
            begin(maze, now);
        return;
    }

    uint32_t elapsed = now - maze->started_ms;
    if (elapsed > MAZE_GROW_MS) elapsed = MAZE_GROW_MS;
    unsigned target = (MAZE_CELLS - 1U) * elapsed / MAZE_GROW_MS;
    unsigned exhausted = 0;
    while (maze->carved < target && exhausted < MAZE_SOURCES) {
        unsigned source = maze->next_source;
        maze->next_source = (uint8_t)((source + 1U) % MAZE_SOURCES);
        exhausted = grow_source(maze, source) ? 0 : exhausted + 1;
    }
    if (maze->carved == MAZE_CELLS - 1U) {
        prepare_route(maze);
        maze->state = MAZE_COMPLETE;
        maze->completed_ms = now;
    }
}

#ifndef MAZE_H
#define MAZE_H

#include <stdint.h>

#define MAZE_LCD_WIDTH 480
#define MAZE_LCD_HEIGHT 800
#define MAZE_COLS 30
#define MAZE_ROWS 50
#define MAZE_CELL_PIXELS 16
#define MAZE_CELLS (MAZE_COLS * MAZE_ROWS)
#define MAZE_SOURCES 8
#define MAZE_UNVISITED UINT8_MAX
#define MAZE_GROW_MS 4000U
#define MAZE_HOLD_MS 3000U
#define MAZE_ROUTE_MS 4000U
#define MAZE_ROUTE_HOLD_MS 3000U

/* Opposite directions differ by one, so the reciprocal bit is d ^ 1. */
typedef enum { MAZE_LEFT, MAZE_RIGHT, MAZE_UP, MAZE_DOWN } MazeDirection;
typedef enum { MAZE_GROWING, MAZE_COMPLETE, MAZE_SOLVING, MAZE_SOLVED } MazeState;

typedef struct {
    uint8_t passages[MAZE_CELLS];
    uint8_t owner[MAZE_CELLS];
    /* Random frontier cells, kept separately so every source gets a turn. */
    uint16_t frontier[MAZE_SOURCES][MAZE_CELLS];
    uint16_t frontier_count[MAZE_SOURCES];
    uint16_t seeds[MAZE_SOURCES];
    uint8_t parent[MAZE_SOURCES];
    uint8_t next_source;
    uint16_t visited, carved;
    uint16_t entry, exit;
    uint16_t path[MAZE_CELLS], path_count;
    uint32_t route_progress; /* Distance revealed along the route, in pixels. */
    uint32_t random, number, started_ms, completed_ms, clock_ms;
    uint32_t route_started_ms, solved_ms;
    MazeState state;
} Maze;

/* A seed gives repeatable host tests; the board supplies startup timer jitter. */
void maze_init(Maze *maze, uint32_t now, uint32_t seed);
void maze_update(Maze *maze, uint32_t now);
void maze_render(uint16_t *framebuffer, const Maze *maze);

#endif

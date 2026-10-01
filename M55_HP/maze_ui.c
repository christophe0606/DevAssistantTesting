#include "maze.h"
#include <stdbool.h>

#define BG ((uint16_t)0x0000)
#define WHITE ((uint16_t)0xFFFF)
#define RED ((uint16_t)0xF800)
#define CELL MAZE_CELL_PIXELS
#define PATH_WIDTH 4

_Static_assert(MAZE_COLS * CELL == MAZE_LCD_WIDTH &&
               MAZE_ROWS * CELL == MAZE_LCD_HEIGHT,
               "Maze cells must cover the full LCD");

static void rect(uint16_t *fb, int x, int y, int w, int h, uint16_t color)
{
    int right = x + w, bottom = y + h;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (right > MAZE_LCD_WIDTH) right = MAZE_LCD_WIDTH;
    if (bottom > MAZE_LCD_HEIGHT) bottom = MAZE_LCD_HEIGHT;
    for (int yy = y; yy < bottom; ++yy)
        for (int xx = x; xx < right; ++xx)
            fb[yy * MAZE_LCD_WIDTH + xx] = color;
}

/* Reveal a prefix of a horizontal or vertical leg, including a partial cell.
 * Return false when the animated tip has not reached the next waypoint. */
static bool route_leg(uint16_t *fb, int x, int y, int next_x, int next_y,
                      uint32_t *remaining)
{
    int dx = next_x - x, dy = next_y - y;
    unsigned distance = (unsigned)(dx < 0 ? -dx : dx) +
                        (unsigned)(dy < 0 ? -dy : dy);
    if (*remaining == 0) return false;
    unsigned shown = *remaining < distance ? *remaining : distance;
    int end_x = x + (dx < 0 ? -(int)shown : dx > 0 ? (int)shown : 0);
    int end_y = y + (dy < 0 ? -(int)shown : dy > 0 ? (int)shown : 0);
    if (dy == 0)
        rect(fb, end_x < x ? end_x : x, y - PATH_WIDTH / 2,
             (int)shown + 1, PATH_WIDTH, RED);
    else
        rect(fb, x - PATH_WIDTH / 2, end_y < y ? end_y : y,
             PATH_WIDTH, (int)shown + 1, RED);
    *remaining -= shown;
    return shown == distance;
}

void maze_render(uint16_t *fb, const Maze *maze)
{
    for (unsigned pixel = 0; pixel < MAZE_LCD_WIDTH * MAZE_LCD_HEIGHT; ++pixel)
        fb[pixel] = BG;

    /* Draw only visited cells' walls. Shared walls have two one-pixel halves;
     * reciprocal passage bits omit both halves at each opening. */
    for (unsigned cell = 0; cell < MAZE_CELLS; ++cell) {
        if (maze->owner[cell] == MAZE_UNVISITED) continue;
        int x = (int)(cell % MAZE_COLS) * CELL;
        int y = (int)(cell / MAZE_COLS) * CELL;
        uint8_t passages = maze->passages[cell];
        if (!(passages & (1U << MAZE_LEFT))) rect(fb, x, y, 1, CELL, WHITE);
        if (!(passages & (1U << MAZE_RIGHT))) rect(fb, x + CELL - 1, y, 1, CELL, WHITE);
        if (cell == maze->entry) {
            rect(fb, x, y, 1, 1, WHITE);
            rect(fb, x + CELL - 1, y, 1, 1, WHITE);
        } else if (!(passages & (1U << MAZE_UP)))
            rect(fb, x, y, CELL, 1, WHITE);
        if (cell == maze->exit) {
            rect(fb, x, y + CELL - 1, 1, 1, WHITE);
            rect(fb, x + CELL - 1, y + CELL - 1, 1, 1, WHITE);
        } else if (!(passages & (1U << MAZE_DOWN)))
            rect(fb, x, y + CELL - 1, CELL, 1, WHITE);
    }

    if (maze->state != MAZE_SOLVING && maze->state != MAZE_SOLVED) return;
    if (maze->path_count == 0) return;
    uint32_t remaining = maze->route_progress;
    int x = (maze->entry % MAZE_COLS) * CELL + CELL / 2, y = 0;
    for (unsigned i = 0; i < maze->path_count; ++i) {
        unsigned cell = maze->path[i];
        int next_x = (int)(cell % MAZE_COLS) * CELL + CELL / 2;
        int next_y = (int)(cell / MAZE_COLS) * CELL + CELL / 2;
        if (!route_leg(fb, x, y, next_x, next_y, &remaining)) return;
        x = next_x;
        y = next_y;
    }
    route_leg(fb, x, y, x, MAZE_LCD_HEIGHT - 1, &remaining);
}

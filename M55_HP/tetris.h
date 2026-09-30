#ifndef TETRIS_H
#define TETRIS_H
#include <stdbool.h>
#include <stdint.h>

#define TETRIS_COLS 10
#define TETRIS_ROWS 22
#define TETRIS_HIDDEN 2
#define KEY_LEFT   (1U << 0)
#define KEY_RIGHT  (1U << 1)
#define KEY_UP     (1U << 2)
#define KEY_DOWN   (1U << 3)
#define KEY_CENTER (1U << 4)

typedef enum { GAME_READY, GAME_PLAYING, GAME_PAUSED, GAME_OVER } GameState;
typedef struct {
    uint8_t board[TETRIS_ROWS][TETRIS_COLS];
    uint8_t piece, rotation, next, bag[7], bag_pos;
    int x, y;
    uint32_t score, lines, level, best, random;
    uint32_t fall_at, lock_at, horizontal_at, down_at, center_at;
    uint32_t previous_keys;
    int horizontal;
    uint8_t lock_resets;
    bool grounded, horizontal_repeating, center_consumed;
    GameState state;
} Tetris;

bool tetris_cell(uint8_t piece, uint8_t rotation, int x, int y);
bool tetris_fits(const Tetris *g, int x, int y, uint8_t rotation);
int tetris_landing_y(const Tetris *g);
void tetris_init(Tetris *g, uint32_t seed);
void tetris_start(Tetris *g, uint32_t now);
void tetris_update(Tetris *g, uint32_t now, uint32_t keys);
void tetris_hard_drop(Tetris *g, uint32_t now);
bool tetris_rotate(Tetris *g, uint32_t now);
#endif

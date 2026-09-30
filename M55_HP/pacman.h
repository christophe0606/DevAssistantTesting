#ifndef PACMAN_H
#define PACMAN_H
#include <stdint.h>
#include <stdbool.h>
#define MAZE_W 19
#define MAZE_H 25
#define GAME_LCD_WIDTH 480
#define GAME_LCD_HEIGHT 800
#define PAC_LEFT 1U
#define PAC_RIGHT 2U
#define PAC_UP 4U
#define PAC_DOWN 8U
#define PAC_CENTER 16U
typedef enum { LEFT, RIGHT, UP, DOWN, NONE } Direction;
typedef enum { GAME_READY, GAME_PLAYING, GAME_PAUSED, GAME_DYING, GAME_LEVEL_CLEAR, GAME_OVER } GameState;
typedef struct { int x, y; Direction dir; uint32_t wait; } Actor;
typedef struct {
    uint8_t dots[MAZE_H][MAZE_W]; /* 0 empty, 1 pellet, 2 power pellet */
    Actor player, ghosts[4];
    Direction wanted;
    GameState state;
    uint32_t score, best, level, lives, remaining;
    uint32_t last_ms, accumulator, clock_ms, power_ms, state_ms, ghost_clock, keys, random;
    unsigned combo;
} Pacman;
bool pacman_wall(int x, int y);
void pacman_init(Pacman *g, uint32_t now);
void pacman_update(Pacman *g, uint32_t now, uint32_t keys);
void pacman_render(uint16_t *fb, const Pacman *g);
#endif

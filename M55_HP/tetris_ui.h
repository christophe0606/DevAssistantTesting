#ifndef TETRIS_UI_H
#define TETRIS_UI_H
#include "tetris.h"
#define GAME_LCD_WIDTH 480
#define GAME_LCD_HEIGHT 800
void tetris_render(uint16_t *pixels, const Tetris *game);
#endif

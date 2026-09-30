#ifndef ARKANOID_UI_H
#define ARKANOID_UI_H
#include "arkanoid.h"
#define GAME_LCD_WIDTH 480
#define GAME_LCD_HEIGHT 800
void arkanoid_render(uint16_t *pixels, const Arkanoid *game);
#endif

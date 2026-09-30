#ifndef ARKANOID_H
#define ARKANOID_H
#include <stdbool.h>
#include <stdint.h>

#define KEY_LEFT   (1U << 0)
#define KEY_RIGHT  (1U << 1)
#define KEY_UP     (1U << 2)
#define KEY_DOWN   (1U << 3)
#define KEY_CENTER (1U << 4)
#define BRICK_COLS 8
#define BRICK_ROWS 6
#define BRICK_X 34.0f
#define BRICK_Y 166.0f
#define BRICK_W 48.0f
#define BRICK_H 22.0f
#define BRICK_PITCH_X 52.0f
#define BRICK_PITCH_Y 28.0f
#define COURT_LEFT 20.0f
#define COURT_RIGHT 460.0f
#define COURT_TOP 130.0f
#define COURT_BOTTOM 724.0f
#define PADDLE_Y 688.0f
#define PADDLE_H 14.0f
#define BALL_RADIUS 7.0f

typedef enum {
    ARK_READY, ARK_SERVE, ARK_PLAYING, ARK_PAUSED, ARK_LEVEL_CLEAR, ARK_GAME_OVER
} ArkState;
typedef struct {
    uint8_t bricks[BRICK_ROWS][BRICK_COLS];
    uint32_t score, best, level, lives, remaining, destroyed;
    uint32_t previous_keys, last_ms, accumulator, wide_ms, flash_ms;
    int flash_row, flash_col;
    float paddle_x, ball_x, ball_y, vx, vy, speed;
    float power_x, power_y;
    float trail_x[8], trail_y[8];
    bool power_active;
    ArkState state;
} Arkanoid;

void arkanoid_init(Arkanoid *g, uint32_t now);
void arkanoid_update(Arkanoid *g, uint32_t now, uint32_t keys);
float arkanoid_paddle_width(const Arkanoid *g);
#endif

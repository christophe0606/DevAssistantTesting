#include "arkanoid.h"
#include "arkanoid_ui.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void empty_court(Arkanoid *g)
{
    arkanoid_init(g,0);
    arkanoid_update(g,0,KEY_CENTER);
    memset(g->bricks,0,sizeof(g->bricks));
    g->remaining = 1;
}

static void advance(Arkanoid *g, unsigned ms, uint32_t keys)
{
    for (unsigned i = 0; i < ms; i += 5) arkanoid_update(g,g->last_ms+5,keys);
}

static void test_controls(void)
{
    Arkanoid g;
    arkanoid_init(&g,0);
    assert(g.state == ARK_READY && g.lives == 3 && g.remaining == 48);
    advance(&g,1000,KEY_LEFT);
    assert(g.paddle_x == COURT_LEFT+44 && g.ball_x == g.paddle_x);
    advance(&g,2000,KEY_RIGHT);
    assert(g.paddle_x == COURT_RIGHT-44);
    float before = g.paddle_x;
    advance(&g,100,KEY_LEFT|KEY_DOWN);
    assert(fabsf(g.paddle_x-(before-17)) < 0.01f);
    arkanoid_update(&g,g.last_ms+5,KEY_CENTER);
    assert(g.state == ARK_PLAYING && g.vy < 0);
    advance(&g,20,KEY_CENTER);
    assert(g.state == ARK_PLAYING); /* Held center is a single edge. */
    advance(&g,5,0);
    advance(&g,5,KEY_CENTER);
    assert(g.state == ARK_PAUSED);
    float x = g.ball_x, y = g.ball_y;
    arkanoid_update(&g,g.last_ms+100000,0);
    assert(g.ball_x == x && g.ball_y == y);
    advance(&g,5,KEY_CENTER);
    assert(g.state == ARK_PLAYING && g.ball_x == x && g.ball_y == y);
}

static void test_walls_and_paddle(void)
{
    Arkanoid g;
    empty_court(&g);
    g.ball_x = COURT_LEFT+BALL_RADIUS+1; g.ball_y = 450;
    g.vx = -400; g.vy = 0;
    advance(&g,10,0);
    assert(g.vx > 0 && g.ball_x >= COURT_LEFT+BALL_RADIUS);
    g.ball_x = COURT_RIGHT-BALL_RADIUS-1; g.vx = 400;
    advance(&g,10,0);
    assert(g.vx < 0 && g.ball_x <= COURT_RIGHT-BALL_RADIUS);
    g.ball_x = 240; g.ball_y = COURT_TOP+BALL_RADIUS+1;
    g.vx = 0; g.vy = -400;
    advance(&g,10,0);
    assert(g.vy > 0 && g.ball_y >= COURT_TOP+BALL_RADIUS);
    for (int side = -1; side <= 1; ++side) {
        g.ball_x = g.paddle_x+side*30;
        g.ball_y = PADDLE_Y-BALL_RADIUS-1;
        g.vx = 0; g.vy = 350;
        advance(&g,10,0);
        assert(g.vy < 0);
        if (side < 0) assert(g.vx < 0);
        if (side > 0) assert(g.vx > 0);
        assert(fabsf(sqrtf(g.vx*g.vx+g.vy*g.vy)-g.speed) < 0.1f);
    }
    /* Below the paddle is never an underside rescue. */
    g.ball_x = g.paddle_x; g.ball_y = PADDLE_Y+PADDLE_H+BALL_RADIUS;
    g.vx = 0; g.vy = 400;
    advance(&g,100,0);
    assert(g.lives == 2 && g.state == ARK_SERVE);
}

static void test_bricks_and_level(void)
{
    Arkanoid g;
    empty_court(&g);
    const float bx = BRICK_X+4*BRICK_PITCH_X+BRICK_W/2;
    const float by = BRICK_Y+2*BRICK_PITCH_Y+BRICK_H+BALL_RADIUS+2;
    g.bricks[2][4] = 2;
    g.ball_x = bx; g.ball_y = by; g.vx = 0; g.vy = -400;
    advance(&g,10,0);
    assert(g.bricks[2][4] == 1 && g.score == 25 && g.vy > 0);
    g.ball_x = bx; g.ball_y = by; g.vx = 0; g.vy = -400;
    advance(&g,10,0);
    assert(g.bricks[2][4] == 0 && g.remaining == 0 && g.state == ARK_LEVEL_CLEAR);
    assert(g.score == 725 && g.best == 725);
    advance(&g,5,KEY_CENTER);
    assert(g.level == 2 && g.remaining > 0 && g.lives == 3 && g.state == ARK_PLAYING);

    empty_court(&g);
    g.remaining = 2;
    g.bricks[3][2] = 1;
    g.ball_x = BRICK_X+2*BRICK_PITCH_X-BALL_RADIUS-2;
    g.ball_y = BRICK_Y+3*BRICK_PITCH_Y+BRICK_H/2;
    g.vx = 400; g.vy = 0;
    advance(&g,10,0);
    assert(g.bricks[3][2] == 0 && g.vx < 0);

    empty_court(&g);
    g.remaining = 2; g.bricks[2][4] = 1;
    g.ball_x = bx; g.ball_y = by+18; g.vx = 0; g.vy = -10000;
    advance(&g,5,0); /* Crosses a brick in a single physics step. */
    assert(g.bricks[2][4] == 0 && g.vy > 0);
}

static void test_lives_restart_and_power(void)
{
    Arkanoid g;
    empty_court(&g);
    g.score = 1000; g.best = 1000;
    for (unsigned remaining = 2; ; --remaining) {
        g.ball_x = 40; g.ball_y = COURT_BOTTOM+BALL_RADIUS;
        g.vx = 0; g.vy = 400;
        advance(&g,5,0);
        assert(g.lives == remaining);
        if (!remaining) break;
        assert(g.state == ARK_SERVE);
        advance(&g,5,KEY_CENTER);
    }
    assert(g.state == ARK_GAME_OVER);
    advance(&g,5,KEY_CENTER);
    assert(g.state == ARK_PLAYING && g.score == 0 && g.best == 1000 && g.lives == 3);

    empty_court(&g);
    g.ball_x = 100; g.ball_y = 400; g.vx = 0; g.vy = -1;
    g.power_active = true; g.power_x = g.paddle_x; g.power_y = PADDLE_Y-8;
    advance(&g,10,0);
    assert(!g.power_active && g.wide_ms > 11900 && arkanoid_paddle_width(&g) == 136);
    g.wide_ms = 5;
    advance(&g,5,0);
    assert(g.wide_ms == 0 && arkanoid_paddle_width(&g) == 88);
    g.power_active = true; g.power_x = 30; g.power_y = COURT_BOTTOM-1;
    advance(&g,20,0);
    assert(!g.power_active && g.wide_ms == 0);
}

static void test_timestep_and_stress(void)
{
    Arkanoid a,b;
    empty_court(&a); b = a;
    advance(&a,100,KEY_LEFT);
    arkanoid_update(&b,37,KEY_LEFT);
    arkanoid_update(&b,63,KEY_LEFT);
    arkanoid_update(&b,100,KEY_LEFT);
    assert(a.ball_x == b.ball_x && a.ball_y == b.ball_y && a.paddle_x == b.paddle_x);
    a.last_ms = UINT32_MAX-2;
    float y = a.ball_y;
    arkanoid_update(&a,3,0);
    assert(a.ball_y != y && a.accumulator == 1);

    arkanoid_init(&a,0);
    uint32_t rng = 7;
    for (unsigned i = 0; i < 100000; ++i) {
        rng = rng*1664525U+1013904223U;
        uint32_t keys = (rng>>20)&31U;
        arkanoid_update(&a,a.last_ms+5,keys);
        assert(a.lives <= 3 && a.remaining <= 48);
        assert(a.paddle_x >= COURT_LEFT+arkanoid_paddle_width(&a)/2-0.01f);
        assert(a.paddle_x <= COURT_RIGHT-arkanoid_paddle_width(&a)/2+0.01f);
        /* Check representations: fast-math can optimize isfinite() to true. */
        uint32_t ball_x_bits, ball_y_bits;
        memcpy(&ball_x_bits,&a.ball_x,sizeof(ball_x_bits));
        memcpy(&ball_y_bits,&a.ball_y,sizeof(ball_y_bits));
        assert((ball_x_bits & 0x7F800000U) != 0x7F800000U);
        assert((ball_y_bits & 0x7F800000U) != 0x7F800000U);
        for (int row = 0; row < 6; ++row)
            for (int col = 0; col < 8; ++col) assert(a.bricks[row][col] <= 2);
    }
}

static uint16_t guarded[GAME_LCD_WIDTH*GAME_LCD_HEIGHT+2];
static void preview(const char *path, const Arkanoid *g)
{
    guarded[0] = 0x1234; guarded[GAME_LCD_WIDTH*GAME_LCD_HEIGHT+1] = 0x5678;
    arkanoid_render(guarded+1,g);
    assert(guarded[0] == 0x1234 && guarded[GAME_LCD_WIDTH*GAME_LCD_HEIGHT+1] == 0x5678);
    FILE *f = fopen(path,"wb"); assert(f);
    fprintf(f,"P6\n480 800\n255\n");
    for (unsigned i = 1; i <= GAME_LCD_WIDTH*GAME_LCD_HEIGHT; ++i) {
        uint16_t c = guarded[i];
        fputc(((c>>11)&31)*255/31,f); fputc(((c>>5)&63)*255/63,f); fputc((c&31)*255/31,f);
    }
    fclose(f);
}

int main(void)
{
    test_controls(); test_walls_and_paddle(); test_bricks_and_level();
    test_lives_restart_and_power(); test_timestep_and_stress();
    Arkanoid g;
    arkanoid_init(&g,0);
    preview("out/arkanoid-ready.ppm",&g);
    g.state = ARK_PLAYING; g.ball_x = 300; g.ball_y = 490;
    for (int i = 0; i < 8; ++i) { g.trail_x[i] = 300-i*2; g.trail_y[i] = 490-i*3; }
    g.bricks[5][4] = 0; g.bricks[4][4] = 0; g.score = 150; g.best = 150;
    g.power_active = true; g.power_x = 188; g.power_y = 420;
    preview("out/arkanoid-playing.ppm",&g);
    g.state = ARK_GAME_OVER;
    preview("out/arkanoid-over.ppm",&g);
    puts("PASS: controls, wall and paddle bounces, swept brick collision, armor, scoring, levels, lives, restart, power-ups, pause, timestep, wrap, 100000 updates, render bounds");
    return 0;
}

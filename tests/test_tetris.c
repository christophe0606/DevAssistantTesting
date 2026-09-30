#include "tetris.h"
#include "tetris_ui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void fresh(Tetris *g)
{
    tetris_init(g, 12345);
    tetris_start(g, 0);
}

static void test_shapes_and_bag(void)
{
    for (unsigned piece = 0; piece < 7; ++piece)
        for (unsigned rot = 0; rot < 4; ++rot) {
            unsigned count = 0;
            for (int y = 0; y < 4; ++y)
                for (int x = 0; x < 4; ++x)
                    count += tetris_cell((uint8_t)piece,(uint8_t)rot,x,y);
            assert(count == 4);
        }
    Tetris g;
    fresh(&g);
    for (int bag = 0; bag < 20; ++bag) {
        unsigned seen = 0;
        for (int piece = 0; piece < 7; ++piece) {
            assert(!(seen & (1U << g.piece)));
            seen |= 1U << g.piece;
            memset(g.board,0,sizeof(g.board));
            tetris_hard_drop(&g,(uint32_t)(bag*7+piece));
            assert(g.state == GAME_PLAYING);
        }
        assert(seen == 127);
    }
}

static void test_clear_and_score(void)
{
    Tetris g;
    fresh(&g);
    for (int y = 18; y < 22; ++y)
        for (int x = 0; x < 10; ++x) g.board[y][x] = x == 4 ? 0 : 3;
    g.piece = 0; g.rotation = 1; g.x = 2; g.y = 18;
    assert(tetris_fits(&g,g.x,g.y,g.rotation));
    tetris_hard_drop(&g,0);
    assert(g.lines == 4 && g.score == 800 && g.best == 800);
    for (int y = 0; y < 22; ++y)
        for (int x = 0; x < 10; ++x) assert(g.board[y][x] == 0);

    fresh(&g);
    for (int x = 0; x < 10; ++x) g.board[21][x] = (x == 4 || x == 5) ? 0 : 1;
    g.board[20][0] = 7;
    g.piece = 1; g.rotation = 0; g.x = 3; g.y = 20;
    g.lines = 9;
    tetris_hard_drop(&g,0);
    assert(g.lines == 10 && g.level == 2 && g.score == 100);
    assert(g.board[21][0] == 7 && g.board[21][4] == 2 && g.board[21][5] == 2);
    assert(g.board[20][0] == 0);
}

static void test_collision_rotation_and_lock(void)
{
    Tetris g;
    fresh(&g);
    g.piece = 0; g.rotation = 1; g.x = -2; g.y = 5;
    assert(tetris_fits(&g,g.x,g.y,g.rotation));
    assert(!tetris_fits(&g,g.x-1,g.y,g.rotation));
    assert(tetris_rotate(&g,0));
    assert(g.x == 0 && g.rotation == 2); /* Wall kick. */
    g.y = 19; g.rotation = 0;
    assert(tetris_rotate(&g,0)); /* Floor kick. */
    assert(tetris_fits(&g,g.x,g.y,g.rotation));

    fresh(&g);
    g.piece = 1; g.x = 3; g.y = 20; g.rotation = 0;
    assert(tetris_landing_y(&g) == 20);
    tetris_update(&g,10,0);
    assert(g.grounded);
    tetris_update(&g,409,0);
    assert(g.board[21][4] == 0);
    tetris_update(&g,410,0);
    assert(g.board[21][4] == 2 && g.y == 0);
}

static void test_input_pause_restart(void)
{
    Tetris g;
    tetris_init(&g,1);
    tetris_update(&g,100,KEY_CENTER);
    assert(g.state == GAME_PLAYING);
    tetris_update(&g,125,0);
    assert(g.y == 0); /* Start press must not hard drop. */
    int start_x = g.x;
    tetris_update(&g,200,KEY_LEFT);
    assert(g.x == start_x-1);
    tetris_update(&g,379,KEY_LEFT);
    assert(g.x == start_x-1);
    tetris_update(&g,380,KEY_LEFT);
    assert(g.x == start_x-2);
    tetris_update(&g,400,KEY_UP);
    unsigned rot = g.rotation;
    tetris_update(&g,450,KEY_UP);
    assert(g.rotation == rot); /* No held-up rotation repeat. */
    tetris_update(&g,500,KEY_DOWN);
    assert(g.y >= 1);
    tetris_update(&g,1000,KEY_CENTER);
    tetris_update(&g,1600,KEY_CENTER);
    assert(g.state == GAME_PAUSED);
    int paused_y = g.y;
    tetris_update(&g,100000,KEY_CENTER);
    assert(g.y == paused_y && g.state == GAME_PAUSED);
    tetris_update(&g,100010,0);
    tetris_update(&g,100100,KEY_CENTER);
    tetris_update(&g,100200,0);
    assert(g.state == GAME_PLAYING && g.y == paused_y);
    tetris_update(&g,100300,KEY_CENTER);
    tetris_update(&g,100400,0);
    assert(g.y == 0 && g.score > 0);
    g.board[0][0] = 1;
    tetris_hard_drop(&g,100500);
    assert(g.state == GAME_OVER);
    uint32_t best = g.best;
    tetris_update(&g,101000,KEY_CENTER);
    assert(g.state == GAME_PLAYING && g.score == 0 && g.best == best);
    for (int y = 0; y < 22; ++y)
        for (int x = 0; x < 10; ++x) assert(!g.board[y][x]);
}

static void test_time_wrap_and_stress(void)
{
    Tetris g;
    fresh(&g);
    g.fall_at = UINT32_MAX-10U;
    tetris_update(&g,15,0);
    assert(g.y == 0);
    tetris_update(&g,850,0);
    assert(g.y == 1);
    uint32_t rng = 7;
    for (uint32_t i = 0; i < 50000; ++i) {
        rng = rng * 1664525U + 1013904223U;
        if (g.state == GAME_OVER) tetris_start(&g,i*37);
        tetris_update(&g,1000+i*37,(rng >> 23) & 31U);
        if (g.state == GAME_PLAYING || g.state == GAME_PAUSED)
            assert(tetris_fits(&g,g.x,g.y,g.rotation));
        for (int y = 0; y < 22; ++y)
            for (int x = 0; x < 10; ++x) assert(g.board[y][x] <= 7);
    }
}

static uint16_t guarded[GAME_LCD_WIDTH*GAME_LCD_HEIGHT+2];
static void save_preview(const char *path, const Tetris *g)
{
    guarded[0] = 0x1234; guarded[GAME_LCD_WIDTH*GAME_LCD_HEIGHT+1] = 0x5678;
    tetris_render(guarded+1,g);
    assert(guarded[0] == 0x1234 && guarded[GAME_LCD_WIDTH*GAME_LCD_HEIGHT+1] == 0x5678);
    FILE *f = fopen(path,"wb");
    assert(f);
    fprintf(f,"P6\n480 800\n255\n");
    for (unsigned i = 1; i <= GAME_LCD_WIDTH*GAME_LCD_HEIGHT; ++i) {
        uint16_t c = guarded[i];
        fputc(((c>>11)&31)*255/31,f);
        fputc(((c>>5)&63)*255/63,f);
        fputc((c&31)*255/31,f);
    }
    fclose(f);
}

int main(void)
{
    test_shapes_and_bag(); test_clear_and_score();
    test_collision_rotation_and_lock(); test_input_pause_restart();
    test_time_wrap_and_stress();
    Tetris g;
    tetris_init(&g,1);
    save_preview("out/tetris-ready.ppm",&g);
    tetris_start(&g,0);
    g.piece = 2; g.rotation = 1; g.x = 4; g.y = 8;
    g.next = 0; g.score = 2450; g.lines = 12; g.level = 2;
    for (int x = 0; x < 10; ++x)
        for (int y = 21; y > 21-(x*3%5+1); --y)
            g.board[y][x] = (uint8_t)(1+x%7);
    save_preview("out/tetris-playing.ppm",&g);
    g.state = GAME_OVER;
    save_preview("out/tetris-over.ppm",&g);
    puts("PASS: shapes, bags, collision, kicks, scoring, clears, lock delay, input, pause, restart, timer wrap, 50000 updates, render bounds");
    return 0;
}

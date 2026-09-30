#include "pacman.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Pacman g;
static uint16_t pixels[GAME_LCD_WIDTH*GAME_LCD_HEIGHT+2];
static uint32_t now;
static void advance(unsigned ms, unsigned keys)
{
    while (ms) { unsigned step = ms > 20 ? 20 : ms; now += step; pacman_update(&g,now,keys); ms -= step; }
}
static void start(void)
{
    now = 0; pacman_init(&g,now); advance(20,PAC_CENTER); advance(1000,0);
    for (int i = 0; i < 4; ++i) g.ghosts[i].wait = 1000000;
    assert(g.state == GAME_PLAYING && !g.state_ms);
}
static void preview(const char *path)
{
    pixels[0] = 0x1234; pixels[GAME_LCD_WIDTH*GAME_LCD_HEIGHT+1] = 0xABCD;
    pacman_render(pixels+1,&g);
    assert(pixels[0] == 0x1234 && pixels[GAME_LCD_WIDTH*GAME_LCD_HEIGHT+1] == 0xABCD);
    if (!path) return;
    FILE *f = fopen(path,"wb"); assert(f);
    fprintf(f,"P6\n480 800\n255\n");
    for (unsigned i = 1; i <= GAME_LCD_WIDTH*GAME_LCD_HEIGHT; ++i) {
        uint16_t p = pixels[i];
        fputc(((p>>11)&31)*255/31,f); fputc(((p>>5)&63)*255/63,f); fputc((p&31)*255/31,f);
    }
    fclose(f);
}
static void connectivity(void)
{
    bool seen[MAZE_H][MAZE_W] = {{false}};
    int q[MAZE_H*MAZE_W], head = 0, tail = 0;
    q[tail++] = 20*MAZE_W+9; seen[20][9] = true;
    const int dx[] = {-1,1,0,0}, dy[] = {0,0,-1,1};
    while (head < tail) {
        int cell = q[head++], x = cell%MAZE_W, y = cell/MAZE_W;
        for (int d = 0; d < 4; ++d) {
            int nx = x+dx[d], ny = y+dy[d];
            if (ny == 12) nx = (nx+MAZE_W)%MAZE_W;
            if (pacman_wall(nx,ny) || seen[ny][nx]) continue;
            seen[ny][nx] = true; q[tail++] = ny*MAZE_W+nx;
        }
    }
    unsigned dots = 0, power = 0;
    for (int y = 0; y < MAZE_H; ++y)
        for (int x = 0; x < MAZE_W; ++x) {
            if (!pacman_wall(x,y)) assert(seen[y][x]);
            if (g.dots[y][x]) { assert(seen[y][x]); ++dots; }
            if (g.dots[y][x] == 2) ++power;
        }
    assert(dots == g.remaining && dots > 200 && power == 4);
}
int main(void)
{
    pacman_init(&g,0); connectivity(); preview("out/pacman-ready.ppm");
    start(); unsigned initial = g.remaining;
    advance(160,PAC_LEFT); assert(g.player.x == 8*8+4 && g.score == 10 && g.remaining == initial-1);
    /* Buffered upward turn cannot occur in a wall, then happens at column 4. */
    advance(160,PAC_LEFT); advance(480,PAC_UP); assert(g.player.x == 4*8+4 && g.player.y == 20*8+4);
    advance(20,0); assert(g.player.y == 20*8+3 && g.player.dir == UP);
    int x = g.player.x, y = g.player.y;
    advance(20,PAC_CENTER); advance(2000,0);
    assert(g.state == GAME_PAUSED && g.player.x == x && g.player.y == y);
    advance(20,PAC_CENTER); assert(g.state == GAME_PLAYING); advance(20,0);
    assert(g.player.y == y-1);
    /* A held direction stops at a wall; tunnel movement wraps. */
    start(); g.player = (Actor){1*8+4,1*8+4,LEFT,0}; g.wanted = LEFT;
    advance(500,PAC_LEFT); assert(g.player.x == 12);
    g.player = (Actor){4,12*8+4,LEFT,0}; advance(160,PAC_LEFT);
    assert(g.player.x == MAZE_W*8-4);
    /* Power pellet, edible ghost, and frightened expiry. */
    start(); g.player = (Actor){2*8+4,1*8+4,LEFT,0};
    advance(160,PAC_LEFT); assert(g.score == 50 && g.power_ms == 6500);
    g.ghosts[0] = g.player; g.ghosts[0].wait = 0;
    advance(20,PAC_LEFT); assert(g.score == 250 && g.ghosts[0].wait > 2000 && g.lives == 3);
    preview("out/pacman-power.ppm");
    g.power_ms = 20; advance(20,PAC_LEFT); assert(g.power_ms == 0);
    /* Death preserves the remaining maze; final life leads to restart. */
    start(); g.ghosts[0] = g.player; g.ghosts[0].wait = 0;
    advance(20,0); assert(g.state == GAME_DYING && g.lives == 2);
    unsigned remaining = g.remaining;
    advance(1400,0); assert(g.state == GAME_PLAYING && g.remaining == remaining && g.lives == 2);
    advance(1000,0); g.lives = 1; g.ghosts[0] = g.player; g.ghosts[0].wait = 0;
    advance(20,0); advance(1400,0); assert(g.state == GAME_OVER);
    preview("out/pacman-over.ppm");
    g.best = 1234; advance(20,PAC_CENTER); assert(g.lives == 3 && g.score == 0 && g.best == 1234);
    /* Last pellet triggers the next maze, preserving score and lives. */
    start(); memset(g.dots,0,sizeof(g.dots)); g.dots[20][8] = 1; g.remaining = 1;
    advance(160,PAC_LEFT); assert(g.state == GAME_LEVEL_CLEAR);
    advance(1800,0); assert(g.level == 2 && g.score == 10 && g.remaining == initial);
    /* Unsigned clock rollover and long scheduler stalls. */
    start(); g.last_ms = UINT32_MAX-9; now = 10; x = g.player.x;
    pacman_update(&g,now,PAC_LEFT); assert(g.player.x == x-1);
    now += 1000000; pacman_update(&g,now,0); assert(g.accumulator < 20);
    /* Exercise actual ghost decisions and collision paths under random input. */
    start(); for (int i = 0; i < 4; ++i) g.ghosts[i].wait = 0;
    uint32_t random = 42;
    for (unsigned n = 0; n < 30000; ++n) {
        random = random*1664525U+1013904223U;
        unsigned keys = 1U<<((random>>24)%4);
        if (g.state == GAME_OVER) keys = PAC_CENTER;
        advance(20,keys);
        assert(g.player.x >= 0 && g.player.x < MAZE_W*8);
        assert(!pacman_wall(g.player.x/8,g.player.y/8));
        for (int i = 0; i < 4; ++i) {
            assert(g.ghosts[i].x >= 0 && g.ghosts[i].x < MAZE_W*8);
            assert(!pacman_wall(g.ghosts[i].x/8,g.ghosts[i].y/8));
        }
        if (n%1000 == 0) preview(NULL);
    }
    start(); for (int i = 0; i < 4; ++i) g.ghosts[i].wait = 0;
    advance(1500,PAC_LEFT); preview("out/pacman-play.ppm");
    puts("PASS: connected maze, pellets, buffered turns, walls, tunnel, power, ghosts, lives, pause, restart, level clear, timer wrap, 30000 updates, framebuffer guards.");
    return 0;
}


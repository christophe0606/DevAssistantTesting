#include "pacman.h"
#include <stdlib.h>
#define RGB(r,g,b) ((uint16_t)(((r)>>3)<<11 | ((g)>>2)<<5 | ((b)>>3)))
#define BG RGB(3,5,14)
#define WHITE RGB(240,245,255)
#define MUTED RGB(149,170,203)
#define YELLOW RGB(255,222,28)
#define BLUE RGB(30,89,255)
#define CELL 24
#define MX 12
#define MY 120
static const uint8_t alphabet[36][7] = {
    {14,17,19,21,25,17,14}, {4,12,4,4,4,4,14},
    {14,17,1,2,4,8,31}, {30,1,1,14,1,1,30},
    {2,6,10,18,31,2,2}, {31,16,16,30,1,1,30},
    {14,16,16,30,17,17,14}, {31,1,2,4,8,8,8},
    {14,17,17,14,17,17,14}, {14,17,17,15,1,1,14},
    {14,17,17,31,17,17,17}, {30,17,17,30,17,17,30},
    {14,17,16,16,16,17,14}, {30,17,17,17,17,17,30},
    {31,16,16,30,16,16,31}, {31,16,16,30,16,16,16},
    {14,17,16,23,17,17,15}, {17,17,17,31,17,17,17},
    {14,4,4,4,4,4,14}, {7,2,2,2,2,18,12},
    {17,18,20,24,20,18,17}, {16,16,16,16,16,16,31},
    {17,27,21,21,17,17,17}, {17,25,21,19,17,17,17},
    {14,17,17,17,17,17,14}, {30,17,17,30,16,16,16},
    {14,17,17,17,21,18,13}, {30,17,17,30,20,18,17},
    {15,16,16,14,1,1,30}, {31,4,4,4,4,4,4},
    {17,17,17,17,17,17,14}, {17,17,17,17,17,10,4},
    {17,17,17,21,21,21,10}, {17,17,10,4,10,17,17},
    {17,17,10,4,4,4,4}, {31,1,2,4,8,16,31}
};

static void rect(uint16_t *fb, int x, int y, int w, int h, uint16_t color)
{
    int right = x + w, bottom = y + h;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (right > GAME_LCD_WIDTH) right = GAME_LCD_WIDTH;
    if (bottom > GAME_LCD_HEIGHT) bottom = GAME_LCD_HEIGHT;
    for (int yy = y; yy < bottom; ++yy)
        for (int xx = x; xx < right; ++xx) fb[yy * GAME_LCD_WIDTH + xx] = color;
}

static void outline(uint16_t *fb, int x, int y, int w, int h, uint16_t color)
{
    rect(fb,x,y,w,2,color); rect(fb,x,y+h-2,w,2,color);
    rect(fb,x,y,2,h,color); rect(fb,x+w-2,y,2,h,color);
}

static void text(uint16_t *fb, int x, int y, const char *s, int scale, uint16_t color)
{
    for (; *s; ++s, x += 6 * scale) {
        int glyph = *s >= '0' && *s <= '9' ? *s - '0' :
                    *s >= 'A' && *s <= 'Z' ? *s - 'A' + 10 : -1;
        if (glyph < 0) continue;
        for (int row = 0; row < 7; ++row)
            for (int col = 0; col < 5; ++col)
                if (alphabet[glyph][row] & (1U << (4-col)))
                    rect(fb,x+col*scale,y+row*scale,scale,scale,color);
    }
}

static void number(uint16_t *fb, int x, int y, uint32_t value, int scale)
{
    char digits[11], out[11];
    int n = 0;
    do { digits[n++] = (char)('0' + value % 10); value /= 10; } while (value);
    for (int i = 0; i < n; ++i) out[i] = digits[n-1-i];
    out[n] = 0;
    /* Keep very large scores inside the sidebar. */
    if (n * 6 * scale > 126) scale = 1;
    text(fb,x,y,out,scale,WHITE);
}


static void circle(uint16_t *fb, int x, int y, int radius, uint16_t color)
{
    for (int yy = -radius; yy <= radius; ++yy)
        for (int xx = -radius; xx <= radius; ++xx)
            if (xx*xx+yy*yy <= radius*radius) rect(fb,x+xx,y+yy,1,1,color);
}
static void player(uint16_t *fb, int x, int y, Direction dir, bool mouth)
{
    for (int yy = -10; yy <= 10; ++yy)
        for (int xx = -10; xx <= 10; ++xx) {
            if (xx*xx+yy*yy > 100) continue;
            int forward = dir == LEFT ? -xx : dir == RIGHT ? xx : dir == UP ? -yy : yy;
            int side = dir == LEFT || dir == RIGHT ? yy : xx;
            if (mouth && forward >= 0 && abs(side)*2 <= forward) continue;
            rect(fb,x+xx,y+yy,1,1,YELLOW);
        }
}
static void ghost(uint16_t *fb, int x, int y, const Actor *a, uint16_t color, bool afraid, bool phase)
{
    circle(fb,x,y-2,10,color);
    rect(fb,x-10,y-2,21,10,color);
    for (int i = -9; i <= 9; i += 6) circle(fb,x+i+(phase ? 0 : 1),y+7,2,color);
    if (afraid) {
        rect(fb,x-5,y-3,3,3,WHITE); rect(fb,x+3,y-3,3,3,WHITE);
        for (int i = -6; i <= 6; ++i) rect(fb,x+i,y+4+(abs(i)%4 < 2),1,2,WHITE);
    } else {
        circle(fb,x-4,y-3,4,WHITE); circle(fb,x+5,y-3,4,WHITE);
        int ex = a->dir == LEFT ? -1 : a->dir == RIGHT ? 1 : 0;
        int ey = a->dir == UP ? -1 : a->dir == DOWN ? 1 : 0;
        rect(fb,x-5+ex,y-4+ey,3,4,BLUE); rect(fb,x+4+ex,y-4+ey,3,4,BLUE);
    }
}
static void centered(uint16_t *fb, int y, const char *s, int scale, uint16_t color)
{
    int len = 0; while (s[len]) ++len;
    text(fb,(GAME_LCD_WIDTH-(len*6-1)*scale)/2,y,s,scale,color);
}
void pacman_render(uint16_t *fb, const Pacman *g)
{
    static const uint16_t colors[4] = {RGB(255,65,79),RGB(255,158,208),RGB(45,225,246),RGB(255,165,53)};
    rect(fb,0,0,GAME_LCD_WIDTH,GAME_LCD_HEIGHT,BG);
    text(fb,16,18,"PAC MAN",4,YELLOW);
    text(fb,314,20,"LEVEL",2,MUTED); number(fb,394,20,g->level,2);
    text(fb,16,65,"SCORE",2,MUTED); number(fb,96,65,g->score,2);
    text(fb,260,65,"BEST",2,MUTED); number(fb,326,65,g->best,2);
    rect(fb,12,99,456,2,RGB(29,43,71));
    for (int y = 0; y < MAZE_H; ++y)
        for (int x = 0; x < MAZE_W; ++x) {
            int px = MX+x*CELL, py = MY+y*CELL;
            if (pacman_wall(x,y)) {
                rect(fb,px,py,CELL,CELL,RGB(6,17,53));
                if (!pacman_wall(x-1,y)) rect(fb,px+1,py,2,CELL,BLUE);
                if (!pacman_wall(x+1,y)) rect(fb,px+21,py,2,CELL,BLUE);
                if (!pacman_wall(x,y-1)) rect(fb,px,py+1,CELL,2,BLUE);
                if (!pacman_wall(x,y+1)) rect(fb,px,py+21,CELL,2,BLUE);
            } else if (g->dots[y][x] == 1) circle(fb,px+12,py+12,2,RGB(255,220,170));
            else if (g->dots[y][x] == 2) circle(fb,px+12,py+12,(g->clock_ms/200)%2 ? 6 : 5,WHITE);
        }
    outline(fb,MX,MY,MAZE_W*CELL,MAZE_H*CELL,BLUE);
    /* Keep the wrap tunnel visibly open on both edges. */
    rect(fb,MX,MY+12*CELL,3,CELL,BG);
    rect(fb,MX+MAZE_W*CELL-3,MY+12*CELL,3,CELL,BG);
    for (int i = 0; i < 4; ++i) {
        const Actor *a = &g->ghosts[i];
        bool afraid = g->power_ms && !a->wait;
        uint16_t color = afraid ? (g->power_ms < 1800 && (g->clock_ms/160)%2 ? RGB(120,150,255) : BLUE) : colors[i];
        int x = MX+a->x*3, y = MY+a->y*3;
        ghost(fb,x,y,a,color,afraid,(g->clock_ms/120)%2);
        if (a->x < 4) ghost(fb,x+MAZE_W*CELL,y,a,color,afraid,false);
        if (a->x > MAZE_W*8-4) ghost(fb,x-MAZE_W*CELL,y,a,color,afraid,false);
    }
    int px = MX+g->player.x*3, py = MY+g->player.y*3;
    bool mouth = (g->clock_ms/100)%2 == 0;
    if (g->state != GAME_DYING || (g->state_ms/100)%2) {
        player(fb,px,py,g->player.dir,mouth);
        if (g->player.x < 4) player(fb,px+MAZE_W*CELL,py,g->player.dir,mouth);
        if (g->player.x > MAZE_W*8-4) player(fb,px-MAZE_W*CELL,py,g->player.dir,mouth);
    }
    text(fb,16,737,"LIVES",2,MUTED);
    for (unsigned i = 0; i < g->lives; ++i) player(fb,105+(int)i*28,744,RIGHT,true);
    if (g->power_ms) {
        text(fb,226,737,"POWER",2,YELLOW);
        rect(fb,304,739,(int)(g->power_ms*150/6500),10,BLUE);
    }
    centered(fb,774,"JOYSTICK MOVE   CENTER PAUSE",2,MUTED);
    if (g->state != GAME_PLAYING || g->state_ms) {
        const char *title = g->state == GAME_READY ? "READY" : g->state == GAME_PAUSED ? "PAUSED" :
            g->state == GAME_OVER ? "GAME OVER" : g->state == GAME_LEVEL_CLEAR ? "MAZE CLEAR" :
            g->state == GAME_DYING ? "CAUGHT" : "GET READY";
        rect(fb,52,348,376,130,BG); outline(fb,52,348,376,130,YELLOW);
        centered(fb,370,title,4,YELLOW);
        const char *hint = g->state == GAME_READY ? "PRESS CENTER TO START" :
            g->state == GAME_PAUSED ? "CENTER TO RESUME" : g->state == GAME_OVER ? "CENTER TO PLAY AGAIN" :
            g->state == GAME_LEVEL_CLEAR ? "NEXT LEVEL" : g->state == GAME_DYING ? "WATCH THE GHOSTS" : "EAT ALL THE DOTS";
        centered(fb,432,hint,2,WHITE);
    }
}

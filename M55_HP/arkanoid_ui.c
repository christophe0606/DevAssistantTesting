#include "arkanoid_ui.h"
#include <math.h>

#define RGB(r,g,b) ((uint16_t)(((r)>>3)<<11 | ((g)>>2)<<5 | ((b)>>3)))
#define BG RGB(10,16,29)
#define PANEL RGB(18,28,45)
#define GRID RGB(28,43,62)
#define WHITE RGB(235,243,255)
#define MUTED RGB(135,159,187)
#define ACCENT RGB(66,222,209)
static const uint16_t palette[6] = {
    RGB(245,92,119), RGB(249,151,69), RGB(251,210,80),
    RGB(80,211,155), RGB(66,179,235), RGB(163,118,244)
};
/* Original compact 5x7 bitmap alphabet; each byte is one top-to-bottom row. */
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

static uint16_t darker(uint16_t c)
{
    return (uint16_t)((c & 0xF7DEU) >> 1);
}

static void disc(uint16_t *fb, int x, int y, int radius, uint16_t color)
{
    for (int dy = -radius; dy <= radius; ++dy) {
        int half = (int)sqrtf((float)(radius*radius-dy*dy));
        rect(fb,x-half,y+dy,half*2+1,1,color);
    }
}

static void centered(uint16_t *fb, int y, const char *label, int scale, uint16_t color)
{
    unsigned length = 0;
    for (const char *p = label; *p; ++p) ++length;
    text(fb,(GAME_LCD_WIDTH-(int)length*6*scale+scale)/2,y,label,scale,color);
}

void arkanoid_render(uint16_t *fb, const Arkanoid *g)
{
    rect(fb,0,0,GAME_LCD_WIDTH,GAME_LCD_HEIGHT,BG);
    text(fb,24,25,"ARKANOID",4,WHITE);
    text(fb,330,22,"BEST",1,MUTED);
    number(fb,330,39,g->best,2);
    rect(fb,24,75,182,44,PANEL);
    rect(fb,216,75,114,44,PANEL);
    rect(fb,340,75,116,44,PANEL);
    text(fb,34,82,"SCORE",1,MUTED); number(fb,34,98,g->score,2);
    text(fb,228,82,"LEVEL",1,MUTED); number(fb,228,98,g->level,2);
    text(fb,352,82,"LIVES",1,MUTED);
    for (uint32_t i = 0; i < g->lives; ++i) disc(fb,358+(int)i*24,106,5,ACCENT);

    rect(fb,18,128,444,598,GRID);
    rect(fb,20,130,440,594,BG);
    /* Quiet starfield behind the play area. */
    for (int i = 0; i < 60; ++i) {
        int x = 28 + (i*137)%420, y = 144 + (i*71)%558;
        rect(fb,x,y,1,1,GRID);
    }
    for (int y = 142; y < 700; y += 64) {
        rect(fb,18,y,3,18,ACCENT); rect(fb,459,y,3,18,ACCENT);
    }

    for (int row = 0; row < BRICK_ROWS; ++row)
        for (int col = 0; col < BRICK_COLS; ++col) {
            int x = (int)(BRICK_X+col*BRICK_PITCH_X);
            int y = (int)(BRICK_Y+row*BRICK_PITCH_Y);
            if (g->bricks[row][col]) {
                uint16_t color = palette[row];
                rect(fb,x+2,y+3,(int)BRICK_W,(int)BRICK_H,RGB(3,8,15));
                rect(fb,x,y,(int)BRICK_W,(int)BRICK_H,darker(color));
                rect(fb,x+2,y+2,(int)BRICK_W-4,(int)BRICK_H-5,color);
                rect(fb,x+3,y+2,(int)BRICK_W-6,2,WHITE);
                if (g->bricks[row][col] > 1) {
                    rect(fb,x+17,y+9,5,5,WHITE); rect(fb,x+26,y+9,5,5,WHITE);
                }
            }
            if (g->flash_ms && row == g->flash_row && col == g->flash_col)
                outline(fb,x-1,y-1,(int)BRICK_W+2,(int)BRICK_H+2,WHITE);
        }

    if (g->power_active) {
        int x = (int)g->power_x, y = (int)g->power_y;
        rect(fb,x-15,y-8,30,16,ACCENT);
        text(fb,x-9,y-3,"W",1,BG);
        rect(fb,x+2,y-3,9,2,BG); rect(fb,x+2,y+2,9,2,BG);
    }
    if (g->state == ARK_PLAYING || g->state == ARK_PAUSED) {
        for (int i = 7; i >= 0; --i) {
            if (g->trail_y[i] > COURT_BOTTOM-2) continue;
            disc(fb,(int)g->trail_x[i],(int)g->trail_y[i],i < 4 ? 4 : 2,
                 i < 4 ? RGB(53,100,138) : GRID);
        }
    }
    if (g->ball_y <= COURT_BOTTOM-BALL_RADIUS) {
        disc(fb,(int)g->ball_x,(int)g->ball_y,7,RGB(252,212,89));
        disc(fb,(int)g->ball_x-1,(int)g->ball_y-2,4,WHITE);
    }
    int width = (int)arkanoid_paddle_width(g);
    int px = (int)g->paddle_x-width/2;
    rect(fb,px+3,(int)PADDLE_Y+5,width,(int)PADDLE_H,RGB(2,7,14));
    rect(fb,px,(int)PADDLE_Y,width,(int)PADDLE_H,RGB(87,151,172));
    rect(fb,px+3,(int)PADDLE_Y+2,width-6,6,WHITE);
    rect(fb,px,(int)PADDLE_Y,12,(int)PADDLE_H,ACCENT);
    rect(fb,px+width-12,(int)PADDLE_Y,12,(int)PADDLE_H,ACCENT);
    if (g->wide_ms) rect(fb,px,713,(int)((uint32_t)width*g->wide_ms/12000U),3,ACCENT);

    centered(fb,746,"LEFT RIGHT MOVE   DOWN SLOW",2,MUTED);
    centered(fb,775,"CENTER LAUNCH OR PAUSE",2,ACCENT);
    if (g->state != ARK_PLAYING) {
        rect(fb,64,389,352,145,PANEL);
        outline(fb,64,389,352,145,ACCENT);
        const char *title = "READY PLAYER";
        const char *detail = "BREAK EVERY BRICK";
        if (g->state == ARK_PAUSED) { title = "PAUSED"; detail = "PRESS CENTER TO RESUME"; }
        else if (g->state == ARK_SERVE) { title = "TRY AGAIN"; detail = "AIM YOUR PADDLE"; }
        else if (g->state == ARK_LEVEL_CLEAR) { title = "LEVEL CLEAR"; detail = "PRESS CENTER FOR NEXT"; }
        else if (g->state == ARK_GAME_OVER) { title = "GAME OVER"; detail = "PRESS CENTER TO RETRY"; }
        centered(fb,412,title,3,WHITE);
        centered(fb,461,detail,2,MUTED);
        centered(fb,499,g->state == ARK_READY || g->state == ARK_SERVE ?
                 "PRESS CENTER TO LAUNCH" : "ARKANOID",2,ACCENT);
    }
}

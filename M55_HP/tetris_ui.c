#include "tetris_ui.h"

#define RGB(r,g,b) ((uint16_t)(((r)>>3)<<11 | ((g)>>2)<<5 | ((b)>>3)))
#define BG RGB(12,18,32)
#define PANEL RGB(20,30,48)
#define GRID RGB(26,39,57)
#define WHITE RGB(231,239,250)
#define MUTED RGB(135,159,187)
#define ACCENT RGB(76,222,199)
#define CELL 28
#define WELL_X 24
#define WELL_Y 142

static const uint16_t palette[8] = {
    PANEL, RGB(41,207,231), RGB(250,208,67), RGB(177,113,250),
    RGB(94,211,131), RGB(247,95,117), RGB(78,132,248), RGB(251,157,66)
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

static void block(uint16_t *fb, int x, int y, int size, uint16_t color, bool ghost)
{
    if (ghost) {
        outline(fb,x+3,y+3,size-6,size-6,darker(color));
        return;
    }
    rect(fb,x+1,y+1,size-2,size-2,darker(color));
    rect(fb,x+2,y+2,size-5,size-5,color);
    rect(fb,x+3,y+3,size-7,2,WHITE);
}

static void active_piece(uint16_t *fb, const Tetris *g, int piece_y, bool ghost)
{
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x) {
            int row = piece_y + y - TETRIS_HIDDEN;
            if (row < 0 || row >= 20) continue;
            if (tetris_cell(g->piece,g->rotation,x,y))
                block(fb,WELL_X+(g->x+x)*CELL,WELL_Y+row*CELL,CELL,
                      palette[g->piece+1],ghost);
        }
}

void tetris_render(uint16_t *fb, const Tetris *g)
{
    rect(fb,0,0,GAME_LCD_WIDTH,GAME_LCD_HEIGHT,BG);
    text(fb,24,26,"TETRIS",5,WHITE);
    text(fb,326,35,"ALIF",3,ACCENT);
    text(fb,326,65,"DEVKIT E8",1,MUTED);
    rect(fb,24,94,432,2,GRID);
    text(fb,24,112,"FALLING BLOCKS",2,ACCENT);

    rect(fb,WELL_X-3,WELL_Y-3,286,566,GRID);
    rect(fb,WELL_X,WELL_Y,280,560,PANEL);
    for (int y = 0; y < 20; ++y)
        for (int x = 0; x < 10; ++x) {
            rect(fb,WELL_X+x*CELL,WELL_Y+y*CELL,CELL-1,CELL-1,BG);
            uint8_t cell = g->board[y+TETRIS_HIDDEN][x];
            if (cell) block(fb,WELL_X+x*CELL,WELL_Y+y*CELL,CELL,palette[cell],false);
        }
    if (g->state == GAME_PLAYING || g->state == GAME_PAUSED) {
        active_piece(fb,g,tetris_landing_y(g),true);
        active_piece(fb,g,g->y,false);
    }
    text(fb,326,150,"SCORE",2,MUTED); number(fb,326,179,g->score,2);
    text(fb,326,225,"LEVEL",2,MUTED); number(fb,326,254,g->level,3);
    text(fb,326,304,"LINES",2,MUTED); number(fb,326,333,g->lines,2);
    text(fb,326,382,"NEXT",2,MUTED);
    rect(fb,324,411,132,100,PANEL);
    if (g->state != GAME_READY) {
        for (int y = 0; y < 4; ++y)
            for (int x = 0; x < 4; ++x)
                if (tetris_cell(g->next,0,x,y))
                    block(fb,340+x*24,426+y*24,24,palette[g->next+1],false);
    }
    text(fb,326,548,"BEST",2,MUTED); number(fb,326,577,g->best,2);
    text(fb,326,638,"7 BAG",2,ACCENT);
    text(fb,326,664,"RANDOMIZER",1,MUTED);

    text(fb,24,726,"LEFT RIGHT MOVE   UP ROTATE",2,MUTED);
    text(fb,24,750,"DOWN SOFT DROP  PRESS HARD DROP",2,MUTED);
    text(fb,24,774,"HOLD CENTER TO PAUSE",2,ACCENT);
    if (g->state != GAME_PLAYING) {
        rect(fb,40,316,248,174,BG);
        outline(fb,40,316,248,174,ACCENT);
        const char *title = g->state == GAME_READY ? "READY" :
                            g->state == GAME_PAUSED ? "PAUSED" : "GAME OVER";
        int title_x = g->state == GAME_OVER ? 56 : g->state == GAME_PAUSED ? 92 : 104;
        text(fb,title_x,344,title,3,WHITE);
        text(fb,68,405,"PRESS CENTER",2,ACCENT);
        text(fb,92,436,g->state == GAME_READY ? "TO START" :
             g->state == GAME_PAUSED ? "TO RESUME" : "TO RETRY",2,MUTED);
    }
}

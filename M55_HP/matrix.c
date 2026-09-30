#include "matrix.h"
#include <string.h>

/* Original 5x7 digits and angular code symbols. The latter are abstract
 * glyphs, with optional mirroring, rather than text that needs translating. */
static const uint8_t glyphs[MATRIX_GLYPHS][7] = {
    {14,17,19,21,25,17,14}, {4,12,4,4,4,4,14},
    {14,17,1,2,4,8,31}, {30,1,1,14,1,1,30},
    {2,6,10,18,31,2,2}, {31,16,16,30,1,1,30},
    {14,16,16,30,17,17,14}, {31,1,2,4,8,8,8},
    {14,17,17,14,17,17,14}, {14,17,17,15,1,1,14},
    {31,4,4,4,4,4,4}, {17,17,17,31,17,17,17},
    {31,16,16,30,16,16,31}, {17,17,10,4,10,17,17},
    {17,25,21,19,17,17,17}, {14,17,17,17,17,17,14},
    {31,1,2,4,8,16,31}, {30,17,17,30,16,16,16},
    {4,31,4,4,4,8,16}, {0,31,1,2,4,8,0},
    {4,4,31,4,4,4,0}, {17,17,1,2,4,8,0},
    {16,16,31,17,2,4,8}, {31,1,1,31,1,1,31},
    {2,2,31,2,6,10,18}, {31,0,31,4,4,8,16},
    {4,4,4,4,4,10,17}, {0,31,17,17,1,2,4},
    {4,31,17,1,2,4,8}, {1,2,4,12,20,4,4},
    {0,31,4,4,4,4,31}, {8,8,15,9,9,17,1},
    {4,21,21,4,4,8,16}, {0,17,9,1,2,4,24},
    {31,4,4,31,4,4,4}, {1,1,1,1,1,2,28},
    {16,16,16,16,17,18,28}, {0,31,0,31,1,2,12},
    {4,8,16,17,17,31,1}, {8,8,31,9,9,17,2},
    {0,31,2,4,8,20,2}, {4,4,31,4,10,17,4},
    {17,10,4,10,17,0,31}, {31,17,17,31,17,17,31},
    {16,8,4,31,4,8,16}, {4,14,21,4,21,14,4},
    {31,16,8,4,2,1,31}, {1,3,5,9,17,31,1}
};

static uint32_t random_next(MatrixRain *m)
{
    uint32_t x = m->random;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return m->random = x;
}

static uint8_t random_glyph(MatrixRain *m)
{
    uint32_t r = random_next(m);
    return (uint8_t)((r % MATRIX_GLYPHS) | ((r >> 16) & 0x40U));
}

static void reset_stream(MatrixRain *m, unsigned layer, unsigned col, int initial)
{
    MatrixStream *s = &m->streams[layer][col];
    s->length = (uint8_t)((layer ? 8U : 14U) + random_next(m) % 18U);
    s->speed_q8 = ((layer ? 3U : 8U) + random_next(m) % (layer ? 5U : 12U)) * 256U;
    s->brightness = (uint8_t)((layer ? 48U : 180U) + random_next(m) % (layer ? 56U : 76U));
    s->remainder = 0;
    /* Populate the initial image immediately, then leave varied gaps
     * between subsequent streams instead of resetting the whole screen. */
    s->head_q8 = initial ? (int32_t)(random_next(m) % (MATRIX_ROWS*256U)) :
                          -(int32_t)((2U+random_next(m)%18U)*256U);
}

void matrix_init(MatrixRain *m, uint32_t seed, uint32_t now)
{
    memset(m,0,sizeof(*m));
    m->random = seed ? seed : 0x4D415452U;
    m->last_ms = now;
    for (unsigned col = 0; col < MATRIX_COLS; ++col) {
        for (unsigned row = 0; row < MATRIX_ROWS; ++row)
            m->glyphs[col][row] = random_glyph(m);
        for (unsigned layer = 0; layer < MATRIX_LAYERS; ++layer)
            reset_stream(m,layer,col,1);
    }
}

void matrix_update(MatrixRain *m, uint32_t now)
{
    uint32_t elapsed = now-m->last_ms;
    m->last_ms = now;
    if (!elapsed) return;
    if (elapsed > 250U) elapsed = 250U;
    for (unsigned layer = 0; layer < MATRIX_LAYERS; ++layer)
        for (unsigned col = 0; col < MATRIX_COLS; ++col) {
            MatrixStream *s = &m->streams[layer][col];
            int32_t before = s->head_q8;
            uint32_t delta = s->speed_q8*elapsed+s->remainder;
            s->head_q8 += (int32_t)(delta/1000U);
            s->remainder = delta%1000U;
            /* Give each newly reached cell a fresh symbol. */
            int first = before < 0 ? 0 : before/256+1;
            int last = s->head_q8/256;
            if (last >= MATRIX_ROWS) last = MATRIX_ROWS-1;
            if (s->head_q8 >= 0)
                for (int row = first; row <= last; ++row)
                    m->glyphs[col][row] = random_glyph(m);
            if (s->head_q8 >= (MATRIX_ROWS+s->length)*256) {
                reset_stream(m,layer,col,0);
                ++m->respawns;
            }
        }
    m->mutation_ms += elapsed;
    while (m->mutation_ms >= 80U) {
        m->mutation_ms -= 80U;
        for (unsigned i = 0; i < 24; ++i) {
            unsigned col = random_next(m)%MATRIX_COLS;
            unsigned row = random_next(m)%MATRIX_ROWS;
            m->glyphs[col][row] = random_glyph(m);
        }
    }
    ++m->updates;
}

static uint16_t rgb(unsigned r, unsigned g, unsigned b)
{
    return (uint16_t)((r>>3)<<11 | (g>>2)<<5 | (b>>3));
}

static void paint(uint16_t *fb, int x, int y, int width, int height, uint16_t color)
{
    /* Glyph and glow bounds are guaranteed inside their 16x16 cell. */
    for (int yy = y; yy < y+height; ++yy)
        for (int xx = x; xx < x+width; ++xx)
            fb[yy*MATRIX_WIDTH+xx] = color;
}

static void draw_glyph(uint16_t *fb, unsigned col, unsigned row, uint8_t glyph,
                       unsigned brightness, unsigned highlight)
{
    int x = (int)col*MATRIX_CELL+3, y = (int)row*MATRIX_CELL+1;
    const uint8_t *shape = glyphs[glyph&0x3FU];
    uint16_t glow = rgb(0,brightness/7,brightness/28);
    uint16_t core = rgb(brightness/32 + highlight*3/4,
                        brightness, brightness/7 + highlight*2/3);
    /* Two passes keep neighboring glow pixels from dimming glyph cores. */
    for (int pass = 0; pass < 2; ++pass)
        for (int gy = 0; gy < 7; ++gy)
            for (int gx = 0; gx < 5; ++gx) {
                unsigned bit = (glyph & 0x40U) ? (unsigned)gx : (unsigned)(4-gx);
                if (shape[gy] & (1U<<bit)) {
                    if (!pass) paint(fb,x+gx*2-1,y+gy*2-1,4,4,glow);
                    else paint(fb,x+gx*2,y+gy*2,2,2,core);
                }
            }
}

void matrix_render(uint16_t *fb, const MatrixRain *m)
{
    memset(fb,0,MATRIX_WIDTH*MATRIX_HEIGHT*sizeof(*fb));
    for (unsigned col = 0; col < MATRIX_COLS; ++col)
        for (unsigned row = 0; row < MATRIX_ROWS; ++row) {
            unsigned brightest = 0, highlight = 0;
            for (unsigned layer = 0; layer < MATRIX_LAYERS; ++layer) {
                const MatrixStream *s = &m->streams[layer][col];
                int32_t distance = s->head_q8-(int32_t)row*256;
                int32_t tail = (int32_t)s->length*256;
                if (distance < 0 || distance >= tail) continue;
                unsigned fade = (unsigned)(tail-distance)*255U/(unsigned)tail;
                unsigned light = (unsigned)s->brightness*fade*fade/(255U*255U);
                if (light > brightest) brightest = light;
                if (!layer && distance < 256) {
                    highlight = 170U;
                    brightest = 255U;
                }
            }
            if (brightest >= 8)
                draw_glyph(fb,col,row,m->glyphs[col][row],brightest,highlight);
        }
}

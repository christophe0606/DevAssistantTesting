#include "matrix.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint16_t guarded[MATRIX_WIDTH*MATRIX_HEIGHT+2];

static uint32_t render_check(const MatrixRain *rain, const char *path)
{
    guarded[0] = 0x1234;
    guarded[MATRIX_WIDTH*MATRIX_HEIGHT+1] = 0x5678;
    memset(guarded+1,0xFF,MATRIX_WIDTH*MATRIX_HEIGHT*2);
    matrix_render(guarded+1,rain);
    assert(guarded[0] == 0x1234 && guarded[MATRIX_WIDTH*MATRIX_HEIGHT+1] == 0x5678);
    uint32_t hash = 2166136261U, lit = 0;
    FILE *f = path ? fopen(path,"wb") : NULL;
    if (path) { assert(f); fprintf(f,"P6\n480 800\n255\n"); }
    for (unsigned i = 1; i <= MATRIX_WIDTH*MATRIX_HEIGHT; ++i) {
        uint16_t color = guarded[i];
        unsigned r = ((color>>11)&31)*255/31;
        unsigned g = ((color>>5)&63)*255/63;
        unsigned b = (color&31)*255/31;
        assert(g >= r && g >= b); /* Entire palette stays green. */
        if (color) ++lit;
        hash = (hash^color)*16777619U;
        if (f) { fputc(r,f); fputc(g,f); fputc(b,f); }
    }
    if (f) fclose(f);
    if (path) assert(lit > 10000 && lit < MATRIX_WIDTH*MATRIX_HEIGHT/2);
    return hash;
}

int main(void)
{
    MatrixRain rain, same;
    matrix_init(&rain,12345,0);
    matrix_init(&same,12345,0);
    assert(memcmp(&rain,&same,sizeof(rain)) == 0);
    uint32_t first = render_check(&rain,"out/matrix-initial.ppm");
    matrix_update(&rain,33);
    assert(render_check(&rain,NULL) != first);

    /* Fractional speed is retained, independent of update granularity. */
    matrix_init(&rain,42,0); same = rain;
    matrix_update(&rain,60);
    matrix_update(&same,20); matrix_update(&same,40); matrix_update(&same,60);
    for (unsigned l = 0; l < MATRIX_LAYERS; ++l)
        for (unsigned c = 0; c < MATRIX_COLS; ++c)
            assert(rain.streams[l][c].head_q8 == same.streams[l][c].head_q8);

    matrix_init(&rain,0,UINT32_MAX-10U);
    int32_t before = rain.streams[0][0].head_q8;
    matrix_update(&rain,22);
    assert(rain.streams[0][0].head_q8 > before); /* Timer wrap. */
    assert(rain.random != 0);
    for (uint32_t frame = 1; frame <= 20000; ++frame) {
        matrix_update(&rain,22+frame*33);
        for (unsigned l = 0; l < MATRIX_LAYERS; ++l)
            for (unsigned c = 0; c < MATRIX_COLS; ++c) {
                const MatrixStream *s = &rain.streams[l][c];
                assert(s->head_q8 >= -19*256);
                assert(s->head_q8 < (MATRIX_ROWS+s->length)*256);
                assert(s->remainder < 1000);
            }
        for (unsigned c = 0; c < MATRIX_COLS; ++c)
            for (unsigned r = 0; r < MATRIX_ROWS; ++r)
                assert((rain.glyphs[c][r]&0x3FU) < MATRIX_GLYPHS);
        if (frame == 90) render_check(&rain,"out/matrix-rain.ppm");
        if (frame % 2000 == 0) render_check(&rain,NULL);
    }
    assert(rain.respawns > 1000);
    /* Exercise glow at every framebuffer edge, on both layers. */
    for (unsigned l = 0; l < MATRIX_LAYERS; ++l)
        for (unsigned c = 0; c < MATRIX_COLS; ++c) {
            rain.streams[l][c].head_q8 = (MATRIX_ROWS-1)*256;
            rain.streams[l][c].length = MATRIX_ROWS;
        }
    render_check(&rain,NULL);
    puts("PASS: animation, deterministic seed, frame timing, timer wrap, 20000 updates, respawns, glyph bounds, green palette, framebuffer guards");
    return 0;
}

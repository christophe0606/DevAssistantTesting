#include "spectrum.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int16_t pcm[SPECTRUM_FFT_SIZE];
static unsigned peak(void)
{
    unsigned p = 1;
    for (unsigned i = 2; i < SPECTRUM_BINS; ++i)
        if (spectrum_magnitude[i] > spectrum_magnitude[p]) p = i;
    return p;
}
int main(void)
{
    assert(spectrum_init() == 0);
    spectrum_compute(pcm);
    for (unsigned i = 0; i < SPECTRUM_BINS; ++i) assert(spectrum_magnitude[i] == 0);
    for (unsigned i = 0; i < SPECTRUM_FFT_SIZE; ++i) pcm[i] = 12000;
    spectrum_compute(pcm);
    for (unsigned i = 0; i < SPECTRUM_BINS; ++i) assert(spectrum_magnitude[i] < 1e-6f);
    const unsigned bins[] = {1, 43, 320, 1000};
    for (unsigned t = 0; t < sizeof(bins)/sizeof(bins[0]); ++t) {
        for (unsigned i = 0; i < SPECTRUM_FFT_SIZE; ++i)
            pcm[i] = (int16_t)(16384.0 * sin(6.283185307179586 * bins[t] * i / SPECTRUM_FFT_SIZE));
        spectrum_compute(pcm);
        assert(peak() == bins[t]);
        assert(spectrum_peak_hz == bins[t] * SPECTRUM_SAMPLE_RATE / SPECTRUM_FFT_SIZE);
        assert(fabsf(spectrum_magnitude[bins[t]] - 0.5f) < 0.001f);
    }
    for (unsigned i = 0; i < SPECTRUM_FFT_SIZE; ++i) pcm[i] = (i & 1) ? -16384 : 16384;
    spectrum_compute(pcm);
    assert(fabsf(spectrum_magnitude[SPECTRUM_BINS-1] - 0.5f) < 0.001f);
    size_t pixels = SPECTRUM_WIDTH * SPECTRUM_HEIGHT;
    uint16_t *storage = malloc((pixels + 2) * sizeof(uint16_t));
    assert(storage);
    storage[0] = storage[pixels+1] = 0x1234;
    for (unsigned mode = 0; mode < 2; ++mode) {
        memset(spectrum_magnitude, 0, sizeof(spectrum_magnitude));
        spectrum_magnitude[512] = 0.1f;
        spectrum_render(storage+1, mode != 0);
        unsigned column = (513 * SPECTRUM_WIDTH + SPECTRUM_BINS - 1) / SPECTRUM_BINS - 1;
        unsigned lit = 0;
        for (unsigned y = SPECTRUM_STRIP; y < SPECTRUM_HEIGHT; ++y)
            if (storage[1+column*SPECTRUM_HEIGHT+SPECTRUM_HEIGHT-1-y] == 0x15FB) ++lit;
        assert(lit >= (mode ? 341U : 45U) && lit <= (mode ? 343U : 46U));
        assert(storage[0] == 0x1234 && storage[pixels+1] == 0x1234);
        FILE *f = fopen(mode ? "out/spectrum-log.ppm" : "out/spectrum-linear.ppm", "wb");
        assert(f);
        fprintf(f, "P6\n800 480\n255\n");
        for (unsigned y = 0; y < SPECTRUM_HEIGHT; ++y)
            for (unsigned x = 0; x < SPECTRUM_WIDTH; ++x) {
                uint16_t c = storage[1+x*SPECTRUM_HEIGHT+SPECTRUM_HEIGHT-1-y];
                unsigned char rgb[] = {(unsigned char)(((c>>11)&31)*255/31),
                    (unsigned char)(((c>>5)&63)*255/63), (unsigned char)((c&31)*255/31)};
                fwrite(rgb, 1, 3, f);
            }
        fclose(f);
    }
    free(storage);
    puts("Spectrum checks passed: silence, DC, tones, amplitude, Nyquist, scales, framebuffer bounds.");
}


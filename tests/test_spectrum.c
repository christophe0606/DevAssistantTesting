#include "spectrum.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int16_t pcm[SPECTRUM_FFT_SIZE];
static uint16_t bar_color(unsigned from_bottom)
{
    const unsigned plot_height = SPECTRUM_HEIGHT - SPECTRUM_STRIP;
    if (from_bottom < plot_height / 2U) return 0x268A; /* green */
    if (from_bottom < plot_height * 4U / 5U) return 0xFD00; /* orange */
    return 0xF186; /* red */
}
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
        spectrum_magnitude[512] = mode ? 0.1f : 0.001f;
        spectrum_render(storage+1, mode != 0);
        unsigned column = (513 * SPECTRUM_PLOT_WIDTH + SPECTRUM_BINS - 1) / SPECTRUM_BINS - 1;
        unsigned lit = 0;
        for (unsigned y = SPECTRUM_STRIP; y < SPECTRUM_HEIGHT; ++y)
            if (storage[1+column*SPECTRUM_HEIGHT+SPECTRUM_HEIGHT-1-y] ==
                bar_color(SPECTRUM_HEIGHT-1-y)) ++lit;
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
    /* Loud input must saturate every column at the plot boundary, including
     * the first and last columns, without touching the indicator strip. */
    spectrum_display_gain = 2.0f;
    for (unsigned mode = 0; mode < 2; ++mode) {
        for (unsigned bin = 0; bin < SPECTRUM_BINS; ++bin)
            spectrum_magnitude[bin] = 10.0f;
        spectrum_render(storage + 1, mode != 0);
        for (unsigned x = 0; x < SPECTRUM_PLOT_WIDTH; ++x) {
            for (unsigned y = SPECTRUM_STRIP; y < SPECTRUM_HEIGHT; ++y)
                assert(storage[1+x*SPECTRUM_HEIGHT+SPECTRUM_HEIGHT-1-y] ==
                       bar_color(SPECTRUM_HEIGHT-1-y));
            assert(storage[1+x*SPECTRUM_HEIGHT+SPECTRUM_HEIGHT-SPECTRUM_STRIP] != 0xF186);
        }
        assert(storage[0] == 0x1234 && storage[pixels+1] == 0x1234);
    }
    /* Slider motion changes height in both modes, without toggling the mode.
     * A 40% baseline should become 20% at minimum and 80% at maximum. */
    for (unsigned mode = 0; mode < 2; ++mode) {
        for (unsigned endpoint = 0; endpoint < 2; ++endpoint) {
            assert(!spectrum_touch(SPECTRUM_WIDTH-1U,
                                   endpoint ? 0U : SPECTRUM_HEIGHT-1U, true));
            assert(spectrum_display_gain == (endpoint ? 2.0f : 0.5f));
            for (unsigned bin = 0; bin < SPECTRUM_BINS; ++bin)
                spectrum_magnitude[bin] = mode ? powf(10.0f, -48.0f / 20.0f) : 0.004f;
            spectrum_render(storage+1, mode != 0);
            unsigned lit = 0;
            for (unsigned row = 0; row < SPECTRUM_HEIGHT-SPECTRUM_STRIP; ++row)
                if (storage[1+row] == bar_color(row)) ++lit;
            unsigned expected = endpoint ? 364U : 91U;
            assert(lit >= expected-1U && lit <= expected+1U);
            /* Even when the spectrum saturates, the slider's outer edge
             * remains its own background, not a spectrum column. */
            assert(storage[1+(SPECTRUM_WIDTH-1U)*SPECTRUM_HEIGHT] == 0x10E5);
            assert(storage[0] == 0x1234 && storage[pixels+1] == 0x1234);
        }
    }
    float previous_gain = spectrum_display_gain;
    assert(spectrum_touch(0, 0, true));
    assert(!spectrum_touch(0, 0, false));
    assert(!spectrum_touch(SPECTRUM_WIDTH, SPECTRUM_HEIGHT, true));
    assert(spectrum_display_gain == previous_gain);
    spectrum_display_gain = 1.0f;
    free(storage);
    puts("Spectrum checks passed: silence, DC, tones, amplitude, Nyquist, scales, framebuffer bounds.");
}


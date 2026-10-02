#include "spectrum.h"
#include "arm_math.h"
#include <math.h>
#define RGB(r,g,b) ((uint16_t)(((r)>>3)<<11 | ((g)>>2)<<5 | ((b)>>3)))
float spectrum_display_gain = 1.0f;

bool spectrum_touch(unsigned x, unsigned y, bool new_contact)
{
    if (x >= SPECTRUM_WIDTH || y >= SPECTRUM_HEIGHT) return false;
    if (x < SPECTRUM_PLOT_WIDTH) return new_contact;
    if (y < SPECTRUM_SLIDER_TOP) y = SPECTRUM_SLIDER_TOP;
    if (y > SPECTRUM_SLIDER_BOTTOM) y = SPECTRUM_SLIDER_BOTTOM;
    spectrum_display_gain = 0.5f + 1.5f * (float)(SPECTRUM_SLIDER_BOTTOM - y) /
                          (float)(SPECTRUM_SLIDER_BOTTOM - SPECTRUM_SLIDER_TOP);
    return false;
}

static void rectangle(uint16_t *fb, unsigned x, unsigned y,
                      unsigned width, unsigned height, uint16_t color)
{
    for (unsigned col = x; col < x + width; ++col)
        arm_fill_q15((q15_t)color,
                     (q15_t *)(fb + col * SPECTRUM_HEIGHT + SPECTRUM_HEIGHT - y - height),
                     height);
}

/* Landscape coordinates map to the native portrait 480 x 800 buffer. */
static void pixel(uint16_t *fb, unsigned x, unsigned y, uint16_t color)
{
    fb[x * SPECTRUM_HEIGHT + (SPECTRUM_HEIGHT - 1U - y)] = color;
}

static void label(uint16_t *fb, unsigned x, const char *s)
{
    static const uint8_t glyphs[][7] = {
        {16,16,16,16,16,16,31}, /* L */
        {14,4,4,4,4,4,14},     /* I */
        {17,25,21,19,17,17,17},/* N */
        {31,16,16,30,16,16,31},/* E */
        {14,17,17,31,17,17,17},/* A */
        {30,17,17,30,20,18,17},/* R */
        {14,17,17,17,17,17,14},/* O */
        {14,17,16,23,17,17,15},/* G */
        {14,17,19,21,25,17,14},/* 0 */
        {4,12,4,4,4,4,14},     /* 1 */
        {14,17,1,2,4,8,31},    /* 2 */
        {2,6,10,18,31,2,2},    /* 4 */
        {14,16,16,30,17,17,14},/* 6 */
        {14,17,17,14,17,17,14},/* 8 */
        {17,18,20,24,20,18,17},/* K */
        {17,17,17,31,17,17,17},/* H */
        {31,1,2,4,8,16,31}     /* Z */
    };
    const char *letters = "LINEAROG012468KHZ";
    for (; *s; ++s, x += 12U) {
        unsigned g = 0;
        while (letters[g] && letters[g] != *s) ++g;
        if (!letters[g]) continue;
        for (unsigned r = 0; r < 7; ++r)
            for (unsigned c = 0; c < 5; ++c)
                if (glyphs[g][r] & (1U << (4U-c)))
                    for (unsigned dy = 0; dy < 2; ++dy)
                        for (unsigned dx = 0; dx < 2; ++dx)
                            pixel(fb, x+c*2U+dx, 5U+r*2U+dy, RGB(225,240,255));
    }
}

void spectrum_render(uint16_t *fb, bool logarithmic)
{
    const unsigned plot_height = SPECTRUM_HEIGHT - SPECTRUM_STRIP;
    const unsigned green_limit = plot_height / 2U;
    const unsigned orange_limit = plot_height * 4U / 5U;
    const float gain = fminf(2.0f, fmaxf(0.5f, spectrum_display_gain));
    for (unsigned x = 0; x < SPECTRUM_PLOT_WIDTH; ++x) {
        /* Keep the entire frequency range in the width left of the slider. */
        unsigned first = x * SPECTRUM_BINS / SPECTRUM_PLOT_WIDTH;
        unsigned end = (x+1U) * SPECTRUM_BINS / SPECTRUM_PLOT_WIDTH;
        float a = 0.0f;
        for (unsigned bin = first; bin < end; ++bin)
            if (spectrum_magnitude[bin] > a) a = spectrum_magnitude[bin];
        /* Display-only gain makes quiet microphone signals visible in linear
         * mode. Clip before converting to a height or filling the buffer. */
        float level = logarithmic ? (20.0f * log10f(fmaxf(a, 0.0001f)) + 80.0f) / 80.0f
                                  : a * SPECTRUM_LINEAR_GAIN;
        level = fminf(1.0f, fmaxf(0.0f, level * gain));
        unsigned height = (unsigned)(level * (float)plot_height);
        /* In portrait memory, each landscape column is contiguous. Use
         * optimized DSP fills so Debug's unoptimized UI stays within 50 ms. */
        q15_t *column = (q15_t *)(fb + x * SPECTRUM_HEIGHT);
        unsigned green_end = height < green_limit ? height : green_limit;
        unsigned orange_end = height < orange_limit ? height : orange_limit;
        arm_fill_q15((q15_t)RGB(32,210,80), column, green_end);
        arm_fill_q15((q15_t)RGB(255,160,0), column + green_end, orange_end - green_end);
        arm_fill_q15((q15_t)RGB(240,48,48), column + orange_end, height - orange_end);
        arm_fill_q15((q15_t)RGB(3,7,15), column + height, plot_height - height);
        arm_fill_q15((q15_t)RGB(20,30,47), column + plot_height, SPECTRUM_STRIP);
    }
    /* Slider has a separate touch area, with no scale markings or value. */
    rectangle(fb, SPECTRUM_PLOT_WIDTH, 0, SPECTRUM_SLIDER_WIDTH,
              SPECTRUM_HEIGHT, RGB(20,30,47));
    const unsigned slider_x = SPECTRUM_PLOT_WIDTH + SPECTRUM_SLIDER_WIDTH / 2U;
    const unsigned slider_y = SPECTRUM_SLIDER_BOTTOM - (unsigned)
        ((gain - 0.5f) / 1.5f * (SPECTRUM_SLIDER_BOTTOM - SPECTRUM_SLIDER_TOP));
    rectangle(fb, slider_x - 2U, SPECTRUM_SLIDER_TOP, 4U,
              SPECTRUM_SLIDER_BOTTOM - SPECTRUM_SLIDER_TOP + 1U, RGB(80,100,120));
    rectangle(fb, slider_x - 16U, slider_y - 8U, 32U, 16U, RGB(225,240,255));
    /* Frequency annotations share the existing strip; no magnitude axes. */
    label(fb, 0, "0HZ");
    label(fb, 80, logarithmic ? "LOG" : "LINEAR");
    label(fb, SPECTRUM_PLOT_WIDTH / 4U - 24U, "6KHZ");
    label(fb, SPECTRUM_PLOT_WIDTH / 2U - 30U, "12KHZ");
    label(fb, SPECTRUM_PLOT_WIDTH * 3U / 4U - 30U, "18KHZ");
    label(fb, SPECTRUM_PLOT_WIDTH - 60U, "24KHZ");
}

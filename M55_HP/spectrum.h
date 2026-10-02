#ifndef SPECTRUM_H
#define SPECTRUM_H
#include <stdint.h>
#include <stdbool.h>
#define SPECTRUM_FFT_SIZE 2048U
#define SPECTRUM_BINS (SPECTRUM_FFT_SIZE / 2U + 1U)
#define SPECTRUM_SAMPLE_RATE 48000U
#define SPECTRUM_WIDTH 800U
#define SPECTRUM_HEIGHT 480U
#define SPECTRUM_STRIP 24U
#define SPECTRUM_LINEAR_GAIN 100.0f
#define SPECTRUM_SLIDER_WIDTH 64U
#define SPECTRUM_PLOT_WIDTH (SPECTRUM_WIDTH - SPECTRUM_SLIDER_WIDTH)
#define SPECTRUM_SLIDER_TOP 48U
#define SPECTRUM_SLIDER_BOTTOM (SPECTRUM_HEIGHT - 25U)
int spectrum_init(void);
void spectrum_compute(const int16_t *pcm);
void spectrum_render(uint16_t *framebuffer, bool logarithmic);
/* Landscape touch coordinates. Returns true for a new plot tap (mode toggle). */
bool spectrum_touch(unsigned x, unsigned y, bool new_contact);
void spectrum_touch_release(void);
void spectrum_ui_reset(void);
extern float spectrum_display_gain;
extern float spectrum_magnitude[SPECTRUM_BINS];
extern volatile uint32_t spectrum_peak_hz;
#endif

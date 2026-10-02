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
int spectrum_init(void);
void spectrum_compute(const int16_t *pcm);
void spectrum_render(uint16_t *framebuffer, bool logarithmic);
extern float spectrum_magnitude[SPECTRUM_BINS];
extern volatile uint32_t spectrum_peak_hz;
#endif

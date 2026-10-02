#include "spectrum.h"
#include "arm_math.h"
#include <math.h>

static arm_rfft_fast_instance_f32 fft;
static float window[SPECTRUM_FFT_SIZE];
static float input[SPECTRUM_FFT_SIZE] __attribute__((aligned(16)));
static float output[SPECTRUM_FFT_SIZE] __attribute__((aligned(16)));
static float amplitude_scale;
float spectrum_magnitude[SPECTRUM_BINS];
volatile uint32_t spectrum_peak_hz;

int spectrum_init(void)
{
    /* Length-specific init lets the linker discard other FFT tables. */
    if (arm_rfft_fast_init_2048_f32(&fft) != ARM_MATH_SUCCESS) return -1;
    arm_hanning_f32(window, SPECTRUM_FFT_SIZE);
    float sum = 0.0f;
    for (unsigned i = 0; i < SPECTRUM_FFT_SIZE; ++i) sum += window[i];
    amplitude_scale = 2.0f / sum;
    return 0;
}

void spectrum_compute(const int16_t *pcm)
{
    /* Remove DC before windowing; signed 16-bit PCM is normalized to FS. */
    arm_q15_to_float(pcm, input, SPECTRUM_FFT_SIZE);
    float mean;
    arm_mean_f32(input, SPECTRUM_FFT_SIZE, &mean);
    arm_offset_f32(input, -mean, input, SPECTRUM_FFT_SIZE);
    arm_mult_f32(input, window, input, SPECTRUM_FFT_SIZE);
    arm_rfft_fast_f32(&fft, input, output, 0);
    /* RFFT packs DC and Nyquist into output[0] and output[1]. */
    spectrum_magnitude[0] = fabsf(output[0]) * amplitude_scale * 0.5f;
    spectrum_magnitude[SPECTRUM_BINS - 1U] = fabsf(output[1]) * amplitude_scale * 0.5f;
    arm_cmplx_mag_f32(output + 2, spectrum_magnitude + 1, SPECTRUM_FFT_SIZE / 2U - 1U);
    arm_scale_f32(spectrum_magnitude + 1, amplitude_scale,
                  spectrum_magnitude + 1, SPECTRUM_FFT_SIZE / 2U - 1U);
    float peak;
    uint32_t bin;
    arm_max_f32(spectrum_magnitude + 1, SPECTRUM_BINS - 1U, &peak, &bin);
    spectrum_peak_hz = (bin + 1U) * SPECTRUM_SAMPLE_RATE / SPECTRUM_FFT_SIZE;
}

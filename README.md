# Microphone spectrum for Alif DevKit-E8

Live microphone spectrum on the standard 480 x 800 RGB565 LCD, rotated into landscape. Frequency spans the 736-pixel plotting area, from DC on the left to 24 kHz on the right. Magnitude occupies 456 of the 480 pixels; the remaining 24-pixel strip shows LINEAR or LOG and frequency labels (0, 6, 12, 18, 24 kHz). Bars are green in the bottom 50% of the plotting area, orange from 50% to 80%, and red above 80%, in both magnitude modes. There are no magnitude axes or grid lines.

Touch the plotting area to switch between linear amplitude (100x display gain, full height at 1% of PCM full scale) and logarithmic amplitude (-80 to 0 dBFS). Log is selected at startup. The unlabelled vertical slider in the 64-pixel strip on the right multiplies the displayed bar height by 0.5 to 2.0 in either mode, starting at 1.0. Drag upward for more gain or downward for less. The combined display height is clipped after applying this gain. The scale is fixed rather than automatically normalized, preserving changes in sound level. Linear gain affects only the display; bar heights saturate at the plotting boundary so they cannot overwrite the indicator strip or leave the framebuffer. A quiet interval of 150 ms rearms the touch toggle because the pack driver returns zero touches when it has no fresh touch report.

## Signal processing

The onboard PDM microphone uses channel 4 on P5_4 data and P6_7 clock. Only the active microphone clock edge is captured; selecting between clock edges by peak-to-peak range incorrectly preferred noise on channel 5. The driver captures signed 16-bit PCM at 48 kHz with its hardware FIR decimation and DC-blocking IIR filters enabled. No further downsampling is used, so no additional software decimation filter is needed. The FIR coefficients follow the pack's channel-4 bare-metal example. Channel 4 uses phase delay 31 and fixed 240x hardware gain (0xF00 in unsigned 8.4 format), matching Alif SDK AudioBackend.cpp, with a DC-blocking coefficient selector of 9. This gain makes microphone sound visible; the original example gain yielded samples too small for the plot. There is no automatic amplitude normalization.

CMSIS-DSP 1.18.0 generates a Hann window with `arm_hanning_f32`. Each frame removes the mean, applies the window, computes a 2048-point real FFT using `arm_rfft_fast_init_2048_f32` and `arm_rfft_fast_f32`, and calculates window-gain-corrected single-sided magnitudes. DC and Nyquist are unpacked separately. Frequency resolution is 23.4375 Hz; each window covers 42.67 ms. Peak pooling maps 1025 bins onto 736 pixels without losing narrow tones. Bar lengths switch between linear and log magnitude; frequency remains linear.

Eight capture slots of 512 mono samples decouple interrupt-driven audio from display work. Capture is rearmed in the completion callback. The foreground maintains the newest 2048 samples for the microphone and services capture while waiting for the LCD. Two RGB565 framebuffers in SRAM0 use cache cleaning and VSYNC swaps. Display deadlines occur every 50 ms (20 fps).

## Build

Open `Blinky.csolution.yml` in CMSIS Solution and select DevKit-E8 (Debug) or DevKit-E8@Release. CMSIS-DSP sources compile with `-O3 -ffast-math` in both configurations. Debug leaves application code unoptimized with debug information. Release applies `-O3 -ffast-math` to the application as well. The solution name is retained for compatibility.

- `M55_HP/main.c`: board setup, PDM capture queue, touch control, display scheduling.
- `M55_HP/spectrum.c`: CMSIS-DSP window, FFT and magnitude processing.
- `M55_HP/spectrum_ui.c`: landscape renderer and scale indicator.

## Verification

Run `./tests/run_tests.ps1` with native Clang and CMSIS_PACK_ROOT set. These tests use actual CMSIS-DSP sources and check silence, DC rejection, low/high frequency tones, amplitude normalization, Nyquist handling, linear/log bar lengths and framebuffer bounds. Landscape previews are written to `out/spectrum-linear.ppm` and `out/spectrum-log.ppm`.

Runtime state is available to the debugger: `app_stage` 8 identifies the main loop; `app_error`, `app_service_error`, `display_events` and `audio_error` should be zero. `frames_presented` and `audio_produced` advance. `frame_period_ms` should be 50; `frame_compute_ms` measures FFT, render and cache cleaning. `deadline_misses` and `audio_dropped_blocks` count scheduling and capture overruns. `microphone_range[0]` contains the peak-to-peak PCM range; `microphone_channel` identifies the displayed input; `spectrum_peak_hz` reports the strongest FFT bin frequency. Halting for debug disrupts audio and can cause FIFO overflow on resume. Capture resets the FIFO, discards the incomplete window and resumes automatically; `audio_overflows` counts these events. Use CMSIS Load and Run to leave the application running for physical checks; ending a halted debug session may leave the core halted.

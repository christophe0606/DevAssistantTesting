/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification permitted under the Alif Semiconductor
 * Software License Agreement: https://alifsemi.com/license
 * Microphone spectrum for the DevKit-E8 standard MIPI LCD.
 */
#include "spectrum.h"
#include <string.h>
#include <stdint.h>
#include <RTE_Components.h>
#include CMSIS_device_header
#include "RTE_Device.h"
#include "Driver_CDC200.h"
#include "Driver_IO.h"
#include "Driver_PDM.h"
#include "Driver_Touch_Screen.h"
#include "pinconf.h"
#include "board_config.h"
#include "se_services_port.h"

#define LCD_WIDTH       RTE_PANEL_HACTIVE_TIME
#define LCD_HEIGHT      RTE_PANEL_VACTIVE_LINE
#define FRAME_MS 50U

#if RTE_CDC200_PIXEL_FORMAT != 2
#error "This renderer requires RGB565 (RTE_CDC200_PIXEL_FORMAT = 2)"
#endif
_Static_assert(LCD_WIDTH == SPECTRUM_HEIGHT && LCD_HEIGHT == SPECTRUM_WIDTH,
               "Spectrum UI requires the standard 480 x 800 LCD");

/* Bulk SRAM is accessible to CDC DMA; DTCM holds normal data and stack. */
static uint16_t framebuffers[2][LCD_HEIGHT][LCD_WIDTH]
    __attribute__((section(".bss.lcd_frame_buf"), aligned(32)));
extern ARM_DRIVER_CDC200 Driver_CDC200;
static volatile uint32_t ms_ticks;
static volatile uint32_t scanline_count;
static volatile uint32_t display_events;

/* Retained failure state, inspectable without serial I/O. */
volatile uint32_t app_stage;
volatile int32_t app_error;
volatile uint32_t app_service_error;
volatile uint32_t frames_presented;

void SysTick_Handler(void)
{
    ++ms_ticks;
}

static void display_callback(uint32_t events)
{
    if (events & ARM_CDC_SCANLINE0_EVENT) {
        ++scanline_count;
    }
    display_events |= events & ARM_CDC_DSI_ERROR_EVENT;
}

static void fail(int32_t error)
{
    app_error = error;
    for (;;) {
        __WFI();
    }
}

static void check_driver(int32_t status)
{
    if (status != ARM_DRIVER_OK) {
        fail(status);
    }
}

static void check_service(uint32_t status, uint32_t service_error)
{
    app_service_error = service_error;
    if (status != SERVICES_REQ_SUCCESS || service_error != 0U) {
        fail(status != SERVICES_REQ_SUCCESS ? (int32_t)status : ARM_DRIVER_ERROR);
    }
}

static void clean_frame(uint32_t index)
{
    SCB_CleanDCache_by_Addr((uint32_t *)framebuffers[index], sizeof(framebuffers[index]));
    __DSB();
}

extern ARM_DRIVER_PDM Driver_PDM;
extern ARM_DRIVER_TOUCH_SCREEN GT911;
#define AUDIO_CHUNK 512U
#define AUDIO_SLOTS 8U
#define AUDIO_CHANNELS 1U
#define MICROPHONE_GAIN_Q8_4 0xF00U /* Alif AudioBackend.cpp: fixed 240x gain. */
/* Interrupt-driven PDM writes CPU-local memory, so no DMA cache coherency
 * operation is needed. The callback rearms capture before returning. */
static int16_t audio[AUDIO_SLOTS][AUDIO_CHUNK * AUDIO_CHANNELS];
static int16_t history[AUDIO_CHANNELS][SPECTRUM_FFT_SIZE];
static volatile uint32_t audio_produced;
static uint32_t audio_consumed, history_samples, audio_at;
static volatile int32_t audio_error;
static volatile bool audio_armed;
static volatile bool audio_restart;
volatile uint32_t audio_overflows;
volatile uint32_t audio_dropped_blocks;
volatile uint32_t logarithmic = 1U;
volatile uint32_t frame_compute_ms, frame_period_ms, deadline_misses;
volatile uint32_t microphone_range[AUDIO_CHANNELS], microphone_channel = 4U;
static uint32_t touch_at, last_contact;
static bool touch_held;

static void touchscreen_init(void)
{
    /* The peripheral drivers do not configure their board pin multiplexers.
     * These are the DevKit-E8 selections in the generated pins.h. */
    const uint32_t i2c_pad = PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_PULL_UP |
                             PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA;
    check_driver(pinconf_set(PORT_7, PIN_2, PINMUX_ALTERNATE_FUNCTION_5, i2c_pad));
    check_driver(pinconf_set(PORT_7, PIN_3, PINMUX_ALTERNATE_FUNCTION_5, i2c_pad));
    check_driver(pinconf_set(PORT_(BOARD_TOUCH_RESET_GPIO_PORT), BOARD_TOUCH_RESET_GPIO_PIN,
                             PINMUX_ALTERNATE_FUNCTION_0, PADCTRL_OUTPUT_DRIVE_STRENGTH_4MA));
    check_driver(pinconf_set(PORT_(BOARD_TOUCH_INT_GPIO_PORT), BOARD_TOUCH_INT_GPIO_PIN,
                             PINMUX_ALTERNATE_FUNCTION_0,
                             PADCTRL_READ_ENABLE | PADCTRL_OUTPUT_DRIVE_STRENGTH_4MA));
    check_driver(GT911.Initialize());
    check_driver(GT911.PowerControl(ARM_POWER_FULL));
}

static void audio_callback(uint32_t events)
{
    /* A core reload can leave peripheral IRQs from the previous program.
     * PowerControl enables the NVIC before resetting the FIFO. Never rearm
     * a stale completion while the driver is still being configured. */
    if (!audio_armed) return;
    if (events & ARM_PDM_EVENT_ERROR) {
        ++audio_overflows;
        audio_restart = true;
        audio_armed = false;
        return;
    }
    if (events & ARM_PDM_EVENT_CAPTURE_COMPLETE) {
        __DMB();
        ++audio_produced;
        int32_t status = Driver_PDM.Receive(audio[audio_produced % AUDIO_SLOTS], AUDIO_CHUNK * AUDIO_CHANNELS);
        if (status != ARM_DRIVER_OK) audio_error = status;
    }
}

static void microphone_init(void)
{
    check_driver(pinconf_set(PORT_(BOARD_PDM_D2_B_GPIO_PORT), BOARD_PDM_D2_B_GPIO_PIN,
                             PINMUX_ALTERNATE_FUNCTION_3,
                             PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_PULL_UP |
                             PADCTRL_OUTPUT_DRIVE_STRENGTH_4MA));
    check_driver(pinconf_set(PORT_(BOARD_PDM_C2_A_GPIO_PORT), BOARD_PDM_C2_A_GPIO_PIN,
                             PINMUX_ALTERNATE_FUNCTION_3, PADCTRL_OUTPUT_DRIVE_STRENGTH_4MA));
    check_driver(Driver_PDM.Initialize(audio_callback));
    check_driver(Driver_PDM.PowerControl(ARM_POWER_FULL));
    check_driver(Driver_PDM.Control(ARM_PDM_MODE, ARM_PDM_MODE_MICROPHONE_SLEEP, 0));
    /* Filter from the pack's bare-metal PDM example.
     * Keep the hardware FIR decimator and DC-blocking IIR enabled. */
    PDM_CH_CONFIG config = { .ch_num = 4, .ch_fir_coef = {
        0x001,0x003,0x003,0x7F4,0x004,0x7ED,0x7F5,0x7F4,0x7D3,
        0x7FE,0x7BC,0x7E5,0x7D9,0x793,0x029,0x72C,0x072,0x2FD
    }, .ch_iir_coef = 9 };
    check_driver(Driver_PDM.Config(&config));
    check_driver(Driver_PDM.Control(ARM_PDM_CHANNEL_PHASE, 4, 0x1F));
    check_driver(Driver_PDM.Control(ARM_PDM_CHANNEL_GAIN, 4, MICROPHONE_GAIN_Q8_4));
    check_driver(Driver_PDM.Control(ARM_PDM_BYPASS_FIR_FILTER, 0, 0));
    check_driver(Driver_PDM.Control(ARM_PDM_BYPASS_IIR_FILTER, 0, 0));
    audio_at = ms_ticks;
    check_driver(Driver_PDM.Control(ARM_PDM_SELECT_CHANNEL,
                 ARM_PDM_MASK_CHANNEL_4, 0));
    check_driver(Driver_PDM.Control(ARM_PDM_MODE, ARM_PDM_MODE_AUDIOFREQ_48K_DECM_64, 0));
    check_driver(Driver_PDM.Receive(audio[0], AUDIO_CHUNK * AUDIO_CHANNELS));
    audio_armed = true;
}

static void service_input(void)
{
    if (audio_restart) {
        /* PDM continues while the core is halted. Reset its FIFO and discard
         * the incomplete window before accepting samples after an overflow. */
        audio_armed = false;
        check_driver(Driver_PDM.PowerControl(ARM_POWER_OFF));
        audio_produced = audio_consumed = history_samples = 0;
        audio_restart = false;
        microphone_init();
    }
    if (audio_error != ARM_DRIVER_OK) fail(audio_error);
    uint32_t available = audio_produced;
    __DMB();
    /* Reserve the slot currently being written. Recover visibly counted
     * overruns without ever transforming a partially captured block. */
    if ((uint32_t)(available - audio_consumed) >= AUDIO_SLOTS) {
        audio_dropped_blocks += available - audio_consumed - (AUDIO_SLOTS - 1U);
        audio_consumed = available - (AUDIO_SLOTS - 1U);
        history_samples = 0;
    }
    while (audio_consumed != available) {
        for (unsigned ch = 0; ch < AUDIO_CHANNELS; ++ch) {
            memmove(history[ch], history[ch] + AUDIO_CHUNK,
                    sizeof(history[ch]) - AUDIO_CHUNK * sizeof(int16_t));
            for (unsigned i = 0; i < AUDIO_CHUNK; ++i)
                history[ch][SPECTRUM_FFT_SIZE - AUDIO_CHUNK + i] =
                    audio[audio_consumed % AUDIO_SLOTS][i * AUDIO_CHANNELS + ch];
        }
        __DMB();
        if ((uint32_t)(audio_produced - audio_consumed) >= AUDIO_SLOTS) {
            history_samples = 0;
            return; /* Retry from a safe slot on the next service. */
        }
        ++audio_consumed;
        if (history_samples < SPECTRUM_FFT_SIZE) history_samples += AUDIO_CHUNK;
        audio_at = ms_ticks;
    }
    uint32_t now = ms_ticks;
    if (now - audio_at > 250U) fail(ARM_DRIVER_ERROR_TIMEOUT);
    if (now - touch_at < 10U) return;
    touch_at = now;
    ARM_TOUCH_STATE state = {0};
    check_driver(GT911.GetState(&state));
    /* GetState reports zero when no fresh interrupt is pending, including
     * during a held finger. Require a quiet interval before another toggle. */
    if (state.numtouches > 0) {
        if (!touch_held) logarithmic ^= 1U;
        touch_held = true;
        last_contact = now;
    } else if (now - last_contact >= 150U) touch_held = false;
}
static void present(uint32_t index)
{
    check_driver(Driver_CDC200.Control(CDC200_FRAMEBUF_UPDATE_VSYNC,
                                      (uint32_t)framebuffers[index]));
    const uint32_t first_scanline = scanline_count;
    const uint32_t started = ms_ticks;
    /* The pack reports the line at the end of active video. Wait for TWO
     * such events so vertical-blank reload has completed before reusing
     * the old front buffer, even when submitted right at blanking. */
    while ((uint32_t)(scanline_count - first_scanline) < 2U) {
        if (display_events != 0U || (uint32_t)(ms_ticks - started) > 250U) {
            fail(ARM_DRIVER_ERROR_TIMEOUT);
        }
        service_input();
        __WFI();
    }
    ++frames_presented;
}

int main(void)
{
    uint32_t service_error = 0U;
    uint32_t status;
    run_profile_t profile = {0};

    app_stage = 1U;
    if (SysTick_Config(SystemCoreClock / 1000U) != 0U) {
        fail(ARM_DRIVER_ERROR);
    }
#if BOARD_CONFIGURE_LVDS_MUX
    check_driver(board_gpios_config());
#endif
    se_services_port_init();

    app_stage = 2U;
    status = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_CLK_100M,
                                         true, &service_error);
    check_service(status, service_error);
    status = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_HFOSC,
                                         true, &service_error);
    check_service(status, service_error);
    status = SERVICES_get_run_cfg(se_services_s_handle, &profile, &service_error);
    check_service(status, service_error);
    /* Preserve existing power requests while adding display resources. */
    profile.memory_blocks |= MRAM_MASK | SRAM0_MASK;
    profile.phy_pwr_gating |= MIPI_PLL_DPHY_MASK | MIPI_TX_DPHY_MASK |
                             MIPI_RX_DPHY_MASK | LDO_PHY_MASK;
    status = SERVICES_set_run_cfg(se_services_s_handle, &profile, &service_error);
    check_service(status, service_error);

    app_stage = 3U;
    if (spectrum_init() != 0) fail(ARM_DRIVER_ERROR);
    for (uint32_t i = 0; i < 2U; ++i) {
        spectrum_render(&framebuffers[i][0][0], logarithmic != 0U);
        clean_frame(i);
    }
    app_stage = 4U;
    check_driver(Driver_CDC200.Initialize(display_callback));
    app_stage = 5U;
    check_driver(Driver_CDC200.PowerControl(ARM_POWER_FULL));
    app_stage = 6U;
    check_driver(Driver_CDC200.Control(CDC200_CONFIGURE_DISPLAY,
                                      (uint32_t)framebuffers[0]));
    check_driver(Driver_CDC200.Control(CDC200_SCANLINE0_EVENT, 1U));
    app_stage = 7U;
    check_driver(Driver_CDC200.Start());

    touchscreen_init();
    status = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_HFOSCx2,
                                         true, &service_error);
    check_service(status, service_error);
    microphone_init();

    app_stage = 8U;
    uint32_t back = 1U;

    uint32_t next_frame = ms_ticks;
    uint32_t previous_frame = ms_ticks;
    for (;;) {
        while ((int32_t)(ms_ticks - next_frame) < 0) {
            service_input();
            __WFI();
        }
        const uint32_t frame_start = ms_ticks;
        frame_period_ms = frame_start - previous_frame;
        previous_frame = frame_start;
        if (display_events != 0U) {
            fail(ARM_DRIVER_ERROR);
        }
        service_input();
        if (history_samples == SPECTRUM_FFT_SIZE) {
            /* The onboard microphone drives channel 4. Channel 5 is the
             * opposite clock edge, whose noise can exceed the signal's range. */
            for (unsigned ch = 0; ch < AUDIO_CHANNELS; ++ch) {
                int32_t low = 32767, high = -32768;
                for (unsigned i = 0; i < SPECTRUM_FFT_SIZE; ++i) {
                    int32_t sample = history[ch][i];
                    if (sample < low) low = sample;
                    if (sample > high) high = sample;
                }
                microphone_range[ch] = (uint32_t)(high - low);
            }
            spectrum_compute(history[0]);
        }
        spectrum_render(&framebuffers[back][0][0], logarithmic != 0U);
        clean_frame(back);
        frame_compute_ms = ms_ticks - frame_start;
        present(back);
        back ^= 1U;
        next_frame += FRAME_MS;
        if ((int32_t)(ms_ticks - next_frame) > 0) {
            ++deadline_misses;
            next_frame = ms_ticks;
        }
    }
}

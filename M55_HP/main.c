/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification permitted under the Alif Semiconductor
 * Software License Agreement: https://alifsemi.com/license
 * CPU XScreenSaver gallery for the DevKit-E8 standard MIPI LCD.
 */
#include "xs_port/xs_gallery.h"
#include <stdint.h>
#include <RTE_Components.h>
#include CMSIS_device_header
#include "RTE_Device.h"
#include "Driver_CDC200.h"
#include "Driver_IO.h"
#include "pinconf.h"
#include "board_config.h"
#include "se_services_port.h"

#define LCD_WIDTH       RTE_PANEL_HACTIVE_TIME
#define LCD_HEIGHT      RTE_PANEL_VACTIVE_LINE
#define FRAME_MS 33U

#if RTE_CDC200_PIXEL_FORMAT != 2
#error "This renderer requires RGB565 (RTE_CDC200_PIXEL_FORMAT = 2)"
#endif
_Static_assert(LCD_WIDTH == XS_HEIGHT * 2 && LCD_HEIGHT == XS_WIDTH * 2,
               "The gallery requires the standard 480 x 800 LCD");

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
        __WFI();
    }
    ++frames_presented;
    xs_gallery_presented(ms_ticks);
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
    profile.memory_blocks |= MRAM_MASK | SRAM0_MASK | SRAM1_MASK;
    profile.phy_pwr_gating |= MIPI_PLL_DPHY_MASK | MIPI_TX_DPHY_MASK |
                             MIPI_RX_DPHY_MASK | LDO_PHY_MASK;
    status = SERVICES_set_run_cfg(se_services_s_handle, &profile, &service_error);
    check_service(status, service_error);

    app_stage = 3U;
    xs_gallery_init(ms_ticks);
    for (uint32_t i = 0; i < 2U; ++i) {
        xs_gallery_render(&framebuffers[i][0][0]);
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

    app_stage = 8U;
    uint32_t back = 1U;

    for (;;) {
        const uint32_t frame_start = ms_ticks;
        if (display_events != 0U) {
            fail(ARM_DRIVER_ERROR);
        }
        xs_gallery_update(ms_ticks);
        xs_gallery_render(&framebuffers[back][0][0]);
        clean_frame(back);
        present(back);
        back ^= 1U;
        while ((uint32_t)(ms_ticks - frame_start) < FRAME_MS) {
                __WFI();
        }
    }
}

/* Ethos-U85 board hooks, following the Ensemble E8 pack's M55_HP layer. */
#include "RTE_Components.h"
#include CMSIS_device_header
#include "ethosu_driver.h"
#include "app_mem_regions.h"

static struct ethosu_driver driver;
volatile uint32_t npu_interrupts;
volatile int32_t npu_error;

void IRQ366_Handler(void)
{
    ++npu_interrupts;
    ethosu_irq_handler(&driver);
}

int ethos_setup(void)
{
    NVIC_DisableIRQ((IRQn_Type)366);
    int status = ethosu_init(&driver, (void *)NPU_HG_BASE, NULL, 0, 1, 1);
    if (!status) status = ethosu_soft_reset(&driver);
    npu_error = status;
    if (status) return status;
    NVIC_ClearPendingIRQ((IRQn_Type)366);
    NVIC_EnableIRQ((IRQn_Type)366);
    return 0;
}

/* Dirty CPU lines must be committed before the NPU reads. Whole-cache hooks
 * also cover the float/quantized boundary buffers allocated by ExecuTorch.
 * Clean+invalidate after completion avoids losing unrelated dirty CPU data.
 */
void ethosu_flush_dcache(const uint64_t *base, const size_t *sizes, int count)
{
    (void)base; (void)sizes; (void)count;
    SCB_CleanDCache();
    __DSB();
}

void ethosu_invalidate_dcache(const uint64_t *base, const size_t *sizes, int count)
{
    (void)base; (void)sizes; (void)count;
    SCB_CleanInvalidateDCache();
    __DSB();
}

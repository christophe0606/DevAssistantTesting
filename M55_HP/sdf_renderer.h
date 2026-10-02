#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    uint32_t frames, tiles, model_calls;
    uint32_t render_ms, inference_ms, march_calls, shadow_calls;
    int32_t error;
} SdfMetrics;
extern volatile SdfMetrics sdf_metrics;
extern volatile uint32_t sdf_half_resolution;
extern volatile uint32_t sdf_animate;
extern volatile float sdf_time;
int sdf_init(void);
int sdf_render(uint16_t *framebuffer, float time, int half);
uint32_t board_millis(void);
void board_service_input(void);
#ifdef __cplusplus
}
#endif

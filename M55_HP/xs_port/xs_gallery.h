#ifndef XS_GALLERY_H
#define XS_GALLERY_H
#include <stdint.h>
#include <stddef.h>
#define XS_WIDTH 400
#define XS_HEIGHT 240
#define XS_DURATION_MS 30000U
#define XS_ARENA0_BYTES (2U * 1024U * 1024U)
#define XS_ARENA1_BYTES (4U * 1024U * 1024U)
#define XS_ARENA_BYTES (XS_ARENA0_BYTES + XS_ARENA1_BYTES)
void xs_gallery_init(uint32_t now);
void xs_gallery_update(uint32_t now);
void xs_gallery_presented(uint32_t now);
void xs_gallery_render(uint16_t *lcd);
unsigned xs_gallery_count(void);
const char *xs_gallery_name(unsigned index);
int xs_gallery_select(unsigned index, uint32_t now);
extern volatile unsigned xs_current_saver;
extern volatile unsigned xs_failures;
extern volatile size_t xs_memory_peak;
extern volatile unsigned xs_last_error;
extern volatile unsigned xs_cycle_enabled;
extern volatile unsigned xs_requested_saver;
extern char xs_error_text[160];
#endif

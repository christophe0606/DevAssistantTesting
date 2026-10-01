#ifndef XS_INTERNAL_H
#define XS_INTERNAL_H
#include "config.h"
#include "jwxyz.h"
#include "xs_gallery.h"
#include <setjmp.h>
struct jwxyz_Drawable { int w, h, depth; uint32_t *pixels; unsigned long background; unsigned references; };
struct jwxyz_GC { XGCValues v; };
struct jwxyz_Display { const struct jwxyz_vtbl *vtbl; };
struct jwxyz_Font { int scale; };
extern Display xs_display;
extern struct jwxyz_Drawable xs_window;
extern uint32_t xs_now;
extern jmp_buf xs_recover;
extern int xs_recover_active;
void xs_memory_reset(void);
void xs_resources_set(const char * const *defaults);
void xs_platform_reset(void);
void xs_local_draw(unsigned kind, unsigned frame);
extern const unsigned char xs_font_bits[256][10];
extern const unsigned char *const xs_picture;
extern const size_t xs_picture_size;
#endif

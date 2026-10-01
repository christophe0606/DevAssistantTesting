/* One active original module, ten-second slots, and reclaimed memory per slot. */
#include "xs_internal.h"
#include "screenhackI.h"
#include "xlockmoreI.h"
#include <stdio.h>
extern struct xscreensaver_function_table *const xs_modules[];
extern const unsigned xs_module_count;
extern const char *const xs_module_names[];
extern const char *progname, *progclass;
volatile unsigned xs_current_saver;
volatile unsigned xs_failures;
volatile unsigned xs_last_error;
/* Debugger controls: hold one saver or request another through the normal
 * main loop, without invoking target functions from the debugger. */
volatile unsigned xs_cycle_enabled=1;
volatile unsigned xs_requested_saver=UINT32_MAX;
char xs_error_text[160];
jmp_buf xs_recover;
int xs_recover_active;
static void *closure;
static struct xscreensaver_function_table *active;
static uint32_t started,next_frame,local_frame;
static int failed;
static int pending_first_frame;
void jwxyz_abort(const char*fmt,...){va_list a;va_start(a,fmt);vsnprintf(xs_error_text,sizeof(xs_error_text),fmt,a);va_end(a);xs_last_error=1;if(xs_recover_active)longjmp(xs_recover,1);for(;;){}}
void xs_abort(void){jwxyz_abort("screensaver aborted");}
void xs_exit(int status){jwxyz_abort("screensaver exited with status %d",status);}
unsigned xs_gallery_count(void){return xs_module_count+2;}
const char*xs_gallery_name(unsigned i){return i<xs_module_count?xs_module_names[i]:i==xs_module_count?"webcollage":"vidwhacker";}
int xs_gallery_select(unsigned index,uint32_t now){
  xs_now=now;xs_recover_active=1;
  if(setjmp(xs_recover)){failed=1;xs_failures++;xs_recover_active=0;return 0;}
  if(active&&closure&&!failed&&active->free_cb)active->free_cb(&xs_display,&xs_window,closure);
  closure=NULL;active=NULL;failed=0;xs_last_error=0;xs_error_text[0]=0;
  xs_memory_reset();xs_platform_reset();xs_current_saver=index%xs_gallery_count();started=next_frame=now;local_frame=0;pending_first_frame=1;
  if(xs_current_saver<xs_module_count){
    active=xs_modules[xs_current_saver];progname=xs_module_names[xs_current_saver];progclass=active->progclass;
    if(active->setup_cb)active->setup_cb(active,active->setup_arg);
    xs_resources_set(active->defaults);
    XSetWindowBackground(&xs_display,&xs_window,get_pixel_resource(&xs_display,0,"background","Background"));
    XClearWindow(&xs_display,&xs_window);
    if(active->setup_cb==xlockmore_setup)closure=((void*(*)(Display*,Window,void*))active->init_cb)(&xs_display,&xs_window,active->setup_arg);
    else closure=active->init_cb(&xs_display,&xs_window);
    if(!closure)jwxyz_abort("initialization returned NULL");
  }
  xs_recover_active=0;return 1;
}
void xs_gallery_init(uint32_t now){xs_gallery_select(0,now);}
void xs_gallery_presented(uint32_t now){if(pending_first_frame){started=now;pending_first_frame=0;}}
void xs_gallery_update(uint32_t now){
  xs_now=now;
  unsigned requested=xs_requested_saver;
  if(requested!=UINT32_MAX){xs_requested_saver=UINT32_MAX;xs_gallery_select(requested,now);}
  else if(xs_cycle_enabled&&!pending_first_frame&&(uint32_t)(now-started)>=XS_DURATION_MS){xs_gallery_select(xs_current_saver+1,now);}
  if(failed||(int32_t)(now-next_frame)<0)return;
  xs_recover_active=1;
  if(setjmp(xs_recover)){failed=1;xs_failures++;xs_recover_active=0;return;}
  unsigned long delay=33000;
  if(active)delay=active->draw_cb(&xs_display,&xs_window,closure);else xs_local_draw(xs_current_saver-xs_module_count,local_frame++);
  /* Bound scheduler work; sub-millisecond desktop delays cannot be displayed. */
  uint32_t ms=(delay+999)/1000;if(ms<16)ms=16;if(ms>1000)ms=1000;
  next_frame=now+ms;xs_recover_active=0;
}
void xs_gallery_render(uint16_t *out){for(int y=0;y<XS_HEIGHT;y++)for(int x=0;x<XS_WIDTH;x++){
  uint32_t p=xs_window.pixels[y*XS_WIDTH+x];uint16_t c=((p>>8)&0xf800)|((p>>5)&0x7e0)|((p>>3)&31);
  size_t pos=(size_t)y*2*XS_WIDTH*2+x*2;out[pos]=out[pos+1]=out[pos+XS_WIDTH*2]=out[pos+XS_WIDTH*2+1]=c;
}}

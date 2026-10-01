/* Local resources, colors, time, images and text for the bare-metal port. */
#include "xs_internal.h"
#include "screenhackI.h"
#include "textclient.h"
#include "ximage-loader.h"
#include <ctype.h>
#include <math.h>
#undef ya_rand_init
const char *progname="xscreensaver";
const char *progclass="XScreenSaver";
Bool mono_p=False;
uint32_t xs_now;
static uint32_t rng=0x67d34912;
static const char * const *resources;
unsigned int ya_random(void){uint32_t x=rng;x^=x<<13;x^=x>>17;x^=x<<5;return rng=x;}
void ya_rand_init(unsigned seed){if(seed)rng=seed;}
int gettimeofday(struct timeval*t,void*tz){t->tv_sec=1700000000L+xs_now/1000;t->tv_usec=(xs_now%1000)*1000;return 0;}
time_t xs_time(time_t*t){time_t n=1700000000+xs_now/1000;if(t)*t=n;return n;}
unsigned getpid(void){return 1;}void screenhack_usleep(unsigned long n){}
void fps_free(fps_state*s){}
double fps_compute(fps_state*s,unsigned long p,double d){return 0;}
void fps_draw(fps_state*s){}
double current_device_rotation(void){return 0;}
char*strtok_r(char*s,const char*sep,char**ctx){if(!s)s=*ctx;if(!s)return NULL;s+=strspn(s,sep);if(!*s){*ctx=s;return NULL;}char*e=s+strcspn(s,sep);if(*e)*e++=0;*ctx=e;return s;}
char*strcasestr(const char*s,const char*n){size_t len=strlen(n);for(;*s;s++)if(!xs_strncasecmp(s,n,len))return(char*)s;return len?NULL:(char*)s;}
int gethostname(char*s,size_t n){if(n){strncpy(s,"Alif E8",n);s[n-1]=0;}return 0;}
void xs_resources_set(const char *const *p){resources=p;}
static const char *lookup(const char*n){
  if(!strcmp(progname,"flow")&&!strcmp(n,"count"))return "180";
  if(!strcmp(progname,"mismunch")&&!strcmp(n,"mismunch"))return "True";
  if(!strcmp(progname,"phosphor")&&!strcmp(n,"phosphorScale"))return "2";
  if(!strcmp(progname,"flow")&&!strcmp(n,"ncolors"))return "64";
  if(!strcmp(progname,"deluxe")&&!strcmp(n,"thickness"))return "10";
  if(!strcmp(progname,"tessellimage")&&!strcmp(n,"maxResolution"))return "160";
  if(!strcmp(progname,"tessellimage")&&!strcmp(n,"maxDepth"))return "800";
  if(!strcmp(progname,"tessellimage")&&!strcmp(n,"cache"))return "False";
  /* Backend choices shared by all modules. */
  if(!strcmp(n,"useSHM")||!strcmp(n,"useDBE")||!strcmp(n,"fps")||!strcmp(n,"use3d"))return "False";
  if(!strcmp(n,"lowrez"))return "True";
  const char *found=NULL;
  if(resources)for(int i=0;resources[i];i++){
    const char*s=resources[i],*colon=strchr(s,':');if(!colon)continue;
    const char*k=colon;while(k>s&&k[-1]!='*'&&k[-1]!='.')--k;
    const char*end=colon;while(end>k&&isspace((unsigned char)end[-1]))end--;
    if((size_t)(end-k)==strlen(n)&&!xs_strncasecmp(k,n,end-k)){found=colon+1;while(isspace((unsigned char)*found))found++;}
  }
  if(found)return found;
  if(!strcmp(n,"background"))return "black";if(!strcmp(n,"foreground"))return "white";
  if(strstr(n,"font")||strstr(n,"Font"))return "monospace 10";
  return NULL;
}
char*get_string_resource(Display*d,char*n,char*c){const char*s=lookup(n);return s?xs_strdup(s):NULL;}
Bool get_boolean_resource(Display*d,char*n,char*c){const char*s=lookup(n);return s&&(!xs_strncasecmp(s,"true",4)||!xs_strncasecmp(s,"yes",3)||atoi(s)!=0);}
int get_integer_resource(Display*d,char*n,char*c){const char*s=lookup(n);return s?strtol(s,NULL,0):0;}
double get_float_resource(Display*d,char*n,char*c){const char*s=lookup(n);return s?strtod(s,NULL):0;}
int parse_time(const char*s,Bool sec,Bool silent){return s?atoi(s)*(sec?1:60):0;}
unsigned get_seconds_resource(Display*d,char*n,char*c){return get_integer_resource(d,n,c);}
unsigned get_minutes_resource(Display*d,char*n,char*c){return get_integer_resource(d,n,c)*60;}
Status XAllocColor(Display*d,Colormap m,XColor*c){c->pixel=0xff000000|((c->red>>8)<<16)|((c->green>>8)<<8)|(c->blue>>8);c->flags=7;return True;}
Status XParseColor(Display*d,Colormap m,const char*s,XColor*c){
  unsigned r=255,g=255,b=255;
  if(!s)s="white";
  if(s[0]=='#') {unsigned long p=strtoul(s+1,NULL,16);size_t n=strlen(s+1);if(n==3){r=((p>>8)&15)*17;g=((p>>4)&15)*17;b=(p&15)*17;}else if(n==12){r=(p>>40)&255;g=(p>>24)&255;b=(p>>8)&255;}else{r=(p>>16)&255;g=(p>>8)&255;b=p&255;}}
  else {
    struct named {const char*n;unsigned p;};static const struct named names[]={
      {"black",0},{"white",0xffffff},{"red",0xff0000},{"green",0x00ff00},{"blue",0x0000ff},{"yellow",0xffff00},{"cyan",0x00ffff},{"magenta",0xff00ff},{"orange",0xffa500},{"purple",0xa020f0},{"pink",0xffc0cb},{"brown",0xa52a2a},{"gray",0xbebebe},{"grey",0xbebebe},{"darkgreen",0x006400},{"navy",0x000080},{"darkblue",0x00008b},{"lightblue",0xadd8e6},{"gold",0xffd700},{"violet",0xee82ee},{"lightgray",0xd3d3d3},{"darkgray",0xa9a9a9},{"lime",0x00ff00}};
    for(unsigned i=0;i<sizeof(names)/sizeof(*names);i++)if(!xs_strcasecmp(s,names[i].n)){r=names[i].p>>16;g=(names[i].p>>8)&255;b=names[i].p&255;break;}
    if(!xs_strncasecmp(s,"gray",4)||!xs_strncasecmp(s,"grey",4))if(isdigit((unsigned char)s[4]))r=g=b=atoi(s+4)*255/100;
  }
  c->red=r*257;c->green=g*257;c->blue=b*257;c->flags=7;return True;
}
Status XAllocNamedColor(Display*d,Colormap m,char*n,XColor*s,XColor*e){XParseColor(d,m,n,s);XAllocColor(d,m,s);if(e)*e=*s;return True;}
int XQueryColor(Display*d,Colormap m,XColor*c){c->red=((c->pixel>>16)&255)*257;c->green=((c->pixel>>8)&255)*257;c->blue=(c->pixel&255)*257;return 0;}
int XQueryColors(Display*d,Colormap m,XColor*c,int n){for(int i=0;i<n;i++)XQueryColor(d,m,c+i);return 0;}
int XFreeColors(Display*d,Colormap m,unsigned long*p,int n,unsigned long pl){return 0;}
Status XAllocColorCells(Display*d,Colormap m,Bool b,unsigned long*p,unsigned n,unsigned long*q,unsigned nq){return False;}
int XStoreColor(Display*d,Colormap m,XColor*c){return 0;}int XStoreColors(Display*d,Colormap m,XColor*c,int n){return 0;}
unsigned get_pixel_resource(Display*d,Colormap m,char*n,char*c){XColor x;XParseColor(d,m,lookup(n),&x);XAllocColor(d,m,&x);return x.pixel;}
int visual_depth(Screen*s,Visual*v){return 32;}int visual_pixmap_depth(Screen*s,Visual*v){return 32;}int visual_class(Screen*s,Visual*v){return TrueColor;}int visual_cells(Screen*s,Visual*v){return 256;}
int screen_number(Screen*s){return 0;}int has_writable_cells(Screen*s,Visual*v){return False;}
void visual_rgb_masks(Screen*s,Visual*v,unsigned long*r,unsigned long*g,unsigned long*b){*r=0xff0000;*g=0xff00;*b=0xff;}
Visual*get_visual(Screen*s,const char*n,Bool a,Bool b){return DefaultVisualOfScreen(s);}
Visual*get_visual_resource(Screen*s,char*n,char*c,Bool b){return DefaultVisualOfScreen(s);}
Visual*find_similar_visual(Screen*s,Screen*t,Visual*v){return v;}
Bool use_subwindow_mode_p(Screen*s,Window w){return False;}
Bool screenhack_event_helper(Display*d,Window w,XEvent*e){return False;}
void screenhack_handle_events(Display*d){}
void clear_gl_error(void){}void check_gl_error(const char*s){}
/* The board maps asset MRAM as Device memory. AC6 can otherwise combine
 * byte expressions into unaligned halfword/word loads from six-byte RLE records.
 */
static uint16_t u16(const unsigned char*p){const volatile unsigned char*b=p;return b[0]|b[1]<<8;}
static uint32_t u32(const unsigned char*p){const volatile unsigned char*b=p;return b[0]|b[1]<<8|b[2]<<16|(uint32_t)b[3]<<24;}
static uint32_t expand565(uint16_t p){unsigned r=(p>>11)&31,g=(p>>5)&63,b=p&31;return 0xff000000|((r*255/31)<<16)|((g*255/63)<<8)|(b*255/31);}
static uint32_t *decode(const unsigned char*p,size_t n,int*w,int*h){
  if(n<8)return NULL;
  /* Asset arrays need not start on a word boundary. Even a short memcmp
   * can become an unaligned load, so read the signature through u32 too. */
  uint32_t magic=u32(p);
  if(magic!=0x31425358U&&magic!=0x31525358U)return NULL;
  *w=u16(p+4);*h=u16(p+6);if(!*w||!*h||*w>4096||*h>4096)return NULL;
  size_t count=(size_t)*w**h;uint32_t*out=xs_calloc(count,4);size_t k=0;
  if(magic==0x31425358U){if(n<8+2*count+(count+7)/8){xs_free(out);return NULL;}const volatile unsigned char*mask=p+8+count*2;for(k=0;k<count;k++)out[k]=expand565(u16(p+8+2*k))&((mask[k/8]>>(k%8))&1?~0U:0xffffff);}
  else if(magic==0x31525358U){for(size_t i=8;i+6<=n && k<count;i+=6){unsigned run=u16(p+i);uint32_t c=u32(p+i+2);if(run>count-k)run=count-k;while(run--)out[k++]=c;}if(k!=count){xs_free(out);return NULL;}}
  else{xs_free(out);return NULL;}return out;
}
Pixmap image_data_to_pixmap(Display*d,Window w,const unsigned char*p,unsigned long n,int*wr,int*hr,Pixmap*mr){int a,b;uint32_t*out=decode(p,n,&a,&b);if(!out)return None;Pixmap pm=xs_calloc(1,sizeof(*pm));*pm=(struct jwxyz_Drawable){a,b,32,out,0,1};if(wr)*wr=a;if(hr)*hr=b;if(mr){Pixmap m=XCreatePixmap(d,w,a,b,1);GC g=XCreateGC(d,m,0,NULL);for(int i=0;i<a*b;i++){XSetForeground(d,g,out[i]>>24?1:0);XDrawPoint(d,m,g,i%a,i/a);}XFreeGC(d,g);*mr=m;}for(int i=0;i<a*b;i++)out[i]|=0xff000000;return pm;}
XImage*image_data_to_ximage(Display*d,Visual*v,const unsigned char*p,unsigned long n){int a,b;uint32_t*out=decode(p,n,&a,&b);if(!out)return NULL;
  /* Upstream expects upside-down RGBA, unlike the drawable's ARGB. */
  XImage*i=XCreateImage(d,v,32,ZPixmap,0,NULL,a,b,8,0);i->data=xs_malloc(a*b*4);
  for(int y=0;y<b;y++)for(int x=0;x<a;x++){uint32_t c=out[y*a+x];XPutPixel(i,x,b-1-y,(c&0xff00ff00)|((c>>16)&255)|((c&255)<<16));}xs_free(out);return i;}
XImage*jwxyz_png_to_ximage(Display*d,Visual*v,const unsigned char*p,unsigned long n){return image_data_to_ximage(d,v,p,n);}
Pixmap file_to_pixmap(Display*d,Window w,const char*f,int*a,int*b,Pixmap*m){return image_data_to_pixmap(d,w,xs_picture,xs_picture_size,a,b,m);}
XImage*file_to_ximage(Display*d,Visual*v,const char*f){return image_data_to_ximage(d,v,xs_picture,xs_picture_size);}
static void load_picture(Display*d,Drawable target,XRectangle*r){int a,b;Pixmap p=image_data_to_pixmap(d,target,xs_picture,xs_picture_size,&a,&b,NULL);
  for(int y=0;y<target->h;y++)for(int x=0;x<target->w;x++)target->pixels[y*target->w+x]=p->pixels[(y*b/target->h)*a+x*a/target->w];
  XFreePixmap(d,p);if(r)*r=(XRectangle){0,0,target->w,target->h};}
void load_image_async(Screen*s,Window w,Drawable p,void(*cb)(Screen*,Window,Drawable,const char*,XRectangle*,void*),void*c){XRectangle r;load_picture(s,p,&r);if(cb)cb(s,w,p,"bundled SOM illustration",&r,c);}
struct async_load_state{XRectangle geometry;};
async_load_state*load_image_async_simple(async_load_state*state,Screen*s,Window w,Drawable p,char**name,XRectangle*r){
  if(state){if(r)*r=state->geometry;xs_free(state);if(name)*name=xs_strdup("bundled SOM illustration");return NULL;}
  async_load_state *next=xs_calloc(1,sizeof(*next));load_picture(s,p,&next->geometry);if(r)*r=next->geometry;return next;}
struct text_data{unsigned pos;};
static const char local_text[]="Welcome to XScreenSaver on the Alif E8.\nThe Cortex-M55 draws these animations using its CPU.\nPatterns, fractals, particles and classic computer displays.\nABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789\n";
text_data*textclient_open(Display*d){return xs_calloc(1,sizeof(text_data));}
void textclient_close(text_data*t){xs_free(t);}
void textclient_reshape(text_data*t,int a,int b,int c,int e,int f){}
int textclient_getc(text_data*t){if(!t)return -1;int c=local_text[t->pos++];if(!c){t->pos=0;return '\n';}return c;}
Bool textclient_puts(text_data*t,const char*s){return False;}Bool textclient_putc_event(text_data*t,XKeyEvent*e){return False;}
XtAppContext XtDisplayToApplicationContext(Display*d){return NULL;}
XtIntervalId XtAppAddTimeOut(XtAppContext a,unsigned long t,XtTimerCallbackProc f,XtPointer p){return NULL;}
void XtRemoveTimeOut(XtIntervalId a){}XtInputId XtAppAddInput(XtAppContext a,int fd,XtPointer fl,XtInputCallbackProc cb,XtPointer p){return NULL;}
void XtRemoveInput(XtInputId a){}XtInputMask XtAppPending(XtAppContext a){return 0;}void XtAppProcessEvent(XtAppContext a,XtInputMask m){}
void Log(const char*f,...){}void jwxyz_logv(Bool e,const char*f,va_list a){}

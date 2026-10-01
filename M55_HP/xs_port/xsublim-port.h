/* Nonblocking screenhack adaptation of the retired xsublim desktop overlay.
 * Original copyright and permission notice remain in hacks/xsublim.c.
 */
#include "screenhack.h"
struct sublim_state {GC gc;XFontStruct *font;unsigned next_word;int shown;};
static const char *const sublim_words[]={"Submit.","Conform.","Obey.","Consume.","Be silent.","Fear.","Waste.","Money.","Watch TV.","Buy needlessly.","Despair quietly.","You are being watched.","They know.","Surrender.","Never question.","Fear the unknown.","Ignorance is strength.","War is peace.","Freedom is slavery.","Abandon all hope.","Resistance is futile.","No escape.","What's that smell?","All praise the company.","Fnord."};
static void*sublim_init(Display*d,Window w){struct sublim_state*s=calloc(1,sizeof(*s));s->gc=XCreateGC(d,w,0,NULL);s->font=XLoadQueryFont(d,"monospace-20");XSetFont(d,s->gc,s->font->fid);return s;}
static unsigned long sublim_draw(Display*d,Window w,void*p){struct sublim_state*s=p;XClearWindow(d,w);if(s->shown){s->shown=0;return 100000;}const char*word=sublim_words[random()%countof(sublim_words)];XDrawString(d,w,s->gc,(240-XTextWidth(s->font,word,strlen(word)))/2,200,word,strlen(word));s->shown=1;return 40000;}
static void sublim_reshape(Display*d,Window w,void*p,unsigned a,unsigned b){}
static Bool sublim_event(Display*d,Window w,void*p,XEvent*e){return False;}
static void sublim_free(Display*d,Window w,void*p){struct sublim_state*s=p;XFreeGC(d,s->gc);XFreeFont(d,s->font);free(s);}
static const char*const sublim_defaults[]={".background: black",".foreground: white",NULL};
static XrmOptionDescRec sublim_options[]={{0,0,0,0}};
XSCREENSAVER_MODULE_2("XSublim",xsublim,sublim)

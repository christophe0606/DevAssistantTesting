/* CPU Xlib subset for the original XScreenSaver screenhack callbacks. */
#include "xs_internal.h"
#include <math.h>
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
static uint32_t window_pixels[XS_WIDTH*XS_HEIGHT]
#ifndef XS_HOST
 __attribute__((section(".bss.lcd_frame_buf"),aligned(32)))
#endif
 ;
struct jwxyz_Drawable xs_window={XS_WIDTH,XS_HEIGHT,32,window_pixels,0xff000000};
static Visual visual={TrueColor,0xff0000,0xff00,0xff,0xff000000};
static struct jwxyz_Font default_font={1};
static struct jwxyz_Drawable *root(Display *d) {return &xs_window;}
static Visual *get_visual_ptr(Display *d) {return &visual;}
static uint32_t pixel(Drawable d,int x,int y) {
  if(!d||x<0||y<0||x>=d->w||y>=d->h)return 0;
  if(d->depth==1)return(((unsigned char*)d->pixels)[y*((d->w+7)/8)+x/8]>>(x%8))&1;
  return d->pixels[y*d->w+x];
}
static uint32_t raster(uint32_t s,uint32_t t,int op) {
  switch(op) {case GXclear:return 0;case GXand:return s&t;case GXxor:return s^t;
    case GXor:return s|t;case GXset:return ~0U;default:return s;}
}
static void put(Drawable d,GC gc,int x,int y,uint32_t p) {
  if(!d || x<0 || y<0 || x>=d->w || y>=d->h) return;
  XGCValues *v=gc?&gc->v:NULL;
  if(v && v->clip_mask && !pixel(v->clip_mask,x-v->clip_x_origin,y-v->clip_y_origin)) return;
  if(v && v->stipple && v->fill_style!=FillSolid) {
    int sx=(x-v->ts_x_origin)%v->stipple->w,sy=(y-v->ts_y_origin)%v->stipple->h;
    if(sx<0)sx+=v->stipple->w;if(sy<0)sy+=v->stipple->h;
    if(!pixel(v->stipple,sx,sy)) {if(v->fill_style==FillStippled)return;p=v->background;}
  }
  if(d->depth==1){unsigned char*q=(unsigned char*)d->pixels+y*((d->w+7)/8)+x/8;unsigned bit=x%8;
    unsigned c=raster(p&1,(*q>>bit)&1,v?v->function:GXcopy)&1;*q=(*q&~(1<<bit))|(c<<bit);return;}
  uint32_t *q=&d->pixels[y*d->w+x];
  if(v && v->alpha_allowed_p && d->depth!=1 && (p>>24)<255) {
    unsigned a=p>>24,b=255-a;
    p=0xff000000|(((((p>>16)&255)*a+((*q>>16)&255)*b)/255)<<16)
      |(((((p>>8)&255)*a+((*q>>8)&255)*b)/255)<<8)|(((p&255)*a+(*q&255)*b)/255);
  }
  *q=raster(d->depth==1 ? p&1 : p,*q,v?v->function:GXcopy);
}
int XDrawPoint(Display *d,Drawable w,GC g,int x,int y) {put(w,g,x,y,g->v.foreground);return 0;}
static int draw_points(Display *d,Drawable w,GC g,XPoint *p,int n,int mode) {
  int x=0,y=0;for(int i=0;i<n;i++){x=mode?x+p[i].x:p[i].x;y=mode?y+p[i].y:p[i].y;XDrawPoint(d,w,g,x,y);}return 0;
}
int XFillRectangle(Display *d,Drawable w,GC g,int x,int y,unsigned width,unsigned height) {
  int x0=MAX(0,x),y0=MAX(0,y),x1=MIN(w->w,(int64_t)x+width),y1=MIN(w->h,(int64_t)y+height);
  for(int j=y0;j<y1;j++)for(int i=x0;i<x1;i++)put(w,g,i,j,g->v.foreground);return 0;
}
static int fill_polygon(Display *,Drawable,GC,XPoint *,int,int,int);
int XDrawLine(Display *d,Drawable w,GC g,int x0,int y0,int x1,int y1) {
  /* Clip before Bresenham: fractal coordinates can be far off-screen. */
  double dx=(double)x1-x0,dy=(double)y1-y0,t0=0,t1=1;
  double p[4]={-dx,dx,-dy,dy},q[4]={x0,w->w-1.0-x0,y0,w->h-1.0-y0};
  for(int i=0;i<4;i++){if(p[i]==0){if(q[i]<0)return 0;}else{double t=q[i]/p[i];if(p[i]<0)t0=MAX(t0,t);else t1=MIN(t1,t);}}
  if(t0>t1)return 0;
  x1=(int)(x0+t1*dx);y1=(int)(y0+t1*dy);x0=(int)(x0+t0*dx);y0=(int)(y0+t0*dy);
  int size=MAX(1,g->v.line_width);
  if(size>1){
    double length=hypot((double)x1-x0,(double)y1-y0);
    if(length==0)return XFillRectangle(d,w,g,x0-size/2,y0-size/2,size,size);
    double ox=-(y1-y0)*size/(2*length),oy=(x1-x0)*size/(2*length);
    XPoint quad[4]={{x0+ox,y0+oy},{x1+ox,y1+oy},{x1-ox,y1-oy},{x0-ox,y0-oy}};
    return fill_polygon(d,w,g,quad,4,Convex,CoordModeOrigin);
  }
  int ax=abs(x1-x0),sx=x0<x1?1:-1,ay=-abs(y1-y0),sy=y0<y1?1:-1,err=ax+ay;
  for(;;){put(w,g,x0,y0,g->v.foreground);
    if(x0==x1&&y0==y1)break;int e=2*err;if(e>=ay){err+=ay;x0+=sx;}if(e<=ax){err+=ax;y0+=sy;}}
  return 0;
}
static int draw_lines(Display *d,Drawable w,GC g,XPoint *p,int n,int mode) {
  if(n<2)return 0;int x=p[0].x,y=p[0].y;
  for(int i=1;i<n;i++){int nx=mode?x+p[i].x:p[i].x,ny=mode?y+p[i].y:p[i].y;XDrawLine(d,w,g,x,y,nx,ny);x=nx;y=ny;}return 0;
}
static int draw_segments(Display *d,Drawable w,GC g,XSegment *p,int n) {for(int i=0;i<n;i++)XDrawLine(d,w,g,p[i].x1,p[i].y1,p[i].x2,p[i].y2);return 0;}
int XDrawRectangle(Display *d,Drawable w,GC g,int x,int y,unsigned a,unsigned b) {
  XDrawLine(d,w,g,x,y,x+a,y);XDrawLine(d,w,g,x+a,y,x+a,y+b);XDrawLine(d,w,g,x+a,y+b,x,y+b);XDrawLine(d,w,g,x,y+b,x,y);return 0;
}
int XFillRectangles(Display *d,Drawable w,GC g,XRectangle *r,int n) {for(int i=0;i<n;i++)XFillRectangle(d,w,g,r[i].x,r[i].y,r[i].width,r[i].height);return 0;}
static int fill_polygon(Display *d,Drawable w,GC g,XPoint *in,int n,int shape,int mode) {
  if(n<3)return 0;
  XPoint *p=xs_malloc(n*sizeof(*p));int x=0,y=0,lo=w->h,hi=0;
  for(int i=0;i<n;i++){x=mode?x+in[i].x:in[i].x;y=mode?y+in[i].y:in[i].y;p[i]=(XPoint){x,y};lo=MIN(lo,y);hi=MAX(hi,y);}
  int *cuts=xs_malloc(n*sizeof(*cuts));
  for(y=MAX(0,lo);y<MIN(w->h,hi);y++) {
    int nc=0;
    for(int i=0,j=n-1;i<n;j=i++) if((p[i].y<=y&&p[j].y>y)||(p[j].y<=y&&p[i].y>y))
      cuts[nc++]=p[i].x+(int)((int64_t)(y-p[i].y)*(p[j].x-p[i].x)/(p[j].y-p[i].y));
    for(int i=1;i<nc;i++){int a=cuts[i],j=i;while(j&&cuts[j-1]>a){cuts[j]=cuts[j-1];j--;}cuts[j]=a;}
    for(int i=0;i+1<nc;i+=2)for(x=MAX(0,cuts[i]);x<MIN(w->w,cuts[i+1]);x++)put(w,g,x,y,g->v.foreground);
  }
  xs_free(cuts);xs_free(p);return 0;
}
static int arc(Display *d,Drawable w,GC g,int x,int y,unsigned a,unsigned b,int start,int sweep,Bool fill) {
  if(!a||!b)return 0;
  double rx=a*0.5,ry=b*0.5,cx=x+rx,cy=y+ry;
  if(fill && abs(sweep)>=360*64) {
    for(int j=MAX(0,y);j<MIN(w->h,(int64_t)y+b);j++){
      double dy=(j+0.5-cy)/ry,z=1-dy*dy;if(z<0)continue;double e=rx*sqrt(z);
      for(int i=MAX(0,(int)ceil(cx-e));i<MIN(w->w,(int)ceil(cx+e));i++)put(w,g,i,j,g->v.foreground);
    }return 0;
  }
  int steps=MAX(2,(int)(abs(sweep)/64.0*(rx+ry)*0.0175));steps=MIN(steps,2048);
  if(fill){XPoint *p=xs_malloc((steps+2)*sizeof(*p));p[0]=(XPoint){cx,cy};for(int i=0;i<=steps;i++){double t=(start+(double)sweep*i/steps)*M_PI/(180*64);p[i+1]=(XPoint){cx+rx*cos(t),cy-ry*sin(t)};}fill_polygon(d,w,g,p,steps+2,Convex,0);xs_free(p);}
  else {double t=start*M_PI/(180*64);int px=cx+rx*cos(t),py=cy-ry*sin(t);for(int i=1;i<=steps;i++){t=(start+(double)sweep*i/steps)*M_PI/(180*64);int nx=cx+rx*cos(t),ny=cy-ry*sin(t);XDrawLine(d,w,g,px,py,nx,ny);px=nx;py=ny;}}return 0;
}
int XDrawArc(Display *d,Drawable w,GC g,int x,int y,unsigned a,unsigned b,int s,int t){return arc(d,w,g,x,y,a,b,s,t,0);}
int XFillArc(Display *d,Drawable w,GC g,int x,int y,unsigned a,unsigned b,int s,int t){return arc(d,w,g,x,y,a,b,s,t,1);}
int XDrawArcs(Display *d,Drawable w,GC g,XArc *p,int n){for(int i=0;i<n;i++)XDrawArc(d,w,g,p[i].x,p[i].y,p[i].width,p[i].height,p[i].angle1,p[i].angle2);return 0;}
int XFillArcs(Display *d,Drawable w,GC g,XArc *p,int n){for(int i=0;i<n;i++)XFillArc(d,w,g,p[i].x,p[i].y,p[i].width,p[i].height,p[i].angle1,p[i].angle2);return 0;}
static void retain_pixmap(Pixmap p){if(p && p!=&xs_window)++p->references;}
static void replace_pixmap(Display*d,Pixmap *slot,Pixmap p){retain_pixmap(p);XFreePixmap(d,*slot);*slot=p;}
int XChangeGC(Display *d,GC g,unsigned long mask,XGCValues *v) {
#define FIELD(bit,f) if(mask&bit)g->v.f=v->f
  FIELD(GCFunction,function);FIELD(GCForeground,foreground);FIELD(GCBackground,background);FIELD(GCLineWidth,line_width);
  FIELD(GCCapStyle,cap_style);FIELD(GCJoinStyle,join_style);FIELD(GCFillRule,fill_rule);FIELD(GCFillStyle,fill_style);
  if(mask&GCStipple)replace_pixmap(d,&g->v.stipple,v->stipple);
  if(mask&GCTile)replace_pixmap(d,&g->v.tile,v->tile);
  if(mask&GCClipMask)replace_pixmap(d,&g->v.clip_mask,v->clip_mask);
  FIELD(GCFont,font);
  FIELD(GCClipXOrigin,clip_x_origin);FIELD(GCClipYOrigin,clip_y_origin);
#undef FIELD
  return 0;
}
static GC create_gc(Display *d,Drawable w,unsigned long mask,XGCValues *v) {
  GC g=xs_calloc(1,sizeof(*g));g->v.function=GXcopy;g->v.foreground=0xffffffff;g->v.background=0xff000000;g->v.font=&default_font;
  if(mask&&v)XChangeGC(d,g,mask,v);return g;
}
static int free_gc(Display *d,GC g){XFreePixmap(d,g->v.clip_mask);XFreePixmap(d,g->v.stipple);XFreePixmap(d,g->v.tile);xs_free(g);return 0;}
int XSetForeground(Display*d,GC g,unsigned long p){g->v.foreground=p;return 0;}
int XSetBackground(Display*d,GC g,unsigned long p){g->v.background=p;return 0;}
int XSetFunction(Display*d,GC g,int p){g->v.function=p;return 0;}
int XSetSubwindowMode(Display*d,GC g,int p){g->v.subwindow_mode=p;return 0;}
int XSetLineAttributes(Display*d,GC g,unsigned a,int b,int c,int e){g->v.line_width=a;g->v.cap_style=c;g->v.join_style=e;return 0;}
int jwxyz_XSetAlphaAllowed(Display*d,GC g,Bool p){g->v.alpha_allowed_p=p;return 0;}
int jwxyz_XSetAntiAliasing(Display*d,GC g,Bool p){g->v.antialias_p=p;return 0;}
int XSetGraphicsExposures(Display*d,GC g,Bool p){return 0;}
static int clip_mask(Display*d,GC g,Pixmap p){replace_pixmap(d,&g->v.clip_mask,p);return 0;}
static int clip_origin(Display*d,GC g,int x,int y){g->v.clip_x_origin=x;g->v.clip_y_origin=y;return 0;}
int XSetWindowBackground(Display*d,Window w,unsigned long p){w->background=p;return 0;}
int XClearArea(Display*d,Window w,int x,int y,int a,int b,Bool e){struct jwxyz_GC g={0};g.v.function=GXcopy;g.v.foreground=w->background;return XFillRectangle(d,w,&g,x,y,a?a:w->w-x,b?b:w->h-y);}
static int clear_window(Display*d,Window w){return XClearArea(d,w,0,0,w->w,w->h,False);}
int XDisplayWidth(Display*d,int n){return XS_WIDTH;}int XDisplayHeight(Display*d,int n){return XS_HEIGHT;}
int XDisplayWidthMM(Display*d,int n){return 60;}int XDisplayHeightMM(Display*d,int n){return 100;}
unsigned long XBlackPixelOfScreen(Screen*s){return 0xff000000;}unsigned long XWhitePixelOfScreen(Screen*s){return 0xffffffff;}unsigned long XCellsOfScreen(Screen*s){return 256;}
Status XGetWindowAttributes(Display*d,Window w,XWindowAttributes*a){memset(a,0,sizeof(*a));a->width=w->w;a->height=w->h;a->depth=w->depth;a->visual=&visual;a->screen=d;return True;}
Status XGetGeometry(Display*d,Drawable w,Window*r,int*x,int*y,unsigned*a,unsigned*b,unsigned*c,unsigned*dep){if(r)*r=&xs_window;if(x)*x=0;if(y)*y=0;if(a)*a=w->w;if(b)*b=w->h;if(c)*c=0;if(dep)*dep=w->depth;return True;}
Pixmap XCreatePixmap(Display*d,Drawable w,unsigned a,unsigned b,unsigned dep){if(!a||!b||a>4096||b>4096)jwxyz_abort("invalid pixmap %ux%u",a,b);Pixmap p=xs_calloc(1,sizeof(*p));p->w=a;p->h=b;p->depth=dep;p->references=1;p->pixels=dep==1?xs_calloc(b,(a+7)/8):xs_calloc((size_t)a*b,4);return p;}
int XFreePixmap(Display*d,Pixmap p){if(p&&p!=&xs_window && --p->references==0){xs_free(p->pixels);xs_free(p);}return 0;}
int XCopyArea(Display*d,Drawable src,Drawable dst,GC g,int sx,int sy,unsigned a,unsigned b,int dx,int dy){
  int xa=MAX(0,MAX(-sx,-dx)),ya=MAX(0,MAX(-sy,-dy)),xb=MIN(a,MIN(src->w-sx,dst->w-dx)),yb=MIN(b,MIN(src->h-sy,dst->h-dy));
  int reverse=src==dst&&(dy>sy||(dy==sy&&dx>sx));
  for(int j=ya;j<yb;j++)for(int i=xa;i<xb;i++){int xx=reverse?xb-1-(i-xa):i,yy=reverse?yb-1-(j-ya):j;put(dst,g,dx+xx,dy+yy,pixel(src,sx+xx,sy+yy));}return 0;
}
static void copy_area(Display*d,Drawable s,Drawable t,GC g,int x,int y,unsigned a,unsigned b,int xx,int yy){XCopyArea(d,s,t,g,x,y,a,b,xx,yy);}
int XCopyPlane(Display*d,Drawable s,Drawable t,GC g,int x,int y,unsigned a,int b,int xx,int yy,unsigned long plane){for(int j=MAX(0,-yy);j<MIN(b,t->h-yy);j++)for(int i=MAX(0,-xx);i<MIN(a,t->w-xx);i++)put(t,g,xx+i,yy+j,(pixel(s,x+i,y+j)&plane)?g->v.foreground:g->v.background);return 0;}
Status XInitImage(XImage *i){i->f.get_pixel=XGetPixel;i->f.put_pixel=XPutPixel;return 1;}
XImage *XCreateImage(Display*d,Visual*v,unsigned dep,int fmt,int off,char*data,unsigned a,unsigned b,int pad,int stride){XImage*i=xs_calloc(1,sizeof(*i));i->width=a;i->height=b;i->depth=dep;i->format=fmt;i->data=data;i->bits_per_pixel=dep==1?1:32;i->bytes_per_line=stride?stride:(dep==1?(a+7)/8:a*4);i->bitmap_pad=pad;i->byte_order=LSBFirst;i->bitmap_bit_order=MSBFirst;i->red_mask=visual.red_mask;i->green_mask=visual.green_mask;i->blue_mask=visual.blue_mask;XInitImage(i);return i;}
unsigned long XGetPixel(XImage*i,int x,int y){if(!i||!i->data||x<0||y<0||x>=i->width||y>=i->height)return 0;unsigned char*p=(unsigned char*)i->data+y*i->bytes_per_line;
  if(i->bits_per_pixel==1)return(p[x/8]>>(i->bitmap_bit_order==MSBFirst?7-x%8:x%8))&1;
  if(i->bits_per_pixel==8)return p[x];if(i->bits_per_pixel==16){uint16_t v;memcpy(&v,p+x*2,2);return v;}uint32_t v;memcpy(&v,p+x*4,4);return v;}
int XPutPixel(XImage*i,int x,int y,unsigned long v){if(!i||!i->data||x<0||y<0||x>=i->width||y>=i->height)return 0;unsigned char*p=(unsigned char*)i->data+y*i->bytes_per_line;
  if(i->bits_per_pixel==1){int bit=i->bitmap_bit_order==MSBFirst?7-x%8:x%8;p[x/8]=(p[x/8]&~(1<<bit))|((v&1)<<bit);}else if(i->bits_per_pixel==8)p[x]=v;else if(i->bits_per_pixel==16){uint16_t z=v;memcpy(p+x*2,&z,2);}else{uint32_t z=v;memcpy(p+x*4,&z,4);}return 0;}
int XDestroyImage(XImage*i){if(i){xs_free(i->data);xs_free(i);}return 0;}
XImage*XGetImage(Display*d,Drawable w,int x,int y,unsigned a,unsigned b,unsigned long mask,int fmt){XImage*i=XCreateImage(d,&visual,mask==1?1:w->depth,fmt,0,NULL,a,b,8,0);i->data=xs_calloc(b,i->bytes_per_line);for(int yy=0;yy<(int)b;yy++)for(int xx=0;xx<(int)a;xx++)XPutPixel(i,xx,yy,pixel(w,x+xx,y+yy)&mask);return i;}
XImage*XSubImage(XImage*s,int x,int y,unsigned a,unsigned b){XImage*i=XCreateImage(&xs_display,&visual,s->depth,s->format,0,NULL,a,b,8,0);i->data=xs_calloc(b,i->bytes_per_line);for(int yy=0;yy<(int)b;yy++)for(int xx=0;xx<(int)a;xx++)XPutPixel(i,xx,yy,XGetPixel(s,x+xx,y+yy));return i;}
static int put_image(Display*d,Drawable w,GC g,XImage*i,int x,int y,int xx,int yy,unsigned a,unsigned b){for(int j=MAX(0,-yy);j<MIN(b,w->h-yy);j++)for(int k=MAX(0,-xx);k<MIN(a,w->w-xx);k++){uint32_t p=XGetPixel(i,x+k,y+j);if(i->depth==1)p=p?g->v.foreground:g->v.background;put(w,g,xx+k,yy+j,p);}return 0;}
static XImage*get_subimage(Display*d,Drawable w,int x,int y,unsigned a,unsigned b,unsigned long m,int f,XImage*i,int xx,int yy){for(int j=0;j<(int)b;j++)for(int k=0;k<(int)a;k++)XPutPixel(i,xx+k,yy+j,pixel(w,x+k,y+j)&m);return i;}
Pixmap XCreatePixmapFromBitmapData(Display*d,Drawable w,const char*data,unsigned a,unsigned b,unsigned long fg,unsigned long bg,unsigned dep){Pixmap p=XCreatePixmap(d,w,a,b,dep);for(int y=0;y<(int)b;y++)for(int x=0;x<(int)a;x++)put(p,NULL,x,y,((data[y*((a+7)/8)+x/8]>>(x%8))&1)?fg:bg);return p;}
XPixmapFormatValues*XListPixmapFormats(Display*d,int*n){XPixmapFormatValues*p=xs_malloc(2*sizeof(*p));p[0]=(XPixmapFormatValues){1,1,8};p[1]=(XPixmapFormatValues){32,32,32};*n=2;return p;}
Bool XQueryPointer(Display*d,Window w,Window*r,Window*c,int*rx,int*ry,int*x,int*y,unsigned*m){if(r)*r=&xs_window;if(c)*c=0;if(rx)*rx=w->w/2;if(ry)*ry=w->h/2;if(x)*x=w->w/2;if(y)*y=w->h/2;if(m)*m=0;return False;}
Bool XTranslateCoordinates(Display*d,Window s,Window t,int x,int y,int*xx,int*yy,Window*c){if(xx)*xx=x;if(yy)*yy=y;if(c)*c=0;return True;}
int XLookupString(XKeyEvent*e,char*r,int n,KeySym*k,XComposeStatus*c){if(k)*k=e->keycode;if(n){r[0]=e->keycode;return 1;}return 0;}
KeySym XKeycodeToKeysym(Display*d,KeyCode k,int i){return k;}
int XFlush(Display*d){return 0;}int XSync(Display*d,Bool b){return 0;}
int XSetFont(Display*d,GC g,Font f){g->v.font=f;return 0;}
Font XLoadFont(Display*d,const char*n){Font f=xs_malloc(sizeof(*f));f->scale=1;int a=0;if(n){const char*p=strrchr(n,'-');if(p)a=atoi(p+1);else{p=strchr(n,':');if(p)a=atoi(p+1);}}f->scale=MAX(1,MIN(4,a/10));return f;}
XFontStruct*XQueryFont(Display*d,Font f){XFontStruct*x=xs_calloc(1,sizeof(*x));x->fid=f;x->min_char_or_byte2=0;x->max_char_or_byte2=255;x->ascent=8*f->scale;x->descent=2*f->scale;x->min_bounds=x->max_bounds=(XCharStruct){0,6*f->scale,7*f->scale,x->ascent,x->descent};x->per_char=xs_malloc(256*sizeof(XCharStruct));for(int i=0;i<256;i++)x->per_char[i]=x->max_bounds;return x;}
XFontStruct*XLoadQueryFont(Display*d,const char*n){return XQueryFont(d,XLoadFont(d,n));}
int XFreeFontInfo(char**n,XFontStruct*f,int count){for(int i=0;i<count;i++){xs_free(f[i].properties);xs_free(f[i].per_char);}xs_free(f);return 0;}
int XUnloadFont(Display*d,Font f){if(f!=&default_font)xs_free(f);return 0;}
int XFreeFont(Display*d,XFontStruct*f){XUnloadFont(d,f->fid);return XFreeFontInfo(NULL,f,1);}
int XTextWidth(XFontStruct*f,const char*s,int n){return n*f->max_bounds.width;}
int XTextExtents(XFontStruct*f,const char*s,int n,int*dir,int*a,int*b,XCharStruct*c){if(dir)*dir=0;if(a)*a=f->ascent;if(b)*b=f->descent;if(c){*c=f->max_bounds;c->width=c->rbearing=n*f->max_bounds.width;}return 0;}
int XTextExtents16(XFontStruct*f,const XChar2b*s,int n,int*dir,int*a,int*b,XCharStruct*c){return XTextExtents(f,NULL,n,dir,a,b,c);}
static void glyph(Display*d,Drawable w,GC g,int x,int y,unsigned c){int scale=g->v.font?g->v.font->scale:1;c=c<256?c:'?';for(int yy=0;yy<10;yy++)for(int xx=0;xx<7;xx++)if(xs_font_bits[c][yy]&(1<<xx))XFillRectangle(d,w,g,x+xx*scale,y-8*scale+yy*scale,scale,scale);}
int XDrawString(Display*d,Drawable w,GC g,int x,int y,const char*s,int n){int scale=g->v.font?g->v.font->scale:1;for(int i=0;i<n;i++)glyph(d,w,g,x+i*7*scale,y,(unsigned char)s[i]);return 0;}
int XDrawString16(Display*d,Drawable w,GC g,int x,int y,const XChar2b*s,int n){int scale=g->v.font?g->v.font->scale:1;for(int i=0;i<n;i++)glyph(d,w,g,x+i*7*scale,y,(s[i].byte1<<8)|s[i].byte2);return 0;}
int XDrawImageString(Display*d,Drawable w,GC g,int x,int y,const char*s,int n){unsigned long p=g->v.foreground;int scale=g->v.font?g->v.font->scale:1;g->v.foreground=g->v.background;XFillRectangle(d,w,g,x,y-8*scale,n*7*scale,10*scale);g->v.foreground=p;return XDrawString(d,w,g,x,y,s,n);}
char*jwxyz_unicode_character_name(Display*d,Font f,unsigned long c){return xs_strdup("character");}
char*XGetAtomName(Display*d,Atom a){return xs_strdup(a==XA_FONT?"FONT":"property");}
static struct jwxyz_vtbl vtbl={.root=root,.visual=get_visual_ptr,.draw_arc=arc,.copy_area=copy_area,.DrawPoints=draw_points,.DrawSegments=draw_segments,.CreateGC=create_gc,.FreeGC=free_gc,.ClearWindow=clear_window,.SetClipMask=clip_mask,.SetClipOrigin=clip_origin,.FillPolygon=fill_polygon,.DrawLines=draw_lines,.PutImage=put_image,.GetSubImage=get_subimage};
Display xs_display={&vtbl};
void xs_platform_reset(void){xs_window.background=0xff000000;for(int i=0;i<XS_WIDTH*XS_HEIGHT;i++)window_pixels[i]=xs_window.background;}

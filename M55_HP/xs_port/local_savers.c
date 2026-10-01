/* Local versions of the two desktop scripts: collage and random image filters.
 * Based on WebCollage and VidWhacker by Jamie Zawinski, 1998-2026.
 * Their original scripts and permission notices are retained in third_party.
 */
#include "xs_internal.h"
#include "ximage-loader.h"
#include "yarandom.h"
void xs_local_draw(unsigned kind,unsigned frame){
  if(frame%30)return;
  int w,h;
  Pixmap p=image_data_to_pixmap(&xs_display,&xs_window,xs_picture,xs_picture_size,&w,&h,NULL);
  if(kind==0){
    int bw=60+random()%120,bh=60+random()%160,x0=random()%XS_WIDTH-bw/3,y0=random()%XS_HEIGHT-bh/3;
    int flip=random()&1;
    for(int y=0;y<bh;y++)for(int x=0;x<bw;x++)if(x0+x>=0&&y0+y>=0&&x0+x<XS_WIDTH&&y0+y<XS_HEIGHT){
      uint32_t c=p->pixels[(y*h/bh)*w+(flip?w-1-x*w/bw:x*w/bw)];
      uint32_t old=xs_window.pixels[(y0+y)*XS_WIDTH+x0+x];
      xs_window.pixels[(y0+y)*XS_WIDTH+x0+x]=0xff000000|(((c&0xfefefe)>>1)+((old&0xfefefe)>>1));
    }
  }else{
    unsigned filter=random()%4,shift=1+random()%25;
    for(int y=0;y<XS_HEIGHT;y++)for(int x=0;x<XS_WIDTH;x++){
      int sx=x*w/XS_WIDTH,sy=y*h/XS_HEIGHT;uint32_t c=p->pixels[sy*w+sx];
      if(filter==0)c^=p->pixels[((sy+shift)%h)*w+(w-1-sx)];
      else if(filter==1){uint32_t q=p->pixels[sy*w+(sx+1)%w];unsigned r=abs((int)((c>>16)&255)-(int)((q>>16)&255))*3,g=abs((int)((c>>8)&255)-(int)((q>>8)&255))*3,b=abs((int)(c&255)-(int)(q&255))*3;c=((r>255?255:r)<<16)|((g>255?255:g)<<8)|(b>255?255:b);}
      else if(filter==2)c=(~c)&0xe0e0e0;
      else c=p->pixels[((sy+shift*(sx/16))%h)*w+sx];
      xs_window.pixels[y*XS_WIDTH+x]=c|0xff000000;
    }
  }
  XFreePixmap(&xs_display,p);
}

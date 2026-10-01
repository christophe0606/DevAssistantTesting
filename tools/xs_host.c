#include "xs_internal.h"
#include <stdio.h>
#include <stdlib.h>
#undef malloc
#undef calloc
#undef free
#undef exit
#undef abort
static uint16_t lcd[480*800+2];
extern int xs_memory_validate(void);
static int rotation(void){
  uint32_t now=UINT32_MAX-5000U;
  unsigned count=xs_gallery_count();
  xs_gallery_init(now);
  /* A slow first frame must still get its full visible slot. */
  now+=15000;
  xs_gallery_update(now);
  if(xs_current_saver!=0)return 9;
  xs_gallery_presented(now);
  for(unsigned slot=0;slot<count*2;slot++){
    unsigned expected=slot%count;
    for(unsigned elapsed=0;elapsed<10000;elapsed+=33){
      xs_gallery_update(now+elapsed);
      xs_gallery_presented(now+elapsed);
      if(xs_current_saver!=expected || xs_last_error || !xs_memory_validate()){
        fprintf(stderr,"rotation %u %s: %s\n",slot,xs_gallery_name(expected),xs_error_text);return 5;
      }
    }
    xs_gallery_update(now+9999);
    if(xs_current_saver!=expected)return 6;
    lcd[480*800]=0x531e;lcd[480*800+1]=0x1234;xs_gallery_render(lcd);
    if(lcd[480*800]!=0x531e||lcd[480*800+1]!=0x1234)return 4;
    now+=10000;
    xs_gallery_update(now);
    xs_gallery_presented(now);
    if(xs_current_saver!=(expected+1)%count || xs_last_error || !xs_memory_validate())return 7;
    printf("rotation slot=%u %s OK\n",slot,xs_gallery_name(expected));fflush(stdout);
  }
  return 0;
}
int main(int argc,char **argv){
  if(argc<2){printf("%u\n",xs_gallery_count());return 0;}
  if(!strcmp(argv[1],"--rotation"))return rotation();
  unsigned index=strtoul(argv[1],NULL,0);uint32_t now=0;
  if(argc>3)now=(uint32_t)strtoul(argv[3],NULL,0);
  unsigned changed=0;size_t peak=0;
  for(unsigned cycle=0;cycle<3;cycle++){
    if(!xs_gallery_select(index,now)){fprintf(stderr,"init: %s\n",xs_error_text);return 1;}
    for(unsigned i=0;i<300;i++){
      xs_gallery_update(now+i*33);
      xs_gallery_presented(now+i*33);
      if(xs_last_error){fprintf(stderr,"draw: %s\n",xs_error_text);return 1;}
      if(!xs_memory_validate()){fprintf(stderr,"allocator corruption\n");return 8;}
      if(xs_memory_peak>peak)peak=xs_memory_peak;
      if(!cycle){unsigned varied=0;uint32_t first=xs_window.pixels[0];for(unsigned j=0;j<XS_WIDTH*XS_HEIGHT;j++)if(xs_window.pixels[j]!=first)varied++;
        if(varied>changed)changed=varied;}
    }
    if(!cycle){
      if(argc>2){FILE*f=fopen(argv[2],"wb");if(!f)return 3;fprintf(f,"P6\n%d %d\n255\n",XS_WIDTH,XS_HEIGHT);
        for(unsigned i=0;i<XS_WIDTH*XS_HEIGHT;i++){uint32_t p=xs_window.pixels[i];unsigned char rgb[3]={p>>16,p>>8,p};fwrite(rgb,1,3,f);}fclose(f);}
    }
    now+=10000;
  }
  lcd[480*800]=0x531e;lcd[480*800+1]=0x1234;xs_gallery_render(lcd);
  if(lcd[480*800]!=0x531e||lcd[480*800+1]!=0x1234)return 4;
  printf("%s varied_pixels=%u peak_bytes=%zu\n",xs_gallery_name(index),changed,peak);
  return changed?0:2;
}

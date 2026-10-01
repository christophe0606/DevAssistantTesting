#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif
#include "xs_internal.h"
#include "pacman.h"
#include "pacman_level.h"
#include <stdio.h>
#include <stdlib.h>
#undef malloc
#undef calloc
#undef free
#undef exit
#undef abort
#undef ya_rand_init
extern void ya_rand_init(unsigned seed);
static uint16_t lcd[480*800+2];
extern int xs_memory_validate(void);
static int rotation(void);
extern int xs_pacman_ghost_path_test(pacmangamestruct *,int,int,unsigned *,unsigned *);
static int pacman_paths(unsigned count){
  for(unsigned seed=1;seed<=count;seed++){
    pacmangamestruct game={0};xs_memory_reset();ya_rand_init(seed);
    pacman_createnewlevel(&game);
    /* Find the jail's connected component independently of the DFS. Blank
     * outside cells and even isolated generated dots need not reach it. */
    unsigned char connected[LEVWIDTH*LEVHEIGHT]={0};
    int queue[LEVWIDTH*LEVHEIGHT],head=0,tail=0,cx,cy;
    pacman_get_jail_opening(&cx,&cy);cy++;
    queue[tail++]=cy*LEVWIDTH+cx;connected[queue[0]]=1;
    while(head<tail){
      int cell=queue[head++],row=cell/LEVWIDTH,col=cell%LEVWIDTH;
      const int dy[4]={-1,0,1,0},dx[4]={0,1,0,-1};
      for(int d=0;d<4;d++){
        int y=row+dy[d],x=col+dx[d];
        if(y<0 || x<0 || y>=LEVHEIGHT || x>=LEVWIDTH)continue;
        int next=y*LEVWIDTH+x;
        if(!connected[next] && pacman_check_pos(&game,y,x,True)){
          connected[next]=1;queue[tail++]=next;
        }
      }
    }
    for(int side=0;side<4;side++){
      int reverse=side&1,cell=reverse?(int)(LEVWIDTH*LEVHEIGHT)-1:0;
      while(cell>=0 && cell<(int)(LEVWIDTH*LEVHEIGHT) &&
            (!pacman_check_pos(&game,cell/LEVWIDTH,cell%LEVWIDTH,True) || (side>=2 && !connected[cell])))cell+=reverse?-1:1;
      unsigned hash=0,length=0;
      fprintf(stderr,"path selecting seed=%u side=%d cell=%d\n",seed,side,cell);
      if(cell<0 || cell>=(int)(LEVWIDTH*LEVHEIGHT))return 14;
      int found=xs_pacman_ghost_path_test(&game,cell/LEVWIDTH,cell%LEVWIDTH,&hash,&length);
      if(found!=connected[cell] || !xs_memory_validate())return 14;
      printf("path seed=%u side=%d found=%d length=%u hash=%08x\n",seed,side,found,length,hash);
    }
    xs_free(game.tiles);
  }
  return 0;
}
/* Test level generation directly with varied seeds: gallery-only checks can
 * happen to choose the preset maze and miss the procedural generator. */
static int pacman_levels(unsigned count){
  for(unsigned seed=1;seed<=count;seed++){
    pacmangamestruct game={0};
    xs_memory_reset();ya_rand_init(seed);
    for(unsigned round=0;round<3;round++){
      int preset=pacman_createnewlevel(&game);
      uint32_t hash=2166136261U;
      for(unsigned i=0;i<sizeof(game.level);i++)hash=(hash^(unsigned char)game.level[i])*16777619U;
      unsigned next_random=ya_random();
      if(!game.dotsleft || !xs_memory_validate())return 10;
      printf("seed=%u round=%u preset=%d dots=%u hash=%08x rng=%08x\n",seed,round,preset,game.dotsleft,hash,next_random);
    }
    xs_free(game.tiles);
    if(!xs_memory_validate())return 11;
  }
  return 0;
}
#ifdef _WIN32
static DWORD WINAPI pacman_small_stack(void *arg){return (DWORD)pacman_levels(*(unsigned *)arg);}
static DWORD WINAPI pacman_paths_thread(void *arg){return (DWORD)pacman_paths(*(unsigned *)arg);}
static int pacman_paths_stack_test(unsigned count){
  HANDLE thread=CreateThread(NULL,65536,pacman_paths_thread,&count,STACK_SIZE_PARAM_IS_A_RESERVATION,NULL);
  if(!thread)return 12;
  WaitForSingleObject(thread,INFINITE);
  DWORD result=13;GetExitCodeThread(thread,&result);CloseHandle(thread);return (int)result;
}
static int pacman_stack_test(unsigned count){
  HANDLE thread=CreateThread(NULL,65536,pacman_small_stack,&count,STACK_SIZE_PARAM_IS_A_RESERVATION,NULL);
  if(!thread)return 12;
  WaitForSingleObject(thread,INFINITE);
  DWORD result=13;GetExitCodeThread(thread,&result);CloseHandle(thread);
  return (int)result;
}
static DWORD WINAPI rotation_small_stack(void *arg){
  ya_rand_init(*(unsigned *)arg);
  return (DWORD)rotation();
}
static int rotation_stack_test(unsigned seed){
  HANDLE thread=CreateThread(NULL,65536,rotation_small_stack,&seed,STACK_SIZE_PARAM_IS_A_RESERVATION,NULL);
  if(!thread)return 12;
  WaitForSingleObject(thread,INFINITE);
  DWORD result=13;GetExitCodeThread(thread,&result);CloseHandle(thread);
  return (int)result;
}
#endif
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
    printf("rotation selecting=%u %s\n",(expected+1)%count,xs_gallery_name((expected+1)%count));fflush(stdout);
    xs_gallery_update(now);
    xs_gallery_presented(now);
    if(xs_current_saver!=(expected+1)%count || xs_last_error || !xs_memory_validate())return 7;
    printf("rotation slot=%u %s OK\n",slot,xs_gallery_name(expected));fflush(stdout);
  }
  return 0;
}
int main(int argc,char **argv){
#ifdef _WIN32
  /* Failing stress tests should report an exit code without opening WER UI. */
  SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
#endif
  if(argc<2){printf("%u\n",xs_gallery_count());return 0;}
  if(!strcmp(argv[1],"--rotation"))return rotation();
  if(!strcmp(argv[1],"--pacman-levels"))return pacman_levels(argc>2?strtoul(argv[2],NULL,0):256);
  if(!strcmp(argv[1],"--pacman-paths"))return pacman_paths(argc>2?strtoul(argv[2],NULL,0):256);
#ifdef _WIN32
  if(!strcmp(argv[1],"--pacman-small-stack"))return pacman_stack_test(argc>2?strtoul(argv[2],NULL,0):256);
  if(!strcmp(argv[1],"--pacman-paths-small-stack"))return pacman_paths_stack_test(argc>2?strtoul(argv[2],NULL,0):256);
  if(!strcmp(argv[1],"--rotation-small-stack"))return rotation_stack_test(argc>2?(unsigned)strtoul(argv[2],NULL,0):0x67d34912U);
#endif
  unsigned index=strtoul(argv[1],NULL,0);uint32_t now=0;
  if(argc>3)now=(uint32_t)strtoul(argv[3],NULL,0);
  if(argc>4)ya_rand_init((unsigned)strtoul(argv[4],NULL,0));
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

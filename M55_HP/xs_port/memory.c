/* Bounded allocator shared by one screensaver at a time. */
#include "xs_internal.h"
#include <limits.h>
#include <ctype.h>
typedef struct __attribute__((aligned(16))) block { size_t size; struct block *next; int available; } block;
static unsigned char arena0[XS_ARENA0_BYTES]
#ifndef XS_HOST
  __attribute__((section(".bss.xs_arena0"), aligned(32)))
#else
  __attribute__((aligned(32)))
#endif
  ;
static unsigned char arena1[XS_ARENA1_BYTES]
#ifndef XS_HOST
  __attribute__((section(".bss.xs_arena1"), aligned(32)))
#else
  __attribute__((aligned(32)))
#endif
  ;
static block *head;
static size_t used;
volatile size_t xs_memory_peak;
void xs_memory_reset(void) {
  block *second = (block *)arena1;
  second->size = sizeof(arena1)-sizeof(block);
  second->next = NULL; second->available = 1;
  head = (block *)arena0; head->size = sizeof(arena0)-sizeof(block);
  head->next = second; head->available = 1; used = 0; xs_memory_peak = 0;
}
#ifdef XS_HOST
/* Detect allocator damage caused by a module before the next reset hides it. */
int xs_memory_validate(void) {
  unsigned char *end=arena0+sizeof(arena0), *expected=arena0;
  size_t allocated=0;
  int pool=0;
  for(block *b=head;b;b=b->next) {
    if((unsigned char *)b!=expected || (size_t)(end-expected)<sizeof(block))return 0;
    if(b->size>(size_t)(end-expected)-sizeof(block) || b->size%16)return 0;
    expected=(unsigned char *)(b+1)+b->size;
    if(!b->available)allocated+=b->size;
    if(expected==end && pool==0) {
      if((unsigned char *)b->next!=arena1)return 0;
      pool=1;expected=arena1;end=arena1+sizeof(arena1);
    } else if((b->next && (unsigned char *)b->next!=expected) || (!b->next && expected!=end))return 0;
  }
  return pool==1 && allocated==used;
}
#endif
void *xs_malloc(size_t n) {
  if (!n) n=1;
  if (n > SIZE_MAX-15) jwxyz_abort("allocation overflow");
  n=(n+15)&~(size_t)15;
  for (block *b=head; b; b=b->next) if (b->available && b->size>=n) {
    if (b->size >= n+sizeof(block)+16) {
      block *t=(block *)((unsigned char *)(b+1)+n);
      t->size=b->size-n-sizeof(block); t->next=b->next; t->available=1;
      b->next=t; b->size=n;
    }
    b->available=0; used+=b->size;
    if (used>xs_memory_peak) xs_memory_peak=used;
    return b+1;
  }
  jwxyz_abort("screensaver exceeds %u-byte arena", XS_ARENA_BYTES);
}
void *xs_calloc(size_t n,size_t s) {
  if (s && n>SIZE_MAX/s) jwxyz_abort("allocation overflow");
  void *p=xs_malloc(n*s); memset(p,0,n*s); return p;
}
void xs_free(void *p) {
  if (!p) return;
  uintptr_t address=(uintptr_t)p;
  if (!((address>=(uintptr_t)arena0+sizeof(block) && address<(uintptr_t)arena0+sizeof(arena0)) ||
        (address>=(uintptr_t)arena1+sizeof(block) && address<(uintptr_t)arena1+sizeof(arena1))))
    jwxyz_abort("invalid free");
  block *b=(block *)p-1;
  if (b->available) jwxyz_abort("double free");
  b->available=1; used-=b->size;
  for (b=head;b && b->next;) {
    if (b->available && b->next->available && b->next!=(block *)arena1 &&
        (unsigned char *)(b+1)+b->size==(unsigned char *)b->next) {
      b->size+=sizeof(block)+b->next->size; b->next=b->next->next;
    } else b=b->next;
  }
}
void *xs_realloc(void *p,size_t n) {
  if (!p) return xs_malloc(n);
  if (!n) { xs_free(p); return NULL; }
  block *b=(block *)p-1;
  if (b->size>=n) return p;
  void *q=xs_malloc(n); memcpy(q,p,b->size); xs_free(p); return q;
}
char *xs_strdup(const char *s) { if(!s) return NULL; size_t n=strlen(s)+1; char *p=xs_malloc(n); memcpy(p,s,n);return p; }
int xs_strncasecmp(const char *a,const char *b,size_t n) {
  while(n--) { int x=tolower((unsigned char)*a++),y=tolower((unsigned char)*b++);if(x!=y || !x) return x-y; }return 0;
}
int xs_strcasecmp(const char *a,const char *b) { return xs_strncasecmp(a,b,SIZE_MAX); }

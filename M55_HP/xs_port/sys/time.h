#ifndef XS_ALIF_TIME_H
#define XS_ALIF_TIME_H
#include <time.h>
#ifndef _TIMEVAL_DEFINED
#define _TIMEVAL_DEFINED
struct timeval { long tv_sec, tv_usec; };
#endif
struct timezone { int tz_minuteswest, tz_dsttime; };
int gettimeofday(struct timeval *, void *);
#endif

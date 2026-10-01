#ifndef XS_ALIF_CONFIG_H
#define XS_ALIF_CONFIG_H
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#define M_LOG2E 1.4426950408889634074
#define M_PI_4 0.7853981633974483096
#define M_SQRT2 1.4142135623730950488
#define XS_ALIF 1
#ifdef XS_HOST
#define XS_ASSET
#else
#define XS_ASSET __attribute__((section(".mram_user")))
#endif
#define time xs_time
time_t xs_time(time_t *);
#define HAVE_UNISTD_H 1
#define GETTIMEOFDAY_TWO_ARGS 1
typedef struct { int type; } XErrorEvent;
#define HAVE_CONFIG_H 1
#define HAVE_JWXYZ 1
#define HAVE_MOBILE 1
#define HAVE_INTTYPES_H 1
#define HAVE_TIME_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_STRCHR 1
#define HAVE_STRDUP 1
#define STANDALONE 1
#define PACKAGE_VERSION "6.16"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
void *xs_malloc(size_t);
void *xs_calloc(size_t, size_t);
void *xs_realloc(void *, size_t);
void xs_free(void *);
char *xs_strdup(const char *);
int xs_strcasecmp(const char *, const char *);
int xs_strncasecmp(const char *, const char *, size_t);
char *strtok_r(char *, const char *, char **);
char *strcasestr(const char *, const char *);
int gethostname(char *, size_t);
#define malloc xs_malloc
#define calloc xs_calloc
#define realloc xs_realloc
#define free xs_free
#define strdup xs_strdup
#define strcasecmp xs_strcasecmp
#define strncasecmp xs_strncasecmp
#endif

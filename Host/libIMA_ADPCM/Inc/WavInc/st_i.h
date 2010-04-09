#ifndef ST_I_H
#define ST_I_H
/*
 * Sound Tools Interal - October 11, 2001
 *
 *   This file is meant for libst internal use only
 *
 * Copyright 2001 Chris Bagwell
 *
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained.
 * Chris Bagwell And Sundry Contributors are not responsible for
 * the consequences of using this software.
 */

#include "st.h"
#include "stconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

//#ifdef HAVE_BYTESWAP_H
//#include <byteswap.h>
//#endif

/* various gcc optimizations and portablity defines */
#ifdef __GNUC__
#define NORET __attribute__((noreturn))
#else
#define NORET
#endif

#ifdef USE_REGPARM
#define REGPARM(n) __attribute__((regparm(n)))
#else
#define REGPARM(n)
#endif

static int st_writes(st_signalinfo_t * sinfo, char *c);
static int st_writeb(st_signalinfo_t * sinfo, uint8_t ub);
static int st_writew(st_signalinfo_t * sinfo, uint16_t uw);
static int st_writedw(st_signalinfo_t * sinfo, uint32_t udw);
static int st_reads(FILE *fp, char *c, st_ssize_t len);
static int st_readw(FILE *fp, uint16_t *uw);
static int st_readdw(FILE *fp, uint32_t *udw);
int wavwritehdr(st_signalinfo_t *sinfo, int second_header) ;
int checkwavfile(FILE *fp, st_signalinfo_t *sinfo);

/* Utilities to byte-swap values, use libc optimized macro's if possible  */
#ifdef HAVE_BYTESWAP_H
#define st_swapw(x) bswap_16(x)
#define st_swapdw(x) bswap_32(x)
#define st_swapf(x) (float)bswap_32((uint32_t)(x))
#else
uint16_t st_swapw(uint16_t uw);
uint32_t st_swapdw(uint32_t udw);
float st_swapf(float f);
#endif
double st_swapd(double d);

int st_is_bigendian(void);
int st_is_littleendian(void);

#ifdef WORDS_BIGENDIAN
#define ST_IS_BIGENDIAN 1
#define ST_IS_LITTLEENDIAN 0
#else
#define ST_IS_BIGENDIAN st_is_bigendian()
#define ST_IS_LITTLEENDIAN st_is_littleendian()
#endif

/* Warning, this is a MAX value used in the library.  Each format and
 * effect may have its own limitations of rate.
 */
#define ST_MAXRATE      50L * 1024 /* maximum sample rate in library */

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2  1.57079632679489661923  /* pi/2 */
#endif

#define ST_INT8_MAX (127)
#define ST_INT16_MAX (32676)
#define ST_INT32_MAX (2147483647)
#define ST_INT64_MAX (9223372036854775807)

/* The following is used at times in libst when alloc()ing buffers
 * to perform file I/O.  It can be useful to pass in similar sized
 * data to get max performance.
 */
#define ST_BUFSIZ 8192


#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif

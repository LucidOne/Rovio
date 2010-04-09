/* ARM is (usually) little-endian but with a big-endian FPU.  */

#ifndef _ENDIAN_H
# error "Never use <bits/endian.h> directly; include <endian.h> instead."
#endif

#ifdef __ARMEB__
#define __BYTE_ORDER __BIG_ENDIAN
#include "linux/byteorder/big_endian.h"
#else
#define __BYTE_ORDER __LITTLE_ENDIAN
#include "linux/byteorder/little_endian.h"
#endif
#define __FLOAT_WORD_ORDER __BIG_ENDIAN

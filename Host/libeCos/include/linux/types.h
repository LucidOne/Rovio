#ifndef __LINUX_TYPES_H__
#define __LINUX_TYPES_H__

#include "cyg/infra/cyg_type.h"
#define HZ	100

#ifndef min_t
#define min_t(type,x,y) ((type)(x) < (type)(y) ? (type)(x): (type)(y))
#endif
#ifndef max_t
#define max_t(type,x,y) ((type)(x) > (type)(y) ? (type)(x): (type)(y))
#endif

#define BITS_PER_LONG 32

#endif /* __LINUX_TYPES_H__ */



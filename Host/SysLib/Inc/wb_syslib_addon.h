#ifndef __SYSLIB_ADDON_H__
#define __SYSLIB_ADDON_H__

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "assert.h"
#include "cyg/hal/hal_arch.h"
#include "cyg/kernel/kapi.h"
#include "cyg/infra/diag.h"
#include "cyg/infra/testcase.h"


#include "wbtypes.h"
#include "wbio.h"
#include "wblib.h"
#include "w99702_reg.h"


#define NON_CACHE(addr)		(0x10000000 | (int) (addr))

__inline int no_diag_printf( const char *fmt, ... )
{
    return 0;
}

//#define printf(...)
//#define fprintf(...)
#define printf diag_printf
//#define sysPrintf(...)
//#define diag_printf no_diag_printf
#endif



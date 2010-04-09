/* heaps.cxx */

/* This is a generated file - do not edit! */

#include "pkgconf/heaps.hxx"

#ifdef RW99702
#include "pkgconf/mlt_arm_w99702_ram.h"
#else
#include "pkgconf/mlt_arm_w90n740_ram.h"
#endif
#include "cyg/infra/cyg_type.h"
#include "cyg/hal/hal_intr.h"
#include "cyg/memalloc/dlmalloc.hxx"


#ifdef HAL_MEM_REAL_REGION_TOP

Cyg_Mempool_dlmalloc cygmem_pool_heap1 ( (cyg_uint8 *)CYGMEM_SECTION_heap1 ,
    HAL_MEM_REAL_REGION_TOP( (cyg_uint8 *)CYGMEM_SECTION_heap1 + CYGMEM_SECTION_heap1_SIZE ) - (cyg_uint8 *)CYGMEM_SECTION_heap1 ) 
        CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_MEMALLOC);

#else

Cyg_Mempool_dlmalloc cygmem_pool_heap1 ( (cyg_uint8 *)CYGMEM_SECTION_heap1 , CYGMEM_SECTION_heap1_SIZE ) CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_MEMALLOC);

#endif

Cyg_Mempool_dlmalloc *cygmem_memalloc_heaps[ 2 ] = { 
    &cygmem_pool_heap1,
    NULL
};

/* EOF heaps.cxx */

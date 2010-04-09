// eCos memory layout - Tue Feb 29 14:11:30 2000

// This is a generated file - do not edit

#include "cyg/infra/cyg_type.h"
#include "stddef.h"

static unsigned int dramsize[] = 
{
	0x0,
	0x200000,
	0x400000,
	0x800000,
	0x1000000,
	0x2000000,
	0x4000000,
	0x8000000
};

#define CYGMEM_REGION_ram (0)
//#define CYGMEM_REGION_ram_SIZE (0x00200000)
#define CYGMEM_REGION_ram_SIZE dramsize[(*(unsigned int volatile *)(0x3FFF0008))>>18]
#define CYGMEM_REGION_ram_ATTR (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)
#ifndef __ASSEMBLER__
extern unsigned long CYG_LABEL_NAME (__heap1) [];
#endif
#define CYGMEM_SECTION_heap1 (((*(CYG_LABEL_NAME (__heap1)))+0x07)&0xFFFFFFF8)
#define CYGMEM_SECTION_heap1_SIZE (CYGMEM_REGION_ram_SIZE - CYGMEM_SECTION_heap1)

// eCos memory layout - Wed Apr 11 13:49:55 2001

// This is a generated file - do not edit

#ifndef __ASSEMBLER__
#include "cyg/infra/cyg_type.h"
#include "stddef.h"

#endif
#define CYGMEM_REGION_ram (0)
#define CYGMEM_REGION_ram_SIZE (0x700000)
#define CYGMEM_REGION_ram_ATTR (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)
#define CYGMEM_REGION_rom (0x7F000000)
#define CYGMEM_REGION_rom_SIZE (0x200000)
#define CYGMEM_REGION_rom_ATTR (CYGMEM_REGION_ATTR_R)
#ifndef __ASSEMBLER__
extern unsigned long CYG_LABEL_NAME (__heap1) [];
#endif
#define CYGMEM_SECTION_heap1 (((*(CYG_LABEL_NAME (__heap1)))+0x07)&0xFFFFFFF8)
#define CYGMEM_SECTION_heap1_SIZE (0x00700000 - CYGMEM_SECTION_heap1)

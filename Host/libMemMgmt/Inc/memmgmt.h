#ifndef __MEMMGMT_H__
#define __MEMMGMT_H__

#include "wb_syslib_addon.h"
#include "../../libList/Inc/list.h"

#define MEM_ALIGNMENT	32
#define MEM_ALIGN_SIZE(size) (((size) + MEM_ALIGNMENT - 1) / MEM_ALIGNMENT * MEM_ALIGNMENT)
#define MEMORYPOOL_SIZE(struct_size,count) \
( \
	MEM_ALIGN_SIZE (sizeof (LIST_T)) \
	+ MEM_ALIGN_SIZE (struct_size) * (count) \
)
#define mgmtInit(pBuffer,szBuffer,TYPE_T) \
	__mgmtInit (pBuffer, szBuffer, sizeof (TYPE_T), (size_t) &((TYPE_T *) 0)->list)

LIST_T* __mgmtInit (void *pBuffer, size_t szBuffer, size_t szEach,
	size_t szListOffset);
LIST_T *getMem(LIST_T *pListHead);
void retMem(LIST_T *pListHead, LIST_T *pListThis);

#endif
#ifndef __MEMORY_POOL_H__
#define __MEMORY_POOL_H__

#define MEM_ALIGNMENT	32

//#define MEM_USE_NON_CACHE
#undef MEM_USE_NON_CACHE



#ifdef MEM_USE_NON_CACHE
#	define MEMPOOL_BUFFER_ADDR(addr)	NON_CACHE(addr)
#else
#	define MEMPOOL_BUFFER_ADDR(addr)	(addr)
#endif

#define MEM_ALIGN_SIZE(size) (((size) + MEM_ALIGNMENT - 1) / MEM_ALIGNMENT * MEM_ALIGNMENT)


/*
Here's a sample:

typedef struct
{
	...
	LIST_T	list;
	UINT32	ref_count;
	...
} SOME_STRUCT_T;
#define MEMORY_BLOCK_NUM	64

char g_acMemoryBuffer[MEMORYPOOL_SIZE (sizeof (SOME_STRUCT_T), MEMORY_BLOCK_NUM)];

LIST_T *pMemListHeader = memInit (
	g_acMemoryBuffer,
	sizeof (g_acMemoryBuffer),
	sizeof (SOME_STRUCT_T),
	(size_t) &((SOME_STRUCT_T *) 0)->list);

*/


/* Initialize a memory pool by connecting the memory blocks with list structure. */
__inline LIST_T *__memInit (void *pBuffer, size_t szBuffer, size_t szEach,
	size_t szListOffset)
{
	if (szBuffer < sizeof (LIST_T))
		return NULL;
	else
	{
		size_t i;
		LIST_T *pListHead = (LIST_T *) pBuffer;
		listInit (pListHead);
	
		szEach = MEM_ALIGN_SIZE (szEach);
		for (i = MEM_ALIGN_SIZE(sizeof (LIST_T));
			i + szEach <= szBuffer; i += szEach)
		{
			LIST_T *pListThis = (LIST_T *)((char *) pBuffer + i + szListOffset);
			listInit (pListThis);
			listAttach (pListHead, pListThis);
		}
		return pListHead;
	}
}

#define memInit(pBuffer,szBuffer,TYPE_T) \
	__memInit (pBuffer, szBuffer, sizeof (TYPE_T), (size_t) &((TYPE_T *) 0)->list)


/* Allocate a memory blocks from a list structure. */
__inline LIST_T *memNew (void *pBuffer)
{
	LIST_T *pReturn;
	LIST_T *pListHead = (LIST_T *) pBuffer;

	if (pListHead->pNext == pListHead)
	{
		pReturn = NULL;	//No enough buffer.
	}
	else
	{
		pReturn = pListHead->pNext;
		listDetach (pReturn);
	}
	return pReturn;
}


/* Free a memory blocks (to the list structure). */
__inline void memDel (void *pBuffer, LIST_T *pListThis)
{
	LIST_T *pListHead = (LIST_T *) pBuffer;
	listAttach (pListHead, pListThis);
}


/* Get the memory block. */
__inline LIST_T *memGet (void *pBuffer, int nIndex)
{
	LIST_T *pListHead = (LIST_T *) pBuffer;
	return listGetAt (pListHead, nIndex);
}

/* Get number of memory blocks. */
__inline int memNum (void *pBuffer)
{
	LIST_T *pListHead = (LIST_T *) pBuffer;
	return listLength (pListHead);
}



#define MEMORYPOOL_SIZE(struct_size,count) \
( \
	MEM_ALIGN_SIZE (sizeof (LIST_T)) \
	+ MEM_ALIGN_SIZE (struct_size) * (count) \
)



#define DECLARE_MEM_POOL(name,TYPE_T) \
/* The global memory pool buffer. */ \
extern char g_acMemPool_##name[]; \
extern int g_iMemPoolCount_##name; \
extern int g_iMemPoolSize_##name; \
/*\
LIST_T *name##InitMemoryPool (void); \
TYPE_T *name##New (void); \
void name##AddRef (TYPE_T *pMemory); \
void name##DecRef (TYPE_T *pMemory); \
int name##GetFreeBlocksNum (void); \
*/\
/* Initialize a memory pool with the specified TYPE_T.*/ \
__inline LIST_T *name##InitMemoryPool (void) \
{ \
	LIST_T *pReturn; \
	sysDisableIRQ (); \
	pReturn = memInit ((void *) MEMPOOL_BUFFER_ADDR (g_acMemPool_##name), g_iMemPoolSize_##name, TYPE_T); \
	sysEnableIRQ (); \
	return pReturn; \
} \
\
/* Increase the reference count. */ \
__inline void name##AddRef (TYPE_T *pMemory) \
{ \
	sysDisableIRQ (); \
	if (pMemory != NULL) \
		pMemory->ref_count++; \
	sysEnableIRQ (); \
} \
\
/* Decrease the reference count. */ \
__inline void name##DecRef (TYPE_T *pMemory) \
{ \
	sysDisableIRQ (); \
	if (pMemory != NULL) \
	{ \
		if (pMemory->ref_count <= 0) \
		{ \
			extern int  diag_printf( const char *fmt, ... ); \
			diag_printf("Memory blocks has already delete(%x)!\n", pMemory); \
			while (1); \
		} \
		pMemory->ref_count--; \
		if (pMemory->ref_count == 0) \
			memDel ((void *) MEMPOOL_BUFFER_ADDR (g_acMemPool_##name), &pMemory->list); \
	} \
	sysEnableIRQ (); \
} \
\
/* Allocate a memory block from memory pool. */ \
__inline TYPE_T *name##New () \
{ \
	LIST_T *pList; \
	TYPE_T *pReturn; \
	sysDisableIRQ (); \
	pList = memNew ((void *) MEMPOOL_BUFFER_ADDR (g_acMemPool_##name)); \
	if (pList == NULL) \
	{ \
		/* dbgSetError (APP_ERR_NO_ENOUGH_MEMORY, "Please increase the memory pool size (%08x)!\n", (UINT32) g_acMemPool_##name); */\
		pReturn = NULL; \
	} \
	else \
	{ \
		pReturn = GetParentAddr (pList, TYPE_T, list); \
		pReturn->ref_count = 1; \
	} \
	sysEnableIRQ (); \
	return pReturn;	\
} \
\
/* Shift the memory blocks in the memory pool. */ \
__inline TYPE_T *name##Shift () \
{ \
	LIST_T *pList; \
	TYPE_T *pReturn; \
	sysDisableIRQ (); \
	pList = memNew ((void *) MEMPOOL_BUFFER_ADDR (g_acMemPool_##name)); \
	if (pList == NULL) \
	{ \
		pReturn = NULL; \
	} \
	else \
	{ \
		memDel ((void *) MEMPOOL_BUFFER_ADDR (g_acMemPool_##name), pList); \
		pReturn = GetParentAddr (pList, TYPE_T, list); \
	} \
	sysEnableIRQ (); \
	return pReturn; \
} \
\
/* Get the memory blocks. */ \
__inline TYPE_T *name##GetMemory (int nIndex) \
{ \
	LIST_T *pList; \
	TYPE_T *pReturn; \
	sysDisableIRQ (); \
	pList = memGet ((void *) MEMPOOL_BUFFER_ADDR (g_acMemPool_##name), nIndex); \
	if (pList == NULL) \
	{ \
		pReturn = NULL; \
	} \
	else \
	{ \
		pReturn = GetParentAddr (pList, TYPE_T, list); \
	} \
	sysEnableIRQ (); \
	return pReturn; \
} \
\
/* Count of free memory blocks. */ \
__inline int name##GetFreeBlocksNum () \
{\
	int nNum; \
	sysDisableIRQ (); \
	nNum = memNum ((void *) MEMPOOL_BUFFER_ADDR (g_acMemPool_##name)); \
	sysEnableIRQ (); \
	return nNum; \
}\
/* Count of total memory blocks. */ \
__inline int name##GetTotalBlocksNum () \
{ \
	return g_iMemPoolCount_##name; \
}\





#define IMPLEMENT_MEM_POOL(name,TYPE_T,count) \
/* The global memory pool buffer. */ \
__align(MEM_ALIGNMENT) char g_acMemPool_##name[MEMORYPOOL_SIZE (sizeof (TYPE_T), (count))]; \
int g_iMemPoolCount_##name = (count); \
int g_iMemPoolSize_##name = MEMORYPOOL_SIZE (sizeof (TYPE_T), (count));





#endif


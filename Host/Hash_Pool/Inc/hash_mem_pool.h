#ifndef __HASH_MEM_POOL_H__
#define __HASH_MEM_POOL_H__

//#define HASH_MEM_POOL_DEBUG



#define HASH_MEM_ALIGNMENT	4

#define HASH_MEM_ALIGN_SIZE(size) (((size) + HASH_MEM_ALIGNMENT - 1) / HASH_MEM_ALIGNMENT * HASH_MEM_ALIGNMENT)



struct HASH_MEM_POOL;

typedef struct HASH_MEM_CHILD_POOL_NODE
{
    char    *acBuffer;
    struct HASH_MEM_POOL *pMemPool;
    LIST_T  list;
}HASH_MEM_CHILD_POOL_NODE_T;

typedef struct HASH_MEM_POOL
{
    int     iBufferSize;
    int     iChildPoolNodeCount;
    HASH_MEM_CHILD_POOL_NODE_T    *pChildPool;
    
    LIST_T  freelist;
    LIST_T  busylist;
}HASH_MEM_POOL_T;

typedef struct HASH_MEM_POOL_HEADER
{
    int     iMemPoolCount;
    int     iMemPoolValidCount;
    HASH_MEM_POOL_T *pMemPool;
    char	*pChildMemPool;
}HASH_MEM_POOL_HEADER_T;

//#define malloc Hash_Mem_Pool_New
//#define free Hash_Mem_Pool_Free


void Hash_Mem_Pool_Destroy(HASH_MEM_POOL_HEADER_T *pHeader);
HASH_MEM_POOL_HEADER_T* Hash_Mem_Pool_Create(int iChildPoolCount);
bool Hash_Mem_Child_Pool_Add(HASH_MEM_POOL_HEADER_T *pHeader, int iBufferSize, int iChildPoolNodeCount);
void *Hash_Mem_Pool_New(HASH_MEM_POOL_HEADER_T *pHeader, int size);
void Hash_Mem_Pool_Free(void *pBuf);
HASH_MEM_POOL_T *Hash_Mem_Pool_FindEmpty(HASH_MEM_POOL_HEADER_T *pHeader);
void Hash_Mem_Pool_Taxis(HASH_MEM_POOL_HEADER_T *pHeader);
int Hash_Mem_Pool_Dump_FreeNum(HASH_MEM_POOL_HEADER_T *pHeader, int size);
void Hash_Mem_Pool_Dump_PoolSize(HASH_MEM_POOL_HEADER_T *pHeader);
bool Hash_Mem_Pool_Implement(HASH_MEM_POOL_HEADER_T *pHeader);

#endif


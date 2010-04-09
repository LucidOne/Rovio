#include "stdio.h"
#include "stdlib.h"
#include "wb_syslib_addon.h"
#include "list.h"
#include "hash_mem_pool.h"

HASH_MEM_POOL_HEADER_T *g_HASH_MEM_POOL;

bool createMemPool(void)
{
    g_HASH_MEM_POOL = Hash_Mem_Pool_Create(7);
    if(g_HASH_MEM_POOL == NULL)
    {
        diag_printf("Create mem pool failed %d\n", __LINE__);
        return false;
    }
    
    if(Hash_Mem_Child_Pool_Add(g_HASH_MEM_POOL, 20*1024, 5) != 5)
    {
        diag_printf("Create mem pool failed %d\n", __LINE__);
        Hash_Mem_Pool_Destroy(g_HASH_MEM_POOL);
        return false;
    }
    
    if(Hash_Mem_Child_Pool_Add(g_HASH_MEM_POOL, 10*1024, 10) != 10)
    {
        diag_printf("Create mem pool failed %d\n", __LINE__);
        Hash_Mem_Pool_Destroy(g_HASH_MEM_POOL);
        return false;
    }
    
    if(Hash_Mem_Child_Pool_Add(g_HASH_MEM_POOL, 5*1024, 10) != 10)
    {
        diag_printf("Create mem pool failed %d\n", __LINE__);
        Hash_Mem_Pool_Destroy(g_HASH_MEM_POOL);
        return false;
    }
    
    if(Hash_Mem_Child_Pool_Add(g_HASH_MEM_POOL, 1024, 10) != 10)
    {
        diag_printf("Create mem pool failed %d\n", __LINE__);
        Hash_Mem_Pool_Destroy(g_HASH_MEM_POOL);
        return false;
    }
    
    if(Hash_Mem_Child_Pool_Add(g_HASH_MEM_POOL, 500, 20) != 20)
    {
        diag_printf("Create mem pool failed %d\n", __LINE__);
        Hash_Mem_Pool_Destroy(g_HASH_MEM_POOL);
        return false;
    }
    
    if(Hash_Mem_Child_Pool_Add(g_HASH_MEM_POOL, 100, 50) != 50)
    {
        diag_printf("Create mem pool failed %d\n", __LINE__);
        Hash_Mem_Pool_Destroy(g_HASH_MEM_POOL);
        return false;
    }
	
    if(Hash_Mem_Child_Pool_Add(g_HASH_MEM_POOL, 50, 100) != 100)
    {
        diag_printf("Create mem pool failed %d\n", __LINE__);
        Hash_Mem_Pool_Destroy(g_HASH_MEM_POOL);
        return false;
    }
	
	if(Hash_Mem_Pool_Implement(g_HASH_MEM_POOL) != true)
	{
        diag_printf("Implement mem pool failed %d\n", __LINE__);
        Hash_Mem_Pool_Destroy(g_HASH_MEM_POOL);
        return false;
	}
    return true;
}

bool destroyMemPool(HASH_MEM_POOL_HEADER_T *pHeader)
{
    if(pHeader == NULL) return false;
    
    Hash_Mem_Pool_Destroy(pHeader);
    return true;
}

void Test_New_Free(int size)
{
    char *ptr[3];
    int i;
    
    memset(ptr, 0, sizeof(ptr));
    for(i = 0; i < sizeof(ptr)/sizeof(char*); i++)
    {
	    ptr[i] = Hash_Mem_Pool_New(g_HASH_MEM_POOL, size);
	    if(ptr[i] == NULL)
	    {
	        diag_printf("%d new %d failed!\n", i, size);
	    }
	}
	
	for(i = sizeof(ptr)/sizeof(char*)-1; i>=0; i--)
	{
        if(ptr[i] != NULL) Hash_Mem_Pool_Free(ptr[i]);
    }
    diag_printf("Free buffer num is %d for size %d\n", 
    			Hash_Mem_Pool_Dump_FreeNum(g_HASH_MEM_POOL, size), size);
}

void Test_New_Limit(void)
{
	char *ptr[6];
	int i;
	
	memset(ptr, 0, sizeof(ptr));
	for(i = 0; i < sizeof(ptr)/sizeof(char*)-1; i++)
	{
		ptr[i] = Hash_Mem_Pool_New(g_HASH_MEM_POOL, 11*1024); // malloc from 20k's mem pool
		if(ptr[i] == NULL)
		{
			diag_printf("new 11*1024 for %d times failed\n", i);
			for(i = 0; i < sizeof(ptr)/sizeof(char*)-1; i++)
			{
				Hash_Mem_Pool_Free(ptr[i]);
			}
		}
	}
	
	ptr[i] = Hash_Mem_Pool_New(g_HASH_MEM_POOL, 11*1024); // malloc from 20k's mem pool
	if(ptr[i] != NULL)
	{
		diag_printf("new success, why?\n");
	}
	else
	{
		diag_printf("new failed, this should be...\n");
	}
	
	for(i = 0; i < sizeof(ptr)/sizeof(char*); i++)
	{
		Hash_Mem_Pool_Free(ptr[i]);
	}
    diag_printf("Free buffer num is %d for size %d\n", 
    			Hash_Mem_Pool_Dump_FreeNum(g_HASH_MEM_POOL, 11*1024), 11*1024);
}

void Test_Free_For_Times(int size)
{
    char *ptr;
    
    ptr = Hash_Mem_Pool_New(g_HASH_MEM_POOL, size);
    if(ptr == NULL)
    {
        diag_printf("new %d failed!\n", size);
    }
    else
    {
        diag_printf("new %d success, to free 2 times...\n", size);
        Hash_Mem_Pool_Free(ptr);
        Hash_Mem_Pool_Free(ptr);
    }
    diag_printf("Free buffer num is %d for size %d\n", 
    			Hash_Mem_Pool_Dump_FreeNum(g_HASH_MEM_POOL, size), size);
}

int main(void)
{
    if(createMemPool() == false)
    {
        diag_printf("create mem pool error\n");
        return 1;
    }
    
    Test_New_Free(10);
    Test_New_Free(100);
    Test_New_Free(101);
    Test_New_Free(500);
    Test_New_Free(501);
    Test_New_Free(1024);
    Test_New_Free(1025);
    Test_New_Free(5*1024);
    Test_New_Free(5*1024+1);
    Test_New_Free(10*1024);
    Test_New_Free(10*1024+1);
    Test_New_Free(20*1024);
    Test_New_Free(20*1024+1);
    Test_New_Free(30*1024);
    
    Test_New_Limit();
    
    Test_Free_For_Times(10);
    
    Test_New_Free(10);
    Test_New_Free(100);
    Test_New_Free(101);
    Test_New_Free(500);
    Test_New_Free(501);
    Test_New_Free(1024);
    Test_New_Free(1025);
    Test_New_Free(5*1024);
    Test_New_Free(5*1024+1);
    Test_New_Free(10*1024);
    Test_New_Free(10*1024+1);
    Test_New_Free(20*1024);
    Test_New_Free(20*1024+1);
    Test_New_Free(30*1024);
    
    Test_New_Limit();
    
    Test_Free_For_Times(10);
    
    destroyMemPool(g_HASH_MEM_POOL);
    
    return 0;
}

#include "memmgmt.h"

static cyg_mutex_t mgmt_mutex;
static int mgmt_IsInit = 0;

LIST_T* __mgmtInit (void *pBuffer, size_t szBuffer, size_t szEach,
	size_t szListOffset)
{
	if(mgmt_IsInit == 0)
    {
        cyg_mutex_init(&mgmt_mutex);
        mgmt_IsInit = 1;
    }    
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

LIST_T *getMem(LIST_T *pListHead)
{
    LIST_T *pReturn;
    
    cyg_mutex_lock(&mgmt_mutex);
	if (pListHead->pNext == pListHead)
	{
		pReturn = NULL;	//No enough buffer.
	}
	else
	{
		pReturn = pListHead->pNext;
		listDetach (pReturn);
	}
    cyg_mutex_unlock(&mgmt_mutex);
	return pReturn;
}

void retMem(LIST_T *pListHead, LIST_T *pListThis)
{
	cyg_mutex_lock(&mgmt_mutex);
    listAttach (pListHead, pListThis);
    cyg_mutex_unlock(&mgmt_mutex);
}






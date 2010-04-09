#ifndef __LIST_H__
#define __LIST_H__



struct tagList;
typedef struct tagList
{
	struct tagList *pPrev;
	struct tagList *pNext;
#ifdef DEBUG_DUMP_LIST
	int index;
#endif
} LIST_T;


#define GetOffsetSize(tParent,eMyName) \
	(int)&((tParent *)0)->eMyName)

#define GetParentAddr(pMe,tParent,eMyName) \
	((tParent *)((char *)(pMe) - (int)&((tParent *)0)->eMyName))


__inline void listInit (LIST_T *pList)
{
	pList->pPrev = pList;
	pList->pNext = pList;
#ifdef DEBUG_DUMP_LIST
	{
		static int index = 0;
		pList->index = index++;
	}
#endif
}

__inline LIST_T *listGetNext(LIST_T *pList)
{
	if (pList->pNext == pList)
		return NULL;
	else
		return pList->pNext;
}

/* Connect two lists. */
__inline void listConnect (LIST_T *pList1, LIST_T *pList2)
{
	LIST_T *pPrev1 = pList1->pPrev;
	LIST_T *pPrev2 = pList2->pPrev;
	pPrev1->pNext = pList2;
	pPrev2->pNext = pList1;
	pList1->pPrev = pPrev2;
	pList2->pPrev = pPrev1;
}


/* Disconnect tow lists. */
__inline void listDisconnect (LIST_T *pList1, LIST_T *pList2)
{
	LIST_T *pPrev1 = pList1->pPrev;
	LIST_T *pPrev2 = pList2->pPrev;
	pPrev1->pNext = pList2;
	pPrev2->pNext = pList1;
	pList1->pPrev = pPrev2;
	pList2->pPrev = pPrev1;
}


__inline void listAttach (LIST_T *pNode1, LIST_T *pNode2)
{
	listConnect (pNode1, pNode2);
}

__inline void listDetach (LIST_T *pNode)
{
	listDisconnect (pNode, pNode->pNext);
}


__inline int listLength (LIST_T *pList)
{
	LIST_T *pNode;
	int i = 0;
	for (pNode = pList->pNext; pNode != pList; pNode = pNode->pNext)
		i++;
	return i;
}


__inline LIST_T *listGetAt (LIST_T *pList, int nIndex)
{
	LIST_T *pNode;

	if (nIndex < 0)
		return NULL;
	
	for (pNode = pList->pNext; pNode != pList; pNode = pNode->pNext)
	{
		if (nIndex-- == 0)
			return pNode;
	}

	return NULL;
}


__inline BOOL listIsEmpty (LIST_T *pList)
{
	if (pList->pNext == pList)
		return TRUE;
	else
		return FALSE;
}

#ifdef DEBUG_DUMP_LIST
static __inline void listDump (LIST_T *pList)
{
	LIST_T *p;
	sysSafePrintf ("List: %d [ ", pList->index);
	for (p = pList->pNext; p != pList; p = p->pNext)
		sysSafePrintf ("%d ", p->index);
	sysSafePrintf ("]\n");
}
#endif

#endif

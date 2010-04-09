#ifndef LIST_H
#define LIST_H

#define PTE do {diag_printf("%s,%d.\n", __FILE__, __LINE__);} while(0)
#define PTI do {diag_printf("%s,%d.\n", __FILE__, __LINE__);} while(0)
#define PAZ do {fprintf(stderr, "%s,%d.\n", __FILE__, __LINE__); getc(stdin);} while(0)
#define PRINT_MEM_OUT do {fprintf(stderr, "Not enough memory in %s %d.\n", __FILE__, __LINE__);} while(0)
//#define PRINT_MEM_OUT

#ifndef __LIST_DEFINED__
#define __LIST_DEFINED__
struct tagLIST;
typedef struct tagLISTNODE
{
	void *pValue;
	struct tagLISTNODE *pPreNode;
	struct tagLISTNODE *pNextNode;
	struct tagLIST *pList;
} LISTNODE;

typedef struct tagLIST
{
	LISTNODE *pFirstNode;
	LISTNODE *pLastNode;
} LIST;
#endif

LIST *httpCreateList( void );
void httpDeleteList(LIST *ppList);
void httpDeleteNode(LISTNODE *pNode);
LISTNODE *httpInsertNodeAfter(LISTNODE *pNode);
LISTNODE *httpInsertNodeBefore(LISTNODE *pNode);
LISTNODE *httpAppendNode(LIST *pList);
LISTNODE *httpGetNodeAt(const LIST *pList, int iIndex);
int httpGetNodeIndex(const LISTNODE *pNode);
int httpGetListLength(const LIST *pList);

#endif

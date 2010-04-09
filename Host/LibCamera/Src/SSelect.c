#include "CommonDef.h"

#ifdef USE_DDNS


#ifndef WLAN
__align (32)
CHAR g_Select_Buf[SELECT_BUFFER_LEN];
#endif

/*********************************************************************
 Emulate C++ objects.
*********************************************************************/
OBJHEADER *ObjInit(OBJHEADER *pParent, size_t tObjSize, OBJHEADER *pObj,
	void (*pfunFinal)(OBJHEADER *pObj),
	OBJHEADER *(*pfunInit)(OBJHEADER *pObj, va_list vaInit),
	...)
{
	int iNewCreate = 0;
	va_list vaInit;

	if (pObj == NULL && pParent != NULL)
	{
		diag_printf("No parent for new created object.\n");
		return NULL;
	}

	if (pObj == NULL)
	{
		pObj = (OBJHEADER *)malloc(tObjSize);
		iNewCreate = 1;
	}
	if (pObj == NULL) return NULL;

	pObj->pfunFinal = NULL;
	if (pParent == NULL) pObj->iOffsetContainer = 0;
	else pObj->iOffsetContainer = (char *)pObj - (char *)pParent;
	pObj->iNewCreate = iNewCreate;

	memset((char *)pObj + sizeof(OBJHEADER), 0, tObjSize - sizeof(OBJHEADER));

	va_start(vaInit, pfunInit);
	if ((*pfunInit)(pObj, vaInit) == NULL)
	{
		if (iNewCreate) free(pObj);
		return NULL;
	}
	else
	{
		pObj->pfunFinal = pfunFinal;
		return pObj;
	}
}


void ObjFinal(OBJHEADER *pObj)
{
	if (pObj == NULL) return;

	if (pObj->iOffsetContainer != 0)
	{
		//Code here to detect if a parent object exists.
		OBJHEADER *pParent = (OBJHEADER *)((char *)pObj - pObj->iOffsetContainer);
		if (pParent->pfunFinal != NULL)
		{
			ObjFinal(pParent);
			return;
		}
	}
	//Code here to call final function of this object.
	if (pObj->pfunFinal != NULL)
	{
		void (*pfunFinal)(OBJHEADER *pObj) = pObj->pfunFinal;
		pObj->pfunFinal = NULL;
		(*pfunFinal)(pObj);
	}
	if (pObj->iNewCreate)
	{
		free(pObj);
	}
}





/*********************************************************************
** FD_T construction & destrucion
**	do not call them directly for ever!
** var vaInit:
**	int iFD, int iType, int iState,
**  int iConnTimeout_InMSec,
**	BOOL bWantRead, BOOL bWantWrite,
**	BOOL (*pfunProcessFD)(struct tagFD_T *pFD, BOOL bReadable, BOOL bWritable)
*********************************************************************/
OBJHEADER *FD_Init(OBJHEADER *pObj, va_list vaInit)
{
	FD_T *pThis = (FD_T *)pObj;

	pThis->iFD = va_arg(vaInit, int);
	pThis->iType = va_arg(vaInit, int);
	pThis->iState = va_arg(vaInit, int);
	pThis->iConnTimeout_InMSec = va_arg(vaInit, int);
	pThis->bWantRead = va_arg(vaInit, BOOL);
	pThis->bWantWrite = va_arg(vaInit, BOOL);
	pThis->pfunProcessFD = va_arg(vaInit, FUNPROCESSFD_T);

	pThis->bReadable = 0;
	pThis->bWritable = 0;
	if (pThis->pfunProcessFD == NULL) return NULL;
	else return pObj;
}

void FD_Final(OBJHEADER *pObj)
{
//	FD_T *pThis = (FD_T *)pObj;
}


void FD_Next(FD_T *pFD,
					int iState,
					BOOL bWantRead,
					BOOL bWantWrite,
					int iConnTimeout_InMSec)
{
	/*
	diag_printf("FD_Next: %d, st: %d, r: %d, w: %d, t: %d\n",
		(int)pFD->iFD, (int)pFD->iState, (int)pFD->bWantRead, (int)pFD->bWantWrite, 
		(int)iConnTimeout_InMSec);
	 */
		
	pFD->iState = iState;
	pFD->bWantRead = bWantRead;
	pFD->bWantWrite = bWantWrite;
	pFD->iConnTimeout_InMSec = iConnTimeout_InMSec;
}






/* struct for inserting a process to a running thread. */
typedef struct
{
	void *(*pfunCustomProcess)(void *pThread, void *pParam);	//the user's function
	void *pParam;	//parameters that's transferred to *pCustomProcess
} CUSTOM_PROCESS_T;

BOOL InsertCustomProcess(MSG_QUEUE *pMsgThread, void *(*pfunCustomProcess)(void *pThread, void *pParam), void *pParam)
{
	CUSTOM_PROCESS_T *pCustomProcess;
	MSG_T msg;

	if (pMsgThread == NULL || pfunCustomProcess == NULL) return FALSE;

	pCustomProcess = (CUSTOM_PROCESS_T *)malloc(sizeof(CUSTOM_PROCESS_T));
	if (pCustomProcess == NULL) return FALSE;

	pCustomProcess->pfunCustomProcess = pfunCustomProcess;
	pCustomProcess->pParam = pParam;

	msg.lMsg = MSG_CUSTOM_PROC;
	msg.lData = (long)pCustomProcess;

	if (!SendMsg(pMsgThread, &msg))
	{
		free((void *)pCustomProcess);
		return FALSE;
	}
	else return TRUE;
}


BOOL AddSelectFD(FD_T *pFD)
{
	MSG_T msg;
	if (pFD == NULL) return FALSE;
	if (pFD->pfunProcessFD == NULL) return FALSE;

	msg.lMsg = MSG_ADD_FD;
	msg.lData = (long)pFD;
	return SendMsg(g_pMsgSelect, &msg);
}

void SelectThread(cyg_addrword_t pParam)
{
	SELECT_THREAD_T sThreadData;	//important data in this thread.
	MSG_T *pMsg = &sThreadData.msg;	//message in the main loop of this thread
	fd_set *pfdReadSet = &sThreadData.fdReadSet;	//readfds for select
	fd_set *pfdWriteSet = &sThreadData.fdWriteSet;	//writefds for select
	LIST *pFDList;
	

	int iSelectRt;
	struct timeval tvSelect;	//timeval for select
	struct timeval *ptvSelect;
	LISTNODE *pNode;
	LISTNODE *pNextNode;
	FD_T *pFD;
	int iFD;
	int iConnTimeout_InMSec;

	SELECT_MSG *pSelectMsg;
	
    sThreadData.pFDList = httpCreateList();	//List that saves fds
	pFDList = sThreadData.pFDList;
	
	if (pFDList == NULL)
	{
		diag_printf("Init fds list error!\n");
		return;
	}
	pSelectMsg = (SELECT_MSG *)ObjInit(NULL, sizeof(SELECT_MSG), NULL,
			SELECT_MSG_Final, SELECT_MSG_Init,
			g_pMsgSelect, pMsg);
	if (pSelectMsg == NULL)
	{
		httpDeleteList(pFDList);
		return ;
	}
	/* Add msg fd */
	if ((pNode = httpAppendNode(pFDList)) == NULL)
	{
		ObjFinal((OBJHEADER *)pSelectMsg);
		httpDeleteList(pFDList);
		return ;
	}
	pNode->pValue = &pSelectMsg->fd;
	FD_ZERO(pfdReadSet);
	FD_ZERO(pfdWriteSet);
	
	diag_printf("Select thread is running\n");
	while (TRUE)
	{
		if (pFDList->pFirstNode == pFDList->pLastNode)
		{//no fds, wait message
			//PTI;
			if (!GetMsg(g_pMsgSelect, pMsg))
			{
				tt_msleep(1000);
				fprintf(stderr, "Msg queue error!\n");
				continue;
			}
		}
		else
		{//select fds.
//			fprintf(stderr, "l: %d\n", httpGetListLength(pFDList));PTI;
			pMsg->lMsg = MSG_NONE;

			iFD = 0;
			iConnTimeout_InMSec = -1;

			for (pNode = pFDList->pFirstNode;
				pNode != pFDList->pLastNode;
				pNode = pNode->pNextNode)
			{
				pFD = (FD_T *)pNode->pValue;
				if (pFD == NULL) continue;

				//diag_printf("set: %d %d %d %d (%d)\n", (int)pFD->bWantRead, (int)pFD->bWantWrite, (int)pFD->iFD, (int)pFD->iConnTimeout_InMSec, (int)pFD->iState);

				if (pFD->iConnTimeout_InMSec >= 0)
				{
					if (iConnTimeout_InMSec < 0)
						iConnTimeout_InMSec = pFD->iConnTimeout_InMSec;
					else if (iConnTimeout_InMSec > pFD->iConnTimeout_InMSec)
						iConnTimeout_InMSec = pFD->iConnTimeout_InMSec;
				}

				if (pFD->iFD < 0)
				{
					//iConnTimeout_InMSec = 0;
					continue;
				}

				if (pFD->bWantRead || pFD->bWantWrite)
				{
					if (iFD < pFD->iFD) iFD = pFD->iFD;
				}

				if (pFD->bWantRead)
				{
					if (!pFD->bReadable)
					{
						//diag_printf("SR:%d\n", pFD->iFD);					
						FD_SET(pFD->iFD, pfdReadSet);
					}
				}
				else if (pFD->bReadable)
				{
					//diag_printf("CR:%d\n", pFD->iFD);
					FD_CLR(pFD->iFD, pfdReadSet);
				}

				if (pFD->bWantWrite)
				{
					if (!pFD->bWritable)
					{
						//diag_printf("SW:%d\n", pFD->iFD);
						FD_SET(pFD->iFD, pfdWriteSet);
					}
				}
				else if (pFD->bWritable)
				{
					//diag_printf("CW:%d\n", pFD->iFD);
					FD_CLR(pFD->iFD, pfdWriteSet);
				}
			}
			if (iConnTimeout_InMSec < 0)
			{
				ptvSelect = NULL;
			}
			else
			{
				tvSelect.tv_sec = iConnTimeout_InMSec / 1000;;
				//tvSelect.tv_usec = (iConnTimeout_InMSec - 1000 * tvSelect.tv_sec) * 1000;
				tvSelect.tv_usec = 0;	//eCos select for tv_usec will spend so much time!!!
				ptvSelect = &tvSelect;
			}

//diag_printf("ptvSelect = %x, %d\n", (int)ptvSelect, iConnTimeout_InMSec);
			
#ifndef WLAN
			if ((iSelectRt = netselect(iFD + 1, pfdReadSet, pfdWriteSet, NULL, ptvSelect,g_Select_Buf, SELECT_BUFFER_LEN)) < 0)
#else
			if ((iSelectRt = select(iFD + 1, pfdReadSet, pfdWriteSet, NULL, ptvSelect)) < 0)
#endif
			{
				//PTI;
				if (errno == EINTR || errno == EBADF)
				{
					//PTI;
					//continue;
					//Do not use "continue" here!!!
				}
				else
				{
					diag_printf("Select error!\n");
					break;
				}
			}
			//PTI;
//diag_printf("iSelectRt = %d, errno = %d\n", iSelectRt, errno);			

			cyg_thread_yield();

			if (iSelectRt <= 0)
			{
				FD_ZERO(pfdReadSet);
				FD_ZERO(pfdWriteSet);
			}
		
			for (pNode = pFDList->pFirstNode;
				pNode != pFDList->pLastNode;
				pNode = pNextNode)
			{
				//PTI;
				pNextNode = pNode->pNextNode;
				pFD = (FD_T *)pNode->pValue;
				if (pFD == NULL) continue;
//printf("%d \n", pFD->iFD);PTI;
				iFD = pFD->iFD;
				
				if (iFD < 0 || iSelectRt <= 0)
				{
					pFD->bReadable = FALSE;
					pFD->bWritable = FALSE;
				}
				else
				{
					pFD->bReadable = (pFD->bWantRead ? (FD_ISSET(iFD, pfdReadSet) ? TRUE : FALSE) : FALSE);
					pFD->bWritable = (pFD->bWantWrite ? (FD_ISSET(iFD, pfdWriteSet) ? TRUE : FALSE) : FALSE);
				}
				
			}

			for (pNode = pFDList->pFirstNode;
				pNode != pFDList->pLastNode;
				pNode = pNextNode)
			{
				//PTI;
				pNextNode = pNode->pNextNode;
				pFD = (FD_T *)pNode->pValue;
				if (pFD == NULL) continue;
//printf("%d \n", pFD->iFD);PTI;
				iFD = pFD->iFD;
/*
diag_printf("pFD->iFD = %d, [%d,%d]  r = %d, w = %d, ---%d,%d\n", pFD->iFD,
	(int)pFD->bWantRead, (int)pFD->bWantWrite,
	(int)pFD->bReadable, (int)pFD->bWritable,
	(int)(iFD<0?0:FD_ISSET(iFD, pfdReadSet)), (int)(iFD<0?0:FD_ISSET(iFD, pfdWriteSet))
	);
*/
//PTI;1
				if (!(*pFD->pfunProcessFD)(pFD, pFD->bReadable, pFD->bWritable))
				{//return FALSE, the FD_T struct should be cleared.
					//PTI;
					if (pFD->bReadable)
					{
						//diag_printf("CR:%d\n", iFD);
						FD_CLR(iFD, pfdReadSet);
					}
					if (pFD->bWritable)
					{
						//diag_printf("CW:%d\n", iFD);
						FD_CLR(iFD, pfdWriteSet);
					}
					ObjFinal((OBJHEADER *)pFD);
					httpDeleteNode(pNode);
				}
				else if (iFD != pFD->iFD)
				{//update fd if fd is changed by *pFD->pfunProcessFD
					//PTI;
					if (pFD->bReadable)
					{
						if (iFD >= 0)
						{
							//diag_printf("CR:%d\n", iFD);
							FD_CLR(iFD, pfdReadSet);
						}
						if (pFD->iFD >= 0)
						{
							//diag_printf("SR:%d\n", pFD->iFD);
							FD_SET(pFD->iFD, pfdReadSet);
						}
					}
					if (pFD->bWritable)
					{
						if (iFD >= 0)
						{
							//diag_printf("CW:%d\n", iFD);
							FD_CLR(iFD, pfdWriteSet);
						}
						if (pFD->iFD >= 0)
						{
							//diag_printf("SW:%d\n", pFD->iFD);
							FD_SET(pFD->iFD, pfdWriteSet);
						}
					}
				}

//PTI;
			}
			//PTI;
		}

		if (pMsg->lMsg == MSG_NONE) continue;
//PTI;
		if (pMsg->lMsg == MSG_QUIT) break;
		else if (pMsg->lMsg == MSG_ADD_FD)
		{
			pFD = (FD_T *)pMsg->lData;
			diag_printf("Add FD: %d\n", pFD->iFD);
			if (pFD != NULL
				&& pFD->pfunProcessFD != NULL
				&& (pNode = httpAppendNode(pFDList)) != NULL)
			{
				//PTI;
				pFD->bReadable = FALSE;
				pFD->bWritable = FALSE;
				pNode->pValue = pFD;
			}
		}
		else if (pMsg->lMsg == MSG_CUSTOM_PROC)
		{
			CUSTOM_PROCESS_T *pCusProcess;
			pCusProcess = (CUSTOM_PROCESS_T *)pMsg->lData;
			if (pCusProcess != NULL)
			{
				(*pCusProcess->pfunCustomProcess)((void *)&sThreadData, pCusProcess->pParam);
				free(pCusProcess);
			}
		}
	}

	//PTI;
	for (pNode = pFDList->pFirstNode;
		pNode != pFDList->pLastNode;
		pNode = pNode->pNextNode)
	{
		pFD = (FD_T *)pNode->pValue;
		if (pFD == NULL) continue;
		//PTI;
		ObjFinal((OBJHEADER *)pFD);
	}

	httpDeleteList(pFDList);
}
#endif



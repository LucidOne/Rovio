#if 1
#include "CommonDef.h"

#ifndef WLAN
#define MSG_BUFFER_LEN 1024
__align (32)
CHAR g_Msg_Buf[MSG_BUFFER_LEN];
#endif

#ifdef EMU_SEMAPHORE
int EMU_sem_init(EMU_sem_t *sem, unsigned int value)
{
	cyg_mutex_init(&sem->ptm);
	cyg_cond_init(&sem->ptc,&sem->ptm);
	sem->uiValue = value;
	return 0;
}

int EMU_sem_wait(EMU_sem_t *sem)
{
	//if (pthread_mutex_lock(&sem->ptm) != 0) return -1;
	if (cyg_mutex_lock(&sem->ptm) != 0) return -1;


	if (sem->uiValue == 0)
		//pthread_cond_wait(&sem->ptc, &sem->ptm);
		cyg_cond_wait(&sem->ptc);

	sem->uiValue--;

	//pthread_mutex_unlock(&sem->ptm);
	cyg_mutex_unlock(&sem->ptm);
	return 0;
}

int EMU_sem_trywait(EMU_sem_t *sem)
{
	//if (pthread_mutex_lock(&sem->ptm) != 0) return -1;
	if (cyg_mutex_lock(&sem->ptm) != 0) return -1;

	if (sem->uiValue == 0)
	{
		//pthread_mutex_unlock(&sem->ptm);
		cyg_mutex_unlock(&sem->ptm);
		return -1;
	}
	else
	{
		sem->uiValue--;
		//pthread_mutex_unlock(&sem->ptm);
		cyg_mutex_unlock(&sem->ptm);
		return 0;
	}
}

int EMU_sem_post(EMU_sem_t *sem)
{
	//if (pthread_mutex_lock(&sem->ptm) != 0) return -1;
	if (cyg_mutex_lock(&sem->ptm) != 0) return -1;

	sem->uiValue++;
	if (sem->uiValue == 1)
		//pthread_cond_signal(&sem->ptc);
		cyg_cond_signal(&sem->ptc);

	//pthread_mutex_unlock(&sem->ptm);
	cyg_mutex_unlock(&sem->ptm);
	return 0;
}

int EMU_sem_destroy(EMU_sem_t *sem)
{
	//pthread_mutex_destroy(&sem->ptm);
	//pthread_cond_destroy(&sem->ptc);
	cyg_mutex_destroy(&sem->ptm);
	cyg_cond_destroy(&sem->ptc);
	return 0;
}
#else

#define EMU_sem_init(a,b) cyg_semaphore_init(a, b)
#define EMU_sem_wait cyg_semaphore_wait
#define EMU_sem_trywait cyg_semaphore_trywait
#define EMU_sem_post cyg_semaphore_post
#define EMU_sem_destroy cyg_semaphore_destroy

#endif


LIST *CreateMsgList()
{
	return httpCreateList();
}

void DeleteMsgList(LIST *plMsgList)
{
	LISTNODE *pNode;
	if (plMsgList == NULL) return;
	for (pNode = plMsgList->pFirstNode; pNode != plMsgList->pLastNode; pNode=pNode->pNextNode)
	{
		if (pNode->pValue != NULL)
			free(pNode->pValue);
	}
	httpDeleteList(plMsgList);
}

MSG_QUEUE *CreateMsgQueue(int iMaxMsgLimit, BOOL bFDEvent)
{
	MSG_QUEUE *pMsgQueue;

	pMsgQueue = (MSG_QUEUE *)malloc(sizeof(MSG_QUEUE));
	if (pMsgQueue == NULL)
	{
		PRINT_MEM_OUT;
		return NULL;
	}

	pMsgQueue->bFDEvent = bFDEvent;
	if (bFDEvent)
	{
#ifndef WLAN
		int iFL;
		if (socket_pipe(pMsgQueue->aiFD,5566,g_Msg_Buf, MSG_BUFFER_LEN) != 0)
#else
		if (socket_pipe(pMsgQueue->aiFD,5566,NULL, NULL) != 0)
#endif
		{
			free(pMsgQueue);
			return NULL;
		}
#ifndef WLAN
		if ((iFL = netfcntl(pMsgQueue->aiFD[0], 3, 0, g_Msg_Buf, MSG_BUFFER_LEN)) != -1)
			netfcntl(pMsgQueue->aiFD[0], 4, iFL | 0X800, g_Msg_Buf, MSG_BUFFER_LEN);
#else
		//if ((iFL = fcntl(pMsgQueue->aiFD[0], 3, 0)) != -1)
		{
			int value = 1;
			ioctl(pMsgQueue->aiFD[0],FIONBIO,&value);
		}
#endif
	}

	if ((pMsgQueue->plMsgList = CreateMsgList()) == NULL)
	{
		free(pMsgQueue);
		return NULL;
	}
	pMsgQueue->iMaxMsgLimit = iMaxMsgLimit;

	cyg_mutex_init(&pMsgQueue->ptmMsg);

	EMU_sem_init(&pMsgQueue->sMsgSemConsumer, 0);
	if (iMaxMsgLimit > 0)
		EMU_sem_init(&pMsgQueue->sMsgSemProducer, iMaxMsgLimit);
	else
		EMU_sem_init(&pMsgQueue->sMsgSemProducer, 0);

	return pMsgQueue;
}

void FreeMsgQueue(MSG_QUEUE *pMsgQueue)
{
	if (pMsgQueue == NULL) return;

	if (pMsgQueue->bFDEvent)
	{
#ifndef WLAN
		netclose(pMsgQueue->aiFD[0], g_Msg_Buf, MSG_BUFFER_LEN);
		netclose(pMsgQueue->aiFD[1], g_Msg_Buf, MSG_BUFFER_LEN);
#else
		close(pMsgQueue->aiFD[0]);
		close(pMsgQueue->aiFD[1]);
#endif
	}

	DeleteMsgList(pMsgQueue->plMsgList);
	cyg_mutex_destroy(&pMsgQueue->ptmMsg);
	EMU_sem_destroy(&pMsgQueue->sMsgSemConsumer);
	EMU_sem_destroy(&pMsgQueue->sMsgSemProducer);
	free(pMsgQueue);
}

BOOL PopMsgNode(MSG_QUEUE *pMsgQueue, MSG_T *pMsg)
{
	MSG_T *pMsgInNode;

	if (pMsgQueue == NULL || pMsg == NULL) return FALSE;

	cyg_mutex_lock(&pMsgQueue->ptmMsg); 
		

	if (pMsgQueue->plMsgList->pFirstNode == pMsgQueue->plMsgList->pLastNode)
	{
		cyg_mutex_unlock(&pMsgQueue->ptmMsg);
		return FALSE;
	}

	pMsgInNode = (MSG_T *)pMsgQueue->plMsgList->pFirstNode->pValue;
	httpDeleteNode(pMsgQueue->plMsgList->pFirstNode);
	if (pMsgQueue->bFDEvent)
	{
		char c;
#ifndef WLAN
		if (netread(pMsgQueue->aiFD[0], &c, 1,g_Msg_Buf, MSG_BUFFER_LEN) != 1)
#else
		if (read(pMsgQueue->aiFD[0], &c, 1) != 1)
#endif
			PTE;
	}
	cyg_mutex_unlock(&pMsgQueue->ptmMsg);

	*pMsg = *pMsgInNode;
	free(pMsgInNode);
	return TRUE;
}

BOOL PushMsgNode(MSG_QUEUE *pMsgQueue, MSG_T *pMsg)
{
	LISTNODE *pNode;
	MSG_T *pMsgInNode;

	if (pMsgQueue == NULL || pMsg == NULL) return FALSE;

	pMsgInNode = (MSG_T *)malloc(sizeof(MSG_T));
	if (pMsgInNode == NULL)
	{
		PRINT_MEM_OUT;
		return FALSE;
	}
	*pMsgInNode = *pMsg;
	
	cyg_mutex_lock(&pMsgQueue->ptmMsg);
	
	pNode = httpAppendNode(pMsgQueue->plMsgList);
	if (pNode == NULL)
	{
		free(pMsgInNode);
		cyg_mutex_unlock(&pMsgQueue->ptmMsg);
		return FALSE;
	}

	pNode->pValue = pMsgInNode;

	if (pMsgQueue->bFDEvent)
	{
		char c;
#ifndef WLAN
		netwrite(pMsgQueue->aiFD[1], &c, 1,g_Msg_Buf, MSG_BUFFER_LEN);
#else
		write(pMsgQueue->aiFD[1], &c, 1);
#endif
	}

	cyg_mutex_unlock(&pMsgQueue->ptmMsg);

	return TRUE;
}

BOOL SendMsg(MSG_QUEUE *pMsgQueue, MSG_T *pMsg)
{
	BOOL bPush;
	if (pMsgQueue == NULL)
		return FALSE;
	if (pMsgQueue->iMaxMsgLimit > 0)
		EMU_sem_wait(&pMsgQueue->sMsgSemProducer);

	bPush = PushMsgNode(pMsgQueue, pMsg);

	EMU_sem_post(&pMsgQueue->sMsgSemConsumer);
	return bPush;
}

BOOL GetMsg(MSG_QUEUE *pMsgQueue, MSG_T *pMsg)
{
	BOOL bPop;
	if (pMsgQueue == NULL)
		return FALSE;
	EMU_sem_wait(&pMsgQueue->sMsgSemConsumer);

	bPop = PopMsgNode(pMsgQueue, pMsg);

	if (pMsgQueue->iMaxMsgLimit > 0)
		EMU_sem_post(&pMsgQueue->sMsgSemProducer);

	return bPop;
}

BOOL TrySendMsg(MSG_QUEUE *pMsgQueue, MSG_T *pMsg)
{
	BOOL bPush;
	if (pMsgQueue == NULL)
		return FALSE;
	if (pMsgQueue->iMaxMsgLimit > 0)
	{
		if (EMU_sem_trywait(&pMsgQueue->sMsgSemProducer) == false)
			return FALSE;
	}

	bPush = PushMsgNode(pMsgQueue, pMsg);

	EMU_sem_post(&pMsgQueue->sMsgSemConsumer);
	return bPush;
}

BOOL TryGetMsg(MSG_QUEUE *pMsgQueue, MSG_T *pMsg)
{
	BOOL bPop;
	if (pMsgQueue == NULL)
		return FALSE;
	if (!(EMU_sem_trywait(&pMsgQueue->sMsgSemConsumer)))
		return FALSE;

	bPop = PopMsgNode(pMsgQueue, pMsg);

	if (pMsgQueue->iMaxMsgLimit > 0)
		EMU_sem_post(&pMsgQueue->sMsgSemProducer);

	return bPop;
}

BOOL IsMsgExist(MSG_QUEUE *pMsgQueue, MSG_T *pMsg)
{
	BOOL bFound = FALSE;
	LIST *plMsgList;
	LISTNODE *pNode;

	/* Check if pMsg is found in pMsgQueue */
	//if (cyg_mutex_lock(&pMsgQueue->ptmMsg) != 0)
	//	return FALSE;
	cyg_mutex_lock(&pMsgQueue->ptmMsg);
	plMsgList = pMsgQueue->plMsgList;
	for (pNode = plMsgList->pFirstNode; pNode != plMsgList->pLastNode; pNode = pNode->pNextNode)
	{
		if (memcmp((MSG_T *)pNode->pValue, pMsg, sizeof(MSG_T)) == 0)
		{
			bFound = TRUE;
			break;
		}
	}
	cyg_mutex_unlock(&pMsgQueue->ptmMsg);

	return bFound;
}


#ifdef USE_DDNS
BOOL SELECT_MSG_Process(FD_T *pFD, BOOL bReadable, BOOL bWritable)
{
	SELECT_MSG *pSelectMsg;
	pSelectMsg = (SELECT_MSG *)((char *)(pFD) - (unsigned long)(&((SELECT_MSG *)0)->fd));
	//PTI;
	if (!bReadable) return TRUE;

	//PTI;
	if (GetMsg(pSelectMsg->pMsgQueue, pSelectMsg->pMsg))
	{
		//PTI;
		return TRUE;
	}
	else
	{
		PTE;
		pSelectMsg->pMsg->lMsg = MSG_NONE;
		return FALSE;
	}
	//PTI;
}

OBJHEADER *SELECT_MSG_Init(OBJHEADER *pObj, va_list vaInit)
{
	SELECT_MSG *pThis = (SELECT_MSG *)pObj;
	MSG_QUEUE *pMsgQueue = va_arg(vaInit, MSG_QUEUE *);
	MSG_T *pMsg = va_arg(vaInit, MSG_T *);

	if (pMsgQueue == NULL
		|| pMsgQueue->aiFD[0] < 0) goto lError;
	pThis->pMsgQueue = pMsgQueue;
	if (pMsg == NULL) goto lError;
	pThis->pMsg = pMsg;

	if (ObjInit((OBJHEADER *)pThis, sizeof(FD_T), (OBJHEADER *)&pThis->fd,
			FD_Final, FD_Init,
			pMsgQueue->aiFD[0], SE_TP_MSG, 0, -1, TRUE, FALSE, SELECT_MSG_Process) == NULL)
		goto lError;
	return pObj;
lError:
	return NULL;
}

void SELECT_MSG_Final(OBJHEADER *pObj)
{
	SELECT_MSG *pThis = (SELECT_MSG *)pObj;
	ObjFinal((OBJHEADER *)&pThis->fd);
}
#endif
#endif
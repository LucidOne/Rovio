#ifndef MSG_H
#define MSG_H

//#define EMU_SEMAPHORE

#ifdef EMU_SEMAPHORE
typedef struct
{
	unsigned int uiValue;
	cyg_mutex_t ptm;
	cyg_cond_t ptc;
	
} EMU_sem_t;
#else
#define EMU_sem_t cyg_sem_t
#endif

#ifndef _MSG_T
#define _MSG_T
typedef struct
{
	long lMsg;
	long lData;
} MSG_T;
#endif
typedef struct
{
	LIST *plMsgList;
	int iMaxMsgLimit;
	EMU_sem_t sMsgSemProducer;	/* semphone for msg producer */
	EMU_sem_t sMsgSemConsumer;	/* semphone for msg consumer */
	cyg_mutex_t ptmMsg;	/* mutex for mail producer & consumer */

	BOOL bFDEvent;	/* if send msg event by file descriptor, for select */
	int aiFD[2];
} MSG_QUEUE;

#ifdef USE_DDNS
#include "SSelect.h"
/* for a fd trigger to function select. */
typedef struct
{
	OBJHEADER oHeader;
	FD_T fd;
	MSG_QUEUE *pMsgQueue;
	MSG_T *pMsg;
} SELECT_MSG;
OBJHEADER *SELECT_MSG_Init(OBJHEADER *pObj, va_list vaInit);
void SELECT_MSG_Final(OBJHEADER *pObj);
#endif

MSG_QUEUE *CreateMsgQueue(int iMaxMsgLimit, BOOL bFDEvent);
void FreeMsgQueue(MSG_QUEUE *pMsgQueue);
BOOL SendMsg(MSG_QUEUE *pMsgQueue, MSG_T *pMsg);
BOOL GetMsg(MSG_QUEUE *pMsgQueue, MSG_T *pMsg);
BOOL TrySendMsg(MSG_QUEUE *pMsgQueue, MSG_T *pMsg);
BOOL TryGetMsg(MSG_QUEUE *pMsgQueue, MSG_T *pMsg);
/* check if pMsg exists in pMsgQueue. */
BOOL IsMsgExist(MSG_QUEUE *pMsgQueue, MSG_T *pMsg);
BOOL SendCameraMsg(long lMsg, long lData);
#endif

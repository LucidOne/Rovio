#ifndef __HIC_CLIENT_H__
#define __HIC_CLIENT_H__

//#define __ARG_PACKET_SEM__

typedef struct
{
	CHAR*	pcArgBuf;
	INT32	iArgBufLen;
	CHAR*	pcArgBufNow;
	
	INT32	iTotalLen;
	INT32	iArgNum;
	
	int		semArgPacketCtrl;
}PACKET_FORM_CONTROL_T;

typedef struct
{
	CHAR*	pcArgBuf;
	CHAR*	pcArgBufNow;
	
	int		semArgPacketCtrl;
}PACKET_PARSE_CONTROL_T;

INT32 ArgPacketFormInit(CHAR *pcBuf, INT32 iBufLen);
INT32 ArgPacketFormAdd(CHAR *pBuf, INT32 iBufLen);
INT32 ArgPacketFormEnd(VOID);
INT32 ArgPacketTrans(CHAR *pBuf, INT32 *piDataLen, INT32 iBufLen);
INT32 ArgPacketParseInit(CHAR *pcBuf, INT32 iBufLen);
INT32 ArgPacketParseHeader(UINT32 *piTotalLen, UINT32 *piArgNum);
INT32 ArgPacketParseArg(CHAR *pBuf, INT32 iBufLen);
INT32 ArgPacketParseArgLen(VOID);
INT32 ArgPacketParseEnd(VOID);

#ifdef __ARG_PACKET_SEM__
INT32 semaphore_creat(INT32 iKey);
BOOL semaphore_setval(INT32 iSemid);
BOOL semaphore_delval(INT32 iSemid);
BOOL semaphore_p(INT32 iSemid);
BOOL semaphore_v(INT32 iSemid);
#endif

#endif

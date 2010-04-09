#include "../../../Libs/Platform/Inc/Platform.h"
#include "../../../Libs/SoftPipe/include/softpipe.h"
#include "../../../Libs/DMA/Inc/DMA.h"
#include "../../../Libs/HIC/Inc/HIC.h"
#include "../../../Libs/CmdSet/Inc/CmdSet.h"
#include "../Inc/HIC_Client.h"

PACKET_FORM_CONTROL_T g_nfControl;
PACKET_PARSE_CONTROL_T g_npControl;
BOOL bPacketFormInite = FALSE, bPacketParseInite = FALSE;

/*
 *	pcBuf:			the buffer to store packeted arguments
 *	iBufLen:		the length of buffer
 *
 *	Return:			success(TRUE) or fail(FALSE)
 */
INT32 ArgPacketFormInit(CHAR *pcBuf, INT32 iBufLen)
{
	if(bPacketFormInite == FALSE)
	{
		bPacketFormInite = TRUE;
#ifdef __ARG_PACKET_SEM__
		g_nfControl.semArgPacketCtrl = semaphore_creat(0);
#endif
	}
#ifdef __ARG_PACKET_SEM__
	if(semaphore_p(g_nfControl.semArgPacketCtrl) == FALSE)
		return FALSE;
#endif
	
	if(pcBuf == NULL || iBufLen <4)
	{
#ifdef __ARG_PACKET_SEM__
		semaphore_delval(g_nfControl.semArgPacketCtrl);
#endif
		return FALSE;
	}
	
	g_nfControl.pcArgBuf = pcBuf;
	g_nfControl.iArgBufLen = iBufLen;
	g_nfControl.pcArgBufNow = pcBuf+sizeof(g_nfControl.iTotalLen)+sizeof(g_nfControl.iArgNum);
	g_nfControl.iTotalLen = 0;
	g_nfControl.iArgNum = 0;
	
	return TRUE;
}

/*
 *	pcBuf:			the buffer that has argument in it
 *	iBufLen:		the length of the argument
 *
 *	Return:			success(TRUE) or fail(FALSE)
 */
INT32 ArgPacketFormAdd(CHAR *pBuf, INT32 iBufLen)
{
	if(pBuf == NULL || iBufLen <=0)
		return FALSE;
	
	if((INT32)(iBufLen + sizeof(iBufLen)) > (INT32)(g_nfControl.iArgBufLen - (g_nfControl.pcArgBufNow - g_nfControl.pcArgBuf)))
	{
		return FALSE;
	}
	
	UINT32_2_UCHAR(iBufLen, (UCHAR*)g_nfControl.pcArgBufNow);
	g_nfControl.pcArgBufNow += sizeof(iBufLen);
	g_nfControl.iTotalLen += sizeof(iBufLen);
	
	memcpy(g_nfControl.pcArgBufNow, pBuf, iBufLen);
	g_nfControl.pcArgBufNow += iBufLen;
	g_nfControl.iTotalLen += iBufLen;
	
	g_nfControl.iArgNum++;
	
	return TRUE;
}

/*
 *	Return:			the total length of the packet in the buffer
 *
 */
INT32 ArgPacketFormEnd(VOID)
{
	if(g_nfControl.iArgNum > 0)
		g_nfControl.iTotalLen += sizeof(g_nfControl.iArgNum);
	
	g_nfControl.pcArgBufNow = g_nfControl.pcArgBuf;
	UINT32_2_UCHAR(g_nfControl.iTotalLen, (UCHAR*)g_nfControl.pcArgBufNow);
	g_nfControl.pcArgBufNow += sizeof(g_nfControl.iTotalLen);
	
	if(g_nfControl.iArgNum > 0)
		UINT32_2_UCHAR(g_nfControl.iArgNum, (UCHAR*)g_nfControl.pcArgBufNow);
	
	g_nfControl.iTotalLen += sizeof(g_nfControl.iTotalLen);
#ifdef __ARG_PACKET_SEM__
	semaphore_delval(g_nfControl.semArgPacketCtrl);
#endif
	return g_nfControl.iTotalLen;
}




/*
 *	pcBuf:				the buffer to store packeted arguments
 *	iBufLen:			the length of buffer
 *
 *	Return:				success(TRUE) or fail(FALSE)
 */
INT32 ArgPacketParseInit(CHAR *pcBuf, INT32 iBufLen)
{
	if(bPacketParseInite == FALSE)
	{
		bPacketParseInite = TRUE;
#ifdef __ARG_PACKET_SEM__
		g_npControl.semArgPacketCtrl = semaphore_creat(g_npControl.semArgPacketCtrl - 100);
#endif
	}
#ifdef __ARG_PACKET_SEM__
	if(semaphore_p(g_npControl.semArgPacketCtrl) == FALSE)
		return FALSE;
#endif
	
	if(pcBuf == NULL)
	{
#ifdef __ARG_PACKET_SEM__
		semaphore_delval(g_npControl.semArgPacketCtrl);
#endif
		return FALSE;
	}
	
	g_npControl.pcArgBuf = pcBuf;
	g_npControl.pcArgBufNow = pcBuf;
	
	return TRUE;
}

/*
 *	piTotalLen:			when return, this stores the total length of remained packet except first four bytes
 *	piArgNum:			when return, this stores the num of arguments in the packet
 *
 *	Return:				success(TRUE) or fail(FALSE)
 */
INT32 ArgPacketParseHeader(UINT32 *piTotalLen, UINT32 *piArgNum)
{
	UINT32 iTotalLen, iArgNum;
	
	//printf("ArgPacketParseHeader(): Header[0]=%d Header[1]=%d Header[2]=%d Header[3]=%d\n", g_npControl.pcArgBufNow[0], g_npControl.pcArgBufNow[1], g_npControl.pcArgBufNow[2], g_npControl.pcArgBufNow[3]);
	UCHAR_2_UINT32((UCHAR*)g_npControl.pcArgBufNow, iTotalLen);
    *piTotalLen = iTotalLen;
    g_npControl.pcArgBufNow += sizeof(iTotalLen);
    
    if(iTotalLen == 0)
    {
    	*piArgNum = 0;
    	return FALSE;
    }
    
	UCHAR_2_UINT32((UCHAR*)g_npControl.pcArgBufNow, iArgNum);
	//printf("ArgPacketParseHeader(): ArgNum[0]=%d Header[1]=%d Header[2]=%d Header[3]=%d\n", g_npControl.pcArgBufNow[0], g_npControl.pcArgBufNow[1], g_npControl.pcArgBufNow[2], g_npControl.pcArgBufNow[3]);
    *piArgNum = iArgNum;
    g_npControl.pcArgBufNow += sizeof(iArgNum);
    
    return TRUE;
}

/*
 *	pcBuf:				when return, the buffer stores parsed argument from the arguments packet
 *	iBufLen:			the length of buffer
 *
 *	Return:				the length of argument in buffer
 */
INT32 ArgPacketParseArg(CHAR *pBuf, INT32 iBufLen)
{
	INT32 iArgLen;
	
	UCHAR_2_UINT32((UCHAR*)g_npControl.pcArgBufNow, iArgLen);
	g_npControl.pcArgBufNow += sizeof(iArgLen);
	
	// If error, prepare to re-parse this argument
	if(iArgLen > iBufLen)
	{
		printf("ArgPacketParseArg(): buffer too small\n");
		g_npControl.pcArgBufNow -= sizeof(iArgLen);
		return 0;
	}
	
	memcpy(pBuf, g_npControl.pcArgBufNow, iArgLen);
	g_npControl.pcArgBufNow += iArgLen;
	
	return iArgLen;
}

/*
 *	Function:			this should be used to get the actual length of the argument when ArgPacketParseArg() return 0
 *
 *	Return:				the actual length of now argument
 *
 */
INT32 ArgPacketParseArgLen(VOID)
{
	INT32 iArgLen;
	
	UCHAR_2_UINT32((UCHAR*)g_npControl.pcArgBufNow, iArgLen);
	return iArgLen;
}

/*
 *	Return:				success(TRUE) or fail(FALSE)
 *
 */
INT32 ArgPacketParseEnd(VOID)
{
#ifdef __ARG_PACKET_SEM__
	semaphore_delval(g_npControl.semArgPacketCtrl);
#endif
	return TRUE;
}

#ifdef __ARG_PACKET_SEM__

/*
 *	Return:	-1 or positive number
 *
 */
INT32 semaphore_creat(INT32 iKey)
{
	UINT32 uiPid;
	
	if(iKey == 0)
		uiPid = (UINT32)getpid();
	else
		uiPid = iKey;
	return semget((key_t)uiPid, 1, 0666 | IPC_CREAT | IPC_EXCEL);
}

BOOL semaphore_setval(INT32 iSemid)
{
	union semun sem_union;
	
	sem_union = 1;
	if(semctl(iSemid, 0, SETVAL, sem_union) == -1)
		return FALSE;
	return TRUE;
}

BOOL semaphore_delval(INT32 iSemid)
{
	union semun sem_union;
	
	if(semctl(iSemid, 0, IPC_RMID, sem_union) == -1)
		return FALSE;
	
	return TRUE;
}

BOOL semaphore_p(INT32 iSemid)
{
	struct sembuf sem_b;
	
	sem_b.sem_num = 0;
	sem_b.sem_op = -1;
	sem_b.sem_flg = SEM_UNDO;
	if(semop(sem_id, &sem_b, 1) == -1)
		return FALSE
	return TRUE;
}

BOOL semaphore_v(INT32 iSemid)
{
	struct sembuf sem_b;
	
	sem_b.sem_num = 0;
	sem_b.sem_op = 1;
	sem_b.sem_flg = SEM_UNDO;
	if(semop(sem_id, &sem_b, 1) == -1)
		return FALSE;
	return TRUE;
}

#endif

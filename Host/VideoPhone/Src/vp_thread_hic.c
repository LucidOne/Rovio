#include "../Inc/inc.h"


#ifdef __HIC_SUPPORT__

__align (MEM_ALIGNMENT)	/* Use alignment so that g_HicThread.aucFIFO has the correct alignment. */
static UCHAR g_aucFIFO[HIC_FIFO_LEN];


#pragma arm section zidata = "non_init"
static HIC_THREAD_T g_HicThread;
#pragma arm section zidata



void hicSetCmdReady (HIC_THREAD_T *pHicThread)
{
	UINT32 uStatus;
	
	uStatus = inpw(REG_HICSR) & ~(bCMDBUSY | bERROR);
	outpw(REG_HICSR, uStatus);
	
	if (pHicThread->bEnableCommandInterrupt)
		hicSetIntr (CMD_HIC_INTR_STATE_CHANGED);
	
	DUMP_HIC_FINISH;
}

void hicSetCmdError (HIC_THREAD_T *pHicThread)
{
	UINT32 uStatus;
	
	uStatus = (inpw(REG_HICSR) & ~bCMDBUSY) | bERROR;
	outpw(REG_HICSR, uStatus);

	if (pHicThread->bEnableCommandInterrupt)
		hicSetIntr (CMD_HIC_INTR_STATE_CHANGED);
		
	DUMP_HIC_FINISH;
}


typedef struct
{
	HIC_THREAD_T *pHicThread;
	UINT32 uAddr;
	UINT32 uLen;
} HIC_DMA_T;

static void __hicStartDma_MemToFifo (void *arg)
{
	HIC_DMA_T *pDma = (HIC_DMA_T *) arg;

	outpw (REG_HICDAR, pDma->uAddr);
	outpw (REG_HICDLR, pDma->uLen);
	outpw (REG_HICFCR, ((inpw (REG_HICFCR) | bDMAEN) & ~bDMAWR));

	if (pDma->pHicThread->bEnableCommandInterrupt)
		hicSetIntr (CMD_HIC_INTR_STATE_CHANGED);
}

static void __hicStartDma_FifoToMem (void *arg)
{
	HIC_DMA_T *pDma = (HIC_DMA_T *) arg;

	outpw (REG_HICDAR, pDma->uAddr);
	outpw (REG_HICDLR, pDma->uLen);
	outpw (REG_HICFCR, ((inpw (REG_HICFCR) | bDMAEN) | bDMAWR));

	if (pDma->pHicThread->bEnableCommandInterrupt)
		hicSetIntr (CMD_HIC_INTR_STATE_CHANGED);
}


static void hicWaitDmaOK (HIC_THREAD_T *pHicThread)
{
	tt_sem_down (&pHicThread->pcDmaFlag.producer);
	tt_sem_up (&pHicThread->pcDmaFlag.producer);
}


#ifdef DEBUG_HIC_FIFO
static UINT32 __GetChecksum (CONST UCHAR *cpucData, UINT32 uLength)
{
	UINT32 uChecksum;
	CONST UCHAR *cpucEnd;
	
	for (cpucEnd = cpucData + uLength, uChecksum = 0;
		cpucData < cpucEnd;
		cpucData++)
	{
		uChecksum = ((uChecksum << 1) | ((uChecksum >> 23) & 0x01));
		uChecksum += (UINT32) *cpucData;
		uChecksum &= 0x00FFFFFF;
	}
	return uChecksum;
}
#endif


void hicStartDma_MemToFifo (HIC_THREAD_T *pHicThread, const void *pAddr, UINT32 uLen)
{
	HIC_DMA_T dma;
	
	while (1)
	{
		UINT32 uCheckLocal = 0;
		UINT32 uCheckRemote = 0;
	
		dma.pHicThread	= pHicThread;
		dma.uAddr		= (UINT32) pAddr;
		dma.uLen		= uLen;

#ifdef DEBUG_HIC_FIFO
		if (pHicThread->bUseFifoChecksum)
		{
			outpw (REG_HICPAR, ((UINT32) HIC_FIFO_START << 24)
				+ (inpw (REG_HICPAR) & 0x00FFFFFF));
		}
#endif

		tt_pc_produce (&pHicThread->pcDmaFlag, NULL, NULL);
		__hicStartDma_MemToFifo ((void *) &dma);
		hicWaitDmaOK (pHicThread);
		
#ifdef DEBUG_HIC_FIFO
		if (pHicThread->bUseFifoChecksum)
		{
			while ((UCHAR) (inpw (REG_HICPAR) >> 24) != HIC_FIFO_LOCAL_CHECKSUM_FILLED);
			uCheckRemote = inpw (REG_HICPAR);
			uCheckRemote &= 0x00FFFFFF;
			
			uCheckLocal = __GetChecksum ((CONST UCHAR *) NON_CACHE (pAddr), uLen);
			uCheckLocal &= 0x00FFFFFF;

			outpw (REG_HICPAR,
				((UINT32) HIC_FIFO_REMOTE_CHECKSUM_FILLED << 24)
				+ uCheckLocal
				);
		
			while ((UCHAR) (inpw (REG_HICPAR) >> 24) != HIC_FIFO_END);	
		}
#endif
		if (uCheckLocal == uCheckRemote)
			break;
		else
		{
			tt_msleep (1);
		}
	}
}


void hicStartDma_FifoToMem (HIC_THREAD_T *pHicThread, void *pAddr, UINT32 uLen)
{
	HIC_DMA_T dma;

	while (1)
	{
		UINT32 uCheckLocal = 0;
		UINT32 uCheckRemote = 0;
	
		dma.pHicThread	= pHicThread;
		dma.uAddr		= (UINT32) pAddr;
		dma.uLen		= uLen;

#ifdef DEBUG_HIC_FIFO
		if (pHicThread->bUseFifoChecksum)
		{
			outpw (REG_HICPAR, ((UINT32) HIC_FIFO_START << 24)
				+ (inpw (REG_HICPAR) & 0x00FFFFFF));
		}
#endif

		tt_pc_produce (&pHicThread->pcDmaFlag, NULL, NULL);
		__hicStartDma_FifoToMem((void *) &dma);
		hicWaitDmaOK (pHicThread);

#ifdef DEBUG_HIC_FIFO
		if (pHicThread->bUseFifoChecksum)
		{
			while ((UCHAR) (inpw (REG_HICPAR) >> 24) != HIC_FIFO_LOCAL_CHECKSUM_FILLED);
			uCheckRemote = inpw (REG_HICPAR);
			uCheckRemote &= 0x00FFFFFF;
			
			uCheckLocal = __GetChecksum ((CONST UCHAR *) NON_CACHE (pAddr), uLen);
			uCheckLocal &= 0x00FFFFFF;

			outpw (REG_HICPAR,
				((UINT32) HIC_FIFO_REMOTE_CHECKSUM_FILLED << 24)
				+ uCheckLocal
				);
		
			while ((UCHAR) (inpw (REG_HICPAR) >> 24) != HIC_FIFO_END);	
		}
#endif
		if (uCheckLocal == uCheckRemote)
			break;
		else
		{
			tt_msleep (1);
		}
	}
}



void hicMassData_MemToFifo (HIC_THREAD_T *pHicThread, const void *pAddr, UINT32 uLen)
{
#ifdef DEBUG_HIC_FIFO
	UINT32 uOK;
	for (uOK = 0; uOK < uLen; uOK += HIC_FIFO_LEN)
	{
		UINT32 uThis;
		if (uOK + HIC_FIFO_LEN <= uLen)
			uThis = HIC_FIFO_LEN;
		else
			uThis = uLen - uOK;
		
		hicStartDma_MemToFifo (pHicThread, (void *) (((char *) pAddr) + uOK), uThis);
	}
#else
	hicStartDma_MemToFifo (pHicThread, (void *) pAddr, uLen);
#endif
}


void hicMassData_FifoToMem (HIC_THREAD_T *pHicThread, void *pAddr, UINT32 uLen)
{
#ifdef DEBUG_HIC_FIFO
	UINT32 uOK;
	for (uOK = 0; uOK < uLen; uOK += HIC_FIFO_LEN)
	{
		UINT32 uThis;
		if (uOK + HIC_FIFO_LEN <= uLen)
			uThis = HIC_FIFO_LEN;
		else
			uThis = uLen - uOK;
		
		hicStartDma_FifoToMem (pHicThread, (void *) (((char *) pAddr) + uOK), uThis);
	}
#else
	hicStartDma_FifoToMem (pHicThread, (void *) pAddr, uLen);
#endif
}



/*
   Return:
	Number of parameters read (or partially read)
 */
int __hicReadCmd45 (CHAR *pcBuffer, UINT32 uLength, UINT32 *puLength,
					UINT32 uParamNum, HIC_CMD_45_T *pCmdParam)
{
	int nParamNum_IO = 0;
	CHAR *pc = pcBuffer;
	CHAR *pcEnd = pcBuffer + uLength;
	UINT32 u;
	
	/* Read the params number from buffer. */
	if (pc + sizeof (uParamNum) > pcEnd)
		goto lOut;	//Error buffer;
	if (uParamNum != UCHAR_2_UINT32((UCHAR *) pc))
		goto lOut;
	pc += sizeof (uParamNum);
	
	/* Read the params from buffer. */
	for (u = 0; u < uParamNum; u++)
	{
		UINT uParamLen_IO;

		if (pc + sizeof (pCmdParam[u].uParamLen) > pcEnd)
			goto lOut;	//Error buffer or terminated buffer
		pCmdParam[u].uParamLen = UCHAR_2_UINT32((UCHAR *) pc);
		pc += sizeof (pCmdParam[u].uParamLen);
		nParamNum_IO++;
		
		uParamLen_IO = pcEnd - pc;
		if (pCmdParam[u].uParamLen < uParamLen_IO)
			uParamLen_IO = pCmdParam[u].uParamLen;

		pCmdParam[u].pcParam = pc;
		pCmdParam[u].uParamLen_IO = uParamLen_IO;
		
		pc += uParamLen_IO;
	}

lOut:
	if (puLength != NULL)
		*puLength = pc - pcBuffer;
	return nParamNum_IO;
}


BOOL hicReadCmd45 (CHAR *pcBuffer, UINT32 uLength,
				   UINT32 uParamNum, HIC_CMD_45_T *pCmdParam)
{
	int nParamNum_IO = __hicReadCmd45 (pcBuffer, uLength, NULL, uParamNum, pCmdParam);
	if (nParamNum_IO != uParamNum)
		return FALSE;
	if (uParamNum > 0)
	{
		if (pCmdParam[uParamNum - 1].uParamLen != pCmdParam[uParamNum - 1].uParamLen_IO)
			return FALSE;
	}
	
	return TRUE;
}

/*
   Return:
	Number of parameters written (or partially written)
 */
int __hicWriteCmd5 (CHAR *pcBuffer, UINT32 uLength, UINT32 *puLength,
					UINT32 uParamNum, HIC_CMD_45_T *pCmdParam)
{
	int nParamNum_IO = 0;
	CHAR *pc = pcBuffer;
	CHAR *pcEnd = pcBuffer + uLength;
	UINT32 u;
	
	if (pc + sizeof (uParamNum) > pcEnd)
		goto lOut;	//No enough buffer;
	/* Write the params number to buffer. */
	UINT32_2_UCHAR (uParamNum, (UCHAR *) pc);
	pc += sizeof (uParamNum);
	
	
	/* Write the params to buffer. */
	for (u = 0; u < uParamNum; u++)
	{
		UINT32 uParamLen_IO;
		if (pc + sizeof (pCmdParam[u].uParamLen) > pcEnd)
			goto lOut;
		UINT32_2_UCHAR (pCmdParam[u].uParamLen, (UCHAR *) pc);
		pc += sizeof (pCmdParam[u].uParamLen);
		nParamNum_IO++;
		
		uParamLen_IO = pcEnd - pc;
		if (pCmdParam[u].uParamLen < uParamLen_IO)
			uParamLen_IO = pCmdParam[u].uParamLen;
		memcpy (pc, pCmdParam[u].pcParam, uParamLen_IO);
		pCmdParam[u].uParamLen_IO = uParamLen_IO;

		pc += uParamLen_IO;
	}

lOut:
	if (puLength != NULL)
		*puLength = pc - pcBuffer;
	return nParamNum_IO;
}


BOOL hicWriteCmd5 (CHAR *pcBuffer, UINT32 uLength, UINT32 *puLength,
				   UINT32 uParamNum, HIC_CMD_45_T *pCmdParam)
{
	int nParamNum_IO = __hicWriteCmd5 (pcBuffer, uLength, puLength, uParamNum, pCmdParam);
	if (nParamNum_IO != uParamNum)
		return FALSE;
	if (uParamNum > 0)
	{
		if (pCmdParam[uParamNum - 1].uParamLen != pCmdParam[uParamNum - 1].uParamLen_IO)
			return FALSE;
	}
	
	return TRUE;	
	
}


static __inline void hicThread_OnOneCmd (HIC_THREAD_T *pHicThread,
	UCHAR ucRegF, UCHAR ucRegE, UCHAR ucRegD,
	UCHAR ucRegC, UCHAR ucRegB, UCHAR ucRegA,
	const CMD_ENTRY_T *pCmdEntry,
	UINT32 uCmdEntryNum)
{
	FUN_CMD_PROC fnCmdProcessor;
	
	if (ucRegE < uCmdEntryNum)
	{
		ASSERT (ucRegE == pCmdEntry[ucRegE].ucSubCmd);
				
		if (pCmdEntry[ucRegE].fnCmdProcessor == NULL)
			fnCmdProcessor = &cmdNull;
		else
			fnCmdProcessor = pCmdEntry[ucRegE].fnCmdProcessor;
	}
	else
		fnCmdProcessor = &cmdNull;
				
	(*fnCmdProcessor) (pHicThread, ucRegF, ucRegE,
		ucRegD, ucRegC, ucRegB, ucRegA);
}


static void hicThread_OnCmd (void *pData)
{
	HIC_THREAD_T *pHicThread = (HIC_THREAD_T *) pData;

	UINT32	uHIC_Cmd	= inpw (REG_HICCMR);
	UINT32	uHIC_Param0	= inpw (REG_HICPAR);
	UINT32	uHIC_Param1	= inpw (REG_HICBLR);
		
	UCHAR	ucRegF	= (UCHAR) (uHIC_Cmd);
	UCHAR	ucRegE	= (UCHAR) (uHIC_Param1 >> 8);
	UCHAR	ucRegD	= (UCHAR) (uHIC_Param1);
	UCHAR	ucRegC	= (UCHAR) (uHIC_Param0 >> 24);
	UCHAR	ucRegB	= (UCHAR) (uHIC_Param0 >> 16);
	UCHAR	ucRegA	= (UCHAR) (uHIC_Param0 >> 8);

#ifdef DEBUG_HIC_FIFO
	pHicThread->bUseFifoChecksum = (0 == (UCHAR) uHIC_Param0 ? FALSE : TRUE);
#endif
	
	if (0)
	{
		int static i;	
		sysSafePrintf ("Command received: %02x %02x (%02x %02x %02x %02x) %d\n", ucRegF, ucRegE, ucRegD, ucRegC, ucRegB, ucRegA, i++);
		//sysprintf ("Command received: %02x %02x (%02x %02x %02x %02x) %d (check=%d)\n", ucRegF, ucRegE, ucRegD, ucRegC, ucRegB, ucRegA, i++, pHicThread->bUseFifoChecksum);
	}

	switch (ucRegF)
	{
		case (UCHAR) 0x56:
		{
			//Clear interrupt
			UINT32 uStatus;
			hicClearIntr ();
			uStatus = inpw(REG_HICSR) & ~(bCMDBUSY | bERROR);
			outpw(REG_HICSR, uStatus);
			break;
		}
		case (UCHAR) 0x57:
			hicThread_OnOneCmd (pHicThread,
				ucRegF, ucRegE, ucRegD, ucRegC, ucRegB, ucRegA,
				g_pCmdEntry_57, g_uCmdEntryNum_57);
			break;

		case (UCHAR) 0x18:
			hicThread_OnOneCmd (pHicThread,
				ucRegF, ucRegE, ucRegD, ucRegC, ucRegB, ucRegA,
				g_pCmdEntry_18, g_uCmdEntryNum_18);
			break;

		default:
			cmdNull (pHicThread, ucRegF, ucRegE, ucRegD, ucRegC, ucRegB, ucRegA);	//No such command.
	}
}


/* Hic thread main entry. */
static void hicThreadEntry (void *arg)
{
	FUN_TT_MSG_PROC	fnMsgProc;
	void			*pData;

	while (1)
	{
		tt_msg_recv (g_HicThread.pMsgQueue, &fnMsgProc, &pData);
		(*fnMsgProc) (pData);
	}
}


/* Hic interrupt handler. */
static void hicIrqHandler(void)
{
	UINT32 uStatus;
	
	uStatus = inpw (REG_HICISR);
	//sysSafePrintf ("REG_HICISR = %08x\n", uStatus);
	
	if ((uStatus & bCMDIS) != 0)
	{
#ifndef ECOS
		if (tt_msg_try_send (g_HicThread.pMsgQueue, hicThread_OnCmd, &g_HicThread) != 0)
#else
		if (tt_msg_try_send (g_HicThread.pMsgQueue, hicThread_OnCmd, &g_HicThread) != true)
#endif
			sysSafePrintf ("HIC ERROR 1\n!");
		outpw (REG_HICISR, bCMDIS);
	}
	
	if ((uStatus & bDMAIS) != 0)
	{
		if (tt_pc_try_consume (&g_HicThread.pcDmaFlag, NULL, NULL) != 0)
			sysSafePrintf ("HIC ERROR 2\n!");
		outpw (REG_HICISR, bDMAIS);
	}
	
	if ((uStatus & bBEIEN) != 0)
	{
		sysSafePrintf ("Hic Error\n");
	}
}


BOOL hicSetInterruptEvent (UINT32 uIntrEvent)
{
	BOOL bRt;
	sysDisableIRQ ();
	
	//sysSafePrintf ("Set intr event: %x\n", uIntrEvent);
	bRt = g_HicThread.bEnableEventInterrupt;
	if (bRt)
	{
		g_HicThread.bEnableEventInterrupt = FALSE;
		g_HicThread.uIntrEvent = uIntrEvent;
		hicSetIntr (CMD_HIC_INTR_APPLICATION_EVENT);
	}
	sysEnableIRQ ();
	
	return bRt;
}


BOOL hicIsInterruptEventEnabled (void)
{
	return g_HicThread.bEnableEventInterrupt;
}


void hicInitThread (void)
{
	/* Hic thread globals. */
	g_HicThread.pucFIFO = g_aucFIFO;
	g_HicThread.bEnableCommandInterrupt = FALSE;
	g_HicThread.bEnableEventInterrupt = FALSE;

	tt_pc_init (&g_HicThread.pcDmaFlag, 1);
	g_HicThread.pMsgQueue = tt_msg_queue_init (g_HicThread.auMsgBuffer,
											   sizeof (g_HicThread.auMsgBuffer));
	tt_create_thread ("hic",
		0,
		g_HicThread.auThreadBuffer,
		sizeof (g_HicThread.auThreadBuffer),
		hicThreadEntry,
		NULL);

	hicInitIntr ();
	sysInstallISR (IRQ_LEVEL_1, IRQ_HIC, (PVOID) hicIrqHandler);
	sysEnableInterrupt (IRQ_HIC);
	outpw (REG_HICIER, (bCMDIEN | bDMAIEN | bBEIEN));
	outpw (REG_HICSR, 0x00);
}

#endif


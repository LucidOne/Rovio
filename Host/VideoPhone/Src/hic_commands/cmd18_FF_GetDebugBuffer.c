#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__

void cmd18_FF_GetDebugBuffer (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	UINT32 uLength = ((UINT32) ucC << 16)
		| ((UINT32) ucB << 8)
		| ((UINT32) ucA);

	DUMP_HIC_COMMAND;
	

	if (uLength != 4)
	{
		hicSaveToReg1 (HIC_ERR_UNKNOWN_CMD_PARAM);
		hicSetCmdReady (pHicThread);
	}
	else
	{
		hicStartDma_FifoToMem (pHicThread, pHicThread->pucFIFO, uLength);

		if (!hicReadCmd45 ((CHAR *) NON_CACHE (pHicThread->pucFIFO), uLength, 0, NULL))
		{
			hicSaveToReg1 (HIC_ERR_UNKNOWN_CMD_PARAM);
			hicSetCmdReady (pHicThread);
		}
		else
		{
			extern char ASM_INFO_BUFFER_BEGIN[];
			extern char ASM_INFO_BUFFER_END[];
			extern char ASM_ERROR_BUFFER_BEGIN[];
			extern char ASM_ERROR_BUFFER_END[];

			UINT32 uInfoAddr	= (UINT32) ASM_INFO_BUFFER_BEGIN;
			UINT32 uInfoSize	= (UINT32) ASM_INFO_BUFFER_END - (UINT32) ASM_INFO_BUFFER_BEGIN;
			UINT32 uErrorAddr	= (UINT32) ASM_ERROR_BUFFER_BEGIN;
			UINT32 uErrorSize	= (UINT32) ASM_ERROR_BUFFER_END - (UINT32) ASM_ERROR_BUFFER_BEGIN;
			HIC_CMD_45_T acCmd[4];
			acCmd[0].uParamLen	= sizeof (UINT32);
			acCmd[0].pcParam	= (CHAR *) &uInfoAddr;
			acCmd[1].uParamLen	= sizeof (UINT32);
			acCmd[1].pcParam	= (CHAR *) &uInfoSize;
			acCmd[2].uParamLen	= sizeof (UINT32);
			acCmd[2].pcParam	= (CHAR *) &uErrorAddr;
			acCmd[3].uParamLen	= sizeof (UINT32);
			acCmd[3].pcParam	= (CHAR *) &uErrorSize;

			if (!hicWriteCmd5 ((CHAR *) NON_CACHE (pHicThread->pucFIFO),
							   HIC_FIFO_LEN, &uLength,
							   sizeof (acCmd) / sizeof (acCmd[0]), acCmd))
			{
				hicSaveToReg1 (HIC_ERR_FIRMWARE_BUG);
				hicSetCmdReady (pHicThread);
			}
			else
			{
				hicSaveToReg1 (uLength);
				hicStartDma_MemToFifo (pHicThread, pHicThread->pucFIFO, uLength);
				hicSetCmdReady (pHicThread);
			}
		}	
	}	
}

#endif

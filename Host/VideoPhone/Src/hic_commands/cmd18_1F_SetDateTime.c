#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd18_1F_SetDateTime (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	struct tm tms;
	UINT32 uLength = ((UINT32) ucC << 16)
				   | ((UINT32) ucB << 8)
				   | ((UINT32) ucA);
	HIC_CMD_45_T acParam[6];
	
	DUMP_HIC_COMMAND;

	/* <1> Read parameters. */
	if (uLength > HIC_FIFO_LEN)
	{
		hicSaveToReg1 (HIC_ERR_UNKNOWN_CMD_PARAM);
		hicSetCmdError (pHicThread);
		return;
	}
	
	hicStartDma_FifoToMem (pHicThread, pHicThread->pucFIFO, uLength);
	if (!hicReadCmd45 ((CHAR *) NON_CACHE (pHicThread->pucFIFO), uLength,
		sizeof (acParam) / sizeof (acParam[0]), acParam))
	{
		hicSaveToReg1 (HIC_ERR_UNKNOWN_CMD_PARAM);
		hicSetCmdError (pHicThread);
		return;
	}

	tms.tm_year	= (int) UCHAR_2_USHORT ((UCHAR *) acParam[0].pcParam);
	tms.tm_mon	= (UCHAR) acParam[1].pcParam[0];
	tms.tm_mday	= (UCHAR) acParam[2].pcParam[0];
	tms.tm_hour	= (UCHAR) acParam[3].pcParam[0];
	tms.tm_min	= (UCHAR) acParam[4].pcParam[0];
	tms.tm_sec	= (UCHAR) acParam[5].pcParam[0];
	
	if (tms.tm_year >= 0 && tms.tm_year <= 9999 - 1900
		&& tms.tm_mon >= 0 && tms.tm_mon <= 11
		&& tms.tm_mday >= 1 && tms.tm_mday <= 31
		&& tms.tm_hour >= 0 && tms.tm_hour <= 23
		&& tms.tm_min >= 0 && tms.tm_min <= 59
		&& tms.tm_sec >= 0 && tms.tm_sec <= 59)
	{
		vinfoSetTime (&tms);
		hicSetCmdReady (pHicThread);
	}
	else
	{
		hicSaveToReg1 (HIC_ERR_UNKNOWN_CMD_PARAM);
		hicSetCmdError (pHicThread);
	}	
}

#endif

#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__

void cmd18_0E_SetLocalVideoSource (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	int				nReturn;
	USHORT			usWidth;
	USHORT			usHeight;
	CMD_ROTATE_E	eRotate;
	
	HIC_CMD_45_T	acParam[3];

	UINT32 uLength = ((UINT32) ucC << 16)
				   | ((UINT32) ucB << 8)
				   | ((UINT32) ucA);

	DUMP_HIC_COMMAND;

	/* <1> Read parameters. */
	if (uLength > HIC_FIFO_LEN)
	{
		hicSaveToReg1 (HIC_ERR_UNKNOWN_CMD_PARAM);
		hicSetCmdError (pHicThread);
		return;
	}
	
	/* <2> Configure. */
	hicStartDma_FifoToMem (pHicThread, pHicThread->pucFIFO, uLength);
	if (hicReadCmd45 ((CHAR *) NON_CACHE (pHicThread->pucFIFO), uLength,
		sizeof (acParam) / sizeof (acParam[0]), acParam))
	{
		usWidth		= UCHAR_2_USHORT ((UCHAR *) acParam[0].pcParam);
		usHeight	= UCHAR_2_USHORT ((UCHAR *) acParam[1].pcParam);
		eRotate		= (CMD_ROTATE_E) UCHAR_2_UINT32 ((UCHAR *) acParam[2].pcParam);
		nReturn = vcptSetWindow (usWidth, usHeight, eRotate);
	}
	else
	{
		nReturn = HIC_ERR_UNKNOWN_CMD_PARAM;
	}

	if (nReturn == 0)
		hicSetCmdReady (pHicThread);
	else
	{
		hicSaveToReg1 (nReturn);
		hicSetCmdError (pHicThread);
	}
}


#endif

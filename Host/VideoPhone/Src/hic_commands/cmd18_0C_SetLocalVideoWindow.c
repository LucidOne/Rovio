#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__

void cmd18_0C_SetLocalVideoWindow (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	BOOL bIsConfigureOK;
	VP_POINT_T tDisplayPos;
	VP_SIZE_T tScalingSize;
	VP_RECT_T tClippedWindow;
	CMD_ROTATE_E	eRotate;
	
	HIC_CMD_45_T	acParam[9];

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
		5, acParam))
	{
		tDisplayPos.sLeft				= (SHORT) UCHAR_2_USHORT ((UCHAR *) acParam[0].pcParam);
		tDisplayPos.sTop				= (SHORT) UCHAR_2_USHORT ((UCHAR *) acParam[1].pcParam);
		tClippedWindow.tSize.usWidth	= UCHAR_2_USHORT ((UCHAR *) acParam[2].pcParam);
		tClippedWindow.tSize.usHeight	= UCHAR_2_USHORT ((UCHAR *) acParam[3].pcParam);
		eRotate		= (CMD_ROTATE_E) UCHAR_2_UINT32 ((UCHAR *) acParam[4].pcParam);
		bIsConfigureOK = vdSetLocalWindow (tDisplayPos, tClippedWindow.tSize, eRotate);
		vcptWaitPrevMsg ();
	}
	else if (hicReadCmd45 ((CHAR *) NON_CACHE (pHicThread->pucFIFO), uLength,
		9, acParam))
	{
		tDisplayPos.sLeft				= (SHORT) UCHAR_2_USHORT ((UCHAR *) acParam[0].pcParam);
		tDisplayPos.sTop				= (SHORT) UCHAR_2_USHORT ((UCHAR *) acParam[1].pcParam);
		tClippedWindow.tSize.usWidth	= UCHAR_2_USHORT ((UCHAR *) acParam[2].pcParam);
		tClippedWindow.tSize.usHeight	= UCHAR_2_USHORT ((UCHAR *) acParam[3].pcParam);
		eRotate							= (CMD_ROTATE_E) UCHAR_2_UINT32 ((UCHAR *) acParam[4].pcParam);
		tScalingSize.usWidth			= UCHAR_2_USHORT ((UCHAR *) acParam[5].pcParam);
		tScalingSize.usHeight			= UCHAR_2_USHORT ((UCHAR *) acParam[6].pcParam);
		tClippedWindow.tPoint.sLeft		= UCHAR_2_USHORT ((UCHAR *) acParam[7].pcParam);
		tClippedWindow.tPoint.sTop		= UCHAR_2_USHORT ((UCHAR *) acParam[8].pcParam);
		bIsConfigureOK = vdSetLocalWindowEx (tDisplayPos,
							tScalingSize,
							tClippedWindow,
							eRotate);
		vcptWaitPrevMsg ();
	}
	else
		bIsConfigureOK = FALSE;
		
	if (bIsConfigureOK)
		hicSetCmdReady (pHicThread);
	else
	{
		hicSaveToReg1 (HIC_ERR_UNKNOWN_CMD_PARAM);
		hicSetCmdError (pHicThread);
	}
}


#endif

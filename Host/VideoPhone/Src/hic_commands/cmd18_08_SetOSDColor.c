#include "../../Inc/inc.h"

#ifdef __HIC_SUPPPORT__

void cmd18_08_SetOSDColor (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	CMD_LCM_OSD_COLOR_E	eOsdColor;
	UCHAR				ucKeyColor_R;
	UCHAR				ucKeyColor_G;
	UCHAR				ucKeyColor_B;
	UCHAR				ucKeyMask_R;
	UCHAR				ucKeyMask_G;
	UCHAR				ucKeyMask_B;
	

	UINT32			uErrNo;
	HIC_CMD_45_T	acParam[3];
	HIC_CMD_45_T	acRespon[1];

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
	
	
	hicStartDma_FifoToMem (pHicThread, pHicThread->pucFIFO, uLength);
	if (!hicReadCmd45 ((CHAR *) NON_CACHE (pHicThread->pucFIFO), uLength,
		sizeof (acParam) / sizeof (acParam[0]), acParam))
	{
		hicSaveToReg1 (HIC_ERR_UNKNOWN_CMD_PARAM);
		hicSetCmdError (pHicThread);
		return;
	}

	eOsdColor		= (CMD_LCM_OSD_COLOR_E) UCHAR_2_UINT32 ((UCHAR *) acParam[0].pcParam);
	ucKeyColor_R	= (UCHAR) acParam[1].pcParam[0];
	ucKeyColor_G	= (UCHAR) acParam[1].pcParam[1];
	ucKeyColor_B	= (UCHAR) acParam[1].pcParam[2];
	ucKeyMask_R		= (UCHAR) acParam[2].pcParam[0];
	ucKeyMask_G		= (UCHAR) acParam[2].pcParam[1];
	ucKeyMask_B		= (UCHAR) acParam[2].pcParam[2];


	/* <2> Configure. */
	uErrNo = vlcmSetOSDColorMode (eOsdColor,
		ucKeyColor_R,
		ucKeyColor_G,
		ucKeyColor_B,
		ucKeyMask_R,
		ucKeyMask_G,
		ucKeyMask_B);


	/* <3> Send response. */
	acRespon[0].pcParam		= (char *) &uErrNo;
	acRespon[0].uParamLen	= sizeof (uErrNo);
	hicWriteCmd5 ((CHAR *) NON_CACHE (pHicThread->pucFIFO), HIC_FIFO_LEN, &uLength,
		sizeof (acRespon) / sizeof (acRespon[0]), acRespon);

	hicSaveToReg1 (uLength);
	hicStartDma_MemToFifo (pHicThread, pHicThread->pucFIFO, uLength);
	hicSetCmdReady (pHicThread);
}

#endif


#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__

void cmd18_06_ConfigureLCM (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	USHORT						usWidth;
	USHORT						usHeight;
	CMD_LCM_COLOR_WIDTH_E		eLcmColor;
	CMD_LCM_CMD_BUS_WIDTH_E		eCmdBus;
	CMD_LCM_DATA_BUS_WIDTH_E	eDataBus;
	int							iMPU_Cmd_RS_pin;
	BOOL						b16t18;
	CMD_LCM_MPU_MODE_E			eMpuMode;

	UINT32			uErrNo;
	HIC_CMD_45_T	acParam[8];
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
	

	/* <2> Configure. */
	hicStartDma_FifoToMem (pHicThread, pHicThread->pucFIFO, uLength);
	if (!hicReadCmd45 ((CHAR *) NON_CACHE (pHicThread->pucFIFO), uLength,
		sizeof (acParam) / sizeof (acParam[0]), acParam))
	{
		hicSaveToReg1 (HIC_ERR_UNKNOWN_CMD_PARAM);
		hicSetCmdError (pHicThread);
		return;
	}

	usWidth		= UCHAR_2_USHORT ((UCHAR *) acParam[0].pcParam);
	usHeight	= UCHAR_2_USHORT ((UCHAR *) acParam[1].pcParam);
	eLcmColor	= (CMD_LCM_COLOR_WIDTH_E) UCHAR_2_UINT32 ((UCHAR *) acParam[2].pcParam);
	eCmdBus		= (CMD_LCM_CMD_BUS_WIDTH_E) UCHAR_2_UINT32 ((UCHAR *) acParam[3].pcParam);
	eDataBus	= (CMD_LCM_DATA_BUS_WIDTH_E) UCHAR_2_UINT32 ((UCHAR *) acParam[4].pcParam);
	iMPU_Cmd_RS_pin	= (int) UCHAR_2_UINT32 ((UCHAR *) acParam[5].pcParam);
	b16t18		= (BOOL) UCHAR_2_UINT32 ((UCHAR *) acParam[6].pcParam);
	eMpuMode	= (CMD_LCM_MPU_MODE_E) UCHAR_2_UINT32 ((UCHAR *) acParam[7].pcParam);


	/* <3> Send response. */
	uErrNo = vlcmConfigure (usWidth, usHeight,
			eLcmColor,
			eCmdBus,
			eDataBus,
			iMPU_Cmd_RS_pin,
			b16t18,
			eMpuMode);

	acRespon[0].pcParam		= (char *) &uErrNo;
	acRespon[0].uParamLen	= sizeof (uErrNo);
	hicWriteCmd5 ((CHAR *) NON_CACHE (pHicThread->pucFIFO), HIC_FIFO_LEN, &uLength,
		sizeof (acRespon) / sizeof (acRespon[0]), acRespon);

	hicSaveToReg1 (uLength);
	hicStartDma_MemToFifo (pHicThread, pHicThread->pucFIFO, uLength);
	hicSetCmdReady (pHicThread);
}

#endif


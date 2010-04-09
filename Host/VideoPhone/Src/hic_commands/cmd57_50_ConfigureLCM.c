#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd57_50_ConfigureLCM (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
#if defined ENV_RADIOTEL

	UINT32 uErrNo;

	CMD_LCM_COLOR_WIDTH_E eLcmColor;
	CMD_LCM_CMD_BUS_WIDTH_E eCmdBus;
	
	switch ((WB_LCM_COLOR_WIDTH_E) ucB)
	{
		case WB_LCM_COLOR_WIDTH_16:
			eLcmColor = CMD_LCM_COLOR_WIDTH_16;
			break;
		case WB_LCM_COLOR_WIDTH_18:
			eLcmColor = CMD_LCM_COLOR_WIDTH_18;
			break;
		case WB_LCM_COLOR_WIDTH_12:
			eLcmColor = CMD_LCM_COLOR_WIDTH_12;
			break;
		case WB_LCM_COLOR_WIDTH_24:
			eLcmColor = CMD_LCM_COLOR_WIDTH_24;
			break;
		default:
			hicSaveToReg1 (APP_ERR_CAPABILITY_LIMIT);
			hicSetCmdError (pHicThread);
			return;
	}

	switch ((WB_LCM_BUS_WIDTH_E) ucC)
	{
		case WB_LCM_BUS_WIDTH_16_18:
			eCmdBus = CMD_LCM_CMD_BUS_WIDTH_16;
			break;
		case WB_LCM_BUS_WIDTH_8_9:
			eCmdBus = CMD_LCM_CMD_BUS_WIDTH_8;
			break;
		default:
			hicSaveToReg1 (APP_ERR_CAPABILITY_LIMIT);
			hicSetCmdError (pHicThread);
			return;
	}

	uErrNo = vlcmConfigure (240, 320,
			eLcmColor,
			eCmdBus,
			CMD_LCM_DATA_BUS_WIDTH_16,
			0,
			FALSE,
			CMD_LCM_80_MPU);

	hicSaveToReg1 (uErrNo);
	hicSetCmdReady (pHicThread);
#else
	cmdNull (pHicThread, ucCmd, ucSubCmd,
		ucD, ucC, ucB, ucA);
#endif
}

#endif

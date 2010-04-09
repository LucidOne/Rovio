#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__

void cmd18_01_GetSystemCapability (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	DUMP_HIC_COMMAND;
	
	switch ((CMD_SYSTEM_CAPABILITY_E) ucD)
	{
		case CMD_SYSTEM_CAPABILITY_MAX_LCM_PIXELS:
			hicSaveToReg1 (MAX_LCM_WIDTHxHEIGHT);
			hicSetCmdReady (pHicThread);
			break;
		case CMD_SYSTEM_CAPABILITY_MAX_AUDIO_QUALITY_LEVEL:
			hicSaveToReg1 (MAX_AMR_ENCODER_LEVEL);
			hicSetCmdReady (pHicThread);
			break;
		case CMD_SYSTEM_CAPABILITY_MAX_MP4_BITRATE:
			hicSaveToReg1 (MAX_MP4_BITRATE);
			hicSetCmdReady (pHicThread);
			break;
		case CMD_SYSTEM_CAPABILITY_MAX_MP4_PIXELS:
			hicSaveToReg1 (MAX_MP4_WIDTHxHEIGHT);
			hicSetCmdReady (pHicThread);
			break;
		default:
			hicSaveToReg1 (HIC_ERR_UNKNOWN_CMD_PARAM);
			hicSetCmdError (pHicThread);
	}
}

#endif

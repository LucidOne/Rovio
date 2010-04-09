#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd18_1B_SetAudioFormat (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	CMD_AUDIO_FORMAT_E eEncodeFormat = (CMD_AUDIO_FORMAT_E) ucA;
	CMD_AUDIO_FORMAT_E eDecodeFormat = (CMD_AUDIO_FORMAT_E) ucB;
	
	DUMP_HIC_COMMAND;
	
	vauSetFormat (eEncodeFormat, eDecodeFormat);
	hicSetCmdReady (pHicThread);
}

#endif

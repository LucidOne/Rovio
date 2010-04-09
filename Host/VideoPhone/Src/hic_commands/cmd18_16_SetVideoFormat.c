#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd18_16_SetVideoFormat (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	CMD_VIDEO_FORMAT_E eFormat = (CMD_VIDEO_FORMAT_E) ucA;
	
	DUMP_HIC_COMMAND;
	
	
	vmp4encSetFormat (eFormat);
	hicSetCmdReady (pHicThread);
}

#endif

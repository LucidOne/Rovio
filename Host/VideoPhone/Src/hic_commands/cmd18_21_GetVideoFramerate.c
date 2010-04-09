#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd18_21_GetVideoFramerate (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	UINT32 uFrameRate;

	DUMP_HIC_COMMAND;
	
	vmp4encGetQuality (&uFrameRate, NULL);
	hicSaveToReg1 (uFrameRate);

	hicSetCmdReady (pHicThread);
}

#endif

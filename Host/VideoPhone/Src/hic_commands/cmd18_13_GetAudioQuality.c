#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd18_13_GetAudioQuality (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	DUMP_HIC_COMMAND;
	

	hicSaveToReg4 (0, 0, 0,
		(UCHAR) vauGetQualityLevel ());

	hicSetCmdReady (pHicThread);
}

#endif

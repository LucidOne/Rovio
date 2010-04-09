#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd18_15_GetVideoBitrate (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	UINT32 uBitRate;

	DUMP_HIC_COMMAND;
	
	vmp4encGetQuality (NULL, &uBitRate);
	hicSaveToReg1 (uBitRate);

	hicSetCmdReady (pHicThread);
}

#endif

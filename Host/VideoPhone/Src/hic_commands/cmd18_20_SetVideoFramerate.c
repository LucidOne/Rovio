#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd18_20_SetVideoFramerate (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	INT rt;
	UINT32 uBitRate;
	UINT32 uFrameRate = ((UINT32) ucD << 24)
		| ((UINT32) ucC << 16)
		| ((UINT32) ucB << 8)
		| ((UINT32) ucA);
	
	DUMP_HIC_COMMAND;
	
	
	vmp4encGetQuality (NULL, &uBitRate);
	
	rt = vmp4encSetQuality (uFrameRate, uBitRate);
	if (rt == 0)
		hicSetCmdReady (pHicThread);
	else
	{
		hicSaveToReg1 ((UINT32) rt);
		hicSetCmdError (pHicThread);
	}
}

#endif

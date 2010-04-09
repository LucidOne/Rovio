#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd18_12_SetAudioQuality (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	INT rt;
	INT nQuality = (INT) ucA;

	DUMP_HIC_COMMAND;
	
	
	rt = vauSetQualityLevel (nQuality);
	if (rt == 0)
		hicSetCmdReady (pHicThread);
	else
	{
		hicSaveToReg1 ((UINT32) rt);
		hicSetCmdError (pHicThread);
	}
}

#endif

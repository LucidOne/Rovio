#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd18_1A_GetMotionsNum (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	DUMP_HIC_COMMAND;
	

	hicSaveToReg1 (vdGetMotionsNum ());

	hicSetCmdReady (pHicThread);
}

#endif

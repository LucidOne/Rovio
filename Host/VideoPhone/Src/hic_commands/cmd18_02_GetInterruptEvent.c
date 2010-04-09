#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__

void cmd18_02_GetInterruptEvent (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	DUMP_HIC_COMMAND;
	
	if (ucD != 0)
		hicClearIntr ();
	
	hicSaveToReg1 (pHicThread->uIntrEvent);
	hicSetCmdReady (pHicThread);
}

#endif

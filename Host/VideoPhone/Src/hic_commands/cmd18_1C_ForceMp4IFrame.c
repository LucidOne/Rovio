#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd18_1C_ForceMP4IFrame (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	UINT32 uMillisecond = (((UINT32) ucD) << 8)
		| ((UINT32) ucC);
	UINT32 uNum = (((UINT32) ucB) << 8)
		| ((UINT32) ucA);

	DUMP_HIC_COMMAND;
	
	/* Clear encoded buffer. */
	vmp4Lock ();
	
	//vmp4ClearBuffer ();
	//Do not clear buffer! otherwise the header may be cleared.
	
	vmp4ForceIFrame (uNum, uMillisecond);
	vmp4Unlock ();

	hicSetCmdReady (pHicThread);
}

#endif

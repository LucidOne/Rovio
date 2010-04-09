#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd18_18_EnableVideoBlur (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	BOOL bEnable = (ucD == 0 ? FALSE : TRUE);

	DUMP_HIC_COMMAND;
	
	vdLock ();
	vdEnableMP4Blur (bEnable);
	vdUnlock ();

	hicSetCmdReady (pHicThread);
}

#endif

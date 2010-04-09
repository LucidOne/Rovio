#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd18_10_SetVideoZIndex (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	BOOL bLocalOnTop = (ucD == 0 ? FALSE : TRUE);

	DUMP_HIC_COMMAND;
	

	vdSetZIndex (bLocalOnTop);
	
	vcptWaitPrevMsg ();
	hicSetCmdReady (pHicThread);
}

#endif

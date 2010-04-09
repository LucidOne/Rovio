#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd18_07_SelectLCM (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	DUMP_HIC_COMMAND;
	
	cmdNull (pHicThread, ucCmd, ucSubCmd, ucD, ucC, ucB, ucA);
}

#endif

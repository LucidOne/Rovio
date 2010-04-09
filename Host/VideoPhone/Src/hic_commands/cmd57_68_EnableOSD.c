#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd57_68_EnableOSD (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	DUMP_HIC_COMMAND;
	
	cmd18_0B_EnableDisplay (pHicThread, ucCmd, ucSubCmd,
		CMD_DISPLAY_OSD, ucD, 0, 0);
}

#endif

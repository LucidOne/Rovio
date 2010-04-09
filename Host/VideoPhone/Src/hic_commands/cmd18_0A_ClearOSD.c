#include "../../Inc/inc.h"


#ifdef __HIC_SUPPORT__

void cmd18_0A_ClearOSD (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	BOOL bClearBackground = (ucD == 0 ? FALSE : TRUE);
	UCHAR ucColorR = ucC;
	UCHAR ucColorG = ucB;
	UCHAR ucColorB = ucA;

	DUMP_HIC_COMMAND;
	

	if (!vlcmIsConfigured ())
	{
		hicSaveToReg1 (APP_ERR_LCM_NO_CONFIG);
		hicSetCmdError (pHicThread);
		return;		
	}

	if (bClearBackground)
		vlcmFillLCMBuffer (ucColorR, ucColorG, ucColorB);
	else
		vlcmFillOSDBuffer (ucColorR, ucColorG, ucColorB);

	PTL;
	vlcmStartRefresh (FALSE);
	PTL;
	
	hicSetCmdReady (pHicThread);
}

#endif

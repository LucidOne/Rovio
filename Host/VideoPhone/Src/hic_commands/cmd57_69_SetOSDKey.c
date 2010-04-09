#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd57_69_SetOSDKey (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	UINT32 uErrNo;
	CMD_LCM_OSD_COLOR_E eOsdColor;
	UCHAR ucKeyColor_R;
	UCHAR ucKeyColor_G;
	UCHAR ucKeyColor_B;
	UCHAR ucKeyMask_R;
	UCHAR ucKeyMask_G;
	UCHAR ucKeyMask_B;

	DUMP_HIC_COMMAND;
	
	vlcmGetOSDColorMode (&eOsdColor,
		&ucKeyColor_R,
		&ucKeyColor_G,
		&ucKeyColor_B,
		&ucKeyMask_R,
		&ucKeyMask_G,
		&ucKeyMask_B);

	if (ucD == 0)
	{/* Set OSD key pattern */
		ucKeyColor_R = ucC;
		ucKeyColor_G = ucB;
		ucKeyColor_B = ucA;
	}
	else
	{/* Set OSD key mask */
		ucKeyMask_R = ucC;
		ucKeyMask_G = ucB;
		ucKeyMask_B = ucA;
	}
	
	uErrNo = vlcmSetOSDColorMode (eOsdColor,
		ucKeyColor_R,
		ucKeyColor_G,
		ucKeyColor_B,
		ucKeyMask_R,
		ucKeyMask_G,
		ucKeyMask_B);

	hicSaveToReg1 (uErrNo);
	hicSetCmdReady (pHicThread);
}

#endif

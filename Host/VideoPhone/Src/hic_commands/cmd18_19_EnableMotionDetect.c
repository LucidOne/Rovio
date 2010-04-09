#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd18_19_EnableMotionDetect (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	BOOL bEnable = (ucD == 0 ? FALSE : TRUE);
	CMD_MOTION_DETECT_SENSIBILITY_E eSen1 = (CMD_MOTION_DETECT_SENSIBILITY_E) ucA;
	IMG_CMP_SENSIBILITY_E eSen2;

	DUMP_HIC_COMMAND;
	
	vdLock ();
	switch (eSen1)
	{
	case CMD_MOTION_DETECT_LOW:
		eSen2 = IMG_CMP_SENSIBILITY_LOW;
		break;
	case CMD_MOTION_DETECT_MIDDLE:
		eSen2 = IMG_CMP_SENSIBILITY_MIDDLE;
		break;
	case CMD_MOTION_DETECT_HIGH:
	default:
		eSen2 = IMG_CMP_SENSIBILITY_HIGH;
		break;
	}
	vdEnableMotionDetect (bEnable, eSen2);
	vdUnlock ();

	hicSetCmdReady (pHicThread);
}

#endif

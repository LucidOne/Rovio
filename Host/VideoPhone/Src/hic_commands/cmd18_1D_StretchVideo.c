#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd18_1D_StretchVideo (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	UINT8 ucVaScaleV_Round;
	UINT8 ucVaScaleH_Round;
	UINT16 usVaScaleV;
	UINT16 usVaScaleH;
	
	DUMP_HIC_COMMAND;
	
	usVaScaleV = (((UINT16) ucD) << 8) + (UINT16) ucC;
	usVaScaleH = (((UINT16) ucB) << 8) + (UINT16) ucA;
	
	if (usVaScaleV < (UINT16) 100
		|| usVaScaleV >= (UINT16) 800
		|| usVaScaleH < (UINT16) 100
		|| usVaScaleH >= (UINT16) 800)
	{
		hicSaveToReg1 (APP_ERR_CAPABILITY_LIMIT);
		hicSetCmdError (pHicThread);
		return;
	}
		
	ucVaScaleV_Round = (UINT8) (usVaScaleV / 100);
	usVaScaleV = (UINT16) ((usVaScaleV - ucVaScaleV_Round * 100UL) * 1024UL / 100UL);
	ucVaScaleH_Round = (UINT8) (usVaScaleH / 100);
	usVaScaleH = (UINT16) ((usVaScaleH - ucVaScaleH_Round * 100UL) * 1024UL / 100UL);
	

	vlcmLock ();
	vpostVA_Stream_Scaling_Ctrl (ucVaScaleV_Round, usVaScaleV,
		ucVaScaleH_Round, usVaScaleH,
		scall_mode_Interpolation);
	vlcmUnlock ();

	hicSetCmdReady (pHicThread);
}

#endif

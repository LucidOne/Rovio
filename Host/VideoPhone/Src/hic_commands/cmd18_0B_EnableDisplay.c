#include "../../Inc/inc.h"


#ifdef __HIC_SUPPORT__

void cmd18_0B_EnableDisplay (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	CMD_DISPLAY_TYPE_E eDisplayType = (CMD_DISPLAY_TYPE_E) ucD;
	BOOL bEnable = (ucC == 0 ? FALSE : TRUE);

	DUMP_HIC_COMMAND;

	if (!vlcmIsConfigured ())
	{
		hicSaveToReg1 (APP_ERR_LCM_NO_CONFIG);
		hicSetCmdError (pHicThread);
		return;		
	}

	switch (eDisplayType)
	{
		case CMD_DISPLAY_OSD:
		{
			static BOOL bPrevousStatus = FALSE;
			if (bPrevousStatus != bEnable)
			{
				bPrevousStatus = bEnable;
			
				vlcmLock ();
				if (bEnable)
					vpostOSD_Enable ();
				else
					vpostOSD_Disable ();
				vlcmUnlock ();
				vlcmStartRefresh (FALSE);
			}
			hicSetCmdReady (pHicThread);
			break;
		}
		case CMD_DISPLAY_LOCAL_VIDEO:
		{
			static BOOL bPrevousStatus = FALSE;
			if (bPrevousStatus != bEnable)
			{
				bPrevousStatus = bEnable;

				if (bEnable)
					vcptStart ();
				else
					vcptStop ();

				vdEnableLocalWindow (bEnable);
				vcptWaitPrevMsg ();
			}
			hicSetCmdReady (pHicThread);
			break;
		}
		case CMD_DISPLAY_REMOTE_VIDEO:
		{
			static BOOL bPrevousStatus = FALSE;
			if (bPrevousStatus != bEnable)
			{
				VP_VIDEO_T *pVideo;
				bPrevousStatus = bEnable;
				
				vdLock ();
				pVideo = vdGetSettings ();
		
				vmp4Lock ();
				if (bEnable)
				{
					vmp4decAddRef (pVideo);
					vdEnableRemoteWindow (bEnable);
				}
				else
				{
					vdEnableRemoteWindow (bEnable);
					vmp4decDecRef ();
				}
				vmp4Unlock ();

				vdUnlock ();
			
				vcptWaitPrevMsg ();
			}
			hicSetCmdReady (pHicThread);
			break;
		}
		default:
			hicSaveToReg1 (HIC_ERR_UNKNOWN_CMD_PARAM);
			hicSetCmdError (pHicThread);	
	}
}

#endif

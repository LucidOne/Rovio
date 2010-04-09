#include "../../Inc/inc.h"


#ifdef __HIC_SUPPORT__

void cmd18_0F_EnableVideoEncoder (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	static BOOL bPrevousStatus = FALSE;
	

	BOOL bEnable = (ucD == 0 ? FALSE : TRUE);

	DUMP_HIC_COMMAND;
	

	if (bPrevousStatus != bEnable)
	{
		bPrevousStatus = bEnable;
	
		if (bEnable)
		{
			VP_VIDEO_T *pVideo;
			
			vdLock ();
			pVideo = vdGetSettings ();
		
			/* Enable MP4 encoding. */
			vmp4Lock ();
			vdEnableLocalMP4 (bEnable);
			vmp4encAddRef (pVideo);
			vmp4Unlock ();
			
			vdUnlock ();
		
			/* Start capture. */
			vcptStart ();
		}
		else
		{
			/* Stop capture. */
			vcptStop ();
		
			/* Disable MP4 encoding. */
			vmp4Lock ();
			vdEnableLocalMP4 (bEnable);
			vmp4encDecRef ();
		
			/* Clear encoded buffer. */
			vmp4ClearBuffer ();
		
			vmp4Unlock ();
		}
		
		vcptWaitPrevMsg ();
	}
	
	hicSetCmdReady (pHicThread);
}

#endif


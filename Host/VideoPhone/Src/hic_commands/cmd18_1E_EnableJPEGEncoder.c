#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd18_1E_EnableJPEGEncoder (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
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
			vdLock ();
			vjpegLock();
		
			TURN_ON_JPEG_CLOCK;
			tt_msleep (30);

			vdEnableLocalJPEG (bEnable);
			vjpegEncInit();
			//vjpegDecInit();
	        jpegInitializeEngine();
	        vjpegUnlock ();
   			vdUnlock ();
        
			/* Start capture. */
			vcptStart ();
		}
		else
		{
			/* Stop capture. */
			vcptStop ();
			
			vjpegLock();
			vdEnableLocalJPEG (bEnable);
			TURN_OFF_JPEG_CLOCK;
			vjpegUnlock ();	
		}

   	}
	
	hicSetCmdReady (pHicThread);
}

#endif

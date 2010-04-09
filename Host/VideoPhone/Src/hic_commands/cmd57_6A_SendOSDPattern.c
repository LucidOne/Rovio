#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__




void cmd57_6A_SendOSDPattern (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	static UINT32 uAddress = 0;
	UINT32 uColorWidth;
	VP_BUFFER_OSD_T *pOSDBuffer;
	
	DUMP_HIC_COMMAND;
	
	
	if (!vlcmIsConfigured ())
	{
		hicSaveToReg1 (APP_ERR_LCM_NO_CONFIG);
		hicSetCmdError (pHicThread);
		return;		
	}

	switch (ucD)
	{
		case (UCHAR) 0x00:	//Send address
		case (UCHAR) 0x02:	//Send address
			uAddress = ((UINT32) ucC << 16)
				   | ((UINT32) ucB << 8)
				   | ((UINT32) ucA);
			break;
			
		case (UCHAR) 0x01:	//Send mass OSD data
		{
			UINT32 uLength = ((UINT32) ucC << 16)
				   | ((UINT32) ucB << 8)
				   | ((UINT32) ucA);
			
			vlcmLock ();
			pOSDBuffer = vlcmGetOSDBuffer ();
			if (uAddress + uLength <= sizeof (pOSDBuffer->aucData))
			{
				hicStartDma_FifoToMem (pHicThread, pOSDBuffer->aucData, uLength);
			}
			vlcmUnlock ();
			break;
		}
		case (UCHAR) 0x03:	//Send data
		{
			UCHAR acPattern[4];
			vlcmLock ();
			uColorWidth = vlcmGetOSDColorWidth ();
			acPattern[0] = ucA;
			acPattern[1] = ucB;
			acPattern[2] = ucC;			
			
			pOSDBuffer = vlcmGetOSDBuffer ();
			
			if (uAddress + uColorWidth <= sizeof (pOSDBuffer->aucData))
			{
				memcpy ((void *) (NON_CACHE (pOSDBuffer->aucData) + uAddress), acPattern,
					uColorWidth);
			}
			vlcmUnlock ();
			break;
		}	
		case (UCHAR) 0x04:	//Flush
		{
			vlcmStartRefresh (FALSE);
			break;
		}
		default:;
	}
	
	
	hicSetCmdReady (pHicThread);
	
}


#endif

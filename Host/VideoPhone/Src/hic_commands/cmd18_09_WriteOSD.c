#include "../../Inc/inc.h"


#ifdef __HIC_SUPPORT__

__inline void COPY_OSD_DATA(char * cpucBuffer,UINT32 uLength,
	UINT32 uPixelWidth, UINT32 usScreenWidth, UINT32 usScreenHeight,
	INT nLeft, INT nTop, INT nRight, INT nBottom, INT nWidth, INT nHeight,
	INT *nX,
	INT *nY,
	UCHAR **ppucOSDBuffer
)
{
	UINT32 u;
	CONST UCHAR *alias_cpucBuffer = (CONST UCHAR *) (cpucBuffer);
	UINT32 alias_uLength = (UINT32) uLength;

	for (u = 0; u < alias_uLength; u += uPixelWidth)
	{
		if (*nX >= 0 && *nX < (INT) usScreenWidth
			&& *nY >= 0 && *nY < (INT) usScreenHeight)
			memcpy (*ppucOSDBuffer, alias_cpucBuffer, uPixelWidth);

		(*nX)++;
		*ppucOSDBuffer += uPixelWidth;
		if (*nX >= nRight)
		{
			*nX = nLeft;
			(*nY)++;

			*ppucOSDBuffer += ((INT) usScreenWidth - nWidth) * uPixelWidth;
		}

		alias_cpucBuffer += uPixelWidth;
	}
}



void cmd18_09_WriteOSD (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	INT	nLeft;
	INT nTop;
	INT nWidth;
	INT nHeight;

	VP_BUFFER_OSD_T *pOSDBuffer;
	VP_BUFFER_OSD_T *pOSDBuffer_old;
	UCHAR *pucOSDBuffer;
	USHORT usScreenWidth;
	USHORT usScreenHeight;
	UINT32 uPixelWidth;
	INT nX;
	INT nY;
	INT nRight;
	INT nBottom;
	
	
	HIC_CMD_45_T	acParam[5];
	
	UINT32 uLength_OK;
	UINT32 uLength = ((UINT32) ucC << 16)
				   | ((UINT32) ucB << 8)
				   | ((UINT32) ucA);

	DUMP_HIC_COMMAND;
	

	if (!vlcmIsConfigured ())
	{
		hicSaveToReg1 (APP_ERR_LCM_NO_CONFIG);
		hicSetCmdError (pHicThread);
		return;		
	}

	/* <1> Try to read first block. */
	uLength_OK = (HIC_FIFO_LEN < uLength ? HIC_FIFO_LEN : uLength);
	hicStartDma_FifoToMem (pHicThread, pHicThread->pucFIFO, uLength_OK);
	
	if (__hicReadCmd45 ((CHAR *) NON_CACHE (pHicThread->pucFIFO), uLength_OK, NULL,
		sizeof (acParam) / sizeof (acParam[0]), acParam)
		!= sizeof (acParam) / sizeof (acParam[0]))
	{
		hicSaveToReg1 (HIC_ERR_UNKNOWN_CMD_PARAM);
		hicSetCmdError (pHicThread);
		return;		
	}

	/* <2> Get OSD rectangle. */
	nLeft = (INT) (SHORT) UCHAR_2_USHORT ((UCHAR *) acParam[0].pcParam);
	nTop = (INT) (SHORT) UCHAR_2_USHORT ((UCHAR *) acParam[1].pcParam);
	nWidth = (INT) UCHAR_2_USHORT ((UCHAR *) acParam[2].pcParam);
	nHeight = (INT) UCHAR_2_USHORT ((UCHAR *) acParam[3].pcParam);
	
sysSafePrintf ("%d %d %d %d\n", nLeft, nTop, nWidth, nHeight);
	/* <3> Copy OSD data */
	pOSDBuffer = bufOSDNew ();
	if (pOSDBuffer == NULL)
	{
		hicSaveToReg1 (HIC_ERR_FIRMWARE_BUG);
		hicSetCmdError (pHicThread);
		return;
	}
	pucOSDBuffer = (UCHAR *) NON_CACHE (pOSDBuffer->aucData);

	
	vlcmGetSize (&usScreenWidth, &usScreenHeight);
	uPixelWidth = vlcmGetOSDColorWidth ();

	/* Copy old OSD buffer first. */
	vlcmLock ();
	pOSDBuffer_old = vlcmGetOSDBuffer ();
	
	vgfxLock ();
	gfxMemcpy (pOSDBuffer->aucData, pOSDBuffer_old->aucData,
		vlcmGetOSDColorWidth () * usScreenWidth * usScreenHeight);
	vgfxUnlock ();
	
	vlcmUnlock ();

	
	nX = nLeft;
	nY = nTop;
	nRight = nLeft + nWidth;
	nBottom = nTop + nHeight;
	pucOSDBuffer += (nX + nY * usScreenWidth) * uPixelWidth;

	COPY_OSD_DATA (acParam[4].pcParam, acParam[4].uParamLen_IO,
		uPixelWidth, usScreenWidth, usScreenHeight,
		nLeft, nTop, nRight, nBottom, nWidth, nHeight,
		&nX, &nY, &pucOSDBuffer
		);
	
	/* <4> Copy the remaining OSD data */
	if (nLeft == 0 && nWidth == (INT) usScreenWidth && uLength_OK < uLength)
	{
		hicStartDma_FifoToMem (pHicThread, pucOSDBuffer, uLength - uLength_OK);
	}
	else
	{	
		for (; uLength_OK < uLength; )
		{
			UINT32 uLengthThis = (uLength_OK + HIC_FIFO_LEN < uLength ? HIC_FIFO_LEN : uLength - uLength_OK);
			hicStartDma_FifoToMem (pHicThread, pHicThread->pucFIFO, uLengthThis);
		
			uLength_OK += uLengthThis;
		
			COPY_OSD_DATA ((CHAR *) NON_CACHE (pHicThread->pucFIFO), uLengthThis,
				uPixelWidth, usScreenWidth, usScreenHeight,
				nLeft, nTop, nRight, nBottom, nWidth, nHeight,
				&nX, &nY, &pucOSDBuffer		
				);
		}
	}

	sysSafePrintf ("Read OSD OK\n");

	pOSDBuffer_old = vlcmSetOSDBuffer (pOSDBuffer);
	bufOSDDecRef (pOSDBuffer_old);
	
	vlcmStartRefresh (FALSE);
	
	//vlcmWaitRefreshOK ();
	sysSafePrintf ("Refresh .... ok\n");
	
	
	hicSetCmdReady (pHicThread);	
}


#endif

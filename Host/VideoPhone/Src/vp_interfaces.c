#include "../Inc/inc.h"

#ifndef AU_REC_INT_NUM
#define AU_REC_INT_NUM		7
#endif

//#define INTERFACES_DEBUG

typedef struct
{
	/* Mailbox for core and read's communication */
	//cyg_handle_t	cygSendMsgQueue;
	//cyg_mbox		cygSendMBox;
	TT_RMUTEX_T		mtIOEvent;
	TT_COND_T		cdIOEvent;
	
	//cyg_mutex_t		cygMtVideoMsg;
	//cyg_cond_t		cygCdVideoMsg;
	//UINT32			uMP4Count;
	//UINT32			uJPGCount;
	//UINT32			uMotionCount;
	//UINT32			uLoopCount;
	UINT32			uEventWaitCount;
	UINT32			uEventInterrupt;
	
	//cyg_handle_t	cygSendMsgQueueAudio;
	//cyg_mbox		cygSendMBoxAudio;
	
	/* Mutex for stop and read */
	//cyg_mutex_t		cygMtMP4;
	//cyg_mutex_t		cygMtAudio;
	//cyg_mutex_t		cygMtJpeg;
	
	/* For motion detect */
	//INT32			iMotionCount;
	
	/* For read and write */
	//cyg_mutex_t		cygMtRead;
	cyg_mutex_t		cygMtWrite;
	//cyg_mutex_t		cygMtReadAudio;
	//cyg_mutex_t		cygMtReadVideo;
	
	/* For ioctl */
	cyg_mutex_t		cygMtMP4Enable;
	//cyg_mutex_t		cygMtAudioEnable;
	TT_RMUTEX_T		cygMtAudioEnable;
	BOOL			bPrevousAudioStatus;
	
	cyg_mutex_t		cygMtJpegEnable;
	cyg_mutex_t		cygMtIoctl;
	
	//int				iAudioCount;
	//int				iVideoCount;
	//int				iAudioMask;
}IO_THREAD_T;

IO_THREAD_T	g_IOThread;

int iothread_Init(void)
{
	/* Init Mailbox for core and read's communication */
	//cyg_mbox_create(&(g_IOThread.cygSendMsgQueue), &(g_IOThread.cygSendMBox));
	tt_rmutex_init(&g_IOThread.mtIOEvent);
	tt_cond_init(&g_IOThread.cdIOEvent, &g_IOThread.mtIOEvent);
	
	//cyg_mutex_init(&g_IOThread.cygMtVideoMsg);
	//cyg_cond_init(&g_IOThread.cygCdVideoMsg, &g_IOThread.cygMtVideoMsg);
	//g_IOThread.uMP4Count = 0;
	//g_IOThread.uJPGCount = 0;
	//g_IOThread.uMotionCount = 0;
	//g_IOThread.uLoopCount = 0;
	g_IOThread.uEventWaitCount = 0;
	g_IOThread.uEventInterrupt = 0;
	
	
	//cyg_mbox_create(&(g_IOThread.cygSendMsgQueueAudio), &(g_IOThread.cygSendMBoxAudio));
	
	/* Init Mutex for stop and read */
	//cyg_mutex_init(&g_IOThread.cygMtMP4);
	//cyg_mutex_init(&g_IOThread.cygMtAudio);
	//cyg_mutex_init(&g_IOThread.cygMtJpeg);
	//cyg_mutex_lock(&g_IOThread.cygMtMP4);
	//cyg_mutex_lock(&g_IOThread.cygMtAudio);
	//cyg_mutex_lock(&g_IOThread.cygMtJpeg);
	
	/* For motion detect */
	//g_IOThread.iMotionCount = 0;
	
	/* For recv and register */
	//cyg_mutex_init(&g_IOThread.cygMtRead);
	cyg_mutex_init(&g_IOThread.cygMtWrite);
	//cyg_mutex_init(&g_IOThread.cygMtReadAudio);
	//cyg_mutex_init(&g_IOThread.cygMtReadVideo);
	
	/* Init ioctl lock */
	cyg_mutex_init(&g_IOThread.cygMtMP4Enable);
	
	//cyg_mutex_init(&g_IOThread.cygMtAudioEnable);
	tt_rmutex_init(&g_IOThread.cygMtAudioEnable);
	g_IOThread.bPrevousAudioStatus = FALSE;
	
	cyg_mutex_init(&g_IOThread.cygMtJpegEnable);
	cyg_mutex_init(&g_IOThread.cygMtIoctl);
	
	return TRUE;
}

#if 0
int iothread_ReadAudio(IO_THREAD_READ_T *pArg)
{
	MEDIA_TYPE_T	tmpType;
	
	VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *pAudioBuf;
	
	cyg_mutex_lock(&g_IOThread.cygMtReadAudio);
	
	while(1)
	{
		tmpType = (MEDIA_TYPE_T)(UINT32)cyg_mbox_get(g_IOThread.cygSendMsgQueueAudio);
		if(tmpType == NULL)
		{
			diag_printf("iothread_ReadAudio(): the send mbox be released...? Thread over!!\n");
			cyg_mutex_unlock(&g_IOThread.cygMtReadAudio);
			return IO_THREAD_READ_OVER;
		}
		
		switch(tmpType)
		{
			case MEDIA_TYPE_AUDIO:
				g_IOThread.iAudioCount++;
				
				if(cyg_mutex_trylock(&g_IOThread.cygMtAudio) == false)
				{
					iothread_SendAudioNotify(MEDIA_TYPE_AUDIO);
					continue;
				}
				
				pAudioBuf = vauGetEncBuffer_Local ();
				if (pAudioBuf == NULL)
				{
					printf("iothread_Read(): can't get audio data\n");
					cyg_mutex_unlock(&g_IOThread.cygMtAudio);
					continue;
				}
				
				pArg->mediatype = MEDIA_TYPE_AUDIO;
				pArg->txlen = pAudioBuf->nSize;
				//pArg->txbuf = (UCHAR*)NON_CACHE(pAudioBuf->aucAudio);
				pArg->txbuf = (UCHAR*)(pAudioBuf->aucAudio);
				pArg->extradata = (VOID*)pAudioBuf;
				return IO_THREAD_READ_OK;
			default:
				diag_printf("iothread_ReadAudio(): media type%d not support\n", tmpType);
				continue;
		}
	}
}

int iothread_ReadAudio_Complete(IO_THREAD_READ_T *pArg)
{
	VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *pAudioBuf;
	
	switch(pArg->mediatype)
	{
		/* Get media packet and set mpType,mpData and mpLength */
		case MEDIA_TYPE_AUDIO:
			pAudioBuf = pArg->extradata;
			
			cyg_interrupt_disable();
			listDetach (&pAudioBuf->list);
			bufEncAudioDecRef (pAudioBuf);
			cyg_interrupt_enable();
			
			cyg_mutex_unlock(&g_IOThread.cygMtAudio);
			cyg_mutex_unlock(&g_IOThread.cygMtReadAudio);
			return TRUE;
		default:
			diag_printf("iothread_ReadAudio_Complete(): media type%d not support\n", pArg->mediatype);
			return FALSE;
	}
}


int iothread_ReadVideo(IO_THREAD_READ_T *pArg)
{
	VP_BUFFER_MP4_BITSTREAM_T	*pMP4Buf;
	VP_BUFFER_JPEG_ENCODER_T	*pJPEGBuf;
	
	cyg_mutex_lock(&g_IOThread.cygMtReadVideo);
	
	while(1)
	{
		int nFristChoice;
		BOOL bHaveMP4 = FALSE;
		BOOL bHaveJPG = FALSE;
		BOOL bHaveMotion = FALSE;

		//tmpType = (MEDIA_TYPE_T)(UINT32)cyg_mbox_get(g_IOThread.cygSendMsgQueue);
		
		cyg_mutex_lock(&g_IOThread.cygMtVideoMsg);
		if (!(g_IOThread.uMP4Count > 0)
			&& !(g_IOThread.uJPGCount > 0)
			&& !(g_IOThread.uMotionCount > 0)
			)
			cyg_cond_wait(&g_IOThread.cygCdVideoMsg);
		
		++g_IOThread.uLoopCount;
		if (g_IOThread.uMP4Count > 0)
		{
			--g_IOThread.uMP4Count;
			bHaveMP4 = TRUE;
		}
		if (g_IOThread.uJPGCount > 0)
		{
			--g_IOThread.uJPGCount;
			bHaveJPG = TRUE;
		}
		if (g_IOThread.uMotionCount > 0)
		{
			--g_IOThread.uMotionCount;
			bHaveMotion = TRUE;
		}		
		cyg_mutex_unlock(&g_IOThread.cygMtVideoMsg);

		for (nFristChoice = 0; nFristChoice < 3; ++nFristChoice)
		{
			switch ((nFristChoice + g_IOThread.uLoopCount) % 3)
			{
				case 0:
					if (bHaveMP4)
					{
						/* Get media packet and set mpType,mpData and mpLength */
						g_IOThread.iVideoCount++;
				
						if(cyg_mutex_trylock(&g_IOThread.cygMtMP4) == false)
						{
							iothread_SendNotify(MEDIA_TYPE_VIDEO);
							continue;
						}
				
						pMP4Buf = vmp4encGetBuffer ();
						if (pMP4Buf == NULL)
						{
							printf("iothread_ReadVideo(): can't get mp4 data\n");
							cyg_mutex_unlock(&g_IOThread.cygMtMP4);
							continue;
						}
				
						listDetach (&pMP4Buf->list);
						pArg->mediatype = MEDIA_TYPE_VIDEO;
						pArg->txlen = pMP4Buf->uLength;
						pArg->txbuf = (UCHAR*)NON_CACHE(pMP4Buf->aucBitstream);
						pArg->extradata = (VOID*)pMP4Buf;
						return IO_THREAD_READ_OK;
					}
					break;
				case 1:
					if (bHaveJPG)
					{
						if(cyg_mutex_trylock(&g_IOThread.cygMtJpeg) == false)
						{
							iothread_SendNotify(MEDIA_TYPE_JPEG);
							continue;
						}
				
						pJPEGBuf = vjpegGetEncBuffer ();
						if (pJPEGBuf == NULL)
						{
							printf("iothread_ReadVideo(): can't get jpeg data\n");
							cyg_mutex_unlock(&g_IOThread.cygMtJpeg);
							continue;
						}
				
						listDetach (&pJPEGBuf->list);
						pArg->mediatype = MEDIA_TYPE_JPEG;
						pArg->txlen = pJPEGBuf->uJPEGDatasize;
						pArg->txbuf = (UCHAR*)NON_CACHE(pJPEGBuf->aucJPEGBitstream);
						pArg->extradata = (VOID*)pJPEGBuf;
						return IO_THREAD_READ_OK;
					}
					break;
				case 2:
					if (bHaveMotion)
					{
						pArg->mediatype = MEDIA_TYPE_MOTION;
						pArg->txlen = 0;
				
						g_IOThread.iMotionCount--;
				
						cyg_mutex_unlock(&g_IOThread.cygMtReadVideo);
						return IO_THREAD_READ_OK;
					}
					break;
				default:;
			}

		}
		continue;
	}
}

int iothread_ReadVideo_Complete(IO_THREAD_READ_T *pArg)
{
	VP_BUFFER_MP4_BITSTREAM_T	*pMP4Buf;
	VP_BUFFER_JPEG_ENCODER_T	*pJPEGBuf;
	
	switch(pArg->mediatype)
	{
		/* Get media packet and set mpType,mpData and mpLength */
		case MEDIA_TYPE_VIDEO:
			pMP4Buf = pArg->extradata;
			
			cyg_interrupt_disable();
			listDetach (&pMP4Buf->list);
			bufMP4BitstreamDecRef (pMP4Buf);
			cyg_interrupt_enable();
			
			cyg_mutex_unlock(&g_IOThread.cygMtMP4);
			cyg_mutex_unlock(&g_IOThread.cygMtReadVideo);
			return TRUE;
		case MEDIA_TYPE_JPEG:
			pJPEGBuf = pArg->extradata;
				
			cyg_interrupt_disable();
			listDetach (&pJPEGBuf->list);
			vjpegFreeEncBuffer (pJPEGBuf);
			cyg_interrupt_enable();
			
			cyg_mutex_unlock(&g_IOThread.cygMtJpeg);
			cyg_mutex_unlock(&g_IOThread.cygMtReadVideo);
			return TRUE;
		case MEDIA_TYPE_MOTION:
			return TRUE;
		default:
			diag_printf("iothread_ReadVideo_Complete(): media type%d not support\n", pArg->mediatype);
			return FALSE;
	}
}


void iothread_ClearNotify(MEDIA_TYPE_T mpType)
{
	cyg_mutex_lock(&g_IOThread.cygMtVideoMsg);
	switch (mpType)
	{
		case MEDIA_TYPE_VIDEO:
			g_IOThread.uMP4Count = 0;
			break;
		case MEDIA_TYPE_JPEG:
			g_IOThread.uJPGCount = 0;
			break;
		case MEDIA_TYPE_MOTION:
			g_IOThread.uMotionCount = 0;
			break;		
		default:
			;
	}
	cyg_mutex_unlock(&g_IOThread.cygMtVideoMsg);
}

int iothread_SendNotify(MEDIA_TYPE_T mpType)
{
#ifdef INTERFACES_DEBUG
	printf("iothread_SendNotify(): mails=%d\n", cyg_mbox_peek(g_IOThread.cygSendMsgQueue));
#endif
	
	if((mpType == MEDIA_TYPE_MOTION) && (g_IOThread.iMotionCount > 0)) return true;
	if(mpType == MEDIA_TYPE_MOTION) g_IOThread.iMotionCount++;
	
#if 0	
	//if(cyg_mbox_tryput(g_IOThread.cygSendMsgQueue, (void*)mpType) == true)
	if(cyg_mbox_put(g_IOThread.cygSendMsgQueue, (void*)mpType) == true)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
#else

	cyg_mutex_lock(&g_IOThread.cygMtVideoMsg);
	switch (mpType)
	{
		case MEDIA_TYPE_VIDEO:
//diag_printf("Notify MP4\n");			
			++g_IOThread.uMP4Count;
			break;
		case MEDIA_TYPE_JPEG:
//diag_printf("Notify JPEG\n");			
			++g_IOThread.uJPGCount;
			break;
		case MEDIA_TYPE_MOTION:
			++g_IOThread.uMotionCount;
			break;		
		default:
			cyg_mutex_unlock(&g_IOThread.cygMtVideoMsg);
			return FALSE;
	}
	cyg_cond_signal(&g_IOThread.cygCdVideoMsg);
	cyg_mutex_unlock(&g_IOThread.cygMtVideoMsg);
	return TRUE;
#endif
}

int iothread_SendAudioNotify(MEDIA_TYPE_T mpType)
{
#ifdef INTERFACES_DEBUG
	printf("iothread_SendAudioNotify(): mails=%d\n", cyg_mbox_peek(g_IOThread.cygSendMsgQueueAudio));
#endif
	if(cyg_mbox_put(g_IOThread.cygSendMsgQueueAudio, (void*)mpType) == true)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


#else

/*
 * ppMP4Buf,	MP4 buffer
 * ppJPEGBuf,	JPEG buffer
 * ppAudioBuf,	audio buffer
 * pMotionCount,count of motion detected
 *	Set to NULL if the corresponding event need not to be selected.
 */
void iothread_EventRead(
	VP_BUFFER_MP4_BITSTREAM_T			**ppMP4Buf,
	VP_BUFFER_JPEG_ENCODER_T			**ppJPEGBuf,
	VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T	**ppAudioBuf,
	int									*pMotionCount
	)
{
	BOOL bHaveEvent = FALSE;
	
	if (ppMP4Buf == NULL
		&& ppJPEGBuf == NULL
		&& ppAudioBuf == NULL
		&& pMotionCount == NULL)
		return;	/* Nothing need to be read */
	
	iothread_EventLock();
	
	++g_IOThread.uEventWaitCount;
	
	while (1)
	{
		if (g_IOThread.uEventInterrupt > 0)
		{
			--g_IOThread.uEventInterrupt;
			break;
		}
		
		if (ppMP4Buf != NULL)
		{
			sysDisableIRQ ();
			*ppMP4Buf = vmp4encGetBuffer ();
			if (*ppMP4Buf != NULL)
			{
				listDetach (&(*ppMP4Buf)->list);
				bHaveEvent = TRUE;
			}
			sysEnableIRQ ();
		}
	
		if (ppJPEGBuf != NULL)
		{
			sysDisableIRQ ();
			*ppJPEGBuf = vjpegGetEncBuffer ();
			if (*ppJPEGBuf != NULL)
			{
				listDetach (&(*ppJPEGBuf)->list);
				bHaveEvent = TRUE;
			}
			sysEnableIRQ ();
		}
	
		if (ppAudioBuf != NULL)
		{
			vauLock ();
			*ppAudioBuf = vauGetEncBuffer_Local ();
			if (*ppAudioBuf != NULL)
			{
				listDetach (&(*ppAudioBuf)->list);
				bHaveEvent = TRUE;
			}
			vauUnlock ();
		}
	
		if (pMotionCount != NULL)
		{
			VP_VIDEO_T *pVideo = vdGetSettings ();
			if (pVideo->bIsEnableMotionDetect
				&& pVideo->uMotionsSinceLast != 0)
			{
				vdLock();
				*pMotionCount = pVideo->uMotionsSinceLast;
				pVideo->uMotionsSinceLast = 0;
				bHaveEvent = TRUE;
				vdUnlock();
			}
		}
		
		if (bHaveEvent != FALSE)
			break;

		tt_cond_wait(&g_IOThread.cdIOEvent);
	}
	
	--g_IOThread.uEventWaitCount;
	iothread_EventUnlock();
}


void iothread_EventInterrupt()
{
	iothread_EventLock();
	g_IOThread.uEventInterrupt = g_IOThread.uEventWaitCount;
	iothread_EventNotify();
	iothread_EventUnlock();
}

void iothread_EventNotify()
{
	tt_cond_broadcast(&g_IOThread.cdIOEvent);
}


void iothread_EventLock()
{
	tt_rmutex_lock(&g_IOThread.mtIOEvent);
}

void iothread_EventUnlock()
{
	tt_rmutex_unlock(&g_IOThread.mtIOEvent);
}
#endif



int iothread_Write(IO_THREAD_READ_T *pArg)
{
	VP_BUFFER_MP4_DECODER_T *pMP4DecBuf;
	VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *pEncAudio;
    VP_BUFFER_MP4_DECODER_T *pEncJpeg;
	
	int iAudioCount;
	int iBufSize;
    VP_JPEG_DEC_STATE_E eDecodeResult;
    
    MEDIA_TYPE_T mpType = pArg->mediatype;
    UCHAR *mpData = pArg->txbuf;
    UINT32 mpLength = pArg->txlen;
    
	if(mpData == NULL || mpLength == 0) return false;
	
	/* To copy data to encode buffer */
	cyg_mutex_lock(&g_IOThread.cygMtWrite);
	switch(mpType)
	{
		case MEDIA_TYPE_VIDEO:
			pMP4DecBuf = vmp4decGetBuffer ();
			if(pMP4DecBuf == NULL)
			{
				diag_printf("iothread_Write(): can't get video buffer\n");
				cyg_mutex_unlock(&g_IOThread.cygMtWrite);
				return FALSE;
			}
			
			if(mpLength > sizeof(pMP4DecBuf->aucBitstream))
			{
				diag_printf("iothread_Write(): mp4 data too large\n");
				cyg_mutex_unlock(&g_IOThread.cygMtWrite);
				return FALSE;
			}
			memcpy((char*) NON_CACHE (pMP4DecBuf->aucBitstream), (char*)  (mpData), mpLength);
			
			vmp4decPrepare ((UINT32) NON_CACHE (pMP4DecBuf->aucBitstream), mpLength);
			break;
		case MEDIA_TYPE_AUDIO:
			pEncAudio = vauGetEncBuffer_Remote ();
			if(pEncAudio == NULL)
			{
				printf("iothread_Write(): can't get audio buffer\n");
				cyg_mutex_unlock(&g_IOThread.cygMtWrite);
				return FALSE;
			}
			
			iAudioCount = 0;
			iBufSize = sizeof(pEncAudio->aucAudio);
			while(mpLength > iBufSize)
			{
				memcpy((char*) NON_CACHE (pEncAudio->aucAudio), (char*)  (mpData + iAudioCount), iBufSize);
				vauDecode ((UCHAR*) NON_CACHE (pEncAudio->aucAudio), iBufSize);
				mpLength -= iBufSize;
				iAudioCount += iBufSize;
				//tt_msleep(60);
			}
			
			if(mpLength != 0)
			{
				memcpy((char*) NON_CACHE (pEncAudio->aucAudio), (char*)  (mpData + iAudioCount), mpLength);
				vauDecode ((UCHAR*) NON_CACHE (pEncAudio->aucAudio), mpLength);
			}
			break;
		case MEDIA_TYPE_JPEG:
            vjpegLock();
            {
            	JPEGDECINFO_T *pDecInfo = pArg->extradata;
            	
                pEncJpeg = vmp4decGetBuffer();
                
                if(pEncJpeg == NULL)
                {
                	printf("iothread_Write(): can't get jpeg buffer\n");
   	                vjpegUnlock ();
					cyg_mutex_unlock(&g_IOThread.cygMtWrite);
                	return FALSE;
                }
                
            	vjpegDecSetHW(pDecInfo->uPixels, pDecInfo->uLines);
				if(mpLength > sizeof(pEncJpeg->aucBitstream))
				{
					diag_printf("iothread_Write(): jpeg data too large\n");
   	                vjpegUnlock ();
					cyg_mutex_unlock(&g_IOThread.cygMtWrite);
					return FALSE;
				}
                memcpy ((char*) NON_CACHE (pEncJpeg->aucBitstream), (char*)  (mpData), mpLength);
                
#ifdef INTERFACES_DEBUG
            	printf("iothread_Write(): jpeg to decode\n");
#endif
                eDecodeResult = decodeJPEGPrimaryImage ((void*)pEncJpeg);
#ifdef INTERFACES_DEBUG
            	printf("iothread_Write(): jpeg decode ok\n");
#endif
                if (eDecodeResult == VP_JPEG_DEC_OK)
                {
					void vdUpdateRemote (void*);
					vdUpdateRemote ((void*)1);
                }
                vjpegUnlock ();
			}
			break;
		default:
			printf("iothread_Write(): media type%d not supported\n", mpType);
			cyg_mutex_unlock(&g_IOThread.cygMtWrite);
			return FALSE;
	}
	
	cyg_mutex_unlock(&g_IOThread.cygMtWrite);
	return TRUE;
}

/*
 * function:
 *		Set notification music to be played.
 *		You can switch music at any time, don't need to wait for last music completed.
 *		But should iothread_WriteNotification(NULL, 0, NULL) before reusing the last buffer.
 * arguments:
 *		pucBuffer		The music data buffer
 *		iLen			The music data length
 *		callback		The callback function when music playing completed.
 *						If no callback, set NULL
 */
int iothread_WriteNotification(UCHAR *pucBuffer, INT32 iLen, void(*callback)(void))
{
	vauNotificationPlay(pucBuffer, iLen, callback);
	return iLen;
}



__inline void COPY_OSD_DATA(char * cpucBuffer,UINT32 uLength,
	UINT32 uPixelWidth, UINT32 usScreenWidth, UINT32 usScreenHeight,
	int nLeft, int nTop, int nRight, int nBottom, int nWidth, int nHeight,
	int *nX,
	int *nY,
	UCHAR **ppucOSDBuffer
)
{
	UINT32 u;
	CONST UCHAR *alias_cpucBuffer = (CONST UCHAR *) (cpucBuffer);
	UINT32 alias_uLength = (UINT32) uLength;

	for (u = 0; u < alias_uLength; u += uPixelWidth)
	{
		if (*nX >= 0 && *nX < (int) usScreenWidth
			&& *nY >= 0 && *nY < (int) usScreenHeight)
			memcpy (*ppucOSDBuffer, alias_cpucBuffer, uPixelWidth);

		(*nX)++;
		*ppucOSDBuffer += uPixelWidth;
		if (*nX >= nRight)
		{
			*nX = nLeft;
			(*nY)++;

			*ppucOSDBuffer += ((int) usScreenWidth - nWidth) * uPixelWidth;
		}

		alias_cpucBuffer += uPixelWidth;
	}
}

BOOL wb702WriteOSD (int nLeft, int nTop, int nWidth, int nHeight, CONST UCHAR *cpucData, UINT32 uDataLen)
{

	VP_BUFFER_OSD_T *pOSDBuffer;
	VP_BUFFER_OSD_T *pOSDBuffer_old;
	UCHAR *pucOSDBuffer;
	USHORT usScreenWidth;
	USHORT usScreenHeight;
	UINT32 uPixelWidth;
	int nX;
	int nY;
	int nRight;
	int nBottom;
	
	
	if (vlcmIsConfigured () == FALSE)
	{
		printf("wbvWriteOSD(): LCM hasn't been configured\n");
		return FALSE;
	}

	
#ifdef INTERFACES_DEBUG
	printf ("wbvWriteOSD(): %d %d %d %d\n", nLeft, nTop, nWidth, nHeight);
#endif
	
	/* <3> Copy OSD data */
	pOSDBuffer = bufOSDNew ();
	if (pOSDBuffer == NULL)
	{
		printf("wbvWriteOSD(): Get OSD buffer failed\n");
		return FALSE;
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

	COPY_OSD_DATA ((char*)cpucData, uDataLen,
		uPixelWidth, usScreenWidth, usScreenHeight,
		nLeft, nTop, nRight, nBottom, nWidth, nHeight,
		&nX, &nY, &pucOSDBuffer
		);
	
#ifdef INTERFACES_DEBUG
	printf ("wbvWriteOSD(): Read OSD OK\n");
#endif

	pOSDBuffer_old = vlcmSetOSDBuffer (pOSDBuffer);
	bufOSDDecRef (pOSDBuffer_old);
	
	vlcmStartRefresh (FALSE);
	
#ifdef INTERFACES_DEBUG
	printf ("wbvWriteOSD(): Refresh .... ok\n");
#endif
	
	return TRUE;	
}

int wb702EnableMP4Encoder(BOOL bEnable)
{
	static BOOL bPrevousStatus = FALSE;
	
	cyg_mutex_lock(&g_IOThread.cygMtMP4Enable);
	if (bPrevousStatus != bEnable)
	{
		bPrevousStatus = bEnable;
	
		if (bEnable == TRUE)
		{
			VP_VIDEO_T *pVideo;
			
			//cyg_mutex_unlock(&g_IOThread.cygMtMP4);
			
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
			//cyg_mutex_lock(&g_IOThread.cygMtMP4);
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
	cyg_mutex_unlock(&g_IOThread.cygMtMP4Enable);
	return TRUE;
}

int wb702EnableAudioEncoder(BOOL bEnable)
{
	tt_rmutex_lock(&g_IOThread.cygMtAudioEnable);
	if (g_IOThread.bPrevousAudioStatus != bEnable)
	{
		g_IOThread.bPrevousAudioStatus = bEnable;
        if (bEnable == TRUE)
        {
			//cyg_mutex_unlock(&g_IOThread.cygMtAudio);
            vauStartRecord ();
        }
        else
        {
			//cyg_mutex_lock(&g_IOThread.cygMtAudio);
            vauStopRecord ();
            
            /* Clear encoded buffer. */
            while (1)
            {
                VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *pBitstream;
                
                vauLock ();
                pBitstream = vauGetEncBuffer_Local ();
                if (pBitstream != NULL)
                {
                    listDetach (&pBitstream->list);
                    bufEncAudioDecRef (pBitstream);
                }
                vauUnlock ();
                
                if (pBitstream == NULL)
                    break;
            }
        }
	}
	tt_rmutex_unlock(&g_IOThread.cygMtAudioEnable);
	return TRUE;
}


BOOL wb702LockAndDisableAudio()
{
	vauStopRecord();
	vauStopPlay();
	return TRUE;
}

void wb702UnlockAndRestorePreviousAudio(BOOL bPreviousState)
{
	audio_WM8978_test();
	vauStartPlay();
	vauStartRecord();
}



void wb702EnableJPEGEncoder(BOOL bEnable)
{
	static BOOL bPrevousStatus = FALSE;
	
	cyg_mutex_lock(&g_IOThread.cygMtJpegEnable);
	if (bPrevousStatus != bEnable)
	{
		bPrevousStatus = bEnable;
		
		if (bEnable == TRUE)
		{
			//cyg_mutex_unlock(&g_IOThread.cygMtJpeg);
			
			vdLock ();
			vjpegLock();
		
			TURN_ON_JPEG_CLOCK;
			tt_msleep (30);

			vdEnableLocalJPEG (bEnable);
			vjpegEncInit();
			vjpegDecInit();
	        jpegInitializeEngine();
	        vjpegUnlock ();
   			vdUnlock ();
        
			/* Start capture. */
			vcptStart ();
		}
		else
		{
			//cyg_mutex_lock(&g_IOThread.cygMtJpeg);
			/* Stop capture. */
			vcptStop ();
			
			vjpegLock();
			vdEnableLocalJPEG (bEnable);
			TURN_OFF_JPEG_CLOCK;
			
			vjpegClearBuffer();
			vjpegUnlock ();	
		}

   	}
	cyg_mutex_unlock(&g_IOThread.cygMtJpegEnable);
}

void wb702EnableFMI(BOOL bEnable)
{
	static BOOL bPrevousStatus = FALSE;
	
	if (bPrevousStatus != bEnable)
	{
		bPrevousStatus = bEnable;
		
		if (bEnable == TRUE)
		{
			TURN_ON_FMI_CLOCK;
		}
		else
		{
			TURN_OFF_FMI_CLOCK;
		}

   	}
}

BOOL wb702_ioctl (unsigned int cmd, unsigned long arg)
{
	static BOOL bPrevousStatus_OSD = FALSE;
	static BOOL bPrevousStatus_Local = FALSE;
	static BOOL bPrevousStatus_Remote = FALSE;
	
	CONFIGURE_LCM_t	config_lcm;
	DISPLAY_LCM_t		disp_lcm;
	FILL_OSD_t			fill_osd;
	OSD_COLOR_t		osd_color;
	SET_LOCAL_VIDEO_SOURCE_t	video_src;
	SET_VIDEO_WIN_t	video_win;
	WB_VERSION_t		Ver_t;
	WB_VERSION_Ext_t   VerExt_t;
	UCHAR		array[64];
	PWb_Tx_t	posd,parg;
	WRITE_OSD_t osd;
	WB_MOTION_DETECT_t * pmotion_t;
	AUDIO_FORMAT_T *paudiotype;
	UCHAR		*ptr ;
	int ret = false;
	
	UINT uStatus;
	UCHAR ucTmp;
	int iStatus;
	
	cyg_mutex_lock(&g_IOThread.cygMtIoctl);
	switch(cmd)
	{
	case WB702CONFIGLCM:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl]  WB702CONFIGLCM  \n");
#endif
		memcpy(&config_lcm,(void*)arg, sizeof(CONFIGURE_LCM_t));
		
		ret = vlcmConfigure (config_lcm.usWidth,
				config_lcm.usHeight,
				config_lcm.eLcmColor,
				config_lcm.eCmdBus,
				config_lcm.eDataBus,
				config_lcm.iMPU_Cmd_RS_pin,
				config_lcm.b16t18,
				config_lcm.eMpuMode);
		
		if(ret == 0) ret = true;
		else ret = false;
		
		break;
		
	case WB702ENABLEDISP:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl]  WB702ENABLEDISP  \n");
#endif

		if (vlcmIsConfigured () == FALSE)
		{
			printf("LCM hasn't been configured\n");
			ret = false;
			break;
		}
		
		memcpy(&disp_lcm, (void*)arg, sizeof(DISPLAY_LCM_t));
			
		switch (disp_lcm.eDisplayType)
		{
			case CMD_DISPLAY_OSD:
			{
				if (bPrevousStatus_OSD != disp_lcm.bEnable)
				{
					bPrevousStatus_OSD = disp_lcm.bEnable;
				
					vlcmLock ();
					if (disp_lcm.bEnable == TRUE)
						vpostOSD_Enable ();
					else
						vpostOSD_Disable ();
					vlcmUnlock ();
					vlcmStartRefresh (FALSE);
				}
				ret = true;
				break;
			}
			case CMD_DISPLAY_LOCAL_VIDEO:
			{
				if (bPrevousStatus_Local != disp_lcm.bEnable)
				{
					bPrevousStatus_Local = disp_lcm.bEnable;

					/*
					if (disp_lcm.bEnable)
						vcptStart ();
					else
						vcptStop ();
					*/

					vdEnableLocalWindow (disp_lcm.bEnable);
					vcptWaitPrevMsg ();
				}
				ret = true;
				break;
			}
			case CMD_DISPLAY_REMOTE_VIDEO:
			{
				if (bPrevousStatus_Remote != disp_lcm.bEnable)
				{
					VP_VIDEO_T *pVideo;
					bPrevousStatus_Remote = disp_lcm.bEnable;
					
					vdLock ();
					pVideo = vdGetSettings ();
			
					vmp4Lock ();
					if (disp_lcm.bEnable == TRUE)
					{
						vmp4decAddRef (pVideo);
						vdEnableRemoteWindow (disp_lcm.bEnable);
					}
					else
					{
						vdEnableRemoteWindow (disp_lcm.bEnable);
						vmp4decDecRef ();
					}
					vmp4Unlock ();

					vdUnlock ();
				
					vcptWaitPrevMsg ();
				}
				ret = true;
				break;
			}
			default:
				ret = false;
				break;
		}

		break;
		
	case WB702SETOSDCOL:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl]  WB702SETOSDCOL  \n");
#endif

		memcpy(&osd_color, (void*)arg, sizeof(OSD_COLOR_t));
		
		ret = vlcmSetOSDColorMode(osd_color.eOsdColor,
						osd_color.ucKeyColor_R,
						osd_color.ucKeyColor_G,
						osd_color.ucKeyColor_B,
						osd_color.ucKeyMask_R,
						osd_color.ucKeyMask_G,
						osd_color.ucKeyMask_B);
		if(ret == 0) ret = true;
		else ret = false;
		
		break;
		
	case WB702FILLOSD:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl] WB702FILLOSD \n ");
#endif

		if (vlcmIsConfigured () == FALSE)
		{
			printf("LCM hasn't been configured\n");
			ret = false;
			break;
		}
		
		memcpy(&fill_osd, (void*)arg, sizeof(FILL_OSD_t));
		
		if (fill_osd.bFillBackground == TRUE)
			vlcmFillLCMBuffer (fill_osd.ucR, fill_osd.ucG, fill_osd.ucB);
		else
			vlcmFillOSDBuffer (fill_osd.ucR, fill_osd.ucG, fill_osd.ucB);

		vlcmStartRefresh (FALSE);
		ret = true;
		
		break;
		
	case WB702SETOSD:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl] WB702SETOSD \n ");
#endif
		memcpy(array,(UCHAR *)arg,sizeof(Wb_Tx_t));
		
		posd = (PWb_Tx_t)array;

		ptr = posd->txbuf;
		
		osd.sLeft = *ptr;		ptr +=2;
		osd.sTop = *ptr;		ptr +=2;
		osd.usWidth = *ptr;	ptr +=2;
		osd.usHeight = *ptr;	ptr +=2;
		ptr +=4;		osd.cpucData = ptr;

		ret = wb702WriteOSD(osd.sLeft, osd.sTop, osd.usWidth, osd.usHeight, osd.cpucData, (osd.usWidth*osd.usHeight*2));
		break;
		
	case WB702SETLOCALVIDEOSRC:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl] WB702SETLOCALVIDEOSRC \n ");
#endif

		memcpy(&video_src,(void*)arg,sizeof(SET_LOCAL_VIDEO_SOURCE_t));

		ret = vcptSetWindow(video_src.usWidth,video_src.usHeight,video_src.eRotate);
		if(ret == 0) ret = true;
		else ret = false;
		
		break;
		
	case WB702SETLOCALVIDEOWIN:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl] WB702SETLOCALVIDEOWIN \n ");
#endif

		memcpy(&video_win,(void*)arg,sizeof(SET_VIDEO_WIN_t));

		{
			VP_POINT_T		tDisplayPos;
			VP_RECT_T		tClippedWindow;
			CMD_ROTATE_E	eRotate;
			
			tDisplayPos.sLeft				= video_win.sLeft;
			tDisplayPos.sTop				= video_win.sTop;
			tClippedWindow.tSize.usWidth	= video_win.usWidth;
			tClippedWindow.tSize.usHeight	= video_win.usHeight;
			eRotate							= (CMD_ROTATE_E) video_win.eRotate;
			ret = vdSetLocalWindow (tDisplayPos, tClippedWindow.tSize, eRotate);
			vcptWaitPrevMsg ();
		}

		break;
		
	case WB702SETREMOTEVIDEOWIN:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl] WB702SETREMOTEVIDEOWIN \n ");
#endif

		memcpy(&video_win,(void*)arg,sizeof(SET_VIDEO_WIN_t));

		{
			VP_POINT_T tDisplayPos;
			VP_RECT_T tClippedWindow;
			CMD_ROTATE_E	eRotate;
			
			tDisplayPos.sLeft				= video_win.sLeft;
			tDisplayPos.sTop				= video_win.sTop;
			tClippedWindow.tSize.usWidth	= video_win.usWidth;
			tClippedWindow.tSize.usHeight	= video_win.usHeight;
			eRotate							= (CMD_ROTATE_E) video_win.eRotate;
			ret = vdSetRemoteWindow (tDisplayPos, tClippedWindow.tSize, eRotate);
		}
		
		break;
		
	case WB702SETVIDEOFORMAT:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl] WB702SETVIDEOFORMAT  \n");
#endif
		
		memcpy(&iStatus,(void*)arg,sizeof(int));
		
		if(iStatus == CMD_VIDEO_MP4)
			vmp4encSetFormat(CMD_VIDEO_MP4);
		else
			vmp4encSetFormat(CMD_VIDEO_H263);
		ret = true;

		break;
		
	case WB702ENALBEMP4ENCODER:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl] WB702ENALBEMP4ENCODER  %s \n",*(int *)arg ? "Enable" : "Disable");
#endif

		memcpy(&iStatus,(void*)arg,sizeof(int));
		
		wb702EnableMP4Encoder(iStatus);
		ret = true;

		break;
		
	case WB702ENALBEAUDIOENCODER:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl] WB702ENALBEAUDIOENCODER  %s \n",*(int *)arg ? "Enable" : "Disable");
#endif

		memcpy(&iStatus,(void*)arg,sizeof(int));
		
		wb702EnableAudioEncoder(iStatus);
		ret = true;
		
		break;
		
	case WB702INDEX_Z:	//local on top
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl] WB702INDEX_Z \n");
#endif

		memcpy(&iStatus,(void*)arg,sizeof(int));
		
		vdSetZIndex (iStatus);
		vcptWaitPrevMsg ();
		ret = true;

		break;
			
	case WB702ENABLEENCODER:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl] (1) WB702ENABLEENCODER  \n");
#endif

		memcpy(&iStatus,(void*)arg,sizeof(int));

		if (iStatus)
		{
			//atomic_set(&adapter->resetflg,1);

			//wb_reset_queue(adapter);
			//wb702_enable_int();			
			
			//if(adapter->videoenable)
				wb702EnableMP4Encoder (TRUE);
			
			//if(adapter->audioenable)
				wb702EnableAudioEncoder (TRUE);
			
			//ret  = wbvEnableCommandInterrupt (FALSE, TRUE);
		}
		else
		{
				wb702EnableMP4Encoder (FALSE);
				wb702EnableAudioEncoder (FALSE);
		
		}
		ret = true;
		
		break;
	
	case WB702SETVIDEOBITRATE:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl]  WB702SETVIDEOBITRATE  \n");
#endif

		memcpy(&uStatus,(void*)arg,sizeof(int));
		
		{
			UINT32 uFrameRate;
			vmp4encGetQuality (&uFrameRate, NULL);
			ret = vmp4encSetQuality (uFrameRate, uStatus);
			if(ret == 0) ret =true;
			else ret = false;
		}
		
		break;
		
	case WB702GETFIRMARENUM:
		{
			const char *pc = g_apcRevision[1];	//Change date time
			int nYear;
			int nMonth;
			int nDay;
			int nHour;
			int nMin;
		
			pc = strstr (pc, "$Date:");

			/* Parse date string. */
			if (pc != NULL && (pc = strpbrk (pc, " :-/")) != NULL)
				nYear = atoi (pc += strspn (pc, " :-/")) - 1970;	
			else
				nYear = 0;
			
			if (pc != NULL && (pc = strpbrk (pc, " :-/")) != NULL)
				nMonth = atoi (pc += strspn (pc, " :-/"));
			else
				nMonth = 0;
			
			if (pc != NULL && (pc = strpbrk (pc, " :-/")) != NULL)
				nDay = atoi (pc += strspn (pc, " :-/"));
			else
				nDay = 0;
				
			/* Parse time string. */
			if (pc != NULL && (pc = strpbrk (pc, " :-/")) != NULL)
				nHour = atoi (pc += strspn (pc, " :-/"));
			else
				nHour = 0;
			
			if (pc != NULL && (pc = strpbrk (pc, " :-/")) != NULL)
				nMin = atoi (pc += strspn (pc, " :-/"));
			else
				nMin = 0;

#ifdef INTERFACES_DEBUG
			printf ("Ver: [%s][%s]%d %d %d %d %d\n", __DATE__, __TIME__,
				nYear, nMonth, nDay, nHour, nMin);
#endif
			Ver_t.Month = nMonth;
			Ver_t.Day = nDay;
			Ver_t.Hour = nHour;
			Ver_t.Min = nMin;
			memcpy((void *)arg,(UCHAR *)&Ver_t,sizeof(WB_VERSION_t));
		}
		ret = true;
		break;
		
	case WB702GETFIRMARESTR:
		{
			/* g_apcRevision[0]~[2] is revision, changedate and builddate */
			int iLen, i;
			for(i = 0; i < 3; i++)
			{
				iLen = strlen(g_apcRevision[i]) + 1;
				if(iLen > 32) iLen = 32;
				switch (i)
				{
					case 0:
						memcpy(VerExt_t.version, g_apcRevision[i], iLen);
						break;
					case 1:
						memcpy(VerExt_t.changedate, g_apcRevision[i], iLen);
						break;
					case 2:
						memcpy(VerExt_t.builddate, g_apcRevision[i], iLen);
						break;
					default:
						break;
				}
			}
			
			memcpy((void *)arg,(UCHAR *)&VerExt_t,sizeof(WB_VERSION_Ext_t));
		}
		ret = true;
		break;

	case WB702GETAUDIOQUALITY:
		ucTmp = vauGetQualityLevel ();
		memcpy((void *)arg,(UCHAR *)&ucTmp,sizeof(UCHAR));
		ret = true;
		
		break;
		 
	case WB702SETAUDIOQUALITY:
		memcpy(&ucTmp,(UCHAR *)arg,sizeof(UCHAR));
		
		ret = vauSetQualityLevel ((int)ucTmp);
		if(ret == 0) ret =true;
		else ret = false;
		
		break;
		
	case WB702GETVIDEOBITRATE:
		ret = vmp4encGetQuality (NULL, (UINT32 *)&iStatus);
		if(ret == 0) ret =true;
		else ret = false;
		
		if(ret == true) memcpy((void *)arg,(UCHAR *)&iStatus,sizeof(int));
		
		break;
		
	case WB702ENABLEMOTIONDETECT:
		memcpy(array,(UCHAR *)arg,sizeof(WB_MOTION_DETECT_t));
		pmotion_t = (WB_MOTION_DETECT_t *)array;
		
		{
			BOOL bEnable = (pmotion_t->flags == 0 ? FALSE : TRUE);
			CMD_MOTION_DETECT_SENSIBILITY_E eSen1 = (CMD_MOTION_DETECT_SENSIBILITY_E) pmotion_t->type;
			IMG_CMP_SENSIBILITY_E eSen2;

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
		}
		ret = true;
		
		break;
		
	case WB702GETMOTIONSNUM:
		uStatus = vdGetMotionsNum ();
		memcpy((void *)arg,(UCHAR *)&uStatus,sizeof(int));
		ret = true;
		
		break;
		
	case WB702SETAUDIOTYPE:
		memcpy(array,(UCHAR *)arg,sizeof(AUDIO_FORMAT_T));
		
		paudiotype = (AUDIO_FORMAT_T*)array;
		
        printf("audio format: encode(%d) decode(%d)\n", paudiotype->eEncodeFormat, paudiotype->eDecodeFormat);
        vauSetFormat (paudiotype->eEncodeFormat, paudiotype->eDecodeFormat);
		ret = true;
		
		break;
		
	case WB702SETBITMAP:	//need modify to wb702WriteOSD??
	
		memcpy(array,(UCHAR *)arg,sizeof(Wb_Tx_t));
		
		parg = (PWb_Tx_t)array;
		ptr = parg->txbuf;
		
		//ret = lcmbpBitBltRGB565(parg->mediatype,ptr,parg->txlen);
//		ret = wb702WriteOSD(0, 0, 128, 160, ptr, (128*160*2));
		ret = wb702WriteOSD(0, 0, 320, 240, ptr, (320*240*2));
		
		break;
	case WB702ENALBEMP4IFRAME:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl] force mp4 I frame. num : %d\n",(*(int *)arg));
#endif
	{
		UINT32 uMillisecond = (*(int *)arg) & 0xFFFF0000;
		UINT32 uNum = (*(int *)arg) & 0x0000FFFF;
		
		//cyg_mutex_lock(&g_IOThread.cygMtMP4);
		/* Clear encoded buffer. */
		vmp4Lock ();
		//vmp4ClearBuffer ();
		//Do not clear buffer! otherwise the header may be cleared.
		vmp4ForceIFrame (uNum, uMillisecond);
		vmp4Unlock ();

		//cyg_mutex_unlock(&g_IOThread.cygMtMP4);
		ret = true;
		break;
	}
		
	case WB702STRETCHVIDEO:
		{
			UINT8 ucVaScaleV_Round;
			UINT8 ucVaScaleH_Round;
			UINT16 usVaScaleV;
			UINT16 usVaScaleH;
			
			struct stretch_video scale;

			memcpy(&scale,(struct stretch_video *)arg,sizeof(struct stretch_video));
			
	#ifdef INTERFACES_DEBUG
			printf("[wb_ioctl] stretch video scale: v%d - h%d \n",scale.scale_V,scale.scale_H);
	#endif
			
			usVaScaleV = scale.scale_V;
			usVaScaleH = scale.scale_H;
			
			if (usVaScaleV < (UINT16) 100
				|| usVaScaleV >= (UINT16) 800
				|| usVaScaleH < (UINT16) 100
				|| usVaScaleH >= (UINT16) 800)
			{
				ret = false;
				break;
			}
				
			ucVaScaleV_Round = (UINT8) (usVaScaleV / 100);
			usVaScaleV = (UINT16) ((usVaScaleV - ucVaScaleV_Round * 100UL) * 1024UL / 100UL);
			ucVaScaleH_Round = (UINT8) (usVaScaleH / 100);
			usVaScaleH = (UINT16) ((usVaScaleH - ucVaScaleH_Round * 100UL) * 1024UL / 100UL);
			

			vlcmLock ();
			vpostVA_Stream_Scaling_Ctrl (ucVaScaleV_Round, usVaScaleV,
				ucVaScaleH_Round, usVaScaleH,
				SCALE_MODE_INTERPOLATION);
			vlcmUnlock ();
			
			ret = true;
		}
		break;
		
	case WB702SETJPEG:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl]  WB702SETJPEG  \n");
#endif
		wb702EnableJPEGEncoder(*(BOOL *)arg);
		ret = true;
		
		break;
	
	case WB702SETDATETIME:
		{
			CMD_TM_T *tm = (CMD_TM_T *)arg;
			struct tm tms;
	#ifdef INTERFACES_DEBUG
			printf("[wb_ioctl]  WB702SETDATETIME  \n");
	#endif

			tms.tm_year	= tm->usYear;
			tms.tm_mon	= tm->ucMon;
			tms.tm_mday	= tm->ucMDay;
			tms.tm_hour	= tm->ucHour;
			tms.tm_min	= tm->ucMin;
			tms.tm_sec	= tm->ucSec;
			
			if (tms.tm_year >= 0 && tms.tm_year <= 9999 - 1900
				&& tms.tm_mon >= 0 && tms.tm_mon <= 11
				&& tms.tm_mday >= 1 && tms.tm_mday <= 31
				&& tms.tm_hour >= 0 && tms.tm_hour <= 23
				&& tms.tm_min >= 0 && tms.tm_min <= 59
				&& tms.tm_sec >= 0 && tms.tm_sec <= 59)
			{
				vinfoSetTime (&tms);
				ret = true;
			}
			else
			{
				ret = false;
			}	
			
			break;
		}
	case WB702GETDATETIME:
		{
			CMD_TM_T *tm = (CMD_TM_T *)arg;
			struct tm tms;
	#ifdef INTERFACES_DEBUG
			printf("[wb_ioctl]  WB702GETDATETIME  \n");
	#endif
			
			tm->usYear	= tms.tm_year;
			tm->ucMon	= tms.tm_mon;
			tm->ucMDay	= tms.tm_mday;
			tm->ucHour	= tms.tm_hour;
			tm->ucMin	= tms.tm_min;
			tm->ucSec	= tms.tm_sec;
			
			ret = true;
			break;
		}
	case WB702SUSPENDFW:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl]  WB702SUSPENDFW  \n");
#endif
		/* Stop audio. */
		vauUninit ();
		/* Stop catpure. */
		vcptUninitThread ();
		/* Stop LCM */
		
		/* Stop MP4 */
		
		/* Stop VPE */
		vvpeUninit();

		sysDisableIRQ ();
		//sysStopTimer (TIMER0);
		
		outpw (REG_CLKCON, inpw (REG_CLKCON) | CPUCLK_EN | APBCLK_EN | HCLK2_EN);
		outpw (REG_CLKSEL, SYSTEM_CLK_FROM_XTAL);
		outpw (REG_APLLCON, PLL_OUT_OFF);
		outpw (REG_UPLLCON, PLL_OUT_OFF);
		sysEnableIRQ ();
		ret = true;
		
		break;
	
	case OLDWB702GETFWVERSION:
		{
			const char *pc = g_apcRevision[0];	//SVN version number.
			int nVer;
			unsigned char ver[4];
			
			pc = strstr (pc, "$Revision:");
			
			/* Parse version number. */
			if (pc != NULL && (pc = strpbrk (pc, " :-/")) != NULL)
				nVer = atoi (pc += strspn (pc, " :-/"));
			else
				nVer = 0;
				
#ifdef INTERFACES_DEBUG
			printf ("Ver: %d\n", nVer);
#endif
			
			memset(ver, 0, 4);
			ver[0] = (UCHAR) (nVer / 256);
			ver[2] = (UCHAR) (nVer % 256);
			
			memcpy((void *)arg,ver,4);
			
		}
		ret = true;
		
		break;
		
	case OLDWB702CFGLCM:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl]  OLDWB702CFGLCM  \n");
#endif
		{
			struct {
				WB_LCM_NUMBER_E eNumber;
				WB_LCM_BUS_WIDTH_E eBus;
				WB_LCM_COLOR_WIDTH_E eColor;
				}cfglcm;
			CMD_LCM_COLOR_WIDTH_E eLcmColor;
			CMD_LCM_CMD_BUS_WIDTH_E eCmdBus;
			
			memcpy((void *)&cfglcm,(void *)arg,sizeof(cfglcm));
			
			switch ((WB_LCM_COLOR_WIDTH_E) cfglcm.eColor)
			{
				case WB_LCM_COLOR_WIDTH_16:
					eLcmColor = CMD_LCM_COLOR_WIDTH_16;
					break;
				case WB_LCM_COLOR_WIDTH_18:
					eLcmColor = CMD_LCM_COLOR_WIDTH_18;
					break;
				case WB_LCM_COLOR_WIDTH_12:
					eLcmColor = CMD_LCM_COLOR_WIDTH_12;
					break;
				case WB_LCM_COLOR_WIDTH_24:
					eLcmColor = CMD_LCM_COLOR_WIDTH_24;
					break;
				default:
					cyg_mutex_unlock(&g_IOThread.cygMtIoctl);
					return false;
			}

			switch ((WB_LCM_BUS_WIDTH_E) cfglcm.eBus)
			{
				case WB_LCM_BUS_WIDTH_16_18:
					eCmdBus = CMD_LCM_CMD_BUS_WIDTH_16;
					break;
				case WB_LCM_BUS_WIDTH_8_9:
					eCmdBus = CMD_LCM_CMD_BUS_WIDTH_8;
					break;
				default:
					cyg_mutex_unlock(&g_IOThread.cygMtIoctl);
					return false;
			}

			ret = vlcmConfigure (240, 320,
					eLcmColor,
					eCmdBus,
					CMD_LCM_DATA_BUS_WIDTH_16,
					0,
					FALSE,
					CMD_LCM_80_MPU);
			if(ret == 0)
				ret = true;
			else
			{
				printf("old configure lcm failure\n");
				ret = false;
			}
		}
		break;
		
	case OLDWB702ENABLEOSD:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl]  OLDWB702ENABLEOSD  \n");
#endif

		if (!vlcmIsConfigured ())
		{
			printf("LCM hasn't been configured\n");
			ret = false;
			break;
		}
		
		iStatus = arg;
			
		if (bPrevousStatus_OSD !=iStatus)
		{
			bPrevousStatus_OSD = iStatus;
		
			vlcmLock ();
			if (iStatus)
				vpostOSD_Enable ();
			else
				vpostOSD_Disable ();
			vlcmUnlock ();
			vlcmStartRefresh (FALSE);
		}
		ret = true;
		break;
		
	case OLDWB702SETOSDKEYMASK:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl]  OLDWB702SETOSDKEYMASK  \n");
#endif
		{
			CMD_LCM_OSD_COLOR_E eOsdColor;
			UCHAR ucKeyColor_R;
			UCHAR ucKeyColor_G;
			UCHAR ucKeyColor_B;
			UCHAR ucKeyMask_R;
			UCHAR ucKeyMask_G;
			UCHAR ucKeyMask_B;
			
			unsigned char mask[3];
			memcpy(mask,(unsigned char *)arg,3);
			
			vlcmGetOSDColorMode (&eOsdColor,
				&ucKeyColor_R,
				&ucKeyColor_G,
				&ucKeyColor_B,
				&ucKeyMask_R,
				&ucKeyMask_G,
				&ucKeyMask_B);

			/* Set OSD key mask */
			ucKeyMask_R = mask[0];
			ucKeyMask_G = mask[1];
			ucKeyMask_B = mask[2];
			
			ret = vlcmSetOSDColorMode (eOsdColor,
				ucKeyColor_R,
				ucKeyColor_G,
				ucKeyColor_B,
				ucKeyMask_R,
				ucKeyMask_G,
				ucKeyMask_B);
			
			if(ret == 0) ret = true;
			else
			{
				printf("old set osd mask failure\n");
				ret = false;
			}
		}
		break;
		
	case OLDWB702SNDPATTERN:	
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl]  OLDWB702SNDPATTERN  \n");
#endif
		memcpy(array,(UCHAR *)arg,sizeof(Wb_Tx_t));
		
		parg = (PWb_Tx_t)array;
		if(parg->txlen <= 0)
		{
			ret = false;
			break;
		}
		
		{
			static UINT32 uAddress = 0;
			VP_BUFFER_OSD_T *pOSDBuffer;
			UINT32 uLength = parg->txlen;
			
			uAddress = parg->mediatype;
			
			if (vlcmIsConfigured () == FALSE)
			{
				printf("LCM hasn't been configured\n");
				ret = false;
				break;		
			}

			vlcmLock ();
			pOSDBuffer = vlcmGetOSDBuffer ();
			if (uAddress + uLength <= sizeof (pOSDBuffer->aucData))
			{
				memcpy (pOSDBuffer->aucData, parg->txbuf, uLength);
			}
			vlcmUnlock ();
			
			ret = true;
		}
		break;
		
	case OLDWB702OSDFLUSH:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl]  OLDWB702OSDFLUSH  \n");
#endif
		vlcmStartRefresh (FALSE);
		ret = true;

		break;
		
	case WB702SETVIDEOFRAMERATE:
		memcpy(&uStatus,(void*)arg,sizeof(int));
		
		{
			UINT32 uBitRate;
			vmp4encGetQuality (NULL, &uBitRate);
			ret = vmp4encSetQuality (uStatus, uBitRate);
			if(ret == 0) ret =true;
			else ret = false;
		}
		break;
	case WB702GETVIDEOFRAMERATE:
		ret = vmp4encGetQuality ((UINT32 *)&iStatus, NULL);
		if(ret == 0) ret =true;
		else ret = false;
		
		if(ret == true) memcpy((void *)arg,(UCHAR *)&iStatus,sizeof(int));
		
		break;
	
	case WB702SETJPEGQUALITY:
		memcpy(&uStatus,(void*)arg,sizeof(int));
		
		vjpegSetQuality(uStatus);
		break;
		
	case WB702GETJPEGQUALITY:
		iStatus = vjpegGetQuality ();
		
		memcpy((void *)arg,(UCHAR *)&iStatus,sizeof(int));
		
		ret = true;
		break;
	
	case WB702SETVIDEODYNAMICBITRATE:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl]  WB702SETVIDEODYNAMICBITRATE  \n");
#endif

		{
			VP_VIDEO_T *videoSetting = vdGetSettings();
			if(videoSetting->bIsEnableLocalMP4 != TRUE)
			{
				ret = false;
			}
			else
			{
				memcpy(&uStatus,(void*)arg,sizeof(int));
				//diag_printf("to set dynamic bitrate %d*1024!!\n", uStatus/1024);
				//cyg_interrupt_disable();
				//if(mp4SetEncoderBitrate(uStatus, videoSetting->tLocalWin.tSourceSize.usHeight) != 0)
				if(vmp4encSetBitrate(uStatus, videoSetting->tLocalWin.tSourceSize.usHeight) != 0)
				{
					ret = false;
				}
				else
				{
					ret = true;
				}
				//cyg_interrupt_enable();
			}
		}
		
		break;
		
	case WB702SETVIDEODYNAMICFRAMERATE:
#ifdef INTERFACES_DEBUG
		printf("[wb_ioctl]  WB702SETVIDEODYNAMICFRAMERATE  \n");
#endif

		{
			VP_VIDEO_T *videoSetting = vdGetSettings();
			if(videoSetting->bIsEnableLocalMP4 != TRUE)
			{
				ret = false;
			}
			else
			{
				memcpy(&uStatus,(void*)arg,sizeof(int));
				//diag_printf("to set dynamic frame %d!!\n", uStatus);
				//cyg_interrupt_disable();
				//if(mp4SetEncoderFramerate(uStatus, videoSetting->tLocalWin.tSourceSize.usHeight) != 0)
				if(vmp4encSetFramerate(uStatus, videoSetting->tLocalWin.tSourceSize.usHeight) != 0)
				{
					ret = false;
				}
				else
				{
					ret = true;
				}
				//cyg_interrupt_enable();
			}
		}
		
		break;
		
	case WB702SETAUDIOPLAYVOLUME:
		{
			WB_AUDIO_VOLUME audioVolume;
			memcpy((CHAR*)&audioVolume, (CHAR*)arg, sizeof(WB_AUDIO_VOLUME));
			
			ret = vauSetPlayVolume(audioVolume.lvolume, audioVolume.rvolume);
			
			if (ret != -1)
			{
				ret = true;
			}
			else
			{
				ret = false;
			}
			
			break;
		}
	
	case WB702SETAUDIORECORDVOLUME:
		{
			WB_AUDIO_VOLUME audioVolume;
			memcpy((CHAR*)&audioVolume, (CHAR*)arg, sizeof(WB_AUDIO_VOLUME));
			
			ret = vauSetRecordVolume(audioVolume.lvolume, audioVolume.rvolume);
			
			if (ret != -1)
			{
				ret = true;
			}
			else
			{
				ret = false;
			}
			
			break;
		}
	
	case WB702SETAUDIONOTIFICATIONVOLUME:
		{
			INT volume;
			memcpy((CHAR*)&volume, (CHAR*)arg, sizeof(INT));
			
			ret = vauSetNotificationVolume(volume);
			
			if (ret != -1)
			{
				ret = true;
			}
			else
			{
				ret = false;
			}
			
			break;
		}
	
	case WB702SETVIDEOCONTRASTBRIGHT:
		{
			WB_VIDEO_CONTRAST_BRIGHT videocb;
			memcpy((CHAR*)&videocb, (CHAR*)arg, sizeof(WB_VIDEO_CONTRAST_BRIGHT));
			
			vcptSetContrastBright(videocb.contrast, videocb.bright);
			ret = true;
			
			break;
		}
	
	case WB702ENABLEDRAWIMAGETIME:
		{
			BOOL bImageTime;
			memcpy((CHAR*)&bImageTime, (CHAR*)arg, sizeof(BOOL));
			
			vcptEnableDrawImageTime(bImageTime);
			ret = true;
			
			break;
		}
	
	case WB702SETDRAWCONTENT:
		{
			INFO_CONTENT infoContent;
			memcpy(&infoContent, (INFO_CONTENT*)arg, sizeof(INFO_CONTENT));
			
			vinfoSetContent(infoContent.index, infoContent.pcContent, infoContent.x, infoContent.y);
			ret = true;
			
			break;
		}
	
	case WB702CLEARDRAWCONTENT:
		{
			INT index;
			memcpy(&index, (char*)arg, sizeof(INT));
			
			vinfoClearContent(index);
			ret = true;
			
			break;
		}
	
	default:
		printf("[wb_ioctl] not support ioctl command !\n");
	}
	
	cyg_mutex_unlock(&g_IOThread.cygMtIoctl);
	return ret;
}






int wb702ConfigureLCM(USHORT usWidth,		/* LCM width */
						USHORT	 usHeight,	/* LCM height */ 
						CMD_LCM_COLOR_WIDTH_E eLcmColor,
						CMD_LCM_CMD_BUS_WIDTH_E eCmdBus,
						CMD_LCM_DATA_BUS_WIDTH_E eDataBus,
						int iMPU_Cmd_RS_pin,/* 0 or 1 (when send MPU command) */
						BOOL b16t18,			/* Convert 16-bit video to 18-bit? */
						CMD_LCM_MPU_MODE_E eMpuMode)
{
	CONFIGURE_LCM_t arg;
	
	arg.usWidth = usWidth;
	arg.usHeight = usHeight;
	arg.eLcmColor = eLcmColor;
	arg.eCmdBus = eCmdBus;
	arg.eDataBus = eDataBus;
	arg.iMPU_Cmd_RS_pin = iMPU_Cmd_RS_pin;
	arg.b16t18 = b16t18;
	arg.eMpuMode = eMpuMode;
	
	if(wb702_ioctl(WB702CONFIGLCM, (unsigned long)&arg)!=true)
	{
		printf("configure lcm error \n");
		return -1;
	}
	return 0;
}

int wb702EnableDisplay(CMD_DISPLAY_TYPE_E eDisplayType, BOOL bEnable)
{
	DISPLAY_LCM_t arg;
	
	arg.eDisplayType = eDisplayType;
	arg.bEnable = bEnable;
	if(wb702_ioctl(WB702ENABLEDISP, (unsigned long)&arg)!=true)
	{
		printf(" enable display error \n");
		return -1;
	}
	return 0;
}

int wb702SetOSDColor(CMD_LCM_OSD_COLOR_E eOsdColor,
			UCHAR ucKeyColor_R, UCHAR ucKeyColor_G, UCHAR ucKeyColor_B,
			UCHAR ucKeyMask_R,  UCHAR ucKeyMask_G, UCHAR ucKeyMask_B)
{
	OSD_COLOR_t arg;
	
	arg.eOsdColor = eOsdColor;
	arg.ucKeyColor_R = ucKeyColor_R;
	arg.ucKeyColor_G = ucKeyColor_G;
	arg.ucKeyColor_B = ucKeyColor_B;
	arg.ucKeyMask_R = ucKeyMask_R;
	arg.ucKeyMask_G = ucKeyMask_G;
	arg.ucKeyMask_B = ucKeyMask_B;
	
	if(wb702_ioctl(WB702SETOSDCOL, (unsigned long)&arg)!=true)
	{
		printf(" set osd color error \n");
		return -1;
	}
	return 0;
}

int wb702FillOSD(BOOL bFillBackground, UCHAR ucR, UCHAR ucG, UCHAR ucB)
{
	FILL_OSD_t arg;
	
	arg.bFillBackground = bFillBackground;
	arg.ucR = ucR;
	arg.ucG = ucG;
	arg.ucB = ucB;
	
	if(wb702_ioctl(WB702FILLOSD, (unsigned long)&arg)!=true)
	{
		printf(" fill osd error \n");
		return -1;
	}
	return 0;
}

int wb702SetLocalVideoSource(USHORT usWidth, USHORT usHeight, CMD_ROTATE_E eRotate)
{
	SET_LOCAL_VIDEO_SOURCE_t arg;
	
	arg.usWidth = usWidth;
	arg.usHeight = usHeight;
	arg.eRotate = eRotate;
	
	if(wb702_ioctl(WB702SETLOCALVIDEOSRC, (unsigned long)&arg)!=true)
	{
		printf(" set local video source error \n");
		return -1;
	}
	return 0;
}

int wb702OSDDisplay()
{
	return 0;
}

int wb702SetLocalVideoWindow(SHORT sLeft, SHORT sTop, USHORT usWidth, USHORT usHeight, CMD_ROTATE_E eRotate)
{
	SET_VIDEO_WIN_t arg;
	
	arg.sLeft = sLeft;
	arg.sTop = sTop;
	arg.usWidth = usWidth;
	arg.usHeight = usHeight;
	arg.eRotate = eRotate;
	
	if(wb702_ioctl(WB702SETLOCALVIDEOWIN, (unsigned long)&arg)!=true)
	{
		printf(" set local video window error \n");
		return -1;
	}
	return 0;
}

int wb702SetRemoteVideoWindow( SHORT sLeft, SHORT sTop, USHORT usWidth, USHORT usHeight, CMD_ROTATE_E eRotate)
{
	SET_VIDEO_WIN_t arg;
	
	arg.sLeft = sLeft;
	arg.sTop = sTop;
	arg.usWidth = usWidth;
	arg.usHeight = usHeight;
	arg.eRotate = eRotate;
	
	if(wb702_ioctl(WB702SETREMOTEVIDEOWIN, (unsigned long)&arg)!=true)
	{
		printf("set remote video window error !\n");
		return -1;
	}
	return 0;
}

int wb702SetVideoZIndex(BOOL enable)
{
	int var = enable;
	
	if(wb702_ioctl(WB702INDEX_Z, (unsigned long)&var)!=true)
	{
		printf("set video Z index error %d \n",enable);
		return -1;
	}
	return 0;
}

int wb702EnableEncoder(BOOL enable)
{
	int var = enable;
	
	if(wb702_ioctl(WB702ENABLEENCODER, (unsigned long)&var)!=true)
	{
		printf("enalbe encoder error %d \n",enable);
		return -1;
	}
	return 0;
}

int wb702SetFrame(WB_FRAME_TYPE frame)
{
	/* Now just send one frame for each read/write */
	/*
	int var = frame;
	
	if(wb702_ioctl(WB702SETFRAME, (unsigned long)&var)!=true)
	{
		printf("set frame errror %d \n",frame);
		return -1;
	}
	*/
	return 0;
}

int wb702SetVideoFormat(CMD_VIDEO_FORMAT_E format)
{
	int var = format;
	
	if(wb702_ioctl(WB702SETVIDEOFORMAT, (unsigned long)&var)!=true)
	{
		printf("set video format error %d \n",format);
		return -1;
	}
	return 0;
}

int wb702SetOSD(Wb_Tx_t *buf)
{
	int ret = 0;

	ret = wb702_ioctl(WB702SETOSD,(unsigned long)buf) ;
	if(ret != true)
	{
		printf(" set osd error \n");
		return -1;
	}
	return 0;	
}

int wb702GetFirmwareVersion(WB_VERSION_t *version)
{
	int ret = 0;

	ret = wb702_ioctl(WB702GETFIRMARENUM,(unsigned long)version);
	if(ret != true)
	{
		printf(" get firmware version error \n");
		return -1;
	}
	return 0;
}

int wb702GetFirmwareVersionExt(WB_VERSION_Ext_t * version)
{
	int ret = 0;

	ret = wb702_ioctl(WB702GETFIRMARESTR,(unsigned long)version);
	if(ret != true)
	{
		printf(" get firmware version extent error \n");
		return -1;
	}
	return 0;
}

int wb702GetAudioQuality(char * quality)
{
	int ret = 0;

	ret = wb702_ioctl(WB702GETAUDIOQUALITY,(unsigned long)quality);
	if(ret != true)
	{
		printf(" get audio quality error \n");
		return -1;
	}
	return 0;
}

int wb702SetAudioQuality(char * quality)
{
	int ret = 0;

	ret = wb702_ioctl(WB702SETAUDIOQUALITY,(unsigned long)quality);
	if(ret != true)
	{
		printf(" set audio quality error \n");
		return -1;
	}
	return 0;
}

 int wb702GetVideoBitRate(char * bitrate)
 {
	int ret = 0;

	ret = wb702_ioctl(WB702GETVIDEOBITRATE,(unsigned long)bitrate);
	if(ret != true)
	{
		printf(" get video bit rate error \n");
		return -1;
	}
	return 0;
}

 int wb702SetVideoBitRate(UINT  bitrate)
 {
	int ret = 0;
	int var = bitrate;
	
	ret = wb702_ioctl(WB702SETVIDEOBITRATE,(unsigned long)&var);
	if(ret != true)
	{
		printf(" set video bit rate error \n");
		return -1;
	}
	return 0;
}

 //add motion detect command interface 2006.08.01
 int wb702EnableMotionDetect(BOOL enable, CMD_MOTION_DETECT_SENSIBILITY_E type)
 {
 	int ret = 0;
	WB_MOTION_DETECT_t motion;

	motion.flags = enable;
	motion.type = type;
	
 	ret = wb702_ioctl(WB702ENABLEMOTIONDETECT,(unsigned long)&motion);
	if(ret != true)
	{
		printf(" set Motion Detect error \n");
		return -1;
	}
	return 0;
 }

 int wb702GetMotionsNum(UINT * num)
 {
 	int ret = 0;

	ret = wb702_ioctl(WB702GETMOTIONSNUM,(unsigned long)num);
	if(ret != true)
	{
		printf(" get Motion Detect error \n");
		return -1;
	}
	return 0;
 }
int wb702SetAudioType(CMD_AUDIO_FORMAT_E encodeType, CMD_AUDIO_FORMAT_E decodeType)
 {
 	int ret = 0;
	AUDIO_FORMAT_T var;

	var.eEncodeFormat = encodeType;
	var.eDecodeFormat = decodeType;
	ret = wb702_ioctl(WB702SETAUDIOTYPE,(unsigned long)&var);
	if(ret != true)
	{
		printf(" get Motion Detect error \n");
		return -1;
	}
	return 0;
 }

//add bypass mode 2006.10.09
int wb702BypassBitmap(int position, UCHAR *src, int size)
{
	int ret = -1;
	Wb_Tx_t val;

	 val.mediatype = position;
	 val.txlen = size;
	 val.txbuf = src;
	 
	if(src == NULL)
	{
		printf("pointer is NULL \n");
		return ret;
	}
	
	ret = wb702_ioctl(WB702SETBITMAP,(unsigned long)&val);
	if(ret != true)
	{
		printf("bypass write bitmap error \n");
		return -1;
	}
	
	return val.txlen;
}

int wb702OutMP4IFrame(UINT num)
{
	int ret = 0;

	ret = wb702_ioctl(WB702ENALBEMP4IFRAME,(unsigned long)&num);
	if(ret != true)
	{
		printf(" output mp4 I frame error \n");
		return -1;
	}
	return 0;
}

int wb702StretchVideoWin(USHORT scale_v ,USHORT scale_h)
{
	int ret = -1;
	struct stretch_video val;

	 val.scale_V = scale_v;
	 val.scale_H = scale_h;
	
	ret = wb702_ioctl(WB702STRETCHVIDEO,(unsigned long)&val);
	if(ret != true)
	{
		printf("stretch video error \n");
		return -1;
	}
	
	return 0;
}

int wb702SetJPEG(BOOL val)
{
	int ret = 0;

	ret = wb702_ioctl(WB702SETJPEG,(unsigned long)&val);
	if(ret != true)
	{
		printf(" Set jpeg error \n");
		return -1;
	}
	return 0;
}

int wb702SetDateTime(const struct tm *tms)
{
	int ret = 0;
	CMD_TM_T cmd_tm;
	cmd_tm.usYear = tms->tm_year;
	cmd_tm.ucMon = tms->tm_mon;
	cmd_tm.ucMDay = tms->tm_mday;
	cmd_tm.ucHour = tms->tm_hour;
	cmd_tm.ucMin = tms->tm_min;
	cmd_tm.ucSec = tms->tm_sec;

	ret = wb702_ioctl(WB702SETDATETIME,(unsigned long)&cmd_tm);
	if(ret != true)
	{
		printf(" Set date time error \n");
		return -1;
	}
	return 0;
}

int wb702GetDateTime(struct tm *tms)
{
	int ret = 0;
	CMD_TM_T cmd_tm;
	
	ret = wb702_ioctl(WB702GETDATETIME, (unsigned long)&cmd_tm);
	if(ret != true)
	{
		printf(" Get date time error \n");
		return -1;
	}
	
	tms->tm_year	= cmd_tm.usYear;
	tms->tm_mon		= cmd_tm.ucMon;
	tms->tm_mday	= cmd_tm.ucMDay;
	tms->tm_hour	= cmd_tm.ucHour;
	tms->tm_min		= cmd_tm.ucMin;
	tms->tm_sec		= cmd_tm.ucSec;
	
	return 0;
}

int wb702EnableSuspend(WB_SUSPEND_TYPE_E eOption)
{
	int ret = 0;

	ret = wb702_ioctl(WB702SUSPENDFW,(unsigned long)&eOption);
	if(ret != true)
	{
		printf(" enalbe suspend error \n");
		return -1;
	}
	return ret;
}

int wb702OldGetFWVer(short * major, short *minor)
{
	int ret = 0;
	char var[4];
	
	ret = wb702_ioctl(OLDWB702GETFWVERSION,(unsigned long)var);
	if(ret != true)
	{
		printf(" old get firmware version error \n");
		return -1;
	}

	*major = var[0]<<8 |var[1];
	*minor = var[2]<<8 |var[3];
	
	return 0;
}

int wb702OldCfgLCM(WB_LCM_NUMBER_E eNumber,
					WB_LCM_BUS_WIDTH_E eBus,
					WB_LCM_COLOR_WIDTH_E eColor)
{
	int ret = 0;
	struct {
			WB_LCM_NUMBER_E eNumber;
			WB_LCM_BUS_WIDTH_E eBus;
			WB_LCM_COLOR_WIDTH_E eColor;
		}cfglcm;
	
	cfglcm.eNumber = eNumber;
	cfglcm.eBus = eBus;
	cfglcm.eColor = eColor;
	
	ret = wb702_ioctl(OLDWB702CFGLCM,(unsigned long)&cfglcm);
	if(ret != true)
	{
		printf(" old configure lcm error \n");
		return -1;
	}
		
	return ret;
}

int wb702OldEnableOSD(BOOL enable)
{
	int ret = 0;

	ret = wb702_ioctl(OLDWB702ENABLEOSD,enable);
	if(ret != true)
	{
		printf(" old enable OSD error \n");
		return -1;
	}
		
	return ret;

}

int wb702OldSetOSDKeyMask(UCHAR ucR, UCHAR ucG, UCHAR ucB)
{
	int ret = 0;
	unsigned char var[3];

	var[0] = ucR;	var[1] = ucG;	var[2] = ucB;
	
	ret = wb702_ioctl(OLDWB702SETOSDKEYMASK, (unsigned long)var);
	if(ret != true)
	{
		printf(" old set OSD key mask error \n");
		return -1;
	}
		
	return ret;

}

int wb702OldSndOSDPattern(UCHAR * cpucBuffer,
							UINT32 uBufferLength, UINT32 uAddress)
{
	int ret = 0;
	Wb_Tx_t var;

	var.mediatype = uAddress;
	var.txlen = uBufferLength;
	var.txbuf = cpucBuffer;
	 
	ret = wb702_ioctl(OLDWB702SNDPATTERN,(unsigned long)&var);
	if(ret != true)
	{
		printf(" old send OSD pattern error \n");
		return -1;
	}
		
	return ret;

}

int wb702OldOSDFlush(void)
{
	int ret = 0;
	 
	ret = wb702_ioctl(OLDWB702OSDFLUSH,(unsigned long)&ret);
	if(ret != true)
	{
		printf(" old OSD flush error \n");
		return -1;
	}
		
	return ret;

}

int wb702SetVideoFramerate(UINT framerate)
{
	int ret = 0;
	int var = framerate;
	
	ret = wb702_ioctl(WB702SETVIDEOFRAMERATE,(unsigned long)&var);
	if(ret < 0)
	{
		printf(" set video frame rate error \n");
		return -1;
	}
	return ret;
}

int wb702GetVideoFramerate(UINT * framerate)
{
	int ret = 0;
	
	ret = wb702_ioctl(WB702GETVIDEOFRAMERATE,(unsigned long)framerate);
	if(ret < 0)
	{
		printf(" get video frame rate error \n");
		return -1;
	}
	return ret;
}

int wb702SetJPEGQuality(UINT quality)
{
	int ret = 0;
	int var = quality;
	
	if(var >2) var = 1;
	var = 3 - var;
	
	ret = wb702_ioctl(WB702SETJPEGQUALITY,(unsigned long)&var);
	if(ret < 0)
	{
		printf(" set jpeg quality error \n");
		return -1;
	}
	return ret;
}

int wb702GetJPEGQuality(UINT * quality)
{
	int ret = 0;
	
	ret = wb702_ioctl(WB702GETJPEGQUALITY,(unsigned long)quality);
	if(ret < 0)
	{
		printf(" get jpeg quality error \n");
		return -1;
	}
	if(*quality < 1 || *quality > 3) *quality = 2;
	*quality = 3 - *quality;
	return ret;
}

int wb702SetVideoDynamicBitrate(UINT32 bitrate)
{
	int ret = 0;
	int var = bitrate;
	
	ret = wb702_ioctl(WB702SETVIDEODYNAMICBITRATE,(unsigned long)&var);
	if(ret < 0)
	{
		printf(" set video dynamic bitrate error \n");
		return -1;
	}
	return ret;
}

int wb702SetVideoDynamicFramerate(UINT32 framerate)
{
	int ret = 0;
	int var = framerate;
	
	ret = wb702_ioctl(WB702SETVIDEODYNAMICFRAMERATE,(unsigned long)&var);
	if(ret < 0)
	{
		printf(" set video dynamic framerate error \n");
		return -1;
	}
	return ret;
}

/*
 * arguments:
 *		pcSrcImage	jpeg image data
 *		iSrcLength	jpeg image data size
 *		iWidth		jpge image width
 *		iHeight		jpeg image height
 * usage:
 *		Display a jpeg on LCD as full screen background
 *		(that is the size of jpeg must be equal to LCD's size)
 *
 */
BOOL wb702JPEG2RGB565(UCHAR *pcSrcImage, UCHAR *pcDesImage, int iSrcLength, int iWidth, int iHeight)
{
#ifdef JPEG_DEC_WITH_MP4_BUFFER
    VP_BUFFER_MP4_DECODER_T *pEncJpeg;
#else
    VP_BUFFER_JPEG_DECODER_T *pDecJpeg;
#endif
    VP_JPEG_DEC_STATE_E eDecodeResult;
    VP_VIDEO_T *pVideoSetting;
    
	vjpegLock();
	
#ifdef INTERFACES_DEBUG
	diag_printf("wb702JPEG2RGB565(): begin--------------->\n");
#endif
	
	/* Check whether jpeg engine is enabled by wb702SetJPEG() */
	pVideoSetting = vdGetSettings();
	if(pVideoSetting->bIsEnableJPEG == FALSE)
	{
		//diag_printf("wb702JPEG2RGB565(): JPEG not enabled\n");
		vjpegUnlock ();
		return FALSE;
	}
	
	/* Get buffer for decode */
#ifdef JPEG_DEC_WITH_MP4_BUFFER	// To use mp4 decode buffer or jpeg decode buffer
	pEncJpeg = vmp4decGetBuffer();
	if(pEncJpeg == NULL)
	{
		printf("wb702JPEG2RGB565(): can't get jpeg buffer\n");
		vjpegUnlock ();
		return FALSE;
	}
#else
	pDecJpeg = bufDecJpegNew();
	if(pDecJpeg == NULL)
	{
		printf("wb702JPEG2RGB565(): can't get jpeg buffer\n");
		vjpegUnlock ();
		return FALSE;
	}
	//diag_printf("decode buffer 0x%x, len 0x%x\n", pDecJpeg, MAX_MP4_WIDTHxHEIGHT+MAX_MP4_WIDTHxHEIGHT/2);
#endif
	
	/* Set source image's width and height to jpeg engine */
	vjpegDecSetHW(iWidth, iHeight);
	
	/* Copy source image data to buffer */
#ifdef JPEG_DEC_WITH_MP4_BUFFER
	if(iSrcLength > sizeof(pEncJpeg->aucBitstream))
	{
		diag_printf("wb702JPEG2RGB565(): image data too large\n");
		vjpegUnlock ();
		return FALSE;
	}
	memcpy ((char*) NON_CACHE (pEncJpeg->aucBitstream), (char*)  (pcSrcImage), iSrcLength);
#else
	pDecJpeg->pSrcImage = pcSrcImage;
#endif
	
#ifdef JPEG_DEC_WITH_MP4_BUFFER
	eDecodeResult = decodeJPEGPrimaryImage ((void*)pEncJpeg);
#else
	eDecodeResult = decodeJPEGPrimaryImage ((void*)pDecJpeg);
#endif
	
	/* Convert yuv to rbg565 */
	if (eDecodeResult == VP_JPEG_DEC_OK)
	{
		T_VPE_FC fc;
		UINT32 uYStartAddress;
		UINT32 uUStartAddress;
		UINT32 uVStartAddress;
		
		JPEGDECINFO_T *pjpegdecinfo = vjpegdecGetInfo();
		USHORT usHeight, usWidth;
		
#ifdef JPEG_DEC_WITH_MP4_BUFFER
		uYStartAddress = (UINT32)pEncJpeg->aucOutput;
		uUStartAddress = (UINT32)((char*)uYStartAddress+vjpegdecGetYSize(&g_vpJPEG.jpegdecinf));
		uVStartAddress = (UINT32)((char*)uUStartAddress+vjpegdecGetUVSize(&g_vpJPEG.jpegdecinf));
#else
		uYStartAddress = (UINT32)pDecJpeg->aucY;
		uUStartAddress = (UINT32)pDecJpeg->aucU;
		uVStartAddress = (UINT32)pDecJpeg->aucV;
#endif
		
		vdLock ();
#if defined OPT_CAPTURE_PLANAR_YUV420
		fc.eMode = C_YUV_PL420_RGB565;
#elif defined OPT_CAPTURE_PLANAR_YUV422
		fc.eMode = C_YUV_PL422_RGB565;
#endif
		usWidth = pjpegdecinfo->uPixels;
		usHeight = pjpegdecinfo->uLines;
		fc.uSrcImgHW		= ((UINT32) usHeight << 16) + (UINT32) usWidth;
		fc.uSrcYPacAddr		= uYStartAddress;
		fc.uSrcUAddr		= uUStartAddress;
		fc.uSrcVAddr		= uVStartAddress;
		fc.uDesYPacAddr		= (UINT32) pcDesImage;
		fc.uDesUAddr		= 0;
		fc.uDesVAddr		= 0;
		vdUnlock ();
		
		vvpeLock ();	
		vpeFormatConversion (&fc);
		if (vvpeTrigger () == 0)
		{
			vvpeWaitProcessOK ();
		}
		else
		{
#ifndef JPEG_DEC_WITH_MP4_BUFFER
			bufDecJpegDecRef(pDecJpeg);
#endif
			vvpeUnlock ();
			vjpegUnlock ();
			return FALSE;
		}
		vvpeUnlock ();
	}
#ifndef JPEG_DEC_WITH_MP4_BUFFER
	bufDecJpegDecRef(pDecJpeg);
#endif

#ifdef INTERFACES_DEBUG
	diag_printf("wb702JPEG2RGB565(): end<---------------\n");
#endif

	vjpegUnlock ();
	
	return TRUE;
}

int wb702SetAudioPlayVolume(int lvolume, int rvolume) //0~31
{
	int ret = 0;
	WB_AUDIO_VOLUME audioVolume;
	
	audioVolume.lvolume = lvolume;
	audioVolume.rvolume = rvolume;
	ret = wb702_ioctl(WB702SETAUDIOPLAYVOLUME, (unsigned long)&audioVolume);
	if (ret != true)
	{
		printf("wb702SetAudioPlayVolume(): ioctl error \n");
		return -1;
	}
		
	return ret;

}

int wb702SetAudioRecordVolume(int lvolume, int rvolume) //0~31
{
	int ret = 0;
	WB_AUDIO_VOLUME audioVolume;
	
	audioVolume.lvolume = lvolume;
	audioVolume.rvolume = rvolume;
	ret = wb702_ioctl(WB702SETAUDIORECORDVOLUME, (unsigned long)&audioVolume);
	if (ret != true)
	{
		printf("wb702SetAudioRecordVolume(): ioctl error \n");
		return -1;
	}
		
	return ret;

}

int wb702SetAudioNotificationVolume(int volume) //2~60
{
	int ret = 0;
	
	ret = wb702_ioctl(WB702SETAUDIONOTIFICATIONVOLUME, (unsigned long)&volume);
	if (ret != true)
	{
		printf("wb702SetAudioNotificationVolume(): ioctl error \n");
		return -1;
	}
		
	return ret;

}

int wb702SetVideoContrastBright(int contrast, int bright)
{
	int ret = 0;
	WB_VIDEO_CONTRAST_BRIGHT videocb;
	
	videocb.contrast = contrast;
	videocb.bright = bright;
	ret = wb702_ioctl(WB702SETVIDEOCONTRASTBRIGHT, (unsigned long)&videocb);
	if (ret != true)
	{
		printf("wb702SetVideoContrastBright(): ioctl error \n");
		return -1;
	}
	
	return ret;
}

int wb702EnableDrawImageTime(BOOL bEnable)
{
	int ret = 0;
	
	ret = wb702_ioctl(WB702ENABLEDRAWIMAGETIME, (unsigned long)&bEnable);
	if (ret != true)
	{
		printf("wb702EnableDrawImageTime(): ioctl error \n");
		return -1;
	}
	
	return ret;
}

int wb702SetDrawContent(int index, char *pcContent, int x, int y)
{
	int ret = 0;
	INFO_CONTENT infoContent;
	
	infoContent.index = index;
	infoContent.pcContent = pcContent;
	infoContent.x = x;
	infoContent.y = y;
	
	ret = wb702_ioctl(WB702SETDRAWCONTENT, (unsigned long)&infoContent);
	if (ret != true)
	{
		printf("wb702SetDrawContent(): ioctl error \n");
		return -1;
	}
	
	return ret;
}

int wb702ClearDrawContent(int index)
{
	int ret = 0;
	
	ret = wb702_ioctl(WB702CLEARDRAWCONTENT, (unsigned long)index);
	if(ret != true)
	{
		printf("wb702ClearDrawContent(): ioctl error \n");
		return -1;
	}
	
	return ret;
}

static char *find_token(const char *buf, int buflen, const char *token, int tokenlen)
{
	int i, j;
	char *pos = NULL;
	
	if(buf == NULL || token == NULL)
	{
		return NULL;
	}
	if(buflen <= 0 || tokenlen <= 0 || buflen < tokenlen)
	{
		return NULL;
	}
	
	for(i = 0; i < buflen; i++)
	{
		for(j = 0; j < tokenlen; j++)
		{
			if(buf[i+j] != token[j]) break;
		}
		if(j >= tokenlen)
		{
			pos = (char*)(buf + i);
			break;
		}
	}
	return pos;
}

/*
 * function:
 *		Init MP4_CHECK_INDEX before using mp4_check_init
 * arguments:
 *		framebuf		Frame buffer for mp4 frame collection
 *		framebuflen		Frame buffer size
 *		bcontinuecheck	Whether one mp4 frame is seperated into two packets.
 *		checklevel		About error check level.
 *		funcname		Function name for debug info.
 *
 */
void mp4_check_index_init(MP4_CHECK_INDEX *pmp4checkindex, char *framebuf, int framebuflen,
							BOOL bcontinuecheck,MP4_CHECK_INDEX_LEVEL checklevel, char *funcname)
{
	if(pmp4checkindex == NULL || framebuf == NULL)
	{
		return;
	}
	
	memset(pmp4checkindex, 0, sizeof(MP4_CHECK_INDEX));
	
	pmp4checkindex->pcmp4buf = framebuf;
	pmp4checkindex->imp4buflen = framebuflen;
	
	pmp4checkindex->funcname = funcname;
	pmp4checkindex->bcontinuecheck = bcontinuecheck;
	pmp4checkindex->checklevel = checklevel;
}

/*
 * function:
 *		Check whether the mp4 frame's index is continuous
 *
 */
void mp4_check_index(MP4_CHECK_INDEX *pmp4checkindex)
{
	int		lastmp4index;
	int		lastmp4indexprintcount;
	BOOL	lastmp4indexprint;
	char	*funcname;
	
	BOOL bcontinuecheck;
	MP4_CHECK_INDEX_LEVEL checklevel;
	
	char 	*framebuf = pmp4checkindex->pcmp4buf;
	int 	framebuflen = pmp4checkindex->imp4bufdatalen;
	int		datalen = pmp4checkindex->imp4bufdatalen;
	
	lastmp4index = pmp4checkindex->lastmp4index;
	lastmp4indexprintcount = pmp4checkindex->lastmp4indexprintcount;
	lastmp4indexprint = pmp4checkindex->lastmp4indexprint;
	funcname = pmp4checkindex->funcname;
	if(funcname == NULL)
	{
		funcname = "mp4_check_index";
	}
	
	bcontinuecheck = pmp4checkindex->bcontinuecheck;
	checklevel = pmp4checkindex->checklevel;

	while(framebuflen > 0)
	{
		char *pbegin = NULL, *pend = NULL;
		char *pnextbegin = NULL;
		int index;
		int checkfrom;
		int checkdatalen;
		int endcount;
		
		/* Check start token position */
		pbegin = find_token(framebuf, framebuflen, MP4ADDATIONBEGIN, strlen(MP4ADDATIONBEGIN));
		if(pbegin == NULL)
		{
			
			if(checklevel == MP4_CHECK_INDEX_LEVEL2)
			{
				diag_printf("%s(): can't find start token!!!!!!!!!!\n", funcname);
				if(bcontinuecheck == FALSE)
				{
					framebuflen = 0;
				}
				goto checkend;
			}
			
			/* To judge reason: (1) data format error. (2) frame lost. */
			/* Check whether MP4ADDATIONEND exists. If exists, MP4ADDATIONBEGIN is lost */
			checkfrom = 0;
			endcount = 0;
			do
			{
				pend = find_token(framebuf + checkfrom, framebuflen, MP4ADDATIONEND, strlen(MP4ADDATIONEND));
				if(pend != NULL)
				{
					checkfrom = pend + strlen(MP4ADDATIONEND) - framebuf;
					framebuflen -= checkfrom;
					endcount++;
				}
			}while(pend != NULL);
			if(endcount != 0)
			{
				diag_printf("%s(): start token is lost, because find %d end tokens!!!!!!!!!!\n", funcname, endcount);
			}
			
			/* Whether keep remained data */
			if(bcontinuecheck == TRUE)	/* Because begin token maybe parsed into two packets */
			{
				pend = framebuf + checkfrom;
				memmove(framebuf, pend, framebuflen);
			}
			else						/* User make sure begin and end token contained in one pakcet */
			{
				framebuflen = 0;
			}
			goto checkend;
		}
		else
		{
			pbegin += strlen(MP4ADDATIONBEGIN);
			pnextbegin = find_token(pbegin, framebuflen - (pbegin - framebuf), 
									MP4ADDATIONBEGIN, strlen(MP4ADDATIONBEGIN));
		}
		
		/* Check end token position */
		if(pnextbegin != NULL)
		{	/* End token should not after the second begin token */
			checkdatalen = pnextbegin - framebuf;
		}
		else
		{
			checkdatalen = framebuflen;
		}
		
		checkfrom = 0;
		endcount = 0;
		while(1)
		{
			pend = find_token(framebuf + checkfrom, checkdatalen - checkfrom, 
								MP4ADDATIONEND, strlen(MP4ADDATIONEND));
			if(pend == NULL)
			{
				break;
			}
			pend += strlen(MP4ADDATIONEND);
			
			if(pend < pbegin)
			{
				diag_printf("%s(): find a end token before start token, start token is lost!!!!!!!!!!\n", funcname);
			}
			else
			{
				endcount++;
			}
			checkfrom = pend - framebuf;
		}
		
		if(endcount == 0)
		{
			if(checklevel >= MP4_CHECK_INDEX_LEVEL1)
			{
				/* Whether parsed into two packets */
				if(pnextbegin != NULL)
				{	/* Not parsed */
					diag_printf("%s(): can't find end token, LOST!!!!!!!!!!\n", funcname);
				}
			}
			
			if(bcontinuecheck == TRUE)
			{
				if(pnextbegin != NULL)
				{
					framebuflen -= (pnextbegin - framebuf);
					memmove(framebuf, pnextbegin, framebuflen);
				}
				else
				{	/* Maybe parsed into two packets, so data is kept for re-construct a complete frame */
					goto checkend;
				}
			}
			else
			{
				framebuflen = 0;
			}
			goto indexend;
		}
		else
		{
			if(endcount != 1)
			{
				diag_printf("%s(): find more than 1 end token(%d)!!!!!!!!!!\n", funcname, endcount);
			}
			
			memcpy(&index, pbegin + sizeof(UINT32), sizeof(int));
			if(bcontinuecheck == TRUE)	/* Keep remained data, and check next frame */
			{
				framebuflen -= checkfrom;
				memmove(framebuf, framebuf + checkfrom, framebuflen);
			}
			else						/* Check just one frame */
			{
				framebuflen = 0;
			}
		}
		/*	// The space may not be 10, because frame may be parsed into two tcp packets
		else if(pend - pbegin != 10)
		{	// Check token block length
			diag_printf("%s(): index block len %d\n", funcname, pend-pbegin);
		}
		*/
		
		if(lastmp4index + 1 != index)
		{
			diag_printf("%s(): last index=%d, buffer data len=%d\n", funcname, lastmp4index, datalen);
			/* Index not continuous, start print the following ten index */
			lastmp4indexprint = true;
			lastmp4indexprintcount = 0;
		}
		/* When index not continuous, print the 10 immediate frame's index */
		if(lastmp4indexprint == true && lastmp4indexprintcount < 10)
		{
			diag_printf("%s(): index %d\n", funcname, index);
			lastmp4indexprintcount++;
		}
		if(lastmp4indexprint == true && lastmp4indexprintcount == 10)
		{
			/* Print complete */
			lastmp4indexprint = false;
			lastmp4indexprintcount = 0;
		}
		/* Record the newest index */
		lastmp4index = index;
indexend:
		do {
		}while(0);
	}

checkend:	
	pmp4checkindex->lastmp4index = lastmp4index;
	pmp4checkindex->lastmp4indexprintcount = lastmp4indexprintcount;
	pmp4checkindex->lastmp4indexprint = lastmp4indexprint;
	
	pmp4checkindex->imp4bufdatalen = framebuflen;

}

/*
 * function:
 *		Add mp4 data into MP4_CHECK_INDEX
 * arguments:
 *		framebuf		MP4 data
 *		framebuflen		MP4 data length
 *
 */
BOOL mp4_check_index_adddata(char *framebuf, int framebuflen, MP4_CHECK_INDEX *pmp4checkindex)
{
	int datalen = pmp4checkindex->imp4bufdatalen;
	
	if(datalen + framebuflen > pmp4checkindex->imp4buflen)
	{
		diag_printf("%s(): framecheckbuf is full!!!!!!!!!!\n", pmp4checkindex->funcname);
		return FALSE;
	}
	else
	{
		memcpy(pmp4checkindex->pcmp4buf + datalen, framebuf, framebuflen);
		datalen += framebuflen;
	}
	pmp4checkindex->imp4bufdatalen = datalen;
	
	return TRUE;
}



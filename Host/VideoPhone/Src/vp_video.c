#include "../Inc/inc.h"


typedef struct
{
	/* Thread content. */
	UINT32 auThreadBuffer[TT_THREAD_BUFFER_SIZE (2*1024) / sizeof (UINT32)];
	/* Message queue for the thread. */
#ifndef ECOS
	UINT32 auMsgBuffer[TT_MSG_BUFFER_SIZE (2) / sizeof (UINT32)];
	TT_MSG_QUEUE_T *pMsgQueue;
#else
	cyg_mbox auMsgBuffer;
	cyg_handle_t pMsgQueue;
	
	cyg_handle_t cygHandle;
	cyg_thread cygThread;
#endif
} VP_VIDEO_THREAD_T;


#pragma arm section zidata = "non_init"
static VP_VIDEO_T g_vpVideo;
static VP_VIDEO_THREAD_T g_vpVideoThread;
#pragma arm section zidata


#ifndef ECOS
static void vdThreadEntry (void *arg)
#else
static void vdThreadEntry (cyg_addrword_t arg)
#endif
{
	FUN_TT_MSG_PROC	fnMsgProc;
	void			*pData;

	while (1)
	{
		tt_msg_recv (g_vpVideoThread.pMsgQueue, &fnMsgProc, &pData);
		(*fnMsgProc) (pData);
	}
}

#ifdef ECOS
void vdThread_OnUpdateRemote_OnRefresh (UINT32 uYAddr, VP_VIDEO_T *pVideo, void *pInfo, UINT32 uType)
{
	T_VPE_FCDDA fc;
	VP_BUFFER_VIDEO_T *pTmpBuffer[2];
	MP4DECINFO_T *pMP4Info;
	JPEGDECINFO_T *pJPEGInfo;
	
	if(uType == 0)
	{
		pMP4Info = (MP4DECINFO_T *)pInfo;
	}
	else
	{
		pJPEGInfo = (JPEGDECINFO_T *)pInfo;
	}
	
	sysDisableIRQ ();
	pTmpBuffer[0] = bufVideoNew ();
	pTmpBuffer[1] = bufVideoNew ();
	sysEnableIRQ ();
	if (pTmpBuffer[0] == NULL || pTmpBuffer[1] == NULL)
	{
		bufVideoDecRef (pTmpBuffer[0]);
		bufVideoDecRef (pTmpBuffer[1]);
		return;
	}
	
	
	/* <1> Do blur filter. */
	if (pVideo->bIsEnableMP4Blur)
	{
		GFX_FILTER_CONTROL_T filter_ctl;
		GFX_FILTER_BLOCK_T src_image, dest_image;
	    GFX_SIZE_T image_size;

#if 1


		/* Gaussian */
		//filter_ctl.uFilter0     = 0x00031110;
		//filter_ctl.uFilter1     = 0x12122121;
		/* Mean */
		filter_ctl.uFilter0     = 0x00013C70;
		filter_ctl.uFilter1     = 0x11111111;
		/* Mine */
		//filter_ctl.uFilter0     = 0x00022490;
		//filter_ctl.uFilter1     = 0x12122121;
		
		filter_ctl.uMaxClamp    = 0x00000000;
		filter_ctl.uOffset      = 0x00000000;
		filter_ctl.uThreshold   = 0x000000FF;
    
		// Y block
		if(uType == 0)
		{
			image_size.nWidth = pMP4Info->uPixels;
			image_size.nHeight = pMP4Info->uLines;
		}
		else
		{
			image_size.nWidth = pJPEGInfo->uPixels;
			image_size.nHeight = pJPEGInfo->uLines;
		}

		src_image.uStartAddr = uYAddr;
		src_image.uPitch = image_size.nWidth;
    
		dest_image.uStartAddr = (UINT32) pTmpBuffer[0]->aucData;
		dest_image.uPitch = image_size.nWidth;

		vgfxLock ();
		gfxImageFilter(&dest_image, &src_image, image_size, &filter_ctl);
		vgfxWaitEngineReady ();
		vgfxUnlock ();
		
		

		// U block
#if defined OPT_CAPTURE_PLANAR_YUV420
		if(uType == 0)
		{
			image_size.nWidth = pMP4Info->uPixels / 2;
			image_size.nHeight = pMP4Info->uLines / 2;
		}
		else
		{
			image_size.nWidth = pJPEGInfo->uPixels / 2;
			image_size.nHeight = pJPEGInfo->uLines / 2;
		}
#elif defined OPT_CAPTURE_PLANAR_YUV422
		if(uType == 0)
		{
			image_size.nWidth = pMP4Info->uPixels / 2;
			image_size.nHeight = pMP4Info->uLines;
		}
		else
		{
			image_size.nWidth = pJPEGInfo->uPixels / 2;
			image_size.nHeight = pJPEGInfo->uLines;
		}
#endif
		src_image.uPitch = image_size.nWidth;
		dest_image.uPitch = image_size.nWidth;

		if(uType == 0)
		{
			src_image.uStartAddr += vmp4decGetYSize (pMP4Info);
			dest_image.uStartAddr += vmp4decGetYSize (pMP4Info);
		}
		else
		{
			src_image.uStartAddr += vjpegdecGetYSize (pJPEGInfo);
			dest_image.uStartAddr += vjpegdecGetYSize (pJPEGInfo);
		}
		vgfxLock ();
		gfxImageFilter(&dest_image, &src_image, image_size, &filter_ctl);
		vgfxWaitEngineReady ();
		vgfxUnlock ();

		// V block
		if(uType == 0)
		{
			src_image.uStartAddr += vmp4decGetUVSize (pMP4Info);
			dest_image.uStartAddr += vmp4decGetUVSize (pMP4Info);
		}
		else
		{
			src_image.uStartAddr += vjpegdecGetUVSize (pJPEGInfo);
			dest_image.uStartAddr += vjpegdecGetUVSize (pJPEGInfo);
		}
		vgfxLock ();
		gfxImageFilter(&dest_image, &src_image, image_size, &filter_ctl);
		vgfxWaitEngineReady ();
		vgfxUnlock ();

		
		uYAddr = (UINT32) pTmpBuffer[0]->aucData;
		SWAP (pTmpBuffer[0], pTmpBuffer[1], VP_BUFFER_VIDEO_T *);
#endif

#if 1
		/* Gaussian */
		filter_ctl.uFilter0     = 0x00031110;
		filter_ctl.uFilter1     = 0x12122121;
		/* Mean */
		//filter_ctl.uFilter0     = 0x00013C70;
		//filter_ctl.uFilter1     = 0x11111111;
		/* Mine */
		//filter_ctl.uFilter0     = 0x00022490;
		//filter_ctl.uFilter1     = 0x12122121;
		
		filter_ctl.uMaxClamp    = 0x00000000;
		filter_ctl.uOffset      = 0x00000000;
		filter_ctl.uThreshold   = 0x000000FF;
    
		// Y block
		if(uType == 0)
		{
			image_size.nWidth = pMP4Info->uPixels;
			image_size.nHeight = pMP4Info->uLines;
		}
		else
		{
			image_size.nWidth = pJPEGInfo->uPixels;
			image_size.nHeight = pJPEGInfo->uLines;
		}

		src_image.uStartAddr = uYAddr;
		src_image.uPitch = image_size.nWidth;
    
		dest_image.uStartAddr = (UINT32) pTmpBuffer[0]->aucData;
		dest_image.uPitch = image_size.nWidth;

		vgfxLock ();
		gfxImageFilter(&dest_image, &src_image, image_size, &filter_ctl);
		vgfxWaitEngineReady ();
		vgfxUnlock ();
		
		

		// U block
#if defined OPT_CAPTURE_PLANAR_YUV420
		if(uType == 0)
		{
			image_size.nWidth = pMP4Info->uPixels / 2;
			image_size.nHeight = pMP4Info->uLines / 2;
		}
		else
		{
			image_size.nWidth = pJPEGInfo->uPixels / 2;
			image_size.nHeight = pJPEGInfo->uLines / 2;
		}
#elif defined OPT_CAPTURE_PLANAR_YUV422
		if(uType == 0)
		{
			image_size.nWidth = pMP4Info->uPixels / 2;
			image_size.nHeight = pMP4Info->uLines;
		}
		else
		{
			image_size.nWidth = pJPEGInfo->uPixels / 2;
			image_size.nHeight = pJPEGInfo->uLines;
		}
#endif
		src_image.uPitch = image_size.nWidth;
		dest_image.uPitch = image_size.nWidth;

		if(uType == 0)
		{
			src_image.uStartAddr += vmp4decGetYSize (pMP4Info);
			dest_image.uStartAddr += vmp4decGetYSize (pMP4Info);
		}
		else
		{
			src_image.uStartAddr += vjpegdecGetYSize (pJPEGInfo);
			dest_image.uStartAddr += vjpegdecGetYSize (pJPEGInfo);
		}
		vgfxLock ();
		gfxImageFilter(&dest_image, &src_image, image_size, &filter_ctl);
		vgfxWaitEngineReady ();
		vgfxUnlock ();

		// V block
		if(uType == 0)
		{
			src_image.uStartAddr += vmp4decGetUVSize (pMP4Info);
			dest_image.uStartAddr += vmp4decGetUVSize (pMP4Info);
		}
		else
		{
			src_image.uStartAddr += vjpegdecGetUVSize (pJPEGInfo);
			dest_image.uStartAddr += vjpegdecGetUVSize (pJPEGInfo);
		}
		vgfxLock ();
		gfxImageFilter(&dest_image, &src_image, image_size, &filter_ctl);
		gfxWaitEngineReady ();
		vgfxUnlock ();

		
		uYAddr = (UINT32) pTmpBuffer[0]->aucData;
		SWAP (pTmpBuffer[0], pTmpBuffer[1], VP_BUFFER_VIDEO_T *);
#endif
	}

	
	/* <2> Convert planar data to rgb and do scaling. */
	vdLock ();
#if defined OPT_CAPTURE_PLANAR_YUV420
	fc.eMode			= C_YUV_PL420_RGB565;
#elif defined OPT_CAPTURE_PLANAR_YUV422
	fc.eMode			= C_YUV_PL422_RGB565;
#endif
	if(uType == 0)
	{
		fc.uSrcImgHW		= ((UINT32) pMP4Info->uLines << 16) + (UINT32) pMP4Info->uPixels;
	}
	else
	{
		fc.uSrcImgHW		= ((UINT32) pJPEGInfo->uLines << 16) + (UINT32) pJPEGInfo->uPixels;
	}
		
	if (pVideo->tRemoteWin.eRotate == CMD_ROTATE_LEFT
		|| pVideo->tRemoteWin.eRotate == CMD_ROTATE_RIGHT)
		fc.uDesImgHW		= ((UINT32) pVideo->tRemoteWin.tScalingSize.usWidth << 16) + (UINT32) pVideo->tRemoteWin.tScalingSize.usHeight;
	else
		fc.uDesImgHW		= ((UINT32) pVideo->tRemoteWin.tScalingSize.usHeight << 16) + (UINT32) pVideo->tRemoteWin.tScalingSize.usWidth;
			
	fc.uOffset			= 0;
	fc.uSrcYPacAddr		= (UINT32) uYAddr;
	if(uType == 0)
	{
		fc.uSrcUAddr		= fc.uSrcYPacAddr + vmp4decGetYSize (pMP4Info);
		fc.uSrcVAddr		= fc.uSrcUAddr + vmp4decGetUVSize (pMP4Info);
	}
	else
	{
		fc.uSrcUAddr		= fc.uSrcYPacAddr + vjpegdecGetYSize (pJPEGInfo);
		fc.uSrcVAddr		= fc.uSrcUAddr + vjpegdecGetUVSize (pJPEGInfo);
	}
	fc.uDesYPacAddr		= (UINT32) pTmpBuffer[0]->aucData;
	fc.uDesUAddr		= 0;
	fc.uDesVAddr		= 0;
	vdUnlock ();
	
	vvpeLock ();
	vpeFormatConversionDDA (&fc);
	if (vvpeTrigger () == 0)
		vvpeWaitProcessOK ();
	vvpeUnlock ();
	
	/* <3> Rotate */
	if (pVideo->tRemoteWin.eRotate != CMD_ROTATE_NORMAL)
	{
		T_VPE_ROTATION rt;
		
		SWAP (pTmpBuffer[0], pTmpBuffer[1], VP_BUFFER_VIDEO_T *);
		
		vdLock ();
		rt.eSrcFormat	= VPE_ROTATION_PACKET_RGB565;
		switch (pVideo->tRemoteWin.eRotate)
		{
			case CMD_ROTATE_LEFT:
				rt.eRotationMode = VPE_ROTATION_LEFT; break;
			case CMD_ROTATE_RIGHT:
				rt.eRotationMode = VPE_ROTATION_RIGHT; break;
			case CMD_ROTATE_FLIP:
				rt.eRotationMode = VPE_ROTATION_FLIP; break;
			case CMD_ROTATE_MIRROR:
				rt.eRotationMode = VPE_ROTATION_MIRROR; break;
			case CMD_ROTATE_R180:
				rt.eRotationMode = VPE_ROTATION_R180; break;
		}
		
		if (pVideo->tRemoteWin.eRotate == CMD_ROTATE_LEFT
			|| pVideo->tRemoteWin.eRotate == CMD_ROTATE_RIGHT)
			rt.uSrcImgHW	= ((UINT32) pVideo->tRemoteWin.tScalingSize.usWidth << 16) + (UINT32) pVideo->tRemoteWin.tScalingSize.usHeight;
		else
			rt.uSrcImgHW	= ((UINT32) pVideo->tRemoteWin.tScalingSize.usHeight << 16) + (UINT32) pVideo->tRemoteWin.tScalingSize.usWidth;

		rt.uSrcOffset	= 0;
		rt.uDesOffset	= 0;
		rt.uSrcYPacAddr = (UINT32) pTmpBuffer[1]->aucData;
		rt.uSrcUAddr	= 0;
		rt.uSrcVAddr	= 0;
		rt.uDesYPacAddr	= (UINT32) pTmpBuffer[0]->aucData;
		rt.uDesUAddr	= 0;
		rt.uDesVAddr	= 0;
		vdUnlock ();

		vvpeLock ();	
		vpeRotation (&rt);
		if (vvpeTrigger () == 0)
			vvpeWaitProcessOK ();
		vvpeUnlock ();
	}


	vdLock ();
	SWAP (pTmpBuffer[0], pVideo->tRemoteWin.pImage, VP_BUFFER_VIDEO_T *);
	vdUnlock ();


	bufVideoDecRef (pTmpBuffer[0]);
	bufVideoDecRef (pTmpBuffer[1]);
	

	/* <3> Bitbit to screen. */
	vdRefreshLcm ();
}
#endif

void vdInit ()
{
	memset (&g_vpVideo, 0, sizeof (g_vpVideo));
	tt_rmutex_init (&g_vpVideo.tLock);
	g_vpVideo.bIsLcmConfigured	= FALSE;
	
	g_vpVideo.uMotionsSinceLast		= 0;
	g_vpVideo.uMotionsSinceBoot		= 0;
	g_vpVideo.eMotionSensibility	= IMG_CMP_SENSIBILITY_HIGH;
	g_vpVideo.bIsEnableMotionDetect	= FALSE;
	g_vpVideo.uMotionIgnoreInitCount= 0;
	g_vpVideo.apMotionBuffer[0]		= NULL;
	g_vpVideo.apMotionBuffer[1]		= NULL;
	
	g_vpVideo.bIsEnableLocalMP4 = FALSE;
	g_vpVideo.bIsEnableMP4Blur	= FALSE;
	g_vpVideo.bLocalOnTop		= TRUE;
	
	g_vpVideo.bIsEnableJPEG		= FALSE;
	
#if defined OPT_USE_VIDEO_ENCODER
	g_vpVideo.tLocalWin.pImage	= bufVideoNew ();
#else
	g_vpVideo.tLocalWin.pImage	= NULL;
#endif
#if defined OPT_USE_VIDEO_DECODER
	g_vpVideo.tRemoteWin.pImage	= bufVideoNew ();
#else
	g_vpVideo.tRemoteWin.pImage	= NULL;
#endif
	
	

	/* Create thread & message queue. */
#ifndef ECOS
	g_vpVideoThread.pMsgQueue = tt_msg_queue_init (g_vpVideoThread.auMsgBuffer,
											   sizeof (g_vpVideoThread.auMsgBuffer));
	tt_create_thread ("video",
		1,
		g_vpVideoThread.auThreadBuffer,
		sizeof (g_vpVideoThread.auThreadBuffer),
		vdThreadEntry,
		NULL);
#else
	tt_msg_queue_init (&(g_vpVideoThread.pMsgQueue), &(g_vpVideoThread.auMsgBuffer),
											   sizeof (g_vpVideoThread.auMsgBuffer));
	cyg_thread_create (10, &vdThreadEntry, NULL, "video",
		g_vpVideoThread.auThreadBuffer, sizeof (g_vpVideoThread.auThreadBuffer),
		&(g_vpVideoThread.cygHandle), &(g_vpVideoThread.cygThread));
	cyg_thread_resume(g_vpVideoThread.cygHandle);
#endif
	
}


void vdSetLCMSize (VP_SIZE_T tSize)
{
	vdLock();
	g_vpVideo.tLcmSize.usWidth	= tSize.usWidth;
	g_vpVideo.tLcmSize.usHeight	= tSize.usHeight;
	g_vpVideo.bIsLcmConfigured = TRUE;
	vdUnlock();
}



static void vdAdjustLCM (VP_VIDEO_WINDOW_T *pWindow)
{
	if (!g_vpVideo.bIsLcmConfigured)
		return;	//Nothing to be done

	/* Check if the video is out of screen range. */
	if (pWindow->tDisplayPos.sLeft < 0)
	{
		pWindow->tClippedWindow.tPoint.sLeft -= pWindow->tDisplayPos.sLeft;
		pWindow->tClippedWindow.tSize.usWidth += pWindow->tDisplayPos.sLeft;
		pWindow->tDisplayPos.sLeft = 0;
		
		if (pWindow->tClippedWindow.tPoint.sLeft < 0
			|| pWindow->tClippedWindow.tSize.usWidth > pWindow->tScalingSize.usWidth)
		{
			pWindow->tClippedWindow.tPoint.sLeft = 0;
			pWindow->tClippedWindow.tSize.usWidth = 0;
		}
	}
	
	
	if (pWindow->tDisplayPos.sTop < 0)
	{
		pWindow->tClippedWindow.tPoint.sTop -= pWindow->tDisplayPos.sTop;
		pWindow->tClippedWindow.tSize.usHeight += pWindow->tDisplayPos.sTop;
		pWindow->tDisplayPos.sTop = 0;
		
		if (pWindow->tClippedWindow.tPoint.sTop < 0
			|| pWindow->tClippedWindow.tSize.usHeight > pWindow->tScalingSize.usHeight)
		{
			pWindow->tClippedWindow.tPoint.sTop = 0;
			pWindow->tClippedWindow.tSize.usHeight = 0;
		}
	}
	
	if (pWindow->tDisplayPos.sLeft + pWindow->tClippedWindow.tSize.usWidth
		> g_vpVideo.tLcmSize.usWidth)
	{
		if (pWindow->tDisplayPos.sLeft > g_vpVideo.tLcmSize.usWidth)
		{
			pWindow->tDisplayPos.sLeft = 0;
			pWindow->tClippedWindow.tSize.usWidth = 0;
		}
		else
		{
			pWindow->tClippedWindow.tSize.usWidth = g_vpVideo.tLcmSize.usWidth - pWindow->tDisplayPos.sLeft;
			if (pWindow->tClippedWindow.tSize.usWidth > pWindow->tScalingSize.usWidth)
			{
				pWindow->tClippedWindow.tPoint.sLeft = 0;
				pWindow->tClippedWindow.tSize.usWidth = 0;
			}
		}
	}
	
	if (pWindow->tDisplayPos.sTop + pWindow->tClippedWindow.tSize.usHeight
		> g_vpVideo.tLcmSize.usHeight)
	{
		if (pWindow->tDisplayPos.sTop > g_vpVideo.tLcmSize.usHeight)
		{
			pWindow->tDisplayPos.sTop = 0;
			pWindow->tClippedWindow.tSize.usHeight = 0;
		}
		else
		{
			pWindow->tClippedWindow.tSize.usHeight = g_vpVideo.tLcmSize.usHeight - pWindow->tDisplayPos.sTop;
			if (pWindow->tClippedWindow.tSize.usHeight > pWindow->tScalingSize.usHeight)
			{
				pWindow->tClippedWindow.tPoint.sTop = 0;
				pWindow->tClippedWindow.tSize.usHeight = 0;
			}
		}
	}

}


static BOOL vdAdjustWindow (VP_VIDEO_WINDOW_T *pWindow)
{
	VP_SIZE_T	tSourceSize;
	VP_SIZE_T	tScalingSize;

	if (pWindow->tSourceSize.usWidth == 0
		|| pWindow->tSourceSize.usHeight == 0)
		return TRUE;	//Nothing to be done

	if (!pWindow->bAutoCenterClippedWindow)
	{
		vdAdjustLCM (pWindow);
		return TRUE;
	}

	/* <1> Get rotated source size if necessary. */
	if (pWindow->eRotate == CMD_ROTATE_LEFT
		|| pWindow->eRotate == CMD_ROTATE_RIGHT)
	{/* On roate 90 mode, swap source width and height. */
		tSourceSize.usWidth		= pWindow->tSourceSize.usHeight;
		tSourceSize.usHeight	= pWindow->tSourceSize.usWidth;
	}
	else
		tSourceSize = pWindow->tSourceSize;

	/* <2> Calculate scaling size by the source_width / source_height. */
	tScalingSize = pWindow->tClippedWindow.tSize;
	if (tScalingSize.usWidth *
			(UINT32) tSourceSize.usHeight / tSourceSize.usWidth
			> (UINT32) tScalingSize.usHeight)
		tScalingSize.usHeight = tScalingSize.usWidth *
			(UINT32) tSourceSize.usHeight / tSourceSize.usWidth;

	if (tScalingSize.usHeight *
			(UINT32) tSourceSize.usWidth / tSourceSize.usHeight
			> (UINT32) tScalingSize.usWidth)
		tScalingSize.usWidth = tScalingSize.usHeight *
			(UINT32) tSourceSize.usWidth / tSourceSize.usHeight;
	
	/* <3> Adjust scaling size by checking if the full width and height can be matched by a scaling factor. */
	tScalingSize.usWidth = EvaluateScalingSize (tSourceSize.usWidth, tScalingSize.usWidth, 255, +16);
	tScalingSize.usHeight = EvaluateScalingSize (tSourceSize.usHeight, tScalingSize.usHeight, 255, +16);
	if (tScalingSize.usWidth == 0 || tScalingSize.usHeight == 0)
	{
#ifndef ECOS
		dbgSetError (APP_ERR_CAPABILITY_LIMIT, "Can not set vidoe size: %d %d (video window: %d %d)\n", 
			(int) tSourceSize.usWidth, (int) tSourceSize.usHeight,
			(int) pWindow->tClippedWindow.tSize.usWidth, (int) pWindow->tClippedWindow.tSize.usHeight);
#else
		printf ("Can not set vidoe size: %d %d (video window: %d %d)\n", 
			(int) tSourceSize.usWidth, (int) tSourceSize.usHeight,
			(int) pWindow->tClippedWindow.tSize.usWidth, (int) pWindow->tClippedWindow.tSize.usHeight);
#endif
		return FALSE;
	}
	else if (tScalingSize.usWidth > tSourceSize.usWidth
		|| tScalingSize.usHeight > tSourceSize.usHeight)
	{/* Too large clipped size. */
		//return FALSE;
		tScalingSize = tSourceSize;
	}
	pWindow->tScalingSize = tScalingSize;
	
	/* <4> Check if the clip window is out of range */
	if (pWindow->tClippedWindow.tSize.usWidth > pWindow->tScalingSize.usWidth)
		pWindow->tClippedWindow.tSize.usWidth = pWindow->tScalingSize.usWidth;
	if (pWindow->tClippedWindow.tSize.usHeight > pWindow->tScalingSize.usHeight)
		pWindow->tClippedWindow.tSize.usHeight = pWindow->tScalingSize.usHeight;
	
	/* <5> Set clipped offset. */
	pWindow->tClippedWindow.tPoint.sLeft =
		(SHORT) (pWindow->tScalingSize.usWidth - pWindow->tClippedWindow.tSize.usWidth) / 2;
	pWindow->tClippedWindow.tPoint.sTop =
		(SHORT) (pWindow->tScalingSize.usHeight - pWindow->tClippedWindow.tSize.usHeight) / 2;
	
	/* <6> Adjust clipped window. */
	vdAdjustLCM (pWindow);

/*	
	sysSafePrintf ("%d %d %d %d\n", 
		(int) pWindow->tClippedWindow.tPoint.sLeft,
		(int) pWindow->tClippedWindow.tPoint.sTop,
		(int) pWindow->tClippedWindow.tSize.usWidth,
		(int) pWindow->tClippedWindow.tSize.usHeight);
*/
	
	return TRUE;
}



static BOOL vdSetWindowEx (VP_VIDEO_WINDOW_T *pWindow,
						   VP_SIZE_T tScalingSize,
						   VP_RECT_T tClippedWindow,
						   VP_POINT_T tDisplayPos,
						   CMD_ROTATE_E eRotate)
{
	VP_VIDEO_WINDOW_T tWindow;
	BOOL bReturn;
	
	vdLock();
	
	tWindow = *pWindow;

	pWindow->tScalingSize	= tScalingSize;
	pWindow->tClippedWindow	= tClippedWindow;
	pWindow->tDisplayPos	= tDisplayPos;
	pWindow->eRotate		= eRotate;
	pWindow->bAutoCenterClippedWindow = FALSE;
	
	bReturn = vdAdjustWindow (pWindow);
	if (bReturn == FALSE)
		*pWindow = tWindow;

	vdUnlock();
	return bReturn;
}


static BOOL vdSetWindow (VP_VIDEO_WINDOW_T *pWindow,
						 VP_SIZE_T tClippedSize,
						 VP_POINT_T tDisplayPos,
						 CMD_ROTATE_E eRotate)
{
	VP_VIDEO_WINDOW_T tWindow;
	BOOL bReturn;

	vdLock();
	
	tWindow = *pWindow;
	
	pWindow->tScalingSize.usWidth	= 0;
	pWindow->tScalingSize.usHeight	= 0;
	pWindow->tClippedWindow.tPoint.sLeft	= 0;
	pWindow->tClippedWindow.tPoint.sTop		= 0;
	pWindow->tClippedWindow.tSize			= tClippedSize;
	pWindow->tDisplayPos	= tDisplayPos;
	pWindow->eRotate		= eRotate;
	pWindow->bAutoCenterClippedWindow = TRUE;
	
	bReturn = vdAdjustWindow (pWindow);
	if (bReturn == FALSE)
		*pWindow = tWindow;

	vdUnlock();
	return bReturn;
}


static BOOL vdSetSourceVideoSize (VP_VIDEO_WINDOW_T *pWindow, VP_SIZE_T tSourceSize)
{
	VP_VIDEO_WINDOW_T tWindow;
	BOOL bReturn;

	vdLock();
	
	tWindow = *pWindow;
	pWindow->tSourceSize = tSourceSize;
	
	bReturn = vdAdjustWindow (pWindow);
	if (bReturn == FALSE)
		*pWindow = tWindow;

	vdUnlock();
	return bReturn;	
}


BOOL vdSetLocalWindowEx (VP_POINT_T tDisplayPos,
						 VP_SIZE_T tScalingSize,
						 VP_RECT_T tClippedWindow,
						 CMD_ROTATE_E eRotate)

{
	return vdSetWindowEx (&g_vpVideo.tLocalWin,
						  tScalingSize,
						  tClippedWindow,
						  tDisplayPos,
						  eRotate);
}


BOOL vdSetLocalWindow (VP_POINT_T tDisplayPos,
					   VP_SIZE_T tClippedSize,
					   CMD_ROTATE_E eRotate)
{
	return vdSetWindow (&g_vpVideo.tLocalWin,
						tClippedSize,
						tDisplayPos,
						eRotate);
}


BOOL vdSetLocalSourceWindow (VP_SIZE_T tSourceSize)
{
	return vdSetSourceVideoSize (&g_vpVideo.tLocalWin, tSourceSize);
}


BOOL vdSetRemoteWindowEx (VP_POINT_T tDisplayPos,
						  VP_SIZE_T tScalingSize,
						  VP_RECT_T tClippedWindow,
						  CMD_ROTATE_E eRotate)

{
	return vdSetWindowEx (&g_vpVideo.tRemoteWin,
						  tScalingSize,
						  tClippedWindow,
						  tDisplayPos,
						  eRotate);
}


BOOL vdSetRemoteWindow (VP_POINT_T tDisplayPos,
						VP_SIZE_T tClippedSize,
						CMD_ROTATE_E eRotate)
{
	return vdSetWindow (&g_vpVideo.tRemoteWin,
						tClippedSize,
						tDisplayPos,
						eRotate);
}


BOOL vdSetRemoteSourceWindow (VP_SIZE_T tSourceSize)
{
	return vdSetSourceVideoSize (&g_vpVideo.tRemoteWin, tSourceSize);
}


void vdEnableLocalWindow (BOOL bEnable)
{
	g_vpVideo.tLocalWin.bEnable = bEnable;
}

void vdEnableRemoteWindow (BOOL bEnable)
{
	g_vpVideo.tRemoteWin.bEnable = bEnable;
}

void vdEnableLocalMP4 (BOOL bEnable)
{
	g_vpVideo.bIsEnableLocalMP4 = bEnable;
}

void vdEnableMP4Blur (BOOL bEnable)
{
	g_vpVideo.bIsEnableMP4Blur = bEnable;
}

void vdEnableLocalJPEG (BOOL bEnable)
{
	g_vpVideo.bIsEnableJPEG = bEnable;
}

void vdSetZIndex (BOOL bLocalOnTop)
{
	g_vpVideo.bLocalOnTop = bLocalOnTop;
}

void vdEnableMotionDetect (BOOL bEnable, IMG_CMP_SENSIBILITY_E eSensibility)
{
	if ((g_vpVideo.bIsEnableMotionDetect == FALSE) && (bEnable == TRUE))
	{
		g_vpVideo.uMotionIgnoreInitCount = 0;
	}
	g_vpVideo.bIsEnableMotionDetect = bEnable;
	g_vpVideo.eMotionSensibility = eSensibility;
}

UINT32 vdGetMotionsNum (void)
{
	return g_vpVideo.uMotionsSinceBoot;
}


VP_VIDEO_T *vdGetSettings (void)
{
	return &g_vpVideo;
}


static void __vdRefreshLcm (void)
{
	int i;
	VP_VIDEO_T *pVideo;
	VP_VIDEO_WINDOW_T *pWin[2];

#ifndef ECOS
	if (vlcmTryLock () != 0)
		return;
#else
	if (vlcmTryLock () != true)
		return;
#endif

	vdLock ();
	pVideo = vdGetSettings ();
	if (pVideo->bLocalOnTop)
	{
		pWin[0] = &pVideo->tRemoteWin;
		pWin[1] = &pVideo->tLocalWin;
	}
	else
	{
		pWin[1] = &pVideo->tRemoteWin;
		pWin[0] = &pVideo->tLocalWin;
	}

	vgfxLock ();
	for (i = 0; i < sizeof (pWin) / sizeof (pWin[0]); i++)
	{
		if (pWin[i]->bEnable && pWin[i]->pImage != NULL)
		{
			GFX_SURFACE_T surface;
			GFX_RECT_T rect;
			GFX_PNT_T point;


			surface.nWidth		= (INT) pWin[i]->tScalingSize.usWidth;
			surface.nHeight		= (INT) pWin[i]->tScalingSize.usHeight;
			surface.nPitch		= surface.nWidth * 2;
			surface.uStartAddr	= (INT) pWin[i]->pImage->aucData;
			gfxSetSrcSurface (&surface);

			surface.nWidth		= (INT) pVideo->tLcmSize.usWidth;
			surface.nHeight		= (INT) pVideo->tLcmSize.usHeight;
			surface.nPitch		= surface.nWidth * 2;
			surface.uStartAddr	= (INT) vlcmGetLCMBuffer ()->aucData;
			gfxSetDestSurface (&surface);

			rect.fC.nLeft	= (INT) pWin[i]->tClippedWindow.tPoint.sLeft;
			rect.fC.nTop	= (INT) pWin[i]->tClippedWindow.tPoint.sTop;
			rect.fC.nRight	= (INT) pWin[i]->tClippedWindow.tPoint.sLeft + pWin[i]->tClippedWindow.tSize.usWidth - 1;
			rect.fC.nBottom	= (INT) pWin[i]->tClippedWindow.tPoint.sTop + pWin[i]->tClippedWindow.tSize.usHeight - 1;
			
			point.nX	= (INT) pWin[i]->tDisplayPos.sLeft;
			point.nY	= (INT) pWin[i]->tDisplayPos.sTop;

			gfxPutImage (rect, point);
			vgfxWaitEngineReady ();		
			
			//sysprintf ("Write: (%d) %d %d\n", i, pWin[i]->pImage->aucData, vlcmGetLCMBuffer ()->aucData);
		}
	}
	vgfxUnlock ();
	vdUnlock ();
	
	vlcmStartRefresh (TRUE);
	vlcmUnlock ();
}



static void vdThread_OnRefreshLcm (void *arg)
{
	__vdRefreshLcm ();
}


void vdRefreshLcm (void)
{
	tt_msg_try_send (g_vpVideoThread.pMsgQueue, vdThread_OnRefreshLcm, NULL);
}



#ifndef ECOS
void cmd18_04_SendMediaBitstream_OnRefresh (UINT32 uYAddr, VP_VIDEO_T *pVideo, void *pInfo, UINT32 uType);
#endif
static void vdThread_OnUpdateRemote (void *arg)
{
	VP_VIDEO_T *pVideo;
	VP_BUFFER_MP4_DECODER_T *pMP4DecBuf = vmp4decGetBuffer ();

	pVideo = vdGetSettings ();
	if ((pVideo->tRemoteWin.bEnable == TRUE) && (vlcmIsConfigured () == TRUE))
	{
#ifndef ECOS
		if(arg == NULL)
		{
			MP4DECINFO_T *pMP4Info = vmp4decGetInfo ();
			vdSetRemoteSourceWindow (VP_SIZE (pMP4Info->uPixels, pMP4Info->uLines));
			cmd18_04_SendMediaBitstream_OnRefresh ((UINT32) NON_CACHE (pMP4DecBuf->aucOutput), pVideo, pMP4Info, 0);
		}
		else
		{
			JPEGDECINFO_T *pJPEGInfo = vjpegdecGetInfo();
			vdSetRemoteSourceWindow (VP_SIZE (pJPEGInfo->uPixels, pJPEGInfo->uLines));
			cmd18_04_SendMediaBitstream_OnRefresh ((UINT32) NON_CACHE (pMP4DecBuf->aucOutput), pVideo, pJPEGInfo, 1);
		
		}
#else
		if(arg == NULL)
		{
			MP4DECINFO_T *pMP4Info = vmp4decGetInfo ();
			vdSetRemoteSourceWindow (VP_SIZE (pMP4Info->uPixels, pMP4Info->uLines));
			vdThread_OnUpdateRemote_OnRefresh ((UINT32) NON_CACHE (pMP4DecBuf->aucOutput), pVideo, pMP4Info, 0);
		}
		else
		{
			JPEGDECINFO_T *pJPEGInfo = vjpegdecGetInfo();
			vdSetRemoteSourceWindow (VP_SIZE (pJPEGInfo->uPixels, pJPEGInfo->uLines));
			vdThread_OnUpdateRemote_OnRefresh ((UINT32) NON_CACHE (pMP4DecBuf->aucOutput), pVideo, pJPEGInfo, 1);
		
		}
#endif
	}
}



void vdDelMotionBuffer (void)
{
	if (g_vpVideo.apMotionBuffer[0] != NULL)
	{
		bufMotionDecRef (g_vpVideo.apMotionBuffer[0]);
		g_vpVideo.apMotionBuffer[0] = NULL;
	}
}

void vdAddMotionBuffer (VP_BUFFER_MOTION_DETECT_T *pMotionBuffer)
{
	vdDelMotionBuffer ();
	g_vpVideo.apMotionBuffer[0] = g_vpVideo.apMotionBuffer[1];
	g_vpVideo.apMotionBuffer[1] = pMotionBuffer;
}


//void vdUpdateRemote (void)
void vdUpdateRemote (void *parg)
{
	tt_msg_try_send (g_vpVideoThread.pMsgQueue, vdThread_OnUpdateRemote, parg);
}


void vdLock (void)
{
	tt_rmutex_lock (&g_vpVideo.tLock);
}

void vdUnlock (void)
{
	tt_rmutex_unlock (&g_vpVideo.tLock);
}



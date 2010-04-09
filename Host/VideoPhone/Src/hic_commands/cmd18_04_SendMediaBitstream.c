#include "../../Inc/inc.h"


#ifdef __HIC_SUPPORT__


void cmd18_04_SendMediaBitstream_OnRefresh (UINT32 uYAddr, VP_VIDEO_T *pVideo, void *pInfo, UINT32 uType)
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



void cmd18_04_SendMediaBitstream (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	CMD_MEDIA_TYPE_E	eMediaType = (CMD_MEDIA_TYPE_E) ucD;
	UINT32 uLength = ((UINT32) ucC << 16)
				   | ((UINT32) ucB << 8)
				   | ((UINT32) ucA);

	DUMP_HIC_COMMAND;
	
	switch (eMediaType)
	{
		case CMD_MEDIA_VIDEO:
		{
			VP_BUFFER_MP4_DECODER_T *pMP4DecBuf = vmp4decGetBuffer ();
			
			if (uLength > sizeof (pMP4DecBuf->aucBitstream))
			{
				hicSaveToReg1 (HIC_ERR_TOO_LONG_DATA);
				hicSetCmdError (pHicThread);
			}
			else if (pMP4DecBuf != NULL)
			{
				VP_MP4_DEC_STATE_E eEncodeResult;
				
				hicMassData_FifoToMem (pHicThread, (void *) NON_CACHE (pMP4DecBuf->aucBitstream), uLength);

//Debugging
#ifdef MP4_TEST_DUMP_HEADER
{
int i;
for (i = 0; i < 20; i++)
	sysSafePrintf ("%02x ", (int) *(UCHAR *) (NON_CACHE (pMP4DecBuf->aucBitstream) + i));
sysSafePrintf ("\nLast: ");

for (i = 20; i > 0; i--)
	sysSafePrintf ("%02x ", (int) *(UCHAR *) (NON_CACHE (pMP4DecBuf->aucBitstream) + uLength - i));
sysSafePrintf ("\n");
}
#endif

//Debugging
#ifdef MP4_TEST_CHECKSUM
{
	UINT32 mp4GetChecksum (CONST UCHAR *cpucData, UINT32 uLength);
	UINT32 uCheck = mp4GetChecksum ((UCHAR *) NON_CACHE (pMP4DecBuf->aucBitstream), uLength - 8);
	INT nIndex = UCHAR_2_UINT32 ((UCHAR *) NON_CACHE (pMP4DecBuf->aucBitstream) + uLength - 4);

	if (uCheck != UCHAR_2_UINT32 ((UCHAR *) NON_CACHE (pMP4DecBuf->aucBitstream) + uLength - 8))
		sysSafePrintf ("Checksum error (mp4 index: %d)!\n", nIndex);
	else
		sysSafePrintf ("Checksum OK (mp4 index: %d)\n", nIndex);
}
#endif


#if 1
				eEncodeResult = vmp4decPrepare ((UINT32) NON_CACHE (pMP4DecBuf->aucBitstream), uLength);
#elif 0
				vmp4Lock ();
				if (vmp4decStart ((UINT32) NON_CACHE (pMP4DecBuf->aucBitstream), uLength) != 0)
					eEncodeResult = VP_MP4_DEC_ERROR;
				else
					eEncodeResult= vmp4decWaitOK ();
				sysSafePrintf ("Dec result: %d\n", eEncodeResult);
				
				if (eEncodeResult == VP_MP4_DEC_OK)
				{
					MP4DECINFO_T *pMP4Info = vmp4decGetInfo ();
					pMP4DecBuf->nOutputLength = vmp4decGetYSize (pMP4Info) + vmp4decGetUVSize (pMP4Info) * 2;
				}
				else
					pMP4DecBuf->nOutputLength = 0;
				vmp4Unlock ();

				/* Bitblt to screen. */
				if (eEncodeResult == VP_MP4_DEC_OK)
				{
					void vdUpdateRemote (void*);
					vdUpdateRemote (NULL);
				}
#endif

		
				hicSetCmdReady (pHicThread);	
			}
			else
			{
				hicSaveToReg1 (APP_ERR_CAPABILITY_LIMIT);
				hicSetCmdError (pHicThread);
			}
			
			break;
		}
		case CMD_MEDIA_AUDIO:
		{
			VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *pEncAudio = vauGetEncBuffer_Remote ();
			//sysSafePrintf ("AAA Length: %d\n", uLength);
			if (uLength > sizeof (pEncAudio->aucAudio))
			{
				hicSaveToReg1 (HIC_ERR_TOO_LONG_DATA);
				hicSetCmdError (pHicThread);				
			}
			else if (pEncAudio != NULL)
			{
				hicMassData_FifoToMem (pHicThread, pEncAudio->aucAudio, uLength);
				vauDecode (pEncAudio->aucAudio, uLength);
				hicSetCmdReady (pHicThread);
			}
			else
			{
				hicSaveToReg1 (APP_ERR_CAPABILITY_LIMIT);
				hicSetCmdError (pHicThread);
			}
			break;
		}
		case CMD_MEDIA_JPEG:
		{
            //if(vjpegTryLock("cmd18_04_SendMediaBitstream")==TRUE)
            vjpegLock();
            {
#ifndef JPEG_DEC_WITH_MP4_BUFFER
                VP_BUFFER_JPEG_ENCODER_T *pEncJpeg = vjpegNewEncBuffer();
#else
                VP_BUFFER_MP4_DECODER_T *pEncJpeg = vmp4decGetBuffer();
			//memset(pEncJpeg->aucBitstream, 0, 0x40000);
			//memset(pEncJpeg->aucOutput, 0, MAX_MP4_WIDTHxHEIGHT * 3 / 2);
#endif
                
                if(pEncJpeg == NULL)
                {
                    hicSaveToReg1 (APP_ERR_CAPABILITY_LIMIT);
                    hicSetCmdError (pHicThread);
                }
#ifndef JPEG_DEC_WITH_MP4_BUFFER
                else if(uLength > sizeof(pEncJpeg->aucJPEGBitstream))
#else
                else if(uLength > sizeof(pEncJpeg->aucBitstream))
#endif
                {
                    hicSaveToReg1 (HIC_ERR_TOO_LONG_DATA);
                    hicSetCmdError (pHicThread);				
                }
                else
                {
                    VP_JPEG_DEC_STATE_E eDecodeResult;
                    
#ifndef JPEG_DEC_WITH_MP4_BUFFER
                    hicMassData_FifoToMem (pHicThread, pEncJpeg->aucJPEGBitstream, uLength);
#else
                    hicMassData_FifoToMem (pHicThread, pEncJpeg->aucBitstream, uLength);
#endif
                    eDecodeResult = decodeJPEGPrimaryImage ((void*)pEncJpeg);

                    if (eDecodeResult == VP_JPEG_DEC_OK)
                    {
					void vdUpdateRemote (void*);
					vdUpdateRemote ((void*)1);
                    }

#ifndef JPEG_DEC_WITH_MP4_BUFFER
                    vjpegFreeEncBuffer(pEncJpeg);
                    vjpegFreeDecBuffer (g_vpJPEG.pJPEGDecodeBuffer);
#endif
                    hicSetCmdReady (pHicThread);
                }
                vjpegUnlock ();
			}
			break;
		}
		default:;
	}
}




#ifdef MP4_TEST_CHECKSUM
UINT32 mp4GetChecksum (CONST UCHAR *cpucData, UINT32 uLength)
{
	UINT32 uChecksum;
	CONST UCHAR *cpucEnd;
	
	for (cpucEnd = cpucData + uLength, uChecksum = 0;
		cpucData < cpucEnd;
		cpucData++)
	{
		uChecksum = ((uChecksum << 1) | ((uChecksum >> 23) & 0x01));
		uChecksum += (UINT32) *cpucData;
		uChecksum &= 0x00FFFFFF;
	}
	return uChecksum;
}
#endif

#endif


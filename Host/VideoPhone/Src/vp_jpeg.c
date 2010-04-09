#include "../Inc/inc.h"

#pragma arm section zidata = "non_init"
VP_JPEG_T g_vpJPEG;
#pragma arm section zidata

BOOL bJPEGInit = FALSE;

/* Quantization-table 0 ~ 2 */
UINT8 ucQTable0[64] = { 0x06, 0x04, 0x04, 0x05, 0x04, 0x04, 0x06, 0x05,
						0x05, 0x05, 0x06, 0x06, 0x06, 0x07, 0x08, 0x0E,
						0x09, 0x08, 0x08, 0x08, 0x08, 0x11, 0x0C, 0x0D,
						0x0A, 0x0E, 0x14, 0x11, 0x15, 0x14, 0x13, 0x11,
						0x13, 0x13, 0x16, 0x18, 0x1F, 0x1A, 0x16, 0x17,
						0x1D, 0x17, 0x13, 0x13, 0x1B, 0x25, 0x1B, 0x1D,
						0x20, 0x21, 0x23, 0x23, 0x23, 0x15, 0x1A, 0x26,
						0x29, 0x26, 0x22, 0x28, 0x1F, 0x22, 0x23, 0x21 },
	  ucQTable1[64] = { 0x06, 0x06, 0x06, 0x08, 0x07, 0x08, 0x10, 0x09,
						0x09, 0x10, 0x21, 0x16, 0x13, 0x16, 0x21, 0x21,
						0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21,
						0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21,
						0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21,
						0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21,
						0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21,
						0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21 },
	  ucQTable2[64] = { 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
						0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
						0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
						0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
						0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
						0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
						0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
						0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03 };

void vjpegInit(void)
{
#ifndef ECOS
    tt_rmutex_init(&g_vpJPEG.mtLock);
    tt_rmutex_init(&g_vpJPEG.mtLockEncoder);
    tt_rmutex_init(&g_vpJPEG.mtLockDecoder);
    
    tt_sem_init(&g_vpJPEG.semEncoder, 1);
    tt_sem_init(&g_vpJPEG.semDecoder, 1);
	tt_sem_down(&g_vpJPEG.semEncoder);
	tt_sem_down(&g_vpJPEG.semDecoder);
#else
    cyg_mutex_init(&g_vpJPEG.mtLock);
    cyg_mutex_init(&g_vpJPEG.mtLockEncoder);
    cyg_mutex_init(&g_vpJPEG.mtLockDecoder);
    
    cyg_semaphore_init(&g_vpJPEG.semEncoder, 1);
    cyg_semaphore_init(&g_vpJPEG.semDecoder, 1);
	cyg_semaphore_wait(&g_vpJPEG.semEncoder);
	cyg_semaphore_wait(&g_vpJPEG.semDecoder);
#endif
    
    g_vpJPEG.nRefCount = 0;
    g_vpJPEG.nRefCount_Encoder = 0;
    g_vpJPEG.nRefCount_Decoder = 0;
        
    g_vpJPEG.pJPEGEncodeBuffer = NULL;
    
   	listInit (&g_vpJPEG.listEncodedJPEG);
   	
   	g_vpJPEG.nJPEGQua = 2;
   	g_vpJPEG.bOnTheFly = TRUE;
   	g_vpJPEG.nOnTheFlyCount = 0;

#ifndef ECOS
    sysInstallISR(IRQ_LEVEL_1, IRQ_JPEG, (void*)jpegIntHandler);
#else
	cyg_interrupt_disable();
	cyg_interrupt_create(IRQ_JPEG, IRQ_LEVEL_1, NULL, &jpegIntHandler, &jpegIntHandler_DSR,
				&(g_vpJPEG.cygIntrHandle), &(g_vpJPEG.cygIntrBuffer));
	cyg_interrupt_attach(g_vpJPEG.cygIntrHandle);
	cyg_interrupt_unmask(IRQ_JPEG);
	cyg_interrupt_enable();
#endif

    jpegSetIRQHandler(C_JPEG_CALLBACK_ENCODE_COMPLETE_INTERRUPT, vjpegEncoderCom_Callback);
    jpegSetIRQHandler(C_JPEG_CALLBACK_DECODE_COMPLETE_INTERRUPT, vjpegDecoderCom_Callback);
    jpegSetIRQHandler(C_JPEG_CALLBACK_DECODE_ERROR_INTERRUPT, vjpegDecoderErr_Callback);
	if(g_vpJPEG.bOnTheFly == TRUE)
	{
		jpegSetIRQHandler(C_JPEG_CALLBACK_ENCODE_SWONTHEFLY_WAIT_INTERRUPT, vjpegOnTheFlyCom_Callback);
	}
		
    bJPEGInit = TRUE;
}

void vjpegEncInit(void)
{
	JPEG_ENCODE_SETTING_T       *tEncodeSetting = &(g_vpJPEG.tJpegEncodeSetting);
	JPEG_IMAGE_T *pImgSource = &(tEncodeSetting->imgSource);
    
	JPEGENCINFO_T *pjpegencinf = &(g_vpJPEG.jpegencinf);
	VP_VIDEO_T *pvideosetting = vdGetSettings();
    
   	pjpegencinf->uPixels = pvideosetting->tLocalWin.tSourceSize.usWidth;
   	pjpegencinf->uLines = pvideosetting->tLocalWin.tSourceSize.usHeight;

	/* Configure common setting */
	tEncodeSetting->eEncodeObject = C_JPEG_ENC_OBJECT_P;
	tEncodeSetting->eTablesHeaderFormat = C_JPEG_ENC_GENERAL;
	tEncodeSetting->eQTabNumber = C_JPEG_TWO_QTAB;

#if defined OPT_CAPTURE_PLANAR_YUV422
	pImgSource->eImageFormat = C_JPEG_YUV422;
#else
	pImgSource->eImageFormat = C_JPEG_YUV420;
#endif
	pImgSource->uImageWidth = (&g_vpJPEG.jpegencinf)->uPixels;
	pImgSource->uImageHeight = (&g_vpJPEG.jpegencinf)->uLines;
	tEncodeSetting->uYStride = 0x00000000;
	tEncodeSetting->uUStride = 0x00000000;
	tEncodeSetting->uVStride = 0x00000000;

	/* Configure the primary image */    
	tEncodeSetting->uPIncludeHeader = C_JPEG_ENC_P_QTAB_INCLUDE | C_JPEG_ENC_P_HTAB_INCLUDE;
	tEncodeSetting->uPRestartInterval = 4;

	/* Configure the thumbnail image */
	tEncodeSetting->uTIncludeHeader = C_JPEG_ENC_T_QTAB_INCLUDE | C_JPEG_ENC_T_HTAB_INCLUDE;
	tEncodeSetting->uTRestartInterval = 1;
	tEncodeSetting->uTOffset = 0x15000; /* About 64k bytes */
	tEncodeSetting->uTWidthDScaleF = 4;
	tEncodeSetting->uTHeightDScaleF = 4;
}

void vjpegDecInit(void)
{
 	JPEG_DECODE_SETTING_T       *tDecodeSetting = &(g_vpJPEG.tJpegDecodeSetting);

	tDecodeSetting->eDecodeImgSel = C_JPEG_DEC_P_IMAGE;
	tDecodeSetting->eDecodeErrorAction = C_JPEG_DEC_ERROR_ABORT;
	tDecodeSetting->eDecodeHTabSource = C_JPEG_DEC_USERHTAB;

	tDecodeSetting->eScaling = C_JPEG_DEC_NO_SCALING;

	tDecodeSetting->bIsSetWindow = FALSE;
}

/*
 * usage:
 *		Set source image's width and height
 */
void vjpegDecSetHW(int iWidth, int iHeight)
{
	JPEGDECINFO_T *pjpegdecinfo = &(g_vpJPEG.jpegdecinf);
	
	pjpegdecinfo->uPixels = iWidth;
	pjpegdecinfo->uLines = iHeight;
}

int SetYUVRawData(JPEG_ENCODE_SETTING_T *tJpegEncodeSetting, UINT32 uYAddr, UINT32 uYSize, UINT32 uUVSize)
{
	JPEG_IMAGE_T *pImgSource = &(tJpegEncodeSetting->imgSource);
	UINT32 width			= pImgSource->uImageWidth,
		   height			= pImgSource->uImageHeight;
	UINT32 UVHeight,	UVWidth;

	UINT32 *JPEGBitstream;
	UINT32 uYStartAddress;
	UINT32 uUStartAddress;
	UINT32 uVStartAddress;

	sysDisableIRQ ();
	if (g_vpJPEG.pJPEGEncodeBuffer != NULL)
	{
		diag_printf("g_vpJPEG.pJPEGEncodeBuffer not empty!\n");
		while(1);
	}
   	g_vpJPEG.pJPEGEncodeBuffer = bufEncJpegNew ();
   	sysEnableIRQ ();
   	
   	if(g_vpJPEG.pJPEGEncodeBuffer == NULL)
   	{
   		sysSafePrintf("g_vpJPEG.pJPEGEncodeBuffer = bufEncJpegNew () failed\n");
   		return FALSE;
   	}
    JPEGBitstream  = (UINT32 *)(g_vpJPEG.pJPEGEncodeBuffer->aucJPEGBitstream);
	uYStartAddress = NON_CACHE(uYAddr);
	uUStartAddress = NON_CACHE(uYStartAddress + uYSize);
	uVStartAddress = NON_CACHE(uUStartAddress + uUVSize);

	if(width % 16)
	    width = (width & 0xFFFFFFF0) + 16;
	if(height % 8)
	    height = (height & 0xFFFFFFF8) + 8;
	    
	if(width % 2) 
    	UVWidth = width / 2 + 1;       
	else
        UVWidth = width / 2;	    

    if(pImgSource->eImageFormat == C_JPEG_YUV420)
        UVHeight = height / 2;
    else
        UVHeight = height; 

    tJpegEncodeSetting->uYStride = width;
    tJpegEncodeSetting->uUStride = UVWidth;
    tJpegEncodeSetting->uVStride = UVWidth;
   
    tJpegEncodeSetting->uJpegBitstreamStartAddress = (UINT32)JPEGBitstream;
	pImgSource->ImgAddress.YUVAddress.uYStartAddress = (UINT32)uYStartAddress;
	pImgSource->ImgAddress.YUVAddress.uUStartAddress = (UINT32)uUStartAddress;
	pImgSource->ImgAddress.YUVAddress.uVStartAddress = (UINT32)uVStartAddress;

   return TRUE;	
}

UINT32 encodeJPEGwithThumbnailImage(UINT32 uYAddr, UINT32 uYSize, UINT32 uUVSize)
{
	UINT8 *ucQTable[] = {ucQTable0, ucQTable1, ucQTable2};
    JPEG_ENCODE_SETTING_T *tEncodeSetting = &g_vpJPEG.tJpegEncodeSetting;
	int retv;

	if(SetYUVRawData(tEncodeSetting, uYAddr, uYSize, uUVSize)==FALSE)
		return VP_JPEG_ENC_ERROR;

	jpegInitializeEngine();

	retv = jpegEncSetBasicOption(tEncodeSetting);
	if(retv != Successful)
		return (UINT32)VP_JPEG_ENC_ERROR;


	if(g_vpJPEG.bOnTheFly == TRUE)
	{
		retv = jpegEncSetMemorySWOnTheFly(TRUE, sizeof (g_vpJPEG.pJPEGEncodeBuffer->aucJPEGBitstream));
		if(retv != Successful)
			return (UINT32)VP_JPEG_ENC_ERROR;
		
		g_vpJPEG.nOnTheFlyCount = 0;
	}

	//qzhang reduce jpeg quality
	//F_JPEG_ADJUST_PQTAB(15,(g_vpJPEG.nJPEGQua * 5));
	{
		int nQua = g_vpJPEG.nJPEGQua * 2 + 9;
		if (nQua > 15) nQua = 15;
		F_JPEG_ADJUST_PQTAB(15, nQua);
		//F_JPEG_ADJUST_PQTAB(15,g_vpJPEG.nJPEGQua);
		//F_JPEG_ADJUST_PQTAB(1,(g_vpJPEG.nJPEGQua * 5));
		F_JPEG_ADJUST_TQTAB(15,15);
	}

	jpegEncSetMemoryAccessMode(C_JPEG_STILL_FIXBUFFER, 0);
	jpegSetMemoryAddress(C_JPEG_YUV_BUFFER_0, C_JPEG_BITSTREAM_BUFFER_0,
							tEncodeSetting->imgSource.ImgAddress.YUVAddress.uYStartAddress,
							tEncodeSetting->imgSource.ImgAddress.YUVAddress.uUStartAddress,
							tEncodeSetting->imgSource.ImgAddress.YUVAddress.uVStartAddress,
							tEncodeSetting->uJpegBitstreamStartAddress);

	jpegEncWriteQTable(C_JPEG_TWO_QTAB, ucQTable);

	if(g_vpJPEG.bOnTheFly == TRUE)
	{
		F_JPEG_ENABLE_AND_CLEAR_INTERRUPT(C_JPEG_ENC_INTER_COMPLETE_ENABLE|C_JPEG_INTER_OUTPUTWAIT_ENABLE);
	}
	else
	{
		F_JPEG_ENABLE_AND_CLEAR_INTERRUPT(C_JPEG_ENC_INTER_COMPLETE_ENABLE);
	}

	jpegEncStartProcess(C_JPEG_STILL_FIXBUFFER);

	/*
    retv = jpegEncWaitProcess(1);
	if(retv != Successful)
		return (UINT32)VP_JPEG_ENC_ERROR;
	*/
	vjpegWaitEncoderCom();

	if(g_vpJPEG.bOnTheFly == TRUE)
	{
		if(g_vpJPEG.nOnTheFlyCount > 1)
		{
			return (UINT32)VP_JPEG_ENC_ERROR;
		}
	}

	tEncodeSetting->imgPrimary.ImgAddress.JpegAddress.uImgSize = F_JPEG_GET_P_IMAGESIZE;
	tEncodeSetting->imgThumbnail.ImgAddress.JpegAddress.uImgSize = F_JPEG_GET_T_IMAGESIZE;
    g_vpJPEG.pJPEGEncodeBuffer->uJPEGDatasize = F_JPEG_GET_P_IMAGESIZE;

	/*
	printf("Pimage address= %x, length= %x.\nTimage address= %x, length= %x.\n",
								tEncodeSetting->imgPrimary.ImgAddress.JpegAddress.uStartAddress,
								tEncodeSetting->imgPrimary.ImgAddress.JpegAddress.uImgSize,
								tEncodeSetting->imgThumbnail.ImgAddress.JpegAddress.uStartAddress,
								tEncodeSetting->imgThumbnail.ImgAddress.JpegAddress.uImgSize);
	*/
	return (UINT32)VP_JPEG_ENC_OK;
}

#ifndef JPEG_DEC_WITH_MP4_BUFFER
void SetRGBRawData(JPEG_DECODE_SETTING_T *tDecodeSetting, VP_BUFFER_JPEG_DECODER_T *pJpeg)
#else
void SetRGBRawData(JPEG_DECODE_SETTING_T *tDecodeSetting, VP_BUFFER_MP4_DECODER_T *pJpeg)
#endif
{
	UINT32 *JPEGBitstream;
	UINT32 uYStartAddress;
	UINT32 uUStartAddress;
	UINT32 uVStartAddress;

    if(tDecodeSetting == NULL || pJpeg == NULL)
    {
    	diag_printf("SetRGBRawData(): arguments is NULL\n");
    	return;
    }

#ifndef JPEG_DEC_WITH_MP4_BUFFER
	/* jpeg decode buffer */
    JPEGBitstream  = (UINT32 *)NON_CACHE(pJpeg->pSrcImage);
	uYStartAddress = (UINT32 )pJpeg->aucY;
	uUStartAddress = (UINT32 )pJpeg->aucU;
	uVStartAddress = (UINT32 )pJpeg->aucV;
#else
	/* mp4 decode buffer */
    JPEGBitstream  = (UINT32 *)NON_CACHE(pJpeg->aucBitstream);
	uYStartAddress = (UINT32)pJpeg->aucOutput;
	uUStartAddress = (UINT32)((char*)uYStartAddress+vjpegdecGetYSize(&g_vpJPEG.jpegdecinf));
	uVStartAddress = (UINT32)((char*)uUStartAddress+vjpegdecGetUVSize(&g_vpJPEG.jpegdecinf));
#endif
    
	tDecodeSetting->uJpegBitstreamStartAddress = (UINT32)JPEGBitstream;
	tDecodeSetting->imgOutput.ImgAddress.YUVAddress.uYStartAddress = (UINT32)uYStartAddress;
	tDecodeSetting->imgOutput.ImgAddress.YUVAddress.uUStartAddress = (UINT32)uUStartAddress;
	tDecodeSetting->imgOutput.ImgAddress.YUVAddress.uVStartAddress = (UINT32)uVStartAddress;
}

int g_DecodeStatus = false;

#ifndef JPEG_DEC_WITH_MP4_BUFFER
UINT32 decodeJPEGPrimaryImage(VP_BUFFER_JPEG_DECODER_T *pJpeg)
#else
UINT32 decodeJPEGPrimaryImage(VP_BUFFER_MP4_DECODER_T *pJpeg)
#endif
{
    JPEG_DECODE_SETTING_T *tDecodeSetting = &(g_vpJPEG.tJpegDecodeSetting);
    JPEGDECINFO_T *pjpegdecinf = &(g_vpJPEG.jpegdecinf);
	int retv;

	/* Set default values for decoder. */
	SetRGBRawData(tDecodeSetting, pJpeg);

	/* Initialization for encoder or decoder */
	jpegInitializeEngine();

	retv = jpegDecSetBasicOption(tDecodeSetting);
	if(retv != Successful)
	{
		return (UINT32)VP_JPEG_DEC_ERROR;
	}

	jpegSetMemoryAddress(C_JPEG_YUV_BUFFER_0, C_JPEG_BITSTREAM_BUFFER_0,
						 tDecodeSetting->imgOutput.ImgAddress.YUVAddress.uYStartAddress,
						 tDecodeSetting->imgOutput.ImgAddress.YUVAddress.uUStartAddress,
						 tDecodeSetting->imgOutput.ImgAddress.YUVAddress.uVStartAddress,
						 tDecodeSetting->uJpegBitstreamStartAddress);

	/* Enable and Clear the Interrupt */
	F_JPEG_ENABLE_AND_CLEAR_INTERRUPT(C_JPEG_DEC_INTER_COMPLETE_ENABLE | C_JPEG_DEC_INTER_ERROR_ENABLE);

	/* Do decoding procedure */
	jpegDecStartProcess();

	/*
    retv = jpegDecWaitProcess(1);
	if(retv != Successful)
	{
		return (UINT32)VP_JPEG_DEC_ERROR;
	}
	*/
	vjpegWaitDecoderCom();
	if(g_DecodeStatus == false)
	{
		printf("decodeJPEGPrimaryImage(): decode error\n");
		return VP_JPEG_DEC_ERROR;
	}

	jpegDecGetImage(&g_vpJPEG.tJpegDecodeSetting);
	pjpegdecinf->uPixels = g_vpJPEG.tJpegDecodeSetting.imgOutput.uImageWidth;
	pjpegdecinf->uLines = g_vpJPEG.tJpegDecodeSetting.imgOutput.uImageHeight;

	return (UINT32)VP_JPEG_DEC_OK;
}


UINT32 vjpegEncoderCom_Callback(void)
{
#ifndef ECOS
	tt_sem_up(&g_vpJPEG.semEncoder);
#else
	cyg_semaphore_post(&g_vpJPEG.semEncoder);
#endif
	return 0;
}

UINT32 vjpegDecoderCom_Callback(void)
{
#ifndef ECOS
	tt_sem_up(&g_vpJPEG.semDecoder);
#else
	g_DecodeStatus = true;
	cyg_semaphore_post(&g_vpJPEG.semDecoder);
#endif
	return 0;
}

UINT32 vjpegDecoderErr_Callback(void)
{
#ifndef ECOS
	tt_sem_up(&g_vpJPEG.semDecoder);
#else
	g_DecodeStatus = false;
	cyg_semaphore_post(&g_vpJPEG.semDecoder);
#endif
	return 0;
}

UINT32 vjpegOnTheFlyCom_Callback(void)
{
	g_vpJPEG.nOnTheFlyCount++;
	return 0;
}

void vjpegWaitEncoderCom(void)
{
#ifndef ECOS
	tt_sem_down(&g_vpJPEG.semEncoder);
#else
	cyg_semaphore_wait(&g_vpJPEG.semEncoder);
#endif
}

void vjpegWaitDecoderCom(void)
{
#ifndef ECOS
	tt_sem_down(&g_vpJPEG.semDecoder);
#else
	cyg_semaphore_wait(&g_vpJPEG.semDecoder);
#endif
}

void vjpegAddEncBuffer(void)
{
	sysDisableIRQ ();
    listAttach(&g_vpJPEG.listEncodedJPEG, &g_vpJPEG.pJPEGEncodeBuffer->list);
    g_vpJPEG.pJPEGEncodeBuffer = NULL;
    sysEnableIRQ ();
}

void vjpegSendEncBuffer(void)
{
	sysSafePrintf ("Set jpeg intr\n");
#ifndef ECOS
	__cmd18_03_EnableCommandInterrupt_CheckIntr ();
#else
	//iothread_SendNotify(MEDIA_TYPE_JPEG);
	iothread_EventNotify();
#endif
}

VP_BUFFER_JPEG_ENCODER_T *vjpegGetEncBuffer(void)
{
 	VP_BUFFER_JPEG_ENCODER_T *pReturn;
	LIST_T *pNode;
    
	sysDisableIRQ ();
    pNode = g_vpJPEG.listEncodedJPEG.pNext;
 	if (pNode == &g_vpJPEG.listEncodedJPEG)
		pReturn = NULL;
	else
	{
		pReturn = GetParentAddr (pNode, VP_BUFFER_JPEG_ENCODER_T, list);
		sysSafePrintf ("Get buffer: %d\n", &pReturn->list);
	}

	sysEnableIRQ ();
    return pReturn;
}


void vjpegClearBuffer (void)
{
	sysDisableIRQ ();
	while (1)
	{
		VP_BUFFER_JPEG_ENCODER_T *pBitstream;
			
		
		pBitstream = vjpegGetEncBuffer ();
		if (pBitstream == NULL)
			break;
		else
		{
			listDetach (&pBitstream->list);
			vjpegFreeEncBuffer (pBitstream);
		}
	}
	
	if (g_vpJPEG.pJPEGEncodeBuffer != NULL)
	{
		bufEncJpegDecRef (g_vpJPEG.pJPEGEncodeBuffer);	
		g_vpJPEG.pJPEGEncodeBuffer = NULL;
	}

	sysEnableIRQ ();
	
	//iothread_ClearNotify(MEDIA_TYPE_JPEG);
}


VP_BUFFER_JPEG_ENCODER_T *vjpegNewEncBuffer(void)
{
    return bufEncJpegNew();
}

void vjpegFreeEncBuffer(VP_BUFFER_JPEG_ENCODER_T *pEncBuf)
{
	if(pEncBuf == NULL) return;
    bufEncJpegDecRef(pEncBuf);
}

#ifndef JPEG_DEC_WITH_MP4_BUFFER
VP_BUFFER_JPEG_DECODER_T *vjpegNewDecBuffer(void)
{
    return bufDecJpegNew();
}

void vjpegFreeDecBuffer(VP_BUFFER_JPEG_DECODER_T *pDecBuf)
{
	if(pDecBuf == NULL) return;
    bufDecJpegDecRef(pDecBuf);
}
#endif


void vjpegLock (void)
{
#ifndef ECOS
	tt_rmutex_lock (&g_vpJPEG.mtLock);
#else
	cyg_mutex_lock (&g_vpJPEG.mtLock);
#endif
}

BOOL vjpegTryLock (void)
{
#ifndef ECOS
	if(tt_rmutex_try_lock (&g_vpJPEG.mtLock)==0)
#else
	if(cyg_mutex_trylock (&g_vpJPEG.mtLock)==true)
#endif
		return TRUE;
	else
		return FALSE;
}

void vjpegUnlock (void)
{
	//g_vpJPEG.nRefCount = 0;
#ifndef ECOS
	tt_rmutex_unlock (&g_vpJPEG.mtLock);
#else
	cyg_mutex_unlock (&g_vpJPEG.mtLock);
#endif
}

JPEGDECINFO_T *vjpegdecGetInfo (void)
{
	return &g_vpJPEG.jpegdecinf;
}

UINT32 vjpegdecGetYSize (JPEGDECINFO_T *pJPEGInfo)
{
	return pJPEGInfo->uLines * pJPEGInfo->uPixels;
}

UINT32 vjpegdecGetUVSize (JPEGDECINFO_T *pJPEGInfo)
{
	return vjpegdecGetYSize (pJPEGInfo) /
	//return MAX_MP4_WIDTHxHEIGHT /
#if defined OPT_CAPTURE_PLANAR_YUV420
		4
#elif defined OPT_CAPTURE_PLANAR_YUV422
		2
#endif
	;
}

void vjpegSetQuality(UINT32 uQua)
{
	g_vpJPEG.nJPEGQua = uQua;
}

UINT32 vjpegGetQuality(void)
{
	return g_vpJPEG.nJPEGQua;
}

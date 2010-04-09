#include "../../Inc/CommonDef.h"


#ifdef USE_W99702_CAMERA

extern void RtspThreadRun(W99702_DATA_EXCHANGE_T* g_dataW99702, CMD_AUDIO_FORMAT_E audiotype, CMD_VIDEO_FORMAT_E videotype);

BOOL W99702_GetOneImage(char *pcImgBuf, int iImgBufMaxLen, int *piImgBufLen,W99702_DATA_EXCHANGE_T* ExData,BOOL RecordInit)
{
	//VP_BUFFER_MP4_BITSTREAM_T			*pMP4Buf = NULL;
	VP_BUFFER_JPEG_ENCODER_T			*pJPEGBuf = NULL;
	//VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T	*pAudioBuf = NULL;
	//int									nMotionCount;
	
	//iothread_EventRead(&pMP4Buf, &pJPEGBuf, &pAudioBuf, &nMotionCount);
	
	//diag_printf("Before get jpage\n");
	iothread_EventRead(NULL, &pJPEGBuf, NULL, NULL);
	//diag_printf("Before get jpage1\n");
	if (pJPEGBuf == NULL)
		return FALSE;
	//diag_printf("Before get jpage2\n");

	if (pJPEGBuf->uJPEGDatasize > iImgBufMaxLen)
		return FALSE;
	
	memcpy(pcImgBuf, (UCHAR*)NON_CACHE(pJPEGBuf->aucJPEGBitstream), pJPEGBuf->uJPEGDatasize);
	*piImgBufLen = pJPEGBuf->uJPEGDatasize;
	
	bufEncJpegDecRef(pJPEGBuf);
	return (pJPEGBuf != NULL ? TRUE : FALSE);
}


static void W99702_InitParam ()
{	
/*
	if (g_ConfigParam.eVideoFormat != CMD_VIDEO_H263
		&& g_ConfigParam.eVideoFormat != CMD_VIDEO_MP4)
		g_ConfigParam.eVideoFormat = CMD_VIDEO_MP4;
	if (g_ConfigParam.eAudioFormat != CMD_AUDIO_AMR
		&& g_ConfigParam.eAudioFormat != CMD_AUDIO_IMA_ADPCM)
		g_ConfigParam.eAudioFormat = CMD_AUDIO_IMA_ADPCM;
*/	
	wb702SetVideoBitRate (32*1024);	
	wb702SetLocalVideoSource ( 176, 144, CMD_ROTATE_NORMAL);
	wb702SetFrame( WB_READ_FRAME);
	wb702SetVideoFormat (g_ConfigParam.eVideoFormat);
	wb702SetAudioType (g_ConfigParam.eAudioFormat, CMD_AUDIO_PCM);
	wb702EnableMP4Encoder(TRUE);
    wb702EnableAudioEncoder(TRUE);
	wb702SetJPEG ( TRUE);
	vp_bitrate_control_change_enable(TRUE);
}

BOOL W99702_OpenCamera(W99702_DATA_EXCHANGE_T* pExData,UsedBuf* asbuf,unsigned int *aslen)
{

    //cyg_mutex_init(&pExData->lock);
	//tt_pc_init (&pExData->pcVideo, 1);
	//cyg_semaphore_wait(&(pExData->pcVideo).producer);
	
	//tt_pc_init (&pExData->pcAudio, 1);
	//pExData->uAudioFrames = 0;
	//pExData->uVideoFrames = 0;

	/* Init devices. */
	W99702_InitParam ();
	
	

	/* Init motion detect data. */
	g_WebCamState.iMP4HeaderLen = 0;
	g_WebCamState.bAsfMP4Header = FALSE;
	
	RtspThreadRun(pExData,g_ConfigParam.eAudioFormat,g_ConfigParam.eVideoFormat);	
	return TRUE;
}

BOOL W99702_CloseCamera(W99702_DATA_EXCHANGE_T* pExData)
{
	diag_printf("Before Delete RTSP Server!\n");
	RtspThreadRelease();
	diag_printf("Delete RTSP Server!\n");

	return TRUE;
}

BOOL __W99702_SetImageQuality(int iQuality, int iResX, int iResY,W99702_DATA_EXCHANGE_T* pExData)
{
	if (iQuality >= 3)
		return FALSE;
	else
	{
		const int aaBitrateTable[4][3] =
		{
			{64*1024,	128*1024,	192*1024},	//176 * 144
			{256*1024,	512*1024,	768*1024},	//320 * 240
			{256*1024,	512*1024,	768*1024},	//352 * 288
			{512*1024,	1024*1024,	2304*1024},	//640 * 480
		};
		wb702SetJPEGQuality(iQuality);	
		if (iResX <= 176)		//176 * 144
		{ 
			diag_printf ("Set quality %d for (%d * %d) => %d\n", iQuality, iResX, iResY, aaBitrateTable[0][iQuality]);			
			wb702SetVideoBitRate ( aaBitrateTable[0][iQuality]);			
		}
		else if (iResX <= 320)	//320 * 240
		{
			diag_printf ("Set quality %d for (%d * %d) => %d\n", iQuality, iResX, iResY, aaBitrateTable[1][iQuality]);
			wb702SetVideoBitRate (aaBitrateTable[1][iQuality]);
		}
		else if (iResX <= 352)	//352 * 288
		{
			diag_printf ("Set quality %d for (%d * %d) => %d\n", iQuality, iResX, iResY, aaBitrateTable[2][iQuality]);
			wb702SetVideoBitRate ( aaBitrateTable[2][iQuality]);
		}
		else					//640 * 480
		{
			diag_printf ("Set quality %d for (%d * %d) => %d\n", iQuality, iResX, iResY, aaBitrateTable[3][iQuality]);
			wb702SetVideoBitRate (aaBitrateTable[3][iQuality]);
		}
	}

	return TRUE;
}




BOOL W99702_SetFramerate(int iFramerate,W99702_DATA_EXCHANGE_T* pExData)
{
	BOOL bRt;
	int iFramerate_old;
	
	//cyg_mutex_lock (&pExData->lock);
	diag_printf ("Framerate: %d\n", iFramerate);

	W99702_GetFramerate ( &iFramerate_old);
	if (iFramerate != iFramerate_old)
	{
		if (iFramerate > 0)
			bRt = wb702SetVideoFramerate (iFramerate);
		else
			bRt = FALSE;
	}
	else
		bRt = TRUE;
	
	//cyg_mutex_unlock (&pExData->lock);

	return bRt;
}


BOOL W99702_GetFramerate(int *piFramerate)
{
	if (piFramerate != NULL)
		*piFramerate = (int) g_WebCamState.ucFramerate;
	return TRUE;
}


BOOL W99702_SetImageQuality( int iQuality,W99702_DATA_EXCHANGE_T* pExData)
{
	BOOL bRt;
	int iQuality_old;
	int iResX, iResY;
	
	//cyg_mutex_lock (&pExData->lock);	
	diag_printf ("Quality: %d\n", iQuality);
	
	W99702_GetImageQuality ( &iQuality_old);
	if (iQuality != iQuality_old)
	{
		W99702_GetImageResolution ( &iResX, &iResY);
		bRt = __W99702_SetImageQuality (iQuality, iResX, iResY,pExData);
	}
	else
		bRt = TRUE;
	
	//cyg_mutex_unlock (&pExData->lock);
	
	return bRt;
}

BOOL W99702_GetBrightness(int *piBrightness)
{
	if (piBrightness != NULL)
		*piBrightness = (int) g_WebCamState.ucBright;
	return TRUE;
}


BOOL W99702_SetImageBrightness( int iBright,W99702_DATA_EXCHANGE_T* pExData)
{
	BOOL bRt;
	int iBright_old;
	
	//cyg_mutex_lock (&pExData->lock);	
	
	W99702_GetBrightness ( &iBright_old);
	if (iBright != iBright_old)
	{
#if 0	//xhchen
		bRt = TRUE;//wb702SetVideoContrastBright(0,iBright); 
#else
		diag_printf ("Change Brightness to: %d\n", iBright);
		bRt = wb702SetVideoContrastBright(0,iBright); 
#endif
	}
	else
		bRt = TRUE;
	
	//cyg_mutex_unlock (&pExData->lock);
	
	return bRt;
}

BOOL W99702_GetSpeakerVolume(int *piVolume)
{
	if (piVolume != NULL)
		*piVolume = (int) g_WebCamState.ucSpeakerVolume;
	return TRUE;
}

BOOL W99702_SetSpeakerVolume( int iVolume,W99702_DATA_EXCHANGE_T* pExData)
{
	BOOL bRt;
	int iVolume_old;
	
	//cyg_mutex_lock (&pExData->lock);	
	diag_printf ("Speaker Volume: %d\n", iVolume);
	
	W99702_GetSpeakerVolume( &iVolume_old);
	if (iVolume != iVolume_old)
	{		
		bRt = wb702SetAudioPlayVolume(iVolume, iVolume);     //调整板子端speaker播放的声音大小
	}
	else
		bRt = TRUE;
	
	//cyg_mutex_unlock (&pExData->lock);
	
	return bRt;
}

BOOL W99702_GetMicVolume(int *piVolume)
{
	if (piVolume != NULL)
		*piVolume = (int) g_WebCamState.ucMicVolume;
	return TRUE;
}
BOOL W99702_SetMicVolume( int iVolume,W99702_DATA_EXCHANGE_T* pExData)
{
	BOOL bRt;
	int iVolume_old;
	
	//cyg_mutex_lock (&pExData->lock);	
	diag_printf ("Mic Volume: %d\n", iVolume);
	
	W99702_GetMicVolume( &iVolume_old);
	if (iVolume != iVolume_old)
	{		
		bRt = wb702SetAudioRecordVolume(iVolume, iVolume); //调整板子端Mic录制的声音大小
	}
	else
		bRt = TRUE;
	
	//cyg_mutex_unlock (&pExData->lock);
	
	return bRt;
}


BOOL W99702_GetImageQuality(int *piQuality)
{
	if (piQuality != NULL)
		*piQuality = (int) g_WebCamState.ucCompressionRatio;
	return TRUE;	
}

BOOL W99702_SetImageResolution(int iResX, int iResY,W99702_DATA_EXCHANGE_T* pExData)
{
	int iResX_old, iResY_old;
	BOOL bRt = FALSE;
	printf ("Resolution: %d %d\n", iResX, iResY);
	
    //cyg_mutex_lock (&pExData->lock);
	
	W99702_GetImageResolution ( &iResX_old, &iResY_old);
	if (iResX != iResX_old || iResY != iResY_old)
	{
		int iQuality;
		wb702SetJPEG ( FALSE);
		wb702EnableMP4Encoder ( FALSE);
		wb702SetLocalVideoSource (iResX, iResY, CMD_ROTATE_NORMAL);
		
		W99702_GetImageQuality( &iQuality);
		bRt = __W99702_SetImageQuality (iQuality, iResX, iResY,pExData);
		wb702EnableMP4Encoder (TRUE);
		wb702SetJPEG (TRUE);
	}
	
    //cyg_mutex_unlock (&pExData->lock);
    
	
	return bRt;
}

BOOL W99702_GetImageResolution(int *piResX, int *piResY)
{
	switch (g_WebCamState.ucResolution)
	{
		case 0:
			if (piResX != NULL) *piResX = 176;
			if (piResY != NULL) *piResY = 144;
			break;
		case 1:
			if (piResX != NULL) *piResX = 320;
			if (piResY != NULL) *piResY = 240;
			break;
		case 2:
			if (piResX != NULL) *piResX = 352;
			if (piResY != NULL) *piResY = 288;
			break;
		case 3:
			if (piResX != NULL) *piResX = 640;
			if (piResY != NULL) *piResY = 480;
			break;
		default:
			return FALSE;
	}
	return TRUE;	
}

#endif


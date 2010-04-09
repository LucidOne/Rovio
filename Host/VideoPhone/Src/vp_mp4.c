#include "../Inc/inc.h"


typedef struct
{
#ifndef ECOS
	TT_RMUTEX_T		mtLock;
	TT_RMUTEX_T		mtLockEncoder;
	TT_RMUTEX_T		mtLockDecoder;
#else
	cyg_mutex_t		mtLock;
	cyg_mutex_t		mtLockEncoder;
	cyg_mutex_t		mtLockDecoder;
#endif
	int				nRefCount;
	int				nRefCount_Encoder;
	int				nRefCount_Decoder;

	MP4ENCINFO_T	mp4encinf;
	MP4DECINFO_T	mp4decinf;

	UINT32				uEncFrameRateStatic;	/* MP4 engine's encoding frame rate (2~99)*/
	UINT32				uEncFrameRate;			/* MP4 outputing frame rate (1~99)*/
	UINT32				uEncBitRate;
	CMD_VIDEO_FORMAT_E	eEncFormat;
	UINT32				uEncIFrameNum_Hacker;
	UINT32				uEncIFrameNum_TimeHacker;
	BOOL				uEncIFrameNum_Enable_TimeHacker;
	
	VP_BUFFER_MP4_ENCODER_T	*pMP4EncoderBuffer;
	VP_BUFFER_MP4_DECODER_T *pMP4DecoderBuffer;
	BOOL bDecInit;
	
	VP_BUFFER_MP4_BITSTREAM_T *apMP4EnocderBitstream[2]; // In encoding
	LIST_T					listBitstream; // Encoded (encode complete)
	
	TT_PC_T					pcEncFlag;
	TT_PC_T					pcDecFlag;
	VP_MP4_ENC_STATE_E		eEncState;
	VP_MP4_DEC_STATE_E		eDecState;
	
	int nEncodeIndex;
} VP_MP4_T;


#pragma arm section zidata = "non_init"
static VP_MP4_T g_vpMP4;
#pragma arm section zidata


typedef struct VP_MP4_SPLITTER_T VP_MP4_SPLITTER_T;
static void mp4Splitter_Init (VP_MP4_SPLITTER_T *pSplitter, char *pcBuffer, size_t szMaxLength)
{
	pSplitter->bHeader			= TRUE;
	pSplitter->pcBuffer			= pcBuffer;
	pSplitter->szMaxLength		= szMaxLength;
	pSplitter->szLength			= 0;
	pSplitter->pcCheckPointer	= pcBuffer;
	pSplitter->szTotalLength	= 0;
}


static void mp4Splitter_DelData (VP_MP4_SPLITTER_T *pSplitter, size_t szLength)
{
	if (szLength > pSplitter->szLength)
		szLength = pSplitter->szLength;
	
	memmove (pSplitter->pcBuffer,
		pSplitter->pcBuffer + szLength,
		pSplitter->szLength - szLength);
	pSplitter->szLength			-= szLength;
	pSplitter->pcCheckPointer	-= szLength;
	
	if (pSplitter->pcCheckPointer < pSplitter->pcBuffer)
		pSplitter->pcCheckPointer = pSplitter->pcBuffer;
}


static size_t mp4Splitter_GetFrame (VP_MP4_SPLITTER_T *pSplitter)
{
	if (pSplitter->szLength > 10)
	{
		char *pcEnd	= pSplitter->pcBuffer + pSplitter->szLength - 10;
		char *pc		= pSplitter->pcCheckPointer;
		for (; pc < pcEnd; pc++)
		{
			if (pc[0] == (char) 0x00 && pc[1] == (char) 0x00)
			{
				/* MP4_VISUAL_OBJECT_SEQUENCE_START_CODE: 0x000001B0 */
				if (pc[2] == (char) 0x01 && pc[3] == (char) 0xB0
					&& pc[5] == (char) 0x00 && pc[6] == (char) 0x00 && pc[7] == (char) 0x01 && pc[8] == (char) 0xB5)
				{
					pSplitter->bHeader = TRUE;
					break;
				}
				
				/* MP4_VIDEO_OBJECT_PLANE_START_CODE: 0x000001B6 */
				if (pc[2] == (char) 0x01 && pc[3] == (char) 0xB6)
				{
					if (pSplitter->bHeader == TRUE)
						pSplitter->bHeader = FALSE;
					else
						break;
				}
				
				/* H.263 */
				if ((pc[2] & 0xF0) != 0)
					break;
			}
		}
		
		if (pc < pcEnd)
		{/* Frame found! */
			pSplitter->pcCheckPointer = pc + 2;
			
			pSplitter->szGetTLength = pSplitter->szTotalLength - pSplitter->szLength;
			return pc - pSplitter->pcBuffer;
		}
		else
		{/* Frame not found! */
			pSplitter->pcCheckPointer = pc;
		}
	}
	
	return 0;
}


static void mp4Splitter_AddData (VP_MP4_SPLITTER_T *pSplitter, const char *cpcBuffer, size_t szLength)
{
	/* Make sure that there is enough buffer to add the data. */
	if (szLength > pSplitter->szMaxLength)
		return;
	
	if (pSplitter->szLength + szLength > pSplitter->szMaxLength)
	{
		mp4Splitter_DelData (pSplitter, pSplitter->szLength + szLength - pSplitter->szMaxLength);
	}
	
	/* Copy the data. */
	memmove (pSplitter->pcBuffer + pSplitter->szLength,
		cpcBuffer,
		szLength);
	pSplitter->szLength += szLength;

	/* Check the valid MP4/H.263 clip. */
	
	pSplitter->szTotalLength += szLength;
}








static void vmp4Callback_Null (void)
{
}


static void vmp4encDoneCallback (void)
{
	VP_BUFFER_MP4_BITSTREAM_T *pBitstream = g_vpMP4.apMP4EnocderBitstream[0];
	
	ASSERT (pBitstream != NULL);
	//PTL;
	g_vpMP4.eEncState = VP_MP4_ENC_OK;
	
	pBitstream->uLength = mp4GetEncBitStreamLength() + VP_BUF_APPEND_SIZE;
	//sysSafePrintf ("Encode length: %d\n", pBitstream->uLength);

	if (pBitstream->uLength > 0)
	{
		pBitstream->nIndex = g_vpMP4.nEncodeIndex++;
#ifdef MP4_TEST_CHECKSUM
		{
			UINT32 mp4GetChecksum (CONST UCHAR *cpucData, UINT32 uLength);
			//UINT32 uCheck = mp4GetChecksum ((UCHAR *) NON_CACHE (pBitstream->aucBitstream), 
			//								pBitstream->uLength);
			memcpy((UCHAR *) NON_CACHE (pBitstream->aucBitstream) + pBitstream->uLength, 
					MP4ADDATIONBEGIN, strlen(MP4ADDATIONBEGIN));
			pBitstream->uLength += strlen(MP4ADDATIONBEGIN);
			
			//memcpy ((UCHAR *) NON_CACHE (pBitstream->aucBitstream) + pBitstream->uLength, &uCheck, sizeof (UINT32));
			memcpy ((UCHAR *) NON_CACHE (pBitstream->aucBitstream) + pBitstream->uLength, 
					&pBitstream->uLength, sizeof (UINT32));
			//memset ((UCHAR *) NON_CACHE (pBitstream->aucBitstream) + pBitstream->uLength + 2, 
			//		0xFF, sizeof (UINT32) - 2);
			pBitstream->uLength += sizeof (UINT32);
	
			memcpy ((UCHAR *) NON_CACHE (pBitstream->aucBitstream) + pBitstream->uLength, 
					&pBitstream->nIndex, sizeof (int));
			//memset ((UCHAR *) NON_CACHE (pBitstream->aucBitstream) + pBitstream->uLength + 1, 
			//		0xFF, sizeof (int) - 1);	
			pBitstream->uLength += sizeof (int);
		}
#endif

		
	
		if (g_vpMP4.apMP4EnocderBitstream[1] != NULL)
		{
			/* Calculate time interval. */
			UINT32 uTimeInterval = g_vpMP4.apMP4EnocderBitstream[0]->tTimeStamp
				- g_vpMP4.apMP4EnocderBitstream[1]->tTimeStamp;
			if ((uTimeInterval & 0x0FFFF) == 0)
				uTimeInterval = 1000 / g_vpMP4.uEncFrameRate;
		
			/* Pop apMP4EnocderBitstream[1]. */
			pBitstream = g_vpMP4.apMP4EnocderBitstream[1];
			
			/* Append time interval. */
			//memcpy ((UCHAR *) NON_CACHE (pBitstream->aucBitstream) + pBitstream->uLength, &uTimeInterval, 2);
			//pBitstream->uLength += 2;
			
			/* Append timestamp */
			memcpy ((UCHAR *) NON_CACHE (pBitstream->aucBitstream) + pBitstream->uLength, &pBitstream->tTimeStamp, 8);
			pBitstream->uLength += 8;
			
#ifdef MP4_TEST_CHECKSUM
			memcpy((UCHAR *) NON_CACHE (pBitstream->aucBitstream) + pBitstream->uLength, 
					MP4ADDATIONEND, strlen(MP4ADDATIONEND));
			pBitstream->uLength += strlen(MP4ADDATIONEND);
#endif
			listAttach (&g_vpMP4.listBitstream, &pBitstream->list);
			//sysSafePrintf ("Attach: %d\n", &pBitstream->list);
		}
			
		/* Push apMP4EnocderBitstream[0]. */
		g_vpMP4.apMP4EnocderBitstream[1] = g_vpMP4.apMP4EnocderBitstream[0];
	}
	else
		bufMP4BitstreamDecRef (pBitstream);
	
	tt_pc_try_consume (&g_vpMP4.pcEncFlag, NULL, NULL);
	g_vpMP4.apMP4EnocderBitstream[0] = NULL;
}


static void vmp4encErrorCallback (void)
{
	VP_BUFFER_MP4_BITSTREAM_T *pBitstream = g_vpMP4.apMP4EnocderBitstream[0];
	g_vpMP4.apMP4EnocderBitstream[0] = NULL;
	
	ASSERT (pBitstream != NULL);
	
	PTL;
	g_vpMP4.eEncState = VP_MP4_ENC_ERROR;

	bufMP4BitstreamDecRef (pBitstream);
	
	//mp4ResetEncoder ();
	tt_pc_try_consume (&g_vpMP4.pcEncFlag, NULL, NULL);
}


static void vmp4decDoneCallback (void)
{
	//UINT32 vti = vmp4GetCurrentVTI();
	//UINT32 size = vmp4GetDecBitstreamLength();		
	//UINT32 Yaddr = vmp4GetCurrentDisplaybufAddr();
	//PTL;
	//sysSafePrintf ("Dec: %d %d %d\n", vti, size, Yaddr);
	//PTL;
	g_vpMP4.eDecState = VP_MP4_DEC_OK;
	tt_pc_try_consume (&g_vpMP4.pcDecFlag, NULL, NULL);
}

static void vmp4decWaitCallback (void)
{
	PTL;
	g_vpMP4.eDecState = VP_MP4_DEC_WAIT;
	
	//g_vpMP4.eDecState = VP_MP4_DEC_ERROR;
	//mp4ResumeDecoder ();
	mp4ResetDecoder ();
	
	tt_pc_try_consume (&g_vpMP4.pcDecFlag, NULL, NULL);
}

static void vmp4decErrorCallback (void)
{
	PTL;
	mp4ResumeDecoder ();
	//mp4ResetDecoder ();
	
	g_vpMP4.eDecState = VP_MP4_DEC_ERROR;
	tt_pc_try_consume (&g_vpMP4.pcDecFlag, NULL, NULL);
}




void vmp4Init(void)
{
#ifndef ECOS
	tt_rmutex_init (&g_vpMP4.mtLock);
	tt_rmutex_init (&g_vpMP4.mtLockEncoder);
	tt_rmutex_init (&g_vpMP4.mtLockDecoder);
#else
	cyg_mutex_init (&g_vpMP4.mtLock);
	cyg_mutex_init (&g_vpMP4.mtLockEncoder);
	cyg_mutex_init (&g_vpMP4.mtLockDecoder);
#endif
	g_vpMP4.nRefCount			= 0;
	g_vpMP4.nRefCount_Encoder	= 0;
	g_vpMP4.nRefCount_Decoder	= 0;
	
	g_vpMP4.uEncFrameRateStatic	= 30;
	g_vpMP4.uEncFrameRate		= 30;
	g_vpMP4.uEncBitRate			= 64 * 1024;
	g_vpMP4.eEncFormat			= CMD_VIDEO_H263;

	g_vpMP4.pMP4EncoderBuffer = bufMP4EncoderNew ();
#if defined OPT_USE_VIDEO_ENCODER
	ASSERT (g_vpMP4.pMP4EncoderBuffer != NULL);
#endif

	g_vpMP4.pMP4DecoderBuffer = bufMP4DecoderNew ();
#if defined OPT_USE_VIDEO_DECODER
	ASSERT (g_vpMP4.pMP4DecoderBuffer != NULL);
	
	g_vpMP4.pMP4DecoderBuffer->nOutputLength = 0;
#endif
	

	g_vpMP4.apMP4EnocderBitstream[0] = NULL;
	g_vpMP4.apMP4EnocderBitstream[1] = NULL;
	listInit (&g_vpMP4.listBitstream);

	tt_pc_init (&g_vpMP4.pcEncFlag, 1);
	tt_pc_init (&g_vpMP4.pcDecFlag, 1);
}


/* Reference count for MP4 engine. */
static void vmp4AddRef (void)
{
	if (g_vpMP4.nRefCount++ == 0)
	{
		TURN_ON_MPEG4_CLOCK;
		tt_msleep (30);
	}
}


static void vmp4DecRef (void)
{
	ASSERT (g_vpMP4.nRefCount > 0);
	if (--g_vpMP4.nRefCount == 0)
	{
		TURN_OFF_MPEG4_CLOCK;
	}
}

/*
 * bHeader_Tailer:	header or tailer (TURE=header, FALSE=tailer)
 *
 */
VP_BUFFER_MP4_BITSTREAM_T *__vmp4encGetBuffer (BOOL bHeader_Tailer)
{
	VP_BUFFER_MP4_BITSTREAM_T *pReturn;
	LIST_T *pNode;
	sysDisableIRQ ();
	
	if (bHeader_Tailer)
		pNode = g_vpMP4.listBitstream.pNext;
	else
		pNode = g_vpMP4.listBitstream.pPrev;
	if (pNode == &g_vpMP4.listBitstream)
		pReturn = NULL;
	else
	{
		pReturn = GetParentAddr (pNode, VP_BUFFER_MP4_BITSTREAM_T, list);
		//sysSafePrintf ("Get buffer: %d\n", &pReturn->list);
	}
	sysEnableIRQ ();
	return pReturn;
}


VP_BUFFER_MP4_BITSTREAM_T *vmp4encGetBuffer (void)
{
	return __vmp4encGetBuffer (TRUE);
}



/* Reference count for MP4 encoder. */
void vmp4encAddRef (VP_VIDEO_T *pVideo)
{
	vmp4AddRef ();
	if (g_vpMP4.nRefCount_Encoder++ == 0)
	{/* Set encoder. */
		UINT32 uHeadBitstreamLength;
		VP_BUFFER_MP4_BITSTREAM_T *pBitstream;

		mp4InstallCallBackFunction (MP4_ENCODE_COMPLETE_CALL_BACK, (PVOID) vmp4Callback_Null);
		mp4InstallCallBackFunction (MP4_ENCODE_ERROR_CALL_BACK, (PVOID) vmp4Callback_Null);	

		g_vpMP4.nEncodeIndex = 0;

		sysDisableIRQ ();
		pBitstream = bufMP4BitstreamNew ();
		if (pBitstream == NULL)
		{
			pBitstream = __vmp4encGetBuffer (FALSE);
			if (pBitstream != NULL)
			{
				listDetach (&pBitstream->list);
				bufMP4BitstreamDecRef (pBitstream);
			}
			pBitstream = bufMP4BitstreamNew ();
		}
		sysEnableIRQ ();
		ASSERT (pBitstream != NULL);
		
		/* Set encoder. */
		memset (&g_vpMP4.mp4encinf, 0, sizeof (g_vpMP4.mp4encinf));

		/* Codec control */
		g_vpMP4.mp4encinf.ucBaserate		= 30;
		//g_vpMP4.mp4encinf.ucBaserate		= (g_vpMP4.uEncFrameRateStatic < 5 ? 5: g_vpMP4.uEncFrameRateStatic);
		g_vpMP4.mp4encinf.qintra			= 20; // 1~31
		g_vpMP4.mp4encinf.qinter			= 20; // 1~31
		//g_vpMP4.mp4encinf.bitrate			= 768*1024;
		g_vpMP4.mp4encinf.bitrate			= g_vpMP4.uEncBitRate;
		g_vpMP4.mp4encinf.allow_framedrop	= 1;
		
		/* Buffer allocate */
		memset (
			(void *) NON_CACHE (g_vpMP4.pMP4EncoderBuffer->aucReference),
			0,
			sizeof (g_vpMP4.pMP4EncoderBuffer->aucReference));
		memset (
			(void *) NON_CACHE (g_vpMP4.pMP4EncoderBuffer->aucReconstructed),
			0,
			sizeof (g_vpMP4.pMP4EncoderBuffer->aucReconstructed));
		
		g_vpMP4.mp4encinf.uReferenceaddr		= (UINT32) NON_CACHE (g_vpMP4.pMP4EncoderBuffer->aucReference);
		g_vpMP4.mp4encinf.uReconstructaddr		= (UINT32) NON_CACHE (g_vpMP4.pMP4EncoderBuffer->aucReconstructed);
		g_vpMP4.mp4encinf.uBitstreamheaderaddr	= (UINT32) NON_CACHE (pBitstream->aucBitstream);
		g_vpMP4.mp4encinf.uEncodedbufsize		= 0;
		/* Bit stream information */

		if (g_vpMP4.eEncFormat == CMD_VIDEO_MP4)
			g_vpMP4.mp4encinf.codectype = MPEG4_BITSTREAM;
		else
			g_vpMP4.mp4encinf.codectype = H263_SHORT_HEADER_BITSTREAM;
		
		g_vpMP4.mp4encinf.vop_time_increment_resolution	= 30000;	
		g_vpMP4.mp4encinf.PframenumbeteweenIfram		= 64;	// -1 : I frame then all P frame
		g_vpMP4.mp4encinf.video_packet_interval			= 0;	//0:disable ,others specify Video Packet Interval
		g_vpMP4.mp4encinf.uPixels						= pVideo->tLocalWin.tSourceSize.usWidth;
		g_vpMP4.mp4encinf.uLines						= pVideo->tLocalWin.tSourceSize.usHeight;
		g_vpMP4.mp4encinf.quant_type					= 0;	// 0: H.263Q, 1: MPEG4 Q
	
		uHeadBitstreamLength	= mp4SetEncoder(&g_vpMP4.mp4encinf);
		pBitstream->uLength		= uHeadBitstreamLength;
		if (pBitstream->uLength > 0)
		{
			/* Append mp4 "VisualObjectSequence" and "VisualObject". */
			UCHAR aucExtraHeader[] = {0x00, 0x00, 0x01, 0xB0, 0x03, 0x00, 0x00, 0x01, 0xB5, 0x09};
			memmove ((char *) NON_CACHE (pBitstream->aucBitstream + VP_BUF_APPEND_SIZE) + sizeof (aucExtraHeader), 
				(char *) NON_CACHE (pBitstream->aucBitstream),
				pBitstream->uLength);
			pBitstream->uLength += sizeof (aucExtraHeader) + VP_BUF_APPEND_SIZE;
			memcpy ((char *) NON_CACHE (pBitstream->aucBitstream + VP_BUF_APPEND_SIZE), aucExtraHeader, sizeof (aucExtraHeader));
			
			/* Append timestamp */
			pBitstream->tTimeStamp = cyg_current_time();
			memcpy ((UCHAR *) NON_CACHE (pBitstream->aucBitstream) + pBitstream->uLength, &pBitstream->tTimeStamp, 8);
			pBitstream->uLength += 8;
			
			/* Sent the interrupt event. */
			sysDisableIRQ ();
			listAttach (&g_vpMP4.listBitstream, &pBitstream->list);
			sysEnableIRQ ();
		}
		else
		{
			diag_printf("encode mp4 header error!!!\n");
			sysDisableIRQ ();
			bufMP4BitstreamDecRef (pBitstream);
			sysEnableIRQ ();
		}
#ifndef ECOS
		__cmd18_03_EnableCommandInterrupt_CheckIntr ();
#else
		//iothread_SendNotify(MEDIA_TYPE_VIDEO);
		iothread_EventNotify();
#endif
		
		sysSafePrintf ("MP4 Length: %d\n", uHeadBitstreamLength);

		vmp4ForceIFrame (5, 3000);
		
		outpw (REG_ENCODER_CONTROL_IO, sizeof (pBitstream->aucBitstream));

		mp4InstallCallBackFunction (MP4_ENCODE_COMPLETE_CALL_BACK, (PVOID) vmp4encDoneCallback);
		mp4InstallCallBackFunction (MP4_ENCODE_ERROR_CALL_BACK, (PVOID) vmp4encErrorCallback);	

#ifndef ECOS
		sysEnableInterrupt(IRQ_MPEG);			// Enable MP4 Interrupt	
#else
		cyg_interrupt_unmask(IRQ_MPEG);
#endif
    	mp4InstallISR();
	}
}


void vmp4encDecRef (void)
{
	ASSERT (g_vpMP4.nRefCount_Encoder > 0);
	if (--g_vpMP4.nRefCount_Encoder == 0)
	{
		mp4InstallCallBackFunction (MP4_ENCODE_COMPLETE_CALL_BACK, (PVOID) vmp4Callback_Null);
		mp4InstallCallBackFunction (MP4_ENCODE_ERROR_CALL_BACK, (PVOID) vmp4Callback_Null);		
	}
	vmp4DecRef ();
}


/* Reference count for MP4 decoder. */
void vmp4decAddRef (VP_VIDEO_T *pVideo)
{
	vmp4AddRef ();
	if (g_vpMP4.nRefCount_Decoder++ == 0)
	{
		/* Init decode buffer. */
		mp4Splitter_Init (&g_vpMP4.pMP4DecoderBuffer->splitter,
			(char *) NON_CACHE (g_vpMP4.pMP4DecoderBuffer->aucSplitterBuffer),
			sizeof (g_vpMP4.pMP4DecoderBuffer->aucSplitterBuffer));
	
		/* Set decoder. */
		memset (&g_vpMP4.mp4decinf, 0, sizeof (g_vpMP4.mp4decinf));
		g_vpMP4.bDecInit = FALSE;

		mp4InstallCallBackFunction (MP4_DECODE_COMPLETE_CALL_BACK, (PVOID) vmp4decDoneCallback);
		mp4InstallCallBackFunction (MP4_DECODE_WAIT_CALL_BACK, (PVOID) vmp4decWaitCallback);
		mp4InstallCallBackFunction (MP4_DECODE_ERROR_CALL_BACK, (PVOID) vmp4decErrorCallback);
	}
}


void vmp4decDecRef (void)
{
	ASSERT (g_vpMP4.nRefCount_Decoder > 0);
	if (--g_vpMP4.nRefCount_Decoder == 0)
	{//Nothing to do.
		mp4InstallCallBackFunction (MP4_DECODE_COMPLETE_CALL_BACK, (PVOID) vmp4Callback_Null);
		mp4InstallCallBackFunction (MP4_DECODE_WAIT_CALL_BACK, (PVOID) vmp4Callback_Null);
		mp4InstallCallBackFunction (MP4_DECODE_ERROR_CALL_BACK, (PVOID) vmp4Callback_Null);
	}
	vmp4DecRef ();
}




int vmp4encStart (UINT32 uYUV_Addr, cyg_tick_count_t tTimeStamp)
{
	VP_BUFFER_MP4_BITSTREAM_T *pBitstream;
	int rt;

	pBitstream = bufMP4BitstreamNew ();
	if (pBitstream == NULL)
	{
		//PTL;
		return -1;	//No buffer to save encoded data.
	}
	
	rt = tt_pc_try_produce (&g_vpMP4.pcEncFlag, NULL, NULL);
	if (rt == 0)
	{
		BOOL bForceIFrame = FALSE;
		pBitstream->tTimeStamp = tTimeStamp;
		g_vpMP4.apMP4EnocderBitstream[0] = pBitstream;
		mp4SetNewYUVEncoderAddr ((UINT32) NON_CACHE (uYUV_Addr));
		mp4SetNewEncBitStreamAddr((UINT32) NON_CACHE (pBitstream->aucBitstream + VP_BUF_APPEND_SIZE));	//8 byte for user defined timestamp.
		

		if (g_vpMP4.uEncIFrameNum_Hacker > 0)
		{
			g_vpMP4.uEncIFrameNum_Hacker--;
			bForceIFrame = TRUE;
		}

		if (g_vpMP4.uEncIFrameNum_Enable_TimeHacker)
		{
			if (tt_get_ticks () < g_vpMP4.uEncIFrameNum_TimeHacker)
				bForceIFrame = TRUE;
			else
				g_vpMP4.uEncIFrameNum_Enable_TimeHacker = FALSE;
		}
			
		if (bForceIFrame == TRUE)
		{
			outpw (REG_ENCODER_CONTROL, inpw (REG_ENCODER_CONTROL) & ~0x00000300);
		}
		
		mp4StartEncoder();
	}
	else
		bufMP4BitstreamDecRef (pBitstream);

	return rt;
}


VP_MP4_ENC_STATE_E vmp4encWaitOK (void)
{
	VP_MP4_ENC_STATE_E eEncState;
#ifndef ECOS
	tt_sem_down (&g_vpMP4.pcEncFlag.producer);
	eEncState = g_vpMP4.eEncState;
	tt_sem_up (&g_vpMP4.pcEncFlag.producer);
#else
	cyg_semaphore_wait (&g_vpMP4.pcEncFlag.producer);
	eEncState = g_vpMP4.eEncState;
	cyg_semaphore_post (&g_vpMP4.pcEncFlag.producer);
#endif
	
#ifndef ECOS
	__cmd18_03_EnableCommandInterrupt_CheckIntr ();
#else
	if(eEncState == VP_MP4_ENC_OK)
	{
		//iothread_SendNotify(MEDIA_TYPE_VIDEO);
		iothread_EventNotify();
	}
#endif
	return eEncState;
}



//void vmp4encSetBitrate 

int vmp4decStart (UINT32 uBitstream_Addr, UINT32 uBitstream_Size)
{
	int rt = tt_pc_try_produce (&g_vpMP4.pcDecFlag, NULL, NULL);

	if (rt == 0)
	{
		UCHAR *pMP4Header = (UCHAR *) uBitstream_Addr;
		if (!g_vpMP4.bDecInit
			|| (pMP4Header[0] == 0x00 && pMP4Header[1] == 0x00 && pMP4Header[2] == 0x01 && pMP4Header[3] == 0xB0
				&& pMP4Header[5] == 0x00 && pMP4Header[6] == 0x00 && pMP4Header[7] == 0x01 && pMP4Header[8] == 0xB5)
			)
		{
			INT nDecoderRes;
			mp4ResumeDecoder ();
			g_vpMP4.mp4decinf.uBitstreamaddr	= (UINT32) NON_CACHE (uBitstream_Addr);
			//g_vpMP4.mp4decinf.uDecodedbufsize	= sizeof (g_vpMP4.pMP4DecoderBuffer->aucOutput);
			g_vpMP4.mp4decinf.uDecodedbufsize	= uBitstream_Size;
			nDecoderRes = mp4SetDecoder(&g_vpMP4.mp4decinf);
			
#if 1
			sysSafePrintf ("MPEG4 bistream....(%d)\n", uBitstream_Size);
			sysSafePrintf ("vop_time_increment_resolution = 0x%x(%d)\n",g_vpMP4.mp4decinf.vti_res,g_vpMP4.mp4decinf.vti_res);
			sysSafePrintf ("vti size = 0x%x\n",g_vpMP4.mp4decinf.vti_size);	
			sysSafePrintf ("video_object_layer_width = 0x%x(%d)\n",g_vpMP4.mp4decinf.uPixels,g_vpMP4.mp4decinf.uPixels);		
			sysSafePrintf ("video_object_layer_height = 0x%x(%d)\n",g_vpMP4.mp4decinf.uLines,g_vpMP4.mp4decinf.uLines);	
			sysSafePrintf ("short_header = %d(0:H.263,1:MPEG)\n",g_vpMP4.mp4decinf.short_header);
			sysSafePrintf ("quant_type = 0x%x(0:H.263,1:MPEG)\n",g_vpMP4.mp4decinf.quant_type);	
#endif
			if (nDecoderRes != MP4_NO_ERR
				|| (int) g_vpMP4.mp4decinf.uPixels * (int) g_vpMP4.mp4decinf.uLines <= 0
				|| (int) g_vpMP4.mp4decinf.uPixels * (int) g_vpMP4.mp4decinf.uLines > MAX_MP4_WIDTHxHEIGHT)
			{
				tt_pc_consume (&g_vpMP4.pcDecFlag, NULL, NULL);
				memset (&g_vpMP4.mp4decinf, 0, sizeof (g_vpMP4.mp4decinf));
				return -1;
			}

			mp4SetDecBufferAddr ((UINT32) NON_CACHE (g_vpMP4.pMP4DecoderBuffer->aucReference),
							 (UINT32) NON_CACHE (g_vpMP4.pMP4DecoderBuffer->aucOutput));
			g_vpMP4.bDecInit = TRUE;
			g_vpMP4.eDecState = VP_MP4_DEC_OK;
#ifndef ECOS
			sysEnableInterrupt(IRQ_MPEG);			// Enable MP4 Interrupt	
#else
			cyg_interrupt_unmask(IRQ_MPEG);
#endif
	    	mp4InstallISR();
		}

		mp4SetNewDecBitStreamAddr (NON_CACHE (uBitstream_Addr));
		outpw (REG_DECODER_CONTROL_IO, sizeof (g_vpMP4.pMP4DecoderBuffer->aucOutput));
		//outpw (REG_DECODER_CONTROL_IO,uBitstream_Size);

		//if (g_vpMP4.eDecState == VP_MP4_DEC_WAIT)
		//	mp4ResumeDecoder ();
		//else
		{
			if (mp4StartDecoder() != MP4_NO_ERR)
				rt = -1;
		}
	}

	return rt;
}


VP_MP4_DEC_STATE_E vmp4decPrepare (UINT32 uBitstream_Addr, UINT32 uBitstream_Size)
{
	VP_MP4_DEC_STATE_E eDecodeResult = VP_MP4_DEC_WAIT;
	VP_BUFFER_MP4_DECODER_T *pMP4DecBuf = vmp4decGetBuffer ();
	size_t szLen;

	mp4Splitter_AddData (&g_vpMP4.pMP4DecoderBuffer->splitter,
		(char *) uBitstream_Addr, uBitstream_Size);

	while ((szLen = mp4Splitter_GetFrame (&g_vpMP4.pMP4DecoderBuffer->splitter)) > 0)
	{
		vmp4Lock ();
		//printf ("Dec start: %d\n", szLen);
		if (vmp4decStart ((UINT32) g_vpMP4.pMP4DecoderBuffer->splitter.pcBuffer, szLen) != 0)
			eDecodeResult = VP_MP4_DEC_ERROR;
		else
			eDecodeResult= vmp4decWaitOK ();
		
		sysSafePrintf ("Dec result: %d, %d\n", eDecodeResult, g_vpMP4.eDecState);
				
		if (eDecodeResult == VP_MP4_DEC_OK)
		{
			MP4DECINFO_T *pMP4Info = vmp4decGetInfo ();
			pMP4DecBuf->nOutputLength = vmp4decGetYSize (pMP4Info) + vmp4decGetUVSize (pMP4Info) * 2;
		}
		else
			pMP4DecBuf->nOutputLength = 0;
		vmp4Unlock ();

		/* Bitblt to screen. */
		if (eDecodeResult == VP_MP4_DEC_OK)
		{
			void vdUpdateRemote (void*);
			vdUpdateRemote (NULL);
		}		
		
		
		mp4Splitter_DelData (&g_vpMP4.pMP4DecoderBuffer->splitter, szLen);
	}
	
	return eDecodeResult;
}


VP_MP4_DEC_STATE_E vmp4decWaitOK (void)
{
	VP_MP4_DEC_STATE_E eDecState;
#ifndef ECOS
	tt_sem_down (&g_vpMP4.pcDecFlag.producer);
	eDecState = g_vpMP4.eDecState;
	tt_sem_up (&g_vpMP4.pcDecFlag.producer);
#else
	cyg_semaphore_wait (&g_vpMP4.pcDecFlag.producer);
	eDecState = g_vpMP4.eDecState;
	cyg_semaphore_post (&g_vpMP4.pcDecFlag.producer);
#endif
	
	return eDecState;
}


void vmp4ForceIFrame (UINT32 uNum, UINT32 uMillisecond)
{
	//vmp4Lock ();
	g_vpMP4.uEncIFrameNum_Hacker = uNum;	//uNum frames
	
	//or uMillisecond msec.
	g_vpMP4.uEncIFrameNum_TimeHacker = tt_get_ticks () + tt_msec_to_ticks (uMillisecond);
	g_vpMP4.uEncIFrameNum_Enable_TimeHacker = TRUE;
	//vmp4Unlock ();
}


int vmp4encSetBitrate(UINT32 uBitRate, UINT32 uHeight)
{
	int rt = 0;
	
	vmp4Lock ();
	g_vpMP4.uEncBitRate  = uBitRate;
	if (mp4SetEncoderBitrate(uBitRate, uHeight) != 0)
	{
		rt = -1;
	}
	vmp4Unlock ();
	
	return rt;
}


int vmp4encSetFramerate(UINT32 uFrameRate, UINT32 uHeight)
{
	int rt = 0;
	
	if (uFrameRate >= 100)
	{
		uFrameRate = 90;
	}
	
	vmp4Lock ();
	g_vpMP4.uEncFrameRate  = uFrameRate < 1 ? 1: uFrameRate;
	/* 
	   When frame rate is small, each mp4 picture will be finer,
	   and the picture will be larger.
	   So don't set the framerate into mp4 engine in order to
	   decrease the size of mp4 data stream
	*/
	/*
	if (mp4SetEncoderFramerate(uFrameRate, uHeight) != 0)
	{
		rt = -1;
	}
	*/
	vmp4Unlock ();
	
	return rt;
}


int vmp4encSetQuality (UINT32 uFrameRate, UINT32 uBitRate)
{
	if (uFrameRate == 0 || uFrameRate >= 100)
		return -1;

	vmp4Lock ();
	g_vpMP4.uEncFrameRateStatic = uFrameRate;
	g_vpMP4.uEncFrameRate = uFrameRate;
	g_vpMP4.uEncBitRate = uBitRate;

	if (g_vpMP4.nRefCount_Encoder != 0)
	{
		VP_VIDEO_T *pVideo;
		
		vdLock ();
		pVideo = vdGetSettings ();
		vmp4encDecRef ();
		vmp4encAddRef (pVideo);
		vdUnlock ();
	}
	vmp4Unlock ();
	
	return 0;
}


int vmp4encGetQuality (UINT32 *puFrameRate, UINT32 *puBitRate)
{
	vmp4Lock ();
	if (puFrameRate != NULL)
		*puFrameRate = g_vpMP4.uEncFrameRateStatic;
	if (puBitRate != NULL)
		*puBitRate = g_vpMP4.uEncBitRate;
	vmp4Unlock ();
	
	return 0;
}


int vmp4encSetFormat (CMD_VIDEO_FORMAT_E eFormat)
{
	vmp4Lock ();
	g_vpMP4.eEncFormat = eFormat;

	if (g_vpMP4.nRefCount_Encoder != 0)
	{
		VP_VIDEO_T *pVideo;
		
		vdLock ();
		pVideo = vdGetSettings ();
		vmp4encDecRef ();
		vmp4encAddRef (pVideo);
		vdUnlock ();
	}	
	vmp4Unlock ();
	
	return 0;
}


void vmp4ClearBuffer (void)
{
	int i;

	sysDisableIRQ ();
	while (1)
	{
		VP_BUFFER_MP4_BITSTREAM_T *pBitstream;
			
		
		pBitstream = vmp4encGetBuffer ();
		if (pBitstream == NULL)
			break;
		else
		{
			listDetach (&pBitstream->list);
			bufMP4BitstreamDecRef (pBitstream);
		}
	}
	
	for (i = 0;
		i < sizeof (g_vpMP4.apMP4EnocderBitstream) / sizeof (g_vpMP4.apMP4EnocderBitstream[0]);
		i++)
	{
		if (g_vpMP4.apMP4EnocderBitstream[i] != NULL)
		{
			bufMP4BitstreamDecRef (g_vpMP4.apMP4EnocderBitstream[i]);	
			g_vpMP4.apMP4EnocderBitstream[i] = NULL;
		}
	}

	sysEnableIRQ ();
	
	//iothread_ClearNotify(MEDIA_TYPE_VIDEO);
}


VP_BUFFER_MP4_DECODER_T *vmp4decGetBuffer (void)
{
	return g_vpMP4.pMP4DecoderBuffer;
}

MP4DECINFO_T *vmp4decGetInfo (void)
{
	return &g_vpMP4.mp4decinf;
}






void vmp4Lock (void)
{
#ifndef ECOS
	tt_rmutex_lock (&g_vpMP4.mtLock);
#else
	cyg_mutex_lock (&g_vpMP4.mtLock);
#endif
}

int vmp4TryLock (void)
{
#ifndef ECOS
	return tt_rmutex_try_lock (&g_vpMP4.mtLock);
#else
	return cyg_mutex_trylock (&g_vpMP4.mtLock);
#endif
}

void vmp4Unlock (void)
{
#ifndef ECOS
	tt_rmutex_unlock (&g_vpMP4.mtLock);
#else
	cyg_mutex_unlock (&g_vpMP4.mtLock);
#endif
}


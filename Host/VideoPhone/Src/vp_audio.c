#include "../Inc/inc.h"

#define DECODE_WAIT
#define AUDIO_DEC_SPLITTER_BUF_NUM	1
#define AUDIO_ENC_MSG_NUM	3
#define AUDIO_DEC_MSG_NUM	3

#define AUDIO_NOTIFICATION_VOLUME_MAX 60

typedef struct
{
	UCHAR	aucBuffer[MEM_ALIGN_SIZE (MAX(OPT_AUDIO_MAX_DECODE_SIZE, 2 * (2 + MAX (MAX (OPT_AUDIO_AMR_ENCODED_SIZE, OPT_AUDIO_IMA_ENCODED_SIZE), OPT_AUDIO_BUFFER_SIZE))))];
	size_t	szMaxLength;
	size_t	szLength;
	size_t	szTotalLength;

	LIST_T list;
	int ref_count;

} VP_AUDIO_SPLITTER_T;
DECLARE_MEM_POOL (bufAuSplitter, VP_AUDIO_SPLITTER_T)
IMPLEMENT_MEM_POOL (bufAuSplitter, VP_AUDIO_SPLITTER_T, AUDIO_DEC_SPLITTER_BUF_NUM)

typedef struct
{
	void *pucAudioBuffer;
	cyg_tick_count_t tTimeStamp;
	//UINT32 uTimeStamp;

	LIST_T list;
	int ref_count;
} VP_AUDIO_ENC_MSG_T;
DECLARE_MEM_POOL (bufEncMsg, VP_AUDIO_ENC_MSG_T)
IMPLEMENT_MEM_POOL (bufEncMsg, VP_AUDIO_ENC_MSG_T, AUDIO_ENC_MSG_NUM)


typedef struct
{
	UINT32 auEncThreadBuffer[TT_THREAD_BUFFER_SIZE (16*1024) / sizeof (UINT32)];
	UINT32 auDecThreadBuffer[TT_THREAD_BUFFER_SIZE (16*1024) / sizeof (UINT32)];
#ifndef ECOS
	UINT32 auEncMsgBuffer[TT_MSG_BUFFER_SIZE (AUDIO_ENC_MSG_NUM) / sizeof (UINT32)];
	TT_MSG_QUEUE_T *pEncMsgQueue;

	UINT32 auDecMsgBuffer[TT_MSG_BUFFER_SIZE (AUDIO_DEC_MSG_NUM) / sizeof (UINT32)];
	TT_MSG_QUEUE_T *pDecMsgQueue;


	TT_RMUTEX_T mtLock;

#else
	cyg_mbox		auEncMsgBuffer;	//this is for storing msg ( a array of void* "void *itemqueue[10]")
	cyg_handle_t	pEncMsgQueue;	//mail box
	cyg_handle_t	cygEncHandle;
	cyg_thread		cygEncThread;
	
	cyg_mbox		auDecMsgBuffer;	//this is for storing msg ( a array of void* "void *itemqueue[10]")
	cyg_handle_t	pDecMsgQueue;	//mail box
	cyg_handle_t	cygDecHandle;
	cyg_thread		cygDecThread;
	
	TT_RMUTEX_T mtLock;
	//cyg_mutex_t	mtLock;
	TT_COND_T cPlayWait;
#endif
	
	BOOL bPlayEnabled;
	BOOL bRecordEnabled;
	
	VP_BUFFER_PCM_AUDIO_T *pPlayBuffer;
	VP_BUFFER_PCM_AUDIO_T *pRecordBuffer;
	
	VP_AUDIO_SPLITTER_T *pSplitterBuffer;

	CMD_AUDIO_FORMAT_E eEncodeFormat;
	CMD_AUDIO_FORMAT_E eDecodeFormat;

	int nAMRQualityLevel;
	LIST_T listEncodedAudio_Local;
	VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *pEncodedAudio_Remote;
	VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *apEncodedAudio_Local[2];
	
	UINT32 uIMA_ADPCM_RingBuffer_Length;
	UCHAR aucIMA_ADPCM_RingBuffer[OPT_AUDIO_IMA_SOURCE_SIZE];
	
	

	LIST_T listPCM_Local;
	LIST_T listPCM_Remote;
	TT_PC_T	pcRemotePCM;
	
	int iLAWSize;
	
	/* Notification playback */
	cyg_mutex_t mutNotificationLock;
	UCHAR *pucNotificationBuffer;
	int iNotificationBufferLen;
	int iNotificationBufferPlayFrom;
	int iNotificationVolume;
	void (*notificationCallback) (void);
#ifndef ECOS
	ECHOCANCELL_STRUCT ecControl;
#endif
	
	/* To test if the interrupt has been triggered. */
	int iPlayIrqCount;
	int iRecordIrqCount;
} VP_AUDIO_T;





#pragma arm section zidata = "non_init"
static VP_AUDIO_T g_vpAudio;
#pragma arm section zidata




static void auSplitter_Init (VP_AUDIO_SPLITTER_T *pSplitter)
{
	if (pSplitter != NULL)
	{
		pSplitter->szMaxLength		= sizeof (pSplitter->aucBuffer);
		pSplitter->szLength			= 0;
		pSplitter->szTotalLength	= 0;
	}
}

static void auSplitter_DelData (VP_AUDIO_SPLITTER_T *pSplitter, size_t szLength)
{
	if (szLength > pSplitter->szLength)
		szLength = pSplitter->szLength;
	
	memmove (pSplitter->aucBuffer,
		pSplitter->aucBuffer + szLength,
		pSplitter->szLength - szLength);
	pSplitter->szLength			-= szLength;
	
}

static size_t auSplitter_FreeSize (VP_AUDIO_SPLITTER_T *pSplitter)
{
	return pSplitter->szMaxLength - pSplitter->szLength;
}


static void auSplitter_AddData (VP_AUDIO_SPLITTER_T *pSplitter, const UCHAR *cpucBuffer, size_t szLength)
{
	/* Make sure that there is enough buffer to add the data. */
	if (szLength > pSplitter->szMaxLength)
		return;
	
	if (pSplitter->szLength + szLength > pSplitter->szMaxLength)
	{
		auSplitter_DelData (pSplitter, pSplitter->szLength + szLength - pSplitter->szMaxLength);
	}
	
	/* Copy the data. */
	memmove (pSplitter->aucBuffer + pSplitter->szLength,
		cpucBuffer,
		szLength);
	pSplitter->szLength += szLength;

	/* Check the valid MP4/H.263 clip. */
	
	pSplitter->szTotalLength += szLength;
}

static size_t auSplitter_GetFrame (VP_AUDIO_SPLITTER_T *pSplitter)
{
	//if (pSplitter->szLength 
	switch (g_vpAudio.eDecodeFormat)
	{
		case CMD_AUDIO_AMR:
		{
			if (pSplitter->szLength >= 1)
			{
				size_t aszLength[] = {13, 14, 16, 18, 20, 21, 27, 32}; 
				UCHAR ucMode = (pSplitter->aucBuffer[0] >> (UCHAR) 3) & (UCHAR) 0x07;
				size_t szLength = aszLength[ucMode];
			
				if (pSplitter->szLength < szLength)
					return 0;
				else
					return szLength;
			}
			break;
		}		
		case CMD_AUDIO_PCM:
		{
			if (pSplitter->szLength >= OPT_AUDIO_BUFFER_SIZE)
				return OPT_AUDIO_BUFFER_SIZE;
			break;
		}
		case CMD_AUDIO_IMA_ADPCM:
			/* Not supported */
			break;
		case CMD_AUDIO_ALAW:
		{
			if (pSplitter->szLength >= g_vpAudio.iLAWSize)
				return g_vpAudio.iLAWSize;
			break;
		}
		case CMD_AUDIO_ULAW:
		{
			if (pSplitter->szLength >= g_vpAudio.iLAWSize)
				return g_vpAudio.iLAWSize;
			break;
		}
		default:;
	}
	return 0;
}






VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *vauGetEncBuffer_Remote (void)
{
	return g_vpAudio.pEncodedAudio_Remote;
}



VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *vauGetEncBuffer_Local (void)
{

	VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *pReturn;
	vauLock ();
	if (g_vpAudio.listEncodedAudio_Local.pNext == &g_vpAudio.listEncodedAudio_Local)
		pReturn = NULL;
	else
		pReturn = GetParentAddr (g_vpAudio.listEncodedAudio_Local.pNext, VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T, list);
	vauUnlock ();
	return pReturn;
}


static void __vauRecord_AttachBuffer (VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *pEncAudio)
{
#if 0
	BOOL bSetIntr = FALSE;
	vauLock ();
	
	g_vpAudio.apEncodedAudio_Local[0] = pEncAudio;
	if (g_vpAudio.apEncodedAudio_Local[1] != NULL)
	{
		/* Calculate time interval. */
		UINT32 uTimeInterval = g_vpAudio.apEncodedAudio_Local[0]->uTimeStamp
			- g_vpAudio.apEncodedAudio_Local[1]->uTimeStamp;
		if ((uTimeInterval & 0x0FFFF) == 0)
			uTimeInterval = 20;	//20 millisecond;
		
		/* Pop apEncodedAudio_Local[1]. */
		pEncAudio = g_vpAudio.apEncodedAudio_Local[1];
			
		/* Append time interval. */
		memcpy ((UCHAR *) NON_CACHE (pEncAudio->aucAudio) + pEncAudio->nSize, &uTimeInterval, 2);
		pEncAudio->nSize += 2;
		listAttach (&g_vpAudio.listEncodedAudio_Local, &pEncAudio->list);
		bSetIntr = TRUE;
	}
	/* Push apEncodedAudio_Local[0]. */
	g_vpAudio.apEncodedAudio_Local[1] = g_vpAudio.apEncodedAudio_Local[0];
	g_vpAudio.apEncodedAudio_Local[0] = NULL;

	vauUnlock ();
	
	if (bSetIntr == TRUE)
		__cmd18_03_EnableCommandInterrupt_CheckIntr ();
#else
	vauLock ();
	/* Append time stamp. */
	memcpy (pEncAudio->aucAudio + pEncAudio->nSize, &pEncAudio->tTimeStamp, 8);	
	pEncAudio->nSize += 8;
	//diag_printf("App: %d, l=%d\n", (int)pEncAudio->tTimeStamp, pEncAudio->nSize);

	listAttach (&g_vpAudio.listEncodedAudio_Local, &pEncAudio->list);
		
	vauUnlock ();
#ifndef ECOS
	__cmd18_03_EnableCommandInterrupt_CheckIntr ();
#else
	//iothread_SendAudioNotify(MEDIA_TYPE_AUDIO);
	iothread_EventNotify();
#endif
#endif
}


static void __vauRecord_AMR_or_PCM (VP_BUFFER_LOCAL_PCM_BITSTREAM_T *pLocalPCM, CMD_AUDIO_FORMAT_E eFormat)
{
	VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *pEncAudio;
	
	pEncAudio = bufEncAudioNew ();
	if (pEncAudio != NULL)
	{
		if (eFormat == CMD_AUDIO_AMR)
		{/* AMR */
			amrEncode (g_vpAudio.nAMRQualityLevel, 0, (short *) pLocalPCM->aucPCM, (short *) (pEncAudio->aucAudio + VP_BUF_APPEND_SIZE), &pEncAudio->nSize);
			pEncAudio->nSize += VP_BUF_APPEND_SIZE;
		}
		else
		{/* ADPCM */
			pEncAudio->nSize = OPT_AUDIO_BUFFER_SIZE;
			memcpy ((short *) (pEncAudio->aucAudio + VP_BUF_APPEND_SIZE), (short *) pLocalPCM->aucPCM, pEncAudio->nSize);
			pEncAudio->nSize += VP_BUF_APPEND_SIZE;
		}

		//sysSafePrintf ("Encode length: %d\n", pEncAudio->nSize);
		
		//pEncAudio->uTimeStamp = pLocalPCM->uTimeStamp;
		pEncAudio->tTimeStamp = pLocalPCM->tTimeStamp;
		
		/* Attach the encoded data to the AMR data list. */
		__vauRecord_AttachBuffer (pEncAudio);
	}
	else
		sysSafePrintf ("No record buffer!\n");
}


static void __vauRecord_IMA_ADPCM (VP_BUFFER_LOCAL_PCM_BITSTREAM_T *pLocalPCM, CMD_AUDIO_FORMAT_E eFormat)
{
	UINT32 uLen;
	
	for (uLen = 0; uLen < sizeof (pLocalPCM->aucPCM); )
	{
		UINT32 uLenThis = sizeof (g_vpAudio.aucIMA_ADPCM_RingBuffer) - g_vpAudio.uIMA_ADPCM_RingBuffer_Length;

		if (uLenThis == 0)
		{
			VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *pEncAudio;
	
			pEncAudio = bufEncAudioNew ();
			if (pEncAudio != NULL)
			{
				INT nPacketSize;
				INT nBlockSize = imaadpcmSamplePerBlock () * 2;
		
				imaadpcmBlockEnc ((CHAR *) g_vpAudio.aucIMA_ADPCM_RingBuffer, nBlockSize, 
					(CHAR *) (pEncAudio->aucAudio + VP_BUF_APPEND_SIZE),
					&nPacketSize);
		
				//sysprintf ("nBlockSize = %d => %d\n", nBlockSize, nPacketSize);
				pEncAudio->nSize = nPacketSize + VP_BUF_APPEND_SIZE;
				pEncAudio->tTimeStamp = pLocalPCM->tTimeStamp;
				//pEncAudio->uTimeStamp = pLocalPCM->uTimeStamp;
				__vauRecord_AttachBuffer (pEncAudio);
			}
			
			g_vpAudio.uIMA_ADPCM_RingBuffer_Length = 0;
			uLenThis = sizeof (g_vpAudio.aucIMA_ADPCM_RingBuffer);
		}
		
		
		if (uLenThis > sizeof (pLocalPCM->aucPCM) - uLen)
			uLenThis = sizeof (pLocalPCM->aucPCM) - uLen;
		memcpy (g_vpAudio.aucIMA_ADPCM_RingBuffer + g_vpAudio.uIMA_ADPCM_RingBuffer_Length,
			pLocalPCM->aucPCM + uLen,
			uLenThis);
		g_vpAudio.uIMA_ADPCM_RingBuffer_Length += uLenThis;
		uLen += uLenThis;		
	}	
}


static void __vauRecord_ALAW_OR_ULAW (VP_BUFFER_LOCAL_PCM_BITSTREAM_T *pLocalPCM, CMD_AUDIO_FORMAT_E eFormat)
{
	VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *pEncAudio;
	EncodeLAW_T encode_info;
	
	pEncAudio = bufEncAudioNew ();
	if (pEncAudio != NULL)
	{
		if (eFormat == CMD_AUDIO_ALAW)
		{/* ALAW */
			encode_info.dfSrcFormat = EncodeLAW_FORMAT_LINEAR;
			encode_info.dfDstFormat = EncodeLAW_FORMAT_ALAW;
			encode_info.ucSrcData = pLocalPCM->aucPCM;
			encode_info.iSrcDataSize = OPT_AUDIO_BUFFER_SIZE;
			encode_info.ucDstData = pEncAudio->aucAudio + VP_BUF_APPEND_SIZE;
			encode_info.iDstDataSize = sizeof(pEncAudio->aucAudio);
if (0)
{
	int i;
	unsigned char *puc = (unsigned char *)pLocalPCM->aucPCM;
	for (i = 0; i < OPT_AUDIO_BUFFER_SIZE; i += 2)
	{
		unsigned char tmp;
		tmp = puc[i];
		puc[i] = puc[i+1];
		puc[i+1] = tmp;
	}
}	

			pEncAudio->nSize = EncodeLAW(&encode_info);
			if(pEncAudio->nSize == -1)
			{
				diag_printf("encode law failed\n");
				pEncAudio->nSize = 0;
			}
			g_vpAudio.iLAWSize = pEncAudio->nSize;
			pEncAudio->nSize += VP_BUF_APPEND_SIZE;
		}
		else
		{/* ULAW */
			encode_info.dfSrcFormat = EncodeLAW_FORMAT_LINEAR;
			encode_info.dfDstFormat = EncodeLAW_FORMAT_ULAW;
			encode_info.ucSrcData = pLocalPCM->aucPCM;
			encode_info.iSrcDataSize = OPT_AUDIO_BUFFER_SIZE;
			encode_info.ucDstData = pEncAudio->aucAudio + VP_BUF_APPEND_SIZE;
			encode_info.iDstDataSize = sizeof(pEncAudio->aucAudio);
			
			pEncAudio->nSize = EncodeLAW(&encode_info);
			if(pEncAudio->nSize == -1)
			{
				diag_printf("encode law failed\n");
				pEncAudio->nSize = 0;
			}
			g_vpAudio.iLAWSize = pEncAudio->nSize;
			pEncAudio->nSize += VP_BUF_APPEND_SIZE;
		}

		//sysSafePrintf ("Encode length: %d\n", pEncAudio->nSize);
		
		pEncAudio->tTimeStamp = pLocalPCM->tTimeStamp;
		//pEncAudio->uTimeStamp = pLocalPCM->uTimeStamp;
		/* Attach the encoded data to the AMR data list. */
		__vauRecord_AttachBuffer (pEncAudio);
	}
	else
		sysSafePrintf ("No record buffer!\n");
}


static void vauThread_OnRecord (void *pData)
{
	VP_AUDIO_ENC_MSG_T *pEncMsg = (VP_AUDIO_ENC_MSG_T *) pData;
#ifndef ECOS
	VP_BUFFER_REMOTE_PCM_BITSTREAM_T *pRemotePCM;
#endif
	VP_BUFFER_LOCAL_PCM_BITSTREAM_T *pLocalPCM;
	
	/* Allocate a PCM buffer. */
	vauLock ();
	pLocalPCM = bufLocalPCMNew ();
	if (pLocalPCM == NULL
		&& &g_vpAudio.listPCM_Local != g_vpAudio.listPCM_Local.pNext)
	{
		/* Delete the head block on used PCM lists. */
		pLocalPCM = GetParentAddr (g_vpAudio.listPCM_Local.pNext, VP_BUFFER_LOCAL_PCM_BITSTREAM_T, list);
		listDetach (&pLocalPCM->list);
		bufLocalPCMDecRef (pLocalPCM);
		
		/* Try to re-allocate one. */
		pLocalPCM = bufLocalPCMNew ();		
	}
	vauUnlock ();
	ASSERT (pLocalPCM != NULL)
	

	memcpy (pLocalPCM->aucPCM, (short *) NON_CACHE (pEncMsg->pucAudioBuffer), OPT_AUDIO_BUFFER_SIZE);
	pLocalPCM->tTimeStamp = pEncMsg->tTimeStamp;
	//pLocalPCM->uTimeStamp = pEncMsg->uTimeStamp;
	bufEncMsgDecRef (pEncMsg);

#ifndef ECOS
	/* AEC processing */
	vauLock ();
	if (!listIsEmpty (&g_vpAudio.listPCM_Remote))
		pRemotePCM = GetParentAddr (g_vpAudio.listPCM_Remote.pNext, VP_BUFFER_REMOTE_PCM_BITSTREAM_T, list);
	else
		pRemotePCM = NULL;

	if (pRemotePCM != NULL)
	{
		g_vpAudio.ecControl.bAECEEC_SW = FALSE;
		EchoCancellationProcessing(&g_vpAudio.ecControl, (short *) pRemotePCM->aucPCM, AMR_BLOCK, (short *) pLocalPCM->aucPCM);		
	}
	vauUnlock ();
#endif
	
	/* Encoder Audio */
	switch (g_vpAudio.eEncodeFormat)
	{
		case CMD_AUDIO_AMR:
			__vauRecord_AMR_or_PCM (pLocalPCM, CMD_AUDIO_AMR);
			break;
		case CMD_AUDIO_PCM:
			__vauRecord_AMR_or_PCM (pLocalPCM, CMD_AUDIO_PCM);
			break;
		case CMD_AUDIO_IMA_ADPCM:
			__vauRecord_IMA_ADPCM (pLocalPCM, CMD_AUDIO_IMA_ADPCM);
			break;
		case CMD_AUDIO_ULAW:
			__vauRecord_ALAW_OR_ULAW (pLocalPCM, CMD_AUDIO_ULAW);
			break;
		case CMD_AUDIO_ALAW:
			__vauRecord_ALAW_OR_ULAW (pLocalPCM, CMD_AUDIO_ALAW);
			break;
	}

	vauLock ();
	listAttach (&g_vpAudio.listPCM_Local, &pLocalPCM->list);
	vauUnlock ();	

#if 0	//Test loopback
	vauDecode (pEncAudio->aucAudio, pEncAudio->nSize);
	
	vauLock ();
	listDetach (&pEncAudio->list);
	bufEncAudioDecRef (pEncAudio);
	vauUnlock ();
#endif


}



typedef struct
{
	CONST UCHAR		*cpucData;
	UINT32			uLen;
} VP_AUDIO_DECODER_T;


void __vauDecode (void *arg)
{
	VP_AUDIO_DECODER_T *pAudio = (VP_AUDIO_DECODER_T *) arg;

#ifndef ECOS
	VP_BUFFER_LOCAL_PCM_BITSTREAM_T *pLocalPCM;
#endif
	VP_BUFFER_REMOTE_PCM_BITSTREAM_T *pRemotePCM;
	//BOOL bBanlance = FALSE;

	/* Allocate a PCM buffer. */
	vauLock ();
	pRemotePCM = bufRemotePCMNew ();
	/* Must be None-NULL, this is ensured by producer */
	if (pRemotePCM == NULL)
	{
		diag_printf("__vauDecode(): No decode buffer!!!\n");
		{
			int producer, consumer;
			cyg_semaphore_peek(&g_vpAudio.pcRemotePCM.producer, &producer);
			cyg_semaphore_peek(&g_vpAudio.pcRemotePCM.consumer, &consumer);
			diag_printf("producer %d, consumer %d, free %d, used %d\n", 
					producer, consumer, bufRemotePCMGetFreeBlocksNum(), listLength (&g_vpAudio.listPCM_Remote));
		}
		while(1);
	}
	/*
	if (pRemotePCM == NULL
		&& &g_vpAudio.listPCM_Remote != g_vpAudio.listPCM_Remote.pNext)
	{
		pRemotePCM = GetParentAddr (g_vpAudio.listPCM_Remote.pNext, VP_BUFFER_REMOTE_PCM_BITSTREAM_T, list);
		listDetach (&pRemotePCM->list);
		tt_pc_consume (&g_vpAudio.pcRemotePCM, NULL, NULL); // to ensure consume count equal to listPCM_Remote num
		bBanlance = TRUE;
		{
			int i;
			cyg_semaphore_peek(&g_vpAudio.pcRemotePCM.consumer, &i);
			diag_printf("get consume %d, len %d\n", i, listLength (&g_vpAudio.listPCM_Remote));
		}
		
		bufRemotePCMDecRef (pRemotePCM);
		
		pRemotePCM = bufRemotePCMNew ();
	}
	*/
	vauUnlock ();
	
	ASSERT (pRemotePCM != NULL)
	/* Decode audio */

	switch(g_vpAudio.eDecodeFormat)
	{
		case CMD_AUDIO_AMR:
		{/* AMR */
			amrDecode ((char *) pAudio->cpucData, (short *) (pRemotePCM->aucPCM));
			break;
		}
		case CMD_AUDIO_PCM:
		{/* ADPCM */
			memcpy ((short *) (pRemotePCM->aucPCM), pAudio->cpucData, sizeof (pRemotePCM->aucPCM));
			break;
		}
		case CMD_AUDIO_IMA_ADPCM:
		{/* IMA-ADPCM */
			//INT nBlockSize;

			//imaadpcmBlockDec((CHAR *) NON_CACHE (pAudio->cpucData), 32,
			//	(CHAR *) pRemotePCM->aucPCM, &nBlockSize); /* IMA ADPCM DECODE API */
			//sysprintf ("Decode ima adpcm!%d\n", nBlockSize);
			break;
		}
		case CMD_AUDIO_ULAW:
		{
			DecodeLAW_T decode_info;
			
			decode_info.dfSrcFormat = DecodeLAW_FORMAT_ULAW;
			decode_info.dfDstFormat = DecodeLAW_FORMAT_LINEAR;
			decode_info.ucSrcData = (UCHAR*)pAudio->cpucData;
			decode_info.iSrcDataSize = pAudio->uLen;
			decode_info.ucDstData = pRemotePCM->aucPCM;
			decode_info.iDstDataSize = sizeof(pRemotePCM->aucPCM);
			
			if(DecodeLAW(&decode_info) == -1)
			{
				diag_printf("decode law failed\n");
				/* This should be discarded, but it's difficult to deal with consumer */
				//bufRemotePCMDecRef(pRemotePCM);
				//return;
			}
			break;
		}
		case CMD_AUDIO_ALAW:
		{
			DecodeLAW_T decode_info;
			
			decode_info.dfSrcFormat = DecodeLAW_FORMAT_ALAW;
			decode_info.dfDstFormat = DecodeLAW_FORMAT_LINEAR;
			decode_info.ucSrcData = (UCHAR*)pAudio->cpucData;
			decode_info.iSrcDataSize = pAudio->uLen;
			decode_info.ucDstData = pRemotePCM->aucPCM;
			decode_info.iDstDataSize = sizeof(pRemotePCM->aucPCM);
			
			if(DecodeLAW(&decode_info) == -1)
			{
				diag_printf("decode law failed\n");
				/* This should be discarded, but it's difficult to deal with consumer */
				//bufRemotePCMDecRef(pRemotePCM);
				//return;
			}
			break;
		}
		default:
		{
			sysSafePrintf ("Unknonw audio length: %d\n", pAudio->uLen);
			break;
		}
	}

#ifndef ECOS
	/* EEC processing. */
	vauLock ();
	if (listIsEmpty (&g_vpAudio.listPCM_Local) == FALSE)
		pLocalPCM = GetParentAddr (g_vpAudio.listPCM_Local.pNext, VP_BUFFER_LOCAL_PCM_BITSTREAM_T, list);
	else
		pLocalPCM = NULL;
	
	if (pLocalPCM != NULL)
	{
		g_vpAudio.ecControl.bAECEEC_SW = TRUE;
		if (g_vpAudio.ecControl.bEECTrain == 0)
		{
			int i;
			for (i = 0; i < AMR_BLOCK; i++)
				((short *) pLocalPCM->aucPCM)[i] = GetWhiteNoise ();
			ElectricalEchoCancellationTraining (&g_vpAudio.ecControl,
				(short *) pRemotePCM->aucPCM,
				AMR_BLOCK,
				10,
				(short *) pLocalPCM->aucPCM);	
		}
		else
		{
			EchoCancellationProcessing (&g_vpAudio.ecControl,
				(short *) pRemotePCM->aucPCM,
				AMR_BLOCK,
				(short *) pLocalPCM->aucPCM);
		}
	}
	vauUnlock ();		
#endif

	vauLock ();
	listAttach (&g_vpAudio.listPCM_Remote, &pRemotePCM->list);
	vauUnlock ();
}


static void vauThread_OnPlay (void *pData)
{
	
	while (1)
	{
		VP_AUDIO_DECODER_T audio;
		size_t szLen;
		
		vauLock ();
		szLen = auSplitter_GetFrame (g_vpAudio.pSplitterBuffer);
		vauUnlock ();
		if (szLen <= 0)
		{
			break;
		}
		
		audio.cpucData	= g_vpAudio.pSplitterBuffer->aucBuffer;
		audio.uLen		= szLen;

#ifdef DECODE_WAIT
		tt_pc_produce (&g_vpAudio.pcRemotePCM, __vauDecode, (void *) &audio);
#else
		__vauDecode ((void *) &audio);
#endif
		vauLock ();
		auSplitter_DelData (g_vpAudio.pSplitterBuffer, szLen);
		tt_cond_signal (&g_vpAudio.cPlayWait);
		vauUnlock ();
	}
}

void vauDecode (CONST UCHAR *cpucData, UINT32 uLen)
{
	UINT32 uSize = sizeof(g_vpAudio.pSplitterBuffer->aucBuffer) / 2;
	
	while (uLen > 0)
	{
		UINT32 uLenThis = uLen;
		if (uLenThis >= uSize)
			uLenThis = uSize;

		vauLock ();
		while (auSplitter_FreeSize (g_vpAudio.pSplitterBuffer) < uLenThis)
			tt_cond_wait (&g_vpAudio.cPlayWait);
		auSplitter_AddData (g_vpAudio.pSplitterBuffer, cpucData, uLenThis);
		vauUnlock ();
		
		tt_msg_try_send (g_vpAudio.pDecMsgQueue, &vauThread_OnPlay, NULL);
		
		cpucData += uLenThis;
		uLen -= uLenThis;
	}
}


#ifndef ECOS
static void vauThreadEntry (void *arg)
#else
static void vauThreadEntry (cyg_addrword_t arg)
#endif
{
	FUN_TT_MSG_PROC	fnMsgProc;
	void			*pData;
#ifndef ECOS
	TT_MSG_QUEUE_T	*pMsgQueue = (TT_MSG_QUEUE_T *) arg;
#else
	cyg_handle_t	pMsgQueue = (cyg_handle_t)arg;
#endif

	while (1)
	{
		tt_msg_recv (pMsgQueue, &fnMsgProc, &pData);
		(*fnMsgProc) (pData);
	}
}



static INT vauRecordCallback (UINT8 *pucBuff, UINT32 uDataLen)
{
	VP_AUDIO_ENC_MSG_T *pEncMsg;
	
	g_vpAudio.iRecordIrqCount++;
	
	pEncMsg = bufEncMsgNew ();
	if (pEncMsg != NULL)
	{
		pEncMsg->pucAudioBuffer = (void *) pucBuff;
		//pEncMsg->uTimeStamp = tt_ticks_to_msec (tt_get_ticks ());
		pEncMsg->tTimeStamp = cyg_current_time();
	
		if (tt_msg_try_send (g_vpAudio.pEncMsgQueue, vauThread_OnRecord, (void *) pEncMsg) != true)
			bufEncMsgDecRef (pEncMsg);
	}
	
	return 0;
}

static void vauDisplayAllThreads(void)
{
	cyg_handle_t	threadHandle;
	cyg_uint16		threadID;
	cyg_thread_info threadInfo;
	
	threadHandle = 0;
	threadID     = 0;
	
	diag_printf("/***********************vauDisplayAllThreads***********************/\n");
	diag_printf("%-20s  %-10s  %-5s    %-10s    %-10s\n",
				"Name", "Handle", "State", "StackSize", "StackUsed");
	while(cyg_thread_get_next(&threadHandle, &threadID) != false)
	{
		if(cyg_thread_get_info(threadHandle, threadID, &threadInfo) != false)
		{
			diag_printf("%-20s  0x%-10x  %-5d  0x%-10x  0x%-10x\n", 
						threadInfo.name, threadInfo.handle, threadInfo.state, threadInfo.stack_size, threadInfo.stack_used);
		}
	}
	diag_printf("/******************************************************************/\n");
}

static INT vauPlayCallback (UINT8 *pucBuff, UINT32 uDataLen)
{
	g_vpAudio.iPlayIrqCount++;
#ifdef DECODE_WAIT
	//if (vauCanLock () == 0)
	//vauLock ();
	if (vauTryLockInDsr () == 0)
	{
		if (tt_pc_try_consume (&g_vpAudio.pcRemotePCM, NULL, NULL) == 0)
		{
			//if (listLength (&g_vpAudio.listPCM_Remote) >= 1)
			if (listIsEmpty (&g_vpAudio.listPCM_Remote) == FALSE)
			{
				VP_BUFFER_REMOTE_PCM_BITSTREAM_T *pPCM = GetParentAddr (g_vpAudio.listPCM_Remote.pNext, 
																		VP_BUFFER_REMOTE_PCM_BITSTREAM_T, list);
				listDetach (&pPCM->list);
				memcpy ((void *) NON_CACHE (pucBuff), pPCM->aucPCM, uDataLen);
				vauNotificationGet((UCHAR*) NON_CACHE(pucBuff), uDataLen);
				bufRemotePCMDecRef (pPCM);
				vauUnlock ();
				return 0;
			}
			else
			{
				cyg_handle_t auPlayId = cyg_thread_self();
				vauDisplayAllThreads();
				diag_printf ("vauPlayCallback(0x%x): No play buffer!\n", auPlayId);
				diag_printf("listPCM_Remote=0x%x, listPCM_Remote.pNext=0x%x, listPCM_Remote.pPrev=0x%x\n", 
							&(g_vpAudio.listPCM_Remote), g_vpAudio.listPCM_Remote.pNext, g_vpAudio.listPCM_Remote.pPrev);
				/*
				{
					int producer, consumer;
					cyg_semaphore_peek(&g_vpAudio.pcRemotePCM.producer, &producer);
					cyg_semaphore_peek(&g_vpAudio.pcRemotePCM.consumer, &consumer);
					diag_printf("producer %d, consumer %d, free %d, used %d\n", 
							producer, consumer, bufRemotePCMGetFreeBlocksNum(), listLength (&g_vpAudio.listPCM_Remote));
				}
				*/
				{
					char *listHeader = (char*)&g_vpAudio.listPCM_Remote;
					char *listNode;
					diag_printf("/******************* vauPlayCallback ********************/\n");
					diag_printf("** listPCM_Remote Header 0x%x\n", listHeader);
					diag_printf("/********************************************************/\n");
				}
				tt_msleep(10000);
				sysDisableIRQ();
				while (1);	//Can not run to here!!!
			}
		}
		vauUnlock ();
	}
	else
		sysSafePrintf ("Audio device locked.\n");

	memset ((void *) NON_CACHE (pucBuff), 0, uDataLen);
	vauNotificationGet((UCHAR*) NON_CACHE(pucBuff), uDataLen);

#else
	//if (vauCanLock () == 0)
	//vauLock ();
	if (vauTryLockInDsr () == 0)
	{
		if (listLength (&g_vpAudio.listPCM_Remote) >= 1)
		{
			VP_BUFFER_REMOTE_PCM_BITSTREAM_T *pPCM = GetParentAddr (g_vpAudio.listPCM_Remote.pNext, 
																	VP_BUFFER_REMOTE_PCM_BITSTREAM_T, list);
			listDetach (&pPCM->list);
			memcpy ((void *) NON_CACHE (pucBuff), pPCM->aucPCM, uDataLen);
			bufRemotePCMDecRef (pPCM);
			vauUnlock ();
			return 0;
		}
	    vauUnlock ();
	}

	memset ((void *) NON_CACHE (pucBuff), 0, uDataLen);
#endif

	return 0;
}



void audio_WM8978_test( )
{
	int apll, audio_div, mclk_div;
// Audio init
/***********************************************************************************/
	AU_I2C_TYPE_T cfg;
	
#if defined CONFIG_AUDIO_WM8978_GPIO
	cfg.bIsGPIO = TRUE;
	cfg.uSDIN = 5;
	//qzhang 2007/06/13
	//i2cType.uSCLK = 4;
	cfg.uSCLK = 1;
#elif defined CONFIG_AUDIO_WM8978_NO_GPIO
    cfg.bIsGPIO = 0;
    cfg.uSDIN = 1;
    cfg.uSCLK = 0;
#else
#	error "No define for AUDIO type!"
#endif
    audioSetI2CType(cfg);	

#if 1
	audioHeadphoneDetectionLevel(1); /* High level = 1(winbond), low level = 0 */

	/* To avoid pop-noise of WM8978 */
	//audio_set_clk(0x652F, 0x00F00000, 0x000F0000);
	audioSetDacVolume(-255, -255);
	audioSetHeadphoneVolume(-56, -56);
	audioSetSpeakerVolume(-56, -56);
	audioI2SInit();
	WM8978_Init();
	WM8978_DAC_Setup();
	audioSetDacVolume(-255, -255);
	audioSetHeadphoneVolume(-56, -56);
	audioSetSpeakerVolume(-56, -56);
	WM8978_R1_Init(1);
	/* To fine-tune the volume balance between headphone and speaker */
	//audio_set_clk(0x652F, 0x00F00000, 0x00000000);
	audioSetHeadphoneSpeakerBalance(6);	//13 -> 0
	
/***********************************************************************************/
#endif

//      audio_device_init(1, 5);
        audioSelectInputPath(IN_PATH_MICL, TRUE);              //MIC path !
        audioSelectInputPath(IN_PATH_MICR, FALSE);             
        audioSelectInputPath(IN_PATH_LINEIN1, FALSE);   //FM path !    
        audioSelectInputPath(IN_PATH_LINEIN2, FALSE);   //LINE-IN path !
//        audioSelectOutputPath(OUT_PATH_SP, TRUE);
        audioBypassDisable();

        audio_get_clk_cfg(AU_SAMPLE_RATE_8000, &apll, &audio_div, &mclk_div);
        audio_set_clk(apll, audio_div, mclk_div);                      

#if defined OPT_USE_AUDIO_DECODER
	audioSetPlayBuff ((UINT32) NON_CACHE (g_vpAudio.pPlayBuffer->aucPCM), sizeof (g_vpAudio.pPlayBuffer->aucPCM));
#endif

#if defined OPT_USE_AUDIO_ENCODER
	audioSetRecBuff ((UINT32) NON_CACHE (g_vpAudio.pRecordBuffer->aucPCM), sizeof (g_vpAudio.pRecordBuffer->aucPCM));
#endif

#if defined OPT_USE_AUDIO_DECODER
	vauStartPlay ();
#endif
     
    


}



INT vauInit (void)
{
	INT rt;
	bufAuSplitterInitMemoryPool ();
	bufEncMsgInitMemoryPool ();

	rt = audioEnable (OPT_AUDIO_PLAY_DEVICE, OPT_AUDIO_RECORD_DEVICE);
	if (rt != 0)
		return rt;

	g_vpAudio.bPlayEnabled		= FALSE;
	g_vpAudio.bRecordEnabled	= FALSE;
	g_vpAudio.iPlayIrqCount		= 0;
	g_vpAudio.iRecordIrqCount	= 0;
	

	g_vpAudio.eEncodeFormat		= CMD_AUDIO_AMR;
	//g_vpAudio.eEncodeFormat			= CMD_AUDIO_PCM;
	//g_vpAudio.eEncodeFormat			= CMD_AUDIO_IMA_ADPCM;
	g_vpAudio.eDecodeFormat		= g_vpAudio.eEncodeFormat;

	/* AMR init */
	amrInitEncode ();
	amrInitDecode ();
	g_vpAudio.nAMRQualityLevel	= MAX_AMR_ENCODER_LEVEL;

	/* IMA ADPCM init */
	imaadpcmInit (1, AU_SAMPLE_RATE_8000, OPT_AUDIO_IMA_ENCODED_SIZE);
	ASSERT (sizeof (g_vpAudio.aucIMA_ADPCM_RingBuffer) == imaadpcmSamplePerBlock () * sizeof (short));
	g_vpAudio.uIMA_ADPCM_RingBuffer_Length = 0;
	
	/* LAW init */
	g_vpAudio.iLAWSize = 0;
	
	/* Notification playback */
	cyg_mutex_init(&g_vpAudio.mutNotificationLock);
	g_vpAudio.pucNotificationBuffer = NULL;
	g_vpAudio.iNotificationBufferLen = 0;
	g_vpAudio.iNotificationBufferPlayFrom = 0;
	g_vpAudio.iNotificationVolume = 2;
	g_vpAudio.notificationCallback = NULL;
	
	g_vpAudio.apEncodedAudio_Local[0] = NULL;
	g_vpAudio.apEncodedAudio_Local[1] = NULL;

#if defined OPT_USE_AUDIO_DECODER
	g_vpAudio.pEncodedAudio_Remote = bufEncAudioNew ();
	ASSERT (g_vpAudio.pEncodedAudio_Remote != NULL);
#else
	g_vpAudio.pEncodedAudio_Remote = NULL;
#endif
	listInit (&g_vpAudio.listEncodedAudio_Local);
	listInit (&g_vpAudio.listPCM_Local);
	listInit (&g_vpAudio.listPCM_Remote);
	tt_pc_init (&g_vpAudio.pcRemotePCM, bufRemotePCMGetTotalBlocksNum ());
	tt_rmutex_init (&g_vpAudio.mtLock);
	tt_cond_init (&g_vpAudio.cPlayWait, &g_vpAudio.mtLock);

#ifndef ECOS
	g_vpAudio.pDecMsgQueue = tt_msg_queue_init (g_vpAudio.auDecMsgBuffer,
											 sizeof (g_vpAudio.auDecMsgBuffer));
	g_vpAudio.pEncMsgQueue = tt_msg_queue_init (g_vpAudio.auEncMsgBuffer,
											 sizeof (g_vpAudio.auEncMsgBuffer));

	tt_create_thread ("audio_dec",
		1,
		g_vpAudio.auDecThreadBuffer,
		sizeof (g_vpAudio.auDecThreadBuffer),
		vauThreadEntry,
		g_vpAudio.pDecMsgQueue);
	tt_create_thread ("audio_enc",
		1,
		g_vpAudio.auEncThreadBuffer,
		sizeof (g_vpAudio.auEncThreadBuffer),
		vauThreadEntry,
		g_vpAudio.pEncMsgQueue);
#else
	tt_msg_queue_init (&(g_vpAudio.pDecMsgQueue), &(g_vpAudio.auDecMsgBuffer), sizeof (g_vpAudio.auDecMsgBuffer));
	tt_msg_queue_init (&(g_vpAudio.pEncMsgQueue), &(g_vpAudio.auEncMsgBuffer), sizeof (g_vpAudio.auEncMsgBuffer));
	
	cyg_thread_create(10, &vauThreadEntry, (cyg_addrword_t)(g_vpAudio.pDecMsgQueue), "audio_dec",
		g_vpAudio.auDecThreadBuffer, sizeof(g_vpAudio.auDecThreadBuffer),
		&(g_vpAudio.cygDecHandle), &(g_vpAudio.cygDecThread));
	cyg_thread_create(10, &vauThreadEntry, (cyg_addrword_t)(g_vpAudio.pEncMsgQueue), "audio_enc",
		g_vpAudio.auEncThreadBuffer, sizeof(g_vpAudio.auEncThreadBuffer),
		&(g_vpAudio.cygEncHandle), &(g_vpAudio.cygEncThread));
	cyg_thread_resume(g_vpAudio.cygDecHandle);
	cyg_thread_resume(g_vpAudio.cygEncHandle);
		
#endif


#if defined OPT_USE_AUDIO_DECODER
	g_vpAudio.pPlayBuffer	= bufAudioNew ();
	ASSERT (g_vpAudio.pPlayBuffer != NULL);
	g_vpAudio.pSplitterBuffer	= bufAuSplitterNew ();
	ASSERT (g_vpAudio.pSplitterBuffer != NULL);
#else
	g_vpAudio.pPlayBuffer	= NULL;
	g_vpAudio.pSplitterBuffer	= NULL;
#endif
	auSplitter_Init (g_vpAudio.pSplitterBuffer);
	
#if defined OPT_USE_AUDIO_ENCODER
	g_vpAudio.pRecordBuffer	= bufAudioNew ();
	ASSERT (g_vpAudio.pRecordBuffer != NULL);
#else
	g_vpAudio.pRecordBuffer	= NULL;
#endif
	
#if 1
	audio_WM8978_test( );
#else
	{
		AU_I2C_TYPE_T	i2cType;
#if defined ENV_DEMOBOARD_V11
		i2cType.bIsGPIO = FALSE;
		i2cType.uSDIN = 1;
		i2cType.uSCLK = 0;
#else
		i2cType.bIsGPIO = TRUE;
		i2cType.uSDIN = 5;
		//qzhang 2007/06/13
		//i2cType.uSCLK = 4;
		i2cType.uSCLK = 1;
#endif
		audioSetI2CType(i2cType);
	}

	
#if defined OPT_USE_AUDIO_DECODER
	audioSetPlayBuff ((UINT32) NON_CACHE (g_vpAudio.pPlayBuffer->aucPCM), sizeof (g_vpAudio.pPlayBuffer->aucPCM));
#endif

#if defined OPT_USE_AUDIO_ENCODER
	audioSetRecBuff ((UINT32) NON_CACHE (g_vpAudio.pRecordBuffer->aucPCM), sizeof (g_vpAudio.pRecordBuffer->aucPCM));
#endif

#if defined OPT_USE_AUDIO_DECODER
	vauStartPlay ();
#endif
	/* vauStartRecord is done by wb702EnableAudioEncoder */
	//vauStartRecord ();
#endif	
	return 0;
}



void vauUninit ()
{
	vauLock ();
	
	vauStopRecord ();
	vauStopPlay ();
	
	vauUnlock ();
}


static void __vauThread_OnWaitFrame (void *pArg)
{
#ifndef ECOS
	TT_SEM_T *ptMsgFlag = (TT_SEM_T *) pArg;
	tt_sem_up (ptMsgFlag);
#else
	cyg_sem_t *ptMsgFlag = (cyg_sem_t *) pArg;
	cyg_semaphore_post (ptMsgFlag);
#endif
}

static void vauEncWaitPrevMsg (void)
{
#ifndef ECOS
	TT_SEM_T tMsgFlag;
	/* Sent message to capture thread. */
	tt_sem_init (&tMsgFlag, 0);
	tt_msg_send (g_vpAudio.pEncMsgQueue, __vauThread_OnWaitFrame, (void *) &tMsgFlag);
	tt_sem_down (&tMsgFlag);
#else
	cyg_sem_t tMsgFlag;
	/* Sent message to capture thread. */
	cyg_semaphore_init (&tMsgFlag, 0);
	tt_msg_send (g_vpAudio.pEncMsgQueue, __vauThread_OnWaitFrame, (void *) &tMsgFlag);
	cyg_semaphore_wait (&tMsgFlag);
#endif
}


static void vauDecWaitPrevMsg (void)
{
#ifndef ECOS
	TT_SEM_T tMsgFlag;
	/* Sent message to capture thread. */
	tt_sem_init (&tMsgFlag, 0);
	tt_msg_send (g_vpAudio.pDecMsgQueue, __vauThread_OnWaitFrame, (void *) &tMsgFlag);
	tt_sem_down (&tMsgFlag);
#else
	cyg_sem_t tMsgFlag;
	/* Sent message to capture thread. */
	cyg_semaphore_init (&tMsgFlag, 0);
	tt_msg_send (g_vpAudio.pDecMsgQueue, __vauThread_OnWaitFrame, (void *) &tMsgFlag);
	cyg_semaphore_wait (&tMsgFlag);
#endif
}


INT vauStartPlay (void)
{
	INT rt;
	
	vauLock ();

	if (g_vpAudio.bPlayEnabled == FALSE)
	{
#ifndef ECOS
		EchoCancellationReset (&g_vpAudio.ecControl);
#endif
		rt = audioStartPlay (vauPlayCallback, AU_SAMPLE_RATE_8000, 1);
#ifndef ECOS
		sysSetLocalInterrupt(DISABLE_IRQ);
#endif
		//sysDisableIRQ ();
		audioSetPlayVolume (20, 20);
		//sysEnableIRQ ();

		g_vpAudio.iPlayIrqCount		= 0;
		tt_msleep(100);
		if (g_vpAudio.iPlayIrqCount == 0)
			ledError(LED_ERROR_AUDIO_CODEC);
		
		g_vpAudio.bPlayEnabled = TRUE;
	}
	
	vauUnlock ();
	
	vauDecWaitPrevMsg ();
	PTL;
	return rt;
}


INT vauStopPlay (void)
{
	INT rt;

	vauLock ();	
	
	if (g_vpAudio.bPlayEnabled == TRUE)
	{
		rt = audioStopPlay ();
		g_vpAudio.bPlayEnabled = FALSE;
	}
	
	vauUnlock ();
		
	//vauDecWaitPrevMsg ();
	return rt;
}


INT vauStartRecord (void)
{
	INT rt;

	vauLock ();	
	
	if (g_vpAudio.bRecordEnabled == FALSE)
	{
		rt = audioStartRecord (vauRecordCallback, AU_SAMPLE_RATE_8000, 1);
#ifndef ECOS
		sysSetLocalInterrupt(DISABLE_IRQ);
#endif
		//sysDisableIRQ ();
		audioSetRecordVolume (24, 24);
		//audioSetDacVolume(0, 0);
		audioSetDacVolume(-6, -6);
		//sysEnableIRQ ();
		
		g_vpAudio.iRecordIrqCount		= 0;
		tt_msleep(100);
		if (g_vpAudio.iRecordIrqCount == 0)
			ledError(LED_ERROR_AUDIO_CODEC);
		
		g_vpAudio.bRecordEnabled = TRUE;
	}
	vauUnlock ();
	
	vauEncWaitPrevMsg ();
	return rt;
}


INT vauStopRecord (void)
{
	INT rt;

	vauLock ();
	if (g_vpAudio.bRecordEnabled == TRUE)
	{
		rt = audioStopRecord ();
		g_vpAudio.bRecordEnabled = FALSE;
	}
	vauUnlock ();	
	
	//vauEncWaitPrevMsg ();
	return rt;
}


INT vauSetQualityLevel (INT nLevel)
{
	if (nLevel >= 0 && nLevel <= MAX_AMR_ENCODER_LEVEL)
	{
		g_vpAudio.nAMRQualityLevel = nLevel;
		return 0;
	}
	else
		return APP_ERR_CAPABILITY_LIMIT;
}


INT vauGetQualityLevel (void)
{
	return g_vpAudio.nAMRQualityLevel;
}

INT vauSetPlayVolume (INT lvolume, INT rvolume)
{
	INT rt;
	
	if (lvolume < 0)
	{
		lvolume = 0;
	}
	else if (lvolume > 31)
	{
		lvolume = 31;
	}
	if (rvolume < 0)
	{
		rvolume = 0;
	}
	else if (rvolume > 31)
	{
		rvolume = 31;
	}
	
	vauLock ();

	if (g_vpAudio.bPlayEnabled == TRUE)
	{
		rt = audioSetPlayVolume (lvolume, rvolume);
	}
	else
	{
		rt = -1;
	}
	
	vauUnlock ();
	
	return rt;
}

INT vauSetRecordVolume (INT lvolume, INT rvolume)
{
	INT rt;
	
	if (lvolume < 0)
	{
		lvolume = 0;
	}
	else if (lvolume > 31)
	{
		lvolume = 31;
	}
	if (rvolume < 0)
	{
		rvolume = 0;
	}
	else if (rvolume > 31)
	{
		rvolume = 31;
	}

#if 0	//xhchen, audio greater than 20 may have so may noise
	lvolume = lvolume * 20 / 32;
	rvolume = rvolume * 20 / 32;
#endif	
	
	vauLock ();

	if (g_vpAudio.bRecordEnabled == TRUE)
	{
		rt = audioSetRecordVolume (lvolume, rvolume);
	}
	else
	{
		rt = -1;
	}
	
	vauUnlock ();
	
	return rt;
}

INT vauSetNotificationVolume(INT volume)
{
	if (volume <= 1)
	{
		volume = 2;
	}
	else if (volume > AUDIO_NOTIFICATION_VOLUME_MAX)
	{
		volume = AUDIO_NOTIFICATION_VOLUME_MAX;
	}

	g_vpAudio.iNotificationVolume = volume;
	return 0;
}

INT vauSetFormat (CMD_AUDIO_FORMAT_E eEncodeFormat, CMD_AUDIO_FORMAT_E eDecodeFormat)
{
	g_vpAudio.eEncodeFormat = eEncodeFormat;
	g_vpAudio.eDecodeFormat = eDecodeFormat;
	return 0;
}


void audioUSleep (UINT32 uMicroSecond)
{
	tt_usleep (uMicroSecond);
}



void vauLock (void)
{
	tt_rmutex_lock (&g_vpAudio.mtLock);
}

int vauTryLockInThd (void)
{
	return tt_rmutex_try_lock_in_thd (&g_vpAudio.mtLock);
}

int vauTryLockInDsr (void)
{
	return tt_rmutex_try_lock_in_dsr (&g_vpAudio.mtLock);
}

int vauCanLock__remove_it (void)
{
	return tt_rmutex_can_lock__remove_it (&g_vpAudio.mtLock);
}

void vauUnlock (void)
{
	tt_rmutex_unlock (&g_vpAudio.mtLock);
}


void vauClearBuffer (void)
{
	int i;
	vauLock ();
	sysDisableIRQ ();
	while (1)
	{
		VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *pEncAudio;
		
		pEncAudio = vauGetEncBuffer_Local ();
		if (pEncAudio == NULL)
			break;
		else
		{
			listDetach (&pEncAudio->list);
			bufEncAudioDecRef (pEncAudio);
		}
	}

	for (i = 0;
		i < sizeof (g_vpAudio.apEncodedAudio_Local) / sizeof (g_vpAudio.apEncodedAudio_Local[0]);
		i++)
	{
		if (g_vpAudio.apEncodedAudio_Local[i] != NULL)
		{
			bufEncAudioDecRef (g_vpAudio.apEncodedAudio_Local[i]);	
			g_vpAudio.apEncodedAudio_Local[i] = NULL;
		}
	}

	sysEnableIRQ ();
	vauUnlock ();	
}

void vauNotificationPlay(UCHAR *pucBuffer, INT32 iBufferLen, void(*callback)(void))
{
	cyg_mutex_lock(&g_vpAudio.mutNotificationLock);
	
	/*
	 * If g_vpAudio.iNotificationVolume >= AUDIO_NOTIFICATION_VOLUME_MAX,
	 * it means no sound is played out.
	 */
	if (g_vpAudio.iNotificationVolume < AUDIO_NOTIFICATION_VOLUME_MAX)
	{
		g_vpAudio.pucNotificationBuffer = pucBuffer;
		g_vpAudio.iNotificationBufferLen = iBufferLen;
		g_vpAudio.iNotificationBufferPlayFrom = 0;
		g_vpAudio.notificationCallback = callback;
	}
	else
	{
		g_vpAudio.pucNotificationBuffer = NULL;
		g_vpAudio.iNotificationBufferLen = 0;
		g_vpAudio.iNotificationBufferPlayFrom = 0;
		g_vpAudio.notificationCallback = NULL;
	}
	
	cyg_mutex_unlock(&g_vpAudio.mutNotificationLock);
}

void vauNotificationGet(UCHAR *pucBuffer, UINT32 uiBufferLen)
{
	if (cyg_mutex_trylock(&g_vpAudio.mutNotificationLock) == TRUE)
	{
		if (g_vpAudio.pucNotificationBuffer != NULL)
		{
			int i;
			int copylen;
			int datalen;
			short pcm1, pcm2;
			datalen = g_vpAudio.iNotificationBufferLen - g_vpAudio.iNotificationBufferPlayFrom;
			copylen = (datalen > uiBufferLen ? uiBufferLen : datalen);
			
			for (i = 0; i < copylen; i += sizeof(short))
			{
				memcpy(&pcm1, pucBuffer + i, sizeof(short));
				memcpy(&pcm2, g_vpAudio.pucNotificationBuffer + g_vpAudio.iNotificationBufferPlayFrom + i, sizeof(short));
				pcm1 = pcm1 / 2 + pcm2 / g_vpAudio.iNotificationVolume;
				memcpy(pucBuffer + i, &pcm1, sizeof(short));
			}
			g_vpAudio.iNotificationBufferPlayFrom += copylen;
			
			if (g_vpAudio.iNotificationBufferPlayFrom >= g_vpAudio.iNotificationBufferLen)
			{
				if(g_vpAudio.notificationCallback != NULL)
				{
					(*g_vpAudio.notificationCallback)();
				}
				g_vpAudio.pucNotificationBuffer = NULL;
				g_vpAudio.iNotificationBufferLen = 0;
				g_vpAudio.iNotificationBufferPlayFrom = 0;
				g_vpAudio.notificationCallback = NULL;
			}
		}
		cyg_mutex_unlock(&g_vpAudio.mutNotificationLock);
	}
}



#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__

void cmd18_05_RecvMediaBitstream (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	CMD_MEDIA_TYPE_E	eMediaType = (CMD_MEDIA_TYPE_E) ucD;
	CMD_TYPE_E			eCmdType = (CMD_TYPE_E) ucA;

	DUMP_HIC_COMMAND;
		
	switch ((CMD_MEDIA_TYPE_E) eMediaType)
	{
		case CMD_MEDIA_VIDEO:
		{
			VP_BUFFER_MP4_BITSTREAM_T *pBitstream;
			pBitstream = vmp4encGetBuffer ();
			if (pBitstream == NULL)
			{
				hicSaveToReg1 (0);
				tt_msleep (1);
			}
			else
			{
				hicSaveToReg1 (pBitstream->uLength);
				if (eCmdType == CMD_TYPE_3)
				{
					hicMassData_MemToFifo (pHicThread, pBitstream->aucBitstream, pBitstream->uLength);
					sysDisableIRQ ();
					listDetach (&pBitstream->list);
					bufMP4BitstreamDecRef (pBitstream);
					sysEnableIRQ ();
				}
			}
			hicSetCmdReady (pHicThread);
			break;
		}
		case CMD_MEDIA_AUDIO:
		{
			VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *pBitstream;
			pBitstream = vauGetEncBuffer_Local ();
			if (pBitstream == NULL)
			{
				hicSaveToReg1 (0);
				tt_msleep (1);
			}
			else
			{
				hicSaveToReg1 (pBitstream->nSize);
				if (eCmdType == CMD_TYPE_3)
				{
					hicMassData_MemToFifo (pHicThread, pBitstream->aucAudio, pBitstream->nSize);
					vauLock ();
					listDetach (&pBitstream->list);
					bufEncAudioDecRef (pBitstream);
					vauUnlock ();
				}
			}
			hicSetCmdReady (pHicThread);
			break;
		}
		case CMD_MEDIA_JPEG:
		{
			VP_BUFFER_JPEG_ENCODER_T *pEncBuf;
			
			pEncBuf = vjpegGetEncBuffer ();
			if (pEncBuf == NULL)
			{
				hicSaveToReg1 (0);
				tt_msleep (1);
			}
			else
			{
				hicSaveToReg1 (pEncBuf->uJPEGDatasize);
				if (eCmdType == CMD_TYPE_3)
				{
					hicMassData_MemToFifo (pHicThread, pEncBuf->aucJPEGBitstream, pEncBuf->uJPEGDatasize);
					sysDisableIRQ ();
					listDetach (&pEncBuf->list);
					vjpegFreeEncBuffer (pEncBuf);
					sysEnableIRQ ();
				}
			}
			hicSetCmdReady (pHicThread);
			break;
		}
		default:
			hicSaveToReg1 (HIC_ERR_UNKNOWN_CMD_PARAM);
			hicSetCmdError (pHicThread);;
	}


}

#endif



#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd18_17_GetDecodedMedia (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	CMD_MEDIA_TYPE_E	eMediaType = (CMD_MEDIA_TYPE_E) ucD;
	CMD_TYPE_E			eCmdType = (CMD_TYPE_E) ucA;

	DUMP_HIC_COMMAND;
	
	switch (eMediaType)
	{
		case CMD_MEDIA_VIDEO:
		{
			VP_BUFFER_MP4_DECODER_T *pMP4DecBuf;
			pMP4DecBuf = vmp4decGetBuffer ();
			ASSERT (pMP4DecBuf != NULL);
			
			hicSaveToReg1 (pMP4DecBuf->nOutputLength);
			if (eCmdType == CMD_TYPE_3)
			{
				hicMassData_MemToFifo (pHicThread, pMP4DecBuf->aucOutput, pMP4DecBuf->nOutputLength);
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

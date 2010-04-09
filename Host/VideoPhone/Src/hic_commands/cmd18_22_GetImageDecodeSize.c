#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd18_22_GetImageDecodeSize (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	CMD_MEDIA_TYPE_E	eMediaType = (CMD_MEDIA_TYPE_E) ucD;
	
	DUMP_HIC_COMMAND;

	switch (eMediaType)
	{
		case CMD_MEDIA_VIDEO:
		{
			MP4DECINFO_T *pMP4Info = vmp4decGetInfo ();
			hicSaveToReg4 (
				(UCHAR) (pMP4Info->uPixels >> 8),
				(UCHAR) (pMP4Info->uPixels & (UCHAR) 0xFF),
				(UCHAR) (pMP4Info->uLines >> 8),
				(UCHAR) (pMP4Info->uLines & (UCHAR) 0xFF));
			hicSetCmdReady (pHicThread);
			break;
		}
		case CMD_MEDIA_JPEG:
		{
			JPEGDECINFO_T *pJPEGInfo = vjpegdecGetInfo();
			hicSaveToReg4 (
				(UCHAR) (pJPEGInfo->uPixels >> 8),
				(UCHAR) (pJPEGInfo->uPixels & (UCHAR) 0xFF),
				(UCHAR) (pJPEGInfo->uLines >> 8),
				(UCHAR) (pJPEGInfo->uLines & (UCHAR) 0xFF));
			hicSetCmdReady (pHicThread);
			break;
		}
		default:
		{
			hicSaveToReg1 (HIC_ERR_UNKNOWN_CMD_PARAM);
			hicSetCmdError (pHicThread);			
		}
	}	
}

#endif

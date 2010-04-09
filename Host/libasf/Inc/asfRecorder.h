#ifndef __ASF_IN_IPCAM_H__
#define __ASF_IN_IPCAM_H__

#include "asf_buffer_stream.h"
#include "asf_simple_index_object.h"
#include "asf_object.h"
typedef struct
{
	Rec_Buffer_Stream	*pRec_Buffer_Data;
	Asf_Buffer_Stream asf_stream;// = malloc(sizeof(Asf_Buffer_Stream));
	Head_Object_For_Record	asf_head_object;
	UINT8		*pucSampleBuff;
	INT			nSamplesPerBlock;
	INT			nBlockSize;
	UINT32 		uSampleSize;
	UINT32		uVidFrameInterval;
	UINT64		uVidNextSampleTime;
	UINT64 		uMediaDuration;
	INT 			nStatus;
	

	unsigned usImageWidth;
	unsigned usImageHeight;
	UINT32 uAuRecSRate;

#	define M4V_MAX_HEADER_LEN         128
	struct
	{
		char pucMpeg4Header[M4V_MAX_HEADER_LEN];
		UINT32 uMpeg4HeaderLength;
	} video_info;

	UINT32 uAuRecMediaLen;
	UINT32 uAuTotalFrames;
	UINT32 uVidTotalFrames;
	BOOL bIsRecordVideo;
	GUID  File_ID;
	ASF_QWORD	_uTotalFileSize;
	ASF_QWORD	_uASFFileLength;
	UINT32	_uASFRecEndTime;


} _ASF_RECORD_T;


INT asfRecorderInit (_ASF_RECORD_T *pRecordArg);
INT asfRecorderAddVideo (_ASF_RECORD_T *pRecordArg,char *pcData, size_t szLen);
INT asfRecorderAddAudio (_ASF_RECORD_T *pRecordArg,char *pcData, size_t szLen);
ASF_QWORD asfRecorderEnd (_ASF_RECORD_T *pRecordArg);
#endif

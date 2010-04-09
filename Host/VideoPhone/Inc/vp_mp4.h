#ifndef __VP_MP4_H__
#define __VP_MP4_H__


typedef enum
{
	VP_MP4_ENC_OK,
	VP_MP4_ENC_ERROR
} VP_MP4_ENC_STATE_E;

typedef enum
{
	VP_MP4_DEC_OK,
	VP_MP4_DEC_ERROR,
	VP_MP4_DEC_WAIT
} VP_MP4_DEC_STATE_E;

void vmp4Init(void);
void vmp4encAddRef (VP_VIDEO_T *pVideo);
void vmp4encDecRef (void);
void vmp4decAddRef (VP_VIDEO_T *pVideo);
void vmp4decDecRef (void);

int vmp4encStart (UINT32 uYUV_Addr, cyg_tick_count_t TimeStamp);
VP_MP4_ENC_STATE_E vmp4encWaitOK (void);
VP_BUFFER_MP4_BITSTREAM_T *vmp4encGetBuffer (void);
int vmp4decStart (UINT32 uBitstream_Addr, UINT32 uBitstream_Size);
VP_MP4_DEC_STATE_E vmp4decWaitOK (void);
VP_MP4_DEC_STATE_E vmp4decPrepare (UINT32 uBitstream_Addr, UINT32 uBitstream_Size);

int vmp4encSetBitrate(UINT32 uBitRate, UINT32 uHeight);
int vmp4encSetFramerate(UINT32 uFrameRate, UINT32 uHeight);

void vmp4ForceIFrame (UINT32 uNum, UINT32 uMillisecond);
int vmp4encSetQuality (UINT32 uFrameRate, UINT32 uBitRate);
int vmp4encGetQuality (UINT32 *puFrameRate, UINT32 *puBitRate);
int vmp4encSetFormat (CMD_VIDEO_FORMAT_E eFormat);

VP_BUFFER_MP4_DECODER_T *vmp4decGetBuffer (void);
void vmp4ClearBuffer (void);
MP4DECINFO_T *vmp4decGetInfo (void);

static __inline UINT32 vmp4decGetYSize (MP4DECINFO_T *pMP4Info)
{
	return pMP4Info->uLines * pMP4Info->uPixels;
}

static __inline UINT32 vmp4decGetUVSize (MP4DECINFO_T *pMP4Info)
{
	return vmp4decGetYSize (pMP4Info) /
#if defined OPT_CAPTURE_PLANAR_YUV420
		4
#elif defined OPT_CAPTURE_PLANAR_YUV422
		2
#endif
	;
}



void vmp4Lock (void);
int vmp4TryLock (void);
void vmp4Unlock (void);

#endif

#ifndef __VP_BUFFER_H__
#define __VP_BUFFER_H__



#define VP_BUF_APPEND_SIZE	16

typedef struct
{
	UCHAR aucData[MEM_ALIGN_SIZE (MAX_LCM_WIDTHxHEIGHT * MAX_LCM_BYTES_PER_PIXEL)];

	LIST_T list;
	int ref_count;
} VP_BUFFER_OSD_T;
DECLARE_MEM_POOL (bufOSD, VP_BUFFER_OSD_T)


typedef struct
{
	UCHAR aucData[MEM_ALIGN_SIZE (MAX_LCM_WIDTHxHEIGHT * MAX_LCM_BYTES_PER_PIXEL)];

	LIST_T list;
	int ref_count;
} VP_BUFFER_LCM_T;
DECLARE_MEM_POOL (bufLCM, VP_BUFFER_LCM_T)


typedef struct
{
	UCHAR aucData[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT * MAX_LCM_BYTES_PER_PIXEL)];
	
	LIST_T list;
	int ref_count;
} VP_BUFFER_VIDEO_T;
DECLARE_MEM_POOL (bufVideo, VP_BUFFER_VIDEO_T)


typedef struct
{
	/* Motion-detect use the size of width/8 and height/8. */
	UCHAR aucData[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT / 8 / 8)];
	UCHAR aucUV_Dummy[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT / 8 / 8 / 4)];
	
	LIST_T list;
	int ref_count;
} VP_BUFFER_MOTION_DETECT_T;
DECLARE_MEM_POOL (bufMotion, VP_BUFFER_MOTION_DETECT_T)


typedef struct
{
	UCHAR aucY[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT)];
#if defined OPT_CAPTURE_PLANAR_YUV422
	UCHAR aucU[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT / 2)];
	UCHAR aucV[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT / 2)];
#elif defined OPT_CAPTURE_PLANAR_YUV420
	UCHAR aucU[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT / 4)];
	UCHAR aucV[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT / 4)];
#endif

	//UINT32 uTimeStamp;
	cyg_tick_count_t tTimeStamp;
	LIST_T list;
	int ref_count;
} VP_BUFFER_CPT_PLANAR_T;
DECLARE_MEM_POOL (bufCptPlanar, VP_BUFFER_CPT_PLANAR_T)





typedef struct
{
	UCHAR aucBitstream[MEM_ALIGN_SIZE (MAX_ENCODED_IMAGE_SIZE)];
	UCHAR aucSplitterBuffer[MEM_ALIGN_SIZE (MAX_ENCODED_IMAGE_SIZE)];
#if defined OPT_CAPTURE_PLANAR_YUV420
	UCHAR aucReference[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT * 3 / 2)];
	UCHAR aucOutput[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT * 3 / 2)];
#elif defined OPT_CAPTURE_PLANAR_YUV422
	UCHAR aucReference[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT * 4 / 2)];
	UCHAR aucOutput[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT * 4 / 2)];
#endif
	INT nOutputLength;
	struct VP_MP4_SPLITTER_T
	{
		BOOL	bHeader;
		char	*pcBuffer;
		size_t	szMaxLength;
		size_t	szLength;
		char	*pcCheckPointer;
		size_t	szTotalLength;
		size_t	szGetTLength;
	} splitter;

	LIST_T list;
	int ref_count;
} VP_BUFFER_MP4_DECODER_T;
DECLARE_MEM_POOL (bufMP4Decoder, VP_BUFFER_MP4_DECODER_T)


typedef struct
{
#if defined OPT_CAPTURE_PLANAR_YUV420
	UCHAR aucReference[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT * 3 / 2)];
	UCHAR aucReconstructed[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT * 3 / 2)];
#elif defined OPT_CAPTURE_PLANAR_YUV422
	UCHAR aucReference[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT * 4 / 2)];
	UCHAR aucReconstructed[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT * 4 / 2)];
#endif

	LIST_T list;
	int ref_count;
} VP_BUFFER_MP4_ENCODER_T;
DECLARE_MEM_POOL (bufMP4Encoder, VP_BUFFER_MP4_ENCODER_T)

typedef struct
{
	/*
		I'm not sure the maximum bitstream size used by MP4.
		YC suggest the following size:
			for 640*480: 80K
			for 352*288: 56K
		Buffer with 64K bytes used here. Hope it's OK.
	 */
	/* VP_BUF_APPEND_SIZE more bytes for user-defined data */
	/* 8 more bytes for timestamp. */
	UCHAR aucBitstream[MEM_ALIGN_SIZE (VP_BUF_APPEND_SIZE + MAX_ENCODED_IMAGE_SIZE + 8)];
	UINT32 uLength;
	//UINT32 uTimeStamp;
	cyg_tick_count_t tTimeStamp;
	int nIndex;
	
	LIST_T list;
	int ref_count;
} VP_BUFFER_MP4_BITSTREAM_T;
DECLARE_MEM_POOL (bufMP4Bitstream, VP_BUFFER_MP4_BITSTREAM_T)


typedef struct
{
	UCHAR aucPCM[MEM_ALIGN_SIZE (OPT_AUDIO_BUFFER_SIZE) * 2];

	cyg_tick_count_t tTimeStamp;
	LIST_T list;
	int ref_count;
} VP_BUFFER_PCM_AUDIO_T;
DECLARE_MEM_POOL (bufAudio, VP_BUFFER_PCM_AUDIO_T)


typedef struct
{
	UCHAR aucPCM[MEM_ALIGN_SIZE (OPT_AUDIO_BUFFER_SIZE)];

	cyg_tick_count_t tTimeStamp;
	//UINT32 uTimeStamp;
	LIST_T list;
	int ref_count;
} VP_BUFFER_LOCAL_PCM_BITSTREAM_T;
DECLARE_MEM_POOL (bufLocalPCM, VP_BUFFER_LOCAL_PCM_BITSTREAM_T)


typedef struct
{
	UCHAR aucPCM[MEM_ALIGN_SIZE (OPT_AUDIO_BUFFER_SIZE)];

	LIST_T list;
	int ref_count;
} VP_BUFFER_REMOTE_PCM_BITSTREAM_T;
DECLARE_MEM_POOL (bufRemotePCM, VP_BUFFER_REMOTE_PCM_BITSTREAM_T)



typedef struct
{
	/* VP_BUF_APPEND_SIZE more bytes for user-defined data */
	/* 8 more bytes for timestamp. */
	UCHAR aucAudio[MEM_ALIGN_SIZE (MAX (OPT_AUDIO_MAX_DECODE_SIZE, VP_BUF_APPEND_SIZE + 8 + MAX (MAX (OPT_AUDIO_AMR_ENCODED_SIZE, OPT_AUDIO_IMA_ENCODED_SIZE), OPT_AUDIO_BUFFER_SIZE)))];
	int nSize;
	cyg_tick_count_t tTimeStamp;
	//UINT32 uTimeStamp;

	LIST_T list;
	int ref_count;
} VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T;
DECLARE_MEM_POOL (bufEncAudio, VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T)


typedef struct
{
//	UCHAR aucJPEGBitstream[MEM_ALIGN_SIZE (MAX_ENCODED_IMAGE_SIZE)];
	UCHAR aucJPEGBitstream[MEM_ALIGN_SIZE (MAX_ENCODED_JPEG_SIZE)];
	UINT32 uJPEGDatasize;
	
	LIST_T list;
	int ref_count;
} VP_BUFFER_JPEG_ENCODER_T;
DECLARE_MEM_POOL (bufEncJpeg, VP_BUFFER_JPEG_ENCODER_T)

#ifndef JPEG_DEC_WITH_MP4_BUFFER
typedef struct
{
	UCHAR aucY[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT)];
#if defined OPT_CAPTURE_PLANAR_YUV422
	UCHAR aucU[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT / 2)];
	UCHAR aucV[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT / 2)];
#elif defined OPT_CAPTURE_PLANAR_YUV420
	UCHAR aucU[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT / 4)];
	UCHAR aucV[MEM_ALIGN_SIZE (MAX_MP4_WIDTHxHEIGHT / 4)];
#endif
	
	UCHAR *pSrcImage;
	UINT32 uTimeStamp;
	LIST_T list;
	int ref_count;
} VP_BUFFER_JPEG_DECODER_T;
DECLARE_MEM_POOL (bufDecJpeg, VP_BUFFER_JPEG_DECODER_T)
#endif

void bufInit (void);
#endif

#include "../Inc/inc.h"



#pragma arm section zidata = "non_init"


/* Buffer for capturing planar data */
/*
	1. Two for capture
	2. One for convert captured image. (rotate..)
 */
#if defined OPT_USE_VIDEO_ENCODER
//#	define BUF_CPT_NUM	3
#	define BUF_CPT_NUM	2
#else
#	define BUF_CPT_NUM	0
#endif
IMPLEMENT_MEM_POOL (bufCptPlanar, VP_BUFFER_CPT_PLANAR_T, BUF_CPT_NUM)

/* Buffer for mp4 decoder */
#if defined OPT_USE_VIDEO_DECODER
#	define BUF_MP4_DECODER_NUM	1
#else
#	define BUF_MP4_DECODER_NUM	0
#endif
IMPLEMENT_MEM_POOL (bufMP4Decoder, VP_BUFFER_MP4_DECODER_T, BUF_MP4_DECODER_NUM)

/* Buffer for mp4 encoder */
#if defined OPT_USE_VIDEO_ENCODER
#	define BUF_MP4_ENCODER_NUM	1
#else
#	define BUF_MP4_ENCODER_NUM	0
#endif
IMPLEMENT_MEM_POOL (bufMP4Encoder, VP_BUFFER_MP4_ENCODER_T, BUF_MP4_ENCODER_NUM)


/* Buffer for encoded mp4 data */
#if defined OPT_USE_VIDEO_ENCODER
#	define BUF_MP4_BITSTREAM_NUM	3
//#	define BUF_MP4_BITSTREAM_NUM	2
#else
#	define BUF_MP4_BITSTREAM_NUM	0
#endif
IMPLEMENT_MEM_POOL (bufMP4Bitstream, VP_BUFFER_MP4_BITSTREAM_T, BUF_MP4_BITSTREAM_NUM)


/* Buffer for audio playback and record. */
#if defined OPT_USE_AUDIO_ENCODER
#	define BUF_AUDIO_NUM_a	1
#else
#	define BUF_AUDIO_NUM_a	0
#endif
#if defined OPT_USE_AUDIO_DECODER
#	define BUF_AUDIO_NUM_b	1
#else
#	define BUF_AUDIO_NUM_b	0
#endif
IMPLEMENT_MEM_POOL (bufAudio, VP_BUFFER_PCM_AUDIO_T, BUF_AUDIO_NUM_a + BUF_AUDIO_NUM_b)



/* Buffer for local pcm bistream */
#if defined OPT_USE_AUDIO_ENCODER
#	define BUF_LOCAL_PCM_NUM	2
#else
#	define BUF_LOCAL_PCM_NUM	0
#endif
IMPLEMENT_MEM_POOL (bufLocalPCM, VP_BUFFER_LOCAL_PCM_BITSTREAM_T, BUF_LOCAL_PCM_NUM)


/* Buffer for remote pcm bistream */
#if defined OPT_USE_AUDIO_DECODER
#	define BUF_REMOTE_PCM_NUM	4
#else
#	define BUF_REMOTE_PCM_NUM	0
#endif
IMPLEMENT_MEM_POOL (bufRemotePCM, VP_BUFFER_REMOTE_PCM_BITSTREAM_T, BUF_REMOTE_PCM_NUM)



/* Buffer for encoded audio bistream */
#if defined OPT_USE_AUDIO_ENCODER
#	define BUF_ENCODED_AUDIO_BITSTRAM_NUM_a	3
#else
#	define BUF_ENCODED_AUDIO_BITSTRAM_NUM_a	0
#endif
#if defined OPT_USE_AUDIO_DECODER
#	define BUF_ENCODED_AUDIO_BITSTRAM_NUM_b	1
#else
#	define BUF_ENCODED_AUDIO_BITSTRAM_NUM_b	0
#endif
IMPLEMENT_MEM_POOL (bufEncAudio, VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T, BUF_ENCODED_AUDIO_BITSTRAM_NUM_a + BUF_ENCODED_AUDIO_BITSTRAM_NUM_b)


/* Buffer for JPEG encoder. */
#if defined OPT_USE_AUDIO_ENCODER
#	define BUF_ENCODED_JPEG_BITSTRAM_NUM 2
#else
#	define BUF_ENCODED_JPEG_BITSTRAM_NUM 0
#endif
IMPLEMENT_MEM_POOL (bufEncJpeg, VP_BUFFER_JPEG_ENCODER_T, BUF_ENCODED_JPEG_BITSTRAM_NUM)

/* Buffer for JPEG decoder. */
#ifndef JPEG_DEC_WITH_MP4_BUFFER
#if defined OPT_USE_AUDIO_ENCODER
#	define BUF_DNCODED_JPEG_NUM 1
#else
#	define BUF_DNCODED_JPEG_NUM 0
#endif
IMPLEMENT_MEM_POOL (bufDecJpeg, VP_BUFFER_JPEG_DECODER_T, BUF_DNCODED_JPEG_NUM)
#endif


#pragma arm section zidata


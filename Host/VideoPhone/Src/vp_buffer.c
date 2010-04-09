#include "../Inc/inc.h"



#pragma arm section zidata = "non_init"


/* Buffer for OSD */
/*
	1. One for OSD screen
	2. One OSD update
 */
#if defined OPT_USE_LCM && defined OPT_USE_OSD
#	define BUF_OSD_NUM	2
#else
#	define BUF_OSD_NUM	0
#endif
IMPLEMENT_MEM_POOL (bufOSD, VP_BUFFER_OSD_T, BUF_OSD_NUM)


/* Buffer for LCM */
/*
	1. One for LCD Screen
*/
#if defined OPT_USE_LCM
#	define BUF_LCM_NUM	1
#else
#	define BUF_LCM_NUM	0
#endif
IMPLEMENT_MEM_POOL (bufLCM, VP_BUFFER_LCM_T, BUF_LCM_NUM)


/* Buffer for video */
/*
	1. One for Local video window
	2. One for Remote video window
	3. Two temp buffer for convert and resize local video
	4. Two temp buffer for convert and resize remote video
 */
#if defined OPT_USE_LCM && defined OPT_USE_VIDEO_ENCODER && defined OPT_VIDEO_ON_LCM
#	define BUF_VIDEO_NUM_a	3
#else
#	define BUF_VIDEO_NUM_a	0
#endif
#if defined OPT_USE_LCM && defined OPT_USE_VIDEO_DECODER && defined OPT_VIDEO_ON_LCM
#	define BUF_VIDEO_NUM_b	3
#else
#	define BUF_VIDEO_NUM_b	0
#endif
IMPLEMENT_MEM_POOL (bufVideo, VP_BUFFER_VIDEO_T, BUF_VIDEO_NUM_a + BUF_VIDEO_NUM_b)


/* Buffer for motion detect */
#if defined OPT_USE_MOTION
#	define BUF_MOTION_NUM 2
#else
#	define BUF_MOTION_NUM 0
#endif
IMPLEMENT_MEM_POOL (bufMotion, VP_BUFFER_MOTION_DETECT_T, BUF_MOTION_NUM)



#pragma arm section zidata



void bufInit (void)
{
	bufOSDInitMemoryPool ();
	bufLCMInitMemoryPool ();
	bufVideoInitMemoryPool ();
	bufMotionInitMemoryPool ();
	bufCptPlanarInitMemoryPool ();
	bufMP4DecoderInitMemoryPool ();
	bufMP4EncoderInitMemoryPool ();
	bufMP4BitstreamInitMemoryPool ();
	
	bufAudioInitMemoryPool ();
	bufLocalPCMInitMemoryPool ();
	bufRemotePCMInitMemoryPool ();
	bufEncAudioInitMemoryPool ();
	
	bufEncJpegInitMemoryPool ();
#ifndef JPEG_DEC_WITH_MP4_BUFFER
	bufDecJpegInitMemoryPool ();
#endif
}

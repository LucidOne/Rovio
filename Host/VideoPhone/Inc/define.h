#ifndef __DEFINE_H__
#define __DEFINE_H__


#define OPT_AUDIO_AMR_SOURCE_SIZE			(AMR_ENC_LENGTH * sizeof (short))
#define OPT_AUDIO_AMR_ENCODED_SIZE			AMR_ENC_MAX_PACKET_SIZE
#define OPT_AUDIO_IMA_SOURCE_SIZE			(1017 * sizeof (short))
#define OPT_AUDIO_IMA_ENCODED_SIZE			(IMA_PACKET_SIZE * sizeof (short))
#define OPT_AUDIO_BUFFER_SIZE				OPT_AUDIO_AMR_SOURCE_SIZE
#define OPT_AUDIO_MAX_DECODE_SIZE			1024



#if defined ENV_DEMOBOARD_V17
#include "define_demoboard_v17.h"

#elif defined ENV_DEMOBOARD_V17_ENCODE_ONLY
#include "define_demoboard_v17_encode_only.h"

#elif defined ENV_DEMOBOARD_V17_DECODE_ONLY
#include "define_demoboard_v17_decode_only.h"

#elif defined ENV_DEMOBOARD_V11
#include "define_demoboard_v11.h"

#elif defined ENV_RADIOTEL
#include "define_radiotel.h"

#elif defined ENV_IPCAM_WIFI
#include "define_ipcam_wifi.h"

#else
#error "No hardware environment defined"
#endif


#if MAX_MP4_WIDTHxHEIGHT <= (176 * 144)
#	define MAX_ENCODED_IMAGE_SIZE	(64*1024)
#	define MAX_ENCODED_JPEG_SIZE	(64*1024)
#	define MAX_MP4_BITRATE			(512*1024)
#elif MAX_MP4_WIDTHxHEIGHT <= (352 * 288)
//#	define MAX_ENCODED_IMAGE_SIZE	(128*1024)
#	define MAX_ENCODED_IMAGE_SIZE	(64*1024)
#	define MAX_ENCODED_JPEG_SIZE	(128*1024)
#	define MAX_MP4_BITRATE			(1024*1024)
#else
//#	define MAX_ENCODED_IMAGE_SIZE	(256*1024)
#	define MAX_ENCODED_IMAGE_SIZE	(128*1024)
#	define MAX_ENCODED_JPEG_SIZE	(64*1024)
#	define MAX_MP4_BITRATE			(1024*1024)
#endif


#if (defined OPT_CAPTURE_PLANAR_YUV420 && defined OPT_CAPTURE_PLANAR_YUV422) \
	|| (!defined OPT_CAPTURE_PLANAR_YUV420 && !defined OPT_CAPTURE_PLANAR_YUV422)
#error "Please select OPT_CAPTURE_PLANAR_YUV420 or OPT_CAPTURE_PLANAR_YUV422"
#endif


#endif

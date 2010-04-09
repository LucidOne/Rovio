#ifndef _MFL_CONFIG_H_
#define _MFL_CONFIG_H_

/*----------------------------------------------------------------------*
 *                                                                      *
 *  MFL internal settings                                               *
 *                                                                      *
 *----------------------------------------------------------------------*/
   
/* MP4/3GP playback meta data cache */
#define MP4_META_CACHE
#define MP4_MC_BLK_CNT			8		
#define MP4_MC_BLK_SIZ			1024

/* MP4/3GP temporary files */
extern  CHAR TEMP_DIR_ASCII[];
#define TEMP_STSZ_ASCII			"stsz"
#define TEMP_STTS_ASCII			"stts"
#define TEMP_STSO_ASCII			"stso"
#define TEMP_STSS_ASCII			"stss"
#define TEMP_STSC_ASCII			"stsc"
#define TEMP_STCO_ASCII			"stco"

/* buffer */
#define AUDIO_BUFF_SIZE			(1152*32+32)
//#define AUDIO_BUFF_SIZE			(1152*128)

#define AUDIO_DECODE_BUFF_SIZE  (8192)
								/* 1768x2 at least for MIDI */
								/* 8192 at least for AAC */

/* default time scale  */
#define MOVIE_TIME_SCALE		100
#define M4V_TIME_SCALE			3000

#define M4V_MAX_HEADER_LEN		128

/* AMR related */
#define AMRN_FRAME_PER_SAMPLE	1
#define AMRN_ENCODE_MODE_122	0x80		/* 12.2K mode */

/* message printing */
#define WARN(...)
#define DBPRINTF(...)

#endif	/* _MFL_CONFIG_H_ */
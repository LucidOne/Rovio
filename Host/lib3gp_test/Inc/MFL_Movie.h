#ifndef _MFL_Movie_h_
#define _MFL_Movie_h_

#include "wbtypes.h"

#include "MediaFileLib.h"
#include "MFL_Config.h"
#include "MFL_Stream.h"
#include "MP4_Descriptor.h"
#include "MP4_Atom.h"

typedef struct meta_cache_t
{
	INT		pnCbStartOff[MP4_MC_BLK_CNT];
	UINT32	puCbLastAccessTime[MP4_MC_BLK_CNT];
	UINT8	*pucCacheBuff;
}	META_CACHE_T;

typedef enum mv_stat_e
{
	MVS_PLAY_STOP = 0,
	MVS_REC_STOP = 0,
	MVS_PLAY_NORMAL = 1,
	MVS_PLAY_PAUSE,
	MVS_PLAY_SEEKING,
	MVS_PLAY_CLIPPING,
	MVS_REC_NORMAL = 100,
	MVS_REC_PAUSE
} MV_STAT_E;


typedef struct track_t
{
	//UINT32	fpos_trak;			/* position of the trak in mp4 file */
	//UINT32	trak_atom_len;		/* length of trak atom and all sub-atoms */
	
	UINT32	uCreationTime;
	UINT32	uModificationTime;
	UINT32	uTrackId;
	UINT32	uTrackDuration;		/* duaration in movie¡¦s time coordinate system */
	
	UINT32	uMediaTimeScale;	/* time-scale for media in this track */
	UINT64	uMediaDuration;		/* length of this media (in the scale of the uMediaTimeScale */
	UINT32	uTimeScale;

	UINT32	uSampleCount;		/* total number of samples in this track */
	UINT32	uSampleSize;		/* default sample size */
	
	STREAM_T tStrmStsz;			/* temporary file for sample size */
	STREAM_T tStrmStts;			/* temporary file sample time to decode */
	STREAM_T tStrmStso;			/* temporary file for sample offset */
	STREAM_T tStrmStss;			/* temporary file for video intra samples */
	
	UINT32 	uHandlerType;		/* from handler-type of 'hdlr' (odsm, vide, soun) */
	ES_Descriptor_T ES_D;		/* ES_Descriptor from 'esds' */

	INT		nH264LenghthSize;	/* AVC NAL unit size length */
	INT		nH264ParamPos;		/* AVC sequence parameter set position */

	/* for MP4 file generation */
	UINT8	pucMpeg4Header[M4V_MAX_HEADER_LEN];	/* video decoder specific data */
	UINT32	uMpeg4HeaderLength;	/* length of video decoder specific data */
	
	/* for playback run-time usage */
	BOOL	bDetermined;		/* the track type has beed determined or not */
	
	/* for record/playback run-time usage */
	UINT32	uLastDecodeTime;	/* RECORD - the decode time of last sample, in media time-scale */
	UINT32	uSttsPos;
	UINT32	uSttsEntryCnt;		/* RECORD - for STTS, the entry count of STTS atom */
	UINT32	uSttsSampleCnt;		/* for STTS */
	UINT32	uSttsDeltaTime;		/* for STTS, the current decode delta time */
	UINT64	u64SttsAccDelta;	/* accumulated delta time */
	UINT32	uStcoPos;			/* for STCO adjustment */
	UINT32	uStcoCurOff;
	UINT32	uStscPos;
	UINT32	uStscNumOfRoc;
	UINT32	uStscNextChunk;
	UINT32	uStscSamplesPerChunk;
	UINT32	uStscSampleCnt;
	UINT32	uStszPos;
	UINT32	uStssPos;
	UINT32	uStssEntryCnt;
	
	/* backup meta information for fast seek */
	UINT32	uSttsPosBk;
	UINT32	uStcoPosBk;
	UINT32	uStscPosBk;
	UINT32	uStszPosBk;
	UINT32	uStssPosBk;
	UINT32	uStscNumOfRocBk;
	
	/* for data chaining */
	struct track_t  *next;
	
}	TRACK_T;


typedef struct
{
	UINT16		wModeSet;			/* the active codec mode mapping */
	UINT8		ucModeChangePeriod;	/* the mode changes only at a multiple of N frames */
	UINT8		ucFramesPerSample;	/* number of frames to be considered as 'one sample' inside the 3GP file. */
}	AMR_T;


typedef struct 
{
	STREAM_T 	tInMediaStrm;		/* input media stream */
	STREAM_T 	tInMetaStrm;		/* input meta stream */
	STREAM_T	tOutMediaStrm;		/* output media data stream */
	STREAM_T	tOutMetaStrm;		/* output meta data stream */
	UINT32		uCreationTime;		/* seconds since 1904/01/01 00:00 */
	UINT32		uModificationTime; 	/* seconds since 1904/01/01 00:00 */
	UINT32		uTimeScale;			/* number of units pass in one second */
	UINT32		uDuration;			/* length of the movie presentation in uTimeScale */
	UINT8		nAuTrackCnt;		/* number of audio track */
	UINT8		nVidTrackCnt;		/* number of video track */
	INT			nAuFrameCnt;		/* total number of audio frames */
	INT			nVidFrameCnt;		/* total number of video frames */
	AMR_T		tAmr;				/* AMR specific info */
	
	TRACK_T		*ptAuTrack;
	TRACK_T		*ptVidTrack;
	TRACK_T		*ptMP4Track;		/* for run-time parsing usage */
	TRACK_T		*ptCurTrack;		/* for run-time parsing usage */
	
	UINT32		uMP4MetaSize;
	UINT32		uUdtaPos;			

	/* the followings are movie information */
	UINT32		uMP4MediaDataStart;	/* the file position of mdat content */
	UINT8		ucAudioProfile;		/* for AAC profile */
	UINT8		ucAudioLevel;		/* for AAC level */
	UINT8		ucVideoProfile;		/* for M4V profile */
	UINT8		ucVideoLevel;		/* for M4V level */
	
	/* proprietary informatin */
	BOOL  		bIsIac3gp;
	UINT8		pucIac3gpPsw[8];
}	MOVIE_T;


/* 3GP/MP4 fast forward cache */
typedef struct mp4_ffc_t
{
	BOOL		bIsCacheValid;
	UINT32		uTimePos;
	
   	UINT32		uVidStszPos;
   	UINT32		uVidStscPos;
   	UINT32		uVidSttsPos;
   	UINT32		uVidStcoPos;
	UINT32		uVidStssPos;
   	UINT32		uVidStscNumOfRoc;
   	UINT32		uVidStscSampleCnt;
   	UINT32		uVidStscNextChunk;
   	UINT32		uVidStscSamplesPerChunk;
   	UINT32		uVidStcoCurOff;
   	UINT32		uVidSttsSampleCnt;
   	UINT64		u64VidSttsAccDelta;
   	UINT32		uVidSttsDeltaTime;
   	UINT32		uVidFramesPlayed;
   	UINT32		uVidPlayMediaLen;
   	INT			nVidJiffySampleN;
   	INT			nVidJiffyStssN;
   	
   	UINT32		uAuStszPos;
   	UINT32		uAuStscPos;
   	UINT32		uAuSttsPos;
   	UINT32		uAuStcoPos;
   	UINT32		uAuStscNumOfRoc;
   	UINT32		uAuStscSampleCnt;
   	UINT32		uAuStscNextChunk;
   	UINT32		uAuStscSamplesPerChunk;
   	UINT32		uAuStcoCurOff;
   	UINT32		uAuSttsSampleCnt;
   	UINT64		u64AuSttsAccDelta;
   	UINT32		uAuFramesPlayed;
   	UINT32		uAuPlayMediaLen;
   	UINT32		uJiffyPcmCnt;
}	MP4_FFC_T;


#ifdef FOR_BIRD
typedef struct __INFO_STR
{
	unsigned char 		fs;  			/* transfer to sampling rate */
										/* 
											fs = 8  represented by 8Khz
											fs = 16 represented by 16Khz
											fs = 24 represented by 24Khz
											fs = 32 represented by 32Khz
											fs = 48 represented by 48Khz
										*/
	char				bISfirstframe;  /* indicate the first frame */
	unsigned short 		remainder;  	/* to next frame interpolation information */ 
	short				lastvalue;		/* previous frame last value */
	short 				buffin[160];	/* for AMR one frame out sample number with one channel(mono)*/
	short 				*buffout; 		/* output sample buffer with changed by fs */

	 
} INFO_STR;

extern INFO_STR _t8KUpTo48K;
extern short _8KpcmToNKpcm(INFO_STR *ContrlBox);

#endif


/* Macros */
#define GET32_B(x)	((x[0] << 24)|(x[1] << 16)|(x[2] << 8)|x[3])
#define GET64_B(x)	(((UINT64)((x[0]<<24)|(x[1]<<16)|(x[2]<<8)|x[3])) << 32) | ((buff[4]<<24)|(buff[5]<<16) | (buff[6]<<8) | buff[7])

#define	MIN(x,y)	(((x) <	(y)) ? (x) : (y))
//#define	MAX(x,y)	(((x) >	(y)) ? (x) : (y))


/*
 * MFL internal used global variable
 */
extern UINT8  *_mfl_pucAuDecodeBuff;
extern INT  _nSpeedNumerator, _nSpeedDenuminator;

/*
 * External Functions
 */
/* movie group */
INT  mfl_common_init(MV_CFG_T *ptMvCfg);

/* mp4 group */
extern INT  mfl_mp4_preview(MV_CFG_T *ptMvCfg);
extern INT  mfl_mp4_player(MV_CFG_T *ptMvCfg);
extern INT  mfl_mp4_recorder(MV_CFG_T *ptMvCfg);
extern INT  mfl_mp4_clipper(MV_CFG_T *ptMvCfg);
extern INT  mfl_mp4_write_movie_meta(MV_CFG_T *ptMvCfg);
extern VOID mfl_mp4_print_track_info(TRACK_T *track);
extern VOID mfl_mp4_print_movie_info(VOID);
extern VOID mfl_mp4_add_track(TRACK_T * *track_list, TRACK_T *new_track);
extern INT  mfl_mp4_new_au_track(MV_CFG_T *ptMvCfg);
extern INT  mfl_mp4_new_vid_track(MV_CFG_T *ptMvCfg);
extern VOID mfl_mp4_destroy_track(TRACK_T *track);

extern INT  mfl_mp4_read_atom_header(STREAM_T *stream, UINT32 *unAtomType, UINT32 *uAtomSize);
extern INT  mfl_mp4_write_atom_header(STREAM_T *stream, UINT32 unAtomType, UINT32 uAtomSize, BOOL bIsFullAtom, UINT32 uVersionFlag);
extern INT  mfl_mp4_parsing_atom(MV_CFG_T *ptMvCfg, UINT32 unAtomType, UINT32 uAtomSize);
extern VOID mfl_mp4_back_write_atom(STREAM_T *stream, UINT32 uAtomPos, UINT32 uAtomType, INT bIsFullAtom, UINT32 uVersionFlag);
extern INT  mfl_mp4_write_sizeOfInstance(STREAM_T *stream, INT sizeOfInstance);
extern INT  mfl_mp4_parse_OD(ES_Descriptor_T *ES_D, INT *readCnt);
extern INT  mfl_mp4_parse_ESD(MV_CFG_T *ptMvCfg, ES_Descriptor_T *ES_D, INT *readCnt);
extern INT  mfl_mp4_write_ESD(MV_CFG_T *ptMvCfg, TRACK_T *track, STREAM_T *stream);
extern INT  mfl_mp4_parse_pce(STREAM_T *stream, GASpecificConfig_T *ga, INT *readCnt);
//extern INT  mfl_mp4_set_pce(STREAM_T *stream, GASpecificConfig_T *ga);
extern INT  mfl_mp4_write_pce(ES_Descriptor_T *ES_D, STREAM_T *stream);
extern UINT32  mfl_mp4_sfreq_lookup(INT idx);
extern INT  mfl_mp4_sfreq_index(UINT32 sfreq);
extern INT  mp4_get_next_sample(MV_CFG_T *ptMvCfg, TRACK_T *ptTrack, UINT8 *pucBuff, UINT32 *uSampleSize, META_CACHE_T *ptMC);
extern INT  mp4_fast_seek(MV_CFG_T *ptMvCfg, META_CACHE_T *ptMC, INT uTimePos, BOOL bIsHwIndependent);
extern INT  create_track_temp_files(MV_CFG_T *ptMvCfg, TRACK_T *ptTrack);
extern VOID  rewind_track_temp_files(TRACK_T *ptTrack);
extern INT  vid_get_next_decode_time(MV_CFG_T *ptMvCfg, META_CACHE_T *ptMC, UINT64 *u64DecodeTime);
extern INT  read_meta_with_cache(STREAM_T *ptInMediaStrm, INT nFileOffset, UINT32 *uValue, META_CACHE_T *ptMC);

/* AAC group */
extern INT  mfl_aac_preview(MV_CFG_T *ptMvCfg);
extern INT  mfl_aac_player(MV_CFG_T *ptMvCfg);
extern INT  mfl_aac_recorder(MV_CFG_T *ptMvCfg);
extern INT  mfl_aac_clipper(MV_CFG_T *ptMvCfg);

/* MP3 group */
extern VOID GetID3Tag(MV_CFG_T *ptMvCfg, INT *nID3V2Offset, INT *nID3V2Length);
extern INT  mfl_mp3_preview(MV_CFG_T *ptMvCfg);
extern INT  mfl_mp3_player(MV_CFG_T *ptMvCfg, UINT8 *pucMetaBuff, INT nMetaSize);
extern INT  mfl_mp3_clipper(MV_CFG_T *ptMvCfg);

/* WMA group */
extern INT  mfl_wma_preview(MV_CFG_T *ptMvCfg);
extern INT  mfl_wma_player(MV_CFG_T *ptMvCfg);

/* AMR group */
extern INT  mfl_amr_preview(MV_CFG_T *ptMvCfg);
extern INT  mfl_amr_player(MV_CFG_T *ptMvCfg);
extern INT  mfl_amr_recorder(MV_CFG_T *ptMvCfg);


/* WAV group */
extern INT  mfl_wav_preview(MV_CFG_T *ptMvCfg);
extern INT  mfl_wav_player(MV_CFG_T *ptMvCfg);
extern INT  mfl_wav_recorder(MV_CFG_T *ptMvCfg);

/* MIDI group */
extern INT  mfl_midi_preview(MV_CFG_T *ptMvCfg);
extern INT  mfl_midi_player(MV_CFG_T *ptMvCfg);

/* ASF group */
extern INT  mfl_asf_player(MV_CFG_T *ptMvCfg);
extern INT  mfl_asf_preview(MV_CFG_T *ptMvCfg);
extern INT	mfl_asf_recorder(MV_CFG_T *ptMvCfg);


/* W99702 audio group */
extern INT mfl_au_init_device(MV_CFG_T *ptMvCfg, BOOL bPlay, BOOL bRecord);
extern VOID mfl_au_stop_device(MV_CFG_T *ptMvCfg, BOOL bPlay, BOOL bPlayRemaining, BOOL bRecord);

extern INT mfl_au_start_record(MV_CFG_T *ptMvCfg);
extern UINT32 mfl_au_rec_buff_used_space(VOID);
extern UINT32 mfl_au_rec_next_addr(UINT32 uFrameSize);
extern VOID mfl_au_set_rec_cnt(UINT32 value);
extern UINT32 mfl_au_rec_data_cnt(MV_CFG_T *ptMvCfg);

extern INT mfl_au_start_play(MV_CFG_T *ptMvCfg);
extern UINT32 mfl_au_play_buff_size(VOID);
extern UINT32 mfl_au_play_space_to_end(VOID);
extern VOID mfl_au_clear_play_buff(MV_CFG_T *ptMvCfg);
extern UINT32 mfl_au_play_buff_free_space(VOID);
extern VOID mfl_au_set_play_cnt(UINT32 value);
extern UINT32 mfl_au_play_data_cnt(MV_CFG_T *ptMvCfg);
extern INT  mfl_au_post_processing(MV_CFG_T *ptMvCfg, UINT8 *pucOutBuff, INT nFrameSize, INT nSamples);


/* Others group */
extern INT  mfl_wait_flag_set(volatile INT *flag, INT time_out);
extern INT  mfl_open_temp_file(BOOL bUseTempFile, STREAM_T *stream, CHAR *szFName, INT nId, INT nAccess);

/* 
 * Export Functions - 2nd layers
 */
extern MOVIE_T *mfl_new_movie(VOID);
extern VOID mfl_destroy_movie(MOVIE_T *);
extern INT  mfl_parse_mp4_file(MV_CFG_T *ptMvCfg);
extern INT  mfl_write_mp4metadata(VOID);

#endif	/* _MFL_Movie_h_ */



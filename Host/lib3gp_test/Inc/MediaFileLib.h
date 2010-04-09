#ifndef _MEDIA_FILE_LIB_H_
#define _MEDIA_FILE_LIB_H_


/*-------------------------------------------------------------------------*
 *  MFL error code table                                                   *
 *-------------------------------------------------------------------------*/
/* general errors */
#define MFL_ERR_NO_MEMORY		0xFFFF8000	/* no memory */
#define MFL_ERR_HARDWARE		0xFFFF8002	/* hardware general error */
#define MFL_ERR_NO_CALLBACK		0xFFFF8004	/* must provide callback function */
#define MFL_ERR_AU_UNSUPPORT	0xFFFF8006	/* not supported audio type */
#define MFL_ERR_VID_UNSUPPORT	0xFFFF8008	/* not supported video type */
#define MFL_ERR_TRACK_UNSUPPORT	0xFFFF800A	/* not supported track type */
#define MFL_ERR_OP_UNSUPPORT	0xFFFF800C	/* unsupported operation */
#define MFL_ERR_PREV_UNSUPPORT	0xFFFF800E	/* preview of this media type was not supported or not enabled */
#define MFL_ERR_FUN_USAGE		0xFFFF8010	/* incorrect function call parameter */
#define MFL_ERR_RESOURCE_MEM	0xFFFF8012	/* memory is not enough to play/record a media file */
/* stream I/O errors */
#define MFL_ERR_FILE_OPEN		0xFFFF8020	/* cannot open file */
#define MFL_ERR_FILE_TEMP		0xFFFF8022	/* temporary file access failure */
#define MFL_ERR_STREAM_IO		0xFFFF8024	/* stream access error */
#define MFL_ERR_STREAM_INIT		0xFFFF8026	/* stream was not opened */
#define MFL_ERR_STREAM_EOF		0xFFFF8028	/* encounter EOF of file */
#define MFL_ERR_STREAM_SEEK		0xFFFF802A	/* stream seek error */
#define MFL_ERR_STREAM_TYPE		0xFFFF802C	/* incorrect stream type */
#define MFL_ERR_STREAM_METHOD	0xFFFF8030	/* missing stream method */
#define MFL_ERR_STREAM_MEMOUT	0xFFFF8032	/* recorded data has been over the application provided memory buffer */
#define MFL_INVALID_BITSTREAM	0xFFFF8034	/* invalid audio/video bitstream format */
/* MP4/3GP file errors */
#define MFL_ERR_MP4_FILE		0xFFFF8050	/* wrong MP4 file format or not supported */
#define MFL_ERR_3GP_FILE		0xFFFF8052	/* wrong 3GP file format or not supported */
#define MFL_ERR_NO_TKHD			0xFFFF8054	/* preceding 'tkhd' not present */
#define MFL_ERR_MP4_STSC		0xFFFF8056	/* MP4 'stsc' atom contain error */
#define MFL_ERR_MP4_STTS		0xFFFF8058	/* MP4 'stts' atom contain error */
#define MFL_ERR_ATOM_UNKNOWN	0xFFFF805A	/* unknown atom type */
#define MFL_ERR_DATA_REF		0xFFFF805C	/* media data is not in the same file, currently not supported */
#define MFL_ERR_SAMPLE_OVERSIZE	0xFFFF8060	/* MP4/3GP video or audio sample size too large */
#define MFL_ERR_PLAY_DONE		0xFFFF8062	/* MFL internal use, not an error */
#define MFL_ERR_SEEK_FAIL 		0xFFFF8070	/* MFL can not seek to specified time position */
#define MFL_ERR_CLIP_EXCEED		0xFFFF8072	/* media clipping position exceed range */
#define MFL_ERR_CLIP_LOCATE  	0xFFFF8074	/* media clipping can not locate position */
#define MFL_ERR_CLIP_CREATE		0xFFFF8076	/* media clipping can not create output file */
#define MFL_ERR_H264_COMBO		0xFFFF8078	/* MFL internal use, not an error */
/* AMR errors */
#define MFL_ERR_AMR_FORMAT		0xFFFF8090	/* incorrect AMR frame format */
#define MFL_ERR_AMR_DECODE		0xFFFF8092	/* AMR decode error */
#define MFL_ERR_AMR_ENCODE		0xFFFF8094	/* AMR encode error */
#define MFL_AMR_DECODER_INIT	0xFFFF8096	/* failed to init AMR decoder */
#define MFL_AMR_ENCODER_INIT	0xFFFF8098	/* failed to init AMR decoder */
#define MFL_ERR_AMR_UNSUPPORT	0xFFFF809A	/* unsupported AMR frame type */
/* AAC errors */
#define MFL_ERR_AAC_FORMAT		0xFFFF80B0	/* incorrect AAC frame format */
#define MFL_ERR_AAC_DECODE		0xFFFF80B2	/* AAC decode error */
#define MFL_ERR_AAC_ENCODE		0xFFFF80B4	/* AAC encode error */
#define MFL_ERR_AAC_UNSUPPORT	0xFFFF80B6	/* unsupported AAC frame type */
#define MFL_ERR_AAC_CHANNELS	0xFFFF80B8	/* more than 2 channels is not supported */
/* MP3 errors */
#define MFL_ERR_MP3_FORMAT		0xFFFF80D0	/* incorrect MP3 frame format */
#define MFL_ERR_MP3_DECODE		0xFFFF80D2	/* MP3 decode error */
/* WAV errors */
#define MFL_ERR_WAV_FILE		0xFFFF80F0	/* wave file format error */
#define MFL_ERR_WAV_UNSUPPORT	0xFFFF80F2	/* unsupported wave format */
#define MFL_ERR_ADPCM_SUPPORT	0xFFFF80F4	/* can only handle 4-bit IMA ADPCM in wav files */
/* hardware engine errors */
#define MFL_ERR_HW_NOT_READY	0xFFFF8100	/* the picture is the same as the last one */
#define MFL_ERR_VID_NOT_DTIME	0xFFFF8102	/* not specified decode time, internal use */
#define MFL_ERR_SHORT_BUFF		0xFFFF8104	/* buffer size is not enough */
#define MFL_ERR_VID_DEC_ERR		0xFFFF8106	/* video decode error */
#define MFL_ERR_VID_DEC_BUSY	0xFFFF8108	/* video decoder is busy */
#define MFL_ERR_VID_ENC_ERR		0xFFFF810A	/* video encode error */
#define MFL_ERR_AU_PBUF_FULL	0xFFFF810C	/* audio play buffer full, internal use */
#define MFL_ERR_AU_RDATA_LOW	0xFFFF8110	/* audio record data not enough, internal use */
#define MFL_ERR_AU_UNDERFLOW	0xFFFF8112	/* audio record data underflow, internal use */
#define MFL_ERR_AU_OVERFLOW		0xFFFF8114	/* audio play data overflow, internal use */
/* MPEG4 codec error */
#define MFL_ERR_H263_SIZE		0xFFFF8140	/* wrong H.263 header */
#define MFL_ERR_VOL_NOT_FOUND	0xFFFF8150	/* MPEG4 video VOL not found */
/* Audio post processing error */
#define MFL_ERR_3D_INIT			0xFFFF8160	/* 3D initialize error */
/* other errors */
#define MFL_ERR_INFO_NA			0xFFFF81E0	/* media information not enough */
#define MFL_ERR_UNKNOWN_MEDIA	0xFFFF81E2	/* unknow media type */
#define MFL_ERR_MOVIE_PLAYING	0xFFFF81E4	/* movie is still in play */
#define MFL_ERR_ULTRAM_TMPF		0xFFFF81E6	/* ultra merge file stream must use temp file */

/*-------------------------------------------------------------------------*
 *  Media type enumeration                                                 *
 *-------------------------------------------------------------------------*/
typedef enum media_type_e
{
	MFL_MEDIA_MP4,
	MFL_MEDIA_3GP,
	MFL_MEDIA_M4V,
	MFL_MEDIA_AAC,
	MFL_MEDIA_AMR,
	MFL_MEDIA_MP3,
	MFL_MEDIA_ADPCM,
	MFL_MEDIA_WAV,
	MFL_MEDIA_MIDI,
	MFL_MEDIA_AVI,
	MFL_MEDIA_ASF,
	MFL_MEDIA_WMV,
	MFL_MEDIA_WMA,
	MFL_MEDIA_SMAF,
	MFL_MEDIA_M4A,
	MFL_MEDIA_WBKTV,
	MFL_MEDIA_UNKNOWN = 1000
} MEDIA_TYPE_E;


/*-------------------------------------------------------------------------*
 *  S/W audio codec enumeration                                            *
 *-------------------------------------------------------------------------*/
typedef enum au_codec_e
{
	MFL_CODEC_PCM,
	MFL_CODEC_AMRNB,
	MFL_CODEC_AMRWB,
	MFL_CODEC_AAC,
	MFL_CODEC_AACP,
	MFL_CODEC_EAACP,
	MFL_CODEC_ADPCM,
	MFL_CODEC_MP3,
	MFL_CODEC_UNKNOWN = 1000
} AU_CODEC_E;


/*-------------------------------------------------------------------------*
 *  H/W video codec enumeration                                            *
 *-------------------------------------------------------------------------*/
typedef enum vid_codec_e
{
	MFL_CODEC_H263,
	MFL_CODEC_MPEG4,
	MFL_CODEC_H264
} VID_CODEC_E;



/*-------------------------------------------------------------------------*
 *  MFL data stream type enumeration                                       *
 *-------------------------------------------------------------------------*/
typedef enum stream_type_e
{
	MFL_STREAM_MEMORY,
	MFL_STREAM_FILE,
	MFL_STREAM_USER
} STRM_TYPE_E;


/*-------------------------------------------------------------------------*
 *  Physical audio playback device type enumeration                        *
 *-------------------------------------------------------------------------*/
typedef enum au_play_dev_e
{
	MFL_PLAY_AC97 = 0,
	MFL_PLAY_I2S = 1,
	MFL_PLAY_UDA1345TS = 2,
	MFL_PLAY_DAC = 4,
	MFL_PLAY_MA3 = 5,
	MFL_PLAY_MA5 = 6,
	MFL_PLAY_W5691 = 7,
	MFL_PLAY_WM8753 = 8,
	MFL_PLAY_WM8751 = 9,
	MFL_PLAY_WM8978 = 10,
	MFL_PLAY_MA5I = 11,
	MFL_PLAY_MA5SI = 12,
	MFL_PLAY_W56964 = 13,
	MFL_PLAY_AK4569 = 14,
	MFL_PLAY_TIAIC31 = 15,
	MFL_PLAY_WM8731 = 16
} AU_PLAY_DEV_E;


/*-------------------------------------------------------------------------*
 *  Physical audio record device type enumeration                          *
 *-------------------------------------------------------------------------*/
typedef enum au_rec_dev_e
{
	MFL_REC_AC97 = 0,
	MFL_REC_I2S = 1,
	MFL_REC_UDA1345TS = 2,
	MFL_REC_ADC = 3,
	MFL_REC_W5691 = 7,
	MFL_REC_WM8753 = 8,
	MFL_REC_WM8751 = 9,
	MFL_REC_WM8978 = 10,
	MFL_REC_AK4569 = 14,
	MFL_REC_TIAIC31 = 15,
	MFL_REC_WM8731 = 16
} AU_REC_DEV_E;


/*-------------------------------------------------------------------------*
 *  Audio sampling rate enumeration                                        *
 *-------------------------------------------------------------------------*/
typedef enum au_srate_e
{
	AU_SRATE_8000 = 8000,
	AU_SRATE_11025 = 11025,
	AU_SRATE_12000 = 12000,
	AU_SRATE_16000 = 16000,
	AU_SRATE_22050 = 22050,
	AU_SRATE_24000 = 24000,
	AU_SRATE_32000 = 32000,
	AU_SRATE_44100 = 44100,
	AU_SRATE_48000 = 48000
} AU_SRATE_E;


/*-------------------------------------------------------------------------*
 *  MFL playback control enumeration                                       *
 *-------------------------------------------------------------------------*/
typedef enum play_ctrl_e
{
	PLAY_CTRL_FF = 0,
	PLAY_CTRL_FB,
	PLAY_CTRL_FS,
	PLAY_CTRL_PAUSE,
	PLAY_CTRL_RESUME,
	PLAY_CTRL_STOP,
	PLAY_CTRL_SPEED
} PLAY_CTRL_E;


enum 
{
	PLAY_SPEED_QUARTER = 0x0104,
	PLAY_SPEED_HALF = 0x0102,
	PLAY_SPEED_NORMAL = 0x0101,
	PLAY_SPEED_DOUBLE = 0x0201,
	PLAY_SPEED_QUADRUPLE = 0x0401
};


/*-------------------------------------------------------------------------*
 *  MFL record control enumeration                                         *
 *-------------------------------------------------------------------------*/
typedef enum rec_ctrl_e
{
	REC_CTRL_PAUSE = 0,
	REC_CTRL_RESUME,
	REC_CTRL_STOP
} REC_CTRL_E;


/*-------------------------------------------------------------------------*
 *  MFL equalizer settings enumeration                                     *
 *-------------------------------------------------------------------------*/
typedef enum eq_eft_e
{
	EQ_DEFAULT = 0,
	EQ_CLASSICAL,
	EQ_CLUB,
	EQ_DANCE,
	EQ_FULL_BASS,
	EQ_LIVE,
	EQ_POP,
	EQ_ROCK, 
	EQ_SOFT,
	EQ_USER_DEFINED
} EQ_EFT_E;


/*-------------------------------------------------------------------------*
 *  QSound 3D surround settings                                            *
 *-------------------------------------------------------------------------*/
typedef enum 
{
    MFL_Q3D_ROOM = 1,
    MFL_Q3D_BATHROOM,
    MFL_Q3D_CONCERTHALL,
    MFL_Q3D_CAVE,
    MFL_Q3D_ARENA,
    MFL_Q3D_FOREST,
    MFL_Q3D_CITY,
    MFL_Q3D_MOUNTAINS,
    MFL_Q3D_UNDERWATER,
	MFL_Q3D_AUDITORIUM,
	MFL_Q3D_ALLEY,
	MFL_Q3D_HALLWAY,
	MFL_Q3D_HANGAR,
	MFL_Q3D_HANGER,
	MFL_Q3D_LIVINGROOM,
	MFL_Q3D_SMALLROOM,
	MFL_Q3D_MEDIUMROOM,
	MFL_Q3D_LARGEROOM,
	MFL_Q3D_MEDIUMHALL,
	MFL_Q3D_LARGEHALL,
	MFL_Q3D_PLATE,
	MFL_Q3D_GENERIC,
	MFL_Q3D_PADDEDCELL,
	MFL_Q3D_STONEROOM,
	MFL_Q3D_CARPETEDHALLWAY,
	MFL_Q3D_STONECORRIDOR,
	MFL_Q3D_QUARRY,
	MFL_Q3D_PLAIN,
	MFL_Q3D_PARKINGLOT,
	MFL_Q3D_SEWERPIPE
} Q3D_REVERB_T;

typedef enum 
{
	MFL_Q3D_SPEAKERS,
	MFL_Q3D_HEADPHONES
} Q3D_OUTTYPE_T;

typedef enum 
{
	MFL_Q3D_GEOMETRY_SPEAKER_DESKTOP,
	MFL_Q3D_GEOMETRY_SPEAKER_FRONT,
	MFL_Q3D_GEOMETRY_SPEAKER_SIDE
} Q3D_GEOMETRY_T;


typedef struct q3d_config_t
{
	Q3D_REVERB_T	eQ3D_Reverb;
	Q3D_OUTTYPE_T	eQ3D_OutType;
	Q3D_GEOMETRY_T	eQ3D_Geometry;
}	Q3D_CONFIG_T;

/*-------------------------------------------------------------------------*
 *  MFL data stream function set                                           *
 *-------------------------------------------------------------------------*/
struct stream_t;

typedef struct strm_fun
{
	INT (*open)(struct stream_t *ptStream, CHAR *suFileName, CHAR *szAsciiName, 
	             UINT8 *pbMemBase, UINT32 ulMemLen, INT access);
	INT (*is_opened)(struct stream_t *ptStream);
    INT (*seek)(struct stream_t *ptStream, INT nOffset, INT nSeek);
    INT (*get_pos)(struct stream_t *ptStream);
	INT (*close)(struct stream_t *ptStream);
	INT (*read)(struct stream_t *ptStream, UINT8 *pucBuff, INT nCount, BOOL bIsSkip);
	INT (*write)(struct stream_t *ptStream, UINT8 *pucBuff, INT nCount);
	INT (*peek)(struct stream_t *ptStream, UINT8 *pucBuff, INT nCount);
	INT (*get_bsize)(struct stream_t *ptStream);
}	STRM_FUN_T;


typedef struct stream_t
{
	STRM_TYPE_E eStrmType;
	UINT8		*pucMemBase;	/* start address of memory block used by this stream */
	UINT8		*pucMemEnd;		/* end address of memory block used by this stream */
	UINT32		uMemValidSize;  /* valid size in the memory block used by this stream */ 
	UINT8		*pucMemPtr;		/* current read/write address of stream in memory */
	BOOL		bIsRealocable;	/* the in-memory can be realloc */
	UINT8		ucByte;			/* the last read bytes */
	UINT8		ucBitRemainder;	/* bits not read in the last byte */
	INT			hFile;			/* file handle */
	STRM_FUN_T  *ptStrmFun;		/* stream method */
}	STREAM_T;


/*-------------------------------------------------------------------------*
 *  MFL ID3 tag information (stored in movie information)                  *
 *-------------------------------------------------------------------------*/
typedef struct id3_ent_t
{
	INT 	nLength;			/* tag length */
	CHAR 	sTag[128];			/* tag content */
	BOOL 	bIsUnicode;			/* 1: is unicode, 0: is ASCII */
}	ID3_ENT_T; 

typedef struct id3_pic_t
{
	BOOL       bHasPic;			/* 1: has picture, 0: has no picture in ID3 tag */
	CHAR       cType;
	ID3_ENT_T  tTitle;			
	INT        nPicOffset;		/* picture offset in file */
	INT        nPicSize;		/* picture size */
	struct id3_pic_t  *ptNextPic;	/* always NULL in current version */
}	ID3_PIC_T;

typedef struct id3_tag_t
{
	ID3_ENT_T  	tTitle;
	ID3_ENT_T  	tArtist;
	ID3_ENT_T  	tAlbum;
	ID3_ENT_T  	tComment;

	CHAR 		szYear[16];
	CHAR 		szTrack[16];
	CHAR 		szGenre[16];
	CHAR    	cVersion;		/* reversion of ID3v2. ID3v2.2.0 = 0x20, ID3v2.3.0 = 0x30  */
								/* reversion of ID3v1. ID3v1.0 = 0x00, ID3v1.1 = 0x10 */
	ID3_PIC_T  	tPicture;
}	ID3_TAG_T;


/*-------------------------------------------------------------------------*
 *  MFL movie information (run-time and preview)                           *
 *-------------------------------------------------------------------------*/
typedef struct mv_info
{
	UINT32		uInputFileSize;		/* the file size of input media file */
	UINT32		uMovieLength;		/* in 1/100 seconds */
	UINT32		uPlayCurTimePos;	/* for playback, the play time position, in 1/100 seconds */
	UINT32		uCreationTime;
	UINT32		uModificationTime;
	
	/* audio */
	AU_CODEC_E	eAuCodecType;
	UINT32		uAudioLength;		/* in 1/100 seconds */
	INT			nAuRecChnNum;		/* 1: record single channel, Otherwise: record left and right channels */
	AU_SRATE_E 	eAuRecSRate;		/* audio record sampling rate */
	UINT32		uAuRecBitRate;		/* audio record bit rate */
	UINT32		uAuRecMediaLen; 	/* Currently recorded audio data length */
	BOOL		bIsVBR;				/* input audio file is VBR or not */
	INT			nAuPlayChnNum;		/* 1:Mono, 2:Stero */
	AU_SRATE_E 	eAuPlaySRate;		/* audio playback sampling rate */
	UINT32		uAuPlayBitRate;		/* audio playback bit rate */
	UINT32		uAuTotalFrames;		/* For playback, it's the total number of audio frames. For recording, it's the currently recorded frame number. */
	UINT32		uAuFramesPlayed;	/* Indicate how many audio frames have been played */
	UINT32		uAuPlayMediaLen;	/* Indicate how many audio data have been played (bytes) */
	UINT32		uAuMP4BuffSizeDB;	/* MP4 audio decode buffer size (recorded in MP4 file) */
	UINT32		uAuMP4AvgBitRate;	/* MP4 audio average bit rate (recorded in MP4 file) */
	UINT32		uAuMP4MaxBitRate;   /* MP4 audio maximum bit rate (recorded in MP4 file) */

	/* video */
	VID_CODEC_E	eVidCodecType;
	UINT32		uVideoLength;		/* in 1/100 seconds */
	UINT32		uVideoFrameRate;	/* only available in MP4/3GP/ASF/AVI files */
	BOOL		bIsShortHeader;		/* TRUE:H.263, FALSE: MPEG4 */
	UINT16		usImageWidth;
	UINT16		usImageHeight;
	UINT32		uVidTotalFrames;	/* For playback, it's the total number of video frames. For recording, it's the currently recorded frame number. */
	UINT32		uVidFramesPlayed;	/* Indicate how many video frames have been played */
	UINT32		uVidPlayMediaLen;	/* Indicate how many video data have been played (bytes) */
	UINT32		uVidRecMediaLen;	/* Currently recorded video data length */
	UINT32		uVidMP4BuffSizeDB;	/* MP4 video decode buffer size (recorded in MP4 file) */
	UINT32		uVidMP4AvgBitRate;	/* MP4 video average bit rate (record/playback) */
	UINT32		uVidMP4MaxBitRate;  /* MP4 video maximum bit rate (recorded in MP4 file) */
	INT			nMPEG4HeaderPos;	/* The file offset of MPEG4 video header in the input MP4 file */
	INT			nMPEG4HeaderLen;	/* The length of MPEG4 video header in the input MP4 file */
	INT			n1stVidFramePos;	/* The file offset of first video frame in the input MP4/3GP file */
	INT			n1stVidFrameLen;	/* The lenght of first video frame in the input MP4/3GP file */

	INT			nMediaClipProgress;	/* The progress of media clipping, 0 ~ 100 */
	INT			nRecDataPerSec;		/* Recorder consumed storage space per second (in bytes) */
	INT			nMP4RecMetaReserved;/* MP4/3GP recorder required reserving meta data storage space (in bytes) */
	
	INT			nMP4RecMetaSize;	/* Only available in 3GP/MP4 record to memory */
	INT         nMP4RecMediaSize;	/* Only available in 3GP/MP4 record to memory */
	
	/* ID3 tag */
	ID3_TAG_T	tID3Tag;
	INT			puVisualData[32];	/* value range 0~31 */
	
	/* MP3 lyric */
	INT			nLyricLenInFile;	/* 0: no lyric, otherwise: the length of MP3 lyric in the MP3 file */
	INT			nLyricOffsetInFile; /* 0: no lyric, otherwise: the file offset of lyric in the Mp3 file */
	UINT32		uLyricCurTimeStamp;	/* in 1/100 seconds, time offset of the current lyric from the start of MP3 */
	CHAR		pcLyricCur[256];	/* on playback MP3, it contained the current lyric */
	UINT32		uLyricNextTimeStamp;/* in 1/100 seconds, time offset of the next lyric from the start of MP3 */
	CHAR		pcLyricNext[256];	/* on playback MP3, it contained the current lyric */

	INT			nReserved1;
	INT			nReserved2;
	INT			nReserved3;
	INT			nReserved4;

}	MV_INFO_T;



/*-------------------------------------------------------------------------*
 *  MFL movie configuration (playback, record, and preview)                *
 *-------------------------------------------------------------------------*/
typedef struct mv_cfg_t
{
	/* Media and stream */
	MEDIA_TYPE_E	eInMediaType;		/* PLAY   - indicae the type of media to be played */
	MEDIA_TYPE_E	eOutMediaType;		/* RECORD - indicate the type of media to generate */

	STRM_TYPE_E 	eInStrmType;		/* PLAY   - indicae the input stream method */
	STRM_TYPE_E 	eOutStrmType;		/* RECORD - indicate the output stream method */
	
	AU_CODEC_E		eAuCodecType;		/* RECORD - indicate the audio encode type */
	VID_CODEC_E		eVidCodecType;		/* RECORD - indicate the video encode type */

	STRM_FUN_T		*ptStrmUserFun;		/* BOTH   - user defined streaming method */

	CHAR			*suInMediaFile;		/* PLAY   - if in stream type is MFL_STREAM_FILE */
	CHAR			*szIMFAscii;		/* PLAY   - if in stream type is MFL_STREAM_FILE */
	CHAR			*suInMetaFile;		/* PLAY   - if in stream type is MFL_STREAM_FILE */
	CHAR			*szITFAscii;		/* PLAY   - if in stream type is MFL_STREAM_FILE */
	CHAR			*suOutMediaFile;	/* RECORD - if out stream type is MFL_STREAM_FILE */
	CHAR			*szOMFAscii;		/* RECORD - if out stream type is MFL_STREAM_FILE */
	CHAR			*suOutMetaFile;		/* RECORD - if out stream type is MFL_STREAM_FILE */
	CHAR			*szOTFAscii;		/* RECORD - if out stream type is MFL_STREAM_FILE */
	
	UINT32			uInMediaMemAddr;	/* PLAY   - if in stream type is MFL_STREAM_MEMORY */
	UINT32			uInMediaMemSize;	/* PLAY   - if in stream type is MFL_STREAM_MEMORY */
	UINT32			uInMetaMemAddr;		/* PLAY   - if in stream type is MFL_STREAM_MEMORY */
	UINT32			uInMetaMemSize;		/* PLAY   - if in stream type is MFL_STREAM_MEMORY */
	UINT32			uOutMediaMemAddr;	/* RECORD - if out stream type is MFL_STREAM_MEMORY */
	UINT32			uOutMediaMemSize;	/* RECORD - if out stream type is MFL_STREAM_MEMORY */
	UINT32			uOutMetaMemAddr;	/* RECORD - if out stream type is MFL_STREAM_MEMORY */
	UINT32			uOutMetaMemSize;	/* RECORD - if out stream type is MFL_STREAM_MEMORY */

	BOOL			bUseTempFile;		/* PLAY   - use temporary file? */
	INT				uStartPlaytimePos;	/* PLAY   - On MP3 playback start, just jump to a 
	                                                specific time offset then start playback. The time position unit is 1/100 seconds. */
	BOOL			bStartAndPause;		/* PLAY   - On MP4/3GP playback, start and pause */
	INT				nClipStartTime;		/* CLIP   - media clipping start time offset (in 1/100 secs) */
	INT				nClipEndTime;		/* CLIP   - media clipping end time offset (in 1/100 secs) */
	BOOL			bDoClipVideo;		/* CLIP   - clip video or not */
	
	/* audio */
	BOOL			bIsRecordAudio;		/* RECORD - 1: recording audio, 0: No */
	INT				nAuABRScanFrameCnt;	/* PLAY   - on playback, ask MFL scan how many leading frames
	                                                to evaluate average bit rate. -1 means scan the whole file */
	INT				nAudioPlayVolume;	/* PLAY   - volume of playback, 0~31, 31 is max. */
	INT				nAudioRecVolume;	/* RECORD - volume of playback, 0~31, 31 is max. */
	UINT8			nAuRecChnNum;		/* RECORD - 1: record single channel, Otherwise: record left and right channels */
	BOOL            bIsSbcMode;			/* PLAY   - work with Bluetooth SBC */
	AU_PLAY_DEV_E	eAudioPlayDevice;	/* PLAY   - specify the audio playback audio device */
	AU_REC_DEV_E	eAudioRecDevice;	/* RECORD - specify the audio record device */
	AU_SRATE_E		eAuRecSRate;		/* RECORD - audio record sampling rate */
	UINT32			uAuRecBitRate;		/* RECORD - audio record initial bit rate */
	UINT32			uAuBuffSizeDB;		/* RECORD - audio decoder required buffer size */
	UINT32			uAuRecAvgBitRate;	/* RECORD - audio record average bit rate */
	UINT32			uAuRecMaxBitRate;	/* RECORD - audio record maxmum bit rate */
	
	/* video */
	CHAR			bIsRecordVideo;		/* RECORD - 1: recording video, 0: No */
	INT				nMP4VidMaxSSize;	/* PLAY   - direct MFL should allocate how many memory for MP4/3GP video sample buffer */
	INT				nVidPlayFrate;		/* PLAY   - video playback frame rate, only used in M4V file */
	INT				nVidRecFrate;		/* RECORD - video record frame rate */
	INT				nVidRecIntraIntval;	/* RECORD - video record intra frame interval, -1: one first intra, 0: all intra */
	UINT16			sVidRecWidth;		/* RECORD - width of record image */
	UINT16			sVidRecHeight;		/* RECORD - height of record image */
	UINT32			uVidBuffSizeDB;		/* RECORD - video decoder required buffer size */
	UINT32			uVidRecAvgBitRate;	/* RECORD - video record average bit rate */
	UINT32			uVidRecMaxBitRate;	/* RECORD - video record maxmum bit rate */
	
	/* callback	 */
	VOID (*ap_time)(struct mv_cfg_t *ptMvCfg);
	INT  (*au_sbc_init)(struct mv_cfg_t *ptMvCfg);
	VOID (*au_sbc_reset_buff)(VOID);
	INT  (*au_sbc_encode)(struct mv_cfg_t *ptMvCfg, UINT8 *pucPcmBuff, INT nPcmDataLen);
	BOOL (*au_is_sbc_ready)(VOID);
	INT  (*vid_init_decode)(VID_CODEC_E eDecoder, UINT8 *pucBitStrmBuff, UINT32 uBitStrmSize, BOOL *bIsShortHeader, 
							UINT16 *usImageWidth, UINT16 *usImageHeight);
	INT  (*vid_init_encode)(UINT8 *pucM4VHeader, UINT32 *puHeaderSize,
							BOOL bIsH263, UINT16 usImageWidth, UINT16 usImageHeight);
	INT  (*vid_enc_frame)(PUINT8 *pucFrameBuff, UINT32 *puFrameSize);
	VOID (*vid_rec_frame_done)(UINT8 *pucFrameBuff);
	INT  (*vid_dec_frame)(VID_CODEC_E eDecoder, UINT8 *pucFrameBuff, UINT32 *puFrameSize);
	INT  (*vid_dec_state)(VOID);
	VOID (*au_on_start)(struct mv_cfg_t *ptMvCfg);
	VOID (*au_on_stop)(struct mv_cfg_t *ptMvCfg);
	VOID (*the_end)(struct mv_cfg_t *ptMvCfg);
	
	/*
	 * others - for MFL internal used
	 */
	VOID			*data_mv;			
	VOID			*data_info;			
	INT				data_play_action;
	INT				data_play_param;
	INT				data_rec_action;
	INT				data_rec_param;
}  MV_CFG_T;



/*-------------------------------------------------------------------------*
 *  MFL KTV maker settings                                                 *
 *-------------------------------------------------------------------------*/
typedef struct ktv_cfg_t
{
	AU_CODEC_E		eAuCodecType;		/* MP3 or AAC */

	CHAR			*suInMP4File;		/* Full path name of the input MP4/3GP file */
	CHAR			*suInAudioFile;		/* Full path name of the input MP3/AAC file */
	CHAR			*suInLyricFile;		/* Full path name of the input lyric file */
	CHAR			*suOutMediaFile;	/* Output media file */
	CHAR			*szOMFAscii;		
	CHAR			*suOutMetaFile;		/* Output meta file */
	CHAR			*szOTFAscii;		

	BOOL			bStopIfVideoEnd;	/* If there's no more video frames, stop KTV maker. */
	BOOL			bStopIfAudioEnd;	/* If there's no more audio frames, stop KTV maker. */
	INT				nVideoClipStart;	/* The video clip start	time position in 1/100 seconds */
	INT				nVideoClipEnd;		/* The video clip end time position in 1/100 seconds */
	INT				nAudioClipStart;	/* The video clip start	time position in 1/100 seconds */
	INT				nAudioClipEnd;		/* The video clip end time position in 1/100 seconds */
	
	BOOL			bUseTempFile;
	INT				nMP4VidMaxSSize;
	
	/* callback	 */
	VOID (*ap_time)(struct mv_cfg_t *ptMvCfgKtv);
	VOID (*the_end)(struct mv_cfg_t *ptMvCfgKtv);
	
	/*
	 * others - for MFL internal used
	 */
	MV_CFG_T		tMvCfgMp4;
	MV_CFG_T		tMvCfgAu;
	MV_CFG_T		tMvCfgKtv;
}  KTV_CFG_T;


/*-------------------------------------------------------------------------*
 *  MFL API List                                                           *
 *-------------------------------------------------------------------------*/
extern INT  mflMediaPlayer(MV_CFG_T *ptMvCfg);
extern INT  mflMovieMaker(MV_CFG_T *ptMvCfg);
extern INT  mflMediaClipper(MV_CFG_T *ptMvCfg);
extern INT  mflKtvMaker(KTV_CFG_T *ptKtvMk);
extern INT  mflGetMovieInfo(MV_CFG_T *ptMvCfg, MV_INFO_T **ptMvInfo);
extern INT  mflPlayControl(MV_CFG_T *ptMvCfg, PLAY_CTRL_E ePlayCtrl, INT nParam);
extern INT  mflRecControl(MV_CFG_T *ptMvCfg, REC_CTRL_E eRecCtrl, INT nParam);
extern INT  mflSetEqualizer(EQ_EFT_E eEqEft, INT nPreAmp, INT *pnBands);
extern INT  mflGetEqualizer(MV_CFG_T *ptMvCfg, INT *pnBands);
extern VOID mflEnableVisualizer(INT nFrameInterval);
extern VOID mflDisableVisualizer(VOID);
extern VOID mflDisableEqualizer(VOID);
extern VOID mflSetAudioPlayVolume(MV_CFG_T *ptMvCfg, INT volume);
extern VOID mflSetAudioRecVolume(MV_CFG_T *ptMvCfg, INT volume);
extern INT	mflPreviewMediaInfo(MV_CFG_T *ptMvCfg, MV_INFO_T *ptMinfo);
extern INT  mflEnable3DSurround(VOID *pConfig); 
extern INT  mflDisable3DSurround(VOID);
extern INT  mflTuneQ3DSurround(INT nSpread, INT nDelay);
extern VOID  mflAuGetAudioRecData(UINT32 *uStartAddress, UINT32 *uLength);

#endif	/* _MEDIA_FILE_LIB_H_ */
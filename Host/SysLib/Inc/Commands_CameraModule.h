#ifndef __COMMANDS_CAMMOD_H__
#define __COMMANDS_CAMMOD_H__




/* Constant in the command set. */
typedef enum
{
	WB_ENABLE					= 1,
	WB_DISABLE					= 0
} WB_SWITCH_E;

typedef enum
{
	WB_OK						= 0,
	WB_ERROR					= 1
} WB_RETURN_E;

/* wbcSetAECMode */
typedef enum
{
	WB_AEC_NORMAL_LIGHT			= 0,
	WB_AEC_NORMAL_LIGHT_60HZ	= 1,
	WB_AEC_NORMAL_LIGHT_50HZ	= 2,
	WB_AEC_LOW_LIGHT_60HZ		= 3,
	WB_AEC_LOW_LIGHT_50HZ		= 4,
	WB_AEC_LOW_LIGHT			= 5,
	WB_AEC_DAY_LIGHT			= 6,
	WB_AEC_CLOUDY				= 7,
	WB_AEC_FLUORESCENT			= 8,
	WB_AEC_FLUORESCENT_60HZ		= 9,
	WB_AEC_FLUORESCENT_50HZ		= 10,
	WB_AEC_INCANDESCENT			= 11,
	WB_AEC_INCANDESCENT_60HZ	= 12,
	WB_AEC_INCANDESCENT_50HZ	= 13
} WB_AEC_MODE_E;


/* wbcGetJPEGFormat */
/* wbcSetJPEGFormat */
/* wbcDecodeJPEGBitstream */
typedef enum
{
	WB_YUV420					= 0,
	WB_YUV422					= 1,
	WB_YUV444					= 2,
	WB_GRAY_LEVEL				= 3
} WB_IMAGE_FORMAT_E;

typedef enum
{
	WB_DECODE_TO_BUFFER			= 0,
	WB_DECODE_TO_BUFFER_AND_LCD	= 1,
	WB_DECODE_ONLY				= 2
} WB_DECODE_TO_E;

/* wbcGetDecodedJPEGInformation */
typedef enum
{
	WB_DECODE_GET_ORIGINAL		= 0,
	WB_DECODE_GET_SCALED		= 1,
	WB_DECODE_GET_CLIP_SIZE		= 2
} WB_DECODE_OPTION_E;

typedef enum
{
	WB_IMG_128X96				= 0x01,
	WB_IMG_160X120				= 0x02,
	WB_IMG_176X144				= 0x04,
	WB_IMG_320X240				= 0x08,
	WB_IMG_352X288				= 0x10,
	WB_IMG_640X480				= 0x20,
	WB_IMG_128X160				= 0x40
} WB_IMAGE_SIZE_E;


/* wbcSetOperationMode */
/* Operation mode. */
typedef enum
{
	WB_OP_MODE_SINGLE_CAPTURE	= 0,
	WB_OP_MODE_BURST_CAPTURE	= 1,
	WB_OP_MODE_VIDEO_CAPTURE	= 2,
	WB_OP_MODE_SINGLE_PLAYBACK	= 3,
	WB_OP_MODE_VIDEO_PLAYBACK	= 4,
	WB_OP_MODE_COMIC_CAPTURE	= 5,
	WB_OP_MODE_MOVIE_RECORD		= 8,
	WB_OP_MODE_MOVIE_PLAYBACK	= 9,
	WB_OP_MODE_AUDIO_RECORD		= 10,
	WB_OP_MODE_AUDIO_PLAYBACK	= 11
} WB_OP_MODE_E;

typedef enum
{
/* Sub mode for "WB_OP_MODE_MOVIE_RECORD". */
	WB_OP_SUBMODE_MPEG4			= 0,
	WB_OP_SUBMODE_MPEG4_AAC		= 1,
	WB_OP_SUBMODE_3GP			= 2,
/* Sub mode for "WB_OP_MODE_AUDIO_RECORD" and "WB_OP_MODE_AUDIO_PLAYBACK". */
	WB_OP_SUBMODE_AAC			= 1,
	WB_OP_SUBMODE_AMR			= 2,
	WB_OP_SUBMODE_ADPCM			= 3,
/* Sub mode for "WB_OP_MODE_AUDIO_PLAYBACK". */
	WB_OP_SUBMODE_MP3			= 0,
	WB_OP_SUBMODE_WMA			= 4
} WB_OP_SUBMODE_E;

/* Format for decoded image. */
typedef enum
{
	WB_OP_FORMAT_YUV422			= 0x01,
	WB_OP_FORMAT_RGB565			= 0x02,
	WB_OP_FORMAT_RGB555			= 0x03,
	WB_OP_FORMAT_RGB444			= 0x04,
	WB_OP_FORMAT_RGB332			= 0x05,
	WB_OP_FORMAT_RGB888			= 0x06,
	WB_OP_FORMAT_OSD_YUV422		= 0x10,
	WB_OP_FORMAT_OSD_RGB565		= 0x20,
	WB_OP_FORMAT_OSD_RGB555		= 0x30,
	WB_OP_FORMAT_OSD_RGB444		= 0x40,
	WB_OP_FORMAT_OSD_RGB332		= 0x50,
	WB_OP_FORMAT_OSD_RGB888		= 0x60
} WB_OP_FORMAT_E;

/* Sensor powser. */
typedef enum
{
	WB_OP_SENSOR_OFF			= 0,
	WB_OP_SENSOR_ON				= 1
} WB_OP_SENSOR_POWER_E;

/* wbcSnapshot */
typedef enum
{
	WB_SNAP_COMIC_CONTINUSOUS	= 0,
	WB_SNAP_COMIC_ONE_BY_ONE	= 1
} WB_SNAP_COMICE_E;

typedef enum
{
	WB_SNAP_VIDEO_START			= 0,
	WB_SNAP_VIDEO_STOP			= 0xFFFFU
} WB_SNAP_VIDEO_E;

typedef enum
{
	WB_SNAP_MOVIE_START			= 0,
	WB_SNAP_MOVIE_PAUSE			= 0xFFFDU,
	WB_SNAP_MOVIE_RESUME		= 0xFFFEU,
	WB_SNAP_MOVIE_STOP			= 0xFFFFU
} WB_SNAP_MOVIE_E;

/* wbcSetRotation */
typedef enum
{
	WB_ROT_NORMAL				= 0,
	WB_ROT_LEFT_90				= 1,
	WB_ROT_RIGHT_90				= 2,
	WB_MIRROR					= 3,
	WB_FLIP						= 4,
	WB_ROT_180					= 5
} WB_ROT_E;

/* wbcConfigureLCM */
typedef enum
{
	WB_LCM_NO					= 0x00,
	WB_LCM_ONE					= 0x01,
	WB_LCM_TWO					= 0x02,
	WB_LCM_FIRST				= 0x11,
	WB_LCM_SECOND				= 0x12
} WB_LCM_NUMBER_E;

typedef enum
{
	WB_LCM_BUS_WIDTH_16_18		= 0,
	WB_LCM_BUS_WIDTH_8_9		= 1
} WB_LCM_BUS_WIDTH_E;

typedef enum
{
	WB_LCM_COLOR_WIDTH_16		= 0,
	WB_LCM_COLOR_WIDTH_18		= 1,
	WB_LCM_COLOR_WIDTH_8		= 2,
	WB_LCM_COLOR_WIDTH_12		= 3,
	WB_LCM_COLOR_WIDTH_24		= 4
} WB_LCM_COLOR_WIDTH_E;


/* wbcSuspendModule */
typedef enum
{
	WB_SUSPEND_KEEP_SDRAM		= 0,
	WB_SUSPEND_DROP_SDRAM		= 1
} WB_SUSPEND_TYPE_E;

/* wbcSetLCMControlMode */
typedef enum
{
	WB_LCM_BYPASS				= 0,
	WB_LCM_BRIDGE				= 1
} WB_LCM_CONTROL_MODE_E;

/* wbcGetCaptureOrPlaybackStatus */
typedef enum
{
	WB_RUNNING					= 0,
	WB_STOPPED					= 1
} WB_PROCESS_E;

/* wbcSetI2CMode */
typedef enum
{
	WB_I2C_16BIT				= 0,
	WB_I2C_8BIT					= 1
} WB_I2C_ACCESS_MODE_E;

/* wbcPlaybackVideoClip */
typedef enum
{
	WB_VIDEO_PLAYBACK_PAUSE		= 0,
	WB_VIDEO_PLAYBACK_FORWARD	= 1,
	WB_VIDEO_PLAYBACK_BACKWARD	= 2,
	WB_VIDEO_PLAYBACK_STOP		= 3
} WB_VIDEO_PLAYBACK_OPERATION_E;

typedef enum
{
	WB_VIDEO_PLAYBACK_NORMAL	= 0,
	WB_VIDOE_PLAYBACK_REPEAT	= 1
} WB_VIDEO_PLAYBACK_FLAG_E;

/* wbcPlaybackMovie */
typedef enum
{
	WB_MOVIE_PLAYBACK_PAUSE		= 0,
	WB_MOVIE_PLAYBACK_START		= 1,
	WB_MOVIE_PLAYBACK_STARTPAUSE= 2,
	WB_MOVIE_PLAYBACK_STOP		= 3,
	WB_MOVIE_PLAYBACK_FORWARD	= 4,
	WB_MOVIE_PLAYBACK_BACKWARD	= 5,
	WB_MOVIE_PLAYBACK_JUMP		= 6
} WB_MOVIE_PLAYBACK_OPERATION_E;

/* wbcSetImageEffect */
typedef enum
{
	WB_IMAGE_NORMAL				= 0,
	WB_IMAGE_GRAY				= 1,
	WB_IMAGE_SEPIA				= 2,
	WB_IMAGE_NEGATIVE			= 3,
	WB_IMAGE_SOLARIZE			= 4,
	WB_IMAGE_THRESHOLD_Y		= 5,
	WB_IMAGE_THRESHOLD_UV		= 6
} WB_IMAGE_EFFECT_E;

typedef enum
{
	WB_BUFFER_INFO_OSD_ADDRESS	= 1,
	WB_BUFFER_INFO_OSD_SIZE		= 2
} WB_BUFFER_INFO_E;

/* wbcSetAudioChannels */
typedef enum
{
	WB_AUDIO_STEREO				= 0,
	WB_AUDIO_JOINT_STEREO		= 1,
	WB_AUDIO_DUAL_CHANNEL		= 2
} WB_AUDIO_STEREO_TYPE_E;

typedef enum
{
	WB_AUDIO_STEREO_NORMAL		= 0,
	WB_AUDIO_STEREO_INTENSITY	= 1,
	WB_AUDIO_STEREO_MID_SIDE	= 2,
	WB_AUDIO_STEREO_BOTH		= 3
} WB_AUDIO_STEREO_EXT_TYPE_E;

/* wbcSetAudioOperation */
typedef enum
{
	WB_AUDIO_STOP				= 0,
	WB_AUDIO_START				= 1,
	WB_AUDIO_PAUSE				= 2,
	WB_AUDIO_CONTINUE			= 3,
	WB_AUDIO_FORWARD			= 4,
	WB_AUDIO_BACKWARD			= 5,
	WB_AUDIO_JUMP				= 6,
	WB_AUDIO_DISCARD_DATA		= 8,
	WB_AUDIO_GET_STATUS			= 9,
	WB_AUDIO_CLEAR_ERROR		= 10
} WB_AUDIO_OPERATION_E;


/* For "wbcSetPlaybackAudioVolume.... */
typedef enum
{
	WB_AUDIO_SET_PLAYBACK_VOLUME		= 0,
	WB_AUDIO_GET_PLAYBACK_VOLUME		= 1,
	WB_AUDIO_GET_PLAYBACK_VOLUME_RANGE	= 2,
	WB_AUDIO_PLAYBACK_MUTE_ENABLE		= 3,
	WB_AUDIO_PLAYBACK_MUTE_DISABLE		= 4,
	WB_AUDIO_GET_PLAYBACK_MUTE_STATUS	= 5,
	WB_AUDIO_SET_RECORD_VOLUME			= 6,
	WB_AUDIO_GET_RECORD_VOLUME			= 7,
	WB_AUDIO_GET_RECORD_VOLUME_RNAGE	= 8
} WB_AUDIO_VOLUME_E;


/* wbcSetAudioFormat */
typedef enum
{
	WB_AUDIO_MP3				= 0,
	WB_AUDIO_AAC				= 1,
	WB_AUDIO_AMR				= 2,
	WB_AUDIO_ADPCM				= 3,
	WB_AUDIO_WMA				= 4,
	WB_AUDIO_M4A				= 5,
	WB_AUDIO_MIDI				= 6,
	WB_AUDIO_G723_1				= 7
} WB_AUDIO_FORMAT_E;


/* wbcGetMediaTicks */
typedef enum
{
	WB_MEDIA_TICKS_TOTAL		= 0,
	WB_MEDIA_TICKS_PLAYED		= 1,
	WB_MEDIA_TICKS_PER_SECOND	= 2
} WB_MEDIA_TICKS_E;

/* wbcSetEqualizer */
/* wbcGetEqualizer */
typedef enum
{
	WB_AUDIO_EQ_DISABLE			= 0,
	WB_AUDIO_EQ_USER			= 1,
	WB_AUDIO_EQ_CLASSICAL		= 2,
	WB_AUDIO_EQ_CLUB			= 3,
	WB_AUDIO_EQ_DANCE			= 4,
	WB_AUDIO_EQ_FULLBASS		= 5,
	WB_AUDIO_EQ_LIVE			= 6,
	WB_AUDIO_EQ_POP				= 7,
	WB_AUDIO_EQ_ROCK			= 8,
	WB_AUDIO_EQ_SOFT			= 9
} WB_AUDIO_EQ_E;

/* wbcGetID3Tags */
typedef enum
{
	WB_AUDIO_ID3_TITLE			= 1,
	WB_AUDIO_ID3_ARTIST			= 2,
	WB_AUDIO_ID3_ALBUM			= 3,
	WB_AUDIO_ID3_YEAR			= 4,
	WB_AUDIO_ID3_COMMENT		= 5,
	WB_AUDIO_ID3_TRACK			= 6,
	WB_AUDIO_ID3_GENRE			= 7
} WB_AUDIO_ID3_TAGS_E;


/* wbcSetSlideShowTransitionEffect */
typedef enum
{
	WB_SLIDE_NO_TRANSITION				= 0,
	WB_SLIDE_COVER_DOWN					= 1,
	WB_SLIDE_COVER_LEFT					= 2,
	WB_SLIDE_COVER_RIGHT				= 3,
	WB_SLIDE_COVER_UP					= 4,
	WB_SLIDE_COVER_LEFT_DOWN			= 5,
	WB_SLIDE_COVER_LEFT_UP				= 6,
	WB_SLIDE_COVER_RIGHT_DOWN			= 7,
	WB_SLIDE_COVER_RIGHT_UP				= 8,
	WB_SLIDE_UNCOVER_DOWN				= 9,
	WB_SLIDE_UNCOVER_LEFT				= 10,
	WB_SLIDE_UNCOVER_RIGHT				= 11,
	WB_SLIDE_UNCOVER_UP					= 12,
	WB_SLIDE_UNCOVER_LEFT_DOWN			= 13,
	WB_SLIDE_UNCOVER_LEFT_UP			= 14,
	WB_SLIDE_UNCOVER_RIGHT_DOWN			= 15,
	WB_SLIDE_UNCOVER_RIGHT_UP			= 16,
	WB_SLIDE_TEAR_DOWN					= 17,
	WB_SLIDE_TEAR_LEFT					= 18,
	WB_SLIDE_TEAR_RIGHT					= 19,
	WB_SLIDE_TEAR_UP					= 20,
	WB_SLIDE_BLINDS_VERTICAL			= 21,
	WB_SLIDE_BLINDS_HORIZONTAL			= 22,
	WB_SLIDE_SPLIT_HORIZONTAL_IN		= 23,
	WB_SLIDE_SPLIT_HORIZONTAL_OUT		= 24,
	WB_SLIDE_SPLIT_VERTICAL_IN			= 25,
	WB_SLIDE_SPLIT_VERTICAL_OUT			= 26,
	WB_CHECKERBOARD_ACROSS				= 27,
	WB_CHECKERBOARD_DOWN				= 28,
	WB_BOX_IN							= 29,
	WB_BOX_OUT							= 30,
	WB_FADE								= 31,
	WB_FADE_FROM_BLACK					= 32
} WB_SLIDE_SHOW_TRANSITION_EFFECT_E;


typedef enum
{
	WB_GPIO								= 0x00,
	WB_GPIO_A							= 0x04,
	WB_GPIO_B							= 0x08,
	WB_GPIO_S							= 0x0C
} WB_GPIO_E;


/* wbcGetFlashMemoryCardStatus */
/* wbcFormatFlashMemoryCard */
typedef enum
{
	WB_CARD_OK					= 0,
	WB_CARD_NONE				= 1,
	WB_CARD_NOT_INDENTIFIED		= 2,
	WB_CARD_WRITE_PROTECTED		= 3,
	WB_CARD_INITIALIZING		= 4
} WB_CARD_STATUS_E;

typedef enum
{
	WB_DIRECTORY				= 1,
	WB_FILE						= 0
} WB_FS_TYPE_E;


/* wbcGetFileAttribute */
/* wbcSetFileAttribute */
/* wbcFindFirstFile */
typedef enum
{
	WB_FS_ATTR_READONLY			= 0x01,
	WB_FS_ATTR_HIDDEN			= 0x02,
	WB_FS_ATTR_SYSTEM			= 0x04,
	WB_FS_ATTR_VOLUME_LABLE		= 0x08,
	/* LABLE, spelling mistake, do not remove it for compatibility. */
	WB_FS_ATTR_VOLUME_LABEL		= 0x08, 
	WB_FS_ATTR_DIRECTORY		= 0x10,
	WB_FS_ATTR_ARCHIVE			= 0x20
} WB_FS_ATTR_E;

/* wbcOpenFile */
typedef enum
{
	WB_O_RDONLY			= 0x0001,      /* Open for read only*/
	WB_O_WRONLY			= 0x0002,      /* Open for write only*/
	WB_O_RDWR			= 0x0003,      /* Read/write access allowed.*/
	WB_O_APPEND			= 0x0004,      /* Seek to eof on each write*/
	WB_O_CREATE			= 0x0008,      /* Create the file if it does not exist.*/
	WB_O_TRUNC			= 0x0010,      /* Truncate the file if it already exists*/
	WB_O_EXCL			= 0x0020,      /* Fail if creating and already exists */
	WB_O_DIR			= 0x0080       /* Open a directory file */
} WB_O_FLAGS_E;

/* wbcSeekFile */
typedef enum
{
	WB_SEEK_BEGIN		= 0,
	WB_SEEK_CURRENT		= 1,
	WB_SEEK_END			= 2
} WB_SEEK_POSITION_E;


/* wbcSetCaptureZoom */
typedef enum
{
	WB_CAPZOOM_OP_DISABLE		= 0,
	WB_CAPZOOM_OP_ZOOM_IN		= 1,
	WB_CAPZOOM_OP_ZOOM_OUT		= 2,
	WB_CAPZOOM_OP_STOP			= 3,
	WB_CAPZOOM_OP_GET_STATUS	= 4,
	WB_CAPZOOM_OP_SET_TO_LINEAR	= 5,
	WB_CAPZOOM_OP_SET_TO_STEP	= 6,
	WB_CAPZOOM_OP_SET_FACTOR	= 7,
	WB_CAPZOOM_OP_GET_MAX_RATIO	= 8
} WB_CAPZOOM_OPERATION_E;


typedef enum
{
	WB_CAPZOOM_STATUS_STOP		= 0,
	WB_CAPZOOM_STATUS_MOVINT	= 1
} WB_CAPZOOM_STATUS_E;


typedef enum
{
	WB_CAPZOOM_LINEAR			= 5,
	WB_CAPZOOM_STEP				= 6
} WB_CAPZOOM_MODE_E;


/* wbcSetPlaybackZoom */
enum
{
	__WB_PLAYZOOM_ENABLE		= 0,
	__WB_PLAYZOOM_PAN_TILE		= 1
};

enum
{
	__WB_PLAYZOOM_SELECT_PAN	= 0,
	__WB_PLAYZOOM_SELECT_TILE	= 1
};

enum
{
	__WB_PLAYZOOM_LEFT_UP		= 0,
	__WB_PLAYZOOM_RIGHT_DOWN	= 1
};

typedef enum
{
	WB_PLAYZOOM_ENABLE			= (__WB_PLAYZOOM_ENABLE << 24),
	WB_PLAYZOOM_PAN_LEFT		= (__WB_PLAYZOOM_PAN_TILE << 24)
								| (__WB_PLAYZOOM_SELECT_PAN << 16)
								| (__WB_PLAYZOOM_LEFT_UP << 8),
	WB_PLAYZOOM_PAN_RIGHT		= (__WB_PLAYZOOM_PAN_TILE << 24)
								| (__WB_PLAYZOOM_SELECT_PAN << 16)
								| (__WB_PLAYZOOM_RIGHT_DOWN << 8),
	WB_PLAYZOOM_TILE_UP			= (__WB_PLAYZOOM_PAN_TILE << 24)
								| (__WB_PLAYZOOM_SELECT_TILE << 16)
								| (__WB_PLAYZOOM_LEFT_UP << 8),
	WB_PLAYZOOM_TILE_DOWN		= (__WB_PLAYZOOM_PAN_TILE << 24)
								| (__WB_PLAYZOOM_SELECT_TILE << 16)
								| (__WB_PLAYZOOM_RIGHT_DOWN << 8)
} WB_PLAYZOOM_E;



/* wbcSetInputSource */
typedef enum
{
	WB_INPUT_SRC_SENSOR			= 0,
	WB_INPUT_SRC_TV_TUNER		= 1
} WB_INPUT_SRC_E;

typedef enum
{
	WB_INPUT_TV_NTSC			= 0,
	WB_INPUT_TV_PAL				= 1
} WB_INPUT_TV_E;


/* wbcGetParsedInformation */
typedef enum
{
	WB_PARSE_MEDIA_TITLE				= 0x01,
	WB_PARSE_MEDIA_ARTIST				= 0x02,
	WB_PARSE_MEDIA_ALBUM				= 0x03,
	WB_PARSE_MEDIA_YEAR					= 0x04,
	WB_PARSE_MEDIA_COMMENT				= 0x05,
	WB_PARSE_MEDIA_TRACK_NUMBER			= 0x06,
	WB_PARSE_MEDIA_GENRE				= 0x07,
	WB_PARSE_MEDIA_AUDIO_TYPE			= 0x10,
	WB_PARSE_MEDIA_AUDIO_SAMPLING_RATE	= 0x11,
	WB_PARSE_MEDIA_IS_VBR				= 0x12,
	WB_PARSE_MEDIA_AUDIO_CHANNELS		= 0x13,
	WB_PARSE_MEDIA_AUDIO_BITRATE		= 0x14,
	WB_PARSE_MDEIA_AUDIO_LENGTH			= 0x15,
	WB_PARSE_MEDIA_VIDEO_TYPE			= 0x20,
	WB_PARSE_MEDIA_FRAME_RATE			= 0x21,
	WB_PARSE_MEDIA_IMAGE_WIDTH			= 0x22,
	WB_PARSE_MEDIA_IMAGE_HEIGHT			= 0x23,
	WB_PARSE_MEDIA_VIDEO_BITRATE		= 0x24,
	WB_PARSE_MEDIA_VIDEO_LENGTH			= 0x25,
	WB_PARSE_MEDIA_SHORT_HEADER			= 0x26,
	WB_PARSE_MEDIA_TICKS_10MSEC_UNIT	= 0x30
} WB_PARSE_MEDIA_INFO_E;


/* wbcSet3DSurround */
typedef enum
{
	WB_3D_ENABLE_DISABLE				= 0x00,
	WB_3D_GET_STATUS					= 0x01,
	WB_3D_SET_OUTPUT_TYPE				= 0x02,
	WB_3D_GET_OUTPUT_TYPE				= 0x03,
	WB_3D_SET_GEOMETRY_TYPE				= 0x04,
	WB_3D_GET_GEOMETRY_TYPE				= 0x05,
	WB_3D_SET_REVERB_TYPE				= 0x06,
	WB_3D_GET_REVERB_TYPE				= 0x07
} WB_3D_OPERATION_E;

typedef enum
{
	WB_3D_OUTPUT_SPEAKER				= 0x00,
	WB_3D_OUTPUT_HEADPHONE				= 0x01
} WB_3D_OUTPUT_TYPE_E;

typedef enum
{
	WB_3D_GEOMETRY_DESKTOP				= 0x01,
	WB_3D_GEOMETRY_FRONT				= 0x02,
	WB_3D_GEOMETRY_SIDE					= 0x03
} WB_3D_GEOMETRY_TYPE_E;


typedef struct
{
	BOOL	bFirmwareRunning;
	
	WB_OP_MODE_E			wbcSetOperationMode_eMode;
	WB_OP_FORMAT_E			wbcSetOperationMode_eFormat;
	WB_OP_SUBMODE_E			wbcSetOperationMode_eSubMode;
	WB_OP_SENSOR_POWER_E	wbcSetOperationMode_eSensorPower;

	UCHAR	wbcSelectLCM_ucNumber;
	USHORT	wbcSetLCMResolution_usWidth1;
	USHORT	wbcSetLCMResolution_usHeight1;
	USHORT	wbcSetLCMResolution_usWidth2;
	USHORT	wbcSetLCMResolution_usHeight2;
	
	USHORT	wbcSetExtentOffset_usWidth1;
	USHORT	wbcSetExtentOffset_usHeight1;
	USHORT	wbcSetExtentOffset_usWidth2;
	USHORT	wbcSetExtentOffset_usHeight2;

	USHORT wbcSnapshot_usBurstCount;
	
	WB_SWITCH_E wbcEnablePreview_eSwitch;
} WB_COMMAND_HANDLE_T;
extern WB_COMMAND_HANDLE_T g_wbcHandle;

#define WSTR_CHAR char

BOOL wbcGetFirmwareVersion (UCHAR * pucMajorVer, UCHAR * pucMinorVer);
BOOL wbcGetBitstream_NoData (USHORT usIndex, UINT32 * puLength);
BOOL wbcGetBitstream_WithData (USHORT usIndex, UINT32 * puLength);
BOOL wbcSetAECMode (WB_AEC_MODE_E eMode);
BOOL wbcGetJPEGFormat (WB_IMAGE_FORMAT_E * peFormat);
BOOL wbcSetJPEGFormat (WB_IMAGE_FORMAT_E eFormat);
BOOL wbcSetJPEGRestartInterval (USHORT usValue);
BOOL wbcSetImageSize (WB_IMAGE_SIZE_E eSize);
BOOL wbcGetImageSize (WB_IMAGE_SIZE_E * peSize);
BOOL wbcGetJPEGRestartInterval (USHORT * pusValue);
BOOL wbcGetQuantizationIndex (UCHAR * pucIndex);
BOOL wbcI2CWrite8 (UCHAR ucAddress, UCHAR ucIndex, UCHAR ucValue);
BOOL wbcI2CWrite16 (UCHAR ucAddress, UCHAR ucIndex, USHORT usValue);
BOOL wbcI2CRead8 (UCHAR ucAddress,
                  UCHAR ucIndex,
                  UCHAR * pucAddress,
                  UCHAR * pucIndex, UCHAR * pucValue);
BOOL wbcI2CRead16 (UCHAR ucAddress,
                   UCHAR ucIndex,
                   UCHAR * pucAddress,
                   UCHAR * pucIndex,
                   USHORT * pusValue);
BOOL wbcSetQuantizationIndex (UCHAR ucIndex);
BOOL wbcGetQuantizationRange (UCHAR * pucIndex);
BOOL wbcSetOperationMode (WB_OP_MODE_E eMode,
                          WB_OP_FORMAT_E eFormat,
                          WB_OP_SUBMODE_E eSubMode,
                          WB_OP_SENSOR_POWER_E eSensorPower);
BOOL wbcSetSystemDate (USHORT usYear, UCHAR ucMonth, UCHAR ucDay);
BOOL wbcSetSystemTime (UCHAR ucHour, UCHAR ucMinute, UCHAR ucSecond);
BOOL wbcSnapshot (USHORT usCount);
BOOL wbcSnapshot_Single (VOID);
BOOL wbcSnapshot_Burst (USHORT usCount);
BOOL wbcSnapshot_Comic (WB_SNAP_COMICE_E eAction);
BOOL wbcSnapshot_Video (WB_SNAP_VIDEO_E eAction);
BOOL wbcSnapshot_Movie (WB_SNAP_MOVIE_E eAction);
BOOL wbcSendBitstream_WithData (CONST UCHAR * cpucBuffer,
                                UINT32 uBufferLength);
BOOL wbcSendBitstream_End (VOID);
BOOL wbcDecodeJPEGBitstream (USHORT usIndex,
                             WB_DECODE_TO_E eOutput,
                             WB_IMAGE_FORMAT_E * peFormat,
                             WB_RETURN_E * peStatus);
BOOL wbcGetDecodedJPEGInformation (WB_DECODE_OPTION_E eOption,
                                   USHORT * pusWidth,
                                   USHORT * pusHeight);
BOOL wbcGetDecodedJPEGData (UINT32 * puLength);
BOOL wbcSetRotation (BOOL bFlip, BOOL bMirror, WB_ROT_E eRotate);
BOOL wbcResizeJPEG (UCHAR ucScaleN, UCHAR ucScaleM);
BOOL wbcSetDigitalZoom (USHORT usWidth, USHORT usHeight);
BOOL wbcGPIOControl_Write (USHORT usGPIO);
BOOL wbcGPIOControl_Read (USHORT * pusGPIO);
BOOL wbcGPIOControl_SetDirection (USHORT usGPIO);
BOOL wbcGPIOControl_GetDirection (USHORT * pusGPIO);
BOOL wbcGPIOControl_MaskWrite (UCHAR ucGroup, UCHAR ucMask,
                               UCHAR ucData);
BOOL wbcGPIOControl_MaskRead (UCHAR ucGroup,
                              UCHAR ucMask, UCHAR * pucData);
BOOL wbcGPIOControl_MaskSetDirection (UCHAR ucGroup, UCHAR ucMask,
                                      UCHAR ucData);
BOOL wbcChangeFirmware (VOID);
BOOL wbcConfigureLCM (WB_LCM_NUMBER_E eNumber, WB_LCM_BUS_WIDTH_E eBus, WB_LCM_COLOR_WIDTH_E eColor);
BOOL wbcSelectLCM (UCHAR ucNumber);
BOOL wbcSetLCMResolution (USHORT usWidth, USHORT usHeight);
BOOL wbcSetVideoImageWindowPosition (USHORT usX, USHORT usY);
BOOL wbcSetVideoImageWindowSize (USHORT usWidth, USHORT usHeight);
BOOL wbcWriteLCMBuffer_Method1 (CONST UCHAR * cpucBuffer,
                                UINT32 uBufferLength, UINT32 uAddress);
BOOL wbcWriteLCMBuffer_Method2 (CONST UCHAR * cpucBuffer,
                                UINT32 uBufferLength, UINT32 uAddress);
BOOL wbcWriteLCMBuffer_Fill (UCHAR ucR, UCHAR ucG, UCHAR ucB);
BOOL wbcWriteLCMBuffer_Flush (VOID);
BOOL wbcSuspendModule (WB_SUSPEND_TYPE_E eOption);
BOOL wbcSetLCMControlMode (WB_LCM_CONTROL_MODE_E eMode);
BOOL wbcEnablePreview (WB_SWITCH_E eSwitch);
BOOL wbcGetCaptureOrPlaybackStatus (WB_PROCESS_E * peCaptureStatus,
                                    WB_PROCESS_E * pePlaybackStatus,
                                    USHORT * pusFrame);
BOOL wbcSetFrameRate (UCHAR ucScaleN, UCHAR ucScaleM,
                      UCHAR ucSensorFrameRate);
BOOL wbcSetGrid (UCHAR ucHGrid, UCHAR ucVGrid);
BOOL wbcPlaybackVideoClip (WB_VIDEO_PLAYBACK_OPERATION_E eOperation,
						   WB_VIDEO_PLAYBACK_FLAG_E eFlag);
BOOL wbcPlaybackMovie (WB_MOVIE_PLAYBACK_OPERATION_E eOperation,
					   USHORT usSeconds);
BOOL wbcSendVideoClipHeaderInformation (CONST UCHAR * cpucBuffer,
                                        UINT32 uBufferLength);
BOOL wbcSetImageEffect (WB_IMAGE_EFFECT_E eEffect,
                        UCHAR ucY, UCHAR ucU, UCHAR ucV);
BOOL wbcSendStickerMakerPacketMask (CONST UCHAR * cpucBuffer,
                                    UINT32 uBufferLength);
BOOL wbcSendStickerMakerPlanarMask (CONST UCHAR * cpucBuffer,
                                    UINT32 uBufferLength);
BOOL wbcSendStickerMakerPattern (CONST UCHAR * cpucBuffer,
                                 UINT32 uBufferLength);
BOOL wbcDisableStickerMaker (VOID);
BOOL wbcEnableOSD (WB_SWITCH_E eSwitch);
BOOL wbcSetOSDKeyPattern (UCHAR ucR, UCHAR ucG, UCHAR ucB);
BOOL wbcSetOSDKeyMask (UCHAR ucR, UCHAR ucG, UCHAR ucB);
BOOL wbcSendOSDPattern_Method1 (CONST UCHAR * cpucBuffer,
                                UINT32 uBufferLength, UINT32 uAddress);
BOOL wbcSendOSDPattern_Method2 (CONST UCHAR * cpucBuffer,
                                UINT32 uBufferLength, UINT32 uAddress);
BOOL wbcSendOSDPattern_Flush (VOID);
BOOL wbcSetBlinkingRate (UCHAR ucCycle);
BOOL wbcEnableBlinking (WB_SWITCH_E eSwitch);
BOOL wbcSetBlendingWeight (UCHAR ucWeight);
BOOL wbcEnableBlending (WB_SWITCH_E eSwitch);
BOOL wbcGetPlanarYData (USHORT usIndex, UINT32 * puLength);
BOOL wbcGetPlanarUData (USHORT usIndex, UINT32 * puLength);
BOOL wbcGetPlanarVData (USHORT usIndex, UINT32 * puLength);
BOOL wbcSendPlanarYData (CONST UCHAR * cpucBuffer,
                         UINT32 uBufferLength);
BOOL wbcSendPlanarUData (CONST UCHAR * cpucBuffer,
                         UINT32 uBufferLength);
BOOL wbcSendPlanarVData (CONST UCHAR * cpucBuffer,
                         UINT32 uBufferLength);
BOOL wbcEncodeJPEG (USHORT usWidth, USHORT usHeight);
BOOL wbcSetPanandTile (USHORT usX, USHORT usY);
BOOL wbcSetExtentOffset (USHORT usWidth_Offset, USHORT usHeight_Offset);
BOOL wbcSetI2CMode (UCHAR ucAddress, WB_I2C_ACCESS_MODE_E eMode);
BOOL wbcGetBitstreamBufferSize_All (UINT32 * puLength);
BOOL wbcGetBitstreamBufferSize_Free (UINT32 * puLength);
BOOL wbcAlternativeSetImageSize (USHORT usWidth, USHORT usHeight);
BOOL wbcAlternativeGetImageSize (USHORT * pusWidth,
                                 USHORT * pusHeight);
BOOL wbcEnableCommandInterrupt (UCHAR ucSetting);
BOOL wbcGetInterruptStatus (UCHAR * pucStatus);
BOOL wbcGetBufferInformation (WB_BUFFER_INFO_E eIndex, UINT32 * puValue);
BOOL wbcSetAudioSamplingRate (UINT32 uHz);
BOOL wbcGetAudioSamplingRate (UINT32 * puHz);
BOOL wbcSetAudioBitRate (UINT32 uBits);
BOOL wbcGetAudioBitRate (BOOL * pbVBR, UINT32 * puBits);
BOOL wbcSetAudioChannels (UCHAR ucChannel,
                          WB_AUDIO_STEREO_TYPE_E eStereoType,
                          WB_AUDIO_STEREO_EXT_TYPE_E eStereoExtType);
BOOL wbcGetAudioChannels (UCHAR * pucChannel,
                          WB_AUDIO_STEREO_TYPE_E * peStereoType,
                          WB_AUDIO_STEREO_EXT_TYPE_E * peStereoExtType);
BOOL wbcGetAudioAncillaryData (UINT32 * puLength);
BOOL wbcSetAudioOperation (WB_AUDIO_OPERATION_E eOperation,
                           USHORT usSeconds,
                           WB_RETURN_E * peError, WB_PROCESS_E * peStop);
BOOL wbcSetPlaybackAudioVolume (UCHAR ucVolume);
BOOL wbcGetCurrentPlaybackAudioVolume (UCHAR * pucVolume);
BOOL wbcGetMaxPlaybackAudioVolume (UCHAR * pucVolume);
BOOL wbcSetVideoBitRate (UINT32 uBitRate);
BOOL wbcGetVideoBitRate (UINT32 *puBitRate);
BOOL wbcEnableMute (WB_SWITCH_E eSwitch);
BOOL wbcGetMuteStatus (WB_SWITCH_E * peSwitch);
BOOL wbcSetRecordAudioVolume (UCHAR ucVolume);
BOOL wbcGetCurrentRecordAudioVolume (UCHAR * pucVolume);
BOOL wbcGetMaxRecordAudioVolume (UCHAR * pucVolume);
BOOL wbcSetAudioFormat (WB_AUDIO_FORMAT_E ucFormat);
BOOL wbcGetMediaTicks (WB_MEDIA_TICKS_E eOption, UINT32 *puValue);
BOOL wbcSetEqualizer (WB_AUDIO_EQ_E ucEQ);
BOOL wbcGetEqualizer (WB_AUDIO_EQ_E * peEQ);
BOOL wbcSetUserDefinedEqualizer (CONST CHAR cpcBands[12]);
BOOL wbcGetUserDefinedEqualizer (CHAR pcBands[12]);
BOOL wbcGetAudioSpectrum (UCHAR ucGroup, UCHAR pucBand[8]);
BOOL wbcGetID3Tags (WB_AUDIO_ID3_TAGS_E eOption,			//Which tags
					UCHAR * pucInformation,	//Tag information
					UINT32 uLength,			//Max length of tag information
					UINT32 * puRealLength	//Return length of tag information
					);
BOOL wbcSetMPEG4IntraInterval (USHORT usInterval);
BOOL wbcSetSlideShowTransitionEffect (WB_SLIDE_SHOW_TRANSITION_EFFECT_E eEffect);
//BOOL wbcSetMPEG4IntraInterval
BOOL wbcGetFlashMemoryCardStatus (UCHAR ucDriver, WB_CARD_STATUS_E * peStatus);
BOOL wbcFormatFlashMemoryCard (UCHAR ucDriver, WB_CARD_STATUS_E * peStatus);
BOOL wbcGetFlashMemoryCardCapacity_All (UCHAR ucDriver, UINT32 * puSize);
BOOL wbcGetFlashMemoryCardCapacity_Free (UCHAR ucDriver, UINT32 * puSize);
BOOL wbcSetShortFilename (UINT32 uNumber,
						  CONST UCHAR *cpucName,
						  INT * pnStatus);
BOOL wbcCreateSubDirectory (CONST WSTR_CHAR *cpwFullPath,
                            INT * pnStatus);
BOOL wbcRemoveSubDirectory (CONST WSTR_CHAR *cpwFullPath,
                            INT * pnStatus);
BOOL wbcOpenFile (INT32 * pnHandle,
                  CONST WSTR_CHAR *cpwFullPath, WB_O_FLAGS_E eFlag);
BOOL wbcReadFile (INT32 nHandle,
                  UCHAR * pucData,
                  UINT32 uLength,
                  UINT32 * puRealLength,
                  INT * pnStatus);
BOOL wbcWriteFile (INT32 nHandle,
                   CONST UCHAR * cpucData,
                   UINT32 uLength,
                   UINT32 * puRealLength,
                   INT * pnStatus);
BOOL wbcCloseFile (INT32 nHandle, INT *pnStatus);
BOOL wbcDeleteFile (CONST WSTR_CHAR *cpwFullPath, INT * pnStatus);
BOOL wbcSeekFile (INT32 nHandle,
                  INT nOffset, WB_SEEK_POSITION_E eFlag, INT * pnStatus);
BOOL wbcGetFilePosition (INT nHandle, INT * pnStatus);
BOOL wbcSetFileDateTime(INT nHandle,
						CONST WSTR_CHAR *cpwFullPath,
						UCHAR ucDay,	//Ex: 08 in "Jan 08 2004"
						UCHAR ucMonth,	//Ex: 01 in "Jan 08 2004"
						UCHAR ucYear,	//Ex: 24(2004-1980) in "Jan 08 2004"
						UCHAR ucMin,	//Ex: 57 in "23:57:56"
						UCHAR ucSec,	//Ex: 56 in "23:57:56"
						UCHAR ucHour,	//Ex: 23 in "23:57:56"
						INT *pnStatus
						);
BOOL wbcGetFileDateTime(INT nHandle,
						CONST WSTR_CHAR *cpwFullPath,
						UCHAR *pucDay,	//Ex: 08 in "Jan 08 2004"
						UCHAR *pucMonth,	//Ex: 01 in "Jan 08 2004"
						UCHAR *pucYear,	//Ex: 24(2004-1980) in "Jan 08 2004"
						UCHAR *pucMin,	//Ex: 57 in "23:57:56"
						UCHAR *pucSec,	//Ex: 56 in "23:57:56"
						UCHAR *pucHour,	//Ex: 23 in "23:57:56"
						INT *pnStatus
						);
BOOL wbcGetFileLength(INT nHandle,
					  CONST WSTR_CHAR *cpwFullPath,
					  UINT32 * puLength,
					  INT *pnStatus);
BOOL wbcCopyFile(CONST WSTR_CHAR *cpwOldFullPath,
				 CONST WSTR_CHAR *cpwNewFullPath,
				 INT32 *pnStatus);
BOOL wbcMoveFile(CONST WSTR_CHAR *cpwOldFullPath,
				 CONST WSTR_CHAR *cpwNewFullPath,
				 WB_FS_TYPE_E eFlag,	//1, directory, 0, file.
				 INT32 *pnStatus);
BOOL wbcSetFileAttribute(INT nHandle,
						 CONST WSTR_CHAR *cpwFullPath,
						 WB_FS_ATTR_E eAttribute,
						 INT *pnStatus
						 );
BOOL wbcGetFileAttribute(INT nHandle,
						 CONST WSTR_CHAR *cpwFullPath,
						 WB_FS_ATTR_E *peAttribute,
						 INT *pnStatus
						 );
BOOL wbcFindFirstFile(CONST WSTR_CHAR *cpwFullPath,
					  CONST WSTR_CHAR *cpwFileNameMask,
					  WB_FS_ATTR_E eAttributeMask,
					  WSTR_CHAR *pwFileNameBuffer,
					  UINT32 uFileNameBufferLength,
					  UINT32 *puFileNameBufferLength_Used,
					  INT32 *pnHandle);
BOOL wbcFindNextFile(INT32 nHandle,
					 WSTR_CHAR *pwFileNameBuffer,
					 UINT32 uFileNameBufferLength,
					 UINT32 *puFileNameBufferLength_Used,
					 INT32 *pnStatus);
BOOL wbcFindClose(INT nHandle);
BOOL wbcGetShortFilename (INT nHandle,
						  UCHAR *pwFileNameBuffer,
						  UINT32 uFileNameBufferLength,
						  UINT32 *puFileNameBufferLength_Used);
BOOL wbcLoadFromMemoryCard(CONST WSTR_CHAR *cpwFullPath,
						   INT32 *pnStatus);
BOOL wbcSaveToMemoryCard(UINT32 uIndex,
						 CONST WSTR_CHAR *cpwFullPath,
						 INT32 *pnStatus);
BOOL wbcLoadBackgroundAudioFromMemoryCard (
	CONST WSTR_CHAR *cpwFullPath,
	INT32 *pnStatus);
BOOL wbcSetOverlayWindowPosition (USHORT usX, USHORT usY);
BOOL wbcSetOverlayWindowSize (USHORT usWidth, USHORT usHeight);
BOOL wbcSetCaptureZoom(WB_CAPZOOM_OPERATION_E eOperation,
					   UCHAR ucStep,
					   WB_CAPZOOM_STATUS_E *peStatus,
					   UCHAR *pucMaxSteps,
					   UCHAR *pucCurrentStep);
BOOL wbcGetMaxZoomRatio(USHORT *pusMinCrop, USHORT *pusMaxCrop);
BOOL wbcSendMediaData (CONST UCHAR *cpucData, UINT32 uLength);
BOOL wbcSetPlaybackZoom (WB_PLAYZOOM_E eOption,
						 UCHAR *pucLeftBorder,
						 UCHAR *pucUpBorder,
						 UCHAR *pucRightBorder,
						 UCHAR *pucBottomBorder);
BOOL wbcBadPixelCompensation(UINT32 uAddress);
BOOL wbcLensShadingCompensation(UINT32 uCenterPos,
								UINT32 uYR,
								UINT32 uUG,
								UINT32 uVB);
BOOL wbcSelectInputSource(WB_INPUT_SRC_E eSource, WB_INPUT_TV_E eTV);
BOOL wbcI2CMultipleWrite(UINT32 uAddress, UINT32 uLength, CONST UCHAR *cpucData);
BOOL wbcI2CMultipleRead(UINT32 uAddress, UINT32 uLength, UCHAR *pucData);
BOOL wbcParseMediaOpen(UINT32 uFrames,
					   CONST WSTR_CHAR *cpwFullPath,
					   INT32 *pnStatus,
					   INT32 *pnHandle);
BOOL wbcGetParsedInformation(INT32 nHandle,
							 WB_PARSE_MEDIA_INFO_E eIndex,
							 UCHAR * pucInformation,	//Information data buffer
							 UINT32 uLength,			//Max length of information buffer
							 UINT32 * puRealLength	//Return length of information data
							 );
BOOL wbcParseMediaClose (INT32 nHandle);
BOOL wbcSetNANDReservedAreaSize (UINT32 uSize);
BOOL wbcGetNANDReservedAreaSize (UINT32 *puSize);
BOOL wbcSetNANDReservedAreaAccessAddress (UINT32 uAddress);
BOOL wbcEarseNANDReservedArea (VOID);
BOOL wbcReadNANDReservedArea (UCHAR *pucData, UINT32 uLength);
BOOL wbcWriteNANDReservedArea (CONST UCHAR *cpucData, UINT32 uLength);
BOOL wbcGetDirectoryInformation (CONST WSTR_CHAR *cpwFullPath,
								 INT *pnStatus,
								 UINT32 *puFileCount,
								 UINT32 *puDirectoryCount,
								 UINT32 *puTotalSize);
BOOL wbcSet3DSurround (WB_3D_OPERATION_E eOperation,
					   UCHAR ucSetting,
					   UCHAR *pucStatus);

#endif

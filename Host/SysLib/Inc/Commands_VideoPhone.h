#ifndef __COMMANDS_VIDEOPHONE_H__
#define __COMMANDS_VIDEOPHONE_H__


/* Interrupt status. */
typedef enum
{
	CMD_HIC_INTR_MASK				= 0x70,
	CMD_HIC_INTR_STATE_CHANGED		= 0x00,
	CMD_HIC_INTR_APPLICATION_EVENT	= 0x10
} CMD_HIC_INTR_E;


typedef enum
{
	CMD_TYPE_1	= (0x01 << 0),
	CMD_TYPE_2	= (0x01 << 1),
	CMD_TYPE_3	= (0x01 << 2),
	CMD_TYPE_4	= (0x01 << 3),
	CMD_TYPE_5	= (0x01 << 4)
} CMD_TYPE_E;


typedef enum
{
	CMD_INTR_AUDIO			= 0,
	CMD_INTR_VIDEO			= 1,
	CMD_INTR_MOTION_DETECT	= 2,
	CMD_INTR_JPEG			= 3,
    CMD_INTR_REMOTEFUNC     = 4
} CMD_INTR_EVENT_E;


typedef enum
{
	CMD_SYSTEM_CAPABILITY_MAX_LCM_PIXELS			= 0x01,
	CMD_SYSTEM_CAPABILITY_MAX_AUDIO_QUALITY_LEVEL	= 0x02,
	CMD_SYSTEM_CAPABILITY_MAX_MP4_BITRATE			= 0x03,
	CMD_SYSTEM_CAPABILITY_MAX_MP4_PIXELS			= 0404
} CMD_SYSTEM_CAPABILITY_E;

typedef enum
{
	CMD_ENABLE					= 1,
	CMD_DISABLE					= 0
} CMD_SWITCH_E;


typedef enum
{
	CMD_MEDIA_AUDIO	= 0,
	CMD_MEDIA_VIDEO	= 1,
	CMD_MEDIA_JPEG	= 2,
	CMD_MEDIA_REMOTEFUNC	= 3
} CMD_MEDIA_TYPE_E;


typedef enum
{
	CMD_LCM_NUM_NONE		= 0x00,
	CMD_LCM_NUM_1ST			= 0x01,
	CMD_LCM_NUM_2ND			= 0x02,
	CMD_LCM_1LCM_ON_BUS1	= 0x11,
	CMD_LCM_2LCM_ON_BUS1	= 0x12,
	CMD_LCM_1LCM_ON_BUS2	= 0x21,
	CMD_LCM_2LCM_ON_BUS2	= 0x22
} CMD_LCM_NUM_E;


typedef enum
{
	CMD_LCM_CMD_BUS_WIDTH_8		= 0,
	CMD_LCM_CMD_BUS_WIDTH_16	= 1
} CMD_LCM_CMD_BUS_WIDTH_E;

typedef enum
{
	CMD_LCM_DATA_BUS_WIDTH_8	= 0,
	CMD_LCM_DATA_BUS_WIDTH_9	= 1,
	CMD_LCM_DATA_BUS_WIDTH_16	= 2,
	CMD_LCM_DATA_BUS_WIDTH_18	= 3
} CMD_LCM_DATA_BUS_WIDTH_E;

typedef enum
{
	CMD_LCM_68_MPU			= 0,
	CMD_LCM_80_MPU			= 1
} CMD_LCM_MPU_MODE_E;

typedef enum
{
	CMD_LCM_COLOR_WIDTH_12	= 0,
	CMD_LCM_COLOR_WIDTH_16	= 1,
	CMD_LCM_COLOR_WIDTH_18	= 2,
	CMD_LCM_COLOR_WIDTH_24	= 3
} CMD_LCM_COLOR_WIDTH_E;

typedef enum
{
	CMD_LCM_OSD_YUV422		= 1,
	CMD_LCM_OSD_YCbCr422	= 2,
	CMD_LCM_OSD_RGB888		= 3,
	CMD_LCM_OSD_RGB666		= 4,
	CMD_LCM_OSD_RGB565		= 5,
	CMD_LCM_OSD_RGB444low	= 6,
	CMD_LCM_OSD_RGB444high	= 7,
	CMD_LCM_OSD_RGB332		= 8
} CMD_LCM_OSD_COLOR_E;

typedef enum
{
	CMD_ROTATE_NORMAL	= 0,
	CMD_ROTATE_LEFT		= 1,
	CMD_ROTATE_RIGHT	= 2,
	CMD_ROTATE_MIRROR	= 3,
	CMD_ROTATE_FLIP		= 4,
	CMD_ROTATE_R180		= 5
} CMD_ROTATE_E;

typedef enum
{
	CMD_DISPLAY_OSD				= 0,
	CMD_DISPLAY_LOCAL_VIDEO		= 1,
	CMD_DISPLAY_REMOTE_VIDEO	= 2
} CMD_DISPLAY_TYPE_E;

typedef enum
{
	CMD_VIDEO_H263		= 0,
	CMD_VIDEO_MP4		= 1
} CMD_VIDEO_FORMAT_E;


typedef enum
{
	CMD_MOTION_DETECT_HIGH		= 0,
	CMD_MOTION_DETECT_MIDDLE	= 1,
	CMD_MOTION_DETECT_LOW		= 2
} CMD_MOTION_DETECT_SENSIBILITY_E;

typedef enum
{
	CMD_AUDIO_AMR		= 0,
	CMD_AUDIO_PCM		= 1,
	CMD_AUDIO_IMA_ADPCM	= 2,
	CMD_AUDIO_ULAW		= 3,
	CMD_AUDIO_ALAW		= 4
} CMD_AUDIO_FORMAT_E;









#define CMD_USE_FIFO_CHECK_SUM	TRUE

VOID wbvClearInterrupt (VOID);
CMD_HIC_INTR_E wbvGetInterruptType (VOID);

BOOL wbvGetFirmwareVersion (UCHAR * pucMonth, UCHAR * pucDay, UCHAR * pucHour, UCHAR * pucMin);
BOOL wbvGetFirmwareVersion5 (UCHAR *pucVersion, UINT32 uMaxVersionLen,
							   UCHAR *pucChangeDate, UINT32 uMaxChangeDateLen,
							   UCHAR *pucBuildDate, UINT32 uMaxBuildDateLen);
BOOL wbvGetSystemCapability (CMD_SYSTEM_CAPABILITY_E eType, UINT32 *puValue);
BOOL wbvGetInterruptEvent (BOOL bClearInterrupt, UCHAR pucReturn[4]);
BOOL wbvEnableCommandInterrupt (BOOL bEnableCommandIntr,
								BOOL bEnableApplicationEventIntr);
BOOL wbvSendMediaBitstream (CMD_MEDIA_TYPE_E eType, CONST UCHAR *cpucData, UINT32 uLength);
BOOL wbvRecvMediaBitstream_NoData (CMD_MEDIA_TYPE_E eType, UINT32 *puLength);
BOOL wbvRecvMediaBitstream_WithData (CMD_MEDIA_TYPE_E eType, UCHAR *pucData);
BOOL wbvConfigureLCM (
	USHORT						usWidth,		/* LCM width */
	USHORT						usHeight,		/* LCM height */ 
	CMD_LCM_COLOR_WIDTH_E		eLcmColor,
	CMD_LCM_CMD_BUS_WIDTH_E		eCmdBus,
	CMD_LCM_DATA_BUS_WIDTH_E	eDataBus,
	int							iMPU_Cmd_RS_pin,/* 0 or 1 (when send MPU command) */
	BOOL						b16t18,			/* Convert 16-bit video to 18-bit? */
	CMD_LCM_MPU_MODE_E			eMpuMode,
	UINT32						*puStatus
	);
BOOL wbvSelectLCM (UCHAR ucNumber, UINT32 *puStatus);
BOOL wbvSetOSDColor (
	CMD_LCM_OSD_COLOR_E	eOsdColor,
	UCHAR				ucKeyColor_R,
	UCHAR				ucKeyColor_G,
	UCHAR				ucKeyColor_B,
	UCHAR				ucKeyMask_R,
	UCHAR				ucKeyMask_G,
	UCHAR				ucKeyMask_B,
	UINT32				*puStatus
	);
BOOL wbvWriteOSD (
	SHORT sLeft, SHORT sTop, USHORT usWidth, USHORT usHeight,
	CONST UCHAR *cpucData, UINT32 uDataLen
	);
BOOL wbvFillOSD (BOOL bFillBackground, UCHAR ucR, UCHAR ucG, UCHAR ucB);
BOOL wbvEnableDisplay (CMD_DISPLAY_TYPE_E eDisplayType, BOOL bEnable);
BOOL wbvSetLocalVideoWindow (
	SHORT sLeft, SHORT sTop, USHORT usWidth, USHORT usHeight, CMD_ROTATE_E eRotate);
BOOL wbvSetRemoteVideoWindow (
	SHORT sLeft, SHORT sTop, USHORT usWidth, USHORT usHeight, CMD_ROTATE_E eRotate);
BOOL wbvSetLocalVideoSource (
	USHORT usWidth, USHORT usHeight, CMD_ROTATE_E eRotate);
BOOL wbvEnableVideoEncoder (BOOL bEnable);
BOOL wbvSetVideoZIndex (BOOL bLocalOnTop);
BOOL wbvEnableAudioEncoder (BOOL bEnable);
BOOL wbvSetAudioQuality (UCHAR ucQuality);
BOOL wbvGetAudioQuality (UCHAR *pucQuality);
BOOL wbvSetVideoBitrate (UINT32 uBitrate);
BOOL wbvGetVideoBitrate (UINT32 *puBitrate);
BOOL wbvSetVideoFormat (CMD_VIDEO_FORMAT_E eFormat);
BOOL wbvGetDecodedMedia_NoData (CMD_MEDIA_TYPE_E eType, UINT32 *puLength);
BOOL wbvGetDecodedMedia_WithData (CMD_MEDIA_TYPE_E eType, UCHAR *pucData);
BOOL wbvEnableVideoBlur (BOOL bEnable);
BOOL wbvEnableMotionDetect (BOOL bEnable, CMD_MOTION_DETECT_SENSIBILITY_E eSensibility);
BOOL wbvGetMotionsNum (UINT32 *puNum);
BOOL wbvSetAudioFormat (CMD_AUDIO_FORMAT_E eFormat);
BOOL wbvForceMP4IFrame (UINT32 uNum);
BOOL wbvStretchVideo (USHORT usScaleV, USHORT usScaleH);
BOOL wbvEnableJPEGEncoder (BOOL bEnable);
BOOL wbvSetDateTime (/* The range of time value is the same as what's in struct tm. */
	USHORT usYear,	/* Year since 1900. */
	UCHAR ucMon,	/* 0 ~ 11 */
	UCHAR ucMDay,	/* 1 ~ 31 */
	UCHAR ucHour,	/* 0 ~ 23 */
	UCHAR ucMin,	/* 0 ~ 59 */
	UCHAR ucSec		/* 0 ~ 59 */
	);
BOOL wbvSetVideoFramerate (UINT32 uFramerate);
BOOL wbvGetVideoFramerate (UINT32 *puFramerate);


BOOL wbvGetDebugBuffer (UINT32 *puInfoAddr, UINT32 *puInfoSize, UINT32 *puErrorAddr, UINT32 *puErrorSize);

#endif

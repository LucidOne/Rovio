#ifndef __VP_INTERFACES_H__
#define __VP_INTERFACES_H__

#include "wbtypes.h"
#include "Commands_Videophone.h"
#include "Commands_CameraModule.h"

#define MP4ADDATIONBEGIN	"=======["
#define MP4ADDATIONEND		"=======]"

typedef enum
{
	WB702CONFIGLCM = 1,
	WB702ENABLEDISP,
	WB702SETOSDCOL,
	WB702FILLOSD,
	WB702SETLOCALVIDEOSRC,
	WB702SETLOCALVIDEOWIN,
	WB702SETREMOTEVIDEOWIN,
	WB702ENALBEMP4ENCODER,
	WB702ENALBEAUDIOENCODER,
	WB702SETOSD,
	WB702INDEX_Z,
	WB702SETFRAME,
	WB702ENABLEENCODER,
	WB702SETVIDEOFORMAT,
	WB702SETVIDEOBITRATE,
	//WB702DOWNLFIRMWARE,
	
	WB702GETFIRMARENUM,
	WB702GETFIRMARESTR,
	WB702GETAUDIOQUALITY,
	WB702SETAUDIOQUALITY,
	WB702GETVIDEOBITRATE,
	
	WB702ENABLEMOTIONDETECT,
	WB702GETMOTIONSNUM,
	WB702SETAUDIOTYPE,
	
	//WB702POLLEVENT,
	//WB702READAUDIO,
	//WB702WRITEAUDIO,
	//WB702READVIDEO,
	//WB702WRITEVIDEO,
	//WB702SETIOCTLRW,
	
	WB702SETBITMAP,
	WB702ENALBEMP4IFRAME,
	WB702STRETCHVIDEO,
	
	//WB702READJPEG,
	//WB702WRITEJPEG,
	WB702SETJPEG,
	WB702SETDATETIME,
	WB702GETDATETIME,
	
	WB702SUSPENDFW,
	OLDWB702GETFWVERSION,
	OLDWB702CFGLCM,
	OLDWB702ENABLEOSD,
	OLDWB702SETOSDKEYMASK,
	OLDWB702SNDPATTERN,
	OLDWB702OSDFLUSH,
	
	WB702ENABLEFMI,
	
	WB702SETVIDEOFRAMERATE,
	WB702GETVIDEOFRAMERATE,
	
	WB702SETJPEGQUALITY,
	WB702GETJPEGQUALITY,
	
	WB702SETVIDEODYNAMICBITRATE,
	WB702SETVIDEODYNAMICFRAMERATE,
	
	WB702SETAUDIOPLAYVOLUME,
	WB702SETAUDIORECORDVOLUME,
	WB702SETAUDIONOTIFICATIONVOLUME,
	WB702SETVIDEOCONTRASTBRIGHT,
	
	WB702ENABLEDRAWIMAGETIME,
	WB702SETDRAWCONTENT,
	WB702CLEARDRAWCONTENT
}IOCTL_CMD_T;

typedef struct configure_lcm
{
	USHORT							usWidth;		/* LCM width */
	USHORT							usHeight;	/* LCM height */ 
	CMD_LCM_COLOR_WIDTH_E		eLcmColor;
	CMD_LCM_CMD_BUS_WIDTH_E	eCmdBus;
	CMD_LCM_DATA_BUS_WIDTH_E	eDataBus;
	int								iMPU_Cmd_RS_pin;/* 0 or 1 (when send MPU command) */
	BOOL							b16t18;			/* Convert 16-bit video to 18-bit? */
	CMD_LCM_MPU_MODE_E			eMpuMode;
}CONFIGURE_LCM_t,*pCONFIGURE_LCM_t;

typedef struct display_lcm
{
	CMD_DISPLAY_TYPE_E eDisplayType;
	BOOL bEnable;
}DISPLAY_LCM_t,*pDISPLAY_LCM_t;

typedef struct osd_color
{
	CMD_LCM_OSD_COLOR_E	eOsdColor;
	UCHAR				ucKeyColor_R;
	UCHAR				ucKeyColor_G;
	UCHAR				ucKeyColor_B;
	UCHAR				ucKeyMask_R;
	UCHAR				ucKeyMask_G;
	UCHAR				ucKeyMask_B;
}OSD_COLOR_t,*pOSD_COLOR_t;

typedef struct fill_osd
{
	BOOL bFillBackground;
	UCHAR ucR;
	UCHAR ucG;
	UCHAR ucB;
}FILL_OSD_t,*pFILL_OSD_t;

typedef struct set_local_video_source
{
	USHORT usWidth;
	USHORT usHeight;
	CMD_ROTATE_E eRotate;
}SET_LOCAL_VIDEO_SOURCE_t,*pSET_LOCAL_VIDEO_SOURCE_t;

typedef struct set_video_win
{
	SHORT sLeft;
	SHORT sTop;
	USHORT usWidth;
	USHORT usHeight;
	CMD_ROTATE_E eRotate;
}SET_VIDEO_WIN_t,*pSET_VIDEO_WIN_t;

typedef enum
{
	CMD_DATA_AUDIO	= 0,
	CMD_DATA_VIDEO	= 1,
	CMD_DATA_OSD          = 100
} CMD_DATA_TYPE_E;

typedef struct Wb_Tx
{
	INT  mediatype;
	INT txlen;
	UCHAR  *txbuf;
}Wb_Tx_t,*PWb_Tx_t;

typedef struct write_osd
{
	SHORT sLeft;
	SHORT sTop;
	USHORT usWidth;
	USHORT usHeight;
	UINT32 uDataLen;
	CONST UCHAR *cpucData;
}WRITE_OSD_t,*PWRITE_OSD_t;

typedef enum
{
	WB_READ_NOFRAME 	= 0,
	WB_READ_FRAME	= 1
} WB_FRAME_TYPE;

typedef struct _version
{
	UCHAR  Month;
	UCHAR  Day;
	UCHAR  Hour;
	UCHAR  Min;
}WB_VERSION_t;

typedef struct _version_str
{
	UCHAR version[32];
	UCHAR changedate[32];
	UCHAR builddate[64];
}WB_VERSION_Ext_t;

typedef struct _motion_detect
{
	BOOL flags; 
	CMD_MOTION_DETECT_SENSIBILITY_E type;
}WB_MOTION_DETECT_t;

typedef enum
{
	IOCTL_AUDIO			= 0x01,
	IOCTL_VIDEO			= 0x02,
	IOCTL_MOTION_DETECT	= 0x04,	
	IOCTL_JPEG			= 0x08
} IOCTL_EVENT_E;

struct stretch_video
{
	USHORT scale_V;
	USHORT scale_H;
};

typedef struct
{
	USHORT usYear;
	UCHAR ucMon;
	UCHAR ucMDay;
	UCHAR ucHour;
	UCHAR ucMin;
	UCHAR ucSec;
} CMD_TM_T;

typedef enum
{
	MEDIA_TYPE_VIDEO = 1,
	MEDIA_TYPE_AUDIO,
	MEDIA_TYPE_JPEG,
	MEDIA_TYPE_MOTION
}MEDIA_TYPE_T;

typedef struct
{
	CMD_AUDIO_FORMAT_E eEncodeFormat;
	CMD_AUDIO_FORMAT_E eDecodeFormat;
}AUDIO_FORMAT_T;

typedef struct
{
	INT		mediatype;
	INT		txlen;
	UCHAR 	*txbuf;
	VOID	*extradata;
}IO_THREAD_READ_T;

typedef enum {
	IO_THREAD_READ_OVER = -1,
	IO_THREAD_READ_ERROR,
	IO_THREAD_READ_OK
}IO_THREAD_READ_STATUS;

typedef struct {
	INT lvolume;
	INT rvolume;
}WB_AUDIO_VOLUME;

typedef struct {
	INT contrast;
	INT bright;
}WB_VIDEO_CONTRAST_BRIGHT;

typedef enum
{
	//MP4_CHECK_INDEX_LEVEL0 = 0,	/* loosest, start token and end token may not exist */
	MP4_CHECK_INDEX_LEVEL1,		/* middle, start token and end token may not exist. 
								   but if start token exists, end token must also exist */
	MP4_CHECK_INDEX_LEVEL2		/* strictest, whether start token or end token not exists, error happen */
}MP4_CHECK_INDEX_LEVEL;

typedef struct
{
	CHAR	*pcmp4buf;					// Frame buffer for mp4 collection
	INT		imp4buflen;					// Frame buffer size
	INT		imp4bufdatalen;				// Not provided for user
	
	CHAR	*funcname;					// Function name for debug info
	
	BOOL	bcontinuecheck;				// Whether one frame is parsed into two packets
	MP4_CHECK_INDEX_LEVEL checklevel;	// Error detect level
	
	INT		lastmp4index;				// Not provided for user
	INT		lastmp4indexprintcount;		// Not provided for user
	BOOL	lastmp4indexprint;			// Not provided for user
}MP4_CHECK_INDEX;

typedef struct
{
	INT		index;
	CHAR	*pcContent;
	INT		x;
	INT		y;
}INFO_CONTENT;

int iothread_Init(void);
//int iothread_SendNotify(MEDIA_TYPE_T mpType);
//void iothread_ClearNotify(MEDIA_TYPE_T mpType);
//int iothread_SendAudioNotify(MEDIA_TYPE_T mpType);
//int iothread_ReadAudio(IO_THREAD_READ_T *pArg);
//int iothread_ReadVideo(IO_THREAD_READ_T *pArg);
//int iothread_ReadAudio_Complete(IO_THREAD_READ_T *pArg);
//int iothread_ReadVideo_Complete(IO_THREAD_READ_T *pArg);
int iothread_Write(IO_THREAD_READ_T *pArg);
int iothread_WriteNotification(UCHAR *pucBuffer, INT32 iLen, void(*callback)(void));

int wb702ConfigureLCM(USHORT usWidth,		/* LCM width */
						USHORT	 usHeight,	/* LCM height */ 
						CMD_LCM_COLOR_WIDTH_E eLcmColor,
						CMD_LCM_CMD_BUS_WIDTH_E eCmdBus,
						CMD_LCM_DATA_BUS_WIDTH_E eDataBus,
						int iMPU_Cmd_RS_pin,/* 0 or 1 (when send MPU command) */
						BOOL b16t18,			/* Convert 16-bit video to 18-bit? */
						CMD_LCM_MPU_MODE_E eMpuMode);

int wb702EnableDisplay(CMD_DISPLAY_TYPE_E eDisplayType, BOOL bEnable);

int wb702SetOSDColor(CMD_LCM_OSD_COLOR_E eOsdColor,
			UCHAR ucKeyColor_R, UCHAR ucKeyColor_G, UCHAR ucKeyColor_B,
			UCHAR ucKeyMask_R,  UCHAR ucKeyMask_G, UCHAR ucKeyMask_B);

int wb702FillOSD(BOOL bFillBackground, UCHAR ucR, UCHAR ucG, UCHAR ucB);

int wb702SetLocalVideoSource(USHORT usWidth, USHORT usHeight, CMD_ROTATE_E eRotate);

int wb702OSDDisplay(void);

int wb702SetLocalVideoWindow(SHORT sLeft, SHORT sTop, USHORT usWidth, USHORT usHeight, CMD_ROTATE_E eRotate);

int wb702SetRemoteVideoWindow(SHORT sLeft, SHORT sTop, USHORT usWidth, USHORT usHeight, CMD_ROTATE_E eRotate);

int wb702SetVideoZIndex(BOOL enable);

int wb702EnableMP4Encoder(BOOL enable);
int wb702EnableAudioEncoder(BOOL enable);
BOOL wb702LockAndDisableAudio(void);
void wb702UnlockAndRestorePreviousAudio(BOOL bPreviousState);

int wb702EnableEncoder(BOOL arg);

int wb702SetFrame(WB_FRAME_TYPE frame);

int wb702SetVideoFormat(CMD_VIDEO_FORMAT_E format);
int wb702SetOSD(Wb_Tx_t *buf);
int wb702GetFirmwareVersion(WB_VERSION_t *version);
int wb702GetFirmwareVersionExt(WB_VERSION_Ext_t * version);
int wb702GetAudioQuality(char * quality);
int wb702SetAudioQuality(char * quality);
int wb702GetVideoBitRate(char * bitrate);
int wb702SetVideoBitRate(UINT bitrate);

//motion detect command interface
int wb702EnableMotionDetect(BOOL enable, CMD_MOTION_DETECT_SENSIBILITY_E type);
int wb702GetMotionsNum(UINT * num);
int wb702SetAudioType(CMD_AUDIO_FORMAT_E encodeType, CMD_AUDIO_FORMAT_E decodeType);

//bypass mode
int wb702BypassBitmap(int position, UCHAR *src, int size);

int wb702OutMP4IFrame(UINT num);
int wb702StretchVideoWin(USHORT scale_v ,USHORT scale_h);

//jpeg operation
int wb702SetJPEG(BOOL val);
int wb702SetDateTime(const struct tm *tms);
int wb702GetDateTime(struct tm *tms);

int wb702EnableSuspend(WB_SUSPEND_TYPE_E eOption);
//int wb702OldGetFWVer(short * major, short *minor);
int wb702OldCfgLCM(WB_LCM_NUMBER_E eNumber,WB_LCM_BUS_WIDTH_E eBus,WB_LCM_COLOR_WIDTH_E eColor);
int wb702OldEnableOSD(BOOL enable);
int wb702OldSetOSDKeyMask(UCHAR ucR, UCHAR ucG, UCHAR ucB);
int wb702OldSndOSDPattern(UCHAR * cpucBuffer,UINT32 uBufferLength, UINT32 uAddress);
int wb702OldOSDFlush(void);

void wb702EnableFMI(BOOL bEnable);

int wb702SetVideoFramerate(UINT framerate);
int wb702GetVideoFramerate(UINT * framerate);

int wb702SetJPEGQuality(UINT quality);
int wb702GetJPEGQuality(UINT * quality);

int wb702SetVideoDynamicBitrate(UINT32 bitrate);
int wb702SetVideoDynamicFramerate(UINT32 framerate);

BOOL wb702JPEG2RGB565(UCHAR *pcSrcImage, UCHAR *pcDesImage, int iSrcLength, int iWidth, int iHeight);

int wb702SetAudioPlayVolume(int lvolume, int rvolume); //0~31
int wb702SetAudioRecordVolume(int lvolume, int rvolume); //0~31
int wb702SetAudioNotificationVolume(int volume); //2~60
int wb702SetVideoContrastBright(int contrast, int bright);	//0~6

int wb702EnableDrawImageTime(BOOL bEnable);
int wb702SetDrawContent(int index, char *pcContent, int x, int y);
int wb702ClearDrawContent(int index);

void mp4_check_index_init(MP4_CHECK_INDEX *pmp4checkindex, char *framebuf, int framebuflen,
							BOOL bcontinuecheck,MP4_CHECK_INDEX_LEVEL checklevel, char *funcname);
void mp4_check_index(MP4_CHECK_INDEX *pmp4checkindex);
BOOL mp4_check_index_adddata(char *framebuf, int framebuflen, MP4_CHECK_INDEX *pmp4checkindex);


void iothread_EventRead(
	VP_BUFFER_MP4_BITSTREAM_T			**ppMP4Buf,
	VP_BUFFER_JPEG_ENCODER_T			**ppJPEGBuf,
	VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T	**ppAudioBuf,
	int									*pMotionCount
	);
void iothread_EventInterrupt(void);
void iothread_EventNotify(void);
void iothread_EventLock(void);
void iothread_EventUnlock(void);


#endif
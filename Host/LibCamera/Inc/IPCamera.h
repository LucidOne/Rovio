#ifndef IPCAMERA_H
#define IPCAMERA_H
#define WLAN

#include "stdio.h"
#include "stdlib.h"
#include "Cdefine.h"
#include "string.h"
#include "stddef.h"
#include "wb_syslib_addon.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "errno.h"
#include "C_list.h"
#include "wbtypes.h"
#include "sys/types.h"
#include "time.h"
#include "cyg/kernel/kapi.h"
#ifdef WLAN
#include "network.h"
#include "pkgconf/libc_startup.h"
#include "netinet/if_ether.h"
#include "cyg/io/eth/netdev.h"
#include "net/wireless.h"
#include "stdarg.h"
#include "assert.h"
#include "net/if.h"
#endif
#include "cyg/io/ppot.h"

#include "HttpServer.h"
#include "globals.h"
//#include "wbfat.h"
#include "wb_fmi.h"
#include "tt_thread.h"
#include "librtspserver.h"
#include "cyg/infra/diag.h"
#ifndef WLAN
#include "RemoteFunc.h"
#include "RemoteNet.h"
#endif

#include "cmp_img.h"
#include "Msg.h"
#include "SSelect.h"

#include "lib_videophone.h"

#include "Commands_VideoPhone.h"
#include "Commands_CameraModule.h"
#include "fontbypass.h"
#include "vp_usb.h"
#include "wbspi.h"
#include "vp_bitrate_control.h"
#include "flash_file.h"
#include "upnp.h"

#ifdef WLAN
#undef ASSERT
#include "wlan_wext.h"
#endif




#define MAX_USERSET_ENABLE	30
#define MAX_PPP_SERVERS		10

typedef struct
{
	int iLeft;
	int iTop;
	int iRight;
	int iBottom;
} RECT_T;

typedef struct
{
	char * AssignBuffer;
	unsigned int AssignLen;
}UsedBuf;

#ifndef __FILE_BUF_T_DEFINED__
#define __FILE_BUF_T_DEFINED__
typedef struct
{
	char *pcFileName;
	char *pcFilePtr;
	int iFileLen;
	struct stat st;
} FILE_BUF_T;
#endif


#define DDNS_TYPE_UNSUPPORTED 0
#define DDNS_TYPE_DYNDNS 1
#define DDNS_TYPE_NOIP 2
#define DDNS_TYPE_DNSOMATIC 3


typedef struct
{
	unsigned char ucFailTime;

	/* Http Server 参数 */
	unsigned short usHttpPort[2];

	/* Wlan 参数 */
	char acWlanESSID[33];
	unsigned char ucWlanOperationMode;
	char acWlanKey[64];
	
	
	unsigned long ulWlanChannel;
	unsigned char acWepAsc[14];
	unsigned char acWep64[5];
	unsigned char acWep128[13];
	unsigned char bWep64UseAsc;
	unsigned char bWep128UseAsc;
	unsigned char ucWepSet;
	unsigned long ulWepAuthentication;
	
	/* IP Assigned 参数 */
	char acCameraName[19];
	unsigned long aulAsDNS[3];
	BOOL abAsNetEnable[2];
	BOOL abUseHardwareMac[2];
	unsigned char acMAC[2][6];
	unsigned char aucIPAssignedWay[2];
	unsigned long ulAsIPAddress[2];
	unsigned long ulAsNetmask[2];
	unsigned long ulAsGateway[2];

	/* PPPoE 参数 */
	BOOL bEnablePPPoE;
	BOOL bPPPoEConnectOnBoot;
	char acPPPoEUserName[64];
	char acPPPoEUserPass[64];
	char acPPPoESeviceName[64];
	unsigned long ulPPPoEDisConnectTime_InSec;

	/* Modem 参数 */
	BOOL bEnableModem;
	BOOL bEnableModemAutoConnect;
	char acModemUserName[64];
	char acModemUserPass[64];
	char acModemPhone[64];
	unsigned long ulModemDisConnectTime_InSec;

	/* 拨上号后的Mail配置 */
	BOOL bIfMailOnDialed;
	char acDial_MailSender[64];
	char acDial_MailReceiver[256];
	char acDial_MailServer[64];
	char acDial_MailUser[64];
	char acDial_MailPassword[64];
	char acDial_MailSubject[64];
	BOOL bDial_MailCheck;

	/* Ftp 参数 */
	char acFtpServer[64];
	char acFtpUser[64];
	char acFtpPass[64];
	char acFtpAccount[64];
	char acFtpUploadPath[256];
	BOOL bFtpEnable;

	/* Mail 参数 */
	char acMailSender[64];
	char acMailReceiver[256];
	char acMailServer[64];
	unsigned short usMailPort;
	char acMailUser[64];
	char acMailPassword[64];
	char acMailSubject[64];
	char acMailBody[512];
	BOOL bMailCheck;
	BOOL bMailEnable;

	/* Monite 参数 */
	RECT_T rMoniteRect;

	/* 用户管理参数 */
	BOOL bUserCheck;	//是否需要检查用户
	int iUserCurNum;	//当前数据库中一共记有多少个用户信息(<MAX_USERSET_ENABLE)
	char aacUserName[MAX_USERSET_ENABLE][24];	//注：用户名和密码长度限制于24
	char aacUserPass[MAX_USERSET_ENABLE][24];	//密码
	char acUserPrivilege[MAX_USERSET_ENABLE];
	char acAdminName[24];	//作为缺省管理员的用户名
	char acAdminPass[24];	//作为缺省管理员的用户密码,未加密

	/* 相机设置 */
	BOOL bDefaultImg;
	CMD_VIDEO_FORMAT_E eVideoFormat;
	CMD_AUDIO_FORMAT_E eAudioFormat;
	unsigned long ulImgDirection;
	unsigned long ulImgResolution;
	unsigned long ulImgQuality;
	unsigned long ulImgFramerate;
	unsigned long ulImgBrightness;
	unsigned long ulImgContrast;
	unsigned long ulImgHue;
	unsigned long ulImgSaturation;
	unsigned long ulImgSharpness;
	unsigned long ulMotionDetectSensitivity;
	unsigned char ucMotionDetectWay;
	
	unsigned long ulSpeakerVolume;
	unsigned long ulMicVolume;

	/* 时间设置 */
	char acNtpServer[64];
	BOOL bUseNtp;
	long lTimeZone_InSec;
	char ShowString[2][128];
	int ShowPos[2];	//0-top left,1-top right,2-bottom left,3-bottom right,4-middle
	int ShowTime;	//0-do not show,1-show time,2-show self string	
	

	/* Dynamic DNS */
	BOOL bEnableDDNS;
	int eDDNSType;
	char acDDNSUserName[24];
	char acDDNSUserPass[24];
	char acDDNSDomainName[32];
	UINT32 ulDDNSIP;
	char acProxyUser[24];
	char acProxyPass[24];
	char acProxy[32];
	int iProxyPort;

	/* Bypass server */
	BOOL bEnableBPS;
	char acBPSDomainName[32];
	//char acBPSDescription[40];
	int aiBPSLocalPort[6];
	char acBPSType[6];
	int iBPSPort;
	int iBPSChannelsNum;
	unsigned uDelayTime_InMSec;
	
	/* PPP server */
	BOOL bPPPEnable;
	char aacPPPServer[MAX_PPP_SERVERS][64];
	int aiPPPPort[MAX_PPP_SERVERS];
	char acPPPUser[24];
	char acPPPPass[24];
	
	
	/* UPnP */
	BOOL bEnableUPnP;
	unsigned short usUPnPPort;

	/* 校验位 */
	unsigned long ulCheckSum;
} CAMERA_CONFIG_PARAM_T;



#define MAX_CAMERA_IMG_LENGTH (128*1024)
//#define MAX_CAMERA_MP4_LENGTH (64*1024)

#define MOTION_DETECT_NONE		'\0'
#define MOTION_DETECT_SOFT		'\1'
#define MOTION_DETECT_HARD		'\2'

#define IP_ASSIGNMENT_NET_LAN '\0'
#define IP_ASSIGNMENT_NET_WLAN '\1'
#define IP_ASSIGNMENT_NET_LAN_AND_WLAN '\2'
#define IP_ASSIGNMENT_MANUALLY	'\0'
#define IP_ASSIGNMENT_DHCP	'\1'

#define WEP_SET_DISABLE '\0'
#define WEP_SET_K64 '\1'
#define WEP_SET_K128 '\2'
#define WEP_SET_K152 '\3'
#define WEP_SET_ASC '\4'

#define CAMERA_OFF 0
#define CAMERA_ON 1
#define ST_MODEM_OFF 0
#define ST_MODEM_ON 1
#define ST_MODEM_TRING 2
#define ST_PPPOE_OFF 0
#define ST_PPPOE_ON 1
#define ST_PPPOE_TRING 2
#define INVALID_PICTURE 999999


typedef struct
{
	int iFlashUpdateState;
	int iFlashWriteSize;
	int iFlashWriteTotalSize;
	int iLinuxSize;
	int iRomimgSize;
} FLASH_UPDATE_T;
#define FIRMWARE_STOP 0
#define FIRMWARE_UPLOADING 1
#define FIRMWARE_FLASHING 2
#define FIRMWARE_FORMAT_ERROR 3

#define DDNS_NO_UPDATE 0
#define DDNS_UPDATTING 1
#define DDNS_UPDATE_OK 2
#define DDNS_UPDATE_FAIL 3
#define DDNS_CHECK_IP_OK 4

typedef struct
{
	long lMsg;
	long lData;
} MSG_CAMERA_T;


typedef struct
{
	unsigned char ucCamera;
	unsigned char ucModem;
	unsigned char ucPPPoE;
	unsigned char ucX_Direction;
	unsigned char ucY_Direction;
	CMD_ROTATE_E eRotate;	//should match ucX_Direction, ucY_Direction
	unsigned char ucBright;
	unsigned char ucContrast;
	unsigned char ucResolution;
	unsigned char ucSaturation;
	unsigned char ucHue;
	unsigned char ucSharpness;
	unsigned char ucCompressionRatio;
	unsigned char ucFramerate;
	unsigned char ucSpeakerVolume;
	unsigned char ucMicVolume;
	
	unsigned char ucFtp;
	unsigned char ucEmail;
	unsigned char ucPrivilege;
	unsigned char ucUserCheck;
	unsigned char ucMotionDetectWay;
	unsigned short usFocus;
	unsigned int uiPicIndex; // 0 - INVALID_PICTURE
	unsigned int uiPicMotionDetectedIndex;	// 0 - INVALID_PICTURE
	unsigned int uiPicSize;
	RECT_T rMoniteRect;

	int iDDNSState;
	int iReadyImg;
	int iImgBufLen[2];
	char *acImgBuf[2];
	
	time_t tAsfStartTime;
	BOOL bAsfMP4Header;
	char acMP4Header[32];
	int iMP4HeaderLen;
	BOOL bShowTime;
	unsigned char WiFiStrength;
	unsigned char BatteryLevel;
	
	BOOL bDHCP_Finished;
	int nHTTPClientNum;
} WEBCAMSTATE_P;



typedef struct
{
	int dummy;
} W99702_DATA_EXCHANGE_T;


#define CA_CONFIG 0
#define CA_AUTH 1

#define MSG_NONE -1
#define MSG_CUSTOM_PROC 0L
#define MSG_CAMERA_OPEN 1L
#define MSG_CAMERA_CLOSE 2L
#define MSG_CAMERA_RES 3L
#define MSG_CAMERA_QUALITY 4L
#define MSG_CAMERA_MONITOR_RECT 5L
#define MSG_QUIT 6L
#define MSG_EXTERNEL_DETECTOR 7L
#define MSG_MAIL 8L
#define MSG_FTP_UPLOAD 9L
#define MSG_CAMERA_BRIGHTNESS 10L
#define MSG_CAMERA_CONTRAST 11L
#define MSG_CAMERA_SATURATION 12L
#define MSG_CAMERA_HUE 13L
#define MSG_CAMERA_SHARPNESS 14L
#define MSG_FLASH_CONFIG_WRITE 15L
#define MSG_FLASH_FIRMWARE_UPDATE 16L
#define MSG_NTP_GETTIME 17L
#define MSG_ADD_FD 18L
#define MSG_CAMERA_MOTION 19L
#define MSG_CAMERA_FRAMERATE 20L
#define MSG_CAMERA_DIRECTION 21L
#define MSG_SPEAKER_VOLUME 22L
#define MSG_MIC_VOLUME 23L
#define MSG_SET_LOGO 24L

#define MAX_POST_LENGTH (10*1024)
//#define FLASH_OFFSET_FIRMWARE 0x00020000
//#define FLASH_OFFSET_CONFINFO 0x7F1f8000
#define FLASH_CONFIG_SIZE	((sizeof (CAMERA_CONFIG_PARAM_T) + 255) / 256 * 256)
#define FIRMWARE_UPDATE_LIMIT_SIZE ((1024+512)*1024)
#define FILE_UPLOAD_LIMIT_SIZE (2*1024*1024)

#ifdef FILESYSTEM
#define FILE_UPLOAD_DEFAULT_DIR "Config"
#define FACTORY_DEFAULT_FILE L"D:\\WCConfig.ini"
#define FLASH_CONFIG_FILE  L"D:\\WCConfig"
#define MY_PID_LOG L"D:\\httpd_id"
#define FLASH_UPDATE_TMP_FILE L"D:\\Firmware.bin"
#define DEBUG_CONFIG_FILE L"D:\\WCConfigDebug.ini"
#define PPPOE_CONF_FILE L"D:\\pppd_info"
#define MODEM_CONF_FILE L"D:\\modem_info"
#else
//#define FLASH_TOTAL_SIZE (1024*1024)
//#define FACTORY_DEFAULT_FILE (1019*1024)
#define FLASH_TOTAL_SIZE (usiMyGetTotalSize())
#endif

#define MAX_IMG_SAVE 1
#define MAX_IMG_INDEX 800000
#define MAX_FTP_LIST_LENGTH 10
#define MAX_MAIL_LIST_LENGTH 10
#define MAX_IMAGE_IN_ON_MAIL 10
#define MAX_LOG_ITEM 500
#define PTD_PRIORITY   10
#define STACKSIZE  (1024*32)
#define STACKSIZE2 (1024*16)
#define System_BUFFER_LEN 1024

extern CAMERA_CONFIG_PARAM_T g_ConfigParam;	/* config info that save to flash */
extern WEBCAMSTATE_P g_WebCamState;	/* current running state */

extern TT_RMUTEX_T g_rmutex;
extern cyg_mutex_t g_ptmConfigParam;	/* mutex for g_ConfigParam */
extern cyg_mutex_t g_ptmState;	/* mutex for running state */
extern cyg_mutex_t g_ptmTimeSetting;
extern cyg_mutex_t g_ptmWebCameraLog; /* mutex for web camera log */

extern const char *g_cNoCacheHeader;
extern const char *g_apcNetworkInterface[2];
extern const char *g_apcWlanOperationMode[6];
extern cyg_thread g_ptdMctest;
extern cyg_handle_t ptdMctest_handle;
extern unsigned char ptdMctest_stack[STACKSIZE2];

extern cyg_thread g_ptdCamera;	/* pthread_t for CameraThread */
extern MSG_QUEUE *g_pMsgCamera;	/* message queue for CameraThread */
extern cyg_handle_t ptdCamera_handle;
extern unsigned char ptdCamera_stack[STACKSIZE];
#ifdef USE_DDNS
extern cyg_thread g_ptdSelect;	/* pthread_t for SelectThread */
extern MSG_QUEUE *g_pMsgSelect;	/* message queue for SelectThread */
extern cyg_handle_t ptdSelect_handle;
extern unsigned char ptdSelect_stack[STACKSIZE];
#endif
extern FLASH_UPDATE_T g_FlashUpdateState;	/* current flash update state */
#ifdef USE_FLASH
extern cyg_thread g_ptdFlash;	/* pthread_t for FlashThread */
extern MSG_QUEUE *g_pMsgFlash;	/* message queue for FlashThread */
extern cyg_handle_t ptdFlash_handle;
extern unsigned char ptdFlash_stack[STACKSIZE];
#endif
extern BOOL (*g_pOnGetImage)(char *pcImgBuffer, int iImgLength);
extern BOOL (*g_pExternalDetector)(char *pcImgBuffer, int iImgLength);

extern BOOL USI_IS_READY;

extern long g_lTimeDelay1970_InSec;
extern long g_lTimeDelayZone_InSec;

extern time_t tTimeServerStart;

#ifndef WLAN
extern CHAR *g_StreamServer_Buf;
#define SELECT_BUFFER_LEN 1024
extern CHAR g_Select_Buf[SELECT_BUFFER_LEN];
#define MCTEST_BUFFER_LEN 1024
extern CHAR g_Mctest_Buf[MCTEST_BUFFER_LEN];
#endif
#endif
#ifndef IMAGECONFIG_H
#define IMAGECONFIG_H
#include "wbtypes.h"

#define ICTL_OK			0
#define ICTL_ERROR		(-1)
#define ICTL_UNAUTHORIZED	(-2)
#define ICTL_INVALID_PARAMETERS	(-3)

typedef struct {
    char Username[24];
    char Passwd[24];
    int Privilege;
}ICTL_HANDLE_T;


typedef struct RES_TYPE
{
    int width;
    int length;    
}RES_TYPE;



/*Struct used to save scaned ap*/
typedef struct _ApScanResult
{
	char mac[14];
	char ssid[33];
	int mode;
	int freq; 
	int signal;
	int noise;
	int encode;
	int connect;
}ApScanResult;


int ictlGetMyself( ICTL_HANDLE_T *pHandle, char *ppcUserName, int *pnPrivilege );
int ictlSetUser( ICTL_HANDLE_T *pHandle, const char *pcUserName, const char *pcPassword, int nPrivilege );
int ictlDelUser( ICTL_HANDLE_T *pHandle, const char *pcUserName );
/*pcUserName 存放用户名的二位数组指针，userlen是每一个用户名最大长度，
pnPrivilege是存放用户权限的指针数组，maxnum是数组元素的个数(MAX_USERSET_ENABLE = 30)
usernum是系统中用户个数*/
int ictlGetUser( ICTL_HANDLE_T *pHandle, char *pcUserName,int userlen, int *pnPrivilege, int maxnum,int *usernum );
int ictlSetUserCheck( ICTL_HANDLE_T *pHandle, BOOL bEnable );
int ictlGetUserCheck( ICTL_HANDLE_T *pHandle, BOOL *pbIsEnabled );
int ictlGetCurrentStatusCode( ICTL_HANDLE_T *pHandle, char *pcStatus, size_t *szStatusLength, size_t szMaxStatusLength );
int ictlAuthUser( ICTL_HANDLE_T *pHandle, const char *pcUserName, const char *pcPassword);

int ictlSetResolution( ICTL_HANDLE_T *pHandle, int pSize );
int ictlGetResolution( ICTL_HANDLE_T *pHandle, int *pSize );
int ictlGetQualityCapability( ICTL_HANDLE_T *pHandle, int *pnLowLevel, int *pnHighLevel );
int ictlSetQuality( ICTL_HANDLE_T *pHandle, int nQualityLevel );
int ictlGetQuality( ICTL_HANDLE_T *pHandle, int *pnQualityLevel );
int ictlGetFramerateCapalibity( ICTL_HANDLE_T *pHandle, int *pnMaxFramerate, int *pnMinFramerate );
int ictlSetFramerate( ICTL_HANDLE_T *pHandle, int nFramerate );
int ictlGetFraemrate( ICTL_HANDLE_T *pHandle, int *pnFramerate );
int ictlSetBrightness(ICTL_HANDLE_T *pHandle, int nBright );
int ictlGetBrightness( ICTL_HANDLE_T *pHandle, int *pnBrightness );
int ictlSetSpeakerVolume(ICTL_HANDLE_T *pHandle, int nVolume );
int ictlGetSpeakerVolume( ICTL_HANDLE_T *pHandle, int *pnVolume );
int ictlSetMicVolume(ICTL_HANDLE_T *pHandle, int nVolume );
int ictlGetMicVolume( ICTL_HANDLE_T *pHandle, int *pnVolume );

int ictlSetMediaFormat( ICTL_HANDLE_T *pHandle, CMD_AUDIO_FORMAT_E eAudioFormat, CMD_VIDEO_FORMAT_E eVideoFormat );
int ictlGetMediaFoamat( ICTL_HANDLE_T *pHandle, CMD_AUDIO_FORMAT_E *peAudioFormat, CMD_VIDEO_FORMAT_E *peVideoFormat );
int ictlGetCurrentJPEG( ICTL_HANDLE_T *pHandle, void *pBuffer, size_t szMaxBufferLength,size_t *jpeglen );
int ictlIsMotionDetectEnabled( ICTL_HANDLE_T *pHandle,	BOOL *pbIsEnabled,
	BOOL *pbIsFtpUploadEnabled,
	BOOL *pbIsMailUploadEnabled,
	BOOL *pbIsDiskRecordEnabled);
int ictlEnableMotionDetect( ICTL_HANDLE_T *pHandle,	BOOL bEnable,BOOL bIsFtpUploadEnabled,BOOL bIsMailUploadEnabled,BOOL bIsDiskRecordEnabled);
/*pcWlanOperationMode (ad-hoc或者infrastructure),pcwepset加密方式(K64,K128,ASC),*/
int ictlSetWlan(ICTL_HANDLE_T *pHandle,
				const char *pcWlanKey,
				const char *pcWlanOperationMode,unsigned long ulWlanChannel,
				const char *pcWepSet,const char *pcWepAsc,const char *pcWep64type,const char *pcWep128type,
				const char *pcWep64,const char *pcWep128,const char *pcWlanESSID);

int ictlGetWlan(ICTL_HANDLE_T *pHandle,
				char *pcWlanKey,
				char *pcWlanOperationMode,unsigned long *ulWlanChannel,
				char *pcWepSet,char *pcWepAsc,
				char *pcWep64type,char *pcWep128type,
				char *pcWep64,char *apcWep128,char *pcWlanESSID);	
int ictlGetIP(ICTL_HANDLE_T *pHandle,char *way,unsigned long *ipadd,unsigned long *gw,unsigned long *netmask,unsigned long *dns,int maxnum,int *num);
int ictlSetIP(ICTL_HANDLE_T *pHandle,const char *way,long ipadd,long gw, long netmask,long *dns,int num);

int ictlReboot( ICTL_HANDLE_T *pHandle);
int ictlSetFactoryDefault(ICTL_HANDLE_T *pHandle);
int ictlUpdateFactoryDefault(ICTL_HANDLE_T *pHandle);

int ictlScanAP( ICTL_HANDLE_T *pHandle, ApScanResult *ap_prop, int * ap_num, int ap_maxnum);

int ictlCtrlMCU(ICTL_HANDLE_T *pHandle,
	const char *pcCommand,
	char *pcResponse,
	size_t szMaxResponse);
						
#if 0
int ictlReadStatusCode( const char *pcStatus, CFG_STATUS *pStatus );
int ictlWriteStatusCode( char *pcStatus, const CFG_STATUS *pStatus );
int ictlSetFtp( ICTL_HANLDE_T *pHandle, BOOL bEnable, .... );
int ictlGetFtp( ICTL_HANLDE_T *pHandle, BOOL *pbIsEnabled, .... );
int ictlSetMail( ICTL_HANLDE_T *pHandle, BOOL bEnable, .... );
int ictlGetMail( ICTL_HANLDE_T *pHandle, BOOL *pbIsEnabled, .... );
int ictlSetRecordPath( ICTL_HANLDE_T *pHandle, BOOL bEnable, WCHAR_T *pwcPath );
int ictlGetRecordPath( ICTL_HANLDE_T *pHandle, BOOL *pbIsEnabled, WCHAR_T **pwcPath );
#endif
int ictlSetMac( ICTL_HANDLE_T *pHandle, const char *Mac );
int ictlGetMac( ICTL_HANDLE_T *pHandle, char *Mac,int MacSize );
int ictlSetDefaultMac( ICTL_HANDLE_T *pHandle, const char *Mac );
#endif
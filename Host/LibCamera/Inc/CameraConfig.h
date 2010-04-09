#ifndef CAMERACONFIG_H
#define CAMERACONFIG_H

/* Calculate the checksum of struct CAMERA_CONFIG_PARAM_T. */
unsigned long GetConfigCheckSum(CAMERA_CONFIG_PARAM_T *pcConfigParam);

/* Read "Web Camera" config value from flash. */
BOOL ReadFlashMemory(CAMERA_CONFIG_PARAM_T *pcConfigParam);

/* Write "Web Camera" config value to falsh. */
BOOL WriteFlashMemory(CAMERA_CONFIG_PARAM_T *pcConfigParam);
BOOL __WriteFlashMemory(CAMERA_CONFIG_PARAM_T *pcConfigParam, BOOL bResetCheckSum, BOOL bOnDefaultArea);

/* Get the flash capability, total size, free space, etc */
void GetFlashCapability(UINT32 *puTotalSize, UINT32 *puFreeBegin, UINT32 *puFreeEnd);

/* Get factory-default config value of "Web Camera". */
BOOL ReadFactoryDefault(CAMERA_CONFIG_PARAM_T *pcConfigParam);
BOOL WriteFactoryDefault(CAMERA_CONFIG_PARAM_T *pcConfigParam);

#ifdef FILESYSTEM
BOOL ReadCameraINI(char *pcFileName, CAMERA_CONFIG_PARAM_T *pcConfigParam);

BOOL WriteCameraINI(char *pcFileName, CAMERA_CONFIG_PARAM_T *pcConfigParam);

/* For the browser to display current config value.
** See Main.c -- RegisterEmbedFun("GetConfig.cgi", Http_GetConfig, NULL); */
int Http_GetConfig(HTTPCONNECTION hConnection, void *pParam);
#endif

/* do with the http request "SetFactoryDefault.cgi" */
void InitDefaultParam(CAMERA_CONFIG_PARAM_T *pcConfigParam);
//int Http_SetFactoryDefault(HTTPCONNECTION hConnection, void *pParam);
int Http_SetFactoryDefault(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
int Http_UpdateFactoryDefault(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);


#endif

#ifndef __W99702_CAMERA_H__
#define __W99702_CAMERA_H__

#ifdef USE_W99702_CAMERA

BOOL W99702_OpenCamera(W99702_DATA_EXCHANGE_T* pExData, UsedBuf *asbuf,unsigned int *aslen);
BOOL W99702_CloseCamera(W99702_DATA_EXCHANGE_T* pExData);
BOOL W99702_GetOneImage(char *pcImgBuf, int iImgBufMaxLen, int *piImgBufLen,W99702_DATA_EXCHANGE_T* ExData,BOOL RecordInit);
BOOL W99702_SetImageResolution( int iResX, int iResY,W99702_DATA_EXCHANGE_T* pExData);
BOOL W99702_GetImageResolution(int *piResX, int *piResY);
BOOL W99702_SetImageQuality( int iQuality,W99702_DATA_EXCHANGE_T* pExData);
BOOL W99702_GetImageQuality( int *piQuality);
BOOL W99702_SetFramerate(int iFramerate,W99702_DATA_EXCHANGE_T* pExData);
BOOL W99702_GetFramerate(int *piFramerate);
BOOL W99702_SetImageBrightness( int iBright,W99702_DATA_EXCHANGE_T* pExData);
BOOL W99702_SetSpeakerVolume( int iVolume,W99702_DATA_EXCHANGE_T* pExData);
BOOL W99702_SetMicVolume( int iVolume,W99702_DATA_EXCHANGE_T* pExData);
#endif



#endif


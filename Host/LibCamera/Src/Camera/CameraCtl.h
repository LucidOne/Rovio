#ifndef CAMERACTL_H
#define CAMERACTL_H

/* pthread for camera */
void CameraThread(cyg_addrword_t pParam);

/* do with the http request "/SetCamera.cgi" */
int Config_SetCamera(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

/* do with the http request "/GetCamera.cgi" */
int Config_GetCamera(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

/* do with the http request "/ChangeDirection.cgi" */
int Config_ChDirection(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

/* do with the http request "/ChangeResolution.cgi" */
int Config_ChResolution(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

/* do with the http request "/ChangeCompressRatio.cgi" */
int Config_ChCompressRatio(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

/* do with the http request "/ChangeFramerate.cgi" */
int Config_ChFramerate(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

/* do with the http request "/ChangeBrightness.cgi" */
int Config_ChBrightness(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
int Config_ChSpeakerVolume(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
int Config_ChMicVolume(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);


/* do with the http request "/SetMotionDetect.cgi" */
int Config_SetMotionDetect(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

/* do with the http request "/GetMotionDetect.cgi" */
int Config_GetMotionDetect(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

/* do with the http request "/SetMediaFormat.cgi" */
int Config_SetMediaFormat(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

/* do with the http request "/GetMediaFormat.cgi" */
int Config_GetMediaFormat(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

#ifdef USE_SERVER_PUSH
/* send continue camera data */
int Http_GetCameraData(HTTPCONNECTION hConnection, void *pParam);
/* drop the connection of sending continue camera data */
int Config_DropData(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
#endif
int Http_SendJpeg(HTTPCONNECTION hConnection, void *pParam);

#endif

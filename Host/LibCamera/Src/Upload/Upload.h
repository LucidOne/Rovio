#ifndef UPLOAD_H
#define UPLOAD_H

/* Set actions after got a interested POST request. */
int Http_Post_Init(HTTPCONNECTION hConnection, void *pParam);

/* Callback function when upload data received. */
int Http_Post_Upload(HTTPCONNECTION hConnection,
								int *piPostState,
								char **ppcPostBuf,
								int *piPostBufLen,
								int *piPostDataLen,
								char *pcFillData,
								int iFillDataLen,
								int iIsMoreData/*bool*/,
								void *pParam/*other parameter for extend use*/);

/* Callback function for upload firmware. */
int Http_Post_FMUpload(HTTPCONNECTION hConnection,
								int *piPostState,
								char **ppcPostBuf,
								int *piPostBufLen,
								int *piPostDataLen,
								char *pcFillData,
								int iFillDataLen,
								int iIsMoreData/*bool*/,
								void *pParam/*other parameter for extend use*/);

/* for /FMFlash.cgi */
int Config_FMFlash(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

/* for /GetUpdateProgress.cgi */
int Config_UpdateProgress(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

/* download the firmware */
int Http_GetFirmware(HTTPCONNECTION hConnection, void *pParam);
#endif

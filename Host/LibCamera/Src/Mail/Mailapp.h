//#ifdef RECORDER
#ifndef MAILAPP_H
#define MAILAPP_H

int Config_SendMail(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
int Config_SetMail(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
int Config_GetMail(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

BOOL EncodeBase64(const char *pccInStr, int iInLen, char *pccOutStr, int iMaxOutSize, int *piOutLen);
#endif
//#endif
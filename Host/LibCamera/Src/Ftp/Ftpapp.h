#ifndef RECORDER
#ifndef FTPAPP_H
#define FTPAPP_H

/* do with the http connection "SetFtp.cgi" */
int Config_SetFtp(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

/* do with the http connection "GetFtp.cgi" */
int Config_GetFtp(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
#endif
#endif

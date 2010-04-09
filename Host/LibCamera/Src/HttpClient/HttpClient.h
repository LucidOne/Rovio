#ifdef USE_DDNS
#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H
void DDNS_Update(void);
void DDNS_CheckIP(void);
void prdAddTask__CheckDDNS(void);

int Config_GetDDNS(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
int Config_SetDDNS(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
#endif


int Http_SendHttpRequest(HTTPCONNECTION hConnection, void *pParam);

#endif

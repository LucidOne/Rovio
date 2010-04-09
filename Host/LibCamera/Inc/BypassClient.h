#ifndef INC__BYPASS_CLIENT_H__
#define INC__BYPASS_CLIENT_H__


/* Default server: rovio.gostai.com:21846 */
void BPC_New (const char *pcServer, int iServerPort, int iLocalPort);
int Config_GetVNet(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
int Config_SetVNet(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

#endif

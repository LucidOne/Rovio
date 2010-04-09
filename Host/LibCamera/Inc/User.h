#ifndef USER_H
#define USER_H
int GetPrivilege(HTTPCONNECTION hConnection);
/* do with the http request "GetMyself.cgi" */
int Config_GetMyself(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

/* do with the http request "SetUser.cgi" */
int Config_SetUser(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

/* do with the http request "DelUser.cgi" */
int Config_DelUser(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

/* do with the http request "GetUser.cgi" */
int Config_GetUser(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

/* do with the http request "SetUserCheck.cgi" */
int Config_SetUserCheck(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
#endif

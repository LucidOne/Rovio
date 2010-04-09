#ifndef REGISTER_H
#define REGISTER_H


/* return 0 for set, 1 for get */
typedef int (*SUBCMD_PFUN)(HTTPCONNECTION hConnection, LIST *pList, int iAction, XML *pReturnXML);
typedef struct
{
	HTTPCONNECTION hConnection;
	LIST *pParamList;
	XML *pReturnXML;
	SUBCMD_PFUN funSub;
	BOOL bWaitData;
} SUBCMD_RUN_STRUCT;
//typedef int (*CMD685_PFUN)(SUBCMD_RUN_STRUCT *pSubCmd);

typedef int (*GETPOST_CALLER_PFUN)(HTTPCONNECTION hConnection, char *pcCmdParam, int iCmdParamLen, void *pParam);

XML *ReturnTypeBegin(HTTPCONNECTION hConnection, LIST *pQueryList);
void ReturnTypeEnd(HTTPCONNECTION hConnection, LIST *QueryList, XML *pReturnType);
void AddHttpText(XML *pReturnXML, const char *pcText);
void AddHttpValue(XML *pReturnXML, const char *pcName, const char *pcValue);
void AddHttpNum(XML *pReturnXML, const char *pcName, long lNum);
XML *AppendXMLArray(XML *pXML, const char *pcName);

int Http_SendRedirectRequest(HTTPCONNECTION hConnection, LIST *pQueryList);

int Http_ConnInit(HTTPCONNECTION hConnection, void *pParam, GETPOST_CALLER_PFUN funCaller);
int Http_CommonCmdInit(HTTPCONNECTION hConnection, void *pParam);
int Http_SubCmdInit(HTTPCONNECTION hConnection, void *pParam);

void GetCheckedTime(long *plTime1970_InSec, long *plTimeZone_InSec);
int Config_SetTime(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
int Config_GetTime(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
int Config_SetLogo(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
int Config_GetLogo(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
void SetCheckedTime(const long *plTime1970_InSec, const long *plTimeZone_InSec);

long GetSystemTimeZone(void);
#endif

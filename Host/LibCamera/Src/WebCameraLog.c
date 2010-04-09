
#include "../Inc/CommonDef.h"

typedef struct
{
	char ucValue[4];
	unsigned char aucOpMAC[6];
	unsigned char ucPrivilege;
	unsigned char ucType;
	unsigned long ulOpIP;
	time_t tTime;
} WEBCAMERA_LOG_T;

static time_t mytime(void)
{
    time_t cur_sec;
    cyg_tick_count_t cur_time;
    cur_time = cyg_current_time();
    cur_sec = (cur_time*10) / 1000;
    return cur_sec;
}
static WEBCAMERA_LOG_T g_LogTable[MAX_LOG_ITEM];
static int g_iLogItemEnd = 0;
static int g_iLogItemStart = 0;

void WebCameraLog(int iViewerPrivilege,
					int iLogType,
					const char *pcLogValue,
					HTTPCONNECTION hConnection)
{
	int iNewEnd;
	int i;
	
	cyg_mutex_lock(&g_ptmWebCameraLog);

	iNewEnd = g_iLogItemEnd + 1;
	if (iNewEnd >= MAX_LOG_ITEM) iNewEnd = 0;
	if (g_iLogItemStart == iNewEnd)
	{
		g_iLogItemStart++;
		if (g_iLogItemStart >= MAX_LOG_ITEM) g_iLogItemStart = 0;
	}

	g_LogTable[g_iLogItemEnd].ucPrivilege = (unsigned char)iViewerPrivilege;
	g_LogTable[g_iLogItemEnd].ucType = (unsigned char)iLogType;
	g_LogTable[g_iLogItemEnd].tTime = mytime() - tTimeServerStart;

	if (pcLogValue == NULL) g_LogTable[g_iLogItemEnd].ucValue[0] = '\0';
	else
	{
		for (i=0; i<4; i++)
		{//not use strncpy because i'm not sure if strncpy add '\0' at the end
			g_LogTable[g_iLogItemEnd].ucValue[i] = pcLogValue[i];
			if (pcLogValue[i] == '\0') break;
		}
	}
	if (hConnection)
	{
		g_LogTable[g_iLogItemEnd].ulOpIP = httpGetClientAddr(hConnection).s_addr;
		memcpy(g_LogTable[g_iLogItemEnd].aucOpMAC, httpGetClientMac(hConnection), 6);
	}
	else
	{
		g_LogTable[g_iLogItemEnd].ulOpIP = 0L;
		bzero(g_LogTable[g_iLogItemEnd].aucOpMAC, 6);
	}

	g_iLogItemEnd = iNewEnd;

	cyg_mutex_unlock(&g_ptmWebCameraLog);
}

int Config_GetLog(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	char acLog[64];
	int i;

	BOOL bSuperUser;

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_USER;
	case CA_CONFIG:
		if (GetPrivilege(hConnection) == 0) //超级用户
			bSuperUser = TRUE;
		else bSuperUser = FALSE;

		sprintf(acLog, "%010lu", (unsigned long)(mytime() - tTimeServerStart));
		AddHttpValue(pReturnXML, "Time", acLog);

		//if (pthread_mutex_lock(&g_ptmWebCameraLog) == 0)
		cyg_mutex_lock(&g_ptmWebCameraLog);
		for (i = g_iLogItemStart; i != g_iLogItemEnd; )
		{
				char acValue[5];
				int j;
				int iLen;

				if (bSuperUser || g_LogTable[i].ucPrivilege != 0)
				{

					for (j=0; j<4 && g_LogTable[i].ucValue[j] != '\0'; j++)
					{
						acValue[j] = g_LogTable[i].ucValue[j];
						acValue[j] = 0x7F & acValue[j];//避免超过127的字符被javascript解释错
						if (acValue[j] < ' ') acValue[j] = ' ';
					}
					while (j < 4) acValue[j++] = ' ';
					acValue[j] = '\0';
					iLen = sprintf(acLog, "%02u%s",
						(int)g_LogTable[i].ucType,
						acValue);

					for (j = 0; j < 4; j++)
						iLen += httpChar2Hex(*((char *)&g_LogTable[i].ulOpIP + j), acLog + iLen);
					for (j = 0; j < 6; j++)
						iLen += httpChar2Hex(g_LogTable[i].aucOpMAC[j], acLog + iLen);

					iLen += sprintf(acLog + iLen, "%010lu", (unsigned long)g_LogTable[i].tTime);

					AddHttpValue(pReturnXML, "Log", acLog);
				}

				i++;
				if (i >= MAX_LOG_ITEM) i = 0;
		}
		cyg_mutex_unlock(&g_ptmWebCameraLog);
		return 0;
	}
	return -1;
}

int OnRequestBegin(HTTPCONNECTION hConnection, void *pParam)
{
	unsigned ulRemoteIP = httpGetClientAddr(hConnection).s_addr;
	unsigned char *pucRemoteMac = (unsigned char *)httpGetClientMac(hConnection);
	BOOL bFound;
	int i;

	bFound = TRUE;
	cyg_mutex_lock(&g_ptmWebCameraLog);
	
	for (i = g_iLogItemStart; i != g_iLogItemEnd; )
	{
		if (g_LogTable[i].ulOpIP == ulRemoteIP
			&& memcmp(g_LogTable[i].aucOpMAC, pucRemoteMac, 6) == 0)
			break;
		i++;
		if (i >= MAX_LOG_ITEM) i = 0;
	}
	if (i == g_iLogItemEnd) 
		bFound = FALSE;
	cyg_mutex_unlock(&g_ptmWebCameraLog);
	

	if (bFound == FALSE)
	{
		WebCameraLog(CL_PRIVILEGE_ADMIN, CL_NEW_CLIENT,	NULL, hConnection);
	}
	return 1;
}

#ifdef RECORDER
#include "../../Inc/CommonDef.h"
#include "arpa/telnet.h"
#include "arpa/ftp.h"
#include "../../libFtp/Inc/libftp.h"


#define HOST_BUFFER_LEN 1024

__align (32)
CHAR g_Host_Buf[HOST_BUFFER_LEN];

int TimeoutRead(int fd, char *pcBuf, int iBufLen, int iTimeout_InMinSec)
{
	struct timeval tv;
	fd_set fdsRead;

	FD_ZERO(&fdsRead);
	FD_SET(fd, &fdsRead);
	tv.tv_sec = iTimeout_InMinSec / 1000;
	tv.tv_usec = (iTimeout_InMinSec - tv.tv_sec * 1000) * 1000;

//printf("&fdsRead:%x\n",&fdsRead);
#ifndef WLAN
	if (netselect(fd+1, &fdsRead, NULL, NULL, &tv,g_Host_Buf ,HOST_BUFFER_LEN ) >= 0)
#else
	if (select(fd+1, &fdsRead, NULL, NULL, &tv ) > 0)
#endif
	{
		if (FD_ISSET(fd, &fdsRead))
		{
			int rt;
//printf("&rt:%x\n",&rt);
#ifndef WLAN
			rt = netread(fd, pcBuf, iBufLen,g_Host_Buf ,HOST_BUFFER_LEN );
#else
			rt = read(fd, pcBuf, iBufLen);
#endif
//printf("iBufLen:%d,rt:%d\n",iBufLen,rt);
			return rt;
		}
	}
	return -1;
}

char *GetRegPath(const char *pcPath)
{
	char *pcRt;
	int iLen;
	int i, j;

	if (pcPath == NULL) pcPath = "/";
	iLen = strlen(pcPath);
	if ((pcRt = (char *)malloc(iLen + 3)) != NULL)
	{
		memcpy(pcRt, pcPath, iLen + 1);
		if (pcRt[iLen - 1] != '/')
		{
			pcRt[iLen++] = '/';
			pcRt[iLen] = '\0';
		}
	}
	else
	{
		PRINT_MEM_OUT;
		return NULL;
	}
	for (i=0, j=0; pcRt[i] != '\0'; i++)
	{
		if (pcRt[i] == '/' || pcRt[i] == '\\')
		{
			if (j==0) pcRt[j++] = '/';
			else if (pcRt[j-1] != '/') pcRt[j++] = '/';
		}
		else pcRt[j++] = pcRt[i];
	}
	pcRt[j] = '\0';

	return pcRt;
}


int Config_SetFtp(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
		break;
	case CA_CONFIG:
		cyg_mutex_lock(&g_ptmConfigParam);
		if (httpIsExistParam(pParamList, "FtpServer"))
			httpMyStrncpy(g_ConfigParam.acFtpServer, httpGetString(pParamList, "FtpServer"), sizeof(g_ConfigParam.acFtpServer));
		if (httpIsExistParam(pParamList, "User"))
			httpMyStrncpy(g_ConfigParam.acFtpUser, httpGetString(pParamList, "User"), sizeof(g_ConfigParam.acFtpUser));
		if (httpIsExistParam(pParamList, "Pass"))
			httpMyStrncpy(g_ConfigParam.acFtpPass, httpGetString(pParamList, "Pass"), sizeof(g_ConfigParam.acFtpPass));
		if (httpIsExistParam(pParamList, "Account"))
			httpMyStrncpy(g_ConfigParam.acFtpAccount, httpGetString(pParamList, "Account"), sizeof(g_ConfigParam.acFtpAccount));
		if (httpIsExistParam(pParamList, "UploadPath"))
		{
			char *pcPath;
			pcPath = GetRegPath(httpGetString(pParamList, "UploadPath"));
			if (pcPath == NULL) g_ConfigParam.acFtpUploadPath[0] = '\0';
			else
			{
				httpMyStrncpy(g_ConfigParam.acFtpUploadPath, pcPath, sizeof(g_ConfigParam.acFtpUploadPath));
				free(pcPath);
			}
		}
		if (httpIsExistParam(pParamList, "Enable"))
			g_ConfigParam.bFtpEnable = httpGetBool(pParamList, "Enable");
		else g_ConfigParam.bFtpEnable = FALSE;
		g_WebCamState.ucFtp = (g_ConfigParam.bFtpEnable?'\1':'\0');
		cyg_mutex_unlock(&g_ptmConfigParam);

		WebCameraLog(CL_PRIVILEGE_ADMIN, CL_SET_FTP, g_ConfigParam.acFtpServer, hConnection);

		WriteFlashMemory(&g_ConfigParam);

		return 0;
		break;
	}
	return -1;
}

int Config_GetFtp(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	char ac[384];

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
		break;
	case CA_CONFIG:
		AddHttpValue(pReturnXML, G_PC_FTPSERVER, g_ConfigParam.acFtpServer);
		AddHttpValue(pReturnXML, G_PC_USER, g_ConfigParam.acFtpUser);
		AddHttpValue(pReturnXML, G_PC_PASS, g_ConfigParam.acFtpPass);
		AddHttpValue(pReturnXML, G_PC_ACCOUNT, g_ConfigParam.acFtpAccount);
		AddHttpValue(pReturnXML, G_PC_UPLOADPATH, g_ConfigParam.acFtpUploadPath);
		httpLong2String((int)g_ConfigParam.bFtpEnable, ac);
		AddHttpValue(pReturnXML, G_PC_ENABLE, ac);
		return 0;
		break;
	}
	return -1;
}
#endif
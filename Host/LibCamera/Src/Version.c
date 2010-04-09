#include "CommonDef.h"
char *g_version[]=
{
#include "../../../ChangeLog.txt"
};

#define VERSION_STR "$Revision: "

void WRGetProductVersionNum(int *pnMajor, int *pnMinor, const char **ppcDate, const char **ppcTime)
{
	const char *pcVerNum = strstr (g_version[0], VERSION_STR);
	
	if (pnMajor != NULL)
		*pnMajor = 5;

	if (pcVerNum != NULL)
	{
		pcVerNum += strlen (VERSION_STR);
		if (pnMinor != NULL)
			*pnMinor = atoi(pcVerNum);
	}
	else
	{
		if (pnMinor != NULL)
			*pnMinor = 0;
	}
	
	if (ppcDate != NULL)
		*ppcDate = __DATE__;
	if (ppcTime != NULL)
		*ppcTime = __TIME__;
}

char *WRGetProductVersion(void)
{
	static char acRt[128] = "";
	int nMajor, nMinor;
	const char *pcDate, *pcTime;
	
	if (acRt[0] != '\0')
		return acRt;

	WRGetProductVersionNum(&nMajor, &nMinor, &pcDate, &pcTime);
	snprintf(acRt,sizeof(acRt), "%s %s %s%d.%d$", pcDate, pcTime,VERSION_STR, nMajor, nMinor);
	return acRt;
}

int Config_GetVer(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_USER;
	case CA_CONFIG:
		AddHttpValue(pReturnXML, "Version", WRGetProductVersion());
		return 0;
	}
	return -1;
}

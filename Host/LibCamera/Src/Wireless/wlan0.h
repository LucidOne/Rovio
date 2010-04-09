#ifndef WLAN_H
#define WLAN_H
/* Run dhcpc on pcInterface. */
#define __WLAN_ATTRIB_PACK__		__attribute__ ((packed))
typedef struct prism_cnfDesireSSID
{
	unsigned short ssidLen __WLAN_ATTRIB_PACK__;
	unsigned char  ssidName[32] __WLAN_ATTRIB_PACK__;
} __WLAN_ATTRIB_PACK__ prism_cnfDesireSSID_t;

BOOL WRRunDHCPClient(const char *pcInterface, BOOL bWait);

BOOL SetWlanESSID(const char *pcInterface, const char *pcWlanID);
char *GetWlanESSID(const char *pcInterface, char *pcWlanID, int iWlanIDMaxLen);
BOOL SetWlanChannel(const char *pcInterface, int iChannel);
BOOL GetWlanChannel(const char *pcInterface, int *piChannel);
BOOL SetWlanOperationMode(const char *pcInterface, int iMode);
BOOL GetWlanOperationMode(const char *pcInterface, int *piMode);
BOOL SetWlanRegion(const char *pcInterface, int nRegionCode);
BOOL SetWlanMTU(const char *pcInterface, int nMTU);
BOOL wlanSetBitrate(const char *pcInterface, int iBitrate);
BOOL wlanGetBitrate(const char *pcInterface, int *piBitrate);
prism_cnfDesireSSID_t *GetWlanESSIDList(const char *pcInterface, int *piWlanESSIDNum);
BOOL IsWlanLinked(const char *pcInterface);
BOOL SetWlanHostSleepCfg(const char *pcInterface, int nGPIO);
BOOL SetWlanHostWakeUpCfg(const char *pcInterface);
BOOL SetWlanPSMode(const char *pcInterface, BOOL bDisable);
BOOL SetWlanDeepSleepMode(const char *pcInterface, BOOL bEnter);

int Config_SetWlan(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
int Config_GetWlan(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
int Config_ScanWlan(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

BOOL SetWlanWepKey(const char *pcInterface,	const void *pAscKey,int iAscKeyLen,int iKeyIndex,int WepAuthentication);

#endif
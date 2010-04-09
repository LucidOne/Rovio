#ifndef WLAN_H
#define WLAN_H

#define PTE do {fprintf(stderr, "%s,%d.\n", __FILE__, __LINE__);} while(0)
#define PTI do {printf("%s,%d.\n", __FILE__, __LINE__);} while(0)
#define PAZ do {fprintf(stderr, "%s,%d.\n", __FILE__, __LINE__); getc(stdin);} while(0)
#define PRINT_MEM_OUT do {fprintf(stderr, "Not enough memory in %s %d.\n", __FILE__, __LINE__);} while(0)


/* Run dhcpc on pcInterface. */
//#define __WLAN_ATTRIB_PACK__		__attribute__ ((packed))
#define __WLAN_ATTRIB_PACK__
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
prism_cnfDesireSSID_t *GetWlanESSIDList(const char *pcInterface, int *piWlanESSIDNum);
BOOL IsWlanLinked(const char *pcInterface);
BOOL SetWlanWepKey(const char *pcInterface,	const void *pAscKey,int iAscKeyLen,int iKeyIndex,int WepAuthentication);

#endif
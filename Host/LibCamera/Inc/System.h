#include "../../wpa_supplicant/Src/includes.h"

#ifndef SYSTEM_H
#define SYSTEM_H

typedef struct
{
	char acIface[8];
	unsigned long ulDestination;
	unsigned long ulGateway;
	unsigned long ulFlags;
	int iRefCnt;
	int iUse;
	int iMetric;
	unsigned long ulMask;
	int iMTU;
	int iWindow;
	int iIRTT;
} PROC_ROUTE_ITEM_T;

/* Initialize network interface */
BOOL InitNetInterface(const char *pcInterface);

/* Set Ip & Netmask */
BOOL SetGeneralIP(const char *pcInterface, unsigned long ulIP, unsigned long ulNetmask);

/* Get Ip Address
Return: if success, return ip address, else return 0; */
unsigned long GetIPAddress(const char *pcInterface);

/* Get Subnet mask
Return: if success, return netmask, else return NULL */
unsigned long GetNetMask(const char *pcInterface);

/* Get default gateway
Return: if success, return default gateway, else return NULL */
unsigned long GetGateway(const char *pcInterface);

/* Set default gateway
Return: if success, return TRUE, otherwise, return FALSE; */
BOOL SetGateway(const char *pcInterface, unsigned long ulIP);

/* The procedure for SIGTERM & SIGINT */
void WebCameraSIGTERM(int iSignal);
BOOL DownInterface(const char *pcInterface);
void thread_join(cyg_handle_t* handle, cyg_thread* thread, cyg_thread_info* info);
int Config_SetIP(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
int Config_GetIP(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

int Config_SetName(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
int Config_GetName(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

int Config_SetHttp(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
int Config_GetHttp(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

int Config_Reboot(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

int Config_ControlMCU(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

void RebootOnConnectionOver(HTTPCONNECTION hConnection);
void set_sockaddr(struct sockaddr_in *sin, unsigned long addr, unsigned short port);
void GetPubIPInfo(unsigned long *pulPublicIpAddress,
							unsigned long *pulPublicSubnetMask,
							unsigned long *pulDefaultGateway,
							unsigned long *pulDNSServer);
//void GetDHCPInfo(void);		
char * Asmalloc(UsedBuf* asbuf,unsigned int *slen,int ulen);	
void SetRandomIP(const char *pcInterface);
BOOL SetIP(enum SET_IP_STATE_E *ip_state);	
BOOL SetWlan(void);	
BOOL netIsSuspendAllowed(void);
void W99802Reboot(void);

int Config_SetMac(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
int Config_GetMac(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

int Config_GetUPnP(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
int Config_SetUPnP(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);


int Config_SetPPP(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
int Config_GetPPP(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);
#endif

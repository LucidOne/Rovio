#include "../Inc/CommonDef.h"
#include "../../wpa_supplicant/Src/includes.h"

#ifdef WLAN
BOOL SetWlanESSID(const char *pcInterface, const char *pcWlanID)
{
	int rt;
	int fd;
	struct iwreq wrq;
	char ac[64];

	if (pcInterface == NULL)
	{
		fprintf(stderr, "illegal call function SetWlanESSID!\n");
		return FALSE;
	}

#ifndef WLAN
	if ((fd = socket(AF_INET,SOCK_DGRAM,0, g_StreamServer_Buf, System_BUFFER_LEN)) < 0) 
		return FALSE;
#else
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) 
		return FALSE;
#endif	
	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, pcInterface);

	if (pcWlanID == NULL)
	{//essid: any
		ac[0] = '\0';
		wrq.u.essid.flags = 0;
	}
	else
	{
		httpMyStrncpy(ac, pcWlanID, sizeof(ac));
		wrq.u.essid.flags = 1;
	}

	wrq.u.essid.pointer = (caddr_t)ac;
	wrq.u.essid.length = strlen(ac) + 1;
	
#ifndef WLAN
	rt = netioctl(fd, SIOCSIWESSID, &wrq, sizeof(wrq),g_StreamServer_Buf, System_BUFFER_LEN);

	netclose(fd, g_StreamServer_Buf, System_BUFFER_LEN);
#else
	if((rt = ioctl(fd, SIOCSIWESSID, &wrq)) < 0)
		diag_printf("scan ESSID %s failed\n",pcWlanID);	
	else
		diag_printf("set ssid finished\n");
	
	close(fd);
#endif	
	return (rt >= 0 ? TRUE : FALSE);
}

char *GetWlanESSID(const char *pcInterface, char *pcWlanID, int iWlanIDMaxLen)
{
	int fd;
	struct iwreq wrq;
	char ac[64];

	if (pcInterface == NULL || pcWlanID == NULL || iWlanIDMaxLen < 0)
	{
		fprintf(stderr, "illegal call function GetWlanESSID!\n");
		return NULL;
	}
#ifndef WLAN
	if ((fd = socket(AF_INET,SOCK_DGRAM,0, g_StreamServer_Buf, System_BUFFER_LEN)) < 0) 
		return NULL;
#else
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) 
		return NULL;
#endif
	/* Get network ID */
	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, pcInterface);
	wrq.u.essid.pointer = (caddr_t)ac;
	wrq.u.essid.length = 0;
	wrq.u.essid.flags = 0;
#ifndef WLAN  
	if(netioctl(fd, SIOCGIWESSID, &wrq, sizeof(wrq),g_StreamServer_Buf, System_BUFFER_LEN) >= 0)
#else
	if(ioctl(fd, SIOCGIWESSID, &wrq) >= 0)
#endif
	{
		if (wrq.u.essid.flags == 0) pcWlanID[0] = '\0';
		else
		{
			if (wrq.u.essid.length < sizeof(ac))
				ac[wrq.u.essid.length] = '\0';
			else ac[sizeof(ac) - 1] = '\0';
			httpMyStrncpy(pcWlanID, ac, iWlanIDMaxLen);
		}
	}
	else pcWlanID[0] = '\0';
#ifndef WLAN 
	netclose(fd, g_StreamServer_Buf, System_BUFFER_LEN);
#else
	close(fd);
#endif
	return pcWlanID;
}

BOOL SetWlanChannel(const char *pcInterface, int iChannel)
{
	int rt;
	int fd;
	struct iwreq wrq;

	if (pcInterface == NULL || iChannel < 0)
	{
		fprintf(stderr, "illegal call function SetWlanChannel!\n");
		return FALSE;
	}
#ifndef WLAN  
	if ((fd = socket(AF_INET,SOCK_DGRAM,0, g_StreamServer_Buf, System_BUFFER_LEN)) < 0) 
		return FALSE;
#else
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) 
		return FALSE;
#endif
	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, pcInterface);
	wrq.u.freq.e = 0;
	wrq.u.freq.m = iChannel;
#ifndef WLAN 
	rt = ioctl(fd, SIOCSIWFREQ, &wrq);
	close(fd);
#else
	if((rt = ioctl(fd, SIOCSIWFREQ, &wrq)) <0)
		diag_printf("SetWlanChannel failed\n");
	close(fd);
#endif
	return (rt >= 0 ? TRUE : FALSE);
}

BOOL GetWlanChannel(const char *pcInterface, int *piChannel)
{
	int rt;
	int fd;
	struct iwreq wrq;

	if (pcInterface == NULL || piChannel == NULL)
	{
		fprintf(stderr, "illegal call function GetWlanChannel!\n");
		return FALSE;
	}
#ifndef WLAN 
	if ((fd = socket(AF_INET,SOCK_DGRAM,0, g_StreamServer_Buf, System_BUFFER_LEN)) < 0) 
		return FALSE;
#else
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
		return FALSE;
#endif
	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, pcInterface);
#ifndef WLAN 
	rt = netioctl(fd, SIOCGIWFREQ, &wrq,sizeof(wrq), g_StreamServer_Buf, System_BUFFER_LEN);
	netclose(fd, g_StreamServer_Buf, System_BUFFER_LEN);
#else
	rt = ioctl(fd, SIOCGIWFREQ, &wrq);
	close(fd);
#endif	

	if (rt >= 0)
	{
		*piChannel = wrq.u.freq.m;
		return TRUE;
	}
	else return FALSE;
}


/*
 * Name: SetWlanWepKey
 * Description: Set wireless WEP key.
 *
 * Parameter:
 *  pcInterface[in]: Net interface for wireless lan.
 *  pAscKey[in]: Buffer length of which >= 14.
 *  iAscKeyLen[in]: The Length of valid buffer pointed by "pAscKey".
 *                  Set the value to -1 if WEP key wants to be
 *                  disabled.
 *  iKeyIndex[in]: Index of WEP keys.
 * No return value.
 */
BOOL SetWlanWepKey(const char *pcInterface,
					const void *pAscKey,
					int iAscKeyLen,
					int iKeyIndex,
					int WepAuthentication)
{
	int fd;
	int rt;
	unsigned char pucKey[IW_ENCODING_TOKEN_MAX];
	struct iwreq wrq;
	
	diag_printf("SetWlanWepKey=%s,%02x%02x%02x,%d,%d,%d\n", pcInterface,
		(int)((unsigned char *)pAscKey)[0],
		(int)((unsigned char *)pAscKey)[1],
		(int)((unsigned char *)pAscKey)[2],
		iAscKeyLen,
		iKeyIndex,
		WepAuthentication);
		
	
	if (pcInterface == NULL) return FALSE;
#ifndef WLAN
	if ((fd = socket(AF_INET,SOCK_DGRAM,0, g_StreamServer_Buf, System_BUFFER_LEN)) < 0)
		return FALSE;
#else
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) 
		return FALSE;	
#endif
	memset(&wrq, 0, sizeof(wrq));

	/* iKeyIndex in ioctl from 1 to 4, while SetWlanWepKey use 0 to 3 */
	iKeyIndex++;

	strcpy(wrq.ifr_name, pcInterface);
	if (iAscKeyLen >= 0)
	{//on
		wrq.u.data.length = iAscKeyLen;
		if(wrq.u.data.length > IW_ENCODING_TOKEN_MAX)
			wrq.u.data.length = IW_ENCODING_TOKEN_MAX;
		if (pAscKey != NULL)
		{
			memcpy(pucKey, pAscKey, wrq.u.data.length);
			wrq.u.data.pointer = (caddr_t)pucKey;
		}
		else wrq.u.data.pointer = NULL;

		if (iKeyIndex > 0 && iKeyIndex < IW_ENCODE_INDEX)
			wrq.u.encoding.flags |= iKeyIndex;

		wrq.u.data.flags |= WepAuthentication;
		//wrq.u.data.flags |= IW_ENCODE_OPEN;
	}
	else
	{//off
		wrq.u.data.length = 0;
		wrq.u.data.pointer = (caddr_t)pucKey;
		wrq.u.data.flags |= IW_ENCODE_DISABLED;
		wrq.u.data.flags |= IW_ENCODE_OPEN;
	}
#ifndef WLAN
	rt = netioctl(fd, SIOCSIWENCODE, &wrq, sizeof(wrq),g_StreamServer_Buf, System_BUFFER_LEN);
	netclose(fd, g_StreamServer_Buf, System_BUFFER_LEN);
#else
	rt = ioctl(fd, SIOCSIWENCODE, &wrq);
	close(fd);
#endif
	return (rt >= 0 ? TRUE : FALSE);
}

/*
 * Name: GetWlanWepKey
 * Description: Get wireless WEP key settings.
 *
 * Parameter:
 *  pcInterface[in]: Net interface for wireless lan.
 *  pAscKey[out]: Buffer length of which >= 14.
 *  piAscKeyLen[out]: The Length of valid buffer pointed by "pAscKey".
 *                    -1 if WEP key is disabled.
 *  piKeyIndex[out]: Index of WEP keys.
 * Return value:
 *  TRUE: Success.
 *  FALSE: Failed.
 */
BOOL GetWlanWepKey(const char *pcInterface,
					void *pAscKey,
					int *piAscKeyLen,
					int *piKeyIndex)
{
	int fd;
	int rt;
	unsigned char pucKey[IW_ENCODING_TOKEN_MAX];
	struct iwreq wrq;

	if (pcInterface == NULL) return FALSE;
#ifndef WLAN
	if ((fd = socket(AF_INET,SOCK_DGRAM,0, g_StreamServer_Buf, System_BUFFER_LEN)) < 0) 
		return FALSE;
#else
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) 
		return FALSE;
#endif
	memset(&wrq, 0, sizeof(wrq));

	strcpy(wrq.ifr_name, pcInterface);
	wrq.u.data.pointer = (caddr_t)pucKey;
	wrq.u.data.length = 0;
	wrq.u.data.flags = 0;
#ifndef WLAN
	rt = netioctl(fd, SIOCGIWENCODE, &wrq, sizeof(wrq),g_StreamServer_Buf, System_BUFFER_LEN);
    netclose(fd, g_StreamServer_Buf, System_BUFFER_LEN);
#else
	rt = ioctl(fd, SIOCGIWENCODE, &wrq);
    close(fd);
#endif
    if (rt >= 0)
    {
    	if (wrq.u.data.flags & IW_ENCODE_DISABLED)
    	{
    		if (piAscKeyLen != NULL) *piAscKeyLen = -1;
    	}
    	else
    	{
    		if (piAscKeyLen != NULL)
    		{
				/* iKeyIndex in ioctl from 1 to 4, while SetWlanWepKey use 0 to 3 */
    			*piAscKeyLen = wrq.u.data.length - 1;
    		}
    		if (pAscKey != NULL)
    			memcpy(pAscKey, pucKey, wrq.u.data.length);
    		if (piKeyIndex != NULL)
    			*piKeyIndex = wrq.u.encoding.flags & 0x000000FF;
    	}
    }

	return (rt >= 0 ? TRUE : FALSE);
}


const char *g_apcWlanOperationMode[6] =
{
	"Auto",
	"Ad-Hoc",
	"Managed",
	"Master",
	"Repeater",
	"Secondary"
};


/*
 * Name: SetWlanOperationMode
 * Description: Set wireless operation mode.
 *
 * Parameter:
 *  pcInterface[in]: Net interface for wireless lan.
 *  iMode[in]: Operation mode.
 * Return value:
 *  TRUE: Success.
 *  FALSE: Failed.
 */
BOOL SetWlanOperationMode(const char *pcInterface, int iMode)
{
	int fd;
	int rt;
	struct iwreq wrq;
	//int buffer_size = 1024;
	//char buffer_size[1024];

	if (pcInterface == NULL) return FALSE;
	if (!(iMode >= 0
		&& iMode < sizeof(g_apcWlanOperationMode) / sizeof(const char *)))
		return FALSE;

	if (iMode != 1 && iMode != 2)
	{
		fprintf(stderr, "Only Ad-Hoc and Managed modes supported.\n");
		return FALSE;
	}

	//if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;
#ifndef WLAN
	if ((fd = socket(AF_INET,SOCK_DGRAM,0, g_StreamServer_Buf, System_BUFFER_LEN)) < 0) 
		return FALSE;
#else
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) 
		return FALSE;
#endif
	memset(&wrq, 0, sizeof(wrq));

	strcpy(wrq.ifr_name, pcInterface);
	wrq.u.mode = iMode;
diag_printf("mode %d\n", iMode);
#ifndef WLAN
	rt = netioctl(fd, SIOCSIWMODE, &wrq,sizeof(wrq), g_StreamServer_Buf, System_BUFFER_LEN);
    netclose(fd, g_StreamServer_Buf, System_BUFFER_LEN);
#else
	rt = ioctl(fd, SIOCSIWMODE, &wrq);
    close(fd);
#endif
	return (rt >= 0 ? TRUE : FALSE);
}


/*
 * Name: GetWlanOperationMode
 * Description: Get wireless operation mode.
 *
 * Parameter:
 *  pcInterface[in]: Net interface for wireless lan.
 *  piMode[out]: Operation mode.
 * Return value:
 *  TRUE: Success.
 *  FALSE: Failed.
 */
BOOL GetWlanOperationMode(const char *pcInterface, int *piMode)
{
	int fd;
	int rt;
	struct iwreq wrq;
	

	if (pcInterface == NULL) return FALSE;

	//if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;
#ifndef WLAN	
	if ((fd = socket(AF_INET,SOCK_DGRAM,0, g_StreamServer_Buf, System_BUFFER_LEN)) < 0) 
		return FALSE;
#else
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) 
		return FALSE;

#endif
	memset(&wrq, 0, sizeof(wrq));

	strcpy(wrq.ifr_name, pcInterface);
#ifndef WLAN
	rt = netioctl(fd, SIOCGIWMODE, &wrq,sizeof(wrq), g_StreamServer_Buf, System_BUFFER_LEN);
    netclose(fd, g_StreamServer_Buf, System_BUFFER_LEN);
#else
	rt = ioctl(fd, SIOCGIWMODE, &wrq);
    close(fd);
#endif
    if (rt < 0
    	|| !(wrq.u.mode >= 0 && wrq.u.mode < sizeof(g_apcWlanOperationMode) / sizeof(const char *)))
    	return FALSE;

	if (piMode != NULL) *piMode = wrq.u.mode;
	return TRUE;
}


BOOL wlanSetBitrate(const char *pcInterface, int iBitrate)
{
	int rt;
	int fd;
	struct iwreq wrq;

	diag_printf("Set bitrate: %s, %d\n", pcInterface, iBitrate);
	if (pcInterface == NULL)
	{
		fprintf(stderr, "illegal call function WlanSetBitrate!\n");
		return FALSE;
	}
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) 
		return FALSE;
	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, pcInterface);
	wrq.u.bitrate.value = iBitrate;
	rt = ioctl(fd, SIOCSIWRATE, &wrq);
	if( rt < 0 )
	{	
		fprintf(stderr, "Set Tx rate failed\n");
	}
	close(fd);
	return (rt >= 0 ? TRUE : FALSE);
}


BOOL wlanGetBitrate(const char *pcInterface, int *piBitrate)
{
	int rt;
	int fd;
	struct iwreq wrq;

	if (pcInterface == NULL)
	{
		fprintf(stderr, "illegal call function WlanGetBitrate!\n");
		return FALSE;
	}
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) 
		return FALSE;
	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, pcInterface);
	rt = ioctl(fd, SIOCGIWRATE, &wrq);
	if( rt < 0 )
	{	
		fprintf(stderr, "Get Tx rate failed\n");
	}
	else if (piBitrate != NULL)
		*piBitrate = wrq.u.bitrate.value;
	close(fd);
	return (rt >= 0 ? TRUE : FALSE);
}


static BOOL ScanWlanAp(const char *pcInterface, int fd)
{
	struct iwreq wrq;

	if (pcInterface == NULL || fd < 0) return FALSE;

	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, pcInterface);
diag_printf("interface: %s\n", pcInterface);
	/* Get number of ap */
#ifndef WLAN
	if(netioctl(fd, 0x8B18, &wrq,sizeof(wrq), g_StreamServer_Buf, System_BUFFER_LEN) < 0)
#else
	if(ioctl(fd, 0x8B18, &wrq) < 0)
#endif
	{
		fprintf(stderr, "SIOCSIWSCAN: %s\n", strerror(errno));
		return FALSE;
	}
	return TRUE;

/*
   	if (wrq.u.data.length > 0) return wrq.u.data.length;
   	else
   	{
PTE;
   		return 0;
   	}
*/
}

static prism_cnfDesireSSID_t *ReadWlanESSIDList(const char *pcInterface, int fd, int *piLength)
{
   	prism_cnfDesireSSID_t *pApList = NULL;
   	int i, j;
   	struct iwreq wrq;

	if (pcInterface == NULL || fd < 0 || piLength == NULL)
		return NULL;

	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, pcInterface);

   	wrq.u.data.length = 0;//get twice.
 #ifndef WLAN
	if(netioctl(fd, SIOCGIWAPLIST, &wrq,sizeof(wrq), g_StreamServer_Buf, System_BUFFER_LEN) < 0)
#else
	if(ioctl(fd, SIOCGIWAPLIST, &wrq) < 0)
#endif
	{
		diag_printf("ioctl SIOCGIWAPLIST < 0\n");
		return NULL;
	}

	if (wrq.u.data.length <= 0)
	{
		diag_printf("wrq.u.data.length = %d\n", wrq.u.data.length);
		return NULL;
	}

	pApList = (prism_cnfDesireSSID_t *)malloc(wrq.u.data.length * sizeof(prism_cnfDesireSSID_t));
	if(pApList == NULL)
	{
		PRINT_MEM_OUT;
		return NULL;
	}

	wrq.u.data.pointer = (caddr_t)pApList;
#ifndef WLAN
	if(netioctl(fd, SIOCGIWAPLIST, &wrq, sizeof(wrq),g_StreamServer_Buf, System_BUFFER_LEN) < 0)
#else
	if(ioctl(fd, SIOCGIWAPLIST, &wrq) < 0)
#endif
	{
		diag_printf("ioctl SIOCGIWAPLIST < 0\n");
		free(pApList);
		return NULL;
	}

	for(i = 0; i < wrq.u.data.length; i++)
	{
		diag_printf("Access point: %d	", i);
		for(j = 0; j < (pApList + i)->ssidLen; j++)
			diag_printf("%c", *((pApList + i)->ssidName + j));
		diag_printf("\n");
	}

	*piLength = wrq.u.data.length;
	return pApList;
}

prism_cnfDesireSSID_t *GetWlanESSIDList(const char *pcInterface, int *piWlanESSIDNum)
{
	int fd;
	//char ac[64];
	int iApNum;
	prism_cnfDesireSSID_t *pRt;

	if (pcInterface == NULL || piWlanESSIDNum == NULL)
	{
		fprintf(stderr, "illegal call function GetWlanESSIDList!\n");
		return NULL;
	}
#ifndef WLAN
	if ((fd = socket(AF_INET,SOCK_DGRAM,0, g_StreamServer_Buf, System_BUFFER_LEN)) < 0) 
		return NULL;
#else
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) 
		return NULL;
#endif

	if (ScanWlanAp(pcInterface, fd))
	{
		pRt = ReadWlanESSIDList(pcInterface, fd, &iApNum);

		if (pRt == NULL) *piWlanESSIDNum = 0;
		else *piWlanESSIDNum = iApNum;
	}
	else PTE;
#ifndef WLAN
	netclose(fd, g_StreamServer_Buf, System_BUFFER_LEN);
#else
	close(fd);
#endif
	

	return pRt;
}


BOOL GetWlanLinkState(const char *pcInterface, int *piState)
{
	int fd;
	int rt;
	int iState;
	struct iwreq wrq;

	if (pcInterface == NULL || piState == NULL) return FALSE;
#ifndef WLAN
	if ((fd = socket(AF_INET,SOCK_DGRAM,0, g_StreamServer_Buf, System_BUFFER_LEN)) < 0) 
		return FALSE;

#else
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) 
		return FALSE;

#endif
	memset(&wrq, 0, sizeof(wrq));

	strcpy(wrq.ifr_name, pcInterface);
	wrq.u.data.pointer = (caddr_t)&iState;
   	wrq.u.data.length = sizeof(int);
#ifndef WLAN
   	rt = netioctl(fd, SIOCIWFIRSTPRIV + 0x9, &wrq,sizeof(wrq), g_StreamServer_Buf, System_BUFFER_LEN);
   	netclose(fd, g_StreamServer_Buf, System_BUFFER_LEN);
#else
   	rt = ioctl(fd, SIOCIWFIRSTPRIV + 0x9, &wrq);
   	close(fd);
#endif


	if (rt >= 0)
	{
		*piState = iState;
		return TRUE;
	}
	else return FALSE;
}


BOOL IsWlanLinked(const char *pcInterface)
{
	int iState;
	if (GetWlanLinkState(pcInterface, &iState) && iState == 1)
		return TRUE;
	else return FALSE;
}

BOOL SetWlanHostSleepCfg(const char *pcInterface, int nGPIO)
{
	int rt;
	int fd;
	unsigned long ipaddr;
	struct iwreq wrq;
	char buf[32];
    int ret = WLAN_STATUS_SUCCESS;

    memset(buf, 0, sizeof(buf));

	if (pcInterface == NULL)
	{
		fprintf(stderr, "illegal call function SetWlanChannel!\n");
		return FALSE;
	}

	ipaddr = GetIPAddress(pcInterface);

	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;

	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, pcInterface);
	/* broadcast=1 ; unicast=2 ; multicast=3 */
	sprintf(buf, "03 %02x 0xa0 %08x", nGPIO, ipaddr);//set criteria/gpio/gap
//	sprintf(buf, "02 %02x 0xa0", nGPIO);//set criteria/gpio/gap
	wrq.u.data.pointer = buf;
	wrq.u.essid.length = strlen(buf) + 1;

	rt = ioctl(fd, WLANHOSTSLEEPCFG, &wrq);
	close(fd);
	if(rt >= 0)
		diag_printf("set host sleep config success %s\n", buf);
	
	return (rt >= 0 ? TRUE : FALSE);
}


BOOL SetWlanHostWakeUpCfg(const char *pcInterface)
{
	int rt;
	int fd;
	struct iwreq wrq;
	char buf[32];
    int ret = WLAN_STATUS_SUCCESS;

    memset(buf, 0, sizeof(buf));

	if (pcInterface == NULL)
	{
		fprintf(stderr, "illegal call function SetWlanChannel!\n");
		return FALSE;
	}
	
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;

	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, pcInterface);
	sprintf(buf, "0xffffffff");//set criteria/gpio/gap
	wrq.u.data.pointer = buf;
	wrq.u.essid.length = strlen(buf) + 1;

	rt = ioctl(fd, WLANHOSTSLEEPCFG, &wrq);
	close(fd);
	if(rt >= 0)
		diag_printf("set host wake up config success %s\n", buf);
	
	return (rt >= 0 ? TRUE : FALSE);
}


BOOL SetWlanPSMode(const char *pcInterface, BOOL bDisable)
{
	int rt;
	int fd;
	struct iwreq wrq;
	char buf[32];
  // int ret = WLAN_STATUS_SUCCESS;
    int gpio, gap;

    memset(buf, 0, sizeof(buf));

	if (pcInterface == NULL)
	{
		fprintf(stderr, "illegal call function SetWlanChannel!\n");
		return FALSE;
	}

	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;

	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, pcInterface);
	
	wrq.u.power.disabled = bDisable;
	wrq.u.power.flags = IW_POWER_ON;

	rt = ioctl(fd, SIOCSIWPOWER, &wrq);
	close(fd);
	if(rt >= 0)
		diag_printf("set power mode to enter power saving auto success\n");
	return (rt >= 0 ? TRUE : FALSE);
}


BOOL SetWlanDeepSleepMode(const char *pcInterface, BOOL bEnter)
{
	int rt;
	int fd;
	struct iwreq wrq;
	struct ifreq *ifr;
	char buf[32];
//    int ret = WLAN_STATUS_SUCCESS;
    int gpio, gap;

    memset(buf, 0, sizeof(buf));

	if (pcInterface == NULL)
	{
		fprintf(stderr, "illegal call function SetWlanChannel!\n");
		return FALSE;
	}

	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;

	memset(&wrq, 0, sizeof(wrq));
	ifr = (struct ifreq *)&wrq;
	strcpy(wrq.ifr_name, pcInterface);
	*(char *)(ifr->ifr_data) = (char)(bEnter ? 1 : 0);//1 for enter; 0 for exit
	
	rt = ioctl(fd, WLANDEEPSLEEP, &wrq);
	if(rt >= 0)
		diag_printf("set host sleep config success\n");
	close(fd);
	
	return (rt >= 0 ? TRUE : FALSE);
}


BOOL SetWlanRegion(const char *pcInterface, int nRegionCode)
{
	int rt;
	int fd;
	struct iwreq wrq;
	

	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;

	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, pcInterface);
	
	*(int *)wrq.u.name = WLANSETREGION;
	*(int *)(wrq.u.name + 4) = nRegionCode;
	rt = ioctl(fd, WLAN_SETONEINT_GETNONE, &wrq);
	diag_printf("Set Wlan region to japan");
	close(fd);	
	return rt >= 0 ? TRUE : FALSE;
}


BOOL SetWlanMTU(const char *pcInterface, int nMTU)
{
	int rt;
	int fd;
	struct ifreq ifr;

	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, pcInterface);
	ifr.ifr_mtu = nMTU;
	rt = ioctl(fd, SIOCSIFMTU, &ifr);
	diag_printf("Set mtu to %d, %s\n", nMTU, (rt >= 0 ? "OK" : "Failed") );
	close(fd);	
	return rt >= 0 ? TRUE : FALSE;
}



#ifndef CONFIGINTERFACE
int Config_SetWlan(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	int i, j;

	const char *pcWlanESSID;
	unsigned long ulWlanChannel;
	const char *pcWlanOperationMode;
	int iWlanOperationMode;
	char acWlanESSID_Before[33];
	int iWlanChannel_Before;
	int iWlanOperationMode_Before;

	unsigned char ucWepSet;
	const char *pcWepSet;
	const char *pcWepAsc;

	const char *apcWep64[WEP64_GROUP_NUMBER];
	unsigned char aacWep64[5];
	const char *pcWep128;
	BOOL bSuccessSetWep = FALSE;

	BOOL bIsChange = FALSE;

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
	case CA_CONFIG:
	/*get mode and config mode*/
		if (httpIsExistParam(pParamList, "Mode"))
		{
			pcWlanOperationMode = httpGetString(pParamList, "Mode");
			for (i = 0; i < sizeof(g_apcWlanOperationMode) / sizeof(const char*); i++)
			{
				if (strcasecmp(pcWlanOperationMode, g_apcWlanOperationMode[i]) == 0)
					break;
			}
			if (!(i < sizeof(g_apcWlanOperationMode) / sizeof(const char *)))
				i = httpString2Long(pcWlanOperationMode);

			if (i < sizeof(g_apcWlanOperationMode) / sizeof(const char *))
			{
				iWlanOperationMode = i;
				if (!GetWlanOperationMode(g_apcNetworkInterface[1], &iWlanOperationMode_Before)
					|| iWlanOperationMode_Before != iWlanOperationMode)
				{
					//if (SetWlanOperationMode(g_apcNetworkInterface[1], iWlanOperationMode))
					//{
						g_ConfigParam.ucWlanOperationMode = (unsigned char)iWlanOperationMode;
						bIsChange = TRUE;
					//}
				}
			}
		}	
		/*get channel and config channel*/	
		if (httpIsExistParam(pParamList, "Channel"))
		{
			ulWlanChannel = (unsigned long)httpGetLong(pParamList, "Channel");
			if (!GetWlanChannel(g_apcNetworkInterface[1], &iWlanChannel_Before)
				|| iWlanChannel_Before != (int)ulWlanChannel)
			{
				//if (SetWlanChannel(g_apcNetworkInterface[1], (int)ulWlanChannel))
				//{
					g_ConfigParam.ulWlanChannel = ulWlanChannel;
					bIsChange = TRUE;
				//}
			}
		}
		/*get wepset and config wepset*/
		pcWepSet = httpGetString(pParamList, "WepSet");
		pcWepAsc = httpGetString(pParamList, "WepAsc");
		
		apcWep64[i] = httpGetString(pParamList, "Wep64");
		memset(aacWep64, 0, sizeof(aacWep64));
		for (j = 0; j < 5; j++)
		{
			if (apcWep64[2 * j] == '\0'
				|| apcWep64[2 * j + 1] == '\0')
				break;
			aacWep64[j] = httpHex2Char(apcWep64 + 2 * j);
		}
				
		pcWep128 = httpGetString(pParamList, "Wep128");
		memset(acWep128, 0, sizeof(acWep128));
		for (j = 0; j < 13; j++)
		{
			if (pcWep128[2 * j] == '\0'
				|| pcWep128[2 * j + 1] == '\0')
				break;
			acWep128[j] = httpHex2Char(pcWep128 + 2 * j);
		}
		if (strcmp(pcWepSet, "Disable") == 0)
			ucWepSet = WEP_SET_DISABLE;
		else if (strcmp(pcWepSet, "K64") == 0)
			ucWepSet = WEP_SET_K64;
		else if (strcmp(pcWepSet, "K128") == 0)
			ucWepSet = WEP_SET_K128;
		else if (strcmp(pcWepSet, "Asc") == 0)
			ucWepSet = WEP_SET_ASC;
		/*
		if (strcmp(pcWepSet, "Disable") == 0)
		{
			ucWepSet = WEP_SET_DISABLE;
			bSuccessSetWep = SetWlanWepKey(g_apcNetworkInterface[1],
											NULL,
											-1,
											-1);
		}
		else if (strcmp(pcWepSet, "K64") == 0)
		{
			ucWepSet = WEP_SET_K64;

			for (i = 0; i < WEP64_GROUP_NUMBER; i++)
			{
				if (!SetWlanWepKey(g_apcNetworkInterface[1],
									aacWep64[i],
									5,
									i)) break;
			}

			if (!(i < WEP64_GROUP_NUMBER))
			{
				if (ulWepDefaultGroup < WEP64_GROUP_NUMBER)
				{
					if (SetWlanWepKey(g_apcNetworkInterface[1],
										NULL,
										0,
										ulWepDefaultGroup))
						bSuccessSetWep = TRUE;
				}
			}
		}
		else if (strcmp(pcWepSet, "K128") == 0)
		{
			ucWepSet = WEP_SET_K128;

			if (SetWlanWepKey(g_apcNetworkInterface[1],
								acWep128,
								13,
								-1)) bSuccessSetWep = TRUE;
		}
		else if (strcmp(pcWepSet, "Asc") == 0)
		{
			ucWepSet = WEP_SET_ASC;
			if (SetWlanWepKey(g_apcNetworkInterface[1],
								pcWepAsc,
								strlen(pcWepAsc),
								-1)) bSuccessSetWep = TRUE;
		}
		*/
		//if (bSuccessSetWep)
		//{
			g_ConfigParam.ucWepSet = ucWepSet;
			httpMyStrncpy((char*)g_ConfigParam.acWepAsc,
				pcWepAsc,
				sizeof(g_ConfigParam.acWepAsc));
				
			memcpy(g_ConfigParam.aacWep64,
				aacWep64,
				sizeof(g_ConfigParam.aacWep64));
			memcpy(g_ConfigParam.acWep128,
				acWep128,
				sizeof(g_ConfigParam.acWep128));

			bIsChange = TRUE;
		//}
		/*get ESSID and config ESSID*/	
		if (httpIsExistParam(pParamList, "ESSID"))
		{
			pcWlanESSID = httpGetString(pParamList, "ESSID");
			if (!GetWlanESSID(g_apcNetworkInterface[1], acWlanESSID_Before, sizeof(acWlanESSID_Before))
				|| strcmp(acWlanESSID_Before, pcWlanESSID) != 0)
			{
				//if (SetWlanESSID(g_apcNetworkInterface[1], pcWlanESSID))
				//{
					httpMyStrncpy((char*)g_ConfigParam.acWlanESSID,
						pcWlanESSID,
						sizeof(g_ConfigParam.acWlanESSID));
					bIsChange = TRUE;
				//}
			}
		}

		if (bIsChange)
		{
			WebCameraLog(CL_PRIVILEGE_COMMON, CL_SET_WLAN, NULL, hConnection);
			WriteFlashMemory(&g_ConfigParam);
		}
		return 0;
	}
	return -1;
}

int Config_GetWlan(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	int iLen;
	char ac[128];
	int i;
	int j;
	//prism_cnfDesireSSID_t *pESSIDList;
	//int iESSIDNum;

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_USER;
	case CA_CONFIG:
		AddHttpValue(pReturnXML, G_PC_ESSID, (char *)g_ConfigParam.acWlanESSID);
		httpLong2String((int)g_ConfigParam.ulWlanChannel, ac);
		AddHttpValue(pReturnXML, G_PC_CHANNEL, ac);
		AddHttpValue(pReturnXML, G_PC_MODE, g_apcWlanOperationMode[((int)g_ConfigParam.ucWlanOperationMode >= 0 && (int)g_ConfigParam.ucWlanOperationMode < sizeof(g_apcWlanOperationMode) / sizeof(const char *)) ? (int)g_ConfigParam.ucWlanOperationMode : 0]);
		AddHttpValue(pReturnXML, G_PC_WEPSET,
			(g_ConfigParam.ucWepSet == WEP_SET_K64 ?
				"K64" :	(g_ConfigParam.ucWepSet == WEP_SET_K128 ?
				"K128" : (g_ConfigParam.ucWepSet == WEP_SET_ASC ?
				"Asc": "Disable"))));
		AddHttpValue(pReturnXML, G_PC_WEPASC,(char*) g_ConfigParam.acWepAsc);
		AddHttpNum(pReturnXML, G_PC_WEPGROUP, 0);
		
		for (iLen = 0, j = 0; j < 5; j++)
		{
			iLen += sprintf(ac + iLen, "%02x",
				(int)g_ConfigParam.aacWep64[j]);
		}
		AddHttpValue(pReturnXML, G_PC_WEP64, ac);

		for (iLen = 0, j = 0; j < 13; j++)
		{
			iLen += sprintf(ac + iLen, "%02x",
				(int)g_ConfigParam.acWep128[j]);
		}
		AddHttpValue(pReturnXML, G_PC_WEP128, ac);
		return 0;
	}
	return -1;
}
#else
int Config_SetWlan(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	const char *pcWlanESSID = NULL;
	unsigned long ulWlanChannel = 0;
	const char *pcWlanOperationMode = NULL;	
	const char *pcWepSet = NULL;
	const char *pcWepAsc = NULL;
	const char *pcWep64type = NULL;
	const char *pcWep128type = NULL;
	const char *pcWep64 = NULL;
	const char *pcWep128 = NULL;
	const char *pcWlanKey = NULL;
	int bIsChange = -1;
	request *req = (request*)hConnection; 

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
	case CA_CONFIG:
		diag_printf("CA_CONFIG Config_SetWiFi!\n");

		/* Get key */
		if (httpIsExistParam(pParamList, "Key"))
			pcWlanKey = httpGetString(pParamList, "Key");

		/*get mode and config mode*/
		if (httpIsExistParam(pParamList, "Mode"))
			pcWlanOperationMode = httpGetString(pParamList, "Mode");
		/*get channel and config channel*/	
		if (httpIsExistParam(pParamList, "Channel"))
			ulWlanChannel = (unsigned long)httpGetLong(pParamList, "Channel");
		else
			ulWlanChannel = g_ConfigParam.ulWlanChannel;
		
		/*get wepset and config wepset*/
		pcWepSet = httpGetString(pParamList, "WepSet");
		
		if(httpIsExistParam(pParamList, "WepAsc"))
			pcWepAsc = httpGetString(pParamList, "WepAsc");
		
		if(httpIsExistParam(pParamList, "Wep64type"))
			pcWep64type = httpGetString(pParamList, "Wep64type");
		pcWep64 = httpGetString(pParamList, "Wep64");
		
		if(httpIsExistParam(pParamList, "Wep128type"))
			pcWep128type = httpGetString(pParamList, "Wep128type");
		diag_printf("64 type %s,128 type %s\n",pcWep64type,pcWep128type);
		pcWep128 = httpGetString(pParamList, "Wep128");

		/*get ESSID and config ESSID*/	
		if (httpIsExistParam(pParamList, "ESSID"))
			pcWlanESSID = httpGetString(pParamList, "ESSID");
diag_printf("mode = %s,channel = %d,pcWepSet==%s\n",pcWlanOperationMode,ulWlanChannel,pcWepSet);
		bIsChange = ictlSetWlan((ICTL_HANDLE_T*)&req->httpPrivilege,
								pcWlanKey,
								pcWlanOperationMode,\
								ulWlanChannel,pcWepSet,pcWepAsc,pcWep64type,\
								pcWep128type,pcWep64,pcWep128,pcWlanESSID);

		if (bIsChange == ICTL_OK)
			WebCameraLog(CL_PRIVILEGE_COMMON, CL_SET_WLAN, NULL, hConnection);			
		/*set new essid*/
		SetWlan();
		//SetIP(); 
		return 0;
	}
	return -1;
}

int Config_GetWlan(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	int iLen;
	char ac[128];
	int j;
	//prism_cnfDesireSSID_t *pESSIDList;
	//int iESSIDNum;

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
	case CA_CONFIG:
		tt_rmutex_lock(&g_rmutex);

		AddHttpValue(pReturnXML, G_PC_ESSID, (char *)g_ConfigParam.acWlanESSID);
		AddHttpValue(pReturnXML, G_PC_MODE,
			g_apcWlanOperationMode[((int)g_ConfigParam.ucWlanOperationMode >= 0 && (int)g_ConfigParam.ucWlanOperationMode < sizeof(g_apcWlanOperationMode) / sizeof(const char *)) ? (int)g_ConfigParam.ucWlanOperationMode : 0]);
		AddHttpValue(pReturnXML, G_PC_KEY, 
			(strlen((char *)g_ConfigParam.acWlanKey) > 0 ? "*" : ""));

		AddHttpNum(pReturnXML, G_PC_CHANNEL, g_ConfigParam.ulWlanChannel);
		/* Useless items */
#if 0
		AddHttpValue(pReturnXML, G_PC_WEPSET, "Disable");
		AddHttpValue(pReturnXML, G_PC_WEPASC, "");
		AddHttpValue(pReturnXML, G_PC_WEPGROUP, "");

		//AddHttpValue(pReturnXML, G_PC_WEP64, ac);
		AddHttpValue(pReturnXML, G_PC_WEP64_TYPE, "Wep64ASC");
		
		//AddHttpValue(pReturnXML, G_PC_WEP128, ac);
		AddHttpValue(pReturnXML, G_PC_WEP128_TYPE, "Wep128ASC");
#else	
		AddHttpValue(pReturnXML, G_PC_WEPSET,
			(g_ConfigParam.ucWepSet == WEP_SET_K64 ?
				"K64" :	(g_ConfigParam.ucWepSet == WEP_SET_K128 ?
				"K128" : (g_ConfigParam.ucWepSet == WEP_SET_ASC ?
				"Asc": "Disable"))));
		AddHttpValue(pReturnXML, G_PC_WEPASC, (strlen((char*) g_ConfigParam.acWepAsc) > 0 ? "*" : ""));
		AddHttpNum(pReturnXML, G_PC_WEPGROUP, 0);
		
		if (g_ConfigParam.bWep64UseAsc)
		{
			memcpy(ac, g_ConfigParam.acWep64, 5);
			ac[5] = '\0';
		}
		else
		{
			for (iLen = 0, j = 0; j < 5; j++)
			{
				iLen += sprintf(ac + iLen, "%02x",
					(int)(unsigned char)g_ConfigParam.acWep64[j]);
			}
		}
		//AddHttpValue(pReturnXML, G_PC_WEP64, ac);
		AddHttpValue(pReturnXML, G_PC_WEP64_TYPE, (g_ConfigParam.bWep64UseAsc ? "Wep64ASC" : "Wep64HEX"));
		
		if (g_ConfigParam.bWep128UseAsc)
		{
			memcpy(ac, g_ConfigParam.acWep128, 13);
			ac[13] = '\0';
		}
		else
		{
			for (iLen = 0, j = 0; j < 13; j++)
			{
				iLen += sprintf(ac + iLen, "%02x",
					(int)(unsigned char)g_ConfigParam.acWep128[j]);
			}
		}
		//AddHttpValue(pReturnXML, G_PC_WEP128, ac);
		AddHttpValue(pReturnXML, G_PC_WEP128_TYPE, (g_ConfigParam.bWep128UseAsc ? "Wep128ASC" : "Wep128HEX"));
		
		{
			enum SET_WIFI_STATE_E wifiState;
			wsp_get_config_state(&wifiState, NULL);
			switch(wifiState)
			{
			case SET_WIFI__TRYING:
				AddHttpValue(pReturnXML, "CurrentWiFiState", "TRYING");
				break;
			case SET_WIFI__OK:
				AddHttpValue(pReturnXML, "CurrentWiFiState", "OK");
				break;
			default:
				AddHttpValue(pReturnXML, "CurrentWiFiState", "NO_INIT");		
			}
		}
		
#endif		
		tt_rmutex_unlock(&g_rmutex);
		return 0;
	}
	return -1;
}
#endif


int Config_ScanWlan(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	ApScanResult ScanAp[32];
	int nLen = 0;
	int i;
	request *req = (request*)hConnection;
	
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_USER;
	case CA_CONFIG:

		// get wireless ap
		if (ictlScanAP((ICTL_HANDLE_T*)&req->httpPrivilege,ScanAp,&nLen,32) == ICTL_OK)
		{ // get ap information
			for(i = 0; i < nLen; i++)
			{
				int nQuality;
				AddHttpValue(pReturnXML, G_PC_ESSID, ScanAp[i].ssid);
				AddHttpValue(pReturnXML, G_PC_MODE, g_apcWlanOperationMode[(ScanAp[i].mode >= 0 && ScanAp[i].mode < sizeof(g_apcWlanOperationMode) / sizeof(const char *)) ? ScanAp[i].mode : 0]);
				AddHttpNum(pReturnXML, "Encode", (ScanAp[i].encode == 0 ? 0 : 1));
				
				//nQuality = ScanAp[i].noise + ScanAp[i].signal;
				//if (nQuality != 0)
				//	nQuality = ScanAp[i].signal * 100 / nQuality;
				nQuality = (int)(signed char)ScanAp[i].signal;
				/*
				RSSI >= -55                ==> Excellent
				-65 <= RSSI < -55      ==> Very Good
				-75 <= RSSI < -65      ==> Good
				-85 <= RSSI < -75      ==> Low
				RSSI < -85                   ==> Poor
				*/
				if (nQuality > 0)
					nQuality = 0;
				else if (nQuality >= -55)
					nQuality = 90 + 10 * (nQuality + 55) / 55;
				else if (nQuality >= -85)
					nQuality = 10 + 80 * (nQuality + 85) / (85 - 55);
				else
					nQuality = 10 * (nQuality + 128) / (128 - 85);

				AddHttpNum(pReturnXML, "Quality", nQuality);
				
				//AddHttpNum(pReturnXML, "Noise", (int)(signed char)ScanAp[i].noise);
			}
       	}
		return 0;
	}
	
	return -1;
}



void wsp_connecting_callback()
{
	ledSetNetwork(LED_NETWORK_TRY_CONNECT);
}

void wsp_connected_callback()
{
	ledSetNetwork(LED_NETWORK_READY);
}

void wsp_init_interface(const char *interface)
{
	InitNetInterface(interface);
}


void wsp_set_ip_addr(const char *interface, enum SET_IP_STATE_E *ip_state)
{
	SetIP(ip_state);
	upnp_refresh();
	
}


#endif


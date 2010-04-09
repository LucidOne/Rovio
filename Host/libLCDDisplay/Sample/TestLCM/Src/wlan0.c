#include "fontbypass.h"
#include "wlan0.h"
#ifdef DHCP

BOOL WRRunDHCPClient(const char *pcInterface, BOOL bWait)
{
	if (g_pidDhcpClient != 0) return TRUE;

	if (RunProgram(&g_pidDhcpClient, "/bin/dhcpc", pcInterface, NULL) == FALSE)
		return FALSE;

    if (bWait)
    {
        while (GetIPAddress (pcInterface) <= 0)
            tt_usleep (10000);
    }

	return TRUE;
}
BOOL RunProgram(volatile pid_t *ppidFork, const char *pcPath, ...)
{
	char *pcArg[20];
	int i;
	va_list arg;
	pid_t pidTmp;
	volatile BOOL bExecvSuccess = TRUE;

	if (ppidFork == NULL)
	{
		fprintf(stderr, "Call RunProgram(), no pid set.\n");
		ppidFork = &pidTmp;
	}

	pcArg[0] = (char *)pcPath;
	va_start(arg, pcPath);
	for (i = 1; i < sizeof(pcArg) / sizeof(const char *) - 1; i++)
	{
		pcArg[i] = va_arg(arg, char *);
		if (pcArg[i] == NULL) break;
	}
	pcArg[i + 1] = NULL;
	va_end(arg);

	if (vfork() == 0)
	{
		char acCygwinPath[128];
		*ppidFork = getpid();
		diag_printf("Before execv %s.\n", pcArg[0]);
		execv(pcArg[0], pcArg);
		sprintf(acCygwinPath, "%s.exe", pcPath);
		pcArg[0] = acCygwinPath;
		execv(pcArg[0], pcArg);
		diag_printf("execv %s error!\n", pcArg[0]);

		*ppidFork = 0;
		bExecvSuccess = FALSE;
		_exit(0);
	}

	return bExecvSuccess;
}
#endif
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

	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;
	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, pcInterface);

	if (pcWlanID == NULL)
	{//essid: any
		ac[0] = '\0';
		wrq.u.essid.flags = 0;
	}
	else
	{
		strncpy(ac, pcWlanID, sizeof(ac));
		wrq.u.essid.flags = 1;
	}

	wrq.u.essid.pointer = (caddr_t)ac;
	wrq.u.essid.length = strlen(ac) + 1;
	
	if((rt = ioctl(fd, SIOCSIWESSID, &wrq)) < 0)
	{
		diag_printf("scan ESSID TP_LINK failed\n");
	}
	diag_printf("set ssid finished\n");
	
	close(fd);
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
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return NULL;
	/* Get network ID */
	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, pcInterface);
	wrq.u.essid.pointer = (caddr_t)ac;
	wrq.u.essid.length = 0;
	wrq.u.essid.flags = 0;
	if(ioctl(fd, SIOCGIWESSID, &wrq) >= 0)
	{
		if (wrq.u.essid.flags == 0) pcWlanID[0] = '\0';
		else
		{
			if (wrq.u.essid.length < sizeof(ac))
				ac[wrq.u.essid.length] = '\0';
			else ac[sizeof(ac) - 1] = '\0';
			strncpy(pcWlanID, ac, iWlanIDMaxLen);
		}
	}
	else pcWlanID[0] = '\0';
	close(fd);
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
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;
	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, pcInterface);
	wrq.u.freq.e = 0;
	wrq.u.freq.m = iChannel;
	if((rt = ioctl(fd, SIOCSIWFREQ, &wrq)) <0)
		diag_printf("SetWlanChannel failed\n");
	close(fd);
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
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;
	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, pcInterface);
	rt = ioctl(fd, SIOCGIWFREQ, &wrq);
	close(fd);

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
	
	if (pcInterface == NULL) return FALSE;
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;	
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
	rt = ioctl(fd, SIOCSIWENCODE, &wrq);
	close(fd);
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
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;
	memset(&wrq, 0, sizeof(wrq));

	strcpy(wrq.ifr_name, pcInterface);
	wrq.u.data.pointer = (caddr_t)pucKey;
	wrq.u.data.length = 0;
	wrq.u.data.flags = 0;
	rt = ioctl(fd, SIOCGIWENCODE, &wrq);
    close(fd);
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
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;
	memset(&wrq, 0, sizeof(wrq));

	strcpy(wrq.ifr_name, pcInterface);
	wrq.u.mode = iMode;
	rt = ioctl(fd, SIOCSIWMODE, &wrq);
    close(fd);
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
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;

	memset(&wrq, 0, sizeof(wrq));

	strcpy(wrq.ifr_name, pcInterface);
	rt = ioctl(fd, SIOCGIWMODE, &wrq);
    close(fd);
    if (rt < 0
    	|| !(wrq.u.mode >= 0 && wrq.u.mode < sizeof(g_apcWlanOperationMode) / sizeof(const char *)))
    	return FALSE;

	if (piMode != NULL) *piMode = wrq.u.mode;
	return TRUE;
}

static BOOL ScanWlanAp(const char *pcInterface, int fd)
{
	struct iwreq wrq;

	if (pcInterface == NULL || fd < 0) return FALSE;

	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, pcInterface);
diag_printf("interface: %s\n", pcInterface);
	/* Get number of ap */
	if(ioctl(fd, 0x8B18, &wrq) < 0)
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
	if(ioctl(fd, SIOCGIWAPLIST, &wrq) < 0)
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
	if(ioctl(fd, SIOCGIWAPLIST, &wrq) < 0)
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
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return NULL;

	if (ScanWlanAp(pcInterface, fd))
	{
		pRt = ReadWlanESSIDList(pcInterface, fd, &iApNum);

		if (pRt == NULL) *piWlanESSIDNum = 0;
		else *piWlanESSIDNum = iApNum;
	}
	else PTE;
	close(fd);
	

	return pRt;
}


BOOL GetWlanLinkState(const char *pcInterface, int *piState)
{
	int fd;
	int rt;
	int iState;
	struct iwreq wrq;

	if (pcInterface == NULL || piState == NULL) return FALSE;
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;

	memset(&wrq, 0, sizeof(wrq));

	strcpy(wrq.ifr_name, pcInterface);
	wrq.u.data.pointer = (caddr_t)&iState;
   	wrq.u.data.length = sizeof(int);
   	rt = ioctl(fd, SIOCIWFIRSTPRIV + 0x9, &wrq);
   	close(fd);


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

#endif


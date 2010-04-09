#include "../../Inc/CommonDef.h"

#define HTTPC_MAX_CONNECT_RETRY 3
#define MAX_URLCGI 10

#ifdef USE_DDNS

//static char *psHostentList[4];
BOOL HTTPC_Process(FD_T *pFD, BOOL bReadable, BOOL bWritable);
struct tagHTTPC;
typedef BOOL (*HTTPC_RESPONSE)(struct tagHTTPC *pHttpFD, char *pcResponse, int iLen);
typedef void (*HTTPC_REQUEST_OVER)(struct tagHTTPC *pHttpFD);
typedef struct tagHTTPC
{
	OBJHEADER oHeader;
	FD_T fd;
	time_t tLastOp;
	time_t tConnTimeout;
	int iConnectRetry;
	HOST_INFO_T hostInfo;
	//char **ppcHost;
	in_addr_t addrHost;

	char *pcServer;
	char *pcPath;
	int iPort;
	char *pcHost;
	char *pcUserAgent;
	char *pcUser;
	char *pcPass;
	char *pcProxy;
	int iProxyPort;
	char *pcProxyUser;
	char *pcProxyPass;
	HTTPC_RESPONSE pfunOnResponse;
	HTTPC_REQUEST_OVER pfunOnRequestOver;
	int iLenTotal;
	int iLenOK;
	char acBuffer[1024];
	char *psHostentList[4];
} HTTPC;

static char *strdup(const char *ps)
{
	char *tmp;
	if (ps == NULL)
		return NULL;

	tmp = malloc(strlen(ps)+1);
	if (tmp == NULL)
		return NULL;

	strcpy(tmp, ps);
	return tmp;
}

static time_t mytime()
{
    time_t cur_sec;
    cyg_tick_count_t cur_time;
    cur_time = cyg_current_time();
    cur_sec = (cur_time*10) / 1000;
    return cur_sec;
}

/*********************************************************************
** HTTPC construction & destrucion
**	do not call them directly for ever!
** var vaInit:
**	const char *pcServer,
**	const char *pcPath,
**	int iPort,
**	const char *pcHost,
**	const char *pcUserAgent,
**	const char *pcUser,
**	const char *pcPass,
**	const char *pcProxy,
**	int iProxyPort,
**	const char *pcProxyUser,
**	const char *pcProxyPass,
**	HTTPC_RESPONSE pfunOnResponse,
**	HTTPC_REQUEST_OVER pfunOnRequestOver
*********************************************************************/
OBJHEADER *HTTPC_Init(OBJHEADER *pObj, va_list vaInit)
{
	HTTPC *pThis = (HTTPC *)pObj;

	const char *pcServer = va_arg(vaInit, const char *);
	const char *pcPath = va_arg(vaInit, const char *);
	int iPort = va_arg(vaInit, int);
	const char *pcHost = va_arg(vaInit, const char *);
	const char *pcUserAgent = va_arg(vaInit, const char *);
	const char *pcUser = va_arg(vaInit, const char *);
	const char *pcPass = va_arg(vaInit, const char *);
	const char *pcProxy = va_arg(vaInit, const char *);
	int iProxyPort = va_arg(vaInit, int);
	const char *pcProxyUser = va_arg(vaInit, const char *);
	const char *pcProxyPass = va_arg(vaInit, const char *);
	HTTPC_RESPONSE pfunOnResponse = va_arg(vaInit, HTTPC_RESPONSE);
	HTTPC_REQUEST_OVER pfunOnRequestOver = va_arg(vaInit, HTTPC_REQUEST_OVER);

	memset((char *)pThis + sizeof(OBJHEADER), 0, sizeof(HTTPC) - sizeof(OBJHEADER));

	pThis->tConnTimeout = 5;//timeout: 5 seconds
	pThis->iConnectRetry = 0;
	
	if (pcServer == NULL || pcServer[0] == '\0')
		goto lE_strdup_pcServer;
	pThis->pcServer = (char*)strdup(pcServer);
	if (pThis->pcServer == NULL) goto lE_strdup_pcServer;

	pThis->pcPath = (char*)strdup(pcPath);	
	if (pThis->pcPath == NULL) goto lE_strdup_pcPath;
	pThis->iPort = iPort;
	if (pcHost != NULL && pcHost[0] != '\0')
	{
		pThis->pcHost = (char*)strdup(pcHost);
		if (pThis->pcHost == NULL) goto lE_strdup_pcHost;
	}
	if (pcUserAgent != NULL && pcUserAgent[0] != '\0')
	{
		pThis->pcUserAgent = (char*)strdup(pcUserAgent);
		if (pThis->pcUserAgent == NULL) goto lE_strdup_pcUserAgent;
	}
	if (pcUser != NULL && pcUser[0] != '\0')
	{
		pThis->pcUser = (char*)strdup(pcUser);
		if (pThis->pcUser == NULL) goto lE_strdup_pcUser;
	}
	if (pcPass != NULL && pcPass[0] != '\0')
	{
		pThis->pcPass = (char*)strdup(pcPass);
		if (pThis->pcPass == NULL) goto lE_strdup_pcPass;
	}
	if (pcProxy != NULL && pcProxy[0] != '\0')
	{
		pThis->pcProxy = (char*)strdup(pcProxy);
		if (pThis->pcProxy == NULL) goto lE_strdup_pcProxy;
	}
	pThis->iProxyPort = iProxyPort;
	if (pcProxyUser != NULL && pcProxyUser[0] != '\0')
	{
		pThis->pcProxyUser = (char*)strdup(pcProxyUser);
		if (pThis->pcProxyUser == NULL) goto lE_strdup_pcProxyUser;
	}
	if (pcProxyPass != NULL && pcProxyPass[0] != '\0')
	{
		pThis->pcProxyPass = (char*)strdup(pcProxyPass);
		if (pThis->pcProxyPass == NULL) goto lE_strdup_pcProxyPass;
	}
	pThis->pfunOnResponse = pfunOnResponse;
	pThis->pfunOnRequestOver = pfunOnRequestOver;


	if (ObjInit((OBJHEADER *)pThis, sizeof(FD_T), (OBJHEADER *)&pThis->fd, FD_Final, FD_Init,
		-1, SE_TP_HTTPC, ST_NET_INIT, 0, FALSE, FALSE, HTTPC_Process) == NULL)
		goto lE_FD_Init;


	PTI;
	return pObj;

lE_FD_Init:
	if (pThis->pcProxyPass != NULL) free(pThis->pcProxyPass);
lE_strdup_pcProxyPass:
	if (pThis->pcProxyUser != NULL) free(pThis->pcProxyUser);
lE_strdup_pcProxyUser:
	if (pThis->pcProxy != NULL) free(pThis->pcProxy);
lE_strdup_pcProxy:
	if (pThis->pcPass != NULL) free(pThis->pcPass);
lE_strdup_pcPass:
	if (pThis->pcUser != NULL) free(pThis->pcUser);
lE_strdup_pcUser:
	if (pThis->pcUserAgent != NULL) free(pThis->pcUserAgent);
lE_strdup_pcUserAgent:
	if (pThis->pcHost != NULL)free(pThis->pcHost);
lE_strdup_pcHost:
	if (pThis->pcPath != NULL)free(pThis->pcPath);
lE_strdup_pcPath:
	if (pThis->pcServer != NULL)free(pThis->pcServer);
lE_strdup_pcServer:
	return NULL;
}

void HTTPC_Final(OBJHEADER *pObj)
{
	HTTPC *pThis = (HTTPC *)pObj;

	PTI;

	ObjFinal((OBJHEADER *)&pThis->fd);

	if (pThis->pcServer != NULL)
		free(pThis->pcServer);
	if (pThis->pcPath != NULL)
		free(pThis->pcPath);
	if (pThis->pcHost != NULL)
		free(pThis->pcHost);
	if (pThis->pcUserAgent != NULL)
		free(pThis->pcUserAgent);
	if (pThis->pcUser != NULL)
		free(pThis->pcUser);
	if (pThis->pcPass != NULL)
		free(pThis->pcPass);
	if (pThis->pcProxy != NULL)
		free(pThis->pcProxy);
	if (pThis->pcProxyUser != NULL)
		free(pThis->pcProxyUser);
	if (pThis->pcProxyPass != NULL)
		free(pThis->pcProxyPass);
}


BOOL HTTPC_Process(FD_T *pFD, BOOL bReadable, BOOL bWritable)
{
	int iLen;
	HTTPC *pHttpFD;
	
	
	pHttpFD = (HTTPC *)((char *)(pFD) - (unsigned long)(&((HTTPC *)0)->fd));


	if (ST_NET_INIT == pFD->iState)
	{
		//PTI;
		diag_printf("pcProxy is %s(%d),pcServer is %s(%d)\n",
			pHttpFD->pcProxy, pHttpFD->iProxyPort,
			pHttpFD->pcServer, pHttpFD->iPort
			);
		ResolveHost(&pHttpFD->hostInfo, (pHttpFD->pcProxy == NULL ? pHttpFD->pcServer : pHttpFD->pcProxy));
		if (pHttpFD->hostInfo.psHostent == NULL
			|| pHttpFD->hostInfo.psHostent->h_addr_list == NULL
			|| pHttpFD->hostInfo.psHostent->h_addr_list[0] == NULL)
		{
			FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
			return TRUE;
		}
		else
		{
#ifndef WLAN
			pHttpFD->psHostentList[0] = pHttpFD->hostInfo.psHostent->h_addr_list0;
			pHttpFD->psHostentList[1] = pHttpFD->hostInfo.psHostent->h_addr_list1;
			pHttpFD->psHostentList[2] = pHttpFD->hostInfo.psHostent->h_addr_list2;
			pHttpFD->psHostentList[3] = pHttpFD->hostInfo.psHostent->h_addr_list3;
			pHttpFD->ppcHost = pHttpFD->psHostentList;
#else
			//pHttpFD->ppcHost = pHttpFD->hostInfo.psHostent->h_addr_list;
			pHttpFD->addrHost = *(unsigned long*)pHttpFD->hostInfo.psHostent->h_addr_list[0];
#endif
			FD_Next(pFD, ST_NET_CONNECT, FALSE, FALSE, 0);
			/*
			printf("%d %d\n", pHttpFD->ppcHost, *(unsigned long*)*pHttpFD->ppcHost);
			{
				struct in_addr in;
				memcpy(&in, *pHttpFD->ppcHost, 4);
				printf("addr=%s\n", inet_ntoa(in,g_Select_Buf, SELECT_BUFFER_LEN));
			}
			*/
			return TRUE;
		}
	}
	else if (ST_NET_CONNECT == pFD->iState)
	{
		int iFl;
		struct sockaddr_in sin;
		diag_printf("start fd is %d\n",pHttpFD);
		//PTI;
		if (pFD->iFD != -1)
		{

#ifndef WLAN
			netclose(pFD->iFD,g_Select_Buf, SELECT_BUFFER_LEN);
#else
			close(pFD->iFD);
#endif
			pFD->iFD = -1;
		}
		
		if (++pHttpFD->iConnectRetry > HTTPC_MAX_CONNECT_RETRY)
		{
			diag_printf("Close connection since retry %d times\n", HTTPC_MAX_CONNECT_RETRY);
			FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
			return TRUE;		
		}
		
		//if (*pHttpFD->ppcHost == NULL)
		//{
		//	FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
		//	return TRUE;
		//}
	#ifndef WLAN
		pFD->iFD = socket(PF_INET,SOCK_STREAM,0,g_Select_Buf, SELECT_BUFFER_LEN);
	#else
		pFD->iFD = socket(PF_INET,SOCK_STREAM,0);	
	#endif
		if (pFD->iFD < 0)
		{
			diag_printf("Can not create socket!\n");
			FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
			return TRUE;
		}

		//PTI;
		/* set socket to O_NDELAY */
	#ifndef WLAN
		iFl = netfcntl(pFD->iFD, 3, 0,g_Select_Buf, SELECT_BUFFER_LEN);
		if (netfcntl(pFD->iFD, 4, iFl | 0x800,g_Select_Buf, SELECT_BUFFER_LEN) < 0)
	#else
		iFl = fcntl(pFD->iFD, F_GETFL, 0);
		//if (fcntl(pFD->iFD, F_SETFL, iFl | O_NDELAY) != 0)
		iFl = 1;
		if(ioctl(pFD->iFD,FIONBIO,&iFl) != 0)
	#endif
		{
			diag_printf("Can not set socket fd to O_NDELAY mode.\n");
			FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
			return TRUE;
		}

		//PTI;
		/* Connect to proxy */
		/*
		{
				struct in_addr in;
				memcpy(&in, *pHttpFD->ppcHost, 4);
				printf("addr=%s\n", inet_ntoa(in,g_Select_Buf, SELECT_BUFFER_LEN));
		}*/
		sin.sin_family = AF_INET;
		{
			int iPort = (pHttpFD->pcProxy == NULL ? pHttpFD->iPort : pHttpFD->iProxyPort);
			if (iPort == 0)
				iPort = 80;
			sin.sin_port = htons(iPort);	//http port number
		}
		//sin.sin_addr.s_addr = *(unsigned long*)*pHttpFD->ppcHost;
		sin.sin_addr.s_addr = pHttpFD->addrHost;
		diag_printf("sin.sin_addr.s_addr = %x\n", sin.sin_addr.s_addr);
		//pHttpFD->ppcHost++;
		//PTI;
		/*		
		{
			unsigned char * p;
			int i;
			p = (unsigned char *)(struct sockaddr *)&sin;
			for(i = 0 ; i < 16; i++)
			{
				printf("sockaddr is %02x\n", (int)*p);
				p++;
			}
		}*/
	#ifndef WLAN
		connect(pFD->iFD,(struct sockaddr *)&sin,sizeof(struct sockaddr),g_Select_Buf, SELECT_BUFFER_LEN);
	#else
		connect(pFD->iFD,(struct sockaddr *)&sin,sizeof(struct sockaddr));	
	#endif
		
		FD_Next(pFD, ST_NET_CONNECTING, TRUE, TRUE, pHttpFD->tConnTimeout * 1000);
		pHttpFD->tLastOp = mytime();
		return TRUE;
	}
	else if (ST_NET_CONNECTING == pFD->iState)
	{
		//PTI;
		if (!(bReadable || bWritable))
		{
			time_t tThisOp = mytime();
			if (tThisOp - pHttpFD->tLastOp > pHttpFD->tConnTimeout)
			{
				FD_Next(pFD, ST_NET_CONNECT, FALSE, FALSE, 0);
			}
			return TRUE;
		}
		else
		{
			int iErrorCode;
			int iErrorCodeLen = sizeof(iErrorCode);
#ifndef WLAN
			if (getsockopt(pFD->iFD, SOL_SOCKET, SO_ERROR, &iErrorCode, &iErrorCodeLen,g_Select_Buf, SELECT_BUFFER_LEN) >= 0)

#else
			if (getsockopt(pFD->iFD, SOL_SOCKET, SO_ERROR, &iErrorCode, (unsigned int *)&iErrorCodeLen) == 0)

#endif
			{
				if (iErrorCode == 0)
				{
					int iLen = 0;
					diag_printf("%s: connection OK.\n", pHttpFD->pcServer);
					pHttpFD->tLastOp = mytime();

					FD_Next(pFD, ST_NET_WRITE, FALSE, TRUE, pHttpFD->tConnTimeout * 1000);

					if (pHttpFD->pcProxy != NULL)
					{
						char acPort[32];
						if (pHttpFD->iPort == 80 || pHttpFD->iPort == 0)
							acPort[0] = '\0';
						else
							sprintf(acPort, ":%d", pHttpFD->iPort);
						iLen = sprintf(pHttpFD->acBuffer,
								"GET HTTP://%s%s%s HTTP/1.1\r\n",
								pHttpFD->pcServer,
								acPort,
								pHttpFD->pcPath);
					}
					else
						iLen = sprintf(pHttpFD->acBuffer,
								"GET %s HTTP/1.1\r\n",
								pHttpFD->pcPath);

					if (pHttpFD->pcHost != NULL)
						iLen += sprintf(pHttpFD->acBuffer + iLen,
								"HOST: %s\r\n",
								pHttpFD->pcHost);
					if (pHttpFD->pcUserAgent != NULL)
						iLen += sprintf(pHttpFD->acBuffer + iLen,
								"User-Agent: %s\r\n",
								pHttpFD->pcUserAgent);
					if (pHttpFD->pcUser != NULL || pHttpFD->pcPass != NULL)
					{
						char ac[256];
						int iInLen, iOutLen;
						//PTI;
						iInLen = sprintf(pHttpFD->acBuffer + iLen, "%s%s%s",
							(pHttpFD->pcUser == NULL ? "" : pHttpFD->pcUser),
							((pHttpFD->pcUser == NULL && pHttpFD->pcPass == NULL)? "" : ":"),
							(pHttpFD->pcPass == NULL ? "" : pHttpFD->pcPass));
						if (EncodeBase64(pHttpFD->acBuffer + iLen, iInLen, ac, sizeof(ac), &iOutLen))
						{
							ac[iOutLen - 2] = '\0';//cut \r\n
							iLen += sprintf(pHttpFD->acBuffer + iLen,
								"Authorization: Basic %s\r\n",
								ac);
						}
					}
					if (pHttpFD->pcProxy != NULL)
					{
						char ac[256];
						int iInLen, iOutLen;
						if (pHttpFD->pcProxyUser == NULL && pHttpFD->pcProxyPass == NULL)
							ac[0] = '\0';
						else
						{
							//PTI;
							iInLen = sprintf(pHttpFD->acBuffer + iLen, "%s%s%s",
								(pHttpFD->pcProxyUser == NULL ? "" : pHttpFD->pcProxyUser),
								((pHttpFD->pcProxyUser == NULL && pHttpFD->pcProxyPass == NULL)? "" : ":"),
								(pHttpFD->pcProxyPass == NULL ? "" : pHttpFD->pcProxyPass));
							if (EncodeBase64(pHttpFD->acBuffer + iLen, iInLen, ac, sizeof(ac), &iOutLen))
							{
								ac[iOutLen - 2] = '\0';//cut \r\n
							}
							else ac[0] = '\0';
							//PTI;
						}
						iLen += sprintf(pHttpFD->acBuffer + iLen,
									"Proxy-Authorization: Basic %s\r\n",
									ac);
					}
					iLen += sprintf(pHttpFD->acBuffer + iLen,
						"ACCEPT: */*\r\n"
						"Pragma: no-cache\r\n"
						"Cache-Control: no-cache\r\n"
						"Connection: close\r\n\r\n");
					//PTI;
					diag_printf("%s", pHttpFD->acBuffer);
					pHttpFD->iLenTotal = iLen;
					pHttpFD->iLenOK = 0;

					return TRUE;
				}
			}
			FD_Next(pFD, ST_NET_CONNECT, FALSE, FALSE, 0);
			return TRUE;
		}

	}
	else if (ST_NET_WRITE == pFD->iState)
	{
		PTE;
		if (bWritable)
		{
			PTE;
#ifndef WLAN
			iLen = netwrite(pFD->iFD, pHttpFD->acBuffer + pHttpFD->iLenOK, pHttpFD->iLenTotal - pHttpFD->iLenOK,g_Select_Buf, SELECT_BUFFER_LEN);

#else
			iLen = write(pFD->iFD, pHttpFD->acBuffer + pHttpFD->iLenOK, pHttpFD->iLenTotal - pHttpFD->iLenOK);
		
#endif
			PTE;
			if (iLen > 0)
			{
				pHttpFD->tLastOp = mytime();
				pHttpFD->iLenOK += iLen;
				if (pHttpFD->iLenOK >= pHttpFD->iLenTotal)
				{
					pHttpFD->iLenOK = 0;
					pHttpFD->iLenTotal = 0;
					FD_Next(pFD, ST_NET_READ, TRUE, FALSE, pHttpFD->tConnTimeout * 1000);
				}
				//diag_printf("Write len ==============%d,%s\n",iLen,pHttpFD->acBuffer + pHttpFD->iLenOK);
				return TRUE;
			}
			else if (!(errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR))
			{
				FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
				return TRUE;
			}
		}
		if (mytime() - pHttpFD->tLastOp > pHttpFD->tConnTimeout)
		{
			FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
			return TRUE;
		}
		return TRUE;
	}
	else if (ST_NET_READ == pFD->iState)
	{
int errno11;
		time_t tThisOp = mytime();
		if (tThisOp - pHttpFD->tLastOp > pHttpFD->tConnTimeout)
		{
			if (pHttpFD->pfunOnResponse != NULL)
				(*pHttpFD->pfunOnResponse)(pHttpFD, pHttpFD->acBuffer, pHttpFD->iLenOK);
			FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
			return TRUE;
		}

		if (!bReadable) return TRUE;
#ifndef WLAN
			iLen = netread(pFD->iFD, pHttpFD->acBuffer + pHttpFD->iLenOK, sizeof(pHttpFD->acBuffer) / 2,g_Select_Buf, SELECT_BUFFER_LEN);

#else
			iLen = read(pFD->iFD, pHttpFD->acBuffer + pHttpFD->iLenOK, sizeof(pHttpFD->acBuffer) / 2);

#endif

	
errno11 = errno;
diag_printf("Read len for server11 =========%d, %d\n",iLen, errno11);		
		if (iLen > 0)
		{
			//diag_printf("Read len for server =========%d,%s\n",iLen,pHttpFD->acBuffer + pHttpFD->iLenOK);
			pHttpFD->iLenOK += iLen;
			diag_printf("iLenOK===%d,sizeof(pHttpFD->acBuffer) / 2===%d\n",pHttpFD->iLenOK,sizeof(pHttpFD->acBuffer) / 2);
			if (pHttpFD->iLenOK >= sizeof(pHttpFD->acBuffer) / 2)
			{
				if (pHttpFD->pfunOnResponse != NULL)
				{
					if (!(*pHttpFD->pfunOnResponse)(pHttpFD, pHttpFD->acBuffer, pHttpFD->iLenOK))
					{
						FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
						return TRUE;
					}
				}
				pHttpFD->iLenOK = 0;
			}
			pHttpFD->tLastOp = tThisOp;
		}
		else
		{
			diag_printf("read errno====================== %d\n", errno11);
			if (!(errno11 == EINTR || errno11 == EAGAIN || errno11 == EWOULDBLOCK))
			{
				if (pHttpFD->pfunOnResponse != NULL)
					(*pHttpFD->pfunOnResponse)(pHttpFD, pHttpFD->acBuffer, pHttpFD->iLenOK);

				FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
				return TRUE;
			}
		}
		return TRUE;
	}
	else if (ST_NET_OVER == pFD->iState)
	{
		diag_printf("HTTP request is over\n");
		if (pHttpFD->pfunOnRequestOver != NULL)
			(*pHttpFD->pfunOnRequestOver)(pHttpFD);
		if (pFD->iFD != -1) 
#ifndef WLAN
			netclose(pFD->iFD,g_Select_Buf, SELECT_BUFFER_LEN);
#else
			close(pFD->iFD);
#endif
		DeleteHost(&pHttpFD->hostInfo);
		return FALSE;
	}
	else return FALSE;
}



/*********************************************************************
** HTTPC construction for CheckIP service
**	do not call them directly for ever!
** var vaInit:
**	int iFD, int iType, int iState,
**	BOOL bWantRead, BOOL bWantWrite,
**	BOOL (*pfunProcessFD)(struct tagFD_T *pFD, BOOL bReadable, BOOL bWritable)
*********************************************************************/
typedef enum
{
	RS_LINE,
	RS_CR,
	RS_LF2,
	RS_CR2,
	RS_FOUND
} RESPONSE_E;


typedef struct
{
	HTTPC oHTTPC;
	char acBody[64];
	size_t szBodyLen;
	RESPONSE_E eResState;
} CKIP_HTTPC;


void DDNS_Update_IP(UINT32 uIP);


OBJHEADER *CkIP_HTTPC_Init(OBJHEADER *pObj, va_list vaInit)
{
	OBJHEADER *pRt;
	pRt = HTTPC_Init(pObj, vaInit);
	if (pRt != NULL)
	{
		((HTTPC *)pRt)->fd.iType = SE_TP_CKIP_HTTPC;
		((CKIP_HTTPC *)pRt)->szBodyLen = 0;
		((CKIP_HTTPC *)pRt)->eResState = RS_LINE;
	}
	return pRt;
}


static void CkIP_RequestOver(CKIP_HTTPC *pHttpFD)
{
	if (g_WebCamState.iDDNSState != DDNS_CHECK_IP_OK)
		g_WebCamState.iDDNSState = DDNS_UPDATE_FAIL;
	diag_printf("IP checked, do dyndns now\n");
}

static BOOL CkIP_Response_dyndns(CKIP_HTTPC *pHttpFD, char *acResponse, int iLen)
{
	char *pc = acResponse;
	while (pc < acResponse + iLen)
	{
		switch (pHttpFD->eResState)
		{
			case RS_LINE:
				if (*pc == '\n')
					pHttpFD->eResState = RS_CR;
				break;
			case RS_CR:
				if (*pc == '\r')
					pHttpFD->eResState = RS_LF2;
				else if (*pc == '\n')
					pHttpFD->eResState = RS_CR2;
				else
					pHttpFD->eResState = RS_LINE;
				break;
			case RS_LF2:
				if (*pc == '\n')
					pHttpFD->eResState = RS_CR2;
				else
					pHttpFD->eResState = RS_LINE;
				break;
			case RS_CR2:
				if (*pc == ':')
					pHttpFD->eResState = RS_FOUND;
				break;
			case RS_FOUND:
				if (pHttpFD->szBodyLen + 1< sizeof(pHttpFD->acBody))
				{					
					if (!
						((*pc >= '0' && *pc <= '9') || *pc == '.' || *pc == '\t' || *pc == ' ')
						)
					{
						UINT32 ulPublicIpAddress = httpString2IP(pHttpFD->acBody);
						diag_printf("Found1[%s][%x]\n", pHttpFD->acBody, ulPublicIpAddress);

						g_WebCamState.iDDNSState = DDNS_CHECK_IP_OK;
						DDNS_Update_IP(ulPublicIpAddress);
						return FALSE;
					}
					else
					{
						pHttpFD->acBody[pHttpFD->szBodyLen++] = *pc;
						pHttpFD->acBody[pHttpFD->szBodyLen] = '\0';
					}
				}
				else
					return FALSE;
				break;
			default:
				return FALSE;	/* Can not go to here. */
		}
		
		++pc;
	}
	return TRUE;
}


static BOOL CkIP_Response_dnsomatic(CKIP_HTTPC *pHttpFD, char *acResponse, int iLen)
{
	char *pc = acResponse;
	while (pc < acResponse + iLen)
	{
		//diag_printf("R: [%c] %d\n", *pc, pHttpFD->eResState);
		switch (pHttpFD->eResState)
		{
			case RS_LINE:
				if (*pc == '\n')
					pHttpFD->eResState = RS_CR;
				break;
			case RS_CR:
				if (*pc == '\r')
					pHttpFD->eResState = RS_LF2;
				else if (*pc == '\n')
					pHttpFD->eResState = RS_CR2;
				else
					pHttpFD->eResState = RS_LINE;
				break;
			case RS_LF2:
				if (*pc == '\n')
					pHttpFD->eResState = RS_CR2;
				else
					pHttpFD->eResState = RS_LINE;
				break;
			case RS_CR2:
				if (pHttpFD->szBodyLen + 1< sizeof(pHttpFD->acBody))
				{					
					if (!
						((*pc >= '0' && *pc <= '9') || *pc == '.' || *pc == '\t' || *pc == ' ')
						)
					{
						UINT32 ulPublicIpAddress = httpString2IP(pHttpFD->acBody);
						diag_printf("Found2[%s][%x]\n", pHttpFD->acBody, ulPublicIpAddress);

						g_WebCamState.iDDNSState = DDNS_CHECK_IP_OK;
						DDNS_Update_IP(ulPublicIpAddress);
						return FALSE;
					}
					else
					{
						pHttpFD->acBody[pHttpFD->szBodyLen++] = *pc;
						pHttpFD->acBody[pHttpFD->szBodyLen] = '\0';
					}
				}
				else
					return FALSE;
				break;
			default:
				return FALSE;	/* Can not go to here. */
		}
		
		++pc;
	}
	return TRUE;
}




/*********************************************************************
** HTTPC construction for DDNS
**	do not call them directly for ever!
** var vaInit:
**	int iFD, int iType, int iState,
**	BOOL bWantRead, BOOL bWantWrite,
**	BOOL (*pfunProcessFD)(struct tagFD_T *pFD, BOOL bReadable, BOOL bWritable)
*********************************************************************/
typedef struct
{
	HTTPC oHTTPC;
	char acBody[64];
	size_t szBodyLen;
	RESPONSE_E eResState;
} DDNS_HTTPC;

OBJHEADER *DDNS_HTTPC_Init(OBJHEADER *pObj, va_list vaInit)
{
	OBJHEADER *pRt;
	pRt = HTTPC_Init(pObj, vaInit);
	if (pRt != NULL)
	{
		((HTTPC *)pRt)->fd.iType = SE_TP_DDNSHTTPC;
		((DDNS_HTTPC *)pRt)->szBodyLen = 0;
		((DDNS_HTTPC *)pRt)->eResState = RS_LINE;
	}	
	return pRt;
}


static void DDNSRequestOver(DDNS_HTTPC *pHttpFD)
{
	if (g_WebCamState.iDDNSState != DDNS_UPDATE_OK)
		g_WebCamState.iDDNSState = DDNS_UPDATE_FAIL;
}

static BOOL DDNSResponse(DDNS_HTTPC *pHttpFD, char *acResponse, int iLen)
{
#if 1
	char *pc = acResponse;
	while (pc < acResponse + iLen)
	{
		//diag_printf("R:%c, st: %d\n", *pc, pHttpFD->eResState);
		switch (pHttpFD->eResState)
		{
			case RS_LINE:
				if (*pc == '\n')
					pHttpFD->eResState = RS_CR;
				break;
			case RS_CR:
				if (*pc == '\r')
					pHttpFD->eResState = RS_LF2;
				else if (*pc == '\n')
					pHttpFD->eResState = RS_CR2;
				else
					pHttpFD->eResState = RS_LINE;
				break;
			case RS_LF2:
				if (*pc == '\n')
					pHttpFD->eResState = RS_CR2;
				else
					pHttpFD->eResState = RS_LINE;
				break;
			case RS_CR2:
				if (pHttpFD->szBodyLen + 1< sizeof(pHttpFD->acBody))
				{					
					pHttpFD->acBody[pHttpFD->szBodyLen++] = *pc;
					pHttpFD->acBody[pHttpFD->szBodyLen] = '\0';
					if (memcmp(pHttpFD->acBody, "nochg", 5) == 0
						|| memcmp(pHttpFD->acBody, "good", 4) == 0)
					{
						g_WebCamState.iDDNSState = DDNS_UPDATE_OK;
						return FALSE;
					}


				}
				else
					return FALSE;
				break;
			default:
				return FALSE;	/* Can not go to here. */
		}
		
		++pc;
	}
	return TRUE;
#else
	char *pc = NULL;

	//write(1, acResponse, iLen);
	diag_printf("Return from server:%s\n",acResponse);
	acResponse[iLen] = '\0';
	pc = strstr(acResponse, "\n\r\n");
	if (pc != NULL) pc += 3;
	else
	{
		pc = strstr(acResponse, "\n\n");
		if (pc != NULL) pc += 2;
	}

	if (pc != NULL)
	{
		diag_printf("[%s]\n", pc);
		if (memcmp(pc, "nochg", 5) == 0
			|| memcmp(pc, "good", 4) == 0)
		{
			//PTE;
			g_WebCamState.iDDNSState = DDNS_UPDATE_OK;
		}
	}
	return TRUE;
#endif
}


static void *AddDDNS(void *pThread, void *pParam)
{
	LISTNODE *pNode;
	LISTNODE *pNextNode;
	FD_T *pNewFD = (FD_T *)pParam;
	SELECT_THREAD_T *pThreadData = (SELECT_THREAD_T *)pThread;

	for (pNode = pThreadData->pFDList->pFirstNode;
		pNode != pThreadData->pFDList->pLastNode;
		pNode = pNextNode)
	{
		FD_T *pFD = (FD_T *)pNode->pValue;
		pNextNode = pNode->pNextNode;

		if (pFD == NULL) continue;
		if (pFD->iType == SE_TP_DDNSHTTPC || pFD->iType == SE_TP_CKIP_HTTPC)
		{
			diag_printf("Delete\n");
			ObjFinal((OBJHEADER *)pFD);
			httpDeleteNode(pNode);
		}
	}

	g_WebCamState.iDDNSState = DDNS_UPDATTING;
	AddSelectFD(pNewFD);
	return NULL;
}


void Do_DNS(int eDDNSType,
			const char *pcHost,
			unsigned long ulIP,
			const char *pcUser,
			const char *pcPass,
			const char *pcProxy,
			int iProxyPort,
			const char *pcProxyUser,
			const char *pcProxyPass)
{
	HTTPC *pHttpC;
	const char *pcServer;
	const char *pcPath;
	const char *pcAgent;
	char acIP[24];
	char acPath[256];

	diag_printf("Do_DNS: type: %d\n", eDDNSType);
	switch (eDDNSType)
	{
		case DDNS_TYPE_DYNDNS:
			pcServer = "members.dyndns.org";
			pcPath = "/nic/update?system=dyndns&hostname=%s&myip=%s&wildcard=NO&backmx=NO&offline=NO&mx=";
			pcAgent = "Rovio Update Client v1.0 roviotech@wowwee.com.hk";
			break;
		case DDNS_TYPE_NOIP:
			pcServer = "dynupdate.no-ip.com";
			pcPath = "/nic/update?hostname=%s&myip=%s";
			pcAgent = "Rovio Update Client v1.0 roviotech@wowwee.com.hk";
			break;
		case DDNS_TYPE_DNSOMATIC:
			pcServer = "updates.dnsomatic.com";
			pcPath = "/nic/update?system=dyndns&hostname=%s&myip=%s&wildcard=NO&backmx=NO&offline=NO&mx=";
			pcAgent = "wowwee Rovio Update Client v1.0";
			break;
		default:
			return;
	}
	


	if (ulIP == 0L)
	{
		unsigned long ulPublicSubnetMask;
		unsigned long ulDefaultGateway;
		unsigned long aulDNSServer[3];
		GetPubIPInfo(&ulIP,&ulPublicSubnetMask,&ulDefaultGateway,aulDNSServer);
	}
	if (ulIP == 0L)
	{
		diag_printf("No IP!\n");
		return;
	}

	if (pcHost == NULL || pcHost[0] == '\0') return;
	if (pcProxy != NULL && pcProxy[0] == '\0') pcProxy = NULL;

	httpIP2String(ulIP, acIP);
	if (strlen(pcPath) + strlen(acIP) + strlen(pcHost) > sizeof(acPath))
	{
		diag_printf("DDNS: Too long host name!\n");
		return;
	}
	sprintf(acPath, pcPath, pcHost, acIP);

	diag_printf("do_dns proxy is %s,server is %s\n",pcProxy,pcServer);
	diag_printf("do_dns path is [%s]\n", acPath);
	if ((pHttpC = (HTTPC *)ObjInit(NULL, sizeof(DDNS_HTTPC), NULL,
					HTTPC_Final, DDNS_HTTPC_Init,
					pcServer,
					acPath,
					80,
					pcServer,
					pcAgent,
					pcUser,
					pcPass,
					pcProxy,
					iProxyPort,
					pcProxyUser,
					pcProxyPass,
					DDNSResponse,
					DDNSRequestOver)) == NULL) return;
	InsertCustomProcess(g_pMsgSelect, &AddDDNS, (void *)&pHttpC->fd);
}


void DDNS_Do_CheckIP(
	int eDDNSType,
	const char *pcProxy,
	int iProxyPort,
	const char *pcProxyUser,
	const char *pcProxyPass
	)
{
	CKIP_HTTPC *pHttpC;
	const char *pcServer;
	BOOL (*fnCkIP_Response)(CKIP_HTTPC *pHttpFD, char *acResponse, int iLen);


	diag_printf("DDNS_Do_CheckIP: type: %d\n", eDDNSType);
	switch(eDDNSType)
	{
	case DDNS_TYPE_DYNDNS:
		pcServer = "checkip.dyndns.com";
		fnCkIP_Response = &CkIP_Response_dyndns;
		break;
	case DDNS_TYPE_NOIP:
		pcServer = "checkip.dyndns.com";
		fnCkIP_Response = &CkIP_Response_dyndns;
		break;
	case DDNS_TYPE_DNSOMATIC:
		pcServer = "myip.dnsomatic.com";
		fnCkIP_Response = &CkIP_Response_dnsomatic;
		break;
	default:
		return;
	}

	if (pcProxy != NULL && pcProxy[0] == '\0') pcProxy = NULL;

	diag_printf("DDNS_CheckIP proxy is %s,server is %s\n",pcProxy,pcServer);
	if ((pHttpC = (CKIP_HTTPC *)ObjInit(NULL, sizeof(CKIP_HTTPC), NULL,
					HTTPC_Final, CkIP_HTTPC_Init,
					pcServer,
					"/",
					80,
					pcServer,
					"Mozilla/4.0 (compatible; MSIE 5.00; Windows 98)",
					NULL,
					NULL,
					pcProxy,
					iProxyPort,
					pcProxyUser,
					pcProxyPass,
					fnCkIP_Response,
					CkIP_RequestOver)) == NULL) return;
	InsertCustomProcess(g_pMsgSelect, &AddDDNS, (void *)&pHttpC->oHTTPC.fd);
}


void DDNS_CheckIP()
{
	cyg_mutex_lock(&g_ptmConfigParam);

	//update dynamic DNS if possible
	diag_printf("proxy server is %s\n",g_ConfigParam.acProxy);
	
	DDNS_Do_CheckIP(g_ConfigParam.eDDNSType,
			g_ConfigParam.acProxy,
			g_ConfigParam.iProxyPort,
			g_ConfigParam.acProxyUser,
			g_ConfigParam.acProxyPass);
	cyg_mutex_unlock(&g_ptmConfigParam);	
}


int Config_GetDDNS(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	const char *pcDDNSState;
	const char *pcService;
	char ac[128];
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
		break;
	case CA_CONFIG:
diag_printf("ddns : %d\n", g_WebCamState.iDDNSState);
		switch (g_WebCamState.iDDNSState)
		{
		case DDNS_UPDATE_OK:
			pcDDNSState = "Updated";
			break;
		case DDNS_UPDATE_FAIL:
			pcDDNSState = "Updating Failed";
			break;
		case DDNS_UPDATTING:
			pcDDNSState = "Updating";
			break;
		case DDNS_CHECK_IP_OK:
			pcDDNSState = "IP Checked";
			break;
		default:
			pcDDNSState = "Not Update";
			break;
		}
		
		switch (g_ConfigParam.eDDNSType)
		{
		case DDNS_TYPE_DYNDNS:
			pcService = "dyndns";
			break;
		case DDNS_TYPE_NOIP:
			pcService = "no-ip";
			break;
		case DDNS_TYPE_DNSOMATIC:
			pcService = "dnsomatic";
			break;
		default:
			pcService = "";
		}
		
		httpLong2String((int)g_ConfigParam.bEnableDDNS, ac);
		AddHttpValue(pReturnXML, G_PC_ENABLE, ac);
		AddHttpValue(pReturnXML, "Service", pcService);
		AddHttpValue(pReturnXML, G_PC_USER, g_ConfigParam.acDDNSUserName);
		AddHttpValue(pReturnXML, G_PC_PASS, g_ConfigParam.acDDNSUserPass);
		AddHttpValue(pReturnXML, G_PC_DOMAINNAME, g_ConfigParam.acDDNSDomainName);
		{
			httpIP2String(g_ConfigParam.ulDDNSIP, ac);
			AddHttpValue(pReturnXML, "IP", ac);
		}
		AddHttpValue(pReturnXML, G_PC_PROXY, g_ConfigParam.acProxy);
		AddHttpNum(pReturnXML, "ProxyPort", g_ConfigParam.iProxyPort);
		AddHttpValue(pReturnXML, G_PC_PROXYUSER, g_ConfigParam.acProxyUser);
		AddHttpValue(pReturnXML, G_PC_PROXYPASS, g_ConfigParam.acProxyPass);
		AddHttpValue(pReturnXML, G_PC_INFO, pcDDNSState);
		return 0;
		break;
	}
	return -1;
}

int Config_SetDDNS(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	BOOL bEnableDDNS;
	const char *pcService;
	int eDDNSType;
	const char *pcDDNSUserName;
	const char *pcDDNSUserPass;
	const char *pcDDNSDomainName;
	const char *pcProxy;
	int iProxyPort;
	const char *pcProxyUser;
	const char *pcProxyPass;
	unsigned long ulIP;

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
		break;
	case CA_CONFIG:
		bEnableDDNS = httpGetBool(pParamList, "Enable");

		pcService = httpGetString(pParamList, "Service");

		if (strcmp(pcService, "dyndns") == 0)
			eDDNSType = DDNS_TYPE_DYNDNS;
		else if (strcmp(pcService, "no-ip") == 0)
			eDDNSType = DDNS_TYPE_NOIP;
		else if (strcmp(pcService, "dnsomatic") == 0)
			eDDNSType = DDNS_TYPE_DNSOMATIC;
		else
			eDDNSType = DDNS_TYPE_UNSUPPORTED;

		pcDDNSUserName = httpGetString(pParamList, "User");			
		pcDDNSUserPass = httpGetString(pParamList, "Pass");
		pcDDNSDomainName = httpGetString(pParamList, "DomainName");
		ulIP = httpGetIP(pParamList, "IP");
		pcProxy = httpGetString(pParamList, "Proxy");
		iProxyPort = httpGetLong(pParamList, "ProxyPort");
		pcProxyUser = httpGetString(pParamList, "ProxyUser");
		pcProxyPass = httpGetString(pParamList, "ProxyPass");

		if (ulIP == g_ConfigParam.ulDDNSIP && ulIP != 0
			&& eDDNSType == g_ConfigParam.eDDNSType
			&& g_WebCamState.iDDNSState == DDNS_UPDATE_OK
			&& strcmp(pcDDNSUserName, g_ConfigParam.acDDNSUserName) == 0
			&& strcmp(pcDDNSUserPass, g_ConfigParam.acDDNSUserPass) == 0
			&& strcmp(pcDDNSDomainName, g_ConfigParam.acDDNSDomainName) == 0
			&& strcmp(pcProxy, g_ConfigParam.acProxy) == 0
			&& iProxyPort == g_ConfigParam.iProxyPort
			&& strcmp(pcProxyUser, g_ConfigParam.acProxyUser) == 0
			&& strcmp(pcProxyPass, g_ConfigParam.acProxyPass) == 0
			&& g_ConfigParam.bEnableDDNS == bEnableDDNS
			) return 0;

		cyg_mutex_lock(&g_ptmConfigParam);
		diag_printf("[%s] %d\n", pcDDNSUserName, eDDNSType);
		g_ConfigParam.eDDNSType = eDDNSType;
		httpMyStrncpy(g_ConfigParam.acDDNSUserName, pcDDNSUserName, sizeof(g_ConfigParam.acDDNSUserName));
		httpMyStrncpy(g_ConfigParam.acDDNSUserPass, pcDDNSUserPass, sizeof(g_ConfigParam.acDDNSUserPass));
		httpMyStrncpy(g_ConfigParam.acDDNSDomainName, pcDDNSDomainName, sizeof(g_ConfigParam.acDDNSDomainName));
		httpMyStrncpy(g_ConfigParam.acProxy, pcProxy, sizeof(g_ConfigParam.acProxy));
		g_ConfigParam.ulDDNSIP = ulIP;
		g_ConfigParam.iProxyPort = iProxyPort;
		httpMyStrncpy(g_ConfigParam.acProxyUser, pcProxyUser, sizeof(g_ConfigParam.acProxyUser));
		httpMyStrncpy(g_ConfigParam.acProxyPass, pcProxyPass, sizeof(g_ConfigParam.acProxyPass));
		g_ConfigParam.bEnableDDNS = bEnableDDNS;
		cyg_mutex_unlock(&g_ptmConfigParam);

		DDNS_Update();

		WriteFlashMemory(&g_ConfigParam);
		return 0;
		break;
	}
	return -1;
}


void DDNS_Update_IP(UINT32 uIP)
{
#if 0
	static unsigned long ulOldIP = 0;
				
	if (ulOldIP == uIP
		|| uIP == 0)
	{
		diag_printf("Do not use dns since IP is not updated.\n");
		return;
	}
	ulOldIP = uIP;
#else
	/* Check if the dns must be updated. */
	struct hostent *pHost = gethostbyname(g_ConfigParam.acDDNSDomainName);
	if (pHost != NULL && pHost->h_addr_list != NULL && *pHost->h_addr_list != NULL
		&& *(unsigned long *)(*pHost->h_addr_list) == uIP)
	{
		diag_printf("Host address need not update: %x %x\n", *(unsigned long *)(*pHost->h_addr_list), uIP);
		return;
	}
#endif

	cyg_mutex_lock(&g_ptmConfigParam);
	//update dynamic DNS if possible
	diag_printf("proxy server is %s\n",g_ConfigParam.acProxy);
	diag_printf("IP is %x\n", uIP);
	
	if (g_ConfigParam.bEnableDDNS)
	{
		Do_DNS(g_ConfigParam.eDDNSType,
			g_ConfigParam.acDDNSDomainName,
			uIP,
			g_ConfigParam.acDDNSUserName,
			g_ConfigParam.acDDNSUserPass,
			g_ConfigParam.acProxy,
			g_ConfigParam.iProxyPort,
			g_ConfigParam.acProxyUser,
			g_ConfigParam.acProxyPass);
	}

	cyg_mutex_unlock(&g_ptmConfigParam);
}


void DDNS_Update()
{
	if (g_ConfigParam.bEnableDDNS)
	{
		if (g_ConfigParam.ulDDNSIP == 0)
			DDNS_CheckIP();
		else
			DDNS_Update_IP(g_ConfigParam.ulDDNSIP);
	}
}

#else

void DDNS_Update()
{
}

void DDNS_CheckIP()
{
}
#endif


static PRD_TASK_T g_prdtskCheckDDNS;


static void prdTask_CheckDDNS(void *pArg)
{
	DDNS_Update();
}

void prdAddTask__CheckDDNS()
{
	prdAddTask(&g_prdtskCheckDDNS, &prdTask_CheckDDNS, 15*60*1000, NULL);
}







/* Send request */
typedef struct
{
	HTTPC oHTTPC;
	char acBody[64];
	size_t szBodyLen;
	RESPONSE_E eResState;
	BOOL bIsRequestOver;
	BOOL bHasResponse;
	BOOL bTransparent;
	cyg_mutex_t mLock;
	cyg_cond_t cData;
} URLCGI_HTTPC;

OBJHEADER *URLCGI_HTTPC_Init(OBJHEADER *pObj, va_list vaInit)
{
	OBJHEADER *pRt;
	
	BOOL bTransparent = va_arg(vaInit, BOOL);
	
	pRt = HTTPC_Init(pObj, vaInit);
	if (pRt != NULL)
	{
		((HTTPC *)pRt)->fd.iType = SE_TP_HTTPC;
		//((CGI_HTTPC *)pRt)->szBodyLen = 0;
		((URLCGI_HTTPC *)pRt)->eResState = RS_LINE;
		((URLCGI_HTTPC *)pRt)->bIsRequestOver = FALSE;
		((URLCGI_HTTPC *)pRt)->bHasResponse = FALSE;
		((URLCGI_HTTPC *)pRt)->bTransparent = bTransparent;
		cyg_mutex_init(&((URLCGI_HTTPC *)pRt)->mLock);
		cyg_cond_init(&((URLCGI_HTTPC *)pRt)->cData, &((URLCGI_HTTPC *)pRt)->mLock);
	}	
	return pRt;
}



static void URLCGI_HTTPCRequestOver(URLCGI_HTTPC *pHttpFD)
{
	diag_printf("int URLCGI_HTTPCRequestOver\n");
	pHttpFD->bIsRequestOver = TRUE;
}

static BOOL URLCGI_HTTPCResponse(URLCGI_HTTPC *pHttpC, char *acResponse, int iLen)
{
	char *pc = acResponse;
	
	cyg_mutex_lock(&pHttpC->mLock);	
	
	pHttpC->bHasResponse = TRUE;

	while (pc < acResponse + iLen)
	{
		int iLenThis;
		
		if (pHttpC->szBodyLen > 0)
		{
			cyg_cond_wait(&pHttpC->cData);
			continue;
		}
		
		iLenThis = acResponse + iLen - pc;
		if (iLenThis > sizeof(pHttpC->acBody))
			iLenThis = sizeof(pHttpC->acBody);

		memmove(pHttpC->acBody, pc, iLenThis);
		pHttpC->szBodyLen = iLenThis;
		
		pc += iLenThis;
	}	
	
	cyg_mutex_unlock(&pHttpC->mLock);	
	return TRUE;
}


static int FillURLCGIData(HTTPCONNECTION hConnection, time_t *ptLastFill, void *p)
{
	URLCGI_HTTPC *pHttpC = (URLCGI_HTTPC *)p;
	int rt = 1;
	
	if (pHttpC == NULL)
	{
		httpSetSendDataOverFun(hConnection, NULL, NULL);
		return 0;
	}
	
	cyg_mutex_lock(&pHttpC->mLock);
	
	if (pHttpC->szBodyLen > 0)
	{
		httpAddBody(hConnection, pHttpC->acBody, pHttpC->szBodyLen);
		pHttpC->szBodyLen = 0;
		rt = 1;
	}
	else if (pHttpC->bIsRequestOver)
	{
		if (!pHttpC->bHasResponse)
		{
			pHttpC->bHasResponse = TRUE;
			
			if (pHttpC->bTransparent)
				httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/plain", FALSE);
				
			httpAddBodyString(hConnection, "<!--no response-->");
			rt = 1;
		}
		else
		{
			httpSetSendDataOverFun(hConnection, NULL, NULL);
			rt = 0;
		}
	}

	cyg_cond_signal(&pHttpC->cData);
	cyg_mutex_unlock(&pHttpC->mLock);
	return rt;
}


static void *URLCGI_DelFD(void *pThread, void *pParam)
{
	LISTNODE *pNode;
	LISTNODE *pNextNode;
	
	URLCGI_HTTPC *pHttpC = (URLCGI_HTTPC *)pParam;
	FD_T *pTheFD = &pHttpC->oHTTPC.fd;
	SELECT_THREAD_T *pThreadData = (SELECT_THREAD_T *)pThread;

	for (pNode = pThreadData->pFDList->pFirstNode;
		pNode != pThreadData->pFDList->pLastNode;
		pNode = pNextNode)
	{
		FD_T *pFD = (FD_T *)pNode->pValue;
		pNextNode = pNode->pNextNode;

		if (pFD == NULL) continue;
		if (pFD == pTheFD)
		{
			diag_printf("Real delete http client\n");
			ObjFinal((OBJHEADER *)pFD);
			httpDeleteNode(pNode);
		}
	}

	diag_printf("Free http client object\n");
	
	cyg_cond_destroy(&pHttpC->cData);
	cyg_mutex_destroy(&pHttpC->mLock);
	free(pHttpC);
	return NULL;
}


static int URLCGI_RequestOver(HTTPCONNECTION hConnection, void *pParam)
{
	URLCGI_HTTPC *pHttpC = (URLCGI_HTTPC *)pParam;
	
	diag_printf("Send msg to delete http client\n");
	InsertCustomProcess(g_pMsgSelect, &URLCGI_DelFD, (void *)pHttpC);
	
	--g_WebCamState.nHTTPClientNum;
	return 0;
}



int Http_SendHttpRequest(HTTPCONNECTION hConnection, void *pParam)
{
	LIST *pList;
	BOOL bIsChange = FALSE;

	const char *pcServer;
	int iPort;
	const char *pcPath;
	const char *pcUser;
	const char *pcPass;
	BOOL bTransparent;

	URLCGI_HTTPC *pHttpC;

	if (httpGetMethod(hConnection) != M_GET) return -1;
	
	pList = httpReadQueryList(hConnection);
	if (pList == NULL)
		return -1;
	
	
	pcServer = httpGetString(pList, "Server");
	
	iPort = 0;
	if (httpIsExistParam(pList, "Port"))
		iPort = httpGetLong(pList, "Port");
	if (iPort == 0)
		iPort = 80;
	
	pcPath = httpGetString(pList, "Path");
	if (pcPath[0] == '\0')
		pcPath = "/";
	
	if (httpIsExistParam(pList, "User"))
		pcUser = httpGetString(pList, "User");
	else
		pcUser = NULL;
	if (httpIsExistParam(pList, "Pass"))
		pcPass = httpGetString(pList, "Pass");
	else
		pcPass = NULL;
	
	
	bTransparent = httpGetBool(pList, "Transparent");
	
	
	

	/* Share the same proxy with DDNS */
	cyg_mutex_lock(&g_ptmConfigParam);
	if (httpIsExistParam(pList, "Proxy"))
	{
		const char *pcProxy = httpGetString(pList, "Proxy");
		if (strcmp(g_ConfigParam.acProxy, pcProxy) != 0)
		{
			httpMyStrncpy(g_ConfigParam.acProxy, pcProxy, sizeof(g_ConfigParam.acProxy));
			bIsChange = TRUE;
		}
	}
	if (httpIsExistParam(pList, "ProxyPort"))
	{
		int iProxyPort = httpGetLong(pList, "ProxyPort");
		if (g_ConfigParam.iProxyPort != iProxyPort)
		{
			g_ConfigParam.iProxyPort = iProxyPort;
			bIsChange = TRUE;
		}
	}
	if (httpIsExistParam(pList, "ProxyUser"))
	{
		const char *pcProxyUser = httpGetString(pList, "ProxyUser");
		if (strcmp(g_ConfigParam.acProxyUser, pcProxyUser) != 0)
		{
			httpMyStrncpy(g_ConfigParam.acProxyUser, pcProxyUser, sizeof(g_ConfigParam.acProxyUser));
			bIsChange = TRUE;
		}
	}
	if (httpIsExistParam(pList, "ProxyPass"))
	{
		const char *pcProxyPass = httpGetString(pList, "ProxyPass");
		if (strcmp(g_ConfigParam.acProxyPass, pcProxyPass) != 0)
		{
			httpMyStrncpy(g_ConfigParam.acProxyPass, pcProxyPass, sizeof(g_ConfigParam.acProxyPass));
			bIsChange = TRUE;
		}
	}
	cyg_mutex_unlock(&g_ptmConfigParam);

	httpDeleteQueryList(pList);
	
	if (bIsChange)
	{
		WriteFlashMemory(&g_ConfigParam);
	}	
	
	
	
	/* Check if the connection is beyond the connection limit */
	if (g_WebCamState.nHTTPClientNum >= MAX_URLCGI)
	{
		httpAddBodyString(hConnection, "<!--max limit-->");
		httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/plain", TRUE);
		return 0;
	}


	/* Create the http client object. */
	pHttpC = (URLCGI_HTTPC *)malloc(sizeof(URLCGI_HTTPC));
	if (pHttpC == NULL)
	{
		httpAddBodyString(hConnection, "<!--no memory-->");
		httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/plain", TRUE);
	}
	else if (ObjInit(NULL, sizeof(URLCGI_HTTPC), (void *)pHttpC,
					HTTPC_Final, URLCGI_HTTPC_Init,
					bTransparent,
					pcServer,
					pcPath,
					iPort,
					pcServer,
					"Rovio Update Client v1.0 roviotech@wowwee.com.hk",
					pcUser,
					pcPass,
					g_ConfigParam.acProxy,
					g_ConfigParam.iProxyPort,
					g_ConfigParam.acProxyUser,
					g_ConfigParam.acProxyPass,
					URLCGI_HTTPCResponse,
					URLCGI_HTTPCRequestOver) == NULL)
	{
		free(pHttpC);
		httpAddBodyString(hConnection, "<!--bad request-->");
		httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/plain", TRUE);
	}
	else
	{
		++g_WebCamState.nHTTPClientNum;
		httpSetRequestOverFun(hConnection, URLCGI_RequestOver, pHttpC);
		httpSetSendDataOverFun(hConnection, FillURLCGIData, pHttpC);
		httpSetKeepAliveMode(hConnection, FALSE);
		
		if (bTransparent)
			httpSetNullHeader(hConnection);
		else
			httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/plain", FALSE);
		AddSelectFD(&pHttpC->oHTTPC.fd);	
	}
	
	return 0;
	
}



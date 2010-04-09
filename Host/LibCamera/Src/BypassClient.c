#include "../../Inc/CommonDef.h"



BOOL BP_LOCAL_Process(FD_T *pFD, BOOL bReadable, BOOL bWritable);
BOOL BP_REMOTE_Process(FD_T *pFD, BOOL bReadable, BOOL bWritable);

struct tagBP_LOCAL;		/* IPCam to bypass client */
struct tagBP_REMOTE;	/* Bypass client to bypass server */


typedef struct tagBP_LOCAL
{
	OBJHEADER oHeader;
	FD_T fd;
	struct tagBP_REMOTE *pRemote;
	time_t tLastOp;
	time_t tConnTimeout;
	
	int iLocalPort;
	char acBuffer[256];
	int iLenTotal;
} BP_LOCAL_T;



typedef struct tagBP_REMOTE
{
	OBJHEADER oHeader;
	FD_T fd;
	struct tagBP_LOCAL *pLocal;
	BOOL bLocalUsed;
	
	time_t tLastOp;
	time_t tConnTimeout;
	HOST_INFO_T hostInfo;
	//char **ppcHost;
	in_addr_t addrHost;

	char *pcServer;
	int iServerPort;
	char *pcMagicStr;
	int iMagicStrLen;

	char acBuffer[256];
	int iLenTotal;

	char *psHostentList[4];
} BP_REMOTE_T;



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
** BP_LOCAL_T construction & destrucion
**	do not call them directly for ever!
** var vaInit:
**	BP_REMOTE_T *pBP_C2C;
**	int iLocalPort,
*********************************************************************/
OBJHEADER *BP_LOCAL_Init(OBJHEADER *pObj, va_list vaInit)
{
	BP_LOCAL_T *pThis = (BP_LOCAL_T *)pObj;

	BP_REMOTE_T *pRemote = va_arg(vaInit, BP_REMOTE_T *);
	int iLocalPort = va_arg(vaInit, int);

	memset((char *)pThis + sizeof(OBJHEADER), 0, sizeof(BP_LOCAL_T) - sizeof(OBJHEADER));

	pThis->pRemote = pRemote;
	pThis->iLocalPort = iLocalPort;
	pThis->tConnTimeout = 10;	/* Set the default timeout to 10 sec. */

	if (ObjInit((OBJHEADER *)pThis, sizeof(FD_T), (OBJHEADER *)&pThis->fd, FD_Final, FD_Init,
		-1, SE_TP_BPC, ST_NET_INIT, 0, FALSE, FALSE, BP_LOCAL_Process) == NULL)
		goto lE_FD_Init;


	//PTI;
	return pObj;

lE_FD_Init:
	return NULL;
}


void BP_LOCAL_Final(OBJHEADER *pObj)
{
	BP_LOCAL_T *pThis = (BP_LOCAL_T *)pObj;

	//PTI;

	ObjFinal((OBJHEADER *)&pThis->fd);
}


BOOL BP_LOCAL_Process(FD_T *pFD, BOOL bReadable, BOOL bWritable)
{
	int iLen;
	BP_LOCAL_T *pLocal;
	
	
	pLocal = (BP_LOCAL_T *)((char *)(pFD) - (unsigned long)(&((BP_LOCAL_T *)0)->fd));


	if (ST_NET_INIT == pFD->iState)
	{
		//PTI;
		//diag_printf("Init local connection for bpserver\n");
		pLocal->iLenTotal = 0;
		FD_Next(pFD, ST_NET_CONNECT, FALSE, FALSE, 0);
		return TRUE;
	}
	else if (ST_NET_CONNECT == pFD->iState)
	{
		int iFl;
		struct sockaddr_in sin;
		if (pFD->iFD != -1)
		{
#ifndef WLAN
			netclose(pFD->iFD,g_Select_Buf, SELECT_BUFFER_LEN);
#else
			close(pFD->iFD);
#endif
			pFD->iFD = -1;
		}
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

		sin.sin_family = AF_INET;
		sin.sin_port = htons(pLocal->iLocalPort);	//port number of bypass server
		sin.sin_addr.s_addr = inet_addr("127.0.0.1");
		//diag_printf("sin.sin_addr.s_addr = %x\n", sin.sin_addr.s_addr);

	#ifndef WLAN
		connect(pFD->iFD,(struct sockaddr *)&sin,sizeof(struct sockaddr),g_Select_Buf, SELECT_BUFFER_LEN);
	#else
		connect(pFD->iFD,(struct sockaddr *)&sin,sizeof(struct sockaddr));	
	#endif
		
		FD_Next(pFD, ST_NET_CONNECTING, TRUE, TRUE, pLocal->tConnTimeout * 1000);
		pLocal->tLastOp = mytime();
		return TRUE;
	}
	else if (ST_NET_CONNECTING == pFD->iState)
	{
		//PTI;
		if (!(bReadable || bWritable))
		{
			time_t tThisOp = mytime();
			if (tThisOp - pLocal->tLastOp > pLocal->tConnTimeout)
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
					//diag_printf("lookback connection OK.\n");
					pLocal->tLastOp = mytime();
					
					/*
					diag_printf("%d--%d %d %d %d\n", 
						(int)pLocal->iLenTotal,
						(int)ST_NET_READ,
						(int)TRUE,
						(int)(pLocal->iLenTotal > 0 ? TRUE : FALSE),
						(int)pLocal->tConnTimeout * 1000
						);
					*/
						
					FD_Next(pFD, ST_NET_READ, TRUE, (pLocal->iLenTotal > 0 ? TRUE : FALSE), pLocal->tConnTimeout * 1000);


					return TRUE;
				}
			}
			FD_Next(pFD, ST_NET_CONNECT, FALSE, FALSE, 0);
			return TRUE;
		}

	}
	else if (ST_NET_READ == pFD->iState)
	{
		BP_REMOTE_T *pRemote;
		
		//PTE;
		if (bWritable && pLocal->iLenTotal > 0)
		{
			//PTE;
#ifndef WLAN
			iLen = netwrite(pFD->iFD, pLocal->acBuffer, pLocal->iLenTotal,g_Select_Buf, SELECT_BUFFER_LEN);

#else
			iLen = write(pFD->iFD, pLocal->acBuffer, pLocal->iLenTotal);
		
#endif
			//PTE;
			if (iLen > 0)
			{
				pLocal->tLastOp = mytime();
				pLocal->iLenTotal -= iLen;
				memmove(pLocal->acBuffer, pLocal->acBuffer + iLen, pLocal->iLenTotal);
			}
			else if (!(errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR))
			{
				diag_printf("local write err = %d\n", errno);
				FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
				return TRUE;
			}
		}
		
		pRemote = (BP_REMOTE_T *) pLocal->pRemote;
		if (bReadable && sizeof(pRemote->acBuffer) > pRemote->iLenTotal)
		{
#ifndef WLAN
			iLen = netread(pFD->iFD, pRemote->acBuffer + pRemote->iLenTotal, sizeof(pRemote->acBuffer) - pRemote->iLenTotal,g_Select_Buf, SELECT_BUFFER_LEN);
#else
			iLen = read(pFD->iFD, pRemote->acBuffer + pRemote->iLenTotal, sizeof(pRemote->acBuffer) - pRemote->iLenTotal);
#endif

			if (iLen > 0)
			{
				//diag_printf("Read len for server =========%d,%s\n",iLen,pHttpFD->acBuffer + pHttpFD->iLenOK);
				pRemote->iLenTotal += iLen;
				pLocal->tLastOp = mytime();;
			}
			else if (!(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
			{
				diag_printf("local read err = %d\n", errno);
				FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
				return TRUE;
			}
		}

		if (mytime() - pLocal->tLastOp > pLocal->tConnTimeout)
		{
			FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
			return TRUE;
		}
		
		//if (pRemote->iLenTotal > 0)
		//{
		//	if (pRemote->fd.iState == ST_NET_READ)
		//	FD_Next(&pRemote->fd, ST_NET_READ, pRemote->fd.bWantRead, TRUE, pRemote->fd.iConnTimeout_InMSec);
		//}
		
		if (pRemote->fd.iState == ST_NET_READ)
		{
			FD_Next(&pRemote->fd, ST_NET_READ,
				(sizeof(pLocal->acBuffer) > pLocal->iLenTotal ? TRUE : FALSE),
				(pRemote->iLenTotal > 0 ? TRUE : FALSE),
				pRemote->fd.iConnTimeout_InMSec);
		}
		
		FD_Next(pFD, ST_NET_READ,
			(sizeof(pRemote->acBuffer) > pRemote->iLenTotal ? TRUE : FALSE),
			(pLocal->iLenTotal > 0 ? TRUE : FALSE),
			pLocal->tConnTimeout * 1000);
		
		//diag_printf("len local=%d, remote=%d\n", pLocal->iLenTotal, pRemote->iLenTotal);
		return TRUE;
	}
	else if (ST_NET_OVER == pFD->iState)
	{
		if (pFD->iFD != -1)
		{
#ifndef WLAN
			netclose(pFD->iFD,g_Select_Buf, SELECT_BUFFER_LEN);
#else
			close(pFD->iFD);
#endif
			pFD->iFD = -1;
		}
#if 1
		//diag_printf("/* Local: Try to connect again. */\n");
		if (pLocal->pRemote->fd.iState == ST_NET_READ)
			FD_Next(pFD, ST_NET_RESET, FALSE, FALSE, 0);
		else
			FD_Next(pFD, ST_NET_HALT, FALSE, FALSE, 60000);
		//FD_Next(&pLocal->pRemote->fd, ST_NET_OVER, FALSE, FALSE, 0);
		return TRUE;
#else
		return FALSE;
#endif
	}
	else if (ST_NET_RESET == pFD->iState)
	{
		if (pFD->iFD != -1)
		{
#ifndef WLAN
			netclose(pFD->iFD,g_Select_Buf, SELECT_BUFFER_LEN);
#else
			close(pFD->iFD);
#endif
			pFD->iFD = -1;
		}
		FD_Next(pFD, ST_NET_INIT, FALSE, FALSE, 0);
		return TRUE;
	}
	else if (ST_NET_HALT == pFD->iState)
	{
	}
	else return FALSE;
}




/*********************************************************************
** BP_REMOTE_T construction & destrucion
**	do not call them directly for ever!
** var vaInit:
**	BP_LOCAL *pLocal;
**	const char *pcServer,
**	int iServerPort,
**	const char *pcMagicStr,
**	int iMagicStrLen;
*********************************************************************/
OBJHEADER *BP_REMOTE_Init(OBJHEADER *pObj, va_list vaInit)
{
	BP_REMOTE_T *pThis = (BP_REMOTE_T *)pObj;

	BP_LOCAL_T *pLocal = va_arg(vaInit, BP_LOCAL_T *);
	const char *pcServer = va_arg(vaInit, const char *);
	int iServerPort = va_arg(vaInit, int);
	const char *pcMagicStr = va_arg(vaInit, const char *);
	int iMagicStrLen = va_arg(vaInit, int);

	memset((char *)pThis + sizeof(OBJHEADER), 0, sizeof(BP_REMOTE_T) - sizeof(OBJHEADER));


	pThis->pLocal = pLocal;
	pThis->pcServer = (char*)strdup(pcServer);
	if (pThis->pcServer == NULL) goto lE_strdup_pcServer;

	pThis->iServerPort = iServerPort;
	pThis->tConnTimeout = 10;	/* Set the default timeout to 10 sec. */
	
	pThis->pcMagicStr = (char *)strdup(pcMagicStr);
	if (pThis->pcMagicStr == NULL) goto lE_strdup_pcMagicStr;
	pThis->iMagicStrLen = iMagicStrLen;

	pLocal->iLenTotal = 0;
	
	if (ObjInit((OBJHEADER *)pThis, sizeof(FD_T), (OBJHEADER *)&pThis->fd, FD_Final, FD_Init,
		-1, SE_TP_BPC, ST_NET_INIT, 0, FALSE, FALSE, BP_REMOTE_Process) == NULL)
		goto lE_FD_Init;


	//PTI;
	return pObj;

lE_FD_Init:
	if (pThis->pcMagicStr != NULL)free(pThis->pcMagicStr);
lE_strdup_pcMagicStr:
	if (pThis->pcServer != NULL)free(pThis->pcServer);
lE_strdup_pcServer:
	return NULL;
}


void BP_REMOTE_Final(OBJHEADER *pObj)
{
	BP_REMOTE_T *pThis = (BP_REMOTE_T *)pObj;

	//PTI;

	ObjFinal((OBJHEADER *)&pThis->fd);

	if (pThis->pcMagicStr != NULL)
		free(pThis->pcMagicStr);
	if (pThis->pcServer != NULL)
		free(pThis->pcServer);
}


BOOL BP_REMOTE_Process(FD_T *pFD, BOOL bReadable, BOOL bWritable)
{
	int iLen;
	BP_REMOTE_T *pRemote;
	
	
	pRemote = (BP_REMOTE_T *)((char *)(pFD) - (unsigned long)(&((BP_REMOTE_T *)0)->fd));


	if (ST_NET_INIT == pFD->iState)
	{
		//PTI;
		//diag_printf("Bypass server is %s(%d)\n", pRemote->pcServer, pRemote->iServerPort);

		ResolveHost(&pRemote->hostInfo, pRemote->pcServer);
		if (pRemote->hostInfo.psHostent == NULL
			|| pRemote->hostInfo.psHostent->h_addr_list == NULL
			|| pRemote->hostInfo.psHostent->h_addr_list[0] == NULL)
		{
			FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 1000);
			return TRUE;
		}
		else
		{
#ifndef WLAN
			pRemote->psHostentList[0] = pRemote->hostInfo.psHostent->h_addr_list0;
			pRemote->psHostentList[1] = pRemote->hostInfo.psHostent->h_addr_list1;
			pRemote->psHostentList[2] = pRemote->hostInfo.psHostent->h_addr_list2;
			pRemote->psHostentList[3] = pRemote->hostInfo.psHostent->h_addr_list3;
			pRemote->ppcHost = pRemote->psHostentList;
#else
			//pRemote->ppcHost = pRemote->hostInfo.psHostent->h_addr_list;
			pRemote->addrHost = *(unsigned long*)pRemote->hostInfo.psHostent->h_addr_list[0];
#endif
			FD_Next(pFD, ST_NET_CONNECT, FALSE, FALSE, 0);
			return TRUE;
		}
	}
	else if (ST_NET_CONNECT == pFD->iState)
	{
		int iFl;
		struct sockaddr_in sin;
		if (pFD->iFD != -1)
		{
#ifndef WLAN
			netclose(pFD->iFD,g_Select_Buf, SELECT_BUFFER_LEN);
#else
			close(pFD->iFD);
#endif
			pFD->iFD = -1;
		}
		//if (*pRemote->ppcHost == NULL)
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

		sin.sin_family = AF_INET;
		{
			sin.sin_port = htons(pRemote->iServerPort);	//port number of bypass server
		}

		//sin.sin_addr.s_addr = *(unsigned long*)*pRemote->ppcHost;
		sin.sin_addr.s_addr = pRemote->addrHost;
		//diag_printf("sin.sin_addr.s_addr = %x\n", sin.sin_addr.s_addr);
		//pRemote->ppcHost++;
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
		
		FD_Next(pFD, ST_NET_CONNECTING, TRUE, TRUE, pRemote->tConnTimeout * 1000);
		pRemote->tLastOp = mytime();
		return TRUE;
	}
	else if (ST_NET_CONNECTING == pFD->iState)
	{
		//PTI;
		if (!(bReadable || bWritable))
		{
			time_t tThisOp = mytime();
			if (tThisOp - pRemote->tLastOp > pRemote->tConnTimeout)
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
					//diag_printf("%s: connection OK.\n", pRemote->pcServer);
					pRemote->tLastOp = mytime();

					memset(pRemote->acBuffer, 0, pRemote->iMagicStrLen);
					strncpy(pRemote->acBuffer, pRemote->pcMagicStr, pRemote->iMagicStrLen);
					pRemote->iLenTotal = pRemote->iMagicStrLen;
					

					FD_Next(pFD, ST_NET_READ, TRUE, (pRemote->iLenTotal > 0 ? TRUE : FALSE), pRemote->tConnTimeout * 1000);
					
					FD_Next(&pRemote->pLocal->fd, ST_NET_RESET, FALSE, FALSE, 0);

					return TRUE;
				}
			}
			FD_Next(pFD, ST_NET_CONNECT, FALSE, FALSE, 0);
			return TRUE;
		}

	}
	else if (ST_NET_READ == pFD->iState)
	{
		BP_LOCAL_T *pLocal;
		
		//PTE;
		if (bWritable && pRemote->iLenTotal > 0)
		{
			//PTE;
#ifndef WLAN
			iLen = netwrite(pFD->iFD, pRemote->acBuffer, pRemote->iLenTotal,g_Select_Buf, SELECT_BUFFER_LEN);

#else
			iLen = write(pFD->iFD, pRemote->acBuffer, pRemote->iLenTotal);
		
#endif
			//PTE;
			if (iLen > 0)
			{
				pRemote->tLastOp = mytime();
				pRemote->iLenTotal -= iLen;
				memmove(pRemote->acBuffer, pRemote->acBuffer + iLen, pRemote->iLenTotal);
			}
			else if (!(errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR))
			{
				diag_printf("remote write err = %d\n", errno);
				FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
				return TRUE;
			}
		}
		
		pLocal = (BP_LOCAL_T *) pRemote->pLocal;
		if (bReadable && sizeof(pLocal->acBuffer) > pLocal->iLenTotal)
		{
#ifndef WLAN
			iLen = netread(pFD->iFD, pLocal->acBuffer + pLocal->iLenTotal, sizeof(pLocal->acBuffer) - pLocal->iLenTotal,g_Select_Buf, SELECT_BUFFER_LEN);
#else
			iLen = read(pFD->iFD, pLocal->acBuffer + pLocal->iLenTotal, sizeof(pLocal->acBuffer) - pLocal->iLenTotal);
#endif

			if (iLen > 0)
			{
				//diag_printf("Read len for server =========%d,%s\n",iLen,pHttpFD->acBuffer + pHttpFD->iLenOK);
				pLocal->iLenTotal += iLen;
				pRemote->tLastOp = mytime();;
			}
			else if (!(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
			{
				diag_printf("remote read err = %d\n", errno);
				FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
				return TRUE;
			}
		}

		if (mytime() - pRemote->tLastOp > pRemote->tConnTimeout)
		{
			FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
			return TRUE;
		}
		
		//if (pLocal->iLenTotal > 0)
		//{
		//	pRemote->bLocalUsed = TRUE;
		//	if (pLocal->fd.iState == ST_NET_READ)
		//		FD_Next(&pLocal->fd, ST_NET_READ, pLocal->fd.bWantRead, TRUE, pLocal->fd.iConnTimeout_InMSec);
		//}
		pRemote->bLocalUsed = TRUE;
		if (pLocal->fd.iState == ST_NET_READ)
		{
			FD_Next(&pLocal->fd, ST_NET_READ,
				(sizeof(pRemote->acBuffer) > pRemote->iLenTotal ? TRUE : FALSE),
				(pLocal->iLenTotal > 0 ? TRUE : FALSE),
				pLocal->fd.iConnTimeout_InMSec);
		}
		
		FD_Next(pFD, ST_NET_READ,
			(sizeof(pLocal->acBuffer) > pLocal->iLenTotal ? TRUE : FALSE),
			(pRemote->iLenTotal > 0 ? TRUE : FALSE),
			pRemote->tConnTimeout * 1000);
		
		//diag_printf("len remote=%d, local=%d\n", pRemote->iLenTotal, pLocal->iLenTotal);
		return TRUE;
	}
	else if (ST_NET_OVER == pFD->iState)
	{
#if 1
		FD_Next(&pRemote->pLocal->fd, ST_NET_HALT, FALSE, FALSE, 60000);
#else	
		if (pRemote->bLocalUsed)
		{
			
			BP_LOCAL_T *pLocal = (BP_LOCAL_T *) pRemote->pLocal;

			/* Clear local buffer */
			pLocal->iLenTotal = 0;
			
			/* Close Local if local is used. */
			if (pLocal->fd.iState == ST_NET_READ)
				FD_Next(&pLocal->fd, ST_NET_OVER, FALSE, FALSE, 0);
		}
#endif
	
		if (pFD->iFD != -1)
		{
#ifndef WLAN
			netclose(pFD->iFD,g_Select_Buf, SELECT_BUFFER_LEN);
#else
			close(pFD->iFD);
#endif
			pFD->iFD = -1;
		}
		DeleteHost(&pRemote->hostInfo);
#if 1
		//diag_printf( "/* Try to connect again. */\n");
		FD_Next(pFD, ST_NET_INIT, FALSE, FALSE, g_ConfigParam.uDelayTime_InMSec);
		return TRUE;
#else
		return FALSE;
#endif
	}
	else return FALSE;
}



#define BP_CONN_NUM 6
static BP_LOCAL_T g_BP_Local[BP_CONN_NUM];
static BP_REMOTE_T g_BP_Remote[BP_CONN_NUM];



/* Default server: rovio.gostai.com:21846 */
void BPC_New (const char *pcServer, int iServerPort, int iDefaultLocalPort)
{
	int i;
#if 0
	if (g_ConfigParam.iBPSChannelsNum >= 1)
	{
		ObjInit(NULL, sizeof(BP_LOCAL_T), (OBJHEADER *)&g_BP_Local[0],
				BP_LOCAL_Final, BP_LOCAL_Init,
				&g_BP_Remote[0],
				iLocalPort);
		ObjInit(NULL, sizeof(BP_REMOTE_T), (OBJHEADER *)&g_BP_Remote[0],
				BP_REMOTE_Final, BP_REMOTE_Init,
				&g_BP_Local[0],
				pcServer,
				iServerPort,
				"ROVIO_VIDEO", 11);
		AddSelectFD (&g_BP_Local[0].fd);
		AddSelectFD (&g_BP_Remote[0].fd);
	}
#endif
			
	for (i = 0; i < BP_CONN_NUM && i < g_ConfigParam.iBPSChannelsNum; ++i)
	{
		const char *pcConnectType;
		int iLocalPort;
		
		if(g_ConfigParam.acBPSType[i] == 'v' || g_ConfigParam.acBPSType[i] =='V')
			pcConnectType = "ROVIO_VIDEO";
		else
			pcConnectType = "ROVIO_CTRL";
		
		if(g_ConfigParam.aiBPSLocalPort[i] == 0)
			iLocalPort = iDefaultLocalPort;
		else
			iLocalPort = g_ConfigParam.aiBPSLocalPort[i];
		
		ObjInit(NULL, sizeof(BP_LOCAL_T), (OBJHEADER *)&g_BP_Local[i],
				BP_LOCAL_Final, BP_LOCAL_Init,
				&g_BP_Remote[i],
				iLocalPort);
		ObjInit(NULL, sizeof(BP_REMOTE_T), (OBJHEADER *)&g_BP_Remote[i],
				BP_REMOTE_Final, BP_REMOTE_Init,
				&g_BP_Local[i],
				pcServer,
				iServerPort,
				pcConnectType, 11);
		AddSelectFD (&g_BP_Local[i].fd);
		AddSelectFD (&g_BP_Remote[i].fd);
	}

}



int Config_GetVNet(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_USER;
		break;
	case CA_CONFIG:
	{
		char acDescription[256];
		int i, nLen;
		tt_rmutex_lock(&g_rmutex);
		AddHttpNum(pReturnXML, "Enable", g_ConfigParam.bEnableBPS);
		AddHttpValue(pReturnXML, "Server", g_ConfigParam.acBPSDomainName);
		AddHttpNum(pReturnXML, "Port", g_ConfigParam.iBPSPort);
		//AddHttpNum(pReturnXML, "Num", g_ConfigParam.iBPSChannelsNum);

		nLen = 0;
		for(i = 0; i < g_ConfigParam.iBPSChannelsNum; ++i)
			nLen += sprintf(acDescription + nLen, "%d%c",
				g_ConfigParam.aiBPSLocalPort[i],
				((g_ConfigParam.acBPSType[i] == 'v' || g_ConfigParam.acBPSType[i] =='V') ? 'V' : 'C')
				);

		AddHttpValue(pReturnXML, "Description", acDescription);
		AddHttpNum(pReturnXML, "Interval", g_ConfigParam.uDelayTime_InMSec);
		tt_rmutex_unlock(&g_rmutex);
		return 0;
		break;
	}
	
	}
	return -1;
}



int Config_SetVNet(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	BOOL bEnable;
	const char *pcServer;
	int iPort;
	
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
		break;
	case CA_CONFIG:
		tt_rmutex_lock(&g_rmutex);
		if (httpIsExistParam(pParamList, "Enable"))
			g_ConfigParam.bEnableBPS = httpGetBool(pParamList, "Enable");
		
		if (httpIsExistParam(pParamList, "Server"))
			httpMyStrncpy(g_ConfigParam.acBPSDomainName,
				httpGetString(pParamList, "Server"),
				sizeof(g_ConfigParam.acBPSDomainName));
		
		if (httpIsExistParam(pParamList, "Port"))
			g_ConfigParam.iBPSPort = httpGetLong(pParamList, "Port");
			
		//if (httpIsExistParam(pParamList, "Num"))
		//	g_ConfigParam.iBPSChannelsNum = httpGetLong(pParamList, "Num");
		
		if (httpIsExistParam(pParamList, "Description"))
		{
			const char *pcDescription = httpGetString(pParamList, "Description");
			int nRet = sscanf(pcDescription, "%d%c%d%c%d%c%d%c%d%c%d%c",
				&g_ConfigParam.aiBPSLocalPort[0],
				&g_ConfigParam.acBPSType[0],
				&g_ConfigParam.aiBPSLocalPort[1],
				&g_ConfigParam.acBPSType[1],
				&g_ConfigParam.aiBPSLocalPort[2],
				&g_ConfigParam.acBPSType[2],
				&g_ConfigParam.aiBPSLocalPort[3],
				&g_ConfigParam.acBPSType[3],
				&g_ConfigParam.aiBPSLocalPort[4],
				&g_ConfigParam.acBPSType[4],
				&g_ConfigParam.aiBPSLocalPort[5],
				&g_ConfigParam.acBPSType[5]
				);
			if (nRet > 0)
				g_ConfigParam.iBPSChannelsNum = nRet / 2;
			else
				g_ConfigParam.iBPSChannelsNum = 0;
		}
		
		if (httpIsExistParam(pParamList, "Interval"))
			g_ConfigParam.uDelayTime_InMSec = httpGetLong(pParamList, "Interval");
			
		tt_rmutex_unlock(&g_rmutex);
		
		WebCameraLog(CL_PRIVILEGE_ADMIN, CL_SET_VNET, NULL, hConnection);					
		WriteFlashMemory(&g_ConfigParam);
		RebootOnConnectionOver(hConnection);		
		
		return 0;
		break;
	}
	return -1;
}



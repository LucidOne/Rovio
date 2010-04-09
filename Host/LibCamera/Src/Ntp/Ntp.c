#include "../../Inc/CommonDef.h"


#ifdef USE_DDNS

#ifndef WLAN
//static char *psHostentList[4];
extern CHAR g_Select_Buf[SELECT_BUFFER_LEN];
#endif

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

HOST_INFO_T *ResolveHost(HOST_INFO_T *pHostInfo, const char *pcServer)
{
	int rt;
#if 0
	int iHerr;
	int iHostBufLen;

	char *pcHostBuf;
	struct hostent sHost;
	if (pcServer == NULL || pHostInfo == NULL) return NULL;

	pHostInfo->pcExtraBuffer = NULL;
	pHostInfo->psHostent = NULL;
	while ((rt = gethostbyname_r(pcServer, &sHost, pcHostBuf, iHostBufLen,
		&pHostInfo->psHostent, &iHerr)) == ERANGE)
	{
		iHostBufLen *= 2;
		pcHostBuf = realloc(pHostInfo->pcExtraBuffer, iHostBufLen);
		if (pcHostBuf == NULL)
		{
			PRINT_MEM_OUT;
			goto lError;
		}
		pHostInfo->pcExtraBuffer = pcHostBuf;
	}
	if (rt == 0 && pHostInfo->psHostent != NULL) goto lOk;
#endif
#ifndef WLAN
	pHostInfo->psHostent = (struct hostent *)gethostbyname(pcServer,g_Select_Buf, SELECT_BUFFER_LEN);
	if (pHostInfo->psHostent != NULL) goto lOk;
#else

	/* Attention, eCos gethostbyname can only use one instance!! */
	pHostInfo->psHostent = (struct hostent *)gethostbyname(pcServer);
	if (pHostInfo->psHostent != (struct hostent *)NULL) goto lOk;
#endif
//lError:
	fprintf(stderr, "Failed to revolve host!\n");
	pHostInfo->psHostent = NULL;
	if (pHostInfo->pcExtraBuffer != NULL)
	{
		free(pHostInfo->pcExtraBuffer);
		pHostInfo->pcExtraBuffer = NULL;
	}
	return NULL;
lOk:
	return pHostInfo;
}

/* free the source created by ResolveHost. */
void DeleteHost(HOST_INFO_T *pHostInfo)
{
	if (pHostInfo == NULL) return;
	if (pHostInfo->pcExtraBuffer != NULL)
		free(pHostInfo->pcExtraBuffer);
}

int FlushSocket(int fd)
{
/* Get rid of any outstanding input, because it may have been hanging around
for a while.  Ignore packet length oddities and return the number of packets
skipped. */

    char buffer[256];
    int  count = 0, k;

/* The code is the obvious. */
    while (1)
    {
 #ifndef WLAN
        k = recv(fd, buffer, 256, 0, g_Select_Buf, SELECT_BUFFER_LEN);
#else
        k = recv(fd, buffer, 256, 0);
#endif
        if (k < 0)
        {
			if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            fprintf(stderr, "Unable to flush socket");
            return -1;
        }
		count++;
    }

    return count;
}
time_t convert_time (double value, int *millisecs) {
/* Convert the time to the ANSI C form. */
    time_t result = (time_t)value;

    if ((*millisecs = (int)(1000.0*(value-result))) >= 1000) {
        *millisecs = 0;
        ++result;
    }
    return result;
}

static int gettimeofday(struct timeval *tv)
 {
     cyg_tick_count_t cur_time;
     cur_time = cyg_current_time();
     tv->tv_sec = cur_time / 100;
     tv->tv_usec = (cur_time % 100) * 10000;
     return 0;
 }
double current_time (double offset)
{
/* Get the current UTC time in seconds since the Epoch plus an offset (usually
the time from the beginning of the century to the Epoch!) */
    struct timeval current;
    if (gettimeofday(&current))
    {
        fprintf(stderr, "Unable to read current machine/system time");
        exit(-1);
	}
    return offset+current.tv_sec+1.0e-6*current.tv_usec;
}

typedef struct
{
    unsigned char ucStatus;
    unsigned char ucVersion;
    unsigned char ucMode;
    unsigned char ucStratum;
    unsigned char ucPolling;
    unsigned char ucPrecision;
    double dDispersion;
    double dReference;
    double dOriginate;
    double dReceive;
    double dTransmit;
    double dCurrent;
    char cReal_transmit[8];
} NTP_DATA_T;

void MakeClientPacket(NTP_DATA_T *pData)
{
    pData->ucStatus = NTP_LI_FUDGE << 6;
    pData->ucStratum = NTP_STRATUM;
    pData->dReference = pData->dDispersion = 0.0;
	pData->ucVersion = NTP_VERSION;
	pData->ucMode = NTP_CLIENT;
	pData->ucPolling = NTP_POLLING;
	pData->ucPrecision = NTP_PRECISION;
	pData->dReceive = pData->dOriginate = 0.0;
    pData->dCurrent = pData->dTransmit = current_time(JAN_1970);
}

void PackNTP(unsigned char *pucPacketBuf, int iPacketLen, NTP_DATA_T *pData)
{
/* Pack the essential data into an NTP packet, bypassing struct layout and
endian problems.  Note that it ignores fields irrelevant to SNTP. */
    int i, k;
    double d;

    memset(pucPacketBuf, 0, iPacketLen);
    pucPacketBuf[0] = (pData->ucStatus<<6)|(pData->ucVersion<<3)|pData->ucMode;
    pucPacketBuf[1] = pData->ucStratum;
    pucPacketBuf[2] = pData->ucPolling;
    pucPacketBuf[3] = pData->ucPrecision;
    if (pData->dOriginate < 0)
    {
		for (i = 0; i < 8; ++i)
	    	pucPacketBuf[NTP_ORIGINATE + i] = (unsigned char)pData->cReal_transmit[i];
		/* Fake a slightly dated reference time */
		d = (pData->dTransmit - 80000.0) / NTP_SCALE;
		for (i = 0; i < 8; ++i)
		{
			if ((k = (int)(d *= 256.0)) >= 256) k = 255;
			pucPacketBuf[NTP_REFERENCE+i] = k;
			d -= k;
		}
    }
    else
    {
        d = pData->dOriginate/NTP_SCALE;
		for (i = 0; i < 8; ++i)
		{
			if ((k = (int)(d *= 256.0)) >= 256) k = 255;
			pucPacketBuf[NTP_ORIGINATE+i] = k;
			d -= k;
		}
    }
    d = pData->dReceive/NTP_SCALE;
    for (i = 0; i < 8; ++i) {
        if ((k = (int)(d *= 256.0)) >= 256) k = 255;
        pucPacketBuf[NTP_RECEIVE+i] = k;
        d -= k;
    }
    d = pData->dTransmit/NTP_SCALE;
    for (i = 0; i < 8; ++i) {
        if ((k = (int)(d *= 256.0)) >= 256) k = 255;
        pucPacketBuf[NTP_TRANSMIT+i] = k;
        d -= k;
    }
}

void UnpackNTP(NTP_DATA_T *pData, unsigned char *pucPacketBuf, int iPacketLen)
{
/* Unpack the essential data from an NTP packet, bypassing struct layout and
endian problems.  Note that it ignores fields irrelevant to SNTP. */

    int i;
    double d;

    pData->dCurrent = current_time(JAN_1970);    /* Best to come first */
    pData->ucStatus = (pucPacketBuf[0] >> 6);
    pData->ucVersion = (pucPacketBuf[0] >> 3)&0x07;
    pData->ucMode = pucPacketBuf[0]&0x07;
    pData->ucStratum = pucPacketBuf[1];
    pData->ucPolling = pucPacketBuf[2];
    pData->ucPrecision = pucPacketBuf[3];
    d = 0.0;
    for (i = 0; i < 4; ++i) d = 256.0*d+pucPacketBuf[NTP_DISP_FIELD+i];
    pData->dDispersion = d/65536.0;
    d = 0.0;
    for (i = 0; i < 8; ++i) d = 256.0*d+pucPacketBuf[NTP_REFERENCE+i];
    pData->dReference = d/NTP_SCALE;
    d = 0.0;
    for (i = 0; i < 8; ++i) d = 256.0*d+pucPacketBuf[NTP_ORIGINATE+i];
    pData->dOriginate = d/NTP_SCALE;
    d = 0.0;
    for (i = 0; i < 8; ++i) d = 256.0*d+pucPacketBuf[NTP_RECEIVE+i];
    pData->dReceive = d/NTP_SCALE;
    for (i = 0; i < 8; ++i)
	pData->cReal_transmit[i] = pucPacketBuf[NTP_TRANSMIT+i];
    d = 0.0;
    for (i = 0; i < 8; ++i) d = 256.0*d+pucPacketBuf[NTP_TRANSMIT+i];
    pData->dTransmit = d/NTP_SCALE;
}

static void NTPDataToTime(NTP_DATA_T *pData)
{
	double x, y, off, err;
	time_t tNow;
	int iMillisecs;

	if (pData == NULL) return;
	
	x = pData->dReceive - pData->dOriginate;
	y = (pData->dTransmit == 0.0 ? 0.0 : pData->dTransmit - pData->dCurrent);
	off = 0.5 * (x + y);
   	err = x - y;
	x = pData->dCurrent - pData->dOriginate;
	if (0.5 * x > err) err = 0.5 * x;

	tNow = convert_time(current_time(off), &iMillisecs);
{
char acTime[32];
ctime_r(&tNow, acTime);
diag_printf("Current time: %s (%d)\n", acTime, iMillisecs);
}
	SetCheckedTime((const long *)&tNow, NULL);
}
BOOL NTPC_Process(FD_T *pFD, BOOL bReadable, BOOL bWritable);

typedef struct tagNTPC
{
	OBJHEADER oHeader;
	FD_T fd;
	time_t tLastOp;
	time_t tConnTimeout;
	HOST_INFO_T hostInfo;
	char **ppcHost;
	char *pcServer;
	int iTryCount;
	int iLenOK;
	NTP_DATA_T nData;	
	char acBuffer[NTP_PACKET_MAX];
	char *psHostentList[4];
} NTPC;
/*********************************************************************
** NTPC construction & destrucion
**	do not call them directly for ever!
** var vaInit:
**	const char *pcServer.
*********************************************************************/
OBJHEADER *NTPC_Init(OBJHEADER *pObj, va_list vaInit)
{
	NTPC *pThis = (NTPC *)pObj;
	const char *pcServer = va_arg(vaInit, const char *);
	HOST_INFO_T hostInfo;

	memset((char *)pThis + sizeof(OBJHEADER), 0, sizeof(NTPC) - sizeof(OBJHEADER));
	pThis->pcServer = (char*)strdup(pcServer);
	if (pThis->pcServer == NULL) goto lE_strdup_pcServer;
	pThis->tConnTimeout = 20;//timeout: 20 seconds
	pThis->hostInfo = hostInfo;

	if (ObjInit((OBJHEADER *)pThis, sizeof(FD_T), (OBJHEADER *)&pThis->fd, FD_Final, FD_Init,
		-1, SE_TP_NTPC, ST_NET_INIT, 0, FALSE, FALSE, NTPC_Process) == NULL)
		goto lE_FD_Init;
	//PTI;
	return pObj;

lE_FD_Init:
	if (pThis->pcServer != NULL)free(pThis->pcServer);
lE_strdup_pcServer:
	return NULL;	
}


void NTPC_Final(OBJHEADER *pObj)
{
	NTPC *pThis = (NTPC *)pObj;

	//PTE;
	ObjFinal((OBJHEADER *)&pThis->fd);

	if (pThis->pcServer != NULL)
		free(pThis->pcServer);
}



BOOL NTPC_Process(FD_T *pFD, BOOL bReadable, BOOL bWritable)
{
	NTPC *pNtpFD;	


	pNtpFD = (NTPC *)((char *)(pFD) - (unsigned long)(&((NTPC *)0)->fd));

	if (ST_NET_INIT == pFD->iState)
	{
		ResolveHost(&pNtpFD->hostInfo, pNtpFD->pcServer);
		if (pNtpFD->hostInfo.psHostent == NULL)
		{
			FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
			return TRUE;
		}
		else
		{
#ifndef WLAN
			pNtpFD->psHostentList[0] = pNtpFD->hostInfo.psHostent->h_addr_list0;
			pNtpFD->psHostentList[1] = pNtpFD->hostInfo.psHostent->h_addr_list1;
			pNtpFD->psHostentList[2] = pNtpFD->hostInfo.psHostent->h_addr_list2;
			pNtpFD->psHostentList[3] = pNtpFD->hostInfo.psHostent->h_addr_list3;
			pNtpFD->ppcHost = pNtpFD->psHostentList;			
#else
			pNtpFD->ppcHost = pNtpFD->hostInfo.psHostent->h_addr_list;
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
			netclose(pFD->iFD, g_StreamServer_Buf, System_BUFFER_LEN);
#else
			close(pFD->iFD);
#endif
			pFD->iFD = -1;
		}
		if (*pNtpFD->ppcHost == NULL)
		{
			FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
			return TRUE;
		}
#ifndef WLAN
		pFD->iFD = socket(PF_INET, SOCK_DGRAM, 0, g_Select_Buf, SELECT_BUFFER_LEN);
#else
		pFD->iFD = socket(PF_INET, SOCK_DGRAM, 0);
#endif
		if (pFD->iFD < 0)
		{
			fprintf(stderr, "Can not create socket!\n");
			FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
			return TRUE;
		}

		/* set socket to O_NDELAY */
#ifndef WLAN
		//iFl = netfcntl(pFD->iFD, 3, 0, g_Select_Buf, SELECT_BUFFER_LEN);
		//if (netfcntl(pFD->iFD, 4, iFl | 0x800, g_Select_Buf, SELECT_BUFFER_LEN) != 0)
#else
		iFl = fcntl(pFD->iFD, F_GETFL, 0);
		//if (fcntl(pFD->iFD, F_SETFL, iFl | O_NDELAY) != 0)
		iFl = 1;
		if(ioctl(pFD->iFD,FIONBIO,&iFl) != 0)
#endif
		{
			fprintf(stderr, "Can not set socket fd to O_NDELAY mode.\n");
			FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
			return TRUE;
		}

		/* Connect to server */
		sin.sin_family = AF_INET;
		sin.sin_port = htons(123);	//ntp port number, 123
		sin.sin_addr.s_addr = *(unsigned long*)*pNtpFD->ppcHost;
		pNtpFD->ppcHost++;
diag_printf("ntp fd is %d\n",pFD->iFD);
#ifndef WLAN
		connect(pFD->iFD, (struct sockaddr *)&sin, sizeof(sin), g_Select_Buf, SELECT_BUFFER_LEN);
#else
		connect(pFD->iFD, (struct sockaddr *)&sin, sizeof(sin));
#endif
		FD_Next(pFD, ST_NET_WRITE, FALSE, TRUE, 0);
		pNtpFD->iTryCount = 0;
		pNtpFD->tLastOp = mytime();
		return TRUE;
	}
	else if (ST_NET_WRITE == pFD->iState)
	{
		char aucTransmit[NTP_PACKET_MIN];
		NTP_DATA_T nData;
//PTE;
		if (pNtpFD->iTryCount >= 5)
		{
			FD_Next(pFD, ST_NET_CONNECT, FALSE, FALSE, 0);
			return TRUE;
		}
		
		MakeClientPacket(&nData);
		PackNTP((unsigned char *)aucTransmit, NTP_PACKET_MIN, &nData);

		FlushSocket(pFD->iFD);
#ifndef WLAN
		if (send(pFD->iFD, aucTransmit, NTP_PACKET_MIN, 0, g_Select_Buf, SELECT_BUFFER_LEN) != NTP_PACKET_MIN)
#else
		if (send(pFD->iFD, aucTransmit, NTP_PACKET_MIN, 0) != NTP_PACKET_MIN)
#endif		
		{
			fprintf(stderr, "Send NTP data error!\n");
			FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
			return TRUE;
		}

		FD_Next(pFD, ST_NET_READ, TRUE, FALSE, pNtpFD->tConnTimeout * 1000);
		pNtpFD->iLenOK = 0;
		pNtpFD->iTryCount++;
		pNtpFD->tLastOp = mytime();
		return TRUE;
	}
	else if (ST_NET_READ == pFD->iState)
	{
		int iLen;
		time_t tThisOp = mytime();

		if (tThisOp - pNtpFD->tLastOp <= pNtpFD->tConnTimeout)
		{
			if (!bReadable) return TRUE;
#ifndef WLAN			
			iLen = netread(pFD->iFD, pNtpFD->acBuffer + pNtpFD->iLenOK, sizeof(pNtpFD->acBuffer) - pNtpFD->iLenOK, g_Select_Buf, SELECT_BUFFER_LEN);
#else
			iLen = read(pFD->iFD, pNtpFD->acBuffer + pNtpFD->iLenOK, sizeof(pNtpFD->acBuffer) - pNtpFD->iLenOK);
#endif
			if (iLen <= 0)
			{
				//fprintf(stderr, "read errno: %d\n", errno);
				if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
					return TRUE;
			}
			else
			{
				//PTE;
				pNtpFD->iLenOK += iLen;
				if (pNtpFD->iLenOK >= NTP_PACKET_MIN && pNtpFD->iLenOK <= NTP_PACKET_MAX)
				{
					NTP_DATA_T nData;
					double dDelay1;
					double dDelay2;
					UnpackNTP(&nData, (unsigned char *)pNtpFD->acBuffer, pNtpFD->iLenOK);
			    	dDelay1 = nData.dTransmit - nData.dReceive;
		   			dDelay2 = nData.dCurrent - nData.dOriginate;

					if (!((nData.ucMode != NTP_SERVER && nData.ucMode != NTP_PASSIVE)
						|| (nData.ucStatus != 0 && nData.ucStatus != 3)
						|| nData.ucVersion < 1
						|| nData.ucVersion > NTP_VERSION_MAX
						|| nData.ucStratum > NTP_STRATUM_MAX
						|| (nData.ucStratum != 0 && nData.ucStratum != NTP_STRATUM_MAX && nData.dReference == 0.0)
						|| nData.dTransmit == 0.0
						|| nData.dOriginate == 0.0
						|| nData.dReceive == 0.0
						|| (nData.dReference != 0.0 && nData.dReceive < nData.dReference)
						|| dDelay1 < 0.0 || dDelay1 > NTP_INSANITY
						|| dDelay2 < 0.0 || dDelay2 > NTP_INSANITY
						|| nData.dDispersion > NTP_INSANITY
						))
					{//ok
						NTPDataToTime(&nData);
						FD_Next(pFD, ST_NET_OVER, FALSE, FALSE, 0);
						return TRUE;
					}
				}
			}
		}


		FD_Next(pFD, ST_NET_WRITE, FALSE, TRUE, 0);
		return TRUE;
	}
	else if (ST_NET_OVER == pFD->iState)
	{
		DeleteHost(&pNtpFD->hostInfo);
		if (pFD->iFD >= 0)
#ifndef WLAN
			 netclose(pFD->iFD, g_Select_Buf, SELECT_BUFFER_LEN);
#else
			close(pFD->iFD);
#endif
		return FALSE;
	}
	else return FALSE;

}


static void *AddNTP(void *pThread, void *pParam)
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
		if (pFD->iType == SE_TP_NTPC)
		{
			//fprintf(stderr, "Delete \n");PTE;
			ObjFinal((OBJHEADER *)pFD);
			httpDeleteNode(pNode);
		}
	}

	//PTE;
	AddSelectFD(pNewFD);
	return NULL;
}

BOOL Do_NTP(const char *pcServer)
{
	NTPC *pNtpC;
	if (pcServer == NULL || pcServer[0] == '\0') return FALSE;

	if ((pNtpC = (NTPC *)ObjInit(NULL, sizeof(NTPC), NULL,
					NTPC_Final, NTPC_Init,
					pcServer)) == NULL) return FALSE;
	return InsertCustomProcess(g_pMsgSelect, &AddNTP, (void *)&pNtpC->fd);
}


void NtpSetTime(char *pcServer)
{
	NTPC *pNtpC;
	if (pcServer == NULL || pcServer[0] == '\0') return;

	if ((pNtpC = (NTPC *)ObjInit(NULL, sizeof(NTPC), NULL,
					NTPC_Final, NTPC_Init,
					pcServer)) == NULL) return;
	InsertCustomProcess(g_pMsgSelect, &AddNTP, (void *)&pNtpC->fd);
}
#endif
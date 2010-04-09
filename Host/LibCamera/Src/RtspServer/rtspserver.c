#include "rtspserver.h"
#include "cyg/kernel/kapi.h"
#include "InInc/tt_msg.h"

cyg_handle_t rtsp_handle;
cyg_thread rtsp_thread;
#pragma arm section zidata = "non_init"
unsigned char rtsp_stack[STACKSIZE];
#pragma arm section zidata

W99702_DATA_EXCHANGE_T* g_RtspData = NULL;

#if 0
"Encoder ALAW {\n"
"	Input \"%s\";\n"	//pcm
"	Output \"%s\";\n"	//alaw
"}\n"
#endif
static const char *g_RtspServer_conf = 
"FrameHeap 4; # Appropriate for one video stream and one audio stream\n"
"Port 554;\n"
"Input OSS {\n"
"	Device /dev/dsp;\n"
"	SampleRate 8000;\n"
"	Output \"%s\";\n"	//IMAADPCM, AMR, ADPCM
"}\n"
"Input V4L {\n"
"	Device /dev/video0;\n"
"	InputType webcam;\n"
"	FrameRate auto;#30;\n"
"	Output \"%s\";\n"	//Mpeg4, H263
"}\n"
"RTSP-Handler Live {\n"
"	Path /webcam;\n"
"	Path /webcam \"RtspServer\" \"test\" \"test\";\n"
"	Track \"%s\";\n"	//Mpeg4, H263
"	Track \"%s\";\n"	//IMAADPCM, AMR, ADPCM
"}\n";

static char rtsp_conf_buf[1024];//512

static enum {                       // Thread state values
        RUNNING    = 0,          // Thread is runnable or running
        SLEEPING   = 1,          // Thread is waiting for something to happen
        COUNTSLEEP = 2,          // Sleep in counted manner
        SUSPENDED  = 4,          // Suspend count is non-zero
        CREATING   = 8,          // Thread is being created
        EXITED     = 16         // Thread has exited
    };


static void rtspSwapHttpTunnelFD(void *pTunnelHandle, int *swap_fd);
static void rtspEndSwapHttpFD(void *pTunnelHandle);


static void rtspFreeVideoBuf(void *pBuffer)
{
	VP_BUFFER_MP4_BITSTREAM_T	*pMP4Buf;
	
	pMP4Buf = GetParentAddr((void*)SET_CACHE(pBuffer), VP_BUFFER_MP4_BITSTREAM_T, aucBitstream);
	
	//diag_printf("free video: %x %x\n", pMP4Buf, pBuffer);
	bufMP4BitstreamDecRef(pMP4Buf);
}


static void rtspGetBackVideo (unsigned char** ppucInputBuf, int *piInputLen)
{
	VP_BUFFER_MP4_BITSTREAM_T	*pMP4Buf = NULL;
	
	while (pMP4Buf == NULL)
	{
		iothread_EventRead (&pMP4Buf, NULL, NULL, NULL);
	}
	
	*ppucInputBuf = (UCHAR*)NON_CACHE(pMP4Buf->aucBitstream);
	*piInputLen = pMP4Buf->uLength;
	
	//diag_printf("video: %x %x\n", pMP4Buf, *ppucInputBuf);
}


static void rtspFreeAudioBuf(void *pBuffer)
{
	VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T *pAudioBuf;
	pAudioBuf = GetParentAddr(pBuffer, VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T, aucAudio);
	
	//diag_printf("free audio: %x %x\n", pAudioBuf, pBuffer);
	
	bufEncAudioDecRef(pAudioBuf);
}


static void rtspGetBackAudio (unsigned char** ppucInputBuf, int *piInputLen)
{
	VP_BUFFER_ENCODED_AUDIO_BITSTREAM_T	*pAudioBuf = NULL;
	
	while (pAudioBuf == NULL)
	{
		iothread_EventRead (NULL, NULL, &pAudioBuf, NULL);
	}
	
	*ppucInputBuf = pAudioBuf->aucAudio;
	*piInputLen = pAudioBuf->nSize;
	
	//diag_printf("audio: %x %x\n", pAudioBuf, *ppucInputBuf);
}


static void rtspEnableVideoEncoder ()
{
	wb702EnableMP4Encoder (TRUE);
	//cyg_mutex_unlock (&g_RtspData->lock);
}

static void rtspDisableVideoEncoder ()
{
    //while(cyg_mutex_trylock (&g_RtspData->lock) == FALSE)
    //	cyg_thread_yield();
    //cyg_mutex_lock(&g_RtspData->lock);
	wb702EnableMP4Encoder (FALSE);
}

static void rtpsInitWBDevice ()
{
}

static void add_config(CMD_AUDIO_FORMAT_E audiotype,CMD_VIDEO_FORMAT_E videotype)
{
    const char *cpcAudioFormat = NULL;
		const char *cpcVideoFormat = NULL;
const char *cpcAudioFormatIn = NULL;
const char *cpcAudioFormatOut = NULL;
		switch (audiotype)
		{
			case CMD_AUDIO_AMR:
				cpcAudioFormat = "AMR";
				break;
			case CMD_AUDIO_PCM:
				cpcAudioFormat = "PCM";
				cpcAudioFormatIn = "PCM";
				cpcAudioFormatOut = "ALAW";
				break;
            case CMD_AUDIO_ALAW:
                cpcAudioFormat = "ALAW";
				break;
			case CMD_AUDIO_IMA_ADPCM:
			default:
				cpcAudioFormat = "IMAADPCM";
		}
		
		switch (videotype)
		{
			case CMD_VIDEO_H263:
				cpcVideoFormat = "H263";
				break;
			case CMD_VIDEO_MP4:
			default:
				cpcVideoFormat = "Mpeg4";
		}		
    sprintf(rtsp_conf_buf, g_RtspServer_conf, cpcAudioFormat, cpcVideoFormat, cpcVideoFormat, cpcAudioFormat);
    //sprintf(rtsp_conf_buf, g_RtspServer_conf, cpcAudioFormat, cpcVideoFormat, cpcAudioFormatIn,cpcAudioFormatOut,cpcVideoFormat, cpcAudioFormatOut);
}

static void rtsp_set_led(void)
{
}

static void rtsp_start(cyg_addrword_t pParam)
{
	/*Set wlan*/
	ledSetNetwork(LED_NETWORK_TRY_CONNECT);
	
#if 1
	SetWlan();
	//SetIP();
#else
	while (!SetWlan() || !SetIP())
	{
		tt_msleep(1000);
	}
#endif
	//ledSetNetwork(LED_NETWORK_READY);
	
	/* Init upnp */
	//YA 
	if (g_ConfigParam.bEnableUPnP)
	{
		extern void init_upnp(unsigned short base_port, unsigned short *port, unsigned char *proto);
		static unsigned short __port[5] = {0, 0, 0, 0, 0};
		static unsigned char __proto[5] = {0, 0, 0, 0, 0};
		
		__port[0] = (g_ConfigParam.usHttpPort[0] == 0 ? 80 : g_ConfigParam.usHttpPort[0]);
		__proto[0] = 0;
		__port[1] = 554;
		__proto[1] = 0;
		__port[2] = 554;
		__proto[2] = 1;
		
		init_upnp((g_ConfigParam.usUPnPPort == 0 ? DEFAULT_UPNP_FIRST_EXT_PORT : g_ConfigParam.usUPnPPort),
			__port, __proto);
	}
	
	/* Fix it: Disable bypass server */
#if 0
		BPC_New (
			//"10.132.249.100",	8888,
			"rovio.gostai.com",	21846,
			//g_ConfigParam.acBPSDomainName, g_ConfigParam.iBPSPort,
			(g_ConfigParam.usHttpPort[0] == 0 ? 80 : g_ConfigParam.usHttpPort[0])
			);
#else
	if (g_ConfigParam.bEnableBPS)
	{
		BPC_New (
			//"10.132.249.100",	8888,
			//"rovio.gostai.com",	21846,
			g_ConfigParam.acBPSDomainName,
			g_ConfigParam.iBPSPort,
			(g_ConfigParam.usHttpPort[0] == 0 ? 80 : g_ConfigParam.usHttpPort[0])
			);
	}
#endif

	
	prdAddTask_CheckNetwork();
	prdAddTask__CheckDDNS();
	
	
	/* Set user and pass for RTSP */
	{
		int i;
		for (i = 0; i < g_ConfigParam.iUserCurNum; i++)
			set_auth_id(g_ConfigParam.aacUserName[i], g_ConfigParam.aacUserPass[i]);
		if (g_ConfigParam.bUserCheck)
			set_auth_enable( );
		else
			set_auth_disable( );
	}

	
	
    
    set_config(rtsp_conf_buf);
    init_wbdevice(rtpsInitWBDevice);
	Set_Swap_FD_Func(&rtspSwapHttpTunnelFD, rtspEndSwapHttpFD);
	set_encoderenable(rtspEnableVideoEncoder);
	set_encoderdisable(rtspDisableVideoEncoder);
	init_get_video(rtspGetBackVideo);
	init_get_audio(rtspGetBackAudio);
	set_unref_buf_video(rtspFreeVideoBuf);
	set_unref_buf_audio(rtspFreeAudioBuf);
	RtspServerStart (0,rtsp_set_led);
}
#if 0
void rtsp_thread_join()
{
    do
    {
        cyg_thread_get_info(rtsp_handle, rtsp_thread.unique_id, &rtsp_info);
        switch(rtsp_info.state)
        {
            case EXITED:
                cyg_thread_delete(rtsp_handle);
                break;
            default:
                cyg_thread_yield();
                break;
        }
    }while(rtsp_info.state<16);
}
#else
void rtsp_thread_join()
{
    BOOL ex = FALSE;
    cyg_thread_info rtsp_info;
    while((ex == FALSE) && (rtsp_handle != NULL))
    {
        cyg_thread_get_info(rtsp_handle, rtsp_thread.unique_id, &rtsp_info);
            //diag_printf("state is %d\n",info->state);
       
        switch(rtsp_info.state)
        {
            case 16:
                cyg_thread_delete(rtsp_handle);
                ex = TRUE;
                break;
            default:
                cyg_thread_yield();
                break;
        }
    }
}
#endif
void RtspThreadRun(W99702_DATA_EXCHANGE_T* W99702, CMD_AUDIO_FORMAT_E audiotype, CMD_VIDEO_FORMAT_E videotype)
{
	if(g_RtspData == NULL && W99702 != NULL)
		g_RtspData = W99702;
    add_config(audiotype, videotype);
    cyg_thread_create(9, &rtsp_start, NULL, "rtsp_start", rtsp_stack, 
                        STACKSIZE, &rtsp_handle, &rtsp_thread);
    cyg_thread_resume(rtsp_handle);
} 

void RtspThreadRelease()
{
    exit_event_loop();
    diag_printf("Rtsp thread seems can not be terminated now..\n");
    //if(rtsp_handle != NULL)
    //    rtsp_thread_join();
}











/* Http tunnel for RTSP */
typedef struct
{
	int fd;
	
	cyg_mutex_t mutex;
	cyg_cond_t cond;
	int *piSwapHttpFD;
	
	int nRefCount;
	char acXSession[64];
	char acInput[4];
	size_t szInput;
	char acOutput[128];
} RTSP_TUNNEL_T;

RTSP_TUNNEL_T g_RtspTunnels[3];


static RTSP_TUNNEL_T *rtspTunnelSearch(const char *pcXSession)
{
	int i;	
	/* Look up the slot with pcXSession */
	RTSP_TUNNEL_T *pRtsp = NULL;
	
	cyg_scheduler_lock();
	diag_printf("Search session: %s\n", pcXSession);
	for (i = 0; i < sizeof(g_RtspTunnels) / sizeof(g_RtspTunnels[0]); ++i)
	{
		if (g_RtspTunnels[i].nRefCount > 0 && strcmp (g_RtspTunnels[i].acXSession, pcXSession) == 0)
		{
			pRtsp = &g_RtspTunnels[i];
			break;
		}
	}
	cyg_scheduler_unlock();
	if (pRtsp == NULL)
		return NULL;
		
	++pRtsp->nRefCount;	
	diag_printf("Found rtsp %x\n", pRtsp);
	
	
	
	
	
	return pRtsp;
}


static RTSP_TUNNEL_T *rtspTunnelCreate(const char *pcXSession)
{
	int i;
	struct sockaddr_in socket_addr;
	
	/* Look up a null slot */
	RTSP_TUNNEL_T *pRtsp = rtspTunnelSearch(pcXSession);
	if (pRtsp != NULL)
		return pRtsp;
	
	cyg_scheduler_lock();
	diag_printf("Create session: %s\n", pcXSession);
	for (i = 0; i < sizeof(g_RtspTunnels) / sizeof(g_RtspTunnels[0]); ++i)
	{
		if (g_RtspTunnels[i].nRefCount <= 0)
		{
			pRtsp = &g_RtspTunnels[i];
			break;
		}
	}
	cyg_scheduler_unlock();
	
	if (pRtsp == NULL)
		return NULL;
	
	cyg_mutex_init(&pRtsp->mutex);
	cyg_cond_init(&pRtsp->cond, &pRtsp->mutex);
	pRtsp->piSwapHttpFD = NULL;
	
	pRtsp->nRefCount = 1;
	snprintf (pRtsp->acXSession, sizeof(pRtsp->acXSession), "%s", pcXSession);
	pRtsp->szInput = 0;
	
	
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_port = htons(554);
	socket_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	socket_addr.sin_len = sizeof(socket_addr);
	bzero(&(socket_addr.sin_zero), 8);
	if( (pRtsp->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket create error\n");
		goto lSocket;
	}

	if(connect(pRtsp->fd, (struct sockaddr*)&socket_addr, sizeof(struct sockaddr)) < 0)
	{
		printf("connect server error\n");
		goto lConnect;        
	}
	
	/* Write rtsp fd to rtsp server */
	sprintf(pRtsp->acOutput,
		"NX_HTTPFD rtsp://127.0.0.1/webcam/ RTSP/1.0\r\n"
		"HANDLE: %d\r\n"
		"\r\n", (int)pRtsp);
	diag_printf("[%d][%s]\n", strlen(pRtsp->acOutput), pRtsp->acOutput);
	write(pRtsp->fd, pRtsp->acOutput, strlen(pRtsp->acOutput));
	
	printf("rtsp tunnel created, handle = %x\n", (int)pRtsp);
	return pRtsp;



lConnect:
	close(pRtsp->fd);
lSocket:
	pRtsp->nRefCount = 0;
	return NULL;	
}

static void rtspTunnelDelete(RTSP_TUNNEL_T *pRtsp)
{
	/* Free tunnel */
	cyg_scheduler_lock();
	
	--pRtsp->nRefCount;
	if(pRtsp->nRefCount == 0)
	{
		diag_printf("Real remove sessions\n");
		if (pRtsp->fd != -1)
			close(pRtsp->fd);
			
		
		cyg_cond_destroy(&pRtsp->cond);
		cyg_mutex_destroy(&pRtsp->mutex);
	}
	
	cyg_scheduler_unlock();
}




static int rtspHttpReqOver(HTTPCONNECTION hConnection, void *pParam)
{
	RTSP_TUNNEL_T *pRtsp = (RTSP_TUNNEL_T *)pParam;
	diag_printf("rtspHttpReqOver, refcount=%d\n", pRtsp->nRefCount);
	
	rtspTunnelDelete(pRtsp);
	
	return 0;
}


static int rtspRtyRead(int fd, char *buffer, size_t len)
{
	struct timeval tv;
	fd_set fdsRead;


	FD_ZERO(&fdsRead);
	FD_SET(fd, &fdsRead);
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	if (select(fd+1, &fdsRead, NULL, NULL, &tv) > 0)
	{
		if (FD_ISSET(fd, &fdsRead))
		{
			int rt;
			rt = read(fd, buffer, len);
			if(rt == 0)
				return -1;
			else if(rt == -1)
			{
				if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)
					return 0;
				else
					return -1;
			}
			return rt;
		}
	}
	return 0;
}



static void rtspSwapHttpTunnelFD(void *pTunnelHandle, int *swap_fd)
{
	int iSwapHttpFD = 0;
	RTSP_TUNNEL_T *pRtsp = (RTSP_TUNNEL_T *)pTunnelHandle;
	
	diag_printf("Swap fd for %x\n", pRtsp);
	
	cyg_mutex_lock(&pRtsp->mutex);
	pRtsp->piSwapHttpFD = &iSwapHttpFD;
	cyg_cond_wait(&pRtsp->cond);
	cyg_mutex_unlock(&pRtsp->mutex);	
	
	diag_printf("Get swap http fd = %d\n", iSwapHttpFD);
	
	*swap_fd = iSwapHttpFD;
}


static void rtspEndSwapHttpFD(void *pTunnelHandle)
{
	rtspTunnelDelete((RTSP_TUNNEL_T *)pTunnelHandle);
}


static int rtspTunnelSendData(HTTPCONNECTION hConnection, time_t *ptLastFill, void *pParam)
{
	RTSP_TUNNEL_T *pRtsp = (RTSP_TUNNEL_T *)pParam;

	int rt;
	
	//diag_printf("About to send data: %d\n", *ptLastFill);
	if (*ptLastFill == (time_t) 0)
		*ptLastFill = time(0);
	
	if (time(0) - *ptLastFill >= 20)
	{
		diag_printf("Timeout\n");
		return 0;
	}
	
	
	if (pRtsp->piSwapHttpFD != NULL)
	{
		diag_printf("About swap fd\n");
		cyg_mutex_lock(&pRtsp->mutex);
		(*pRtsp->piSwapHttpFD) = httpSwapFD(hConnection, -1);
		
		++pRtsp->nRefCount;
		cyg_cond_signal(&pRtsp->cond);
		cyg_mutex_unlock(&pRtsp->mutex);
		return 0;	//close session	
	}
	
	rt = rtspRtyRead( pRtsp->fd, pRtsp->acOutput, sizeof(pRtsp->acOutput));
	if (rt == 0)
	{
		//nothing was read, continue to reading
		return 1;
	}
	else if (rt < 0)
	{
		//close session
		return 0;
	}
	else
	{
		*ptLastFill = time(0);
		//diag_printf("read data from fd = %d, read length=%d\n", pRtsp->fd, rt);
		//write(1, pRtsp->acOutput, rt);
		
		//send and encode rtsp data */
		httpAddBody(hConnection, pRtsp->acOutput, rt);	
		return 1;
	}
}


int rtspTunnelReadData(HTTPCONNECTION hConnection,
								int *piPostState,
								char **ppcPostBuf,
								int *piPostBufLen,
								int *piPostDataLen,
								char *pcFillData,
								int iFillDataLen,
								int iIsMoreData/*bool*/,
								void *pParam/*other parameter for extend use*/)
{
	char acDecode[256];
	size_t szDecode;
	RTSP_TUNNEL_T *pRtsp = (RTSP_TUNNEL_T *)pParam;
	
	diag_printf("Read len %d\n", iFillDataLen);
	//write(1, pcFillData, iFillDataLen);
	
	/* Copy the szInput temp buffer */
	if (pRtsp->szInput != 0)
	{
		size_t szLen = sizeof(pRtsp->acInput) - pRtsp->szInput;
		if (szLen > iFillDataLen)
			szLen = iFillDataLen;
		memcpy (pRtsp->acInput + pRtsp->szInput, pcFillData, szLen);

		pRtsp->szInput += szLen;
		pcFillData += szLen;
		iFillDataLen -= szLen;
		
		assert (pRtsp->szInput <= sizeof(pRtsp->acInput));
		
		if (pRtsp->szInput >= sizeof(pRtsp->acInput))
		{
			szDecode = DecodeBase64(acDecode, sizeof(acDecode), pRtsp->acInput, pRtsp->szInput);
			--szDecode;	//Cut the tailing '\0'
			
			write(pRtsp->fd, acDecode, szDecode);
			//write(1, acDecode, szDecode);
			pRtsp->szInput = 0;	/* Clear szInput temp buffer */
		}
	}
	
	/* Decode the remaining buffer */
	while (iFillDataLen >= sizeof(pRtsp->acInput))
	{
		size_t szLen = iFillDataLen / 4 * 4;
		if (szLen >= sizeof(acDecode) / 3 * 4)
			szLen = sizeof(acDecode) / 3 * 4;
		
		if (szLen <= 0)
			break;
		
		szDecode = DecodeBase64(acDecode, sizeof(acDecode), pcFillData, szLen);
		--szDecode;	//Cut the tailing '\0'
		
		write(pRtsp->fd, acDecode, szDecode);
		//write(1, acDecode, szDecode);
			
		pcFillData += szLen;
		iFillDataLen -= szLen;
	}
	
	/* Copy the remaining buffer to temp buffer */
	assert (pRtsp->szInput + iFillDataLen < sizeof(pRtsp->acInput));
	memcpy (pRtsp->acInput + pRtsp->szInput, pcFillData, iFillDataLen);
	pRtsp->szInput += iFillDataLen;
	
	
	return 1;
}


int Http_RtspTunnel(HTTPCONNECTION hConnection, void *pParam)
{
	diag_printf("In Http_RtspTunnel (method=%d)\n", httpGetMethod(hConnection));
	
	if (httpGetMethod(hConnection) == M_GET)
	{
		RTSP_TUNNEL_T *pRtspTunnel;
		const char *pcXSession;
		char acExtra[256];
		httpSetKeepAliveMode(hConnection, FALSE);
	
		pcXSession = httpGetXSessionCookie(hConnection);
		diag_printf("GET:[%s]\n", pcXSession);
		if (pcXSession == NULL || pcXSession[0] == '\0')
			return -1;

		pRtspTunnel = rtspTunnelCreate(pcXSession);
		if (pRtspTunnel == NULL)
			return -1;
		else
		{
			sprintf(acExtra, "Server: Nuvoton IPCam\r\n"
				"Pragma: no-cache\r\n"
				"Cache-Control: no-cache\r\n"
				"Expires: 01 Jan 1970 00:00:00 GMT\r\n"
				);
			httpSetHeader(hConnection, 200, "OK", "",
					acExtra,
					"application/x-rtsp-tunnelled", FALSE);

			httpSetSendDataOverFun(hConnection, rtspTunnelSendData, pRtspTunnel);
			httpSetRequestOverFun(hConnection, rtspHttpReqOver, pRtspTunnel);
			return 0;
		}

	}
	else if (httpGetMethod(hConnection) == M_POST)
	{
		RTSP_TUNNEL_T *pRtspTunnel;
		const char *pcXSession;
		
		pcXSession = httpGetXSessionCookie(hConnection);
		diag_printf("POST:[%s]\n", pcXSession);
		if (pcXSession == NULL || pcXSession[0] == '\0')
			return 0;	/* Delete the request */
		
		pRtspTunnel = rtspTunnelCreate(pcXSession);
		if (pRtspTunnel == NULL)
			return 0;	/* Delete the request */
		
		httpSetPostDataFun(hConnection, rtspTunnelReadData, pRtspTunnel);
		httpSetRequestOverFun(hConnection, rtspHttpReqOver, pRtspTunnel);
		return 1;
	}
}





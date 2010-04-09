
#include "../../Inc/CommonDef.h"
#ifdef RECORDER
static const char *g_cst_pcHtmlTemplate1 =
"<HTML>\n"
"<HEAD>\n"
"<TITLE>Web Camera Warning</TITLE>\n"
"<META NAME='Author' CONTENT='ChenXiaohui'>\n"
"<style>\n"
".btactive	{ text-decoration:none; color:black; width:22; height:24; font-size:16; border: inset 2 white;}\n"
".btlink		{ text-decoration:none; color:black; width:22; height:24; font-size:16; border: solid 2 #d4d0c8; }\n"
".bthover	{ text-decoration:none; color:black; width:22; height:24; font-size:16; border: outset 2 white;}\n"
"</style>\n"
"<SCRIPT>\n"
"var aImgList = new Array();\n"
"var aTimeList = new Array();\n"
"var iImgIndex = -1;\n"
"var bRun = null;\n"
"\n"
"function ExpImg()\n"
"{\n"
"	if (oExp.innerHTML.charCodeAt(0) == 8857)\n"
"	{\n"
"		oExp.innerHTML = String.fromCharCode(164);\n"
"		oExpDiv.style.display = 'none';\n"
"	}\n"
"	else\n"
"	{\n"
"		oExp.innerHTML = String.fromCharCode(8857);\n"
"		oExpDiv.style.display = 'inline';\n"
"	}\n"
"}\n"
"\n"
"function BodyLoad()\n"
"{\n"
"	for (i=2; i<oImgList.length; i++)\n"
"	{\n"
"		aImgList[i-2] = oImgList[i].src;\n"
"		aTimeList[i-2] = oTimeList[i].innerText;\n"
"	}\n"
"	oCtl.style.display = 'inline';\n"
"	oExpDiv.style.display = 'none';\n"
"	bRun = true;\n"
"	ImgShow();\n"
"}\n"
"\n"
"function PauseImg()\n"
"{\n"
"	bRun = false;\n"
"}\n"
"\n"
"function StartImg()\n"
"{\n"
"	bRun = true;\n"
"	ImgShow();\n"
"}\n"
"\n"
"function ImgShow()\n"
"{\n"
"	if (bRun)\n"
"	{\n"
"		if (aImgList.length > 0)\n"
"		{\n"
"			if (++iImgIndex >= aImgList.length) iImgIndex = 0;\n"
"			oStart.disabled = true;\n"
"			oPause.disabled = false;\n"
"			oImgShow.src = aImgList[iImgIndex];\n"
"			oTimeShow.innerHTML = '<nobr>' + aTimeList[iImgIndex] + '</nobr>';\n"
"			oTimeShow.style.left = oCtl.offsetLeft + 5;\n"
"			oTimeShow.style.top = oCtl.offsetTop + 5;\n"
"			oSlider.value = oSlider.Min + iImgIndex * Math.floor((oSlider.Max-oSlider.Min+1)/(aImgList.length));\n"
"\n"
"			setTimeout('ImgShow();', 200);\n"
"		}\n"
"	}\n"
"	else\n"
"	{\n"
"		oPause.disabled = true;\n"
"		oStart.disabled = false;\n"
"		ImgFromSlider();\n"
"	}\n"
"}\n"
"\n"
"function ShowClick()\n"
"{\n"
"	if (bRun) PauseImg();\n"
"	else StartImg();\n"
"}\n"
"\n"
"function ImgFromSlider()\n"
"{\n"
"	if (aImgList.length > 0)\n"
"	{\n"
"		iImgIndex = Math.floor(oSlider.value * aImgList.length / (oSlider.Max - oSlider.Min));\n"
"		if (iImgIndex < aImgList.length)\n"
"			oImgShow.src = aImgList[iImgIndex];\n"
"	}\n"
"}\n"
"\n"
"function CHF() {oCtl.focus();}\n"
"function MOV(o) {o.className = 'bthover'; CHF();}\n"
"function MOO(o) {o.className = 'btlink'; CHF();}\n"
"function MDW(o) {o.className = 'btactive'; CHF();}\n"
"function MUP(o) {o.className = 'btlink'; CHF();}\n"
"</SCRIPT>\n"
"<SCRIPT FOR=oSlider EVENT='Scroll'>\n"
"	PauseImg();\n"
"	ImgFromSlider();\n"
"</SCRIPT>\n"
"</HEAD>\n"
"\n"
"<BODY>\n"
"<div id=oCtl style='{display:none; width:1pt; border: solid 2pt #d4d0c8; background-color:#d4d0c8;}'>\n"
"<img id=oImgShow onclick='ShowClick();' oncontextmenu='PauseImg();return true;' width=300 height=225><font id=oTimeShow style='{font-size:10pt; color:lightgreen; position:absolute;}'></font>\n"
"<br style='{font-size:4pt;}'>\n"
"<script>\n"
"document.write(\n"
"\"<object id=oSlider classid='clsid:373FF7F0-EB8B-11CD-8820-08002B2F4F5A' id='Slider1' width=300 height=18>\"\n"
"+\"	<param name='_ExtentX' value='2646'>\"\n"
"+\"	<param name='_ExtentY' value='1111'>\"\n"
"+\"	<param name='_Version' value='327682'>\"\n"
"+\"	<param name='BorderStyle' value='0'>\"\n"
"+\"	<param name='MousePointer' value='0'>\"\n"
"+\"	<param name='Enabled' value='1'>\"\n"
"+\"	<param name='OLEDropMode' value='0'>\"\n"
"+\"	<param name='Orientation' value='0'>\"\n"
"+\"	<param name='LargeChange' value='10'>\"\n"
"+\"	<param name='SmallChange' value='1'>\"\n"
"+\"	<param name='Min' value='0'>\"\n"
"+\"	<param name='Max' value='99'>\"\n"
"+\"	<param name='SelectRange' value='0'>\"\n"
"+\"	<param name='SelStart' value='0'>\"\n"
"+\"	<param name='SelLength' value='0'>\"\n"
"+\"	<param name='TickStyle' value='3'>\"\n"
"+\"	<param name='TickFrequency' value='1'>\"\n"
"+\"	<param name='Value' value='0'>\"\n"
"+\"</object>\"\n"
");\n"
"</script>\n"
"<nobr>\n"
"<button id=oStart onclick='StartImg();' class=btlink onmouseover='MOV(this);' onmouseout='MOO(this);' onmousedown='MDW(this);' onmouseup='MUP(this);'>&#9658;</button>\n"
"<button id=oPause onclick='PauseImg();' class=btlink onmouseover='MOV(this);' onmouseout='MOO(this);' onmousedown='MDW(this);' onmouseup='MUP(this);'>&#8214;</button>\n"
"<button id=oExp onclick='ExpImg();' class=btlink onmouseover='MOV(this);' onmouseout='MOO(this);' onmousedown='MDW(this);' onmouseup='MUP(this);'>&#164;</button><font disabled style='{font-size:18pt;}'>|</font>\n"
"<font id=oImgList style='{font-size:12pt; font-family:Arial Black; color:orange;}'><i>Image Viewer</i></font>\n"
"</nobr>\n"
"</div><font id=oTimeList><font id=oTimeList><br id=oImgList>\n"
"<div id=oExpDiv style='{display:inline;}'>\n";

const char *g_cst_pcHtmlTemplate2 =
"</div>\n"
"\n"
"</BODY>\n"
"<SCRIPT>setTimeout('BodyLoad();', 0);</SCRIPT>\n"
"</HTML>\n";
#endif

typedef struct
{
	char *pcBmpBuf;
	unsigned short usResX, usResY;
	RECT_T rMoniteRect;
	time_t tTime;
} IMG_COMP_INFO;

#if 0
char *strreplace(char *src, char *oldstr, char *newstr)   
{   
    char  *needle = NULL;   
    char *p1 = src;
    char *des;
    char *p2;
    int dessize = 640;
    int size = strlen(newstr)-strlen(oldstr);
    if ((des = (char *)malloc(dessize)) == NULL)
    	return NULL;
    p2= des;
    memset(des,0,dessize);
    if(strstr(src, oldstr) == NULL)
    {
    	snprintf(des,dessize,"%s",src);
    	return des;
    }	
    while(((needle = strstr(p1, oldstr)) != NULL) && (dessize >= (strlen(p1)+size)))   
    {         
        //diag_printf("p1=%s,des=%s,p2=%s,needle=%s\n",p1,des,p2,needle);
        strncpy(p2, p1, needle-p1);   
        p2[needle-p1]='\0';   
        strcat(p2, newstr);   
        strcat(p2, needle+strlen(oldstr)); 
        p2 = p2 + (needle - p1) +strlen(newstr);
        dessize = dessize - (needle - p1) - strlen(newstr);
        p1 = needle+strlen(oldstr);
    }
    return des;
}
extern char *g_version[];
#endif

static time_t mytime(void)
{
    time_t cur_sec;
    cyg_tick_count_t cur_time;
    cur_time = cyg_current_time();
    cur_sec = (cur_time*10) / 1000;
    return cur_sec;
}

void SetCameraInitState(void)
{
	SendCameraMsg(MSG_CAMERA_RES, g_ConfigParam.ulImgResolution);
	SendCameraMsg(MSG_CAMERA_QUALITY, g_ConfigParam.ulImgQuality);
	SendCameraMsg(MSG_CAMERA_MOTION, 0);
	SendCameraMsg(MSG_CAMERA_FRAMERATE, g_ConfigParam.ulImgFramerate);
	SendCameraMsg(MSG_CAMERA_BRIGHTNESS, g_ConfigParam.ulImgBrightness);
	SendCameraMsg(MSG_SPEAKER_VOLUME, g_ConfigParam.ulSpeakerVolume);
	SendCameraMsg(MSG_MIC_VOLUME, g_ConfigParam.ulMicVolume);	
	SendCameraMsg(MSG_SET_LOGO,g_ConfigParam.ShowTime);
	//SendCameraMsg(MSG_CAMERA_DIRECTION, g_ConfigParam.ulImgDirection);
}

BOOL SaveJpeg(char *pcJpeg, int iJpegLen, BOOL bIsMotionDetected)
{
	int iNextReadyImg;
	int iNextPicIndex; 
	int iNextMotionDetectedIndex;

	/* 设置新的抓到的图像在g_WebCamState.acImgBuf总的序号 */
	iNextReadyImg = g_WebCamState.iReadyImg + 1;
	if (iNextReadyImg >= 2) iNextReadyImg = 0;

	/* 设置新的图像序号 */
	if (g_WebCamState.uiPicIndex == INVALID_PICTURE)
		iNextPicIndex = 0;
	else iNextPicIndex = g_WebCamState.uiPicIndex + 1;
	if (iNextPicIndex >= MAX_IMG_INDEX) iNextPicIndex = 0;

	/* 设置新的motion detect序号 */
	if (g_WebCamState.uiPicMotionDetectedIndex == INVALID_PICTURE)
		iNextMotionDetectedIndex = 0;
	else iNextMotionDetectedIndex = g_WebCamState.uiPicMotionDetectedIndex + 1;
	if (iNextMotionDetectedIndex >= MAX_IMG_INDEX) iNextMotionDetectedIndex = 0;

	
	cyg_mutex_lock(&g_ptmState);
	g_WebCamState.iReadyImg = iNextReadyImg;
	g_WebCamState.uiPicIndex = iNextPicIndex;
	g_WebCamState.uiPicSize = iJpegLen;
	//g_WebCamState.ucBright = g_ConfigParam.ulImgBrightness;
	g_WebCamState.ucContrast = g_ConfigParam.ulImgContrast;
	g_WebCamState.ucSaturation = g_ConfigParam.ulImgSaturation;
	g_WebCamState.ucHue = g_ConfigParam.ulImgHue;
	g_WebCamState.ucSharpness = g_ConfigParam.ulImgSharpness;
	if (bIsMotionDetected)
		g_WebCamState.uiPicMotionDetectedIndex = iNextMotionDetectedIndex;
	cyg_mutex_unlock(&g_ptmState);
	vcptJpegTimerAck();
	return TRUE;
}
#ifdef RECORDER
void Do_TestSendIpMail(BOOL *bSendIpMail)
{
	unsigned long ulPublicIpAddress;
	unsigned long ulPublicSubnetMask;
	unsigned long ulDefaultGateway;
	unsigned long aulDNSServer[3];
	char acContent[256];
	int iLen;
	int i;
	MAIL_MEM* mail_mem = NULL;

	if (!(*bSendIpMail)) return;
	*bSendIpMail = FALSE;
	
	//if (!g_ConfigParam.bIfMailOnDialed) return;
	GetPubIPInfo(&ulPublicIpAddress,
				&ulPublicSubnetMask,
				&ulDefaultGateway,
				aulDNSServer);
	iLen = sprintf(acContent, "IP: ");
	iLen += httpIP2String(ulPublicIpAddress, acContent + iLen);
	iLen += sprintf(acContent + iLen, "\nNetmask: ");
	iLen += httpIP2String(ulPublicSubnetMask, acContent + iLen);
	iLen += sprintf(acContent + iLen, "\nGateway: ");
	iLen += httpIP2String(ulDefaultGateway, acContent + iLen);
	for (i = 0; i < 3; i++)
	{
		if (aulDNSServer[i] != 0)
		{
			iLen += sprintf(acContent + iLen, "\nDNS Server %d: ", i);
			iLen += httpIP2String(aulDNSServer[i], acContent + iLen);
		}
	}


	cyg_mutex_lock(&g_ptmConfigParam);
	//update dynamic DNS if possible
	diag_printf("proxy server is %s\n",g_ConfigParam.acProxy);
	if (g_ConfigParam.bEnableDDNS)
		Do_DNS(g_ConfigParam.acDDNSDomainName,
			ulPublicIpAddress,
			g_ConfigParam.acDDNSUserName,
			g_ConfigParam.acDDNSUserPass,
			g_ConfigParam.acProxy,
			g_ConfigParam.acProxyUser,
			g_ConfigParam.acProxyPass);


	if (g_ConfigParam.acDial_MailSender[0] != '\0'
		&& g_ConfigParam.acDial_MailReceiver[0] != '\0'
		&& g_ConfigParam.acDial_MailServer[0] != '\0')
	{		
		if(get_mail_mem(&mail_mem) == FALSE)
			diag_printf("Not enough mail memory!\n");
		else
		{
			sendMailMsg(
				g_ConfigParam.acDial_MailSender,
				g_ConfigParam.acDial_MailReceiver,"","",
				(g_ConfigParam.acDial_MailSubject[0]=='\0'?"Web Camera PPP Dialed!":g_ConfigParam.acDial_MailSubject),
				acContent,
				0,
				NULL,
				g_ConfigParam.acDial_MailServer,
				g_ConfigParam.acDial_MailUser,
				(g_ConfigParam.bDial_MailCheck?g_ConfigParam.acDial_MailPassword:NULL),
				MAIL_PPPOE_IP,
				mail_mem);
		}
	}
	cyg_mutex_unlock(&g_ptmConfigParam);
	//cyg_thread_yield();
}
/**/
void Do_TestSendMail(LIST **pplPic2Mail, time_t *ptLastMail, BOOL bForceSend)
{
	char *pcContent;
	int i;
	int iLenList;
	time_t tThisMail;

	iLenList = httpGetListLength(*pplPic2Mail);
	cyg_mutex_lock(&g_ptmConfigParam);
	tThisMail = mytime();

	if ( iLenList > 0 &&
		(bForceSend || iLenList >= MAX_IMAGE_IN_ON_MAIL || tThisMail - *ptLastMail > 10))
	{
		if (g_ConfigParam.acMailSender[0] != '\0'
			&& g_ConfigParam.acMailReceiver[0] != '\0'
			&& g_ConfigParam.acMailServer[0] != '\0')
		{
			int j;
			LISTNODE *pNode;
			FILE_BUF_T *pFb;
			MAIL_MEM* mail_mem = NULL;

			pcContent = malloc(256 * iLenList
				+ strlen(g_cst_pcHtmlTemplate1)
				+ strlen(g_cst_pcHtmlTemplate2));
			if (pcContent != NULL)
			{
				j = 0;
				j += sprintf(pcContent+j, "%s", g_cst_pcHtmlTemplate1);

				if (*pplPic2Mail != NULL)
				{
					for (i=0, pNode = (*pplPic2Mail)->pFirstNode; pNode != (*pplPic2Mail)->pLastNode && i<iLenList; pNode = pNode->pNextNode, i++)
					{
						pFb = (FILE_BUF_T *)pNode->pValue;
						if (pFb)
						{
							char acTime[32];
							time_t tRealTime;
							struct tm tmGRealTime;
							cyg_mutex_lock(&g_ptmTimeSetting);
							tRealTime = pFb->st.st_mtime + g_lTimeDelay1970_InSec - (GetSystemTimeZone() + g_lTimeDelayZone_InSec);
							cyg_mutex_unlock(&g_ptmTimeSetting);
							//cyg_thread_yield();
							gmtime_r(&tRealTime, &tmGRealTime);
							asctime_r(&tmGRealTime, acTime);
							j += sprintf(pcContent+j, "<font id=oTimeList>%s:</font><br><img id=oImgList src='cid:Attachment%d'><p>", acTime, i);
						}
					}
				}
				j += sprintf(pcContent+j, "%s", g_cst_pcHtmlTemplate2);				
			
				if(get_mail_mem(&mail_mem) == FALSE)
					diag_printf("Not enough mail memory!\n");
				else
				{
					sendMailMsg(
						g_ConfigParam.acMailSender,
						g_ConfigParam.acMailReceiver,"","",
						(g_ConfigParam.acMailSubject[0]=='\0'?"Web Camera Warning!":g_ConfigParam.acMailSubject),
						pcContent,
						1,
						*pplPic2Mail,
						g_ConfigParam.acMailServer,
						g_ConfigParam.acMailUser,
						(g_ConfigParam.bMailCheck?g_ConfigParam.acMailPassword:NULL),
						MAIL_MOTION_DETECTED,
						mail_mem);
				}
				free(pcContent);
			}
			else
				PRINT_MEM_OUT;
		}
		else g_WebCamState.ucEmail = '\2';

		DeleteFileList(*pplPic2Mail);
		*pplPic2Mail = (LIST*)CreateFileList();
		*ptLastMail = tThisMail;
	}
	cyg_mutex_unlock(&g_ptmConfigParam);
	//cyg_thread_yield();
}
#endif


#if 0
cyg_thread g_ptdGetAudio;	/* pthread_t for CameraThread */
cyg_handle_t ptdGetAudio_handle;
#pragma arm section zidata = "non_init"
unsigned char ptdGetAudio_stack[STACKSIZE2];
#pragma arm section zidata
volatile BOOL AudioExit = FALSE; 

void GetAudioThread(cyg_addrword_t pParam)
{
	W99702_DATA_EXCHANGE_T * pExData = (W99702_DATA_EXCHANGE_T *)pParam;
	int iRt;
	IO_THREAD_READ_T readarg;
	
	while (!AudioExit)
	{		
		iRt = iothread_ReadAudio(&readarg);
		if(iRt == -1)
		{	
			diag_printf("Encode Core Quit!\n");
			iothread_ReadAudio_Complete(&readarg);	
			continue;
		}
		else if(iRt ==0)
		{
			diag_printf("%d buffer too small.\n",readarg.mediatype);
			iothread_ReadAudio_Complete(&readarg);	
			continue;
		}
		else
		{
			switch(readarg.mediatype)
			{
				case MEDIA_TYPE_AUDIO:
				//diag_printf("get audio\n");
					cyg_semaphore_wait(&(pExData->pcAudio).producer);
					memcpy((char*)(pExData->acAudioBuf), (char*)(readarg.txbuf), readarg.txlen);
					pExData->szAudioLen = readarg.txlen;
					iothread_ReadAudio_Complete(&readarg);
#ifdef RECORDER
					if((pExData->szAudioLen > 0) && RecordInit)
						recorder_add_audio(pExData->acAudioBuf,pExData->szAudioLen);
#endif
					cyg_semaphore_post (&(pExData->pcAudio).consumer);								
					break;
				default:
					break;			
			}
		}
	}
	
}

void Audio_Exit()
{
	cyg_thread_info GetAudioInfo;
	if (ptdGetAudio_handle != NULL)
	{
		AudioExit = TRUE;
		thread_join(&ptdGetAudio_handle,&g_ptdGetAudio,&GetAudioInfo);
		diag_printf("Close the GetAudio Thread!!!");
	}		
	
}
#endif

static BOOL Buffer_Assign(void)
{
	/*buffer for rtsp*/
	int server_size,client_size;
	char *server_mem,*client_mem;

	server_size = get_server_size();	
	server_mem = (char*)malloc(server_size);
	if(server_mem == NULL)
	{
		printf("not enough memory for rtsp server\n");
		return FALSE;
	}
	else 
		memset(server_mem,0,server_size);
	rtsp_server_init(server_mem, server_size);

	client_size = get_rtspmem_size(3);
	client_mem = (char*)malloc(client_size);
	if(client_mem == NULL)
	{
		printf("not enough memory for rtsp client\n");
		free(server_mem);
		return FALSE;
	}
	else
		memset(client_mem,0,client_size);		
	rtsp_mem_init(client_mem, client_size);	
	
#ifdef RECORDER	
	/*init recorder buf	*/
	if((recoreder_init_buf(g_ConfigParam.eVideoFormat,g_ConfigParam.eAudioFormat)) == 0)
	{
		diag_printf("Recorder buffer init ok!\n");
	}
	else
	{	
		recorder_free();
		diag_printf("Recorder buffer init false!\n");
	}
#endif
	return TRUE;
	
}

void CameraThread(cyg_addrword_t pParam)
{
	//IMG_COMP_INFO iPreImage;
#ifdef RECORDER
	LIST *plPic2Mail;	//待发文件列表
	time_t tMail;	//上次发送邮件的时间
	BOOL bSendIpMail = FALSE;
#endif
	MSG_T msg;	//消息
	unsigned int usedlen=0;
	BOOL bExit = FALSE;	//退出标志
	//BOOL bSendIpMail = TRUE; //是否发送IPdata的标志
	BOOL RecordInit = FALSE;
	
	UsedBuf* AsBuf = (UsedBuf*)pParam;
	W99702_DATA_EXCHANGE_T g_dataW99702;
	memset(&g_dataW99702, 0, sizeof(W99702_DATA_EXCHANGE_T));
	
	if (AsBuf == NULL) 
	{
		diag_printf("Camera do not have enough buf\n");
		return;
	}
	
	g_WebCamState.acImgBuf[0] = (char *)Asmalloc(AsBuf,&usedlen,MAX_CAMERA_IMG_LENGTH);	
	g_WebCamState.acImgBuf[1] = (char *)Asmalloc(AsBuf,&usedlen,MAX_CAMERA_IMG_LENGTH);
	if((g_WebCamState.acImgBuf[0] == NULL) || (g_WebCamState.acImgBuf[1] == NULL))
	{
		diag_printf("not enough buf for camera\n");
		return;
	}
			
	/*assigned buffer for rtsp and recorder*/	
	if(Buffer_Assign() == FALSE)
	{
		diag_printf("not enough buffer for rtsp");
		return;
	}
	
#ifdef RECORDER	
	plPic2Mail =(LIST*) CreateFileList();
	tMail = mytime();
#endif
	//iPreImage.pcBmpBuf = NULL;	
	
	while (!bExit)
	{	
		int iC = 0, tm;
		if (!TryGetMsg(g_pMsgCamera, &msg))
		{
#ifdef RECORDER
			Do_TestSendIpMail(&bSendIpMail);
#endif
			W99702_OpenCamera(&g_dataW99702,AsBuf,&usedlen);			
		}
		else if (msg.lMsg == MSG_QUIT) break;
		else
		{
			if (msg.lMsg == MSG_CAMERA_MONITOR_RECT
				|| msg.lMsg == MSG_CUSTOM_PROC)
				free((void *)msg.lData);
			continue;
		}
		
		g_WebCamState.ucCamera = CAMERA_ON;
		SetCameraInitState();
		iC = 0;tm = mytime();	
	
		//cyg_thread_create(PTD_PRIORITY, &GetAudioThread, (cyg_addrword_t) &g_dataW99702, "ptdGetAudio", ptdGetAudio_stack, STACKSIZE2, &ptdGetAudio_handle, &g_ptdGetAudio);
		//if ( ptdGetAudio_handle == NULL)
		//{
		//	fprintf(stderr, "Thread for ftp creation failed!\n");
		//	return;
		//}
		//cyg_thread_resume(ptdGetAudio_handle);
		
		while (!bExit)
		{
		
			if (!TryGetMsg(g_pMsgCamera, &msg))
			{
				int iNextReadyImg;
				char *pcJpeg;
				int *piJpegLen;
#ifdef RECORDER
				Do_TestSendIpMail(&bSendIpMail);
#endif
				iNextReadyImg = g_WebCamState.iReadyImg + 1;
				if (iNextReadyImg >= 2) iNextReadyImg = 0;
				pcJpeg = g_WebCamState.acImgBuf[iNextReadyImg];
				piJpegLen = &g_WebCamState.iImgBufLen[iNextReadyImg];
				if (W99702_GetOneImage(pcJpeg, MAX_CAMERA_IMG_LENGTH, piJpegLen,&g_dataW99702,RecordInit)
					&& (g_pOnGetImage == NULL || (*g_pOnGetImage)(pcJpeg, *piJpegLen)))
				{
					BOOL bMotionDetected = FALSE;
					if (TRUE)	//!bErrorImage)
					{						
						SaveJpeg(pcJpeg, *piJpegLen, bMotionDetected);						
					}
				}
				else
				{
					diag_printf("Return false\n");
					//break;	//读图像错误，退出，等待用户再次开启
				}
			}
			else 
			{
				diag_printf("Get message: %d\n", msg.lMsg);
				switch(msg.lMsg)
				{			
				case MSG_EXTERNEL_DETECTOR:     //??????
					break;
								
				case MSG_QUIT:
					bExit = TRUE;
					break;
				
				case MSG_CAMERA_MOTION:
#ifdef RECORDER
					wb702EnableMotionDetect ((g_ConfigParam.ucMotionDetectWay == MOTION_DETECT_SOFT ? TRUE : FALSE),
						CMD_MOTION_DETECT_MIDDLE);	
				
					if(g_ConfigParam.ucMotionDetectWay == MOTION_DETECT_SOFT)
					{
						recorder_memset();
						RecordInit = TRUE;
						diag_printf("Montion Dectect Enable!\n");
					}
					else
					{	
						RecordInit = FALSE;	
						recorder_dis();						
						diag_printf("Montion Dectect Disable!\n");
					}
#endif
					break;
				
				case MSG_CAMERA_DIRECTION:
					{
						int iResX, iResY;
						CMD_ROTATE_E eRotate = -1;
						W99702_GetImageResolution ( &iResX, &iResY);
						switch (msg.lData)
						{
							case 0x00:	eRotate = CMD_ROTATE_NORMAL;	break;
							case 0x01:	eRotate = CMD_ROTATE_FLIP;		break;
							case 0x02:	eRotate = CMD_ROTATE_MIRROR;	break;
							case 0x03:	eRotate = CMD_ROTATE_R180;		break;
							default:	eRotate = -1;
						}
						if (eRotate != -1)
						{
							wb702SetLocalVideoSource (iResX, iResY, eRotate);
							g_WebCamState.eRotate = eRotate;
							g_WebCamState.ucX_Direction = (unsigned char) ((msg.lData & 0x02) >> 1);
							g_WebCamState.ucY_Direction = (unsigned char) (msg.lData & 0x01);
						}
					}
					break;
				
				case MSG_CAMERA_RES:
					{
						BOOL bRt = FALSE;
						/*H163 not support 320*240 and 640*480, if set these resolution revert to 352*288*/
						switch (msg.lData)
						{
						case 0:
							bRt = W99702_SetImageResolution(176, 144,&g_dataW99702); break;
						case 1:
						{
							if (g_ConfigParam.eVideoFormat == CMD_VIDEO_H263)
							{
								msg.lData = 2;	//Try 352*288
								bRt = W99702_SetImageResolution(352, 288,&g_dataW99702);
							}
							else
								bRt = W99702_SetImageResolution(320, 240,&g_dataW99702);
							break;
						}
						case 2:
							bRt = W99702_SetImageResolution(352, 288,&g_dataW99702); break;
						case 3:
						{
							if (g_ConfigParam.eVideoFormat == CMD_VIDEO_H263)
							{
								msg.lData = 2;	//Try 352*288
								bRt = W99702_SetImageResolution(352, 288,&g_dataW99702);
							}
							else
								bRt = W99702_SetImageResolution(640, 480,&g_dataW99702);
							break;
						}
						default:
							;
						}

						if (bRt == TRUE)
						{
							g_WebCamState.ucResolution = msg.lData;	
						}							
					}
					break;
				
				case MSG_CAMERA_QUALITY:
					if (W99702_SetImageQuality( msg.lData,&g_dataW99702))
						g_WebCamState.ucCompressionRatio = msg.lData;
					break;
				
				case MSG_CAMERA_FRAMERATE:
					if (W99702_SetFramerate( msg.lData,&g_dataW99702))
						g_WebCamState.ucFramerate = msg.lData;
					break;
				
				case MSG_CAMERA_BRIGHTNESS:			
					if((W99702_SetImageBrightness((int)msg.lData, &g_dataW99702)) == TRUE)
						g_WebCamState.ucBright = msg.lData;
					diag_printf("Image brightness is %d\n",g_WebCamState.ucBright);
					break;
				
				case MSG_SPEAKER_VOLUME:
					if(( W99702_SetSpeakerVolume((int)msg.lData,&g_dataW99702))==TRUE)
						g_WebCamState.ucSpeakerVolume = msg.lData;
					diag_printf("Speaker volume is %d\n",g_WebCamState.ucSpeakerVolume);
					break;
				
				case MSG_MIC_VOLUME:
					if(( W99702_SetMicVolume((int)msg.lData,&g_dataW99702))==TRUE)
						g_WebCamState.ucMicVolume = msg.lData;
					diag_printf("mic volume is %d\n",g_WebCamState.ucMicVolume);
					break;
				case MSG_SET_LOGO:
					{
						int i;
						wb702EnableDrawImageTime(false);
						
						for(i = 0; i < 2; i++)
						{
	            			switch(g_ConfigParam.ShowPos[i])
    	        			{
        	       				case 0:
            	        			wb702SetDrawContent(i, g_ConfigParam.ShowString[i],0,0);
                	    			break;
								case 1:
									wb702SetDrawContent(i, g_ConfigParam.ShowString[i],-1,0);
									break;
								case 2:
									wb702SetDrawContent(i, g_ConfigParam.ShowString[i],0,-1);
									break;
								case 3:
									wb702SetDrawContent(i, g_ConfigParam.ShowString[i],-1,-1);
									break;
								default:
									break;
							}
							diag_printf("show string %s at position %d\n",g_ConfigParam.ShowString[i],g_ConfigParam.ShowPos[i]);
						}
           				break;
					}
				default:
					diag_printf("Unknown msg: %d\n", msg.lMsg);
				}
			}
		}
		
		//Audio_Exit();
		W99702_CloseCamera(&g_dataW99702);
		g_WebCamState.ucCamera = CAMERA_OFF;
#ifdef RECORDER
		Do_TestSendMail(&plPic2Mail, &tMail, TRUE);
#endif
	}

	wb702EnableSuspend (0);
	//if (iPreImage.pcBmpBuf != NULL) free(iPreImage.pcBmpBuf);
	if(AsBuf->AssignBuffer != NULL)
		free(AsBuf->AssignBuffer);
	if(AsBuf != NULL)
		free(AsBuf);

}

BOOL SendCameraMsg(long lMsg, long lData)
{
	BOOL bRt;
	MSG_T msg;
	msg.lMsg = lMsg;
	msg.lData = lData;

	diag_printf("After send1\n");
	bRt = SendMsg(g_pMsgCamera, (void *)&msg);
	diag_printf("After send2\n");
	iothread_EventInterrupt();
	diag_printf("After send3\n");
	
	return bRt;
}


static int Config_CameraControl(HTTPCONNECTION hConnection, LIST *pParamList, int iAction,
	char *pcCommand, long lMsgName, unsigned long *pulConfigParam, int iLogType)
{
	long lVal;
	char ac[64];
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
		break;
	case CA_CONFIG:
		if (strlen(httpGetString(pParamList, pcCommand)) != 0)
		{
			lVal = httpGetLong(pParamList, pcCommand);
			httpLong2String(lVal, ac);
			WebCameraLog(CL_PRIVILEGE_COMMON, iLogType, ac, hConnection);
			SendCameraMsg(lMsgName, lVal);
			if (lVal != *pulConfigParam)
			{
				g_ConfigParam.bDefaultImg = FALSE;
				*pulConfigParam = lVal;
				WriteFlashMemory(&g_ConfigParam);
			}
		}
		return 0;
		break;
	}
	return -1;
}

int Config_ChDirection(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
#if 0
	return Config_CameraControl(hConnection, pParamList, iAction,
		"Dir", MSG_CAMERA_DIRECTION,
		&g_ConfigParam.ulImgDirection, CL_CHANGE_DIRECTION);
#else
	return 0;
#endif
}
#ifdef CONFIGINTERFACE
int Config_SetCamera(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	long lVal;
	char ac[64];

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_USER;
		break;
	case CA_CONFIG:
		lVal = httpGetLong (pParamList, "Frequency");
		vcptSetFrequency(lVal);

		httpLong2String(lVal, ac);
		WebCameraLog(CL_PRIVILEGE_COMMON, CL_CHANGE_CAMERA, ac, hConnection);
		return 0;
		break;
	}
	
	return -1;
}


int Config_GetCamera(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	int nFrequencyHz;
	switch (iAction)
	{
	case CA_AUTH:	
		return AUTH_USER;
		break;
	case CA_CONFIG:
		nFrequencyHz = vcptGetFrequency();
		AddHttpNum(pReturnXML, "Frequency", nFrequencyHz);
		
		return 0;
		break;
	}
	
	return -1;
}



int Config_ChResolution(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	long lVal;
	char ac[64];
	request *req = (request*)hConnection;
	char *pcCommand = "ResType";
	switch (iAction)
	{
	case CA_AUTH:
		/* Change to user privilege according to KoiTech's request. */
		return AUTH_USER;
		break;
	case CA_CONFIG:
		if (strlen(httpGetString(pParamList, pcCommand)) != 0)
		{
			lVal = httpGetLong(pParamList, pcCommand);
			httpLong2String(lVal, ac);
			WebCameraLog(CL_PRIVILEGE_COMMON, CL_CHANGE_RESOLUTION, ac, hConnection);
			ictlSetResolution(((ICTL_HANDLE_T*)&req->httpPrivilege),lVal);
		}
		return 0;
		break;
	}
	return -1;	
}
int Config_ChFramerate(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	long lVal;
	char ac[64];
	request *req = (request*)hConnection;
	char *pcCommand = "Framerate";
	switch (iAction)
{
	case CA_AUTH:
		/* Change to user privilege according to KoiTech's request. */
		return AUTH_USER;
		break;
	case CA_CONFIG:
		if (strlen(httpGetString(pParamList, pcCommand)) != 0)
		{
			lVal = httpGetLong(pParamList, pcCommand);
			httpLong2String(lVal, ac);
			WebCameraLog(CL_PRIVILEGE_COMMON, CL_CHANGE_FRAMERATE, ac, hConnection);
			ictlSetFramerate(((ICTL_HANDLE_T*)&req->httpPrivilege),lVal);
		}
		return 0;
		break;
	}
	return -1;		
}

int Config_ChCompressRatio(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	long lVal;
	char ac[64];
	request *req = (request*)hConnection;
	char *pcCommand = "Ratio";
	switch (iAction)
	{
	case CA_AUTH:
		/* Change to user privilege according to KoiTech's request. */
		return AUTH_USER;
		break;
	case CA_CONFIG:
		if (strlen(httpGetString(pParamList, pcCommand)) != 0)
		{
			lVal = httpGetLong(pParamList, pcCommand);
			httpLong2String(lVal, ac);
			WebCameraLog(CL_PRIVILEGE_COMMON, CL_CHANGE_QUALITY, ac, hConnection);
			ictlSetQuality(((ICTL_HANDLE_T*)&req->httpPrivilege),lVal);
		}
		return 0;
		break;
	}
	return -1;		
}

int Config_ChBrightness(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	long lVal;
	char ac[64];
	request *req = (request*)hConnection;
	char *pcCommand = "Brightness";
	switch (iAction)
	{
	case CA_AUTH:
		/* Change to user privilege according to KoiTech's request. */	
		return AUTH_USER;
		break;
	case CA_CONFIG:
		if (strlen(httpGetString(pParamList, pcCommand)) != 0)
		{
			lVal = httpGetLong(pParamList, pcCommand);
			httpLong2String(lVal, ac);
			WebCameraLog(CL_PRIVILEGE_COMMON, CL_CHANGE_BRIGHTNESS, ac, hConnection);
			ictlSetBrightness(((ICTL_HANDLE_T*)&req->httpPrivilege),lVal);
		}
		return 0;
		break;
	}
	return -1;		
}

int Config_ChSpeakerVolume(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	long lVal;
	char ac[64];
	request *req = (request*)hConnection;
	char *pcCommand = "SpeakerVolume";
	switch (iAction)
	{
	case CA_AUTH:
		/* Change to user privilege according to KoiTech's request. */	
		return AUTH_USER;
		break;
	case CA_CONFIG:
		if (strlen(httpGetString(pParamList, pcCommand)) != 0)
		{
			lVal = httpGetLong(pParamList, pcCommand);
			httpLong2String(lVal, ac);
			WebCameraLog(CL_PRIVILEGE_COMMON, CL_CHANGE_SPEAKERVOLUME, ac, hConnection);
			ictlSetSpeakerVolume(((ICTL_HANDLE_T*)&req->httpPrivilege),lVal);
		}
		return 0;
		break;
	}
	return -1;		
}

int Config_ChMicVolume(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	long lVal;
	char ac[64];
	request *req = (request*)hConnection;
	char *pcCommand = "MicVolume";
	switch (iAction)
	{
	case CA_AUTH:
		/* Change to user privilege according to KoiTech's request. */	
		return AUTH_USER;
		break;
	case CA_CONFIG:
		if (strlen(httpGetString(pParamList, pcCommand)) != 0)
		{
			lVal = httpGetLong(pParamList, pcCommand);
			httpLong2String(lVal, ac);
			WebCameraLog(CL_PRIVILEGE_COMMON, CL_CHANGE_MICVOLUME, ac, hConnection);
			ictlSetMicVolume(((ICTL_HANDLE_T*)&req->httpPrivilege),lVal);
		}
		return 0;
		break;
	}
	return -1;		
}

int Config_SetMediaFormat(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	long lAudioFormat;
	long lVideoFormat;
	request *req = (request*)hConnection;
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
		break;
	case CA_CONFIG:
		if (httpIsExistParam (pParamList, G_PC_AUDIO))
			lAudioFormat = httpGetLong (pParamList, G_PC_AUDIO);
		else
			lAudioFormat = (long) g_ConfigParam.eAudioFormat;

		if (httpIsExistParam (pParamList, G_PC_VIDEO))
			lVideoFormat = httpGetLong (pParamList, G_PC_VIDEO);
		else
			lVideoFormat = (long) g_ConfigParam.eVideoFormat;

		if ((CMD_AUDIO_FORMAT_E) lAudioFormat != g_ConfigParam.eAudioFormat
			|| (CMD_VIDEO_FORMAT_E) lVideoFormat != g_ConfigParam.eVideoFormat)
		{
			ictlSetMediaFormat((ICTL_HANDLE_T*)&req->httpPrivilege,lAudioFormat,lVideoFormat);
			
			RebootOnConnectionOver(hConnection);
		}
		return 0;
		break;
	}
	return -1;
}

int Config_GetMediaFormat(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	char ac[128];
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_USER;
		break;
	case CA_CONFIG:
		tt_rmutex_lock(&g_rmutex);
		httpLong2String ((long) g_ConfigParam.eAudioFormat, ac);
		AddHttpValue (pReturnXML, G_PC_AUDIO, ac);
		httpLong2String ((long) g_ConfigParam.eVideoFormat, ac);
		AddHttpValue (pReturnXML, G_PC_VIDEO, ac);
		tt_rmutex_unlock(&g_rmutex);
		return 0;
		break;
	}
	return -1;
}

#else
int Config_ChResolution(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	return Config_CameraControl(hConnection, pParamList, iAction,
		"ResType", MSG_CAMERA_RES,
		&g_ConfigParam.ulImgResolution, CL_CHANGE_RESOLUTION);
}


int Config_ChFramerate(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	return Config_CameraControl(hConnection, pParamList, iAction,
		"Framerate", MSG_CAMERA_FRAMERATE,
		&g_ConfigParam.ulImgFramerate, CL_CHANGE_FRAMERATE);
}

int Config_ChCompressRatio(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	return Config_CameraControl(hConnection, pParamList, iAction,
		"Ratio", MSG_CAMERA_QUALITY,
		&g_ConfigParam.ulImgQuality, CL_CHANGE_QUALITY);
}

int Config_ChBrightness(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	return Config_CameraControl(hConnection, pParamList, iAction,
		"Brightness", MSG_CAMERA_BRIGHTNESS,
		&g_ConfigParam.ulImgBrightness, CL_CHANGE_BRIGHTNESS);
}

int Config_SetMediaFormat(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	long lAudioFormat;
	long lVideoFormat;

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
		break;
	case CA_CONFIG:
		if (httpIsExistParam (pParamList, G_PC_AUDIO))
			lAudioFormat = httpGetLong (pParamList, G_PC_AUDIO);
		else
			lAudioFormat = (long) g_ConfigParam.eAudioFormat;
		
		if (httpIsExistParam (pParamList, G_PC_VIDEO))
			lVideoFormat = httpGetLong (pParamList, G_PC_VIDEO);
		else
			lVideoFormat = (long) g_ConfigParam.eVideoFormat;

		if ((CMD_AUDIO_FORMAT_E) lAudioFormat != g_ConfigParam.eAudioFormat
			|| (CMD_VIDEO_FORMAT_E) lVideoFormat != g_ConfigParam.eVideoFormat)
		{
			g_ConfigParam.eAudioFormat = (CMD_AUDIO_FORMAT_E) lAudioFormat;
			g_ConfigParam.eVideoFormat = (CMD_VIDEO_FORMAT_E) lVideoFormat;

			diag_printf ("Audio: %d Video: %d\n", lAudioFormat, lVideoFormat);
			WriteFlashMemory(&g_ConfigParam);
			RebootOnConnectionOver(hConnection);
		}
		return 0;
		break;
	}
	return -1;
}

int Config_GetMediaFormat(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	char ac[128];

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_USER;
		break;
	case CA_CONFIG:
		httpLong2String ((long) g_ConfigParam.eAudioFormat, ac);
		AddHttpValue (pReturnXML, G_PC_AUDIO, ac);
		httpLong2String ((long) g_ConfigParam.eVideoFormat, ac);
		AddHttpValue (pReturnXML, G_PC_VIDEO, ac);
		return 0;
		break;
	}
	return -1;
}

#endif
int Config_SetMotionDetect(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	BOOL bMotionDetect;
	char ac[32];
	unsigned char ucMotionDetectWayBefore;
	int iSensitivity;
	int iSensitivityBefore;

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
		break;
	case CA_CONFIG:
		iSensitivityBefore = g_ConfigParam.ulMotionDetectSensitivity;
		if (httpIsExistParam(pParamList, "Sensitivity"))
		{
			const char *pc = httpGetString(pParamList, "Sensitivity");
			if (strcasecmp(pc, "high") == 0)
				iSensitivity = 0;
			else if (strcasecmp(pc, "middle") == 0)
				iSensitivity = 1;
			else if (strcasecmp(pc, "low") == 0)
				iSensitivity = 2;
			else iSensitivity = httpString2Long(pc);

			if (iSensitivity >= 0 && iSensitivity <= 2)
				g_ConfigParam.ulMotionDetectSensitivity = iSensitivity;
		}

		bMotionDetect = httpGetBool(pParamList, "Enable");
		ucMotionDetectWayBefore = g_ConfigParam.ucMotionDetectWay;
		if (bMotionDetect)
			g_ConfigParam.ucMotionDetectWay = (g_pExternalDetector?MOTION_DETECT_HARD:MOTION_DETECT_SOFT);
		else
			g_ConfigParam.ucMotionDetectWay = MOTION_DETECT_NONE;

		sprintf(ac, "%d", g_ConfigParam.ucMotionDetectWay);
		WebCameraLog(CL_PRIVILEGE_COMMON, CL_SET_MOTION_DETECT, ac, hConnection);

		if (ucMotionDetectWayBefore != g_ConfigParam.ucMotionDetectWay
			|| iSensitivityBefore != g_ConfigParam.ulMotionDetectSensitivity)
		{
			SendCameraMsg(MSG_CAMERA_MOTION, 0);
			WriteFlashMemory(&g_ConfigParam);
		}
		return 0;
		break;
	}
	return -1;
}

int Config_GetMotionDetect(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	char ac[128];

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
		break;
	case CA_CONFIG:
		httpLong2String((g_ConfigParam.ucMotionDetectWay == MOTION_DETECT_NONE ? 0 : 1), ac);
		AddHttpValue(pReturnXML, G_PC_ENABLE, ac);
		httpLong2String((int)g_ConfigParam.ulMotionDetectSensitivity, ac);
		AddHttpValue(pReturnXML, G_PC_SENSITIVITY, ac);
		return 0;
		break;
	}
	return -1;
}


#ifdef USE_SERVER_PUSH

typedef union
{
	struct Fillld
	{
		unsigned int uiIndex:20;
		unsigned int uiIfSendStatus:1;
		unsigned int uiFillTime:3;
	}Filld;
	void *pParam;
} FILLDATA_T;


static int FillCameraData(HTTPCONNECTION hConnection, time_t *ptLastFill, void *p)
{
#define HD_ADD_NONE 0
#define HD_ADD_STATE 1
#define HD_ADD_CRLF 2
	time_t tCur;
	FILLDATA_T fData;
	int iNextAdd = HD_ADD_NONE;
	BOOL bIsAddJpeg = FALSE;
	char acBuf[128];


	fData.pParam = p;
	tCur = mytime();

	if (httpIsDisconnect(hConnection))
	{
		httpSetSendDataOverFun(hConnection, NULL, NULL);
		return 0;
	}

	cyg_mutex_lock(&g_ptmState);
	if (g_WebCamState.uiPicIndex != (fData.Filld).uiIndex)
	{
		if (g_WebCamState.iReadyImg >=0 && g_WebCamState.iReadyImg <= 1)
		{
			httpAddBodyString(hConnection, "--WINBONDBOUDARY\r\nContent-Type: image/jpeg\r\n\r\n");
			httpAddBody(hConnection,
				g_WebCamState.acImgBuf[g_WebCamState.iReadyImg],
				g_WebCamState.iImgBufLen[g_WebCamState.iReadyImg]);
			if ((fData.Filld).uiIfSendStatus)
				iNextAdd = HD_ADD_STATE;
			bIsAddJpeg = TRUE;
			*ptLastFill = tCur;
		}
		(fData.Filld).uiIndex = g_WebCamState.uiPicIndex;
	}

	if (bIsAddJpeg == FALSE)
	{
		vcptJpegTimerNotify();
		if (tCur - *ptLastFill >= 1)
		{
			if ((fData.Filld).uiIfSendStatus)
				iNextAdd = HD_ADD_STATE;
			else iNextAdd = HD_ADD_CRLF;
			*ptLastFill = tCur;
		}
	}

	switch (iNextAdd)
	{
	case HD_ADD_STATE:
		SetRunTimeState(hConnection);
		GetWebCamStateString(&g_WebCamState, acBuf);
		sprintf(acBuf + strlen(acBuf), "\r\n");
		httpAddBodyString(hConnection, "--WINBONDBOUDARY\r\nContent-Type: text/plain\r\n\r\n");
		httpAddBodyString(hConnection, acBuf);
		break;
	case HD_ADD_CRLF:
		httpAddBodyString(hConnection, "\r\n");
		break;
	default:
		break;
	}

	httpSetSendDataOverFun(hConnection, FillCameraData, fData.pParam);
	cyg_mutex_unlock(&g_ptmState);
//diag_printf("Return from FILLDATA\n");	
	return 1;
}

int Http_GetCameraData(HTTPCONNECTION hConnection, void *pParam)
{
	FILLDATA_T fData;
	LIST *pList;
	
	memset(&fData, 0, sizeof(fData));
//diag_printf("Fill data begin\n");
	pList = httpReadQueryList(hConnection);
	(fData.Filld).uiIfSendStatus = (httpGetBool(pList, "Status")?1:0);
	httpDeleteQueryList(pList);
	(fData.Filld).uiIndex = INVALID_PICTURE;

	vcptJpegTimerNotify();
	vcptJpegTimerWait();
	
	httpSetSendDataOverFun(hConnection, FillCameraData, fData.pParam);

	httpSetKeepAliveMode(hConnection, FALSE);
	httpSetHeader(hConnection, 200, "OK", "",
				g_cNoCacheHeader,
				"multipart/x-mixed-replace;boundary=WINBONDBOUDARY", FALSE);
//diag_printf("Fill data end\n");
	return 0;

}



int Config_DropData(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	const char *pcPath;
	const char *pcQueryString;
	int iFound;

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_USER;
	case CA_CONFIG:
		pcQueryString = httpGetString(pParamList, "Query");
		if (httpIsExistParam(pParamList, "Path"))
			pcPath = httpGetString(pParamList, "Path");
		else
			pcPath = "/GetData.cgi";
		diag_printf("Drop [%s][%s]\n", pcPath, pcQueryString);

		iFound = httpDisconnect(pcPath, pcQueryString);
		AddHttpNum(pReturnXML, "Found", iFound);

		return 0;
	}
	return -1;

}


#endif

int Http_SendJpeg(HTTPCONNECTION hConnection, void *pParam)
{
	BOOL bSuccess;
	if (httpGetMethod(hConnection) != M_GET) return -1;

	vcptJpegTimerNotify();
	vcptJpegTimerWait();


	bSuccess = FALSE;
	cyg_mutex_lock(&g_ptmState);
	//diag_printf("the sen size===%d",g_WebCamState.iImgBufLen[g_WebCamState.iReadyImg]);
	if (g_WebCamState.iReadyImg >=0 && g_WebCamState.iReadyImg <= 1)
	{
		httpAddBody(hConnection,
			g_WebCamState.acImgBuf[g_WebCamState.iReadyImg],
			g_WebCamState.iImgBufLen[g_WebCamState.iReadyImg]);
		httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "image/jpeg", TRUE);
		bSuccess = TRUE;
	}
	cyg_mutex_unlock(&g_ptmState);
	if (bSuccess) return 0;

	SetExtraHeader(hConnection,(char*) g_cNoCacheHeader);
	return -1;
}

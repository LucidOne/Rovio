//#ifdef RECORDER
#include "../../Inc/CommonDef.h"
#include "../../libMail/Inc/libmail.h"

static char m_base64tab[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
#define BASE64_MAXLINE  76
#define EOL  "\r\n"

int Base64BufferSize(int iInputSize)
{
	int iOutSize = (iInputSize + 2) / 3 * 4;                    // 3:4 conversion ratio
	iOutSize += strlen(EOL) * iOutSize / BASE64_MAXLINE + 3;  // Space for newlines and NUL
	return iOutSize;
}

BOOL EncodeBase64(const char *pccInStr, int iInLen, char *pccOutStr, int iMaxOutSize, int *piOutLen)
{
	int i;
	int iInPos;
	int iOutPos;
	int iLineLen;
	int c1,c2,c3;
	char *pc;

	if (pccInStr == NULL || pccOutStr == NULL
		|| iMaxOutSize <= 0 || iInLen < 0
		|| iMaxOutSize < Base64BufferSize(iInLen))
	{
		fprintf(stderr, "Error call EncodeBase64()%d %d %d %d %d\n",
			pccInStr,
			pccOutStr,
			iMaxOutSize,
			iInLen,
			Base64BufferSize(iInLen));
		return FALSE;
	}

	//Set up the parameters prior to the main encoding loop
	iInPos = 0;
	iOutPos = 0;
	iLineLen = 0;

	// Get three characters at a time from the input buffer and encode them
	for (i=0; i<iInLen/3; ++i)
	{
		//Get the next 2 characters
		c1 = pccInStr[iInPos++] & 0xFF;
		c2 = pccInStr[iInPos++] & 0xFF;
		c3 = pccInStr[iInPos++] & 0xFF;

		//Encode into the 4 6 bit characters
		pccOutStr[iOutPos++] = m_base64tab[(c1 & 0xFC) >> 2];
		pccOutStr[iOutPos++] = m_base64tab[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
		pccOutStr[iOutPos++] = m_base64tab[((c2 & 0x0F) << 2) | ((c3 & 0xC0) >> 6)];
		pccOutStr[iOutPos++] = m_base64tab[c3 & 0x3F];
		iLineLen += 4;

		//Handle the case where we have gone over the max line boundary
		if (iLineLen >= BASE64_MAXLINE-3)
		{
			char* pc = EOL;
			pccOutStr[iOutPos++] = *pc++;
			if (*pc)
				pccOutStr[iOutPos++] = *pc;
			iLineLen = 0;
		}
	}

	// Encode the remaining one or two characters in the input buffer
	switch (iInLen % 3)
	{
    case 0:
	{
		pc = EOL;
		pccOutStr[iOutPos++] = *pc++;
		if (*pc)
			pccOutStr[iOutPos++] = *pc;
		break;
	}
    case 1:
	{
		int c1 = pccInStr[iInPos] & 0xFF;
		pccOutStr[iOutPos++] = m_base64tab[(c1 & 0xFC) >> 2];
		pccOutStr[iOutPos++] = m_base64tab[((c1 & 0x03) << 4)];
		pccOutStr[iOutPos++] = '=';
		pccOutStr[iOutPos++] = '=';
		pc = EOL;
		pccOutStr[iOutPos++] = *pc++;
		if (*pc)
			pccOutStr[iOutPos++] = *pc;
		break;
	}
    case 2:
	{
		int c1 = pccInStr[iInPos++] & 0xFF;
		int c2 = pccInStr[iInPos] & 0xFF;
		pccOutStr[iOutPos++] = m_base64tab[(c1 & 0xFC) >> 2];
		pccOutStr[iOutPos++] = m_base64tab[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
		pccOutStr[iOutPos++] = m_base64tab[((c2 & 0x0F) << 2)];
		pccOutStr[iOutPos++] = '=';
		pc = EOL;
		pccOutStr[iOutPos++] = *pc++;
		if (*pc)
			pccOutStr[iOutPos++] = *pc;
		break;
	}
    default:
		fprintf(stderr, "Unknown Error in FILE %s, LINE %d\n", __FILE__, __LINE__);
		return FALSE;
		break;
	}
	pccOutStr[iOutPos] = 0;
	*piOutLen = iOutPos;
	return TRUE;
}
#if 0
int Config_SendMail(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	const char *pcSender;
	const char *pcReceiver;
    const char *pcReceiver_Cc;
    const char *pcReceiver_Bcc;
	const char *pcSubject;
	const char *pcContent;
	const char *pcServer;
	const char *pcAttachment;
	const char *pcUser;
	const char *pcPass;	
	MAIL_MEM* mail_mem = NULL;
	MAIL_MEM* file_mem = NULL;
	LIST *plAttachment;

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_SYSTEM;
		break;
	case CA_CONFIG:
		pcSender = httpGetString(pParamList, "Sender");
		pcReceiver = httpGetString(pParamList, "Receiver");
        pcReceiver_Cc = httpGetString(pParamList, "Receiver_Cc");
        pcReceiver_Bcc = httpGetString(pParamList, "Receiver_Bcc");
		pcSubject = httpGetString(pParamList, "Subject");
		pcContent = httpGetString(pParamList, "Content");
		pcServer = httpGetString(pParamList, "MailServer");
		if (httpIsExistParam(pParamList, "AttachFile"))
			pcAttachment = httpGetString(pParamList, "AttachFile");
		else
			pcAttachment = NULL;
		pcUser = httpGetString(pParamList, "User");
		if (strcmp(httpGetString(pParamList, "CheckFlag"), "CHECK") != 0)
			pcPass = NULL;
		else
			pcPass = httpGetString(pParamList, "PassWord");

		plAttachment = (LIST*)CreateFileList();
		if (pcAttachment != NULL)
		{
			//if (cyg_mutex_lock(&g_ptmState) == 0)
			//{
				cyg_mutex_lock(&g_ptmState);
				if(get_mail_mem(&file_mem) == FALSE)
				{
					diag_printf("Not enough mail memory!\n");
				}	
				else
				{
					AddBufferFileList(plAttachment,
						g_WebCamState.acImgBuf[g_WebCamState.iReadyImg],
						g_WebCamState.iImgBufLen[g_WebCamState.iReadyImg],
						"CamImg",
						file_mem);
				}
				cyg_mutex_unlock(&g_ptmState);
			//}
		}
		if(get_mail_mem(&mail_mem) == FALSE)
		{
			diag_printf("Not enough mail memory!\n");
		}	
		else
		{
			sendMailMsg(
				pcSender,
				pcReceiver,
           		pcReceiver_Cc,
           		pcReceiver_Bcc,
				pcSubject,
				pcContent,
				0,
				plAttachment,
				pcServer,
				pcUser,
				pcPass,
				MAIL_CLIENT_REQUEST,
				mail_mem);
		}
		DeleteFileList(plAttachment);
		return 0;
		break;
	}
	return -1;
}
#else
extern Mail_Info* g_mailinfo;
int Config_SendMail(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
		break;
	case CA_CONFIG:
		vcptJpegTimerNotify();
		vcptJpegTimerWait();
		
		cyg_mutex_lock(&g_ptmState);
			
		if (g_WebCamState.iReadyImg >=0 && g_WebCamState.iReadyImg <= 1)
			DO_TestSendMailFile (g_WebCamState.acImgBuf[g_WebCamState.iReadyImg], g_WebCamState.iImgBufLen[g_WebCamState.iReadyImg],g_mailinfo);
		cyg_mutex_unlock(&g_ptmState);
		return 0;
		break;
	}

	return -1;
}
#endif
int Config_SetMail(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
		break;
	case CA_CONFIG:
		cyg_mutex_lock(&g_ptmConfigParam);
		if (httpIsExistParam(pParamList, "Sender"))
			httpMyStrncpy(g_ConfigParam.acMailSender, httpGetString(pParamList, "Sender"), sizeof(g_ConfigParam.acMailSender));
		if (httpIsExistParam(pParamList, "Receiver"))
			httpMyStrncpy(g_ConfigParam.acMailReceiver, httpGetString(pParamList, "Receiver"), sizeof(g_ConfigParam.acMailReceiver));
		if (httpIsExistParam(pParamList, "MailServer"))
			httpMyStrncpy(g_ConfigParam.acMailServer, httpGetString(pParamList, "MailServer"), sizeof(g_ConfigParam.acMailServer));
		if (httpIsExistParam(pParamList, "Port"))
		{
			g_ConfigParam.usMailPort = httpGetLong(pParamList, "Port");
			if (g_ConfigParam.usMailPort == 0)
				g_ConfigParam.usMailPort = 25;
		}
		if (httpIsExistParam(pParamList, "User"))
			httpMyStrncpy(g_ConfigParam.acMailUser, httpGetString(pParamList, "User"), sizeof(g_ConfigParam.acMailUser));
		if (httpIsExistParam(pParamList, "PassWord"))
			httpMyStrncpy(g_ConfigParam.acMailPassword, httpGetString(pParamList, "PassWord"), sizeof(g_ConfigParam.acMailPassword));
		if (httpIsExistParam(pParamList, "CheckFlag"))
			g_ConfigParam.bMailCheck = strcmp(httpGetString(pParamList, "CheckFlag"), "CHECK")==0;
		else g_ConfigParam.bMailCheck = FALSE;
		if (httpIsExistParam(pParamList, "Subject"))
			httpMyStrncpy(g_ConfigParam.acMailSubject, httpGetString(pParamList, "Subject"), sizeof(g_ConfigParam.acMailSubject));
		if (httpIsExistParam(pParamList, "Body"))
			httpMyStrncpy(g_ConfigParam.acMailBody, httpGetString(pParamList, "Body"), sizeof(g_ConfigParam.acMailBody));
		if (httpIsExistParam(pParamList, "Enable"))
			g_ConfigParam.bMailEnable = httpGetBool(pParamList, "Enable");
		else g_ConfigParam.bMailEnable = FALSE;
		
		g_WebCamState.ucEmail = (g_ConfigParam.bMailEnable?'\1':'\0');

		cyg_mutex_unlock(&g_ptmConfigParam);

		WebCameraLog(CL_PRIVILEGE_ADMIN, CL_SET_MAIL, g_ConfigParam.acMailReceiver, hConnection);

		WriteFlashMemory(&g_ConfigParam);
		return 0;
		break;
	}
	return -1;
}

int Config_GetMail(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	char ac[384];

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
		break;
	case CA_CONFIG:
		AddHttpValue(pReturnXML, G_PC_MAILSERVER, g_ConfigParam.acMailServer);
		AddHttpNum(pReturnXML, "Port", (g_ConfigParam.usMailPort == 0 ? 25 : g_ConfigParam.usMailPort));
		AddHttpValue(pReturnXML, G_PC_SENDER, g_ConfigParam.acMailSender);
		AddHttpValue(pReturnXML, G_PC_RECEIVER, g_ConfigParam.acMailReceiver);
		AddHttpValue(pReturnXML, G_PC_SUBJECT, g_ConfigParam.acMailSubject);
		AddHttpValue(pReturnXML, "Body", g_ConfigParam.acMailBody);
		AddHttpValue(pReturnXML, G_PC_USER, g_ConfigParam.acMailUser);
		AddHttpValue(pReturnXML, "PassWord", g_ConfigParam.acMailPassword);
		httpLong2String((int)g_ConfigParam.bMailCheck, ac);
		AddHttpValue(pReturnXML, G_PC_CHECKFLAG, ac);
		httpLong2String((int)g_ConfigParam.bMailEnable, ac);
		AddHttpValue(pReturnXML, G_PC_ENABLE, ac);
		return 0;
		break;
	}
	return -1;
}
//#endif









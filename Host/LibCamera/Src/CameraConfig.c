#include "CommonDef.h"

#define DIRECT_FLASH
#undef KERNEL_WRITE_FLASH

#ifdef KERNEL_WRITE_FLASH
#define OFFSET_FLASH_CONFIG (1024*2)
#endif


void GetFlashCapability(UINT32 *puTotalSize, UINT32 *puFreeBegin, UINT32 *puFreeEnd)
{
	
	
	if (puTotalSize != NULL)
		*puTotalSize = FLASH_TOTAL_SIZE;

	if (puFreeBegin != NULL)
	{
		UINT32 uTotalLen;
		usiMyRead(4,4,(UINT8*)&uTotalLen);
		*puFreeBegin = ( uTotalLen + 255 ) / 256 * 256;
	}
	
	if (puFreeEnd != NULL)
		*puFreeEnd = FLASH_TOTAL_SIZE - FLASH_CONFIG_SIZE * 2;
}


extern char g_Param4Flash[FLASH_CONFIG_SIZE];

BOOL ReadFlashMemory(CAMERA_CONFIG_PARAM_T *pcConfigParam)
{
#ifdef FILESYSTEM	
	int iFileLen = 0;
	INT	 	 hFile = -1;
	INT      nStatus;
	hFile = fsOpenFile((char *)FLASH_CONFIG_FILE,NULL, O_RDONLY);
	if(hFile < 0)
	{
		diag_printf("(test1)Open file %s error %x \n",FLASH_CONFIG_FILE, hFile);
		return FALSE;
	}
	
	nStatus = fsReadFile(hFile, (UINT8 *)pcConfigParam, sizeof(CAMERA_CONFIG_PARAM_T),&iFileLen);	
	fsCloseFile(hFile);
	if ((nStatus < 0)||(iFileLen != sizeof(CAMERA_CONFIG_PARAM_T)))
	{
		diag_printf("read len is %d,size of config param is %d\n",iFileLen,sizeof(CAMERA_CONFIG_PARAM_T));
		fprintf(stderr, "Camera config file: length error!\n");
		return FALSE;
	}
	
#else
	UINT32 uConfigAddr;

	if (!USI_IS_READY)
	{
		diag_printf("No USI Flash\n");
		return FALSE;
	}

	uConfigAddr = FLASH_TOTAL_SIZE - FLASH_CONFIG_SIZE * 2;

	if (usiMyRead(uConfigAddr ,sizeof(CAMERA_CONFIG_PARAM_T),(UINT8 *)pcConfigParam) != USI_NO_ERR)
	{
		diag_printf("Read config parameters error\n");
		return FALSE;
	}
	
	if (GetConfigCheckSum(pcConfigParam) != pcConfigParam->ulCheckSum)
	{
		uConfigAddr = FLASH_TOTAL_SIZE - FLASH_CONFIG_SIZE;
		if (usiMyRead(uConfigAddr ,sizeof(CAMERA_CONFIG_PARAM_T),(UINT8 *)pcConfigParam) != USI_NO_ERR)
		{
			diag_printf("Read default config parameters error\n");
			return FALSE;
		}
	}
#endif

	if (GetConfigCheckSum(pcConfigParam) != pcConfigParam->ulCheckSum)
	{
		fprintf(stderr, "Camera config file: checksum error!\n");
		diag_printf("%d != %d",GetConfigCheckSum(pcConfigParam) , pcConfigParam->ulCheckSum);
		return FALSE;
	}

	return TRUE;
}

BOOL __WriteFlashMemory(CAMERA_CONFIG_PARAM_T *pcConfigParam, BOOL bResetCheckSum, BOOL bOnDefaultArea)
{
	static BOOL s_bNotResetCheckSum = FALSE;
	UINT32 uConfigAddr;
	
#ifdef FILESYSTEM		
  	INT	 	 hFile = -1;
	INT      nWriteLen, nStatus;
#endif	

	if (bResetCheckSum)
	{
		if (s_bNotResetCheckSum)
			return FALSE;
		pcConfigParam->ulCheckSum = GetConfigCheckSum(pcConfigParam);
	}
	else
		s_bNotResetCheckSum = TRUE;
		
#ifdef FILESYSTEM	
	hFile = fsOpenFile((char*)FLASH_CONFIG_FILE,NULL, O_WRONLY |O_CREAT | O_TRUNC);
	if(hFile < 0)
	{
		diag_printf("(test1)Open file %s error %x \n",FLASH_CONFIG_FILE, hFile);
		return FALSE;
	}
	
	nStatus = fsWriteFile(hFile,(void *)pcConfigParam,  sizeof(CAMERA_CONFIG_PARAM_T), &nWriteLen);
	if((nStatus < 0)||(nWriteLen != sizeof(CAMERA_CONFIG_PARAM_T)))
		diag_printf("save configuration information error\n");
	fsCloseFile(hFile);
#else
	if (!USI_IS_READY)
	{
		diag_printf("No USI Flash\n");
		return FALSE;
	}
	
	cyg_mutex_lock(&g_ptmConfigParam);

	uConfigAddr = bOnDefaultArea ? FLASH_TOTAL_SIZE - FLASH_CONFIG_SIZE : FLASH_TOTAL_SIZE - FLASH_CONFIG_SIZE * 2;
	
	if(usiMyWrite(uConfigAddr, sizeof(CAMERA_CONFIG_PARAM_T),(UINT8 *)pcConfigParam) != USI_NO_ERR
		|| usiMyFlash() != USI_NO_ERR)
	{
		diag_printf("save configuration information error\n");
		cyg_mutex_unlock(&g_ptmConfigParam);
		return FALSE;
	}
	else
	{
		cyg_mutex_unlock(&g_ptmConfigParam);
		diag_printf("write size ======%d,checksum ===%d\n",sizeof(CAMERA_CONFIG_PARAM_T),pcConfigParam->ulCheckSum);
	}
#endif
	return TRUE;
}

BOOL WriteFlashMemory(CAMERA_CONFIG_PARAM_T *pcConfigParam)
{
	return __WriteFlashMemory(pcConfigParam, TRUE, FALSE);
}

#ifdef FILESYSTEM
BOOL ReadCameraINI(char *pcFileName, CAMERA_CONFIG_PARAM_T *pcConfigParam)
{
	int i;
	char acName[24];
	char acValue[128];
	LIST *pList;
	LISTNODE *pNode;

#define __ReadDefault(pcSeg, pcName, acVal) \
	do \
	{ \
		if (!httpReadConfigValue(pcFileName, pcSeg, pcName, acVal, sizeof(acVal))) \
		{ \
			diag_printf( "Can not get config item \"[%s]%s\"\n", pcSeg, pcName ); \
			return FALSE; \
		} \
	} while (0)
	

	memset(pcConfigParam, 0, sizeof(CAMERA_CONFIG_PARAM_T));

	/* 读 Http Server 参数 */
	for (i = 0; i < sizeof(g_ConfigParam.usHttpPort) / sizeof(unsigned short); i++)
		pcConfigParam->usHttpPort[i] = 0;

	/* 读 WLAN 参数 */
	__ReadDefault("WLAN", "ESSID", acValue);
	httpMyStrncpy((char*)pcConfigParam->acWlanESSID, acValue, sizeof(pcConfigParam->acWlanESSID));
	__ReadDefault("WLAN", "Channel", acValue);
	pcConfigParam->ulWlanChannel = httpString2Long(acValue);
	__ReadDefault("WLAN", "Mode", acValue);
	if (acValue[0] == '\0')	pcConfigParam->ucWlanOperationMode = '\2';
	else pcConfigParam->ucWlanOperationMode = (unsigned char)httpString2Long(acValue);

	/* 读IP Assigned 参数 */
	__ReadDefault("IP", "CameraName", acValue);
	httpMyStrncpy(pcConfigParam->acCameraName, acValue, sizeof(pcConfigParam->acCameraName));
	for (i = 0; i < 3; i++)
	{
		sprintf(acName, "DNS%d", i);
		__ReadDefault("IP", acName, acValue);
		pcConfigParam->aulAsDNS[i] = inet_addr(acValue);
	}

	for (i = 0; i < sizeof(g_apcNetworkInterface) / sizeof(const char *); i++)
	{
		sprintf(acName, "IP_%s", g_apcNetworkInterface[i]);
		__ReadDefault(acName, "Enable", acValue);
		pcConfigParam->abAsNetEnable[i] = httpString2Bool(acValue);
		__ReadDefault(acName, "IPAssignment", acValue);
		if (strcasecmp(acValue, "manually") == 0)
			pcConfigParam->aucIPAssignedWay[i] = IP_ASSIGNMENT_MANUALLY;
		else pcConfigParam->aucIPAssignedWay[i] = IP_ASSIGNMENT_DHCP;
		__ReadDefault(acName, "IP", acValue);
		pcConfigParam->ulAsIPAddress[i] = inet_addr(acValue);
		__ReadDefault(acName, "Netmask", acValue);
		pcConfigParam->ulAsNetmask[i] = inet_addr(acValue);
		__ReadDefault(acName, "Gateway", acValue);
		pcConfigParam->ulAsGateway[i] = inet_addr(acValue);
	}

	/* 读 PPPoE 参数 */

	pcConfigParam->bEnablePPPoE = TRUE;
	__ReadDefault("PPPoE", "PPPoEConnectOnBoot", acValue);
	pcConfigParam->bPPPoEConnectOnBoot = httpString2Bool(acValue);
	__ReadDefault("PPPoE", "PPPoEUserName", acValue);
	httpMyStrncpy(pcConfigParam->acPPPoEUserName, acValue, sizeof(pcConfigParam->acPPPoEUserName));
	__ReadDefault("PPPoE", "PPPoEUserPass", acValue);
	httpMyStrncpy(pcConfigParam->acPPPoEUserPass, acValue, sizeof(pcConfigParam->acPPPoEUserPass));
	__ReadDefault("PPPoE", "PPPoESeviceName", acValue);
	httpMyStrncpy(pcConfigParam->acPPPoESeviceName, acValue, sizeof(pcConfigParam->acPPPoESeviceName));
	__ReadDefault("PPPoE", "PPPoEDisConnectTime_InSec", acValue);
	pcConfigParam->ulPPPoEDisConnectTime_InSec = httpString2Long(acValue);

	/* 读 Modem 参数 */
	pcConfigParam->bEnableModem = TRUE;
	__ReadDefault("Modem", "EnableModemAutoConnect", acValue);
	pcConfigParam->bEnableModemAutoConnect = httpString2Bool(acValue);
	__ReadDefault("Modem", "ModemUserName", acValue);
	httpMyStrncpy(pcConfigParam->acModemUserName, acValue, sizeof(pcConfigParam->acModemUserName));
	__ReadDefault("Modem", "ModemUserPass", acValue);
	httpMyStrncpy(pcConfigParam->acModemUserPass, acValue, sizeof(pcConfigParam->acModemUserPass));
	__ReadDefault("Modem", "ModemPhone", acValue);
	httpMyStrncpy(pcConfigParam->acModemPhone, acValue, sizeof(pcConfigParam->acModemPhone));
	__ReadDefault("Modem", "ModemDisConnectTime_InSec", acValue);
	pcConfigParam->ulModemDisConnectTime_InSec = httpString2Long(acValue);

	/* 读PPPoE和Modem的共同参数 */
	__ReadDefault("Dial", "MailOnDial", acValue);
	pcConfigParam->bIfMailOnDialed = httpString2Bool(acValue);
	__ReadDefault("Dial", "DialMailSender", acValue);
	httpMyStrncpy(pcConfigParam->acDial_MailSender, acValue, sizeof(pcConfigParam->acDial_MailSender));
	__ReadDefault("Dial", "DialMailReceiver", acValue);
	httpMyStrncpy(pcConfigParam->acDial_MailReceiver, acValue, sizeof(pcConfigParam->acDial_MailReceiver));
	__ReadDefault("Dial", "DialMailServer", acValue);
	httpMyStrncpy(pcConfigParam->acDial_MailServer, acValue, sizeof(pcConfigParam->acDial_MailServer));
	__ReadDefault("Dial", "DialMailPassword", acValue);
	httpMyStrncpy(pcConfigParam->acDial_MailPassword, acValue, sizeof(pcConfigParam->acDial_MailPassword));
	__ReadDefault("Dial", "DialMailUser", acValue);
	httpMyStrncpy(pcConfigParam->acDial_MailUser, acValue, sizeof(pcConfigParam->acDial_MailUser));
	__ReadDefault("Dial", "DialMailCheckPass", acValue);
	pcConfigParam->bDial_MailCheck = httpString2Bool(acValue);
	__ReadDefault("Dial", "DialMailSubject", acValue);
	httpMyStrncpy(pcConfigParam->acDial_MailSubject, acValue, sizeof(pcConfigParam->acDial_MailSubject));

	/* 读ftp参数 */
	__ReadDefault("Ftp", "FtpServer", acValue);
	httpMyStrncpy(pcConfigParam->acFtpServer, acValue, sizeof(pcConfigParam->acFtpServer));
	__ReadDefault("Ftp", "FtpUser", acValue);
	httpMyStrncpy(pcConfigParam->acFtpUser, acValue, sizeof(pcConfigParam->acFtpUser));
	__ReadDefault("Ftp", "FtpPass", acValue);
	httpMyStrncpy(pcConfigParam->acFtpPass, acValue, sizeof(pcConfigParam->acFtpPass));
	__ReadDefault("Ftp", "FtpAccount", acValue);
	httpMyStrncpy(pcConfigParam->acFtpAccount, acValue, sizeof(pcConfigParam->acFtpAccount));
	__ReadDefault("Ftp", "FtpUploadPath", acValue);
	httpMyStrncpy(pcConfigParam->acFtpUploadPath, acValue, sizeof(pcConfigParam->acFtpUploadPath));
	__ReadDefault("Ftp", "FtpEnable", acValue);
	pcConfigParam->bFtpEnable = httpString2Bool(acValue);

	/* 读 Mail 参数 */
	__ReadDefault("Mail", "MailSender", acValue);
	httpMyStrncpy(pcConfigParam->acMailSender, acValue, sizeof(pcConfigParam->acMailSender));
	__ReadDefault("Mail", "MailReceiver", acValue);
	httpMyStrncpy(pcConfigParam->acMailReceiver, acValue, sizeof(pcConfigParam->acMailReceiver));
	__ReadDefault("Mail", "MailServer", acValue);
	httpMyStrncpy(pcConfigParam->acMailServer, acValue, sizeof(pcConfigParam->acMailServer));
	__ReadDefault("Mail", "MailUser", acValue);
	httpMyStrncpy(pcConfigParam->acMailUser, acValue, sizeof(pcConfigParam->acMailUser));
	__ReadDefault("Mail", "MailPassword", acValue);
	httpMyStrncpy(pcConfigParam->acMailPassword, acValue, sizeof(pcConfigParam->acMailPassword));
	__ReadDefault("Mail", "MailCheckPass", acValue);
	pcConfigParam->bMailCheck = httpString2Bool(acValue);
	__ReadDefault("Mail", "MailSubject", acValue);
	httpMyStrncpy(pcConfigParam->acMailSubject, acValue, sizeof(pcConfigParam->acMailSubject));
	__ReadDefault("Mail", "MailEnable", acValue);
	pcConfigParam->bMailEnable = httpString2Bool(acValue);

	/* 读用户管理参数 */
	__ReadDefault("User", "UserCheck", acValue);
	pcConfigParam->bUserCheck = httpString2Bool(acValue);
	__ReadDefault("User", "AdminName", acValue);
	httpMyStrncpy(pcConfigParam->acAdminName, acValue, sizeof(pcConfigParam->acAdminName));
	__ReadDefault("User", "AdminPass", acValue);
	httpMyStrncpy(pcConfigParam->acAdminPass, acValue, sizeof(pcConfigParam->acAdminPass));

	httpMyStrncpy(pcConfigParam->aacUserName[0], pcConfigParam->acAdminName, sizeof(pcConfigParam->aacUserName[0]));
	httpMyStrncpy(pcConfigParam->aacUserPass[0], pcConfigParam->acAdminPass, sizeof(pcConfigParam->aacUserPass[0]));
	pcConfigParam->acUserPrivilege[0] = AUTH_ADMIN;

	for (i=1; i<MAX_USERSET_ENABLE; i++)
	{
		sprintf(acName, "UserName_%d", i);
		__ReadDefault("User", acName, acValue);
		if (acValue[0] == '\0') break;
		httpMyStrncpy(pcConfigParam->aacUserName[i], acValue, sizeof(pcConfigParam->aacUserName[i]));
		sprintf(acName, "UserPass_%d", i);
		__ReadDefault("User", acName, acValue);
		if (acValue[0] == '\0') break;
		httpMyStrncpy(pcConfigParam->aacUserPass[i], acValue, sizeof(pcConfigParam->aacUserPass[i]));
		sprintf(acName, "UserPrivilege_%d", i);
		__ReadDefault("User", acName, acValue);
		if (acValue[0] == '\0') break;
		pcConfigParam->acUserPrivilege[i] = (char)httpString2Long(acValue);
		i++;
	}

	pcConfigParam->iUserCurNum = i;

	/* 读相机设置 */
	__ReadDefault("Camera", "DefaultImg", acValue);
	pcConfigParam->bDefaultImg = httpString2Bool(acValue);
	__ReadDefault("Camera", "ImgResolution", acValue);
	pcConfigParam->ulImgResolution = httpString2Long(acValue);
	__ReadDefault("Camera", "ImgQuality", acValue);
	pcConfigParam->ulImgQuality = httpString2Long(acValue);
	__ReadDefault("Camera", "ImgFramerate", acValue);
	pcConfigParam->ulImgFramerate = httpString2Long(acValue);
	__ReadDefault("Camera", "ImgBrightness", acValue);
	pcConfigParam->ulImgBrightness = httpString2Long(acValue);
	__ReadDefault("Camera", "ImgContrast", acValue);
	pcConfigParam->ulImgContrast = httpString2Long(acValue);
	__ReadDefault("Camera", "ImgHue", acValue);
	pcConfigParam->ulImgHue = httpString2Long(acValue);
	__ReadDefault("Camera", "ImgSaturation", acValue);
	pcConfigParam->ulImgSaturation = httpString2Long(acValue);
	__ReadDefault("Camera", "ImgSharpness", acValue);
	pcConfigParam->ulImgSharpness = httpString2Long(acValue);
	__ReadDefault("Camera", "VideoFormat", acValue);
	pcConfigParam->eVideoFormat = httpString2Long(acValue);
	__ReadDefault("Camera", "AudioFormat", acValue);

	/* 读 Monite 参数 */
	__ReadDefault("Camera", "MoniteRect", acValue);
	pList = httpSplitString(acValue, ',');
	i = 0;
	if (pList != NULL)
	{
		for (pNode = pList->pFirstNode, i = 0; pNode != pList->pLastNode && i < 4; pNode = pNode->pNextNode, i++)
			*((int *)&pcConfigParam->rMoniteRect + i) = (unsigned long)httpString2Long((char*)pNode->pValue);
		httpDeleteSplitString(pList);
	}
	if (i<4)
	{
		memset(&pcConfigParam->rMoniteRect, 0, sizeof(pcConfigParam->rMoniteRect));
	}
	__ReadDefault("Camera", "MotionDetectSensitivity", acValue);
	pcConfigParam->ulMotionDetectSensitivity = httpString2Long(acValue);
	__ReadDefault("Camera", "MotionDetect", acValue);
	pcConfigParam->ucMotionDetectWay = (unsigned char)httpString2Long(acValue);

	/* 读 Other 参数 */
	__ReadDefault("Other", "TimeZone", acValue);
	pcConfigParam->lTimeZone_InSec = httpString2Long(acValue) * 60;
	__ReadDefault("Other", "NtpServer", acValue);
	httpMyStrncpy(pcConfigParam->acNtpServer, acValue, sizeof(pcConfigParam->acNtpServer));
	__ReadDefault("Other", "UseNtp", acValue);
	pcConfigParam->bUseNtp = httpString2Bool(acValue);

	/* Set CheckSum */
	pcConfigParam->ulCheckSum = GetConfigCheckSum(pcConfigParam);
	return TRUE;
}
BOOL WriteCameraINI(char *pcFileName, CAMERA_CONFIG_PARAM_T *pcConfigParam)
{
	INT hFile;
	int i, j;
	char *filebuffer;
	char ac[64];	
	INT writelen,nstatus;
	size_t szBuffer = 1024;
	
	filebuffer = (char*)malloc(szBuffer);
	if(filebuffer == NULL)
		return FALSE;
	hFile = fsOpenFile(pcFileName,NULL, O_WRONLY |O_CREAT | O_TRUNC);
	if(hFile < 0)
	{
		free(filebuffer);
		diag_printf("(test1)Open file %s error %x \n",pcFileName, hFile);
		return FALSE;
	}
	/* WLAN 参数 */
	j = 0;
	j += snprintf(filebuffer+j, szBuffer - j,"[WLAN]\r\n");
	j += snprintf(filebuffer+j,szBuffer - j, "ESSID = %s\r\n", pcConfigParam->acWlanESSID);
	j += snprintf(filebuffer+j,szBuffer - j, "Channel = %d\r\n", (int)pcConfigParam->ulWlanChannel);
	j += snprintf(filebuffer+j,szBuffer - j, "Mode = %d\r\n", (int)pcConfigParam->ucWlanOperationMode);
	j += snprintf(filebuffer+j,szBuffer - j, "\r\n");

	/* IP Assigned 参数 */
	j += snprintf(filebuffer+j, szBuffer - j,"[IP]\r\n");
	j += snprintf(filebuffer+j, szBuffer - j,"CameraName = %s\r\n", pcConfigParam->acCameraName);
	for (i = 0; i < 3; i++)
	{
		httpIP2String(pcConfigParam->aulAsDNS[i], ac);
		j += snprintf(filebuffer+j,szBuffer - j, "DNS%d = %s\r\n", i, ac);
	}
	j += snprintf(filebuffer+j,szBuffer - j, "\r\n");
	for (i = 0; i < sizeof(g_apcNetworkInterface) / sizeof(const char *); i++)
	{
		j += snprintf(filebuffer+j, szBuffer - j,"[IP_%s]\r\n", g_apcNetworkInterface[i]);
		j += snprintf(filebuffer+j, szBuffer - j,"Enable = %d\r\n", pcConfigParam->abAsNetEnable[i]);
		j += snprintf(filebuffer+j,szBuffer - j, "IPAssignment = %s\r\n",
			(pcConfigParam->aucIPAssignedWay[i]==IP_ASSIGNMENT_MANUALLY?"manually":"dhcp"));
		httpIP2String(pcConfigParam->ulAsIPAddress[i], ac);
		j += snprintf(filebuffer+j,szBuffer - j, "IP = %s\r\n", ac);
		httpIP2String(pcConfigParam->ulAsNetmask[i], ac);
		j += snprintf(filebuffer+j,szBuffer - j, "Netmask = %s\r\n", ac);
		httpIP2String(pcConfigParam->ulAsGateway[i], ac);
		j += snprintf(filebuffer+j, szBuffer - j,"Gateway = %s\r\n", ac);
		j += snprintf(filebuffer+j,szBuffer - j, "\r\n");
	}

	/* PPPoE.htm 参数 */
	j += snprintf(filebuffer+j, szBuffer - j, "[PPPoE]\r\n");
	
	
	j += snprintf(filebuffer+j,szBuffer - j, "PPPoEConnectOnBoot = %d\r\n", (int)pcConfigParam->bPPPoEConnectOnBoot);
	j += snprintf(filebuffer+j,szBuffer - j, "PPPoEUserName = %s\r\n", pcConfigParam->acPPPoEUserName);
	j += snprintf(filebuffer+j,szBuffer - j, "PPPoEUserPass = %s\r\n", pcConfigParam->acPPPoEUserPass);
	j += snprintf(filebuffer+j,szBuffer - j, "PPPoESeviceName = %s\r\n", pcConfigParam->acPPPoESeviceName);
	j += snprintf(filebuffer+j,szBuffer - j, "PPPoEDisConnectTime_InSec = %u\r\n", (unsigned int)pcConfigParam->ulPPPoEDisConnectTime_InSec);
	j += snprintf(filebuffer+j,szBuffer - j, "\r\n");

	/* Modem.htm 参数 */
	j += snprintf(filebuffer+j,szBuffer - j, "[Modem]\r\n");
	j += snprintf(filebuffer+j,szBuffer - j, "EnableModemAutoConnect = %d\r\n", (int)pcConfigParam->bEnableModemAutoConnect);
	j += snprintf(filebuffer+j,szBuffer - j, "ModemUserName = %s\r\n", pcConfigParam->acModemUserName);
	j += snprintf(filebuffer+j,szBuffer - j, "ModemUserPass = %s\r\n", pcConfigParam->acModemUserPass);
	j += snprintf(filebuffer+j,szBuffer - j, "ModemPhone = %s\r\n", pcConfigParam->acModemPhone);
	j += snprintf(filebuffer+j,szBuffer - j, "ModemDisConnectTime_InSec = %u\r\n", (unsigned int)pcConfigParam->ulModemDisConnectTime_InSec);
	j += snprintf(filebuffer+j,szBuffer - j, "\r\n");

	/* 写PPPoE和Modem的共同参数 */
	j += snprintf(filebuffer+j,szBuffer - j, "[Dial]\n");
	j += snprintf(filebuffer+j,szBuffer - j, "MailOnDial = %d\r\n", (int)pcConfigParam->bIfMailOnDialed);
	j += snprintf(filebuffer+j,szBuffer - j, "DialMailSender = %s\r\n", pcConfigParam->acDial_MailSender);
	j += snprintf(filebuffer+j,szBuffer - j, "DialMailReceiver = %s\r\n", pcConfigParam->acDial_MailReceiver);
	j += snprintf(filebuffer+j,szBuffer - j, "DialMailServer = %s\r\n", pcConfigParam->acDial_MailServer);
	j += snprintf(filebuffer+j,szBuffer - j, "DialMailUser = %s\r\n", pcConfigParam->acDial_MailUser);
	j += snprintf(filebuffer+j,szBuffer - j, "DialMailPassword = %s\r\n", pcConfigParam->acDial_MailPassword);
	j += snprintf(filebuffer+j,szBuffer - j, "DialMailCheckPass = %d\r\n", (int)pcConfigParam->bDial_MailCheck);
	j += snprintf(filebuffer+j,szBuffer - j, "DialMailSubject = %s\r\n", pcConfigParam->acDial_MailSubject);
	j += snprintf(filebuffer+j,szBuffer - j, "\r\n");

	/* ftp参数 */
	j += snprintf(filebuffer+j,szBuffer - j, "[Ftp]\n");
	j += snprintf(filebuffer+j,szBuffer - j, "FtpServer = %s\r\n", pcConfigParam->acFtpServer);
	j += snprintf(filebuffer+j,szBuffer - j, "FtpUser = %s\r\n", pcConfigParam->acFtpUser);
	j += snprintf(filebuffer+j,szBuffer - j, "FtpPass = %s\r\n", pcConfigParam->acFtpPass);
	j += snprintf(filebuffer+j,szBuffer - j, "FtpAccount = %s\r\n", pcConfigParam->acFtpAccount);
	j += snprintf(filebuffer+j,szBuffer - j, "FtpUploadPath = %s\r\n", pcConfigParam->acFtpUploadPath);
	j += snprintf(filebuffer+j,szBuffer - j, "FtpEnable = %d\r\n", (int)pcConfigParam->bFtpEnable);
	j += snprintf(filebuffer+j,szBuffer - j, "\r\n");

	/* Mail 参数 */
	j += snprintf(filebuffer+j,szBuffer - j, "[Mail]\r\n");
	j += snprintf(filebuffer+j,szBuffer - j, "MailSender = %s\r\n", pcConfigParam->acMailSender);
	j += snprintf(filebuffer+j,szBuffer - j, "MailReceiver = %s\r\n", pcConfigParam->acMailReceiver);
	j += snprintf(filebuffer+j,szBuffer - j, "MailServer = %s\r\n", pcConfigParam->acMailServer);
	j += snprintf(filebuffer+j,szBuffer - j, "MailUser = %s\r\n", pcConfigParam->acMailUser);
	j += snprintf(filebuffer+j,szBuffer - j, "MailPassword = %s\r\n", pcConfigParam->acMailPassword);
	j += snprintf(filebuffer+j,szBuffer - j, "MailCheckPass = %d\r\n", (int)pcConfigParam->bMailCheck);
	j += snprintf(filebuffer+j,szBuffer - j, "MailSubject = %s\r\n", pcConfigParam->acMailSubject);
	j += snprintf(filebuffer+j,szBuffer - j, "MailEnable = %d\r\n", (int)pcConfigParam->bMailEnable);
	j += snprintf(filebuffer+j,szBuffer - j, "\r\n");

	/* 用户管理参数 */
	j += snprintf(filebuffer+j,szBuffer - j, "[User]\r\n");
	j += snprintf(filebuffer+j,szBuffer - j, "UserCheck = %d\r\n", (int)pcConfigParam->bUserCheck);
	j += snprintf(filebuffer+j,szBuffer - j, "AdminName = %s\r\n", pcConfigParam->acAdminName);
	j += snprintf(filebuffer+j,szBuffer - j, "AdminPass = %s\r\n", pcConfigParam->acAdminPass);
	for (i=1; i<pcConfigParam->iUserCurNum && i<MAX_USERSET_ENABLE; i++)
	{
		j += snprintf(filebuffer+j,szBuffer - j, "UserName_%d = %s\r\n", i, pcConfigParam->aacUserName[i]);
		j += snprintf(filebuffer+j,szBuffer - j, "UserPass_%d = %s\r\n", i, pcConfigParam->aacUserPass[i]);
		j += snprintf(filebuffer+j,szBuffer - j, "UserPrivilege_%d = %d\r\n", i, (int)pcConfigParam->acUserPrivilege[i]);
	}
	j += snprintf(filebuffer+j,szBuffer - j, "\r\n");

	/* 相机设置 */
	j += snprintf(filebuffer+j,szBuffer - j, "[Camera]\r\n");
	j += snprintf(filebuffer+j,szBuffer - j, "DefaultImg = %d\r\n", (int)pcConfigParam->bDefaultImg);
	j += snprintf(filebuffer+j,szBuffer - j, "ImgResolution = %d\r\n", (int)pcConfigParam->ulImgResolution);
	j += snprintf(filebuffer+j,szBuffer - j, "ImgQuality = %d\r\n", (int)pcConfigParam->ulImgQuality);
	j += snprintf(filebuffer+j,szBuffer - j, "ImgFramerate = %d\r\n", (int)pcConfigParam->ulImgFramerate);
	j += snprintf(filebuffer+j,szBuffer - j, "ImgBrightness = %d\r\n", (int)pcConfigParam->ulImgBrightness);
	j += snprintf(filebuffer+j,szBuffer - j, "ImgContrast = %d\r\n", (int)pcConfigParam->ulImgContrast);
	j += snprintf(filebuffer+j,szBuffer - j, "ImgHue = %d\r\n", (int)pcConfigParam->ulImgHue);
	j += snprintf(filebuffer+j,szBuffer - j, "ImgSaturation = %d\r\n", (int)pcConfigParam->ulImgSaturation);
	j += snprintf(filebuffer+j,szBuffer - j, "ImgSharpness = %d\r\n", (int)pcConfigParam->ulImgSharpness);
	/* Monite 参数 */
	j += snprintf(filebuffer+j,szBuffer - j, "MoniteRect = %d, %d, %d, %d\r\n",
		pcConfigParam->rMoniteRect.iLeft,
		pcConfigParam->rMoniteRect.iTop,
		pcConfigParam->rMoniteRect.iRight,
		pcConfigParam->rMoniteRect.iBottom);
	j += snprintf(filebuffer+j, szBuffer - j,"MotionDetectSensitivity = %d\r\n", (int)pcConfigParam->ulMotionDetectSensitivity);
	j += snprintf(filebuffer+j,szBuffer - j, "MotionDetect = %d\r\n", (int)pcConfigParam->ucMotionDetectWay);
	j += snprintf(filebuffer+j,szBuffer - j, "AudioFormat = %d\r\n", (int)pcConfigParam->eAudioFormat);
	j += snprintf(filebuffer+j,szBuffer - j, "VideoFormat = %d\r\n", (int)pcConfigParam->eVideoFormat);
	j += snprintf(filebuffer+j,szBuffer - j, "\r\n");

	/* Other 参数 */
	j += snprintf(filebuffer+j,szBuffer - j, "[Other]\r\n");
	j += snprintf(filebuffer+j,szBuffer - j, "TimeZone = %d\r\n", (int)(pcConfigParam->lTimeZone_InSec / 60));
	j += snprintf(filebuffer+j,szBuffer - j, "NtpServer = %s\r\n", pcConfigParam->acNtpServer);
	j += snprintf(filebuffer+j,szBuffer - j, "UseNtp = %d\r\n", (int)pcConfigParam->bUseNtp);
	//diag_printf("%s\n",filebuffer);

	if (j >= szBuffer)
	{
		fprintf (stderr, "memory overflow..\n");
		free(filebuffer);
		
		fsCloseFile(hFile);
		
		return FALSE;
	}		
	
	nstatus=fsWriteFile(hFile,(UINT8*)filebuffer,j,&writelen);
	
	if((nstatus <0)||(writelen != j))
	{
		diag_printf("write default file error \n");
		free(filebuffer);
		
		fsCloseFile(hFile);
		return FALSE;
	}
	free(filebuffer);

	fsCloseFile(hFile);
	return TRUE;
}
#endif

#if 0
BOOL ReadFactoryDefault(CAMERA_CONFIG_PARAM_T *pcConfigParam)
{
#ifdef FILESYSTEM
	return ReadCameraINI((void *)FACTORY_DEFAULT_FILE, pcConfigParam);
#else
	if (!USI_IS_READY)
	{
		diag_printf("No USI Flash\n");
		return FALSE;
	}
PTI;	
	cyg_mutex_lock(&g_ptmConfigParam);
PTI;
	if (usiMyRead(FACTORY_DEFAULT_FILE,sizeof(CAMERA_CONFIG_PARAM_T),(UINT8 *)pcConfigParam) != USI_NO_ERR)
	{
		diag_printf("Read default parameters error\n");
		cyg_mutex_unlock(&g_ptmConfigParam);
		return FALSE;
	}
	PTI;
	cyg_mutex_unlock(&g_ptmConfigParam);

	if (GetConfigCheckSum(pcConfigParam) != pcConfigParam->ulCheckSum)
	{
		fprintf(stderr, "Camera default config file: checksum error!\n");
		diag_printf("%d != %d",GetConfigCheckSum(pcConfigParam) , pcConfigParam->ulCheckSum);
		return FALSE;
	}

	return TRUE;
#endif
}
BOOL WriteFactoryDefault(CAMERA_CONFIG_PARAM_T *pcConfigParam)
{

	if (!USI_IS_READY)
	{
		diag_printf("No USI Flash\n");
		return FALSE;
	}

	pcConfigParam->ulCheckSum = GetConfigCheckSum(pcConfigParam);
	if(usiMyWrite(FACTORY_DEFAULT_FILE, sizeof(CAMERA_CONFIG_PARAM_T),(UINT8 *)pcConfigParam) != USI_NO_ERR
		|| usiMyFlash() != USI_NO_ERR)
	{
		diag_printf("save configuration information error\n");
		return FALSE;
	}
	else
	{
		diag_printf("write size ======%d,checksum ===%d\n",sizeof(CAMERA_CONFIG_PARAM_T),pcConfigParam->ulCheckSum);
	}

	return TRUE;

}
#endif

unsigned long GetConfigCheckSum(CAMERA_CONFIG_PARAM_T *pcConfigParam)
{
	int i;
	unsigned long ulCheckSum;
	unsigned char *puc;

	ulCheckSum=0;
	puc = (unsigned char *)pcConfigParam;
	for (i=0; i<offsetof(CAMERA_CONFIG_PARAM_T, ulCheckSum); i++)
		ulCheckSum += (unsigned long)puc[i] * i + i;
	for (i=offsetof(CAMERA_CONFIG_PARAM_T, ulCheckSum)+sizeof(pcConfigParam->ulCheckSum); i<sizeof(CAMERA_CONFIG_PARAM_T); i++)
		ulCheckSum += (unsigned long)puc[i] * i + i;
	
	/* Add the sizeof(CAMERA_CONFIG_PARAM_T) so that modification in CAMERA_CONFIG_PARAM_T will reset the checksum. */
	ulCheckSum += sizeof(CAMERA_CONFIG_PARAM_T);
	
	return ulCheckSum;
}

int Http_SetFactoryDefault(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
		break;

	case CA_CONFIG:
#if 0	
#ifdef FILESYSTEM
		g_ConfigParam.ulCheckSum = GetConfigCheckSum(&g_ConfigParam) + 1;
		__WriteFlashMemory(&g_ConfigParam, FALSE, FALSE);
		httpAddBodyString(hConnection, "OK");
		httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/plain", TRUE);
		RebootOnConnectionOver(hConnection);
#else
		if (ReadFactoryDefault(&g_ConfigParam))
		{
			__WriteFlashMemory(&g_ConfigParam, FALSE, FALSE);
			AddHttpValue(pReturnXML, "Result", "1");
		}
		else
		{
			AddHttpValue(pReturnXML, "Result", "0");	
		}
#endif
#else

		/* Call libNS's restore function */
#if defined IPCAM_CONFIG_IP_CAM_VER_1 || defined IPCAM_CONFIG_IP_CAM_VER_2 || defined IPCAM_CONFIG_IP_CAM_VER_3
		ERSP_rovio_libns_restore_defaults();
#elif defined IPCAM_CONFIG_IP_CAM_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_1
#else
#	error "No hardware config defined!"
#endif	

		//InitDefaultParam(&g_ConfigParam);			
		//WriteFlashMemory(&g_ConfigParam);
		g_ConfigParam.ulCheckSum = GetConfigCheckSum(&g_ConfigParam) + 1;
		__WriteFlashMemory(&g_ConfigParam, FALSE, FALSE);		
#endif
		RebootOnConnectionOver(hConnection);
		return 0;		
	}
	return -1;
}


int Http_UpdateFactoryDefault(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
		break;

	case CA_CONFIG:
		__WriteFlashMemory(&g_ConfigParam, TRUE, TRUE);		
		return 0;		
	}
	return -1;
}



#ifdef FILESYSTEM
int Http_GetConfig(HTTPCONNECTION hConnection, void *pParam)
{
	char *pc;

	WriteCameraINI((char*)DEBUG_CONFIG_FILE, &g_ConfigParam);
	if (httpReadWholeFile((char*)DEBUG_CONFIG_FILE, &pc, NULL))
	{
		httpAddBodyString(hConnection, pc);
		free(pc);
	}
	unlink((char*)DEBUG_CONFIG_FILE);
	httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/plain", TRUE);

	return 0;
}
#endif

void InitDefaultParam(CAMERA_CONFIG_PARAM_T *pcConfigParam)
{
	memset(pcConfigParam, 0, sizeof(CAMERA_CONFIG_PARAM_T));
#ifndef WLAN
	/*IP_eth1*/
	g_ConfigParam.abAsNetEnable[0] = TRUE;
	g_ConfigParam.ulAsIPAddress[0] = inet_addr("10.132.11.13");
	g_ConfigParam.ulAsNetmask[0] = inet_addr("255.255.0.0");
	g_ConfigParam.ulAsGateway[0] = inet_addr("10.132.1.254");
	//strcpy((char*)g_ConfigParam.aucIPAssignedWay,"dhcp");
	
	/*IP_wlan0*/
	g_ConfigParam.abAsNetEnable[1] = FALSE;
#else
	/*test for wireless*/
	g_ConfigParam.abAsNetEnable[0] = FALSE;
	g_ConfigParam.abAsNetEnable[1] = TRUE;
	
	g_ConfigParam.abUseHardwareMac[0] = TRUE;
	g_ConfigParam.abUseHardwareMac[1] = TRUE;
	{
		char acMAC[] = CONFIG_DEFAULT_MAC;
		memcpy(g_ConfigParam.acMAC[1], acMAC, sizeof(acMAC));
	}

	strcpy((char*)g_ConfigParam.acWlanESSID, CONFIG_DEFAULT_ESSID);
  	g_ConfigParam.ulAsIPAddress[1] = inet_addr(CONFIG_DEFAULT_IP);
	g_ConfigParam.ulAsNetmask[1] = inet_addr(CONFIG_DEFAULT_NETMASK);
	g_ConfigParam.ulAsGateway[1] = inet_addr(CONFIG_DEFAULT_GATEWAY);
	g_ConfigParam.ucWepSet = WEP_SET_DISABLE;	//WEP_SET_K128;
	g_ConfigParam.ulWepAuthentication =IW_ENCODE_OPEN;// IW_ENCODE_RESTRICTED;
	g_ConfigParam.aucIPAssignedWay[1] = CONFIG_DEFAULT_IP_METHOD;
	
	g_ConfigParam.ulWlanChannel = 5;
#if defined CONFIG_DEFAULT_WIFI_AD_HOC && CONFIG_DEFAULT_WIFI_AD_HOC == 1
	g_ConfigParam.ucWlanOperationMode = IW_MODE_ADHOC;
#else
	g_ConfigParam.ucWlanOperationMode = IW_MODE_INFRA;
#endif
	{
		const char acHex[5] =
		{
			0x41, 0x42, 0x43, 0x44, 0x45
		};
		memcpy((char *)g_ConfigParam.acWep64, acHex, sizeof(acHex) );
	}	
	
	{
		const char acHex[13] =
		{
			0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
			0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C,0x4D
		};
		memcpy((char *)g_ConfigParam.acWep128, acHex, sizeof(acHex) );
	}
#endif
	
	/*Dial*/
	strcpy(g_ConfigParam.acDial_MailSubject, "IP Camera PPP");
	
	/*Dial*/
#if defined IPCAM_CONFIG_IP_CAM_VER_1 || defined IPCAM_CONFIG_IP_CAM_VER_2 || defined IPCAM_CONFIG_IP_CAM_VER_3
	strcpy(g_ConfigParam.acMailSubject, "Rovio Snapshot");
	strcpy(g_ConfigParam.acMailBody, "Check out this photo from my Rovio.");
#elif defined IPCAM_CONFIG_IP_CAM_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_1
	strcpy(g_ConfigParam.acMailSubject, "IP Camera Warning!");
#else
#	error "No hardware config defined!"
#endif
	
	/*User*/
	g_ConfigParam.iUserCurNum = 1;
	g_ConfigParam.bUserCheck = FALSE;
	strcpy(g_ConfigParam.acAdminName, "admin");
	strcpy(g_ConfigParam.acAdminPass, "admin");
	
	strcpy(g_ConfigParam.aacUserName[0], "admin");
	strcpy(g_ConfigParam.aacUserPass[0], "admin");
	 g_ConfigParam.acUserPrivilege[0] = AUTH_ADMIN;
	
	/*Camera*/
	g_ConfigParam.bDefaultImg=TRUE;
	g_ConfigParam.ulImgResolution=2;
	g_ConfigParam.ulImgQuality=1;
	g_ConfigParam.ulImgFramerate=30;
	g_ConfigParam.ulImgBrightness=6;
	g_ConfigParam.ulImgContrast=0;
	g_ConfigParam.ulImgHue=0;
	g_ConfigParam.ulImgSaturation=191;
	g_ConfigParam.ulImgSharpness=0;
	g_ConfigParam.ulSpeakerVolume = 15;
	g_ConfigParam.ulMicVolume = 15;
	
	memset(&(g_ConfigParam.rMoniteRect),0,sizeof(RECT_T));	
	g_ConfigParam.ulMotionDetectSensitivity=0;
	g_ConfigParam.eVideoFormat = CMD_VIDEO_MP4;
	//g_ConfigParam.eAudioFormat = CMD_AUDIO_AMR;
	g_ConfigParam.eAudioFormat = CMD_AUDIO_ALAW;
	
	/*Other*/
	g_ConfigParam.lTimeZone_InSec = 0;
	g_ConfigParam.bUseNtp = FALSE;
	g_ConfigParam.bMailEnable = FALSE;
	g_ConfigParam.bFtpEnable = FALSE;
	g_ConfigParam.bEnableDDNS = FALSE;
	//g_ConfigParam.ShowTime = 1;
	
	/*Bypass server */
	g_ConfigParam.bEnableBPS = FALSE;
	g_ConfigParam.iBPSPort = 8888;
	g_ConfigParam.iBPSChannelsNum	= 1;
	
	/*UPnP*/
	g_ConfigParam.bEnableUPnP = TRUE;
	g_ConfigParam.usUPnPPort = DEFAULT_UPNP_FIRST_EXT_PORT;

#if 0
	strcpy(g_ConfigParam.acMailSender ,"nzhu@winbond.com");
	strcpy(g_ConfigParam.acMailReceiver ,"nzhu@winbond.com");
	strcpy(g_ConfigParam.acMailServer,"10.130.10.38");
	strcpy(g_ConfigParam.acMailUser,"nzhu");
	strcpy(g_ConfigParam.acMailPassword,"7582597");	
	strcpy(g_ConfigParam.acMailSubject,"Camera Image!");
	g_ConfigParam.bMailCheck = TRUE;
	g_ConfigParam.bMailEnable = TRUE;
#endif
	
}

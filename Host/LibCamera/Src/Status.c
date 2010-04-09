#include "CommonDef.h"


void GetWiFiStrength()
{
	int fd;
	struct iwreq wrq;
//	int pdata[2];
	
	

	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return;

	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, "wlan0");
	
#if 0
	wrq.u.data.flags = WLAN_GET_RXINFO;	
	wrq.u.data.pointer = (char *)pdata;
	
	if(ioctl(fd, WLAN_SET_GET_SIXTEEN_INT, &wrq) >=0 )
	{
		g_WebCamState.WiFiStrength = (unsigned char )pdata[1];
	}
#else
	*(int *)wrq.u.name = WLANRSSI;
	*(int *)(wrq.u.name + 4) = 1;
	if(ioctl(fd, WLAN_SETINT_GETINT, &wrq) >=0 )
	{
		g_WebCamState.WiFiStrength = (unsigned char )*(int *)wrq.u.name;
	}
#endif
	diag_printf("WiFi strenth: %x, %x\n", (int)g_WebCamState.WiFiStrength, *(int *)wrq.u.name);
	close(fd);	
	return;
}

int GetWebCamStateString(WEBCAMSTATE_P *pState, char *pcString)
{
	int iCount = 0;

#if defined IPCAM_CONFIG_IP_CAM_VER_1 || defined IPCAM_CONFIG_IP_CAM_VER_2 || defined IPCAM_CONFIG_IP_CAM_VER_3
	unsigned char ucBattery = mcuGetBattery(TRUE, NULL);
#elif defined IPCAM_CONFIG_IP_CAM_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_1
	unsigned char ucBattery = 0xFF;
#else
#	error "No hardware config defined!"
#endif


	pState->BatteryLevel = ucBattery;

	//diag_printf("battery level**********%d\n",pState->BatteryLevel);
	
	GetWiFiStrength();
	
// printf("status brightness =====%d,volume =====%d\n",pState->ucBright,pState->ucVolume);
	iCount += snprintf(pcString+iCount,sizeof(WEBCAMSTATE_P),
		"%02u"	/* 0-1, camera status. */
		"%02u"	/* 2-3, modem status. */
		"%02u"	/* 4-5, pppoe status. */
		"%03u"	/* 6-8, x-direction, reserved */
		"%03u"	/* 9-11, y-direction, reserved */
		"%03u"	/* 12-14, focus, reserved */
		"%03u"	/* 15-17, brightness. */
		"%03u"	/* 18-20, constrast. */
		"%1u"	/* 21, resolution. 9 indicates an unstandard resolution.*/
		"%1u"	/* 22, compression ratio. */
		"%1u"	/* 23, privilege. */
		"%06u"	/* 24-29, picture index. */
		"%1u"	/* 30, email state. */
		"%1u"	/* 31, user check. */
		"%08u"	/* 32-39, image length. */
		"%04u"	/* 40-43, left(motion detect rect). */
		"%04u"	/* 44-47, top(motion detect rect). */
		"%04u"	/* 48-51, right(motion detect rect). */
		"%04u"	/* 52-55, bottom(motion detect rect). */
		"%1u"	/* 56, ftp state. */
		"%03u"	/* 57-59, saturation. */
		"%06u"	/* 60-65, motion detect index. */
		"%03u"	/* 66-68, hue. */
		"%03u"	/* 69-71, sharpness. */
		"%1u"	/* 72, motion detect way. */
		"%1u"	/* 73, sensor frequency. */
		"%1u"	/* 74, channel mode. */
		"%02u"	/* 75-76, channel value. */
		"%03u"	/* 77-79, audio volume. */
		"%1u"	/* 80, dynamic DNS state. */
		"%1u"	/* 81, audio state. */
		"%03u"	/* 82-84, frame rate */
		"%03u"  /* 85-87, Speaker volume */
		"%03u"  /* 88-90, Mic volume */
		"%1u"   /*91, Show Time*/
		"%02x"  /*92-93, WiFi Strength*/
		"%02x", /*94, BatteryLevel*/		
		(unsigned int)(pState->ucCamera),
		(unsigned int)(pState->ucModem),
		(unsigned int)(pState->ucPPPoE),
		(unsigned int)(pState->ucX_Direction),
		(unsigned int)(pState->ucY_Direction),
		(unsigned int)(pState->usFocus),
		(unsigned int)(pState->ucBright),
		(unsigned int)(pState->ucContrast),
		(unsigned int)(pState->ucResolution),
		(unsigned int)(pState->ucCompressionRatio),
		(unsigned int)(pState->ucPrivilege),
		pState->uiPicIndex,
		(unsigned int)(pState->ucEmail),
		(unsigned int)(pState->ucUserCheck),
		pState->uiPicSize,
		(unsigned int)(pState->rMoniteRect.iLeft),
		(unsigned int)(pState->rMoniteRect.iTop),
		(unsigned int)(pState->rMoniteRect.iRight),
		(unsigned int)(pState->rMoniteRect.iBottom),
		(unsigned int)(pState->ucFtp),
		(unsigned int)(pState->ucSaturation),
		(unsigned int)(pState->uiPicMotionDetectedIndex),
		(unsigned int)(pState->ucHue),
		(unsigned int)(pState->ucSharpness),
		(unsigned int)(pState->ucMotionDetectWay),
		(unsigned int) 0,
		(unsigned int) 0,
		(unsigned int) 0,
		(unsigned int) 0,
		(unsigned int)(pState->iDDNSState),
		(unsigned int) 0,
		(unsigned int)(pState->ucFramerate),
		(unsigned int)(pState->ucSpeakerVolume),
		(unsigned int)(pState->ucMicVolume),
		(unsigned int)(pState->bShowTime),
		(unsigned int)(pState->WiFiStrength),
		(unsigned int)(pState->BatteryLevel)
					   
		);
	return iCount;
}

void SetRunTimeState(HTTPCONNECTION hConnection)
{

	g_WebCamState.ucUserCheck = (unsigned char)g_ConfigParam.bUserCheck;
	if (g_WebCamState.ucUserCheck)
		g_WebCamState.ucPrivilege = (unsigned char)GetPrivilege(hConnection);
	else
		g_WebCamState.ucPrivilege = 0;
	g_WebCamState.ucMotionDetectWay = g_ConfigParam.ucMotionDetectWay;
}



int Config_GetStatus(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	char pcBuf[128];

	switch (iAction)

	{

	case CA_AUTH:
		return AUTH_USER;
	case CA_CONFIG:
		SetRunTimeState(hConnection);
		cyg_mutex_lock(&g_ptmState);
		GetWebCamStateString(&g_WebCamState, pcBuf);
		cyg_mutex_unlock(&g_ptmState);
		AddHttpValue(pReturnXML, "Status", pcBuf);
		return 0;
	}
	return -1;
}


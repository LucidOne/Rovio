#include "../Inc/CommonDef.h"
extern const char *g_apcNetworkInterface[2];
#ifndef WLAN
extern CHAR *g_StreamServer_Buf;
#endif

void W99802Reboot(void)
{
#ifndef WLAN
	unsigned int iValue;
	iValue = *((volatile unsigned int *)0x7FF00008);
	iValue |= 1;
	*((volatile unsigned int *)0x7FF00008) = iValue;
#else
	//vp_bitrate_control_uninit();

#if 0	
	DownInterface("wlan0");
	wlan_cleanup_module( );

	/* Reset WiFi card */
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & 0xFFFDFFFF);		/* Set GPIO17 output */
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x00020000);	/* Set GPIO17 high */
	tt_msleep(10);
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & 0xFFFDFFFF);	/* Set GPIO17 low */
	tt_msleep(10);
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x00020000);	/* Set GPIO17 high */
	tt_msleep(10);
#endif	

	//cyg_thread_set_priority(cyg_thread_self(), CYGPKG_NET_FAST_THREAD_PRIORITY - 2);
	
	usiMyLock();
	usiMyFlash();

	cyg_interrupt_disable();
	
//cplu++
	/* Reset WiFi card */
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & 0xFFFDFFFF);		/* Set GPIO17 output */
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x00020000);	/* Set GPIO17 high */
	//tt_msleep(10);
	{ int volatile i; for (i = 0; i < 100000; i++); }
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & 0xFFFDFFFF);	/* Set GPIO17 low */
	//tt_msleep(10);
	{ int volatile i; for (i = 0; i < 100000; i++); }
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x00020000);	/* Set GPIO17 high */
	//tt_msleep(10);
	{ int volatile i; for (i = 0; i < 100000; i++); }
	

	/* Set reboot flag for bootloader */
	diag_printf("Now reboot w99802.......");
#if defined IPCAM_CONFIG_MP4_EVB_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_1
	outpw(REG_HICBUF + 0x4, 0x03);	/* HICBUF1 power ready */
#elif defined IPCAM_CONFIG_IP_CAM_VER_0 || defined IPCAM_CONFIG_IP_CAM_VER_1 || defined IPCAM_CONFIG_IP_CAM_VER_2 || defined IPCAM_CONFIG_IP_CAM_VER_3
	outpw(REG_HICBUF + 0x4, 0x02);	/* HICBUF1 power ready */
#else
#	error "No hardware config defined!"
#endif

	/* Disable LED */
	outpw(REG_GPIOB_OE,inpw(REG_GPIOB_OE) & ~(UINT32)(LED_GPIO_RED | LED_GPIO_GREEN)); //set gpiob24 and gpio25 output mode
	outpw(REG_GPIOB_DAT,inpw(REG_GPIOB_DAT) | (UINT32)(LED_GPIO_RED | LED_GPIO_GREEN)); // set gpiob24 and gpio25 high(not light)

	/* Reset W99802 */
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & 0xFFFFFFF7);		/* Set GPIO3 output */
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & 0xFFFFFFF7);	/* Set GPIO3 low */
	{ int volatile i; for (i = 0; i < 50000; i++); }
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x00000008);	/* Set GPIO3 high */
	{ int volatile i; for (i = 0; i < 50000; i++); }
	
	
	//tt_msleep(100);
	//outpw(REG_HICBUF + 0x4, 0x02);	/* HICBUF1 reset */
	
	//outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & 0xFFFFFFF7);		/* Set GPIO3 output */
	//outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & 0xFFFFFFF7);	/* Set GPIO3 low */
	//tt_msleep(10);
	//outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x00000008);	/* Set GPIO3 high */
#endif
}
/* Set a certain interface flag. */

static int set_flag(int skfd,char *ifname, short flag)
{
    struct ifreq ifr;

//    strcpy(ifr.ifr_name, ifname);
    snprintf(ifr.ifr_name,sizeof(ifr.ifr_name),ifname);

#ifndef WLAN
    if (netioctl(skfd, SIOCGIFFLAGS, &ifr,sizeof(ifr), g_StreamServer_Buf, System_BUFFER_LEN) < 0) 
#else
	if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) 
#endif
    {
		fprintf(stderr, "%s: unknown interface: %s\n",
			ifname,	strerror(errno));
		return (-1);
    }
    //strcpy(ifr.ifr_name, ifname);
    snprintf(ifr.ifr_name,sizeof(ifr.ifr_name),ifname);
    ifr.ifr_flags |= flag;
#ifndef WLAN    
    if (netioctl(skfd, SIOCSIFFLAGS, &ifr,sizeof(ifr), g_StreamServer_Buf, System_BUFFER_LEN) < 0) 
#else
    if (ioctl(skfd, SIOCSIFFLAGS, &ifr) < 0) 
#endif
    {
		perror("SIOCSIFFLAGS");
		return -1;
    }
    return (0);
}
/* Clear a certain interface flag. */

static int clr_flag(int skfd,char *ifname, short flag)
{
    struct ifreq ifr;

    //strcpy(ifr.ifr_name, ifname);
    snprintf(ifr.ifr_name,sizeof(ifr.ifr_name),ifname);
 #ifndef WLAN       
    if ( netioctl(skfd, SIOCGIFFLAGS, &ifr,sizeof(ifr), g_StreamServer_Buf, System_BUFFER_LEN) < 0) 
#else
    if ( ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) 
#endif
    {
		fprintf(stderr, "%s: unknown interface: %s\n",
			ifname, strerror(errno));
		return -1;
    }
    //strcpy(ifr.ifr_name, ifname);
    snprintf(ifr.ifr_name,sizeof(ifr.ifr_name),ifname);
    ifr.ifr_flags &= ~flag;
#ifndef WLAN
    if (netioctl(skfd, SIOCSIFFLAGS, &ifr, sizeof(ifr),g_StreamServer_Buf, System_BUFFER_LEN) < 0) 
#else
	if (ioctl(skfd, SIOCSIFFLAGS, &ifr) < 0) 
#endif    
    {
		perror("SIOCSIFFLAGS");
		return -1;
    }
    return (0);
}


char * Asmalloc(UsedBuf* asbuf,unsigned int *slen,int ulen)
{	
	if((*slen + ulen) <=  asbuf->AssignLen)
	{
		char *ch;
		ch  = asbuf->AssignBuffer + *slen;
		*slen += ulen;
		return ch;
	}
	else
	{
		diag_printf("there is not enought buffer in assigned buffer\n");
		return NULL;
	}
}


BOOL InitNetInterface(const char *pcInterface)
{
	struct ifreq ifr;
	int fd;
	diag_printf("Initializing %s...\n", pcInterface);
#ifndef WLAN
	if ((fd = socket(AF_INET,SOCK_DGRAM,0,g_StreamServer_Buf, System_BUFFER_LEN)) < 0)
#else
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
#endif
	{
		fprintf(stderr, "errno = %d\n", errno);
		return FALSE;
	}
	//strcpy(ifr.ifr_name, pcInterface);
	snprintf(ifr.ifr_name,sizeof(ifr.ifr_name),pcInterface);

	clr_flag(fd, ifr.ifr_name, IFF_UP);
	set_flag(fd, ifr.ifr_name, (IFF_UP | IFF_RUNNING));
#ifndef WLAN
	netclose(fd, g_StreamServer_Buf, System_BUFFER_LEN);
#else
	close(fd);
#endif
	diag_printf("Initialize %s... over \n", pcInterface);
	return TRUE;
}


unsigned long GetGeneralIP(const char *pcInterface, int iRequest)
{
	struct ifreq ifr;
	int fd;
	unsigned long ulRt;

	if (pcInterface == NULL)
	{
		fprintf(stderr, "illegal call function GetGeneralIP!\n");
		return 0L;
	}
#ifndef WLAN
	if ((fd = socket(AF_INET,SOCK_DGRAM,0, g_StreamServer_Buf, System_BUFFER_LEN)) < 0) 
		return 0L;
#else	
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) 
		return 0L;
#endif
	//strcpy(ifr.ifr_name, pcInterface);
	snprintf(ifr.ifr_name,sizeof(ifr.ifr_name),pcInterface);

#ifndef WLAN
	if (netioctl(fd, iRequest, &ifr,sizeof(ifr), g_StreamServer_Buf, System_BUFFER_LEN) < 0)
#else
	if (ioctl(fd, iRequest, &ifr) < 0)
#endif
		ulRt = 0;
	else
		ulRt = (*(struct sockaddr_in *)&(ifr.ifr_addr)).sin_addr.s_addr;
#ifndef WLAN		
	netclose(fd, g_StreamServer_Buf, System_BUFFER_LEN);
#else
	close(fd);
#endif
	return ulRt;
}

BOOL SetGeneralIP(const char *pcInterface, unsigned long ulIP, unsigned long ulNetmask)
{
	struct ifreq ifr;
	int ret;
	int fd;
	struct sockaddr_in *pAddr;

	if (pcInterface == NULL)
	{
		fprintf(stderr, "illegal call function SetGeneralIP!\n");
		return FALSE;
	}
#ifndef WLAN
	if ((fd = socket(AF_INET,SOCK_DGRAM,0, g_StreamServer_Buf, System_BUFFER_LEN)) < 0) 
		return FALSE;
#else
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) 
		return FALSE;
#endif
	memset((void*)&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, pcInterface);
#ifdef WLAN

    if (ioctl(fd, SIOCGIFFLAGS, &ifr)) {
        diag_printf("SIOCSIFFLAGS error %x\n", errno);
        close(fd);
        return false;
    }
    ifr.ifr_flags &= ~IFF_UP;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr)) {
        diag_printf("SIOCSIFFLAGS error %x\n", errno);
        close(fd);
        return false;
    }
    ret = ioctl(fd, SIOCGIFADDR, &ifr);
    if(ret < 0 && (errno != EADDRNOTAVAIL))
    {
        perror("SIOCIFADDR");
    }
    
    ret = ioctl(fd, SIOCDIFADDR, &ifr);
    if(ret < 0 && (errno != EADDRNOTAVAIL))
    {
        perror("SIOCIFADDR");
    }    
    
	ifr.ifr_flags = IFF_UP | IFF_BROADCAST | IFF_RUNNING;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr)) 
    {
        diag_printf("SIOCSIFFLAGS error %x\n", errno);
		close(fd);
        return false;
    }
 #endif
	pAddr = (struct sockaddr_in *)&(ifr.ifr_addr);
	bzero(pAddr, sizeof(struct sockaddr_in));
	pAddr->sin_addr.s_addr = ulIP;
	pAddr->sin_family = AF_INET;
	pAddr->sin_len = sizeof(*pAddr);
#ifndef WLAN	
	if (netioctl(fd, SIOCSIFADDR, &ifr, sizeof(ifr),g_StreamServer_Buf, System_BUFFER_LEN) < 0)
#else
	if (ioctl(fd, SIOCSIFADDR, &ifr) < 0)
#endif
	{
		diag_printf("can not set ip errno = %x\n",errno);
#ifndef WLAN
		netclose(fd, g_StreamServer_Buf, System_BUFFER_LEN);
#else
		close(fd);
#endif
		return FALSE;
	}

	pAddr = (struct sockaddr_in *)&(ifr.ifr_addr);
	bzero(pAddr, sizeof(struct sockaddr_in));
	pAddr->sin_addr.s_addr = ulNetmask;
	pAddr->sin_family = AF_INET;
	pAddr->sin_len = sizeof(*pAddr);
#ifndef WLAN
	if (netioctl(fd, SIOCSIFNETMASK, &ifr,sizeof(ifr), g_StreamServer_Buf, System_BUFFER_LEN) < 0)
#else
	if (ioctl(fd, SIOCSIFNETMASK, &ifr) < 0)
#endif
	{
		diag_printf("Can not set netmask!\n");
#ifndef WLAN
		netclose(fd, g_StreamServer_Buf, System_BUFFER_LEN);
#else
		close(fd);
#endif
		return FALSE;
	}
#ifndef WLAN
	netclose(fd, g_StreamServer_Buf, System_BUFFER_LEN);
#else
	close(fd);
#endif
	return TRUE;
}

unsigned long GetIPAddress(const char *pcInterface)
{
	return GetGeneralIP(pcInterface, SIOCGIFADDR);
}

unsigned long GetNetMask(const char *pcInterface)
{
	return GetGeneralIP(pcInterface, SIOCGIFNETMASK);
}


unsigned long GetGateway(const char *pcInterface)
{
#ifndef WLAN
	return wb740getgateway(pcInterface,g_StreamServer_Buf, System_BUFFER_LEN);
#else
	return 0;
#endif
}

void set_sockaddr(struct sockaddr_in *sin, unsigned long addr, unsigned short port)
{
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = addr;
	sin->sin_port = port;
}

BOOL SetGateway(const char *pcInterface, unsigned long ulIP)
{
#ifndef WLAN
	struct rtentry rItem;
#else
	struct ecos_rtentry rItem;
#endif
	int fd;
	unsigned ulOldGateway;
	int rt;
#ifndef WLAN
	if ((fd = socket(AF_INET,SOCK_DGRAM,0,  g_StreamServer_Buf, System_BUFFER_LEN)) < 0) 
		return FALSE;
#else
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) 
		return FALSE;	
#endif
	memset(&rItem, 0, sizeof(rItem));
#ifndef WLAN
	set_sockaddr((struct sockaddr_in *) &rItem.rt_dst, 0, 0);
	set_sockaddr((struct sockaddr_in *) &rItem.rt_genmask, 0, 0);
	rItem.rt_flags = RTF_UP | RTF_GATEWAY;
#else
	set_sockaddr((struct sockaddr_in *) &rItem.rt_dst, 0, 0);
	set_sockaddr((struct sockaddr_in *) &rItem.rt_genmask, 0, 0);
	rItem.rt_flags = RTF_UP| RTF_GATEWAY;
#endif
	rItem.rt_dev = (char *)pcInterface;
	ulOldGateway = GetGateway(pcInterface);
#ifndef WLAN	
	if (ulOldGateway != 0L)
	{
		set_sockaddr((struct sockaddr_in *)&rItem.rt_gateway, ulOldGateway, 0);
		if ((netioctl_withbuf(fd, SIOCDELRT, &rItem , sizeof(rItem), g_StreamServer_Buf, System_BUFFER_LEN)) < 0)
			fprintf(stderr, "Cannot del default route (%d).\n", errno);
	}

	set_sockaddr((struct sockaddr_in *) &rItem.rt_gateway, ulIP, 0);

	if ((rt = netioctl_withbuf(fd,SIOCADDRT, &rItem, sizeof(rItem),g_StreamServer_Buf,System_BUFFER_LEN)) < 0)
	{
		fprintf(stderr, "Cannot add default route (%d).\n", errno);
	}
	netclose(fd, g_StreamServer_Buf, System_BUFFER_LEN);
#else
	set_sockaddr((struct sockaddr_in *) &rItem.rt_gateway, ulIP, 0);

	if ((rt = ioctl(fd,SIOCDELRT, &rItem)) < 0 && (errno != ESRCH))
	{
		fprintf(stderr, "Cannot delete default route (%d).\n", errno);
	}

	memset(&rItem, 0, sizeof(rItem));
	
	set_sockaddr((struct sockaddr_in *) &rItem.rt_dst, 0, 0);
	set_sockaddr((struct sockaddr_in *) &rItem.rt_genmask, 0, 0);
	rItem.rt_flags = RTF_UP | RTF_GATEWAY;

	rItem.rt_dev = (char *)pcInterface;

	set_sockaddr((struct sockaddr_in *) &rItem.rt_gateway, ulIP, 0);

	if ((rt = ioctl(fd,SIOCADDRT, &rItem)) < 0)
	{
		fprintf(stderr, "Cannot add default route (%d).\n", errno);
	}
	close(fd);	

#endif
	return (rt < 0 ? FALSE : TRUE);
}

void thread_join(cyg_handle_t* handle, cyg_thread* thread, cyg_thread_info* info)
{
    BOOL ex = FALSE;
    while((ex == FALSE) && (handle != NULL))
    {
        cyg_thread_get_info(*handle, thread->unique_id, info);
            //diag_printf("state is %d\n",info->state);
       
        switch(info->state)
        {
            case 16:
                cyg_thread_delete(*handle);
                ex = TRUE;
                break;
            default:
                cyg_thread_yield();
                break;
        }
    }
}

void WebCameraSIGTERM(int iSignal)
{
#if 0

#ifdef USE_DDNS	
	cyg_thread_info SelectInfo;
#endif
#ifdef USE_FLASH
	cyg_thread_info FlaseInfo;
#endif
	cyg_thread_info CameraInfo;
	MSG_T msg;
	msg.lMsg = MSG_QUIT;
	
	//MctestThread_release();
	//diag_printf("Delete Mctest Server!\n");
	
	if (ptdCamera_handle != NULL)
	{
		SendCameraMsg(MSG_QUIT, 0);
		thread_join(&ptdCamera_handle,&g_ptdCamera,&CameraInfo);
		diag_printf("Close the Camera Thread!!!");
	}
	if (g_pMsgCamera != NULL)
	{
		FreeMsgQueue(g_pMsgCamera);
		diag_printf("Delete msg queue (main & camera), Ok!\n");
	}		
	
#ifdef RECORDER 
	recorder_releative_thread_release();
	recorder_free();
	diag_printf("Delete Recorder!\n");
#endif
#ifdef USE_DDNS	
	if (g_pMsgSelect != NULL) SendMsg(g_pMsgSelect, &msg);
	if (ptdSelect_handle != NULL)
	{
		thread_join(&ptdSelect_handle,&g_ptdSelect,&SelectInfo);
	}
	if (g_pMsgSelect != NULL)
	{
		FreeMsgQueue(g_pMsgSelect);
		g_pMsgSelect = NULL;
		diag_printf("Delete msg queue (select), Ok!\n");
	}

#endif
	
#ifdef USE_FLASH
	if (g_pMsgFlash != NULL) SendMsg(g_pMsgFlash, &msg);
	if (ptdFlash_handle != NULL)
	{
		thread_join(&ptdFlash_handle,&g_ptdFlash,&FlaseInfo);
	}
	if (g_pMsgFlash != NULL)
	{
		FreeMsgQueue(g_pMsgFlash);
		g_pMsgFlash = NULL;
		diag_printf("Delete msg queue (flash), Ok!\n");
	}
#endif

#endif
}


int Config_SetIP(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	const char *pcIPAssignedWay;
	const char *pcInterface;
	int iInterface;
	unsigned char ucIPAssignedWay;
	int i;
	BOOL bIsChange = FALSE;

	unsigned long ulClientIP;
	ulClientIP = httpGetClientAddr(hConnection).s_addr;

	switch (iAction)
	{
	case CA_AUTH:
		//diag_printf("CA_AUTH Config_SetIP!\n");
		tt_rmutex_lock(&g_rmutex);
		if (ulClientIP == 0x0100007f) return AUTH_ANY;
		else return AUTH_ADMIN;
		break;
	case CA_CONFIG:
		diag_printf("CA_CONFIG Config_SetIP!\n");
		if (httpIsExistParam(pParamList, "CameraName"))
		{
			httpMyStrncpy(g_ConfigParam.acCameraName, httpGetString(pParamList, "CameraName"), sizeof(g_ConfigParam.acCameraName));
			bIsChange = TRUE;
		}
#ifndef WLAN
		if (!httpIsExistParam(pParamList, "Interface"))
			pcInterface = g_apcNetworkInterface[0];
#else
		if (!httpIsExistParam(pParamList, "Interface"))
			pcInterface = g_apcNetworkInterface[1];
#endif
		else pcInterface = httpGetString(pParamList, "Interface");
		for (iInterface = 0;
			iInterface < sizeof(g_apcNetworkInterface) / sizeof(const char *);
			iInterface++)
			if (strcasecmp(g_apcNetworkInterface[iInterface], pcInterface) == 0)
				break;
		if (iInterface < sizeof(g_apcNetworkInterface) / sizeof(const char *))
		{
			BOOL bAsNetEnable_Before = g_ConfigParam.abAsNetEnable[iInterface];
			g_ConfigParam.abAsNetEnable[iInterface] = httpGetBool(pParamList, "Enable");

			if (g_ConfigParam.abAsNetEnable[iInterface] != bAsNetEnable_Before)
				bIsChange = TRUE;

			pcIPAssignedWay = httpGetString(pParamList, "IPWay");
			if (strcasecmp(pcIPAssignedWay, "manually") == 0)
				ucIPAssignedWay = IP_ASSIGNMENT_MANUALLY;
			else if (strcasecmp(pcIPAssignedWay, "dhcp") == 0)
				ucIPAssignedWay = IP_ASSIGNMENT_DHCP;
			else ucIPAssignedWay = g_ConfigParam.aucIPAssignedWay[iInterface];
			if (ucIPAssignedWay != g_ConfigParam.aucIPAssignedWay[iInterface])
			{
				g_ConfigParam.aucIPAssignedWay[iInterface] = ucIPAssignedWay;
				bIsChange = TRUE;
			}
			if (httpIsExistParam(pParamList, "IP"))
			{
				g_ConfigParam.ulAsIPAddress[iInterface] = httpGetIP(pParamList, "IP");
				bIsChange = TRUE;
			}
			if (httpIsExistParam(pParamList, "Netmask"))
			{
				g_ConfigParam.ulAsNetmask[iInterface] = httpGetIP(pParamList, "Netmask");
				bIsChange = TRUE;
			}
			if (httpIsExistParam(pParamList, "Gateway"))
			{
				g_ConfigParam.ulAsGateway[iInterface] = httpGetIP(pParamList, "Gateway");
				bIsChange = TRUE;
			}
		}

		for (i = 0; i < 3; i++)
		{
			char ac[16];
			sprintf(ac, "DNS%d", i);
			if (httpIsExistParam(pParamList, ac))
			{
				g_ConfigParam.aulAsDNS[i] = httpGetIP(pParamList, ac);
				bIsChange = TRUE;
			}
		}
	
		if (bIsChange == TRUE)
		{
			WebCameraLog(CL_PRIVILEGE_COMMON, CL_SET_IP, pcIPAssignedWay, hConnection);
			WriteFlashMemory(&g_ConfigParam);
		}
		tt_rmutex_unlock(&g_rmutex);
		/*Set new ip address*/
 		//SetIP();
 		wsp_set_network_ip();
 			
		diag_printf("CA_CONFIG Config_SetIP -- end!\n");
		return 0;
		break;
	}
	return -1;
}

int Config_GetIP(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	char ac[128];
	int i;
	const char *pcInterface;
	int iInterface;
	unsigned long ulClientIP;
	ulClientIP = httpGetClientAddr(hConnection).s_addr;

	switch (iAction)
	{
	case CA_AUTH:
		if (ulClientIP == 0x0100007f) return AUTH_ANY;
		else return AUTH_USER;
		break;
	case CA_CONFIG:
		tt_rmutex_lock(&g_rmutex);
		AddHttpValue(pReturnXML, G_PC_CAMERANAME, g_ConfigParam.acCameraName);
		for (i = 0; i < 3; i++)
		{
			char acName[8];
			sprintf(acName, G_PC_DNS_N, i); 
			httpIP2String(g_ConfigParam.aulAsDNS[i], ac);
			AddHttpValue(pReturnXML, acName, ac);
		}
#ifndef WLAN
		if (!httpIsExistParam(pParamList, G_PC_INTERFACE))
			pcInterface = g_apcNetworkInterface[0];
#else
		if (!httpIsExistParam(pParamList, G_PC_INTERFACE))
			pcInterface = g_apcNetworkInterface[1];
#endif
		else pcInterface = httpGetString(pParamList, G_PC_INTERFACE);
		for (iInterface = 0;
			iInterface < sizeof(g_apcNetworkInterface) / sizeof(const char *);
			iInterface++)
			if (strcasecmp(g_apcNetworkInterface[iInterface], pcInterface) == 0)
				break;

		if (iInterface < sizeof(g_apcNetworkInterface) / sizeof(const char *))
		{
			httpLong2String(g_ConfigParam.abAsNetEnable[iInterface], ac);
			AddHttpValue(pReturnXML, G_PC_ENABLE, ac);
			AddHttpValue(pReturnXML, G_PC_IPWAY,
					(g_ConfigParam.aucIPAssignedWay[iInterface] == IP_ASSIGNMENT_MANUALLY?"manually":"dhcp")
				);
			httpIP2String(g_ConfigParam.ulAsIPAddress[iInterface], ac);				
			AddHttpValue(pReturnXML, G_PC_IP, ac);
			httpIP2String(g_ConfigParam.ulAsNetmask[iInterface], ac);
			AddHttpValue(pReturnXML, G_PC_NETMASK, ac);
			httpIP2String(g_ConfigParam.ulAsGateway[iInterface], ac);
			AddHttpValue(pReturnXML, G_PC_GATEWAY, ac);
		}
		
		
		{
			unsigned long ulCurrentIP;
			unsigned long ulCurrentSubnetMask;
			unsigned long ulCurrentGateway;
			unsigned long ulCurrentDNSServer[3];
			enum SET_IP_STATE_E ipState;
			GetPubIPInfo(&ulCurrentIP, &ulCurrentSubnetMask, &ulCurrentGateway, ulCurrentDNSServer);
			httpIP2String(ulCurrentIP, ac);
			AddHttpValue(pReturnXML, "CurrentIP", ac);
			httpIP2String(ulCurrentSubnetMask, ac);
			AddHttpValue(pReturnXML, "CurrentNetmask", ac);
			httpIP2String(ulCurrentGateway, ac);
			AddHttpValue(pReturnXML, "CurrentGateway", ac);
			
			for (i = 0; i < 3; i++)
			{
				char acName[16];
				sprintf(acName, "CurrentDNS%d", i);
				httpIP2String(ulCurrentDNSServer[i], ac);
				AddHttpValue(pReturnXML, acName, ac);
			}
			
			
			wsp_get_config_state(NULL, &ipState);
			switch(ipState)
			{
			case SET_IP__DHCP_TRYING:
				AddHttpValue(pReturnXML, "CurrentIPState", "DHCP_TRYING");
				break;
			case SET_IP__DHCP_FAILED:
				AddHttpValue(pReturnXML, "CurrentIPState", "DHCP_FAILED");
				break;
			case SET_IP__DHCP_OK:
				AddHttpValue(pReturnXML, "CurrentIPState", "DHCP_OK");
				break;
			case SET_IP__STATIC_IP_TRYING:
				AddHttpValue(pReturnXML, "CurrentIPState", "STATIC_IP_TRYING");
				break;
			case SET_IP__STATIC_IP_FAILED:
				AddHttpValue(pReturnXML, "CurrentIPState", "STATIC_IP_FAILED");
				break;
			case SET_IP__STATIC_IP_OK:
				AddHttpValue(pReturnXML, "CurrentIPState", "STATIC_IP_OK");
				break;
			default:
				AddHttpValue(pReturnXML, "CurrentIPState", "NO_INIT");		
			}
		}
		tt_rmutex_unlock(&g_rmutex);
		return 0;
		break;
	}
	return -1;
}

int Config_SetName(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	BOOL bIsChange = FALSE;

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
		break;
	case CA_CONFIG:
		if (httpIsExistParam(pParamList, "CameraName"))
		{
			httpMyStrncpy(g_ConfigParam.acCameraName, httpGetString(pParamList, "CameraName"), sizeof(g_ConfigParam.acCameraName));
			bIsChange = TRUE;
		}

		if (bIsChange == TRUE)
		{
			WebCameraLog(CL_PRIVILEGE_COMMON, CL_SET_NAME, g_ConfigParam.acCameraName, hConnection);
			WriteFlashMemory(&g_ConfigParam);
		}
		return 0;
		break;
	}
	return -1;
}


int Config_GetName(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_USER;
		break;
	case CA_CONFIG:
		AddHttpValue(pReturnXML, G_PC_CAMERANAME, g_ConfigParam.acCameraName);
		return 0;
		break;
	}
	return -1;
}


int Config_SetHttp(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	BOOL bIsChange = FALSE;
	unsigned long ulClientIP;
	int i;

	ulClientIP = httpGetClientAddr(hConnection).s_addr;

	switch (iAction)
	{
	case CA_AUTH:
		if (ulClientIP == 0x0100007f) return AUTH_ANY;
		else return AUTH_ADMIN;
		break;
	case CA_CONFIG:
		if (httpIsExistParam(pParamList, "Port"))
		{
			g_ConfigParam.usHttpPort[0] = (unsigned short)httpGetLong(pParamList, "Port");
			g_ConfigParam.usHttpPort[1] = 0;
			bIsChange = TRUE;
		}
		for (i = 0; i < sizeof(g_ConfigParam.usHttpPort) / sizeof(unsigned short); i++)
		{
			char ac[8];
			sprintf(ac, "Port%d", i);
			if (httpIsExistParam(pParamList, ac))
			{
				g_ConfigParam.usHttpPort[i] = (unsigned short)httpGetLong(pParamList, ac);
				bIsChange = TRUE;
			}
		}

		if (bIsChange == TRUE)
		{
			WebCameraLog(CL_PRIVILEGE_COMMON, CL_SET_HTTPPORT, httpGetString(pParamList, "Port"), hConnection);
			WriteFlashMemory(&g_ConfigParam);
		}
		/*Reboot the http server to change*/
#ifndef WLAN	
		WebCameraSIGTERM(0);
		diag_printf("Reboot the wb740!\n");
		wb740reboot(g_StreamServer_Buf, System_BUFFER_LEN);
#else
		WebCameraSIGTERM(0);
		W99802Reboot();
#endif		
		return 0;
		break;
	}
	return -1;
}

int Config_GetHttp(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	int i;

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_USER;
		break;
	case CA_CONFIG:
		for (i = 0; i < sizeof(g_ConfigParam.usHttpPort) / sizeof(unsigned short); i++)
		{
			char acName[16];
			char ac[64];
			sprintf(acName, G_PC_PORT_N, i);
			httpLong2String(g_ConfigParam.usHttpPort[i], ac);
			AddHttpValue(pReturnXML, acName, ac);
		}
		return 0;
		break;
	}
	return -1;
}


static int onHttpReboot_Safe (HTTPCONNECTION hConnection, time_t *tLastFill, void *pParam)
{
	request *req = (request *) hConnection;
#ifndef WLAN
	netclose (req->fd, g_StreamServer_Buf, System_BUFFER_LEN);
#else
	close(req->fd);
#endif	
#ifndef WLAN	
	WebCameraSIGTERM(0);
	diag_printf("Reboot the wb740!\n");
	wb740reboot(g_StreamServer_Buf, System_BUFFER_LEN);
#else
	WebCameraSIGTERM(0);
	W99802Reboot();
#endif

	return 0;
}

static int onHttpReboot (HTTPCONNECTION hConnection, time_t *tLastFill, void *pParam)
{
	httpAddBodyString (hConnection, " ");
	httpSetSendDataOverFun(hConnection, onHttpReboot_Safe, NULL);
	return 1;
}

void RebootOnConnectionOver(HTTPCONNECTION hConnection)
{
	httpSetKeepAliveMode(hConnection, FALSE);
	httpSetSendDataOverFun(hConnection, onHttpReboot, NULL);
}


int Config_Reboot(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	unsigned long ulClientIP = httpGetClientAddr(hConnection).s_addr;

	switch (iAction)
	{
	case CA_AUTH:
		diag_printf("CA_AUTH Config_Reboot!\n");
		if (ulClientIP == 0x0100007f) return AUTH_ANY;
		else return AUTH_ADMIN;
		break;
	case CA_CONFIG:
	#if 1
		diag_printf("CA_CONFIG Config_Reboot!\n");
		AddHttpValue(pReturnXML, G_PC_RT, "OK");
		RebootOnConnectionOver(hConnection);
	#endif
		return 0;
		break;
	}
	return -1;
}


#include "dhcp.h"
void GetPubIPInfo(unsigned long *pulPublicIpAddress,
							unsigned long *pulPublicSubnetMask,
							unsigned long *pulDefaultGateway,
							unsigned long *pulDNSServer)
{
	BOOL bEnable[2];
	int fd;
	struct ifreq ifr;
	bEnable[0] = g_ConfigParam.abAsNetEnable[0];
	bEnable[1] = g_ConfigParam.abAsNetEnable[1];
	
	
#ifndef WLAN
	if (!bEnable[0] && !bEnable[1]) bEnable[0] = TRUE;
   	fd = socket(AF_INET, SOCK_DGRAM, 0, g_Mctest_Buf, MCTEST_BUFFER_LEN);
	strcpy(ifr.ifr_name, g_apcNetworkInterface[0]);
#else
	if (!bEnable[0] && !bEnable[1]) bEnable[1] = TRUE;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(ifr.ifr_name, g_apcNetworkInterface[1]);
#endif
    if (fd < 0) 
    {
        perror("socket");
        return;
    }

  	/*Get the IP Address*/ 
	if(pulPublicIpAddress != NULL)
	{        
#ifndef WLAN  	   
        if (netioctl(fd, SIOCGIFADDR, &ifr, sizeof(ifr),g_Mctest_Buf, MCTEST_BUFFER_LEN) >= 0)
#else
        if (ioctl(fd, SIOCGIFADDR, &ifr) >= 0)
#endif         
            *pulPublicIpAddress = (unsigned int)(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr);
         else
         	*pulPublicIpAddress = 0;
#ifndef WLAN  	   
        diag_printf("ipaddr Address====%s\n",inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr, g_Mctest_Buf, MCTEST_BUFFER_LEN));  
#else
        diag_printf("ipaddr Address====%s\n",inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));  
#endif 
	}   
	     
	/*Get the Netmask*/
	if(pulPublicSubnetMask != NULL)
	{         	
#ifndef WLAN  	   
        if (netioctl(fd, SIOCGIFNETMASK, &ifr,sizeof(ifr), g_Mctest_Buf, MCTEST_BUFFER_LEN) >= 0)
#else
        if (ioctl(fd, SIOCGIFNETMASK, &ifr) == 0)
#endif         
            *pulPublicSubnetMask = (unsigned int)(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr);
        else
            *pulPublicSubnetMask = 0;
#ifndef WLAN  	   
        diag_printf("Netmask====%s\n",inet_ntoa(((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr, g_Mctest_Buf, MCTEST_BUFFER_LEN));    
#else
        diag_printf("Netmask====%s\n",inet_ntoa(((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr));    
#endif   
	}
	
     /*Get the Gateway*/
	if(pulDefaultGateway != NULL)
	{
#ifndef WLAN         
        {
        	unsigned long Gateway;
        	char tmpgw[16];
        	char *c,*p;
        	int i = 0;
        	Gateway = wb740getgateway((char*)(ifr.ifr_name),g_Mctest_Buf, MCTEST_BUFFER_LEN);
        	diag_printf("Gateway====%s\n",inet_ntoa(*(struct in_addr*)&Gateway, g_Mctest_Buf, MCTEST_BUFFER_LEN));
        	if(Gateway != 0L)
        	{
        		strcpy(tmpgw,inet_ntoa(*(struct in_addr*)&Gateway, g_Mctest_Buf, MCTEST_BUFFER_LEN));
        		p = tmpgw;
        		while(*p != '\0')
        		{
        			c = p;
        			while((*p != '.') && (*p != '\0'))
        				p++;
        			*p = '\0';
        			ipmc[dev_num].gwaddr[i] = (unsigned char)atoi(c);          			
        			if(i < 3)
        			{p++; i++;  }  		
        		}	
				
			} 
			else 
				memset(ipmc[dev_num].gwaddr,0 ,4);
			
		}
#else
		if (g_ConfigParam.aucIPAssignedWay[1] == IP_ASSIGNMENT_MANUALLY)
			*pulDefaultGateway = g_ConfigParam.ulAsGateway[1];
		else
		{
			struct in_addr gateway;
			unsigned int length = sizeof(gateway);
			get_bootp_option(&wlan0_bootp_data, TAG_GATEWAY, (void*)&gateway,&length);
			diag_printf("gateway: %x\n", gateway.s_addr);
			*pulDefaultGateway = gateway.s_addr;
		}
#endif
	}
#ifndef WLAN
    netclose(fd, g_Mctest_Buf, MCTEST_BUFFER_LEN);
#else
    close(fd);
#endif
	if(pulDNSServer != NULL)
	{
		if (g_ConfigParam.aucIPAssignedWay[1] == IP_ASSIGNMENT_MANUALLY)
		{
			*pulDNSServer = g_ConfigParam.aulAsDNS[0];
   			*(pulDNSServer+1) = g_ConfigParam.aulAsDNS[1];
   			*(pulDNSServer+2) = g_ConfigParam.aulAsDNS[2];
   		}
   		else
   		{
			struct in_addr gateway;
			unsigned int length = sizeof(gateway);
			get_bootp_option(&wlan0_bootp_data, TAG_GATEWAY, (void*)&gateway,&length);
			diag_printf("gateway as dns: %x\n", gateway.s_addr);
			
			*pulDefaultGateway = gateway.s_addr;
			*pulDNSServer = gateway.s_addr;
   			*(pulDNSServer+1) = 0;
   			*(pulDNSServer+2) = 0;
   		}
	}
	
	if (g_ConfigParam.aucIPAssignedWay[1] == IP_ASSIGNMENT_DHCP
		&& wlan0_dhcpstate != DHCPSTATE_BOOTP_FALLBACK
		&& wlan0_dhcpstate != DHCPSTATE_BOUND
		)
	{
		if (g_WebCamState.bDHCP_Finished)
		{
			unsigned int GetRandomIP(void);
			unsigned int ulIP = GetRandomIP();
			if (pulPublicIpAddress != NULL)
				*pulPublicIpAddress = ulIP;
			if (pulPublicSubnetMask != NULL)
				*pulPublicSubnetMask = 0x0000FFFF;
			if (pulDefaultGateway != NULL)
				*pulDefaultGateway = ulIP;
		}
		else
		{
			if (pulPublicIpAddress != NULL)
				*pulPublicIpAddress = 0;
			if (pulPublicSubnetMask != NULL)
				*pulPublicSubnetMask = 0xFFFFFFFF;
			if (pulDefaultGateway != NULL)
				*pulDefaultGateway = 0;
		}
		if (pulDNSServer != NULL)
		{
			*pulDNSServer = 0;
			*(pulDNSServer + 1) = 0;
			*(pulDNSServer + 2) = 0;
		}
	}
    return;

}


int Config_ControlMCU(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_USER;
		break;
	case CA_CONFIG:
		if (!httpIsExistParam(pParamList, "parameters"))
			AddHttpValue(pReturnXML, "responses", "error");
		else
		{
			request *req = (request*)hConnection;
			const char *pcMCU;
			char acResponse[257 * 2 + 1];
			pcMCU = httpGetString(pParamList, "parameters");
			
			//diag_printf("MCU = %s\n", pcMCU);
			
			if (strlen(pcMCU) != ((int)(unsigned char)httpHex2Char(pcMCU) + 1)*2)
				AddHttpValue(pReturnXML, "responses", "error_length");
			else if (ictlCtrlMCU((ICTL_HANDLE_T*)&req->httpPrivilege, pcMCU + 2, acResponse + 2, sizeof(acResponse) - 2)
				!= ICTL_OK)
				AddHttpValue(pReturnXML, "responses", "error_response");
			else
			{
				int nLength = strlen(acResponse + 2) / 2;
				char acHex[4];
				httpChar2Hex(nLength, acHex);
				memcpy(acResponse, acHex, 2);
				AddHttpValue(pReturnXML, "responses", acResponse);
			}
			return 0;
		}
		break;
	}
	return -1;
}


static BOOL g_netIsSuspendAllowed = FALSE;
extern int ethernetRunning;
BOOL netIsSuspendAllowed()
{
	if (ethernetRunning == 0)
		return FALSE;
	if (vp_bitrate_control_getspeed() > 0)
		return FALSE;
	return g_netIsSuspendAllowed;
}


BOOL SetWlan(void)
{
#if 1
	DownInterface("wlan0");
	if (g_ConfigParam.ucWlanOperationMode == 1)	/* Disable suspend in Ad-hoc mode */
		g_netIsSuspendAllowed = FALSE;
	
	if (g_ConfigParam.ucWlanOperationMode == (unsigned char)IW_MODE_ADHOC)
	{
		wsp_set_network_adhoc(g_ConfigParam.acWlanESSID,
			g_ConfigParam.acWlanKey,
			g_ConfigParam.ulWlanChannel);
	}
	else
	{
		wsp_set_network_managed(g_ConfigParam.acWlanESSID, g_ConfigParam.acWlanKey);
	}

#else
	DownInterface("wlan0");
	
		/* ÔOWLAN */
		if (g_ConfigParam.ucWlanOperationMode == 1)	/* Disable suspend in Ad-hoc mode */
			g_netIsSuspendAllowed = FALSE;

		if (!SetWlanOperationMode(g_apcNetworkInterface[1],
			(int)g_ConfigParam.ucWlanOperationMode))
			fprintf(stderr, "Can not set operation mode.\n");
		if (!SetWlanChannel(g_apcNetworkInterface[1],
			(int)g_ConfigParam.ulWlanChannel))
			fprintf(stderr, "Can not set wireless channel.\n");
		
		if (!SetWlanRegion(g_apcNetworkInterface[1], 0x40))
			fprintf(stderr, "Can not set the region for wlan\n");
		
		if (!SetWlanMTU(g_apcNetworkInterface[1], 460))
			fprintf(stderr, "Can not set MTU for wlan\n");
	
		if (!wlanSetBitrate(g_apcNetworkInterface[1],-1))
			fprintf(stderr, "Can not set wlan bitrate\n");
			
		switch (g_ConfigParam.ucWepSet)
		{
		case WEP_SET_DISABLE:
			SetWlanWepKey(g_apcNetworkInterface[1],
				NULL,
				-1,
				-1,
				g_ConfigParam.ulWepAuthentication);
			break;
		case WEP_SET_K64:
			SetWlanWepKey(g_apcNetworkInterface[1],
					NULL,
					-1,
					-1,
					g_ConfigParam.ulWepAuthentication);	//Disable WEP key
			SetWlanESSID(g_apcNetworkInterface[1], (const char *)g_ConfigParam.acWlanESSID);
			DownInterface("wlan0");
			if (SetWlanWepKey(g_apcNetworkInterface[1],
					g_ConfigParam.acWep64,
					5,
					-1,
					g_ConfigParam.ulWepAuthentication)	//SET WEP key
				&& SetWlanWepKey(g_apcNetworkInterface[1],
					NULL,
					0,
					-1,
					g_ConfigParam.ulWepAuthentication)//enable WEP
				)
				diag_printf("set wepset k64\n");
			break;
		case WEP_SET_K128:
			SetWlanWepKey(g_apcNetworkInterface[1],
					NULL,
					-1,
					-1,
					g_ConfigParam.ulWepAuthentication);	//Disable WEP key
			SetWlanESSID(g_apcNetworkInterface[1], (const char *)g_ConfigParam.acWlanESSID);
			DownInterface("wlan0");
			if (SetWlanWepKey(g_apcNetworkInterface[1],
					g_ConfigParam.acWep128,
					13,
					-1,
					g_ConfigParam.ulWepAuthentication)//SET WEP key
				&& SetWlanWepKey(g_apcNetworkInterface[1],
					NULL,
					0,
					-1,
					g_ConfigParam.ulWepAuthentication)//enable WEP
				)
				diag_printf("set wepset k128\n");
			break;
		case WEP_SET_ASC:
			SetWlanWepKey(g_apcNetworkInterface[1],
					NULL,
					-1,
					-1,
					g_ConfigParam.ulWepAuthentication);	//Disable WEP key
			SetWlanESSID(g_apcNetworkInterface[1], (const char *)g_ConfigParam.acWlanESSID);
			DownInterface("wlan0");
			if (SetWlanWepKey(g_apcNetworkInterface[1],
							g_ConfigParam.acWepAsc,
							strlen((char *)g_ConfigParam.acWepAsc),
							-1,
							g_ConfigParam.ulWepAuthentication)//SET WEP key
				)
				diag_printf("set wep set ascii\n");
			break;
		}
		if (!SetWlanESSID(g_apcNetworkInterface[1],
			(const char *)g_ConfigParam.acWlanESSID))
		{
			fprintf(stderr, "Can not set wireless ESSID.\n");
			return FALSE;
		}	
		else
			return TRUE;
#endif	
}

BOOL DownInterface(const char *pcInterface)
{
	struct ifreq ifr;
	int ret;
	int fd;

	if (pcInterface == NULL)
	{
		fprintf(stderr, "illegal call function DownInterface!\n");
		return FALSE;
	}
#ifndef WLAN
	if ((fd = socket(AF_INET,SOCK_DGRAM,0, g_StreamServer_Buf, System_BUFFER_LEN)) < 0) 
		return FALSE;
#else
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) 
		return FALSE;
#endif
	memset((void*)&ifr, 0, sizeof(ifr));
//	strcpy(ifr.ifr_name, pcInterface);
	snprintf(ifr.ifr_name,sizeof(ifr.ifr_name),pcInterface);

#ifdef WLAN

    if (ioctl(fd, SIOCGIFFLAGS, &ifr)) {
        diag_printf("SIOCSIFFLAGS error %x\n", errno);
        close(fd);
        return false;
    }
    ifr.ifr_flags &= ~IFF_UP;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr)) {
        diag_printf("SIOCSIFFLAGS error %x\n", errno);
        close(fd);
        return false;
    }
    ret = ioctl(fd, SIOCGIFADDR, &ifr);
    if(ret < 0 && (errno != EADDRNOTAVAIL))
    {
        perror("SIOCIFADDR");
    }
    
    ret = ioctl(fd, SIOCDIFADDR, &ifr);
    if(ret < 0 && (errno != EADDRNOTAVAIL))
    {
        perror("SIOCIFADDR");
    }    
    
	ifr.ifr_flags = IFF_UP | IFF_BROADCAST | IFF_RUNNING;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr)) 
    {
        diag_printf("SIOCSIFFLAGS error %x\n", errno);
        close(fd);
        return false;
    }    
 #endif
 	close(fd);
	return TRUE;
}


unsigned long GetRandomIP()
{
	static int ip1 = 0;
	static int ip2 = 0;
	if (ip1 == 0 && ip2 == 0)
	{
		ip1 = rand() % 254;
		ip2 = rand() % 253 + 1;
	}
	
	/* 169.254.xxx.xxx */
	return (ip2 << 24) + (ip1 << 16) + (254 << 8) + 169;
}

void SetRandomIP(const char *pcInterface)
{
	struct bootp wlan_bootp_data;
	int j = 0;
	char ipadd[16];
	char netmask[16] = "255.255.0.0";
	char gateway[16];
	char broadcast[16] = "169.254.255.255";
	
	unsigned long ulIP = GetRandomIP();
	httpIP2String(ulIP, ipadd);
	strcpy(gateway, ipadd);

	DownInterface(pcInterface);
	diag_printf("IP is %s, NetMask is %s, Gateway is %s,Broadcast is %s\n",ipadd,netmask,gateway,broadcast);
	build_bootp_record(&wlan_bootp_data, pcInterface,
                           ipadd, netmask,broadcast,gateway,gateway);
	show_bootp(pcInterface, &wlan_bootp_data);

	if (!init_net(pcInterface, &wlan_bootp_data)) 
	{
		diag_printf("failed for %s\n",pcInterface);
	}
	for(;j<5;j++)
		init_loopback_interface(0);		
}


void SetNullIP(const char *pcInterface)
{
	struct bootp wlan_bootp_data;
	int j = 0;
	char ipadd[16];
	char netmask[16] = "0.0.0.0";
	char gateway[16];
	char broadcast[16] = "255.255.255.255";
	
	unsigned long ulIP = 0;
	httpIP2String(ulIP, ipadd);
	strcpy(gateway, ipadd);

	DownInterface(pcInterface);
	diag_printf("IP is %s, NetMask is %s, Gateway is %s,Broadcast is %s\n",ipadd,netmask,gateway,broadcast);
	build_bootp_record(&wlan_bootp_data, pcInterface,
                           ipadd, netmask,broadcast,gateway,gateway);
	show_bootp(pcInterface, &wlan_bootp_data);

	if (!init_net(pcInterface, &wlan_bootp_data)) 
	{
		diag_printf("failed for %s\n",pcInterface);
	}
	for(;j<5;j++)
		init_loopback_interface(0);		
}

void OnSetIP()
{
	diag_printf("Set evernt after IP changed\n");
	DDNS_Update();
				
				
	/* PPP server */
	if (g_ConfigParam.bPPPEnable)
	{
		int i;
		char *apcPPPServer[sizeof(g_ConfigParam.aacPPPServer)/sizeof(g_ConfigParam.aacPPPServer[0])];
		for (i = 0; i < sizeof(apcPPPServer)/sizeof(apcPPPServer[0]); ++i)
			apcPPPServer[i] = g_ConfigParam.aacPPPServer[i];
		ppot_connect(apcPPPServer,
			g_ConfigParam.aiPPPPort,
			sizeof(apcPPPServer)/sizeof(apcPPPServer[0]),
			g_ConfigParam.acPPPUser,
			g_ConfigParam.acPPPPass);
	}
}


BOOL SetIP(enum SET_IP_STATE_E *pIPState)
{
		enum SET_IP_STATE_E ipState;
		BOOL bOK = TRUE;
		int i;
		BOOL bEnable[2];
		
		if (pIPState == NULL)
			pIPState = &ipState;
		
		bEnable[0] = g_ConfigParam.abAsNetEnable[0];
		bEnable[1] = g_ConfigParam.abAsNetEnable[1];
#ifndef WLAN
		if (!bEnable[0] && !bEnable[1]) bEnable[0] = TRUE;
#else
		if (!bEnable[0] && !bEnable[1]) bEnable[1] = TRUE;
#endif
		for (i = sizeof(g_apcNetworkInterface) / sizeof(const char *) - 1; i >= 0; i--)
		{
			const char *pcInterface;
			if (!bEnable[i]) continue;
			pcInterface = g_apcNetworkInterface[i];
			
			if (g_ConfigParam.aucIPAssignedWay[i] == IP_ASSIGNMENT_MANUALLY)
			{	
				struct bootp wlan_bootp_data;
    			int j = 0;
				char ipadd[16];
				char netmask[16];
				char gateway[16];
				char broadcast[16];
				httpIP2String(g_ConfigParam.ulAsIPAddress[i],ipadd);
				httpIP2String(g_ConfigParam.ulAsNetmask[i],netmask);
				httpIP2String(g_ConfigParam.ulAsGateway[i],gateway);
				httpIP2String((~g_ConfigParam.ulAsNetmask[i])|g_ConfigParam.ulAsIPAddress[i],broadcast);
				
				*pIPState = SET_IP__STATIC_IP_TRYING;
				DownInterface(pcInterface);
				
				diag_printf("IP is %s, NetMask is %s, Gateway is %s,Broadcast is %s\n",ipadd,netmask,gateway,broadcast);
       			build_bootp_record(&wlan_bootp_data, pcInterface,
                           ipadd, netmask,broadcast,gateway,gateway);
    			show_bootp(pcInterface, &wlan_bootp_data);

    			if (!init_net(pcInterface, &wlan_bootp_data)) 
    			{
    				*pIPState = SET_IP__STATIC_IP_FAILED;
    				diag_printf("failed for %s\n",pcInterface);
    			}
    			else
	   				*pIPState = SET_IP__STATIC_IP_OK;
				/* Set DNS server */
				if (g_ConfigParam.aulAsDNS[0] != 0)
				{
					struct in_addr dns_server;
					dns_server.s_addr = (g_ConfigParam.aulAsDNS[0]);
					cyg_dns_res_init(&dns_server);
				}

    			for(;j<5;j++)
					init_loopback_interface(0);
				
				
			}
			else         
			{	
#ifndef WLAN
					//WRRunDHCPClient(pcInterface, TRUE);
#else				
					int j = 0;
					
					*pIPState = SET_IP__DHCP_TRYING;
					DownInterface(pcInterface);
					
					diag_printf("Set IP by dhcp\n");
					g_WebCamState.bDHCP_Finished = FALSE;
					init_all_network_interfaces();	
					for(;j<5;j++)
						init_loopback_interface(0);
					
					if (wlan0_dhcpstate != DHCPSTATE_BOOTP_FALLBACK
						&& wlan0_dhcpstate != DHCPSTATE_BOUND)
					{
						bOK = FALSE;
						SetRandomIP("wlan0");
						*pIPState = SET_IP__DHCP_FAILED;
					}
					else
						*pIPState = SET_IP__DHCP_OK;

					g_WebCamState.bDHCP_Finished = TRUE;
						
#endif
			}
		}
		/*display the ip address*/
		{
			unsigned long ulIP;
			
			
			GetPubIPInfo(&ulIP, NULL, NULL, NULL);
			diag_printf("******************ip Address====%x\n", ulIP); 
			if (bOK)
			{
				if (g_ConfigParam.ucWlanOperationMode == 1)	/* Disable suspend in Ad-hoc mode */
					g_netIsSuspendAllowed = FALSE;
				else
					g_netIsSuspendAllowed = TRUE;
			}

			diag_printf("bOK=%d\n", bOK);
			if (!bOK)
				return FALSE;
			else
				OnSetIP();
		}		

	return TRUE;
}
#if 0
BOOL GetWlanStatus()
{
	int fd;
	struct iwreq wrq;
	BOOL rt = TRUE;
	

	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;

	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, "wlan0");
	
	if(ioctl(fd, SIOCGIWCONNECTSTAT, &wrq) >= 0)
	{
		if(wrq.u.param.flags == 0)
		{
			diag_printf("***************WiFi not connected********** \n");
			rt = FALSE;
		}	
	}	
		
	close(fd);
	return rt;
	
}
#endif

int Config_SetMac(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
    const char *Mac;
    char *tmp,MacAddress[6];
    int i = 0;
	request *req = (request*)hConnection;
	
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
	case CA_CONFIG:
    	if (httpIsExistParam(pParamList, "mac"))
		{
			Mac = httpGetString(pParamList, "mac");
			diag_printf("set mac address is %s\n",Mac);
			MacAddress[0] = '\0';
			while(((tmp = strchr(Mac,':')) != NULL) && (i <5))
			{
				*tmp = '\0';
				//MacAddress[i] = atoi(Mac);
				MacAddress[i] = strtol(Mac,NULL,16);
				diag_printf("%02x-",MacAddress[i]);
				Mac = tmp+1;
				i++;
			}			
			MacAddress[i] = strtol(Mac,NULL,16);
			diag_printf("%02x-",MacAddress[i]);
			//diag_printf("mac address is %s\n",MacAddress);
        }
		if ((memcmp((char*)g_ConfigParam.acMAC[1],MacAddress,sizeof(MacAddress)) != 0))
		{
			ictlSetMac((ICTL_HANDLE_T*)&req->httpPrivilege,MacAddress);
			RebootOnConnectionOver(hConnection);
		}		
		return 0;
		break;
	}
	return -1;
}

int Config_GetMac(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	char ac[20];
	int i;
	request *req = (request*)hConnection;
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_USER;
	case CA_CONFIG:
		ictlGetMac((ICTL_HANDLE_T*)&req->httpPrivilege,ac,sizeof(ac));
		for (i = 5; i >= 0; --i)
		{
			httpChar2Hex(ac[i], &ac[i*3+0]);
			if (i != 5)
				ac[i*3+2] = ':';
			else
				ac[i*3+2] = '\0';
		}
		AddHttpValue(pReturnXML, G_PC_MAC, ac);		

		return 0;
	}
	return -1;
}



int Config_SetUPnP(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	BOOL bEnableUPnP;
	unsigned short usUPnPPort;

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
	case CA_CONFIG:
		if (httpIsExistParam(pParamList, "Enable"))
			bEnableUPnP = httpGetBool(pParamList, "Enable");
		else
			bEnableUPnP = g_ConfigParam.bEnableUPnP;

		if (httpIsExistParam(pParamList, "Port"))
			usUPnPPort = httpGetLong(pParamList, "Port");
		else
			usUPnPPort = g_ConfigParam.usUPnPPort;


		if (bEnableUPnP != g_ConfigParam.bEnableUPnP
			|| usUPnPPort != g_ConfigParam.usUPnPPort)
		{
			tt_rmutex_lock(&g_rmutex);
			g_ConfigParam.bEnableUPnP = bEnableUPnP;
			g_ConfigParam.usUPnPPort = usUPnPPort;
			tt_rmutex_unlock(&g_rmutex);
			
			WriteFlashMemory(&g_ConfigParam);
			RebootOnConnectionOver(hConnection);
		}

		return 0;
		break;
	}
	return -1;
}

int Config_GetUPnP(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	char acIP[32];
	unsigned short ausPort[5];
	int iSuccessNum;
	
	int i;
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_USER;
	case CA_CONFIG:
		iSuccessNum = upnp_get_port(0, ausPort);
		//AddHttpNum(pReturnXML, "Rt0", iSuccessNum);
		
		if (iSuccessNum < 0)
			iSuccessNum = 0;
		for (i = iSuccessNum; i < sizeof(ausPort) / sizeof(ausPort[0]); ++i)
			ausPort[i] = 0;
		
		if ((iSuccessNum = upnp_get_extip(0, acIP)) < 0)
			acIP[0] = 0;
		
		//AddHttpNum(pReturnXML, "Rt1", iSuccessNum);

		tt_rmutex_lock(&g_rmutex);		
		AddHttpNum(pReturnXML, "Enable", g_ConfigParam.bEnableUPnP);
		AddHttpNum(pReturnXML, "Port", (int) (g_ConfigParam.usUPnPPort == 0 ? DEFAULT_UPNP_FIRST_EXT_PORT : g_ConfigParam.usUPnPPort));
		tt_rmutex_unlock(&g_rmutex);
		
		AddHttpValue(pReturnXML, "IP", acIP);
		AddHttpNum(pReturnXML, "HTTP", (int)ausPort[0]);
		AddHttpNum(pReturnXML, "RTSP_TCP", (int)ausPort[1]);
		AddHttpNum(pReturnXML, "RTSP_UDP", (int)ausPort[2]);

		return 0;
	}
	return -1;
}


int Config_SetPPP(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	BOOL bConfigChanged = FALSE;
	BOOL bPPPEnable;
	const char *apcPPPServer[ sizeof(g_ConfigParam.aacPPPServer) / sizeof(g_ConfigParam.aacPPPServer[0])];
	int aiPPPPort[ sizeof(g_ConfigParam.aiPPPPort) / sizeof(g_ConfigParam.aiPPPPort[0])];
	const char *pcPPPUser;
	const char *pcPPPPass;
	int i;

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
	case CA_CONFIG:
		if (httpIsExistParam(pParamList, "Enable"))
			bPPPEnable = httpGetBool(pParamList, "Enable");
		else
			bPPPEnable = g_ConfigParam.bPPPEnable;
		
		if (bPPPEnable != g_ConfigParam.bPPPEnable)
			bConfigChanged = TRUE;



		for (i = 0; i < sizeof(g_ConfigParam.aacPPPServer) / sizeof(g_ConfigParam.aacPPPServer[0]); ++i)
		{
			char acName[32];
			snprintf(acName, sizeof(acName), "Server%d", i);
			
			apcPPPServer[i] = g_ConfigParam.aacPPPServer[i];
			if (httpIsExistParam(pParamList, acName))
			{
				apcPPPServer[i] = httpGetString(pParamList, acName);
				if (strcmp(apcPPPServer[i], g_ConfigParam.aacPPPServer[i]) != 0)
					bConfigChanged = TRUE;
			}
		}
		if (httpIsExistParam(pParamList, "Server"))
		{
			apcPPPServer[0] = httpGetString(pParamList, "Server");
			if (strcmp(apcPPPServer[0], g_ConfigParam.aacPPPServer[0]) != 0)
				bConfigChanged = TRUE;
		}
			

		for (i = 0; i < sizeof(g_ConfigParam.aiPPPPort) / sizeof(g_ConfigParam.aiPPPPort[0]); ++i)
		{
			char acName[32];
			snprintf(acName, sizeof(acName), "Port%d", i);
			aiPPPPort[i] = g_ConfigParam.aiPPPPort[i];
			if (httpIsExistParam(pParamList, acName))
			{
				aiPPPPort[i] = httpGetLong(pParamList, acName);
				if (aiPPPPort[i] != g_ConfigParam.aiPPPPort[i])
					bConfigChanged = TRUE;
			}
		}
		if (httpIsExistParam(pParamList, "Port"))
		{
			aiPPPPort[0] = httpGetLong(pParamList, "Port");
			if (aiPPPPort[0] != g_ConfigParam.aiPPPPort[0])
				bConfigChanged = TRUE;
		}
			

		if (httpIsExistParam(pParamList, "User"))
			pcPPPUser = httpGetString(pParamList, "User");
		else
			pcPPPUser = g_ConfigParam.acPPPUser;
		if (strcmp(pcPPPUser, g_ConfigParam.acPPPUser) != 0)
			bConfigChanged = TRUE;
		

		if (httpIsExistParam(pParamList, "Pass"))
			pcPPPPass = httpGetString(pParamList, "Pass");
		else
			pcPPPPass = g_ConfigParam.acPPPPass;
		if (strcmp(pcPPPPass, g_ConfigParam.acPPPPass) != 0)
			bConfigChanged = TRUE;

	
		if (bConfigChanged)
		{
			tt_rmutex_lock(&g_rmutex);
			g_ConfigParam.bPPPEnable = bPPPEnable;
			
			for (i = 0; i < sizeof(g_ConfigParam.aacPPPServer) / sizeof(g_ConfigParam.aacPPPServer[0]); ++i)
				httpMyStrncpy(g_ConfigParam.aacPPPServer[i], apcPPPServer[i], sizeof(g_ConfigParam.aacPPPServer[i]));
			for (i = 0; i < sizeof(g_ConfigParam.aiPPPPort) / sizeof(g_ConfigParam.aiPPPPort[0]); ++i)
				g_ConfigParam.aiPPPPort[i] = aiPPPPort[i];
			httpMyStrncpy(g_ConfigParam.acPPPUser, pcPPPUser, sizeof(g_ConfigParam.acPPPUser));
			httpMyStrncpy(g_ConfigParam.acPPPPass, pcPPPPass, sizeof(g_ConfigParam.acPPPPass));
			tt_rmutex_unlock(&g_rmutex);
			
			WriteFlashMemory(&g_ConfigParam);
		}
		
		if (!bPPPEnable)
		{
			/* Disconnect if possible */
			//if (ppot_is_connecting())
			ppot_disconnect();
		}
		
		if(bPPPEnable && bConfigChanged) //&& !ppot_is_connecting())
		{
			ppot_connect(apcPPPServer, aiPPPPort,
				sizeof(apcPPPServer) / sizeof(apcPPPServer[0]),
				pcPPPUser, pcPPPPass);
		}

		return 0;
		break;
	}
	return -1;
}



int Config_GetPPP(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	int i;
	
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_USER;
	case CA_CONFIG:
		tt_rmutex_lock(&g_rmutex);		
		AddHttpNum(pReturnXML, "Enable", g_ConfigParam.bPPPEnable);
		for (i = 0; i < sizeof(g_ConfigParam.aacPPPServer) / sizeof(g_ConfigParam.aacPPPServer[0]); ++i)
		{
			char acName[32];
			snprintf(acName, sizeof(acName), "Server%d", i);
			AddHttpValue(pReturnXML, acName, g_ConfigParam.aacPPPServer[i]);
		}
		for (i = 0; i < sizeof(g_ConfigParam.aiPPPPort) / sizeof(g_ConfigParam.aiPPPPort[0]); ++i)
		{
			char acName[32];
			snprintf(acName, sizeof(acName), "Port%d", i);
			AddHttpNum(pReturnXML, acName, g_ConfigParam.aiPPPPort[i]);
		}
		AddHttpValue(pReturnXML, "User", g_ConfigParam.acPPPUser);
		AddHttpValue(pReturnXML, "Pass", g_ConfigParam.acPPPPass);
		AddHttpValue(pReturnXML, "Status", (ppot_is_online() ? "online" : "offline"));
		tt_rmutex_unlock(&g_rmutex);
		return 0;
	}
	return -1;
}

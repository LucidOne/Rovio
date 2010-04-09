#include "CommonDef.h"
#include "memmgmt.h"
#include "702clk.h"

#if defined IPCAM_CONFIG_IP_CAM_VER_1 || defined IPCAM_CONFIG_IP_CAM_VER_2 || defined IPCAM_CONFIG_IP_CAM_VER_3
#include "ns_api.h"
#elif defined IPCAM_CONFIG_IP_CAM_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_1
#else
#	error "No hardware config defined!"
#endif

#if 0
cyg_thread g_ptdHTTP;	/* pthread_t for CameraThread */
cyg_handle_t ptdHTTP_handle;
#pragma arm section zidata = "non_init"
unsigned char ptdHTTP_stack[STACKSIZE];
#pragma arm section zidata
#endif

int tsUartTestHandler(ICTL_HANDLE_T* ctrlHandle,char ** valueList, int paraNum, char *pcResponse,size_t szMaxResponse);

/* define custom command handler */
int fireHandler(ICTL_HANDLE_T* ctrlHandle,char ** valueList, int paraNum, char *pcResponse,size_t szMaxResponse)
{
	diag_printf("Receive fire command\n");
	diag_printf("Direction : %s\n", valueList[0] );
	diag_printf("strength: %s\n", valueList[1]);
	//return the reply
	strcpy(pcResponse, "fire successfully");
	return 1;
}
int MCUCmdHandler(ICTL_HANDLE_T* ctrlHandle,char ** valueList, int paraNum, char *pcResponse,size_t szMaxResponse) 
{
	// call function
	int ret;
	ret = ictlCtrlMCU(ctrlHandle,valueList[0],pcResponse,szMaxResponse);	
	diag_printf("call ictlCtrlMCU():parameters = [%s]\n", valueList[0]);
	//pcResponse[0] = 0;
	return 1;	
	
}
int tsOnboardTestHandler(ICTL_HANDLE_T* ctrlHandle,char ** valueList, int paraNum, char *pcResponse,size_t szMaxResponse) 
{
#if 1
    
    int nLedErrorCode = 0;
    int nLedStatus = 0;
    
    ledGetResult( &nLedStatus, &nLedErrorCode );
    
    sprintf( pcResponse, "(Status=0x%02x, Error=0x%02x)", nLedStatus, nLedErrorCode );   

	return -1;

#else

    extern int tsEzTestHandler(ICTL_HANDLE_T* ctrlHandle,char ** valueList, int paraNum, char *pcResponse,size_t szMaxResponse); 
    return tsEzTestHandler(ctrlHandle, valueList, paraNum, pcResponse, szMaxResponse); 
	    
#endif
} 


int tsAudioLoopbackTestHandler(ICTL_HANDLE_T* ctrlHandle,char ** valueList, int paraNum, char *pcResponse,size_t szMaxResponse) 
{
    if( strcmp( valueList[0], "start" )== 0 )
    {
		audioSetPlayVolume(20, 20);
		audioBypassEnable();
    }
    else
    if( strcmp( valueList[0], "end" )== 0 )
    {
		audioSetPlayVolume(g_ConfigParam.ulSpeakerVolume, g_ConfigParam.ulSpeakerVolume);
		audioBypassDisable();
    }
    else
    {
	    strcpy(pcResponse, "FAIL");
	    return -1;	
    }    
    
	strcpy(pcResponse, "OK");
	return -1;    
} 


static PRD_TASK_T g_prdtsk_HS;

static void test_HS(void *pArg)
{
	prdDelTask(&g_prdtsk_HS);
	pwr_power_saving();
}


int tsSuspendTestHandler(ICTL_HANDLE_T* ctrlHandle,char ** valueList, int paraNum, char *pcResponse,size_t szMaxResponse) 
{
	if( strcmp( valueList[0], "suspend" )== 0 )
	{
		prdAddTask(&g_prdtsk_HS, &test_HS, 50 * 3, (void *)88);
	}
	else
	{
		strcpy(pcResponse, "FAIL");
		return -1; 
	}    

	strcpy(pcResponse, "OK");
	return -1;    
} 
#if 0
void initLCM(void)
{
	ControlBackLight(6, 0);
	wb702ConfigureLCM(DISPLAYINFOONLCD_MAXWIDTH, DISPLAYINFOONLCD_MAXHEIGHT, 2, 1, 3, 0, TRUE, 1);

	ControlLcmTvPower(2);
	ControlBackLight(6, 1);	
}
#endif

//cplu++start

#define _FOR_EMI_TEST
#ifdef _FOR_EMI_TEST
bool set_ip_address(char *pcInterface, unsigned long ulIP, unsigned long ulNetmask, char* GateWay)
{
	struct ifreq ifr;
	int fd;
	struct sockaddr_in *pAddr;
	int ret;

	if (pcInterface == NULL)
	{
		fprintf(stderr, "illegal call function SetGeneralIP!\n");
		return false;
	}

	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return false;
	
	memset((void*)&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, pcInterface);
    
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
    
    memset((void*)&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, pcInterface);
    ret = ioctl(fd, SIOCGIFADDR, &ifr);
    if(ret < 0 && (errno != EADDRNOTAVAIL))
    {
        diag_printf("SIOCIFADDR error %x\n", errno);
        close(fd);
        return false;
    }
    
    ret = ioctl(fd, SIOCDIFADDR, &ifr);
    if(ret < 0 && (errno != EADDRNOTAVAIL))
    {
        diag_printf("SIOCIFADDR error %x\n", errno);
        close(fd);
        return false;
    }
	

	ifr.ifr_flags = IFF_UP | IFF_BROADCAST | IFF_RUNNING;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr)) {
        diag_printf("SIOCSIFFLAGS error %x\n", errno);
        close(fd);
        return false;
    }

	pAddr = (struct sockaddr_in *)&(ifr.ifr_addr);
	bzero(pAddr, sizeof(struct sockaddr_in));
	pAddr->sin_addr.s_addr = ulIP;
	pAddr->sin_family = AF_INET;
	pAddr->sin_len = sizeof(*pAddr);
	if (ioctl(fd, SIOCSIFADDR, &ifr) < 0)
	{
		fprintf(stderr,"Set Ip Address error\n");
		close(fd);
		return false;
	}

	pAddr = (struct sockaddr_in *)&(ifr.ifr_addr);
	bzero(pAddr, sizeof(struct sockaddr_in));
	pAddr->sin_addr.s_addr = ulNetmask;
	pAddr->sin_family = AF_INET;
	pAddr->sin_len = sizeof(*pAddr);
	if (ioctl(fd, SIOCSIFNETMASK, &ifr) < 0)
	{
		fprintf(stderr,"Set netmask error\n");
		close(fd);
		return false;
	}

	SetGateway(pcInterface, inet_addr(GateWay));
	
	close(fd);
	
	return true;
}

static UINT32 cnt=0;
//static char *TestBuffer = "eCos Wlan0 Test00000";
static char TestBuffer[1514];// = "eCos Wlan0 Test00000"; 1514:HP
static void  do_udp_test()
{
    int i, s1;
    struct sockaddr_in test_chan_slave, test_chan_master;
    struct hostent *host;
    unsigned int start, end;
	int SendCount, SendCountBack, first = 1;
	long long result;
	struct in_addr server;

//	diag_printf("******do_udp_test00\n");
	
//	cyg_thread_delay(300);
	
    s1 = socket(AF_INET, SOCK_DGRAM, 0);
    if (s1 < 0) 
    {
        diag_printf("datagram socket error\n");
        return;
    }
	if (fcntl(s1, F_SETFL, O_NONBLOCK) <0 ) 
	{
//		diag_printf("Set to non-block fail\n");
	}
	
    memset((char *) &test_chan_master, 0, sizeof(test_chan_master));
    test_chan_master.sin_family = AF_INET;
    test_chan_master.sin_len = sizeof(test_chan_master);
    test_chan_master.sin_addr.s_addr = htonl(INADDR_ANY);
    test_chan_master.sin_port = htons(1011);
 
    if (bind(s1, (struct sockaddr *) &test_chan_master, sizeof(test_chan_master)) < 0) 
    {
        diag_printf("bind error\n");
        close(s1);
        while(1);
        return;
    }

	//SendCount = SendCountBack = 128; //2048*8;
	SendCount = SendCountBack = 1; //2048*8;
	
	test_chan_slave.sin_family = AF_INET;
    test_chan_slave.sin_len = sizeof(test_chan_slave);
    test_chan_slave.sin_port = htons(1012);

#if 0
	inet_aton("192.168.30.1", &server);
    cyg_dns_res_init(&server);
    host = gethostbyname("192.168.30.1");
    //host = gethostbyname("127.0.0.1");
    if (host == (struct hostent *)NULL) 
    {
        diag_printf("gethostbyname error\n");
        return;
    }
    else
    {
    	diag_printf("gethostbyname success: %s\n", host->h_addr_list[0]);
    }
    
	memcpy((void*)&test_chan_slave.sin_addr.s_addr, host->h_addr, host->h_length);
#else
	test_chan_slave.sin_addr.s_addr = inet_addr("192.168.30.1");
#endif

	//cyg_thread_delay(300);
//	BreakTest = 0;
	
//	diag_printf("start test\n");
	
//    while(1)
	for (i = 0;  i < SendCount;  i++) 
    {
    	int res;
//    	sprintf(&TestBuffer[strlen(TestBuffer)-5], "%05d", i);
        res = sendto(s1, TestBuffer, strlen(TestBuffer), 0, 
                             (struct sockaddr *)&test_chan_slave, sizeof(test_chan_slave));
        if (res != strlen(TestBuffer)) 
        {
        	diag_printf("sendto error %x, errno = %d\n", res, errno);
            close(s1);	
            if(i > 20)
          		while(1);
        }
      	else
        {
          	if (first)
           	{
           		first = 0;
          		start = cyg_current_time();
          	}
        }
    }
   
    close(s1);
    
#if 0
    end = cyg_current_time();
   
    result = (SendCountBack * sizeof(TestBuffer));
    result = result / (end-start);
    result *= 100;
    

    if(cnt%100 == 0)	//delay 10s  
    {
    	diag_printf("send success %d times %d bytes for each; %dKB/s %dMb/s\n", SendCountBack, strlen(TestBuffer), 
				(int)(result/1024),
				(int)((result * 8)/1024/1024));
	}
#endif	
	cnt++;	
}

int SetScan()
{
	int rt;
	int fd;
	struct iwreq wrq;
	struct iw_event *iwe;
	int length;
	char *p;
	

	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;

	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, "wlan0");
	
	rt = ioctl(fd, SIOCSIWSCAN, &wrq);
	
	if(rt < 0)
	{
		diag_printf("set scan fail %x\n", errno);
	}
	close(fd);
	
	//return 0;
	return rt;
}

BOOL SetWiFiDataRate(UINT32 inRate)
{
	int ret = FALSE;
	ret = wlanSetBitrate("wlan0", inRate);	
	if(!ret)
	{
		diag_printf("SetWiFiDataRate Failed.\n");
	}
	diag_printf("SetWiFiDataRate OK.\n");
	
	return ret;
}

static void  do_wireless_test()
{
	struct in_addr server;
	int ret = FALSE;
	int iDataRate;

/*
	SetWlanOperationMode("wlan0", 1);//AD-HOC
	SetWlanChannel("wlan0", 7);
	SetWlanESSID("wlan0", "IPCAM_ADHOC");

	ret = wlanSetBitrate("wlan0", 11000000);	//Data Rate
	if(!ret)
	{
		diag_printf("wlanSetBitrate Failed.\n");
		while(1);
	}
	ret = wlanGetBitrate("wlan0", &iDataRate);
	if(!ret)
	{
		diag_printf("wlanGetBitrate 01 Failed.\n");
		while(1);
	}
	diag_printf("wlanGetBitrate = %d.\n", iDataRate);
	cyg_thread_delay(50);
	ret = SetADHocGRate("wlan0", TRUE);
	if(!ret)
	{
		diag_printf("SetADHocGRate Failed.\n");
		while(1);
	}
	ret = wlanSetBitrate("wlan0", 6000000);	//Data Rate
	if(!ret)
	{
		diag_printf("wlanSetBitrate Failed.\n");
		while(1);
	}
	cyg_thread_delay(50);
	ret = wlanGetBitrate("wlan0", &iDataRate);
	if(!ret)
	{
		diag_printf("wlanGetBitrate 02 Failed.\n");
		while(1);
	}
	diag_printf("wlanGetBitrate = %d.\n", iDataRate);
	cyg_thread_delay(200);
*/

	diag_printf("Start to SetWlanESSID.\n");
	while(1)
	{	
		SetScan();
		cyg_thread_delay(100);
		ret = SetWlanESSID("wlan0", "EMI_TEST");
		if(ret)
			break;
		cyg_thread_delay(100);
	}
	
	diag_printf("Start to set_ip_address.\n");
	while(1)
	{
		ret = set_ip_address("wlan0", inet_addr("192.168.30.100"),inet_addr("255.255.255.0"),"192.168.30.1");
		if(ret)
			break;
		cyg_thread_delay(100);
	}

	ret = wlanGetBitrate("wlan0", &iDataRate);
	if(!ret)
	{
		diag_printf("wlanGetBitrate 02 Failed.\n");
		while(1);
	}
	diag_printf("wlanGetBitrate = %d.\n", iDataRate);
	
	inet_aton("192.168.30.1", &server);
	cyg_dns_res_init(&server);
	cyg_thread_delay(100);
	
	memset((char*)TestBuffer, 0xFF, sizeof(TestBuffer));
	diag_printf("Start to UDP test\n");	
	
	while(1)	
		do_udp_test();
}


static cyg_handle_t WiFi_handle = NULL;
static cyg_thread  WiFi_thread;
UINT32 WiFi_thread_stack[8192*2];
void test_WiFi_system(cyg_addrword_t pParam)
{
	while(1)
	{
		tt_msleep(2000);
		do_wireless_test();	
		break;	
	}
}
void test_WiFiThread()
{
	cyg_thread_create(CYGNUM_LIBC_MAIN_THREAD_PRIORITY, &test_WiFi_system, (cyg_addrword_t)NULL, "Test WiFi THread", WiFi_thread_stack,
	        sizeof(WiFi_thread_stack), &WiFi_handle, &WiFi_thread);			
	cyg_thread_resume(WiFi_handle);      
}                
int tsWiFiEMI(ICTL_HANDLE_T* ctrlHandle,char ** valueList, int paraNum, char *pcResponse,size_t szMaxResponse) 
{
	BOOL bRet = FALSE;
	if( strcmp( valueList[0], "auto" ) == 0 )
	{
		bRet = TRUE;
		if(bRet)
			strcpy(pcResponse, "Auto DateRate");
	}else
	if( strcmp( valueList[0], "1000000" ) == 0 )
	{
		bRet = SetWiFiDataRate(1000000);
		if(bRet)
			strcpy(pcResponse, "DateRate 1Mbps.");
	}else
	if( strcmp( valueList[0], "2000000" ) == 0 )
	{
		bRet = SetWiFiDataRate(2000000);
		if(bRet)
			strcpy(pcResponse, "DateRate 2Mbps.");
	}else
	if( strcmp( valueList[0], "5500000" ) == 0 )
	{
		bRet = SetWiFiDataRate(5500000);
		if(bRet)
			strcpy(pcResponse, "DateRate 5.5Mbps.");
	}else
	if( strcmp( valueList[0], "11000000" ) == 0 )
	{
		bRet = SetWiFiDataRate(11000000);
		if(bRet)
			strcpy(pcResponse, "DateRate 11Mbps.");
	}else
	if(  strcmp( valueList[0], "6000000" ) == 0 )
	{
		bRet = SetWiFiDataRate(6000000);
		if(bRet)
			strcpy(pcResponse, "DateRate 6Mbps.");
	}else
	if(  strcmp( valueList[0], "9000000" ) == 0 )
	{
		bRet = SetWiFiDataRate(9000000);
		if(bRet)
			strcpy(pcResponse, "DateRate 9Mbps.");
	}else
	if(  strcmp( valueList[0], "12000000" ) == 0 )
	{
		bRet = SetWiFiDataRate(12000000);
		if(bRet)
			strcpy(pcResponse, "DateRate 12Mbps.");
	}else
	if(  strcmp( valueList[0], "18000000" ) == 0 )
	{
		bRet = SetWiFiDataRate(18000000);
		if(bRet)
			strcpy(pcResponse, "DateRate 18Mbps.");
	}else
	if(  strcmp( valueList[0], "24000000" ) == 0 )
	{
		bRet = SetWiFiDataRate(24000000);
		if(bRet)
			strcpy(pcResponse, "DateRate 24Mbps.");
	}else
	if(  strcmp( valueList[0], "36000000" ) == 0 )
	{
		bRet = SetWiFiDataRate(36000000);
		if(bRet)
			strcpy(pcResponse, "DateRate 36Mbps.");
	}else
	if(  strcmp( valueList[0], "48000000" ) == 0 )
	{
		bRet = SetWiFiDataRate(48000000);
		if(bRet)
			strcpy(pcResponse, "DateRate 48Mbps.");
	}else
	if(  strcmp( valueList[0], "54000000" ) == 0 )
	{
		bRet = SetWiFiDataRate(54000000);
		if(bRet)
			strcpy(pcResponse, "DateRate 54Mbps.");
	}else
	{
		strcpy(pcResponse, "Invalid input ... ");
		return -1;
	}
	
	if(bRet)
		test_WiFiThread();
	else
		strcpy(pcResponse, "Set DataRate Failed.");
		
	return -1;    
}
#endif	// end of _FOR_EMI_TEST

int tsGpioTestHandler(ICTL_HANDLE_T* ctrlHandle,char ** valueList, int paraNum, char *pcResponse,size_t szMaxResponse) 
{
	static UINT32 uGpiobData, uGpiobDir; // test gpiob19,26,28,30
	static UINT32 uGpioData, uGpioDir; // test gpio6,gpio10
	static BOOL bStoreGpioStatus=FALSE;


   if((bStoreGpioStatus==FALSE)&&(( strcmp( valueList[0], "0" )== 0 )||( strcmp( valueList[0], "1" )== 0)))
   {	
   	bStoreGpioStatus = TRUE;
	uGpioDir = inpw(REG_GPIO_OE);	
	uGpioData = inpw(REG_GPIO_DAT);
	uGpiobDir = inpw(REG_GPIOB_OE);	
	uGpiobData = inpw(REG_GPIOB_DAT);
	}
	
    if( strcmp( valueList[0], "0" )== 0 )
    {
    		//outpw(REG_PADC0, inpw(REG_PADC0)&~(1<<27));
    		
    		outpw(REG_GPIOB_OE, inpw(REG_GPIOB_OE)&~((1<<19)|(1<<26)|(1<<28)|(1<<30))); // set to output mode
    		outpw(REG_GPIOB_DAT, inpw(REG_GPIOB_DAT)&~((1<<19)|(1<<26)|(1<<28)|(1<<30))); // set  data to low	
    
    		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE)&~((1<<6)|(1<<10)));	//set to output mode
    		outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT)&~((1<<6)|(1<<10)));	// set data to low
   			
    }
    if( strcmp( valueList[0], "1" )== 0)
    {
    		outpw(REG_GPIOB_OE, inpw(REG_GPIOB_OE)&~((1<<19)|(1<<26)|(1<<28)|(1<<30))); // set to output mode
    		outpw(REG_GPIOB_DAT, inpw(REG_GPIOB_DAT)|((1<<19)|(1<<26)|(1<<28)|(1<<30))); // set data to high
    		
    		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE)&~((1<<6)|(1<<10)));	//set to output mode
    		outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT)|((1<<6)|(1<<10)));	// set data to HIGH
    }
    //if( strcmp( valueList[0], "restore" )== 0)
    if(( strcmp( valueList[0], "restore" )== 0)&&(bStoreGpioStatus == TRUE))
    {
		outpw(REG_GPIO_OE, uGpioDir);
		outpw(REG_GPIO_DAT, uGpioData);
		outpw(REG_GPIOB_OE, uGpiobDir);
		outpw(REG_GPIOB_DAT, uGpiobData);
		bStoreGpioStatus = FALSE;
    }
	strcpy(pcResponse, "OK");
	return -1;    
} 

#if 0
//cplu++
extern int _dsp_SensorSlaveAddr;
int tsSensorTestHandler(ICTL_HANDLE_T* ctrlHandle,char ** valueList, int paraNum, char *pcResponse,size_t szMaxResponse) 
{

    long        strtol_tmp1;
    char*       strtol_tmp2;
    UINT32 uiSensorData;
	UINT32 regSerialBusCtrl;

	_dsp_SensorSlaveAddr=0x60;
	i2cSetDeviceSlaveAddr (_dsp_SensorSlaveAddr);
	
	regSerialBusCtrl=i2cGetSerialBusCtrl ()&0x04;	//k07045-2
	if (sysCacheState ())	i2cInitSerialBus (0,0,64);		//k07045-2
	else	i2cInitSerialBus (0,0,1);	
	
    strtol_tmp1 = strtol(valueList[0], &strtol_tmp2, 16);
    i2cWriteI2C((UINT32)((strtol_tmp1&0xFFFF)>>8), (UINT32)(strtol_tmp1&0xFF));
    uiSensorData = i2cReadI2C((UINT32)((strtol_tmp1&0xFFFF)>>8));

//	sprintf( pcResponse, "0x%02x", strtol_tmp1); 	
    sprintf( pcResponse, "(A=0x%02x, D=0x%02x)", (UINT32)((strtol_tmp1&0xFFFF)>>8), uiSensorData );   

    return -1;    
} 
//cplu++end


#endif
#if 0
void HTTPThread(cyg_addrword_t pParam)
{
	if(httpInitHashMemPool() != 0)
	{
		diag_printf("http init mem pool error\n");
		return;
	}
	
	ServerStart();
}
#endif

int main(void)
{

	cyg_interrupt_disable();
	cyg_interrupt_disable();
	cyg_interrupt_disable();
	
	cyg_interrupt_enable();

 	uart_init_setting_block_mode("/dev/ser0");
 	uart_init_setting_block_mode("/dev/ser1");

	/* Init period task service. */
//tim- 	prdInit();
	
	/* Init LED */
//tim- 	ledInit();
//tim- 	ledSetNetwork(LED_NETWORK_TRY_CONNECT);
	

#if defined IPCAM_CONFIG_IP_CAM_VER_1 || defined IPCAM_CONFIG_IP_CAM_VER_2 || defined IPCAM_CONFIG_IP_CAM_VER_3
    /* Init MCU */
     mcuInit();
     prdAddTask_CheckBattery();

#elif defined IPCAM_CONFIG_IP_CAM_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_1
#else
#	error "No hardware config defined!"
#endif
		
	{
		int config_status = 0;
		while(!(config_status = IsWirelessConfigDone()));
		if (config_status != 1)
			ledError(LED_ERROR_WIFI);
	}

//tim-
/*
	VideoPhoneInit();
	fmiSetFMIReferenceClock(72000);
	fmiSetSDOutputClockbykHz(24000);
	
	if(usiMyInitDevice(144) == USI_ERR_DEVICE)
		USI_IS_READY = FALSE;
	else
		USI_IS_READY = TRUE;
	
	if(USI_IS_READY == TRUE)
		Read_FlashFile();		

	// Initialize NS navigation module
#if defined IPCAM_CONFIG_IP_CAM_VER_1 || defined IPCAM_CONFIG_IP_CAM_VER_2 || defined IPCAM_CONFIG_IP_CAM_VER_3
	er_init_ns_nav();
#elif defined IPCAM_CONFIG_IP_CAM_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_1
#else
#	error "No hardware config defined!"
#endif
*/

#ifndef WLAN
	wb702EnableFMI(TRUE);	
	FTH_Init();	
	/*open lcm*/
	initLCM();	
	DisplayInfoOnLCD_Init(DISPLAYINFOONLCD_MAXWIDTH, DISPLAYINFOONLCD_MAXHEIGHT, 1000);
	DisplayInfoOnLCD_Start();
#else
	force_net_dev_linked();
	cyg_do_net_init();

//tim-	vp_bitrate_control_init(1);

//tim-	prdAddTask_CheckSuspend();
	
//tim-	prdAddTask_CheckNetwork();
	
	prdAddTask_CheckUSB();
	
	//LED_FinishVerify();
#endif

#if 0
	cyg_thread_create(PTD_PRIORITY, &HTTPThread, NULL, "ptdHTTP", ptdHTTP_stack, STACKSIZE, &ptdHTTP_handle, &g_ptdHTTP);
	if ( ptdHTTP_handle == NULL)
	{
		fprintf(stderr, "Thread for ftp creation failed!\n");
		return 1;
	}
	cyg_thread_resume(ptdHTTP_handle);	
#else

//tim-
///*
	if(httpInitHashMemPool() != 0)
	{
		diag_printf("http init mem pool error\n");
		return;
	}
//*/	
	/* register command format and handler */
{
	char *paralist[3];
#if 0
	paralist[0] = "direction";
	paralist[1] = "strength";
	
	/* register a command "fire", 
	 * it has 2 parameters, "direction", and "strength"
	 * its handler is fireHandler()
	 */	
	registerCustomCmd("fire", paralist, 2, fireHandler);
	/* register a mcu cmd */
#endif
	paralist[0] = "parameters";
	registerCustomCmd("mcu",paralist, 1, MCUCmdHandler);
}


//PT12:20080411++
#if defined IPCAM_CONFIG_IP_CAM_VER_1 || defined IPCAM_CONFIG_IP_CAM_VER_2 || defined IPCAM_CONFIG_IP_CAM_VER_3
    {
    	char *paralist[1];
    	paralist[0] = "com";
    	registerCustomCmd("uart_test",paralist, 1, tsUartTestHandler);
    	registerCustomCmd("onboard_test",paralist, 0, tsOnboardTestHandler);
    	paralist[0] = "action";
    	registerCustomCmd("audio_test",paralist, 1, tsAudioLoopbackTestHandler);	   
        registerCustomCmd("suspend_test",paralist, 1, tsSuspendTestHandler);	//cplu++
     	registerCustomCmd("gpio_test",paralist, 1, tsGpioTestHandler); //cplu++
    	//cplu--registerCustomCmd("sensor_test",paralist, 1, tsSensorTestHandler); //cplu++ for sensor test
    }
#elif defined IPCAM_CONFIG_IP_CAM_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_1
    {
    	char *paralist[1];
    	paralist[0] = "com";
    	registerCustomCmd("uart_test",paralist, 1, tsUartTestHandler);
    	registerCustomCmd("onboard_test",paralist, 0, tsOnboardTestHandler);
    	paralist[0] = "action";
    	registerCustomCmd("audio_test",paralist, 1, tsAudioLoopbackTestHandler);	   
        registerCustomCmd("suspend_test",paralist, 1, tsSuspendTestHandler);	//cplu++
     	registerCustomCmd("gpio_test",paralist, 1, tsGpioTestHandler); //cplu++
#ifdef _FOR_EMI_TEST
     	registerCustomCmd("EMI_test",paralist, 1, tsWiFiEMI); 		   //@tim++
#endif     	
    	//cplu--registerCustomCmd("sensor_test",paralist, 1, tsSensorTestHandler); //cplu++ for sensor test
	}    	
#else
#	error "No hardware config defined!"
#endif
//PT12:20080411--

#if defined IPCAM_CONFIG_IP_CAM_VER_1 || defined IPCAM_CONFIG_IP_CAM_VER_2 || defined IPCAM_CONFIG_IP_CAM_VER_3
	er_register_ns_cgi();
#elif defined IPCAM_CONFIG_IP_CAM_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_1
#else
#	error "No hardware config defined!"
#endif
	
#ifdef _FOR_EMI_TEST
	// change it to sample;
	VirtualComInit();
	diag_printf("VcomByUSBInit successfully\n");
	return 0;
#endif

	ServerStart();
#endif
	return 0;
	
}

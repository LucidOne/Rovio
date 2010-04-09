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
//cplu++end



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


#if 1
static cyg_handle_t g_wpa_handle = NULL;
static cyg_thread  g_wpa_thread;
UINT32 g_wpa_thread_stack[32*1024 / sizeof(UINT32)];
void wpa_thread(cyg_addrword_t pParam)
{
	wpa_supplicant_main(0,0);
}


void wpa_supplicant_start()
{
	cyg_thread_create(10, &wpa_thread, (cyg_addrword_t)NULL, "wpa", g_wpa_thread_stack,
		sizeof(g_wpa_thread_stack), &g_wpa_handle, &g_wpa_thread);			
		cyg_thread_resume(g_wpa_handle);
}
void SetTestIP(const char *pcInterface)
{
	struct bootp wlan_bootp_data;
	int j = 0;

#if 0
	InitNetInterface(pcInterface);
	SetGeneralIP(pcInterface, 0x8af9820a, 0xFFFF820a );
	//10.132.249.138, 10.132.255.255
#else

#if 1
	char ipadd[16] = "10.132.249.138";
	char netmask[16] = "255.255.0.0";
	char gateway[16] = "10.132.249.1";
	char broadcast[16] = "10.132.255.255";
#else
	char ipadd[16] = "192.168.0.118";
	char netmask[16] = "255.255.255.0";
	char gateway[16] = "192.168.0.1";
	char broadcast[16] = "192.168.0.255";
#endif
	
	DownInterface(pcInterface);
	diag_printf("IP is %s, NetMask is %s, Gateway is %s,Broadcast is %s\n",ipadd,netmask,gateway,broadcast);
	build_bootp_record(&wlan_bootp_data, pcInterface,
                           ipadd, netmask,broadcast,gateway,gateway);
	show_bootp(pcInterface, &wlan_bootp_data);

	if (!init_net(pcInterface, &wlan_bootp_data)) 
	{
		diag_printf("failed for %s\n",pcInterface);
	}
	//for(;j<5;j++)
	//	init_loopback_interface(0);		
#endif
}
#else


#endif


int main(void)
{
	cyg_interrupt_disable();
	cyg_interrupt_disable();
	cyg_interrupt_disable();
	
	cyg_interrupt_enable();

	/* Set clock */
	VideoPhoneInitClock();
	fmiSetFMIReferenceClock(72000);
	fmiSetSDOutputClockbykHz(24000);
	
	/* Init uart */
	uart_init_setting_block_mode("/dev/ser0");
	uart_init_setting_block_mode("/dev/ser1");

	/* Init period task service. */
	prdInit();

	/* Init flash */
	if(usiMyInitDevice(144) == USI_ERR_DEVICE)
		USI_IS_READY = FALSE;
	else
		USI_IS_READY = TRUE;

	/* Init LED */
	ledInit();

	/* Read param */	
	ReadCameraParam();
	

	//init_loopback_interface(0);
	wpa_supplicant_start();
#if 0
	while(1)
	{
		static int i = 0;
		//InitNetInterface("wlan0");
		//SetTestIP("wlan0");
		tt_msleep(7000);
		//continue;
		
		++i;
		if(i % 8 == 4)
			wsp_set_network_managed("link1", "1234567891");
		else if (i % 8 == 0)
			wsp_set_network_adhoc("link2", "");
	}
#endif

	

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
	
	VideoPhoneInit();


	
	if(USI_IS_READY == TRUE)
		Read_FlashFile();		

	// Initialize NS navigation module
#if defined IPCAM_CONFIG_IP_CAM_VER_1 || defined IPCAM_CONFIG_IP_CAM_VER_2 || defined IPCAM_CONFIG_IP_CAM_VER_3
	er_init_ns_nav();
#elif defined IPCAM_CONFIG_IP_CAM_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_1
#else
#	error "No hardware config defined!"
#endif

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
	vp_bitrate_control_init(1);

	prdAddTask_CheckSuspend();
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

	if(httpInitHashMemPool() != 0)
	{
		diag_printf("http init mem pool error\n");
		return;
	}
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
    }
#elif defined IPCAM_CONFIG_IP_CAM_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_1
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
	
	ServerStart();
#endif
	return 0;
	
}

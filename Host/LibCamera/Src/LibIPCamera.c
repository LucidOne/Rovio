#include "CommonDef.h"

CAMERA_CONFIG_PARAM_T g_ConfigParam;
WEBCAMSTATE_P g_WebCamState;

TT_RMUTEX_T g_rmutex;
cyg_mutex_t g_ptmConfigParam;
cyg_mutex_t g_ptmState;
cyg_mutex_t g_ptmTimeSetting;
cyg_mutex_t g_ptmWebCameraLog;	/* mutex for web camera log */
#ifndef WLAN
__align (32)
CHAR g_StreamServer_Buf1[System_BUFFER_LEN];
CHAR *g_StreamServer_Buf;
#endif
FLASH_UPDATE_T g_FlashUpdateState;

//pthread_t for mctest;
cyg_thread g_ptdMctest;
cyg_handle_t ptdMctest_handle;
#pragma arm section zidata = "non_init"
unsigned char ptdMctest_stack[STACKSIZE2];
#pragma arm section zidata

//pthread_t g_ptdCamera;
cyg_thread g_ptdCamera;
MSG_QUEUE *g_pMsgCamera;
cyg_handle_t ptdCamera_handle;
#pragma arm section zidata = "non_init"
unsigned char ptdCamera_stack[STACKSIZE];
#pragma arm section zidata

#ifdef USE_FLASH
//pthread_t g_ptdFlash;
cyg_thread g_ptdFlash;
MSG_QUEUE *g_pMsgFlash;
cyg_handle_t ptdFlash_handle;
#pragma arm section zidata = "non_init"
unsigned char ptdFlash_stack[STACKSIZE];
#pragma arm section zidata
#endif
#ifdef USE_DDNS
//pthread_t g_ptdSelect;
cyg_thread g_ptdSelect;
MSG_QUEUE *g_pMsgSelect;
cyg_handle_t ptdSelect_handle;
#pragma arm section zidata = "non_init"
unsigned char ptdSelect_stack[STACKSIZE];
#pragma arm section zidata
#endif

BOOL (*g_pOnGetImage)(char *pcImgBuffer, int iImgLength) = NULL;
BOOL (*g_pExternalDetector)(char *pcImgBuffer, int iImgLength) = NULL;

#if 1
//long g_lTimeDelay1970_InSec = 2404836240LL;
long g_lTimeDelay1970_InSec = 1199145600LL;
long g_lTimeDelayZone_InSec = -8 * 60 * 60;
#else
long g_lTimeDelay1970_InSec = 0;
long g_lTimeDelayZone_InSec = 0;
#endif

time_t tTimeServerStart;
BOOL USI_IS_READY;

const char *g_cNoCacheHeader = "Pragma: no-cache\r\nCache-Control: no-cache\r\nExpires: 01 Jan 1970 00:00:00 GMT\r\n";
const char *g_apcNetworkInterface[2] = {"eth1", "wlan0"};


static PRD_TASK_T g_prdtskResetFlag_FactoryDefault;
static BOOL g_ResetFlag = FALSE;
static void prdTask_ResetFlag_FactoryDefault(void *pArg)
{
	
	
	prdDelTask(&g_prdtskResetFlag_FactoryDefault);
	
	diag_printf("Reset to factory default\n");
	g_ConfigParam.ucFailTime = 0;
	WriteFlashMemory(&g_ConfigParam);
	
	if (g_ResetFlag)
		ledClearFactoryDefault();
}

void ReadCameraParam(void)
{
	/* 读取配置参数 */
	if (!ReadFlashMemory(&g_ConfigParam)
		|| g_ConfigParam.ucFailTime >= 3)
	{
		fprintf(stderr, "Can not get config parameters, use factory-default instead.\n");
		
		g_ResetFlag = TRUE;
#if 0
		if (!ReadFactoryDefault(&g_ConfigParam))
		{
			fprintf(stderr, "Can not get factory-default parameters!\n");
			InitDefaultParam(&g_ConfigParam);
			
			WriteFactoryDefault(&g_ConfigParam);
			WriteFlashMemory(&g_ConfigParam);
		}
#else
		InitDefaultParam(&g_ConfigParam);			
		
#endif
	}
	else
		++g_ConfigParam.ucFailTime;

	WriteFlashMemory(&g_ConfigParam);

	/* Set led init state */
	if (g_ResetFlag)
		ledSetFactoryDefault();
	else
		ledShowState_PoweredOn();
	
	prdAddTask(&g_prdtskResetFlag_FactoryDefault, &prdTask_ResetFlag_FactoryDefault, 500, NULL);
}


/* 初始化camera的参数 */
void ResetCameraParam(void)
{
	int i;	

	
//diag_printf("mode ==%d,channel===%d,wepset =%d\n",g_ConfigParam.ucWlanOperationMode,g_ConfigParam.ulWlanChannel,g_ConfigParam.ucWepSet);
#ifdef WLAN
	g_ConfigParam.abAsNetEnable[1] = TRUE;
	if (g_ConfigParam.abAsNetEnable[1])
	{
		InitNetInterface(g_apcNetworkInterface[1]);
		/*change the mac address*/
		if (!g_ConfigParam.abUseHardwareMac[1])
		{
			struct ifreq ifr;
			int fd;
			strcpy(ifr.ifr_name,"wlan0");
			if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
				printf("socket error\n");
			else
			{
				memcpy((void*)ifr.ifr_hwaddr.sa_data, g_ConfigParam.acMAC[1], sizeof(g_ConfigParam.acMAC[1]));
				if (ioctl(fd, SIOCSIFHWADDR, &ifr) < 0)
				{
					printf("Set netmask error\n");
				}
				close(fd);
			}			
		}

//		/*Set wlan*/
//		if (!SetWlan())
//		{	
//			LED_WiFi_Error();
//			//return;		
//		}
	}
#else
	g_ConfigParam.abAsNetEnable[0] = TRUE;
	/* 初始化网卡 */
	InitNetInterface(g_apcNetworkInterface[0]);
#endif

	/* 设IP */
//	SetIP();
//	SetRandomIP("wlan0");
	SetNullIP("wlan0");

	/* 设置用户名密码文件 */
	if (g_ConfigParam.iUserCurNum < 0) g_ConfigParam.iUserCurNum = 0;
	/* Set user and pass for HTTP */
	for (i = 0; i < g_ConfigParam.iUserCurNum; i++)
		httpAuthSetUser(g_ConfigParam.aacUserName[i], g_ConfigParam.aacUserPass[i], g_ConfigParam.acUserPrivilege[i]);
	httpEnableUserCheck(g_ConfigParam.bUserCheck);
	
	/* init runtime state */
	memset (&g_WebCamState, 0, sizeof (g_WebCamState));
	g_WebCamState.uiPicIndex = INVALID_PICTURE;
	g_WebCamState.ucCamera = CAMERA_OFF;
	g_WebCamState.uiPicMotionDetectedIndex = INVALID_PICTURE;
	g_WebCamState.ucFtp = (g_ConfigParam.bFtpEnable?'\1':'\0');
	g_WebCamState.ucEmail = (g_ConfigParam.bMailEnable?'\1':'\0');
	g_WebCamState.ucUserCheck = (unsigned char)httpIsEnableUserCheck();
	g_WebCamState.iReadyImg = -1;
	
}

void RegisterSubCmd(const char *pcCmd, SUBCMD_PFUN funData)
{
	httpRegisterEmbedFunEx(pcCmd, Http_SubCmdInit, AUTH_ANY, (void *)funData);
}


#if 0
static cyg_handle_t bs_handle = NULL;
static cyg_thread  bs_thread;
UINT32 bs_thread_stack[32*1024 / sizeof(UINT32)];
void test_break_system(cyg_addrword_t pParam)
{
	while(1)
	{
		tt_msleep(5000);
		pwr_power_saving();
	}


//	int i = 0;

	//cyg_scheduler_lock();
//	while(1)
//	{
//		char buf[64];
		
		
//		cyg_thread_delay(200);
//		diag_printf("In testing: %d\n", i++);
		//while(1);
	
//		sprintf(buf, "i = %d\r\n", i);
//		hi_uart_write(buf, strlen(buf));
		
//	}

	//cyg_scheduler_unlock();
}


void test_sus()
{
	if (bs_handle != NULL)
	{
		cyg_thread_info info;
		thread_join(&bs_handle, &bs_thread, &info);
	}
				cyg_thread_create(PTD_PRIORITY, &test_break_system, (cyg_addrword_t)NULL, "breaking", bs_thread_stack,
                        sizeof(bs_thread_stack), &bs_handle, &bs_thread);			
                cyg_thread_resume(bs_handle);
                
                
}                
#else
void test_sus()
{
}
#endif
                
#if 1
static unsigned short dbg_line_copy[512];
static unsigned short dbg_line_pos_copy;
static unsigned short dbg_line[512];
static unsigned short dbg_line_pos = 0;
void dbg_line_save(int line)
{
	cyg_interrupt_disable();
	dbg_line[dbg_line_pos++] = (unsigned short)line;
	if (dbg_line_pos >= sizeof(dbg_line) / sizeof(dbg_line[0]))
		dbg_line_pos = 0;
	cyg_interrupt_enable();
}
#endif

#if 1	//xhchen - MCU debug
static char uart_log[128];
static int uart_log_pos = 0;
static int uart_log_stop = 0;

char hi_uart_log_read(int blog, char ch)
{
	if (blog && !uart_log_stop)
	{
		uart_log[uart_log_pos++] = 'R';
		uart_log[uart_log_pos++] = ch;
			if (uart_log_pos >= sizeof(uart_log))
			uart_log_pos = 0;
	}
	return ch;
}

char hi_uart_log_write(int blog, char ch)
{
	if (blog && !uart_log_stop)
	{
		uart_log[uart_log_pos++] = 'W';
		uart_log[uart_log_pos++] = ch;
		if (uart_log_pos >= sizeof(uart_log))
			uart_log_pos = 0;
	}
	return ch;
}


void hi_uart_log_stop()
{
	uart_log_stop = 1;
}

#endif

extern int g_ntp_add;
extern int g_ts_video;
extern int g_ts_audio;
extern double max_vd_ratio;
extern double max_ad_ratio;




int create_listen_socket( int port );
int accept_client( int fd_listen );
static cyg_thread g_ptdNetDebug;	/* pthread_t for SelectThread */
static cyg_handle_t g_ptdNetDebug_handle;
static unsigned char *g_ptdNetDebug_stack = NULL;

void NetTestThread(cyg_addrword_t arg)
{
	int listen_fd = create_listen_socket(8181);
	while(listen_fd >= 0)
	{
		int fd = accept_client(listen_fd);
		diag_printf("accept fd=%d\n", fd);
		if(fd >= 0)
		{
			int i;
			for(i = 0; i < 100; ++i)
				write(fd, "abcd\r\n", 6);
			close(fd);
		}
		diag_printf("Retry accept\n");
	}
	diag_printf("After while\n");
	close(listen_fd);
}

#include "ppp.h"
int Config_MemDebug(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
	case CA_CONFIG:
		{
			UINT32 i;
			char acBuf[128];
			UINT32 nLen = 0;
			static cyg_ppp_handle_t ppp_handle;
			const char *pcAction = httpGetString(pParamList, "action");
			
			if(strcmp(pcAction, "net") == 0)
			{
				if(g_ptdNetDebug_stack == NULL)
				{
					g_ptdNetDebug_stack = (unsigned char *)malloc(STACKSIZE);
					if (g_ptdNetDebug_stack != NULL)
					{
						cyg_thread_create(PTD_PRIORITY, &NetTestThread, NULL, "ptdNetTest",
							g_ptdNetDebug_stack, STACKSIZE, &g_ptdNetDebug_handle, &g_ptdNetDebug);
						if (g_ptdNetDebug_handle == NULL)
							fprintf(stderr, "Thread for \"NetTest\" creation failed!\n");
						else
							cyg_thread_resume(g_ptdNetDebug_handle);
					}
				}
			}
			else if (strcmp(pcAction, "print") == 0)
			{
				while(1)
				{
					diag_printf("random%d", rand());
					if(rand()%12==0)
						diag_printf("\n");
				}
			}
			else if (strcmp(pcAction, "pppip") == 0)
			{
				int fd;
				struct ifreq ifr;
				UINT32 ulIP = 0;
				UINT32 ulNetmask = 0;
				char acIP[16];
				char acNetmask[16];
				
				fd = socket(AF_INET, SOCK_DGRAM, 0);
				strcpy(ifr.ifr_name, "ppp0");
				
				if (fd >= 0) 
				{
					if (ioctl(fd, SIOCGIFADDR, &ifr) >= 0)
						ulIP = (unsigned int)(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr);
						
					if (ioctl(fd, SIOCGIFNETMASK, &ifr) == 0)
						ulNetmask = (unsigned int)(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr);
					
					close(fd);
				}
				
				httpIP2String(ulIP, acIP);
				httpIP2String(ulNetmask, acNetmask);
				AddHttpValue(pReturnXML, "IP", acIP);
				AddHttpValue(pReturnXML, "Netmask", acNetmask);
			}
			else if (strcmp(pcAction, "dppp") == 0)
			{
				ppot_disconnect();
			}
			else if (strcmp(pcAction, "ppp") == 0)
			{
				bool ret;
				const char *pcUsername;
				const char *pcPassword;
				const char *pcServer;
				int iPort;
				
				pcUsername = httpGetString(pParamList, "User");
				pcPassword = httpGetString(pParamList, "Pass");
				pcServer = httpGetString(pParamList, "Server");
				iPort = httpGetLong(pParamList, "Port");

/*				
u: xiaohui      p: 0gogorovio0
u: xiaohui1    p: 1gogorovio1
u: xiaohui2    p: 2gogorovio2
*/

				//ppp user: blah password=blahing
				// ssh port:4239, user: xhchen, pass: aeV2ohTi
				

				// Start up PPP
				//ppot_connect("tools.pimpmyrovio.com", 21201);
				//ppot_connect("116.92.2.11", 21201);
				ret = ppot_connect(&pcServer, &iPort, 1, pcUsername, pcPassword);
				AddHttpValue(pReturnXML, "ppot_connect", (ret ? "true" : "false"));
#if 0		
				ppp_handle = cyg_ppp_up( "/dev/sert0", &options );


				// Wait for it to get running
				if( cyg_ppp_wait_up( ppp_handle ) == 0 )
				{
					// Make use of PPP
					//use_ppp();

					// Bring PPP link down
					//cyg_ppp_down( ppp_handle );

					// Wait for connection to go down.
					//cyg_ppp_wait_down( ppp_handle );
				}
#endif			
				//cyg_thread_delay(5000);
				//ppot_disconnect();
				//diag_printf("Disconnect=====================\n");
				//while(1);
			}
			else if (strcmp(pcAction, "rtsp") == 0)
			{
				g_ntp_add = httpGetLong(pParamList, "ntp");
				g_ts_video = httpGetLong(pParamList, "ts_video");
				g_ts_audio = httpGetLong(pParamList, "ts_audio");
				AddHttpNum(pReturnXML, "ntp", g_ntp_add);
				AddHttpNum(pReturnXML, "ts_video", g_ts_video);
				AddHttpNum(pReturnXML, "ts_audio", g_ts_audio);
				AddHttpNum(pReturnXML, "max_vd_ratio", (int)max_vd_ratio);
				AddHttpNum(pReturnXML, "max_ad_ratio", (int)max_vd_ratio);
			}
			else if (strcmp(pcAction, "dir") == 0)
			{
			}
			else if (strcmp(pcAction, "channel") == 0)
			{
				int ch;
				GetWlanChannel("wlan0", &ch);
				AddHttpNum(pReturnXML, "channel", ch);
			}
			else if (strcmp(pcAction, "upnp") == 0)
			{
				upnp_refresh();
				AddHttpValue(pReturnXML, "UPNP", "refresh");
			}
			else if (strcmp(pcAction, "malloc") == 0)
			{
				static unsigned long ulTotalSize = 0;
				unsigned long ulSize = httpGetLong(pParamList, "size");
				void *pAddr = malloc (ulSize);
				if (pAddr != NULL)
					ulTotalSize += ulSize;
				AddHttpNum(pReturnXML, "addr", (int)pAddr);
				AddHttpNum(pReturnXML, "total_size", ulTotalSize);
			}
			else if (strcmp(pcAction, "checkip") == 0)
			{
				DDNS_CheckIP();
			}
			else if (strcmp(pcAction, "ddns") == 0)
			{
				BOOL bEnable = g_ConfigParam.bEnableDDNS;
				g_ConfigParam.bEnableDDNS = TRUE;
				DDNS_Update();
				g_ConfigParam.bEnableDDNS = bEnable;
			}
			else if (strcmp(pcAction, "line") == 0)
			{
				int i;
				char name[16];
				int size = sizeof(dbg_line_copy) / sizeof(dbg_line_copy[0]);
				cyg_interrupt_disable();
				dbg_line_pos_copy = dbg_line_pos;
				memcpy(dbg_line_copy, dbg_line, sizeof(dbg_line));
				cyg_interrupt_enable();	
				
				for (i = 0; i < size; i += 16)
				{
					int j;
					char str[128];
					int len = 0;
					for (j = 0; j < 16; j++)
					{
						len += sprintf(str + len, "%5d", (int) dbg_line_copy[(i+j+dbg_line_pos)%(size)]);
					}
					sprintf(name, "%5d", i);
					AddHttpValue(pReturnXML, name, str);
				}
			}
#if 1	//xhchen - MCU debug
			else if (strcmp(pcAction, "uart") == 0)
			{
				char log[128];
				char name[16];
				int log_pos;
				int i;
				sysDisableIRQ();
				log_pos = uart_log_pos;
				memcpy(log, uart_log, sizeof(log));
				sysEnableIRQ();
				
				
				for (i = 0; i < sizeof(log); i += 16)
				{
					int j;
					char str[32];
					int len = 0;
					for (j = 0; j < 16; j += 2)
					{
						char ch = log[(log_pos + i + j) % sizeof(log)];
						len += sprintf(str + len, "%c", ( ch != '\0' ? ch : '-') );
						ch = log[(log_pos + i + j + 1) % sizeof(log)];
						len += sprintf(str + len, "%02x", (int)(unsigned char)ch);
					}
					
					sprintf(name, "%08x", i / 2);
					AddHttpValue(pReturnXML, name, str);
				}
			}
			else if (strcmp(pcAction, "mcu") == 0)
			{
				void mcuGetErrorStatus(UINT32 *puCmdNo, UINT32 *puCommandCount, BOOL *pbCrashedLock);
				UINT32 uCmdNo;
				UINT32 uCmdCount;
				BOOL bCrashedLock;
				
				mcuGetErrorStatus(&uCmdNo, &uCmdCount, &bCrashedLock);
				AddHttpNum(pReturnXML, "CmdNo", uCmdNo);			
				AddHttpNum(pReturnXML, "LockCrashed", bCrashedLock);
				AddHttpNum(pReturnXML, "CmdCount", uCmdCount);
			}
			else
#endif
			if (strcmp(pcAction, "break") == 0)
			{
				test_sus();
				AddHttpValue(pReturnXML, "break_after", "3 seconds");
			}
			else if (strcmp(pcAction, "ps") == 0)
			{
				outpw(REG_GPIO_IE, (inpw(REG_GPIO_IE)|0x00000010));
				
				SetWlanPSMode("wlan0", FALSE);
			
				AddHttpValue(pReturnXML, "SetWlanPSMode", "wlan0");
			}
			else if (strcmp(pcAction, "cfg") == 0)
			{
				
				SetWlanHostSleepCfg("wlan0", 5);
				AddHttpValue(pReturnXML, "SetWlanHostSleepCfg", "wlan0 5");
			}
			else if (strcmp(pcAction, "gpiob19") == 0)
			{
				/* Set GPIOB for MCU */
				outpw(REG_GPIOB_OE,inpw(REG_GPIOB_OE)&(~0x00080000));		//19 output;
				outpw(REG_GPIOB_DAT,inpw(REG_GPIOB_DAT)&(~0x00080000));	//19 low
				cyg_thread_delay(8);
				outpw(REG_GPIOB_OE,inpw(REG_GPIOB_OE)&(~0x00080000));		//19 output;
				outpw(REG_GPIOB_DAT,inpw(REG_GPIOB_DAT)|(0x00080000));	//19 high   				
				cyg_thread_delay(8);
				
				sprintf(acBuf, "%08x", inpw(REG_GPIOB_OE));
				AddHttpValue(pReturnXML, "REG_GPIOB_OE", acBuf);
				sprintf(acBuf, "%08x", inpw(REG_GPIOB_DAT));
				AddHttpValue(pReturnXML, "REG_GPIOB_DAT", acBuf);
			}
			else if (strcmp(pcAction, "mall") == 0)
			{
				struct mallinfo info = mallinfo();
				AddHttpNum(pReturnXML, "arena", info.arena);
				AddHttpNum(pReturnXML, "ordblks", info.ordblks);
				AddHttpNum(pReturnXML, "smblks", info.smblks);
				AddHttpNum(pReturnXML, "hblks", info.hblks);
				AddHttpNum(pReturnXML, "hblkhd", info.hblkhd);
				AddHttpNum(pReturnXML, "usmblks", info.usmblks);
				AddHttpNum(pReturnXML, "fsmblks", info.fsmblks);
				AddHttpNum(pReturnXML, "uordblks", info.uordblks);
				AddHttpNum(pReturnXML, "fordblks", info.fordblks);
				AddHttpNum(pReturnXML, "keepcost", info.keepcost);
				AddHttpNum(pReturnXML, "maxfree", info.maxfree);
			}
			else if (strcmp(pcAction, "write_mp4") == 0)
			{
				UINT32 uBitrate = httpGetLong(pParamList, "bitrate");
				wb702SetVideoDynamicBitrate(uBitrate);
				sprintf(acBuf, "%d", uBitrate);
				AddHttpValue(pReturnXML, "bitrate", acBuf);
			}
			else if (strcmp(pcAction, "read_flash") == 0)
			{
				UINT32 uAddr = httpGetLong(pParamList, "address");
				UINT32 uSize = httpGetLong(pParamList, "size");
				if (uSize == 0)
					uSize = 1;
				sprintf(acBuf, "%08x[->%08x]", uAddr, uSize);
				AddHttpValue(pReturnXML, "read_flash", acBuf);
				
				for (i = 0; i < uSize; i += 8)
				{
					int j;
					char acFlash[8];
					UINT32 uThisSize = uSize - i;
					if (uThisSize > sizeof(acFlash))
						uThisSize = sizeof(acFlash);
										
					usiMyRead(uAddr + i, uThisSize, (void *)acFlash);
					
					acBuf[0] = '\0';
					nLen = 0;
					for (j = 0; j < uThisSize; ++j)
						nLen += sprintf(acBuf + nLen, "%02x ", (int)(unsigned char)(acFlash[j]) );
					AddHttpValue(pReturnXML, "read_flash", acBuf);
				}
				return 0;				
			}
			else if (strcmp(pcAction, "read_mem") == 0)
			{
				UINT32 uAddr = httpGetLong(pParamList, "address");
				UINT32 uSize = httpGetLong(pParamList, "size");
				if (uSize == 0)
					uSize = 1;
				sprintf(acBuf, "%08x[->%08x]", uAddr, uSize);
				AddHttpValue(pReturnXML, "read_mem", acBuf);
				acBuf[0] = '\0';
				for (i = 0; i < uSize; ++i)
				{
					nLen += sprintf(acBuf + nLen, "%02x ", (int)*(unsigned char *)(uAddr + i) );
					if (( i + 1 ) % 8 == 0)
					{
						AddHttpValue(pReturnXML, "read_mem", acBuf);
						nLen = 0;
						acBuf[0] = '\0';
					}
				}
				if ( i % 8 != 0)
					AddHttpValue(pReturnXML, "read_mem", acBuf);
				return 0;				
			}
			else if (strcmp(pcAction, "write_mem") == 0)
			{
				UINT32 uAddr = httpGetLong(pParamList, "address");
				UINT32 uValue = httpGetLong(pParamList, "value");
				UINT32 uSize = httpGetLong(pParamList, "size");
				if (uSize == 0)
					uSize = 4;
				memcpy((char *)uAddr, &uValue, uSize);
				sprintf(acBuf, "%08x[->%08x]=%08x", uAddr,uSize,uValue);
				AddHttpValue(pReturnXML, "write_mem", acBuf);
				return 0;
			}
			else if (strcmp(pcAction, "write_register") == 0)
			{
				UINT32 uAddr = httpGetULong(pParamList, "address");
				UINT32 uValue = httpGetULong(pParamList, "value");
				UINT32 uSize = httpGetULong(pParamList, "size");
				if (uSize == 0)
					uSize = 4;
				//outpw(address, value);
				memcpy((char *)uAddr, &uValue, uSize);
				sprintf(acBuf, "%08x[->%08x]=%08x", uAddr,uSize,uValue);
				AddHttpValue(pReturnXML, "write_register", acBuf);
				return 0;
			}
#if 1			
			else if (strcmp(pcAction, "set_oe") == 0)
			{
				UINT32 out = httpGetULong(pParamList, "out");
				UINT32 dir = httpGetULong(pParamList, "dir");
				UINT32 uSize = httpGetULong(pParamList, "size");
				
				UINT32 read_oe=0;
				UINT32 read_dat=0;
				UINT32 read_sts=0;

				if (uSize == 0)
					uSize = 4;
				//outpw(address, value);
				if(out==5 || out==18|| out ==0||out==1 )   // gpio5 is out1, gpio18 is out2
				{
					if(dir==1)
						outpw(REG_GPIO_OE, inpw(REG_GPIO_OE)|(1<<out));
					else
						outpw(REG_GPIO_OE, inpw(REG_GPIO_OE)&~(1<<out));
				}
				
				read_oe = inpw(REG_GPIO_OE);
				read_dat = inpw(REG_GPIO_DAT);
				read_sts = inpw(REG_GPIO_STS);
					
				sprintf(acBuf, "%08xoe[%08xdat]%08xsts", read_oe,read_dat,read_sts);
				AddHttpValue(pReturnXML, "set_oe", acBuf);
				return 0;
			}
			
			else if (strcmp(pcAction, "set_dat") == 0)
			{
				UINT32 out = httpGetULong(pParamList, "out");
				UINT32 dat = httpGetULong(pParamList, "dat");
				UINT32 uSize = httpGetULong(pParamList, "size");
				
				UINT32 read_oe=0;
				UINT32 read_dat=0;
				UINT32 read_sts=0;

				if (uSize == 0)
					uSize = 4;
				//outpw(address, value);
				if(out==5 || out==18)   // gpio5 is out1, gpio18 is out2
				{
					if(dat==1)
						outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT)|(1<<out));
					else
						outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT)&~(1<<out));
				}
				
				read_oe = inpw(REG_GPIO_OE);
				read_dat = inpw(REG_GPIO_DAT);
				read_sts = inpw(REG_GPIO_STS);
					
				sprintf(acBuf, "%08xoe[%08xdat]%08xsts", read_oe,read_dat,read_sts);
				AddHttpValue(pReturnXML, "set_dat", acBuf);
				return 0;
			}
			
			else if (strcmp(pcAction, "get_oe") == 0)
			{
				UINT32 uAddr = httpGetULong(pParamList, "address");
				UINT32 uValue = httpGetULong(pParamList, "value");
				UINT32 uSize = httpGetULong(pParamList, "size");
				UINT32 read_oe=0;
				UINT32 read_dat=0;
				UINT32 read_sts=0;
	
				if (uSize == 0)
					uSize = 4;
				read_oe = inpw(REG_GPIO_OE);
				read_dat = inpw(REG_GPIO_DAT);
				read_sts = inpw(REG_GPIO_STS);
				sprintf(acBuf, "%08xoe[%08xdat]%08xsts", read_oe,read_dat,read_sts);
				AddHttpValue(pReturnXML, "get_oe", acBuf);
				return 0;
			}
#endif			
			else if (strcmp(pcAction, "read_i2c") == 0)
			{
				UINT32 uAddr = httpGetLong(pParamList, "address");
				UINT32 uValue = i2cReadI2C(uAddr);
				sprintf(acBuf, "%08x=%08x", uAddr, uValue);
				AddHttpValue(pReturnXML, "read_i2c", acBuf);
				return 0;
			}
			else if (strcmp(pcAction, "write_i2c") == 0)
			{
				UINT32 uAddr = httpGetLong(pParamList, "address");
				UINT32 uValue = httpGetLong(pParamList, "value");
				i2cWriteI2C (uAddr, uValue);
				sprintf(acBuf, "%08x=%08x", uAddr, uValue);
				AddHttpValue(pReturnXML, "write_i2c", acBuf);
				return 0;
			}
			else if (strcmp(pcAction, "debug") == 0)
			{
			}
			else
				AddHttpValue(pReturnXML, "action", "error");
			return 0;
		}		
	}
	return -1;
}



static int FillTestSpeedData(HTTPCONNECTION hConnection, time_t *ptLastFill, void *p)
{
	httpAddBody(hConnection, (void *)0, 1024);
	httpSetSendDataOverFun(hConnection, FillTestSpeedData, NULL);
	return 1;
}
int Http_TestSpeed(HTTPCONNECTION hConnection, void *pParam)
{
	LIST *pList;
	
	httpSetSendDataOverFun(hConnection, FillTestSpeedData, NULL);

	httpSetKeepAliveMode(hConnection, FALSE);
	httpSetHeader(hConnection, 200, "OK", "",
				g_cNoCacheHeader,
				"application/octet-stream", FALSE);
//diag_printf("Fill data end\n");
	return 0;

}



void RegisterUrls()
{	
	httpRegisterEmbedFunEx("speed.cgi", Http_TestSpeed, AUTH_USER, NULL);
	httpRegisterEmbedFunEx("/Jpeg/CamImg*.jpg", Http_SendJpeg, AUTH_USER, NULL);	
	
#ifdef USE_SERVER_PUSH
	httpRegisterEmbedFunEx("GetData.cgi", Http_GetCameraData, AUTH_USER, NULL);
	RegisterSubCmd("DropData.cgi", Config_DropData);
#endif	

	RegisterSubCmd("GetStatus.cgi", Config_GetStatus);	
	RegisterSubCmd("GetLog.cgi", Config_GetLog);
	RegisterSubCmd("GetVer.cgi", Config_GetVer);
#ifdef USE_DDNS
	RegisterSubCmd("GetDDNS.cgi", Config_GetDDNS);
	RegisterSubCmd("SetDDNS.cgi", Config_SetDDNS);
#endif
	RegisterSubCmd("debug.cgi", Config_MemDebug);
	RegisterSubCmd("GetMyself.cgi", Config_GetMyself);
	RegisterSubCmd("GetUser.cgi", Config_GetUser);
	RegisterSubCmd("SetUser.cgi", Config_SetUser);
	RegisterSubCmd("DelUser.cgi", Config_DelUser);
	RegisterSubCmd("SetUserCheck.cgi", Config_SetUserCheck);
	
	RegisterSubCmd("SetTime.cgi", Config_SetTime);
	RegisterSubCmd("GetTime.cgi", Config_GetTime);
	RegisterSubCmd("SetLogo.cgi", Config_SetLogo);
	RegisterSubCmd("GetLogo.cgi", Config_GetLogo);
	
 	RegisterSubCmd("ChangeDirection.cgi", Config_ChDirection);	
	RegisterSubCmd("ChangeResolution.cgi", Config_ChResolution);
	RegisterSubCmd("ChangeCompressRatio.cgi", Config_ChCompressRatio);
	RegisterSubCmd("ChangeFramerate.cgi", Config_ChFramerate);
	RegisterSubCmd("ChangeBrightness.cgi", Config_ChBrightness);
	RegisterSubCmd("ChangeSpeakerVolume.cgi", Config_ChSpeakerVolume);
	RegisterSubCmd("ChangeMicVolume.cgi", Config_ChMicVolume);
	//RegisterSubCmd("
	
	
	RegisterSubCmd("SetMotionDetect.cgi", Config_SetMotionDetect);
	RegisterSubCmd("GetMotionDetect.cgi", Config_GetMotionDetect);
	
	RegisterSubCmd("SetMediaFormat.cgi", Config_SetMediaFormat);
	RegisterSubCmd("GetMediaFormat.cgi", Config_GetMediaFormat);

	RegisterSubCmd("SetCamera.cgi", Config_SetCamera);
	RegisterSubCmd("GetCamera.cgi", Config_GetCamera);
		
	RegisterSubCmd("SetWlan.cgi", Config_SetWlan);
	RegisterSubCmd("GetWlan.cgi", Config_GetWlan);
	RegisterSubCmd("ScanWlan.cgi", Config_ScanWlan);
	
	RegisterSubCmd("SetMac.cgi", Config_SetMac);
	RegisterSubCmd("GetMac.cgi", Config_GetMac);
	

	RegisterSubCmd("SetMail.cgi", Config_SetMail);
	RegisterSubCmd("GetMail.cgi", Config_GetMail);
	RegisterSubCmd("SendMail.cgi", Config_SendMail);
#ifdef RECORDER		
	RegisterSubCmd("SetFtp.cgi", Config_SetFtp);
	RegisterSubCmd("GetFtp.cgi", Config_GetFtp);	
#endif
	RegisterSubCmd("SetHttp.cgi", Config_SetHttp);
	RegisterSubCmd("GetHttp.cgi", Config_GetHttp);
	RegisterSubCmd("SetIP.cgi", Config_SetIP);
	RegisterSubCmd("GetIP.cgi", Config_GetIP);
	RegisterSubCmd("Reboot.cgi", Config_Reboot);
	RegisterSubCmd("SetFactoryDefault.cgi", Http_SetFactoryDefault);
	RegisterSubCmd("UpdateFactoryDefault.cgi", Http_UpdateFactoryDefault);
	
	
	//httpRegisterEmbedFunEx("SetFactoryDefault.cgi", Http_SetFactoryDefault, AUTH_ADMIN, NULL);	
	//httpRegisterEmbedFunEx("Upload.cgi", Http_Post_Init, AUTH_ADMIN, (void*)Http_Post_Upload);
	httpRegisterEmbedFunEx("Upload.cgi", Http_Post_Init, AUTH_ADMIN, (void*)Http_Post_FMUpload);
	//httpRegisterEmbedFunEx("FMUpload.cgi", Http_Post_Init, AUTH_ADMIN, (void*)Http_Post_FMUpload);
	//RegisterSubCmd("FMFlash.cgi", Config_FMFlash); //stat????
	RegisterSubCmd("GetUpdateProgress.cgi", Config_UpdateProgress);
	httpRegisterEmbedFunEx("bin/ipcam.bin", Http_GetFirmware, AUTH_USER, NULL);
	httpRegisterEmbedFunEx("GetAudio.cgi", Http_Post_Init, AUTH_USER, (void*)Http_GetAudio);
	httpRegisterEmbedFunEx("webcam", Http_RtspTunnel, AUTH_USER, NULL);
	httpRegisterEmbedFunEx("Cmd.cgi", Http_CommonCmdInit, AUTH_ANY, NULL);	
	
	RegisterSubCmd("SetName.cgi", Config_SetName);
	RegisterSubCmd("GetName.cgi", Config_GetName);	
	
	RegisterSubCmd("GetVNet.cgi", Config_GetVNet);
	RegisterSubCmd("SetVNet.cgi", Config_SetVNet);
	
	RegisterSubCmd("GetUPnP.cgi", Config_GetUPnP);
	RegisterSubCmd("SetUPnP.cgi", Config_SetUPnP);
	
	RegisterSubCmd("GetPPP.cgi", Config_GetPPP);
	RegisterSubCmd("SetPPP.cgi", Config_SetPPP);
	
	
	httpRegisterEmbedFunEx("SendHttp.cgi", Http_SendHttpRequest, AUTH_USER, NULL);
	
	RegisterSubCmd("EnableTestNet.cgi", Config_EnableTestNet);
	RegisterWebFiles();

	httpRegisterEmbedFunEx("rev.cgi", Http_CommonCmdInit, AUTH_ANY, NULL);	
	RegisterSubCmd("mcu", Config_ControlMCU);
	
//YA uPnP
	 // for UPnP 
	 // provide description of our media server
	 httpRegisterEmbedFunEx("description.xml", Http_SendDesc, AUTH_ANY, NULL);
	 // reject all soap invoke and gena subscription
	 httpRegisterEmbedFunEx("/cmgr/control", Http_Send500, AUTH_ANY, NULL);
	 httpRegisterEmbedFunEx("/cmgr/event", Http_Send500, AUTH_ANY, NULL);
	 httpRegisterEmbedFunEx("/cdir/control", Http_Send500, AUTH_ANY, NULL);
	 httpRegisterEmbedFunEx("/cdir/event", Http_Send500, AUTH_ANY, NULL); 	
	
}


void MailInfoInit(Mail_Info *info)
{
	memset(info,0,sizeof(Mail_Info));
	info->MailSender=g_ConfigParam.acMailSender;
	info->MailReceiver=g_ConfigParam.acMailReceiver;
	info->MailServer=g_ConfigParam.acMailServer;
	info->MailPort=&g_ConfigParam.usMailPort;
	info->MailUser=g_ConfigParam.acMailUser;
	info->MailPassword=g_ConfigParam.acMailPassword;
	info->MailSubject=g_ConfigParam.acMailSubject;
	info->MailBody=g_ConfigParam.acMailBody;
	info->MailCheck=&(g_ConfigParam.bMailCheck);
	info->MailEnable=&(g_ConfigParam.bMailEnable);
	info->Email=&(g_WebCamState.ucEmail);
}

#ifdef RECORDER
void MailInfoInit(Mail_Info *info)
{
	memset(info,0,sizeof(Mail_Info));
	info->MailSender=g_ConfigParam.acMailSender;
	info->MailReceiver=g_ConfigParam.acMailReceiver;
	info->MailServer=g_ConfigParam.acMailServer;
	info->MailUser=g_ConfigParam.acMailUser;
	info->MailPassword=g_ConfigParam.acMailPassword;
	info->MailSubject=g_ConfigParam.acMailSubject;
	info->MailBody=g_ConfigParam.acMailBody;
	info->MailCheck=&(g_ConfigParam.bMailCheck);
	info->MailEnable=&(g_ConfigParam.bMailEnable);
	info->Email=&(g_WebCamState.ucEmail);
}

void FtpInfoInit(Ftp_Info *info)
{
	memset(info,0,sizeof(Ftp_Info));
	info->ConfigParam = &g_ptmConfigParam; 
	info->FtpServer=g_ConfigParam.acFtpServer;
	info->FtpUser=g_ConfigParam.acFtpUser;
	info->FtpPass=g_ConfigParam.acFtpPass;
	info->FtpAccount=g_ConfigParam.acFtpAccount;
	info->FtpUploadPath=g_ConfigParam.acFtpUploadPath;
	info->FtpEnable=&(g_ConfigParam.bFtpEnable);
	info->Ftp=&(g_WebCamState.ucFtp);
}
#endif
Mail_Info* g_mailinfo;
#define STACK 32*1024
cyg_handle_t mail_handle;
cyg_thread  mail_thread;
unsigned char mail_stack[STACK];

void ResetOtherThread(void)
{
	UsedBuf *CameraBuf=NULL;
	if((g_mailinfo = (Mail_Info*)malloc(sizeof(Mail_Info))) ==NULL)
		PRINT_MEM_OUT;
	else
		MailInfoInit(g_mailinfo);
	/*init mail  buffer*/
	{ 
		int mailsize;
		char *mailbuf;
		mailsize  = get_mailmem_size(1);
		mailbuf = (char*)malloc(mailsize);
		if(mailbuf == NULL)
			printf("not enough memory for mail\n");
		else		
			memset(mailbuf, 0, mailsize);	
		mail_mem_init(mailbuf, mailsize);		
		
	}			
	cyg_thread_create(PTD_PRIORITY, &MailStart, (cyg_addrword_t)g_mailinfo, "mail_start", mail_stack,
                        STACK, &mail_handle, &mail_thread);
	if ( mail_handle == NULL)
	{
		printf("Thread for mail creation failed!\n");
		exit(-1);
	}
    
    cyg_thread_resume(mail_handle);
#ifdef RECORDER
	Mail_Info *mailinfo;
	Ftp_Info *ftpinfo;
	if((mailinfo = (Mail_Info*)malloc(sizeof(Mail_Info))) ==NULL)
		PRINT_MEM_OUT;
	else
		MailInfoInit(mailinfo);		
	if((ftpinfo = (Ftp_Info*)malloc(sizeof(Ftp_Info))) == NULL)
	 	PRINT_MEM_OUT;
	else
		FtpInfoInit(ftpinfo);		
	recorder_releative_thread_create(ftpinfo,mailinfo);
	/*init mail and ftp buffer*/
	{ 
		int mailsize,ftpsize;
		char *mailbuf,*ftpbuf;
		mailsize  = get_mailmem_size(0);
		mailbuf = (char*)malloc(mailsize);
		if(mailbuf == NULL)
			printf("not enough memory for mail\n");
		else		
			memset(mailbuf, 0, mailsize);	
		mail_mem_init(mailbuf, mailsize);
		
		ftpsize = get_ftpmem_size(0);
    	ftpbuf = (char*)malloc(ftpsize);
    	if(ftpbuf == NULL)
    		printf("not enough memory for ftp\n");
    	else 
    		memset(ftpbuf, 0, ftpsize);    	
    	ftp_mem_init(ftpbuf, ftpsize);
	}
	{
	   struct mallinfo info;
  	   info = mallinfo();
 	   diag_printf("******max %x, totalfree %x, total allocated %x\n",info.maxfree,info.fordblks, info.uordblks);
	}	
#endif
#ifdef USE_DDNS
	/* set message and thread for ntp */
	if ((g_pMsgSelect = CreateMsgQueue(0, TRUE)) == NULL)
	{
		fprintf(stderr, "Can not create message queue for SelectThread!\n");
		return;
	}
	
	cyg_thread_create(PTD_PRIORITY, &SelectThread, NULL, "ptdSelect", ptdSelect_stack, STACKSIZE, &ptdSelect_handle, &g_ptdSelect);
	if ( ptdSelect_handle == NULL)
	{
		fprintf(stderr, "Thread for \"Select\" creation failed!\n");
		return;
	}
	cyg_thread_resume(ptdSelect_handle);
#endif
#ifdef USE_FLASH
	/* set message and thread for flash */
	if ((g_pMsgFlash = CreateMsgQueue(0, FALSE)) == NULL)
	{
		fprintf(stderr, "Can not create message queue for FtpThread!\n");
		return;
	}
	
	
	cyg_thread_create(PTD_PRIORITY, &FlashThread, NULL, "ptdFlash", ptdFlash_stack, STACKSIZE, &ptdFlash_handle, &g_ptdFlash);
	if ( ptdFlash_handle == NULL)
	{
		fprintf(stderr, "Thread for ftp creation failed!\n");
		return;
	}
	cyg_thread_resume(ptdFlash_handle);	
#endif
	/* set message and thread for camera*/	 
	CameraBuf = (UsedBuf*)malloc(sizeof(UsedBuf));
	if (CameraBuf == NULL)
		return;
	//CameraBuf->AssignLen = MAX_CAMERA_IMG_LENGTH*2+MAX_CAMERA_MP4_LENGTH;
	CameraBuf->AssignLen = MAX_CAMERA_IMG_LENGTH * 2;
	CameraBuf->AssignBuffer  =  (char*)malloc(CameraBuf->AssignLen );
	if(CameraBuf->AssignBuffer  == NULL)
	{
		diag_printf("Malloc Camera Buffer False!!!!!!!\n");
	}
	memset(CameraBuf->AssignBuffer,0,CameraBuf->AssignLen);
	
	/**/
	if ((g_pMsgCamera = CreateMsgQueue(0, FALSE)) == NULL)
	{
		fprintf(stderr, "Can not create message queue for CameraThread!\n");
		return;
	}
	
	cyg_thread_create(PTD_PRIORITY, &CameraThread,(cyg_addrword_t) CameraBuf, "ptdCamera", ptdCamera_stack, STACKSIZE, &ptdCamera_handle, &g_ptdCamera);
	if ( ptdCamera_handle == NULL)
	{
		fprintf(stderr, "Thread for camera creation failed!\n");
		return;
	}
	cyg_thread_resume(ptdCamera_handle);
	
	// change it to sample;
	 VirtualComInit();
	 diag_printf("VcomByUSBInit successfully\n");

}

static time_t mytime(void)
{
    time_t cur_sec;
    cyg_tick_count_t cur_time;
    cur_time = cyg_current_time();
    cur_sec = (cur_time*10) / 1000;
    return cur_sec;
}

int OnHttpInit(HTTPSERVER hServer)
{
	/* http server initilized, add your code here */

	/* 重置其他的程序 */
	//ResetOtherThread();
	/* 记录启动时间 */
	tTimeServerStart = mytime();

 	/* 设置时区及时间 */
 	{
 		long lTime1970_InSec;
	 	GetCheckedTime(&lTime1970_InSec, NULL);
		SetCheckedTime(NULL, &g_ConfigParam.lTimeZone_InSec);
	}
#ifdef USE_DDNS	
	if (g_ConfigParam.bUseNtp)
		NtpSetTime(g_ConfigParam.acNtpServer);
#endif
	return 0;
}


#ifdef FILESYSTEM
BOOL ipcWriteMyPID( VOID )
{
	cyg_handle_t self;
	INT nStatus;
	INT nWriteLen;
	INT hFile;
	
	self = cyg_thread_self( );
	
	hFile = fsOpenFile((char*)MY_PID_LOG,NULL, O_WRONLY |O_CREAT | O_TRUNC);
	if(hFile < 0)
	{
		diag_printf("Open file error %x for writing PID\n", hFile);
		return FALSE;
	}
	
	nStatus = fsWriteFile(hFile, (VOID *)&self, sizeof(self), &nWriteLen);
	fsCloseFile(hFile);

	if((nStatus < 0)||(nWriteLen != sizeof(self)))
	{
		diag_printf("save configuration information error\n");
		return FALSE;
	}

	return TRUE;
}


VOID ipcFormatAllDisk( VOID )
{

}



VOID ipcCheckDisk( VOID )
{
	if ( !ipcWriteMyPID( ) )
		ipcFormatAllDisk( );
}
#endif



void ServerStart(void)
{

	int i, j;	

	char *pcDocRoot = "./";
	static int aiPort[] = {80, 0};
#ifndef WLAN
    FMI_CARD_DETECT_T card;
#endif	
	cyg_mutex_init(&g_ptmConfigParam);
	cyg_mutex_init(&g_ptmState);
	cyg_mutex_init(&g_ptmTimeSetting);
	cyg_mutex_init(&g_ptmWebCameraLog);
	tt_rmutex_init(&g_rmutex);
	
	{
		UINT32 uFlashTotal, uFlashFreeBegin, uFlashFreeEnd;
		GetFlashCapability(&uFlashTotal, &uFlashFreeBegin, &uFlashFreeEnd);
		diag_printf("Flash total: %d, free space [%d - %d]\n", uFlashTotal, uFlashFreeBegin, uFlashFreeEnd);
	}
	
#ifndef WLAN
	/*init the sd card*/
    //do {outpw (REG_CLKCON, inpw (REG_CLKCON) | FMICLK_EN); } while (0);
    fmiSetFMIReferenceClock(OPT_UPLL_OUT_CLOCK/3/1000);    
	fsFixDriveNumber('D', 'C', 'Z');    
    fsInitFileSystem();    
   	card.uCard = FMI_SD_CARD;		// card type
	card.uGPIO = 4;				// card detect GPIO pin
	card.uWriteProtect = 16;			// card detect GPIO pin
	card.uInsert = 0;				// 0/1 which one is insert
	card.nPowerPin = 12;				// card power pin, -1 means no power pin
	card.bIsTFlashCard = FALSE;
	fmiSetCardDetection(&card);
	fmiInitSDDevice();
	diag_printf("Init the SD Card ok!\n");
	g_StreamServer_Buf = (char*)(g_StreamServer_Buf1);
#endif
#ifdef FILESYSTEM
	/* Check if disk need to be formatted. */
	ipcCheckDisk( );	
#endif
	
	/*register reset interrupt*/
	InitResetGPIO();
	// Set HIC ready
   	outpw(REG_HICSR, inpw(REG_HICSR) & 0xFFFFFF7F);

	if (0)
	{
		int j = 0;
		for(;j<5;j++)
			init_loopback_interface(0);
	}

	
	/* 初始化camera的参数 */ 
	ResetCameraParam();    
	/* 挂载需要额外处理的URL */
	RegisterUrls();
	//GetDHCPInfo();

	/* 設置进入不同特权领域的提示语 */
	httpSetAuthPrompt(CONFIG_LOGIN_PROMPT_USER, CONFIG_LOGIN_PROMPT_ADMIN, NULL);
	/* 选择从命令行或者配置参数中设定的http端口 */
	for (i = 0; i < sizeof(g_ConfigParam.usHttpPort) / sizeof(unsigned short); i++)
	{
		if (g_ConfigParam.usHttpPort[i] == 0) g_ConfigParam.usHttpPort[i] = aiPort[i];
	}
	for (i = 0, j = 0; i < sizeof(g_ConfigParam.usHttpPort) / sizeof(unsigned short); i++)
	{
		if (g_ConfigParam.usHttpPort[i] != 0)
			aiPort[j++] = g_ConfigParam.usHttpPort[i];
	}
	aiPort[j] = 0;
	diag_printf("Port is %d,%d\n",aiPort[0],aiPort[1]);

	ResetOtherThread();
	
	{
 	   cyg_thread_create(PTD_PRIORITY, &MctestThread, (cyg_addrword_t)aiPort, "ptdMctest", ptdMctest_stack, STACKSIZE2, &ptdMctest_handle, &g_ptdMctest);
		if ( ptdMctest_handle == NULL)
		{
			fprintf(stderr, "Thread for mctest creation failed!\n");
			return;
		}
		cyg_thread_resume(ptdMctest_handle);
	}

	cyg_thread_set_priority(cyg_thread_self(), PTD_PRIORITY);
	// 开始http服务 	
	httpdStartEx(pcDocRoot, aiPort,3600, 30 * 60 * 60 , 20, OnHttpInit, OnRequestBegin);	

}



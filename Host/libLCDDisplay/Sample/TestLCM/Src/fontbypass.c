#include "fontbypass.h"
#include "wlan0.h"

void initLCM(void);
void setMacAddress(void);

static BOOL g_bDisableDebugMsg = FALSE;

void uart_write( const char *buf, size_t size )
{
#if 1
	if (!g_bDisableDebugMsg)
	{
		size_t i;
	
		for ( i = 0; i < size; i++ )
			hal_if_diag_write_char( buf[i] );
	}
#endif
}

int main(int argc, char *argv[])
{
	char acInterface[] = {'w', 'l', 'a', 'n', '0', '\0'};
	int iOperationMode = 2;
	const char acWep128[13] =
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
		0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C
	};
	unsigned long ulWepAuthentication = IW_ENCODE_OPEN;	//IW_ENCODE_RESTRICTED;
	unsigned long ulWlanChannel = 5;
	
	cyg_interrupt_disable();
	cyg_interrupt_disable();
	cyg_interrupt_disable();
	
	cyg_interrupt_enable();
	VideoPhoneInit();
	
	force_net_dev_linked();
	cyg_do_net_init();
	
	diag_printf("interface=%s\n", acInterface);
	
	setMacAddress();
	if (!SetWlanOperationMode(acInterface,	iOperationMode))
		fprintf(stderr, "Can not set operation mode.\n");
	if (!SetWlanChannel(acInterface, ulWlanChannel))
		fprintf(stderr, "Can not set wireless channel.\n");
/*
	if (SetWlanWepKey(acInterface, acWep128, 13, -1, ulWepAuthentication)) // Set WEP key
	{
		SetWlanWepKey(acInterface, NULL, 0, -1, ulWepAuthentication);	// enable WEP
	}
*/	
	SetWlanESSID(acInterface, "default");
	init_all_network_interfaces();
	
	{
		struct ifreq ifr;
		int s;
	
        if ((s = socket(PF_INET, SOCK_STREAM,0))<0)
        	diag_printf("socket fail\n");
        memset((char *)&ifr, 0, sizeof(ifr));
		strcpy((char*)ifr.ifr_name, (char*)"wlan0");
		ifr.ifr_addr.sa_family = AF_INET;
        if (ioctl(s, SIOCGIFADDR, &ifr) >= 0)
        	diag_printf("ipaddr Address====%s\n",inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
        else
         	diag_printf("ioctl fail\n");
		close(s);
	}
	
	initLCM();
	
	if(DisplayInfoOnLCD_Init(DISPLAYINFOONLCD_MAXWIDTH, DISPLAYINFOONLCD_MAXHEIGHT, 5000) == FALSE)
	{
		diag_printf("DisplayInfoOnLCD_Init error\n");
		return 1;
	}
	DisplayInfoOnLCD_Start();
    return 0;
}

void initLCM(void)
{
	ControlBackLight(6, 0);
	wb702ConfigureLCM(DISPLAYINFOONLCD_MAXWIDTH, DISPLAYINFOONLCD_MAXHEIGHT, 2, 1, 3, 0, TRUE, 1);
	
	ControlLcmTvPower(2);
	ControlBackLight(6, 1);
	
	wb702SetLocalVideoSource(640, 480, 0);
	wb702SetJPEG(TRUE);
}

void setMacAddress(void)
{
	struct ifreq ifr;
	int fd;
	char acMAC[] = {0x00, 0x01, 0x03, 0x20, 0x72, 0x2a};
	
	strcpy(ifr.ifr_name,"wlan0");
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		printf("socket error\n");
		return;
	}
	
	memcpy((void*)ifr.ifr_hwaddr.sa_data, acMAC, sizeof(acMAC));
	if (ioctl(fd, SIOCSIFHWADDR, &ifr) < 0)
	{
		printf("Set netmask error\n");
	}
	
	close(fd);
}
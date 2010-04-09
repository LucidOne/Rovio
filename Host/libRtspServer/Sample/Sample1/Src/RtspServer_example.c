#include "stdio.h"
#include "network.h"
#include "pkgconf/libc_startup.h"
#include "errno.h"
#include "netinet/if_ether.h"
#include "fcntl.h"
#include "cyg/io/eth/netdev.h"
#include "net/if.h"
#include "net/wireless.h"
#include "wbtypes.h"
#include "wb_syslib_addon.h"
#include "lib_videophone.h"
#include "librtspserver.h"

#define MP4

#define STACK 32*1024

#define BUFFER_SIZE 100*1024
__align(32) CHAR g_DataBuffer[BUFFER_SIZE];

cyg_sem_t video_sem0, video_sem1, audio_sem0, audio_sem1;
unsigned char data_stack[STACK];
cyg_thread data_thread;
cyg_handle_t data_handle;

int len = 0;

static const char *g_RtspServer_conf = 
"FrameHeap 6; # Appropriate for one video stream and one audio stream\n"
"Port 554;\n"
"Input V4L {\n"
"	Device /dev/video0;\n"
"	InputType webcam;\n"
"	FrameRate auto;#30;\n"
"	Output \"raw\";\n"
"}\n"
"Input OSS {\n"
"	Device /dev/dsp;\n"
"	SampleRate 8000;\n"
"	Output \"%s\";\n"	//IMAADPCM, AMR, ADPCM
"}\n"
"Encoder MPEG4 {\n"
"	Input \"raw\";\n"
"	Output \"%s\";\n"	//Mpeg4, H263
"	Bitrate 200;\n"
"}\n"
"RTSP-Handler Live {\n"
"	Path /webcam;\n"
"	Path /webcam \"RtspServer\" \"test\" \"test\";\n"
"	Track \"%s\";\n"	//Mpeg4, H263
"	Track \"%s\";\n"	//IMAADPCM, AMR, ADPCM
"}\n";

void set_encoderenableFunc()
{
	//wb702EnableEncoder(TRUE);
	wb702EnableMP4Encoder(TRUE);
}

void set_encoderdisableFunc()
{
	//wb702EnableEncoder(FALSE);
	wb702EnableMP4Encoder(FALSE);
}

void get_back_video(unsigned char* input_buf,int *input_len)
{
	cyg_semaphore_wait(&video_sem0);
	*input_len = len;
	memcpy((char*)(input_buf), (char*)(g_DataBuffer), len);
	cyg_semaphore_post(&video_sem1);
}

void get_back_audio(unsigned char* input_buf,int *input_len)
{
	cyg_semaphore_wait(&audio_sem0);
	*input_len = len;
	memcpy((char*)(input_buf), (char*)(g_DataBuffer), len);
	cyg_semaphore_post(&audio_sem1);
}

void ReadData(cyg_addrword_t data)
{	
	int iRt;
	INT	mediatype = -1;
	IO_THREAD_READ_T readarg;
	//readarg.txbuf = (unsigned char*)g_DataBuffer;
	while(1)
	{
		//readarg.txlen = BUFFER_SIZE;
		iRt = iothread_Read(&readarg);
		
		if(iRt == -1)
		{
			printf("Encode Core quit\n");
			iothread_Read_Complete(&readarg);
		}
		else if(iRt == 0)
		{
			printf("%d buffer too small\n", readarg.mediatype);
			iothread_Read_Complete(&readarg);
			continue;
		}
		else
		{
			if(readarg.txlen<=BUFFER_SIZE)
			{
				memcpy(g_DataBuffer,readarg.txbuf,readarg.txlen);
				len = readarg.txlen;
				mediatype = readarg.mediatype;
				iothread_Read_Complete(&readarg);
			}
			else
			{
				diag_printf("len is too long\n");
				continue;
			}
			switch(mediatype)
			{
				case MEDIA_TYPE_VIDEO:
					//printf("get video, len=%d\n", len);
					cyg_semaphore_post(&video_sem0);
					cyg_semaphore_wait(&video_sem1);
					break;
				case MEDIA_TYPE_AUDIO:
					//printf("get audio, len=%d\n", len);
					cyg_semaphore_post(&audio_sem0);
					cyg_semaphore_wait(&audio_sem1);
					break;
				case MEDIA_TYPE_JPEG:
					printf("get jpeg, len=%d\n", len);
					break;
				case MEDIA_TYPE_MOTION:
					printf("get motion\n");
					break;
				default:
					printf("unknown type %d\n",mediatype);
					break;
			}
		}
	}
}
	
void ReadData_Run()
{	

	cyg_thread_create(10, &ReadData, NULL, "readdata", data_stack,
                        STACK, &data_handle, &data_thread);
    if ( data_handle == NULL)
    {
        printf("Thread for ftp creation failed!\n");
        exit(-1);
    }
    
    cyg_thread_resume(data_handle);
}

void initlcm()
{
	wb702SetVideoBitRate(128*1024);
    wb702SetLocalVideoSource(352, 288, CMD_ROTATE_R180);

    wb702SetFrame(WB_READ_FRAME);
#ifdef MP4    
	wb702SetAudioType(CMD_AUDIO_IMA_ADPCM);
    wb702SetVideoFormat(CMD_VIDEO_MP4);
#else
	wb702SetAudioType(CMD_AUDIO_AMR);
    wb702SetVideoFormat(CMD_VIDEO_H263);
#endif
    //wb702EnableMP4Encoder(TRUE);
    //wb702EnableEncoder(FALSE);
    wb702EnableEncoder(TRUE);
    ReadData_Run();
}

static void set_sockaddr(struct sockaddr_in *sin, unsigned long addr, unsigned short port)
{
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = addr;
	sin->sin_port = port;
}
int SetGateway(int fd, char *pcInterface, unsigned long ulIP)
{
	//struct rtentry rItem;
	struct ecos_rtentry rItem;
	//int fd;
	unsigned ulOldGateway;
	int rt;

	//if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;

	memset(&rItem, 0, sizeof(rItem));

	set_sockaddr((struct sockaddr_in *) &rItem.rt_dst, inet_addr("10.130.0.0"), 0);
	set_sockaddr((struct sockaddr_in *) &rItem.rt_genmask, inet_addr("255.255.0.0"), 0);
	rItem.rt_flags = RTF_UP;// | RTF_HOST;//RTF_GATEWAY;

	rItem.rt_dev = pcInterface;

	set_sockaddr((struct sockaddr_in *) &rItem.rt_gateway, ulIP, 0);

	if ((rt = ioctl(fd,SIOCADDRT, &rItem)) < 0)
	{
		fprintf(stderr, "Cannot add default route (%d).\n", errno);
	}
	//close(fd);
	return (rt < 0 ? FALSE : TRUE);
}

bool set_lo_address(int fd)
{
	struct ifreq ifr;
	struct sockaddr_in *pAddr;
	int rt;
	/* Start lo */
	strcpy(ifr.ifr_name, "lo0");
	
    
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
    	diag_printf("get lo flags error\n");
    }
    else {
    	if(ifr.ifr_flags & IFF_UP) diag_printf("lo up\n");
    	if(ifr.ifr_flags & IFF_RUNNING) diag_printf("lo running\n");
    }

	ifr.ifr_flags = IFF_UP | IFF_RUNNING;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
        diag_printf("SIOCSIFFLAGS error %x\n", errno);
        return false;
    }
   
    pAddr = (struct sockaddr_in *)&(ifr.ifr_addr);
	bzero(pAddr, sizeof(struct sockaddr_in));
	pAddr->sin_addr.s_addr = inet_addr("127.0.0.1");
	pAddr->sin_family = AF_INET;
	ioctl(fd, SIOCSIFADDR, &ifr);
	//if (ioctl(fd, SIOCSIFADDR, &ifr) < 0)
	//{
	//	fprintf(stderr,"Set lo Ip Address error\n");
		//close(fd);
		//return false;
	//}

	pAddr = (struct sockaddr_in *)&(ifr.ifr_addr);
	bzero(pAddr, sizeof(struct sockaddr_in));
	pAddr->sin_addr.s_addr = inet_addr("255.0.0.0");
	pAddr->sin_family = AF_INET;
	if (ioctl(fd, SIOCSIFNETMASK, &ifr) < 0)
	{
		fprintf(stderr,"Set lo netmask error\n");
		close(fd);
		return false;
	}

    // if (ioctl(fd, SIOCGIFADDR, &ifr) >= 0)
    // 	diag_printf("ipaddr Address====%s\n",inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));  
    {
    struct ecos_rtentry rItem;
	//int fd;
	unsigned ulOldGateway;
	

	//if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;

	memset(&rItem, 0, sizeof(rItem));

	set_sockaddr((struct sockaddr_in *) &rItem.rt_dst, inet_addr("127.0.0.0"), 0);
	set_sockaddr((struct sockaddr_in *) &rItem.rt_genmask, inet_addr("255.0.0.0"), 0);
	rItem.rt_flags = RTF_UP;// | RTF_HOST;//RTF_GATEWAY;

	rItem.rt_dev = "lo0";

	set_sockaddr((struct sockaddr_in *) &rItem.rt_gateway, 0, 0);

	if ((rt = ioctl(fd,SIOCADDRT, &rItem)) < 0)
	{
		fprintf(stderr, "Cannot add default route (%d).\n", errno);
	}
	}
	//close(fd);
	return (rt < 0 ? FALSE : TRUE);
   // return true;
}

bool set_ip_address(char *pcInterface, unsigned long ulIP, unsigned long ulNetmask)
{
	struct ifreq ifr;
	int fd;
	struct sockaddr_in *pAddr;
	int i;
	char mac[] = {0x00, 0x02, 0x13, 0x1a, 0x44, 0x78};

	if (pcInterface == NULL)
	{
		fprintf(stderr, "illegal call function SetGeneralIP!\n");
		return false;
	}

	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return false;
	
	strcpy(ifr.ifr_name, pcInterface);
	ifr.ifr_flags = IFF_UP | IFF_BROADCAST | IFF_RUNNING;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr)) {
        diag_printf("SIOCSIFFLAGS error %x\n", errno);
        return false;
    }

	pAddr = (struct sockaddr_in *)&(ifr.ifr_addr);
	bzero(pAddr, sizeof(struct sockaddr_in));
	pAddr->sin_addr.s_addr = ulIP;
	pAddr->sin_family = AF_INET;
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
	if (ioctl(fd, SIOCSIFNETMASK, &ifr) < 0)
	{
		fprintf(stderr,"Set netmask error\n");
		close(fd);
		return false;
	}
	
	SetGateway(fd, pcInterface, inet_addr("10.130.1.254"));
	//set_lo_address(fd);
	for(;i<5;i++)
		init_loopback_interface(i);
#if 0
	memcpy((void*)ifr.ifr_hwaddr.sa_data, mac,6);
	if (ioctl(fd, SIOCSIFHWADDR, &ifr) < 0)
    {
		fprintf(stderr,"Set netmask error\n");
		close(fd);
		return false;
	}
#endif
	close(fd);
	return true;
}
int SetWlanESSID(const char *pcWlanID)
{
	int rt;
	int fd;
	struct iwreq wrq;
	char ac[64];

//	diag_printf("_net_init = %x\n", &_net_init);
	
	//set_ip_address("wlan0", inet_addr("10.130.249.144"),inet_addr("255.255.0.0"));
	
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;

	memset(&wrq, 0, sizeof(wrq));
	strcpy(wrq.ifr_name, "wlan0");

	if (pcWlanID == NULL)
	{//essid: any
		ac[0] = '\0';
		wrq.u.essid.flags = 0;
	}
	else
	{
		strncpy(ac, pcWlanID, sizeof(ac));
		wrq.u.essid.flags = 1;
	}

	wrq.u.essid.pointer = (caddr_t)ac;
	wrq.u.essid.length = strlen(ac) + 1;
	

	if((rt = ioctl(fd, SIOCSIWESSID, &wrq)) < 0)
	{
		diag_printf("scan ESSID TP_LINK failed\n");
	}
	diag_printf("set ssid finished\n");
	
	if((rt = ioctl(fd, SIOCGIWRATE, &wrq)) < 0)
	{
		diag_printf("get Tx rate failed\n");
	}
	else
		diag_printf("get Tx rate %x\n", wrq.u.bitrate);
	
	close(fd);
	
	//init_all_network_interfaces();
	
	set_ip_address("wlan0", inet_addr("10.132.11.1"),inet_addr("255.255.0.0"));
	
	return (rt >= 0 ? TRUE : FALSE);
}

	
int main(int argc, char** argv)
{
	char p[512];
	char *server_mem;
	char *client_mem;
	int client_size;
	int server_size;
	force_net_dev_linked();
	cyg_do_net_init();
	SetWlanESSID("zhuna");
#ifdef MP4	
	sprintf(p, g_RtspServer_conf, "IMAADPCM", "Mpeg4", "Mpeg4", "IMAADPCM");
#else
	sprintf(p, g_RtspServer_conf, "AMR", "H263", "H263", "AMR");	
#endif	
	set_config(p);
	cyg_semaphore_init(&video_sem0, 0);
	cyg_semaphore_init(&video_sem1, 0);
	cyg_semaphore_init(&audio_sem0, 0);
	cyg_semaphore_init(&audio_sem1, 0);
	
	cyg_interrupt_disable();
	cyg_interrupt_disable();
	cyg_interrupt_disable();
	
	cyg_interrupt_enable();
		
	set_auth_id("jfyan","jfyanpass");
	set_auth_id("jfyan1","jfyan1");
	set_auth_id("jfyan2","jfyan2");
	del_auth_id("jfyan1","jfyan1");
	//set_auth_disable();
	VideoPhoneInit();
	fmiSetFMIReferenceClock(112000);
	fmiSetSDOutputClockbykHz(18000);
	//FTH_Init();
	init_wbdevice(initlcm);
	set_encoderenable(set_encoderenableFunc);
	set_encoderdisable(set_encoderdisableFunc);
	init_get_video(get_back_video);
	init_get_audio(get_back_audio);
	server_size = get_server_size();
	server_mem = (char*)malloc(server_size);
	if(server_mem == NULL)
		printf("memory out\n");
	rtsp_server_init(server_mem, server_size);
	client_size = get_rtspmem_size(3);
	client_mem = (char*)malloc(client_size);
	if(client_mem == NULL)
		printf("memory out\n");
	rtsp_mem_init(client_mem, client_size);	
	RtspServerStart(2);
}


#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "network.h"
#include "pkgconf/libc_startup.h"
#include "netinet/if_ether.h"
#include "cyg/io/eth/netdev.h"
#include "net/wireless.h"
#include "stdarg.h"
#include "assert.h"
#include "sys/types.h"
#include "time.h"
#include "cyg/kernel/kapi.h"
#include "fcntl.h"
#include "time.h"
#include "memmgmt.h"
#include "wb_syslib_addon.h"
#include "libmail.h"


#define SENDER "nzhu@winbond.com"
#define RECEIVER "nzhu@winbond.com"
#define RECEIVERCC ""
#define RECEIVERBCC ""
#define SUBJECT "jpeg"
#define CONTEXTTEMPLATE "jpeg"
#define SERVER "10.130.10.38"
#define USER "nzhu"
#define PASS "6990116"

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

bool set_ip_address(char *pcInterface, unsigned long ulIP, unsigned long ulNetmask)
{
	struct ifreq ifr;
	int fd;
	struct sockaddr_in *pAddr;
	char mac[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};

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
	
	set_ip_address("wlan0", inet_addr("10.130.249.111"),inet_addr("255.255.0.0"));
	
	return (rt >= 0 ? TRUE : FALSE);
}

int main(int argc, char *argv[])
{
	int size;
	char* buffer = NULL;
	MAIL_MEM* mail_mem = NULL;
	force_net_dev_linked();
	cyg_do_net_init();
	SetWlanESSID("NETGEAR");

	size = get_mailmem_size(2);
	buffer = (char*)malloc(size);
	if(buffer == NULL)
	{
		printf("memory out\n");
		return FALSE;
	}
	memset(buffer, 0, size);	
	mail_mem_init(buffer, size);
	if(get_mail_mem(&mail_mem) == FALSE)
		return FALSE;
	printf("begin mail send!!!!!!");
	if(mailSend(SENDER, RECEIVER, RECEIVERCC, RECEIVERBCC, SUBJECT, CONTEXTTEMPLATE, 0, NULL, SERVER, USER, PASS, 0, mail_mem) == FALSE)
	{
		printf("sendMail error\n");
		return FALSE;
	}
	
	ret_mail_mem(mail_mem);
	printf("push ok");
	
	return TRUE;
}

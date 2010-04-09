#include "../../../../SysLib/Inc/wb_syslib_addon.h"
#include "../../../../libHIC_Host/Inc/hic_host.h"
#include "../../../libFunc_Through_HIC/Inc/FTH_Int.h"
#include "../../../libRemoteFunc/Inc/RemoteFunc.h"
#include "../../../libRemoteFunc/Inc/RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteFuncTest.h"

void simple_tcpserver(cyg_addrword_t pnetdata);
void simple_udpserver(cyg_addrword_t pnetdata);

typedef struct
{
	int		fd;
	int		iport;
	char	*pbuf;
}NET_DATA_T;

char g_LargeData[] =
{
#include "largedata.h"
};

#pragma arm section zidata = "non_init"
__align (32) CHAR g_RemoteNet_Buf[MEM_ALIGN_SIZE(RNT_BUFFER_LEN)];
__align (32) CHAR g_RemoteNet_Buf1[MAX_THREADS][MEM_ALIGN_SIZE(RNT_BUFFER_LEN)];
#pragma arm section zidata

__align (32) char thread_stack[MAX_THREADS][MEM_ALIGN_SIZE(STACK_SIZE)];
cyg_handle_t thread_handle[MAX_THREADS];
cyg_thread thread[MAX_THREADS];


int sysInit (void);

int main(void)
{
	/* <1> Enable flash. */
	sysFlushCache (I_D_CACHE);
	sysDisableCache ();
	sysEnableCache (CACHE_WRITE_BACK);
	sysInit();
	printf("REG_APLLCON=0x%x REG_UPLLCON=0x%x\n", inpw(REG_APLLCON), inpw(REG_UPLLCON));
	printf("REG_CLKCON=0x%x REG_CLKSEL=0x%x\n", inpw(REG_CLKCON), inpw(REG_CLKSEL));
	printf("REG_CLKDIV0=0x%x REG_CLKDIV1=0x%x\n", inpw(REG_CLKDIV0), inpw(REG_CLKDIV1));
	printf("REG_TICR0=%d\n", inpw(REG_TICR0));
	FTH_Init();
	
#if 0
	test_inet(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	test_gethost(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	test_socket(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	test_netread();
	test_netwrite();
	test_send();
	test_recv();
	test_recvmsg();
	test_sendmsg();
	test_recvfrom(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	test_sendto(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	test_netselect();
	test_getsockname();
	test_getsockopt(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	test_netfcntl(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	test_netioctl(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	test_netioctl_withbuf(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	test_wb740getgateway(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	test_wb740reboot(g_RemoteNet_Buf, RNT_BUFFER_LEN);
#endif
	test_inet(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	test_gethost(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	test_socket(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	test_netread();
	test_netwrite();
	test_send();
	test_recv();
	test_recvmsg();
	test_sendmsg();
	test_recvfrom(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	test_sendto(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	test_netselect();
	test_getsockname();
	test_getsockopt(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	test_netfcntl(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	test_netioctl(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	test_netioctl_withbuf(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	test_wb740getgateway(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	test_wb740reboot(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	
	// insert test program here
	//GetDevInterface();
	//simple_getgateway();
	//simple_ioctl_withbuf();
	//simple_udpserver();
	//simple_tcpserver();
	//simple_inet_addr();
	//simple_uname();
	//simple_gethostbyaddr();
	//simple_multi_thread();
	//wb740reboot(g_RemoteNet_Buf, RNT_BUFFER_LEN);
	return 0;
}

#if 0
cyg_tick_count_t tbegin, tend;
int icount = 0, totallen = 0;

int simple_multi_thread(void)
{
	NET_DATA_T netdata1, netdata2, netdata3, netdata4, netdata5, netdata6, netdata7;
	
	netdata1.iport = 40;
	netdata1.pbuf = (char*)NON_CACHE(g_RemoteNet_Buf1);
	netdata2.iport = 41;
	netdata2.pbuf = (char*)NON_CACHE(g_RemoteNet_Buf2);
	netdata3.iport = 42;
	netdata3.pbuf = (char*)NON_CACHE(g_RemoteNet_Buf3);
	netdata4.iport = 43;
	netdata4.pbuf = (char*)NON_CACHE(g_RemoteNet_Buf4);
	netdata5.iport = 44;
	netdata5.pbuf = (char*)NON_CACHE(g_RemoteNet_Buf5);
	netdata6.iport = 45;
	netdata6.pbuf = (char*)NON_CACHE(g_RemoteNet_Buf6);
	netdata7.iport = 46;
	netdata7.pbuf = (char*)NON_CACHE(g_RemoteNet_Buf7);
	cyg_thread_create(THREAD_PRIORITY, &simple_tcpserver, (cyg_addrword_t)&netdata1, "simple_tcpserver1",
					thread_stack1, STACK_SIZE, &thread_handle1, &thread1);
	cyg_thread_create(THREAD_PRIORITY, &simple_tcpserver, (cyg_addrword_t)&netdata2, "simple_tcpserver2",
					thread_stack2, STACK_SIZE, &thread_handle2, &thread2);
	cyg_thread_create(THREAD_PRIORITY, &simple_tcpserver, (cyg_addrword_t)&netdata3, "simple_tcpserver3",
					thread_stack3, STACK_SIZE, &thread_handle3, &thread3);
	cyg_thread_create(THREAD_PRIORITY, &simple_tcpserver, (cyg_addrword_t)&netdata4, "simple_tcpserver4",
					thread_stack4, STACK_SIZE, &thread_handle4, &thread4);
	cyg_thread_create(THREAD_PRIORITY, &simple_tcpserver, (cyg_addrword_t)&netdata5, "simple_tcpserver5",
					thread_stack5, STACK_SIZE, &thread_handle5, &thread5);
	cyg_thread_create(THREAD_PRIORITY, &simple_tcpserver, (cyg_addrword_t)&netdata6, "simple_tcpserver6",
					thread_stack6, STACK_SIZE, &thread_handle6, &thread6);
	//cyg_thread_create(THREAD_PRIORITY, &simple_tcpserver, (cyg_addrword_t)&netdata7, "simple_tcpserver7",
	//				thread_stack7, STACK_SIZE, &thread_handle7, &thread7);
	/*
	cyg_thread_create(THREAD_PRIORITY, &simple_udpserver, (cyg_addrword_t)&netdata1, "simple_udpserver1",
					thread_stack1, STACK_SIZE, &thread_handle1, &thread1);
	cyg_thread_create(THREAD_PRIORITY, &simple_udpserver, (cyg_addrword_t)&netdata2, "simple_udpserver2",
					thread_stack2, STACK_SIZE, &thread_handle2, &thread2);
	cyg_thread_create(THREAD_PRIORITY, &simple_udpserver, (cyg_addrword_t)&netdata3, "simple_udpserver3",
					thread_stack3, STACK_SIZE, &thread_handle3, &thread3);
	*/
	
	tbegin = cyg_current_time();
	cyg_thread_resume(thread_handle1);
	cyg_thread_resume(thread_handle2);
	cyg_thread_resume(thread_handle3);
	cyg_thread_resume(thread_handle4);
	cyg_thread_resume(thread_handle5);
	cyg_thread_resume(thread_handle6);
	//cyg_thread_resume(thread_handle7);
	
	while(1)
	{
		cyg_thread_delay(100000);
	}
}


#define MAXINTERFACES 10

struct dev_addr{
    unsigned char name[4];
    unsigned char ipaddr[4];
    unsigned char hwaddr[6];
    unsigned char netmask[4];
    unsigned char gwaddr[4];
};

struct dev_addr ipmc[MAXINTERFACES];

int GetDevInterface(void)
{
	int dev_num = 0;
	struct ifreq ifbuf[MAXINTERFACES/2]={0};
	struct ifconf ifc;
	int i,fd, ifnum=0;
	int iInterface;
	unsigned long ulGateway;


	dev_num = 1;

	memset(&ipmc, 0, sizeof(ipmc));
	memset(&ifc, 0, sizeof(ifc));

	fd = socket(AF_INET, SOCK_DGRAM, 0, g_RemoteNet_Buf, RNT_BUFFER_LEN);
	if (fd < 0) 
    {
		perror("socket");
		return fd;
    }

    ifc.ifc_len = sizeof(ifbuf);
    ifc.ifc_buf = (caddr_t)ifbuf;
    if(netioctl_withbuf(fd, SIOCGIFCONF, &ifc,sizeof(ifc), g_RemoteNet_Buf, RNT_BUFFER_LEN) < 0)
    {
        netclose(fd, g_RemoteNet_Buf, RNT_BUFFER_LEN);
        return -1;
    }
    ifnum = ifc.ifc_len/sizeof(struct ifreq);
    for(i=0;i<ifnum;i++)
         diag_printf("interface name are %s\n",ifbuf[i].ifr_name);

   //strcpy(ifbuf[0].ifr_name,"eth1");

    while(ifnum-- > 0)
    {
        if( strlen(ifbuf[ifnum].ifr_name) > 4 ||ifbuf[ifnum].ifr_name == NULL||
           strcmp(ifbuf[ifnum].ifr_name,"lo")==0||
            netioctl(fd, SIOCGIFFLAGS, &ifbuf[ifnum],sizeof(ifbuf[ifnum]), g_RemoteNet_Buf, RNT_BUFFER_LEN) < 0 ||
            !(ifbuf[ifnum].ifr_flags & IFF_UP) ||
            netioctl(fd, SIOCGIFADDR, &ifbuf[ifnum],sizeof(ifbuf[ifnum]), g_RemoteNet_Buf, RNT_BUFFER_LEN) < 0)
            continue;              

		diag_printf("if name ===%s\n",ifbuf[ifnum].ifr_name);
		
		/*Get the MAC Address*/
        if (netioctl(fd, SIOCGIFHWADDR, &ifbuf[ifnum],sizeof(ifbuf[ifnum]), g_RemoteNet_Buf, RNT_BUFFER_LEN) >= 0)
       		diag_printf("MAC Address====%d\n",ifbuf[ifnum].ifr_hwaddr.sa_data);
       	else
       		diag_printf("get mac addr error\n");
       	
		/*Get the Netmask*/
        if (netioctl(fd, SIOCGIFNETMASK, &ifbuf[ifnum],sizeof(ifbuf[ifnum]), g_RemoteNet_Buf, RNT_BUFFER_LEN) >= 0)
       		diag_printf("Netmask====%s\n",inet_ntoa( (((struct sockaddr_in *)&ifbuf[ifnum].ifr_netmask)->sin_addr), g_RemoteNet_Buf, RNT_BUFFER_LEN));    
       	else
       		diag_printf("get netmask error\n");

        /*Get the IP Address*/    
        if (netioctl(fd, SIOCGIFADDR, &ifbuf[dev_num], sizeof(ifbuf[dev_num]), g_RemoteNet_Buf, RNT_BUFFER_LEN) >= 0)
	        diag_printf("ipaddr Address====%s\n",inet_ntoa((((struct sockaddr_in *)&ifbuf[ifnum].ifr_addr)->sin_addr), g_RemoteNet_Buf, RNT_BUFFER_LEN));
	    else
	    	diag_printf("get if addr error\n");
	    
        /*Get the Gateway*/
		ulGateway = wb740getgateway(ifbuf[ifnum].ifr_name, g_RemoteNet_Buf, RNT_BUFFER_LEN);
		diag_printf("Gateway====%s\n",inet_ntoa(*(struct in_addr*)&ulGateway, g_RemoteNet_Buf, RNT_BUFFER_LEN));
        dev_num++;
    }

    netclose(fd, g_RemoteNet_Buf, RNT_BUFFER_LEN);
    return dev_num;
}


int simple_getgateway(void)
{
	unsigned long ulGateway;
	ulGateway = wb740getgateway("eth1", g_RemoteNet_Buf, RNT_BUFFER_LEN);
	printf("eth1 gateway=%s\n", inet_ntoa(*(struct in_addr*)&ulGateway, g_RemoteNet_Buf, RNT_BUFFER_LEN));
	ulGateway = wb740getgateway("eth0", g_RemoteNet_Buf, RNT_BUFFER_LEN);
	printf("eth0 gateway=%s\n", inet_ntoa(*(struct in_addr*)&ulGateway, g_RemoteNet_Buf, RNT_BUFFER_LEN));
	ulGateway = wb740getgateway("lo", g_RemoteNet_Buf, RNT_BUFFER_LEN);
	printf("lo gateway=%s\n", inet_ntoa(*(struct in_addr*)&ulGateway, g_RemoteNet_Buf, RNT_BUFFER_LEN));
	return true;
}

static inline void set_sockaddr(struct sockaddr_in *sin, int addr, short port) 
{ 
sin->sin_family = AF_INET; 
sin->sin_addr.s_addr = addr; 
sin->sin_port = port; 
}

int simple_ioctl_withbuf(void)
{
 	struct ifreq ifbuf[4]={0};
    struct ifconf ifc;
    int fd, ifnum=0;
    
    memset(&ifc, 0, sizeof(ifc));

	fd = socket(AF_INET, SOCK_DGRAM, 0, g_RemoteNet_Buf, RNT_BUFFER_LEN);
	ifc.ifc_len = sizeof(ifbuf);
	ifc.ifc_buf = (caddr_t)ifbuf;

	if(netioctl_withbuf(fd, SIOCGIFCONF, &ifc,sizeof(ifc), g_RemoteNet_Buf, RNT_BUFFER_LEN) < 0)
	{
		printf("simple_ioctl_withbuf(): SIOCGIFCONF error\n");
		return false;
	}
 	
 	for(ifnum = 0; ifnum < 4; ifnum++)
 	{
 		if(strlen(ifbuf[ifnum].ifr_name) != 0)
 			printf("simple_ioctl_withbuf(): SIOCGIFCONF's %d interface is %s\n", ifnum, ifbuf[ifnum].ifr_name);
 	}
 	
 	{
 		struct rtentry rtitem;
 		int dst, mask, gateway;
 		
 		memset(&rtitem, 0, sizeof(rtitem));
 		dst = inet_addr("0.0.0.0", g_RemoteNet_Buf, RNT_BUFFER_LEN);
 		mask = inet_addr("0.0.0.0", g_RemoteNet_Buf, RNT_BUFFER_LEN);
 		gateway = inet_addr("10.132.1.254", g_RemoteNet_Buf, RNT_BUFFER_LEN);
 		
 		set_sockaddr((struct sockaddr_in *) &rtitem.rt_dst, 0, 0);
		set_sockaddr((struct sockaddr_in *) &rtitem.rt_genmask, 0, 0);
		set_sockaddr((struct sockaddr_in *) &rtitem.rt_gateway, gateway, 0);

 		rtitem.rt_flags = RTF_UP | RTF_GATEWAY;
 		rtitem.rt_dev = "eth1";
 		
		printf("dst=%s ", inet_ntoa(((struct sockaddr_in*)&(rtitem.rt_dst))->sin_addr, g_RemoteNet_Buf, RNT_BUFFER_LEN));
		printf("mask=%s ", inet_ntoa(((struct sockaddr_in*)&(rtitem.rt_genmask))->sin_addr, g_RemoteNet_Buf, RNT_BUFFER_LEN));
		printf("gateway=%s ", inet_ntoa(((struct sockaddr_in*)&(rtitem.rt_gateway))->sin_addr, g_RemoteNet_Buf, RNT_BUFFER_LEN));
		printf("if=%s\n", rtitem.rt_dev);
		if(netioctl_withbuf(fd, SIOCDELRT, &rtitem,sizeof(rtitem), g_RemoteNet_Buf, RNT_BUFFER_LEN) < 0)
		{
			printf("simple_ioctl_withbuf(): SIOCDELRT error\n");
			return false;
		}
		printf("simple_ioctl_withbuf(): SIOCDELRT success...\n");
		cyg_thread_delay(500);
		if(netioctl_withbuf(fd, SIOCADDRT, &rtitem,sizeof(rtitem), g_RemoteNet_Buf, RNT_BUFFER_LEN) < 0)
		{
			printf("simple_ioctl_withbuf(): SIOCADDRT error\n");
			return false;
		}
		printf("simple_ioctl_withbuf(): SIOCADDRT success...\n");
 	}
 	
 	
 	netclose(fd, g_RemoteNet_Buf, RNT_BUFFER_LEN);
 	return true;

	
}

int simple_gethostbyaddr(void)
{
	struct in_addr inAddr;
	struct hostent *hp;
	
	if(inet_aton("10.132.11.10", &inAddr, g_RemoteNet_Buf, RNT_BUFFER_LEN) != 0)
	{
		hp = gethostbyaddr((char*)&inAddr, 4, AF_INET, g_RemoteNet_Buf, RNT_BUFFER_LEN);
		if(hp == NULL)
		{
			printf("gethostbyaddr error\n");
			return 1;
		}
		printf("gethostbyaddr: h_name=%s h_alias=%s h_addr_list0=%s\n", 
			hp->h_name, hp->h_aliases, inet_ntoa(*(struct in_addr*)hp->h_addr_list0, g_RemoteNet_Buf, RNT_BUFFER_LEN));
	}
	else
		printf("inet_aton error\n");
	
	return 0;
	
}

int simple_uname(void)
{
	struct hostent *hp;
	struct utsname myname;
	char hname[256];
	
	memset(hname, 0, 256);
	if(gethostname(hname, 256, g_RemoteNet_Buf, RNT_BUFFER_LEN) == -1)
	{
		printf("gethostname error\n");
		return 1;
	}
	printf("gethostname: hname=%s\n", hname);
	
	if(uname(&myname, g_RemoteNet_Buf, RNT_BUFFER_LEN)<0)
	{
		printf("uname error\n");
		return 1;
	}
	printf("uname: sysname=%s nodename=%s release=%s version=%s machine=%s\n", 
		myname.sysname, myname.nodename, myname.release, myname.version, myname.machine);
	
	if((hp = gethostbyname(myname.nodename, g_RemoteNet_Buf, RNT_BUFFER_LEN)) == NULL)
	{
		printf("gethostbyname error\n");
		return 1;
	}
	printf("gethostbyname: h_name=%s h_alias=%s h_addr_list0=%s\n", 
		hp->h_name, hp->h_aliases, inet_ntoa(*(struct in_addr*)hp->h_addr_list0, g_RemoteNet_Buf, RNT_BUFFER_LEN));
	return 0;
}

int simple_inet_addr(void)
{
	struct in_addr inAddr;
	
	inet_aton("192.168.0.1", &inAddr, g_RemoteNet_Buf, RNT_BUFFER_LEN);
	printf("%s\n", inet_ntoa(inAddr, g_RemoteNet_Buf, RNT_BUFFER_LEN));
	return 0;
}

#define BUFSIZE 53200

void simple_tcpserver(cyg_addrword_t pnetdata)
{
	int s, new_s, i, len;
	char *msg;
	struct sockaddr_in sa, r_sa;
	int r_sa_l = sizeof(r_sa);
	struct hostent *hp;
	
	struct sockaddr_in peername;
	int socklen;
	
	int threadid;
	int localcount = 0;
		
	int port = ((NET_DATA_T*)pnetdata)->iport;
	char *pbuf = ((NET_DATA_T*)pnetdata)->pbuf;
	
	//s = ((NET_DATA_T*)pnetdata)->fd;
	threadid = cyg_thread_self();
	
	msg = malloc(BUFSIZE);
	if(msg == NULL)
	{
		printf("simple_tcpserver(%d): malloc error\n", threadid);
		cyg_thread_exit();
	}
	
	printf("simple_tcpserver(%d): to gethostbyname\n", threadid);
	if((hp = gethostbyname(TEST_REMOTEFUNC_HOSTNAME, pbuf, RNT_BUFFER_LEN)) == NULL)
	{
		printf("simple_tcpserver(%d): gethostbyname error!!!!!!\n", threadid);
		cyg_thread_exit();
	}
	printf("simple_tcpserver(%d): gethostbyname ok\n", threadid);
	
	//memcpy(&(r_sa.sin_addr), hp->h_addr_list0, hp->h_length);
	r_sa.sin_addr.s_addr = inet_addr("10.132.11.10", pbuf, RNT_BUFFER_LEN);
	printf("simple_tcpserver(%d): final addr=%s\n", threadid, inet_ntoa(r_sa.sin_addr, pbuf, RNT_BUFFER_LEN));
	r_sa.sin_family = AF_INET;
	r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
	printf("simple_tcpserver(%d): receive from port %d\n", threadid, ntohs(r_sa.sin_port));
	
	if((s = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pbuf, RNT_BUFFER_LEN)) == -1)
	{
		printf("simple_tcpserver(%d): socket error!!!!!!\n", threadid);
		cyg_thread_exit();
	}
	printf("simple_tcpserver(%d): create socket success...\n", threadid);
	printf("simple_tcpserver(%d): socket=%d\n", threadid, s);
	
	/* Test for netfcntl & setsockopt */
	/*
	{
		int sock_opt = 1;

		// server socket is nonblocking
		if (netfcntl(s, F_SETFL, O_NONBLOCK, pbuf, RNT_BUFFER_LEN) == -1)
		{
			printf("netfcntl error\n");
			netclose(s, pbuf, RNT_BUFFER_LEN);
			return -1;
		}

		if ((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (void *) &sock_opt,
						sizeof(sock_opt), pbuf, RNT_BUFFER_LEN)) == -1)
		{
			printf("setsockopt error\n");
			netclose(s, pbuf, RNT_BUFFER_LEN);
			return -1;
		}
	}
	*/
	
	if(bind(s, (struct sockaddr*)&r_sa, sizeof(r_sa), pbuf, RNT_BUFFER_LEN) == -1)
	{
		printf("simple_tcpserver(%d): bind error!!!!!!\n", threadid);
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}
	printf("simple_tcpserver(%d): bind success...\n", threadid);

	/*
	{
		struct sockaddr_in sockname;
		int socklen;
		
		if(getsockname(s, (struct sockaddr*)&sockname, &socklen, pbuf, RNT_BUFFER_LEN) != -1)
		{
			printf("simple_tcpserver(%d): getsockname:sin_family=%d sin_port=%d sin_addr=%s socklen=%d\n", threadid, 
				sockname.sin_family, sockname.sin_port, inet_ntoa(sockname.sin_addr, pbuf, RNT_BUFFER_LEN), socklen);
		}
		else
		{
			printf("simple_tcpserver(%d): getsockname error\n", threadid);
			netclose(s, pbuf, RNT_BUFFER_LEN);
			cyg_thread_exit();
		}
	}
	*/
	
	if(listen(s, 10, pbuf, RNT_BUFFER_LEN) == -1)
	{
		printf("simple_tcpserver(%d): listen error\n", threadid);
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}
	printf("simple_tcpserver(%d): listen success...\n", threadid);
	
	while(1)
	{
		
		if((new_s = accept(s, (struct sockaddr*)&sa, (size_t*)&r_sa_l, pbuf, RNT_BUFFER_LEN)) == -1)
		{
			printf("simple_tcpserver(%d): accept error\n", threadid);
			netclose(s, pbuf, RNT_BUFFER_LEN);
			cyg_thread_exit();
		}
		printf("simple_tcpserver(%d): accept success...\n", threadid);
		
		{
			int val, vallen;
			
			vallen = sizeof(val);
			if(getsockopt(new_s, SOL_SOCKET, SO_RCVBUF, &val, &vallen, pbuf, RNT_BUFFER_LEN) < 0)
			{
				printf("get rcvbuf size error!!!!!!\n");
			}
			printf("recv window size = %d\n", val);
			vallen = sizeof(val);
			if(getsockopt(new_s, SOL_SOCKET, SO_SNDBUF, &val, &vallen, pbuf, RNT_BUFFER_LEN) < 0)
			{
				printf("get sndbuf size error!!!!!!\n");
			}
			printf("send window size = %d\n", val);
			
			vallen = sizeof(val);
			val = 50000;
			if(setsockopt(new_s, SOL_SOCKET, SO_RCVBUF, &val, vallen, pbuf, RNT_BUFFER_LEN) < 0)
			{
				printf("set rcvbuf size error!!!!!!\n");
			}
			if(setsockopt(new_s, SOL_SOCKET, SO_SNDBUF, &val, vallen, pbuf, RNT_BUFFER_LEN) < 0)
			{
				printf("set sndbuf size error!!!!!!\n");
			}
			
			vallen = sizeof(val);
			if(getsockopt(new_s, SOL_SOCKET, SO_RCVBUF, &val, &vallen, pbuf, RNT_BUFFER_LEN) < 0)
			{
				printf("get rcvbuf size error!!!!!!\n");
			}
			printf("new recv window size = %d\n", val);
			vallen = sizeof(val);
			if(getsockopt(new_s, SOL_SOCKET, SO_SNDBUF, &val, &vallen, pbuf, RNT_BUFFER_LEN) < 0)
			{
				printf("get sndbuf size error!!!!!!\n");
			}
			printf("new send window size = %d\n", val);
		}
		
		/*
		{
			if(getpeername(new_s, (struct sockaddr*)&peername, &socklen, pbuf, RNT_BUFFER_LEN) != -1)
			{
				printf("simple_tcpserver(%d): getpeername:sin_family=%d sin_port=%d sin_addr=%s socklen=%d\n", threadid, 
					peername.sin_family, peername.sin_port, inet_ntoa(peername.sin_addr, pbuf, RNT_BUFFER_LEN), socklen);
			}
			else
			{
				printf("simple_tcpserver(%d): getpeername error\n", threadid);
				netclose(new_s, pbuf, RNT_BUFFER_LEN);
				netclose(s, pbuf, RNT_BUFFER_LEN);
				cyg_thread_exit();
			}
		}
		*/
		
		i = 0;
		while(1)
		{
			/* Test recvmsg & sendmsg method */
			/*
			{
				struct msghdr msghdr_msg;
				struct iovec *piov;
				int j, n, copylen;
				int perIov_len;
				
				perIov_len = BUFSIZE/4;
				piov = malloc(4*sizeof(struct iovec));
				for(j = 0; j < 4; j++)
				{
					piov[j].iov_base = malloc(perIov_len);
					piov[j].iov_len = perIov_len;
				}
				msghdr_msg.msg_name = NULL;
				msghdr_msg.msg_namelen = 0;
				msghdr_msg.msg_iov = piov;
				msghdr_msg.msg_iovlen = 4;
				msghdr_msg.msg_control = NULL;
				msghdr_msg.msg_controllen = 0;
				msghdr_msg.msg_flags = 0;
				
				if((len = recvmsg(new_s, &msghdr_msg, 0, pbuf, RNT_BUFFER_LEN)) == -1)
				{
					printf("simple_tcpserver(%d): recvmsg error\n", threadid);
					break;
				}
				if(len == 0)
				{
					printf("simple_tcpserver(%d): recvmsg 0, connection broken!!!!!\n", threadid);
					break;
				}
				n = 0;
				for(j = 0; j < 4; j++)
				{
					//printf("simple_tcpserver(%d): recvmsg piov[%d].iovlen=%d\n", threadid, j, piov[j].iov_len);
					copylen = piov[j].iov_len < (len - n) ? piov[j].iov_len : (len - n);
					memcpy(&msg[n], piov[j].iov_base, copylen);
					n += copylen;
					if(n >= len) break;
				}
				msg[len] = '\0';
				//printf("simple_tcpserver(%d): recvmsg: %s\n", threadid, msg);
				
				len = 17408;
				memcpy(msg, g_LargeData, len);
				//len = sprintf(msg, "ack%d", i);
				n = 0;
				for(j = 0; j < 4; j++)
				{
					copylen = piov[j].iov_len < (len - n) ? piov[j].iov_len : (len - n);
					memcpy(piov[j].iov_base, &msg[n], copylen);
					piov[j].iov_len = copylen;
					//printf("simple_tcpserver(%d): sendmsg piov[%d].iov_len=%d\n", threadid, j, piov[j].iov_len);
					n += copylen;
					if(n >= len) break;
				}
				j++;
				for(; j < 4; j++) piov[j].iov_len = 0;
				
				if((len = sendmsg(new_s, &msghdr_msg, 0, pbuf, RNT_BUFFER_LEN)) == -1)
				{
					printf("simple_tcpserver(%d): sendmsg error\n", threadid);
					break;
				}
				if(len == 0)
				{
					printf("simple_tcpserver(%d): sendmsg 0, connection broken!!!!!\n", threadid);
					break;
				}
				for(j = 0; j < 4; j++)
				{
					free(piov[j].iov_base);
				}
				free(piov);
				i++;
			}
			*/
			
			/* Test recv & send method and fdset operation */
			/*
			{
				fd_set readset, exceptset;
				int maxfd;
				int written_bytes;
				int bytes_left;
				int reconnect;
				
				FD_ZERO(&readset);
				FD_SET(new_s, &readset);
				FD_ZERO(&exceptset);
				FD_SET(new_s, &exceptset);
				maxfd = new_s + 1;
				
				if(netselect(maxfd, &readset, NULL, &exceptset, NULL, pbuf, RNT_BUFFER_LEN) == -1)
				{
					printf("simple_tcpserver(%d): netselect error\n", threadid);
					continue;
				}
				
				if(FD_ISSET(new_s, &readset))
				{
					//printf("simple_tcpserver(%d): readset get\n", threadid);
				}
				else if(FD_ISSET(new_s, &exceptset))
				{
					printf("simple_tcpserver(%d): exceptset get\n", threadid);
					break;
				}
			
				if((len = recv(new_s, msg, BUFSIZE, 0, pbuf, RNT_BUFFER_LEN)) == -1)
				{
					printf("simple_tcpserver(%d): recv error\n", threadid);
					break;
				}
				if(len == 0)
				{
					printf("simple_tcpserver(%d): recv 0, connection broken!!!!!\n", threadid);
					break;
				}
				//printf("simple_tcpserver(%d): recv: %s\n", threadid, msg);
				
				//len = sprintf(msg, "ack%d", i);
				len = 40000;
				memcpy(msg, g_LargeData, len);
				bytes_left = len;
				reconnect = 0;
				while(bytes_left > 0)
				{
					written_bytes = send(new_s, msg+(len-bytes_left), bytes_left, 0, pbuf, RNT_BUFFER_LEN);
					if(written_bytes <= 0)
					{
						if(errno == EINTR)
						{
							written_bytes = 0;
						}
						else
						{
							printf("simple_tcpserver(%d): netwrite error, reconnect!!!!!\n", threadid);
							reconnect = 1;
							break;
						}
					}
					bytes_left -= written_bytes;
				}
				if(reconnect == 1) break;
				i++;
			}
			*/
			
			/* Test for netread & netwrite method */
			{
				int written_bytes;
				int bytes_left;
				int reconnect;
				
				if((len = netread(new_s, msg, BUFSIZE, pbuf, RNT_BUFFER_LEN)) == -1)
				{
					printf("simple_tcpserver(%d): netread error %d\n", threadid, errno);
					break;
				}
				if(len == 0)
				{
					printf("simple_tcpserver(%d): netread 0, connection broken, errno=%d!!!!!\n", threadid, errno);
					break;
				}
				msg[len] = '\0';
				//printf("simple_tcpserver(%d): len %d, netread %s\n", threadid, len, msg);
				//printf("simple_tcpserver(%d): len %d\n", threadid, len);
				
				//len = sprintf(msg, "ack%d", i);
				len = 40000;
				memcpy(msg, g_LargeData, len);
				bytes_left = len;
				reconnect = 0;
				while(bytes_left > 0)
				{
					written_bytes = netwrite(new_s, msg+(len-bytes_left), bytes_left, pbuf, RNT_BUFFER_LEN);
					if(written_bytes <= 0)
					{
						printf("errno=%d\n", errno);
						if(errno == EINTR)
						{
							written_bytes = 0;
						}
						else
						{
							printf("simple_tcpserver(%d): netwrite error, reconnect!!!!!\n", threadid);
							reconnect = 1;
							break;
						}
					}
					//printf("simple_tcpserver(%d): write %d\n", threadid, written_bytes);
					bytes_left -= written_bytes;
					if(bytes_left > 0)
					{
						printf("write %d < %d left %d\n", written_bytes, bytes_left+written_bytes, bytes_left);
					}
				}
				if(reconnect == 1) break;
				i++;
			}
			
			totallen += len;
			
			icount++;
			if((icount>=2000) && (icount%2000==0))
			{
				tend = cyg_current_time();
				printf("rate=%d rate1=%03fk\n", (int)(icount/((tend-tbegin)/100)), (float)(totallen/((tend-tbegin)/100)/1024));
			}
			
			localcount++;
			if(localcount > 100 && localcount % 100 == 0)
			{
				printf("simple_tcpserver: %d\n", port);
			}
		}
		
		printf("simple_tcpserver(%d): to close %d with 0x%x\n", threadid, new_s, pbuf);
		netclose(new_s, pbuf, RNT_BUFFER_LEN);
		printf("simple_tcpserver(%d): close connection ok\n", threadid);
	}
	
	netclose(s, pbuf, RNT_BUFFER_LEN);
	free(msg);
	cyg_thread_exit();
}


void simple_udpserver(cyg_addrword_t pnetdata)
{
	int s, new_s, i, len;
	char *msg;
	struct sockaddr_in sa = {0}, r_sa = {0};
	int r_sa_l = sizeof(r_sa);
	struct hostent *hp;
	
	int threadid;
		
	int port = ((NET_DATA_T*)pnetdata)->iport;
	char *pbuf = ((NET_DATA_T*)pnetdata)->pbuf;
	
	threadid = cyg_thread_self();
	
	msg = malloc(BUFSIZE);
	if(msg == NULL)
	{
		printf("simple_udpserver(%d): malloc error\n", threadid);
		cyg_thread_exit();
	}
	
	if((hp = gethostbyname(TEST_REMOTEFUNC_HOSTNAME, pbuf, RNT_BUFFER_LEN)) == NULL)
	{
		printf("simple_udpserver(%d): gethostbyname error\n", threadid);
		cyg_thread_exit();
	}
	printf("simple_udpserver(%d): gethostbyname ok\n", threadid);
	
	//memcpy(&(r_sa.sin_addr), hp->h_addr_list0, hp->h_length);
	r_sa.sin_addr.s_addr = htonl(INADDR_ANY);
	r_sa.sin_family = AF_INET;
	r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
	printf("simple_udpserver(%d): final addr=%s\n", threadid, inet_ntoa(r_sa.sin_addr, pbuf, RNT_BUFFER_LEN));
	printf("simple_udpserver(%d): receive from port %d\n", threadid, ntohs(r_sa.sin_port));
	
	if((s = socket(AF_INET, SOCK_DGRAM, PF_UNSPEC, pbuf, RNT_BUFFER_LEN)) == -1)
	{
		printf("simple_udpserver(%d): socket error\n", threadid);
		cyg_thread_exit();
	}
	printf("simple_udpserver(%d): create socket success...\n", threadid);
	
	if(bind(s, (struct sockaddr*)&r_sa, sizeof(r_sa), pbuf, RNT_BUFFER_LEN) == -1)
	{
		printf("simple_udpserver(%d): bind error\n", threadid);
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}
	printf("simple_udpserver(%d): bind success...\n", threadid);
	
	{
		struct sockaddr_in sockname;
		int socklen;
		
		if(getsockname(s, (struct sockaddr*)&sockname, &socklen, pbuf, RNT_BUFFER_LEN) != -1)
		{
			printf("simple_udpserver(%d): getsockname sin_family=%d sin_port=%d sin_addr=%s socklen=%d\n", threadid, 
				sockname.sin_family, ntohs(sockname.sin_port), inet_ntoa(sockname.sin_addr, pbuf, RNT_BUFFER_LEN), socklen);
		}
		else
		{
			printf("simple_udpserver(%d): getsockname error\n", threadid);
			netclose(s, pbuf, RNT_BUFFER_LEN);
			cyg_thread_exit();
		}
	}
	
	i = 0;
	while(1)
	{
		if((len = recvfrom(s, msg, BUFSIZE, 0, (struct sockaddr*)&sa, &r_sa_l, pbuf, RNT_BUFFER_LEN)) == -1)
		{
			printf("simple_udpserver(%d): recvfrom error\n", threadid);
			continue;
		}
		if(len == 0)
		{
			printf("simple_udpserver(%d): recvfrom 0, connection broken!!!!!\n", threadid);
			continue;
		}
		msg[len] = '\0';
		//printf("simple_udpserver(%d): recvfrom(%s:%d): %s\n", threadid, inet_ntoa(sa.sin_addr, pbuf, RNT_BUFFER_LEN), ntohs(sa.sin_port), msg);
		
		//len = sprintf(msg, "ack%d", i);
		len = 40000;
		memcpy(msg, g_LargeData, len);
		if(sendto(s, msg, len, 0,  (struct sockaddr*)&sa, r_sa_l, pbuf, RNT_BUFFER_LEN) == -1)
		{
			printf("simple_udpserver(%d): sendto error\n", threadid);
			continue;
		}
		if(len == 0)
		{
			printf("simple_udpserver(%d): sendto 0, connection broken!!!!!\n", threadid);
			continue;
		}
		i++;
	}
	
	netclose(s, pbuf, RNT_BUFFER_LEN);
	
	free(msg);
	cyg_thread_exit();
}

#endif

int sysInit (void)
{
	/* <0> Save threads debug entry. */
#ifndef ECOS
	{
		extern UCHAR ASM_THREADS_DUMP_FUN[];	//A buffer in wb_init.s
		UINT32 uThreadDebugFun_Addr = (UINT32) &tt_dump_threads;
		memcpy (ASM_THREADS_DUMP_FUN, &uThreadDebugFun_Addr, sizeof (UINT32));
	}
#endif
	
	/* <1> Enable flash. */
	sysFlushCache (I_D_CACHE);
	sysDisableCache ();
	sysEnableCache (CACHE_WRITE_BACK);

	/* <2> Disable almost all of the engines for safety. */
	outpw (REG_CLKCON, inpw (REG_CLKCON) | CPUCLK_EN | APBCLK_EN | HCLK2_EN);
	outpw (REG_CLKSEL, SYSTEM_CLK_FROM_XTAL);
	outpw (REG_APLLCON, PLL_OUT_OFF);
	outpw (REG_UPLLCON, PLL_OUT_OFF);
	
	/* <3> Initialize UART. */
	/*
	{
		WB_UART_T uart;
		
		TURN_ON_UART_CLOCK;
		uart.uiFreq = OPT_XIN_CLOCK;
		uart.uiBaudrate = 57600;
		uart.uiDataBits = WB_DATA_BITS_8;
		uart.uiStopBits = WB_STOP_BITS_1;
		uart.uiParity = WB_PARITY_NONE;
		uart.uiRxTriggerLevel = LEVEL_1_BYTE;
		sysInitializeUART(&uart);
		printf ("UART ok\n");
	}
	*/

	/* <4> Set PLL. */
	// SDRAM MCLK automatically on/off
	outpw(REG_SDICON, inpw(REG_SDICON) & 0xBFFFFFFF);
	// Force MPEG4 transfer length as 8 bytes
	outpw(REG_BBLENG1, (inpw(REG_BBLENG1) & 0xFFFFFFF8) | 0x2);
	
	// CLKDIV0
	// Bit 1~0 : APB = HCLK1/?  -------------------------+
	// Bit15~8 : VPOST = PLL/?  -----------------------+ |
	// Bit19~16: USB = PLL/?    ---------------------+ | |
	// Bit23~20: Audio = PLL/?  --------------------+| | |
	// Bit27~24: Sensor = PLL/? -------------------+|| | |
	// Bit31~28: System = PLL/? ------------------+||| | |
	                                         //   |||| | | 
	                                         //   ||||++ |
//	outpw(REG_CLKDIV0, inpw(REG_CLKDIV0)&0x0fc|0x01322302);	//(ok for 108)
#ifndef ECOS
	outpw(REG_CLKDIV0, inpw(REG_CLKDIV0)&0x0fc|0x25362401);	//(ok for 336(112))
#else
	//outpw(REG_CLKDIV0, inpw(REG_CLKDIV0)&0xf00000fc|0x05362401);	//can't modify system clock
	outpw(REG_CLKDIV0, inpw(REG_CLKDIV0)&0x0fc|0x25362401);	//(ok for 336(112))
#endif
//	outpw(REG_CLKDIV0, inpw(REG_CLKDIV0)&0x0fc|0x03331802);	//(ok for 144)

	//Adjust SDRAM
	printf("setting apll to 0x%x, upll to 0x%x\n", pllEvalPLLCON (OPT_XIN_CLOCK, OPT_APLL_OUT_CLOCK), pllEvalPLLCON (OPT_XIN_CLOCK, OPT_UPLL_OUT_CLOCK));
	outpw(REG_APLLCON, pllEvalPLLCON (OPT_XIN_CLOCK, OPT_APLL_OUT_CLOCK));
	outpw(REG_UPLLCON, pllEvalPLLCON (OPT_XIN_CLOCK, OPT_UPLL_OUT_CLOCK));

	                 //  +-------- Bit31~28 : HCLK (from system)
	                 //  |+------- Bit27~24 : CPU (from HCLK)
	                 //  ||+------ Bit23~20 : HCLK1 (from HCLK, must less or equal to HCLK and CPU)
	                 //  |||+----- Bit19~16 : HCLK2 (from HCLK)
	                 //  ||||+---- Bit15~12 : MPEG4, JPEG (from HCLK2)
	                 //  |||||+--- Bit11~8  : 2D, VPE (from HCLK2)
	                 //  ||||||+-- Bit 7~6  : DSP (from HCLK2)
	                 //  |||||||+- Bit 5~0  : Reserved.
//	outpw(REG_CLKDIV1, 0x00105045);	//(ok for 108)
	outpw(REG_CLKDIV1, 0x00105045);	//(ok for 112)
//	outpw(REG_CLKDIV1, 0x00110005);
#ifndef ECOS	//don't need to modify system clock source
	outpw(REG_CLKSEL, SYSTEM_CLK_FROM_UPLL);
#endif
printf("before system clock\n");
	outpw(REG_CLKSEL, SYSTEM_CLK_FROM_UPLL);
printf("system clock ok\n");

	/* <5> Initialize TinyThread to support multi-thread. */
#ifndef ECOS
	tt_init (OPT_XIN_CLOCK, OPT_UPLL_OUT_CLOCK);
#else
	//tt_init (OPT_XIN_CLOCK, OPT_UPLL_OUT_CLOCK);
	printf("reg_tcr0=0x%x reg_ticr0=0x%x\n", inpw(REG_TCR0), inpw(REG_TICR0));
	printf("reg_tcr1=0x%x reg_ticr1=0x%x\n", inpw(REG_TCR1), inpw(REG_TICR1));
#endif

	/* <6> Set other engine clocks. */

	/* Set vpost clock:
	Why I can NOT call it after HIC is transferred. */
	outpw (REG_CLKSEL, inpw (REG_CLKSEL) | VIDEO_CLK_FROM_UPLL);
	tt_msleep (30);
	TURN_ON_VPOST_CLOCK;
	tt_msleep (30);

	/* Set audio clock. */
	outpw (REG_CLKSEL, inpw (REG_CLKSEL) | AUDIO_CLK_FROM_APLL);
	tt_msleep (30);
	TURN_ON_AUDIO_CLOCK;
	tt_msleep (30);
	
	/* Set sensor clock. */
	outpw (REG_CLKSEL, inpw (REG_CLKSEL) | SENSOR_CLK_FROM_UPLL);
	//tt_msleep (30);
	//TURN_ON_DSP_CLOCK;
	//tt_msleep (30);
	//TURN_ON_CAP_CLOCK;

	/* Set MP4 clock. */
	//TURN_ON_MPEG4_CLOCK;
	//tt_msleep (30);
	
	/* Set VPE and 2D clock. */
	TURN_ON_VPE_CLOCK;
	tt_msleep (30);
	TURN_ON_GFX_CLOCK;
	tt_msleep (30);
	
	/* Set USB clock */
#if 0	
	writew(REG_SYS_CFG, readw(REG_SYS_CFG)&0xffffffbf);
	outpw (REG_CLKSEL, inpw (REG_CLKSEL) | USB_CLK_FROM_UPLL);
	tt_msleep (30);
	TURN_ON_USB_CLOCK;
	outpw(REG_GPIO_PE,inpw(REG_GPIO_PE)|0x00000200);//GPIO9,disable pull-up or pull-down  
#endif
	
	return 0;
}



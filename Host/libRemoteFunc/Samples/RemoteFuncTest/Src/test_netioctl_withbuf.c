#include "wb_syslib_addon.h"
#include "hic_host.h"
#include "FTH_Int.h"
#include "RemoteFunc.h"
#include "RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteFuncTest.h"

void test_netioctl_withbuf(char *pBuf, int iBufLen)
{
	test_printf_begin("test_netioctl_withbuf");
	test_netioctl_withbuf_entry(pBuf, iBufLen);
	test_printf_end("test_netioctl_withbuf");
}

static inline void set_sockaddr(struct sockaddr_in *sin, int addr, short port) 
{ 
	sin->sin_family = AF_INET; 
	sin->sin_addr.s_addr = addr; 
	sin->sin_port = port; 
}

void test_netioctl_withbuf_entry(char *pBuf, int iBufLen)
{
	struct rtentry rtitem;
	int dst, mask, gateway;
    int i, fd;
    
	fd = socket(AF_INET, SOCK_DGRAM, 0, pBuf, iBufLen);
	if(fd < 0)
	{
		test_printf_error("test_netioctl_withbuf_entry");
		return;
	}
 		
	memset(&rtitem, 0, sizeof(rtitem));
	if(inet_aton(TEST_NETIOCTL_WITH_BUF_DST, (struct in_addr*)&dst, pBuf, iBufLen) == 0)
	{
		test_printf_error("test_netioctl_withbuf_entry");
 		netclose(fd, pBuf, iBufLen);
		return;
	}
	if(inet_aton(TEST_NETIOCTL_WITH_BUF_MASK, (struct in_addr*)&mask, pBuf, iBufLen) == 0)
	{
		test_printf_error("test_netioctl_withbuf_entry");
 		netclose(fd, pBuf, iBufLen);
		return;
	}
	if(inet_aton(TEST_NETIOCTL_WITH_BUF_GATEWAY, (struct in_addr*)&gateway, pBuf, iBufLen) == 0)
	{
		test_printf_error("test_netioctl_withbuf_entry");
 		netclose(fd, pBuf, iBufLen);
		return;
	}
 		
	set_sockaddr((struct sockaddr_in *) &rtitem.rt_dst, dst, 0);
	set_sockaddr((struct sockaddr_in *) &rtitem.rt_genmask, mask, 0);
	set_sockaddr((struct sockaddr_in *) &rtitem.rt_gateway, gateway, 0);

	rtitem.rt_flags = RTF_UP | RTF_GATEWAY;
	rtitem.rt_dev = "eth1";
 		
	if(netioctl_withbuf(fd, SIOCDELRT, &rtitem, sizeof(rtitem), pBuf, iBufLen) < 0)
	{
		test_printf_error("test_netioctl_withbuf_entry");
 		netclose(fd, pBuf, iBufLen);
		return;
	}
	
	/*
	printf("To check with \"route\" command in 5 seconds");
	for(i = 0; i < 10; i++)
	{
		printf(".");
		tt_msleep(1000);
	}
	printf("\n");
	*/
	
	if(netioctl_withbuf(fd, SIOCADDRT, &rtitem,sizeof(rtitem), pBuf, iBufLen) < 0)
	{
		test_printf_error("test_netioctl_withbuf_entry");
 		netclose(fd, pBuf, iBufLen);
		return;
	}
 	
	test_printf_success("test_netioctl_withbuf_entry");
 	netclose(fd, pBuf, iBufLen);
}

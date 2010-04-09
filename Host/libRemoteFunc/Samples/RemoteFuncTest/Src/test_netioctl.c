#include "wb_syslib_addon.h"
#include "hic_host.h"
#include "FTH_Int.h"
#include "RemoteFunc.h"
#include "RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteFuncTest.h"

void test_netioctl(char *pBuf, int iBufLen)
{
	test_printf_begin("test_netioctl");
	test_netioctl_entry(pBuf, iBufLen);
	test_printf_end("test_netioctl");
}

void test_netioctl_entry(char *pBuf, int iBufLen)
{
	int fd;
	struct ifreq ifbuf;
	int newip, newnetmask;
	int ip, netmask;
	
	fd = socket(AF_INET, SOCK_STREAM, 0, pBuf, iBufLen);
	if(fd < 0)
	{
		test_printf_error("test_netioctl_entry");
		return;
	}
	
	if(inet_aton(TEST_NETIOCTL_IP_ADDR, (struct in_addr*)&newip, pBuf, iBufLen) < 0)
	{
		test_printf_error("test_netioctl_entry");
		netclose(fd, pBuf, iBufLen);
		return;
	}
	if(inet_aton(TEST_NETIOCTL_IP_NETMASK, (struct in_addr*)&newnetmask, pBuf, iBufLen) < 0)
	{
		test_printf_error("test_netioctl_entry");
		netclose(fd, pBuf, iBufLen);
		return;
	}
	
	memset(&ifbuf, 0, sizeof(ifbuf));
	strcpy(ifbuf.ifr_name, TEST_NETIOCTL_INTERFACE_NAME);
	((struct sockaddr_in*)&(ifbuf.ifr_addr))->sin_addr.s_addr = newip;
	((struct sockaddr_in*)&(ifbuf.ifr_addr))->sin_family = AF_INET;
	if(netioctl(fd, SIOCSIFADDR, &ifbuf, sizeof(ifbuf), pBuf, iBufLen) < 0)
	{
		test_printf_error("test_netioctl_entry");
		netclose(fd, pBuf, iBufLen);
		return;
	}
	((struct sockaddr_in*)&(ifbuf.ifr_addr))->sin_addr.s_addr = newnetmask;
	if(netioctl(fd, SIOCSIFNETMASK, &ifbuf, sizeof(ifbuf), pBuf, iBufLen) < 0)
	{
		test_printf_error("test_netioctl_entry");
		netclose(fd, pBuf, iBufLen);
		return;
	}
	
	strcpy(ifbuf.ifr_name, TEST_NETIOCTL_INTERFACE_NAME);
	if(netioctl(fd, SIOCGIFADDR, &ifbuf, sizeof(ifbuf), pBuf, iBufLen) < 0)
	{
		test_printf_error("test_netioctl_entry");
		netclose(fd, pBuf, iBufLen);
		return;
	}
	ip = ((struct sockaddr_in*)&(ifbuf.ifr_addr))->sin_addr.s_addr;
	
	if(netioctl(fd, SIOCGIFNETMASK, &ifbuf, sizeof(ifbuf), pBuf, iBufLen) < 0)
	{
		test_printf_error("test_netioctl_entry");
		netclose(fd, pBuf, iBufLen);
		return;
	}
	netmask = ((struct sockaddr_in*)&(ifbuf.ifr_addr))->sin_addr.s_addr;
	
	if(ip != newip || netmask != newnetmask)
		test_printf_error("test_netioctl_entry");
	else
		test_printf_success("test_netioctl_entry");
	
}



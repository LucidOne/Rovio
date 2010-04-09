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

void test_netioctl_withbuf_entry(char *pBuf, int iBufLen)
{
	int fd;
	struct ifreq ifbuf;
	
	fd = socket(AF_INET, SOCK_STREAM, 0, pBuf, iBufLen);
	if(fd < 0)
	{
		test_printf_error("test_netioctl_withbuf_entry");
		return;
	}
		
	if(netioctl_withbuf(-1, SIOCSIFADDR, &ifbuf, sizeof(ifbuf), pBuf, iBufLen) >= 0)
	{
		test_printf_error("test_netioctl_withbuf_entry");
		netclose(fd, pBuf, iBufLen);
		return;
	}
	
	if(netioctl_withbuf(fd, SIOCSIFADDR, NULL, sizeof(ifbuf), pBuf, iBufLen) >= 0)
	{
		test_printf_error("test_netioctl_withbuf_entry");
		netclose(fd, pBuf, iBufLen);
		return;
	}
	
	if(netioctl_withbuf(fd, SIOCSIFADDR, &ifbuf, 0, pBuf, iBufLen) >= 0)
	{
		test_printf_error("test_netioctl_withbuf_entry");
		netclose(fd, pBuf, iBufLen);
		return;
	}
	
	if(netioctl_withbuf(fd, SIOCSIFADDR, &ifbuf, sizeof(ifbuf), NULL, iBufLen) >= 0)
	{
		test_printf_error("test_netioctl_withbuf_entry");
		netclose(fd, pBuf, iBufLen);
		return;
	}
	
	if(netioctl_withbuf(fd, SIOCSIFADDR, &ifbuf, sizeof(ifbuf), pBuf, 0) >= 0)
	{
		test_printf_error("test_netioctl_withbuf_entry");
		netclose(fd, pBuf, iBufLen);
		return;
	}
	
	test_printf_success("test_netioctl_withbuf_entry");
	netclose(fd, pBuf, iBufLen);
}



#include "wb_syslib_addon.h"
#include "hic_host.h"
#include "FTH_Int.h"
#include "RemoteFunc.h"
#include "RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteFuncTest.h"

void test_netfcntl(char *pBuf, int iBufLen)
{
	test_printf_begin("test_netfcntl");
	test_netfcntl_entry(pBuf, iBufLen);
	test_printf_end("test_netfcntl");
}

void test_netfcntl_entry(char *pBuf, int iBufLen)
{
	int s;
	
	if((s = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pBuf, iBufLen)) == -1)
	{
		test_printf_error("test_netfcntl_entry");
		return;
	}
	
	if(netfcntl(-1, 4, 0x800, pBuf, iBufLen) != -1)
	{
		test_printf_error("test_netfcntl_entry");
		netclose(s, pBuf, iBufLen);
		return;
	}
	
	if(netfcntl(s, 4, 0x800, NULL, iBufLen) != -1)
	{
		test_printf_error("test_netfcntl_entry");
		netclose(s, pBuf, iBufLen);
		return;
	}
	
	if(netfcntl(s, 4, 0x800, pBuf, 0) != -1)
	{
		test_printf_error("test_netfcntl_entry");
		netclose(s, pBuf, iBufLen);
		return;
	}
	
	test_printf_success("test_netfcntl_entry");
	netclose(s, pBuf, iBufLen);
}


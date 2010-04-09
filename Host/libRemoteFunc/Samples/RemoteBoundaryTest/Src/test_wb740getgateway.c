#include "wb_syslib_addon.h"
#include "hic_host.h"
#include "FTH_Int.h"
#include "RemoteFunc.h"
#include "RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteFuncTest.h"

void test_wb740getgateway(char *pBuf, int iBufLen)
{
	test_printf_begin("test_wb740getgateway");
	test_wb740getgateway_entry(pBuf, iBufLen);
	test_printf_end("test_wb740getgateway");
}

void test_wb740getgateway_entry(char *pBuf, int iBufLen)
{
	char strGateway[TEST_WB740GETGATEWAY_INTREFACE_LEN];
	int iGateway;
	
	iGateway = wb740getgateway(NULL, pBuf, iBufLen);
	if(iGateway != 0)
	{
		test_printf_error("test_wb740getgateway_entry");
		return;
	}
	
	iGateway = wb740getgateway(strGateway, NULL, iBufLen);
	if(iGateway != 0)
	{
		test_printf_error("test_wb740getgateway_entry");
		return;
	}
	
	iGateway = wb740getgateway(strGateway, pBuf, 0);
	if(iGateway != 0)
	{
		test_printf_error("test_wb740getgateway_entry");
		return;
	}
	
	test_printf_success("test_wb740getgateway_entry");
}



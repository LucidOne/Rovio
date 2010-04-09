#include "wb_syslib_addon.h"
#include "hic_host.h"
#include "FTH_Int.h"
#include "RemoteFunc.h"
#include "RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteFuncTest.h"

void test_wb740reboot(char *pBuf, int iBufLen)
{
	test_printf_begin("test_wb740reboot");
	test_wb740reboot_entry(pBuf, iBufLen);
	test_printf_end("test_wb740reboot");
}

void test_wb740reboot_entry(char *pBuf, int iBufLen)
{
	if(wb740reboot(NULL, iBufLen) != -1)
	{
		test_printf_error("test_wb740reboot_entry");
		return;
	}
	
	if(wb740reboot(pBuf, 0) != -1)
	{
		test_printf_error("test_wb740reboot_entry");
		return;
	}
	
	test_printf_success("test_wb740reboot_entry");
}



#include "stdio.h"
#include "stdlib.h"
#include "sys/stat.h"
#include "string.h"
#include "wb_syslib_addon.h"
//#include "wbfat.h"
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
//#include "wb_fmi.h"
#include "HttpServer.h"

#include "CUnit.h"
#include "error_handling.h"

void DbgLog (char *strModuleName, const char *strFormat, ...)
{
	va_list ap;
	
	va_start(ap, strFormat);
#if (UART_DBG_LEVEL < DBG_LEVEL_RELEASE)
	diag_printf("/* Error in %s */\n", strModuleName);
	vprintf(strFormat, ap);
#endif
	va_end(ap);
}


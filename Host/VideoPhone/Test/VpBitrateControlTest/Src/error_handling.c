#include "wb_syslib_addon.h"

#include "stdarg.h"
#include "lib_videophone.h"

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


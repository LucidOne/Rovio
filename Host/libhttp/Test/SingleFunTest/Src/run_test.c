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

static BOOL g_bDisableDebugMsg = FALSE;
void uart_write( const char *buf, size_t size )
{
#if 1
	if (!g_bDisableDebugMsg)
	{
		size_t i;
	
		for ( i = 0; i < size; i++ )
			hal_if_diag_write_char( buf[i] );
	}
#endif
}

/* Suites information */
int suite_init(void);
int suite_clean(void);

extern CU_TestInfo test_c_string[];

CU_SuiteInfo test_suites[] =
{
	{"Test C_String.c", suite_init, suite_clean, test_c_string},
	CU_SUITE_INFO_NULL
};


/* Suites initial and clean functions */
int suite_init(void)
{
	return 0;
}

int suite_clean(void)
{
	return 0;
}

int AddTests(void)
{
	ASSERT(CU_get_registry() != NULL);
	ASSERT(CU_is_test_running() == FALSE);
	
	if (CU_register_suites(test_suites) != CUE_SUCCESS)
	{
		DbgLog(__FILE__, "%d CU_register_suites failed - %s\n", __LINE__, CU_get_error_msg());
		return 1;
	}
	return 0;
}

int main (int argc, char **argv)
{
	CU_ErrorCode cuError;
	
	cuError = CU_initialize_registry();
	if (cuError == CUE_NOMEMORY)
	{
		DbgLog(__FILE__, "%d CU_initialize_registry failed\n", __LINE__);
		return 1;
	}
	
	if (AddTests() != 0)
	{
		DbgLog(__FILE__, "line:%d AddTests failed\n", __LINE__);
		return 1;
	}
	CU_console_run_tests();
	
	CU_cleanup_registry();
	return 0;
}




//#include "wb_fmi.h"
#include "lib_videophone.h"

#include "CUnit.h"
#include "error_handling.h"
#include "test_fun.h"

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

CU_SuiteInfo test_suites[] =
{
	{"test_registry_vp_bitrate_control_init.c", suite_init, suite_clean, test_registry_vp_bitrate_control_init},
	{"test_registry_vp_bitrate_control_uninit.c", suite_init, suite_clean, test_registry_vp_bitrate_control_uninit},
	{"test_registry_vp_bitrate_control_linkadd.c", suite_init, suite_clean, test_registry_vp_bitrate_control_linkadd},
	{"test_registry_vp_bitrate_control_linksearch.c", suite_init, suite_clean, test_registry_vp_bitrate_control_linksearch},
	{"test_registry_vp_bitrate_control_linkupdate.c", suite_init, suite_clean, test_registry_vp_bitrate_control_linkupdate},
	{"test_registry_vp_bitrate_control_linkrefresh.c", suite_init, suite_clean, test_registry_vp_bitrate_control_linkrefresh},
	{"test_registry_vp_bitrate_control_linkstat.c", suite_init, suite_clean, test_registry_vp_bitrate_control_linkstat},
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



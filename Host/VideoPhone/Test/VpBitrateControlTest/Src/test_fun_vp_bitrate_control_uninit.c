
//#include "wb_fmi.h"
#include "lib_videophone.h"

#include "CUnit.h"
#include "error_handling.h"

extern LinkTest *g_LinkTest;

/*
 * function:
 *		The code coverage is 100% in vp_bitrate_control_linksearch().
 *
 */
void test_vp_bitrate_control_uninit(void)
{
	BOOL initRt;
	
	initRt = vp_bitrate_control_init(1);
	CU_ASSERT_EQUAL(initRt, TRUE);
	
	vp_bitrate_control_linkadd(1, 100);
	vp_bitrate_control_linkadd(2, 200);
	
	initRt = vp_bitrate_control_uninit();
	CU_ASSERT_EQUAL(initRt, TRUE);
	CU_ASSERT_EQUAL(g_LinkTest, NULL);
}

CU_TestInfo test_registry_vp_bitrate_control_uninit[] =
{
	{"test_vp_bitrate_control_uninit", test_vp_bitrate_control_uninit},
	CU_TEST_INFO_NULL
};



#include "lib_videophone.h"

#include "CUnit.h"
#include "error_handling.h"

extern LinkTest *g_LinkTest;

/*
 * function:
 *		The code coverage is 75% in vp_bitrate_control_init().
 *		The memory out of malloc is not tested.
 *
 */
void test_vp_bitrate_control_init(void)
{
	BOOL initRt;
	
	initRt = vp_bitrate_control_init(1);
	CU_ASSERT_EQUAL(initRt, TRUE);
	
	CU_ASSERT_EQUAL((g_LinkTest->list).pNext, &(g_LinkTest->list));
	CU_ASSERT_EQUAL((g_LinkTest->list).pPrev, &(g_LinkTest->list));
	
	vp_bitrate_control_uninit();
}

CU_TestInfo test_registry_vp_bitrate_control_init[] =
{
	{"test_vp_bitrate_control_init", test_vp_bitrate_control_init},
	CU_TEST_INFO_NULL
};


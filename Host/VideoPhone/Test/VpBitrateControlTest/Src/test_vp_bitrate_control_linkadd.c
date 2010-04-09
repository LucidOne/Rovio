
//#include "wb_fmi.h"
#include "lib_videophone.h"

#include "CUnit.h"
#include "error_handling.h"

extern LinkTest *g_LinkTest;

/*
 * function:
 *		The code coverage is 75% in vp_bitrate_control_linkadd().
 *		The memory out of malloc is not tested.
 *
 */
void test_vp_bitrate_control_linkadd(void)
{
	BOOL rt;
	
	rt = vp_bitrate_control_init(1);
	CU_ASSERT_EQUAL(rt, TRUE);
	
	rt = vp_bitrate_control_linkadd(1, 100);
	CU_ASSERT_EQUAL(rt, TRUE);
	CU_ASSERT_EQUAL((g_LinkTest->list).pNext, (g_LinkTest->list).pPrev);
	CU_ASSERT_EQUAL(GetParentAddr(g_LinkTest->list.pNext, LinkSpeed, list)->fd, 1);
	CU_ASSERT_EQUAL(GetParentAddr(g_LinkTest->list.pNext, LinkSpeed, list)->iWriteBytes, 100);
	
	rt = vp_bitrate_control_uninit();
	CU_ASSERT_EQUAL(rt, TRUE);
	CU_ASSERT_EQUAL(g_LinkTest, NULL);
}

CU_TestInfo test_registry_vp_bitrate_control_linkadd[] =
{
	{"test_vp_bitrate_control_linkadd", test_vp_bitrate_control_linkadd},
	CU_TEST_INFO_NULL
};


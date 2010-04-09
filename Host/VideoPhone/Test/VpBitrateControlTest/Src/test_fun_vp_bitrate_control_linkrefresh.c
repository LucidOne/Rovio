#include "lib_videophone.h"

#include "CUnit.h"
#include "error_handling.h"

extern LinkTest *g_LinkTest;

/*
 * function:
 *		The code coverage is 100% in vp_bitrate_control_linkrefresh().
 *
 */
void test_vp_bitrate_control_linkrefresh(void)
{
	BOOL rt;
	LinkSpeed *pLinkSpeed = NULL;
	
	rt = vp_bitrate_control_init(1);
	CU_ASSERT_EQUAL(rt, TRUE);
	
	rt = vp_bitrate_control_linkadd(1, 100);
	CU_ASSERT_EQUAL(rt, TRUE);
	
	cyg_thread_delay(300);
	
	rt = vp_bitrate_control_linkrefresh();
	CU_ASSERT_EQUAL(rt, TRUE);
	
	pLinkSpeed = vp_bitrate_control_linksearch(1);
	CU_ASSERT_EQUAL(pLinkSpeed, NULL);
	
	rt = vp_bitrate_control_uninit();
	CU_ASSERT_EQUAL(rt, TRUE);

}

CU_TestInfo test_registry_vp_bitrate_control_linkrefresh[] =
{
	{"test_vp_bitrate_control_linkrefresh", test_vp_bitrate_control_linkrefresh},
	CU_TEST_INFO_NULL
};

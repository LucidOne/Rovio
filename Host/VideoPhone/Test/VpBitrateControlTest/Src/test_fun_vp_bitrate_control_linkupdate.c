
#include "lib_videophone.h"

#include "CUnit.h"
#include "error_handling.h"

extern LinkTest *g_LinkTest;

/*
 * function:
 *		The code coverage is 100% in vp_bitrate_control_linkupdate().
 *
 */
void test_vp_bitrate_control_linkupdate(void)
{
	BOOL rt;
	LinkSpeed *pLinkSpeed = NULL;
	
	rt = vp_bitrate_control_init(1);
	CU_ASSERT_EQUAL(rt, TRUE);
	
	rt = vp_bitrate_control_linkupdate(1, 100);
	CU_ASSERT_EQUAL(rt, TRUE);
	
	pLinkSpeed = vp_bitrate_control_linksearch(1);
	CU_ASSERT_NOT_EQUAL(pLinkSpeed, NULL);
	
	rt = vp_bitrate_control_linkupdate(2, 100);
	CU_ASSERT_EQUAL(rt, TRUE);
	
	pLinkSpeed = vp_bitrate_control_linksearch(2);
	CU_ASSERT_NOT_EQUAL(pLinkSpeed, NULL);
	
	rt = vp_bitrate_control_uninit();
	CU_ASSERT_EQUAL(rt, TRUE);
}

CU_TestInfo test_registry_vp_bitrate_control_linkupdate[] =
{
	{"test_vp_bitrate_control_linkupdate", test_vp_bitrate_control_linkupdate},
	CU_TEST_INFO_NULL
};



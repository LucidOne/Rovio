#include "lib_videophone.h"

#include "CUnit.h"
#include "error_handling.h"

extern LinkTest *g_LinkTest;

/*
 * function:
 *		The code coverage is 100% in vp_bitrate_control_linkstat().
 *
 */
void test_vp_bitrate_control_linkstat(void)
{
	BOOL rt;
	INT speed;
	
	rt = vp_bitrate_control_init(1);
	CU_ASSERT_EQUAL(rt, TRUE);
	
	rt = vp_bitrate_control_linkupdate(1, 100);
	CU_ASSERT_EQUAL(rt, TRUE);
	
	vp_bitrate_control_linkstat();
	speed = vp_bitrate_control_getspeed();
	CU_ASSERT_NOT_EQUAL(speed, 0);
	
	cyg_thread_delay(2);
	
	vp_bitrate_control_linkstat();
	speed = vp_bitrate_control_getspeed();
	CU_ASSERT_EQUAL(speed, 0);
	
	rt = vp_bitrate_control_uninit();
	CU_ASSERT_EQUAL(rt, TRUE);
}

CU_TestInfo test_registry_vp_bitrate_control_linkstat[] =
{
	{"test_vp_bitrate_control_linkstat", test_vp_bitrate_control_linkstat},
	CU_TEST_INFO_NULL,
};


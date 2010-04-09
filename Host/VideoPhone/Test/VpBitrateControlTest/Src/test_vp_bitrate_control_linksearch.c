
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
void test_vp_bitrate_control_linksearch(void)
{
	BOOL rt;
	LinkSpeed *pLinkSpeed = NULL;
	
	rt = vp_bitrate_control_init(1);
	CU_ASSERT_EQUAL(rt, TRUE);
	
	/* Search in empty list */
	pLinkSpeed = vp_bitrate_control_linksearch(1);
	CU_ASSERT_EQUAL(pLinkSpeed, NULL);
	
	/* Search added fd */
	rt = vp_bitrate_control_linkadd(1, 100);
	CU_ASSERT_EQUAL(rt, TRUE);
	
	pLinkSpeed = vp_bitrate_control_linksearch(1);
	CU_ASSERT_NOT_EQUAL(pLinkSpeed, NULL);
	CU_ASSERT_EQUAL(pLinkSpeed->fd, 1);
	CU_ASSERT_EQUAL(pLinkSpeed->iWriteBytes, 100);
	
	rt = vp_bitrate_control_linkadd(2, 200);
	CU_ASSERT_EQUAL(rt, TRUE);
	
	pLinkSpeed = vp_bitrate_control_linksearch(2);
	CU_ASSERT_NOT_EQUAL(pLinkSpeed, NULL);
	CU_ASSERT_EQUAL(pLinkSpeed->fd, 2);
	CU_ASSERT_EQUAL(pLinkSpeed->iWriteBytes, 200);
	
	rt = vp_bitrate_control_linkadd(3, 300);
	CU_ASSERT_EQUAL(rt, TRUE);
	
	pLinkSpeed = vp_bitrate_control_linksearch(3);
	CU_ASSERT_NOT_EQUAL(pLinkSpeed, NULL);
	CU_ASSERT_EQUAL(pLinkSpeed->fd, 3);
	CU_ASSERT_EQUAL(pLinkSpeed->iWriteBytes, 300);
	
	rt = vp_bitrate_control_linkadd(4, 400);
	CU_ASSERT_EQUAL(rt, TRUE);
	
	pLinkSpeed = vp_bitrate_control_linksearch(4);
	CU_ASSERT_NOT_EQUAL(pLinkSpeed, NULL);
	CU_ASSERT_EQUAL(pLinkSpeed->fd, 4);
	CU_ASSERT_EQUAL(pLinkSpeed->iWriteBytes, 400);
	
	pLinkSpeed = vp_bitrate_control_linksearch(1);
	CU_ASSERT_NOT_EQUAL(pLinkSpeed, NULL);
	CU_ASSERT_EQUAL(pLinkSpeed->fd, 1);
	CU_ASSERT_EQUAL(pLinkSpeed->iWriteBytes, 100);
	
	pLinkSpeed = vp_bitrate_control_linksearch(3);
	CU_ASSERT_NOT_EQUAL(pLinkSpeed, NULL);
	CU_ASSERT_EQUAL(pLinkSpeed->fd, 3);
	CU_ASSERT_EQUAL(pLinkSpeed->iWriteBytes, 300);
	
	rt = vp_bitrate_control_uninit();
	CU_ASSERT_EQUAL(rt, TRUE);
}

CU_TestInfo test_registry_vp_bitrate_control_linksearch[] =
{
	{"test_vp_bitrate_control_linksearch", test_vp_bitrate_control_linksearch},
	CU_TEST_INFO_NULL
};


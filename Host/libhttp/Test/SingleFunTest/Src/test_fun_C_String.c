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

void test_httpMyStrncpy(void)
{
	const char *strSrc = "This is my string";
	char strDstLarge[30];
	char strDstEqual[18];
	char strDstSmall[4];
	
	/* Destination larger than Souce string length */
	httpMyStrncpy(strDstLarge, strSrc, sizeof(strDstLarge));
	CU_ASSERT_EQUAL(strcmp(strDstLarge, strSrc), 0);
	
	/* Destination equal to Souce string length */
	httpMyStrncpy(strDstEqual, strSrc, sizeof(strDstEqual));
	CU_ASSERT_EQUAL(strcmp(strDstEqual, strSrc), 0);
	
	/* Destination smaller than Souce string length */
	httpMyStrncpy(strDstSmall, strSrc, sizeof(strDstSmall));
	CU_ASSERT_EQUAL(strncmp(strDstSmall, "Thi", strlen("Thi")+1), 0);
}

CU_TestInfo test_c_string[] =
{
	{"test_httpMyStrncpy", test_httpMyStrncpy},
	CU_TEST_INFO_NULL
};


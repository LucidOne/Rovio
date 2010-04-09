#include "wb_syslib_addon.h"
#include "hic_host.h"
#include "FTH_Int.h"
#include "RemoteFunc.h"
#include "RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteFuncTest.h"
#include "test_getsockopt.h"

#define TEST_GETSOCKOPT_INT_VAL 1

typedef struct sock_opts
{
	char	*opt_str;
	int		opt_level;
	int		opt_name;
}sock_opts;

sock_opts opts[] = {
	{"SO_BROADCAST",	SOL_SOCKET,	SO_BROADCAST},
	{"SO_REUSEADDR",	SOL_SOCKET,	SO_REUSEADDR},
	{NULL,				0,			0,			}
};

void test_getsockopt(char *pBuf, int iBufLen)
{
	int fd;
	
	test_printf_begin("test_getsockopt");
	fd = socket(AF_INET, SOCK_STREAM, 0, pBuf, iBufLen);
	if(fd < 0)
	{
		printf("test_getsockopt(): socket error\n");
		return;
	}
	
	test_setsockopt_entry(fd, pBuf, iBufLen);
	test_getsockopt_entry(fd, pBuf, iBufLen);
	
	close(fd);
	
	test_printf_end("test_getsockopt");
}

void test_getsockopt_entry(int fd, char *pBuf, int iBufLen)
{
	sock_opts *pSockOpts;
	int val, len;
	
	pSockOpts = opts;
	len = sizeof(val);
	if(getsockopt(-1, pSockOpts->opt_level, pSockOpts->opt_name,
					&val, &len, pBuf, iBufLen) != -1)
	{
		test_printf_error("test_getsockopt_entry");
		return;
	}
	
	if(getsockopt(fd, pSockOpts->opt_level, pSockOpts->opt_name,
					NULL, &len, pBuf, iBufLen) != -1)
	{
		test_printf_error("test_getsockopt_entry");
		return;
	}
	
	if(getsockopt(fd, pSockOpts->opt_level, pSockOpts->opt_name,
					&val, NULL, pBuf, iBufLen) != -1)
	{
		test_printf_error("test_getsockopt_entry");
		return;
	}
	
	if(getsockopt(fd, pSockOpts->opt_level, pSockOpts->opt_name,
					&val, &len, NULL, iBufLen) != -1)
	{
		test_printf_error("test_getsockopt_entry");
		return;
	}
	
	if(getsockopt(fd, pSockOpts->opt_level, pSockOpts->opt_name,
					&val, &len, pBuf, 0) != -1)
	{
		test_printf_error("test_getsockopt_entry");
		return;
	}
	
	test_printf_success("test_getsockopt_entry");
}

void test_setsockopt_entry(int fd, char *pBuf, int iBufLen)
{
	sock_opts *pSockOpts;
	int val, len;
	
	pSockOpts = opts;
	len = sizeof(val);
	val = TEST_GETSOCKOPT_INT_VAL;
	if(setsockopt(-1, pSockOpts->opt_level, pSockOpts->opt_name,
					&val, len, pBuf, iBufLen) != -1)
	{
		test_printf_error("test_setsockopt_entry");
		return;
	}
	
	if(setsockopt(fd, pSockOpts->opt_level, pSockOpts->opt_name,
					NULL, len, pBuf, iBufLen) != -1)
	{
		test_printf_error("test_setsockopt_entry");
		return;
	}
	
	if(setsockopt(fd, pSockOpts->opt_level, pSockOpts->opt_name,
					&val, 0, pBuf, iBufLen) != -1)
	{
		test_printf_error("test_setsockopt_entry");
		return;
	}
	
	if(setsockopt(fd, pSockOpts->opt_level, pSockOpts->opt_name,
					&val, len, NULL, iBufLen) != -1)
	{
		test_printf_error("test_setsockopt_entry");
		return;
	}
	
	if(setsockopt(fd, pSockOpts->opt_level, pSockOpts->opt_name,
					&val, len, pBuf, 0) != -1)
	{
		test_printf_error("test_setsockopt_entry");
		return;
	}
	
	test_printf_success("test_setsockopt_entry");
}

int set_reuseaddr(int fd, char *pBuf, int iBufLen)
{
	int val, len;
	
	val = 1;
	len = sizeof(val);
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, len, pBuf, iBufLen) == -1)
	{
		printf("set_reuseaddr(): error\n");
		return -1;
	}
	return 0;
}




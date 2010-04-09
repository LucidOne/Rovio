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

typedef union sock_opt_val
{
	int ival;
}sock_opt_val;

typedef struct sock_opts
{
	char	*opt_str;
	int		opt_level;
	int		opt_name;
	char	*(*opt_val_pro)(sock_opt_val *, int);
}sock_opts;

static char *sock_str_flag(sock_opt_val *pval, int ilen);

//sock_opt_val val;

sock_opts opts[] = {
	{"SO_BROADCAST",	SOL_SOCKET,	SO_BROADCAST,	sock_str_flag},
	{"SO_REUSEADDR",	SOL_SOCKET,	SO_REUSEADDR,	sock_str_flag},
	{NULL,				0,			0,				NULL}
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
	
	test_setsockopt_reuseaddr(pBuf, iBufLen);
	test_printf_end("test_getsockopt");
}

void test_getsockopt_entry(int fd, char *pBuf, int iBufLen)
{
	sock_opts *pSockOpts;
	int val, len;
	
	for(pSockOpts = opts; pSockOpts->opt_str != NULL; pSockOpts++)
	{
		len = sizeof(val);
		if(getsockopt(fd, pSockOpts->opt_level, pSockOpts->opt_name,
						&val, &len, pBuf, iBufLen) == -1)
		{
			test_printf_error("test_getsockopt_entry");
			return;
		}
		else
		{
			if(val != TEST_GETSOCKOPT_INT_VAL)
			{
				test_printf_error("test_getsockopt_entry");
				return;
			}
		}
	}
	
	test_printf_success("test_getsockopt_entry");
}

void test_setsockopt_entry(int fd, char *pBuf, int iBufLen)
{
	sock_opts *pSockOpts;
	int val, len;
	
	for(pSockOpts = opts; pSockOpts->opt_str != NULL; pSockOpts++)
	{
		len = sizeof(val);
		val = TEST_GETSOCKOPT_INT_VAL;
		if(setsockopt(fd, pSockOpts->opt_level, pSockOpts->opt_name,
						&val, len, pBuf, iBufLen) == -1)
		{
			test_printf_error("test_setsockopt_entry");
			return;
		}
	}
	
	test_printf_success("test_setsockopt_entry");
}

static char *sock_str_flag(sock_opt_val *pval, int ilen)
{
	char strbuf[128];
	int len;
	
	if(len != sizeof(int))
		snprintf(strbuf, sizeof(strbuf), "size(%d) not sizeof(int)\n", len);
	else
		snprintf(strbuf, sizeof(strbuf), "%s", (pval->ival == 0) ? "off" : "on");
	
	strbuf[sizeof(strbuf) - 1] = '\0';
	printf("%s", strbuf);
	return NULL;
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

void test_setsockopt_reuseaddr(char *pBuf, int iBufLen)
{
	int i, err;
	int fd[TEST_SETSOCKOPT_REUSEADDR_TIMES] = {0};
	struct sockaddr_in r_sa;
	struct hostent *hp;
	
	if((hp = gethostbyname(TEST_REMOTEFUNC_HOSTNAME, pBuf, iBufLen)) == NULL)
	{
		test_printf_error("test_setsockopt_reuseaddr");
		return;
	}
	
	memcpy(&(r_sa.sin_addr), hp->h_addr_list0, hp->h_length);
	r_sa.sin_family = AF_INET;
	r_sa.sin_port = htons(IPPORT_USERRESERVED + TEST_SETSOCKOPT_REUSEADDR_PORT);
	
	err = 0;
	memset(fd, 0, sizeof(fd));
	for(i = 0; i < TEST_SETSOCKOPT_REUSEADDR_TIMES; i++)
	{
		if((fd[i] = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pBuf, iBufLen)) == -1)
		{
			test_printf_error("test_setsockopt_reuseaddr");
			err = 1;
			break;
		}
		
		if(set_reuseaddr(fd[i], pBuf, iBufLen) == -1)
		{
			test_printf_error("test_setsockopt_reuseaddr");
			err = 1;
			break;
		}
		
		if(bind(fd[i], (struct sockaddr*)&r_sa, sizeof(r_sa), pBuf, iBufLen) == -1)
		{
			test_printf_error("test_setsockopt_reuseaddr");
			err = 1;
			break;
		}
	}
	
	for(i = 0; i < TEST_SETSOCKOPT_REUSEADDR_TIMES; i++)
	{
		if(fd[i] != 0) netclose(fd[i], pBuf, iBufLen);
	}
	
	if(err == 1) return;
	
	memset(fd, 0, sizeof(fd));
	for(i = 0; i < TEST_SETSOCKOPT_REUSEADDR_TIMES; i++)
	{
		if((fd[i] = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pBuf, iBufLen)) == -1)
		{
			test_printf_error("test_setsockopt_reuseaddr");
			err = 1;
			break;
		}
		
		if(set_reuseaddr(fd[i], pBuf, iBufLen) == -1)
		{
			test_printf_error("test_setsockopt_reuseaddr");
			err = 1;
			break;
		}
		
		if(bind(fd[i], (struct sockaddr*)&r_sa, sizeof(r_sa), pBuf, iBufLen) == -1)
		{
			test_printf_error("test_setsockopt_reuseaddr");
			err = 1;
			break;
		}
		
		close(fd[i]);
	}
	if(err != 1) test_printf_success("test_setsockopt_reuseaddr");
}




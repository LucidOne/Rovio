#include "wb_syslib_addon.h"
#include "hic_host.h"
#include "FTH_Int.h"
#include "RemoteFunc.h"
#include "RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteFuncTest.h"
#include "test_getsockname.h"

void test_getsockname(char *pBuf, int iBufLen)
{
	test_printf_begin("test_getsockname");
	test_getsockname_entry(pBuf, iBufLen);
	test_printf_end("test_getsockname");
}


void test_getsockname_entry(char *pBuf, int iBufLen)
{
	int s, new_s;
	struct sockaddr_in sa, r_sa;
	int r_sa_l = sizeof(r_sa);
	struct hostent *hp;
	
	int port = TEST_GETPEERNAME_PORT;
	
	if((hp = gethostbyname(TEST_REMOTEFUNC_HOSTNAME, pBuf, iBufLen)) == NULL)
	{
		test_printf_error("test_getsockname_entry");
		return;
	}
	
	memcpy(&(r_sa.sin_addr), hp->h_addr_list0, hp->h_length);
	r_sa.sin_family = AF_INET;
	r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
	
	if((s = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pBuf, iBufLen)) == -1)
	{
		test_printf_error("test_getsockname_entry");
		return;
	}
	
	if(set_reuseaddr(s, pBuf, iBufLen) == -1)
	{
		test_printf_error("test_getsockname_entry");
		netclose(s, pBuf, iBufLen);
		cyg_thread_exit();
	}
	
	if(bind(s, (struct sockaddr*)&r_sa, sizeof(r_sa), pBuf, iBufLen) == -1)
	{
		test_printf_error("test_getsockname_entry");
		netclose(s, pBuf, iBufLen);
		return;
	}
	
	if(getsockname(s, (struct sockaddr*)&sa, &r_sa_l, pBuf, iBufLen) != -1)
	{
		if(sa.sin_addr.s_addr != r_sa.sin_addr.s_addr || sa.sin_port != r_sa.sin_port
			|| sa.sin_family != r_sa.sin_family)
		{
			test_printf_error("getsockname");
			netclose(s, pBuf, iBufLen);
			return;
		}
		test_printf_success("getsockname");
	}
	else
	{
		test_printf_error("getsockname");
		netclose(s, pBuf, iBufLen);
		return;
	}
	
	if(listen(s, 10, pBuf, iBufLen) == -1)
	{
		test_printf_error("test_getsockname_entry");
		netclose(s, pBuf, iBufLen);
		return;
	}
	
	if((new_s = accept(s, (struct sockaddr*)&sa, (size_t*)&r_sa_l, pBuf, iBufLen)) == -1)
	{
		test_printf_error("test_getsockname_entry");
		netclose(s, pBuf, iBufLen);
		return;
	}
	
	if(getpeername(new_s, (struct sockaddr*)&sa, &r_sa_l, pBuf, iBufLen) != -1)
	{
		port = htons(IPPORT_USERRESERVED + TEST_GETPEERNAME_PORT);
		if(sa.sin_addr.s_addr != TEST_GETPEERNAME_ADDR || sa.sin_port != port
			|| sa.sin_family != r_sa.sin_family)
		{
			test_printf_error("getpeername");
			netclose(new_s, pBuf, iBufLen);
			netclose(s, pBuf, iBufLen);
			return;
		}
		test_printf_success("getpeername");
	}
	else
	{
		test_printf_error("getpeername");
		netclose(new_s, pBuf, iBufLen);
		netclose(s, pBuf, iBufLen);
		return;
	}
	
	
	netclose(new_s, pBuf, iBufLen);
	netclose(s, pBuf, iBufLen);
}

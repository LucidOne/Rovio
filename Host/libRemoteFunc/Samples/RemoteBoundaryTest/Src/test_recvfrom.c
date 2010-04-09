#include "wb_syslib_addon.h"
#include "hic_host.h"
#include "FTH_Int.h"
#include "RemoteFunc.h"
#include "RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteFuncTest.h"

void test_recvfrom(char *pBuf, int iBufLen)
{
	test_printf_begin("test_recvfrom");
	test_recvfrom_entry(pBuf, iBufLen);
	test_printf_end("test_recvfrom");
}


static char msg[TEST_RECVFROM_MSG_LEN];

void test_recvfrom_entry(char *pBuf, int iBufLen)
{
	int s, recvfromlen;
	struct sockaddr_in sa, r_sa;
	int r_sa_l = sizeof(r_sa);
	
	int port = TEST_RECVFROM_SERVER_BEGIN_PORT;
	char *precvfrombuf = msg;
	
	r_sa.sin_addr.s_addr = htonl(INADDR_ANY);
	r_sa.sin_family = AF_INET;
	r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
	
	if((s = socket(AF_INET, SOCK_DGRAM, PF_UNSPEC, pBuf, iBufLen)) == -1)
	{
		test_printf_error("test_recvfrom_entry");
		return;
	}
	
	if(set_reuseaddr(s, pBuf, iBufLen) == -1)
	{
		test_printf_error("test_recvfrom_entry");
		netclose(s, pBuf, iBufLen);
		return;
	}
	
	if(bind(s, (struct sockaddr*)&r_sa, sizeof(r_sa), pBuf, iBufLen) == -1)
	{
		test_printf_error("test_recvfrom_entry");
		netclose(s, pBuf, iBufLen);
		return;
	}

	recvfromlen = recvfrom(-1, precvfrombuf, TEST_RECVFROM_MSG_LEN, 0, 
							(struct sockaddr*)&sa, &r_sa_l, pBuf, iBufLen);
	if(recvfromlen >= 0)
	{
		test_printf_error("test_recvfrom_entry");
		netclose(s, pBuf, iBufLen);
		return;
	}
	
	recvfromlen = recvfrom(s, NULL, TEST_RECVFROM_MSG_LEN, 0, 
							(struct sockaddr*)&sa, &r_sa_l, pBuf, iBufLen);
	if(recvfromlen >= 0)
	{
		test_printf_error("test_recvfrom_entry");
		netclose(s, pBuf, iBufLen);
		return;
	}
	
	recvfromlen = recvfrom(s, precvfrombuf, -1, 0, 
							(struct sockaddr*)&sa, &r_sa_l, pBuf, iBufLen);
	if(recvfromlen >= 0)
	{
		test_printf_error("test_recvfrom_entry");
		netclose(s, pBuf, iBufLen);
		return;
	}
	
	recvfromlen = recvfrom(s, precvfrombuf, TEST_RECVFROM_MSG_LEN, 0, 
							(struct sockaddr*)&sa, &r_sa_l, NULL, iBufLen);
	if(recvfromlen >= 0)
	{
		test_printf_error("test_recvfrom_entry");
		netclose(s, pBuf, iBufLen);
		return;
	}
	
	recvfromlen = recvfrom(s, precvfrombuf, TEST_RECVFROM_MSG_LEN, 0, 
							(struct sockaddr*)&sa, &r_sa_l, pBuf, 0);
	if(recvfromlen >= 0)
	{
		test_printf_error("test_recvfrom_entry");
		netclose(s, pBuf, iBufLen);
		return;
	}
	
	test_printf_success("test_recvfrom_entry");
	netclose(s, pBuf, iBufLen);
}

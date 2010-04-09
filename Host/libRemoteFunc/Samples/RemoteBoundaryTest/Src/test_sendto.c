#include "wb_syslib_addon.h"
#include "hic_host.h"
#include "FTH_Int.h"
#include "RemoteFunc.h"
#include "RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteFuncTest.h"

void test_sendto(char *pBuf, int iBufLen)
{
	test_printf_begin("test_sendto");
	test_sendto_entry(pBuf, iBufLen);
	test_printf_end("test_sendto");
}

static char msg[TEST_SENDTO_MSG_LEN];

void test_sendto_entry(char *pBuf, int iBufLen)
{
	int s, len, sendtolen;
	struct sockaddr_in sa;
	
	int port = TEST_SENDTO_SERVER_BEGIN_PORT;
	char *psendtobuf = msg;
	
    if(inet_aton(TEST_SENDTO_SERVER_ADDR, &sa.sin_addr, pBuf, iBufLen) == 0)
    {
		test_printf_error("test_sendto_server");
		return;
    }
    sa.sin_family = AF_INET;
    sa.sin_port = htons(IPPORT_USERRESERVED + port);
    
	if((s = socket(AF_INET, SOCK_DGRAM, PF_UNSPEC, pBuf, iBufLen)) == -1)
	{
		test_printf_error("test_sendto_server");
		return;
	}
	
	len = sprintf(psendtobuf, "%s", TEST_SENDTO_MSG);
	len++;
	sendtolen = sendto(-1, psendtobuf, len, 0, 
						(struct sockaddr*)&sa, sizeof(sa), pBuf, iBufLen);
	if(sendtolen >= 0)
	{
		test_printf_error("test_sendto_server");
		netclose(s, pBuf, iBufLen);
		return;
	}
	
	sendtolen = sendto(s, NULL, len, 0, 
						(struct sockaddr*)&sa, sizeof(sa), pBuf, iBufLen);
	if(sendtolen >= 0)
	{
		test_printf_error("test_sendto_server");
		netclose(s, pBuf, iBufLen);
		return;
	}
	
	sendtolen = sendto(s, psendtobuf, -1, 0, 
						(struct sockaddr*)&sa, sizeof(sa), pBuf, iBufLen);
	if(sendtolen >= 0)
	{
		test_printf_error("test_sendto_server");
		netclose(s, pBuf, iBufLen);
		return;
	}
	
	sendtolen = sendto(s, psendtobuf, len, 0, 
						(struct sockaddr*)&sa, sizeof(sa), NULL, iBufLen);
	if(sendtolen >= 0)
	{
		test_printf_error("test_sendto_server");
		netclose(s, pBuf, iBufLen);
		return;
	}
	
	sendtolen = sendto(s, psendtobuf, len, 0, 
						(struct sockaddr*)&sa, sizeof(sa), pBuf, 0);
	if(sendtolen >= 0)
	{
		test_printf_error("test_sendto_server");
		netclose(s, pBuf, iBufLen);
		return;
	}
	
	test_printf_success("test_sendto_server");
	netclose(s, pBuf, iBufLen);
}

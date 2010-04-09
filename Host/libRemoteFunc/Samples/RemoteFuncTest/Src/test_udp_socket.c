#include "wb_syslib_addon.h"
#include "hic_host.h"
#include "FTH_Int.h"
#include "RemoteFunc.h"
#include "RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteFuncTest.h"

#define TEST_UDP_SOCKET_PORT_BEGIN 1
#define TEST_UDP_SOCKET_PORT_END 50

void test_udp_socket(char *pcBuf, int iBufLen)
{
	test_printf_begin("test_udp_socket");
	test_udp_create_socket(pcBuf, iBufLen);
	test_udp_bind_socket(pcBuf, iBufLen);
	test_printf_end("test_udp_socket");
}

void test_udp_create_socket(char *pcBuf, int iBufLen)
{
	int iPortBegin = TEST_UDP_SOCKET_PORT_BEGIN;
	int iPortEnd = TEST_UDP_SOCKET_PORT_END;
	int port;
	int sockfd[TEST_UDP_SOCKET_PORT_END];
	int i;
	
	port = iPortBegin;
	while(port <= iPortEnd)
	{
		if((sockfd[port-1] = socket(AF_INET, SOCK_DGRAM, PF_UNSPEC, pcBuf, iBufLen)) == -1)
		{
			goto fail;
		}
		port++;
	}
	
fail:
	if(port <= iPortEnd)
	{
		printf("test_udp_create_socket(): create socket for port %d error\n", port);
		test_printf_error("test_udp_create_socket");
	}
	else
	{
		test_printf_success("test_udp_create_socket");
	}
	for(i = iPortBegin; i <= port; i++)
	{
		netclose(sockfd[i-1], pcBuf, iBufLen);
	}
	return;
}

void test_udp_bind_socket(char *pcBuf, int iBufLen)
{
	struct sockaddr_in r_sa;
	struct hostent *hp;
	int i;
	
	int iPortBegin = TEST_UDP_SOCKET_PORT_BEGIN;
	int iPortEnd = TEST_UDP_SOCKET_PORT_END;
	int port;
	int sockfd[TEST_UDP_SOCKET_PORT_END];
	
	char hname[256];
	
	memset(hname, 0, 256);
	if(gethostname(hname, 256, pcBuf, iBufLen) == -1)
	{
		test_printf_error("test_udp_bind_socket");
		return;
	}
	
	port = iPortBegin;
	while(port <= iPortEnd)
	{
		if((hp = gethostbyname(hname, pcBuf, iBufLen)) == NULL)
		{
			test_printf_error("test_udp_bind_socket");
			goto fail;
		}
		
		memcpy(&(r_sa.sin_addr), hp->h_addr_list0, hp->h_length);
		r_sa.sin_family = AF_INET;
		r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
		
		if((sockfd[port-1] = socket(AF_INET, SOCK_DGRAM, PF_UNSPEC, pcBuf, iBufLen)) == -1)
		{
			test_printf_error("test_udp_bind_socket");
			goto fail;
		}
		
		set_reuseaddr(sockfd[port-1], pcBuf, iBufLen);
		if(bind(sockfd[port-1], (struct sockaddr*)&r_sa, sizeof(r_sa), pcBuf, iBufLen) == -1)
		{
			test_printf_error("test_udp_bind_socket");
			goto fail;
		}
		port++;
	}
	
	test_printf_success("test_udp_bind_socket");
	
fail:
	for(i = iPortBegin; i < port; i++)
	{
		netclose(sockfd[i-1], pcBuf, iBufLen);
	}
}

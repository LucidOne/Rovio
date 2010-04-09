#include "wb_syslib_addon.h"
#include "hic_host.h"
#include "FTH_Int.h"
#include "RemoteFunc.h"
#include "RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteFuncTest.h"

#define TEST_TCP_SOCKET_PORT_BEGIN 1
#define TEST_TCP_SOCKET_PORT_END 50

void test_socket(char *pcBuf, int iBufLen)
{
	test_printf_begin("test_socket");
	test_create_socket(pcBuf, iBufLen);
	test_bind_socket(pcBuf, iBufLen);
	test_listen_socket(pcBuf, iBufLen);
	test_accept_socket(pcBuf, iBufLen);
	test_printf_end("test_socket");
}

void test_create_socket(char *pcBuf, int iBufLen)
{
	int sockfd;
	int i;
	
	if((sockfd = socket(-1, SOCK_STREAM, PF_UNSPEC, pcBuf, iBufLen)) != -1)
	{
		test_printf_error("test_create_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}
	
	for(i = 0; i > -2; i--)
	{
		if((sockfd = socket(AF_INET, i, PF_UNSPEC, pcBuf, iBufLen)) != -1)
		{
			test_printf_error("test_create_socket");
			netclose(sockfd, pcBuf, iBufLen);
			return;
		}
	}
	
	if((sockfd = socket(AF_INET, SOCK_STREAM, -1, pcBuf, iBufLen)) != -1)
	{
		test_printf_error("test_create_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}
	
	if((sockfd = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, NULL, iBufLen)) != -1)
	{
		test_printf_error("test_create_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}
	
	if((sockfd = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pcBuf, 0)) != -1)
	{
		test_printf_error("test_create_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}
	test_printf_success("test_create_socket");
}

void test_bind_socket(char *pcBuf, int iBufLen)
{
	struct sockaddr_in r_sa;
	struct hostent *hp;
	
	int port = TEST_TCP_SOCKET_PORT_BEGIN;
	int sockfd;
	
	char hname[256];
	
	memset(hname, 0, 256);
	if(gethostname(hname, 256, pcBuf, iBufLen) == -1)
	{
		test_printf_error("test_bind_socket");
		return;
	}
	
	if((hp = gethostbyname(hname, pcBuf, iBufLen)) == NULL)
	{
		test_printf_error("test_bind_socket");
		return;
	}
		
	if((sockfd = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pcBuf, iBufLen)) == -1)
	{
		test_printf_error("test_bind_socket");
		return;
	}
	
	if(set_reuseaddr(sockfd, pcBuf, iBufLen) == -1)
	{
		test_printf_error("test_bind_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}
	
	memset(&(r_sa.sin_addr), 1, sizeof(r_sa.sin_addr));
	r_sa.sin_family = AF_INET;
	r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
	if(bind(sockfd, (struct sockaddr*)&r_sa, sizeof(r_sa), pcBuf, iBufLen) != -1)
	{
		test_printf_error("test_bind_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}
	
	memcpy(&(r_sa.sin_addr), hp->h_addr_list0, hp->h_length);
	r_sa.sin_family = AF_INET;
	r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
	
	if(bind(-1, (struct sockaddr*)&r_sa, sizeof(r_sa), pcBuf, iBufLen) != -1)
	{
		test_printf_error("test_bind_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}
	
	if(bind(sockfd, NULL, sizeof(r_sa), pcBuf, iBufLen) != -1)
	{
		test_printf_error("test_bind_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}
	
	if(bind(sockfd, (struct sockaddr*)&r_sa, 0, pcBuf, iBufLen) != -1)
	{
		test_printf_error("test_bind_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}
	
	if(bind(sockfd, (struct sockaddr*)&r_sa, sizeof(r_sa), NULL, iBufLen) != -1)
	{
		test_printf_error("test_bind_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}
	
	if(bind(sockfd, (struct sockaddr*)&r_sa, sizeof(r_sa), pcBuf, 0) != -1)
	{
		test_printf_error("test_bind_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}
	
	netclose(sockfd, pcBuf, iBufLen);
	test_printf_success("test_bind_socket");
}

void test_listen_socket(char *pcBuf, int iBufLen)
{
	struct sockaddr_in r_sa;
	struct hostent *hp;
	
	int port = TEST_TCP_SOCKET_PORT_BEGIN;
	int sockfd;
	
	char hname[256];
	
	memset(hname, 0, 256);
	if(gethostname(hname, 256, pcBuf, iBufLen) == -1)
	{
		test_printf_error("test_listen_socket");
		return;
	}
	
	if((hp = gethostbyname(hname, pcBuf, iBufLen)) == NULL)
	{
		test_printf_error("test_listen_socket");
		return;
	}
		
	memcpy(&(r_sa.sin_addr), hp->h_addr_list0, hp->h_length);
	r_sa.sin_family = AF_INET;
	r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
		
	if((sockfd = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pcBuf, iBufLen)) == -1)
	{
		test_printf_error("test_listen_socket");
		return;
	}
		
	if(set_reuseaddr(sockfd, pcBuf, iBufLen) == -1)
	{
		test_printf_error("test_listen_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}
	
	if(bind(sockfd, (struct sockaddr*)&r_sa, sizeof(r_sa), pcBuf, iBufLen) == -1)
	{
		test_printf_error("test_listen_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}

	if(listen(-1, 10, pcBuf, iBufLen) != -1)
	{
		test_printf_error("test_listen_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}
	
	if(listen(sockfd, 10, NULL, iBufLen) != -1)
	{
		test_printf_error("test_listen_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}
	
	if(listen(sockfd, 10, pcBuf, 0) != -1)
	{
		test_printf_error("test_listen_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}
	
	netclose(sockfd, pcBuf, iBufLen);
	test_printf_success("test_listen_socket");
}


void test_accept_socket(char *pcBuf, int iBufLen)
{
	struct sockaddr_in r_sa;
	struct hostent *hp;
	
	int port = TEST_TCP_SOCKET_PORT_BEGIN;
	int sockfd;
	
	char hname[256];
	
	memset(hname, 0, 256);
	if(gethostname(hname, 256, pcBuf, iBufLen) == -1)
	{
		test_printf_error("test_accept_socket");
		return;
	}
	
	if((hp = gethostbyname(hname, pcBuf, iBufLen)) == NULL)
	{
		test_printf_error("test_accept_socket");
		return;
	}
		
	memcpy(&(r_sa.sin_addr), hp->h_addr_list0, hp->h_length);
	r_sa.sin_family = AF_INET;
	r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
		
	if((sockfd = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pcBuf, iBufLen)) == -1)
	{
		test_printf_error("test_accept_socket");
		return;
	}
		
	if(set_reuseaddr(sockfd, pcBuf, iBufLen) == -1)
	{
		test_printf_error("test_accept_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}
	
	if(bind(sockfd, (struct sockaddr*)&r_sa, sizeof(r_sa), pcBuf, iBufLen) == -1)
	{
		test_printf_error("test_accept_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}

	if(listen(sockfd, 10, pcBuf, iBufLen) == -1)
	{
		test_printf_error("test_accept_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}
	
	if(accept(-1, NULL, NULL, pcBuf, iBufLen) != -1)
	{
		test_printf_error("test_accept_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}
	
	if(accept(sockfd, NULL, NULL, NULL, iBufLen) != -1)
	{
		test_printf_error("test_accept_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}
	
	if(accept(sockfd, NULL, NULL, pcBuf, -1) != -1)
	{
		test_printf_error("test_accept_socket");
		netclose(sockfd, pcBuf, iBufLen);
		return;
	}
	
	netclose(sockfd, pcBuf, iBufLen);
	test_printf_success("test_accept_socket");
}


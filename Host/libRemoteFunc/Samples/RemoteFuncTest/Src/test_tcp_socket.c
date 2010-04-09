#include "wb_syslib_addon.h"
#include "hic_host.h"
#include "FTH_Int.h"
#include "RemoteFunc.h"
#include "RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteFuncTest.h"
#include "test_tcp_socket.h"

#define TEST_TCP_SOCKET_PORT_BEGIN 1
#define TEST_TCP_SOCKET_PORT_END 50

void test_tcp_socket(char *pcBuf, int iBufLen)
{
	test_printf_begin("test_tcp_socket");
	test_tcp_create_socket(pcBuf, iBufLen);
	test_tcp_bind_socket(pcBuf, iBufLen);
	test_tcp_listen_socket(pcBuf, iBufLen);
	test_tcp_accept_socket();
	test_printf_end("test_tcp_socket");
}

void test_tcp_create_socket(char *pcBuf, int iBufLen)
{
	int iPortBegin = TEST_TCP_SOCKET_PORT_BEGIN;
	int iPortEnd = TEST_TCP_SOCKET_PORT_END;
	int port;
	int sockfd[TEST_TCP_SOCKET_PORT_END];
	int i;
	
	port = iPortBegin;
	while(port <= iPortEnd)
	{
		if((sockfd[port-1] = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pcBuf, iBufLen)) == -1)
		{
			printf("test_tcp_create_socket(): socket error\n");
			goto fail;
		}
		port++;
	}
	
fail:
	if(port > iPortEnd)
		test_printf_success("test_tcp_create_socket");
	for(i = iPortBegin; i <= port; i++)
	{
		netclose(sockfd[i-1], pcBuf, iBufLen);
	}
	return;
}

void test_tcp_bind_socket(char *pcBuf, int iBufLen)
{
	struct sockaddr_in r_sa;
	struct hostent *hp;
	int i;
	
	int iPortBegin = TEST_TCP_SOCKET_PORT_BEGIN;
	int iPortEnd = TEST_TCP_SOCKET_PORT_END;
	int port;
	int sockfd[TEST_TCP_SOCKET_PORT_END];
	
	char hname[256];
	
	memset(hname, 0, 256);
	if(gethostname(hname, 256, pcBuf, iBufLen) == -1)
	{
		test_printf_error("test_tcp_bind_socket");
		return;
	}
	
	port = iPortBegin;
	while(port <= iPortEnd)
	{
		if((hp = gethostbyname(hname, pcBuf, iBufLen)) == NULL)
		{
			test_printf_error("test_tcp_bind_socket");
			goto fail;
		}
		
		memcpy(&(r_sa.sin_addr), hp->h_addr_list0, hp->h_length);
		r_sa.sin_family = AF_INET;
		r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
		
		if((sockfd[port-1] = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pcBuf, iBufLen)) == -1)
		{
			test_printf_error("test_tcp_bind_socket");
			goto fail;
		}
		
		if(set_reuseaddr(sockfd[port-1], pcBuf, iBufLen) == -1)
		{
			test_printf_error("test_tcp_bind_socket");
			goto fail;
		}
	
		if(bind(sockfd[port-1], (struct sockaddr*)&r_sa, sizeof(r_sa), pcBuf, iBufLen) == -1)
		{
			test_printf_error("test_tcp_bind_socket");
			goto fail;
		}
		port++;
	}
	
fail:
	if(port > iPortEnd)
		test_printf_success("test_tcp_bind_socket");
	for(i = iPortBegin; i <= port; i++)
	{
		netclose(sockfd[i-1], pcBuf, iBufLen);
	}
	return;
}

void test_tcp_listen_socket(char *pcBuf, int iBufLen)
{
	struct sockaddr_in r_sa;
	struct hostent *hp;
	int i;
	
	int iPortBegin = TEST_TCP_SOCKET_PORT_BEGIN;
	int iPortEnd = TEST_TCP_SOCKET_PORT_END;
	int port;
	int sockfd[TEST_TCP_SOCKET_PORT_END];
	
	char hname[256];
	
	memset(hname, 0, 256);
	if(gethostname(hname, 256, pcBuf, iBufLen) == -1)
	{
		test_printf_error("test_tcp_listen_socket");
		return;
	}
	
	port = iPortBegin;
	while(port <= iPortEnd)
	{
		if((hp = gethostbyname(hname, pcBuf, iBufLen)) == NULL)
		{
			test_printf_error("test_tcp_listen_socket");
			goto fail;
		}
		
		memcpy(&(r_sa.sin_addr), hp->h_addr_list0, hp->h_length);
		r_sa.sin_family = AF_INET;
		r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
		
		if((sockfd[port-1] = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pcBuf, iBufLen)) == -1)
		{
			test_printf_error("test_tcp_listen_socket");
			goto fail;
		}
		
		if(set_reuseaddr(sockfd[port-1], pcBuf, iBufLen) == -1)
		{
			test_printf_error("test_tcp_listen_socket");
			goto fail;
		}
	
		if(bind(sockfd[port-1], (struct sockaddr*)&r_sa, sizeof(r_sa), pcBuf, iBufLen) == -1)
		{
			test_printf_error("test_tcp_listen_socket");
			goto fail;
		}

		if(listen(sockfd[port-1], 10, pcBuf, iBufLen) == -1)
		{
			test_printf_error("test_tcp_listen_socket");
			goto fail;
		}
		port++;
	}
	
fail:
	if(port > iPortEnd)
		test_printf_success("test_tcp_listen_socket");
	for(i = iPortBegin; i <= port; i++)
	{
		netclose(sockfd[i-1], pcBuf, iBufLen);
	}
	return;
}


static void test_tcp_socket_thread_join(int ithread_handle)
{
	cyg_thread_info	info;
	cyg_uint16		id = 0;
	cyg_bool		rc;
	
	cyg_handle_t	next_thread = 0;
	cyg_uint16		next_id = 0;
	
	while(cyg_thread_get_next(&next_thread, &next_id) != false)
	{
		if(ithread_handle == next_thread)
		{
			id = next_id;
			break;
		}
	}
	
	while(1)
	{
		rc = cyg_thread_get_info(ithread_handle, id, &info);
		if(rc == false)
		{
			printf("Error get thread %d info\n", ithread_handle);
			break;
		}
		else
		{
			if((info.state & EXITED) != 0)
			{
				break;
			}
		}
		cyg_thread_yield();
	}
}

#define TEST_TCP_SOCKET_SERVER_BEGIN_PORT 40
#define TEST_TCP_ACCEPT_SOCKET_TIMES 1

#define TEST_TCP_SOCKET_SERVER_NUM (MAX_THREADS/2)
static TEST_TCP_SOCKET_DATA_T netdata[TEST_TCP_SOCKET_SERVER_NUM];
void test_tcp_accept_socket(void)
{
	int i, j;
	
	for(i = 0; i < TEST_TCP_SOCKET_SERVER_NUM; i++)
	{
		netdata[i].iport = TEST_TCP_SOCKET_SERVER_BEGIN_PORT+i;
		netdata[i].pbuf = (char*)NON_CACHE(g_RemoteNet_Buf1[i]);
		cyg_thread_create(THREAD_PRIORITY, &test_tcp_socket_server, (cyg_addrword_t)&netdata[i], 
						NULL, thread_stack[i], STACK_SIZE, &thread_handle[i], &thread[i]);
		cyg_thread_resume(thread_handle[i]);
	}
	
	j = TEST_TCP_SOCKET_SERVER_NUM;
	for(i = 0; i < TEST_TCP_SOCKET_SERVER_NUM; i++)
	{
		netdata[i + j].iport = TEST_TCP_SOCKET_SERVER_BEGIN_PORT + j + i;
		netdata[i + j].pbuf = (char*)NON_CACHE(g_RemoteNet_Buf1[i + j]);
		cyg_thread_create(THREAD_PRIORITY, &test_tcp_socket_client, (cyg_addrword_t)&netdata[i + j], 
						NULL, thread_stack[i + j], STACK_SIZE, &thread_handle[i + j], &thread[i + j]);
		cyg_thread_resume(thread_handle[i + j]);
	}
	
	j = TEST_TCP_SOCKET_SERVER_NUM;
	for(i = 0; i < TEST_TCP_SOCKET_SERVER_NUM; i++)
	{
		test_tcp_socket_thread_join(thread_handle[i]);
		cyg_thread_delete(thread_handle[i]);
		test_tcp_socket_thread_join(thread_handle[i + j]);
		cyg_thread_delete(thread_handle[i + j]);
	}
}

void test_tcp_socket_server(cyg_addrword_t pnetdata)
{
	int s, new_s, i;
	struct sockaddr_in sa, r_sa;
	int r_sa_l = sizeof(r_sa);
	struct hostent *hp;
	
	int threadid;
		
	int port = ((TEST_TCP_SOCKET_DATA_T*)pnetdata)->iport;
	char *pbuf = ((TEST_TCP_SOCKET_DATA_T*)pnetdata)->pbuf;
	
	threadid = cyg_thread_self();
	
	if((hp = gethostbyname(TEST_REMOTEFUNC_HOSTNAME, pbuf, RNT_BUFFER_LEN)) == NULL)
	{
		test_printf_error("test_tcp_socket_server");
		cyg_thread_exit();
	}
	
	memcpy(&(r_sa.sin_addr), hp->h_addr_list0, hp->h_length);
	r_sa.sin_family = AF_INET;
	r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
	
	if((s = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pbuf, RNT_BUFFER_LEN)) == -1)
	{
		test_printf_error("test_tcp_socket_server");
		cyg_thread_exit();
	}
	
	if(set_reuseaddr(s, pbuf, RNT_BUFFER_LEN) == -1)
	{
		test_printf_error("test_tcp_socket_server");
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}
	
	if(bind(s, (struct sockaddr*)&r_sa, sizeof(r_sa), pbuf, RNT_BUFFER_LEN) == -1)
	{
		test_printf_error("test_tcp_socket_server");
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}

	if(listen(s, 10, pbuf, RNT_BUFFER_LEN) == -1)
	{
		test_printf_error("test_tcp_socket_server");
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}
	
	for(i = 0; i < TEST_TCP_ACCEPT_SOCKET_TIMES; i++)
	{
		if((new_s = accept(s, (struct sockaddr*)&sa, (size_t*)&r_sa_l, pbuf, RNT_BUFFER_LEN)) == -1)
		{
			test_printf_error("test_tcp_socket_server");
			netclose(s, pbuf, RNT_BUFFER_LEN);
			cyg_thread_exit();
		}
		netclose(new_s, pbuf, RNT_BUFFER_LEN);
	}
	test_printf_success("test_tcp_socket_server");
	
	netclose(s, pbuf, RNT_BUFFER_LEN);
	cyg_thread_exit();
}

void test_tcp_socket_client(cyg_addrword_t pnetdata)
{
	int s, i, len;
	char msg;
	struct sockaddr_in sa, r_sa;
	struct hostent *hp;
	
	int threadid;
		
	int port = ((TEST_TCP_SOCKET_DATA_T*)pnetdata)->iport;
	char *pbuf = ((TEST_TCP_SOCKET_DATA_T*)pnetdata)->pbuf;
	
	threadid = cyg_thread_self();
	
	if((hp = gethostbyname(TEST_REMOTEFUNC_HOSTNAME, pbuf, RNT_BUFFER_LEN)) == NULL)
	{
		test_printf_error("test_tcp_socket_client");
		cyg_thread_exit();
	}
	
	memcpy(&(r_sa.sin_addr), hp->h_addr_list0, hp->h_length);
	r_sa.sin_family = AF_INET;
	r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
	
	memcpy(&(sa.sin_addr), hp->h_addr_list0, hp->h_length);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(IPPORT_USERRESERVED + port - TEST_TCP_SOCKET_SERVER_NUM);
    
    for(i = 0; i < TEST_TCP_ACCEPT_SOCKET_TIMES; i++)
    {
		if((s = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pbuf, RNT_BUFFER_LEN)) == -1)
		{
			test_printf_error("test_tcp_socket_client");
			cyg_thread_exit();
		}
		
		if(set_reuseaddr(s, pbuf, RNT_BUFFER_LEN) == -1)
		{
			test_printf_error("test_tcp_socket_client");
			netclose(s, pbuf, RNT_BUFFER_LEN);
			cyg_thread_exit();
		}
		
		if(bind(s, (struct sockaddr*)&r_sa, sizeof(r_sa), pbuf, RNT_BUFFER_LEN) == -1)
		{
			test_printf_error("test_tcp_socket_client");
			netclose(s, pbuf, RNT_BUFFER_LEN);
			cyg_thread_exit();
		}
    
	    while(connect(s, (struct sockaddr*)&sa, sizeof(sa), pbuf, RNT_BUFFER_LEN) == -1) NULL;
	    
		if((len = netread(s, &msg, sizeof(msg), pbuf, RNT_BUFFER_LEN)) == -1)
		{
			test_printf_error("test_tcp_socket_client");
			break;
		}
		if(len == 0)
		{
			netclose(s, pbuf, RNT_BUFFER_LEN);
			continue;
		}
    }
    
	test_printf_success("test_tcp_socket_client");
	cyg_thread_exit();
}


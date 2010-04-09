#include "wb_syslib_addon.h"
#include "hic_host.h"
#include "FTH_Int.h"
#include "RemoteFunc.h"
#include "RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteFuncTest.h"

void test_netwrite(void)
{
	test_printf_begin("test_netwrite");
	test_netwrite_entry();
	test_printf_end("test_netwrite");
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

static char msg[TEST_NETWRITE_SERVER_NUM][TEST_NETWRITE_MSG_LEN];
static TEST_NETWRITE_DATA_T netdata[TEST_NETREAD_SERVER_NUM];

void test_netwrite_entry(void)
{
	int iPort = TEST_NETWRITE_SERVER_BEGIN_PORT;
	int i;
	
	for(i = 0; i < TEST_NETWRITE_SERVER_NUM; i++)
	{
		netdata[i].iport = iPort+i;
		netdata[i].pbuf = (char*)NON_CACHE(g_RemoteNet_Buf1[i]);
		netdata[i].pwritebuf = msg[i];
		cyg_thread_create(THREAD_PRIORITY, &test_netwrite_server, (cyg_addrword_t)&netdata[i], 
						NULL, thread_stack[i], STACK_SIZE, &thread_handle[i], &thread[i]);
		cyg_thread_resume(thread_handle[i]);
	}
	
	tt_msleep(1000);
	
	for(; i < TEST_NETWRITE_SERVER_NUM * 2; i++)
	{
		netdata[i].iport = iPort+i;
		netdata[i].pbuf = (char*)NON_CACHE(g_RemoteNet_Buf1[i]);
		netdata[i].pwritebuf = msg[i];
		cyg_thread_create(THREAD_PRIORITY, &test_netwrite_client, (cyg_addrword_t)&netdata[i], 
						NULL, thread_stack[i], STACK_SIZE, &thread_handle[i], &thread[i]);
		cyg_thread_resume(thread_handle[i]);
	}
	
	for(i = 0; i < TEST_NETWRITE_SERVER_NUM * 2; i++)
	{
		test_tcp_socket_thread_join(thread_handle[i]);
		cyg_thread_delete(thread_handle[i]);
	}
}

void test_netwrite_server(cyg_addrword_t pnetdata)
{
	int s, new_s;
	struct sockaddr_in sa, r_sa;
	int r_sa_l = sizeof(r_sa);
	struct hostent *hp;
	
	int threadid;
		
	int port = ((TEST_NETWRITE_DATA_T*)pnetdata)->iport;
	char *pbuf = ((TEST_NETWRITE_DATA_T*)pnetdata)->pbuf;
	char *pwritebuf = ((TEST_NETWRITE_DATA_T*)pnetdata)->pwritebuf;
	
	threadid = cyg_thread_self();
	
	if((hp = gethostbyname(TEST_REMOTEFUNC_HOSTNAME, pbuf, RNT_BUFFER_LEN)) == NULL)
	{
		test_printf_error("test_netwrite_server");
		cyg_thread_exit();
	}
	
	memcpy(&(r_sa.sin_addr), hp->h_addr_list0, hp->h_length);
	r_sa.sin_family = AF_INET;
	r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
	
	if((s = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pbuf, RNT_BUFFER_LEN)) == -1)
	{
		test_printf_error("test_netwrite_server");
		cyg_thread_exit();
	}
	
	if(set_reuseaddr(s, pbuf, RNT_BUFFER_LEN) == -1)
	{
		test_printf_error("test_netwrite_server");
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}
	
	if(bind(s, (struct sockaddr*)&r_sa, sizeof(r_sa), pbuf, RNT_BUFFER_LEN) == -1)
	{
		netclose(s, pbuf, RNT_BUFFER_LEN);
		test_printf_error("test_netwrite_server");
		cyg_thread_exit();
	}

	if(listen(s, 10, pbuf, RNT_BUFFER_LEN) == -1)
	{
		netclose(s, pbuf, RNT_BUFFER_LEN);
		test_printf_error("test_netwrite_server");
		cyg_thread_exit();
	}
	
	if((new_s = accept(s, (struct sockaddr*)&sa, (size_t*)&r_sa_l, pbuf, RNT_BUFFER_LEN)) == -1)
	{
		netclose(s, pbuf, RNT_BUFFER_LEN);
		test_printf_error("test_netwrite_server");
		cyg_thread_exit();
	}
	
	if(netwrite(-1, pwritebuf, TEST_NETWRITE_MSG_LEN, pbuf, RNT_BUFFER_LEN) != -1)
	{
		test_printf_error("test_netwrite_server");
		goto fail;
	}
	
	if(netwrite(new_s, NULL, TEST_NETWRITE_MSG_LEN, pbuf, RNT_BUFFER_LEN) != -1)
	{
		test_printf_error("test_netwrite_server");
		goto fail;
	}
	
	if(netwrite(new_s, pwritebuf, 0, pbuf, RNT_BUFFER_LEN) != 0)
	{
		test_printf_error("test_netwrite_server");
		goto fail;
	}
	
	if(netwrite(new_s, pwritebuf, -1, pbuf, RNT_BUFFER_LEN) != -1)
	{
		test_printf_error("test_netwrite_server");
		goto fail;
	}
	
	if(netwrite(new_s, pwritebuf, TEST_NETWRITE_MSG_LEN, NULL, RNT_BUFFER_LEN) != -1)
	{
		test_printf_error("test_netwrite_server");
		goto fail;
	}
	
	if(netwrite(new_s, pwritebuf, TEST_NETWRITE_MSG_LEN, pbuf, 0) != -1)
	{
		test_printf_error("test_netwrite_server");
		goto fail;
	}
	
	test_printf_success("test_netwrite_server");
	
fail:
	netclose(new_s, pbuf, RNT_BUFFER_LEN);
	netclose(s, pbuf, RNT_BUFFER_LEN);
	cyg_thread_exit();
}

void test_netwrite_client(cyg_addrword_t pnetdata)
{
	int s, readlen;
	struct sockaddr_in sa, r_sa;
	struct hostent *hp;
	
	int threadid;
		
	int port = ((TEST_NETWRITE_DATA_T*)pnetdata)->iport;
	char *pbuf = ((TEST_NETWRITE_DATA_T*)pnetdata)->pbuf;
	char *pwritebuf = ((TEST_NETWRITE_DATA_T*)pnetdata)->pwritebuf;
	
	threadid = cyg_thread_self();
	
	if((hp = gethostbyname(TEST_REMOTEFUNC_HOSTNAME, pbuf, RNT_BUFFER_LEN)) == NULL)
	{
		test_printf_error("test_netwrite_client");
		cyg_thread_exit();
	}
	
	memcpy(&(r_sa.sin_addr), hp->h_addr_list0, hp->h_length);
	r_sa.sin_family = AF_INET;
	r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
	
	memcpy(&(sa.sin_addr), hp->h_addr_list0, hp->h_length);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(IPPORT_USERRESERVED + port - TEST_NETWRITE_SERVER_NUM);
	
	if((s = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pbuf, RNT_BUFFER_LEN)) == -1)
	{
		test_printf_error("test_netwrite_client");
		cyg_thread_exit();
	}
	
	if(set_reuseaddr(s, pbuf, RNT_BUFFER_LEN) == -1)
	{
		test_printf_error("test_netwrite_client");
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}
	
	if(bind(s, (struct sockaddr*)&r_sa, sizeof(r_sa), pbuf, RNT_BUFFER_LEN) == -1)
	{
		netclose(s, pbuf, RNT_BUFFER_LEN);
		test_printf_error("test_netwrite_client");
		cyg_thread_exit();
	}

	if(connect(s, (struct sockaddr*)&sa, sizeof(sa), pbuf, RNT_BUFFER_LEN) == -1)
	{
		netclose(s, pbuf, RNT_BUFFER_LEN);
		test_printf_error("test_netwrite_client");
		cyg_thread_exit();
	}
	
	readlen = netread(s, pwritebuf, TEST_NETWRITE_MSG_LEN, pbuf, RNT_BUFFER_LEN);
	netclose(s, pbuf, RNT_BUFFER_LEN);
	cyg_thread_exit();
}

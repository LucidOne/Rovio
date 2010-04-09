#include "wb_syslib_addon.h"
#include "hic_host.h"
#include "FTH_Int.h"
#include "RemoteFunc.h"
#include "RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteFuncTest.h"
#include "test_netselect.h"

void test_netselect(void)
{
	test_printf_begin("test_netselect");
	test_netselect_entry();
	test_printf_end("test_netselect");
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

static char msg[TEST_NETSELECT_SERVER_NUM][TEST_NETSELECT_MSG_LEN];
static TEST_NETSELECT_DATA_T netdata[TEST_NETSELECT_SERVER_NUM];

void test_netselect_entry(void)
{
	int iPort = TEST_NETSELECT_SERVER_BEGIN_PORT;
	int i;
	
	for(i = 0; i < TEST_NETSELECT_SERVER_NUM; i++)
	{
		netdata[i].iport = iPort+i;
		netdata[i].pbuf = (char*)NON_CACHE(g_RemoteNet_Buf1[i]);
		netdata[i].pnetselectbuf = msg[i];
		cyg_thread_create(THREAD_PRIORITY, &test_netselect_server, (cyg_addrword_t)&netdata[i], 
						NULL, thread_stack[i], STACK_SIZE, &thread_handle[i], &thread[i]);
		cyg_thread_resume(thread_handle[i]);
	}
	
	for(i = 0; i < TEST_NETSELECT_SERVER_NUM; i++)
	{
		test_tcp_socket_thread_join(thread_handle[i]);
		cyg_thread_delete(thread_handle[i]);
	}
}

void test_netselect_server(cyg_addrword_t pnetdata)
{
	int s, new_s, readlen;
	struct sockaddr_in sa, r_sa;
	int r_sa_l = sizeof(r_sa);
	struct hostent *hp;
	int ierr = 0;
	
	fd_set readset;
	
	int threadid;
		
	int port = ((TEST_NETSELECT_DATA_T*)pnetdata)->iport;
	char *pbuf = ((TEST_NETSELECT_DATA_T*)pnetdata)->pbuf;
	char *preadbuf = ((TEST_NETSELECT_DATA_T*)pnetdata)->pnetselectbuf;
	
	threadid = cyg_thread_self();
	
	if((hp = gethostbyname(TEST_REMOTEFUNC_HOSTNAME, pbuf, RNT_BUFFER_LEN)) == NULL)
	{
		test_printf_error("test_netselect_server");
		cyg_thread_exit();
	}
	
	memcpy(&(r_sa.sin_addr), hp->h_addr_list0, hp->h_length);
	r_sa.sin_family = AF_INET;
	r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
	
	if((s = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pbuf, RNT_BUFFER_LEN)) == -1)
	{
		test_printf_error("test_netselect_server");
		cyg_thread_exit();
	}
	
	if(set_reuseaddr(s, pbuf, RNT_BUFFER_LEN) == -1)
	{
		test_printf_error("test_netselect_server");
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}
	
	if(bind(s, (struct sockaddr*)&r_sa, sizeof(r_sa), pbuf, RNT_BUFFER_LEN) == -1)
	{
		test_printf_error("test_netselect_server");
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}

	if(listen(s, 10, pbuf, RNT_BUFFER_LEN) == -1)
	{
		test_printf_error("test_netselect_server");
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}
	
	if((new_s = accept(s, (struct sockaddr*)&sa, (size_t*)&r_sa_l, pbuf, RNT_BUFFER_LEN)) == -1)
	{
		test_printf_error("test_netselect_server");
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}
	
	while(1)
	{
		FD_ZERO(&readset);
		FD_SET(new_s, &readset);
		
		if(netselect(new_s + 1, &readset, NULL, NULL, NULL, pbuf, RNT_BUFFER_LEN) == -1)
		{
			test_printf_error("test_netselect_server");
			ierr = 1;
			break;
		}
		
		if(FD_ISSET(new_s, &readset))
		{
			readlen = netread(new_s, preadbuf, TEST_NETSELECT_MSG_LEN, pbuf, RNT_BUFFER_LEN);
			if(readlen < 0)
			{
				test_printf_error("test_netselect_server");
				ierr = 1;
				break;
			}
			if(readlen == 0)
			{
				break;
			}
			if(readlen > 0)
			{
				if(strcmp(preadbuf, TEST_NETSELECT_MSG) != 0)
				{
					ierr = 1;
					test_printf_error("test_netselect_server");
					break;
				}
			}
		}
	}
	
	if(ierr == 0)
		test_printf_success("test_netselect_server");
	
	netclose(new_s, pbuf, RNT_BUFFER_LEN);
	netclose(s, pbuf, RNT_BUFFER_LEN);
	cyg_thread_exit();
}

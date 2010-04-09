#include "wb_syslib_addon.h"
#include "hic_host.h"
#include "FTH_Int.h"
#include "RemoteFunc.h"
#include "RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteFuncTest.h"
#include "test_send.h"

void test_send(void)
{
	test_printf_begin("test_send");
	test_send_entry();
	test_printf_end("test_send");
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

static char msg[TEST_SEND_SERVER_NUM][TEST_SEND_MSG_LEN];
static TEST_SEND_DATA_T netdata[TEST_SEND_SERVER_NUM];

void test_send_entry(void)
{
	int iPort = TEST_SEND_SERVER_BEGIN_PORT;
	int i;
	
	for(i = 0; i < TEST_SEND_SERVER_NUM; i++)
	{
		netdata[i].iport = iPort+i;
		netdata[i].pbuf = (char*)NON_CACHE(g_RemoteNet_Buf1[i]);
		netdata[i].psendbuf = msg[i];
		cyg_thread_create(THREAD_PRIORITY, &test_send_server, (cyg_addrword_t)&netdata[i], 
						NULL, thread_stack[i], STACK_SIZE, &thread_handle[i], &thread[i]);
		cyg_thread_resume(thread_handle[i]);
	}
	
	for(i = 0; i < TEST_SEND_SERVER_NUM; i++)
	{
		test_tcp_socket_thread_join(thread_handle[i]);
		cyg_thread_delete(thread_handle[i]);
	}
}

void test_send_server(cyg_addrword_t pnetdata)
{
	int s, i, len, sendlen;
	struct sockaddr_in sa, r_sa;
	struct hostent *hp;
	
	int threadid;
		
	int port = ((TEST_SEND_DATA_T*)pnetdata)->iport;
	char *pbuf = ((TEST_SEND_DATA_T*)pnetdata)->pbuf;
	char *psendbuf = ((TEST_SEND_DATA_T*)pnetdata)->psendbuf;
	
	threadid = port;
	
    if(inet_aton(TEST_SEND_SERVER_ADDR, &sa.sin_addr, pbuf, RNT_BUFFER_LEN) == 0)
    {
		test_printf_error("test_send_server");
		cyg_thread_exit();
    }
    sa.sin_family = AF_INET;
    sa.sin_port = htons(IPPORT_USERRESERVED + port);
    
	if((hp = gethostbyname(TEST_REMOTEFUNC_HOSTNAME, pbuf, RNT_BUFFER_LEN)) == NULL)
	{
		test_printf_error("test_send_server");
		cyg_thread_exit();
	}
	
	memcpy(&(r_sa.sin_addr), hp->h_addr_list0, hp->h_length);
	r_sa.sin_family = AF_INET;
	r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
	
	if((s = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pbuf, RNT_BUFFER_LEN)) == -1)
	{
		test_printf_error("test_send_server");
		cyg_thread_exit();
	}
	
	if(set_reuseaddr(s, pbuf, RNT_BUFFER_LEN) == -1)
	{
		test_printf_error("test_send_server");
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}
	
	if(set_reuseaddr(s, pbuf, RNT_BUFFER_LEN) == -1)
	{
		test_printf_error("test_send_server");
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}
	
	if(bind(s, (struct sockaddr*)&r_sa, sizeof(r_sa), pbuf, RNT_BUFFER_LEN) == -1)
	{
		test_printf_error("test_send_server");
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}

    if(connect(s, (struct sockaddr*)&sa, sizeof(sa), pbuf, RNT_BUFFER_LEN) == -1)
    {
		test_printf_error("test_send_server");
		cyg_thread_exit();
    }
	
    for(i = 0; i < TEST_SEND_WRITE_TIMES; i++)
	{
		len = sprintf(psendbuf, "%s", TEST_SEND_MSG);
		len++;
		sendlen = send(s, psendbuf, len, 0, pbuf, RNT_BUFFER_LEN);
		if(sendlen < 0)
		{
			test_printf_error("test_send_server");
			break;
		}
	}
	
	if(i == TEST_SEND_WRITE_TIMES)
		test_printf_success("test_send_server");
	
	netclose(s, pbuf, RNT_BUFFER_LEN);
	cyg_thread_exit();
}

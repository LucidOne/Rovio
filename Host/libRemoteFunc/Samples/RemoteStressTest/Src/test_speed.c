#include "../../../../SysLib/Inc/wb_syslib_addon.h"
#include "../../../../libHIC_Host/Inc/hic_host.h"
#include "../../../libFunc_Through_HIC/Inc/FTH_Int.h"
#include "../../../libRemoteFunc/Inc/RemoteFunc.h"
#include "../../../libRemoteFunc/Inc/RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteNetTest.h"

cyg_tick_count_t tbegin, tend;
int icount = 0, totallen = 0, testcount = 0;

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

static char msg[TEST_SPEED_SERVER_NUM * 2][TEST_SPEED_MSG_LEN];
static TEST_SPEED_DATA_T netdata[TEST_SPEED_SERVER_NUM];

void test_speed_entry(void)
{
	int iPort = TEST_SPEED_SERVER_BEGIN_PORT;
	int i;
	
	tbegin = cyg_current_time();
	
	for(i = 0; i < TEST_SPEED_SERVER_NUM; i++)
	{
		netdata[i].iport = iPort+i;
		netdata[i].pbuf = (char*)NON_CACHE(g_RemoteNet_Buf1[i]);
		cyg_thread_create(THREAD_PRIORITY, &test_speed_server, (cyg_addrword_t)&netdata[i], 
						NULL, thread_stack[i], STACK_SIZE, &thread_handle[i], &thread[i]);
		cyg_thread_resume(thread_handle[i]);
	}
	
	for(i = 0; i < TEST_SPEED_SERVER_NUM; i++)
	{
		test_tcp_socket_thread_join(thread_handle[i]);
		cyg_thread_delete(thread_handle[i]);
	}
}


void test_speed_server(cyg_addrword_t pnetdata)
{
	int s, new_s, len;
	char *msg;
	struct sockaddr_in sa, r_sa;
	int r_sa_l = sizeof(r_sa);
	struct hostent *hp;
	
	struct sockaddr_in peername;
	int socklen;
	
	int threadid;
	int localcount = 0;
		
	int port = ((TEST_SPEED_DATA_T*)pnetdata)->iport;
	char *pbuf = ((TEST_SPEED_DATA_T*)pnetdata)->pbuf;
	
	threadid = port;
	
	msg = malloc(TEST_REMOTEFUNC_MSG_LEN);
	if(msg == NULL)
	{
		printf("test_speed_server(%d): malloc error\n", threadid);
		cyg_thread_exit();
	}
	
	if((hp = gethostbyname(TEST_REMOTEFUNC_HOSTNAME, pbuf, RNT_BUFFER_LEN)) == NULL)
	{
		printf("test_speed_server(%d): gethostbyname error!!!!!!\n", threadid);
		cyg_thread_exit();
	}
	
	memcpy(&(r_sa.sin_addr), hp->h_addr_list0, hp->h_length);
	r_sa.sin_family = AF_INET;
	r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
	printf("test_speed_server(%d): addr=%s port %d\n", threadid, 
			inet_ntoa(r_sa.sin_addr, pbuf, RNT_BUFFER_LEN), ntohs(r_sa.sin_port));
	
	if((s = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pbuf, RNT_BUFFER_LEN)) == -1)
	{
		printf("test_speed_server(%d): socket error!!!!!!\n", threadid);
		cyg_thread_exit();
	}
	printf("test_speed_server(%d): create socket success...\n", threadid);
	
	if(bind(s, (struct sockaddr*)&r_sa, sizeof(r_sa), pbuf, RNT_BUFFER_LEN) == -1)
	{
		printf("test_speed_server(%d): bind error!!!!!!\n", threadid);
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}
	printf("test_speed_server(%d): bind success...\n", threadid);

	if(listen(s, 10, pbuf, RNT_BUFFER_LEN) == -1)
	{
		printf("test_speed_server(%d): listen error\n", threadid);
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}
	printf("test_speed_server(%d): listen success...\n", threadid);
	
	while(1)
	{
		
		if((new_s = accept(s, (struct sockaddr*)&sa, (size_t*)&r_sa_l, pbuf, RNT_BUFFER_LEN)) == -1)
		{
			printf("test_speed_server(%d): accept error\n", threadid);
			netclose(s, pbuf, RNT_BUFFER_LEN);
			cyg_thread_exit();
		}
		printf("test_speed_server(%d): accept success...\n", threadid);
				
		while(1)
		{
			/* Test for netread & netwrite method */
			{
				int written_bytes;
				int bytes_left;
				int reconnect;
				
				len = TEST_REMOTEFUNC_MSG_LEN;
				memcpy(msg, g_LargeData, len);
				bytes_left = len;
				reconnect = 0;
				while(bytes_left > 0)
				{
					written_bytes = netwrite(new_s, msg+(len-bytes_left), bytes_left, pbuf, RNT_BUFFER_LEN);
					if(written_bytes <= 0)
					{
						printf("errno=%d\n", errno);
						if(errno == EINTR)
						{
							written_bytes = 0;
						}
						else
						{
							printf("test_speed_server(%d): netwrite error, reconnect!!!!!\n", threadid);
							reconnect = 1;
							break;
						}
					}
					
					bytes_left -= written_bytes;
					if(bytes_left > 0)
					{
						printf("write %d < %d left %d\n", written_bytes, bytes_left+written_bytes, bytes_left);
					}
				}
				if(reconnect == 1) break;
			}
			
			totallen += len;
			
			icount++;
			if((icount>=2000) && (icount%2000==0))
			{
				testcount++;
				tend = cyg_current_time();
				printf("rate=%03fk/s rate=%03df/s \n", (float)(totallen/((tend-tbegin)/100)/1024), (int)(icount/((tend-tbegin)/100)));
			}
			
			localcount++;
			if(localcount > 100 && localcount % 100 == 0)
			{
				printf("test_speed_server: %d\n", port);
			}
			
			if(testcount >= TEST_SPEED_TEST_TIMES)
			{
				printf("test_speed_server(%d): test complete......!!!\n", port);
				break;
			}
		}
		
		netclose(new_s, pbuf, RNT_BUFFER_LEN);
		break;
	}
	
	netclose(s, pbuf, RNT_BUFFER_LEN);
	free(msg);
	cyg_thread_exit();
}


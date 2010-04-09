#ifndef __REMOTEFUNCTEST_H__
#define __REMOTEFUNCTEST_H__

#define OPT_XIN_CLOCK			12000000
#define OPT_UPLL_OUT_CLOCK		(336 * 1000 * 1000)
#define OPT_APLL_OUT_CLOCK		(49152 * 1000)

#define MEM_ALIGNMENT	32
#define MEM_ALIGN_SIZE(size) (((size) + MEM_ALIGNMENT - 1) / MEM_ALIGNMENT * MEM_ALIGNMENT)

#define RNT_BUFFER_LEN 100000
#define THREAD_PRIORITY 10
#define STACK_SIZE 61440

static enum {                       // Thread state values
        RUNNING    = 0,          // Thread is runnable or running
        SLEEPING   = 1,          // Thread is waiting for something to happen
        COUNTSLEEP = 2,          // Sleep in counted manner
        SUSPENDED  = 4,          // Suspend count is non-zero
        CREATING   = 8,          // Thread is being created
        EXITED     = 16         // Thread has exited
}MY_CYG_THREAD_STATE;

/* For all */
#define TEST_REMOTEFUNC_PC_ADDR "10.132.249.4"
#define TEST_REMOTEFUNC_PC_INTADDR 0x04F9840A //10.132.249.4
#define TEST_REMOTEFUNC_LOCAL_ADDR "10.132.11.10"
#define TEST_REMOTEFUNC_SERVER_BEGIN_PORT 40

#define MAX_THREADS 10

#define TEST_REMOTEFUNC_MSG "Hello, World!"
#define TEST_REMOTEFUNC_MSG_LEN 14
#define TEST_REMOTEFUNC_MSG_PART_LEN 7
#define TEST_REMOTEFUNC_MSG_PARTS 2

#define TEST_REMOTEFUNC_INTERFACE_ETH1 "eth1"
#define TEST_REMOTEFUNC_INTERFACE_ETH1_LEN 5
#define TEST_REMOTEFUNC_GATEWAY "10.132.1.254"

#define TEST_REMOTEFUNC_HOSTNAME "zzqing"

/* For test_netread */
#define TEST_NETREAD_SERVER_BEGIN_PORT TEST_REMOTEFUNC_SERVER_BEGIN_PORT

#define TEST_NETREAD_SERVER_NUM (MAX_THREADS/2)

#define TEST_NETREAD_MSG TEST_REMOTEFUNC_MSG
#define TEST_NETREAD_MSG_LEN TEST_REMOTEFUNC_MSG_LEN

/* For test_netwrite */
#define TEST_NETWRITE_SERVER_ADDR TEST_REMOTEFUNC_PC_ADDR
#define TEST_NETWRITE_SERVER_BEGIN_PORT TEST_REMOTEFUNC_SERVER_BEGIN_PORT

#define TEST_NETWRITE_SERVER_NUM (MAX_THREADS/2)
#define TEST_NETWRITE_WRITE_TIMES 100

#define TEST_NETWRITE_MSG TEST_REMOTEFUNC_MSG
#define TEST_NETWRITE_MSG_LEN TEST_REMOTEFUNC_MSG_LEN

/* For test_send */
#define TEST_SEND_SERVER_ADDR TEST_REMOTEFUNC_PC_ADDR
#define TEST_SEND_SERVER_BEGIN_PORT TEST_REMOTEFUNC_SERVER_BEGIN_PORT

#define TEST_SEND_SERVER_NUM (MAX_THREADS/2)
#define TEST_SEND_WRITE_TIMES 100

#define TEST_SEND_MSG TEST_REMOTEFUNC_MSG
#define TEST_SEND_MSG_LEN TEST_REMOTEFUNC_MSG_LEN

/* For test_recv */
#define TEST_RECV_SERVER_BEGIN_PORT TEST_REMOTEFUNC_SERVER_BEGIN_PORT

#define TEST_RECV_SERVER_NUM (MAX_THREADS/2)

#define TEST_RECV_MSG TEST_REMOTEFUNC_MSG
#define TEST_RECV_MSG_LEN TEST_REMOTEFUNC_MSG_LEN

/* For test_sendmsg */
#define TEST_SENDMSG_SERVER_ADDR TEST_REMOTEFUNC_PC_ADDR
#define TEST_SENDMSG_SERVER_BEGIN_PORT TEST_REMOTEFUNC_SERVER_BEGIN_PORT

#define TEST_SENDMSG_SERVER_NUM (MAX_THREADS/2)
#define TEST_SENDMSG_WRITE_TIMES 100

#define TEST_SENDMSG_MSG TEST_REMOTEFUNC_MSG
#define TEST_SENDMSG_MSG_LEN TEST_REMOTEFUNC_MSG_LEN
#define TEST_SENDMSG_MSG_PART_LEN TEST_REMOTEFUNC_MSG_PART_LEN
#define TEST_SENDMSG_MSG_PARTS TEST_REMOTEFUNC_MSG_PARTS

/* For test_recvmsg */
#define TEST_RECVMSG_SERVER_BEGIN_PORT TEST_REMOTEFUNC_SERVER_BEGIN_PORT

#define TEST_RECVMSG_SERVER_NUM (MAX_THREADS/2)

#define TEST_RECVMSG_MSG TEST_REMOTEFUNC_MSG
#define TEST_RECVMSG_MSG_LEN TEST_REMOTEFUNC_MSG_LEN
#define TEST_RECVMSG_MSG_PART_LEN TEST_REMOTEFUNC_MSG_PART_LEN
#define TEST_RECVMSG_MSG_PARTS TEST_REMOTEFUNC_MSG_PARTS

/* For test_recvfrom */
#define TEST_RECVFROM_SERVER_BEGIN_PORT TEST_REMOTEFUNC_SERVER_BEGIN_PORT

#define TEST_RECVFROM_SERVER_NUM (MAX_THREADS/2)
// To ensure we can recv the data from client,
// We just recv one times, and client send 10 times
#define TEST_RECVFROM_RECV_TIMES 1

#define TEST_RECVFROM_MSG TEST_REMOTEFUNC_MSG
#define TEST_RECVFROM_MSG_LEN TEST_REMOTEFUNC_MSG_LEN

/* For test_sendto */
#define TEST_SENDTO_SERVER_ADDR TEST_REMOTEFUNC_PC_ADDR
#define TEST_SENDTO_SERVER_BEGIN_PORT TEST_REMOTEFUNC_SERVER_BEGIN_PORT

#define TEST_SENDTO_SERVER_NUM (MAX_THREADS/2)
// To ensure server can recv the data from client,
// Server just recv one times, and we send 10 times
#define TEST_SENDTO_WRITE_TIMES 10

#define TEST_SENDTO_MSG TEST_REMOTEFUNC_MSG
#define TEST_SENDTO_MSG_LEN TEST_REMOTEFUNC_MSG_LEN

/* For test_netselect */
#define TEST_NETSELECT_SERVER_BEGIN_PORT TEST_REMOTEFUNC_SERVER_BEGIN_PORT

#define TEST_NETSELECT_SERVER_NUM (MAX_THREADS/2)

#define TEST_NETSELECT_MSG TEST_REMOTEFUNC_MSG
#define TEST_NETSELECT_MSG_LEN TEST_REMOTEFUNC_MSG_LEN

/* For test_getsockname */
#define TEST_GETPEERNAME_ADDR TEST_REMOTEFUNC_PC_INTADDR
#define TEST_GETPEERNAME_PORT TEST_REMOTEFUNC_SERVER_BEGIN_PORT

/* For test_getsockopt */
#define TEST_SETSOCKOPT_REUSEADDR_PORT TEST_REMOTEFUNC_SERVER_BEGIN_PORT
#define TEST_SETSOCKOPT_REUSEADDR_TIMES 3

/* For test_netfcntl */
#define TEST_NETFCNTL_SERVER_ADDR TEST_REMOTEFUNC_PC_ADDR
#define TEST_NETFCNTL_SERVER_BEGIN_PORT TEST_REMOTEFUNC_SERVER_BEGIN_PORT

#define TEST_NETFCNTL_SERVER_NUM (MAX_THREADS/2)
#define TEST_NETFCNTL_WRITE_TIMES 100

#define TEST_NETFCNTL_MSG TEST_REMOTEFUNC_MSG
#define TEST_NETFCNTL_MSG_LEN TEST_REMOTEFUNC_MSG_LEN

/* For test_netioctl */
#define TEST_NETIOCTL_INTERFACE_NAME TEST_REMOTEFUNC_INTERFACE_ETH1
#define TEST_NETIOCTL_IP_ADDR "192.168.0.10"
#define TEST_NETIOCTL_IP_NETMASK "255.255.255.0"

/* For test_netioctl_withbuf */
#define TEST_NETIOCTL_WITH_BUF_DST "0.0.0.0"
#define TEST_NETIOCTL_WITH_BUF_MASK "0.0.0.0"
#define TEST_NETIOCTL_WITH_BUF_GATEWAY "10.132.1.254"

/* For test_wb740getgateway */
#define TEST_WB740GETGATEWAY_INTREFACE TEST_REMOTEFUNC_INTERFACE_ETH1
#define TEST_WB740GETGATEWAY_INTREFACE_LEN TEST_REMOTEFUNC_INTERFACE_ETH1_LEN
#define TEST_WB740GETGATEWAY_GATEWAY TEST_REMOTEFUNC_GATEWAY


#define test_printf_begin(strFunc)\
		printf("<<**********%-17s begin**********>>\n", strFunc);
#define test_printf_end(strFunc)\
	printf("<<**********%-17s   end**********>>\n\n", strFunc);

#define test_printf_error(strFunc)\
		printf("<<%-30s: failed......%d!!!\n", strFunc, __LINE__)
#define test_printf_success(strFunc)\
		printf("<<%-30s: passed......!!!\n", strFunc)

#define PTI printf("%s--%d\n", __FILE__, __LINE__);


extern CHAR g_RemoteNet_Buf[MEM_ALIGN_SIZE(RNT_BUFFER_LEN)];
extern CHAR g_RemoteNet_Buf1[MAX_THREADS][MEM_ALIGN_SIZE(RNT_BUFFER_LEN)];

extern char thread_stack[MAX_THREADS][MEM_ALIGN_SIZE(STACK_SIZE)];
extern cyg_handle_t thread_handle[MAX_THREADS];
extern cyg_thread thread[MAX_THREADS];

#include "test_inet_addr.h"
#include "test_gethostbyname.h"
#include "test_tcp_socket.h"
#include "test_udp_socket.h"
#include "test_netread.h"
#include "test_netwrite.h"
#include "test_send.h"
#include "test_recv.h"
#include "test_sendmsg.h"
#include "test_recvmsg.h"
#include "test_recvfrom.h"
#include "test_sendto.h"
#include "test_netselect.h"
#include "test_getsockname.h"
#include "test_getsockopt.h"
#include "test_netfcntl.h"
#include "test_netioctl.h"
#include "test_netioctl_withbuf.h"
#include "test_wb740getgateway.h"
#include "test_wb740reboot.h"


#endif

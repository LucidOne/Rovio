#ifndef __REMOTENETTEST_H__
#define __REMOTENETTEST_H__

#define OPT_XIN_CLOCK			12000000
#define OPT_UPLL_OUT_CLOCK		(336 * 1000 * 1000)
#define OPT_APLL_OUT_CLOCK		(49152 * 1000)

#define MEM_ALIGNMENT	32
#define MEM_ALIGN_SIZE(size) (((size) + MEM_ALIGNMENT - 1) / MEM_ALIGNMENT * MEM_ALIGNMENT)

#define RNT_BUFFER_LEN 100000
#define THREAD_PRIORITY 10
#define STACK_SIZE 61440

/* For all */
#define TEST_REMOTEFUNC_SERVER_BEGIN_PORT 40

#define MAX_THREADS 10
#define TEST_REMOTEFUNC_MSG_LEN (40*1024)

#define TEST_REMOTEFUNC_HOSTNAME "zzqing"

#define TEST_SPEED_TEST_TIMES 5

/* For test_speed */
#define TEST_SPEED_SERVER_BEGIN_PORT TEST_REMOTEFUNC_SERVER_BEGIN_PORT

#define TEST_SPEED_SERVER_NUM (MAX_THREADS/2)

#define TEST_SPEED_MSG_LEN TEST_REMOTEFUNC_MSG_LEN


static enum {                       // Thread state values
        RUNNING    = 0,          // Thread is runnable or running
        SLEEPING   = 1,          // Thread is waiting for something to happen
        COUNTSLEEP = 2,          // Sleep in counted manner
        SUSPENDED  = 4,          // Suspend count is non-zero
        CREATING   = 8,          // Thread is being created
        EXITED     = 16         // Thread has exited
}MY_CYG_THREAD_STATE;


typedef struct
{
	int		iport;
	char	*pbuf;
}TEST_SPEED_DATA_T;



extern char g_LargeData[];

extern CHAR g_RemoteNet_Buf[MEM_ALIGN_SIZE(RNT_BUFFER_LEN)];
extern CHAR g_RemoteNet_Buf1[MAX_THREADS][MEM_ALIGN_SIZE(RNT_BUFFER_LEN)];

extern char thread_stack[MAX_THREADS][MEM_ALIGN_SIZE(STACK_SIZE)];
extern cyg_handle_t thread_handle[MAX_THREADS];
extern cyg_thread thread[MAX_THREADS];


#include "test_speed.h"

#endif

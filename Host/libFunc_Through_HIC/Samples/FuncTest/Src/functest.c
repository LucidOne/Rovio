#include "wb_syslib_addon.h"
#include "hic_host.h"
#include "FTH_Int.h"

#if 1
static const char g_RtspServer_conf[] = 
"aasdfsadfasdf";
#endif


#define FHT_TEST_NUMBER_OF_WORKERS    1
#define FTH_TEST_WORKER_PRIORITY     10
#define FTH_TEST_WORKER_STACKSIZE    (30*1024)

static unsigned char FTH_Test_Worker_Stacks[FHT_TEST_NUMBER_OF_WORKERS][FTH_TEST_WORKER_STACKSIZE];
static cyg_handle_t FTH_Test_Worker_Handles[FHT_TEST_NUMBER_OF_WORKERS];
static cyg_thread FTH_Test_Worker_Threads[FHT_TEST_NUMBER_OF_WORKERS];

#define FTH_TEST_HIC_BUFFER_LEN 30*1024
__align (32)
CHAR g_FTHTest_Hic_Buf[FHT_TEST_NUMBER_OF_WORKERS][FTH_TEST_HIC_BUFFER_LEN];
char g_FTHTest_Result[FHT_TEST_NUMBER_OF_WORKERS][FTH_TEST_HIC_BUFFER_LEN];

static void FTH_Test_worker(cyg_addrword_t data)
{
	int i;
	cyg_handle_t threadid;
	
	threadid = cyg_thread_self();
	printf("threadid=%d\n", threadid);
	
	while(1);
    cyg_thread_exit();
}

void FTH_Test_Start(void)
{
    int i;

    for (i = 0; i < FHT_TEST_NUMBER_OF_WORKERS; i++) 
    {
        cyg_thread_create(FTH_TEST_WORKER_PRIORITY, &FTH_Test_worker, i, "FTH_Test_worker",
                          FTH_Test_Worker_Stacks[i], FTH_TEST_WORKER_STACKSIZE,
                          &(FTH_Test_Worker_Handles[i]), &(FTH_Test_Worker_Threads[i]));
        cyg_thread_resume(FTH_Test_Worker_Handles[i]);
    }
}

int main(void)
{
    char buf[128];
    unsigned int len, totallen = 0;
    char *pc = buf;
    
	/* <1> Enable flash. */
	sysFlushCache (I_D_CACHE);
	sysDisableCache ();
	sysEnableCache (CACHE_WRITE_BACK);

	printf ("Create test thread for func_through_hic\n");

    FTH_Init();
    
    FTH_Test_Start();
	while(1);
    return 0;
}

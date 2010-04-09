#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "cyg/hal/hal_arch.h"           // CYGNUM_HAL_STACK_SIZE_TYPICAL
#include "cyg/kernel/kapi.h"
#include "cyg/infra/testcase.h"

#include "wblib.h"
#include "wb_fmi.h"

#define CACHE_ON

#define SD_TEST
//#define SM_TEST
//#define SM_TEST_2K
//#define CF_TEST

#define FMI_TEST_SIZE	1024 * 256

void fmi_test(cyg_addrword_t data)
{
	int rate, i, tmp, time0;
	unsigned int btime, etime, sector;
	unsigned char *pSrc, *pDst;

	pSrc = (unsigned char *)0x10200000;
	pDst = (unsigned char *)0x10500000;

	tmp = 0;
	for (i=0; i<FMI_TEST_SIZE; i++)
	{
		*(pSrc + i) = (tmp + i) & 0xff;
		if (((i+1) % 0xff) == 0)
			tmp++;
	}

	// initial FMI
	fmiInitDevice();

#ifdef SD_TEST
	memset(pDst, 0, FMI_TEST_SIZE);

	sector = fmiSDDeviceInit();
	printf("total sectors [%d]\n", sector);

	sector = FMI_TEST_SIZE / 512;

	btime = cyg_current_time();
	fmiSD_Write(4000, FMI_TEST_SIZE/512, (unsigned int)pSrc);
	etime = cyg_current_time();
	time0 = etime - btime;
	if (time0 == 0)	time0 = 1;
	rate = FMI_TEST_SIZE / 1024 / time0 * 100;
	printf("SD write => %d Kbytes/sec [%d]\n", rate, time0);

	btime = cyg_current_time();
	fmiSD_Read(4000, FMI_TEST_SIZE/512, (unsigned int)pDst);
	etime = cyg_current_time();
	time0 = etime - btime;
	if (time0 == 0)	time0 = 1;
	rate = FMI_TEST_SIZE / 1024 / time0 * 100;
	printf("SD read => %d Kbytes/sec\n", rate);

	for (i=0; i<FMI_TEST_SIZE; i++)
	{
		if (*(pSrc + i) != *(pDst + i))
		{
			printf("error!! <%d> Src[%x], Dst[%x]\n", i, *(pSrc + i), *(pDst + i));
			exit(0);
		}
	}

#endif

#ifdef SM_TEST
	memset(pDst, 0, FMI_TEST_SIZE);

	sector = fmiSMDeviceInit(1);
	fmiSM_ChipErase();
	printf("total sectors [%d]\n", sector);

	sector = FMI_TEST_SIZE / 512;
	btime = cyg_current_time();
	fmiSM_Write(300, FMI_TEST_SIZE/512, (unsigned int)pSrc);
	etime = cyg_current_time();
	time0 = etime - btime;
	if (time0 == 0)	time0 = 1;
	rate = FMI_TEST_SIZE / 1024 / time0 * 100;
	printf("SM write => %d Kbytes/sec\n", rate);

	btime = cyg_current_time();
	fmiSM_Read(300, FMI_TEST_SIZE/512, (unsigned int)pDst);
	etime = cyg_current_time();
	time0 = etime - btime;
	if (time0 == 0)	time0 = 1;
	rate = FMI_TEST_SIZE / 1024 / time0 * 100;
	printf("SM read => %d Kbytes/sec\n", rate);

	for (i=0; i<FMI_TEST_SIZE; i++)
	{
		if (*(pSrc + i) != *(pDst + i))
		{
			printf("error!! <%d> Src[%x], Dst[%x]\n", i, *(pSrc + i), *(pDst + i));
			exit(0);
		}
	}

#endif

#ifdef SM_TEST_2K
	memset(pDst, 0, FMI_TEST_SIZE);

	sector = fmiSMDeviceInit(0);
	fmiSM_ChipErase();
	printf("total sectors [%d]\n", sector);

	sector = FMI_TEST_SIZE / 512;
	btime = cyg_current_time();
	fmiSM_Write(300, FMI_TEST_SIZE/512, (unsigned int)pSrc);
	etime = cyg_current_time();
	time0 = etime - btime;
	if (time0 == 0)	time0 = 1;
	rate = FMI_TEST_SIZE / 1024 / time0 * 100;
	printf("SM 2K write => %d Kbytes/sec\n", rate);

	btime = cyg_current_time();
	fmiSM_Read(300, FMI_TEST_SIZE/512, (unsigned int)pDst);
	etime = cyg_current_time();
	time0 = etime - btime;
	if (time0 == 0)	time0 = 1;
	rate = FMI_TEST_SIZE / 1024 / time0 * 100;
	printf("SM 2K read => %d Kbytes/sec\n", rate);

	for (i=0; i<FMI_TEST_SIZE; i++)
	{
		if (*(pSrc + i) != *(pDst + i))
		{
			printf("error!! <%d> Src[%x], Dst[%x]\n", i, *(pSrc + i), *(pDst + i));
			exit(0);
		}
	}

#endif

#ifdef CF_TEST
	memset(pDst, 0, FMI_TEST_SIZE);

	sector = fmiCFDeviceInit();
	printf("total sectors [%d]\n", sector);

	sector = FMI_TEST_SIZE / 512;

	btime = cyg_current_time();
	fmiCF_Write(300, FMI_TEST_SIZE/512, (unsigned int)pSrc);
	etime = cyg_current_time();
	time0 = etime - btime;
	if (time0 == 0)	time0 = 1;
	rate = FMI_TEST_SIZE / 1024 / time0 * 100;
	printf("CF write => %d Kbytes/sec[%d]\n", rate, time0);

	btime = cyg_current_time();
	fmiCF_Read(300, FMI_TEST_SIZE/512, (unsigned int)pDst);
	etime = cyg_current_time();
	time0 = etime - btime;
	if (time0 == 0)	time0 = 1;
	rate = FMI_TEST_SIZE / 1024 / time0 * 100;
	printf("CF read => %d Kbytes/sec\n", rate);

	for (i=0; i<FMI_TEST_SIZE; i++)
	{
		if (*(pSrc + i) != *(pDst + i))
		{
			printf("error!! <%d> Src[%x], Dst[%x]\n", i, *(pSrc + i), *(pDst + i));
			exit(0);
		}
	}

#endif
}


__align(4) static UINT8 stack[4096];

int main()
{

	cyg_handle_t	thread_handle;
	cyg_thread		thread_holder;

	sysEnableCache(CACHE_WRITE_BACK);

    CYG_TEST_INIT();
 	cyg_thread_create(10, fmi_test, 0, "fmi_test",
        				(void *)stack, 4096, &thread_handle, &thread_holder);
    cyg_thread_resume(thread_handle);
    CYG_TEST_PASS_FINISH("Thread 0 OK");
}



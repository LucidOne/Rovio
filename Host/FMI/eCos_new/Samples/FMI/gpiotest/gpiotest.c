#include "stdio.h"
#include "cyg/hal/hal_arch.h"           // CYGNUM_HAL_STACK_SIZE_TYPICAL
#include "cyg/kernel/kapi.h"
#include "cyg/infra/testcase.h"

#include "wblib.h"
#include "wb_fmi.h"

#define SD_TEST
#define SM_TEST

unsigned int volatile SD_Flag=0, SM_Flag=0;

void SD_InsertDevice()
{
	SD_Flag = 1;
}

void SD_RemoveDevice()
{
	SD_Flag = 0;
}

void SM_InsertDevice()
{
	SM_Flag = 1;
}

void SM_RemoveDevice()
{
	SM_Flag = 0;
}


void fmi_demo(cyg_addrword_t data)
{
	unsigned char *ptr;
	int status, count=0;

	ptr = (unsigned char *)0x10100000;

	fmiInitDevice();

#ifdef SD_TEST
	fmiSetCallBack(FMI_SD_CARD, (void *)SD_RemoveDevice, (void *)SD_InsertDevice);
	status = fmiSDDeviceInit();
	if (status == FMI_NO_SD_CARD)
	{
		printf("No SD Card!!\n");
		while(1)
		{
			if (SD_Flag == 1)
			{
				fmiSDDeviceInit();
				printf("SD card insert!!\n");
				break;
			}
		}
	}
	else
		SD_Flag = 1;

	while(1)
	{
		printf("%d\n", ++count);
		status = fmiSD_Read(1000, 512, (unsigned int)ptr);
		if (status == FMI_NO_SD_CARD)
		{
			printf("SD card remove!!\n");
			while(1)
			{
				if (SD_Flag == 1)
				{
					fmiSDDeviceInit();
					printf("SD card insert!!\n");
					break;
				}
			}
		}
	}
#endif

#ifdef SM_TEST
	fmiSetCallBack(FMI_SM_CARD, (void *)SM_RemoveDevice, (void *)SM_InsertDevice);
	status = fmiSMDeviceInit(1);
	if (status == FMI_NO_SM_CARD)
	{
		printf("No SM Card!!\n");
		while(1)
		{
			if (SM_Flag == 1)
			{
				fmiSMDeviceInit(1);
				printf("SM card insert!!\n");
				break;
			}
		}
	}
	else
		SM_Flag = 1;

	while(1)
	{
		printf("%d\n", ++count);
		status = fmiSM_Read(1000, 512, (unsigned int)ptr);
		if (status == FMI_NO_SM_CARD)
		{
			printf("SM card remove!!\n");
			while(1)
			{
				if (SM_Flag == 1)
				{
					fmiSMDeviceInit(1);
					printf("SM card insert!!\n");
					break;
				}
			}
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
 	cyg_thread_create(10, fmi_demo, 0, "fmi_demo",
        				(void *)stack, 4096, &thread_handle, &thread_holder);
    cyg_thread_resume(thread_handle);
    CYG_TEST_PASS_FINISH("Thread 0 OK");
}



/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2002 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/

/****************************************************************************
 *
 * FILENAME
 *     W90N740.c
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     This file contains initializing steps for W90N740.
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *     None
 *
 * HISTORY
 *     11/02/04		 Ver 1.0 Created by PC34 xhchen
 *
 * REMARK
 *     None
 **************************************************************************/

#include "../../Inc/Platform.h"


#include <rt_misc.h>
#if 1
__value_in_regs struct __initial_stackheap __user_initial_stackheap(
	unsigned R0, unsigned SP, unsigned R2, unsigned SL)
{
	struct __initial_stackheap config;
#if 0
	config.heap_base =		0x00240000;
	config.heap_limit =		0x00800000;
	config.stack_base =		0x00800000;
	config.stack_limit =	0x00100000;
#else
	extern unsigned int Image$$ZI$$Limit;
	config.heap_base =		(unsigned int)&Image$$ZI$$Limit;	//0x00240000;
	config.heap_limit =		0x00700000;
	config.stack_base =		0x00800000;
	config.stack_limit =	0x00700000;
#endif
	return config;
}
#else
extern unsigned int Image$$ZI$$Limit;
__value_in_regs struct R0_R3 {unsigned heap_base, stack_base, heap_limit, stack_limit;}
    __user_initial_stackheap(unsigned int R0, unsigned int SP, unsigned int R2, unsigned int SL)
{
    struct R0_R3 config;

    //config.heap_base = 0x00060000;
    config.heap_base = (unsigned int)&Image$$ZI$$Limit;
    config.stack_base = SP;

return config;
}
#endif


static VOID sysInitCFI(VOID)
{
	D_TRACE (0, "sysInitCFI\n");
	/* enable the external I/O bank, set byte address alignment, set 32 bit data bus */
	*((unsigned int volatile *) EXTIO_BANK) = (unsigned int) (EXTIO_BASE_ADDR | EXTIO_SIZE |
											   EXTIO_BUS_WIDTH_32 | EXTIO_tACC | EXTIO_tCOH |
											   EXTIO_tACS | EXTIO_tCOS
											   );

	{
		sysPrintf ("reg = %x\n", *(volatile UINT32 *)0xFFF01018);
	}
}


BOOL sysInitialize(VOID)
{
	INT32 nReturn;

	/* Initialize UART. */
	{
		WB_UART tUART =
		{
			15000000,		//freq
			115200,			//baud_rate
			WB_DATA_BITS_8,	//data_bits
			WB_STOP_BITS_1,	//stop_bits
			WB_PARITY_NONE,	//parity
			LEVEL_1_BYTE,	//rx_trigger_level
		};
		nReturn = WB_InitializeUART(&tUART);
		if (nReturn != Successful) return FALSE;
	}

	sysPrintf("Uart..ok\n");

	/* IRQ for W99702. */
	WB_SetInterruptType (W90N740_nIRQ0, LOW_LEVEL_SENSITIVE);
	
	/* Initialize TIMER1. */
	{
		WB_SetTimerReferenceClock(TIMER1, 15000000);
		WB_StartTimer(TIMER1, 1000, PERIODIC_MODE);
	}
	
	sysPrintf("Timer..ok\n");

	sysInitCFI();

	sysPrintf("CFI..ok\n");
	return TRUE;
}


INT sysPrintf (CONST CHAR *cpcFormat, ...)
{
	static char acBuffer[256];
	INT rt;
	va_list ap;
	va_start (ap, cpcFormat);
	rt = vsnprintf (acBuffer, sizeof (acBuffer), cpcFormat, ap);
	va_end (ap);
	WB_Printf("%s", acBuffer);
	return 0;
}


INT sysGetChar (VOID)
{
	return getchar ();
}


UINT32 sysGetTickCount(VOID)
{
	return WB_GetTicks(TIMER1);
}


VOID sysMSleep(UINT32 uMilliSecond)
{
	UINT32 uMilliSecondRemain = uMilliSecond;
	UINT32 uTicksBase = sysGetTickCount();
	UINT32 uTicks;

	//Check system ticks untill "uMilliSecond" time elapsed.
	while (uMilliSecondRemain > 0
		&& uMilliSecondRemain <= uMilliSecond)
	{
		uTicks = sysGetTickCount();
		uMilliSecondRemain = uMilliSecond - (uTicks - uTicksBase);
		//when ticks rewind,  uTicks < uTicksBase
		//ulMilliSecondRemain = ulMilliSecond - (uTicks + 0x100000000 - uTicksBase)
		//It must be in the same case.
	}
}


VOID *sysMalloc(size_t szSize)
{
	VOID *p;
	//sysDisableInterrupt (2);
	p = malloc (szSize);
	//sysEnableInterrupt (2);
	return p;
}

VOID *sysRealloc(void *pMem, size_t szSize)
{
	VOID *p;
	//sysDisableInterrupt (2);
	p = realloc (pMem, szSize);
	//sysEnableInterrupt (2);
	return p;
}

VOID sysFree(void *pMem)
{
	//sysDisableInterrupt (2);
	free (pMem);
	//sysEnableInterrupt (2);
}


//Waiting object
VOID sysInitWaitObj (SYS_WAIT_OBJ_T *pWaitObj)
{
	pWaitObj->sWaitObj.bWait = FALSE;
}


INT sysWait (SYS_WAIT_OBJ_T *pWaitObj, UINT32 uTimeout_MSec)
{
	/*
		On some operation system, the following code may be implemented
		by semaphore or other more effective code.
	 */


	INT nReturn = 0;
	UINT32 uTicks = sysGetTickCount ();
	while (pWaitObj->sWaitObj.bWait == FALSE)
	{
		if (sysGetTickCount () - uTicks > uTimeout_MSec)
		{
			nReturn = -1;
			break;
		}
	}
	
	if (nReturn == 0)
		pWaitObj->sWaitObj.bWait = FALSE;


	return nReturn;
}


VOID sysWakeup (SYS_WAIT_OBJ_T *pWaitObj)
{
	pWaitObj->sWaitObj.bWait = TRUE;
}

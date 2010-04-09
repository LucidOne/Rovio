/*
 * semi_add.c: Other added functions for semihost.
 * $Id: Timer.c,v 1.1 2006/01/17 09:42:12 xhchen Exp $
 *
 * Copyright (C) xhchen@winbond.com.tw
 * All rights reserved.
 */


#include "../../Inc/Platform.h"
#include "../Inc/InInc/W90N740.h"

INT32 sysDisableInterrupt (UINT32 uIntNo);
INT32 sysEnableInterrupt (UINT32 uIntNo);
PVOID sysInstallISR (INT32 nIntTypeLevel, UINT32 uIntNo, PVOID fnNewISR);
INT32 sysSetInterruptType (UINT32 uIntNo, UINT32 uIntSourceType);


static time_t g_tCurrentTime_InSec = 0;
static time_t g_tCurrentTime_InMSec = 0;

VOID sysTimerinterrupt(VOID)
{
	//UART_Printf("In timer %x.\n");
	TISR = 2;

	g_tCurrentTime_InMSec++;
	if (g_tCurrentTime_InMSec >= 1000)
	{
		g_tCurrentTime_InSec++;
		g_tCurrentTime_InMSec = 0;
	}
	//}
}


VOID sysInitTime(VOID)
{

    /*----- disable timer -----*/
    TCR_0 = 0;
    TCR_1 = 0;

#define GPIO_CFG		0xFFF83000
	*((volatile INT *)GPIO_CFG) = 0x00050D0;

    AIC_SCR_TINT0 = 0x41;  /* high-level sensitive, priority level 1 */
    /*----- timer 0 : periodic mode, 100 tick/sec -----*/
    TICR_0 = 0x96;

	sysSetInterruptType(TINT0, HIGH_LEVEL_SENSITIVE);
	sysInstallISR(IRQ_LEVEL_1, TINT0, (PVOID)sysTimerinterrupt);

	sysEnableInterrupt(TINT0);
    TISR = 0;
	TCR_0 = 0xe8000063;
}


VOID sysGetTime(volatile time_t *ptSec, volatile time_t *ptMSec)
{
	sysDisableInterrupt(TINT0);
	*ptSec = g_tCurrentTime_InSec;
	*ptMSec = g_tCurrentTime_InMSec;
	sysEnableInterrupt(TINT0);
}


/*
 * Name: sysMSleep
 * Description: Sleep n milliseconds.
 *
 * Parameter:
 *  n[in]: Milliseconds.
 * Return value:
 *  Ignored.
 */
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





UINT32 sysGetTickCount(VOID)
{
	time_t tSec, tMSec;
	sysGetTime(&tSec, &tMSec);
	return (UINT32)tMSec + (UINT32)1000 * (UINT32)tSec;
}



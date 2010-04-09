/*
 * irq.c: Set interrupt handler.
 * $Id: irq.c,v 1.1 2006/01/17 09:42:12 xhchen Exp $
 *
 * Copyright (C) Winbond co,. Ltd.
 * All rights reserved.
 */


#include "../../Inc/Platform.h"
#include "../Inc/InInc/W90N740.h"


INT32 sysDisableInterrupt (UINT32 uIntNo)
{
	AIC_MDCR = (1<<(uIntNo));
	return 0;
}

INT32 sysEnableInterrupt (UINT32 uIntNo)
{
	AIC_MECR = (1<<(uIntNo));
	return 0;
}


static UINT32 sysDisable_Int_All(VOID)
{
	AIC_MDCR = 0xffff;
	return 0;
}


typedef VOID (*FUN_SYS_ISR)();
static FUN_SYS_ISR g_fnISR[32];

PVOID sysInstallISR (INT32 nIntTypeLevel, UINT32 uIntNo, PVOID fnNewISR)
{
	UINT32 uRegValue;
	
	PVOID fnOldISR;
	if (uIntNo >= sizeof(g_fnISR) / sizeof(FUN_SYS_ISR)) return NULL;

	uRegValue = AIC_SCR(uIntNo) & 0xFFFFFFF8;
	AIC_SCR(uIntNo) = (uRegValue | nIntTypeLevel);
	
	fnOldISR = (PVOID)g_fnISR[uIntNo];
	g_fnISR[uIntNo] = (FUN_SYS_ISR)fnNewISR;
	return fnOldISR;
}


INT32 sysSetInterruptType (UINT32 uIntNo, UINT32 uIntSourceType)
{
	UINT32 uRegValue;
	if (uIntNo >= sizeof(g_fnISR) / sizeof(FUN_SYS_ISR)) return -1;
	
	uRegValue = AIC_SCR(uIntNo) & 0xFFFFFF3F;
	AIC_SCR(uIntNo) = (uRegValue | uIntSourceType);
	return 0;
}

/*
 * Name: __irq_IRQ_IntHandler
 * Description: Interrupt service.
 *
 * No parameter.
 * No return value.
 */
VOID __sysirq_IRQ_IntHandler(VOID)
{
 	unsigned int IPER, ISNR;

 	IPER = AIC_IPER >> 2;
 	ISNR = AIC_ISNR;

	if (IPER == ISNR)
	{
		if (sizeof(g_fnISR) / sizeof(FUN_SYS_ISR) > (UINT32)ISNR
			&& g_fnISR[(UINT32)ISNR] != NULL)
			(*(g_fnISR[(UINT32)ISNR]))();
 	}

 	AIC_EOSCR = 0;
}


static VOID sysIRQ_IntHandler(VOID)
{
	__asm__ __volatile__(
		"IRQ_IntHandler:\n\
		stmdb	sp!, {r0 - r12}			@ save r0 - r4\n\
		stmdb	sp!, {r14}			@ save r0 - r4\n\
		bl	__sysirq_IRQ_IntHandler\n\
		ldmia	sp!, {r14}			@ load r0 - pc, cpsr\n\
		ldmia	sp!, {r0 - r12}			@ load r0 - pc, cpsr\n\
		sub lr,lr,#4\n\
		movs pc , lr"
	);
}


/*
 * Name: sysInitInterrupt
 * Description: Initialize interrupt handler.
 *
 * No parameter.
 * No return value.
 */
VOID sysInitInterrupt()
{
	unsigned int temp;
 	/* clear CPSR I bit */
  	__asm__ __volatile__(
      	"MRS    %0, CPSR \n\
      	AND    %0, %0, #0x7F \n\
      	MSR    CPSR_c, %0"
      	: "=r"(temp)
  	);

	memset(g_fnISR, 0, sizeof(g_fnISR));
	*(volatile unsigned int *)0x38 = (unsigned int)sysIRQ_IntHandler;
	sysDisable_Int_All();
}





VOID sysDisableWbcIsr (VOID)
{
	sysDisableInterrupt (nIRQ0);
}

VOID sysEnableWbcIsr (VOID)
{
	sysEnableInterrupt (nIRQ0);
}


PVOID sysInstallWbcIsr (PVOID fnNewISR)
{
	sysSetInterruptType (nIRQ0, LOW_LEVEL_SENSITIVE);
	return sysInstallISR (IRQ_LEVEL_1, nIRQ0, fnNewISR);
}



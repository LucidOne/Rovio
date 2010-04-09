#include "../Inc/CommonDef.h"


static PRD_TASK_T g_prdtskCheckUSB;

UINT8 Read_GPIO7(void);
/* Check USB pluggin status */
static void prdTask_CheckUSB(void *pArg)
{
	if (Read_GPIO7())
		prdReset_CheckSuspend();
}


void prdAddTask_CheckUSB()
{
	prdAddTask(&g_prdtskCheckUSB, &prdTask_CheckUSB, 100, NULL);
}


void prdDelTask_CheckUSB()
{
	prdDelTask(&g_prdtskCheckUSB);
}


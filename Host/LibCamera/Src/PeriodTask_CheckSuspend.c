#include "../Inc/CommonDef.h"


static PRD_TASK_T g_prdtskCheckSuspend;


#define SUSPEND_TIME_SEC	30	


BOOL nsIsSuspendAllowed()
{
	return TRUE;
}

/* Check if it is possible to suspend */
static void prdTask_CheckSuspend(void *pArg)
{
return;
	if (netIsSuspendAllowed()
#if defined IPCAM_CONFIG_IP_CAM_VER_1 || defined IPCAM_CONFIG_IP_CAM_VER_2 || defined IPCAM_CONFIG_IP_CAM_VER_3
		&& mcuIsSuspendAllowed()
		&& nsIsSuspendAllowed()
#elif defined IPCAM_CONFIG_IP_CAM_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_1
#else
#	error "No hardware config defined!"
#endif
		)
	{
		pwr_power_saving();
	}
}


void prdReset_CheckSuspend()
{
	prdLock();	
	prdDelTask_CheckSuspend();
	prdAddTask_CheckSuspend();	
	prdUnlock();
}


void prdAddTask_CheckSuspend()
{
	prdAddTask(&g_prdtskCheckSuspend, &prdTask_CheckSuspend, SUSPEND_TIME_SEC * 100, NULL);
}


void prdDelTask_CheckSuspend()
{
	prdDelTask(&g_prdtskCheckSuspend);
}


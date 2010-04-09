#include "../Inc/CommonDef.h"


static PRD_TASK_T g_prdtskCheckBattery;

/* Check battery level */
static void prdTask_CheckBattery(void *pArg)
{
	BOOL bOnCharge;
#if defined IPCAM_CONFIG_IP_CAM_VER_1 || defined IPCAM_CONFIG_IP_CAM_VER_2 || defined IPCAM_CONFIG_IP_CAM_VER_3
	if(mcuGetBattery(FALSE, &bOnCharge) <= (unsigned char)0x71 && !bOnCharge )
		ledSetLowBattery(TRUE);
	else
		ledSetLowBattery(FALSE);

#elif defined IPCAM_CONFIG_IP_CAM_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_1
#else
#	error "No hardware config defined!"
#endif

}


void prdAddTask_CheckBattery()
{
	prdAddTask(&g_prdtskCheckBattery, &prdTask_CheckBattery, 200, NULL);
}


void prdDelTask_CheckBattery()
{
	prdDelTask(&g_prdtskCheckBattery);
}


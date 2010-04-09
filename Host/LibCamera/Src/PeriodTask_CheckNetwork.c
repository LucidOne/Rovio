#include "../Inc/CommonDef.h"


static PRD_TASK_T g_prdtskCheckNetwork;


static void prdTask_CheckNetwork_NoActive(void *pArg);
static void prdTask_CheckNetwork_TryConnect(void *pArg);
static void prdTask_CheckNetwork_TryConnectOver(void *pArg);
static void prdTask_CheckNetwork_Active(void *pArg);




/* Check network activity */
static void prdTask_CheckNetwork_NoActive(void *pArg)
{
	if(vp_bitrate_control_getspeed() > 0)
	{
		ledSetNetwork(LED_NETWORK_TRY_CONNECT);
		prdLock();
		prdDelTask(&g_prdtskCheckNetwork);
		prdAddTask(&g_prdtskCheckNetwork, &prdTask_CheckNetwork_TryConnect, 0, NULL);
		prdUnlock();
	}
	/*
	else if(ethernetRunning == 0)
	{
		ledSetNetwork(LED_NETWORK_TRY_CONNECT);
		prdLock();
		prdDelTask(&g_prdtskCheckNetwork);
		prdAddTask(&g_prdtskCheckNetwork, &prdTask_CheckNetwork_NoActive, 120, NULL);
		prdUnlock();
	}
	else
	{
		ledSetNetwork(LED_NETWORK_READY);
	}
	*/
}


static void prdTask_CheckNetwork_TryConnect(void *pArg)
{
	prdLock();
	prdDelTask(&g_prdtskCheckNetwork);
	prdAddTask(&g_prdtskCheckNetwork, &prdTask_CheckNetwork_TryConnectOver, 400, NULL);
	prdUnlock();
	
	prdReset_CheckSuspend();
}


static void prdTask_CheckNetwork_TryConnectOver(void *pArg)
{
	ledSetNetwork(LED_NETWORK_STREAMING);
	prdLock();
	prdDelTask(&g_prdtskCheckNetwork);
	prdAddTask(&g_prdtskCheckNetwork, &prdTask_CheckNetwork_Active, 120, NULL);
	prdUnlock();
}


static void prdTask_CheckNetwork_Active(void *pArg)
{
	unsigned int nTransSpeed = vp_bitrate_control_getspeed();

	if(nTransSpeed == 0)
	{
		ledSetNetwork(LED_NETWORK_READY);
		prdLock();
		prdDelTask(&g_prdtskCheckNetwork);
		prdAddTask(&g_prdtskCheckNetwork, &prdTask_CheckNetwork_NoActive, 120, NULL);
		prdUnlock();
	}
	else
	{
		/* Reset suspend timeout */
		prdReset_CheckSuspend();
	}
}


void prdAddTask_CheckNetwork()
{
	//ledSetNetwork(LED_NETWORK_READY);
	prdAddTask(&g_prdtskCheckNetwork, &prdTask_CheckNetwork_NoActive, 120, NULL);
}


void prdDelTask_CheckNetwork()
{
	prdDelTask(&g_prdtskCheckNetwork);
}




#include "../Inc/inc.h"


typedef struct
{
	int					nRefCount;
	TT_RMUTEX_T			mtLock;
	TT_PC_T				pcIrqFlag;
	
#ifdef ECOS
//	cyg_handle_t		cygIntrHandle;
//	cyg_interrupt		cygIntrBuffer;
	
	T_ECOS_VPE			ecosvpe_t;
#endif
} VP_MPVPE_T;

#pragma arm section zidata = "non_init"
static VP_MPVPE_T g_vpMyVPE;
#pragma arm section zidata


static void vvpeAddRef (void)
{
	if (g_vpMyVPE.nRefCount == 0)
	{
		//TURN_ON_VPE_CLOCK;
#ifndef ECOS
		vpeInit (TRUE);		
#else
		vpeInit (TRUE, &(g_vpMyVPE.ecosvpe_t));		
#endif
	}
	g_vpMyVPE.nRefCount++;
}


static void vvpeDecRef (void)
{
	g_vpMyVPE.nRefCount--;
	if (g_vpMyVPE.nRefCount == 0)
	{
		//TURN_OFF_VPE_CLOCK;
	}
}


static void vvpeOnProcessOK (void)
{
#ifndef ECOS
	tt_pc_try_consume (&g_vpMyVPE.pcIrqFlag, NULL, 0);
#else
	tt_pc_try_consume (&g_vpMyVPE.pcIrqFlag, NULL, 0);
#endif
}

void vvpeInit (void)
{
	tt_rmutex_init (&g_vpMyVPE.mtLock);
	tt_pc_init (&g_vpMyVPE.pcIrqFlag, 1);
	g_vpMyVPE.nRefCount = 0;
	
	vvpeAddRef ();
#ifndef ECOS
	sysInstallISR (IRQ_LEVEL_1, IRQ_VPE, (PVOID)vpeIntHandler);
//#else
//	cyg_interrupt_create(IRQ_VPE, IRQ_LEVEL_1, NULL, &vpeIntHandler, NULL,
//				&(g_vpMyVPE.cygIntrHandle), &(g_vpMyVPE.cygIntrBuffer));
#endif
	vpeSetIRQHandler ((PVOID) vvpeOnProcessOK);
	//vpeInit (TRUE);
	vvpeDecRef ();
}

void vvpeUninit (void)
{
	vpeSetIRQHandler ((PVOID) NULL);
	vpeInit (FALSE, &(g_vpMyVPE.ecosvpe_t));	
}


int vvpeTrigger (void)
{
	int rt = tt_pc_try_produce (&g_vpMyVPE.pcIrqFlag, NULL, NULL);
	
	if (rt == 0)
	{
		vpe_ClrIntFlag ();
		vpeTrigger ();
	}
	
	return rt;
}


		

void vvpeWaitProcessOK (void)
{
#ifndef ECOS
	tt_sem_down (&g_vpMyVPE.pcIrqFlag.producer);
	tt_sem_up (&g_vpMyVPE.pcIrqFlag.producer);
#else
	cyg_semaphore_wait (&g_vpMyVPE.pcIrqFlag.producer);
	cyg_semaphore_post (&g_vpMyVPE.pcIrqFlag.producer);
#endif
}

void vvpeLock (void)
{
	tt_rmutex_lock (&g_vpMyVPE.mtLock);
	vvpeAddRef ();
}

int vvpeCanLock____remove_it (void)
{
	return tt_rmutex_can_lock__remove_it (&g_vpMyVPE.mtLock);
}

int vvpeTryLockInThd (void)
{
	int rt = tt_rmutex_try_lock_in_thd (&g_vpMyVPE.mtLock);
	if (rt == 0)
		vvpeAddRef ();
	return rt;
}

int vvpeTryLockInDsr (void)
{
	int rt = tt_rmutex_try_lock_in_dsr (&g_vpMyVPE.mtLock);
	if (rt == 0)
		vvpeAddRef ();
	return rt;
}

void vvpeUnlock (void)
{
	vvpeDecRef ();
	tt_rmutex_unlock (&g_vpMyVPE.mtLock);
}


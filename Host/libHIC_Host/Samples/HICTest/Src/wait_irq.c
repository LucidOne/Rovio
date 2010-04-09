#include "../../../../SysLib/Inc/wb_syslib_addon.h"
#include "../../../../libHIC_Host/Inc/hic_host.h"
#include "../Inc/wait_irq.h"


#ifdef OS_SUPPORT
void myWait (HIC_WAIT_IRQ_OBJ_T *pWaitIrqObj)
{
	MY_WAIT_OBJ_T *pMyWaitObj = GetParentAddr (pWaitIrqObj, MY_WAIT_OBJ_T, waitObj);

	//cyg_flag_wait (&pMyWaitObj->flagWakeup, 1, CYG_FLAG_WAITMODE_CLR | CYG_FLAG_WAITMODE_AND);
	//Warning: Please do not use "cyg_flag_wait" here, "cyg_flag_setbits" may be called before 
	//         "cyg_flag_wait". In this case, the thread would be hold on "cyg_flag_wait" forever.
	cyg_flag_value_t val;
	while ((val = cyg_flag_poll (&pMyWaitObj->flagWakeup, 1, CYG_FLAG_WAITMODE_CLR | CYG_FLAG_WAITMODE_AND)) == 0);
}

void myWakeup (HIC_WAIT_IRQ_OBJ_T *pWaitIrqObj)
{
	MY_WAIT_OBJ_T *pMyWaitObj = GetParentAddr (pWaitIrqObj, MY_WAIT_OBJ_T, waitObj);
	cyg_flag_setbits (&pMyWaitObj->flagWakeup, 1);
}

void myMSleep (HIC_WAIT_IRQ_OBJ_T *pWaitIrqObj, UINT32 uMilliSecond)
{
	printf ("In delay\n");
	cyg_thread_delay (uMilliSecond);
}

void myInitWaitObj (MY_WAIT_OBJ_T *pMyWaitObj)
{
	printf ("Init:\n");
	cyg_flag_init (&pMyWaitObj->flagWakeup);
	cyg_flag_maskbits (&pMyWaitObj->flagWakeup, 0);
	pMyWaitObj->waitObj.fnWait		= &myWait;
	pMyWaitObj->waitObj.fnWakeup	= &myWakeup;
	pMyWaitObj->waitObj.fnMSleep	= &myMSleep;
}

#else

void myWait (HIC_WAIT_IRQ_OBJ_T *pWaitIrqObj)
{
	MY_WAIT_OBJ_T *pMyWaitObj = GetParentAddr (pWaitIrqObj, MY_WAIT_OBJ_T, waitObj);
	while (!pMyWaitObj->bWakeup)
	{
		static int i;
		printf ("%d\n", i++);
	}
	
	pMyWaitObj->bWakeup = FALSE;
}

void myWakeup (HIC_WAIT_IRQ_OBJ_T *pWaitIrqObj)
{
	MY_WAIT_OBJ_T *pMyWaitObj = GetParentAddr (pWaitIrqObj, MY_WAIT_OBJ_T, waitObj);
	pMyWaitObj->bWakeup = TRUE;
}

void myMSleep (HIC_WAIT_IRQ_OBJ_T *pWaitIrqObj, UINT32 uMilliSecond)
{
	UINT32 i;
	for (i = 0; i < uMilliSecond; i++)
	{
		UINT32 volatile j;
		for (j = 0; j < 1000; j++);
	}
}

void myInitWaitObj (MY_WAIT_OBJ_T *pMyWaitObj)
{
	pMyWaitObj->bWakeup = FALSE;
	pMyWaitObj->waitObj.fnWait		= &myWait;
	pMyWaitObj->waitObj.fnWakeup	= &myWakeup;
	pMyWaitObj->waitObj.fnMSleep	= &myMSleep;
}

#endif

#include "../../../../SysLib/Inc/wb_syslib_addon.h"
#include "../../../../libHIC_Host/Inc/hic_host.h"
#include "../Inc/wait_irq.h"
#include "../Inc/commands.h"




int sysInit (void)
{
	/* <1> Enable flash. */
	sysFlushCache (I_D_CACHE);
	sysDisableCache ();
	sysEnableCache (CACHE_WRITE_BACK);

	return 0;
}





///////////////////////////////////////////////////////////////////////////////////////////



void hicOnCmd_InIRQ (void *pArg, UCHAR aucReg[6])
{
	HIC_COMMAND_T *pCmd = (HIC_COMMAND_T *) pArg;
	memcpy (pCmd->aucReg, aucReg, sizeof (pCmd->aucReg));
	
	/* Wakeup the waiting process. */
	{
		HIC_WAIT_IRQ_OBJ_T *pWaitObj = &pCmd->cmd_wait_obj.waitObj;
		(*pWaitObj->fnWakeup) (pWaitObj);
	}
}


#define PTI do {printf ("In line: %d\n", __LINE__);} while (0)



#pragma arm section zidata = "nozero"
	__align(4) static UINT8 g_uMainainStack[128*1024];
#pragma arm section zidata
static cyg_handle_t g_HandleTest;
static cyg_thread	g_ThreadTest;

void hicTest(cyg_addrword_t data)
{
	MY_WAIT_OBJ_T dma_wait_obj;
	HIC_COMMAND_T hic_cmd;

	PTI;
	
	myInitWaitObj (&dma_wait_obj);
	myInitWaitObj (&hic_cmd.cmd_wait_obj);
	hicInit (&hicOnCmd_InIRQ, &hic_cmd, &dma_wait_obj.waitObj);
	
	PTI;
	while (1)
	{
		/* Wait a command. */
		HIC_WAIT_IRQ_OBJ_T *pWaitObj = &hic_cmd.cmd_wait_obj.waitObj;
		(*pWaitObj->fnWait) (pWaitObj);
		
		PTI;
		hicOnCmd (&hic_cmd);		
	}
}


int main ()
{
	/* <1> Enable flash. */
	sysFlushCache (I_D_CACHE);
	sysDisableCache ();
	sysEnableCache (CACHE_WRITE_BACK);

	printf ("Create test thread for hic\n");
	cyg_thread_create(10, hicTest, 0, "hic_test",
		(void *)g_uMainainStack, sizeof(g_uMainainStack),
		&g_HandleTest, &g_ThreadTest);
	cyg_thread_resume(g_HandleTest);
	
}

#include "../../SysLib/Inc/wb_syslib_addon.h"
#include "../Inc/hic_host.h"
#include "../../../Baseband/Libs/HIC/Inc/InInc/HIC_FIFO.h"


HIC_HOST_T g_HICHost;

static cyg_uint32 hicIrqHandler(cyg_vector_t vector, cyg_addrword_t data)
{
	UINT32 uStatus;
	
	cyg_interrupt_mask(vector);
	uStatus = inpw (REG_HICISR);
	g_HICHost.uStatus = uStatus;
	//sysSafePrintf ("REG_HICISR = %08x\n", uStatus);
	
	if ((uStatus & bCMDIS) != 0)
	{
		UINT32	uHIC_Cmd	= inpw (REG_HICCMR);
		UINT32	uHIC_Param0	= inpw (REG_HICPAR);
		UINT32	uHIC_Param1	= inpw (REG_HICBLR);
		
		g_HICHost.aucReg[5] = (UCHAR) (uHIC_Cmd);
		g_HICHost.aucReg[4] = (UCHAR) (uHIC_Param1 >> 8);
		g_HICHost.aucReg[3] = (UCHAR) (uHIC_Param1);
		g_HICHost.aucReg[2] = (UCHAR) (uHIC_Param0 >> 24);
		g_HICHost.aucReg[1] = (UCHAR) (uHIC_Param0 >> 16);
		g_HICHost.aucReg[0] = (UCHAR) (uHIC_Param0 >> 8);

		g_HICHost.bUseFifoChecksum = (0 == (UCHAR) uHIC_Param0 ? FALSE : TRUE);

		//Clear interrupt
		outpw (REG_HICISR, bCMDIS);
	}

	if ((uStatus & bDMAIS) != 0)
	{
		//Clear interrupt
		outpw (REG_HICISR, bDMAIS);
	}
	
	if ((uStatus & bBEIEN) != 0)
	{
		assert (0 && "Error: can not go to here.");
	}
	
	cyg_interrupt_acknowledge(vector);
	return CYG_ISR_CALL_DSR;
}

static void hicIrqHandlerDSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
	if((g_HICHost.uStatus & bCMDIS) != 0)
	{
		if (g_HICHost.onCmd != NULL)
			(*g_HICHost.onCmd) (g_HICHost.pCmd_Arg, g_HICHost.aucReg);
	}
	
	if ((g_HICHost.uStatus & bDMAIS) != 0)
	{
		(*g_HICHost.pWaitObj->fnWakeup) (g_HICHost.pWaitObj);
	}
	cyg_interrupt_unmask(vector);
}


void hicInit (
	FUN_HIC_ONCMD fnOnCmd,
	void *pCmd_Arg,
	HIC_WAIT_IRQ_OBJ_T *pWaitObj)
{
	cyg_interrupt_mask(IRQ_HIC);

	g_HICHost.onCmd		= fnOnCmd;
	g_HICHost.pCmd_Arg	= pCmd_Arg;
	g_HICHost.pWaitObj	= pWaitObj;
	
	hicInitIntr ();
	/* Install interrupt handler */
	cyg_interrupt_disable();
    cyg_interrupt_create(IRQ_HIC, 1, 0, hicIrqHandler, hicIrqHandlerDSR,
    	&g_HICHost.int_handle_HIC, &g_HICHost.int_holder_HIC);
    cyg_interrupt_attach(g_HICHost.int_handle_HIC);
	cyg_interrupt_unmask(IRQ_HIC);
    cyg_interrupt_enable();
	outpw (REG_HICIER, (bCMDIEN | bDMAIEN | bBEIEN));
	outpw (REG_HICSR, 0x00);
}



#ifndef __HIC_H__
#define __HIC_H__




typedef struct tagHIC_WAIT_IRQ_OBJ_T
{
	void (*fnWait) (struct tagHIC_WAIT_IRQ_OBJ_T *pWaitIrqObj);
	void (*fnWakeup) (struct tagHIC_WAIT_IRQ_OBJ_T *pWaitIrqObj);
	void (*fnMSleep) (struct tagHIC_WAIT_IRQ_OBJ_T *pWaitIrqObj, UINT32 uMilliSecond);
} HIC_WAIT_IRQ_OBJ_T;


typedef enum
{
	HIC_INTR_MASK				= 0x70,
	HIC_INTR_STATE_CHANGED		= 0x00,
	HIC_INTR_APPLICATION_EVENT	= 0x10
} HIC_INTR_E;

typedef void (*FUN_HIC_ONCMD) (void *pArg, UCHAR aucReg[6]);

typedef struct
{
	UINT32	uParamLen;
	UINT32	uParamLen_IO;
	CHAR	*pcParam;
} HIC_CMD_45_T;




/* Initialize. */
void hicInit (
	FUN_HIC_ONCMD fnOnCmd,
	void *pCmd_Arg,
	HIC_WAIT_IRQ_OBJ_T *pWaitObj);

/* Transfer mass data through dma. */
void hicMassData_MemToFifo (const void *cpAddr, UINT32 uLen);
void hicMassData_FifoToMem (void *pAddr, UINT32 uLen);

/* Set return state for protocol command. */
void hicSetCmdReady (void);
void hicSetCmdError (void);

/* Parse type IV, V protocol command. */
BOOL hicReadCmd45 (CHAR *pcBuffer, UINT32 uLength,
				   UINT32 uParamNum, HIC_CMD_45_T *pCmdParam);
BOOL hicWriteCmd5 (CHAR *pcBuffer, UINT32 uLength, UINT32 *puLength,
				   UINT32 uParamNum, HIC_CMD_45_T *pCmdParam);

/* Set return value for protocol command. */
__inline void hicSaveToReg1 (UINT32 uReg)
{
	outpw (REG_HICPAR, uReg);
}

__inline void hicSaveToReg4 (UCHAR ucRegC, UCHAR ucRegB, UCHAR ucRegA, UCHAR ucReg9)
{
	UINT32 uHICPAR;
	uHICPAR = (((UINT32) ucRegC) << 24)
			| (((UINT32) ucRegB) << 16)
			| (((UINT32) ucRegA) << 8)
			| ((UINT32) ucReg9);
	outpw (REG_HICPAR, uHICPAR);
}


/* Interrupt to baseband. */
__inline void hicSetIntr (HIC_INTR_E eEventType)
{
	UINT32 uStatus;
	// GPIO3 set to high
	//sysSafePrintf ("Set hic interrupt!\n");
	
	uStatus = inpw (REG_HICSR);
	uStatus = (uStatus & ~ (UINT32) HIC_INTR_MASK) | eEventType;
	outpw (REG_HICSR, uStatus);
	
	outpw (REG_GPIO_DAT, inpw (REG_GPIO_DAT) & ~(UINT32) 0x08);
}

__inline void hicClearIntr (void)
{
	// GPIO3 set to low
	//sysSafePrintf ("Clear hic interrupt!\n");
	
	outpw (REG_GPIO_DAT, inpw (REG_GPIO_DAT) | (UINT32) 0x08);
}

__inline void hicInitIntr (void)
{
	hicClearIntr ();
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~(UINT32) 0x08);
}



#include "./InInc/hic_host_internal.h"



#endif

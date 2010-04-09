#ifndef __VP_THREAD_HIC_H__
#define __VP_THREAD_HIC_H__


#ifdef __HIC_SUPPORT__
/******************************************************************************
 *
 * HIC Register bit map
 *
 *****************************************************************************/
/* HICFCR */
#define bAWAKEEN	(0x4000)
#define bINDEXEN	(0x2000)	/* Index mode enable */
#define bLCM1		(0x1000)	/* LCM1 selection */
#define bDASEL		(0x800)	/* Data Append Selection */
#define bHBF		(0x400)	/* High Byte First */
#define bfDWIDTH_8	(0x0)		/* 8-bit */
#define bfDWIDTH_9	(0x200)	/* 9-bit */
#define bfDWIDTH_16	(0x100)	/* 16-bit */
#define bfDWIDTH_18	(0x300)	/* 18-bit */
#define bfDSTDF_332	(0x40)	/* destination RGB332 */
#define bfDSTDF_565 (0x80)	/* destination RGB565 */
#define bfDSTDF_888	(0xC0)	/* destination RGB888 */
#define bfSRCDF_332	(0x10)	/* source RGB332 */
#define bfSRCDF_565 (0x20)	/* source RGB565 */
#define bfSRCDF_666	(0x30)	/* source RGB888 */
#define bDMAWR		(0x08)	/* DMA R/W direction, 0:SDRAM->FIFO, 1:FIFO->SDRAM */
#define bDirec		(0x08)	/* R/W direction */
#define bDMAEN		(0x04)	/* DMA enable */
#define bSWRST		(0x02)	/* software reset */
#define bHICEN		(0x01)	/* HIC function enable */

/* HICIER */
#define bBEIEN		(0x10)	/* bus error interrupt enable */
#define bDMAIEN		(0x08)	/* software DAM completion interrupt enbale */
#define bBWIEN		(0x04)	/* burst write completion interrupt enable */
#define bBRIEN		(0x02)	/* burst read completion interrupt enable */
#define bCMDIEN		(0x01)	/* command received interrupt enable */

/* HICISR */
#define bBEIS		(0x10)	/* bus error interrupt status */
#define bDMAIS		(0x08)	/* software DAM completion interrupt status */
#define bBWIS		(0x04)	/* burst write completion interrupt status */
#define bBRIS		(0x02)	/* burst read completion interrupt status */
#define bCMDIS		(0x01)	/* command received interrupt status */

/* HICSR */
#define bCMDBUSY	(0x80)	/* command busy */
#define bDRQ 		(0x08)	/* data request */
#define bERROR		(0x01)	/* command error */

#define HIC_FIFO_LEN	128


typedef struct
{
	UCHAR *pucFIFO;
	UINT32 auThreadBuffer[TT_THREAD_BUFFER_SIZE (4*1024) / sizeof (UINT32)];
	UINT32 auMsgBuffer[TT_MSG_BUFFER_SIZE (3) / sizeof (UINT32)];
	TT_MSG_QUEUE_T *pMsgQueue;
	TT_PC_T pcDmaFlag;

	BOOL bEnableCommandInterrupt;
	BOOL bEnableEventInterrupt;

#define DEBUG_HIC_FIFO
#ifdef DEBUG_HIC_FIFO
	BOOL bUseFifoChecksum;
#endif
	UINT32 uIntrEvent;

} HIC_THREAD_T;


void hicInitThread (void);
void hicSetCmdReady (HIC_THREAD_T *pHicThread);
void hicSetCmdError (HIC_THREAD_T *pHicThread);
void hicStartDma_MemToFifo (HIC_THREAD_T *pHicThread, const void *pAddr, UINT32 uLen);
void hicStartDma_FifoToMem (HIC_THREAD_T *pHicThread, void *pAddr, UINT32 uLen);
void hicMassData_MemToFifo (HIC_THREAD_T *pHicThread, const void *pAddr, UINT32 uLen);
void hicMassData_FifoToMem (HIC_THREAD_T *pHicThread, void *pAddr, UINT32 uLen);

typedef struct
{
	UINT32	uParamLen;
	UINT32	uParamLen_IO;
	CHAR	*pcParam;
} HIC_CMD_45_T;


int __hicReadCmd45 (CHAR *pcBuffer, UINT32 uLength, UINT32 *puLength,
					UINT32 uParamNum, HIC_CMD_45_T *pCmdParam);
int __hicWriteCmd5 (CHAR *pcBuffer, UINT32 uLength, UINT32 *puLength,
					UINT32 uParamNum, HIC_CMD_45_T *pCmdParam);
BOOL hicReadCmd45 (CHAR *pcBuffer, UINT32 uLength,
				   UINT32 uParamNum, HIC_CMD_45_T *pCmdParam);
BOOL hicWriteCmd5 (CHAR *pcBuffer, UINT32 uLength, UINT32 *puLength,
				   UINT32 uParamNum, HIC_CMD_45_T *pCmdParam);


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

__inline void hicSetIntr (CMD_HIC_INTR_E eEventType)
{
	UINT32 uStatus;
	// GPIO3 set to high
	//sysSafePrintf ("Set hic interrupt!\n");
	
	uStatus = inpw (REG_HICSR);
	uStatus = (uStatus & ~ (UINT32) CMD_HIC_INTR_MASK) | eEventType;
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


BOOL hicSetInterruptEvent (UINT32 uIntrEvent);
BOOL hicIsInterruptEventEnabled (void);

#endif



#endif

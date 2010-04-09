#ifndef __HIC_HOST_INTERNAL_H__
#define __HIC_HOST_INTERNAL_H__


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
	cyg_handle_t	int_handle_HIC;
	cyg_interrupt	int_holder_HIC;

	BOOL bEnableCommandInterrupt;
	BOOL bUseFifoChecksum;

	FUN_HIC_ONCMD	onCmd;
	void			*pCmd_Arg;
	HIC_WAIT_IRQ_OBJ_T	*pWaitObj;
	
	UINT32			uStatus;
	UCHAR			aucReg[6];
} HIC_HOST_T;

extern HIC_HOST_T g_HICHost;


int __hicReadCmd45 (CHAR *pcBuffer, UINT32 uLength, UINT32 *puLength,
					UINT32 uParamNum, HIC_CMD_45_T *pCmdParam);
int __hicWriteCmd5 (CHAR *pcBuffer, UINT32 uLength, UINT32 *puLength,
					UINT32 uParamNum, HIC_CMD_45_T *pCmdParam);


#endif

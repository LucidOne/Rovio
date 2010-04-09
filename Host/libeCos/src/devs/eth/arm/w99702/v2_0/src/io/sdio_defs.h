/*
 * 	File: sdio_def.h
 * 	Definitions for the Low Level SDIO Driver
 *
 *  Copyright © Marvell International Ltd. and/or its affiliates, 2003-2006
 */
#ifndef __SDIO_DEF__H
#define __SDIO_DEF__H

#define	PXA_MMC_BLKSZ_MAX 		(1<<9)  /* Actually 1023 */
#define	PXA_MMC_NOB_MAX 		((1<<16)-2)
#define	PXA_MMC_BLOCKS_PER_BUFFER 	(2)

#define	CMD(x)				(x)

#define	MMC_I_MASK_ALL			(0x1fffUL)
#define	MMC_DISABLE_SPI			(0x0000UL)
#define	MMC_RES_TO_MAX 			(0x007fUL)

#define	MMC_I_REG_RES_ERR		(0x01UL << 9)
#define	MMC_I_REG_DAT_ERR		(0x01UL << 8)

#define MMC_I_MASK_END_CMD_RES		(0x01UL << 2)
#define MMC_I_REG_END_CMD_RES		(0x01UL << 2)

#define	MMC_I_MASK_PRG_DONE             (0x01UL << 1)
#define	MMC_I_REG_PRG_DONE		(0x01UL << 1)

#define	MMC_I_REG_DATA_TRAN_DONE	(0x01UL)
#define	MMC_I_MASK_DATA_TRAN_DONE       (0x01UL)

#define	MMC_I_REG_CLK_IS_OFF		(0x01UL << 4)

#define	MMC_CMDAT_BLOCK			(0x0000UL << 4)
#define	MMC_CMDAT_SD_1DAT		(0x0000UL << 8)
#define	MMC_CMDAT_WR_RD			(0x0001UL << 3)
#define	MMC_CMDAT_READ			(0x0000UL << 3)

#define	MMC_CMDAT_R1			(0x0001UL | MMC_CMDAT_SDIO_INT_EN)

#define	MMC_CMDAT_R2			(0x0002UL | MMC_CMDAT_SDIO_INT_EN)

#define	MMC_CMDAT_R3            	(0x0003UL | MMC_CMDAT_SDIO_INT_EN)

#define	MMC_I_MASK_SDIO_INT             (0x01UL << 11)

#define	MMC_PRTBUF_BUF_FULL		(0x00UL)
#define	MMC_STRPCL_START_CLK		(0x0002UL)

#define	MMC_I_MASK_RES_ERR		(0x01UL << 9)
#define	MMC_I_MASK_DAT_ERR		(0x01UL << 8)

#ifndef	MMC_CMDAT_INIT
#define	MMC_CMDAT_INIT			(0x0001UL << 6)
#endif

#ifndef	MMC_STAT_RES_CRC_ERR
#define	MMC_STAT_RES_CRC_ERR		(1 << 5)
#endif

#ifndef	MMC_STAT_DAT_ERR_TOKEN
#define	MMC_STAT_DAT_ERR_TOKEN		(1 << 4)
#endif

#ifndef	MMC_STAT_CRC_RD_ERR
#define	MMC_STAT_CRC_RD_ERR		(1 << 3)
#endif

#ifndef	MMC_STAT_TIME_OUT_RES
#define	MMC_STAT_TIME_OUT_RES		(1 << 1)
#endif

#ifndef	MMC_STAT_TIME_OUT_READ
#define	MMC_STAT_TIME_OUT_READ		(1 << 0)
#endif

#ifndef	MMC_I_END_CMD_RES
#define	MMC_I_END_CMD_RES		(1 << 2)
#endif

#ifndef	MMC_CLKRT_FREQ_9_75MHZ
#define	MMC_CLKRT_FREQ_9_75MHZ		(1 << 0)
#endif

#ifndef	MMC_I_SDIO_INT
#define	MMC_I_SDIO_INT			(1 << 11)
#endif

#ifndef	MMC_I_CLK_IS_OFF
#define	MMC_I_CLK_IS_OFF		(1 << 4)
#endif

#ifndef	MMC_CLKRT_FREQ_19_5MHZ
#define	MMC_CLKRT_FREQ_19_5MHZ		(0 << 0)
#endif

#ifndef	MMC_CMDAT_SD_4DAT
#define	MMC_CMDAT_SD_4DAT		(1 << 8)
#endif

#ifndef	MMC_CMDAT_DMA_EN
#define	MMC_CMDAT_DMA_EN		(1 << 7)
#endif

#ifndef	MMC_CMDAT_SDIO_INT_EN
#define	MMC_CMDAT_SDIO_INT_EN		(0x0001UL << 11)
#endif

#ifndef	MMC_STRPCL_STOP_CLK
#define	MMC_STRPCL_STOP_CLK		(0x0001UL)
#endif

#ifndef	MMC_CMDAT_DATA_EN
#define	MMC_CMDAT_DATA_EN		(0x0001UL << 2)
#endif

#ifndef	MMC_STAT_CLK_EN
#define	MMC_STAT_CLK_EN			(0x0001UL << 8)
#endif

/* MMC_RXFIFO physical address */
#define	MMC_RXFIFO_PHYS_ADDR	 	0x41100040
/* MMC_TXFIFO physical address */
#define	MMC_TXFIFO_PHYS_ADDR 		0x41100044

#define	CISTPL_MANFID			0x20
#define	CISTPL_END			0xff

#define	PXA_MMC_RESPONSE(ctrlr, idx) 	((ctrlr->mmc_res)[idx])

#define PXA_MMC_IODATA_SIZE 		(PXA_MMC_BLOCKS_PER_BUFFER * \
							PXA_MMC_BLKSZ_MAX)

#define MMC_STAT_ERRORS (MMC_STAT_RES_CRC_ERR|MMC_STAT_DAT_ERR_TOKEN \
        |MMC_STAT_CRC_RD_ERR|MMC_STAT_TIME_OUT_RES \
        |MMC_STAT_TIME_OUT_READ)

#define SDIO_STATE_LABEL( state ) ( \
	(state == SDIO_FSM_IDLE) ? "IDLE" : \
	(state == SDIO_FSM_CLK_OFF) ? "CLK_OFF" : \
	(state == SDIO_FSM_END_CMD) ? "END_CMD" : \
	(state == SDIO_FSM_BUFFER_IN_TRANSIT) ? "IN_TRANSIT" : \
	(state == SDIO_FSM_END_BUFFER) ? "END_BUFFER" : \
	(state == SDIO_FSM_END_IO) ? "END_IO" : \
	(state == SDIO_FSM_END_PRG) ? "END_PRG" : "UNKNOWN" )

#define SET_PXA_GPIO_MODE 		//pxa_gpio_mode
#define SET_PXA_IRQ_TYPE(gpio) 		set_irq_type(IRQ_GPIO(gpio), IRQT_FALLING)
#define DMA_BUF_ALLOC(paddr,size)	dma_alloc_coherent(NULL, size, paddr, flags);
#define DMA_BUF_FREE(vaddr,paddr,size)		dma_free_coherent(NULL, size, vaddr, paddr);

#endif /*__SDIO_DEF__H*/

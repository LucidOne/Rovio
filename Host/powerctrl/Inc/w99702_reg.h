/**********************************************************************************************************
 *                                                                          
 * Copyright (c) 2004 - 2006 Winbond Electronics Corp. All rights reserved.      
 *                                                                         
 * FILENAME
 *     w99702_reg.h
 *
 * VERSION
 *     1.1
 *
 * DESCRIPTION
 *     This file contains the register map of W99702 processor. The included H/W Macro functions are
 *     listed below.
 *
 *     0. System Address Map
 *     1. SDRAM Controller
 *     2. Global Controller
 *     3. Clock Controller
 *     4. GPIO
 *     5. Fast Serial Bus
 *     6. Host Bus Interface
 *     7. Flash memory Card Controller (FMI)
 *     8. Audio Interface
 *     9. USB Device
 *    10. LCM
 *    11. Sensor DSP
 *    12. Video Capture Engine
 *    13. JPEG Codec
 *    14. MPEG-4 Video Codec
 *    15. 2-D Graphic Engine
 *    16. Video Processing Engine
 *    17. Motion Estimation Engine
 *    18. High Speed UART
 *    19. UART
 *    20. Timer
 *    21. Interrupt Controller (AIC)
 *    22. Universal Serial Interface
 *    23. External Bus Interface 
 *
 * HISTORY
 *     08/31/2004		 Ver 1.0 Created by PC30 MNCheng
 *     09/21/2004        Add the MPEG4 DMA registers 
 *
 * REMARK
 *     None
 *     
 **********************************************************************************************************/
#ifndef _W99702_H
#define _W99702_H

/**********************************************************************************************************
 *                                                               
 * 0. System Address Map - Defines the register base address of each Macro 
 *                         function.
 *
 **********************************************************************************************************/
#define    SDRM_BA  0x3FFF0000 /* SDRAM */
#define    GCR_BA    0x7FF00000 /* Global Control */
#define    CLK_BA    0x7FF00200 /* Clock Control */
#define    GPIO_BA   0x7FF00300 /* GPIO Control */
#define    FSB_BA    0x7FF00400 /* Fast Serial Bus */
#define    HIC_BA    0x7FF01000 /* Host Bus Interface */
#define    FMI_BA    0x7FF02000 /* Flash Memory Card Interface */
#define    ADO_BA    0x7FF03000 /* Audio Interface */
#define    USB_BA    0x7FF04000 /* USB Device */
#define    LCM_BA    0x7FF05000 /* Display, LCM Interface & Bypass */
#define    DSP_BA    0x7FF08000 /* Sensor DSP */
#define    CAP_BA    0x7FF09000 /* Video Capture Engine */
#define    JPG_BA    0x7FF0A000 /* JPEG Codec */
#define    MP4_BA    0x7FF0B000 /* MPEG-4 Video Codec */
#define    GE_BA     0x7FF0C000 /* 2-D Graphic Engine */
#define    VPE_BA    0x7FF0D000 /* Video Processing Engine */
#define    ME_BA     0x7FF0E000 /* Motion Estimation Engine */
#define    HSU_BA    0x7FFC0000 /* High Speed UART */
#define    UART_BA   0x7FFC1000 /* UART */
#define    TMR_BA    0x7FFC2000 /* Timer */
#define    AIC_BA    0x7FFC3000 /* Interrupt Controller */
#define    USI_BA    0x7FFC4000 /* Universal Serial Interface */
#define    EBI_BA    0xFDFF0000 /* External Bus Interface (EBI) */


/**********************************************************************************************************
 *                                                               
 * 1. SDRAM Control Registers  
 *
 **********************************************************************************************************/
#define REG_SDICEN		(SDRM_BA + 0x000)    /* SDIC enable register */
#define REG_SDICON		(SDRM_BA + 0x004)    /* SDIC control register */
#define REG_SDCONF0		(SDRM_BA + 0x008)    /* SDRAM bank 0 configuration register */
#define REG_SDCONF1		(SDRM_BA + 0x00C)    /* SDRAM bank 1 configuration register */
#define REG_SDTIME0		(SDRM_BA + 0x010)    /* SDRAM bank 0 timing control register */
#define REG_SDTIME1		(SDRM_BA + 0x014)    /* SDRAM bank 1 timing control register */
#define REG_ARBCON		(SDRM_BA + 0x018)    /* Arbiter control register */
#define REG_TESTCR		(SDRM_BA + 0x020)    /* SDRAM test control register */
#define REG_TSTATUS		(SDRM_BA + 0x024)    /* SDRAM test status register */
#define REG_TFDATA		(SDRM_BA + 0x028)    /* SDRAM test fail data */
#define REG_TFADDR		(SDRM_BA + 0x02C)    /* SDRAM test fail address */
#define REG_CKSKEW		(SDRM_BA + 0xF00)    /* Clock skew control register (for testing) */
 
 
/**********************************************************************************************************
 *                                                               
 * 2. Global Control Registers  
 *
 **********************************************************************************************************/
#define REG_PDID		(GCR_BA+0x000)    /* Product Identifier Register */
#define REG_SYS_CFG		(GCR_BA+0x004)    /* System Power-On Configuration Control Register */
#define REG_CPUCR		(GCR_BA+0x008)    /* CPU Control (Reset/Initial) Register */
#define REG_MISCR		(GCR_BA+0x00C)    /* Miscellaneous Control Register */
#define REG_PADC0		(GCR_BA+0x020)    /* PAD Function Control Register 0 */
#define REG_PADC1		(GCR_BA+0x024)    /* PAD Function Control Register 1 */
#define REG_PADC2		(GCR_BA+0x028)    /* PAD Function Control Register 2 */
#define REG_AHB_PRI		(GCR_BA+0x100)    /* AHB Priority Control Register */
#define REG_BBLENG0		(GCR_BA+0x104)    /* Burst Length Control Register for AHB - Lite	*/
#define REG_BBLENG1		(GCR_BA+0x108)    /* Burst Length Control Register for AHB Master	*/


/**********************************************************************************************************
 *                                                               
 * 3. Clock Control Registers  
 *
 **********************************************************************************************************/
#define REG_PWRCON		(CLK_BA + 0x00)    /* System Power Down Control Register */
#define REG_CLKCON		(CLK_BA + 0x04)    /* Clock Enable Control Register */
#define REG_APLLCON		(CLK_BA + 0x08)    /* APLL Control Register */
#define REG_UPLLCON		(CLK_BA + 0x0C)    /* UPLL Control Register */
#define REG_CLKSEL		(CLK_BA + 0x10)	   /* Clock Source Select Control Register */
#define REG_CLKDIV0		(CLK_BA + 0x14)	   /* Clock Divider Number Register 0 */
#define REG_CLKDIV1		(CLK_BA + 0x18)	   /* Clock Divider Number Register 1 */
#define REG_PLLWCR		(CLK_BA + 0x1C)	   /* PLL Wake Up Counter Control Register */
#define REG_FLCCR		(CLK_BA + 0x20)	   /* Flash Light Charge Counter Register */
#define REG_PWMCR		(CLK_BA + 0x24)	   /* Pulse Width Modulation Clock Counter Register */


/**********************************************************************************************************
 *                                                               
 * 4. GPIO Control Registers  
 *
 **********************************************************************************************************/
/* GPIO Pins */ 
#define REG_GPIO_OE			(GPIO_BA+0x00)   /* GPIO Pins Output Enable Control Register */
#define REG_GPIO_DAT		(GPIO_BA+0x04)   /* GPIO Pins Data Register */
#define REG_GPIO_STS		(GPIO_BA+0x08)   /* GPIO Pins Status Register */
#define REG_GPIO_IE			(GPIO_BA+0x0c)   /* GPIO Pins Interrupt Enable Control Register */
#define REG_GPIO_IS			(GPIO_BA+0x10)   /* GPIO Pins Interrupt Status Control Register */
#define REG_GPIO_PE			(GPIO_BA+0x14)   /* GPIO Pull-Up/Down Enable Control Register */
#define REG_GPIO_DEBOUNCE	(GPIO_BA+0x18)   /* GPIO Pins De-Bounce Control Register */

/* GPIO-A Pins */
#define REG_GPIOA_OE		(GPIO_BA+0x20)   /* GPIO-A Pins Output Enable Control Register */
#define REG_GPIOA_DAT		(GPIO_BA+0x24)   /* GPIO-A Pins Data Register */
#define REG_GPIOA_STS		(GPIO_BA+0x28)   /* GPIO-A Pins Status Register */
#define REG_GPIOA_PE		(GPIO_BA+0x2c)   /* GPIO-A Pull-Up/Down Enable Control Register */

/* GPIO-B Pins */
#define REG_GPIOB_OE		(GPIO_BA+0x30)   /* GPIO-B Pins Output Enable Control Register */
#define REG_GPIOB_DAT		(GPIO_BA+0x34)   /* GPIO-B Pins Data Register */
#define REG_GPIOB_STS		(GPIO_BA+0x38)   /* GPIO-B Pins Status Register */
#define REG_GPIOB_PE		(GPIO_BA+0x3c)   /* GPIO-B Pull-Up/Down Enable Control Register */

/* GPIO-S Pins */
#define REG_GPIOS_OE		(GPIO_BA+0x40)   /* GPIO-S Pins Output Enable Control Register */
#define REG_GPIOS_DAT		(GPIO_BA+0x44)   /* GPIO-S Pins Data Register */
#define REG_GPIOS_STS		(GPIO_BA+0x48)   /* GPIO-S Pins Status Register */
#define REG_GPIOS_PE		(GPIO_BA+0x4c)   /* GPIO-S Pull-Up/Down Enable Control Register */


/**********************************************************************************************************
 *                                                               
 * 5. Fast Serial Bus Control Registers  
 *
 **********************************************************************************************************/
#define REG_FastSerialBusTrigger		(FSB_BA+0x00)    /* FSB Trigger and SCK Control */
#define REG_FastSerialBusStatus			(FSB_BA+0x04)    /* FSB Status Register */
#define REG_FastSerialBusCR				(FSB_BA+0x08)    /* FSB Control Register */
#define REG_FastSerialBusSCKSDA0		(FSB_BA+0x0C)    /* FSB SCKSDA 0 register */
#define REG_FastSerialBusSCKSDA1		(FSB_BA+0x10)    /* FSB SCKSDA 1 register */
#define REG_SerialBusCR					(FSB_BA+0x14)    /* FSB Software Mode Register */
 
 
/**********************************************************************************************************
 *                                                               
 * 6. Host Bus Interface Control Registers  
 *
 **********************************************************************************************************/
#define REG_HICFCR	(HIC_BA+0x00)		/* HIC function control REGister */
#define REG_HICDAR	(HIC_BA+0x04)		/* HIC software DMA address REGister */
#define REG_HICDLR	(HIC_BA+0x08)		/* HIC software DMA length REGister */
#define REG_HICIER	(HIC_BA+0x0C)		/* HIC interrupt enable REGister */
#define REG_HICISR	(HIC_BA+0x10)		/* HIC interrupt status REGister */
#define REG_HICBIS	(HIC_BA+0x14)		/* HIC BIST REGister */
#define REG_HICCMR	(HIC_BA+0x80)		/* HIC command port REGister */
#define REG_HICPAR	(HIC_BA+0x84)		/* HIC parameter/address port REGister */
#define REG_HICBLR	(HIC_BA+0x88)		/* HIC parameter/burst transfer count REGister */
#define REG_HICDR	(HIC_BA+0x8C)		/* HIC data port REGister */
#define REG_HICSR	(HIC_BA+0x90)		/* HIC status port REGister */
#define REG_HICBUF	(HIC_BA+0x100)		/* HIC internal buffer starting address */
#define _HICBUF_NUM	(64)
 
 
/**********************************************************************************************************
 *                                                               
 * 7. Flash memory Card Control Registers  
 *
 **********************************************************************************************************/
/* Flash Memory Interface Registers definition */
#define REG_FMICR    (FMI_BA+0x00)    /* FMI control register */
#define REG_FMIDSA   (FMI_BA+0x04)    /* FMI DMA transfer starting address register */
#define REG_FMIBCR   (FMI_BA+0x08)    /* FMI DMA byte count register */
#define REG_FMIIER   (FMI_BA+0x0C)    /* FMI interrupt enable register */
#define REG_FMIISR   (FMI_BA+0x10)    /* FMI interrupt status register */
#define REG_FB0_0    (FMI_BA+0x400)   /* Flash buffer 0 */
#define REG_FB1_0    (FMI_BA+0x800)   /* Flash buffer 1 */

/* CompactFlash Registers definition */
#define REG_CFDR     (FMI_BA+0x100)   /* CF IDE data register */
#define REG_CFFER    (FMI_BA+0x104)   /* CF IDE feature/error register */
#define REG_CFSCR    (FMI_BA+0x108)   /* CF IDE sector count register */
#define REG_CFSNR    (FMI_BA+0x10C)   /* CF IDE sector number register */
#define REG_CFCLR    (FMI_BA+0x110)   /* CF IDE cylinder low register */
#define REG_CFCHR    (FMI_BA+0x114)   /* CF IDE cylinder high register */
#define REG_CFSCHR   (FMI_BA+0x118)   /* CF IDE select card/head register */
#define REG_CFCSR    (FMI_BA+0x11C)   /* CF IDE command/status register */
#define REG_CFDCR    (FMI_BA+0x120)   /* CF IDE device control/alt status register (HW mode)
                                         CF IDE address output control register (SW mode) */
#define REG_CFDAR    (FMI_BA+0x124)   /* CF IDE drive address register (HW mode)
                                         CF IDE CE2_/CE1_/REG_ output control register (SW mode) */
#define REG_CFCR     (FMI_BA+0x128)   /* CF IDE control register */
#define REG_CFTCR    (FMI_BA+0x12C)   /* CF IDE timing control register */
#define REG_CFIER    (FMI_BA+0x130)   /* CF interrupt enable register */
#define REG_CFISR    (FMI_BA+0x134)   /* CF interrupt status register */

/* SmartMedia Registers definition */
#define REG_SMCR     (FMI_BA+0x200)   /* SmartMedia control register */
#define REG_SMCMD    (FMI_BA+0x204)   /* SmartMedia command port register */
#define REG_SMADDR   (FMI_BA+0x208)   /* SmartMedia address port register */
#define REG_SMMADDR  (FMI_BA+0x20C)   /* SmartMedia multi-cycle address port register */
#define REG_SMDATA   (FMI_BA+0x210)   /* SmartMedia data port register */
#define REG_SMIER    (FMI_BA+0x214)   /* SmartMedia interrupt enable register */
#define REG_SMISR    (FMI_BA+0x218)   /* SmartMedia input signal and interrupt status register */
#define REG_SMECC0   (FMI_BA+0x21C)   /* SmartMedia error correction code 0 regisrer */
#define REG_SMECC1   (FMI_BA+0x220)   /* SmartMedia error correction code 1 regisrer */
#define REG_SMECC2   (FMI_BA+0x224)   /* SmartMedia error correction code 2 regisrer */
#define REG_SMECC3   (FMI_BA+0x228)   /* SmartMedia error correction code 3 regisrer */
#define REG_SMRA_0   (FMI_BA+0x22C)   /* SmartMedia redundant area register */
#define REG_SMRA_1   (FMI_BA+0x230)   /* SmartMedia redundant area register */
#define REG_SMRA_2   (FMI_BA+0x234)   /* SmartMedia redundant area register */
#define REG_SMRA_3   (FMI_BA+0x238)   /* SmartMedia redundant area register */
#define REG_SMRA_4   (FMI_BA+0x23C)   /* SmartMedia redundant area register */
#define REG_SMRA_5   (FMI_BA+0x240)   /* SmartMedia redundant area register */
#define REG_SMRA_6   (FMI_BA+0x244)   /* SmartMedia redundant area register */
#define REG_SMRA_7   (FMI_BA+0x248)   /* SmartMedia redundant area register */
#define REG_SMRA_8   (FMI_BA+0x24C)   /* SmartMedia redundant area register */
#define REG_SMRA_9   (FMI_BA+0x250)   /* SmartMedia redundant area register */
#define REG_SMRA_10  (FMI_BA+0x254)   /* SmartMedia redundant area register */
#define REG_SMRA_11  (FMI_BA+0x258)   /* SmartMedia redundant area register */
#define REG_SMRA_12  (FMI_BA+0x25C)   /* SmartMedia redundant area register */
#define REG_SMRA_13  (FMI_BA+0x260)   /* SmartMedia redundant area register */
#define REG_SMRA_14  (FMI_BA+0x264)   /* SmartMedia redundant area register */
#define REG_SMRA_15  (FMI_BA+0x268)   /* SmartMedia redundant area register */

/* Secure Digit Registers definition */
#define REG_SDCR     (FMI_BA+0x300)   /* SD control register */
#define REG_SDHINI   (FMI_BA+0x304)   /* SD host initial register */
#define REG_SDIER    (FMI_BA+0x308)   /* SD interrupt enable register */
#define REG_SDISR    (FMI_BA+0x30C)   /* SD interrupt status register */
#define REG_SDARG    (FMI_BA+0x310)   /* SD command argument register */
#define REG_SDRSP0   (FMI_BA+0x314)   /* SD receive response token register 0 */
#define REG_SDRSP1   (FMI_BA+0x318)   /* SD receive response token register 1 */
#define REG_SDBLEN   (FMI_BA+0x31C)   /* SD block length register */
 
 
/**********************************************************************************************************
 *                                                               
 * 8. Audio Interface Control Registers  
 *
 **********************************************************************************************************/
#define REG_ACTL_CON			(ADO_BA + 0x00)   /* Audio controller control register */
#define REG_ACTL_RESET			(ADO_BA + 0x04)   /* Sub block reset control register */
#define REG_ACTL_RDSTB			(ADO_BA + 0x08)   /* DMA destination base address register for record */
#define REG_ACTL_RDST_LENGTH	(ADO_BA + 0x0C)   /* DMA destination length register for record */
#define REG_ACTL_PDSTB			(ADO_BA + 0x18)   /* DMA destination base address register for play */
#define REG_ACTL_PDST_LENGTH	(ADO_BA + 0x1C)   /* DMA destination length register for play */
#define REG_ACTL_RSR			(ADO_BA + 0x14)   /* Record status register */
#define REG_ACTL_PSR			(ADO_BA + 0x24)   /* Play status register */
#define REG_ACTL_IISCON			(ADO_BA + 0x28)   /* IIS control register */
#define REG_ACTL_ACCON			(ADO_BA + 0x2C)   /* AC-link control register */
#define REG_ACTL_ACOS0			(ADO_BA + 0x30)   /* AC-link out slot 0 */
#define REG_ACTL_ACOS1			(ADO_BA + 0x34)   /* AC-link out slot 1 */
#define REG_ACTL_ACOS2			(ADO_BA + 0x38)   /* AC-link out slot 2 */
#define REG_ACTL_ACIS0			(ADO_BA + 0x3C)   /* AC-link in slot 0 */
#define REG_ACTL_ACIS1			(ADO_BA + 0x40)   /* AC-link in slot 1 */
#define REG_ACTL_ACIS2			(ADO_BA + 0x44)   /* AC-link in slot 2 */
#define REG_ACTL_ADCON			(ADO_BA + 0x48)   /* ADC0 control register */
#define REG_ACTL_M80CON			(ADO_BA + 0x4C)   /* M80 interface control register */
#define REG_ACTL_M80DATA0		(ADO_BA + 0x50)   /* M80 data0 register */
#define REG_ACTL_M80DATA1		(ADO_BA + 0x54)   /* M80 data1 register */
#define REG_ACTL_M80DATA2		(ADO_BA + 0x58)   /* M80 data2 register */
#define REG_ACTL_M80DATA3		(ADO_BA + 0x5C)   /* M80 data3 register */
#define REG_ACTL_M80ADDR		(ADO_BA + 0x60)   /* M80 interface start address register */
#define REG_ACTL_M80SRADDR		(ADO_BA + 0x64)   /* M80 interface start address register of right channel */
#define REG_ACTL_M80SIZE		(ADO_BA + 0x70)   /* M80 interface data size register */
#define REG_ACTL_DACON			(ADO_BA + 0x74)   /* DAC control register */
 
 
/**********************************************************************************************************
 *                                                               
 * 9. USB Device Control Registers  
 *
 **********************************************************************************************************/
#define REG_USB_CTL	  		(USB_BA+0x00)    /* USB control register */
#define REG_USB_CVCMD		(USB_BA+0x04)    /* USB class or vendor command register */
#define REG_USB_IE	  		(USB_BA+0x08)    /* USB interrupt enable register */
#define REG_USB_IS	  		(USB_BA+0x0c)    /* USB interrupt status register */
#define REG_USB_IC	  		(USB_BA+0x10)    /* USB interrupt status clear register */
#define REG_USB_IFSTR		(USB_BA+0x14)    /* USB interface and string register */
#define REG_USB_ODATA0		(USB_BA+0x18)    /* USB control transfer-out port 0 register */
#define REG_USB_ODATA1		(USB_BA+0x1C)    /* USB control transfer-out port 1 register */
#define REG_USB_ODATA2		(USB_BA+0x20)    /* USB control transfer-out port 2 register */
#define REG_USB_ODATA3		(USB_BA+0x24)    /* USB control transfer-out port 3 register */
#define REG_USB_IDATA0		(USB_BA+0x28)    /* USB control transfer-in data port 0 register */
#define REG_USB_IDATA1		(USB_BA+0x2C)    /* USB control transfer-in data port 1 register */
#define REG_USB_IDATA2		(USB_BA+0x30)    /* USB control transfer-in data port 2 register */
#define REG_USB_IDATA3		(USB_BA+0x34)    /* USB control transfer-in data port 2 register */
#define REG_USB_SIE			(USB_BA+0x38)    /* USB SIE status Register */
#define REG_USB_ENG			(USB_BA+0x3c)    /* USB Engine Register */
#define REG_USB_CTLS		(USB_BA+0x40)    /* USB control transfer status register */
#define REG_USB_CONFD		(USB_BA+0x44)    /* USB Configured Value register */
#define REG_USB_EPA_INFO	(USB_BA+0x48)    /* USB endpoint A information register */
#define REG_USB_EPA_CTL		(USB_BA+0x4c)    /* USB endpoint A control register */
#define REG_USB_EPA_IE		(USB_BA+0x50)    /* USB endpoint A Interrupt Enable register */
#define REG_USB_EPA_IC		(USB_BA+0x54)    /* USB endpoint A interrupt clear register */
#define REG_USB_EPA_IS		(USB_BA+0x58)    /* USB endpoint A interrupt status register */
#define REG_USB_EPA_ADDR	(USB_BA+0x5c)    /* USB endpoint A address register */
#define REG_USB_EPA_LENTH	(USB_BA+0x60)    /* USB endpoint A transfer length register */
#define REG_USB_EPB_INFO	(USB_BA+0x64)    /* USB endpoint B information register */
#define REG_USB_EPB_CTL		(USB_BA+0x68)    /* USB endpoint B control register */
#define REG_USB_EPB_IE		(USB_BA+0x6c)    /* USB endpoint B Interrupt Enable register */
#define REG_USB_EPB_IC		(USB_BA+0x70)    /* USB endpoint B interrupt clear register */
#define REG_USB_EPB_IS		(USB_BA+0x74)    /* USB endpoint B interrupt status register */
#define REG_USB_EPB_ADDR	(USB_BA+0x78)    /* USB endpoint B address register */
#define REG_USB_EPB_LENTH	(USB_BA+0x7c)    /* USB endpoint B transfer length register */
#define REG_USB_EPC_INFO	(USB_BA+0x80)    /* USB endpoint C information register */
#define REG_USB_EPC_CTL		(USB_BA+0x84)    /* USB endpoint C control register */
#define REG_USB_EPC_IE		(USB_BA+0x88)    /* USB endpoint C Interrupt Enable register */
#define REG_USB_EPC_IC		(USB_BA+0x8c)    /* USB endpoint C interrupt clear register */
#define REG_USB_EPC_IS		(USB_BA+0x90)    /* USB endpoint C interrupt status register */
#define REG_USB_EPC_ADDR	(USB_BA+0x94)    /* USB endpoint C address register */
#define REG_USB_EPC_LENTH	(USB_BA+0x98)    /* USB endpoint C transfer length register */
#define REG_USB_EPA_XFER	(USB_BA+0x9c)    /* USB endpoint A remain transfer length register */
#define REG_USB_EPA_PKT		(USB_BA+0xa0)    /* USB endpoint A remain packet length register */
#define REG_USB_EPB_XFER	(USB_BA+0xa4)    /* USB endpoint B remain transfer length register */
#define REG_USB_EPB_PKT		(USB_BA+0xa8)    /* USB endpoint B remain packet length register */
#define REG_USB_EPC_XFER	(USB_BA+0xac)    /* USB endpoint C remain transfer length register */
#define REG_USB_EPC_PKT		(USB_BA+0xb0)    /* USB endpoint C remain packet length register */
 
 
/**********************************************************************************************************
 *                                                               
 * 10. LCM Control Registers  
 *
 **********************************************************************************************************/
#define REG_LCM_DCCS	     (LCM_BA+0x00)    /* Display Controller Control/Status Register */
#define REG_LCM_DEV_CTRL     (LCM_BA+0x04)    /* Display Output Device Control Register */
#define REG_LCM_MPU_CMD	     (LCM_BA+0x08)    /* MPU-Interface LCD Write Command */
#define REG_LCM_INT_CS	     (LCM_BA+0x0c)    /* Interrupt Control/Status Register */
#define REG_LCM_CRTC_SIZE    (LCM_BA+0x10)    /* CRTC Display Size Control Register */
#define REG_LCM_CRTC_DEND    (LCM_BA+0x14)    /* CRTC Display Enable End */
#define REG_LCM_CRTC_HR	     (LCM_BA+0x18)    /* CRTC Internal Horizontal Retrace Control Register */
#define REG_LCM_CRTC_HSYNC   (LCM_BA+0x1C)    /* CRTC Horizontal Sync Control Register */
#define REG_LCM_CRTC_VR	     (LCM_BA+0x20)    /* CRTC Internal Vertical Retrace Control Register */
#define REG_LCM_VA_BADDR0    (LCM_BA+0x24)    /* Video Stream Frame Buffer-0 Starting Address */
#define REG_LCM_VA_BADDR1    (LCM_BA+0x28)    /* Video Stream Frame Buffer-1 Starting Address */
#define REG_LCM_VA_FBCTRL    (LCM_BA+0x2C)    /* Video Stream Frame Buffer Control Register */
#define REG_LCM_VA_SCALE     (LCM_BA+0x30)    /* Video Stream Scaling Control Register */
#define REG_LCM_VA_TEST      (LCM_BA+0x34)    /* Test mode control register */
#define REG_LCM_OSD_WIN_S    (LCM_BA+0x40)    /* OSD Window Starting Coordinates */
#define REG_LCM_OSD_WIN_E    (LCM_BA+0x44)    /* OSD Window Ending Coordinates */
#define REG_LCM_OSD_BADDR    (LCM_BA+0x48)    /* OSD Stream Frame Buffer Starting Address */
#define REG_LCM_OSD_FBCTRL   (LCM_BA+0x4c)    /* OSD Stream Frame Buffer Control Register */
#define REG_LCM_OSD_OVERLAY  (LCM_BA+0x50)    /* OSD Overlay Control Register */
#define REG_LCM_OSD_CKEY     (LCM_BA+0x54)    /* OSD Overlay Color-Key Pattern Register */
#define REG_LCM_OSD_CMASK    (LCM_BA+0x58)    /* OSD Overlay Color-Key Mask Register */


/**********************************************************************************************************
 *                                                               
 * 11. Sensor DSP Control Registers  
 *
 **********************************************************************************************************/
#define REG_DSPFunctionCR		(DSP_BA+0x00)    /* Switches of Sub-blocks */
#define	REG_DSPInterruptCR		(DSP_BA+0x04)    /* Interrupt Mask and Status */

/* Sensor interface control REG_isters */
#define	REG_DSPInterfaceCR		(DSP_BA+0x08)    /* Sensor Interface */
#define REG_DSPHsyncCR1			(DSP_BA+0x0C)    /* Master mode Hsync position */
#define REG_DSPHsyncCR2			(DSP_BA+0x10)    /* Master mode Hsync width */
#define REG_DSPVsyncCR1			(DSP_BA+0x14)    /* Master mode Vsync position */
#define REG_DSPVsyncCR2			(DSP_BA+0x18)    /* Master mode Vsync width */
#define REG_DSPLineCounter		(DSP_BA+0x1C)    /* Current Line Position */

/* Black level control REG_isters */
#define	REG_DSPBlackLMode		(DSP_BA+0x20)    /* Black Level Clamping Mode */
#define REG_DSPBlackLCropCR1	(DSP_BA+0x24)    /* Black Level Clamping Parameters */
#define	REG_DSPBlackLCropCR2	(DSP_BA+0x28)    /* Position of Black Level Window */
#define REG_DSPUserBlackLCR		(DSP_BA+0x2C)    /* User defined black level */
#define	REG_DSPFrameDBL			(DSP_BA+0x30)    /* Frame base detected black level */

/* Cropping window */
#define REG_DSPCropCR1			(DSP_BA+0x34)    /* Starting Address of Cropping Window */
#define	REG_DSPCropCR2			(DSP_BA+0x38)    /* Width and Height of Cropping Window */

/* digital programmable gain multiplier */
#define REG_DSPColorBalCR1		(DSP_BA+0x3C)    /* Gain factor for R and B */
#define	REG_DSPColorBalCR2		(DSP_BA+0x40)    /* Gain factor for Gr and Gb */

/* MCG and edge enhancement */
#define	REG_DSPMCGCR			(DSP_BA+0x44)    /* MCG Threshold */
#define	REG_DSPEdgeConfigCR		(DSP_BA+0x48)    /* Edge Configuration */

/* Color correction matrix */
#define	REG_DSPColorMatrixCR1	(DSP_BA+0x4C)    /* Color Correction Matrix -1 */
#define	REG_DSPColorMatrixCR2	(DSP_BA+0x50)    /* Color Correction Matrix -2 */
#define	REG_DSPColorMatrixCR3	(DSP_BA+0x54)    /* Color Correction Matrix -3 */

/* High saturation suppression */
#define	REG_DSPHSSCR			(DSP_BA+0x60)    /* High Saturation Suppression */

/* Gamma correction matrix */
#define	REG_DSPGammaCR			(DSP_BA+0x64)    /* Gamma Correction Type Selection */
#define	REG_DSPGammaTbl1		(DSP_BA+0x70)    /* Gamma Correction Table 0x70 ~ 0xAC */
#define	REG_DSPGammaTbl2		(DSP_BA+0x74)
#define	REG_DSPGammaTbl3		(DSP_BA+0x78)
#define	REG_DSPGammaTbl4		(DSP_BA+0x7C)
#define	REG_DSPGammaTbl5		(DSP_BA+0x80)
#define	REG_DSPGammaTbl6		(DSP_BA+0x84)
#define	REG_DSPGammaTbl7		(DSP_BA+0x88)
#define	REG_DSPGammaTbl8		(DSP_BA+0x8C)
#define	REG_DSPGammaTbl9		(DSP_BA+0x90)
#define	REG_DSPGammaTbl10		(DSP_BA+0x94)
#define	REG_DSPGammaTbl11		(DSP_BA+0x98)
#define	REG_DSPGammaTbl12		(DSP_BA+0x9C)
#define	REG_DSPGammaTbl13		(DSP_BA+0xA0)
#define	REG_DSPGammaTbl14		(DSP_BA+0xA4)
#define	REG_DSPGammaTbl15		(DSP_BA+0xA8)
#define	REG_DSPGammaTbl16		(DSP_BA+0xAC)

/* Auto Exposure */
#define REG_DSPAECCR1			(DSP_BA+0xB0)    /* Foreground Window */
#define REG_DSPAECAvg			(DSP_BA+0xB4)    /* AEC Statistics */

/* Auto White Balance */
#define	REG_DSPAWBWndCR			(DSP_BA+0xB8)    /* Window for AWB */
#define	REG_DSPAWBWOCR1			(DSP_BA+0xBC)    /* White object definition */
#define	REG_DSPAWBWOCR2			(DSP_BA+0xC0)    /* White object definition */
#define REG_DSPAWBCR			(DSP_BA+0xC4)    /* White object definition */
#define	REG_DSPAWBWOCount		(DSP_BA+0xC8)    /* Total White Points */
#define	REG_DSPAWBWOAvg			(DSP_BA+0xCC)    /* Average R, G, B of White Points */

/* Auto Focus */
#define	REG_DSPAFCR				(DSP_BA+0xD0)    /* AF Window Setting */
#define	REG_DSPAFreport			(DSP_BA+0xD4)    /* Auto Focus Statistics */

/* Histogram */
#define	REG_DSPHistoCR			(DSP_BA+0xD8)    /* Histogram Configuration */
#define	REG_DSPHistoReport1		(DSP_BA+0xDC)    /* Histogram Statistics -1 */
#define	REG_DSPHistoReport2		(DSP_BA+0xE0)    /* Histogram Statistics -2 */
#define	REG_DSPHistoReport3		(DSP_BA+0xE4)    /* Histogram Statistics -3 */
#define	REG_DSPHistoReport4		(DSP_BA+0xE8)    /* Histogram Statistics -4 */
#define	REG_DSPHistoReport5		(DSP_BA+0xEC)    /* Histogram Statistics -5 */
#define	REG_DSPHistoReport6 	(DSP_BA+0xF0)    /* Histogram Statistics -6 */

/* Sub-Windows */
#define REG_DSPSubWndCR1		(DSP_BA+0xF4)    /* Sub-Windows Definitions */
#define REG_DSPSubWndCR2		(DSP_BA+0xF8)    /* Sub-Windows Definitions */
#define REG_DSPSubWndAvgY1		(DSP_BA+0x100)   /* Reported values 0x100 ~ 0x13C */
#define REG_DSPSubWndAvgY2		(DSP_BA+0x104)
#define REG_DSPSubWndAvgY3		(DSP_BA+0x108)
#define REG_DSPSubWndAvgY4		(DSP_BA+0x10C)
#define REG_DSPSubWndAvgR1		(DSP_BA+0x110)
#define REG_DSPSubWndAvgR2		(DSP_BA+0x114)
#define REG_DSPSubWndAvgR3		(DSP_BA+0x118)
#define REG_DSPSubWndAvgR4		(DSP_BA+0x11C)
#define REG_DSPSubWndAvgG1		(DSP_BA+0x120)
#define REG_DSPSubWndAvgG2		(DSP_BA+0x124)
#define REG_DSPSubWndAvgG3		(DSP_BA+0x128)
#define REG_DSPSubWndAvgG4		(DSP_BA+0x12C)
#define REG_DSPSubWndAvgB1		(DSP_BA+0x130)
#define REG_DSPSubWndAvgB2		(DSP_BA+0x134)
#define REG_DSPSubWndAvgB3		(DSP_BA+0x138)
#define REG_DSPSubWndAvgB4		(DSP_BA+0x13C)

/* Bad uPixels */
#define	REG_DSPBadPixelCR		(DSP_BA+0x140)    /* Bad-Pixel Positions */
#define REG_DSPBadPixelIndex	(DSP_BA+0x144)    /* Bad-Pixel Index */
 
 
/**********************************************************************************************************
 *                                                               
 * 12. Video Capture Engine Control Registers  
 *
 **********************************************************************************************************/
#define REG_CAPEngine             (CAP_BA+0x00)   /* VPRE Engine Status */
#define REG_CAPFuncEnable         (CAP_BA+0x04)   /* Sub-Function Control */
#define REG_CAPColorMode          (CAP_BA+0x08)   /* VPRE Color Feature Register */
#define REG_CAPRotationMode       (CAP_BA+0x0C)   /* VPRE Rotation Mode Register */
#define REG_CAPInterfaceConf      (CAP_BA+0x10)   /* VPRE interface configuration */
#define REG_CAPCropWinStarPos     (CAP_BA+0x14)   /* VPRE Cropping Window Starting Position */
#define REG_CAPCropWinSize        (CAP_BA+0x18)   /* VPRE Cropping Window Size */
#define REG_CAPDownScaleFilter    (CAP_BA+0x1C)   /* Down Scale Factor Register */
#define REG_CAPPlaDownScale       (CAP_BA+0x20)   /* Planar Scale Factor Register */
#define REG_CAPlaRealSize         (CAP_BA+0x24)   /* Planar Real Image Size */
#define REG_CAPPacDownScale       (CAP_BA+0x28)   /* Packet Scale Factor Register */
#define REG_CAPPacRealSize        (CAP_BA+0x2C)   /* Packet Real Image Size */
#define REG_CAPPlaYFB0StaAddr     (CAP_BA+0x30)   /* Planar Y pipe Frame buffer 0 starting address */
#define REG_CAPPlaUFB0StaAddr     (CAP_BA+0x34)   /* Planar U pipe Frame buffer 0 starting address */
#define REG_CAPPlaVFB0StaAddr     (CAP_BA+0x38)   /* Planar V pipe Frame buffer 0 starting addres */
#define REG_CAPPacFB0StaAddr      (CAP_BA+0x3C)   /* Packet pipe Frame buffer 0 starting address */
#define REG_CAPPlaYFB1StaAddr     (CAP_BA+0x40)   /* Planar Y pipe Frame buffer 1 starting address */
#define REG_CAPPlaUFB1StaAddr     (CAP_BA+0x44)   /* Planar U pipe Frame buffer 1 starting address */
#define REG_CAPPlaVFB1StaAddr     (CAP_BA+0x48)   /* Planar V pipe Frame buffer 1 starting address */
#define REG_CAPPacFB1StaAddr      (CAP_BA+0x4C)   /* Packet pipe Frame buffer 1 starting address */
#define REG_CAPPlaMaskStaAddr     (CAP_BA+0x50)   /* Planar Overlap mask starting address */
#define REG_CAPPacMaskStaAddr     (CAP_BA+0x54)   /* Packet Overlap mask starting address */
#define REG_CAPPlaLineOffset      (CAP_BA+0x58)   /* Planar line offset value */
#define REG_CAPPacLineOffset      (CAP_BA+0x5C)   /* Packet line offset value */
#define REG_CAPFIFOThreshold      (CAP_BA+0x60)   /* FIFO threshold value */
#define	REG_LensShadingCR1		  (CAP_BA+0x64)   /* CFA type and shift value */
#define	REG_LensShadingCR2	      (CAP_BA+0x68)   /* The center position */
#define	REG_LensShadingCR3	      (CAP_BA+0x6C)   /* The coefficients for Y or R */
#define	REG_LensShadingCR4	      (CAP_BA+0x70)   /* The coefficients for U or G */
#define	REG_LensShadingCR5	      (CAP_BA+0x74)   /* The coefficients for V or B */
#define	REG_DSPVideoQuaCR1	      (CAP_BA+0x78)   /* Brightness and Contrast */
#define	REG_DSPVideoQuaCR2	      (CAP_BA+0x7C)   /* Hue and Saturation */

 
/**********************************************************************************************************
 *                                                               
 * 13. JPEG Codec Control Registers  
 *
 **********************************************************************************************************/
#define JREGNUM			(0x1BC / 4 + 1)    /* The number of JPEG registers. */

#define REG_JMCR		(JPG_BA + 0x000)   /* JPEG Mode Control Register */
#define REG_JHEADER		(JPG_BA + 0x004)   /* JPEG Encode Header Control Register */
#define REG_JITCR		(JPG_BA + 0x008)   /* JPEG Image Type Control Register */
#define REG_JPRIQC		(JPG_BA + 0x010)   /* JPEG Encode Primary Q-Table Control Register */
#define REG_JTHBQC		(JPG_BA + 0x014)   /* JPEG Encode Thumbnail Q-Table Control Register */
#define REG_JPRIWH		(JPG_BA + 0x018)   /* JPEG Encode Primary Width/Height Register */
#define REG_JTHBWH		(JPG_BA + 0x01C)   /* JPEG Encode Thumbnail Width/Height Register */
#define REG_JPRST		(JPG_BA + 0x020)   /* JPEG Encode Primary Restart Interval Register */
#define REG_JTRST		(JPG_BA + 0x024)   /* JPEG Encode Thumbnail Restart Interval Register */
#define REG_JDECWH		(JPG_BA + 0x028)   /* JPEG Decode Image Width/Height Register */
#define REG_JINTCR		(JPG_BA + 0x02C)   /* JPEG Interrupt Control and Status Register */
#define REG_JDCOLC		(JPG_BA + 0x030)   /* JPEG Decode Image Color Control Register */
#define REG_JDCOLP		(JPG_BA + 0x034)   /* JPEG Decode Image Color Pattern Register */
#define REG_JTEST		(JPG_BA + 0x040)   /* JPEG Test Mode Control Register */
#define REG_JWINDEC0	(JPG_BA + 0x044)   /* JPEG Window Decode Mode Control Register 0 */
#define REG_JWINDEC1	(JPG_BA + 0x048)   /* JPEG Window Decode Mode Control Register 1 */
#define REG_JWINDEC2	(JPG_BA + 0x04C)   /* JPEG Window Decode Mode Control Register 2 */
#define REG_JMACR		(JPG_BA + 0x050)   /* JPEG Memory Address Mode Control Registe */
#define REG_JPSCALU		(JPG_BA + 0x054)   /* JPEG Primary Scaling-Up Control Register */
#define REG_JPSCALD		(JPG_BA + 0x058)   /* JPEG Primary Scaling-Down Control Register */
#define REG_JTSCALD		(JPG_BA + 0x05C)   /* JPEG Thumbnail Scaling-Down Control Register */
#define REG_JDBCR		(JPG_BA + 0x060)   /* Dual-Buffer Control Register */
#define REG_JRESERVE	(JPG_BA + 0x070)   /* Primary Encode Bit-stream Reserved Size Register */
#define REG_JOFFSET		(JPG_BA + 0x074)   /* Address Offset Between Primary/Thumbnail Register */
#define REG_JFSTRIDE	(JPG_BA + 0x078)   /* JPEG Encode Bit-stream Frame Stride Register */
#define REG_JYADDR0		(JPG_BA + 0x07C)   /* Y Component Frame Buffer-0 Start Address Register */
#define REG_JUADDR0		(JPG_BA + 0x080)   /* U Component Frame Buffer-0 Start Address Register */
#define REG_JVADDR0		(JPG_BA + 0x084)   /* V Component Frame Buffer-0 Start Address Register */
#define REG_JYADDR1		(JPG_BA + 0x088)   /* Y Component Frame Buffer-1 Start Address Register */
#define REG_JUADDR1		(JPG_BA + 0x08C)   /* U Component Frame Buffer-1 Start Address Register */
#define REG_JVADDR1		(JPG_BA + 0x090)   /* V Component Frame Buffer-1 Start Address Register */
#define REG_JYSTRIDE	(JPG_BA + 0x094)   /* Y Component Frame Buffer Stride Register */
#define REG_JUSTRIDE	(JPG_BA + 0x098)   /* U Component Frame Buffer Stride Register */
#define REG_JVSTRIDE	(JPG_BA + 0x09C)   /* V Component Frame Buffer Stride Register */
#define REG_JIOADDR0	(JPG_BA + 0x0A0)   /* Bit-stream Frame Buffer-0 Start Address Register */
#define REG_JIOADDR1	(JPG_BA + 0x0A4)   /* Bit-stream Frame Buffer-1 Start Address Register */
#define REG_JPRI_SIZE	(JPG_BA + 0x0A8)   /* JPEG Encode Primary Bit-stream Size Register */
#define REG_JTHB_SIZE	(JPG_BA + 0x0AC)   /* JPEG Encode Thumbnail Bit-stream Size Register */
#define REG_JSRCWH		(JPG_BA + 0x0B0)   /* JPEG Encode Source Image Width/Height Register */

/* QTable 0 ~ 2 
   1. Needs to be programmed in byte unit.
   2. While reading QTAB, it must read twice to get the correct data due to the QTAB is realized by SRAM. */
#define REG_JQTAB0		(JPG_BA + 0x100) /* The start address of JPEG Quantization-Table 0. 0x100 ~ 0x13C */
#define REG_JQTAB1		(JPG_BA + 0x140) /* The start address of JPEG Quantization-Table 1. 0x140 ~ 0x17C */
#define REG_JQTAB2		(JPG_BA + 0x180) /* The start address of JPEG Quantization-Table 2. 0x180 ~ 0x1BC */
 
 
/**********************************************************************************************************
 *                                                               
 * 14. MPEG-4 Video Codec Control Registers  
 *
 **********************************************************************************************************/
/* Encoder DMA map */ 
#define	REG_MP4_INTERRUPT					(MP4_BA+0x038) 
#define	REG_MP4_HOSTIF_DELAY				(MP4_BA+0x040) 

/* Encoder DMA map */ 
#define	REG_ENC_MBF_DMA_CSR				    (MP4_BA+0x3A8)
#define	REG_ENC_MBF_DMA_THSH				(MP4_BA+0x3B0)
#define	REG_ENC_MBF_DMA_ADRS				(MP4_BA+0x3B8)

#define	REG_ENC_MOF_DMA_CSR				    (MP4_BA+0x3C8)
#define	REG_ENC_MOF_DMA_THSH				(MP4_BA+0x3D0)
#define	REG_ENC_MOF_DMA_ADRS				(MP4_BA+0x3D8)

#define	REG_ENC_MIF_DMA_CSR				    (MP4_BA+0x3E8)
#define	REG_ENC_MIF_DMA_THSH				(MP4_BA+0x3F0)
#define	REG_ENC_MIF_DMA_ADRS				(MP4_BA+0x3F8)

/* Decoder DMA map */ 
#define	REG_DEC_MBF_DMA_CSR				    (MP4_BA+0x308)
#define	REG_DEC_MBF_DMA_THSH				(MP4_BA+0x310)
#define	REG_DEC_MBF_DMA_ADRS				(MP4_BA+0x318)

#define	REG_DEC_MOF_DMA_CSR				    (MP4_BA+0x328)
#define	REG_DEC_MOF_DMA_THSH				(MP4_BA+0x330)
#define	REG_DEC_MOF_DMA_ADRS				(MP4_BA+0x338)

#define	REG_DEC_MIF_DMA_CSR				    (MP4_BA+0x348)
#define	REG_DEC_MIF_DMA_THSH				(MP4_BA+0x350)
#define	REG_DEC_MIF_DMA_ADRS				(MP4_BA+0x358)

/* Encoder register map */
#define	REG_ENCODER_CONTROL					(MP4_BA+0x400)	
#define REG_ENCODER_CONTROL_WIDTH			(MP4_BA+0x404)
#define REG_ENCODER_STATUS					(MP4_BA+0x408)	
#define REG_ENCODER_CONTROL_HEIGHT			(MP4_BA+0x40c)	

#define REG_ENCODER_CONTROL_IRQ				(MP4_BA+0x410)
#define REG_ENCODER_CONTROL_VTI				(MP4_BA+0x414)
#define REG_ENCODER_COUNT					(MP4_BA+0x418)
#define REG_ENCODER_CONTROL_FUP				(MP4_BA+0x41C)

#define REG_ENCODER_RATE_OUT				(MP4_BA+0x420)
#define REG_ENCODER_CONFIG					(MP4_BA+0x424)
#define REG_ENCODER_IDENTITY				(MP4_BA+0x428)
#define REG_ENCODER_RATE_IN					(MP4_BA+0x42C)

#define REG_ENCODER_RATE_EX					(MP4_BA+0x430)
#define REG_ENCODER_CONTROL_IO				(MP4_BA+0x434)
#define REG_ENCODER_ADR_INP_Y				(MP4_BA+0x438)
#define REG_ENCODER_ADR_INP_U				(MP4_BA+0x43C)

#define REG_ENCODER_ADR_INP_V				(MP4_BA+0x440)
#define REG_ENCODER_ADR_REF					(MP4_BA+0x444)
#define REG_ENCODER_ADR_REC					(MP4_BA+0x448)	
#define REG_ENCODER_ADR_BIT					(MP4_BA+0x44C)

#define REG_ENCODER_RATE_MAD         		(MP4_BA+0x450)
#define REG_ENCODER_CONTROL_FILTER   		(MP4_BA+0x454)
#define REG_ENCODER_DISTORTION      		(MP4_BA+0x458)
#define REG_ENCODER_QUANT_MATRIX     		(MP4_BA+0x45C)

/*  Decoder REG_ister map */
#define REG_DECODER_CONTROL		     		(MP4_BA+0x600)
#define REG_DECODER_CONTROL_WIDTH	 		(MP4_BA+0x604)
#define REG_DECODER_STATUS            		(MP4_BA+0x608)
#define REG_DECODER_CONTROL_HEIGHT    		(MP4_BA+0x60C)

#define REG_DECODER_CONTROL_IRQ		 		(MP4_BA+0x610)
#define REG_DECODER_CONTROL_IO        		(MP4_BA+0x614)
#define REG_DECODER_STATUS_VTI		 		(MP4_BA+0x618)
#define REG_DECODER_ADR_BIT           		(MP4_BA+0x61C)

#define REG_DECODER_COUNT             		(MP4_BA+0x620)
#define REG_DECODER_ADR_REF_Y         		(MP4_BA+0x624)
#define REG_DECODER_IDENTITY         		(MP4_BA+0x628)
#define REG_DECODER_ADR_REF_U        		(MP4_BA+0x62C)

#define REG_DECODER_ADR_REF_V        		(MP4_BA+0x630)
#define REG_DECODER_ADR_OUT_Y         		(MP4_BA+0x634)
#define REG_DECODER_ADR_OUT_U         		(MP4_BA+0x638)
#define REG_DECODER_ADR_OUT_V         		(MP4_BA+0x63C)

#define REG_DECODER_QUANT_MATRIX      		(MP4_BA+0x640) 
 
 
/**********************************************************************************************************
 *                                                               
 * 15. 2-D Graphic Engine Control Registers  
 *
 **********************************************************************************************************/
#define REG_GE_TRIGGER                  (GE_BA+0x000)      /* trigger control */
#define REG_GE_SRC_START00_ADDR         (GE_BA+0x004)      /* source start (0,0) of X/Y mode */
#define REG_GE_FILTER_SRC_ADDR          (GE_BA+0x004)      /* shared with CR004 */
#define REG_GE_FILTER0                  (GE_BA+0x008)      /* 3x3 filter parameter 1 */
#define REG_GE_FILTER1                  (GE_BA+0x00C)      /* 3x3 filter parameter 2 */
#define REG_GE_INTS                     (GE_BA+0x010)      /* interrupt status */
#define REG_GE_PAT_START_ADDR           (GE_BA+0x014)      /* pattern start */
#define REG_GE_ERR_TERM_STEP_CONST      (GE_BA+0x018)      /* line parameter 1 */
#define REG_GE_FILTER_THRESHOLD         (GE_BA+0x018)      /* 3x3 filter threshold for max clamping */
#define REG_GE_INIT_ERR_COUNT           (GE_BA+0x01C)      /* line parameter 2 */
#define REG_GE_CONTROL                  (GE_BA+0x020)      /* graphics engine control */
#define REG_GE_BACK_COLOR               (GE_BA+0x024)      /* background color */
#define REG_GE_FORE_COLOR               (GE_BA+0x028)      /* foreground color */
#define REG_GE_TRANSPARENCY_COLOR       (GE_BA+0x02C)      /* color key */
#define REG_GE_TRANSPARENCY_MASK        (GE_BA+0x030)      /* color key mask */
#define REG_GE_DEST_START00_ADDR        (GE_BA+0x034)      /* dest start (0,0) of X/Y mode */
#define REG_GE_FILTER_DEST_ADDR         (GE_BA+0x034)      /* shared with CR034 */
#define REG_GE_PITCH                    (GE_BA+0x038)      /* pitch of X/Y mode */
#define REG_GE_FILTER_PITCH             (GE_BA+0x038)      /* shared with CR038 */
#define REG_GE_SRC_START_ADDR           (GE_BA+0x03C)      /* source start of BitBLT */
#define REG_GE_DEST_START_ADDR          (GE_BA+0x040)      /* dest start of BitBLT */
#define REG_GE_DIMENSION                (GE_BA+0x044)      /* width/height of X/Y BitBLT */
#define REG_GE_FILTER_DIMENSION         (GE_BA+0x044)      /* shared with CR044 */
#define REG_GE_CLIP_TL                  (GE_BA+0x048)      /* clip top-left */
#define REG_GE_CLIP_BR                  (GE_BA+0x04C)      /* clip bottom-right */
#define REG_GE_PATA                     (GE_BA+0x050)      /* mono pattern 1 */
#define REG_GE_PATB                     (GE_BA+0x054)      /* mono pattern 2 */
#define REG_GE_WRITE_MASK               (GE_BA+0x058)      /* write plane mask */
#define REG_GE_MISC                     (GE_BA+0x05C)      /* misc control */
#define REG_GE_DATA_PORT0               (GE_BA+0x060)      /* HostBLT data port 0x60 ~ 0x7C */
#define REG_GE_DATA_PORT1               (GE_BA+0x064)
#define REG_GE_DATA_PORT2               (GE_BA+0x068)
#define REG_GE_DATA_PORT3               (GE_BA+0x06C)
#define REG_GE_DATA_PORT4               (GE_BA+0x070)
#define REG_GE_DATA_PORT5               (GE_BA+0x074)
#define REG_GE_DATA_PORT6               (GE_BA+0x078)
#define REG_GE_DATA_PORT7               (GE_BA+0x07C) 
 
 
 
/**********************************************************************************************************
 *                                                               
 * 16. Video Processing Engine Control Registers  
 *
 **********************************************************************************************************/
#define REG_VPEEngineTrigger		(VPE_BA+0x00)   /* Video Process Engine Trigger Control Register */
#define REG_VPEDdaFilterCC			(VPE_BA+0x04)   /* DDA 3X3 Filter Central Pixel Tap Coefficient */
#define REG_VPEDdaFilterLU			(VPE_BA+0x08)   /* DDA 3X3 Filter0, Other 4 Coefficients Around Central Pixel */
#define REG_VPEDdaFilterRB			(VPE_BA+0x0C)   /* DDA 3X3 Filter1, Other 4 Coefficients Around Central Pixel */
#define REG_VPEIntStatus			(VPE_BA+0x10)   /* Interrupt Status Register */
#define REG_VPEPacSrcStaAddr		(VPE_BA+0x14)   /* Packet Source Start Address */
#define REG_VPEUVDesSrcPitch		(VPE_BA+0x18)   /* Planar U/V Destination/Source Pitch Register */
#define REG_VPEReset				(VPE_BA+0x1C)   /* Video Process Engine Reset Control Register */
#define REG_VPECommand				(VPE_BA+0x20)   /* Video Process Engine Command Control Register */
#define REG_VPEYSrcStaAddr			(VPE_BA+0x24)   /* Video Process Engine Y Space Planar Start Address */
#define REG_VPEUSrcStaAddr			(VPE_BA+0x28)   /* Video Process Engine U Space Planar Start Address */
#define REG_VPEVSrcStaAddr			(VPE_BA+0x2C)   /* Video Process Engine V Space Planar Start Address */
#define REG_VPEDdaFactor			(VPE_BA+0x30)   /* DDA Vertical and Horizontal Scaling Factor N/M Register */
#define REG_VPEFcPacDesStaAddr		(VPE_BA+0x34)   /* Data Format Conversion Packet Destination Start Address */
#define REG_VPEDesSrcPtich			(VPE_BA+0x38)   /* Video Process Engine Destination/Source Pitch Register */
#define REG_VPEDdaPacDesStaAddr		(VPE_BA+0x3C)   /* DDA Down Scaling Packet Destination Start Address */
#define REG_VPERotRefPacDesStaAddr	(VPE_BA+0x40)   /* Rotate Reference Packet Destination Start Address */
#define REG_VPESrcHW				(VPE_BA+0x44)   /* Video Process Engine Width/Height Dimension Register */
#define REG_VPEDdaYDesStaAddr		(VPE_BA+0x48)   /* DDA Y Pipe Destination Start Address */
#define REG_VPEDdaUDesStaAddr		(VPE_BA+0x4C)   /* DDA U Pipe Destination Start Address */
#define REG_VPEDdaVDesStaAddr		(VPE_BA+0x50)   /* DDA V Pipe Destination Start Address */
#define REG_VPERotRefYDesStaAddr	(VPE_BA+0x54)   /* Rotate Reference Y Pipe Destination Start Address */
#define REG_VPERotRefUDesStaAddr	(VPE_BA+0x58)   /* Rotate Reference U Pipe Destination Start Address */
#define REG_VPERotRefVDesStaAddr	(VPE_BA+0x5C)   /* Rotate Reference V Pipe Destination Start Address */ 
 

/**********************************************************************************************************
 *                                                               
 * 17. Motion Estimation Engine Control Registers  
 *
 **********************************************************************************************************/
#define REG_ME_CR			(ME_BA+0x000)    /* ME Control Register */
#define REG_ME_INTCR		(ME_BA+0x004)    /* ME Interrupt Control Register */
#define REG_ME_CADDR		(ME_BA+0x008)    /* Y Component of Current Block Start Address Register */
#define REG_ME_CSTRIDE		(ME_BA+0x00C)    /* Y Component of Current Frame Buffer Stride Register */
#define REG_ME_PADDR		(ME_BA+0x010)    /* Y Component of Previous Block Start Address Register */
#define REG_ME_PSTRIDE		(ME_BA+0x014)    /* Y Component of Previous Frame Buffer Stride Register */
#define REG_ME_MV			(ME_BA+0x018)    /* Motion Vector */
#define REG_ME_MAD			(ME_BA+0x01C)    /* Mean Absolute Difference of Motion Vector */
   
 
/**********************************************************************************************************
 *                                                               
 * 18. High Speed UART Control Registers  
 *
 **********************************************************************************************************/
#define REG_HS_COM_RX     (HSU_BA+0x00)     /* (R) RX buffer */
#define REG_HS_COM_TX     (HSU_BA+0x00)     /* (W) TX buffer */
#define REG_HS_COM_IER    (HSU_BA+0x04)     /* Interrupt enable register */
#define REG_HS_COM_LSB    (HSU_BA+0x00)     /* Divisor latch LSB */
#define REG_HS_COM_MSB    (HSU_BA+0x04)     /* Divisor latch MSB */
#define REG_HS_COM_IIR    (HSU_BA+0x08)     /* (R) Interrupt ident. register */
#define REG_HS_COM_FCR    (HSU_BA+0x08)     /* (W) FIFO control register */
#define REG_HS_COM_LCR    (HSU_BA+0x0C)     /* Line control register */
#define REG_HS_COM_MCR    (HSU_BA+0x10)     /* Modem control register */
#define	REG_HS_COM_LSR    (HSU_BA+0x14)     /* (R) Line status register */
#define	REG_HS_COM_TOR    (HSU_BA+0x1C)     /* Time out register */ 
 
 
/**********************************************************************************************************
 *                                                               
 * 19. UART Control Registers  
 *
 **********************************************************************************************************/
#define REG_COM_TX     (UART_BA+0x0)    /* (W) TX buffer */
#define REG_COM_RX     (UART_BA+0x0)    /* (R) RX buffer */
#define REG_COM_LSB    (UART_BA+0x0)    /* Divisor latch LSB */
#define REG_COM_MSB    (UART_BA+0x04)   /* Divisor latch MSB */
#define REG_COM_IER    (UART_BA+0x04)   /* Interrupt enable register */
#define REG_COM_IIR    (UART_BA+0x08)   /* (R) Interrupt ident. register */
#define REG_COM_FCR    (UART_BA+0x08)   /* (W) FIFO control register */
#define REG_COM_LCR    (UART_BA+0x0C)   /* Line control register */
#define REG_COM_MCR    (UART_BA+0x10)   /* Modem control register */
#define	REG_COM_LSR    (UART_BA+0x14)   /* (R) Line status register */
#define REG_COM_MSR    (UART_BA+0x18)   /* (R) Modem status register */
#define	REG_COM_TOR    (UART_BA+0x1C)   /* (R) Time out register */
 
 
/**********************************************************************************************************
 *                                                               
 * 20. Timer Control Registers  
 *
 **********************************************************************************************************/
#define	REG_TCR0     (TMR_BA+0x0)    /* Control Register 0 */
#define	REG_TCR1     (TMR_BA+0x04)   /* Control Register 1 */
#define	REG_TICR0    (TMR_BA+0x08)   /* Initial Control Register 0 */
#define	REG_TICR1    (TMR_BA+0x0C)   /* Initial Control Register 1 */
#define	REG_TDR0     (TMR_BA+0x10)   /* Data Register 0 */
#define	REG_TDR1     (TMR_BA+0x14)   /* Data Register 1 */
#define	REG_TISR     (TMR_BA+0x18)   /* Interrupt Status Register */
#define REG_WTCR     (TMR_BA+0x1C)   /* Watchdog Timer Control Register */
 
 
/**********************************************************************************************************
 *                                                               
 * 21. Interrupt Control Registers  
 *
 **********************************************************************************************************/
#define REG_AIC_SCR1    (AIC_BA+0x04)    /* Source control register 1 */
#define REG_AIC_SCR2    (AIC_BA+0x08)    /* Source control register 2 */
#define REG_AIC_SCR3    (AIC_BA+0x0C)    /* Source control register 3 */
#define REG_AIC_SCR4    (AIC_BA+0x10)    /* Source control register 4 */
#define REG_AIC_SCR5    (AIC_BA+0x14)    /* Source control register 5 */
#define REG_AIC_SCR6    (AIC_BA+0x18)    /* Source control register 6 */
#define REG_AIC_SCR7    (AIC_BA+0x1C)    /* Source control register 7 */
#define REG_AIC_SCR8    (AIC_BA+0x20)    /* Source control register 8 */
#define REG_AIC_SCR9    (AIC_BA+0x24)    /* Source control register 9 */
#define REG_AIC_SCR10   (AIC_BA+0x28)    /* Source control register 10 */
#define REG_AIC_SCR11   (AIC_BA+0x2C)    /* Source control register 11 */
#define REG_AIC_SCR12   (AIC_BA+0x30)    /* Source control register 12 */
#define REG_AIC_SCR13   (AIC_BA+0x34)    /* Source control register 13 */
#define REG_AIC_SCR14   (AIC_BA+0x38)    /* Source control register 14 */
#define REG_AIC_SCR15   (AIC_BA+0x3C)    /* Source control register 15 */
#define REG_AIC_SCR16   (AIC_BA+0x40)    /* Source control register 16 */
#define REG_AIC_SCR17   (AIC_BA+0x44)    /* Source control register 17 */
#define REG_AIC_SCR18   (AIC_BA+0x48)    /* Source control register 18 */
#define REG_AIC_SCR19   (AIC_BA+0x4C)    /* Source control register 19 */
#define REG_AIC_SCR20   (AIC_BA+0x50)    /* Source control register 20 */
#define REG_AIC_SCR21   (AIC_BA+0x54)    /* Source control register 21 */
#define REG_AIC_SCR22   (AIC_BA+0x58)    /* Source control register 22 */
#define REG_AIC_SCR23   (AIC_BA+0x5C)    /* Source control register 23 */
#define REG_AIC_SCR24   (AIC_BA+0x60)    /* Source control register 24 */
#define REG_AIC_SCR25   (AIC_BA+0x64)    /* Source control register 25 */
#define REG_AIC_SCR26   (AIC_BA+0x68)    /* Source control register 26 */
#define REG_AIC_IRSR    (AIC_BA+0x100)   /* Interrupt raw status register */
#define REG_AIC_IASR    (AIC_BA+0x104)   /* Interrupt active status register */
#define REG_AIC_ISR     (AIC_BA+0x108)   /* Interrupt status register */
#define REG_AIC_IPER    (AIC_BA+0x10C)   /* Interrupt priority encoding register */
#define REG_AIC_ISNR    (AIC_BA+0x110)   /* Interrupt source number register */
#define REG_AIC_IMR     (AIC_BA+0x114)   /* Interrupt mask register */
#define REG_AIC_OISR    (AIC_BA+0x118)   /* Output interrupt status register */
#define REG_AIC_MECR    (AIC_BA+0x120)   /* Mask enable command register */
#define REG_AIC_MDCR    (AIC_BA+0x124)   /* Mask disable command register */
#define REG_AIC_SSCR    (AIC_BA+0x128)   /* Source set command register */
#define REG_AIC_SCCR    (AIC_BA+0x12C)   /* Source clear command register */
#define REG_AIC_EOSCR   (AIC_BA+0x130)   /* End of service command register */
#define REG_AIC_TEST    (AIC_BA+0x200)   /* ICE/Debug mode register */

  
/**********************************************************************************************************
 *                                                               
 * 22. Universal Serial Interface Control Registers  
 *
 **********************************************************************************************************/
#define	REG_USI_CNTRL		(USI_BA+0x0)     /* Control and Status Register */
#define	REG_USI_DIVIDER		(USI_BA+0x04)    /* Clock Divider Register */
#define	REG_USI_SSR			(USI_BA+0x08)    /* Slave Select Register */
#define	REG_USI_Rx0			(USI_BA+0x10)    /* Data Receive Register 0 */
#define	REG_USI_Rx1			(USI_BA+0x14)    /* Data Receive Register 1 */
#define	REG_USI_Rx2			(USI_BA+0x18)    /* Data Receive Register 2 */
#define	REG_USI_Rx3			(USI_BA+0x1C)    /* Data Receive Register 3 */
#define	REG_USI_Tx0			(USI_BA+0x10)    /* Data Transmit Register 0 */
#define	REG_USI_Tx1			(USI_BA+0x14)    /* Data Transmit Register 1 */
#define	REG_USI_Tx2			(USI_BA+0x18)    /* Data Transmit Register 2 */
#define	REG_USI_Tx3			(USI_BA+0x1C)    /* Data Transmit Register 3 */
 
 
/**********************************************************************************************************
 *                                                               
 * 23. External Bus Interface Control Registers  
 *
 **********************************************************************************************************/
#define	REG_EBISIZE0	(EBI_BA + 0x00)    /* EBI Bank 0 Size Control Register */
#define	REG_EBISIZE1	(EBI_BA + 0x04)    /* EBI Bank 1 Size Control Register */
#define	REG_EBISIZE2	(EBI_BA + 0x08)    /* EBI Bank 2 Size Control Register */
#define	REG_EBISIZE3	(EBI_BA + 0x0C)    /* EBI Bank 3 Size Control Register */
#define	REG_EBITIM0		(EBI_BA + 0x10)    /* EBI Bank0 Timing Control Register */
#define	REG_EBITIM1		(EBI_BA + 0x14)    /* EBI Bank1 Timing Control Register */ 
#define	REG_EBITIM2		(EBI_BA + 0x18)    /* EBI Bank2 Timing Control Register */ 
#define	REG_EBITIM3		(EBI_BA + 0x1C)    /* EBI Bank3 Timing Control Register */ 

 
#endif /* _W99702_H */
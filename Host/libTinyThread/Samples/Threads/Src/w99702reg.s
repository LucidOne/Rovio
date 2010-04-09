SDRM_BA	EQU	0x3FFF0000 ; SDRAM 
GCR_BA	EQU	0x7FF00000 ; Global Control 
CLK_BA	EQU	0x7FF00200 ; Clock Control 
GPIO_BA	EQU	0x7FF00300 ; GPIO Control 
FSB_BA	EQU	0x7FF00400 ; Fast Serial Bus 
HIC_BA	EQU	0x7FF01000 ; Host Bus Interface 
FMI_BA	EQU	0x7FF02000 ; Flash Memory Card Interface 
ADO_BA	EQU	0x7FF03000 ; Audio Interface 
USB_BA	EQU	0x7FF04000 ; USB Device 
LCM_BA	EQU	0x7FF05000 ; Display, LCM Interface & Bypass 
DSP_BA	EQU	0x7FF08000 ; Sensor DSP 
CAP_BA	EQU	0x7FF09000 ; Video Capture Engine 
JPG_BA	EQU	0x7FF0A000 ; JPEG Codec 
MP4_BA	EQU	0x7FF0B000 ; MPEG-4 Video Codec 
GE_BA	EQU	0x7FF0C000 ; 2-D Graphic Engine 
VPE_BA	EQU	0x7FF0D000 ; Video Processing Engine 
ME_BA	EQU	0x7FF0E000 ; Motion Estimation Engine 
HSU_BA	EQU	0x7FFC0000 ; High Speed UART 
UART_BA	EQU	0x7FFC1000 ; UART 
TMR_BA	EQU	0x7FFC2000 ; Timer 
AIC_BA	EQU	0x7FFC3000 ; Interrupt Controller 
USI_BA	EQU	0x7FFC4000 ; Universal Serial Interface 
EBI_BA	EQU	0xFDFF0000 ; External Bus Interface (EBI) 


;*********************************************************************************************************
;*                                                               
;* 1. SDRAM Control Registers  
;*
;*********************************************************************************************************
REG_SDICEN	EQU	(SDRM_BA + 0x000)    ; SDIC enable register 
REG_SDICON	EQU	(SDRM_BA + 0x004)    ; SDIC control register 
REG_SDCONF0	EQU	(SDRM_BA + 0x008)    ; SDRAM bank 0 configuration register 
REG_SDCONF1	EQU	(SDRM_BA + 0x00C)    ; SDRAM bank 1 configuration register 
REG_SDTIME0	EQU	(SDRM_BA + 0x010)    ; SDRAM bank 0 timing control register 
REG_SDTIME1	EQU	(SDRM_BA + 0x014)    ; SDRAM bank 1 timing control register 
REG_ARBCON	EQU	(SDRM_BA + 0x018)    ; Arbiter control register 
REG_TESTCR	EQU	(SDRM_BA + 0x020)    ; SDRAM test control register 
REG_TSTATUS	EQU	(SDRM_BA + 0x024)    ; SDRAM test status register 
REG_TFDATA	EQU	(SDRM_BA + 0x028)    ; SDRAM test fail data 
REG_TFADDR	EQU	(SDRM_BA + 0x02C)    ; SDRAM test fail address 
REG_CKSKEW	EQU	(SDRM_BA + 0xF00)    ; Clock skew control register (for testing) 
 
 
;*********************************************************************************************************
;*                                                               
;* 2. Global Control Registers  
;*
;*********************************************************************************************************
REG_PDID	EQU	(GCR_BA+0x000)    ; Product Identifier Register 
REG_SYS_CFG	EQU	(GCR_BA+0x004)    ; System Power-On Configuration Control Register 
REG_CPUCR	EQU	(GCR_BA+0x008)    ; CPU Control (Reset/Initial) Register 
REG_MISCR	EQU	(GCR_BA+0x00C)    ; Miscellaneous Control Register 
REG_PADC0	EQU	(GCR_BA+0x020)    ; PAD Function Control Register 0 
REG_PADC1	EQU	(GCR_BA+0x024)    ; PAD Function Control Register 1 
REG_PADC2	EQU	(GCR_BA+0x028)    ; PAD Function Control Register 2 
REG_AHB_PRI	EQU	(GCR_BA+0x100)    ; AHB Priority Control Register 
REG_BBLENG0	EQU	(GCR_BA+0x104)    ; Burst Length Control Register for AHB - Lite	
REG_BBLENG1	EQU	(GCR_BA+0x108)    ; Burst Length Control Register for AHB Master	


;*********************************************************************************************************
;*                                                               
;* 3. Clock Control Registers  
;*
;*********************************************************************************************************
REG_PWRCON	EQU	(CLK_BA + 0x00)    ; System Power Down Control Register 
REG_CLKCON	EQU	(CLK_BA + 0x04)    ; Clock Enable Control Register 
REG_APLLCON	EQU	(CLK_BA + 0x08)    ; APLL Control Register 
REG_UPLLCON	EQU	(CLK_BA + 0x0C)    ; UPLL Control Register 
REG_CLKSEL	EQU	(CLK_BA + 0x10)	   ; Clock Source Select Control Register 
REG_CLKDIV0	EQU	(CLK_BA + 0x14)	   ; Clock Divider Number Register 0 
REG_CLKDIV1	EQU	(CLK_BA + 0x18)	   ; Clock Divider Number Register 1 
REG_PLLWCR	EQU	(CLK_BA + 0x1C)	   ; PLL Wake Up Counter Control Register 
REG_FLCCR	EQU	(CLK_BA + 0x20)	   ; Flash Light Charge Counter Register 
REG_PWMCR	EQU	(CLK_BA + 0x24)	   ; Pulse Width Modulation Clock Counter Register 


;*********************************************************************************************************
;*                                                               
;* 4. GPIO Control Registers  
;*
;*********************************************************************************************************
; GPIO Pins  
REG_GPIO_OE		EQU	(GPIO_BA+0x00)   ; GPIO Pins Output Enable Control Register 
REG_GPIO_DAT	EQU	(GPIO_BA+0x04)   ; GPIO Pins Data Register 
REG_GPIO_STS	EQU	(GPIO_BA+0x08)   ; GPIO Pins Status Register 
REG_GPIO_IE		EQU	(GPIO_BA+0x0c)   ; GPIO Pins Interrupt Enable Control Register 
REG_GPIO_IS		EQU	(GPIO_BA+0x10)   ; GPIO Pins Interrupt Status Control Register 
REG_GPIO_PE		EQU	(GPIO_BA+0x14)   ; GPIO Pull-Up/Down Enable Control Register 
REG_GPIO_DEBOUNCE	EQU	(GPIO_BA+0x18)   ; GPIO Pins De-Bounce Control Register 

; GPIO-A Pins 
REG_GPIOA_OE	EQU	(GPIO_BA+0x20)   ; GPIO-A Pins Output Enable Control Register 
REG_GPIOA_DAT	EQU	(GPIO_BA+0x24)   ; GPIO-A Pins Data Register 
REG_GPIOA_STS	EQU	(GPIO_BA+0x28)   ; GPIO-A Pins Status Register 
REG_GPIOA_PE	EQU	(GPIO_BA+0x2c)   ; GPIO-A Pull-Up/Down Enable Control Register 

; GPIO-B Pins 
REG_GPIOB_OE	EQU	(GPIO_BA+0x30)   ; GPIO-B Pins Output Enable Control Register 
REG_GPIOB_DAT	EQU	(GPIO_BA+0x34)   ; GPIO-B Pins Data Register 
REG_GPIOB_STS	EQU	(GPIO_BA+0x38)   ; GPIO-B Pins Status Register 
REG_GPIOB_PE	EQU	(GPIO_BA+0x3c)   ; GPIO-B Pull-Up/Down Enable Control Register 

; GPIO-S Pins 
REG_GPIOS_OE	EQU	(GPIO_BA+0x40)   ; GPIO-S Pins Output Enable Control Register 
REG_GPIOS_DAT	EQU	(GPIO_BA+0x44)   ; GPIO-S Pins Data Register 
REG_GPIOS_STS	EQU	(GPIO_BA+0x48)   ; GPIO-S Pins Status Register 
REG_GPIOS_PE	EQU	(GPIO_BA+0x4c)   ; GPIO-S Pull-Up/Down Enable Control Register 


;*********************************************************************************************************
;*                                                               
;* 5. Fast Serial Bus Control Registers  
;*
;*********************************************************************************************************
REG_FastSerialBusTrigger	EQU	(FSB_BA+0x00)    ; FSB Trigger and SCK Control 
REG_FastSerialBusStatus		EQU	(FSB_BA+0x04)    ; FSB Status Register 
REG_FastSerialBusCR			EQU	(FSB_BA+0x08)    ; FSB Control Register 
REG_FastSerialBusSCKSDA0	EQU	(FSB_BA+0x0C)    ; FSB SCKSDA 0 register 
REG_FastSerialBusSCKSDA1	EQU	(FSB_BA+0x10)    ; FSB SCKSDA 1 register 
REG_SerialBusCR				EQU	(FSB_BA+0x14)    ; FSB Software Mode Register 
 
 
;*********************************************************************************************************
;*                                                               
;* 6. Host Bus Interface Control Registers  
;*
;*********************************************************************************************************
REG_HICFCR	EQU	(HIC_BA+0x00)		; HIC function control REGister 
REG_HICDAR	EQU	(HIC_BA+0x04)		; HIC software DMA address REGister 
REG_HICDLR	EQU	(HIC_BA+0x08)		; HIC software DMA length REGister 
REG_HICIER	EQU	(HIC_BA+0x0C)		; HIC interrupt enable REGister 
REG_HICISR	EQU	(HIC_BA+0x10)		; HIC interrupt status REGister 
REG_HICBIS	EQU	(HIC_BA+0x14)		; HIC BIST REGister 
REG_HICCMR	EQU	(HIC_BA+0x80)		; HIC command port REGister 
REG_HICPAR	EQU	(HIC_BA+0x84)		; HIC parameter/address port REGister 
REG_HICBLR	EQU	(HIC_BA+0x88)		; HIC parameter/burst transfer count REGister 
REG_HICDR	EQU	(HIC_BA+0x8C)		; HIC data port REGister 
REG_HICSR	EQU	(HIC_BA+0x90)		; HIC status port REGister 
REG_HICBUF	EQU	(HIC_BA+0x100)		; HIC internal buffer starting address 
_HICBUF_NUM	EQU	(64)
 
 
;*********************************************************************************************************
;*                                                               
;* 7. Flash memory Card Control Registers  
;*
;*********************************************************************************************************
; Flash Memory Interface Registers definition 
REG_FMICR	EQU	(FMI_BA+0x00)    ; FMI control register 
REG_FMIDSA	EQU	(FMI_BA+0x04)    ; FMI DMA transfer starting address register 
REG_FMIBCR	EQU	(FMI_BA+0x08)    ; FMI DMA byte count register 
REG_FMIIER	EQU	(FMI_BA+0x0C)    ; FMI interrupt enable register 
REG_FMIISR	EQU	(FMI_BA+0x10)    ; FMI interrupt status register 
REG_FB0_0	EQU	(FMI_BA+0x400)   ; Flash buffer 0 
REG_FB1_0	EQU	(FMI_BA+0x800)   ; Flash buffer 1 

; CompactFlash Registers definition 
REG_CFDR	EQU	(FMI_BA+0x100)   ; CF IDE data register 
REG_CFFER	EQU	(FMI_BA+0x104)   ; CF IDE feature/error register 
REG_CFSCR	EQU	(FMI_BA+0x108)   ; CF IDE sector count register 
REG_CFSNR	EQU	(FMI_BA+0x10C)   ; CF IDE sector number register 
REG_CFCLR	EQU	(FMI_BA+0x110)   ; CF IDE cylinder low register 
REG_CFCHR	EQU	(FMI_BA+0x114)   ; CF IDE cylinder high register 
REG_CFSCHR	EQU	(FMI_BA+0x118)   ; CF IDE select card/head register 
REG_CFCSR	EQU	(FMI_BA+0x11C)   ; CF IDE command/status register 
REG_CFDCR	EQU	(FMI_BA+0x120)   ; CF IDE device control/alt status register (HW mode)
                                 ; CF IDE address output control register (SW mode) 
REG_CFDAR	EQU	(FMI_BA+0x124)   ; CF IDE drive address register (HW mode)
                                 ; CF IDE CE2_/CE1_/REG_ output control register (SW mode) 
REG_CFCR	EQU	(FMI_BA+0x128)   ; CF IDE control register 
REG_CFTCR	EQU	(FMI_BA+0x12C)   ; CF IDE timing control register 
REG_CFIER	EQU	(FMI_BA+0x130)   ; CF interrupt enable register 
REG_CFISR	EQU	(FMI_BA+0x134)   ; CF interrupt status register 

; SmartMedia Registers definition 
REG_SMCR	EQU	(FMI_BA+0x200)   ; SmartMedia control register 
REG_SMCMD	EQU	(FMI_BA+0x204)   ; SmartMedia command port register 
REG_SMADDR	EQU	(FMI_BA+0x208)   ; SmartMedia address port register 
REG_SMMADDR	EQU	(FMI_BA+0x20C)   ; SmartMedia multi-cycle address port register 
REG_SMDATA	EQU	(FMI_BA+0x210)   ; SmartMedia data port register 
REG_SMIER	EQU	(FMI_BA+0x214)   ; SmartMedia interrupt enable register 
REG_SMISR	EQU	(FMI_BA+0x218)   ; SmartMedia input signal and interrupt status register 
REG_SMECC0	EQU	(FMI_BA+0x21C)   ; SmartMedia error correction code 0 regisrer 
REG_SMECC1	EQU	(FMI_BA+0x220)   ; SmartMedia error correction code 1 regisrer 
REG_SMECC2	EQU	(FMI_BA+0x224)   ; SmartMedia error correction code 2 regisrer 
REG_SMECC3	EQU	(FMI_BA+0x228)   ; SmartMedia error correction code 3 regisrer 
REG_SMRA_0	EQU	(FMI_BA+0x22C)   ; SmartMedia redundant area register 
REG_SMRA_1	EQU	(FMI_BA+0x230)   ; SmartMedia redundant area register 
REG_SMRA_2	EQU	(FMI_BA+0x234)   ; SmartMedia redundant area register 
REG_SMRA_3	EQU	(FMI_BA+0x238)   ; SmartMedia redundant area register 
REG_SMRA_4	EQU	(FMI_BA+0x23C)   ; SmartMedia redundant area register 
REG_SMRA_5	EQU	(FMI_BA+0x240)   ; SmartMedia redundant area register 
REG_SMRA_6	EQU	(FMI_BA+0x244)   ; SmartMedia redundant area register 
REG_SMRA_7	EQU	(FMI_BA+0x248)   ; SmartMedia redundant area register 
REG_SMRA_8	EQU	(FMI_BA+0x24C)   ; SmartMedia redundant area register 
REG_SMRA_9	EQU	(FMI_BA+0x250)   ; SmartMedia redundant area register 
REG_SMRA_10	EQU	(FMI_BA+0x254)   ; SmartMedia redundant area register 
REG_SMRA_11	EQU	(FMI_BA+0x258)   ; SmartMedia redundant area register 
REG_SMRA_12	EQU	(FMI_BA+0x25C)   ; SmartMedia redundant area register 
REG_SMRA_13	EQU	(FMI_BA+0x260)   ; SmartMedia redundant area register 
REG_SMRA_14	EQU	(FMI_BA+0x264)   ; SmartMedia redundant area register 
REG_SMRA_15	EQU	(FMI_BA+0x268)   ; SmartMedia redundant area register 

; Secure Digit Registers definition 
REG_SDCR	EQU	(FMI_BA+0x300)   ; SD control register 
REG_SDHINI	EQU	(FMI_BA+0x304)   ; SD host initial register 
REG_SDIER	EQU	(FMI_BA+0x308)   ; SD interrupt enable register 
REG_SDISR	EQU	(FMI_BA+0x30C)   ; SD interrupt status register 
REG_SDARG	EQU	(FMI_BA+0x310)   ; SD command argument register 
REG_SDRSP0	EQU	(FMI_BA+0x314)   ; SD receive response token register 0 
REG_SDRSP1	EQU	(FMI_BA+0x318)   ; SD receive response token register 1 
REG_SDBLEN	EQU	(FMI_BA+0x31C)   ; SD block length register 
 
 
;*********************************************************************************************************
;*                                                               
;* 8. Audio Interface Control Registers  
;*
;*********************************************************************************************************
REG_ACTL_CON	EQU	(ADO_BA + 0x00)   ; Audio controller control register 
REG_ACTL_RESET	EQU	(ADO_BA + 0x04)   ; Sub block reset control register 
REG_ACTL_RDSTB	EQU	(ADO_BA + 0x08)   ; DMA destination base address register for record 
REG_ACTL_RDST_LENGTH	EQU	(ADO_BA + 0x0C)   ; DMA destination length register for record 
REG_ACTL_PDSTB	EQU	(ADO_BA + 0x18)   ; DMA destination base address register for play 
REG_ACTL_PDST_LENGTH	EQU	(ADO_BA + 0x1C)   ; DMA destination length register for play 
REG_ACTL_RSR	EQU	(ADO_BA + 0x14)   ; Record status register 
REG_ACTL_PSR	EQU	(ADO_BA + 0x24)   ; Play status register 
REG_ACTL_IISCON	EQU	(ADO_BA + 0x28)   ; IIS control register 
REG_ACTL_ACCON	EQU	(ADO_BA + 0x2C)   ; AC-link control register 
REG_ACTL_ACOS0	EQU	(ADO_BA + 0x30)   ; AC-link out slot 0 
REG_ACTL_ACOS1	EQU	(ADO_BA + 0x34)   ; AC-link out slot 1 
REG_ACTL_ACOS2	EQU	(ADO_BA + 0x38)   ; AC-link out slot 2 
REG_ACTL_ACIS0	EQU	(ADO_BA + 0x3C)   ; AC-link in slot 0 
REG_ACTL_ACIS1	EQU	(ADO_BA + 0x40)   ; AC-link in slot 1 
REG_ACTL_ACIS2	EQU	(ADO_BA + 0x44)   ; AC-link in slot 2 
REG_ACTL_ADCON	EQU	(ADO_BA + 0x48)   ; ADC0 control register 
REG_ACTL_M80CON	EQU	(ADO_BA + 0x4C)   ; M80 interface control register 
REG_ACTL_M80DATA0	EQU	(ADO_BA + 0x50)   ; M80 data0 register 
REG_ACTL_M80DATA1	EQU	(ADO_BA + 0x54)   ; M80 data1 register 
REG_ACTL_M80DATA2	EQU	(ADO_BA + 0x58)   ; M80 data2 register 
REG_ACTL_M80DATA3	EQU	(ADO_BA + 0x5C)   ; M80 data3 register 
REG_ACTL_M80ADDR	EQU	(ADO_BA + 0x60)   ; M80 interface start address register 
REG_ACTL_M80SRADDR	EQU	(ADO_BA + 0x64)   ; M80 interface start address register of right channel 
REG_ACTL_M80SIZE	EQU	(ADO_BA + 0x70)   ; M80 interface data size register 
REG_ACTL_DACON	EQU	(ADO_BA + 0x74)   ; DAC control register 
 
 
;*********************************************************************************************************
;*                                                               
;* 9. USB Device Control Registers  
;*
;*********************************************************************************************************
REG_USB_CTL		EQU	(USB_BA+0x00)    ; USB control register 
REG_USB_CVCMD	EQU	(USB_BA+0x04)    ; USB class or vendor command register 
REG_USB_IE		EQU	(USB_BA+0x08)    ; USB interrupt enable register 
REG_USB_IS		EQU	(USB_BA+0x0c)    ; USB interrupt status register 
REG_USB_IC		EQU	(USB_BA+0x10)    ; USB interrupt status clear register 
REG_USB_IFSTR	EQU	(USB_BA+0x14)    ; USB interface and string register 
REG_USB_ODATA0	EQU	(USB_BA+0x18)    ; USB control transfer-out port 0 register 
REG_USB_ODATA1	EQU	(USB_BA+0x1C)    ; USB control transfer-out port 1 register 
REG_USB_ODATA2	EQU	(USB_BA+0x20)    ; USB control transfer-out port 2 register 
REG_USB_ODATA3	EQU	(USB_BA+0x24)    ; USB control transfer-out port 3 register 
REG_USB_IDATA0	EQU	(USB_BA+0x28)    ; USB control transfer-in data port 0 register 
REG_USB_IDATA1	EQU	(USB_BA+0x2C)    ; USB control transfer-in data port 1 register 
REG_USB_IDATA2	EQU	(USB_BA+0x30)    ; USB control transfer-in data port 2 register 
REG_USB_IDATA3	EQU	(USB_BA+0x34)    ; USB control transfer-in data port 2 register 
REG_USB_SIE		EQU	(USB_BA+0x38)    ; USB SIE status Register 
REG_USB_ENG		EQU	(USB_BA+0x3c)    ; USB Engine Register 
REG_USB_CTLS	EQU	(USB_BA+0x40)    ; USB control transfer status register 
REG_USB_CONFD	EQU	(USB_BA+0x44)    ; USB Configured Value register 
REG_USB_EPA_INFO	EQU	(USB_BA+0x48)    ; USB endpoint A information register 
REG_USB_EPA_CTL	EQU	(USB_BA+0x4c)    ; USB endpoint A control register 
REG_USB_EPA_IE	EQU	(USB_BA+0x50)    ; USB endpoint A Interrupt Enable register 
REG_USB_EPA_IC	EQU	(USB_BA+0x54)    ; USB endpoint A interrupt clear register 
REG_USB_EPA_IS	EQU	(USB_BA+0x58)    ; USB endpoint A interrupt status register 
REG_USB_EPA_ADDR	EQU	(USB_BA+0x5c)    ; USB endpoint A address register 
REG_USB_EPA_LENTH	EQU	(USB_BA+0x60)    ; USB endpoint A transfer length register 
REG_USB_EPB_INFO	EQU	(USB_BA+0x64)    ; USB endpoint B information register 
REG_USB_EPB_CTL	EQU	(USB_BA+0x68)    ; USB endpoint B control register 
REG_USB_EPB_IE	EQU	(USB_BA+0x6c)    ; USB endpoint B Interrupt Enable register 
REG_USB_EPB_IC	EQU	(USB_BA+0x70)    ; USB endpoint B interrupt clear register 
REG_USB_EPB_IS	EQU	(USB_BA+0x74)    ; USB endpoint B interrupt status register 
REG_USB_EPB_ADDR	EQU	(USB_BA+0x78)    ; USB endpoint B address register 
REG_USB_EPB_LENTH	EQU	(USB_BA+0x7c)    ; USB endpoint B transfer length register 
REG_USB_EPC_INFO	EQU	(USB_BA+0x80)    ; USB endpoint C information register 
REG_USB_EPC_CTL	EQU	(USB_BA+0x84)    ; USB endpoint C control register 
REG_USB_EPC_IE	EQU	(USB_BA+0x88)    ; USB endpoint C Interrupt Enable register 
REG_USB_EPC_IC	EQU	(USB_BA+0x8c)    ; USB endpoint C interrupt clear register 
REG_USB_EPC_IS	EQU	(USB_BA+0x90)    ; USB endpoint C interrupt status register 
REG_USB_EPC_ADDR	EQU	(USB_BA+0x94)    ; USB endpoint C address register 
REG_USB_EPC_LENTH	EQU	(USB_BA+0x98)    ; USB endpoint C transfer length register 
REG_USB_EPA_XFER	EQU	(USB_BA+0x9c)    ; USB endpoint A remain transfer length register 
REG_USB_EPA_PKT	EQU	(USB_BA+0xa0)    ; USB endpoint A remain packet length register 
REG_USB_EPB_XFER	EQU	(USB_BA+0xa4)    ; USB endpoint B remain transfer length register 
REG_USB_EPB_PKT	EQU	(USB_BA+0xa8)    ; USB endpoint B remain packet length register 
REG_USB_EPC_XFER	EQU	(USB_BA+0xac)    ; USB endpoint C remain transfer length register 
REG_USB_EPC_PKT	EQU	(USB_BA+0xb0)    ; USB endpoint C remain packet length register 
 
 
;*********************************************************************************************************
;*                                                               
;* 10. LCM Control Registers  
;*
;*********************************************************************************************************
REG_LCM_DCCS		EQU	(LCM_BA+0x00)    ; Display Controller Control/Status Register 
REG_LCM_DEV_CTRL	EQU	(LCM_BA+0x04)    ; Display Output Device Control Register 
REG_LCM_MPU_CMD		EQU	(LCM_BA+0x08)    ; MPU-Interface LCD Write Command 
REG_LCM_INT_CS		EQU	(LCM_BA+0x0c)    ; Interrupt Control/Status Register 
REG_LCM_CRTC_SIZE	EQU	(LCM_BA+0x10)    ; CRTC Display Size Control Register 
REG_LCM_CRTC_DEND	EQU	(LCM_BA+0x14)    ; CRTC Display Enable End 
REG_LCM_CRTC_HR		EQU	(LCM_BA+0x18)    ; CRTC Internal Horizontal Retrace Control Register 
REG_LCM_CRTC_HSYNC	EQU	(LCM_BA+0x1C)    ; CRTC Horizontal Sync Control Register 
REG_LCM_CRTC_VR		EQU	(LCM_BA+0x20)    ; CRTC Internal Vertical Retrace Control Register 
REG_LCM_VA_BADDR0	EQU	(LCM_BA+0x24)    ; Video Stream Frame Buffer-0 Starting Address 
REG_LCM_VA_BADDR1	EQU	(LCM_BA+0x28)    ; Video Stream Frame Buffer-1 Starting Address 
REG_LCM_VA_FBCTRL	EQU	(LCM_BA+0x2C)    ; Video Stream Frame Buffer Control Register 
REG_LCM_VA_SCALE	EQU	(LCM_BA+0x30)    ; Video Stream Scaling Control Register 
REG_LCM_VA_TEST		EQU	(LCM_BA+0x34)    ; Test mode control register 
REG_LCM_OSD_WIN_S	EQU	(LCM_BA+0x40)    ; OSD Window Starting Coordinates 
REG_LCM_OSD_WIN_E	EQU	(LCM_BA+0x44)    ; OSD Window Ending Coordinates 
REG_LCM_OSD_BADDR	EQU	(LCM_BA+0x48)    ; OSD Stream Frame Buffer Starting Address 
REG_LCM_OSD_FBCTRL	EQU	(LCM_BA+0x4c)    ; OSD Stream Frame Buffer Control Register 
REG_LCM_OSD_OVERLAY	EQU	(LCM_BA+0x50)    ; OSD Overlay Control Register 
REG_LCM_OSD_CKEY	EQU	(LCM_BA+0x54)    ; OSD Overlay Color-Key Pattern Register 
REG_LCM_OSD_CMASK	EQU	(LCM_BA+0x58)    ; OSD Overlay Color-Key Mask Register 


;*********************************************************************************************************
;*                                                               
;* 11. Sensor DSP Control Registers  
;*
;*********************************************************************************************************
REG_DSPFunctionCR	EQU	(DSP_BA+0x00)    ; Switches of Sub-blocks 
REG_DSPInterruptCR	EQU	(DSP_BA+0x04)    ; Interrupt Mask and Status 

; Sensor interface control REG_isters 
REG_DSPInterfaceCR	EQU	(DSP_BA+0x08)    ; Sensor Interface 
REG_DSPHsyncCR1	EQU	(DSP_BA+0x0C)    ; Master mode Hsync position 
REG_DSPHsyncCR2	EQU	(DSP_BA+0x10)    ; Master mode Hsync width 
REG_DSPVsyncCR1	EQU	(DSP_BA+0x14)    ; Master mode Vsync position 
REG_DSPVsyncCR2	EQU	(DSP_BA+0x18)    ; Master mode Vsync width 
REG_DSPLineCounter	EQU	(DSP_BA+0x1C)    ; Current Line Position 

; Black level control REG_isters 
REG_DSPBlackLMode	EQU	(DSP_BA+0x20)    ; Black Level Clamping Mode 
REG_DSPBlackLCropCR1	EQU	(DSP_BA+0x24)    ; Black Level Clamping Parameters 
REG_DSPBlackLCropCR2	EQU	(DSP_BA+0x28)    ; Position of Black Level Window 
REG_DSPUserBlackLCR	EQU	(DSP_BA+0x2C)    ; User defined black level 
REG_DSPFrameDBL	EQU	(DSP_BA+0x30)    ; Frame base detected black level 

; Cropping window 
REG_DSPCropCR1	EQU	(DSP_BA+0x34)    ; Starting Address of Cropping Window 
REG_DSPCropCR2	EQU	(DSP_BA+0x38)    ; Width and Height of Cropping Window 

; digital programmable gain multiplier 
REG_DSPColorBalCR1	EQU	(DSP_BA+0x3C)    ; Gain factor for R and B 
REG_DSPColorBalCR2	EQU	(DSP_BA+0x40)    ; Gain factor for Gr and Gb 

; MCG and edge enhancement 
REG_DSPMCGCR	EQU	(DSP_BA+0x44)    ; MCG Threshold 
REG_DSPEdgeConfigCR	EQU	(DSP_BA+0x48)    ; Edge Configuration 

; Color correction matrix 
REG_DSPColorMatrixCR1	EQU	(DSP_BA+0x4C)    ; Color Correction Matrix -1 
REG_DSPColorMatrixCR2	EQU	(DSP_BA+0x50)    ; Color Correction Matrix -2 
REG_DSPColorMatrixCR3	EQU	(DSP_BA+0x54)    ; Color Correction Matrix -3 

; High saturation suppression 
REG_DSPHSSCR	EQU	(DSP_BA+0x60)    ; High Saturation Suppression 

; Gamma correction matrix 
REG_DSPGammaCR	EQU	(DSP_BA+0x64)    ; Gamma Correction Type Selection 
REG_DSPGammaTbl1	EQU	(DSP_BA+0x70)    ; Gamma Correction Table 0x70 ~ 0xAC 
REG_DSPGammaTbl2	EQU	(DSP_BA+0x74)
REG_DSPGammaTbl3	EQU	(DSP_BA+0x78)
REG_DSPGammaTbl4	EQU	(DSP_BA+0x7C)
REG_DSPGammaTbl5	EQU	(DSP_BA+0x80)
REG_DSPGammaTbl6	EQU	(DSP_BA+0x84)
REG_DSPGammaTbl7	EQU	(DSP_BA+0x88)
REG_DSPGammaTbl8	EQU	(DSP_BA+0x8C)
REG_DSPGammaTbl9	EQU	(DSP_BA+0x90)
REG_DSPGammaTbl10	EQU	(DSP_BA+0x94)
REG_DSPGammaTbl11	EQU	(DSP_BA+0x98)
REG_DSPGammaTbl12	EQU	(DSP_BA+0x9C)
REG_DSPGammaTbl13	EQU	(DSP_BA+0xA0)
REG_DSPGammaTbl14	EQU	(DSP_BA+0xA4)
REG_DSPGammaTbl15	EQU	(DSP_BA+0xA8)
REG_DSPGammaTbl16	EQU	(DSP_BA+0xAC)

; Auto Exposure 
REG_DSPAECCR1	EQU	(DSP_BA+0xB0)    ; Foreground Window 
REG_DSPAECAvg	EQU	(DSP_BA+0xB4)    ; AEC Statistics 

; Auto White Balance 
REG_DSPAWBWndCR	EQU	(DSP_BA+0xB8)    ; Window for AWB 
REG_DSPAWBWOCR1	EQU	(DSP_BA+0xBC)    ; White object definition 
REG_DSPAWBWOCR2	EQU	(DSP_BA+0xC0)    ; White object definition 
REG_DSPAWBCR	EQU	(DSP_BA+0xC4)    ; White object definition 
REG_DSPAWBWOCount	EQU	(DSP_BA+0xC8)    ; Total White Points 
REG_DSPAWBWOAvg	EQU	(DSP_BA+0xCC)    ; Average R, G, B of White Points 

; Auto Focus 
REG_DSPAFCR	EQU	(DSP_BA+0xD0)    ; AF Window Setting 
REG_DSPAFreport	EQU	(DSP_BA+0xD4)    ; Auto Focus Statistics 

; Histogram 
REG_DSPHistoCR	EQU	(DSP_BA+0xD8)    ; Histogram Configuration 
REG_DSPHistoReport1	EQU	(DSP_BA+0xDC)    ; Histogram Statistics -1 
REG_DSPHistoReport2	EQU	(DSP_BA+0xE0)    ; Histogram Statistics -2 
REG_DSPHistoReport3	EQU	(DSP_BA+0xE4)    ; Histogram Statistics -3 
REG_DSPHistoReport4	EQU	(DSP_BA+0xE8)    ; Histogram Statistics -4 
REG_DSPHistoReport5	EQU	(DSP_BA+0xEC)    ; Histogram Statistics -5 
REG_DSPHistoReport6	EQU	(DSP_BA+0xF0)    ; Histogram Statistics -6 

; Sub-Windows 
REG_DSPSubWndCR1	EQU	(DSP_BA+0xF4)    ; Sub-Windows Definitions 
REG_DSPSubWndCR2	EQU	(DSP_BA+0xF8)    ; Sub-Windows Definitions 
REG_DSPSubWndAvgY1	EQU	(DSP_BA+0x100)   ; Reported values 0x100 ~ 0x13C 
REG_DSPSubWndAvgY2	EQU	(DSP_BA+0x104)
REG_DSPSubWndAvgY3	EQU	(DSP_BA+0x108)
REG_DSPSubWndAvgY4	EQU	(DSP_BA+0x10C)
REG_DSPSubWndAvgR1	EQU	(DSP_BA+0x110)
REG_DSPSubWndAvgR2	EQU	(DSP_BA+0x114)
REG_DSPSubWndAvgR3	EQU	(DSP_BA+0x118)
REG_DSPSubWndAvgR4	EQU	(DSP_BA+0x11C)
REG_DSPSubWndAvgG1	EQU	(DSP_BA+0x120)
REG_DSPSubWndAvgG2	EQU	(DSP_BA+0x124)
REG_DSPSubWndAvgG3	EQU	(DSP_BA+0x128)
REG_DSPSubWndAvgG4	EQU	(DSP_BA+0x12C)
REG_DSPSubWndAvgB1	EQU	(DSP_BA+0x130)
REG_DSPSubWndAvgB2	EQU	(DSP_BA+0x134)
REG_DSPSubWndAvgB3	EQU	(DSP_BA+0x138)
REG_DSPSubWndAvgB4	EQU	(DSP_BA+0x13C)

; Bad uPixels 
REG_DSPBadPixelCR	EQU	(DSP_BA+0x140)    ; Bad-Pixel Positions 
REG_DSPBadPixelIndex	EQU	(DSP_BA+0x144)    ; Bad-Pixel Index 
 
 
;*********************************************************************************************************
;*                                                               
;* 12. Video Capture Engine Control Registers  
;*
;*********************************************************************************************************
REG_CAPEngine	EQU	(CAP_BA+0x00)   ; VPRE Engine Status 
REG_CAPFuncEnable	EQU	(CAP_BA+0x04)   ; Sub-Function Control 
REG_CAPColorMode	EQU	(CAP_BA+0x08)   ; VPRE Color Feature Register 
REG_CAPRotationMode	EQU	(CAP_BA+0x0C)   ; VPRE Rotation Mode Register 
REG_CAPInterfaceConf	EQU	(CAP_BA+0x10)   ; VPRE interface configuration 
REG_CAPCropWinStarPos	EQU	(CAP_BA+0x14)   ; VPRE Cropping Window Starting Position 
REG_CAPCropWinSize	EQU	(CAP_BA+0x18)   ; VPRE Cropping Window Size 
REG_CAPDownScaleFilter	EQU	(CAP_BA+0x1C)   ; Down Scale Factor Register 
REG_CAPPlaDownScale	EQU	(CAP_BA+0x20)   ; Planar Scale Factor Register 
REG_CAPlaRealSize	EQU	(CAP_BA+0x24)   ; Planar Real Image Size 
REG_CAPPacDownScale	EQU	(CAP_BA+0x28)   ; Packet Scale Factor Register 
REG_CAPPacRealSize	EQU	(CAP_BA+0x2C)   ; Packet Real Image Size 
REG_CAPPlaYFB0StaAddr	EQU	(CAP_BA+0x30)   ; Planar Y pipe Frame buffer 0 starting address 
REG_CAPPlaUFB0StaAddr	EQU	(CAP_BA+0x34)   ; Planar U pipe Frame buffer 0 starting address 
REG_CAPPlaVFB0StaAddr	EQU	(CAP_BA+0x38)   ; Planar V pipe Frame buffer 0 starting addres 
REG_CAPPacFB0StaAddr	EQU	(CAP_BA+0x3C)   ; Packet pipe Frame buffer 0 starting address 
REG_CAPPlaYFB1StaAddr	EQU	(CAP_BA+0x40)   ; Planar Y pipe Frame buffer 1 starting address 
REG_CAPPlaUFB1StaAddr	EQU	(CAP_BA+0x44)   ; Planar U pipe Frame buffer 1 starting address 
REG_CAPPlaVFB1StaAddr	EQU	(CAP_BA+0x48)   ; Planar V pipe Frame buffer 1 starting address 
REG_CAPPacFB1StaAddr	EQU	(CAP_BA+0x4C)   ; Packet pipe Frame buffer 1 starting address 
REG_CAPPlaMaskStaAddr	EQU	(CAP_BA+0x50)   ; Planar Overlap mask starting address 
REG_CAPPacMaskStaAddr	EQU	(CAP_BA+0x54)   ; Packet Overlap mask starting address 
REG_CAPPlaLineOffset	EQU	(CAP_BA+0x58)   ; Planar line offset value 
REG_CAPPacLineOffset	EQU	(CAP_BA+0x5C)   ; Packet line offset value 
REG_CAPFIFOThreshold	EQU	(CAP_BA+0x60)   ; FIFO threshold value 
REG_LensShadingCR1	EQU	(CAP_BA+0x64)   ; CFA type and shift value 
REG_LensShadingCR2	EQU	(CAP_BA+0x68)   ; The center position 
REG_LensShadingCR3	EQU	(CAP_BA+0x6C)   ; The coefficients for Y or R 
REG_LensShadingCR4	EQU	(CAP_BA+0x70)   ; The coefficients for U or G 
REG_LensShadingCR5	EQU	(CAP_BA+0x74)   ; The coefficients for V or B 
REG_DSPVideoQuaCR1	EQU	(CAP_BA+0x78)   ; Brightness and Contrast 
REG_DSPVideoQuaCR2	EQU	(CAP_BA+0x7C)   ; Hue and Saturation 

 
;*********************************************************************************************************
;*                                                               
;* 13. JPEG Codec Control Registers  
;*
;*********************************************************************************************************
JREGNUM	EQU	(0x1BC / 4 + 1)    ; The number of JPEG registers. 

REG_JMCR	EQU	(JPG_BA + 0x000)   ; JPEG Mode Control Register 
REG_JHEADER	EQU	(JPG_BA + 0x004)   ; JPEG Encode Header Control Register 
REG_JITCR	EQU	(JPG_BA + 0x008)   ; JPEG Image Type Control Register 
REG_JPRIQC	EQU	(JPG_BA + 0x010)   ; JPEG Encode Primary Q-Table Control Register 
REG_JTHBQC	EQU	(JPG_BA + 0x014)   ; JPEG Encode Thumbnail Q-Table Control Register 
REG_JPRIWH	EQU	(JPG_BA + 0x018)   ; JPEG Encode Primary Width/Height Register 
REG_JTHBWH	EQU	(JPG_BA + 0x01C)   ; JPEG Encode Thumbnail Width/Height Register 
REG_JPRST	EQU	(JPG_BA + 0x020)   ; JPEG Encode Primary Restart Interval Register 
REG_JTRST	EQU	(JPG_BA + 0x024)   ; JPEG Encode Thumbnail Restart Interval Register 
REG_JDECWH	EQU	(JPG_BA + 0x028)   ; JPEG Decode Image Width/Height Register 
REG_JINTCR	EQU	(JPG_BA + 0x02C)   ; JPEG Interrupt Control and Status Register 
REG_JDCOLC	EQU	(JPG_BA + 0x030)   ; JPEG Decode Image Color Control Register 
REG_JDCOLP	EQU	(JPG_BA + 0x034)   ; JPEG Decode Image Color Pattern Register 
REG_JTEST	EQU	(JPG_BA + 0x040)   ; JPEG Test Mode Control Register 
REG_JWINDEC0	EQU	(JPG_BA + 0x044)   ; JPEG Window Decode Mode Control Register 0 
REG_JWINDEC1	EQU	(JPG_BA + 0x048)   ; JPEG Window Decode Mode Control Register 1 
REG_JWINDEC2	EQU	(JPG_BA + 0x04C)   ; JPEG Window Decode Mode Control Register 2 
REG_JMACR	EQU	(JPG_BA + 0x050)   ; JPEG Memory Address Mode Control Registe 
REG_JPSCALU	EQU	(JPG_BA + 0x054)   ; JPEG Primary Scaling-Up Control Register 
REG_JPSCALD	EQU	(JPG_BA + 0x058)   ; JPEG Primary Scaling-Down Control Register 
REG_JTSCALD	EQU	(JPG_BA + 0x05C)   ; JPEG Thumbnail Scaling-Down Control Register 
REG_JDBCR	EQU	(JPG_BA + 0x060)   ; Dual-Buffer Control Register 
REG_JRESERVE	EQU	(JPG_BA + 0x070)   ; Primary Encode Bit-stream Reserved Size Register 
REG_JOFFSET	EQU	(JPG_BA + 0x074)   ; Address Offset Between Primary/Thumbnail Register 
REG_JFSTRIDE	EQU	(JPG_BA + 0x078)   ; JPEG Encode Bit-stream Frame Stride Register 
REG_JYADDR0	EQU	(JPG_BA + 0x07C)   ; Y Component Frame Buffer-0 Start Address Register 
REG_JUADDR0	EQU	(JPG_BA + 0x080)   ; U Component Frame Buffer-0 Start Address Register 
REG_JVADDR0	EQU	(JPG_BA + 0x084)   ; V Component Frame Buffer-0 Start Address Register 
REG_JYADDR1	EQU	(JPG_BA + 0x088)   ; Y Component Frame Buffer-1 Start Address Register 
REG_JUADDR1	EQU	(JPG_BA + 0x08C)   ; U Component Frame Buffer-1 Start Address Register 
REG_JVADDR1	EQU	(JPG_BA + 0x090)   ; V Component Frame Buffer-1 Start Address Register 
REG_JYSTRIDE	EQU	(JPG_BA + 0x094)   ; Y Component Frame Buffer Stride Register 
REG_JUSTRIDE	EQU	(JPG_BA + 0x098)   ; U Component Frame Buffer Stride Register 
REG_JVSTRIDE	EQU	(JPG_BA + 0x09C)   ; V Component Frame Buffer Stride Register 
REG_JIOADDR0	EQU	(JPG_BA + 0x0A0)   ; Bit-stream Frame Buffer-0 Start Address Register 
REG_JIOADDR1	EQU	(JPG_BA + 0x0A4)   ; Bit-stream Frame Buffer-1 Start Address Register 
REG_JPRI_SIZE	EQU	(JPG_BA + 0x0A8)   ; JPEG Encode Primary Bit-stream Size Register 
REG_JTHB_SIZE	EQU	(JPG_BA + 0x0AC)   ; JPEG Encode Thumbnail Bit-stream Size Register 
REG_JSRCWH	EQU	(JPG_BA + 0x0B0)   ; JPEG Encode Source Image Width/Height Register 

; QTable 0 ~ 2 
;   1. Needs to be programmed in byte unit.
;   2. While reading QTAB, it must read twice to get the correct data due to the QTAB is realized by SRAM. 
REG_JQTAB0	EQU	(JPG_BA + 0x100) ; The start address of JPEG Quantization-Table 0. 0x100 ~ 0x13C 
REG_JQTAB1	EQU	(JPG_BA + 0x140) ; The start address of JPEG Quantization-Table 1. 0x140 ~ 0x17C 
REG_JQTAB2	EQU	(JPG_BA + 0x180) ; The start address of JPEG Quantization-Table 2. 0x180 ~ 0x1BC 
 
 
;*********************************************************************************************************
;*                                                               
;* 14. MPEG-4 Video Codec Control Registers  
;*
;*********************************************************************************************************
; Encoder DMA map  
REG_MP4_INTERRUPT	EQU	(MP4_BA+0x038) 
REG_MP4_HOSTIF_DELAY	EQU	(MP4_BA+0x040) 

; Encoder DMA map  
REG_ENC_MBF_DMA_CSR	EQU	(MP4_BA+0x3A8)
REG_ENC_MBF_DMA_THSH	EQU	(MP4_BA+0x3B0)
REG_ENC_MBF_DMA_ADRS	EQU	(MP4_BA+0x3B8)

REG_ENC_MOF_DMA_CSR	EQU	(MP4_BA+0x3C8)
REG_ENC_MOF_DMA_THSH	EQU	(MP4_BA+0x3D0)
REG_ENC_MOF_DMA_ADRS	EQU	(MP4_BA+0x3D8)

REG_ENC_MIF_DMA_CSR	EQU	(MP4_BA+0x3E8)
REG_ENC_MIF_DMA_THSH	EQU	(MP4_BA+0x3F0)
REG_ENC_MIF_DMA_ADRS	EQU	(MP4_BA+0x3F8)

; Decoder DMA map  
REG_DEC_MBF_DMA_CSR	EQU	(MP4_BA+0x308)
REG_DEC_MBF_DMA_THSH	EQU	(MP4_BA+0x310)
REG_DEC_MBF_DMA_ADRS	EQU	(MP4_BA+0x318)

REG_DEC_MOF_DMA_CSR	EQU	(MP4_BA+0x328)
REG_DEC_MOF_DMA_THSH	EQU	(MP4_BA+0x330)
REG_DEC_MOF_DMA_ADRS	EQU	(MP4_BA+0x338)

REG_DEC_MIF_DMA_CSR	EQU	(MP4_BA+0x348)
REG_DEC_MIF_DMA_THSH	EQU	(MP4_BA+0x350)
REG_DEC_MIF_DMA_ADRS	EQU	(MP4_BA+0x358)

; Encoder register map 
REG_ENCODER_CONTROL	EQU	(MP4_BA+0x400)	
REG_ENCODER_CONTROL_WIDTH	EQU	(MP4_BA+0x404)
REG_ENCODER_STATUS	EQU	(MP4_BA+0x408)	
REG_ENCODER_CONTROL_HEIGHT	EQU	(MP4_BA+0x40c)	

REG_ENCODER_CONTROL_IRQ	EQU	(MP4_BA+0x410)
REG_ENCODER_CONTROL_VTI	EQU	(MP4_BA+0x414)
REG_ENCODER_COUNT	EQU	(MP4_BA+0x418)
REG_ENCODER_CONTROL_FUP	EQU	(MP4_BA+0x41C)

REG_ENCODER_RATE_OUT	EQU	(MP4_BA+0x420)
REG_ENCODER_CONFIG	EQU	(MP4_BA+0x424)
REG_ENCODER_IDENTITY	EQU	(MP4_BA+0x428)
REG_ENCODER_RATE_IN	EQU	(MP4_BA+0x42C)

REG_ENCODER_RATE_EX	EQU	(MP4_BA+0x430)
REG_ENCODER_CONTROL_IO	EQU	(MP4_BA+0x434)
REG_ENCODER_ADR_INP_Y	EQU	(MP4_BA+0x438)
REG_ENCODER_ADR_INP_U	EQU	(MP4_BA+0x43C)

REG_ENCODER_ADR_INP_V	EQU	(MP4_BA+0x440)
REG_ENCODER_ADR_REF	EQU	(MP4_BA+0x444)
REG_ENCODER_ADR_REC	EQU	(MP4_BA+0x448)	
REG_ENCODER_ADR_BIT	EQU	(MP4_BA+0x44C)

REG_ENCODER_RATE_MAD	EQU	(MP4_BA+0x450)
REG_ENCODER_CONTROL_FILTER	EQU	(MP4_BA+0x454)
REG_ENCODER_DISTORTION	EQU	(MP4_BA+0x458)
REG_ENCODER_QUANT_MATRIX	EQU	(MP4_BA+0x45C)

;  Decoder REG_ister map 
REG_DECODER_CONTROL	EQU	(MP4_BA+0x600)
REG_DECODER_CONTROL_WIDTH	EQU	(MP4_BA+0x604)
REG_DECODER_STATUS	EQU	(MP4_BA+0x608)
REG_DECODER_CONTROL_HEIGHT	EQU	(MP4_BA+0x60C)

REG_DECODER_CONTROL_IRQ	EQU	(MP4_BA+0x610)
REG_DECODER_CONTROL_IO	EQU	(MP4_BA+0x614)
REG_DECODER_STATUS_VTI	EQU	(MP4_BA+0x618)
REG_DECODER_ADR_BIT	EQU	(MP4_BA+0x61C)

REG_DECODER_COUNT	EQU	(MP4_BA+0x620)
REG_DECODER_ADR_REF_Y	EQU	(MP4_BA+0x624)
REG_DECODER_IDENTITY	EQU	(MP4_BA+0x628)
REG_DECODER_ADR_REF_U	EQU	(MP4_BA+0x62C)

REG_DECODER_ADR_REF_V	EQU	(MP4_BA+0x630)
REG_DECODER_ADR_OUT_Y	EQU	(MP4_BA+0x634)
REG_DECODER_ADR_OUT_U	EQU	(MP4_BA+0x638)
REG_DECODER_ADR_OUT_V	EQU	(MP4_BA+0x63C)

REG_DECODER_QUANT_MATRIX	EQU	(MP4_BA+0x640) 
 
 
;*********************************************************************************************************
;*                                                               
;* 15. 2-D Graphic Engine Control Registers  
;*
;*********************************************************************************************************
REG_GE_TRIGGER	EQU	(GE_BA+0x000)      ; trigger control 
REG_GE_SRC_START00_ADDR	EQU	(GE_BA+0x004)      ; source start (0,0) of X/Y mode 
REG_GE_FILTER_SRC_ADDR	EQU	(GE_BA+0x004)      ; shared with CR004 
REG_GE_FILTER0	EQU	(GE_BA+0x008)      ; 3x3 filter parameter 1 
REG_GE_FILTER1	EQU	(GE_BA+0x00C)      ; 3x3 filter parameter 2 
REG_GE_INTS	EQU	(GE_BA+0x010)      ; interrupt status 
REG_GE_PAT_START_ADDR	EQU	(GE_BA+0x014)      ; pattern start 
REG_GE_ERR_TERM_STEP_CONST	EQU	(GE_BA+0x018)      ; line parameter 1 
REG_GE_FILTER_THRESHOLD	EQU	(GE_BA+0x018)      ; 3x3 filter threshold for max clamping 
REG_GE_INIT_ERR_COUNT	EQU	(GE_BA+0x01C)      ; line parameter 2 
REG_GE_CONTROL	EQU	(GE_BA+0x020)      ; graphics engine control 
REG_GE_BACK_COLOR	EQU	(GE_BA+0x024)      ; background color 
REG_GE_FORE_COLOR	EQU	(GE_BA+0x028)      ; foreground color 
REG_GE_TRANSPARENCY_COLOR	EQU	(GE_BA+0x02C)      ; color key 
REG_GE_TRANSPARENCY_MASK	EQU	(GE_BA+0x030)      ; color key mask 
REG_GE_DEST_START00_ADDR	EQU	(GE_BA+0x034)      ; dest start (0,0) of X/Y mode 
REG_GE_FILTER_DEST_ADDR	EQU	(GE_BA+0x034)      ; shared with CR034 
REG_GE_PITCH	EQU	(GE_BA+0x038)      ; pitch of X/Y mode 
REG_GE_FILTER_PITCH	EQU	(GE_BA+0x038)      ; shared with CR038 
REG_GE_SRC_START_ADDR	EQU	(GE_BA+0x03C)      ; source start of BitBLT 
REG_GE_DEST_START_ADDR	EQU	(GE_BA+0x040)      ; dest start of BitBLT 
REG_GE_DIMENSION	EQU	(GE_BA+0x044)      ; width/height of X/Y BitBLT 
REG_GE_FILTER_DIMENSION	EQU	(GE_BA+0x044)      ; shared with CR044 
REG_GE_CLIP_TL	EQU	(GE_BA+0x048)      ; clip top-left 
REG_GE_CLIP_BR	EQU	(GE_BA+0x04C)      ; clip bottom-right 
REG_GE_PATA	EQU	(GE_BA+0x050)      ; mono pattern 1 
REG_GE_PATB	EQU	(GE_BA+0x054)      ; mono pattern 2 
REG_GE_WRITE_MASK	EQU	(GE_BA+0x058)      ; write plane mask 
REG_GE_MISC	EQU	(GE_BA+0x05C)      ; misc control 
REG_GE_DATA_PORT0	EQU	(GE_BA+0x060)      ; HostBLT data port 0x60 ~ 0x7C 
REG_GE_DATA_PORT1	EQU	(GE_BA+0x064)
REG_GE_DATA_PORT2	EQU	(GE_BA+0x068)
REG_GE_DATA_PORT3	EQU	(GE_BA+0x06C)
REG_GE_DATA_PORT4	EQU	(GE_BA+0x070)
REG_GE_DATA_PORT5	EQU	(GE_BA+0x074)
REG_GE_DATA_PORT6	EQU	(GE_BA+0x078)
REG_GE_DATA_PORT7	EQU	(GE_BA+0x07C) 
 
 
 
;*********************************************************************************************************
;*                                                               
;* 16. Video Processing Engine Control Registers  
;*
;*********************************************************************************************************
REG_VPEEngineTrigger	EQU	(VPE_BA+0x00)   ; Video Process Engine Trigger Control Register 
REG_VPEDdaFilterCC	EQU	(VPE_BA+0x04)   ; DDA 3X3 Filter Central Pixel Tap Coefficient 
REG_VPEDdaFilterLU	EQU	(VPE_BA+0x08)   ; DDA 3X3 Filter0, Other 4 Coefficients Around Central Pixel 
REG_VPEDdaFilterRB	EQU	(VPE_BA+0x0C)   ; DDA 3X3 Filter1, Other 4 Coefficients Around Central Pixel 
REG_VPEIntStatus	EQU	(VPE_BA+0x10)   ; Interrupt Status Register 
REG_VPEPacSrcStaAddr	EQU	(VPE_BA+0x14)   ; Packet Source Start Address 
REG_VPEUVDesSrcPitch	EQU	(VPE_BA+0x18)   ; Planar U/V Destination/Source Pitch Register 
REG_VPEReset	EQU	(VPE_BA+0x1C)   ; Video Process Engine Reset Control Register 
REG_VPECommand	EQU	(VPE_BA+0x20)   ; Video Process Engine Command Control Register 
REG_VPEYSrcStaAddr	EQU	(VPE_BA+0x24)   ; Video Process Engine Y Space Planar Start Address 
REG_VPEUSrcStaAddr	EQU	(VPE_BA+0x28)   ; Video Process Engine U Space Planar Start Address 
REG_VPEVSrcStaAddr	EQU	(VPE_BA+0x2C)   ; Video Process Engine V Space Planar Start Address 
REG_VPEDdaFactor	EQU	(VPE_BA+0x30)   ; DDA Vertical and Horizontal Scaling Factor N/M Register 
REG_VPEFcPacDesStaAddr	EQU	(VPE_BA+0x34)   ; Data Format Conversion Packet Destination Start Address 
REG_VPEDesSrcPtich	EQU	(VPE_BA+0x38)   ; Video Process Engine Destination/Source Pitch Register 
REG_VPEDdaPacDesStaAddr	EQU	(VPE_BA+0x3C)   ; DDA Down Scaling Packet Destination Start Address 
REG_VPERotRefPacDesStaAddr	EQU	(VPE_BA+0x40)   ; Rotate Reference Packet Destination Start Address 
REG_VPESrcHW	EQU	(VPE_BA+0x44)   ; Video Process Engine Width/Height Dimension Register 
REG_VPEDdaYDesStaAddr	EQU	(VPE_BA+0x48)   ; DDA Y Pipe Destination Start Address 
REG_VPEDdaUDesStaAddr	EQU	(VPE_BA+0x4C)   ; DDA U Pipe Destination Start Address 
REG_VPEDdaVDesStaAddr	EQU	(VPE_BA+0x50)   ; DDA V Pipe Destination Start Address 
REG_VPERotRefYDesStaAddr	EQU	(VPE_BA+0x54)   ; Rotate Reference Y Pipe Destination Start Address 
REG_VPERotRefUDesStaAddr	EQU	(VPE_BA+0x58)   ; Rotate Reference U Pipe Destination Start Address 
REG_VPERotRefVDesStaAddr	EQU	(VPE_BA+0x5C)   ; Rotate Reference V Pipe Destination Start Address  
 
 
;*********************************************************************************************************
;*                                                               
;* 17. Motion Estimation Engine Control Registers  
;*
;*********************************************************************************************************
REG_ME_CR	EQU	(ME_BA+0x000)    ; ME Control Register 
REG_ME_INTCR	EQU	(ME_BA+0x004)    ; ME Interrupt Control Register 
REG_ME_CADDR	EQU	(ME_BA+0x008)    ; Y Component of Current Block Start Address Register 
REG_ME_CSTRIDE	EQU	(ME_BA+0x00C)    ; Y Component of Current Frame Buffer Stride Register 
REG_ME_PADDR	EQU	(ME_BA+0x010)    ; Y Component of Previous Block Start Address Register 
REG_ME_PSTRIDE	EQU	(ME_BA+0x014)    ; Y Component of Previous Frame Buffer Stride Register 
REG_ME_MV	EQU	(ME_BA+0x018)    ; Motion Vector 
REG_ME_MAD	EQU	(ME_BA+0x01C)    ; Mean Absolute Difference of Motion Vector 
   
 
;*********************************************************************************************************
;*                                                               
;* 18. High Speed UART Control Registers  
;*
;*********************************************************************************************************
REG_HS_COM_RX	EQU	(HSU_BA+0x00)     ; (R) RX buffer 
REG_HS_COM_TX	EQU	(HSU_BA+0x00)     ; (W) TX buffer 
REG_HS_COM_IER	EQU	(HSU_BA+0x04)     ; Interrupt enable register 
REG_HS_COM_LSB	EQU	(HSU_BA+0x00)     ; Divisor latch LSB 
REG_HS_COM_MSB	EQU	(HSU_BA+0x04)     ; Divisor latch MSB 
REG_HS_COM_IIR	EQU	(HSU_BA+0x08)     ; (R) Interrupt ident. register 
REG_HS_COM_FCR	EQU	(HSU_BA+0x08)     ; (W) FIFO control register 
REG_HS_COM_LCR	EQU	(HSU_BA+0x0C)     ; Line control register 
REG_HS_COM_MCR	EQU	(HSU_BA+0x10)     ; Modem control register 
REG_HS_COM_LSR	EQU	(HSU_BA+0x14)     ; (R) Line status register 
REG_HS_COM_TOR	EQU	(HSU_BA+0x1C)     ; Time out register  
 
 
;*********************************************************************************************************
;*                                                               
;* 19. UART Control Registers  
;*
;*********************************************************************************************************
REG_COM_TX	EQU	(UART_BA+0x0)    ; (W) TX buffer 
REG_COM_RX	EQU	(UART_BA+0x0)    ; (R) RX buffer 
REG_COM_LSB	EQU	(UART_BA+0x0)    ; Divisor latch LSB 
REG_COM_MSB	EQU	(UART_BA+0x04)   ; Divisor latch MSB 
REG_COM_IER	EQU	(UART_BA+0x04)   ; Interrupt enable register 
REG_COM_IIR	EQU	(UART_BA+0x08)   ; (R) Interrupt ident. register 
REG_COM_FCR	EQU	(UART_BA+0x08)   ; (W) FIFO control register 
REG_COM_LCR	EQU	(UART_BA+0x0C)   ; Line control register 
REG_COM_MCR	EQU	(UART_BA+0x10)   ; Modem control register 
REG_COM_LSR	EQU	(UART_BA+0x14)   ; (R) Line status register 
REG_COM_MSR	EQU	(UART_BA+0x18)   ; (R) Modem status register 
REG_COM_TOR	EQU	(UART_BA+0x1C)   ; (R) Time out register 
 
 
;*********************************************************************************************************
;*                                                               
;* 20. Timer Control Registers  
;*
;*********************************************************************************************************
REG_TCR0	EQU	(TMR_BA+0x0)    ; Control Register 0 
REG_TCR1	EQU	(TMR_BA+0x04)   ; Control Register 1 
REG_TICR0	EQU	(TMR_BA+0x08)   ; Initial Control Register 0 
REG_TICR1	EQU	(TMR_BA+0x0C)   ; Initial Control Register 1 
REG_TDR0	EQU	(TMR_BA+0x10)   ; Data Register 0 
REG_TDR1	EQU	(TMR_BA+0x14)   ; Data Register 1 
REG_TISR	EQU	(TMR_BA+0x18)   ; Interrupt Status Register 
REG_WTCR	EQU	(TMR_BA+0x1C)   ; Watchdog Timer Control Register 
 
 
;*********************************************************************************************************
;*                                                               
;* 21. Interrupt Control Registers  
;*
;*********************************************************************************************************
REG_AIC_SCR1	EQU	(AIC_BA+0x04)    ; Source control register 1 
REG_AIC_SCR2	EQU	(AIC_BA+0x08)    ; Source control register 2 
REG_AIC_SCR3	EQU	(AIC_BA+0x0C)    ; Source control register 3 
REG_AIC_SCR4	EQU	(AIC_BA+0x10)    ; Source control register 4 
REG_AIC_SCR5	EQU	(AIC_BA+0x14)    ; Source control register 5 
REG_AIC_SCR6	EQU	(AIC_BA+0x18)    ; Source control register 6 
REG_AIC_SCR7	EQU	(AIC_BA+0x1C)    ; Source control register 7 
REG_AIC_SCR8	EQU	(AIC_BA+0x20)    ; Source control register 8 
REG_AIC_SCR9	EQU	(AIC_BA+0x24)    ; Source control register 9 
REG_AIC_SCR10	EQU	(AIC_BA+0x28)    ; Source control register 10 
REG_AIC_SCR11	EQU	(AIC_BA+0x2C)    ; Source control register 11 
REG_AIC_SCR12	EQU	(AIC_BA+0x30)    ; Source control register 12 
REG_AIC_SCR13	EQU	(AIC_BA+0x34)    ; Source control register 13 
REG_AIC_SCR14	EQU	(AIC_BA+0x38)    ; Source control register 14 
REG_AIC_SCR15	EQU	(AIC_BA+0x3C)    ; Source control register 15 
REG_AIC_SCR16	EQU	(AIC_BA+0x40)    ; Source control register 16 
REG_AIC_SCR17	EQU	(AIC_BA+0x44)    ; Source control register 17 
REG_AIC_SCR18	EQU	(AIC_BA+0x48)    ; Source control register 18 
REG_AIC_SCR19	EQU	(AIC_BA+0x4C)    ; Source control register 19 
REG_AIC_SCR20	EQU	(AIC_BA+0x50)    ; Source control register 20 
REG_AIC_SCR21	EQU	(AIC_BA+0x54)    ; Source control register 21 
REG_AIC_SCR22	EQU	(AIC_BA+0x58)    ; Source control register 22 
REG_AIC_SCR23	EQU	(AIC_BA+0x5C)    ; Source control register 23 
REG_AIC_SCR24	EQU	(AIC_BA+0x60)    ; Source control register 24 
REG_AIC_SCR25	EQU	(AIC_BA+0x64)    ; Source control register 25 
REG_AIC_SCR26	EQU	(AIC_BA+0x68)    ; Source control register 26 
REG_AIC_IRSR	EQU	(AIC_BA+0x100)   ; Interrupt raw status register 
REG_AIC_IASR	EQU	(AIC_BA+0x104)   ; Interrupt active status register 
REG_AIC_ISR	EQU	(AIC_BA+0x108)   ; Interrupt status register 
REG_AIC_IPER	EQU	(AIC_BA+0x10C)   ; Interrupt priority encoding register 
REG_AIC_ISNR	EQU	(AIC_BA+0x110)   ; Interrupt source number register 
REG_AIC_IMR	EQU	(AIC_BA+0x114)   ; Interrupt mask register 
REG_AIC_OISR	EQU	(AIC_BA+0x118)   ; Output interrupt status register 
REG_AIC_MECR	EQU	(AIC_BA+0x120)   ; Mask enable command register 
REG_AIC_MDCR	EQU	(AIC_BA+0x124)   ; Mask disable command register 
REG_AIC_SSCR	EQU	(AIC_BA+0x128)   ; Source set command register 
REG_AIC_SCCR	EQU	(AIC_BA+0x12C)   ; Source clear command register 
REG_AIC_EOSCR	EQU	(AIC_BA+0x130)   ; End of service command register 
REG_AIC_TEST	EQU	(AIC_BA+0x200)   ; ICE/Debug mode register 

  
;*********************************************************************************************************
;*                                                               
;* 22. Universal Serial Interface Control Registers  
;*
;*********************************************************************************************************
REG_USI_CNTRL	EQU	(USI_BA+0x0)     ; Control and Status Register 
REG_USI_DIVIDER	EQU	(USI_BA+0x04)    ; Clock Divider Register 
REG_USI_SSR	EQU	(USI_BA+0x08)    ; Slave Select Register 
REG_USI_Rx0	EQU	(USI_BA+0x10)    ; Data Receive Register 0 
REG_USI_Rx1	EQU	(USI_BA+0x14)    ; Data Receive Register 1 
REG_USI_Rx2	EQU	(USI_BA+0x18)    ; Data Receive Register 2 
REG_USI_Rx3	EQU	(USI_BA+0x1C)    ; Data Receive Register 3 
REG_USI_Tx0	EQU	(USI_BA+0x10)    ; Data Transmit Register 0 
REG_USI_Tx1	EQU	(USI_BA+0x14)    ; Data Transmit Register 1 
REG_USI_Tx2	EQU	(USI_BA+0x18)    ; Data Transmit Register 2 
REG_USI_Tx3	EQU	(USI_BA+0x1C)    ; Data Transmit Register 3 
 
 
;*********************************************************************************************************
;*                                                               
;* 23. External Bus Interface Control Registers  
;*
;*********************************************************************************************************
REG_EBISIZE0	EQU	(EBI_BA + 0x00)    ; EBI Bank 0 Size Control Register 
REG_EBISIZE1	EQU	(EBI_BA + 0x04)    ; EBI Bank 1 Size Control Register 
REG_EBISIZE2	EQU	(EBI_BA + 0x08)    ; EBI Bank 2 Size Control Register 
REG_EBISIZE3	EQU	(EBI_BA + 0x0C)    ; EBI Bank 3 Size Control Register 
REG_EBITIM0	EQU	(EBI_BA + 0x10)    ; EBI Bank0 Timing Control Register 
REG_EBITIM1	EQU	(EBI_BA + 0x14)    ; EBI Bank1 Timing Control Register  
REG_EBITIM2	EQU	(EBI_BA + 0x18)    ; EBI Bank2 Timing Control Register  
REG_EBITIM3	EQU	(EBI_BA + 0x1C)    ; EBI Bank3 Timing Control Register  

	END

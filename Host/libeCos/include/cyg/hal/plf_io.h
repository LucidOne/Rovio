#ifndef CYGONCE_HAL_PLF_IO_H
#define CYGONCE_HAL_PLF_IO_H
//=============================================================================
//
//      plf_io.h
//
//      Platform specific registers
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   clyu
// Contributors:clyu
// Date:        2003-11-25
// Purpose:     ARM/W90N740 W99702 platform specific registers
// Description: 
// Usage:       #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

/*
 * define W90N740 W99702 CPU master clock
 */
#include "pkgconf/system.h"


#define MHz		1000000

#ifdef CYG_HAL_CPUTYPE_W99702
#define fMCLK_MHz	(12 * MHz)
#else
#define fMCLK_MHz	(15 * MHz)
#endif

#define fMCLK		(fMCLK_MHz / MHz)
#define MCLK2		(fMCLK_MHz / 2)

#define pcibios_assign_all_busses()        1

/*
 * ASIC Address Definition
 */
#ifdef CYG_HAL_CPUTYPE_W99702
#define		SDRM_BA		0x3FFF0000
#define		GCR_BA		0x7FF00000
#define		CLK_BA		0x7FF00200
#define		GPIO_BA		0x7FF00300
#define		FSB_BA		0x7FF00400
#define		HIC_BA		0x7FF01000
#define		FMI_BA		0x7FF02000
#define		ADO_BA		0x7FF03000
#define		USB_BA		0x7FF04000
#define		LCM_BA		0x7FF05000
#define		DSP_BA		0x7FF08000
#define		CAP_BA		0x7FF09000
#define		JPG_BA		0x7FF0A000
#define		MP4_BA		0x7FF0B000
#define		GE_BA		0x7FF0C000
#define		VPE_BA		0x7FF0D000
#define		ME_BA		0x7FF0E000
#define		HSU_BA		0x7FFC0000
#define		UART_BA		0x7FFC1000
#define		TMR_BA		0x7FFC2000
#define		AIC_BA		0x7FFC3000
#define		USI_BA		0x7FFC4000
#define		EBI_BA		0xFDFF0000
#else //!W99702
#define Base_Addr	0xFFF00000
#define  _SYS_BASE_MAC  		0xFFF03000
#define  MAC_OFFSET  	0x0
#define  MAC_0_OFFSET  	MAC_OFFSET
#define  MAC_1_OFFSEST  (0x800+MAC_OFFSET)

#define w740_WriteReg(reg,val,which)      (*((unsigned int volatile*)(_SYS_BASE_MAC+(which)*0x800+reg))=(val))
#define w740_ReadReg(reg,which)           (*((unsigned int volatile*)(_SYS_BASE_MAC+reg+(which)*0x800)))

#define w740_WriteCam1(which,x,lsw,msw) \
		w740_WriteReg(CAM1L+(x)*CAM_ENTRY_SIZE,lsw,which);\
		w740_WriteReg(CAM1M+(x)*CAM_ENTRY_SIZE,msw,which);
#endif //!W99702

#define VPint	*(unsigned int volatile*)
#define VPshort	*(unsigned short volatile*)
#define VPchar	*(unsigned char volatile*)

#ifndef CSR_WRITE
#define CSR_WRITE(addr,data)	(VPint(addr) = (data))
#endif

#ifndef CSR_READ
#define CSR_READ(addr)	(VPint(addr))
#endif

#ifndef CAM_Reg
#define CAM_Reg(x)		(VPint(CAMBASE+(x*0x4)))
#endif

#ifndef CYG_HAL_CPUTYPE_W99702
/* ************************ */
/* System Manager Registers */
/* ************************ */
#define PDID		(Base_Addr+0x00000)
#define ARBCON		(Base_Addr+0x00004)
#define PLLCON		(Base_Addr+0x00008)
#define CLKSEL		(Base_Addr+0x0000c)

/*****************************/
/* Cache Control Register Map*/
/*****************************/
#define CAHCNF		(Base_Addr+0x02000)
#define CAHCON		(Base_Addr+0x02004)
#define CAHADR		(Base_Addr+0x02008)

#define NON_CANCHABLE 0x80000000
//#define NONCACHE
/*****************************/
/* EBI Control Registers Map */
/*****************************/
#define EBICON		(Base_Addr+0x01000)
#define ROMCON0		(Base_Addr+0x01004)
#define DRAMCON0	(Base_Addr+0x01008)
#define DRAMCON1	(Base_Addr+0x0100c)
#define SDTIME0		(Base_Addr+0x01010)
#define SDTIME1		(Base_Addr+0x01014)

/********************************/
/* GPIO Controller Registers Map*/
/********************************/
#define GPIO_CFG		(Base_Addr+0x83000)
#define GPIO_DIR		(Base_Addr+0x83004)
#define GPIO_DATAOUT	(Base_Addr+0x83008)
#define GPIO_DATAIN		(Base_Addr+0x8300c)
#define GPIO_DEBNCE		(Base_Addr+0x83010)


/* *********************** */
/* Ethernet BDMA Registers */
/* *********************** */
#define BDMATXCON	(Base_Addr+0x9000)
#define BDMARXCON	(Base_Addr+0x9004)
#define BDMATXPTR	(Base_Addr+0x9008)
#define BDMARXPTR	(Base_Addr+0x900C)
#define BDMARXLSZ	(Base_Addr+0x9010)
#define BDMASTAT	(Base_Addr+0x9014)
#define CAMBASE		(Base_Addr+0x9100)


//DMA Registers
#define TXDLSA			(MAC_OFFSET+0x9c) //Transmit Descriptor Link List Start Address Regiser 
#define RXDLSA			(MAC_OFFSET+0xa0) //Receive Descriptor LInk List Start Addresss Register
#define DMARFC			(MAC_OFFSET+0xa4) //DMA Receive Frame Control Register
#define TSDR			(MAC_OFFSET+0xa8) //Transmit Start Demand Register
#define RSDR			(MAC_OFFSET+0xac) //Recevie Start Demand Register
#define FIFOTHD			(MAC_OFFSET+0xb0) //FIFO Threshold Adjustment Register

/*
 * CAM		0x9100 ~ 0x917C
 * BDMATXBUF	0x9200 ~ 0x92FC
 * BDMARXBUF	0x9800 ~ 0x99FC
 */

/* ********************** */
/* Ethernet MAC Registers */
/* ********************** */
#define MACON		(MAC_OFFSET+0xA000)
#define CAMCON		(MAC_OFFSET+0xA004)
#define MACTXCON	(MAC_OFFSET+0xA008)
#define MACTXSTAT	(MAC_OFFSET+0xA00C)
#define MACRXCON	(MAC_OFFSET+0xA010)
#define MACRXSTAT	(MAC_OFFSET+0xA014)
#define STADATA		(MAC_OFFSET+0xA018)
#define STACON		(MAC_OFFSET+0xA01C)
//#define CAMEN		(MAC_OFFSET+0xA028)
#define EMISSCNT	(MAC_OFFSET+0xA03C)
#define EPZCNT		(MAC_OFFSET+0xA040)
#define ERMPZCNT	(MAC_OFFSET+0xA044)
#define EXTSTAT		(MAC_OFFSET+0x9040)




#define  CAMCMR			(MAC_OFFSET)    //CAM Command Regiser
#define  CAMEN			(MAC_OFFSET+0x4)//CAM ennable regiser
#define  CAM1M			(MAC_OFFSET+0x8)//CAM1 Most significant Word register
#define  CAM1L			(MAC_OFFSET+0xc)//CAM1 Least Significant Word Register
#define  CAM_ENTRY_SIZE		0x8     //CAM  entry size
#define  CAM_ENTRIES		0x16    //CAM  entries

//mac regiseters
#define MIEN			(MAC_OFFSET+0x88) //MAC Interrupt Enable Register
#define MCMDR			(MAC_OFFSET+0x8c) //MAC Command Regiser
#define MIID			(MAC_OFFSET+0x90) //MII Management Data Register
#define MIIDA			(MAC_OFFSET+0x94) //MII Management Data Control and Address Register
#define MPCNT			(MAC_OFFSET+0x98) //Missed Packet Counter Register


//EMC Status Register
#define MISTA			(MAC_OFFSET+0xb4) //MAC Interrupter Status Register
#define MGSTA			(MAC_OFFSET+0xb8) //MAC General Status Register
#define MRPC			(MAC_OFFSET+0xbc)  //MAC Receive Pauese counter register
#define MRPCC			(MAC_OFFSET+0xc0) //MAC Receive Pauese Current Count Regiser
#define MREPC			(MAC_OFFSET+0xc4)  //MAC Remote pause count retister
//DMA Registers
#define DMARFS			(MAC_OFFSET+0xc8) //DMA Receive Frame Status Register
#define CTXDSA			(MAC_OFFSET+0xcc) //Current Transmit Descriptor Start Addresss Register
#define CTXBSA			(MAC_OFFSET+0xd0) //Current Transmit Buffer Start Address Regiser
#define CRXDSA			(MAC_OFFSET+0xd4) //Current Receive Descriptor start Address regiser
#define CRXBSA			(MAC_OFFSET+0xd8) //Current Receive Buffer Start Address Regiser
//Debug Mode Receive Finite State Machine Registers
#define RXFSM			(MAC_OFFSET+0x200)
#define TXFSM			(MAC_OFFSET+0x204)
#define FSM0			(MAC_OFFSET+0x208)
#define FSM1			(MAC_OFFSET+0x20c)



/* ************************ */
/* HDLC Channel A Registers */
/* ************************ */

/* ************************ */
/* HDLC Channel B Registers */
/* ************************ */

/* ******************* */
/* I/O Ports Registers */
/* ******************* */
#define IOPMOD		(Base_Addr+0x5000)
#define IOPCON		(Base_Addr+0x5004)
#define IOPDATA		(Base_Addr+0x5008)


/* ****************************** */
/* AIC Registers Map			  */
/* ****************************** */
#define AIC_SCR1	(Base_Addr+0x82004)
#define AIC_SCR2	(Base_Addr+0x82008)
#define AIC_SCR3	(Base_Addr+0x8200c)
#define AIC_SCR4	(Base_Addr+0x82010)
#define AIC_SCR5	(Base_Addr+0x82014)
#define AIC_SCR6	(Base_Addr+0x82018)
#define AIC_SCR7	(Base_Addr+0x8201c)
#define AIC_SCR8	(Base_Addr+0x82010)
#define AIC_SCR9	(Base_Addr+0x82024)
#define AIC_SCR10	(Base_Addr+0x82028)
#define AIC_SCR11	(Base_Addr+0x8202c)
#define AIC_SCR12	(Base_Addr+0x82030)
#define AIC_SCR13	(Base_Addr+0x82034)
#define AIC_SCR14	(Base_Addr+0x82038)
#define AIC_SCR15	(Base_Addr+0x8203c)
#define AIC_SCR16	(Base_Addr+0x82040)
#define AIC_SCR17	(Base_Addr+0x82044)
#define AIC_SCR18	(Base_Addr+0x82048)

#define AIC_IRSR	(Base_Addr+0x82100)
#define AIC_IASR	(Base_Addr+0x82104)
#define AIC_ISR		(Base_Addr+0x82108)
#define AIC_IPER	(Base_Addr+0x8210c)
#define AIC_ISNR	(Base_Addr+0x82110)
#define AIC_IMR		(Base_Addr+0x82114)
#define AIC_OISR	(Base_Addr+0x82118)
#define AIC_MECR	(Base_Addr+0x82120)
#define AIC_MDCR	(Base_Addr+0x82124)
#define AIC_SSCR	(Base_Addr+0x82128)
#define AIC_SCCR	(Base_Addr+0x8212c)
#define AIC_EOSCR	(Base_Addr+0x82130)
#define AIC_TEST	(Base_Addr+0x82200)


#define IntScr(index,value)		(VPint(Base_Addr+0x82000+4*index)=value)
#define IntPend		(VPint(AIC_EOSCR))
#define IntMask		(VPint(AIC_MDCR))
#define IntUnMask	(VPint(AIC_MECR))

#define INT_ENABLE(n)		IntUnMask = (1<<(n))
#define INT_DISABLE(n)		IntMask = (1<<(n))
//#define CLEAR_PEND_INT(n)	IntPend = (0)
//#define SET_PEND_INT(n)		IntPndTst |= (1<<(n))

/* ***************** */
/* I2C Bus Registers */
/* ***************** */

/* ************** */
/* GDMA Registers */
/* ************** */

/* ************** */
/* UART Registers */
/* ************** */

#define DEBUG_CONSOLE	(0)

#define COM_TX			(Base_Addr+0x80000)
#define COM_RX			(Base_Addr+0x80000)
#define COM_DLL 		(Base_Addr+0x80000)
#define COM_DLM 		(Base_Addr+0x80004)
#define COM_IER 		(Base_Addr+0x80004)
#define COM_IIR 		(Base_Addr+0x80008)
#define COM_FCR 		(Base_Addr+0x80008)
#define COM_LCR 		(Base_Addr+0x8000c)
#define COM_MCR 		(Base_Addr+0x80010)
#define COM_LSR 		(Base_Addr+0x80014)
#define COM_MSR 		(Base_Addr+0x80018)
#define COM_TOR 		(Base_Addr+0x8001c)


#else //W99702

#define NON_CANCHABLE 0x10000000

/* ************************ */
/* System Manager Control Registers */
/* ************************ */

#define PDID		(GCR_BA+0x00000)
#define SYS_CFG		(GCR_BA+0x00004)
#define CPUCR		(GCR_BA+0x00008)
#define MISCR		(GCR_BA+0x0000C)
#define PADC0		(GCR_BA+0x00020)
#define PADC1		(GCR_BA+0x00024)
#define PADC2		(GCR_BA+0x00028)
#define AHB_PRI		(GCR_BA+0x00100)
#define BBLENG0		(GCR_BA+0x00104)
#define BBLENG1		(GCR_BA+0x00108)

/* ************************ */
/* Clock Controller Registers */
/* ************************ */

#define PWRCON		(CLK_BA + 0x00)
#define CLKCON		(CLK_BA + 0x04)
#define APLLCON		(CLK_BA + 0x08)
#define UPLLCON		(CLK_BA + 0x0C)
#define CLKSEL		(CLK_BA + 0x10)
#define CLKDIV0		(CLK_BA + 0x14)
#define CLKDIV1		(CLK_BA + 0x18)
#define PLLWCR		(CLK_BA + 0x1C)
#define FLCCR		(CLK_BA + 0x20)
#define PWMCR		(CLK_BA + 0x24)

/* ************************ */
/* GPIO Controller Registers */
/* ************************ */

#define GPIOOE	(GPIO_BA+0x00)
#define GPIOD	(GPIO_BA+0x04)
#define GPIOS	(GPIO_BA+0x08)
#define GPIOIE	(GPIO_BA+0x0C)
#define GPIOIS	(GPIO_BA+0x10)
#define GPIOPE	(GPIO_BA+0x14)
#define GPIOAOE	(GPIO_BA+0x20)
#define GPIOAD	(GPIO_BA+0x24)
#define GPIOAS	(GPIO_BA+0x28)
#define GPIOAPE	(GPIO_BA+0x2C)
#define GPIOBOE	(GPIO_BA+0x30)
#define GPIOBD	(GPIO_BA+0x34)
#define GPIOBS	(GPIO_BA+0x38)
#define GPIOBPE	(GPIO_BA+0x3C)
#define GPIOSOE	(GPIO_BA+0x40)
#define GPIOSD	(GPIO_BA+0x44)
#define GPIOSS	(GPIO_BA+0x48)
#define GPIOSPE	(GPIO_BA+0x4C)
#define DEBNCE_CTRL	(GPIO_BA+0x18)

/* ************************ */
/* Fast Serial Bus interface Controller Registers */
/* ************************ */

#define FSBTRIG	(FSB_BA + 0x00)
#define I2CSTSR	(FSB_BA + 0x04)
//#define FSBSTSR	(I2C_BA + 0x08)
#define SCKSDA0	(FSB_BA + 0x0C)
#define SCKSDA1	(FSB_BA + 0x10)
#define FSBSW	(FSB_BA + 0x14)

/* ************************ */
/* EBI Control Register */
/* ************************ */
#define EBISIZE0	(EBI_BA + 0x000)
#define EBISIZE1	(EBI_BA + 0x004)
#define EBISIZE2	(EBI_BA + 0x008)
#define EBISIZE3	(EBI_BA + 0x00C)

#define EBITIM0		(EBI_BA + 0x10)
#define EBITIM1		(EBI_BA + 0x14)
#define EBITIM2		(EBI_BA + 0x18)
#define EBITIM3		(EBI_BA + 0x1C)

/* ************************ */
/* SDRAM Control Registers */
/* ************************ */

#define SDICEN	(SDRM_BA + 0x000)
#define SDICON	(SDRM_BA + 0x004)
#define SDCONF0	(SDRM_BA + 0x008)
#define SDCONF1	(SDRM_BA + 0x00C)
#define SDTIME0	(SDRM_BA + 0x010)
#define SDTIME1	(SDRM_BA + 0x014)
#define ARBCON	(SDRM_BA + 0x018)
#define TESTCR	(SDRM_BA + 0x020)
#define TSTATUS	(SDRM_BA + 0x024)
#define TFDATA	(SDRM_BA + 0x028)
#define TFADDR	(SDRM_BA + 0x02C)
#define CKSKEW	(SDRM_BA + 0xF00)


/* ****************************** */
/* AIC Registers Map			  */
/* ****************************** */
#define AIC_SCR1	(AIC_BA+0x004)
#define AIC_SCR2	(AIC_BA+0x008)
#define AIC_SCR3	(AIC_BA+0x00c)
#define AIC_SCR4	(AIC_BA+0x010)
#define AIC_SCR5	(AIC_BA+0x014)
#define AIC_SCR6	(AIC_BA+0x018)
#define AIC_SCR7	(AIC_BA+0x01c)
#define AIC_SCR8	(AIC_BA+0x010)
#define AIC_SCR9	(AIC_BA+0x024)
#define AIC_SCR10	(AIC_BA+0x028)
#define AIC_SCR11	(AIC_BA+0x02c)
#define AIC_SCR12	(AIC_BA+0x030)
#define AIC_SCR13	(AIC_BA+0x034)
#define AIC_SCR14	(AIC_BA+0x038)
#define AIC_SCR15	(AIC_BA+0x03c)
#define AIC_SCR16	(AIC_BA+0x040)
#define AIC_SCR17	(AIC_BA+0x044)
#define AIC_SCR18	(AIC_BA+0x048)
#define AIC_SCR19	(AIC_BA+0x04C)
#define AIC_SCR20	(AIC_BA+0x050)
#define AIC_SCR21	(AIC_BA+0x054)
#define AIC_SCR22	(AIC_BA+0x058)
#define AIC_SCR23	(AIC_BA+0x05C)
#define AIC_SCR24	(AIC_BA+0x060)
#define AIC_SCR25	(AIC_BA+0x064)
#define AIC_SCR26	(AIC_BA+0x068)

#define AIC_IRSR	(AIC_BA+0x100)
#define AIC_IASR	(AIC_BA+0x104)
#define AIC_ISR		(AIC_BA+0x108)
#define AIC_IPER	(AIC_BA+0x10c)
#define AIC_ISNR	(AIC_BA+0x110)
#define AIC_IMR		(AIC_BA+0x114)
#define AIC_OISR	(AIC_BA+0x118)
#define AIC_MECR	(AIC_BA+0x120)
#define AIC_MDCR	(AIC_BA+0x124)
#define AIC_SSCR	(AIC_BA+0x128)
#define AIC_SCCR	(AIC_BA+0x12c)
#define AIC_EOSCR	(AIC_BA+0x130)
#define AIC_TEST	(AIC_BA+0x200)


#define IntScr(index,value)		(VPint(AIC_BA+4*index)=value)
#define IntPend		(VPint(AIC_EOSCR))
#define IntMask		(VPint(AIC_MDCR))
#define IntUnMask	(VPint(AIC_MECR))

#define INT_ENABLE(n)		IntUnMask = (1<<(n))
#define INT_DISABLE(n)		IntMask = (1<<(n))
//#define CLEAR_PEND_INT(n)	IntPend = (0)
//#define SET_PEND_INT(n)		IntPndTst |= (1<<(n))

/* ************** */
/* UART Registers */
/* ************** */

#define DEBUG_CONSOLE	(0)

#define COM_TX			(0x00)
#define COM_RX			(0x00)
#define COM_DLL 		(0x00)
#define COM_DLM 		(0x04)
#define COM_IER 		(0x04)
#define COM_IIR 		(0x08)
#define COM_FCR 		(0x08)
#define COM_LCR 		(0x0c)
#define COM_MCR 		(0x10)
#define COM_LSR 		(0x14)
#define COM_MSR 		(0x18)
#define COM_TOR 		(0x1c)

/* ************** */
/* High Speed UART Control Registers */
/* ************** */

#define RBR		(HSU_BA + 0x00)
#define THR		(HSU_BA + 0x04)
#define IER		(HSU_BA + 0x08)
#define DLL		(HSU_BA + 0x0C)
#define DLM		(HSU_BA + 0x10)
#define IIR		(HSU_BA + 0x14)
#define FCR		(HSU_BA + 0x18)
#define LCR		(HSU_BA + 0x1C)
#define LSR		(HSU_BA + 0x20)
#define TOR		(HSU_BA + 0x24)


#endif //W99702

#define UART_LSR_OE	0x02		// Overrun error
#define UART_LSR_PE	0x04		// Parity error
#define UART_LSR_FE	0x08		// Frame error
#define UART_LSR_BI	0x10		// Break detect
//#define UART_LSR_DTR	0x10		// Data terminal ready
#define UART_LSR_DR 0x01
#define UART_LSR_THRE 0x20
#define UART_IIR_NIP	0x01		// Transmit buffer register empty
#define UART_IIR_THRE	0x02		// Transmit buffer register empty
#define UART_IIR_DR		0x04		// Receive data ready
#define UART_IIR_TOUT	0x0C		// Transmit buffer register empty
#define UART_LSR_TEMT	0x40		// Transmit complete

#define UART_LCR_WLEN5	0x00
#define UART_LCR_WLEN6	0x01
#define UART_LCR_WLEN7	0x02
#define UART_LCR_WLEN8	0x03
#define UART_LCR_PARITY	0x08
#define UART_LCR_NPAR	0x00
#define UART_LCR_OPAR	0x00
#define UART_LCR_EPAR	0x10
#define UART_LCR_SPAR	0x20
#define UART_LCR_SBC	0x40
#define UART_LCR_NSB	0x00
#define UART_LCR_NSB1_5	0x04

#define UART_GCR_RX_INT	0x01
#define UART_GCR_TX_INT	0x08
#define UART_GCR_RX_STAT_INT	0x04

#define UART_IER_MSI
#define UART_IER_RLSI	0x04
#define UART_IER_THRI	0x02
#define UART_IER_RDI	0x01
	
#define UART_MSR		0
#define UART_MSR_DCD		0
#define UART_MSR_RI		0
#define UART_MSR_DSR		0
#define UART_MSR_CTS		0
#define UART_MSR_DDCD		0
#define UART_MSR_TERI		0
#define UART_MSR_DDSR		0
#define UART_MSR_DCTS		0
#define UART_MSR_ANY_DELTA	0


#ifdef CYG_HAL_CPUTYPE_W99702
#define UART_BASE0	0x7FFC1000
#define UART_BASE1	0x7FFC0000
#else
#define UART_BASE0	COM_TX
#endif

#define BASE_BAUD0	57600
#define BASE_BAUD1	9600

#if DEBUG_CONSOLE == 0
	#define DEBUG_TX_BUFF_BASE	COM_TX
	#define DEBUG_RX_BUFF_BASE	COM_RX
	#define DEBUG_UARTLCON_BASE	COM_LCR
	#define DEBUG_UARTCONT_BASE	COM_LCR                  
	#define DEBUG_UARTBRD_BASE	COM_DLL
	#define DEBUG_CHK_STAT_BASE	COM_IIR
#endif

#define DEBUG_ULCR_REG_VAL	(0x3)
#define DEBUG_ULCR_REG_VAL	(0x3)
#define DEBUG_UDLL_REG_VAL	(0x6)
#define DEBUG_RX_CHECK_BIT	(0X20)
#define DEBUG_TX_CAN_CHECK_BIT	(0X40)
#define DEBUG_TX_DONE_CHECK_BIT	(0X80)


#ifndef CYG_HAL_CPUTYPE_W99702
/* **************** */
/* Timers Registers */
/* **************** */
#define TCR0		(Base_Addr+0x81000)
#define TCR1		(Base_Addr+0x81004)
#define TICR0		(Base_Addr+0x81008)
#define TICR1		(Base_Addr+0x8100c)
#define TDR0		(Base_Addr+0x81010)
#define TDR1		(Base_Addr+0x81014)
#define TISR		(Base_Addr+0x81018)
#define WTCR		(Base_Addr+0x8101c)

#else
/* **************** */
/* Timers Registers */
/* **************** */
#define TCR0		(TMR_BA+0x00)
#define TCR1		(TMR_BA+0x04)
#define TICR0		(TMR_BA+0x08)
#define TICR1		(TMR_BA+0x0c)
#define TDR0		(TMR_BA+0x10)
#define TDR1		(TMR_BA+0x14)
#define TISR		(TMR_BA+0x18)
#define WTCR		(TMR_BA+0x1c)
#endif
/* **************** */
/* TCR Registers */
/* **************** */

#define	CE	0x40000000

/*******************/
/* SYSCFG Register */
/*******************/

#define SYS_INIT_BASE	EXTDBWTH
#define rSYSCFG		(0x87FFFF90)	/* disable Cache/Write buffer */

/**********************************/
/* System Memory Control Register */
/**********************************/
#define DSR0		(2<<0)	/* ROM Bank0 */
#define DSR1		(0<<2)	/* 0: Disable, 1: Byte, 2: Half-Word, 3: Word */
#define DSR2		(0<<4)
#define DSR3		(0<<6)
#define DSR4		(0<<8)
#define DSR5		(0<<10)
#define DSD0		(2<<12) /* RAM Bank0 */
#define DSD1		(0<<14)
#define DSD2		(0<<16)
#define DSD3		(0<<18)
#define DSX0		(0<<20)	/* EXTIO0 */
#define DSX1		(0<<22)
#define DSX2		(0<<24)
#define DSX3		(0<<26)

#define rEXTDBWTH	(DSR0|DSR1|DSR2|DSR3|DSR4|DSR5 | DSD0|DSD1|DSD2|DSD3 | DSX0|DSX1|DSX2|DSX3)

/****************************************/
/* ROMCON0: ROM Bank 0 Control Register */
/****************************************/
#define PMC0		(0x0<<0)	/*00: Normal ROM   01: 4 word page*/
					/*10: 8 word page  11:16 word page*/
#define tPA0		(0x0<<2)	/*00: 5 cycles     01: 2 cycles*/
					/*10: 3 cycles     11: 4 cycles*/
#define tACC0		(0x6<<4)	/*000: Disable bank 001: 2 cycles*/
					/*010: 3 cycles     011: 4 cycles*/
					/*110: 7 cycles     111: Reserved*/
#define ROM_BASE0_R	((0x00000000>>16)<<10)
#define ROM_NEXT0_R	((0x00200000>>16)<<20)
#define ROM_BASE0_B	((0x01000000>>16)<<10)
#define ROM_NEXT0_B	((0x01200000>>16)<<20)
#define rROMCON0_R	(ROM_NEXT0_R|ROM_BASE0_R|tACC0|tPA0|PMC0)
#define rROMCON0_B	(ROM_NEXT0_B|ROM_BASE0_B|tACC0|tPA0|PMC0)

#define rROMCON1	0x0
#define rROMCON2	0x0
#define rROMCON3	0x0
#define rROMCON4	0x0
#define rROMCON5	0x0


/********************************************/
/* SDRAMCON0: SDRAM Bank 0 Control Register */
/********************************************/
#define StRC0		(0x1<<7)
#define StRP0		(0x3<<8)
#define SDRAM_BASE0_R	((0x01000000>>16)<<10)
#define SDRAM_NEXT0_R	((0x01800000>>16)<<20)
#define SDRAM_BASE0_B	((0x00000000>>16)<<10)
#define SDRAM_NEXT0_B	((0x00800000>>16)<<20)
#define SCAN0		(0x0<<30)
#define rSDRAMCON0_R	(SCAN0|SDRAM_NEXT0_R|SDRAM_BASE0_R|StRP0|StRC0)
#define rSDRAMCON0_B	(SCAN0|SDRAM_NEXT0_B|SDRAM_BASE0_B|StRP0|StRC0)

#define rSDRAMCON1	0x0
#define rSDRAMCON2	0x0
#define rSDRAMCON3	0x0

/************************************************/
/* DRAM Refresh & External I/O Control Register */
/************************************************/
#define ExtIOBase	(0x360<<0)
#define VSF		(0x1<<15)
#define REN		(0x1<<16)
#define tCHR		(0x0<<17)
#define tCSR		(0x0<<20)
#define RefCountValue	((2048+1-(16*fMCLK))<<21)
#define rREFEXTCON	(RefCountValue|tCSR|tCHR|REN|VSF|ExtIOBase)

/********/
/* Misc */
/********/

#define TMOD_TIMER0_VAL	0x3	/* Timer0  TOGGLE, and Run */
#define TAG_BASE	0x11000000

#define HARD_RESET_NOW()

/*PCI*/
#define PCIBIOS_MIN_IO		0x6000
#define PCIBIOS_MIN_MEM 	0x01000000


//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_HAL_PLF_IO_H

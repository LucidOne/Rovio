//==========================================================================
//
//      w99702_misc.c
//
//      HAL misc board support code for Winbond w99702
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    clyu
// Contributors: clyu
// Date:         2004-04-30
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include "pkgconf/hal.h"

#include "cyg/infra/cyg_type.h"         // base types
#include "cyg/infra/cyg_trac.h"         // tracing macros
#include "cyg/infra/cyg_ass.h"          // assertion macros
#include "cyg/infra/diag.h"             // diag_printf()

#include "cyg/hal/hal_io.h"             // IO macros
#include "cyg/hal/hal_arch.h"           // Register state info
#include "cyg/hal/hal_diag.h"
#include "cyg/hal/hal_cache.h"
#include "cyg/hal/hal_intr.h"           // necessary?
#include "cyg/hal/hal_if.h"             // calling interface
#include "cyg/hal/hal_misc.h"           // helper functions
#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
#include "cyg/hal/drv_api.h"            // HAL ISR support
#endif

#include "w99702.h"
#include "cyg/hal/plf_io.h"
#include "pkgconf/mlt_arm_w99702_ram.h"
//#define SW
//======================================================================
// Use Timer0 for kernel clock

static cyg_uint32 _period;

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
static cyg_interrupt abort_interrupt;
static cyg_handle_t  abort_interrupt_handle;

// This ISR is called only for the Abort button interrupt
static int
w99702_abort_isr(cyg_vector_t vector, cyg_addrword_t data, HAL_SavedRegisters *regs)
{
    cyg_hal_user_break((CYG_ADDRWORD*)regs);
    cyg_drv_interrupt_acknowledge(CYGNUM_HAL_INTERRUPT_EXT0);
    return 0;  // No need to run DSR
}
#endif

void hal_clock_initialize(cyg_uint32 period)
{

    cyg_uint32 volatile tmod;
	cyg_uint32 clock_config;
    
    
	HAL_READ_UINT32(CLKCON, clock_config);
	HAL_WRITE_UINT32 (CLKCON, clock_config | 0x1080);//enable Timer/APB Bridge clock
	tmod = 0x1000;
	while(tmod--);
	/*----- disable timer -----*/
	HAL_WRITE_UINT32(TCR0, 0);
   	HAL_WRITE_UINT32(TCR1, 0);
   	
	/* configure GPIO */
	//CSR_WRITE(GPIO_CFG, 0x00150D0);//15AD8
	//HAL_WRITE_UINT32(GPIO_CFG, 0x00050D0);//15AD8
diag_printf("Timer Period=%d\n",period);//period = 
	//HAL_WRITE_UINT32 (AIC_SCR15, 0x41);  /* high-level sensitive, priority level 1 */
	/*----- timer 0 : periodic mode, 100 tick/sec -----*/
	HAL_WRITE_UINT32(TICR0, period);//5dc//3a98->10//249f0//0xBB8->50//1d4c->20
	//CSR_WRITE(TICR0, 0x4b0);//12M
	
		/*----- clear interrupt flag bit -----*/
	HAL_WRITE_UINT32(TISR, 0);  /* clear for safty */   

	HAL_WRITE_UINT32(TCR0, 0x68000000);//clyu 20050421
	hal_interrupt_unmask(CYGNUM_HAL_INTERRUPT_TIMER0);
    
    _period = period;

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_EXT0,
                             99,           // Priority
                             0,            // Data item passed to interrupt handler
                             ks32c_abort_isr,
                             0,
                             &abort_interrupt_handle,
                             &abort_interrupt);
    cyg_drv_interrupt_attach(abort_interrupt_handle);
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_EXT0);
#endif

}

void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
    _period = period;
}

void hal_clock_read(cyg_uint32 *pvalue)
{
    cyg_uint32 value;

    HAL_READ_UINT32(TDR0, value);
    *pvalue = _period - value;
}


/* Define Cache type  */
#define CACHE_WRITE_BACK		0
#define CACHE_WRITE_THROUGH		1
/* Define constants for use Cache in service parameters.  */
#define CACHE_4M		2
#define CACHE_8M		3
#define CACHE_16M		4
#define CACHE_32M		5
#define I_CACHE			6
#define D_CACHE			7
#define I_D_CACHE		8

void hal_enable_cache(cyg_uint32 uCacheOpMode)
{
	int temp;

	__asm
	{
		/*----- disable Protection Unit -----*/
		MRC p15, 0, temp, c1, c0, 0 	/* read Control register */
		BIC	temp, temp, 0x01
		MCR p15, 0, temp, c1, c0, 0 	/* write Control register */

		/*----- flush cache & write buffer -----*/
		MOV temp, 0x0
		MCR p15, 0, temp, c7, c5, 0 /* flush I cache */
		MCR p15, 0, temp, c7, c6, 0 /* flush D cache */
		MCR p15, 0, temp, c7, c10,4 /* drain write buffer */
	}
	
	/*----- Assign region 0 ~ 4 -----*/
	switch (CYGMEM_REGION_ram_SIZE)
	{
		case 0x800000:
			__asm
			{
				MOV temp, 0x2D
				MCR p15, 0, temp, c6, c0, 0	/* region 0 : 0 ~ 8MB */
				MOV temp, 0x1000002D
				MCR p15, 0, temp, c6, c1, 0	/* region 1 : 0x10000000 ~ 0x10800000 */
			}
			break;

		case 0x1000000:
			__asm
			{
				MOV temp, 0x2F
				MCR p15, 0, temp, c6, c0, 0	/* region 0 : 0 ~ 16MB */
				MOV temp, 0x1000002F
				MCR p15, 0, temp, c6, c1, 0	/* region 1 : 0x10000000 ~ 0x11000000 */
			}
			break;

		case 0x2000000:
			__asm
			{
				MOV temp, 0x31
				MCR p15, 0, temp, c6, c0, 0	/* region 0 : 0 ~ 32MB */
				MOV temp, 0x10000031
				MCR p15, 0, temp, c6, c1, 0	/* region 1 : 0x10000000 ~ 0x12000000 */
			}
			break;

		default:	// 8M
			__asm
			{
				MOV temp, 0x2D
				MCR p15, 0, temp, c6, c0, 0	/* region 0 : 0 ~ 8MB */
				MOV temp, 0x1000002D
				MCR p15, 0, temp, c6, c1, 0	/* region 1 : 0x10000000 ~ 0x10800000 */
			}
			break;
	}

	__asm
	{
		MOV temp, 0x7ff00027
		MCR p15, 0, temp, c6, c2, 0	/* region 2 : 0x7ff00000 ~ 0x80000000 */
		MOV temp, 0x30000037
		MCR p15, 0, temp, c6, c3, 0	/* region 3 : 0x30000000 ~ 0x40000000 */
		MOV temp, 0xf0000037
		MCR p15, 0, temp, c6, c4, 0	/* region 4 : 0xf0000000 ~ 0xffffffff */

		/*----- read/write permission -----*/
		MOV temp, 0x33333 /* priv - r/w, user - r/w for region 0 ~ 4 */
		MCR p15, 0, temp, c5, c0, 2 /* data access permission bits */
		MCR p15, 0, temp, c5, c0, 3 /* instruction access permission bits */

		/*----- I & D Cacheable -----*/
		MOV temp, 0x01	/* region 1~4 - disabled, region 0 - enabled */
		MCR p15, 0, temp, c2, c0, 0 /* data cacheable bits */
		MCR p15, 0, temp, c2, c0, 1 /* instruction cacheable bits */
	}


	if (uCacheOpMode == CACHE_WRITE_BACK)
	{
		__asm
		{
			/*----- Write Buffer -----*/
			MCR p15, 0, temp, c3, c0, 0
		}
	}
	else//write through
	{
		temp = 0x00;
		__asm
		{
			/*----- Write Buffer -----*/
			MCR p15, 0, temp, c3, c0, 0
		}
	}
	
	__asm
	{
		/*----- enable I/D cache and Protection Unit -----*/
		MRC p15, 0, temp, c1, c0, 0 	/* read Control register */
		ORR temp, temp, 0x1005			/* I-bit12, D-bit2, P-bit1 */
		MCR p15, 0, temp, c1, c0, 0
	}
	//_sys_IsCacheOn = TRUE;
}

extern void sys_flush_and_clean_dcache(void);
void hal_disable_cache()
{
	int temp;

	sys_flush_and_clean_dcache();
	__asm
	{
		/*----- flush I, D cache & write buffer -----*/
		MOV temp, 0x0
		MCR p15, 0, temp, c7, c5, 0 /* flush I cache */
		MCR p15, 0, temp, c7, c6, 0 /* flush D cache */
		MCR p15, 0, temp, c7, c10,4 /* drain write buffer */		

		/*----- disable I/D cache and Protection Unit -----*/
		MRC p15, 0, temp, c1, c0, 0 	/* read Control register */
		BIC	temp, temp, 0x1005
		MCR p15, 0, temp, c1, c0, 0 	/* write Control register */
	}
	//_sys_IsCacheOn = FALSE;
}


void hal_flush_cache(cyg_int32 nCacheType)
{
	int temp;

	switch (nCacheType)
	{
		case I_CACHE:
			__asm
			{
				/*----- flush I-cache -----*/
				MOV temp, 0x0
				MCR p15, 0, temp, c7, c5, 0 /* flush I cache */
			}
			break;

		case D_CACHE:
			sys_flush_and_clean_dcache();
			__asm
			{
				/*----- flush D-cache & write buffer -----*/
				MOV temp, 0x0
				MCR p15, 0, temp, c7, c6, 0 /* flush D cache */
				MCR p15, 0, temp, c7, c10,4 /* drain write buffer */
			}
			break;

		case I_D_CACHE:
			sys_flush_and_clean_dcache();
			__asm
			{
				/*----- flush I, D cache & write buffer -----*/
				MOV temp, 0x0
				MCR p15, 0, temp, c7, c5, 0 /* flush I cache */
				MCR p15, 0, temp, c7, c6, 0 /* flush D cache */
				MCR p15, 0, temp, c7, c10,4 /* drain write buffer */		
			}
			break;

		default:
			;
	}
}

void hal_invalid_cache()
{
	int temp;

	__asm
	{
		/*----- flush I, D cache -----*/
		MOV temp, 0x0
		MCR p15, 0, temp, c7, c5, 0 /* flush I cache */
		MCR p15, 0, temp, c7, c6, 0 /* flush D cache */
	}
}


//======================================================================
// Interrupt controller stuff
#if 0
extern volatile  tInterruptController w99702_int;  // Interrupt controller registers
extern volatile  unsigned long EXTACON0;    // Extern access control reg
extern volatile  unsigned long EXTACON1;    // Extern access control reg
volatile  unsigned long IOPCON;// I/O Port Control reg
volatile  unsigned long IOPMOD;
extern volatile  unsigned long SYSCON;
#endif

void hal_hardware_init(void)
{
    cyg_uint32 intmask, cachectrl;
    cyg_uint32 volatile tmp=0x1000;
	
	//bootloader enable vpost engine. vpost engine clock can't disable as this only.
	//need wait when vpost don't get data
	// or  AHB bus  will halt
	//HAL_WRITE_UINT32(CLKCON, 0x02C01000);//20060622
	
    HAL_WRITE_UINT32(AIC_MDCR, 0xFFFFFFFF); /* disable all interrupts */
    HAL_WRITE_UINT32(AIC_IRSR, 0x00000000); /* disable all interrupts */
    HAL_WRITE_UINT32(AIC_IASR, 0x00000000); /* disable all interrupts */
    HAL_WRITE_UINT32(GPIOIE, 0x00000000); /* disable all GPIO interrupts */
    //HAL_WRITE_UINT32(DEBNCE_CTRL,0xffffffff);//GPIO de-bounce hardware error; can't use
    while(tmp-->0);
    
    HAL_WRITE_UINT32(GPIOIS,0xffffffff);
    

#if 0 //99702 no Netcard
	//stop netcard
	w740_WriteReg(FIFOTHD,0x10000,0);
	w740_WriteReg(FIFOTHD,0x10000,1);
#endif
    // Set up eCos/ROM interfaces
    hal_if_init();
	
#if 1
	hal_disable_cache();
	hal_flush_cache(I_D_CACHE);
	hal_invalid_cache();
	
	//hal_enable_cache(CACHE_WRITE_BACK);
	hal_enable_cache(CACHE_WRITE_THROUGH);
#endif
}


// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.

int hal_IRQ_handler(void)
{

#ifdef SW
    // Do hardware-level IRQ handling
    cyg_uint32 irq_status;
    HAL_READ_UINT32(AIC_ISR, irq_status);
//diag_printf("irq_status:%x,MAX:%x\n",irq_status,CYGNUM_HAL_ISR_MAX);
    if (CYGNUM_HAL_ISR_MAX >= irq_status)
    {
    	cyg_uint32 irq=26,ISR=0x4000000;
    	for(;ISR>1;ISR=ISR>>1,irq--)
    	{
    		if(!(ISR&irq_status))
			{
				continue;
			}
//			diag_printf("irq = %d\n",irq);
        	return irq;
        }
    }
#else
	cyg_uint32 irq_number;
    HAL_READ_UINT32(AIC_IPER, irq_number);
    HAL_READ_UINT32(AIC_ISNR, irq_number);
	if (irq_number<=26)
		return irq_number;
#endif
	return CYGNUM_HAL_INTERRUPT_NONE;

}


//
// Interrupt control
//

void hal_interrupt_mask(int vector)
{
    INT_DISABLE(vector);
}

void hal_interrupt_unmask(int vector)
{
    INT_ENABLE(vector);
}

void hal_interrupt_acknowledge(int vector)
{
   // HAL_WRITE_UINT32(KS32C_INTPND, (1<<vector));
	if(vector == CYGNUM_HAL_INTERRUPT_RTC)
   		HAL_WRITE_UINT32(TISR,2); /* clear TIF0 */
#ifndef SW
	HAL_WRITE_UINT32(AIC_EOSCR, 0x01);
#endif
}


#define WB_MIN_INT_SOURCE  1
#define WB_MAX_INT_SOURCE  26
#define WB_NUM_OF_AICREG   27
/* Define AIC source control register address */
static unsigned int WB_AICRegAddr[WB_NUM_OF_AICREG] = { 0,
				AIC_SCR1,	AIC_SCR2,	AIC_SCR3,	AIC_SCR4,
				AIC_SCR5,	AIC_SCR6,	AIC_SCR7,	AIC_SCR8,								
				AIC_SCR9,	AIC_SCR10,	AIC_SCR11,	AIC_SCR12,
				AIC_SCR13,	AIC_SCR14,	AIC_SCR15,	AIC_SCR16,
				AIC_SCR17,	AIC_SCR18,	AIC_SCR19,	AIC_SCR20,
				AIC_SCR21,	AIC_SCR22,	AIC_SCR23,	AIC_SCR24,
				AIC_SCR25,	AIC_SCR26
			};

/*
	Level:	false for edge trigger
			true for level trigger
	up:		false for faling edge/low level
			true for rising edge/high level
*/
void hal_interrupt_configure(int vector, int level, int up)
{
   unsigned int regAddr, regValue;

#define LOW_LEVEL_SENSITIVE        0x00
#define HIGH_LEVEL_SENSITIVE       0x40
#define NEGATIVE_EDGE_TRIGGER      0x80
#define POSITIVE_EDGE_TRIGGER      0xC0

	if((vector > WB_MAX_INT_SOURCE) || (vector < WB_MIN_INT_SOURCE))
		return;
	
	regAddr = WB_AICRegAddr[vector];
	HAL_READ_UINT32(regAddr, regValue);
	regValue &= 0xFFFFFF3F;
	
	if(!level && !up)
		regValue |= NEGATIVE_EDGE_TRIGGER;
	else if(!level && up)
		regValue |= POSITIVE_EDGE_TRIGGER;
	else if(level && !up)
		regValue |= LOW_LEVEL_SENSITIVE;
	else if(level && up)
		regValue |= HIGH_LEVEL_SENSITIVE;
		
	HAL_WRITE_UINT32(regAddr, regValue);

   return;

}

//set priority
void hal_interrupt_set_level(int vector, int level)
{
	unsigned int regAddr, regValue;
	
	if ((vector > WB_MAX_INT_SOURCE) || (vector < WB_MIN_INT_SOURCE))
		return;

	regAddr = WB_AICRegAddr[vector];
	HAL_READ_UINT32(regAddr, regValue);
	regValue &= 0xFFFFFFF8;
	
	HAL_WRITE_UINT32(regAddr, (regValue | level));

	return;
}

void hal_show_IRQ(int vector, int data, int handler)
{
}

// -------------------------------------------------------------------------
//
// Delay for some number of micro-seconds
//
void hal_delay_us(cyg_int32 usecs)
{
    cyg_uint32 count;
    cyg_uint32 ticks = ((CYGNUM_HAL_RTC_PERIOD*CYGNUM_HAL_RTC_DENOMINATOR)/1000000) * usecs;
    cyg_uint32 tmod;

	 HAL_READ_UINT32(TCR1, tmod);
	 tmod &= ~(CE);
	 HAL_WRITE_UINT32(TCR1, tmod);
	
	/*----- timer 1 : periodic mode, 100 tick/sec -----*/
	HAL_WRITE_UINT32(TICR1, 0x5dc);//5dc//3a98->10//249f0//0xBB8->50//1d4c->20

	HAL_WRITE_UINT32(TCR1, 0xe8000063);

   
    // Wait for timer to underflow. Can't test the timer completion
    // bit without actually enabling the interrupt. So instead watch
    // the counter.
    ticks /= 2;                         // wait for this threshold

    // Wait till timer counts below threshold
    do {
        HAL_READ_UINT32(TDR1, count);
    } while (count >= ticks);
    // then wait for it to be reloaded
    do {
        HAL_READ_UINT32(TDR1, count);
    } while (count < ticks);

}

void hal_reset(void)
{
  CYG_INTERRUPT_STATE old;
  cyg_uint32 iValue;
  HAL_DISABLE_INTERRUPTS(old);
  
  HAL_READ_UINT32(0xFFF0000C,iValue);
  iValue |= 1;
  HAL_WRITE_UINT32(0xFFF0000C, iValue);
}

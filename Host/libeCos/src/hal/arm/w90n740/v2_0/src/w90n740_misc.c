//==========================================================================
//
//      w90n740_misc.c
//
//      HAL misc board support code for Winbond w90n740
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
// Date:         2003-11-27
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

#include "w90n740.h"
#include "cyg/hal/plf_io.h"
//#define SW
//======================================================================
// Use Timer0 for kernel clock

static cyg_uint32 _period;

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
static cyg_interrupt abort_interrupt;
static cyg_handle_t  abort_interrupt_handle;

// This ISR is called only for the Abort button interrupt
static int
w90n740_abort_isr(cyg_vector_t vector, cyg_addrword_t data, HAL_SavedRegisters *regs)
{
    cyg_hal_user_break((CYG_ADDRWORD*)regs);
    cyg_drv_interrupt_acknowledge(CYGNUM_HAL_INTERRUPT_EXT0);
    return 0;  // No need to run DSR
}
#endif

void hal_clock_initialize(cyg_uint32 period)
{

    cyg_uint32 tmod;

	/*----- disable timer -----*/
	HAL_WRITE_UINT32(TCR0, 0);
   	HAL_WRITE_UINT32(TCR1, 0);
   	
	/* configure GPIO */
	//CSR_WRITE(GPIO_CFG, 0x00150D0);//15AD8
	HAL_WRITE_UINT32(GPIO_CFG, 0x00050D0);//15AD8

	HAL_WRITE_UINT32 (AIC_SCR7, 0x41);  /* high-level sensitive, priority level 1 */
	/*----- timer 0 : periodic mode, 100 tick/sec -----*/
	//clyu 20050421 (period)/100
	HAL_WRITE_UINT32(TICR0, (period));//5dc//3a98->10//249f0//0xBB8->50//1d4c->20

diag_printf("Timer Period=%d\n",period);//period = 

		/*----- clear interrupt flag bit -----*/
	HAL_WRITE_UINT32(TISR, 0);  /* clear for safty */   

	HAL_WRITE_UINT32(TCR0, 0xe8000000);//clyu 20050421 0xe8000063
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

//======================================================================
// Interrupt controller stuff
#if 0
extern volatile  tInterruptController w90n740_int;  // Interrupt controller registers
extern volatile  unsigned long EXTACON0;    // Extern access control reg
extern volatile  unsigned long EXTACON1;    // Extern access control reg
volatile  unsigned long IOPCON;// I/O Port Control reg
volatile  unsigned long IOPMOD;
extern volatile  unsigned long SYSCON;
#endif

void hal_hardware_init(void)
{
    cyg_uint32 intmask, cachectrl;
	
	//stop netcard
	w740_WriteReg(FIFOTHD,0x10000,0);
	w740_WriteReg(FIFOTHD,0x10000,1);
	
    // Set up eCos/ROM interfaces
    hal_if_init();

    HAL_WRITE_UINT32(AIC_MDCR, 0x7FFFE); /* disable all interrupts */

	HAL_WRITE_UINT32(CAHCNF,0x0);/*Close Cache*/
	HAL_WRITE_UINT32(CAHCON,0x87);/*Flush Cache*/

	do{
		HAL_READ_UINT32(CAHCON,cachectrl);
	}while(cachectrl!=0);

	HAL_WRITE_UINT32(CAHCNF,0x7);/*Open Cache*/

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
    	cyg_uint32 irq=18,ISR=0x40000;
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

void hal_interrupt_configure(int vector, int level, int up)
{
}

void hal_interrupt_set_level(int vector, int level)
{
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

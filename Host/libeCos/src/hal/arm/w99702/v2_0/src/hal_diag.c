/*=============================================================================
//
//      hal_diag.c
//
//      HAL diagnostic output code
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
// Date:        2003-11-26
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include "pkgconf/hal.h"
#include CYGBLD_HAL_PLATFORM_H

#include "cyg/infra/cyg_type.h"         // base types

#include "cyg/hal/hal_arch.h"           // SAVE/RESTORE GP macros
#include "cyg/hal/hal_io.h"             // IO macros
#include "cyg/hal/hal_if.h"             // interface API
#include "cyg/hal/hal_intr.h"           // HAL_ENABLE/MASK/UNMASK_INTERRUPTS
#include "cyg/hal/hal_misc.h"           // Helper functions
#include "cyg/hal/drv_api.h"            // CYG_ISR_HANDLED

#include "cyg/hal/plf_io.h"             // SIO registers

#if 0
struct serial_baudtable
{
	unsigned int baudrate;
	unsigned int div;
};
#endif

static unsigned int baudrate_div(unsigned int baudrate)
{
	unsigned int div;
	div = fMCLK_MHz/(baudrate*16);
	if ((fMCLK_MHz % (baudrate * 16)) > ((baudrate * 16) / 2))
		div++;
	div -= 2;
	return div;
}

//-----------------------------------------------------------------------------
typedef struct {
    int baudrate;
    cyg_uint8* base;
    cyg_int32 msec_timeout;
    int isr_vector_rx;
    int isr_vector_tx;
} channel_data_t;

//-----------------------------------------------------------------------------

static void
cyg_hal_plf_serial_init_channel(void* __ch_data)
{
    cyg_uint32 base = (cyg_uint32)((channel_data_t*)__ch_data)->base;
    cyg_uint32 volatile div;
    cyg_uint32 volatile tmp, pad;
    cyg_uint32 volatile i;
    cyg_uint32 _mBaudValue;
    
	HAL_READ_UINT32(CLKCON, tmp);
	
	HAL_READ_UINT32(PADC0, pad);
	if(base == UART_BASE0)
	{
		HAL_WRITE_UINT32(PADC0, pad | 0x00040000);//enable UART pin
		if((tmp & 0x30) != 0x10)
		{
			HAL_WRITE_UINT32(CLKCON, (tmp & 0xFFFFFFCF) | 0x1010);//enable APB Bridge/UART clock disable UART1
			for(i=0;i<0x1000;i++);
		}
	}
	else
	{
		HAL_WRITE_UINT32(PADC0, pad | 0x00080000);//enable UART pin
		if((tmp & 0x0C) != 0x04)
		{
			HAL_WRITE_UINT32(CLKCON, (tmp & 0xFFFFFFF3) | 0x1004);
			for(i=0;i<0x1000;i++);
		}
	}
	
	HAL_READ_UINT32(base + COM_FCR, tmp);
	HAL_WRITE_UINT32(base + COM_FCR, tmp | 0x06);//reset Tx/Rx FIFO
    // 8-1-no parity.
    HAL_WRITE_UINT32 (base + COM_LCR, 0x80); /* select divisor latch registers */

	div=baudrate_div(((channel_data_t*)__ch_data)->baudrate);
    HAL_WRITE_UINT32(base + COM_DLL, div&0xFF);
    HAL_WRITE_UINT32(base + COM_DLM, (div>>8)&0xFF);
    
    HAL_READ_UINT32(base + COM_LCR, tmp);
    HAL_WRITE_UINT32(base + COM_LCR, tmp & 0x7F);

    HAL_WRITE_UINT32(base + COM_LCR, UART_LCR_NPAR|UART_LCR_WLEN8|UART_LCR_NSB); /* none parity, 8 data bits, 1 stop bits */
	
    // Mask interrupts
    HAL_INTERRUPT_MASK(((channel_data_t*)__ch_data)->isr_vector_rx);
    HAL_INTERRUPT_MASK(((channel_data_t*)__ch_data)->isr_vector_tx);
	
    // Enable RX and TX
    //HAL_WRITE_UINT32(base + COM_IER, UART_IER_RDI|UART_IER_THRI);

}

void
cyg_hal_plf_serial_putc(void *__ch_data, char c)
{
	cyg_uint32 base = (cyg_uint32)((channel_data_t*)__ch_data)->base;
    cyg_uint32 status, ch;
    CYGARC_HAL_SAVE_GP();

    do {
        HAL_READ_UINT32(base + COM_LSR, status);
    } while ((status & UART_LSR_THRE) == 0);
	
    ch = (cyg_uint32)c;
    HAL_WRITE_UINT32(base + COM_TX, ch);
    
    do {
        HAL_READ_UINT32(base + COM_LSR, status);
    } while ((status & UART_LSR_THRE) == 0);
    
    

    CYGARC_HAL_RESTORE_GP();
}

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint32 base = (cyg_uint32)chan->base;
    cyg_uint32 stat;
    cyg_uint32 c;

    HAL_READ_UINT32(base + COM_LSR, stat);
    if ((stat & UART_LSR_DR) == 0)
        return false;

    HAL_READ_UINT32(base + COM_RX, c);
    *ch = (cyg_uint8)(c & 0xff);

    HAL_INTERRUPT_ACKNOWLEDGE(chan->isr_vector_rx);

    return true;
}

cyg_uint8
cyg_hal_plf_serial_getc(void* __ch_data)
{
    cyg_uint8 ch;
    CYGARC_HAL_SAVE_GP();

    while(!cyg_hal_plf_serial_getc_nonblock(__ch_data, &ch));

    CYGARC_HAL_RESTORE_GP();
    return ch;
}

static void
cyg_hal_plf_serial_write(void* __ch_data, const cyg_uint8* __buf, 
                         cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        cyg_hal_plf_serial_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_plf_serial_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        *__buf++ = cyg_hal_plf_serial_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}

cyg_bool
cyg_hal_plf_serial_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();

    delay_count = chan->msec_timeout * 10; // delay in .1 ms steps

    for(;;) {
        res = cyg_hal_plf_serial_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
        
        CYGACC_CALL_IF_DELAY_US(100);
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static int
cyg_hal_plf_serial_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    static int irq_state = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    int ret = 0;
    CYGARC_HAL_SAVE_GP();

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        irq_state = 1;
        HAL_INTERRUPT_ACKNOWLEDGE(chan->isr_vector_rx);
        HAL_INTERRUPT_UNMASK(chan->isr_vector_rx);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;
        HAL_INTERRUPT_MASK(chan->isr_vector_rx);
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = chan->isr_vector_rx;
        break;
    case __COMMCTL_SET_TIMEOUT:
    {
        va_list ap;

        va_start(ap, __func);

        ret = chan->msec_timeout;
        chan->msec_timeout = va_arg(ap, cyg_uint32);

        va_end(ap);
    }        
    default:
        break;
    }
    CYGARC_HAL_RESTORE_GP();
    return ret;
}

static int
cyg_hal_plf_serial_isr(void *__ch_data, int* __ctrlc, 
                       CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    int res = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint32 c;
    cyg_uint8 ch;
    cyg_uint32 stat;
    cyg_uint32 base = (cyg_uint32)chan->base;
    CYGARC_HAL_SAVE_GP();

    *__ctrlc = 0;
    HAL_READ_UINT32(base + COM_IIR,stat);
	do {
		if (stat & UART_IIR_DR)
		{
			//do{
				HAL_READ_UINT32(base + COM_RX,c);
				ch = (cyg_uint8)(c & 0xff);
        		if( cyg_hal_is_break((char*) &ch , 1 ) )
            		*__ctrlc = 1;

        		res = CYG_ISR_HANDLED;
        	//	HAL_READ_UINT32(COM_LSR,status);
			//} while ((status & UART_LSR_DR) && (max_count-- > 0));
		}
// 1/28/2003 Winbond w99702 doesn't have modem relate registers
//		check_modem_status(info);

//		if (status & UART_IIR_THRE)
//			transmit_chars(0);
		HAL_READ_UINT32(base + COM_IIR,stat);

	} while (stat & (UART_IIR_DR | UART_IIR_THRE));

    HAL_INTERRUPT_ACKNOWLEDGE(chan->isr_vector_rx);

    CYGARC_HAL_RESTORE_GP();
    return res;
}


static channel_data_t w99702_ser_channels[2] = {
    { BASE_BAUD0, (cyg_uint8*)UART_BASE0, 1000, CYGNUM_HAL_INTERRUPT_UART0_RX, CYGNUM_HAL_INTERRUPT_UART0_TX },
    { BASE_BAUD1, (cyg_uint8*)UART_BASE1, 1000, CYGNUM_HAL_INTERRUPT_UART1_RX, CYGNUM_HAL_INTERRUPT_UART1_TX },
};

static void
cyg_hal_plf_serial_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    // Init channels
    cyg_hal_plf_serial_init_channel(&w99702_ser_channels[0]);
    cyg_hal_plf_serial_init_channel(&w99702_ser_channels[1]);

    // Setup procs in the vector table

    // Set channel 0
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &w99702_ser_channels[0]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);
	
	// Set channel 1
    CYGACC_CALL_IF_SET_CONSOLE_COMM(1);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &w99702_ser_channels[1]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);
	
    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
}

void
cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;

    if (initialized)
        return;

    initialized = 1;

    cyg_hal_plf_serial_init();
}

//-----------------------------------------------------------------------------
// LED
void
hal_diag_led(int mask)
{
#if 0
    cyg_uint32 l;

    HAL_READ_UINT32(KS32C_IOPDATA, l);
    l &= ~0x000000f0;
    l |= (mask & 0xf) << 4;
    HAL_WRITE_UINT32(KS32C_IOPDATA, l);
#endif
}

//-----------------------------------------------------------------------------
// End of hal_diag.c

//==========================================================================
//
//      io/serial/arm/w90n740_serial_with_ints.c
//
//      ARM W90N740 Serial I/O Interface Module (interrupt driven)
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
// Author(s):    David A Rusling
// Contributors: Philippe Robin
// Date:         November 7, 2000
// Purpose:      W90N740 Serial I/O module (interrupt driven)
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include "pkgconf/system.h"
#include "pkgconf/io.h"
#include "pkgconf/io_serial.h"
#include "cyg/io/io.h"
#include "cyg/hal/hal_intr.h"
#include "cyg/io/devtab.h"
#include "cyg/io/serial.h"
#include "cyg/infra/diag.h"

#ifdef CYGPKG_IO_SERIAL_ARM_W90N740
#include "w90n740_serial.h"

typedef struct w90n740_serial_info {
    CYG_ADDRWORD   base;
    CYG_WORD       int_num;
    cyg_interrupt  serial_interrupt;
    cyg_handle_t   serial_interrupt_handle;
} w90n740_serial_info;

static bool w90n740_serial_init(struct cyg_devtab_entry *tab);
static bool w90n740_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo w90n740_serial_lookup(struct cyg_devtab_entry **tab, 
                                   struct cyg_devtab_entry *sub_tab,
                                   const char *name);
static unsigned char w90n740_serial_getc(serial_channel *chan);
static Cyg_ErrNo w90n740_serial_set_config(serial_channel *chan, cyg_uint32 key,
					      const void *xbuf, cyg_uint32 *len);
static void w90n740_serial_start_xmit(serial_channel *chan);
static void w90n740_serial_stop_xmit(serial_channel *chan);

static cyg_uint32 w90n740_serial_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       w90n740_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);

static SERIAL_FUNS(w90n740_serial_funs, 
                   w90n740_serial_putc, 
                   w90n740_serial_getc,
                   w90n740_serial_set_config,
                   w90n740_serial_start_xmit,
                   w90n740_serial_stop_xmit
    );


#ifdef CYGPKG_IO_SERIAL_ARM_W90N740_SERIAL0
#define UART_BASE0           COM_TX	 /*  UART 0 */
static w90n740_serial_info w90n740_serial_info0 = {UART_BASE0, INT_UARTINT};
#if CYGNUM_IO_SERIAL_ARM_W90N740_SERIAL0_BUFSIZE > 0
static unsigned char w90n740_serial_out_buf0[CYGNUM_IO_SERIAL_ARM_W90N740_SERIAL0_BUFSIZE];
static unsigned char w90n740_serial_in_buf0[CYGNUM_IO_SERIAL_ARM_W90N740_SERIAL0_BUFSIZE];
unsigned char bluetooth_buf[CYGNUM_IO_SERIAL_ARM_W90N740_SERIAL0_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(w90n740_serial_channel0,
                                       w90n740_serial_funs, 
                                       w90n740_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_W90N740_SERIAL0_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &w90n740_serial_out_buf0[0], sizeof(w90n740_serial_out_buf0),
                                       &w90n740_serial_in_buf0[0], sizeof(w90n740_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(w90n740_serial_channel0,
                      w90n740_serial_funs, 
                      w90n740_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_W90N740_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

#pragma arm section rwdata = "devtab"
DEVTAB_ENTRY(w90n740_serial_io0, 
             CYGDAT_IO_SERIAL_ARM_W90N740_SERIAL0_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             w90n740_serial_init, 
             w90n740_serial_lookup,     // Serial driver may need initializing
             &w90n740_serial_channel0
    );
#pragma arm section rwdata

#endif //  CYGPKG_IO_SERIAL_ARM_W90N740_SERIAL0

struct cyg_devtab_entry *serial_driver;

#define GET_INTERRUPT_STATUS(p)			IO_READ(COM_IIR)
#define GET_STATUS(p)					(IO_READ(COM_LSR))
#define GET_CHAR(p)						(IO_READ(COM_RX)&0xFF)
#define PUT_CHAR(p, c)					(IO_WRITE((COM_TX), ((c)&0xFF)))
#define IO_READ(p)                        (*(volatile unsigned int *)(p))
#define IO_WRITE(p, c)                    (*(volatile unsigned int *)(p) = (c))
#define RX_DATA(s)		          (((s) & UART_LSR_DR) != 0)
#define TX_READY(s)		          (((s) & UART_LSR_THRE) != 0)
#define TX_EMPTY(p)		          ((GET_STATUS(p) & UART_LSR_THRE) != 0)

// debugging help
static int chars_rx = 0 ;
static int chars_tx = 0 ;

cyg_sem_t serial740_sem;//clyu

// Internal function to actually configure the hardware to desired baud rate, etc.
static bool
w90n740_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    w90n740_serial_info *w90n740_chan = (w90n740_serial_info *)chan->dev_priv;
    unsigned int port = (unsigned int)w90n740_chan->base;
    unsigned short baud_divisor = baudrate_div(select_baud[new_config->baud]);
    unsigned int _lcr ;

    // first, disable everything 
    disable_uart_all_interrupt();

    // Set baud rate 
    IO_WRITE(COM_LCR, 0x80); /* select divisor latch registers */
    IO_WRITE(COM_DLL, (baud_divisor&0xFF));
    IO_WRITE(COM_DLM, ((baud_divisor>>8)&0xFF));

    // ----------v----------v----------v----------v---------- 
    // NOTE: MUST BE WRITTEN LAST (AFTER UARTLCR_M & UARTLCR_L) 
    // ----------^----------^----------^----------^----------
    _lcr = 
      select_word_length[new_config->word_length - CYGNUM_SERIAL_WORD_LENGTH_5] | 
      select_stop_bits[new_config->stop] |
      select_parity[new_config->parity] ;

    IO_WRITE(COM_LCR, _lcr);
    
	
    // save the configuration
    if (new_config != &chan->config) {
        chan->config = *new_config;
    }
    
    //clyu 2005/04/12
    // finally, enable the uart 
    enable_uart_rx_interrupt();

    // success
    return true;
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
w90n740_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    w90n740_serial_info *w90n740_chan = (w90n740_serial_info *)chan->dev_priv;
    
    cyg_semaphore_init(&serial740_sem,0);//clyu
	
	serial_driver = tab;
//diag_printf("w90n740_chan->int_num=%d\n",w90n740_chan->int_num);

#ifdef CYGDBG_IO_INIT
    diag_printf("W90N740 SERIAL init - dev: %x.%d\n", w90n740_chan->base, w90n740_chan->int_num);
#endif
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    if (chan->out_cbuf.len != 0) {
        cyg_drv_interrupt_create(w90n740_chan->int_num,
                                 99,                     // Priority - what goes here?
                                 (cyg_addrword_t)chan,   //  Data item passed to interrupt handler
                                 w90n740_serial_ISR,
                                 w90n740_serial_DSR,
                                 &w90n740_chan->serial_interrupt_handle,
                                 &w90n740_chan->serial_interrupt);
        cyg_drv_interrupt_attach(w90n740_chan->serial_interrupt_handle);
        cyg_drv_interrupt_unmask(w90n740_chan->int_num);
    }
    w90n740_serial_config_port(chan, &chan->config, true);
    
    IO_WRITE(COM_FCR, 0xCF);/* 8-byte FIFO trigger level, reset Tx and Rx FIFO */
    IO_WRITE(COM_MCR, 0x00);
	IO_WRITE(COM_TOR, 0x80 + 0x20);

    // finally, enable the uart 
    enable_uart_rx_interrupt();
    
    return true;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
w90n740_serial_lookup(struct cyg_devtab_entry **tab, 
                  struct cyg_devtab_entry *sub_tab,
                  const char *name)
{
    serial_channel *chan = (serial_channel *)(*tab)->priv;
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    return ENOERR;
}

// Send a character to the device output buffer.
// Return 'true' if character is sent to device
static int wrcount = 8;
static bool
w90n740_serial_putc(serial_channel *chan, unsigned char c)
{
    w90n740_serial_info *w90n740_chan = (w90n740_serial_info *)chan->dev_priv;
    unsigned int status = GET_STATUS(w90n740_chan->base);

#if 0
    if (TX_READY(status)){
//    if(wrcount--){
// Transmit buffer is empty
        PUT_CHAR(w90n740_chan->base, c);
        
		chars_tx++ ;
        return true;
    } else {
// No space
		//wrcount = 8;
		return false;
    }
#else
	while(!TX_READY( GET_STATUS(w90n740_chan->base) ));
	PUT_CHAR(w90n740_chan->base, c);
        
	chars_tx++ ;
    return true;
#endif
}

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char 
w90n740_serial_getc(serial_channel *chan)
{
    unsigned char c;
    w90n740_serial_info *w90n740_chan = (w90n740_serial_info *)chan->dev_priv;
    unsigned int status ;

    do {
        status = GET_STATUS(w90n740_chan->base) ;
    } while (!RX_DATA(status)) ;                   // Wait for char

    chars_rx++ ;

    // get it 
    c = GET_CHAR(w90n740_chan->base) ;
    return c;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
w90n740_serial_set_config(serial_channel *chan, cyg_uint32 key, const void *xbuf,
                      cyg_uint32 *len)
{
    w90n740_serial_info *w90n740_chan = (w90n740_serial_info *)chan->dev_priv;

    switch (key) {
    case CYG_IO_SET_CONFIG_SERIAL_INFO:
      {
        cyg_serial_info_t *config = (cyg_serial_info_t *)xbuf;
        if ( *len < sizeof(cyg_serial_info_t) ) {
            return -EINVAL;
        }
        *len = sizeof(cyg_serial_info_t);
        if ( true != w90n740_serial_config_port(chan, config, false) )
            return -EINVAL;
      }
      break;
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_HW
#ifdef FIXME
    case CYG_IO_SET_CONFIG_SERIAL_HW_RX_FLOW_THROTTLE:
      {
          volatile struct serial_port *port = (volatile struct serial_port *)w90n740_chan->base;
          cyg_uint8 *f = (cyg_uint8 *)xbuf;
          unsigned char mask=0;
          if ( *len < *f )
              return -EINVAL;
          
          if ( chan->config.flags & CYGNUM_SERIAL_FLOW_RTSCTS_RX )
              mask = MCR_RTS;
          if ( chan->config.flags & CYGNUM_SERIAL_FLOW_DSRDTR_RX )
              mask |= MCR_DTR;
          if (*f) // we should throttle
              port->REG_mcr &= ~mask;
          else // we should no longer throttle
              port->REG_mcr |= mask;
      }
      break;
    case CYG_IO_SET_CONFIG_SERIAL_HW_FLOW_CONFIG:
        // Nothing to do because we do support both RTSCTS and DSRDTR flow
        // control.
        // Other targets would clear any unsupported flags here.
        // We just return ENOERR.
      break;
#else
#error "Flow control for w90n740 not integrated!"
#endif
#endif
    default:
        return -EINVAL;
    }
    return ENOERR;
}

// Enable the transmitter on the device
static void
w90n740_serial_start_xmit(serial_channel *chan)
{
    w90n740_serial_info *w90n740_chan = (w90n740_serial_info *)chan->dev_priv;

	enable_uart_tx_interrupt();
 
}

// Disable the transmitter on the device
static void 
w90n740_serial_stop_xmit(serial_channel *chan)
{
    w90n740_serial_info *w90n740_chan = (w90n740_serial_info *)chan->dev_priv;

    disable_uart_tx_interrupt();
}

// Serial I/O - low level interrupt handler (ISR)
static cyg_uint32 
w90n740_serial_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    w90n740_serial_info *w90n740_chan = (w90n740_serial_info *)chan->dev_priv;

    cyg_drv_interrupt_mask(w90n740_chan->int_num);
	cyg_drv_interrupt_acknowledge(w90n740_chan->int_num);

   	return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level interrupt handler (DSR)
static void       
w90n740_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    w90n740_serial_info *w90n740_chan = (w90n740_serial_info *)chan->dev_priv;
    volatile unsigned char isr = GET_INTERRUPT_STATUS(w90n740_chan->base) ;
    volatile unsigned int status;
    int received = 0;

again:
	isr = isr&0x0E;
    if((isr == UART_IIR_DR) || (isr == UART_IIR_THRE) || (isr == UART_IIR_TOUT)) {
        if (isr == UART_IIR_THRE) {
            (chan->callbacks->xmt_char)(chan);//serial_xmt_char
        } else if(isr == UART_IIR_DR)
        {
        	int i = 0;
        	for (i=0; i<8; i++)
			{
				chars_rx++ ;
            	(chan->callbacks->rcv_char)(chan, GET_CHAR(w90n740_chan->base));
			}
			while(RX_DATA(GET_STATUS(w90n740_chan->base)))
	    	{
	    		chars_rx++ ;
            	(chan->callbacks->rcv_char)(chan, GET_CHAR(w90n740_chan->base));
            }
#ifdef CYGPKG_NET_BLUEZ_STACK
			received = 1;
#endif
        }
        else if(isr == UART_IIR_TOUT) 
        {
	    	while(RX_DATA(GET_STATUS(w90n740_chan->base)))
	    	{
	    		chars_rx++ ;
            	(chan->callbacks->rcv_char)(chan, GET_CHAR(w90n740_chan->base));
            }
#ifdef CYGPKG_NET_BLUEZ_STACK
            received = 1;
#endif
        }
        else
        {
        	//error
        }
		isr = GET_INTERRUPT_STATUS(w90n740_chan->base) ;
		if(!(isr & UART_IIR_NIP))
			goto again;

#ifdef CYGPKG_NET_BLUEZ_STACK
		if(received)
		{
			chan->receive = 1;
    	    (chan->callbacks->rcv_char)(chan, 0);
        	chan->receive = 0;
        }
#endif
	}
   // cyg_semaphore_post(&serial740_sem);//clyu
    cyg_drv_interrupt_unmask(w90n740_chan->int_num);
}
#endif

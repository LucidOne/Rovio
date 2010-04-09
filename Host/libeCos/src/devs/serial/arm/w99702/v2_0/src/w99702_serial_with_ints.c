//==========================================================================
//
//      io/serial/arm/w99702_serial_with_ints.c
//
//      WINBOND W90702 Serial I/O Interface Module (interrupt driven)
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
// Contributors: winbond
// Date:         4/30/2004
// Purpose:      W99702 Serial I/O module (interrupt driven)
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

#ifdef CYGPKG_IO_SERIAL_ARM_W99702
#include "w99702_serial.h"

typedef struct w99702_serial_info {
    CYG_ADDRWORD   base;
    CYG_WORD       int_num;
    cyg_interrupt  serial_interrupt;
    cyg_handle_t   serial_interrupt_handle;
} w99702_serial_info;

static bool w99702_serial_init(struct cyg_devtab_entry *tab);
static bool w99702_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo w99702_serial_lookup(struct cyg_devtab_entry **tab, 
                                   struct cyg_devtab_entry *sub_tab,
                                   const char *name);
static unsigned char w99702_serial_getc(serial_channel *chan);
static Cyg_ErrNo w99702_serial_set_config(serial_channel *chan, cyg_uint32 key,
					      const void *xbuf, cyg_uint32 *len);
static void w99702_serial_start_xmit(serial_channel *chan);
static void w99702_serial_stop_xmit(serial_channel *chan);

static cyg_uint32 w99702_serial_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       w99702_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);

static SERIAL_FUNS(w99702_serial_funs, 
                   w99702_serial_putc, 
                   w99702_serial_getc,
                   w99702_serial_set_config,
                   w99702_serial_start_xmit,
                   w99702_serial_stop_xmit
    );


#ifdef CYGPKG_IO_SERIAL_ARM_W99702_SERIAL0
//#define UART_BASE0           COM_TX	 /*  UART 0 */
static w99702_serial_info w99702_serial_info0 = {UART_BASE0, INT_UART0INT};
#if CYGNUM_IO_SERIAL_ARM_W99702_SERIAL0_BUFSIZE > 0
static unsigned char w99702_serial_out_buf0[CYGNUM_IO_SERIAL_ARM_W99702_SERIAL0_BUFSIZE];
static unsigned char w99702_serial_in_buf0[CYGNUM_IO_SERIAL_ARM_W99702_SERIAL0_BUFSIZE];
unsigned char bluetooth_buf[CYGNUM_IO_SERIAL_ARM_W99702_SERIAL0_BUFSIZE];

static serial_channel w99702_serial_channel0 = {
    &w99702_serial_funs,
    &cyg_io_serial_callbacks,
    &(w99702_serial_info0),
    CYG_SERIAL_INFO_INIT(CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_W99702_SERIAL0_BAUD), CYG_SERIAL_STOP_DEFAULT, CYG_SERIAL_PARITY_DEFAULT, CYG_SERIAL_WORD_LENGTH_DEFAULT, CYG_SERIAL_FLAGS_DEFAULT),
    false,
    CBUF_INIT(&w99702_serial_out_buf0[0], sizeof(w99702_serial_out_buf0)),
    CBUF_INIT(&w99702_serial_in_buf0[0],  sizeof(w99702_serial_in_buf0))
};

#else
static SERIAL_CHANNEL(w99702_serial_channel0,
                      w99702_serial_funs, 
                      w99702_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_W99702_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

#pragma arm section rwdata = "devtab"
DEVTAB_ENTRY(w99702_serial_io0, 
             CYGDAT_IO_SERIAL_ARM_W99702_SERIAL0_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             w99702_serial_init, 
             w99702_serial_lookup,     // Serial driver may need initializing
             &w99702_serial_channel0
    );
#pragma arm section rwdata

#endif //  CYGPKG_IO_SERIAL_ARM_W99702_SERIAL0

#ifdef CYGPKG_IO_SERIAL_ARM_W99702_SERIAL1
//#define UART_BASE0           COM_TX	 /*  UART 0 */
static w99702_serial_info w99702_serial_info1 = {UART_BASE1, INT_UART1INT};
#if CYGNUM_IO_SERIAL_ARM_W99702_SERIAL1_BUFSIZE > 0
static unsigned char w99702_serial_out_buf1[CYGNUM_IO_SERIAL_ARM_W99702_SERIAL1_BUFSIZE];
static unsigned char w99702_serial_in_buf1[CYGNUM_IO_SERIAL_ARM_W99702_SERIAL1_BUFSIZE];
unsigned char bluetooth_buf1[CYGNUM_IO_SERIAL_ARM_W99702_SERIAL1_BUFSIZE];

static serial_channel w99702_serial_channel1 = {
    &w99702_serial_funs,
    &cyg_io_serial_callbacks,
    &(w99702_serial_info1),
    CYG_SERIAL_INFO_INIT(CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_W99702_SERIAL1_BAUD), CYG_SERIAL_STOP_DEFAULT, CYG_SERIAL_PARITY_DEFAULT, CYG_SERIAL_WORD_LENGTH_DEFAULT, CYG_SERIAL_FLAGS_DEFAULT),
    false,
    CBUF_INIT(&w99702_serial_out_buf1[0], sizeof(w99702_serial_out_buf1)),
    CBUF_INIT(&w99702_serial_in_buf1[0],  sizeof(w99702_serial_in_buf1))
};

#else
static SERIAL_CHANNEL(w99702_serial_channel1,
                      w99702_serial_funs, 
                      w99702_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_W99702_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

#pragma arm section rwdata = "devtab"
DEVTAB_ENTRY(w99702_serial_io1, 
             CYGDAT_IO_SERIAL_ARM_W99702_SERIAL1_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             w99702_serial_init, 
             w99702_serial_lookup,     // Serial driver may need initializing
             &w99702_serial_channel1
    );
#pragma arm section rwdata

#endif //  CYGPKG_IO_SERIAL_ARM_W99702_SERIAL1

struct cyg_devtab_entry *serial_driver;

#define GET_INTERRUPT_STATUS(p)           IO_READ(p + COM_IIR)
#define GET_STATUS(p)		          (IO_READ(p + COM_LSR))
#if 1	//xhchen - MCU debug
char hi_uart_log_read(int blog, char ch);
char hi_uart_log_write(int blog, char ch);
#define GET_CHAR(p)		          (hi_uart_log_read((p==UART_BASE1),IO_READ(p + COM_RX)))
#define PUT_CHAR(p, c)		          (IO_WRITE((p + COM_TX), hi_uart_log_write((p==UART_BASE1),(c))))
#else
#define GET_CHAR(p)		          (IO_READ(p + COM_RX))
#define PUT_CHAR(p, c)		          (IO_WRITE((p + COM_TX), (c)))
#endif
#define IO_READ(p)                        ((*(unsigned int volatile*)(p)) & 0xFF)
#define IO_WRITE(p, c)                    (*(unsigned int volatile *)(p) = (c))
#define RX_DATA(s)		          (((s) & UART_LSR_DR) != 0)
#if 0	//xhchen 2008-06-03
#define TX_READY(s)		          (((s) & UART_LSR_THRE) != 0)
#else
#define TX_READY(s)		          (((s) & UART_LSR_TEMT) != 0)
#endif
#define TX_EMPTY(p)		          ((GET_STATUS(p) & UART_LSR_THRE) != 0)

// debugging help
static int chars_rx = 0 ;
static int chars_tx = 0 ;

// Internal function to actually configure the hardware to desired baud rate, etc.
static bool
w99702_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    w99702_serial_info *w99702_chan = (w99702_serial_info *)chan->dev_priv;
    unsigned int port = (unsigned int)w99702_chan->base;
    unsigned short baud_divisor = baudrate_div(select_baud[new_config->baud]);
    unsigned int _lcr ;

//diag_printf("baud index=%d, baud=%d, baud_divisor=%d\n",new_config->baud,select_baud[new_config->baud],baud_divisor);
    // first, disable everything 
    disable_uart_all_interrupt(port);
	
	//IO_WRITE(port + COM_FCR, IO_READ(port + COM_FCR) | 0x06);//reset Tx/Rx FIFO
    // Set baud rate 
    IO_WRITE(port + COM_LCR, 0x80); /* select divisor latch registers */
    IO_WRITE(port + COM_DLL, (baud_divisor&0xFF));
    IO_WRITE(port + COM_DLM, ((baud_divisor>>8)&0xFF));
	IO_WRITE(port + COM_LCR, IO_READ(port + COM_LCR) & 0x7F);
	
    // ----------v----------v----------v----------v---------- 
    // NOTE: MUST BE WRITTEN LAST (AFTER UARTLCR_M & UARTLCR_L) 
    // ----------^----------^----------^----------^----------
    _lcr = 
      select_word_length[new_config->word_length - CYGNUM_SERIAL_WORD_LENGTH_5] | 
      select_stop_bits[new_config->stop] |
      select_parity[new_config->parity] ;

    IO_WRITE(port + COM_LCR, _lcr);
    
	
    // save the configuration
    if (new_config != &chan->config) {
        chan->config = *new_config;
    }
	
	//clyu 2005/04/12
	enable_uart_rx_interrupt(port);
	
    // success
    return true;
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
w99702_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    w99702_serial_info *w99702_chan = (w99702_serial_info *)chan->dev_priv;
    cyg_uint32 clock_config;
    cyg_uint32 volatile i;
    
	serial_driver = tab;
  
    clock_config = CSR_READ(CLKCON);
    
    if(w99702_chan->base == UART0_BA)
    {
		CSR_WRITE(PADC0, CSR_READ(PADC0) | 0x00040000);//enable UART0 pin
	    if((clock_config & 0x30) != 0x10)
    	{
			CSR_WRITE(CLKCON, (clock_config & 0xFFFFFFCF) | 0x1010);//enable APB Bridge/UART2 clock disable UART1//0xFFFFFFCF
			for(i=0;i<0x1000;i++);
		}
	}
	else//UART1
	{
		CSR_WRITE(PADC0, CSR_READ(PADC0) | 0x00080000);//enable UART1 pin
		if((clock_config & 0x0C) != 0x04)
    	{
			CSR_WRITE(CLKCON, (clock_config & 0xFFFFFFF3) | 0x1004);//enable APB Bridge/UART2 clock disable UART1//0xFFFFFFCF
			for(i=0;i<0x1000;i++);
		}
	}
    
//#ifdef CYGDBG_IO_INIT
    diag_printf("w99702 SERIAL init - dev: %x.%d\n", w99702_chan->base, w99702_chan->int_num);
//#endif
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    if (chan->out_cbuf.len != 0) {
        cyg_drv_interrupt_create(w99702_chan->int_num,
                                 1,	//99,                     // Priority - what goes here?
                                 (cyg_addrword_t)chan,   //  Data item passed to interrupt handler
                                 w99702_serial_ISR,
                                 w99702_serial_DSR,
                                 &w99702_chan->serial_interrupt_handle,
                                 &w99702_chan->serial_interrupt);
        cyg_drv_interrupt_attach(w99702_chan->serial_interrupt_handle);
        cyg_drv_interrupt_unmask(w99702_chan->int_num);
        cyg_interrupt_enable();
    }
    w99702_serial_config_port(chan, &chan->config, true);
    IO_WRITE(w99702_chan->base + COM_FCR, IO_READ(w99702_chan->base + COM_FCR) | 0x06);//reset Tx/Rx FIFO
    
    if(w99702_chan->base == UART0_BA)
    	IO_WRITE(w99702_chan->base + COM_FCR, 0x81);/* 8-byte FIFO trigger level*/
    else
    	IO_WRITE(w99702_chan->base + COM_FCR, 0x51);/* 46-byte FIFO trigger level*/
    IO_WRITE(w99702_chan->base + COM_MCR, 0x00);
	IO_WRITE(w99702_chan->base + COM_TOR, 0x80 + 0x10);
	IO_WRITE(w99702_chan->base + COM_IER, 0x10);//avoid clock hold when connect to Multi-ICE
    enable_uart_rx_interrupt(w99702_chan->base);
    
    return true;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
w99702_serial_lookup(struct cyg_devtab_entry **tab, 
                  struct cyg_devtab_entry *sub_tab,
                  const char *name)
{
    serial_channel *chan = (serial_channel *)(*tab)->priv;
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    return ENOERR;
}

static int sendcnt = 16;
// Send a character to the device output buffer.
// Return 'true' if character is sent to device
static inline bool
w99702_serial_putc(serial_channel *chan, unsigned char c)
{
    w99702_serial_info *w99702_chan = (w99702_serial_info *)chan->dev_priv;
#if 1	//xhchen 2008-06-03
    if(!TX_READY( GET_STATUS(w99702_chan->base) ))
    	return false;

    PUT_CHAR(w99702_chan->base, c);
    return true;

#elif 1
    while(!TX_READY( GET_STATUS(w99702_chan->base) ));
    PUT_CHAR(w99702_chan->base, c);
    return true;
#else
    if(sendcnt--)
    {
    	while(!TX_READY( GET_STATUS(w99702_chan->base) ));
		PUT_CHAR(w99702_chan->base, c);
		chars_tx++ ;
    	return true;
    }
    return false;
#endif
}

// Fetch a character from the device input buffer, waiting if necessary
static inline unsigned char 
w99702_serial_getc(serial_channel *chan)
{
    unsigned char c;
    w99702_serial_info *w99702_chan = (w99702_serial_info *)chan->dev_priv;
    unsigned int status ;

    do {
        status = GET_STATUS(w99702_chan->base) ;
    } while (!RX_DATA(status)) ;                   // Wait for char

    //chars_rx++ ;

    // get it 
    c = GET_CHAR(w99702_chan->base) ;
    return c;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
w99702_serial_set_config(serial_channel *chan, cyg_uint32 key, const void *xbuf,
                      cyg_uint32 *len)
{
    w99702_serial_info *w99702_chan = (w99702_serial_info *)chan->dev_priv;

    switch (key) {
    case CYG_IO_SET_CONFIG_SERIAL_INFO:
      {
        cyg_serial_info_t *config = (cyg_serial_info_t *)xbuf;
        if ( *len < sizeof(cyg_serial_info_t) ) {
            return -EINVAL;
        }
        *len = sizeof(cyg_serial_info_t);

        if ( true != w99702_serial_config_port(chan, config, false) )
            return -EINVAL;
      }
      break;
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_HW
#ifdef FIXME
    case CYG_IO_SET_CONFIG_SERIAL_HW_RX_FLOW_THROTTLE:
      {
          volatile struct serial_port *port = (volatile struct serial_port *)w99702_chan->base;
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
#error "Flow control for w99702 not integrated!"
#endif
#endif
    default:
        return -EINVAL;
    }
    return ENOERR;
}

// Enable the transmitter on the device
static void
w99702_serial_start_xmit(serial_channel *chan)
{
    w99702_serial_info *w99702_chan = (w99702_serial_info *)chan->dev_priv;
	
	enable_uart_tx_interrupt(w99702_chan->base);
 
}

// Disable the transmitter on the device
static void 
w99702_serial_stop_xmit(serial_channel *chan)
{
    w99702_serial_info *w99702_chan = (w99702_serial_info *)chan->dev_priv;

    disable_uart_tx_interrupt(w99702_chan->base);
}

// Serial I/O - low level interrupt handler (ISR)
static cyg_uint32 
w99702_serial_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    w99702_serial_info *w99702_chan = (w99702_serial_info *)chan->dev_priv;

    cyg_drv_interrupt_mask(w99702_chan->int_num);
    cyg_drv_interrupt_acknowledge(w99702_chan->int_num);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level interrupt handler (DSR)
static void       
w99702_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    w99702_serial_info *w99702_chan = (w99702_serial_info *)chan->dev_priv;
    unsigned char volatile isr = GET_INTERRUPT_STATUS(w99702_chan->base) ;
    unsigned int volatile status;
    int received = 0;
    int _avail, _space_avail;
    unsigned char* _space;
    rcv_req_reply_t _block_status;

again:
	isr = isr&0x0E;
    if((isr == UART_IIR_DR) || (isr == UART_IIR_THRE) || (isr == UART_IIR_TOUT)) {
#if 1	//xhchen 2008-06-03: use the same code for two UARTs.
		if(1)
#else
        if(vector == INT_UART0INT)
#endif
        {
        	if (isr == UART_IIR_THRE) {
	        	sendcnt = 16;
    	        (chan->callbacks->xmt_char)(chan);//serial_xmt_char
        	}
	        else if(isr == UART_IIR_DR)
    	    {
#if 1	//xhchen 2008-06-03: return immediately if no data
	    		while(RX_DATA(GET_STATUS(w99702_chan->base)))
	    		{
//	    			chars_rx++ ;
	            	(chan->callbacks->rcv_char)(chan, GET_CHAR(w99702_chan->base));
    	        }
#else
        		int cnt;
        		cnt = 8;
        		chars_rx += cnt;
	    		while(cnt--)
            		(chan->callbacks->rcv_char)(chan, GET_CHAR(w99702_chan->base));
#endif            		
        	}
	        else if(isr == UART_IIR_TOUT) 
    	    {
	    		while(RX_DATA(GET_STATUS(w99702_chan->base)))
	    		{
	    			chars_rx++ ;
	            	(chan->callbacks->rcv_char)(chan, GET_CHAR(w99702_chan->base));
    	        }
#ifdef CYGPKG_NET_BLUEZ_STACK
        	    received = 1;
#endif
        	}
        	else
        	{
        		//error
        	}
        }
        else
        {
    	    if (isr == UART_IIR_THRE) {
	        	_avail = 8;
	        	_block_status = (chan->callbacks->data_xmt_req)(chan, _avail, 
                                                        &_space_avail, &_space);
				if (CYG_XMT_OK == _block_status) {
			        // Transfer the data in block(s).
			        do{
    	    		    int i = _space_avail;
			            while (i--) {
			            	while(!TX_READY( GET_STATUS(w99702_chan->base) ));
        			        PUT_CHAR(w99702_chan->base, *_space++);
                			_avail--;
            			}
			            (chan->callbacks->data_xmt_done)(chan, _space_avail);
    	    		}while (_avail > 0 && 
        	        		 (CYG_XMT_OK == (chan->callbacks->data_xmt_req)(chan, _avail,
                                                                &_space_avail, &_space)));
			    }
			    else if (CYG_XMT_DISABLED == _block_status) {
			    	 while (_space--)
            			(chan->callbacks->xmt_char)(chan);
	        	}
        	}
	        else if(isr == UART_IIR_DR)
    	    {
    	    	_avail = 46;
        		chars_rx += _avail;
            	
            	_block_status = (chan->callbacks->data_rcv_req)(chan, _avail, 
                                                        &_space_avail, &_space);
    	        
    	        if (CYG_RCV_OK == _block_status) {
		            // Transfer the data in block(s).
    		        do {
        		        int i = _space_avail;
            		    while(i--) {

    						while (!RX_DATA(GET_STATUS(w99702_chan->base))) ;
                    		*_space++ = GET_CHAR(w99702_chan->base);
		                    _avail--;
    		            }
						(chan->callbacks->data_rcv_done)(chan, _space_avail);
					} while (_avail > 0 &&
                	     (CYG_RCV_OK == (chan->callbacks->data_rcv_req)(chan, _avail, 
                                                                    &_space_avail, &_space)));
        		}
        		else
        		{
					while(_avail--)
            			(chan->callbacks->rcv_char)(chan, GET_CHAR(w99702_chan->base));
        		}
            	
        	}
	        else if(isr == UART_IIR_TOUT) 
    	    {
	    		while(RX_DATA(GET_STATUS(w99702_chan->base)))
	    		{
	    			chars_rx++ ;
	            	(chan->callbacks->rcv_char)(chan, GET_CHAR(w99702_chan->base));
    	        }
#ifdef CYGPKG_NET_BLUEZ_STACK
        	    received = 1;
#endif
        	}
        	else
        	{
        		//error
        	}
    	}
#if 0	//xhchen 2008-06-03: do not retry
		isr = GET_INTERRUPT_STATUS(w99702_chan->base) ;
		if(!(isr & UART_IIR_NIP))
			goto again;
#endif

#ifdef CYGPKG_NET_BLUEZ_STACK
		if(received)
		{
			chan->receive = 1;
    	    (chan->callbacks->rcv_char)(chan, 0);
        	chan->receive = 0;
        }
#endif
	}
   // cyg_semaphore_post(&serial702_sem);//clyu
    cyg_drv_interrupt_unmask(w99702_chan->int_num);
}
#endif

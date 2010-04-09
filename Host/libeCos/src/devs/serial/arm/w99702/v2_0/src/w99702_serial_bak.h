#ifndef CYGONCE_ARM_W99702_SERIAL_H
#define CYGONCE_ARM_W99702_SERIAL_H

// ====================================================================
//
//      w99702_serial.h
//
//      Device I/O - Description of ARM W90N740 serial hardware
//
// ====================================================================
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
// ====================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           clyu
// Contributors:        winbond
// Date:        	April 30, 2004
// Purpose:     	Internal interfaces for serial I/O drivers
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

// Description of serial ports on ARM W99702

struct serial_port {
    unsigned char _byte[32];
};

// Little-endian version
#if (CYG_BYTEORDER == CYG_LSBFIRST)

#define reg(n) _byte[n*4]

#else // Big-endian version

#define reg(n) _byte[(n*4)^3]

#endif

#define INT_UART0INT	13
#define INT_UART1INT	12
#define		UART0_BA		0x7FFC1000
#define		UART1_BA		0x7FFC0000

#define CSR_READ(p)                        (*(volatile unsigned int *)(p))
#define CSR_WRITE(p, c)                    (*(unsigned int *)(p) = (c))

#define disable_uart_tx_interrupt(base)		\
{						\
	CSR_WRITE(base + COM_IER, CSR_READ(base + COM_IER)&0x1D);	\
}

#define disable_uart_rx_interrupt(base)		\
{						\
	CSR_WRITE(base + COM_IER, CSR_READ(base + COM_IER)&0x1E);	\
}

#define enable_uart_tx_interrupt(base)		\
{						\
	if(!(CSR_READ(base + COM_IER)&0x02))	\
		CSR_WRITE(base + COM_IER, CSR_READ(base + COM_IER)|0x02);	\
}

#define enable_uart_rx_interrupt(base)		\
{						\
	CSR_WRITE(base + COM_IER, CSR_READ(base + COM_IER)|0x1);	\
}

unsigned int baudrate_div(unsigned int baudrate)
{
	unsigned int div;
	div = fMCLK_MHz/(baudrate*16);
	if ((fMCLK_MHz % (baudrate * 16)) > ((baudrate * 16) / 2))
		div++;
	div -= 2;
	return div;
}
/* -------------------------------------------------------------------------------
 *  From Winbond w99702
 * -------------------------------------------------------------------------------
 *  UART Register Offsets.
 *  
 */
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

#define UART_LSR_OE	0x02		// Overrun error
#define UART_LSR_PE	0x04		// Parity error
#define UART_LSR_FE	0x08		// Frame error
#define UART_LSR_BI	0x10		// Break detect
//#define UART_LSR_DTR	0x10		// Data terminal ready
#define UART_LSR_DR 0x01
#define UART_LSR_THRE 0x20
#define UART_IIR_DR	0x04		// Receive data ready
#define UART_IIR_THRE	0x02		// Transmit buffer register empty
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
#define UART_LCR_NSB	0x04

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

#define ARM_BAUD_460800                 0
#define ARM_BAUD_230400                 2
#define ARM_BAUD_115200                 6
#define ARM_BAUD_57600                  0x0E
#define ARM_BAUD_38400                  0x16
#define ARM_BAUD_19200                  0x2E
#define ARM_BAUD_9600                   0x5F
#define ARM_BAUD_4800                   0xC1
#define ARM_BAUD_2400                   0x184
#define ARM_BAUD_1200                   0x30B

static unsigned char select_word_length[] = {
    UART_LCR_WLEN5,    // 5 bits / word (char)
    UART_LCR_WLEN6,
    UART_LCR_WLEN7,
    UART_LCR_WLEN8
};

static unsigned char select_stop_bits[] = {
    0,
    UART_LCR_NSB,    // 1 stop bit
    UART_LCR_NSB,  // 1.5 stop bit
    UART_LCR_NSB     // 2 stop bits
};

static unsigned char select_parity[] = {
    UART_LCR_NPAR,     // No parity
    UART_LCR_PARITY|UART_LCR_EPAR,     // Even parity
    UART_LCR_PARITY|UART_LCR_OPAR,     // Odd parity
    0,     // Mark parity
    0,     // Space parity
};

#endif // CYGONCE_ARM_W99702_SERIAL_H

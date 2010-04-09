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
#include "cyg/hal/plf_io.h"

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

#define disable_uart_all_interrupt(base)	\
{						\
	CSR_WRITE(base + COM_IER, 0);	\
}

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

struct serial_baudtable
{
	unsigned int baudrate;
	unsigned int div;
};

static unsigned int select_baud[] = {
    0,               // Unused
    50,               // 50
    75,               // 75
    110,               // 110
    134,               // 134.5
    150,               // 150
    200,               // 200
    300,               // 300
    600,               // 600
    1200,   // 1200
    1800,               // 1800
    2400,   // 2400
    3600,               // 3600
    4800,   // 4800
    7200,               // 7200
    9600,   // 9600
    14400,  // 14400
    19200,  // 19200
    38400,  // 38400
    57600,  // 57600
    115200, // 115200
    230400, // 230400
};

static __inline unsigned int baudrate_div(unsigned int baudrate)
{
	unsigned int div;
	div = fMCLK_MHz/(baudrate*16);
	if ((fMCLK_MHz % (baudrate * 16)) > ((baudrate * 16) / 2))
		div++;
	div -= 2;
	return div;
}

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

#endif // CYGONCE_ARM_W90N740_SERIAL_H

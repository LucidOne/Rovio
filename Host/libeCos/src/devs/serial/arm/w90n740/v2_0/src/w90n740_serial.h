#ifndef CYGONCE_ARM_W90N740_SERIAL_H
#define CYGONCE_ARM_W90N740_SERIAL_H

// ====================================================================
//
//      w90n740_serial.h
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
// Date:        	November 26, 2003
// Purpose:     	Internal interfaces for serial I/O drivers
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

// Description of serial ports on ARM W90N740

struct serial_port {
    unsigned char _byte[32];
};

// Little-endian version
#if (CYG_BYTEORDER == CYG_LSBFIRST)

#define reg(n) _byte[n*4]

#else // Big-endian version

#define reg(n) _byte[(n*4)^3]

#endif

#define INT_UARTINT	6
#define Base_Addr	0xFFF00000

#define CSR_READ(p)                        (*(volatile unsigned int *)(p))
#define CSR_WRITE(p, c)                    (*(unsigned int *)(p) = (c))

#define disable_uart_all_interrupt(line)	\
{						\
	CSR_WRITE(COM_IER, 0);	\
}
#define disable_uart_tx_interrupt(line)		\
{						\
	CSR_WRITE(COM_IER, CSR_READ(COM_IER)&0x1D);	\
}

#define disable_uart_rx_interrupt(line)		\
{						\
	CSR_WRITE(COM_IER, CSR_READ(COM_IER)&0x1E);	\
}

#define enable_uart_tx_interrupt(line)		\
{						\
	if(!((CSR_READ(COM_IER)&0x02)))	\
		CSR_WRITE(COM_IER, CSR_READ(COM_IER)|0x02);	\
}

#define enable_uart_rx_interrupt(line)		\
{						\
	CSR_WRITE(COM_IER, CSR_READ(COM_IER)|0x1);	\
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


struct serial_baudtable uart_baudrate[] =
{
	{  1200, 0x30B},
	{  2400, 0x184},
	{  4800, 0xC1},
	{  9600, 0x5F},
	{ 19200, 0x2E},
	{ 38400, 0x16},
	{ 57600, 0x0E},
	{115200, 0x06},
	{230400, 0x02},
	{460860, 0x00}
};
unsigned int baudrate_div(unsigned int baudrate)
{
	int i;
	int len = sizeof(uart_baudrate)/sizeof(struct serial_baudtable);
	for(i = 0; i < len; i++)
		if(uart_baudrate[i].baudrate == baudrate)
			return uart_baudrate[i].div;
	return 0;
}
/* -------------------------------------------------------------------------------
 *  From Winbond w90n740
 * -------------------------------------------------------------------------------
 *  UART Register Offsets.
 *  
 */
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
    0,    // 1 stop bit
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

extern cyg_sem_t serial740_sem;//clyu

#endif // CYGONCE_ARM_W90N740_SERIAL_H

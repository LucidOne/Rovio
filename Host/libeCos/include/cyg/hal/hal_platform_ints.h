#ifndef CYGONCE_HAL_PLATFORM_INTS_H
#define CYGONCE_HAL_PLATFORM_INTS_H
//==========================================================================
//
//      hal_platform_ints.h
//
//      
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
// Author(s):    gthomas
// Contributors: gthomas, jskov
//               Grant Edwards <grante@visi.com>
// Date:         2001-07-31
// Purpose:      
// Description:  
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#ifndef  CYG_HAL_CPUTYPE_W99702
#define CYGNUM_HAL_INTERRUPT_WDTINT   1
#define CYGNUM_HAL_INTERRUPT_EXT0     2
#define CYGNUM_HAL_INTERRUPT_EXT1     3
#define CYGNUM_HAL_INTERRUPT_EXT2     4
#define CYGNUM_HAL_INTERRUPT_EXT3     5
#define CYGNUM_HAL_INTERRUPT_UART0_TX 6
#define CYGNUM_HAL_INTERRUPT_UART0_RX 6
# if !defined(CYG_HAL_CPUTYPE)
# error CYG_HAL_CPUTYPE not defined
# endif
#define CYGNUM_HAL_INTERRUPT_TIMER0   7
#define CYGNUM_HAL_INTERRUPT_TIMER1   8
#define CYGNUM_HAL_INTERRUPT_USB0	  9
#define CYGNUM_HAL_INTERRUPT_USB1    10
#define CYGNUM_HAL_INTERRUPT_Reserved0  11
#define CYGNUM_HAL_INTERRUPT_Reserved1  12
#define CYGNUM_HAL_INTERRUPT_ETH_BDMA_TX  13
#define CYGNUM_HAL_INTERRUPT_ETH_BDMA_RX  14
#define CYGNUM_HAL_INTERRUPT_ETH_MAC_TX0   13
#define CYGNUM_HAL_INTERRUPT_ETH_MAC_RX0   15
#define CYGNUM_HAL_INTERRUPT_ETH_MAC_TX1   14
#define CYGNUM_HAL_INTERRUPT_ETH_MAC_RX1   16
#define CYGNUM_HAL_INTERRUPT_GDMAINT0   17
#define CYGNUM_HAL_INTERRUPT_GDMAINT1   18

#define CYGNUM_HAL_ISR_MIN                        2
#define CYGNUM_HAL_ISR_MAX                        0x7FFFE
#define CYGNUM_HAL_ISR_COUNT                      18

#else //W99702
#define CYGNUM_HAL_INTERRUPT_WDTINT   1
#define CYGNUM_HAL_INTERRUPT_EXT0     2
#define CYGNUM_HAL_INTERRUPT_EXT1     3
#define CYGNUM_HAL_INTERRUPT_EXT2     4
#define CYGNUM_HAL_INTERRUPT_EXT3     5
#define CYGNUM_HAL_INTERRUPT_UART0_TX 13
#define CYGNUM_HAL_INTERRUPT_UART0_RX 13
#define CYGNUM_HAL_INTERRUPT_UART1_TX 12
#define CYGNUM_HAL_INTERRUPT_UART1_RX 12
# if !defined(CYG_HAL_CPUTYPE)
# error CYG_HAL_CPUTYPE not defined
# endif
#define CYGNUM_HAL_INTERRUPT_ADOP   6
#define CYGNUM_HAL_INTERRUPT_ADOR   7
#define CYGNUM_HAL_INTERRUPT_DSP	8
#define CYGNUM_HAL_INTERRUPT_VCE	9
#define CYGNUM_HAL_INTERRUPT_HIC	10
#define CYGNUM_HAL_INTERRUPT_LCM	11

#define CYGNUM_HAL_INTERRUPT_TIMER0		15
#define CYGNUM_HAL_INTERRUPT_TIMER1		14
#define CYGNUM_HAL_INTERRUPT_JPEG		16
#define CYGNUM_HAL_INTERRUPT_MPEG		17
#define CYGNUM_HAL_INTERRUPT_2DGE		18
#define CYGNUM_HAL_INTERRUPT_VPE		19
#define CYGNUM_HAL_INTERRUPT_ME			20
#define CYGNUM_HAL_INTERRUPT_FMI		21
#define CYGNUM_HAL_INTERRUPT_USB		22
#define CYGNUM_HAL_INTERRUPT_GPIO	    23
#define CYGNUM_HAL_INTERRUPT_USI	    24
#define CYGNUM_HAL_INTERRUPT_FI2C	    25
#define CYGNUM_HAL_INTERRUPT_PWR	    26


#define CYGNUM_HAL_ISR_MIN                        2
#define CYGNUM_HAL_ISR_MAX                        0x7FFFFFE
#define CYGNUM_HAL_ISR_COUNT                      27

#endif

// The vector used by the Real time clock

#define CYGNUM_HAL_INTERRUPT_RTC                  CYGNUM_HAL_INTERRUPT_TIMER0

//----------------------------------------------------------------------------
// Reset.
#define HAL_PLATFORM_RESET()

#define HAL_PLATFORM_RESET_ENTRY 0x00000000

#endif // CYGONCE_HAL_PLATFORM_INTS_H

//==========================================================================
//
//      sdram.c
//
//      SDRAM programming
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
// Date:         2003-12-23
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include "pkgconf/system.h"

#include "cyg/hal/hal_arch.h"
#include "cyg/hal/hal_intr.h"

#include "cyg/hal/hal_cache.h"

#include "pkgconf/io_sdram.h"
#include "string.h"
#define  _FLASH_PRIVATE_
#include "cyg/io/block.h"

// When this flag is set, do not actually jump to the relocated code.
// This can be used for running the function in place (RAM startup only),
// allowing calls to diag_printf() and similar.

struct sdram_info sdram_info;
#define CYGNUM_IO_SDRAM_SECTOR_SIZE	512

int
sdram_init()//void *work_space, int work_space_size)
{
    int err;

    if (sdram_info.init) return SDRAM_ERR_OK;
    //sdram_info.work_space = work_space;
    //sdram_info.work_space_size = work_space_size;
    if ((err = sdram_hwr_init()) != SDRAM_ERR_OK) {
        return err;
    }
    sdram_info.block_size = CYGNUM_IO_SDRAM_SECTOR_SIZE;
    sdram_info.blocks = CYGNUM_IO_SDRAM_BLOCK_LENGTH_1/sdram_info.block_size;
    sdram_info.block_mask = ~(sdram_info.block_size-1);
    sdram_info.start = CYGNUM_IO_SDRAM_BLOCK_BASE_1;
    sdram_info.end	= (void*)((cyg_uint32)sdram_info.start + CYGNUM_IO_SDRAM_BLOCK_LENGTH_1);
    sdram_info.init = 1;
    return SDRAM_ERR_OK;
}


char *
sdram_errmsg(int err)
{
    switch (err) {
    case SDRAM_ERR_OK:
        return "No error - operation complete";
    default:
        return "Unknown error";
    }
}

// EOF io/block/..../sdram.c

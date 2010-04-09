#ifndef CYGONCE_IO_PIPELIB_H
#define CYGONCE_IO_PIPELIB_H
//==========================================================================
//
//      pipelib.h
//
//      A pipe driver library
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Waverider Communications Inc.
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
// Author(s): Alex Paulis   
// Contributors: 
// Date: Nov 8, 2004
// Purpose: Provide a simple pipe driver that allows data to transfered from
//          one thread to another via a file (implemented by the pipe driver).      
// Description: Access to the data in the driver is FIFO. Selects are provided
//              for both reads and writes. The read returns when there is at
//              one character in the buffer. The write returns when all of the
//              data has been written.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include "cyg/infra/cyg_type.h"
#include "cyg/io/devtab.h"          // device stuff
#include "cyg/fileio/fileio.h"      // select - selinfo

// ----------------------------------------------------------------------------
// PRIVATE information.  The following info should not be used by caller
// ----------------------------------------------------------------------------
typedef struct pipelib_info_s
{
    void                    *storage_pv;    // pipe data storage area (buffer)
    cyg_uint32              storageSize_u32;// size of storage area in bytes
    cyg_tick_count_t        waitTime_t;     // time to wait on read or write
    volatile cyg_uint32     dataLength_u32; // number of bytes currently in buffer
    volatile cyg_uint32     readIndex_u32;  // read buffer index
    volatile cyg_uint32     writeIndex_u32; // write buffer index
    cyg_cond_t              wait_t;         // buffer access condition variable
    cyg_mutex_t             lock_t;         // buffer mutex
    struct CYG_SELINFO_TAG  selinfoRx;      // select info for reading
    struct CYG_SELINFO_TAG  selinfoTx;      // select info for writing
} PIPELIB_INFO_S;

extern cyg_devio_table_t pipelibDevioTable;

// ----------------------------------------------------------------------------
// Provided for symbol information only. Do not call directly

bool pipelib_init0 (
    struct cyg_devtab_entry *tab);

bool pipelib_init (
    struct cyg_devtab_entry *tab);

// ----------------------------------------------------------------------------
// End PRIVATE information.
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Wait forever timeout value. Do not return from a read until at least one byte
//   has been read or from a write until all of the data has been copied
#define PIPE_TIMEOUT_WAIT_FOREVER        (-1) 

// ----------------------------------------------------------------------------
// Do not wait for all of the data to be copied. Copy as much as possible and
// return.
#define PIPE_TIMEOUT_NO_WAIT             (0)

// ----------------------------------------------------------------------------
// This macro creates a pipe device in the device table.
//   The application calls this macro at file scope to instantiate a pipe 
//     instance, supplying a static buffer as storage for data in the pipe.
//   It then uses open() with the name to get a file descriptor to read,
//     write and select on the pipe.

// PIPELIB_DEVICE(
//   label,                      // a unique label (not used by app)
//   char       *name,           // name of the device, must begin with "/dev/" 
//                               //   for lookup to work. eg: "/dev/pipe0"
//   void       *storageArea,    // pointer to the storage area
//   cyg_uint32 storageAreaSize, // size of the storage area
//   cyg_tick_count_t waitTime_t)// wait this long for read or write to complete
//                               //   may also be PIPE_TIMEOUT_WAIT_FOREVER or PIPE_TIMEOUT_NO_WAIT

#define PIPELIB_DEVICE(_l_pipe,_pipeName,_pipeStorageArea,_pipeStorageAreaSize,_pipeWaitTime)\
static PIPELIB_INFO_S _l_pipe##_private =                                      \
{                                                                              \
   _pipeStorageArea,                                                           \
   _pipeStorageAreaSize,                                                       \
   _pipeWaitTime,                                                              \
};                                                                             \
DEVTAB_ENTRY(_l_pipe,_pipeName,0,&pipelibDevioTable,pipelib_init,NULL,&_l_pipe##_private);


// ----------------------------------------------------------------------------
// Discard any data that may be in the pipe

void pipelib_purge (
    int     pipelibFd);    // the file descriptor of pipe to purge


int pipe_create(int *pipe_index);
void pipe_destroy(int fd);


#endif // CYGONCE_IO_PIPELIB_H

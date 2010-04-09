/**************************************************************************
 * FreeDOS 32 BIOSDisk Driver                                             *
 * Disk drive support via BIOS                                            *
 * by Salvo Isaja                                                         *
 *                                                                        *
 * Copyright (C) 2001-2003, Salvatore Isaja                               *
 *                                                                        *
 * This is "request.c" - BIOSDisk driver request function                 *
 *                                                                        *
 *                                                                        *
 * This file is part of the FreeDOS32 BIOSDisk Driver.                    *
 *                                                                        *
 * The FreeDOS32 BIOSDisk Driver is free software; you can redistribute   *
 * it and/or modify it under the terms of the GNU General Public License  *
 * as published by the Free Software Foundation; either version 2 of the  *
 * License, or (at your option) any later version.                        *
 *                                                                        *
 * The FreeDOS32 BIOSDisk Driver is distributed in the hope that it will  *
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with the FreeDOS32 BIOSDisk Driver; see the file COPYING;        *
 * if not, write to the Free Software Foundation, Inc.,                   *
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA                *
 **************************************************************************/

#include "_errors.h"
#include "fat.h"
#include "biosdisk.h"

int biosdisk_request(DWORD function, void *params)
{
    Disk *d;
    switch (function)
    {
#ifdef FATREMOVABLE
        case FD32_MEDIACHANGE:
        {
            fd32_mediachange_t *x = (fd32_mediachange_t *) params;
            if (x->Size < sizeof(fd32_mediachange_t)) return FD32_EFORMAT;
            d = (Disk *) x->DeviceId;
            if (!(d->priv_flags & REMOVABLE)) return FD32_EINVAL;
            if (d->priv_flags & CHANGELINE) return biosdisk_mediachange(d);
            /* If disk does not support change line, always report a disk change */
            return 1;
        }
#endif
        case FD32_BLOCKREAD:
        {
            fd32_blockread_t *x = (fd32_blockread_t *) params;
            cyg_io_handle_t t;
            int len,ret;
            if (x->Size < sizeof(fd32_blockread_t)) return FD32_EFORMAT;
            d = (Disk *) x->DeviceId;
            t = (cyg_io_handle_t)d->type;
            len = x->NumBlocks * d->block_size;
            ret = cyg_io_bread(t, x->Buffer, (cyg_uint32*)&len, (x->Start*d->block_size));
            x->NumBlocks = len/d->block_size;
            return ret;
            //return biosdisk_read(d, x->Start, x->NumBlocks, x->Buffer);
        }
        #ifdef BIOSDISK_WRITE
        case FD32_BLOCKWRITE:
        {
            fd32_blockwrite_t *x = (fd32_blockwrite_t *) params;
            cyg_io_handle_t t;
            int len,ret;
            if (x->Size < sizeof(fd32_blockwrite_t)) return FD32_EFORMAT;
            d = (Disk *) x->DeviceId;
            t = (cyg_io_handle_t)d->type;
            len = x->NumBlocks * d->block_size;
            ret = cyg_io_bwrite(t, x->Buffer, (cyg_uint32*)&len, (x->Start*d->block_size));
            x->NumBlocks = len/d->block_size;
            return ret;
            //return biosdisk_write(d, x->Start, x->NumBlocks, x->Buffer);
        }
        #endif
        case FD32_BLOCKINFO:
        {
            fd32_blockinfo_t *x = (fd32_blockinfo_t *) params;
            if (x->Size < sizeof(fd32_blockinfo_t)) return FD32_EFORMAT;
            d = (Disk *) x->DeviceId;
            x->BlockSize   = d->block_size;
            x->TotalBlocks = d->total_blocks;
            x->Type        = d->type;
            //x->MultiBootId = d->multiboot_id;
            return 0;
        }
    }
    return FD32_EINVAL;
}

/**************************************************************************
 * FreeDOS 32 BIOSDisk Driver                                             *
 * Disk drive support via BIOS                                            *
 * by Salvo Isaja                                                         *
 *                                                                        *
 * Copyright (C) 2001-2003, Salvatore Isaja                               *
 *                                                                        *
 * This is "biosdisk.h" - Defines and declarations                        *
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

#ifndef __FD32_BIOSDISK_H
#define __FD32_BIOSDISK_H

#include "hw-data.h"
#include "devices.h"

/* Use the following defines to add features to the BIOSDisk driver */
#define BIOSDISK_WRITE   /* Define this to enable write support           */
#define BIOSDISK_FD32DEV /* Define this to register FD32 devices          */
#define BIOSDISK_SHOWMSG /* Define this to show messages during detection */

/* Mnemonics for the private flags of the Disk structure */
enum
{
    EXTAVAIL   = 1 << 0, /* BIOS extensions are available  */
    KNOWSPORTS = 1 << 1, /* The BIOS knows drive ports     */
    SECONDARY  = 1 << 2, /* Device is on secondary channel */
    SLAVE      = 1 << 3, /* Device is slave                */
    REMOVABLE  = 1 << 4, /* Device has removable media     */
    CHANGELINE = 1 << 5  /* Change line is supported       */
};

/* BIOSDisk device structure */
typedef struct
{
    unsigned open_count;
    DWORD    first_sector;
    unsigned bios_number;
    unsigned priv_flags;
    DWORD    bios_c, bios_h, bios_s;
    DWORD    phys_c, phys_h, phys_s;
    DWORD    block_size;   /* As defined in FD32_BLOCKINFO */
    DWORD    total_blocks; /* As defined in FD32_BLOCKINFO */
    DWORD    type;         /* As defined in FD32_BLOCKINFO */
    DWORD    multiboot_id; /* As defined in FD32_BLOCKINFO */
}
Disk;

/* Standard BIOS disk functions */
int biosdisk_stdread (const Disk *d, DWORD start, DWORD count, void *buffer);
int biosdisk_stdwrite(const Disk *d, DWORD start, DWORD count, const void *buffer);

/* IBM/MS Extended BIOS disk functions */
int biosdisk_extread (const Disk *d, DWORD start, DWORD count, void *buffer);
int biosdisk_extwrite(const Disk *d, DWORD start, DWORD count, const void *buffer);

/* Operations common to Standard and IBM/MS Extended BIOSes */
int biosdisk_read       (const Disk *d, DWORD start, DWORD count, void *buffer);
int biosdisk_write      (const Disk *d, DWORD start, DWORD count, const void *buffer);
int biosdisk_devopen    (Disk *d);
int biosdisk_devclose   (Disk *d);
int biosdisk_mediachange(const Disk *d);

/* Initialization functions */
int biosdisk_detect  (void);
int biosdisk_scanpart(const Disk *d, const char *dev_name);

/* Driver request function */
fd32_request_t biosdisk_request;

#endif /* #ifndef __FD32_BIOSDISK_H */


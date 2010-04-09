/**************************************************************************
 * FreeDOS 32 FAT Driver                                                  *
 * by Salvo Isaja                                                         *
 *                                                                        *
 * Copyright (C) 2001-2003, Salvatore Isaja                               *
 *                                                                        *
 * This is "mount.c" - Mount a FAT volume initializing its data           *
 *                                                                        *
 *                                                                        *
 * This file is part of the FreeDOS 32 FAT Driver.                        *
 *                                                                        *
 * The FreeDOS 32 FAT Driver is free software; you can redistribute it    *
 * and/or modify it under the terms of the GNU General Public License     *
 * as published by the Free Software Foundation; either version 2 of the  *
 * License, or (at your option) any later version.                        *
 *                                                                        *
 * The FreeDOS 32 FAT Driver is distributed in the hope that it will be   *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with the FreeDOS 32 FAT Driver; see the file COPYING;            *
 * if not, write to the Free Software Foundation, Inc.,                   *
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA                *
 **************************************************************************/

#include "fat.h"

//#define DEBUG

/* Define the DEBUG symbol in order to activate driver's log output */
#ifdef DEBUG
 #define LOG_PRINTF(s) s
#else
 #define LOG_PRINTF(s)
#endif


/* This function replaces the hosting block device's request funciton */
/* when a volume is mounted on it.                                    */
static fd32_request_t mounted_request;
static int mounted_request(DWORD Function, void *Params)
{
  fd32_ismounted_t *Im = (fd32_ismounted_t *) Params;
  /* If a request different from ISMOUNTED is presented to a device, */
  /* the call will fail because a file system is mounted on it.      */
  if (Function != FD32_ISMOUNTED) return FD32_ELOCKED;
  if (Im->Size < sizeof(fd32_ismounted_t)) return FD32_EFORMAT;
  Im->fsreq   = fat_request;
  Im->FSDevId = Im->DeviceId;
  return 0;
}


/* The starting sector of the root directory of a FAT12/FAT16 volume.  */
/* For FAT32 volumes, the root directory is just a file like all other */
/* directories, starting at cluster BPB_RootClus.                      */
/* Called by fat_mount.                                                */
static __inline DWORD first_root_dir_sector(tBpb *Bpb)
{
  return Bpb->BPB_ResvdSecCnt + (Bpb->BPB_NumFATs * Bpb->BPB_FATSz16);
}


/* The count of sectors occupied by the root directory. Always 0 for FAT32. */
/* Called by first_data_sector and data_clusters.                           */
static inline DWORD root_dir_sectors(tBpb *Bpb)
{
  return ((Bpb->BPB_RootEntCnt * 32) + (Bpb->BPB_BytsPerSec - 1)) / Bpb->BPB_BytsPerSec;
}


/* The start of the data region, i.e. the first sector of cluster 2. */
/* Called by fat_mount.                                              */
static inline DWORD first_data_sector(tBpb *Bpb)
{
  DWORD FATSz;

  if (Bpb->BPB_FATSz16 != 0) FATSz = Bpb->BPB_FATSz16;
                        else FATSz = Bpb->BPB_FATSz32;
  return Bpb->BPB_ResvdSecCnt + (Bpb->BPB_NumFATs * FATSz) + root_dir_sectors(Bpb);
}


/* The count of clusters in the data region of the volume. */
/* Called by determine_fat_type and fat_mount.             */
static inline DWORD data_clusters(tBpb *Bpb)
{
  DWORD FATSz, TotSec;

  /* For a FAT12/FAT16 volume, BPB_FATSz16 must be non-zero, so we don't */
  /* have to bother to pass a tFat1216_Bpb structure instead.            */
  /* The BPB_FATSz32, only present in a FAT32 BPB, will be used only if  */
  /* if the BPB_FATSz16 is zero, i.e. if the volume is FAT32.            */
  if (Bpb->BPB_FATSz16  != 0) FATSz = Bpb->BPB_FATSz16;
                         else FATSz = Bpb->BPB_FATSz32;
  if (Bpb->BPB_TotSec16 != 0) TotSec = Bpb->BPB_TotSec16;
                         else TotSec = Bpb->BPB_TotSec32;
  return (TotSec - (Bpb->BPB_ResvdSecCnt + (Bpb->BPB_NumFATs * FATSz)
         + root_dir_sectors(Bpb))) / Bpb->BPB_SecPerClus;
}


/* Determines the FAT type according to the count of data clusters */
/* starting at cluster 2, according to Micros*ft specifications.   */
/* The "<" sign and the 4085 and 65525 limits are strongly said by */
/* Micros*ft to be correct.                                        */
/* Returns: FATTYPE_FAT12, FATTYPE_FAT16 or FATTYPE_FAT32.         */
/* Called by fat_mount.                                            */
static tFatType determine_fat_type(tBpb *Bpb)
{
  DWORD CountOfClusters = data_clusters(Bpb);

  if (CountOfClusters < 4085) return FAT12;
  else
  if (CountOfClusters < 65525) return FAT16;
  else
  return FAT32;
}


/* The number of free data clusters in the FAT Volume.        */
/* NOTE: We read only the first FAT of the volume (number 0). */
/* On success returns 0 and fills FreeClusters.               */
/* Returns a negative error code on failure.                  */
/* Called by fat_mount.                                       */
static int free_cluster_count(tVolume *V, DWORD *FreeClusters)
{
  DWORD Count = 0, k, Value;
  int   Res;
  switch (V->FatType)
  {
    case FAT12 : for (k = 2; k < V->DataClusters + 2; k++)
                 {
                   if ((Res = fat12_read_entry(V, k, 0, &Value))) return Res;
                   if (Value == 0) Count++;
                 }
                 *FreeClusters = Count;
                 return 0;
    case FAT16 : for (k = 2; k < V->DataClusters + 2; k++)
                 {
                   if ((Res = fat16_read_entry(V, k, 0, &Value))) return Res;
                   if (Value == 0) Count++;
                 }
                 *FreeClusters = Count;
                 return 0;
    case FAT32 : for (k = 2; k < V->DataClusters + 2; k++)
                 {
                   if ((Res = fat32_read_entry(V, k, 0, &Value))) return Res;
                   if (Value == 0) Count++;
                 }
                 *FreeClusters = Count;
                 return 0;
  }
  return 0;
}


/* Frees all dynamic data of a volume.                        */
/* Called by fat_mount if an error occurs, or by fat_unmount. */
static void free_volume(tVolume *V)
{
  int k;

  #ifdef FATBUFFERS
  if (V->Buffers)
  {
    for (k = 0; k < V->NumBuffers; k++)
      fd32_kmem_free(V->Buffers[k].Data, V->Bpb.BPB_BytsPerSec);
    fd32_kmem_free(V->Buffers, sizeof(tBuffer) * V->NumBuffers);
  }
  #endif
  V->VolSig = 0x00000000; /* Invalidate signature against bad pointers */
  if (V) fd32_kmem_free(V, sizeof(tVolume));
}


/* Unmounts a FAT volume, closing all open files and freeing  */
/* all data structures used by the FAT driver for the volume. */
/* Returns zero on success, nonzero on failure.               */
int fat_unmount(tVolume *V)
{
  int Res;
  /* If there are open files the volume cannot be unmounted */
  if (fat_openfiles(V)) return FD32_EACCES;
  #ifdef FAT_FD32DEV
  /* Restore the original device data for the hosting block device */
  if ((Res = fd32_dev_replace(V->hBlkDev, V->blkreq, V->BlkDev)) < 0) return Res;
  #endif
  free_volume(V);
  LOG_PRINTF(printf("FAT volume succesfully unmounted.\n"));
  return 0;
}


/* Given a buffer containing the first sector of a block device, checks  */
/* if it contains a valid FAT BIOS Parameter Block. Only the fields      */
/* common to FAT12/FAT16 and FAT32 BPBs are checked. The DskSz parameter */
/* is the size in sectors of the block device.                           */
/* Returns 0 on success, or a negative error code on failure.            */
/* Called by read_bpb.                                                   */
static int check_bpb(BYTE *SecBuf, DWORD DskSz)
{
  DWORD  TotSec;
  tBpb  *Bpb = (tBpb *) SecBuf;

  /* Check for the 0xAA55 signature at offset 510 of the boot sector */
  if (*((WORD *) &SecBuf[510]) != 0xAA55)
  {
    LOG_PRINTF(printf("Boot sector signature 0xAA55 not found\n"));
    return FD32_EMEDIA;
  }

  /* Check volume size */
  if (Bpb->BPB_TotSec16 == 0)
  {
    if (Bpb->BPB_TotSec32 == 0)
    {
      LOG_PRINTF(printf("Both BPB_TotSec16 and BPB_TotSec32 are zero\n"));
      return FD32_EMEDIA;
    }
    TotSec = Bpb->BPB_TotSec32;
  }
  else
  {
    if (Bpb->BPB_TotSec32 != 0)
    {
      LOG_PRINTF(printf("Both BPB_TotSec16 and BPB_TotSec32 are nonzero\n"));
      return FD32_EMEDIA;
    }
    TotSec = Bpb->BPB_TotSec16;
  }
  if (TotSec > DskSz)
  {
    LOG_PRINTF(printf("BPB_TotSec16/32 is larger than block device size: %lu > %lu\n",
                TotSec, DskSz));
    return FD32_EMEDIA;
  }

  /* BPB_BytsPerSec can be 512, 1024, 2048 or 4096 */
  switch (Bpb->BPB_BytsPerSec)
  {
    case 512: case 1024: case 2048: case 4096: break;
    default:
      LOG_PRINTF(printf("Invalid BPB_BytsPerSec: %u\n", Bpb->BPB_BytsPerSec));
      return FD32_EMEDIA;
  }

  /* BPB_SecPerCluster can be 1, 2, 4, 8, 16, 32, 64, 128 */
  switch (Bpb->BPB_SecPerClus)
  {
    case 1: case 2: case 4: case 8: case 16: case 32: case 64: case 128: break;
    default:
      LOG_PRINTF(printf("Invalid BPB_SecPerClus: %u\n", Bpb->BPB_SecPerClus));
      return FD32_EMEDIA;
  }

  /* BPB_Media can be 0xF0, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF */
  if ((Bpb->BPB_Media != 0xF0) && (Bpb->BPB_Media < 0xF8))
  {
    LOG_PRINTF(printf("Invalid BPB_Media: %u\n", Bpb->BPB_Media));
    return FD32_EMEDIA;
  }

  return 0;
}


/* Given a volume with the blkreq and BlkDev fields initialized, reads    */
/* its boot sector, checks if the volume is FAT, determines the FAT type  */
/* and fills the passed Bpb structure with a valid FAT32 BPB to manage    */
/* the volume.                                                            */
/* Note: this function directly interfaces with the hosting block device. */
/* Returns 0 on success, or a negative error code on failure.             */
/* Called by fat_mount and fat_mediachange.                               */
static int read_bpb(tVolume *V, tBpb *Bpb, tFatType *FatType)
{
  int               Res;
  BYTE             *BootSector;
  fd32_blockinfo_t  Bi;
  fd32_blockread_t  Br;

  /* Get block size and number of blocks in Bi.BlockSize and Bi.TotalBlocks */
  Bi.Size     = sizeof(fd32_blockinfo_t);
  Bi.DeviceId = V->BlkDev;
  if ((Res = V->blkreq(FD32_BLOCKINFO, &Bi)) < 0)
  {
    LOG_PRINTF(printf("Cannot get block device parameters\n"));
    return Res;
  }
  /* Read the first block of the device */
  BootSector = (BYTE *) fd32_kmem_get(Bi.BlockSize);
  if (BootSector == NULL) return FD32_ENOMEM;
  Br.Size      = sizeof(fd32_blockread_t);
  Br.DeviceId  = V->BlkDev;
  Br.Start     = 0;
  Br.Buffer    = BootSector;
  Br.NumBlocks = 1;
  if ((Res = V->blkreq(FD32_BLOCKREAD, &Br)) < 0)
  {
    LOG_PRINTF(printf("Cannot read the boot sector\n"));
    fd32_kmem_free(BootSector, Bi.BlockSize);
    return FD32_EGENERAL;
  }
  /* Check if the BPB is valid */
  if ((Res = check_bpb(BootSector, Bi.TotalBlocks)) < 0)
  {
    fd32_kmem_free(BootSector, Bi.BlockSize);
    return Res;
  }
  /* Initialize the volume's BPB, using also the FSInfo if FAT32 */
  *FatType = determine_fat_type((tBpb *) BootSector);

  if (*FatType == FAT32)
  {
    /* Use the FAT32 BPB as is */
    memcpy((void*)Bpb, BootSector, sizeof(tBpb));
    /* Check the FAT32 version */
    if (Bpb->BPB_FSVer > 0x0000)
    {
      LOG_PRINTF(printf("Unknown FAT32 version in BPB_FSVer: %04X\n", Bpb->BPB_FSVer));
      fd32_kmem_free(BootSector, Bi.BlockSize);
      return FD32_EMEDIA;
    }
  }
  else
  {
    /* Clear the unused tFat32_Bpb fields */
    memset((void*)Bpb, 0, sizeof(tBpb));
    /* The first 36 bytes of FAT12/FAT16 BPBs and FAT32 BPBs are the same */
    memcpy((void*)Bpb, BootSector, 36);
    /* The 26 bytes of FAT12/FAT16 BPBs starting at offset 36 are the same */
    /* as the 26 bytes of FAT32 BPBs starting at offset 64                 */
    memcpy((void*)&Bpb->BS_DrvNum, BootSector + 36, 26);
  }

  fd32_kmem_free(BootSector, Bi.BlockSize);
  #if 0
  /* This is to check the volume serial number and label */
  diag_printf("BPB_TotSec16=%x\n",Bpb->BPB_TotSec16);
  diag_printf("BS_BootSig=%x\n",Bpb->BS_BootSig);
  diag_printf("Volume serial number: %08lx\n", Bpb->BS_VolID);
  diag_printf("Volume label: %c%c%c%c%c%c%c%c%c%c%c\n",
              Bpb->BS_VolLab[0], Bpb->BS_VolLab[1], Bpb->BS_VolLab[2], Bpb->BS_VolLab[3], Bpb->BS_VolLab[4],
              Bpb->BS_VolLab[5], Bpb->BS_VolLab[6], Bpb->BS_VolLab[7], Bpb->BS_VolLab[8], Bpb->BS_VolLab[9],
              Bpb->BS_VolLab[10]);
  diag_printf("Volume FS type: %c%c%c%c%c%c%c%c\n",
              Bpb->BS_FilSysType[0], Bpb->BS_FilSysType[1], Bpb->BS_FilSysType[2], Bpb->BS_FilSysType[3],
              Bpb->BS_FilSysType[4], Bpb->BS_FilSysType[5], Bpb->BS_FilSysType[6], Bpb->BS_FilSysType[7]);
  #endif
  return 0;
}


/* A simple macro that frees the volume and exits the mount function */
#define ABORT_MOUNT(V, Res) { free_volume(V); return Res; }


/* Initializes (mounts) a FAT volume.                */
/* On success, returns 0 and fills the NewV pointer. */
/* On failure, returns a negative error code.        */
int fat_mount(DWORD hDev, tVolume **NewV)
{
  int      Res, k;
  tVolume *V;
  Disk *D;
  cyg_uint32 len;
  cyg_io_sdram_getconfig_devsize_t ds;
  cyg_io_sdram_getconfig_blocksize_t bs;
  int err;

  len = sizeof (ds);
  /* Allocate the FAT volume structure */
  if (!NewV) return FD32_EINVAL;
  V = (tVolume *) fd32_kmem_get(sizeof(tVolume));
  if(!V) return -ENOMEM;
  memset(V, 0, sizeof(tVolume));
  D = (Disk *) fd32_kmem_get(sizeof(Disk));
  if(!D) return -ENOMEM;
  memset(D, 0, sizeof(Disk));
  
  err = cyg_io_get_config((cyg_io_handle_t)hDev,
				CYG_IO_GET_CONFIG_SDRAM_DEVSIZE, &ds, &len);
  if (err != ENOERR) {
		D1(printf
		   ("fat: cyg_io_get_config failed to get dev size: %d\n",
		    err));
		return err;
	}
	len = sizeof (bs);
	bs.offset = 0;
	err = cyg_io_get_config((cyg_io_handle_t)hDev,
				CYG_IO_GET_CONFIG_SDRAM_BLOCKSIZE, &bs, &len);
	if (err != ENOERR) {
		D1(printf
		   ("fat: cyg_io_get_config failed to get block size: %d\n",
		    err));
		return err;
	}
  D->block_size = bs.block_size;
  D->total_blocks = ds.dev_size/bs.block_size;
  D->type = hDev;
  
  V->BlkDev = D;
  V->blkreq = &biosdisk_request;
  V->hBlkDev = hDev;
  V->VolSig  = FAT_VOLSIG;
  LOG_PRINTF(printf("Trying to mount a FAT volume on device '%x'\n", hDev));

  /* Read the boot sector and initialize the BPB */
  Res = read_bpb(V, &V->Bpb, &V->FatType);
  if (Res < 0) ABORT_MOUNT(V, Res);
  V->DataClusters    = data_clusters(&V->Bpb);
  V->FirstDataSector = first_data_sector(&V->Bpb);

  #ifdef FATBUFFERS
  /* Initialize volume's buffers */
  V->NumBuffers = FAT_MAX_BUFFERS; /* TODO: Make it user selectable */
  V->Buffers = (tBuffer *) fd32_kmem_get(sizeof(tBuffer) * V->NumBuffers);
  if (V->Buffers == NULL) ABORT_MOUNT(V, FD32_ENOMEM);
  memset(V->Buffers, 0, sizeof(tBuffer) * V->NumBuffers);
  for (k = 0; k < V->NumBuffers; k++)
  {
    V->Buffers[k].Data = (BYTE *) fd32_kmem_get(V->Bpb.BPB_BytsPerSec);
    if (V->Buffers[k].Data == NULL) ABORT_MOUNT(V, FD32_ENOMEM);
  }
  #endif

  if (V->FatType == FAT32)
  {
    /* Read the FAT32 FSInfo Sector */
    tFSInfo *FSInfo;

    if ((Res = fat_readbuf(V, V->Bpb.BPB_FSInfo)) < 0) ABORT_MOUNT(V, Res);
    FSInfo = (tFSInfo *) V->Buffers[Res].Data;

    /* Check FAT32 FSInfo signatures */
    if (FSInfo->LeadSig  != 0x41615252) ABORT_MOUNT(V, FD32_EMEDIA);
    if (FSInfo->StrucSig != 0x61417272) ABORT_MOUNT(V, FD32_EMEDIA);
    if (FSInfo->TrailSig != 0xAA550000) ABORT_MOUNT(V, FD32_EMEDIA);
    /* Assign FSInfo parameters to FAT volume fields */
    V->FSI_Free_Count = FSInfo->Free_Count;
    V->FSI_Nxt_Free   = FSInfo->Nxt_Free;
  }
  else /* if not FAT32 */
  {
    V->FSI_Free_Count  = 0xFFFFFFFF;
    V->FSI_Nxt_Free    = 0xFFFFFFFF;
    V->FirstRootSector = first_root_dir_sector(&V->Bpb);
  }

  /* If the count of free clusters is not available or it's greater    */
  /* than the count of clusters of the volume (DataClusters + 1) we    */
  /* need to compute it by scanning the FAT.                           */
  if ((V->FSI_Free_Count == 0xFFFFFFFF) ||
      (V->FSI_Free_Count > V->DataClusters + 1))
  {
    LOG_PRINTF(printf("Free cluster count not available. Scanning the FAT...\n"));
    Res = free_cluster_count(V, &V->FSI_Free_Count);
    if (Res) ABORT_MOUNT(V, Res);
  }
  LOG_PRINTF(printf("%lu/%lu clusters available\n", V->FSI_Free_Count, V->DataClusters));
  *NewV = V;

  #ifdef FAT_FD32DEV
  /* Request function and DeviceId of the hosting block device are   */
  /* backed up in V->blkreq and V->BlkDev, so we replace the device. */
  if ((Res = fd32_dev_replace(hDev, mounted_request, V)) < 0) ABORT_MOUNT(V, Res);
  #endif

  #ifdef DEBUG
  switch (V->FatType)
  {
    case FAT12 : LOG_PRINTF(printf("FAT12 ")); break;
    case FAT16 : LOG_PRINTF(printf("FAT16 ")); break;
    case FAT32 : LOG_PRINTF(printf("FAT32 ")); break;
  }
  LOG_PRINTF(printf("volume successfully mounted on device '%x'\n", hDev));
  #endif
  return 0;
}


#ifdef FATREMOVABLE
/* Checks if the media hosting a FAT volume has been changed.                 */
/* If media has been changed, but there are open files, returns FD32_ECHANGE. */
/* If there are not open files the volume is unmounted and FD32_ENMOUNT is    */
/* returned. If media has not been changed, returns 0.                        */
/* Note: this function directly interfaces with the hosting block device.     */
int fat_mediachange(tVolume *V)
{
  int                Res;
  tFatType           FatType;
  tBpb               Bpb;
  fd32_mediachange_t Mc;

  Mc.Size     = sizeof(fd32_mediachange_t);
  Mc.DeviceId = V->BlkDev;
  Res = V->blkreq(FD32_MEDIACHANGE, &Mc);
  if (Res == FD32_EINVAL) return 0; /* Device without removable media */
  if (Res <= 0) return Res;

  Res = read_bpb(V, &Bpb, &FatType);
  LOG_PRINTF(printf("Read_bpb result: %08x\n", Res));
  if ((Res < 0) && (Res != FD32_EMEDIA)) return Res;
  /* TODO: To increase security here, we may also want to compare non-dirty
           buffers with disk sectors */
  if ((Res == FD32_EMEDIA) || (V->FatType != FatType) || memcmp(&V->Bpb, &Bpb, sizeof(tBpb)))
  {
    if (fat_openfiles(V)) return FD32_ECHANGE;
    fat_unmount(V);
    return FD32_ENMOUNT;
  }
  return 0;
}
#endif

/* Partition types supported by the FAT driver */
static const struct { BYTE Id; char *Name; } PartitionTypes[] =
{
  { 0x01, "FAT12"                       },
  { 0x04, "FAT16 up to 32 MB"           },
  { 0x06, "FAT16 over 32 MB"            },
  { 0x0B, "FAT32"                       },
  { 0x0C, "FAT32 using LBA BIOS"        },
  { 0x0E, "FAT16 using LBA BIOS"        },
  { 0x1B, "Hidden FAT32"                },
  { 0x1C, "Hidden FAT32 using LBA BIOS" },
  { 0x1E, "Hidden VFAT"                 },
  { 0, 0 }
};

/* Checks if the passed partition signature is supported by the FAT driver. */
/* Returns zero if supported, nonzero if not.                               */
int fat_partcheck(BYTE PartSig)
{
  int k;
  for (k = 0; PartitionTypes[k].Id; k++)
    if (PartitionTypes[k].Id == PartSig)
    {
      LOG_PRINTF(printf("Partition type is %02xh:%s\n", k, PartitionTypes[k].Name));
      return 0;
    }
  return -1;
}

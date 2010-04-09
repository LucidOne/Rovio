/**************************************************************************
 * FreeDOS 32 FAT Driver                                                  *
 * by Salvo Isaja                                                         *
 *                                                                        *
 * Copyright (C) 2001-2003, Salvatore Isaja                               *
 *                                                                        *
 * This is "support.c" - Small generic support functions                  *
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


/* The sector number of the first sector of a valid data cluster N, where: */
/* - the first valid data cluster is 2 (cluster 0 and 1 are reserved)      */
/* - the last valid data cluster is Vol.DataClusters + 1                   */
/* - the number of clusters (including reserved) is Vol.DataClusters + 2   */
/* Returns 0 if the N is out of range of valid clusters.                   */
/* Called by open_existing (open.c), fat_creat (creat.c),                  */
/* fat_read (readwrit.c) and fat_write (readwrit.c).                       */
DWORD fat_first_sector_of_cluster(DWORD N, tVolume *V)
{
  if ((N < 2) || (N > V->DataClusters + 1)) return 0;
  return ((N - 2) * V->Bpb.BPB_SecPerClus) + V->FirstDataSector;
}


/* Gets the current date and time in standard DOS format:          */
/*  Time bits: 15-11 hours (0-23), 10-5 minutes, 4-0 seconds/2     */
/*  Date bits: 15-9 year-1980, 8-5 month, 4-0 day                  */
/*  Hund: hundredths of second past the time in Time (0-199).      */
/* Pointers can be NULL.                                           */
/* Called by fat_creat (creat.c), fat_read, truncate_or_extend     */
/* and fat_write (readwrit.c).                                     */
/* TODO: This function works only for the FD32 target, since DJGPP's getdate and gettime return void! */
void fat_timestamps(WORD *Time, WORD *Date, BYTE *Hund)
{
	time_t CurrentSecond = cyg_timestamp();
	if(Time)
		*Time = CurrentSecond;
	if(Date)
		*Date = CurrentSecond;
	if(*Hund)
		*Hund = CurrentSecond;
#if 0
  fd32_date_t CurrentDate;
  fd32_time_t CurrentTime;

  fd32_get_date(&CurrentDate);
  fd32_get_time(&CurrentTime);

  if (Date) *Date =   (CurrentDate.Day & 0x1F)
                  +  ((CurrentDate.Mon & 0x0F) << 5)
                  + (((CurrentDate.Year - 1980) & 0xFF) << 9);
  if (Hund) *Hund = CurrentTime.Hund + CurrentTime.Sec % 2 * 100;
  if (Time) *Time = ((CurrentTime.Sec / 2) & 0x1F)
                  + ((CurrentTime.Min  & 0x3F) << 5)
                  + ((CurrentTime.Hour & 0x1F) << 11);
#endif
}


/* Moves the file pointer (the target position for read and write  */
/* operations) for a file, according to the origin:                */
/*  0 - the file pointer is moved to the offset specified          */
/*  1 - the file pointer is moved relative to its current position */
/*  2 - the file pointer is moved from the end of the file (note   */
/*      that this Origin is not valid for directories, since the   */
/*      FileSize for a directory is always zero).                  */
/* On success, returns 0 and, if Result is not NULL, fills Result  */
/* with the new absolute target position.                          */
/* This is a public driver function.                               */
int fat_lseek(tFile *F, long long int *Offset, int Origin)
{
  switch (Origin)
  {
    case FD32_SEEKSET : F->TargetPos = *Offset; break;
    case FD32_SEEKCUR : F->TargetPos += *Offset; break;
    case FD32_SEEKEND : if (F->DirEntry.Attr & FD32_ADIR) return FD32_EINVAL;
                        F->TargetPos = F->DirEntry.FileSize + *Offset;
                        break;
    default           : return FD32_EINVAL;
  }
  *Offset = F->TargetPos;
  return 0;
}


/* Gets file system informations.                             */
/* Returns 0 on success, or a negative error code on failure. */
int fat_get_fsinfo(fd32_fs_info_t *Fsi)
{
  if (Fsi->Size < sizeof(fd32_fs_info_t)) return FD32_EFORMAT;
  #ifdef FATLFN
  Fsi->Flags   = FD32_FSICASEPRES | FD32_FSIUNICODE | FD32_FSILFN;
  Fsi->NameMax = FD32_LFNMAX;
  Fsi->PathMax = FD32_LFNPMAX;
  #else
  Fsi->Flags   = 0;
  Fsi->NameMax = FD32_SFNMAX;
  Fsi->PathMax = FD32_SFNPMAX;
  #endif
  strncpy(Fsi->FSName, "FAT", Fsi->FSNameSize - 1);
  return 0;
}


/* Gets allocation informations on a FAT volume.              */
/* Returns 0 on success, or a negative error code on failure. */
int fat_get_fsfree(fd32_getfsfree_t *F)
{
  tVolume *V;
  int      Res;

  if (F->Size < sizeof(fd32_getfsfree_t)) return FD32_EFORMAT;
  V = (tVolume *) F->DeviceId;
  if (V->VolSig != FAT_VOLSIG) return FD32_ENODEV;
  #ifdef FATREMOVABLE
  if ((Res = fat_mediachange(V)) < 0) return Res;
  #endif
  F->SecPerClus  = V->Bpb.BPB_SecPerClus;
  F->BytesPerSec = V->Bpb.BPB_BytsPerSec;
  F->AvailClus   = V->FSI_Free_Count;
  F->TotalClus   = V->DataClusters;
  return 0;
}


#if 0
/* The bytes actually allocated for the file F (usually > FileSize) */
static inline DWORD file_occupation_in_bytes(tFile *F)
{
  return clusters_amount(F->FileSize, &F->Volume->Bpb)
         * F->Volume->Bpb.BPB_BytsPerSec
         * F->Volume->Bpb.BPB_SecPerClus;
}
#endif

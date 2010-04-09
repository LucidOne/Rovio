/**************************************************************************
 * FreeDOS 32 FAT Driver                                                  *
 * by Salvo Isaja                                                         *
 *                                                                        *
 * Copyright (C) 2001-2003, Salvatore Isaja                               *
 *                                                                        *
 * This is "fat16.c" - Low level internal services to read/write entries  *
 *                     of the file allocation table in FAT16 format       *
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


/* The location of a valid cluster N into the FAT16 FAT       */
/* Returns 0 on success, or a negative error code on failure. */
/* Called by fat16_read_entry and fat16_write_entry.          */
static int fat16_cluster_entry(tVolume *V, DWORD N, int FatNum,
                               DWORD *Sector, DWORD *EntryOffset)
{
  DWORD FATSz, FATOffset;

  if (N > V->DataClusters + 1) return FD32_EISEEK;
  FATOffset = N * 2;
  if (V->Bpb.BPB_FATSz16 != 0) FATSz = V->Bpb.BPB_FATSz16;
                          else FATSz = V->Bpb.BPB_FATSz32;

  *Sector      = FatNum * FATSz + V->Bpb.BPB_ResvdSecCnt + FATOffset / V->Bpb.BPB_BytsPerSec;
  *EntryOffset = FATOffset % V->Bpb.BPB_BytsPerSec;
  return 0;
}


/* Reads the value of the specified cluster entry in a FAT16 FAT.      */
/* Returns 0 on success, or a negative error code on failure.          */
/* Called by free_cluster_count (init.c), first_free_cluster (seek.c), */
/* advance_byte_position (seek.c), truncate_or_extend (write.c) and    */
/* unlink_clusters (unlink.c).                                         */
int fat16_read_entry(tVolume *V, DWORD N, int FatNum, DWORD *Value)
{
  int   Res;
  DWORD Sector, EntryOffset;

  Res = fat16_cluster_entry(V, N, FatNum, &Sector, &EntryOffset);
  if (Res) return Res;
  if ((Res = fat_readbuf(V, Sector)) < 0) return Res;
  *Value = *((WORD *) &V->Buffers[Res].Data[EntryOffset]);
  return 0;
}


#ifdef FATWRITE
/* Writes the value of the specified cluster entry in a FAT16 FAT.          */
/* Returns 0 on success, or a negative error code on failure.               */
/* Called by allocate_and_link_new_cluster (seek.c), allocate_first_cluster */
/* (seek.c), truncate_or_extend (write.c) and fat16_unlink.                 */
int fat16_write_entry(tVolume *V, DWORD N, int FatNum, DWORD Value)
{
  int   Res;
  DWORD Sector, EntryOffset;

  Res = fat16_cluster_entry(V, N, FatNum, &Sector, &EntryOffset);
  if (Res) return Res;
  if ((Res = fat_readbuf(V, Sector)) < 0) return Res;
  *((WORD *) &V->Buffers[Res].Data[EntryOffset]) = (WORD) Value;
  if ((Res = fat_writebuf(V, Res)) < 0) return Res;
  return 0;
}


/* Unlinks (marks as free) all the clusters of the chain starting from */
/* cluster Cluster (that must be part of a valid chain) until the EOC. */
/* Called by fat_unlink and truncate_or_extend (readwrit.c).           */
int fat16_unlink(tVolume *V, DWORD Cluster)
{
  int   k, Res;
  DWORD Next;

  do
  {
    if ((Res = fat16_read_entry(V, Cluster, 0, &Next))) return Res;
    /* Update every FAT in the volume */
    for (k = 0; k < V->Bpb.BPB_NumFATs; k++)
      if ((Res = fat16_write_entry(V, Cluster, k, 0))) return Res;
    Cluster = Next;
  }
  while (!(FAT16_EOC(Next)));
  return 0;
}
#endif /* #ifdef FATWRITE */

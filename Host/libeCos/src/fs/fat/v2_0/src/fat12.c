/**************************************************************************
 * FreeDOS 32 FAT Driver                                                  *
 * by Salvo Isaja                                                         *
 *                                                                        *
 * Copyright (C) 2001-2003, Salvatore Isaja                               *
 *                                                                        *
 * This is "fat12.c" - Low level internal services to read/write entries  *
 *                     of the file allocation table in FAT12 format       *
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

/* The location of a valid cluster N into the FAT12 FAT       */
/* Returns 0 on success, or a negative error code on failure. */
/* Called by fat12_read_entry and fat12_write_entry.          */
static int fat12_cluster_entry(tVolume *V, DWORD N, int FatNum,
                               DWORD *Sector, DWORD *EntryOffset)
{
  DWORD FATSz, FATOffset;

  if (N > V->DataClusters + 1) return FD32_EISEEK;
  /* Multiply by 1.5 rounding down */
  
  FATOffset = N + (N / 2);

  if (V->Bpb.BPB_FATSz16 != 0) FATSz = V->Bpb.BPB_FATSz16;
                          else FATSz = V->Bpb.BPB_FATSz32;

  *Sector      = FatNum * FATSz + V->Bpb.BPB_ResvdSecCnt
               + FATOffset / V->Bpb.BPB_BytsPerSec;
  *EntryOffset = FATOffset % V->Bpb.BPB_BytsPerSec;
  return 0;
}


/* Reads the value of the specified cluster entry in a FAT12 FAT.      */
/* Returns 0 on success, or a negative error code on failure.          */
/* Called by free_cluster_count (init.c), first_free_cluster (seek.c), */
/* advance_byte_position (seek.c), truncate_or_extend (write.c) and    */
/* unlink_clusters (unlink.c).                                         */
int fat12_read_entry(tVolume *V, DWORD N, int FatNum, DWORD *Value)
{
  /* Since the size of a FAT12 entry is not multiple of 2, */
  /* an entry could span a sector boundary.                */
  int   NumBuf1, NumBuf2;
  int   Res;
  DWORD Sector, EntryOffset;

  Res = fat12_cluster_entry(V, N, FatNum, &Sector, &EntryOffset);
  if (Res) return Res;
  
  LOG_PRINTF(printf("fat12_read_entry: Cluster entry offset %lu Secotr %d N %d FatNum %d\n", EntryOffset,Sector,N,FatNum));
  
  /* Check if the entry doesn't span a sector boundary */
  if (EntryOffset < (DWORD) V->Bpb.BPB_BytsPerSec - 1)
  {
    if ((NumBuf1 = fat_readbuf(V, Sector)) < 0) return NumBuf1;
    //*Value = *((WORD *) &V->Buffers[NumBuf1].Data[EntryOffset]);
    *Value = V->Buffers[NumBuf1].Data[EntryOffset];
    *Value |= V->Buffers[NumBuf1].Data[EntryOffset+1] << 8;
  }
  else
  {
    LOG_PRINTF(printf("FAT12 read: Cluster entry %lu spans sector boundary\n", N));
    if ((NumBuf1 = fat_readbuf(V, Sector))     < 0) return NumBuf1;
    if ((NumBuf2 = fat_readbuf(V, Sector + 1)) < 0) return NumBuf2;
    *Value = V->Buffers[NumBuf1].Data[EntryOffset]
           + (V->Buffers[NumBuf2].Data[0] << 8);
  }
  /* If the entry number is odd, we need the highest 12 bits of the 16-bit */
  /* Value. If the entry number is even, we need the lowest 12 bits.       */
  if (N & 0x0001) *Value >>= 4;
             else *Value &= 0x0FFF;

  return 0;
}


#ifdef FATWRITE
/* Writes the value of the specified cluster entry in a FAT12 FAT.          */
/* Returns 0 on success, or a negative error code on failure.               */
/* Called by allocate_and_link_new_cluster (seek.c), allocate_first_cluster */
/* (seek.c), truncate_or_extend (write.c) and fat12_unlink.                 */
int fat12_write_entry(tVolume *V, DWORD N, int FatNum, DWORD Value)
{
  /* Since the size of a FAT12 entry is not multiple of 2, */
  /* an entry could span a sector boundary.                */
  int   NumBuf1, NumBuf2;
  int   Res;
  DWORD Sector, EntryOffset;


  Res = fat12_cluster_entry(V, N, FatNum, &Sector, &EntryOffset);
  if (Res) return Res;
  /* Check if the entry doesn't span a sector boundary */
  if (EntryOffset < (DWORD) V->Bpb.BPB_BytsPerSec - 1)
  {
    if ((NumBuf1 = fat_readbuf(V, Sector)) < 0) return NumBuf1;
    /* If the entry number is odd, we need to place the entry value in    */
    /* the highest 12 bits of a 16-bit SecBuff entry. If the entry number */
    /* is even, we need to place the entry value in the lower 12 bits.    */
    if (N & 0x0001)
    {
      Value <<= 4;
      //*((WORD *) &V->Buffers[NumBuf1].Data[EntryOffset]) &= 0x000F;
      V->Buffers[NumBuf1].Data[EntryOffset] &= 0x0F;//clyu Modify for ARM core
    }
    else
    {
      Value &= 0x0FFF;
      //*((WORD *) &V->Buffers[NumBuf1].Data[EntryOffset]) &= 0xF000;
      V->Buffers[NumBuf1].Data[EntryOffset+1] &= 0xF0;
    }
    //*((WORD *) &V->Buffers[NumBuf1].Data[EntryOffset]) |= Value;
    V->Buffers[NumBuf1].Data[EntryOffset] |= Value&0x00FF;
    V->Buffers[NumBuf1].Data[EntryOffset+1] |= (Value>>8)&0x00FF;
    if ((Res = fat_writebuf(V, NumBuf1)) < 0) return Res;
  }
  else
  {
    if ((NumBuf1 = fat_readbuf(V, Sector))     < 0) return NumBuf1;
    if ((NumBuf2 = fat_readbuf(V, Sector + 1)) < 0) return NumBuf2;
    /* If the entry number is odd, we need to place the entry value in    */
    /* the highest 12 bits of a 16-bit SecBuff entry. If the entry number */
    /* is even, we need to place the entry value in the lower 12 bits.    */
    if (N & 0x0001)
    {
      Value <<= 4;
      V->Buffers[NumBuf1].Data[EntryOffset] &= 0x0F; /* LSB */
      V->Buffers[NumBuf2].Data[0]            = 0x00; /* MSB */
    }
    else
    {
      Value &= 0x0FFF;
      V->Buffers[NumBuf1].Data[EntryOffset] = (BYTE) Value; /* LSB */
      V->Buffers[NumBuf2].Data[0]          &= 0xF0; /* MSB */
    }
    V->Buffers[NumBuf1].Data[EntryOffset] |= (BYTE) Value; /* LSB */
    V->Buffers[NumBuf2].Data[0]           |= Value >> 8;   /* MSB */
    if ((Res = fat_writebuf(V, NumBuf1)) < 0) return Res;
    if ((Res = fat_writebuf(V, NumBuf2)) < 0) return Res;
    LOG_PRINTF(printf("FAT12 write: Cluster entry %lu spans sector boundary\n", N));
  }
  return 0;
}


/* Unlinks (marks as free) all the clusters of the chain starting from */
/* cluster Cluster (that must be part of a valid chain) until the EOC. */
/* Called by fat_unlink and truncate_or_extend (readwrit.c).           */
int fat12_unlink(tVolume *V, DWORD Cluster)
{
  int   k, Res;
  DWORD Next;

  do
  {
    LOG_PRINTF(printf("Unlinking FAT12 cluster %lu\n", Cluster));
    if ((Res = fat12_read_entry(V, Cluster, 0, &Next))) return Res;
    /* Update every FAT in the volume */
    for (k = 0; k < V->Bpb.BPB_NumFATs; k++)
      if ((Res = fat12_write_entry(V, Cluster, k, 0))) return Res;
    Cluster = Next;
  }
  while (!(FAT12_EOC(Next)));
  return 0;
}
#endif /* #ifdef FATWRITE */

/**************************************************************************
 * FreeDOS 32 FAT Driver                                                  *
 * by Salvo Isaja                                                         *
 *                                                                        *
 * Copyright (C) 2001-2003, Salvatore Isaja                               *
 *                                                                        *
 * This is "blockio.c" - Services to access the hosting block device      *
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

/* Buffer flags */
#define USED  0x1 /* Buffer has been used at least once */
#define DIRTY 0x2 /* Buffer contains data to be written */


/* Searches for a volume's buffer contaning the specified sector. */
/* Returns the buffer number on success, or -1 on failure.        */
/* Called by fat_readbuf.                                         */
static int search_for_buffer(tVolume *V, DWORD Sector)
{
  int k;
  for (k = 0; k < V->NumBuffers; k++)
    if (V->Buffers[k].Flags & USED)
      if (Sector == V->Buffers[k].StartingSector) return k;
  return -1;
}


/* Returns the number of the least recently used volume's buffer. */
/* Called by fat_readbuf.                                         */
static int find_least_recently_used(tVolume *V)
{
  int   k, Lru = 0;
  DWORD MinAccess = V->BufferAccess;
  for (k = 0; k < V->NumBuffers; k++)
    /* Never used buffers have LastAccess == 0, so it works */
    if (V->Buffers[k].LastAccess < MinAccess)
    {
      MinAccess = V->Buffers[k].LastAccess;
      Lru       = k;
    }
  return Lru;
}


#ifdef FATWRITE
/* Writes the content of the specified buffer in the hosting block */
/* device, marking the buffer as not dirty.                        */
/* Returns 0 on success, or a negative error code on failure.      */
/* Called by fat_flushall and fat_readbuf.                         */
static int flush_buffer(tVolume *V, int NumBuf)
{
  int               Res;
  fd32_blockwrite_t Params;

  if (V->Buffers[NumBuf].Flags & DIRTY)
  {
    Params.Size      = sizeof(fd32_blockwrite_t);
    Params.DeviceId  = V->BlkDev;
    Params.Start     = V->Buffers[NumBuf].StartingSector;
    Params.Buffer    = V->Buffers[NumBuf].Data;
    Params.NumBlocks = 1;
    Res = V->blkreq(FD32_BLOCKWRITE, &Params);
    if (Res < 0) return FD32_EGENERAL;
    V->Buffers[NumBuf].Flags &= ~DIRTY;
  }
  return 0;
}


/* Flushes all buffers of the specified volume, calling flush_buffer. */
/* Returns 0 on success, or a negative error code on failure.         */
/* Called by fat_fflush (open.c) and fat_write (readwrit.c).          */
int fat_flushall(tVolume *V)
{
  int k, Res;
  /* Update FSI_Free_Count and FSI_Nxt_Free in the FAT32 FSInfo sector */
  if (V->FatType == FAT32)
  {
    if ((Res = fat_readbuf(V, 1)) < 0) return Res;
    ((tFSInfo *) V->Buffers[Res].Data)->Free_Count = V->FSI_Free_Count;
    ((tFSInfo *) V->Buffers[Res].Data)->Nxt_Free   = V->FSI_Nxt_Free;
    if ((Res = fat_writebuf(V, Res)) < 0) return Res;
  }
  /* Flush all volume's buffers */
  for (k = 0; k < V->NumBuffers; k++)
  {
    Res = flush_buffer(V, k);
    if (Res) return Res;
  }
  return 0;
}


/* If buffers are enabled, it just marks a buffer as dirty.              */
/* If buffers are disabled, writes a buffer to the hosting block device. */
/* Returns 0 on success, or a negative error code on failure.            */
int fat_writebuf(tVolume *V, int NumBuf)
{
//LOG_PRINTF(printf("fat_writebuf(0x%x) the buffer %i written.\n", V,NumBuf));
  #ifdef FATBUFFERS
  V->Buffers[NumBuf].Flags |= DIRTY;
  return 0;
  #else
  int               Res;
  fd32_blockwrite_t Params;

  Params.Size      = sizeof(fd32_blockwrite_t);
  Params.DeviceId  = V->BlkDev;
  Params.Start     = V->Buffers[NumBuf].StartingSector;
  Params.Buffer    = V->Buffers[NumBuf].Data;
  Params.NumBlocks = 1;
//LOG_PRINTF(printf("V->blkreq=0x%x,&biosdisk_request=0x%x\n",V->blkreq,&biosdisk_request));
  Res = V->blkreq(FD32_BLOCKWRITE, &Params);
  if (Res < 0) return FD32_EGENERAL;
  return 0;
  #endif
}
#endif


/* Performs a buffered read from the hosting block device.       */
/* If the required sector is not already buffered, flushes the   */
/* least recently used buffer and uses it to load in the sector. */
/* Puts the buffer number in NumBuf.                             */
/* Returns zero on success, or a negative error code on failure. */
/* Called by fat??_read_entry and fat??_write_entry in fat??.c,  */
/* fat_read and fat_write in readwrit.c.                         */
int fat_readbuf(tVolume *V, DWORD Sector)
{
  int              Res;
  int              NumBuf;
  fd32_blockread_t Params;

  if ((NumBuf = search_for_buffer(V, Sector)) == -1)
  {
    V->BufferMiss++;
    NumBuf = find_least_recently_used(V);
    #ifdef FATWRITE
    Res = flush_buffer(V, NumBuf);
    if (Res < 0) return Res;
    #endif
    Params.Size      = sizeof(fd32_blockread_t);
    Params.DeviceId  = V->BlkDev;
    Params.Start     = Sector;
    Params.Buffer    = V->Buffers[NumBuf].Data;
    Params.NumBlocks = 1;
    Res = V->blkreq(FD32_BLOCKREAD, &Params);
    if (Res < 0) return FD32_EGENERAL;
    V->Buffers[NumBuf].StartingSector = Sector;
    V->Buffers[NumBuf].Flags |= USED;
  }
  else V->BufferHit++;
  V->Buffers[NumBuf].LastAccess = ++V->BufferAccess;
  return NumBuf;
}

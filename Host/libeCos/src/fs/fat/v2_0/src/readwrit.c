/**************************************************************************
 * FreeDOS 32 FAT Driver                                                  *
 * by Salvo Isaja                                                         *
 *                                                                        *
 * Copyright (C) 2001-2003, Salvatore Isaja                               *
 *                                                                        *
 * This is "readwrit.c" - Read or write a block of data from/to a file    *
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
#include "cyg/infra/diag.h"

//#define DEBUG

/* Define the DEBUG symbol in order to activate driver's log output */
#ifdef DEBUG
 #define LOG_PRINTF(s) s
#else
 #define LOG_PRINTF(s)
#endif

/* Mnemonics for the operation parameter of move_to_targetpos (seek.c) */
typedef enum { MOVE_ON_READ, MOVE_ON_WRITE } tMoveType;

#ifdef FATWRITE
/* Searches a volume for a free cluster in the specified range of clusters. */
/* On success, returns 0, updates V->FSI_Nxt_Free and fills Cluster.        */
/* If no free cluster is found in the specified range, returns FD32_ENOSPC. */
/* On other failure returns a negative error code.                          */
static int free_cluster_in_range(tVolume *V, DWORD From, DWORD To, DWORD *Cluster)
{
  DWORD k, Value;
  int   Res;
  switch (V->FatType)
  {
    case FAT12 : for (k = From; k < To; k++)
                 {
                   if ((Res = fat12_read_entry(V, k, 0, &Value))) return Res;
                   if (Value == 0)
                   {
                     *Cluster = V->FSI_Nxt_Free = k;
                     return 0;
                   }
                 }
                 break;
    case FAT16 : for (k = From; k < To; k++)
                 {
                   if ((Res = fat16_read_entry(V, k, 0, &Value))) return Res;
                   if (Value == 0)
                   {
                     *Cluster = V->FSI_Nxt_Free = k;
                     return 0;
                   }
                 }
                 break;
    case FAT32 : for (k = From; k < To; k++)
                 {
                   if ((Res = fat32_read_entry(V, k, 0, &Value))) return Res;
                   if (Value == 0)
                   {
                     *Cluster = V->FSI_Nxt_Free = k;
                     return 0;
                   }
                 }
                 break;
  }
  return FD32_ENOSPC;
}


/* The number of the first free data cluster found in the FAT volume.  */
/* On success, returns 0, updates V->FSI_Nxt_Free and fills Cluster.   */
/* On failure, returns a negative error code, notably FD32_ENOSPC.     */
/* Called by allocate_and_link_new_cluster and allocate_first_cluster. */
static int first_free_cluster(tVolume *V, DWORD *Cluster)
{
  int Res;

  /* If the hint in V->FSI_Nxt_Free is valid, we start searching for free */
  /* cluster from that value. Otherwise we invalidate V->FSI_Nxt_free and */
  /* search the whole volume.                                             */
  if ((V->FSI_Nxt_Free == 0xFFFFFFFF) || (V->FSI_Nxt_Free < 2)
   || (V->FSI_Nxt_Free > V->DataClusters + 1))
  {
    V->FSI_Nxt_Free = 2;
    Res = free_cluster_in_range(V, 2, V->DataClusters + 2, Cluster);
    if (Res != FD32_ENOSPC) return Res; /* Includes the case Res == 0 */
    V->FSI_Nxt_Free = 0xFFFFFFFF;
    return FD32_ENOSPC;
  }
  Res = free_cluster_in_range(V, V->FSI_Nxt_Free, V->DataClusters + 2, Cluster);
  if (Res != FD32_ENOSPC) return Res; /* Includes the case Res == 0 */
  /* If a free cluster is not found after the V->FSI_Nxt_Free cluster, */
  /* we may have it before V->FSI_Nxt_Free.                            */
  Res = free_cluster_in_range(V, 2, V->FSI_Nxt_Free, Cluster);
  if (Res != FD32_ENOSPC) return Res; /* Includes the case Res == 0 */
  V->FSI_Nxt_Free = 0xFFFFFFFF;
  return FD32_ENOSPC;
}


/* Allocates a free cluster and links it to the PrevCluster of a file,   */
/* that is expected to be set to the last allocated cluster (that marked */
/* with EOC). Then marks that new cluster entry with EOC.                */
/* On success returns 0 and fills Cluster with the new cluster number.   */
/* Returns a negative error code on failure.                             */
/* Called by move_to_targetpos.                                          */
static int allocate_and_link_new_cluster(tFile *F, DWORD *Cluster)
{
  int   k, Res;
  DWORD NewCluster;

  Res = first_free_cluster(F->V, &NewCluster);
  if (Res < 0) return Res;

  /* Update every FAT in the volume */
  for (k = 0; k < F->V->Bpb.BPB_NumFATs; k++)
  {
    /* Link the current cluster to the new cluster allocated */
    switch (F->V->FatType)
    {
      case FAT12 : Res = fat12_write_entry(F->V, F->PrevCluster, k, NewCluster); break;
      case FAT16 : Res = fat16_write_entry(F->V, F->PrevCluster, k, NewCluster); break;
      case FAT32 : Res = fat32_write_entry(F->V, F->PrevCluster, k, NewCluster); break;
    }
    if (Res < 0) return Res;

    /* Mark the new cluster allocated with EOC */
    switch (F->V->FatType)
    {
      case FAT12 : Res = fat12_write_entry(F->V, NewCluster, k, 0x0FFF); break;
      case FAT16 : Res = fat16_write_entry(F->V, NewCluster, k, 0xFFFF); break;
      case FAT32 : Res = fat32_write_entry(F->V, NewCluster, k, 0x0FFFFFFF); break;
    }
    if (Res < 0) return Res;
  }
  *Cluster = NewCluster;
  return 0;
}


/* Allocates a free cluster to be used as first cluster of a file, that    */
/* is supposed to be a zero length file. Then marks that new cluster entry */
/* with EOC.                                                               */
/* On success returns 0 and fills Cluster with the new cluster number.     */
/* Returns a negative error code on failure.                               */
/* Called by move_to_targetpos.                                            */
static int allocate_first_cluster(tFile *F, DWORD *Cluster)
{
  int   k, Res;
  DWORD NewCluster;

  Res = first_free_cluster(F->V, &NewCluster);
  if (Res < 0) return Res;

  /* Make the new cluster allocated the first cluster */
  F->DirEntry.FstClusHI = (WORD) (NewCluster >> 16);
  F->DirEntry.FstClusLO = (WORD)  NewCluster;

  /* Update every FAT in the volume */
  for (k = 0; k < F->V->Bpb.BPB_NumFATs; k++)
  {
    /* Mark the new cluster allocated with EOC */
    switch (F->V->FatType)
    {
      case FAT12 : Res = fat12_write_entry(F->V, NewCluster, k, 0x0FFF); break;
      case FAT16 : Res = fat16_write_entry(F->V, NewCluster, k, 0xFFFF); break;
      case FAT32 : Res = fat32_write_entry(F->V, NewCluster, k, 0x0FFFFFFF); break;
    }
    if (Res) return Res;
  }
  *Cluster = NewCluster;
  return 0;
}
#endif /* #ifdef FATWRITE */


/* Returns nonzero if the pointer of the file has reached the End Of File. */
/* Called by move_to_targetpos.                                            */
static int end_of_file(tFile *F)
{
  if ISROOT(F)
  {
    /* Case 1: the end of a FAT12/FAT16 root directory */
    if (F->FilePos == (long long int) F->V->Bpb.BPB_RootEntCnt * 32) return 1;
  }
  else
  {
    /* Case 2: the file is not a directory, and we are at the last byte */
    if ((F->FilePos == F->DirEntry.FileSize)
     && (!(F->DirEntry.Attr & FD32_ADIR))) return 1;

    /* Case 3: we arrived at the last cluster, that marked with EOC */
    switch (F->V->FatType)
    {
      case FAT12 : if (FAT12_EOC(F->Cluster)) return 1;
      case FAT16 : if (FAT16_EOC(F->Cluster)) return 1;
      case FAT32 : if (FAT32_EOC(F->Cluster)) return 1;
    }
  }
  return 0;
}


/* Move the file pointer forward by one byte, jumping to next sector */
/* or to next cluster if required.                                   */
/* Returns 0 on success, or a negative error code on failure.        */
/* Called by move_to_targetpos.                                      */
/* TODO: The process can be optimized jumping more than one
         byte a time, and changing fat_read and fat_write in
         order to transfer more bytes per read/write operation. */
static int advance_byte_position(tFile *F)
{
  int Res = 0;
  
  F->FilePos++;
  F->ByteInSector++;
  if (F->ByteInSector == F->V->Bpb.BPB_BytsPerSec)
  {
    F->ByteInSector = 0;
    F->SectorInCluster++;

    /* If the file is not a FAT12/FAT16 root, if we are at the cluster */
    /* boundary we jump to the next cluster of the file chain.         */
    if (!ISROOT(F) && (F->SectorInCluster == F->V->Bpb.BPB_SecPerClus))
    {
      F->SectorInCluster = 0;
      F->PrevCluster     = F->Cluster;
      switch (F->V->FatType)
      {
        case FAT12 : Res = fat12_read_entry(F->V, F->Cluster, 0, &F->Cluster); break;
        case FAT16 : Res = fat16_read_entry(F->V, F->Cluster, 0, &F->Cluster); break;
        case FAT32 : Res = fat32_read_entry(F->V, F->Cluster, 0, &F->Cluster); break;
      }
      if (Res) return Res;
      LOG_PRINTF(printf("Jump to cluster: %lu\n", F->Cluster));
    }
  }
  return 0;
}


/* Tries to move the actual position in the file up to the target   */
/* position specified by the file pointer F->TargetPos.             */
/* Returns 0 on success, or a negative error code on failure.       */
/* Called by fat_read and fat_write before every byte transfer, and */
/* by truncate_or_extend for extension.                             */
static int move_to_targetpos(tFile *F, tMoveType Op)
{
  #ifdef FATWRITE
  DWORD NewCluster;
  int   Res;
  #endif
  
  /* Check if we want to go before the beginning of the file */
  if (F->TargetPos < 0) return FD32_EISEEK;

  /* If we are beyond the target position, we restart from the beginning */
  if (F->FilePos > F->TargetPos)
  {
    F->FilePos         = 0;
    F->Cluster         = FIRSTCLUSTER(F->DirEntry);
    F->SectorInCluster = 0;
    F->ByteInSector    = 0;
  }
  /* And now perform the linear search from the current position */
  /* to the target position.                                     */
  do
  {
    /* If the file is zero bytes long it is a different story, that follows */
    if ((FIRSTCLUSTER(F->DirEntry) == 0) && F->DirEntrySector)
    {
      /* If we are reading from F, at EOF we just exit */
      if (Op == MOVE_ON_READ) return FAT_RET_EOF;

      #ifdef FATWRITE
      /* If we are writing, we increment the file size and allocate */
      /* a new cluster that will be the file's first cluster.       */
      if ((Res = allocate_first_cluster(F, &NewCluster)) < 0) return Res;
      if (!(F->DirEntry.Attr & FD32_ADIR)) F->DirEntry.FileSize++;
      F->PrevCluster = F->Cluster = NewCluster;
      F->DirEntryChanged = 1;
      if ((Res = fat_syncentry(F)) < 0) return Res;
      #endif
    }

    if (F->FilePos != F->TargetPos) advance_byte_position(F);

    if (end_of_file(F))
    {
      /* If we are reading from the file, or the file is a FAT12/FAT16 */
      /* root directory, at EOF we just exit.                          */
      if ((Op == MOVE_ON_READ) || ISROOT(F)) return FAT_RET_EOF;

      #ifdef FATWRITE
      /* But if we are writing, we increment the file size and allocate */
      /* a new cluster if we are at the last cluster boundary.          */
      if ((F->ByteInSector == 0) && (F->SectorInCluster == 0))
      {
        Res = allocate_and_link_new_cluster(F, &NewCluster);
        if (Res < 0) return Res; /* The disk may be full */
        F->Cluster = NewCluster;
      }
      if (!(F->DirEntry.Attr & FD32_ADIR)) F->DirEntry.FileSize++;
      F->DirEntryChanged = 1;
      if ((Res = fat_syncentry(F)) < 0) return Res;
      #endif
    }
  }
  while (F->FilePos != F->TargetPos);
  return 0;
}


/*** FAT POSITIONING ANALYSIS ***

FilePos is the actual byte position inside the file;
FileSize is the file size in bytes, considered only if the file is not a dir;
TargetPos is the byte position to be reached, even out of the file extension;

Eof : for a file is FilePos==FileSize
      for a directory is Cluster==EOC (End Of Clusterchain)
      for a FAT12/FAT16 root directory is FilePos==BPB_RootEntCnt*32

1) FileSize>=0, 0<=FilePos<=FileSize, TargetPos<0;
Read, Write : exit reporting invalid condition (seek before the beginning)

2) FileSize>0, 0<=FilePos<=FileSize, FilePos==TargetPos;
Read  : exit if Eof, else do nothing
Write : if Eof, extend the file incrementing FileSize and allocating a new
        cluster if the boundary of the last cluster has been reached, unless
        the file is a FAT12/FAT16 root directory (which cannot be extended);
        if not Eof, do nothing

3) FileSize>0, 0<=FilePos<=FileSize, FilePos!=TargetPos;
Read  : advance FilePos until FilePos==TargetPos, exiting if Eof if reached
Write : advance FilePos until FilePos==TargetPos; while Eof if reached the
        file is extended, incrementing FileSize and allocating a new cluster
        if the boundary of the last cluster has been reached, unless the file
        is a FAT12/FAT16 root directory (which cannot be extended)

4) FileSize==0, FilePos==0, FilePos==TargetPos==0;
Read  : the same as case 2), but we are for sure at Eof
Write : since we are for sure at Eof, we need to extend the file, which we
        know is not a FAT12/FAT16 root directory; so we allocate a new cluster
        from scratch and set it as first cluster, and mark it into the FAT
        as last cluster of the chain; increment FileSize, next we continue
        with cases 2) and 3)

5) FileSize==0, FilePos==0, FilePos!=TargetPos>=0;
Read  : similar to case 3), but we have to exit immediatly because of Eof
Write : the same as case 4)

*/


/* Reads at most Size bytes from a file into a buffer.                */
/* It may read less bytes than requested if End Of File is reached.   */
/* Returns the actual number of bytes read (zero meaning End Of File) */
/* on success, or a negative error code on failure.                   */
/* This is a public driver function.                                  */
int fat_read(tFile *F, void *Buffer, int Size)
{
  DWORD k;
  int   Res;
  int   NumBuf;

  LOG_PRINTF(printf("FAT: reading %i bytes\n", Size));
  /* Check if reading from file is allowed */
  if (((F->Mode & FD32_OACCESS) != FD32_OREAD)
   && ((F->Mode & FD32_OACCESS) != FD32_ORDWR)) return FD32_EACCES;

  for (k = 0; k < Size; k++)
  {
    Res = move_to_targetpos(F, MOVE_ON_READ);
    if (Res)
    {
      if (Res != FAT_RET_EOF) return Res;
      /* If a partial read occurred, return the number of bytes read */
      break;
    }
    /* Then we load the sector we sought and we read the byte we want */
    if ISROOT(F)
      NumBuf = fat_readbuf(F->V, F->SectorInCluster + F->V->FirstRootSector);
     else
      NumBuf = fat_readbuf(F->V, F->SectorInCluster +
                           fat_first_sector_of_cluster(F->Cluster, F->V));
    if (NumBuf < 0) return NumBuf;
    ((BYTE *) Buffer)[k] = F->V->Buffers[NumBuf].Data[F->ByteInSector];

    /* Finally we advance the file position */
    F->TargetPos++;
  }
  #ifdef FATWRITE
  /* Successful exit, whole Buffer read */
  if (((F->Mode & FD32_OACCESS) != FD32_ORDNA) && (!(F->Mode & FD32_ODIR)))
  {
    fat_timestamps(NULL, (WORD*)&F->DirEntry.LstAccDate, NULL);
    if ((Res = fat_syncentry(F)) < 0) return Res;
  }
  #endif
  LOG_PRINTF(printf("FAT: %i bytes read\n", Size));
  return k;
}


#ifdef FATWRITE
/* The amount of clusters required to store N bytes */
/* Called by block_is_too_large.                    */
static DWORD clusters_amount(DWORD N, tBpb *Bpb)
{
  return (N + Bpb->BPB_BytsPerSec * Bpb->BPB_SecPerClus - 1)
           / (Bpb->BPB_BytsPerSec * Bpb->BPB_SecPerClus);
}


/* Returns nonzero if the block of size Size to write into the file is */
/* too large to fit into the free volume space.                        */
/* NOTE: Doens't work if the file is a directory because FileSize==0.  */
/* Called by fat_write.                                                */
static inline int block_is_too_large(tFile *F, DWORD Size)
{
  DWORD FinalOccupation   = clusters_amount(F->TargetPos + Size, &F->V->Bpb);
  DWORD InitialOccupation = clusters_amount(F->DirEntry.FileSize, &F->V->Bpb);

  /* Always return success for directories */
  if (F->DirEntry.Attr & FD32_ADIR) return 0;

  LOG_PRINTF(printf("Initial occupation : %lu\n", InitialOccupation));
  LOG_PRINTF(printf("Final occupation   : %lu\n", FinalOccupation));
  LOG_PRINTF(printf("Free clusters      : %lu\n", F->V->FSI_Free_Count));

  /* If the file is not longer than before we are ok */
  if (FinalOccupation <= InitialOccupation) return 0;
  /* If the file surplus fits into the disk's free clusters we are ok */
  if (FinalOccupation - InitialOccupation <= F->V->FSI_Free_Count) return 0;
  /* Otherwise the block is too large */
  return 1;
}


/* Truncates or extends a file to the position TargetPos, as required */
/* by a write of zero bytes.                                          */
/* Returns 0 on success, or a negative error code on failure.         */
/* Called by fat_write.                                               */
static int truncate_or_extend(tFile *F)
{
  int   Res = 0;
  DWORD k;

  if (F->TargetPos == 0)
  {
    /* TargetPos == 0 is Truncate to zero length, unlinking */
    /* all file's clusters, including the first.            */
    LOG_PRINTF(("Truncating the file to zero length\n"));
    /* If the file is already zero bytes long we do nothing */
    if (FIRSTCLUSTER(F->DirEntry) == 0) return 0;
    switch (F->V->FatType)
    {
      case FAT12 : Res = fat12_unlink(F->V, FIRSTCLUSTER(F->DirEntry)); break;
      case FAT16 : Res = fat16_unlink(F->V, FIRSTCLUSTER(F->DirEntry)); break;
      case FAT32 : Res = fat32_unlink(F->V, FIRSTCLUSTER(F->DirEntry)); break;
    }
    if (Res < 0) return Res;
    F->DirEntry.FstClusHI = 0;
    F->DirEntry.FstClusLO = 0;
    F->DirEntry.FileSize  = 0;
  }
  else
  {
    /* TragetPos > 0 is Truncate/Extend at TargetPos position _excluded_,  */
    /* so we go to position TargetPos-1, since a writing seek to TargetPos */
    /* would allocate one byte more.                                       */
    F->TargetPos--;
    Res = move_to_targetpos(F, MOVE_ON_WRITE);
    F->TargetPos++;
    if (Res < 0) return Res;
    /* Now if there are clusters beyond there we unlink */
    /* them, and mark the current cluster with EOC.     */
    LOG_PRINTF(printf("Unlinking clusters beyond the truncation position\n"));
    switch (F->V->FatType)
    {
      case FAT12:
        /* Get the next cluster in k and unlink from cluster k */
        if ((Res = fat12_read_entry(F->V, F->Cluster, 0, &k)) < 0) return Res;
        if (!(FAT12_EOC(k))) fat12_unlink(F->V, k);
        /* Mark the cluster k with EOC */
        for (k = 0; k < F->V->Bpb.BPB_NumFATs; k++)
          if ((Res = fat12_write_entry(F->V, F->Cluster, k, 0x0FFF)) < 0)
            return Res;
        break;

      case FAT16:
        /* Get the next cluster in k and unlink from cluster k */
        if ((Res = fat16_read_entry(F->V, F->Cluster, 0, &k)) < 0) return Res;
        if (!(FAT16_EOC(k))) fat16_unlink(F->V, k);
        /* Mark the cluster k with EOC */
        for (k = 0; k < F->V->Bpb.BPB_NumFATs; k++)
          if ((Res = fat16_write_entry(F->V, F->Cluster, k, 0xFFFF)) < 0)
            return Res;
        break;

      case FAT32:
        /* Get the next cluster in k and unlink from cluster k */
        if ((Res = fat32_read_entry(F->V, F->Cluster, 0, &k)) < 0) return Res;
        if (!(FAT32_EOC(k))) fat32_unlink(F->V, k);
        /* Mark the cluster k with EOC */
        for (k = 0; k < F->V->Bpb.BPB_NumFATs; k++)
          if ((Res = fat32_write_entry(F->V, F->Cluster, k, 0x0FFFFFFF)) < 0)
            return Res;
        break;
    }
    /* And finally set the new file size to where we've sought */
    F->DirEntry.FileSize = (DWORD) F->TargetPos;
  }
  if (!(F->Mode & FD32_ODIR))
  {
    fat_timestamps((WORD*)&F->DirEntry.WrtTime, (WORD*)&F->DirEntry.WrtDate, NULL);
    F->DirEntry.LstAccDate = F->DirEntry.WrtDate;
    F->DirEntry.Attr      |= FD32_AARCHIV;
  }
  LOG_PRINTF(printf("File successfully truncated/extended at offset %lu\n",
              F->DirEntry.FileSize));
  fat_syncpos(F);
  F->DirEntryChanged = 1;
  return fat_syncentry(F);
}


/* Writes Size bytes from Buffer into a file.               */
/* Returns the number of bytes actually written on success, */
/* or a negative error code on failure.                     */
/* This is a public driver function.                        */
int fat_write(tFile *F, void *Buffer, int Size)
{
  int k;
  int Res;
  int NumBuf;

  LOG_PRINTF(printf("FAT write: %i bytes\n", Size));

  /* Check if writing into the file is allowed */
  if (((F->Mode & FD32_OACCESS) != FD32_OWRITE)
   && ((F->Mode & FD32_OACCESS) != FD32_ORDWR)) return FD32_EACCES;
  if (F->DirEntry.Attr & FD32_ARDONLY) return FD32_EACCES;

  /* Check if the file will fits into disk space after writing this block */
  if (block_is_too_large(F, Size)) return FD32_ENOSPC;

  /* Special case: if Size is zero, the file has to be truncated/extended */
  /* to the current target position, if it's not a FAT12/FAT16 root dir.  */
  if (Size == 0)
  {
    if ISROOT(F) return FD32_EACCES;
            else return truncate_or_extend(F);
  }
  /* Now really write */
  for (k = 0; k < Size; k++)
  {
    Res = move_to_targetpos(F, MOVE_ON_WRITE);
    /* This is possible only if the file is a FAT12/FAT16 */
    /* root directory with no more room.                  */
    if (Res == FAT_RET_EOF) return FD32_EACCES;
    else if (Res < 0) return Res;

    /* Then we load the sector we sought, change the byte */
    /* we want and write back the sector.                 */
    if ISROOT(F)
      NumBuf = fat_readbuf(F->V, F->SectorInCluster + F->V->FirstRootSector);
    else
      NumBuf = fat_readbuf(F->V, F->SectorInCluster +
                           fat_first_sector_of_cluster(F->Cluster, F->V));
    if (NumBuf < 0) return NumBuf;
    F->V->Buffers[NumBuf].Data[F->ByteInSector] = Buffer ? ((BYTE *) Buffer)[k] : 0;

    if ((Res = fat_writebuf(F->V, NumBuf)) < 0) return Res;
    /* Finally we advance the file position */
    F->TargetPos++;
  }
  /* Successful exit, whole Buffer written */
  if (!(F->Mode & FD32_ODIR))
  {
    fat_timestamps((WORD*)&F->DirEntry.WrtTime, (WORD*)&F->DirEntry.WrtDate, NULL);
    F->DirEntry.LstAccDate = F->DirEntry.WrtDate;
    F->DirEntry.Attr      |= FD32_AARCHIV;
    F->DirEntryChanged = 1;
    if ((Res = fat_syncentry(F)) < 0) return Res;
  }
  if (F->Mode & FD32_OCOMMIT)
    if ((Res = fat_flushall(F->V))) return Res;
  LOG_PRINTF(printf("%i bytes written.\n", k));
  return k;
}
#endif /* #ifdef FATWRITE */

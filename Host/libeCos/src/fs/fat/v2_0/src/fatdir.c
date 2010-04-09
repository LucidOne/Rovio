/**************************************************************************
 * FreeDOS 32 FAT Driver                                                  *
 * by Salvo Isaja                                                         *
 *                                                                        *
 * Copyright (C) 2001-2003, Salvatore Isaja                               *
 *                                                                        *
 * This is "dir.c" - Remove and create directories                        *
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

#ifdef FATWRITE
/* Removes the specified directory, checking if it's empty and */
/* calling fat_unlink (creat.c)                                */
/* Returns 0 on success, or a negative error code on failure.  */
/* This is a public driver function.                           */
int fat_rmdir(tVolume *V, char *DirName)
{
  tFile     *F;
  tDirEntry  D;
  int        Res;

  LOG_PRINTF(printf("FAT rmdir: %s\n", DirName));
  /* First we open the directory to see if removing it is a valid operation */
  Res = fat_open(V, DirName, FD32_OREAD | FD32_ODIR | FD32_OEXIST,
                 FD32_ANONE, 0, &F);
  if (Res < 0) return Res;
  
  /* Check if trying to remove the current directory */
//  if (firstcluster(F) == V->CurDirCluster) return FD32_ERROR_RMDIR_CURRENT;

  /* Check if trying to remove the root directory */
  if (((V->FatType == FAT32)
    && (FIRSTCLUSTER(F->DirEntry) == V->Bpb.BPB_RootClus))
    || ISROOT(F)) return FD32_EACCES;

  LOG_PRINTF(printf("Checking if directory is empty\n"));
  /* Check if the directory contains dot and dotdot as first two entries */
  Res = fat_read(F, (void*)&D, 32);
  if (Res < 0) { fat_close(F); return Res; }
  if (memcmp((void*)".          ", (void*)D.Name, 11)) return FD32_EACCES;
  Res = fat_read(F, (void*)&D, 32);
  if (Res < 0) { fat_close(F); return Res; }
  if (memcmp((void*)"..         ",(void*) D.Name, 11)) return FD32_EACCES;
  /* And finally check that the following entries are free */
  do
  {
    Res = fat_read(F, (void*)&D, 32);
    if (Res < 0) { fat_close(F); return Res; }
    if ((D.Name[0] != FREEENT) && (D.Name[0] != ENDOFDIR)) return FD32_EACCES;
  }
  while (D.Name[0] != ENDOFDIR);
  /* If we arrive here, it's Ok, we can unlink the directory */
  Res = fat_close(F);
  if (Res < 0) return Res;
  LOG_PRINTF(printf("Unlinking the directory file\n"));
  return fat_unlink(V, DirName, FD32_ADIR);
}


/* Creates a new directory, preparing the "." and ".." entries and */
/* filling the rest of the first sector with nulls.                */
/* Returns 0 on success, or a negative error code on failure.      */
/* This is a public driver function.                               */
int fat_mkdir(tVolume *V, char *DirName)
{
  tFile     *F;
  tDirEntry  D;
  int        Res;

  LOG_PRINTF(printf("FAT mkdir: %s\n", DirName));
  Res = fat_open(V, DirName, FD32_ORDWR | FD32_ODIR | FD32_OCREAT,
                 FD32_ADIR, 0, &F);
  if (Res < 0) return Res;

  /* Set to zero the whole cluster and rewind */
  LOG_PRINTF(printf("Zeroing cluster\n"));
  Res = fat_write(F, NULL, V->Bpb.BPB_BytsPerSec * V->Bpb.BPB_SecPerClus);
  if (Res < 0) { fat_close(F); return Res; }
  F->TargetPos = 0;
    
  /* Prepare the "dot" entry */
  LOG_PRINTF(printf("Writing dot entry\n"));
  D = F->DirEntry;
  memcpy((void*)D.Name, ".          ", 11);
  Res = fat_write(F, (void*)&D, 32);
  if (Res < 0) { fat_close(F); return Res; }
  
  /* Prepare the "dotdot" entry */
  LOG_PRINTF(printf("Writing dotdot entry\n"));
  /* If the ".." directory is the root, its first cluster */
  /* must be set to zero even if the volume is FAT32.     */
  D.FstClusHI = D.FstClusLO = 0;
  if (F->ParentFstClus != V->Bpb.BPB_RootClus)
  {
    D.FstClusHI = (WORD) (F->ParentFstClus >> 16);
    D.FstClusLO = (WORD)  F->ParentFstClus;
  }
  memcpy((void*)D.Name, "..         ", 11);
  Res = fat_write(F, (void*)&D, 32);
  if (Res < 0) { fat_close(F); return Res; }

  LOG_PRINTF(printf("Closing the new directory\n"));
  return fat_close(F);
}
#endif /* #ifdef FATWRITE */

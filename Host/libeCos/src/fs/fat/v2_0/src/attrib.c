/**************************************************************************
 * FreeDOS 32 FAT Driver                                                  *
 * by Salvo Isaja                                                         *
 *                                                                        *
 * Copyright (C) 2001-2003, Salvatore Isaja                               *
 *                                                                        *
 * This is "attrib.c" - Set/get attributes and time stamps of a file      *
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


/* Gets file attributes and time stamps.                      */
/* Returns 0 on success, or a negative error code on failure. */
/* This is a public driver function.                          */
int fat_get_attr(tFile *F, fd32_fs_attr_t *A)
{
  if (A->Size < sizeof(fd32_fs_attr_t)) return FD32_EFORMAT;
  A->Attr  = (WORD) F->DirEntry.Attr;
  A->MDate = F->DirEntry.WrtDate;
  A->MTime = F->DirEntry.WrtTime;
  A->ADate = F->DirEntry.LstAccDate;
  A->CDate = F->DirEntry.CrtDate;
  A->CTime = F->DirEntry.CrtTime;
  A->CHund = F->DirEntry.CrtTimeTenth;
  return 0;
}


#ifdef FATWRITE
/* Sets file attributes and time stamps.                      */
/* Returns 0 on success, or a negative error code on failure. */
/* This is a public driver function.                          */
int fat_set_attr(tFile *F, fd32_fs_attr_t *A)
{
  if (A->Size < sizeof(fd32_fs_attr_t)) return FD32_EFORMAT;
  F->DirEntry.Attr         = (BYTE) A->Attr;
  F->DirEntry.WrtDate      = A->MDate;
  F->DirEntry.WrtTime      = A->MTime;
  F->DirEntry.LstAccDate   = A->ADate;
  F->DirEntry.CrtDate      = A->CDate;
  F->DirEntry.CrtTime      = A->CTime;
  F->DirEntry.CrtTimeTenth = (BYTE) A->CHund;
  #ifdef FATSHARE
  F->DirEntryChanged = 1;
  return fat_syncentry(F);
  #else
  return 0;
  #endif
}
#endif

/**************************************************************************
 * FreeDOS 32 FAT Driver                                                  *
 * by Salvo Isaja                                                         *
 *                                                                        *
 * Copyright (C) 2001-2003, Salvatore Isaja                               *
 *                                                                        *
 * This is "creat.c" - File creation and deletion services                *
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

#ifdef FATWRITE

//#define DEBUG
/* Define the DEBUG symbol in order to activate driver's log output */
#ifdef DEBUG
 #define LOG_PRINTF(s) s
#else
 #define LOG_PRINTF(s)
#endif


/* Copies the 16-bit character Ch into the LFN slot Slot in the position */
/* SlotPos, taking care of the slot structure.                           */
/* Called by split_lfn.                                                  */
static void copy_char_in_lfn_slot(tLfnEntry *Slot, int SlotPos, UTF16 Ch)
{
  if ((SlotPos >= 0)  && (SlotPos <= 4))  Slot->Name0_4  [SlotPos]    = Ch;
  else
  if ((SlotPos >= 5)  && (SlotPos <= 10)) Slot->Name5_10 [SlotPos-5]  = Ch;
  else
  if ((SlotPos >= 11) && (SlotPos <= 12)) Slot->Name11_12[SlotPos-11] = Ch;
}


/* Splits the long file name in the string LongName to fit in up to 20 */
/* LFN slots, using the short name of the dir entry D to compute the   */
/* short name checksum.                                                */
/* On success, returns 0 and fills the Slot array with LFN entries and */
/* NumSlots with the number of entries occupied.                       */
/* On failure, returns a negative error code.                          */
/* Called by allocate_lfn_dir_entries.                                 */
static int split_lfn(tLfnEntry *Slot, tDirEntry *D, char *LongName, int *NumSlots)
{
  int   NamePos  = 0;
  int   SlotPos  = 0;
  int   Order    = 0;
  BYTE  Checksum = lfn_checksum(D);
  UTF16 LfnUtf16[FD32_LFNPMAX];

  /* Long file names are stored in UTF-16 */
  if (fd32_utf8to16(LongName, LfnUtf16)) return FD32_EUTF8;

  /* Initialize the first slot */
  Slot[0].Order    = 1;
  Slot[0].Attr     = FD32_ALNGNAM;
  Slot[0].Reserved = 0;
  Slot[0].Checksum = Checksum;
  Slot[0].FstClus  = 0;

  while (LfnUtf16[NamePos])
  {
    if (SlotPos == 13)
    {
      SlotPos = 0;
      Order++;
      Slot[Order].Order    = Order + 1; /* 1-based numeration */
      Slot[Order].Attr     = FD32_ALNGNAM;
      Slot[Order].Reserved = 0;
      Slot[Order].Checksum = Checksum;
      Slot[Order].FstClus  = 0;
    }
    copy_char_in_lfn_slot(&Slot[Order], SlotPos++, LfnUtf16[NamePos]);
    if (++NamePos == FD32_LFNMAX) return FD32_EFORMAT;
  }
  /* Mark the slot as last */
  Slot[Order].Order |= 0x40;
  /* Insert an Unicode NULL terminator, only if it fits into the slots */
  copy_char_in_lfn_slot(&Slot[Order], SlotPos++, 0x0000);
  /* Pad the remaining characters of the slot with FFFFh */
  while (SlotPos < 13) copy_char_in_lfn_slot(&Slot[Order], SlotPos++, 0xFFFF);

  *NumSlots = Order + 1;
  return 0;
}


/* Marks as free the specified entries of an open directory.      */
/* EntryOffset is the byte offset of the short name entry to be   */
/* freed, preceded by LfnEntries LFN slots.                       */
/* Called by fat_unlink and fat_rename.                           */
static int free_dir_entries(tFile *F, DWORD EntryOffset, int LfnEntries)
{
  int  Res, k;
  BYTE FirstChar = FREEENT;
  
  /* There are LfnEntries + 1 entries starting */
  /* at offset EntryOffset - LfnEntries * 32.  */
  for (k = LfnEntries; k >= 0; k--)
  {
    F->TargetPos = EntryOffset - k * 32;
    Res = fat_write(F, &FirstChar, 1);
    if (Res < 0) return Res;
  }
  return 0;
}


/* Searches an open directory for NumEntries free consecutive entries. */
/* On success, returns the not negative byte offset of the first free  */
/* entry. Returns a negative error code on failure.                    */
/* Called by allocate_sfn_dir_entry and allocate_lfn_dir_entries.      */
static int find_empty_dir_entries(tFile *F, int NumEntries)
{
  int       Found = 0; /* Number of consecutive free entries found */
  int       EntryOffset = 0;
  int       Res;
  tDirEntry E;

  /* Scans the whole directory for free entries */
  F->TargetPos = 0;
  Res = fat_read(F, (void*)&E, 32);
  if (Res < 0) return Res;
  while (Res > 0)
  {
    if ((E.Name[0] == FREEENT) || (E.Name[0] == ENDOFDIR))
    {
      if (Found > 0) Found++;
      else
      {
        Found       = 1;
        EntryOffset = F->TargetPos - Res;
      }
    }
    else Found = 0;
    if (Found == NumEntries) return EntryOffset;
    Res = fat_read(F, (void*)&E, 32);
    if (Res < 0) return Res;
  }
  /* At this point the directory file is at Eof. If possible, we need to */
  /* extend the directory writing a whole cluster filled with zeroes,    */
  /* next we return back to the beginning of that cluster.               */
  LOG_PRINTF(printf("End of dir at pos %lu\n", (DWORD) F->TargetPos));
  /* A FAT12/FAT16 root directory cannot be extended */
  if ISROOT(F) return FD32_EACCES;
  /* A directory cannot have more than 65536 entries */
  if (F->TargetPos == 65536 * 32) return FD32_EACCES;
  EntryOffset = F->TargetPos;
  /* The following write could fail if disk is full */
  Res = fat_write(F, NULL, F->V->Bpb.BPB_BytsPerSec * F->V->Bpb.BPB_SecPerClus);
  if (Res < 0) return Res;
  LOG_PRINTF(printf("Directory file extended to find room\n"));
  return EntryOffset;
}


/* Allocates one entry of the specified open directory and fills it    */
/* with the directory entry D.                                         */
/* On success, returns the byte offset of the allocated entry.         */
/* On failure, returns a negative error code.                          */
/* Called by allocate_lfn_dir_entry (if not LFN entries are required), */
/* and, ifndef FATLFN, by fat_creat and fat_rename.                    */
static int allocate_sfn_dir_entry(tFile *F, tDirEntry *D, char *FileName)
{
  int Res, EntryOffset;

  /* Get the name in FCB format, wildcards not allowed */
  if (fd32_build_fcb_name((BYTE*)D->Name, FileName)) return FD32_EFORMAT;
    
  /* Search for a free directory entry, extending the dir if needed */
  LOG_PRINTF(printf("Searching for a free directory entry\n"));
  if ((EntryOffset = find_empty_dir_entries(F, 1)) < 0) return EntryOffset;
  /* Write the new directory entry into the free entry */
  LOG_PRINTF(printf("Writing SFN dir entry at offset %i\n", EntryOffset));
  F->TargetPos = EntryOffset;
  Res = fat_write(F, (void*)D, 32);
  if (Res < 0) return Res;
  return EntryOffset;
}


/* Allocates 32-bytes directory entries of the specified open directory */
/* to store a long file name, using D as directory entry for the short  */
/* name alias.                                                          */
/* On success, returns the byte offset of the short name entry.         */
/* On failure, returns a negative error code.                           */
/* Called fat_creat and fat_rename.                                     */
static int allocate_lfn_dir_entries(tFile *F, tDirEntry *D, char *FileName, WORD Hint)
{
  BYTE      ShortName[11];
  tLfnEntry Slot[20];
  int       EntryOffset;
  int       LfnEntries;
  int       Res;
  int       k;

  /* gen_short_fname already checks if the LFN is valid */
  /* if (!lfn_is_valid(FileName)) return FD32_EFORMAT;  */
  /* TODO: gen_short_fname cross reference without fat_ prefix */
  Res = gen_short_fname(F, FileName, ShortName, Hint);
  if (Res < 0) return Res;

  /* If Res == 0 no long name entries are required */
  if (Res == 0) return allocate_sfn_dir_entry(F, D, FileName);

  /* Generate a 32-byte Directory Entry for the short name alias */
  memcpy((void*)D->Name, ShortName, 11);

  /* Split the long name into 32-byte slots */
  Res = split_lfn(Slot, D, FileName, &LfnEntries);
  if (Res) return Res;

  /* Search for NumSlots + 1 (LFN entries plus short name entry) */
  /* free directory entries, expanding the dir if needed.        */
  LOG_PRINTF(printf("Searching for %i free dir entries\n", LfnEntries + 1));
  EntryOffset = find_empty_dir_entries(F, LfnEntries + 1);
  if (EntryOffset < 0) return EntryOffset;
  /* Write the new directory entries into the free entries */
  LOG_PRINTF(printf("Writing LFN dir entries at offset %i\n", EntryOffset));
  F->TargetPos = EntryOffset;
  for (k = LfnEntries - 1; k >= 0; k--)
  {
    Res = fat_write(F, (void*)&Slot[k], 32);
    if (Res < 0) return Res;
  }
  EntryOffset = F->TargetPos;
  Res = fat_write(F, (void*)D, 32);
  if (Res < 0) return Res;
  return EntryOffset;
}


/* Creates a zero length file, allocating a new directory entry, in */
/* the directory specified by the file structure Fp, and keeps the  */
/* file open in the file structure Ff.                              */
/* Returns 0 on success, or a negative error code on failure.       */
/* Called by fat_open.                                              */
int fat_creat(tFile *Fp, tFile *Ff, char *Name, BYTE Attr, WORD AliasHint)
{
  int       Res;
  tDirEntry D;

  /* Initialize the directory entry */
  memset((void*)&D, 0, sizeof(tDirEntry));
  D.Attr         = Attr;
  fat_timestamps((WORD*)&D.CrtTime, (WORD*)&D.CrtDate, (BYTE*)&D.CrtTimeTenth);
  D.LstAccDate   = D.CrtDate;
  D.WrtTime      = D.CrtTime;
  D.WrtDate      = D.CrtDate;

  /* Switch the access mode to READWRITE for the following operation */
  Fp->Mode = (Fp->Mode & ~FD32_OACCESS) | FD32_ORDWR;
  #ifdef FATLFN
  Res = allocate_lfn_dir_entries(Fp, &D, Name, AliasHint);
  #else
  Res = allocate_sfn_dir_entry(Fp, &D, Name);
  #endif
  if (Res < 0) return Res;

  /* Now fill the passed Ff file structure with the status of the new file */
  Ff->V              = Fp->V;
  Ff->DirEntryOffset = Res;
  if ISROOT(Fp) /* ISROOT checks Fp->DirEntrySector */
    Ff->DirEntrySector = Fp->SectorInCluster + Fp->V->FirstRootSector;
   else
    Ff->DirEntrySector = Fp->SectorInCluster
                       + fat_first_sector_of_cluster(Fp->Cluster, Fp->V);
  Ff->DirEntrySecOff  = Fp->ByteInSector - 31;
  Ff->ParentFstClus   = (Fp->DirEntry.FstClusHI << 16) + Fp->DirEntry.FstClusLO;
  Ff->Mode            = 0;
  Ff->DirEntry        = D;
  Ff->DirEntryChanged = 1;
  /* Rewind the file */
  Ff->TargetPos       = 0;
  Ff->FilePos         = 0;
  Ff->Cluster         = FIRSTCLUSTER(Ff->DirEntry);
  Ff->PrevCluster     = 0;
  Ff->SectorInCluster = 0;
  Ff->ByteInSector    = 0;

  /* The opening mode */
  Fp->Mode &= ~FD32_OACCESS;
  return 0;
}


/* Renames a file or moves a file across directories of the same volume.     */
/* Open files cannot be renamed. The name cache for the file is invalidated. */
/* Returns 0 on success, or a negative error code on failure.                */
/* This is a public driver function.                                         */
int fat_rename(tVolume *V, char *OldFullName, char *NewFullName)
{
  char      OldPath[FD32_LFNPMAX];
  char      NewPath[FD32_LFNPMAX];
  char      OldName[FD32_LFNMAX];
  char      NewName[FD32_LFNMAX];
  tFile    *SrcDir;
  tFile    *DstDir;
  tFatFind  D;
  tFileId   Fid;
  int       Res;

  /* Open the source directory */
  /* TODO: Find a decent place for split_path... */
  split_path(OldFullName, OldPath, OldName);
  Res = fat_open(V, OldPath, FD32_ORDWR | FD32_OEXIST | FD32_ODIR, FD32_ANONE, 0, &SrcDir);
  if (Res < 0) return Res;

  /* Find the source name and get all its informations */
  Res = fat_find(SrcDir, OldName, FD32_FRNONE | FD32_FAALL, &D);
  if (Res < 0)
  {
    fat_close(SrcDir);
    return Res;
  }

  #ifdef FATSHARE
  Fid.V              = V;
  Fid.ParentFstClus  = FIRSTCLUSTER(SrcDir->DirEntry);
  Fid.DirEntryOffset = D.EntryOffset;
  if (fat_isopen(&Fid))
  {
    fat_close(SrcDir);
    return FD32_EACCES; /* Open files cannot be renamed */
  }
  #endif

  #ifdef FATNAMECACHE
  /* Invalidate the name cache for that file */
  /* TODO: Fix name cache */
  for (Res = 0; Res < V->NumFiles; Res++)
    if (is_the_same_file(&V->Files[Res], &Fid))
    {
      V->Files[Res].CacheLastAccess  = 0;
      V->Files[Res].CacheFileName[0] = 0;
    }
  #endif

  /* Open the destination directory */
  split_path(NewFullName, NewPath, NewName);
  Res = fat_open(V, NewPath, FD32_ORDWR | FD32_OEXIST | FD32_ODIR, FD32_ANONE, 0, &DstDir);
  if (Res < 0) return Res;
  /* Check if the destination name already exists */
  Res = fat_find(DstDir, NewName, FD32_FRNONE | FD32_FAALL, NULL);
  if (Res == 0)
  {
    fat_close(SrcDir);
    fat_close(DstDir);
    return FD32_EEXIST;
  }

  /* Delete the old entries from the source directory */
  Res = free_dir_entries(SrcDir, D.EntryOffset, D.LfnEntries);
  if (Res < 0)
  {
    fat_close(SrcDir);
    fat_close(DstDir);
    return Res;
  }

  /* And allocate the new entries in the destination directory */
  #ifdef FATLFN
  Res = allocate_lfn_dir_entries(DstDir, &D.SfnEntry, NewName, 1);
  #else
  Res = allocate_sfn_dir_entry(DstDir, &D.SfnEntry, NewName);
  #endif
  
  if (Res < 0)
  {
    /* On failure restore the old entries in the source directory */
    #ifdef FATLFN
    allocate_lfn_dir_entries(SrcDir, &D.SfnEntry, OldName, 1);
    #else
    allocate_sfn_dir_entry(SrcDir, &D.SfnEntry, OldName);
    #endif
  }
  fat_close(SrcDir);
  fat_close(DstDir);
  return (Res < 0) ? Res : 0;
}


/* Deletes the specified file(s) according to flags.                    */
/* Open files cannot be deleted. The file(s) name cache is invalidated. */
/* Returns 0 on success, or a negative error code on failure.           */
/* This is a public driver function, and is called by fat_rmdir (dir.c) */
int fat_unlink(tVolume *V, char *FileName, DWORD Flags)
{
  tFile    *Dir;
  tFatFind  D;
  tFileId   Fid;
  int       Res;
  char      Path[FD32_LFNPMAX];
  char      Name[FD32_LFNMAX];
  int       NoneDeleted = 1; /* Check if something has been deleted   */
  long long int SaveDirPos;      /* Save the dir position while searching */
/*
  TODO: This will go in the INT 21h handler.
  unsigned char AllowableAttributes = FD32_ATTR_READONLY | FD32_ATTR_ARCHIVE |
                                      FD32_ATTR_HIDDEN   | FD32_ATTR_SYSTEM;
  if (RequiredAttributes == FD32_ATTR_DIRECTORY)
    AllowableAttributes |= FD32_ATTR_DIRECTORY;
*/
  
  split_path(FileName, Path, Name);

  LOG_PRINTF(printf("FAT unlink: file %s in directory %s\n", Name, Path));
  Res = fat_open(V, Path, FD32_ORDWR | FD32_OEXIST | FD32_ODIR, FD32_ANONE, 0, &Dir);
  if (Res < 0) return Res;

  /* Repeat until no more matches are found */
  for (;;)
  {
    LOG_PRINTF(printf("FAT unlink: searching for file\n"));
    Res = fat_find(Dir, Name, Flags, &D);
    SaveDirPos = Dir->TargetPos;

    /* Check if error (usually file not found or read only file) */
    if (Res < 0)
    {
      fat_close(Dir);
      if (Res != FD32_ENMFILE) return Res;
      if (NoneDeleted) return FD32_ENOENT;
      return 0;
    }
    if (D.SfnEntry.Attr & FD32_ARDONLY) return FD32_EACCES;

    #ifdef FATSHARE
    Fid.V              = V;
    Fid.ParentFstClus  = FIRSTCLUSTER(Dir->DirEntry);
    Fid.DirEntryOffset = D.EntryOffset;
    if (fat_isopen(&Fid))
    {
      fat_close(Dir);
      return FD32_EACCES; /* Open files cannot be deleted */
    }
    #endif

    #ifdef FATNAMECACHE
    /* Invalidate the name cache for that file */
    for (Res = 0; Res < V->NumFiles; Res++)
      if (is_the_same_file(&V->Files[Res], &Fid))
      {
        V->Files[Res].CacheLastAccess  = 0;
        V->Files[Res].CacheFileName[0] = 0;
      }
    #endif

    LOG_PRINTF(printf("FAT unlink: Now unlinking clusters\n"));
    /* If the file is not zero bytes long we unlink its cluster chain */
    if (FIRSTCLUSTER(D.SfnEntry) != 0)
    {
      switch (V->FatType)
      {
        case FAT12 : Res = fat12_unlink(V, FIRSTCLUSTER(D.SfnEntry)); break;
        case FAT16 : Res = fat16_unlink(V, FIRSTCLUSTER(D.SfnEntry)); break;
        case FAT32 : Res = fat32_unlink(V, FIRSTCLUSTER(D.SfnEntry)); break;
      }
      if (Res < 0) return Res;
    }

    LOG_PRINTF(printf("FAT unlink: ...and free dir entries\n"));
    Res = free_dir_entries(Dir, D.EntryOffset, D.LfnEntries);
    if (Res < 0) return Res;
    NoneDeleted = 0;
    Dir->TargetPos = SaveDirPos;
  }
}
#endif /* #ifdef FATWRITE */

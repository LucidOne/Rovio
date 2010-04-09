/**************************************************************************
 * FreeDOS 32 FAT Driver                                                  *
 * by Salvo Isaja                                                         *
 *                                                                        *
 * Copyright (C) 2001-2003, Salvatore Isaja                               *
 *                                                                        *
 * This is "readdir.c" - Services to find a specified directory entry     *
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

/* Define the DEBUG symbol in order to activate driver's log output */
#ifdef DEBUG
 #define LOG_PRINTF(s) s
#else
 #define LOG_PRINTF(s)
#endif

/* To fetch a LFN we use a circular buffer of directory entries.       */
/* A long file name is up to 255 characters long, so 20 entries are    */
/* needed, plus one additional entry for the short name alias. We scan */
/* the directory and place the entry that we read into the Buffer at   */
/* the offset BufferPos. When we find a valid short name, we backtrace */
/* the Buffer with LfnSlot to extract the long file name, if present.  */
#define LFN_FETCH_SLOTS 21

/* Fetches a long file name before a short name entry, if present, from the */
/* specified Buffer of directory entries to a 16-bit character string.      */
/* Returns the number of directory entries used by the long name, or 0 if   */
/* no long name is present before the short name entry.                     */
/* Called by fat_readdir and find.                                          */
static int fetch_lfn(tDirEntry *Buffer, int BufferPos, WORD *Name)
{
  tLfnEntry *LfnSlot  = (tLfnEntry *) &Buffer[BufferPos];
  BYTE       Checksum = lfn_checksum(&Buffer[BufferPos]);
  int        Order    = 0;
  int        NamePos  = 0;
  int        k;

  do
  {
    if (--LfnSlot < (tLfnEntry *) Buffer) LfnSlot += LFN_FETCH_SLOTS;
    if (LfnSlot->Attr != FD32_ALNGNAM) return 0;
    if (++Order != (LfnSlot->Order & 0x1F)) return 0;
    if (Checksum != LfnSlot->Checksum) return 0;
    /* Ok, the LFN slot is valid, attach it to the long name */
    for (k = 0; k < 5; k++) Name[NamePos++] = LfnSlot->Name0_4[k];
    for (k = 0; k < 6; k++) Name[NamePos++] = LfnSlot->Name5_10[k];
    for (k = 0; k < 2; k++) Name[NamePos++] = LfnSlot->Name11_12[k];
  }
  while (!(LfnSlot->Order & 0x40));
  if (Name[NamePos - 1] != 0x0000) Name[NamePos] = 0x0000;
  return Order;
}


/* Searches an open directory for files matching the file specification */
/* and the search flags.                                                */
/* On success, returns 0, fills the FindData structure.                 */
/* Returns a negative error code on failure.                            */
static int readdir(tFile *Dir, tFatFind *Ff)
{
  tDirEntry  Buffer[LFN_FETCH_SLOTS];
  int        BufferPos = 0;
  int        NumRead;
  WORD       LongNameUtf16[FD32_LFNMAX];

  if (!(Dir->DirEntry.Attr & FD32_ADIR)) return FD32_EBADF;

  NumRead = fat_read(Dir, (void*)&Buffer[0], 32);
  if (NumRead < 0) {printf("01 fat_read return < 0\n");return NumRead;}
  while (NumRead > 0)
  {
    /* If the directory name starts with FAT_DIR_ENFOFDIR */
    /* we don't need to scan further.                     */
    if (Buffer[BufferPos].Name[0] == ENDOFDIR) return FD32_ENMFILE;

    /* If the entry is marked as deleded we skip to the next entry */
    if (Buffer[BufferPos].Name[0] != FREEENT)
    {
      /* If we find a file without the Long Name attribute, it is a valid */
      /* short name. We then try to extract a long name before it.        */
      if (Buffer[BufferPos].Attr != FD32_ALNGNAM)
      {
        Ff->SfnEntry    = Buffer[BufferPos];
        Ff->EntryOffset = Dir->TargetPos - NumRead;
        fd32_expand_fcb_name(Ff->ShortName, (BYTE*)Buffer[BufferPos].Name);
        #ifdef FATLFN
        Ff->LfnEntries = fetch_lfn(Buffer, BufferPos, LongNameUtf16);
        if (Ff->LfnEntries)
          fd32_utf16to8(LongNameUtf16, Ff->LongName);
         else
          strcpy(Ff->LongName, Ff->ShortName);
        #else
        Ff->LfnEntries  = 0;
        strcpy(Ff->LongName, Ff->ShortName);
        #endif
        LOG_PRINTF(("Found: %-14s%s\n", Ff->ShortName, Ff->LongName));
        return 0;
      }
    }
    if (++BufferPos == LFN_FETCH_SLOTS) BufferPos = 0;
    NumRead = fat_read(Dir, (void*)&Buffer[BufferPos], 32);
    if (NumRead < 0) {printf("02 fat_read return < 0\n");return NumRead;}
  } /* while (NumRead > 0) */
  return FD32_ENMFILE;
}


/* The READDIR system call.                                    */
/* Reads the next directory entry of the open directory Dir in */
/* the passed LFN finddata structure.                          */
/* Returns 0 on success, or a negative error code on failure.  */
int fat_readdir(tFile *Dir, fd32_fs_lfnfind_t *Entry)
{
  tFatFind F;
  int      Res;

  if (!Entry) return FD32_EINVAL;
  if ((Res = readdir(Dir, &F)) < 0) return Res;

  Entry->Attr   = F.SfnEntry.Attr;
  Entry->CTime  = F.SfnEntry.CrtTime + (F.SfnEntry.CrtDate << 16);
  Entry->ATime  = F.SfnEntry.LstAccDate << 16;
  Entry->MTime  = F.SfnEntry.WrtTime + (F.SfnEntry.WrtDate << 16);
  Entry->SizeHi = 0;
  Entry->SizeLo = F.SfnEntry.FileSize;
  ((tFindRes *) Entry->Reserved)->EntryCount = Dir->TargetPos >> 5;
  ((tFindRes *) Entry->Reserved)->FirstDirCluster = FIRSTCLUSTER(Dir->DirEntry);
  strcpy(Entry->LongName, F.LongName);
  strcpy(Entry->ShortName, F.ShortName);
  return 0;
}


/* Searches an open directory for files matching the file specification and   */
/* the search flags.                                                          */
/* On success, returns 0 and fills the passed find data structure (optional). */
/* Returns a negative error code on failure.                                  */
int fat_find(tFile *Dir, char *FileSpec, DWORD Flags, tFatFind *Ff)
{
  int      Res;
  tFatFind TempFf;
  BYTE     AllowableAttr = (BYTE) Flags;
  BYTE     RequiredAttr  = (BYTE) (Flags >> 8);

  while ((Res = readdir(Dir, &TempFf)) == 0)
  {
    if (((AllowableAttr | TempFf.SfnEntry.Attr) == AllowableAttr)
     && ((RequiredAttr & TempFf.SfnEntry.Attr) == RequiredAttr))
    {
      #ifdef FATLFN
      if (utf8_stricmp(TempFf.LongName, FileSpec) == 0)
      {
        if (Ff) *Ff = TempFf;
        return 0;
      }
      #endif
      if (utf8_stricmp(TempFf.ShortName, FileSpec) == 0)
      {
        if (Ff) *Ff = TempFf;
        return 0;
      }
    }
  }
  return Res;
}

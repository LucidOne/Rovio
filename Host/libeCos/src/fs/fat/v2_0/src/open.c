/**************************************************************************
 * FreeDOS 32 FAT Driver                                                  *
 * by Salvo Isaja                                                         *
 *                                                                        *
 * Copyright (C) 2001-2003, Salvatore Isaja                               *
 *                                                                        *
 * This is "open.c" - Open, create and close a file (or even a directory  *
 *                    as a file) in any directory, allocating and freeing *
 *                    handles                                             *
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

#ifdef FATNAMECACHE
 #include <unicode.h>
#endif

//#define DEBUG

/* Define the DEBUG symbol in order to activate driver's log output */
#ifdef DEBUG
 #define LOG_PRINTF(s) s
#else
 #define LOG_PRINTF(s)
#endif


#ifdef FATSHARE
/* According to the RBIL #001269: Values for DOS 7.x file sharing behavior. */
/* The matrix: SharingBehaviour[sharing mode of other instances]            */
/*                             [sharing mode of current instance]           */
/* encodes sharing conflict flags as 16-bit words, as follows:              */
/*      R  W RW NA                                                          */
/*  R   N  N  N  Y => 1              R  = Read                              */
/*  W   Y  Y  Y  Y => F => 0x1F11    W  = Write                             */
/*  RW  N  N  N  Y => 1              RW = Read/Write                        */
/*  NA  N  N  N  Y => 1              NA = Read w/o updating last access     */
/* If the appropriate sharing bit is reset a sharing conflict occurred.     */
static const WORD SharingBehaviour[5][5] =
{ /* others \ curr  COMPAT  DENYRW  DENYRD  DENYWR  DENYNO */
  /* COMPAT    */ { 0xFEE9, 0x0000, 0x9009, 0x1111, 0x9999 },
  /* DENYALL   */ { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
  /* DENYREAD  */ { 0x9009, 0x0000, 0x9009, 0x1911, 0x9999 },
  /* DENYWRITE */ { 0x000F, 0x0000, 0x400F, 0x0401, 0x444F },
  /* DENYNONE  */ { 0xF00F, 0x0000, 0xF00F, 0x1F11, 0xFFFF }
};
#endif

/* File structures */
#if 0
static int    NumFiles; /* Max number of open files         */
static tFile *Files;    /* Dynamic array of file structures */
#else
static int    NumFiles = 20; /* Max number of open files         */
static tFile  Files[20];    /* Dynamic array of file structures */
#endif


/* Returns the number of open files for a given volume */
int fat_openfiles(tVolume *V)
{
  int k, Open = 0;
  for (k = 0; k < NumFiles; k++)
    if ((Files[k].V == V) && (Files[k].References)) Open++;
  return Open;
}


/* Returns nonzero if the specified file is open, zero if not. */
/* Called by fat_rename and fat_unlink (creat.c).              */
int fat_isopen(tFileId *Fid)
{
  int k;
  for (k = 0; k < NumFiles; k++)
    if ((Files[k].References) && SAMEFILE(&Files[k], Fid)) return 1;
  return 0;
}


#ifdef FATWRITE
/* Writes the file's directory entry into the parent directory.  */
/* Called by fat_syncentry if sharing is enabled, by file_fflush */
/* if sharing is disabled.                                       */
/* Returns 0 on success, or a negative error code on failure.    */
static int write_direntry(tFile *F)
{
  int Buf;
  /* If the file is the root directory it doesn't have a dir entry */
  if (((F->V->FatType == FAT32)
   && (FIRSTCLUSTER(F->DirEntry) == F->V->Bpb.BPB_RootClus))
   || ISROOT(F)) return 0;
  /* If the file is not the root, write its directory entry */
  if ((Buf = fat_readbuf(F->V, F->DirEntrySector)) < 0) return Buf;
  memcpy(F->V->Buffers[Buf].Data + F->DirEntrySecOff, (void*)&F->DirEntry, 32);
  F->DirEntryChanged = 0;
  return fat_writebuf(F->V, Buf);
}


/* Syncronizes the directory entry of all open instances of a file. */
/* Called by fat_set_attr (attrib.c), move_to_targetpos, fat_read,  */
/* truncate_or_extend and fat_write (readwrit.c).                   */
/* Returns 0 on success, or a negative error code on failure.       */
int fat_syncentry(tFile *F)
{
  tFile *F1; /* F1 is used for other instances of F */
  int    k;

  if (!F->DirEntryChanged) return 0;
  for (k = 0; k < NumFiles; k++)
  {
    F1 = &Files[k];
    if ((F != F1) && SAMEFILE(F1, F))
    {
      F1->DirEntryChanged = 0;
      F1->DirEntry = F->DirEntry;
    }
  }
  return write_direntry(F);
}
#endif


/* Syncronizes the physical file position of all open instances of a file.  */
/* Used to avoid access past the end of file when an instance is truncated. */
/* Called by truncate_or_extend (write.c).                                  */
void fat_syncpos(tFile *F)
{
  tFile *F1; /* F1 is used for other instances of F */
  int    k;

  for (k = 0; k < NumFiles; k++)
  {
    F1 = &Files[k];
    if ((F != F1) && SAMEFILE(F1, F))
      if (F1->FilePos > F->FilePos)
      {
        F1->FilePos         = F->FilePos;
        F1->PrevCluster     = F->PrevCluster;
        F1->Cluster         = F->Cluster;
        F1->SectorInCluster = F->SectorInCluster;
        F1->ByteInSector    = F->ByteInSector;
      }
  }
}


/* Searches for an unused file structure of the volume V. */
/* Returns the not negative file structure number on      */
/* success, or a negative number on failure.              */
/* Called by descend_path, fat_open and reopen_dir.       */
static tFile *take_file(void)
{
  #ifdef FATNAMECACHE
  int   k, Lru = -1;
  DWORD MinAccess = V->BufferAccess;
  for (k = 0; k < V->NumFiles; k++)
    if ((V->Files[k].CacheLastAccess < MinAccess)
     && (V->Files[k].References == 0))
    {
      MinAccess = V->Files[k].CacheLastAccess;
      Lru       = k;
    }
  if (Lru != -1)
  {
    V->Files[Lru].V          = V;
    V->Files[Lru].References = 1;
    return &V->Files[Lru];
  }
  return NULL;
  #else
  int k;
  for (k = 0; k < NumFiles; k++)
    if (Files[k].References == 0)
    {
      Files[k].References = 1;
      Files[k].FilSig     = FAT_FILSIG;
      return &Files[k];
    }
  return NULL;
  #endif
}


/* Splits a full valid path name (path + file name) into its path and  */
/* file name components.                                               */
/* Called by fat_open, fat_unlink (creat.c), fsvol_lfn_findfirst       */
/* (find.c), fsvol_dos_findfirst (find.c) and fsvol_rename (rename.c). */
/* TODO: Fix list of callers and name... */
void split_path(char *FullPath, char *Path, char *Name)
{
  char *NameStart;
  char *s;
  
  /* Move to the end of the FullPath string */
  for (s = FullPath; *s; s++);
  /* Now move backward until '/' is found. The name starts after the '/' */
  for (; (*s != '/') && (s != FullPath); s--);
  if (*s == '/') s++;
  /* And copy the string from this point until the end into Name */
  for (NameStart = s; (*Name++ = *s++););
 
  /* Finally we copy the portion of FullPath before the file name    */
  /* into Path, removing the non-root trailing backslash if present. */
  for (s = FullPath; s < NameStart; s++, Path++) *Path = *s;
  if ((NameStart > FullPath + 1) && (*(Path - 1) == '/')) Path--;
  *Path = 0;
}


/* Resets both the physical position and the target position of an open file. */
/* Called by open_existing, descend_path and fat_open.                        */
static void rewind_file(tFile *F)
{
  F->TargetPos       = 0;
  F->FilePos         = 0;
  F->Cluster         = FIRSTCLUSTER(F->DirEntry);
  F->PrevCluster     = 0;
  F->SectorInCluster = 0;
  F->ByteInSector    = 0;
}


/* Sets the file opening mode according to Mode parameter.         */
/* All file structure's mode flags are replaced.                   */
/* Returns 0 on success, or a negative error code on failure.      */
/* Called by open_existing, descend_path, fat_open and reopen_dir. */
static int set_opening_mode(tFile *F, DWORD Mode)
{
  #ifdef FATSHARE
  tFile   *F1;  /* F1 is used for other instances of F */
  int      k;
  int      FShare, F1Share; /* SharingBehaviour matrix indices */
  int      FMode,  F1Mode;  /* SharingBehaviour matrix masks   */
  #endif
  //printf("F->DirEntry.Attr=0x%x\n",F->DirEntry.Attr);
  /* Directory consistency check */
  if (F->DirEntry.Attr & FD32_ADIR)
  {
    if (!(Mode & FD32_ODIR)) return FD32_EACCES;
  }
  else
  {
    if (Mode & FD32_ODIR) return FD32_ENOTDIR;
  }
  /* Check if the access code is valid */
  switch (Mode & FD32_OACCESS)
  {
    case FD32_OWRITE:
    case FD32_ORDWR :
      #ifdef FATWRITE
      if (F->DirEntry.Attr & FD32_ARDONLY) return FD32_EACCES;
      #else
      return FD32_EROFS;
      #endif
  }
  F->Mode = Mode;

  #ifdef FATSHARE
  /* Check if reopening a file is a valid operation, according to its  */
  /* access and sharing modes and access and sharing modes of other    */
  /* instances of the file. DOS 7 sharing behaviour is used.           */
  /* FIX ME: According to the opening flag, should generate INT 24h on */
  /*         critical error.                                           */
  for (k = 0; k < NumFiles; k++)
  {
    F1 = &Files[k];
    if ((F != F1) && SAMEFILE(F1, F))
    {
      switch (F1->Mode & FD32_OACCESS)
      {
        case FD32_OREAD  : F1Mode = 0xF000; break;
        case FD32_OWRITE : F1Mode = 0x0F00; break;
        case FD32_ORDWR  : F1Mode = 0x00F0; break;
        case FD32_ORDNA  : F1Mode = 0x000F; break;
        default          : return FD32_EACODE;
      }
      switch (F->Mode & FD32_OACCESS)
      {
        case FD32_OREAD  : FMode = 0x8888; break;
        case FD32_OWRITE : FMode = 0x4444; break;
        case FD32_ORDWR  : FMode = 0x2222; break;
        case FD32_ORDNA  : FMode = 0x1111; break;
        default          : return FD32_EACODE;
      }
      switch (F1->Mode & FD32_OSHARE)
      {
        case FD32_OCOMPAT : F1Share = 0; break;
        case FD32_ODENYRW : F1Share = 1; break;
        case FD32_ODENYWR : F1Share = 2; break;
        case FD32_ODENYRD : F1Share = 3; break;
        case FD32_ODENYNO : F1Share = 4; break;
        default           : return FD32_EACODE;
      }
      switch (F->Mode & FD32_OSHARE)
      {
        case FD32_OCOMPAT : FShare = 0; break;
        case FD32_ODENYRW : FShare = 1; break;
        case FD32_ODENYWR : FShare = 2; break;
        case FD32_ODENYRD : FShare = 3; break;
        case FD32_ODENYNO : FShare = 4; break;
        default           : return FD32_EACODE;
      }
      if (!(SharingBehaviour[F1Share][FShare] & F1Mode & FMode))
      {
        if (FShare == 0) return FD32_EVSHAR; /* INT 24h */
                         return FD32_EACCES;
      }
    } /* if ((F != F1) && SAMEFILE(F1, F)) */
  }
  #endif /* #ifdef FATSHARE */
  return 0;
}


/* Opens an existing file in the directory specified by the file    */
/* structure Fp, using the passed directory entry D to fill in the  */
/* file structure Ff for the file.                                  */
/* The file structures Fp and Ff may be the same if overwriting the */
/* parent directory file structure is needed.                       */
/* Returns 0 on success, or a negative error code on failure.       */
/* Called by descend_path and fat_open.                             */
static int open_existing(tFile *Fp, tFile *Ff, tDirEntry *D, DWORD Mode)
{
  Ff->V              = Fp->V;
  Ff->References     = 1;
  Ff->FilSig         = FAT_FILSIG;
  Ff->DirEntryOffset = Fp->TargetPos - 32;
  if ISROOT(Fp) /* ISROOT checks Fp->DirEntrySector */
    Ff->DirEntrySector = Fp->SectorInCluster + Fp->V->FirstRootSector;
   else
    Ff->DirEntrySector = Fp->SectorInCluster
                       + fat_first_sector_of_cluster(Fp->Cluster, Fp->V);
  Ff->DirEntrySecOff = Fp->ByteInSector - 31;
  Ff->ParentFstClus  = FIRSTCLUSTER(Fp->DirEntry);
  Ff->Mode           = 0;
  Ff->DirEntry       = *D;
  Ff->DirEntryChanged = 0;
  /* Check if we are going to open the root directory, identified as a dir */
  /* starting at cluster 0, like in "..", even if the volume is FAT32.     */
  if ((FIRSTCLUSTER(Ff->DirEntry) == 0) && (Ff->DirEntry.Attr & FD32_ADIR))
  {
    if (Ff->V->FatType != FAT32) Ff->DirEntrySector = 0;
    Ff->DirEntry.FstClusHI = (WORD) (Ff->V->Bpb.BPB_RootClus >> 16);
    Ff->DirEntry.FstClusLO = (WORD)  Ff->V->Bpb.BPB_RootClus;
  }
  rewind_file(Ff);
  return set_opening_mode(Ff, Mode);
}


/* Descends a path, opening each component and returning a pointer to  */
/* a file structure for the last path component in the Fp pointer.     */
/* This file structure can be used to manage the parent directory of   */
/* a file to open.                                                     */
/* If the same path has been already opened before, a file structure   */
/* used for it may be present, with the path name cached. If that is   */
/* the case, that file structure is returned without descending again. */
/* Returns 0 on success, or a negative error code on failure.          */
static int descend_path(tVolume *V, char *Path, tFile **Fp)
{
  tFatFind D;
  int      Res;
  char     Component[FD32_LFNMAX];
  char    *pComponent;
  tFile   *F;
  #ifdef FATNAMECACHE
  char    *SavePath = Path;
  #endif

  LOG_PRINTF(printf("FAT descend_path: \"%s\"\n", Path));

  #ifdef FATNAMECACHE
  if (*Path) for (Res = 0; Res < V->NumFiles; Res++)
    if (utf8_stricmp(Path, V->Files[Res].CacheFileName) == 0)
    {
      LOG_PRINTF(printf("Path to open found in the name cache\n"));
      F = &V->Files[Res];
      /* If the file is already open copy its status to a new structure */
      if (F->References)
      {
        if ((F = get_fd(V)) == NULL) return FD32_EMFILE;
        memcpy(F, &V->Files[Res], sizeof(tFile));
      }
      F->Mode       &= ROOT;
      F->References  = 1;
      rewind_file(F);
      Res = set_opening_mode(F, FD32_OREAD | FD32_ODIR);
      if (Res < 0) return Res;
      *Fp = F;
      return 0;
    }
  #endif

  /* Allocate a file descriptor to descend the path */
  if ((F = take_file()) == NULL) return FD32_EMFILE;
  F->V    = V;
  F->Mode = 0;
  memset((void*)&F->DirEntry, 0, sizeof(tDirEntry));
  F->DirEntryChanged = 0;
  F->DirEntry.Attr = FD32_ADIR;
  /* Start to descend the path from the root directory */
  if (*Path == '/') Path++;
  if (V->FatType != FAT32) F->DirEntrySector = 0;
  F->DirEntry.FstClusHI = (WORD) (V->Bpb.BPB_RootClus >> 16);
  F->DirEntry.FstClusLO = (WORD)  V->Bpb.BPB_RootClus;
  rewind_file(F);
  Res = set_opening_mode(F, FD32_OREAD | FD32_ODIR);
  if (Res < 0) { F->References = 0; return Res; }

  /* If the Path string is emtpy just exit, */
  /* otherwise open each path component.    */
  if (!(*Path)) { *Fp = F; return 0; }
  while (*Path)
  {
    for (pComponent = Component; *Path;)
    {
      if (*Path == '/') { Path++; break; }
      *pComponent++ = *Path++;
    }
    *pComponent = 0;
    LOG_PRINTF(printf("Path component: \"%s\"\n", Component));
    /* Search for the file named Component in the directory */
    Res = fat_find(F, Component, FD32_FRNONE | FD32_FAALL, &D);
    if (Res < 0)
    {
      if (Res == FD32_ENMFILE) Res = FD32_ENOTDIR;
      return Res;
    }
    /* And open the path component overwriting of the file structure */
    Res = open_existing(F, F, &D.SfnEntry, FD32_OREAD | FD32_ODIR);
    if (Res < 0) { F->References = 0; return Res; }
  }
  #ifdef FATNAMECACHE
  F->CacheLastAccess = F->V->BufferAccess;
  strcpy(F->CacheFileName, SavePath);
  #endif
  *Fp = F;
  return 0;
}


/* Checks if the Mode parameter passed to the open call is valid. */
/* Called by fat_open.                                            */
static int validate_open_arguments(DWORD Mode)
{
  /* Check if action code is valid */
  switch (Mode & 0x000F0000)
  {
    case 0          :
    case FD32_OEXIST:
    case FD32_OTRUNC: break;
    default         : return FD32_EINVAL;
  }
  switch (Mode & 0x00F00000)
  {
    case 0          :
    case FD32_OCREAT: break;
    default         : return FD32_EINVAL;
  }
  /* Check if access and sharing modes are valid */
  switch (Mode & FD32_OACCESS)
  {
    case FD32_OREAD :
    case FD32_OWRITE:
    case FD32_ORDWR :
    case FD32_ORDNA : break;
    default         : return FD32_EACODE;
  }
  switch (Mode & FD32_OSHARE)
  {
    case FD32_OCOMPAT:
    case FD32_ODENYRW:
    case FD32_ODENYWR:
    case FD32_ODENYRD:
    case FD32_ODENYNO: break;
    default          : return FD32_EACODE;
  }
  return 0;
}


/* Opens, creates or replaces a file.                             */
/* On success, returns the action taken (file opened, created or  */
/* replaced) and fills the F parameter with a pointer to the file */
/* structure for the open file.                                   */
/* Returns FD32_OROPEN, FD32_ORTRUNC, FD32_ORCREAT on success, or */
/* a negative error code on failure.                              */
int fat_open(tVolume *V, char *FileName, DWORD Mode, WORD Attr,
             WORD AliasHint, tFile **F)
{
  int      Res;
  char     Path[FD32_LFNPMAX];
  char     Name[FD32_LFNMAX];
  tFatFind D;
  tFile   *Ff;
  tFile   *Fp;

  LOG_PRINTF(printf("FAT: Opening '%s' Mode = %x\n", FileName,Mode));
  if ((Res = validate_open_arguments(Mode))) return Res;

  #ifdef FATNAMECACHE
  /* The path to open can never be the root because it's not cached */
  if (*FileName) for (Res = 0; Res < V->NumFiles; Res++)
    if (utf8_stricmp(FileName, V->Files[Res].CacheFileName) == 0)
    {
      LOG_PRINTF(printf("FAT: File to open found in the name cache\n"));
      if ((!(Mode & FD32_OEXIST)) && (!(Mode & FD32_OTRUNC)))
        return FD32_EACCES;
      Ff = &V->Files[Res];
      /* If the file is already open copy its status to a new structure */
      if (Ff->References)
      {
        if ((Ff = get_fd(V)) == NULL) return FD32_EMFILE;
        memcpy(Ff, &V->Files[Res], sizeof(tFile));
      }
      Ff->Mode       = 0;
      Ff->References = 1;
      rewind_file(Ff);
      Res = set_opening_mode(Ff, Mode);
      if (Res < 0) return Res;
      /* Truncate the file to zero length if required */
      if (Mode & FD32_OTRUNC)
      {
        #ifdef FATWRITE
        if ((Res = fat_write(Ff, NULL, 0)) < 0) { Ff->References = 0; return Res; }
        *F = Ff;
        return FD32_ORTRUNC;
        #else
        Ff->References = 0;
        return FD32_EROFS;
        #endif
      }
      *F = Ff;
      return FD32_OROPEN;
    }
  #endif
LOG_PRINTF(printf("split_path..."));
  /* Use the Fp file structure to descend the path. If no file  */
  /* name is provided (the FileName string ends with '\') apply */
  /* the specified opening mode directly to Fp and return it.   */
  split_path(FileName, Path, Name);
 LOG_PRINTF(printf("Path=%s,Name=%s ",Path,Name));
  if ((Res = descend_path(V, Path, &Fp)) < 0) return Res;
  LOG_PRINTF(printf("*Name=0x%x\n",*Name));
  if (!(*Name))
  {
    Res = set_opening_mode(Fp, Mode);
    if (Res < 0) { Fp->References = 0; return Res; }
    *F = Fp;
    return FD32_OROPEN;
  }

  /* If a file name has been provided, find it in the parent directory */
  LOG_PRINTF(printf("FAT: Opening the actual file...\n"));
  Res = fat_find(Fp, Name, FD32_FRNONE | FD32_FAALL, &D);
  if (Res < 0)
  {
  	LOG_PRINTF(printf("fat_find No Exist...\n"));
    if (Res != FD32_ENMFILE) { Fp->References = 0; return Res; }
    /* If the file does not exist and creation is not required return error */
    if (!(Mode & FD32_OCREAT))
    {
      LOG_PRINTF(printf("FAT: File does not exist, FD32_OCREAT not specified.\n"));
      Fp->References = 0;
      if (Mode & FD32_ODIR) return FD32_ENOTDIR;
      return FD32_ENOENT;
    }
    /* Otherwise create the file */
    #ifdef FATWRITE
     if ((Ff = take_file()) == NULL) { Fp->References = 0; return FD32_EMFILE; }
     if (!(Mode & FD32_OALIAS)) AliasHint = 1;
     Res = fat_creat(Fp, Ff, Name, Attr, AliasHint);
     Fp->References = 0;
     if (Res < 0) { Ff->References = 0; return Res; }
     Res = set_opening_mode(Ff, Mode);
     if (Res < 0) { Ff->References = 0; return Res; }
     #ifdef FATNAMECACHE
      Ff->CacheLastAccess = Ff->V->BufferAccess;
      strcpy(Ff->CacheFileName, FileName);
     #endif
     *F = Ff;
     LOG_PRINTF(printf("FAT: File successfully created.\n"));
     return FD32_ORCREAT;
    #else
     Fp->References = 0;
     return FD32_EROFS;
    #endif
  }

  /* If we arrive here, the file already exists */
  if ((!(Mode & FD32_OEXIST)) && (!(Mode & FD32_OTRUNC))) { Fp->References = 0; return FD32_EACCES; }
  /* Allocate a file descriptor for the file and open it */

  if ((Ff = take_file()) == NULL) { Fp->References = 0; return FD32_EMFILE; }

  Res = open_existing(Fp, Ff, &D.SfnEntry, Mode);
  Fp->References = 0;

  if (Res < 0) { Ff->References = 0; return Res; }
  #ifdef FATNAMECACHE
   Ff->CacheLastAccess = Ff->V->BufferAccess;
   strcpy(Ff->CacheFileName, FileName);
  #endif

  /* Truncate the file to zero length if required */
  if (Mode & FD32_OTRUNC)
  {
    #ifdef FATWRITE
     if ((Res = fat_write(Ff, NULL, 0)) < 0) { Ff->References = 0; return Res; }
     *F = Ff;
     LOG_PRINTF(printf("FAT: File truncated as per FD32_OTRUNC.\n"));
     return FD32_ORTRUNC;
    #else
     Ff->References = 0;
     return FD32_EROFS;
    #endif
  }
  *F = Ff;
  LOG_PRINTF(printf("FAT: File successfully opened.\n"));
  return FD32_OROPEN;
}


/* Opens a directory knowing its first cluster, as required for */
/* a DOS-style FINDNEXt service.                                */
/* Returns 0 on success, or a negative error code on failure.   */
int fat_reopendir(tVolume *V, tFindRes *Id, tFile **F)
{
  int Res;
  
  if ((*F = take_file()) == NULL) return FD32_EMFILE;

  if (Id->FirstDirCluster == 0) (*F)->DirEntrySector = 0;
  (*F)->DirEntry.Attr      = FD32_ADIR;
  (*F)->DirEntry.FstClusHI = (WORD) (Id->FirstDirCluster >> 16);
  (*F)->DirEntry.FstClusLO = (WORD)  Id->FirstDirCluster;
  (*F)->TargetPos       = Id->EntryCount << 5;
  (*F)->FilePos         = 0;
  (*F)->Cluster         = Id->FirstDirCluster;
  (*F)->PrevCluster     = 0;
  (*F)->SectorInCluster = 0;
  (*F)->ByteInSector    = 0;
  
  Res = set_opening_mode(*F, FD32_OREAD | FD32_ODIR);
  if (Res < 0) { (*F)->References = 0; return Res; }
  return 0;
}


#ifdef FATWRITE
/* Flushes the content of volume's buffers and updates the directory entry. */
/* Returns 0 on success, or a negative error code on failure.               */
/* This is a public driver function.                                        */
int fat_fflush(tFile *F)
{
  int    Res;

  //LOG_PRINTF(printf("FAT fflush...\n"));
  #ifdef FATSHARE
  if ((Res = fat_syncentry(F)) < 0) return Res;
  #else
  if ((Res = write_direntry(F)) < 0) return Res;
  #endif

  return fat_flushall(F->V);
}
#endif /* #ifdef FATWRITE */


/* Decreases the counter of references of a file.             */
/* If the file has no more references, flushes buffers and    */
/* updates the file's directory entry.                        */
/* Returns 0 on success, or a negative error code on failure. */
/* This is a public driver function.                          */
int fat_close(tFile *F)
{
  if (F->References == 0) return FD32_EBADF;
  #ifdef FATWRITE
  if (F->References - 1 == 0)
  {
    int Res;
   
    if ((Res = fat_fflush(F)) < 0) return Res;
  }
  #endif

  LOG_PRINTF(printf("File closed. Volume buffers: %lu hits, %lu misses on %lu\n",
              F->V->BufferHit, F->V->BufferMiss, F->V->BufferAccess));
  return --F->References;
}

/**************************************************************************
 * FreeDOS32 File System Layer                                            *
 * Wrappers for file system driver functions, SFT and JFT support         *
 * by Salvo Isaja                                                         *
 *                                                                        *
 * Copyright (C) 2002, Salvatore Isaja                                    *
 *                                                                        *
 * The FreeDOS32 File System Layer is part of FreeDOS32.                  *
 *                                                                        *
 * FreeDOS32 is free software; you can redistribute it and/or modify it   *
 * under the terms of the GNU General Public License as published by the  *
 * Free Software Foundation; either version 2 of the License, or (at your *
 * option) any later version.                                             *
 *                                                                        *
 * FreeDOS32 is distributed in the hope that it will be useful, but       *
 * WITHOUT ANY WARRANTY; without even the implied warranty                *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with FreeDOS32; see the file COPYING; if not, write to the       *
 * Free Software Foundation, Inc.,                                        *
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA                *
 **************************************************************************/

#ifndef __FD32_FILESYS_H
#define __FD32_FILESYS_H

#include "devices.h"


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* File and path names limits, in bytes, including the NULL terminator */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define FD32_LFNPMAX 260 /* Max length for a long file name path  */
#define FD32_LFNMAX  255 /* Max length for a long file name       */
#define FD32_SFNPMAX 64  /* Max length for a short file name path */
#define FD32_SFNMAX  14  /* Max length for a short file name      */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* OPEN system call - Flags for opening mode and action taken  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Access codes */
#define FD32_OACCESS  0x0007    /* Bit mask for access type       */
#define FD32_OREAD    0x0000    /* Allow only reads from file     */
#define FD32_OWRITE   0x0001    /* Allow only writes into file    */
#define FD32_ORDWR    0x0002    /* Allow both reads and writes    */
#define FD32_ORDNA    0x0004    /* Read only without updating the */
                                /* last access date               */
/* Sharing modes and inheritance */
#define FD32_OSHARE   0x0070    /* Bit mask for sharing type      */
#define FD32_OCOMPAT  0x0000    /* Compatibility mode             */
#define FD32_ODENYRW  0x0010    /* Deny r/w by other handles      */
#define FD32_ODENYWR  0x0020    /* Deny write by other handles    */
#define FD32_ODENYRD  0x0030    /* Deny read by other handles     */
#define FD32_ODENYNO  0x0040    /* Allow full access by others    */
#define FD32_ONOINHER (1 << 7)  /* Child processes will not       */
                                /* inherit the file               */
/* Extended LFN open flags */
#define FD32_ONOBUFF  (1 << 8)  /* Do not use buffered I/O        */
#define FD32_ONOCOMPR (1 << 9)  /* Do not compress files (N/A)    */
#define FD32_OALIAS   (1 << 10) /* Use the numeric hint for alias */
#define FD32_ONOINT24 (1 << 13) /* Do not generate INT 24 on fail */
#define FD32_OCOMMIT  (1 << 14) /* Commit file at every write     */
/* Action to take */
#define FD32_OEXIST   (1 << 16) /* Open existing file             */
#define FD32_OTRUNC   (1 << 17) /* Truncate existing file         */
#define FD32_OCREAT   (1 << 20) /* Create unexisting file         */
#define FD32_ODIR     (1 << 24) /* Open a directory as a file     */
#define FD32_OFILEID  (1 << 25) /* Interpret *FileName as a fileid */


/* * * * * * * * * * * * * * * * * * * * * * * * */
/* CHMOD system call - Flags for file attributes */
/* * * * * * * * * * * * * * * * * * * * * * * * */
#define FD32_ARDONLY 0x01 /* Read only file                          */
#define FD32_AHIDDEN 0x02 /* Hidden file                             */
#define FD32_ASYSTEM 0x04 /* System file                             */
#define FD32_AVOLID  0x08 /* Volume label                            */
#define FD32_ADIR    0x10 /* Directory                               */
#define FD32_AARCHIV 0x20 /* File modified since last backup         */
#define FD32_ALNGNAM 0x0F /* Long file name directory slot (R+H+S+V) */
#define FD32_AALL    0x3F /* Select all attributes                   */
#define FD32_ANONE   0x00 /* Select no attributes                    */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Search flags for FINDFIRST/FINDNEXT and UNLINK system calls */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Allowable attributes */
#define FD32_FALLOW   0x00FF /* Allowable attributes mask */
#define FD32_FARDONLY FD32_ARDONLY
#define FD32_FAHIDDEN FD32_AHIDDEN
#define FD32_FASYSTEM FD32_ASYSTEM
#define FD32_FAVOLID  FD32_AVOLID
#define FD32_FADIR    FD32_ADIR
#define FD32_FAARCHIV FD32_AARCHIV
#define FD32_FAALL    FD32_AALL
#define FD32_FANONE   FD32_ANONE
/* Required attributes */
#define FD32_FREQUIRD 0xFF00 /* Allowable attributes mask */
#define FD32_FRRDONLY (FD32_ARDONLY << 8)
#define FD32_FRHIDDEN (FD32_AHIDDEN << 8)
#define FD32_FRSYSTEM (FD32_ASYSTEM << 8)
#define FD32_FRVOLID  (FD32_AVOLID  << 8)
#define FD32_FRDIR    (FD32_ADIR    << 8)
#define FD32_FRARCHIV (FD32_AARCHIV << 8)
#define FD32_FRALL    (FD32_AALL    << 8)
#define FD32_FRNONE   (FD32_ANONE   << 8)
/* Other search flags */
#define FD32_FDOSDATE (1 << 16) /* Use DOS date and time format */
#define FD32_FWILDCRD (1 << 17) /* Allow wildcards              */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* GEN_SHORT_FNAME system call - Mnemonics for flags and return values */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* TODO: Pack these huge mnemonics... */
#define FD32_GENSFN_FORMAT_MASK   (0xFF << 8)
#define FD32_GENSFN_FORMAT_FCB    (0x00 << 8)
#define FD32_GENSFN_FORMAT_NORMAL (0x01 << 8)

#define FD32_GENSFN_CASE_CHANGED  (1 << 0)
#define FD32_GENSFN_WAS_INVALID   (1 << 1)


/* * * * * * * * * * * * * * * * * * * * * * * */
/* TRUENAME system call - Mnemonics for flags  */
/* * * * * * * * * * * * * * * * * * * * * * * */
enum
{
  FD32_TNSUBST = 1 << 0, /* Resolve SUBSTed drives */
  FD32_TNDOTS  = 1 << 1  /* Collapse dot entries   */
};


/* * * * * * * * * */
/* Data structures */
/* * * * * * * * * */


/* Parameters structure for "get/set attributes" functions */
typedef struct
{
  DWORD Size;  /* Size in bytes of this structure                */
  WORD  Attr;  /* Standard DOS file attributes mask              */
  WORD  MDate; /* Last modification date in DOS format           */
  WORD  MTime; /* Last modification time in DOS format           */
  WORD  ADate; /* Last access date in DOS format                 */
  WORD  CDate; /* File creation date in DOS format               */
  WORD  CTime; /* File creation time in DOS format               */
  WORD  CHund; /* Hundredths of second of the file creation time */
}
fd32_fs_attr_t;


/* Format of the finddata record for LFN search operations */
__packed typedef struct
{
  DWORD Attr;          /* File attributes                                 */
  QWORD CTime;         /* File creation time in Win32 or DOS format       */
  QWORD ATime;         /* Last access time in Win32 or DOS format         */
  QWORD MTime;         /* Last modification time in Win32 or DOS format   */
  DWORD SizeHi;        /* High 32 bits of the file size in bytes          */
  DWORD SizeLo;        /* Low 32 bits of the file size in bytes           */
  BYTE  Reserved[8];   /* Reserved bytes, must be ignored                 */
  char  LongName[260]; /* Null-terminated long file name in Unicode UTF-8 */
  char  ShortName[14]; /* Null-terminated short file name in ASCII        */
}
fd32_fs_lfnfind_t;


/* Format of the finddata record and search status for DOS style search */
__packed typedef struct
{
  /* Private fields to store the search status */
  BYTE  SearchDrive;        /* A=1 etc., remote if bit 7 set               */
  BYTE  SearchTemplate[11]; /* Name to find in absolute format (? allowed) */
  BYTE  SearchAttr;         /* Search attributes                           */
  BYTE  Reserved[8];        /* Can be used to store the directory position */
  /* Public fields to store the search result */
  BYTE  Attr;               /* File attributes                             */
  WORD  MTime;              /* Last modification time in DOS format        */
  WORD  MDate;              /* Last modification date in DOS format        */
  DWORD Size;               /* File size in bytes                          */
  char  Name[13];           /* ASCIZ short file name found                 */
}
fd32_fs_dosfind_t;


/* Parameters structure for the "get file system informations" function */
typedef struct
{
  DWORD  Size;       /* IN:  Size in bytes of this structure    */
  DWORD  FSNameSize; /* IN:  Size in bytes of the FSName buffer */
  char  *FSName;     /* OUT: Buffer to store the FS name        */
  DWORD  Flags;      /* OUT: File system flags                  */
  DWORD  NameMax;    /* OUT: Max length of a file name          */
  DWORD  PathMax;    /* OUT: Max length of a path               */
}
fd32_fs_info_t;
/* File system flags can be a combination of the following */
enum
{
  FD32_FSICASESENS = 1 << 0,  /* Searches are case sensitive         */
  FD32_FSICASEPRES = 1 << 1,  /* Preserves case in directory entries */
  FD32_FSIUNICODE  = 1 << 2,  /* Uses Unicode in file names          */
  FD32_FSILFN      = 1 << 14, /* Supports DOS LFN services           */
  FD32_FSICOMPR    = 1 << 15  /* Volume is compressed                */
};


/* * * * * * * * * * * */
/* Function prototypes */
/* * * * * * * * * * * */

/* DIR.C - Directory management functions */
int  fd32_mkdir            (char *DirName);
int  fd32_rmdir            (char *DirName);

/* DRIVES.C - Drives management functions */
int  fd32_add_fs(fd32_request_t *request);
int  fd32_get_drive(char *FileSpec, fd32_request_t **request, void **DeviceId,
                    char **Path);
int  fd32_set_default_drive(char Drive);
char fd32_get_default_drive();
void fd32_set_boot_device  (DWORD MultiBootId);

/* FS.C - File system functions */
int  fd32_open             (char *FileName, DWORD Mode, WORD Attr, WORD AliasHint, int *Result);
int  fd32_close            (int Handle);
int  fd32_fflush           (int Handle);
int  fd32_read             (int Handle, void *Buffer, int Size);
int  fd32_write            (int Handle, void *Buffer, int Size);
int  fd32_unlink           (char *FileName, DWORD Flags);
int  fd32_lseek            (int Handle, long long int Offset, int Origin, long long int *Result);
int  fd32_dup              (int Handle);
int  fd32_forcedup         (int Handle, int NewHandle);
int  fd32_get_attributes   (int Handle, fd32_fs_attr_t *A);
int  fd32_set_attributes   (int Handle, fd32_fs_attr_t *A);
int  fd32_rename           (char *OldName, char *NewName);
int  fd32_get_fsinfo       (char *PathName, fd32_fs_info_t *FSInfo);
int  fd32_get_fsfree       (char *PathName, fd32_getfsfree_t *FSFree);


/* FIND.C - Find services */
int  fd32_dos_findfirst    (char *FileSpec, BYTE Attrib, fd32_fs_dosfind_t *Dta);
int  fd32_dos_findnext     (fd32_fs_dosfind_t *Dta);
int  fd32_lfn_findfirst    (char *FileSpec, DWORD Flags, fd32_fs_lfnfind_t *FindData);
int  fd32_lfn_findnext     (int Handle, DWORD Flags, fd32_fs_lfnfind_t *FindData);
int  fd32_lfn_findclose    (int Handle);

/* NAMES.C - File name management services */
int  fd32_gen_short_fname  (char *Dest, char *Source, DWORD Flags);
int  fd32_build_fcb_name   (BYTE *Dest, char *Source);
int  fd32_expand_fcb_name  (char *Dest, BYTE *Source);
int  fd32_compare_fcb_names(BYTE *Name1, BYTE *Name2);

/* TRUENAME.C - File name canonicalization, current directory and subst */
int  fd32_create_subst     (char *DriveAlias, char *Target);
int  fd32_terminate_subst  (char *DriveAlias);
int  fd32_query_subst      (char *DriveAlias, char *Target);
int  fd32_chdir            (char *DirName);
int  fd32_getcwd           (const char *Drive, char *Dest);
int  fd32_truename         (char *Dest, char *Source, DWORD Flags);
int  fd32_sfn_truename     (char *Dest, char *Source);

/* WILDCARD.C - Compare file name with wildcards support */
int  utf8_fnameicmp        (char *s1, char *s2);

#endif /* ifndef __FD32_FILESYS_H */


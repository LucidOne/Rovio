/**************************************************************************
 * FreeDOS 32 FAT Driver                                                  *
 * by Salvo Isaja                                                         *
 *                                                                        *
 * Copyright (C) 2001-2003, Salvatore Isaja                               *
 *                                                                        *
 * This is "lfn.c" - Convert long file names in the standard DOS          *
 *                   directory entry format and generate short name       *
 *                   aliases from long file names                         *
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
 #define LOG_PRINTF(s) fd32_log_printf s
#else
 #define LOG_PRINTF(s)
#endif


/* Calculate the 8-bit checksum for the long file name from its */
/* corresponding short name.                                    */
/* Called by split_lfn and find (find.c).                       */
BYTE lfn_checksum(tDirEntry *D)
{
  int Sum = 0, i;
  for (i = 0; i < 11; i++)
  {
    Sum = (((Sum & 1) << 7) | ((Sum & 0xFE) >> 1)) + D->Name[i];
  }
  return Sum;
}


#ifdef FATWRITE
#if 0
/* Returns nonzero if a UTF-8 string is a valid FAT long file name. */
/* Called by allocate_lfn_dir_entries (direntry.c).                 */
int lfn_is_valid(char *s)
{
  for (; *s; s++)
  {
    if (Ch < 0x20) return 0;
    switch (Ch)
    {
      /* + , ; = [ ] . are not valid for short names but are ok for LFN */
      case 0x22 : /* " */
      case 0x2A : /* * */
      case 0x2F : /* / */
      case 0x3A : /* : */
      case 0x3C : /* < */
      case 0x3E : /* > */
      case 0x3F : /* ? */
      case 0x5C : /* \ */
      case 0x7C : /* | */ return 0;
    }
  }
  return 1;
}
#endif


int oemcp_skipchar(char *Dest);

/* Generate a valid 8.3 file name for a long file name, and makes sure */
/* the generated name is unique in the specified directory appending a */
/* "~Counter" if necessary.                                            */
/* Returns 0 if the passed long name is already a valid 8.3 file name  */
/* (including upper case), thus no extra directory entry is required.  */
/* Returns a positive number on successful generation of a short name  */
/* alias for a long file name which is not a valid 8.3 name.           */
/* Returns a negative error code on failure.                           */
/* Called by allocate_lfn_dir_entries (direntry.c).                    */
int gen_short_fname(tFile *Dir, char *LongName, BYTE *ShortName, WORD Hint)
{
  BYTE       Aux[11];
  BYTE       szCounter[6];
  int        Counter, szCounterLen;
  int        k, Res;
  char      *s;
  tDirEntry  E;

  LOG_PRINTF(printf("Generating unique short file name for \"%s\"\n", LongName));
  Res = fd32_gen_short_fname((char *)ShortName, LongName, FD32_GENSFN_FORMAT_FCB);
  LOG_PRINTF(printf("fd32_gen_short_fname returned %08x\n", Res));
  if (Res <= 0) return Res;
  /* TODO: Check case change! */
  if (!(Res & FD32_GENSFN_WAS_INVALID)) return 1;

  /* Now append "~Counter" to the short name, incrementing "Counter" */
  /* until the file name is unique in that Path                      */
  for (Counter = Hint; Counter < 65536; Counter++)
  {
    memcpy(Aux, ShortName, 11);
    itoa(Counter, szCounter, 10);
    szCounterLen = strlen((void*)szCounter);
    for (k = 0; k < 7 - szCounterLen; k += oemcp_skipchar((char *)&Aux[k]));
    /* Append the "~Counter" to the name */
    /* TODO: The "~Counter" shouldn't be right justified if name is shorter than 8! */
    Aux[k++] = '~';
    for (s = (char *)szCounter; *s; Aux[k++] = *s++);

    /* Search for the generated name */
    LOG_PRINTF(printf("Checking if ~%i makes the name unique\n", Counter));
    Dir->TargetPos = 0;
    for (;;)
    {
      if ((Res = fat_read(Dir, (void *)&E, sizeof(tDirEntry))) < 0) return Res;
      if ((Res == 0) || (E.Name[0] == 0))
      {
        memcpy(ShortName, Aux, 11);
        return 1;
      }
      if (fd32_compare_fcb_names((BYTE*)E.Name, Aux) == 0) break;
    }
  }
  return FD32_EACCES;
}
#endif /* #ifdef FATWRITE */

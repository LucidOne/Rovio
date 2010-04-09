/**************************************************************************
 * FreeDOS32 File System Layer                                            *
 * Wrappers for file system driver functions and JFT support              *
 * by Salvo Isaja                                                         *
 *                                                                        *
 * Copyright (C) 2002-2003, Salvatore Isaja                               *
 *                                                                        *
 * This is "names.c" - Generate short (8.3) name alias for long file      *
 *                     names and manage FCB-format names.                 *
 *                                                                        *
 *                                                                        *
 * This file is part of the FreeDOS32 File System Layer (the SOFTWARE).   *
 *                                                                        *
 * The SOFTWARE is free software; you can redistribute it and/or modify   *
 * it under the terms of the GNU General Public License as published by   *
 * the Free Software Foundation; either version 2 of the License, or (at  *
 * your option) any later version.                                        *
 *                                                                        *
 * The SOFTWARE is distributed in the hope that it will be useful, but    *
 * WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with the SOFTWARE; see the file GPL.txt;                         *
 * if not, write to the Free Software Foundation, Inc.,                   *
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA                *
 **************************************************************************/
#include "fat.h"


/* TODO: Flags for code page of long and short name are not used. */


int oemcp_to_utf8(char *Source, UTF8 *Dest);
int utf8_to_oemcp(UTF8 *Source, int SourceSize, char *Dest, int DestSize);
int oemcp_skipchar(char *Dest);


/* Generates a valid 8.3 file name from a valid long file name. */
/* The generated short name has not a numeric tail.             */
/* Returns 0 on success, or a negative error code on failure.   */
int fd32_gen_short_fname(char *Dest, char *Source, DWORD Flags)
{
  BYTE  ShortName[11] = "           ";
  char  Purified[FD32_LFNPMAX]; /* A long name without invalid 8.3 characters */
  char *DotPos = NULL; /* Position of the last embedded dot in Source */
  char *s;
  int   Res = 0;

  /* Find the last embedded dot, if present */
  if (!(*Source)) return FD32_EFORMAT;
  for (s = Source + 1; *s; s++)
    if ((*s == '.') && (*(s-1) != '.') && (*(s+1) != '.') && *(s+1))
      DotPos = s;

  /* Convert all characters of Source that are invalid for short names */
  for (s = Purified; *Source;)
    if (!(*Source & 0x80))
    {
      if ((*Source >= 'a') && (*Source <= 'z'))
      {
        *s++ = *Source + 'A' - 'a';
        Res |= FD32_GENSFN_CASE_CHANGED;
      }
      else if (*Source < 0x20) return FD32_EFORMAT;
      else switch (*Source)
      {
        /* Spaces and periods must be removed */
        case ' ': break;
        case '.': if (Source == DotPos) DotPos = s, *s++ = '.';
                                   else Res |= FD32_GENSFN_WAS_INVALID;
                  break;
        /* + , ; = [ ] are valid for LFN but not for short names */
        case '+': case ',': case ';': case '=': case '[': case ']':
          *s++ = '_'; Res |= FD32_GENSFN_WAS_INVALID; break;
        /* Check for invalid LFN characters */
        case '"': case '*' : case '/': case ':': case '<': case '>':
        case '?': case '\\': case '|':
          return FD32_EFORMAT;
        /* Return any other single-byte character unchanged */
        default : *s++ = *Source;
      }
      Source++;
    }
    else /* Process extended characters */
    {
      UTF32 Ch, upCh;
      int   Skip;

      if ((Skip = fd32_utf8to32(Source, &Ch)) < 0) return FD32_EUTF8;
      Source += Skip;
      upCh = unicode_toupper(Ch);
      if (upCh != Ch) Res |= FD32_GENSFN_CASE_CHANGED;
      if (upCh < 0x80) Res |= FD32_GENSFN_WAS_INVALID;
      s += fd32_utf32to8(upCh, s);
    }
  *s = 0;

  /* Convert the Purified name to the OEM code page in FCB format */
  /* TODO: Must report WAS_INVALID if an extended char maps to ASCII! */
  if (utf8_to_oemcp((UTF8 *)Purified, DotPos ? DotPos - Purified : -1, (char *)ShortName, 8))
    Res |= FD32_GENSFN_WAS_INVALID;
  if (DotPos) if (utf8_to_oemcp((UTF8 *)DotPos + 1, -1, (char *)&ShortName[8], 3))
                Res |= FD32_GENSFN_WAS_INVALID;
  if (ShortName[0] == ' ') return FD32_EFORMAT;
  if (ShortName[0] == 0xE5) Dest[0] = (char) 0x05;

  /* Return the generated short name in the specified format */
  switch (Flags & FD32_GENSFN_FORMAT_MASK)
  {
    case FD32_GENSFN_FORMAT_FCB    : memcpy(Dest, ShortName, 11);
                                     return Res;
    case FD32_GENSFN_FORMAT_NORMAL : fd32_expand_fcb_name(Dest, ShortName);
                                     return Res;
    default                        : return FD32_EINVAL;
  }
}


/* Given a file name Source in UTF-8, checks if it's valid */
/* and returns in Dest the file name in FCB format.        */
/* On success, returns 0 if no wildcards are present, or a */
/* positive number if '?' wildcards are present in Dest.   */
/* On failure, returns a negative error code.              */
int fd32_build_fcb_name(BYTE *Dest, char *Source)
{
  int   WildCards = 0;
  char  SourceU[FD32_LFNPMAX];
  int   Res;
  int   k;

  /* Name and extension have to be padded with spaces */
  memset(Dest, 0x20, 11);
  
  /* Build ".          " and "..         " if Source is "." or ".." */
  if ((strcmp(Source, ".") == 0) || (strcmp(Source, "..") == 0))
  {
    for (; *Source; Source++, Dest++) *Dest = *Source;
    return 0;
  }

  if ((Res = utf8_strupr(SourceU, Source)) < 0) return FD32_EUTF8;
  for (k = 0; (SourceU[k] != '.') && SourceU[k]; k++);
  utf8_to_oemcp((UTF8 *)SourceU, SourceU[k] ? k : -1, (char*)Dest, 8);
  if (SourceU[k]) utf8_to_oemcp((UTF8 *)&SourceU[k + 1], -1, (char *)&Dest[8], 3);

  if (Dest[0] == ' ') return FD32_EFORMAT;
  if (Dest[0] == 0xE5) Dest[0] = 0x05;
  for (k = 0; k < 11;)
  {
    if (Dest[k] < 0x20) return FD32_EFORMAT;
    switch (Dest[k])
    {
      case '"': case '+': case ',': case '.': case '/': case ':': case ';':
      case '<': case '=': case '>': case '[': case '\\': case ']':  case '|':
        return FD32_EFORMAT;
      case '?': WildCards = 1;
                k++;
                break;
      case '*': WildCards = 1;
                if (k < 8) for (; k < 8; k++) Dest[k] = '?';
                else for (; k < 11; k++) Dest[k] = '?';
                break;
      default : k += oemcp_skipchar((char *)&Dest[k]);
    }
  }
  return WildCards;
}


/* Gets a UTF-8 short file name from an FCB file name. */
int fd32_expand_fcb_name(char *Dest, BYTE *Source)
{
  BYTE *NameEnd;
  BYTE *ExtEnd;
  char  Aux[13];
  BYTE *s = Source;
  char *a = Aux;

  /* Count padding spaces at the end of the name and the extension */
  for (NameEnd = Source + 7;  *NameEnd == ' '; NameEnd--);
  for (ExtEnd  = Source + 10; *ExtEnd  == ' '; ExtEnd--);

  /* Put the name, a dot and the extension in Aux */
  if (*s == 0x05) *a++ = (char) 0xE5, s++;
  for (; s <= NameEnd; *a++ = (char) *s++);
  if (Source + 8 <= ExtEnd) *a++ = '.';
  for (s = Source + 8; s <= ExtEnd; *a++ = (char) *s++);
  *a = 0;

  /* And finally convert Aux from the OEM code page to UTF-8 */
  return oemcp_to_utf8(Aux, (UTF8 *)Dest);
}


/* Compares two file names in FCB format. Name2 may contain '?' wildcards. */
/* Returns zero if the names match, or nonzero if they don't match.        */
/* TODO: Works only on single byte character sets!                       */
int fd32_compare_fcb_names(BYTE *Name1, BYTE *Name2)
{
  int k;
  for (k = 0; k < 11; k++)
  {
    if (Name2[k] == '?') continue;
    if (Name1[k] != Name2[k]) return -1;
  }
  return 0;
}


#if 0
int main()
{
  int  Res;
  char Name[14];

  memset(Name, 0, sizeof(Name));
  #if 1
  Res = fd32_gen_short_fname(Name, "...CaZ.ZĖ.TXT...",
                             FD32_GENSFN_FORMAT_NORMAL);
  #else
  Res = fd32_build_fcb_name(Name, "0123456789.01234");
  #endif
  printf("%i: \"%s\"\n", Res, Name);
  return 0;
}
#endif

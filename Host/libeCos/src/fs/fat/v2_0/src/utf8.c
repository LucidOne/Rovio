/**************************************************************************
 * FreeDOS 32 Unicode support library                                     *
 * by Salvo Isaja                                                         *
 *                                                                        *
 * Copyright (C) 2001-2002, Salvatore Isaja                               *
 *                                                                        *
 * This file is part of FreeDOS 32                                        *
 *                                                                        *
 * FreeDOS 32 is free software; you can redistribute it and/or modify it  *
 * under the terms of the GNU General Public License as published by the  *
 * Free Software Foundation; either version 2 of the License, or (at your *
 * option) any later version.                                             *
 *                                                                        *
 * FreeDOS 32 is distributed in the hope that it will be useful, but      *
 * WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with FreeDOS 32; see the file GPL.txt; if not, write to          *
 * the Free Software Foundation, Inc.                                     *
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA                *
 **************************************************************************/

#include "fat.h"

/* Converts a UTF-8 character to Unicode scalar value (same as UTF-32).  */
/* On success, returns the number of BYTEs taken by the character.       */
/* On failure, returns FD32_EUTF8.                                       */
/*                                                                       */
/* The conversion is done according to the following rules:              */
/*                                                                       */
/*           Scalar                               UTF-8                  */
/* 00000000 00000000 0xxxxxxx <-> 0xxxxxxx                               */
/* 00000000 00000yyy yyxxxxxx <-> 110yyyyy  10xxxxxx                     */
/* 00000000 zzzzyyyy yyxxxxxx <-> 1110zzzz  10yyyyyy  10xxxxxx           */
/* 000uuuuu zzzzyyyy yyxxxxxx <-> 11110uuu  10uuzzzz  10yyyyyy  10xxxxxx */
/*                                                                       */
/* NOTE: For optimization reasons, it is assumed that this function is   */
/* not called when the UTF-8 character is not multi-byte. In this case   */
/* the caller should process the single-byte character directly.         */
int fd32_utf8to32(const UTF8 *s, UTF32 *Ch)
{
  /* "if 0" means that it's assumed that the UTF-8 character is multi-byte */
  #if 0
  if (!(*s & 0x80))
  {
    *Ch = *s;
    return 1;
  }
  #endif
  if ((*s & 0xE0) == 0xC0)
  {
    *Ch = (*s++ & 0x8F) << 6;
    if ((*s & 0xC0) != 0x80) return FD32_EUTF8;
    *Ch += *s++ & 0x3F;
    return 2;
  }
  if ((*s & 0xF0) == 0xE0)
  {
    *Ch = (*s++ & 0x0F) << 12;
    if ((*s & 0xC0) != 0x80) return FD32_EUTF8;
    *Ch += (*s++ & 0x3F) << 6;
    if ((*s & 0xC0) != 0x80) return FD32_EUTF8;
    *Ch += *s++ & 0x3F;
    return 3;
  }
  if ((*s & 0xF8) == 0xF0)
  {
    *Ch = (*s++ & 0x07) << 18;
    if ((*s & 0xC0) != 0x80) return FD32_EUTF8;
    *Ch = (*s++ & 0x3F) << 12;
    if ((*s & 0xC0) != 0x80) return FD32_EUTF8;
    *Ch += (*s++ & 0x3F) << 6;
    if ((*s & 0xC0) != 0x80) return FD32_EUTF8;
    *Ch += *s++ & 0x3F;
    return 4;
  }
  return FD32_EUTF8;
}


/* Converts a Unicode scalar value (same as UTF-32) to UTF-8.         */
/* On success, returns the number of BYTEs taken by the character.    */
/* On failure, returns FD32_EUTF32 (invalid scalar value).            */
/* See fd32_utf8to32 comments for conversion details.                 */
/* NOTE: For optimization reasons, it is assumed that this function   */
/* is not called when the UTF-8 character is not multi-byte. In this  */
/* case the caller should process the single-byte character directly. */
int fd32_utf32to8(UTF32 Ch, UTF8 *s)
{
  /* "if 0" means that it's assumed that the UTF-8 character is multi-byte */
  #if 0
  if (Ch < 0x000080)
  {
    *s = (UTF8) Ch;
    return 1;
  }
  #endif
  if (Ch < 0x000800)
  {
    *s++ = (UTF8) (0xC0 + ((Ch & 0x0007C0) >> 6));
    *s   = (UTF8) (0x80 +  (Ch & 0x00003F));
    return 2;
  }
  if (Ch < 0x010000)
  {
    *s++ = (UTF8) (0xE0 + ((Ch & 0x00F000) >> 12));
    *s++ = (UTF8) (0x80 + ((Ch & 0x000FC0) >> 6));
    *s   = (UTF8) (0x80 +  (Ch & 0x00003F));
    return 3;
  }
  if (Ch < 0x200000)
  {
    *s++ = (UTF8) (0xF0 + ((Ch & 0xFC0000) >> 18));
    *s++ = (UTF8) (0x80 + ((Ch & 0x03F000) >> 12));
    *s++ = (UTF8) (0x80 + ((Ch & 0x000FC0) >> 6));
    *s   = (UTF8) (0x80 +  (Ch & 0x00003F));
    return 4;
  }
  return FD32_EUTF32;
}


/* Compares two Unicode UTF-8 strings, disregarding case.                */
/* Returns 0 if the strings match, a positive value if they don't match, */
/* or a FD32_EUTF8 if an invalid UTF-8 sequence is found.                */
int utf8_stricmp(const UTF8 *s1, const UTF8 *s2)
{
  UTF32 Ch1, Ch2;
  int   Res;

  for (;;)
  {
    if (!(*s2 & 0x80))
    {
      if (toupper(*s1) != toupper(*s2)) return 1;
      if (*s1 == 0) return 0;
      s1++;
      s2++;
    }
    else
    {
      if ((Res = fd32_utf8to32(s1, &Ch1)) < 0) return FD32_EUTF8;
      s1 += Res;
      if ((Res = fd32_utf8to32(s2, &Ch2)) < 0) return FD32_EUTF8;
      s2 += Res;
      if (unicode_toupper(Ch1) != unicode_toupper(Ch2)) return 1;
    }
  }
}


/* Converts a UTF-8 string to upper case.                          */
/* Returns 0 if case has not been changed (already in upper case), */
/* a positive number if case has been changed or FD32_EUTF8 if an  */
/* invalid UTF-8 sequence is found.                                */
int utf8_strupr(UTF8 *Dest, const UTF8 *Source)
{
  UTF32 Ch, upCh;
  int   CaseChanged = 0; /* Case not changed by default */
  int   Res;

  for (;;)
  {
    /* Try to do it faster on single-byte characters */
    if (!(*Source & 0x80))
    {
      if ((*Source >= 'a') && (*Source <= 'z'))
      {
        CaseChanged = 1;
        *Dest++ = *Source++ + 'A' - 'a';
      }
      else if ((*Dest++ = *Source++) == 0x00) return CaseChanged;
    }
    else /* Process extended characters */
    {
      if ((Res = fd32_utf8to32(Source, &Ch)) < 0) return FD32_EUTF8;
      Source += Res;
      upCh = unicode_toupper(Ch);
      if (upCh != Ch) CaseChanged = 1;
      Dest += fd32_utf32to8(upCh, Dest);
    }
  }
}

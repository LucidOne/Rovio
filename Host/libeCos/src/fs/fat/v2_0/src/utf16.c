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


/* Converts a UTF-16 character to Unicode scalar value (same as UTF-32).  */
/* On success, returns the number of WORDs taken by the character.        */
/* On failure, returns FD32_EUTF16.                                       */
/*                                                                        */
/* The conversion is done according to the following rules:               */
/*                                                                        */
/*           Scalar                              UTF-16                   */
/* 00000000 zzzzyyyy yyxxxxxx <-> zzzzyyyy yyxxxxxx                       */
/* 000uuuuu zzzzyyyy yyxxxxxx <-> 110110ww wwzzzzyy  110111yy yyxxxxxx    */
/* where wwww = uuuuu - 1.                                                */
int fd32_utf16to32(const UTF16 *s, UTF32 *Ch)
{
  if ((*s & 0xFC00) != 0xD800)
  {
    *Ch = *s;
    return 1;
  }
  *Ch = ((*s++ & 0x03FF) << 10) + 0x010000;
  if ((*s & 0xFC00) != 0xDC00) return FD32_EUTF16;
  *Ch += *s & 0x03FF;
  return 2;
}


/* Converts a Unicode scalar value (same as UTF-32) to UTF-16.     */
/* On success, returns the number of WORDs taken by the character. */
/* On failure, returns FD32_EUTF32 (invalid scalar value).         */
/* See fd32_utf16to32 comments for conversion details.             */
int fd32_utf32to16(UTF32 Ch, UTF16 *s)
{
  if (Ch < 0x010000)
  {
    *s = (UTF16) Ch;
    return 1;
  }
  if (Ch < 0x200000)
  {
    *s++ = (UTF16) (0xD800 + (((Ch >> 16) - 1) << 6) + ((Ch & 0x00FC00) >> 2));
    *s   = (UTF16) (0xDC00 + (Ch & 0x0003FF));
    return 2;
  }
  return FD32_EUTF32;
}

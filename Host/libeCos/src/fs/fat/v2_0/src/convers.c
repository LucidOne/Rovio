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


/* Converts a UTF-8 string into UTF-16.         */
/* Returns 0 on success, FD32_EUTF8 on failure. */
int fd32_utf8to16(const UTF8 *Utf8, UTF16 *Utf16)
{
  UTF32 Ch;
  int   Res;

  for (;;)
  {
    /* If the UTF-8 character is not multi-byte, process it directly. */
    if (!(*Utf8 & 0x80))
    {
      if ((*(Utf16++) = (UTF16) *(Utf8++)) == 0x0000) return 0;
    }
    else
    {
      if ((Res = fd32_utf8to32(Utf8, &Ch)) < 0) return FD32_EUTF8;
      Utf8  += Res;
      Utf16 += fd32_utf32to16(Ch, Utf16);
    }
  }
}


/* Converts a UTF-16 string into UTF-8.          */
/* Returns 0 on success, FD32_EUTF16 on failure. */
int fd32_utf16to8(const UTF16 *Utf16, UTF8 *Utf8)
{
  UTF32 Ch;
  int   Res;

  for (;;)
  {
    /* If the UTF-16 character fits in a single-byte UTF-8 character, */
    /* process it directly.                                           */
    if (*Utf16 < 0x0080)
    {
      if ((*(Utf8++) = (UTF8) *(Utf16++)) == 0x00) return 0;
    }
    else
    {
      if ((Res = fd32_utf16to32(Utf16, &Ch)) < 0) return FD32_EUTF16;
      Utf16 += Res;
      Utf8  += fd32_utf32to8(Ch, Utf8);
    }
  }
}

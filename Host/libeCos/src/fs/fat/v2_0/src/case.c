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

#include "toupper.h"

/* Returns the upper case form of a Unicode scalar character */
UTF32 unicode_toupper(UTF32 Ch)
{
  /* from LATIN SMALL LETTER A to LATIN SMALL LETTER Z */
  if ((Ch >= 0x0061) && (Ch <= 0x007A)) return Ch - 0x0020;

  /* from CYRILLIC SMALL LETTER A to CYRILLIC SMALL LETTER YA */
  if ((Ch >= 0x0430) && (Ch <= 0x044F)) return Ch - 0x0020;

  /* from FULLWIDTH LATIN SMALL LETTER A to FULLWIDTH LATIN SMALL LETTER Z */
  if ((Ch >= 0xFF41) && (Ch <= 0xFF5A)) return Ch - 0x0020;

  /* from DESERET SMALL LETTER LONG I to DESERET SMALL LETTER ENG */
  if ((Ch >= 0x10428) && (Ch <= 0x1044D)) return Ch - 0x0028;

  /* Binary search in the UCD_toupper table */
  if ((Ch >= 0x00B5) && (Ch <= 0x24E9))
  {
    unsigned Semi, Lo = 0, Hi = (sizeof(UCD_toupper) >> 2) - 1;

    for (;;)
    {
      Semi = (Lo + Hi) >> 1;
      if (UCD_toupper[Semi].Scalar == Ch) return UCD_toupper[Semi].ToUpper;
      if (Hi - Lo == 0) break;
      if (UCD_toupper[Semi].Scalar < Ch) Lo = Semi - 1;
                                    else Hi = Semi + 1;
    }
  }

  /* If we arrive here, we don't know how to put Ch in upper case */
  return Ch;
}

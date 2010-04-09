FD32 FAT DRIVER
by Salvo Isaja

A driver that allows to use the FAT file system, the standard
one for DOS and Windows machines, under FD32. The most important
features of the FAT Driver are: FAT12, FAT16 and FAT32 support,
file length up to 4 GiB, Long File Names, buffered I/O,
integrated sharing support, name cache for fast open.
For further informations, read the full documentation on the web:
  http://freedos-32.sourceforge.net/fat.html

HOW TO COMPILE FOR FREEDOS 32
Use the makefile "Makefile".
From the command line, use "make" to generate the "fat.o" object
file to be used as FD32 driver.
At present it has to be loaded after disk drivers.

HOW TO COMPILE FOR THE DOS TARGET
Use the makefile "Makefile.dj".
Use "make -f Makefile.dj" to generate the "fat.o" object file
targetted for the DJGPP libraries.

DEPENDENCES
- unicode.o
- devices.o
- cp437.o
The fat.o driver uses the Devices Engine (devices.o) for devices
management, the Unicode Support Library (unicode.o) and the local
code page support (only cp437.o at present) to deal with file
name strings.
- filesys.o
Load the FAT Driver after the File System Layer (filesys.o) in
order to let it know that a new file system driver is available
to mount file systems on its drives. The FAT Driver also uses the
file name management services of the FS Layer.

LICENSE AND DISCLAIMER
FreeDOS 32 FAT Driver
Copyright (C) 2001-2003, Salvatore Isaja

The FreeDOS 32 FAT Driver is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The FreeDOS 32 FAT Driver is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the FreeDOS32 FAT Driver; see the file COPYING;
if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

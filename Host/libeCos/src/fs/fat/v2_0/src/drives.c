/**************************************************************************
 * FreeDOS32 File System Layer                                            *
 * Wrappers for file system driver functions and JFT support              *
 * by Salvo Isaja                                                         *
 *                                                                        *
 * Copyright (C) 2002-2003, Salvatore Isaja                               *
 *                                                                        *
 * This is "drives.c" - Services to handle drives and assign letters.     *
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

#define NULBOOTDEV 0xFFFFFFFF

#define DRIVES_FIXED /* Define this to enable fixed letter assignment */

static DWORD BootDevice   = NULBOOTDEV; /* From MultiBoot info */
static char  DefaultDrive = 0; /* Drive letter, or 0 if not defined */
static int   Drives[26]; /* Bind drive letters to FD32 device handles */
#ifdef DRIVES_FIXED
static char *FixedLetters[26];
#endif

/* Registered file system driver request functions */
/* TODO: Make the size of this dynamic! */
static int             NumFSReq = 20;
static fd32_request_t *fsreq[20];


/* Assign drive letters dynamically, searching for block devices of the  */
/* specified type, as enumerated, starting from the passed drive number. */
/* Returns 0 on success, or a negative error code on failure.            */
/* Called by assign_drive_letters.                                       */
static int dynamic_assign(DWORD Type, int d)
{
#if 0//clyu
  int                hDev, Res;
  fd32_request_t    *request;
  void              *DeviceId;
  fd32_blockinfo_t   Bi;
  char               DevName[51];

  for (hDev = fd32_dev_first(); hDev != FD32_ENMDEV; hDev = fd32_dev_next(hDev))
  {
    /* Check if this device is already assigned (even with fixed letters) */
    for (Res = 0; Res < 26; Res++) if (Drives[Res] == hDev) continue;
    /* Get block device informations */
    fd32_dev_get(hDev, &request, &DeviceId, DevName, 50);
    Bi.Size     = sizeof(fd32_blockinfo_t);
    Bi.DeviceId = DeviceId;
    if (request(FD32_BLOCKINFO, &Bi) < 0) continue;
    /* If the block device type is not what we are searching we skip it */
    if (FD32_BITYP(Bi.Type) != Type) continue;
    /* If no file system driver can handle such a partition we skip it */
    if (FD32_BIPAR(Bi.Type))
    {
      fd32_partcheck_t Pc;
      Pc.Size   = sizeof(fd32_partcheck_t);
      Pc.PartId = FD32_BIPAR(Bi.Type);
      for (Res = 0; Res < NumFSReq; Res++)
        if (fsreq[Res]) if (fsreq[Res](FD32_PARTCHECK, &Pc) == 0) break;
      if (Res == NumFSReq) continue;
    }
    /* Assign the next available drive number (0 is 'A') to the device */
    while (Drives[d] != FD32_ENODEV) if (d++ == 'Z' - 'A') return FD32_ENODRV;
    Drives[d] = hDev;
    fd32_message("FS Layer: '%c' drive assigned to device '%s'\n", d + 'A', DevName);
    /* Set the default drive to this drive if boot device is known, */
    /* otherwise set the default drive to the first drive detected. */
    if (DefaultDrive == 0)
      if ((BootDevice == NULBOOTDEV) || (BootDevice == Bi.MultiBootId))
        DefaultDrive = d + 'A';
  }
  return 0;
#endif
	return FD32_ENODRV;
}


static int assign_drive_letters(void)
{
#if 0//clyu
  #ifdef DRIVES_FIXED
  int hDev;
  #endif
  int k;

  /* Initialize the Drives array with all drives unassigned */
  for (k = 0; k < 26; k++) Drives[k] = FD32_ENODEV;

  #ifdef DRIVES_FIXED
  /* Assign fixed drive letters */
  memset(FixedLetters, 0, sizeof(FixedLetters));
  
  FixedLetters[0] = "rom1";
//  FixedLetters['C' - 'A'] = "hda5";
//  FixedLetters['D' - 'A'] = "hda5";
  for (k = 0; k < 26; k++)
    if (FixedLetters[k])
    {
      if ((hDev = fd32_dev_search(FixedLetters[k])) < 0)
        fd32_message("Warning: drive %c can not be %s, device not found",
                     k + 'A', FixedLetters[k]);
       else
        /* TODO: Should check if hDev is a valid block device! */
        Drives[k] = hDev;
    }
  #endif

  /* Assign remaining drive letters according to the Win2000 behaviour */
  dynamic_assign(FD32_BIACT, 'C' - 'A'); /* All active partitions first */
  dynamic_assign(FD32_BILOG, 'C' - 'A'); /* Logical drives or removable */
  dynamic_assign(FD32_BIPRI, 'C' - 'A'); /* Other primary partitions    */
  dynamic_assign(FD32_BIFLO, 'A' - 'A'); /* Floppy drives               */
  dynamic_assign(FD32_BICD,  'D' - 'A'); /* CD-ROM drives (CD-R? DVD?)  */
  return 0;
#endif
	return FD32_ENODEV;
}


/* Uses all the registered file system drivers to try to   */
/* mount a file system in the specified device.            */
/* On success, returns the index of the file system driver */
/* request function and fills FsDev with a pointer to the  */
/* device identifier for the mounted.                      */
/* On failure, returns a negative error code.              */
/* At present it is only called by fd32_get_drive.         */
static int fd32_mount(DWORD hDev, void **FsDev)
{
  int          k, Res;
  fd32_mount_t M;

  M.Size = sizeof(fd32_mount_t);
  M.hDev = hDev;

  for (k = 0; k < NumFSReq; k++)
    if (fsreq[k])
    {
      if ((Res = fsreq[k](FD32_MOUNT, &M)) == 0)
      {
        *FsDev = M.FsDev;
        return k;
      }
      if (Res != FD32_EMEDIA) return Res;
    }
  return FD32_EMEDIA;
}


/* Gets the file system driver request funciton and the file system device    */
/* identifier for the drive specified in a file name, if present, or to the   */
/* default drive letter if not.                                               */
/* A drive specification can be a drive letter or a valid device name.        */
/* If the device is not mounted the fd32_mount function is called in order    */
/* to try to mount a file system.                                             */
/* On success, returns the detected drive letter (if defined, 0 if not),      */
/* fills request with the pointer to the file system driver request function, */
/* DeviceId with the file system device identifier, and Path with a pointer   */
/* to the beginning of the path, removing the drive specification.            */
/* Returns a negative error code on failure.                                  */
/* TODO: Doesn't fill Path if device name is provided */
int fd32_get_drive(char *FileSpec, fd32_request_t **request, void **DeviceId,
                   char **Path)
{

#if 0//clyu
  fd32_request_t   *blkreq; /* Request function for the block device    */
  void             *BlkDev; /* Identifier of the block device           */
  fd32_ismounted_t  Im;     /* To check if the drive is already mounted */
  char              Drive[FD32_LFNPMAX]; /* To store the drive specification */
  int               Res; /* Return codes */
  int               Letter    = DefaultDrive; /* Default drive if nothing is specified */
  int               hDev      = Drives[DefaultDrive - 'A']; /* Default drive if nothing is specified */
  char             *pDrive    = Drive;    /* Pointer into the Drive string     */
  char             *pFileName = FileSpec; /* Pointer into the FileSpec string */

  /* Copy the drive specification (if present) into the Drive string */
  if (Path) *Path = FileSpec;
  for (; *pFileName && (*pFileName != ':'); *(pDrive++) = *(pFileName++));
  /* If a drive has been specified, use it instead of the default drive */
  if (*pFileName)
  {
    *pDrive = 0;
    /* If a 1-byte drive has been specified, use the drive letters array */
    if (Drive[1] == 0)
    {
      Letter = toupper(Drive[0]);
      if ((Letter < 'A') || (Letter > 'Z')) return FD32_ENODRV;
      if (Path) *Path = pFileName + 1;
      hDev = Drives[Letter - 'A'];
    }
    else /* search for a device with the specified name */
    {
      Letter = 0;
      if ((hDev = fd32_dev_search(Drive)) < 0) return FD32_ENODRV;
    }
  }
  if ((Res = fd32_dev_get(hDev, &blkreq, &BlkDev, NULL, 0)) < 0) return Res;
  /* Check if it's mounted */
  Im.Size     = sizeof(fd32_ismounted_t);
  Im.DeviceId = BlkDev;
  Res = blkreq(FD32_ISMOUNTED, &Im);
  if (Res == 0)
  {
    *request  = Im.fsreq;
    *DeviceId = Im.FSDevId;
    return Letter;
  }
  if (Res != FD32_EINVAL) return Res; /* TODO: Function not supported */
  /* If it's not mounted, mount it */
  if ((Res = fd32_mount(hDev, DeviceId)) < 0) return Res;
  *request = fsreq[Res];
  return Letter;
#endif
	return FD32_ENODRV;
}


#if 0
/* Returns the file name without the drive specification part. */
/* At present it is useless.                                   */
static char *get_name_without_drive(char *FileName)
{
  char *s;
  for (s = FileName; *s; s++)
    if (*s == ':') return s + 1;
  return FileName;
}
#endif


/* The SET DEFAULT DRIVE system call.                 */
/* Returns the number of available drives on success, */
/* or a negative error code on failure.               */
int fd32_set_default_drive(char Drive)
{
  /* TODO: LASTDRIVE should be read frm Config.sys */
  if ((toupper(Drive) < 'A') || (toupper(Drive) > 'Z')) return FD32_ENODRV;
  if (Drives[Drive - 'A'] != FD32_ENODEV)
    DefaultDrive = toupper(Drive);
  /* TODO: From the RBIL (INT 21h, AH=0Eh)
           under DOS 3.0+, the return value is the greatest of 5,
           the value of LASTDRIVE= in CONFIG.SYS, and the number of
           drives actually present. */
  return 'Z' - 'A' + 1;
}


/* The GET DEFAULT DRIVE system call.               */
/* Returns the letter of the current default drive. */
char fd32_get_default_drive()
{
  return DefaultDrive;
}


/* Registers a new file system driver request function to the FS Layer */
int fd32_add_fs(fd32_request_t *request)
{
  int k;
  for (k = 0; k < NumFSReq; k++)
    if (!fsreq[k])
    {
      fsreq[k] = request;
      return assign_drive_letters();
    }
  return FD32_ENOMEM;
}


void fd32_set_boot_device(DWORD MultiBootId)
{
  BootDevice = MultiBootId;
}

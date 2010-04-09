#ifndef __FD32_ERRORS_H
#define __FD32_ERRORS_H

/* Error codes */
#define FD32_E(x)     (0x80000000 | (x))
#define FD32_EINVAL   FD32_E(0x01) /* Invalid argument        */
#define FD32_ENOENT   FD32_E(0x02) /* (FIXME: file not found) No such file or directory */
#define FD32_ENOTDIR  FD32_E(0x03) /* (FIXME: path not found) Not a directory */
#define FD32_EMFILE   FD32_E(0x04) /* Too many open files     */
#define FD32_EACCES   FD32_E(0x05) /* Access denied           */
#define FD32_EBADF    FD32_E(0x06) /* Invalid file handle     */
#define FD32_ENOMEM   FD32_E(0x08) /* Insufficient memory     */
#define FD32_EFORMAT  FD32_E(0x0B) /* Format invalid          */
#define FD32_EACODE   FD32_E(0x0C) /* Access code invalid     */
#define FD32_EIDATA   FD32_E(0x0D) /* Data invalid            */
#define FD32_ENODRV   FD32_E(0x0F) /* Invalid drive           */
#define FD32_EBUSY    FD32_E(0x10) /* Attempt to remove the current directory */
#define FD32_EXDEV    FD32_E(0x11) /* Not same device         */
#define FD32_ENMFILE  FD32_E(0x12) /* No more files           */
#define FD32_EROFS    FD32_E(0x13) /* Read-only file system   */
#define FD32_ENODEV   FD32_E(0x14) /* No such device          */
#define FD32_ENOTRDY  FD32_E(0x15) /* Drive not ready         */
#define FD32_ECRC     FD32_E(0x17) /* CRC error               */
#define FD32_EISEEK   FD32_E(0x19) /* Invalid seek            */
#define FD32_EMEDIA   FD32_E(0x1A) /* Unknown media (not DOS) */
#define FD32_ENOSEC   FD32_E(0x1B) /* Sector not found        */
#define FD32_EWRITE   FD32_E(0x1D) /* Write fault             */
#define FD32_EREAD    FD32_E(0x1E) /* Read fault              */
#define FD32_EGENERAL FD32_E(0x1F) /* General failure         */
#define FD32_EVSHAR   FD32_E(0x20) /* Sharing violation       */
#define FD32_EVLOCK   FD32_E(0x21) /* Lock violation          */
#define FD32_ECHANGE  FD32_E(0x22) /* Invalid media change (ES:DI -> media ID structure)(see #1546) */
#define FD32_EOINPUT  FD32_E(0x26) /* Out of input            */
#define FD32_ENOSPC   FD32_E(0x27) /* No space left on drive  */
#define FD32_EEXIST   FD32_E(0x50) /* File exists             */
#define FD32_EMKDIR   FD32_E(0x52) /* Cannot make directory   */
#define FD32_EINT24   FD32_E(0x53) /* Fail on INT 24          */
#define FD32_ENOTLCK  FD32_E(0xB0) /* Not locked              */
#define FD32_ELOCKED  FD32_E(0xB1) /* Locked in drive         */
#define FD32_ENOTREM  FD32_E(0xB2) /* Media not removable     */
#define FD32_ENOLCK   FD32_E(0xB4) /* No more locks available */
#define FD32_EEJECT   FD32_E(0xB5) /* Eject request failed    */
/* FD32 defined error codes */
#define FD32_ENMOUNT  FD32_E(0x100) /* File system not mounted */
#define FD32_EUTF8    FD32_E(0x101) /* Invalid UTF-8 sequence  */
#define FD32_EUTF16   FD32_E(0x102) /* Invalid UTF-32 sequence */
#define FD32_EUTF32   FD32_E(0x103) /* Invalid Unicode char    */
#define FD32_ENMDEV   FD32_E(0x104) /* No more devices         */


#endif /* #ifndef __FD32_ERRORS_H */


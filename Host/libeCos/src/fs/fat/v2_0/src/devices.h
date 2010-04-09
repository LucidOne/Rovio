#ifndef __FD32_DEV_H
#define __FD32_DEV_H

/* Driver request function prototype */
typedef int fd32_request_t(DWORD Function, void *Params);

/* Mnenomics for function numbers in a driver request function */
enum
{
  /* File System functions */
  FD32_READ = 0x200,
  FD32_WRITE,
  FD32_LSEEK,
  FD32_OPEN,
  FD32_CLOSE,
  FD32_FFLUSH,
  FD32_OPENFILE,
  FD32_MKDIR,
  FD32_RMDIR,
  FD32_MOUNT,
  FD32_PARTCHECK,
  FD32_READDIR,
  FD32_UNLINK,
  FD32_RENAME,
  FD32_UNMOUNT,
  FD32_GETATTR,
  FD32_SETATTR,
  FD32_REOPENDIR,
  FD32_GETFSINFO,
  FD32_GETFSFREE,
  /* Block device functions */
  FD32_BLOCKWRITE = 0x100,
  FD32_BLOCKREAD,
  FD32_BLOCKINFO,
  FD32_MEDIACHANGE,
  FD32_ISMOUNTED
};


/****************************************************************************/
/* READ - Read data from a file or character device                         */
/****************************************************************************/
typedef struct fd32_read
{
  DWORD  Size;        /* Size in bytes of this structure  */
  void  *DeviceId;    /* Identifier of the device or file */
  void  *Buffer;      /* Pointer to the transfer buffer   */
  DWORD  BufferBytes; /* Number of bytes to read          */
}
fd32_read_t;
/* Returns: the number of bytes actually read */


/****************************************************************************/
/* WRITE - Write data into a file or character device                       */
/****************************************************************************/
typedef struct fd32_write
{
  DWORD  Size;        /* Size in bytes of this structure  */
  void  *DeviceId;    /* Identifier of the device or file */
  void  *Buffer;      /* Pointer to the transfer buffer   */
  DWORD  BufferBytes; /* Number of bytes to write         */
}
fd32_write_t;
/* Returns: the number of bytes actually written */


/****************************************************************************/
/* LSEEK - Moves the file pointer of a file or character device             */
/****************************************************************************/
typedef struct fd32_lseek
{
  DWORD     Size;     /* Size in bytes of this structure      */
  void     *DeviceId; /* Identifier of the device or file     */
  long long int Offset;   /* IN: Byte offset to seek to           */
                      /* OUT: New byte offset from file start */
  DWORD     Origin;   /* Position from which to seek          */
}
fd32_lseek_t;
/* The origin parameter can be one of the following */
enum
{
  FD32_SEEKSET = 0, /* Seek from the beginning of the file */
  FD32_SEEKCUR = 1, /* Seek from the current file position */
  FD32_SEEKEND = 2  /* Seek from the end of file           */
};
/* Returns 0 on success */


/****************************************************************************/
/* OPEN - Increase the open references counter of a file or device          */
/****************************************************************************/
typedef struct fd32_open
{
  DWORD  Size;     /* Size in bytes of this structure  */
  void  *DeviceId; /* Identifier of the device or file */
}
fd32_open_t;
/* Returns the number of references after the call */


/****************************************************************************/
/* CLOSE - Decrease the open references counter of a file or device         */
/****************************************************************************/
typedef struct fd32_close
{
  DWORD  Size;     /* Size in bytes of this structure  */
  void  *DeviceId; /* Identifier of the device or file */
}
fd32_close_t;
/* Returns the number of references after the call */


/****************************************************************************/
/* FFLUSH - Commit all unwritten data to a file or device                   */
/****************************************************************************/
typedef struct fd32_fflush
{
  DWORD  Size;     /* Size in bytes of this structure  */
  void  *DeviceId; /* Identifier of the device or file */
}
fd32_fflush_t;
/* Returns 0 on success */


/****************************************************************************/
/* OPENFILE - Open, create or truncate a file                               */
/****************************************************************************/
typedef struct fd32_openfile
{
  DWORD  Size;      /* Size in bytes of this structure       */
  void  *DeviceId;  /* Identifier of the file system device  */
  void  *FileId;    /* OUT: Identifier for the open file     */
  char  *FileName;  /* Canonicalized file name               */
  DWORD  Mode;      /* File opening mode                     */
  WORD   Attr;      /* Attributes mask for creation          */
  WORD   AliasHint; /* FAT only: LFN alias hint for creation */
}
fd32_openfile_t;
/* On success returns a not negative code for the action taken */
enum
{
  FD32_OROPEN  = 1, /* File has been opened    */
  FD32_ORCREAT = 2, /* File has been created   */
  FD32_ORTRUNC = 3  /* File has been truncated */
};


/****************************************************************************/
/* MKDIR - Create directory                                                 */
/****************************************************************************/
typedef struct fd32_mkdir
{
  DWORD  Size;     /* Size in bytes of this structure      */
  void  *DeviceId; /* Identifier of the file system device */
  char  *DirName;  /* Canonicalized directory name         */
}
fd32_mkdir_t;
/* Returns 0 on success */


/****************************************************************************/
/* RMDIR - Remove directory                                                 */
/****************************************************************************/
//#define FD32_RMDIR 0x208
typedef struct fd32_rmdir
{
  DWORD  Size;     /* Size in bytes of this structure      */
  void  *DeviceId; /* Identifier of the file system device */
  char  *DirName;  /* Canonicalized directory name         */
}
fd32_rmdir_t;
/* Returns 0 on success */


/****************************************************************************/
/* MOUNT - Mount file system on block device                                */
/****************************************************************************/
typedef struct fd32_mount
{
  DWORD  Size;     /* Size in bytes of this structure             */
  DWORD  hDev;     /* Handle of the hosting block device          */
  void  *FsDev;    /* OUT: Identifier of the new FS device        */
}
fd32_mount_t;
/* Returns 0 on success */


/****************************************************************************/
/* PARTCHECK - Check if a file system driver can handle a partition type    */
/****************************************************************************/
typedef struct fd32_partcheck
{
  DWORD Size;   /* Size in bytes of this structure  */
  BYTE  PartId; /* Partition identifier             */
}
fd32_partcheck_t;
/* Returns zero if partition is supported, nonzero if not */


/****************************************************************************/
/* READDIR - Read a directory entry from an open directory                  */
/****************************************************************************/
typedef struct fd32_readdir
{
  DWORD  Size;  /* Size in bytes of this structure */
  void  *DirId; /* Identifier of the directory     */
  void  *Entry; /* OUT: LFN finddata structure     */
}
fd32_readdir_t;
/* Returns 0 on success */


/****************************************************************************/
/* UNLINK - Remove a closed file                                            */
/****************************************************************************/
typedef struct fd32_unlink
{
  DWORD  Size;     /* Size in bytes of this structure             */
  void  *DeviceId; /* Identifier of the file system device        */
  char  *FileName; /* Canonicalized file name (wildcards allowed) */
  DWORD  Flags;    /* Search flags (the same as find functions)   */
}
fd32_unlink_t;
/* Returns 0 on success */


/****************************************************************************/
/* RENAME - Rename a file or move a file across the same volume             */
/****************************************************************************/
typedef struct fd32_rename
{
  DWORD  Size;     /* Size in bytes of this structure          */
  void  *DeviceId; /* Identifier of the file system device     */
  char  *OldName;  /* Canonicalized name of the file to rename */
  char  *NewName;  /* Canonicalized new name                   */
}
fd32_rename_t;
/* Returns 0 on success */


/****************************************************************************/
/* UNMOUNT - Unmount file system on block device                            */
/****************************************************************************/
typedef struct fd32_unmount
{
  DWORD  Size;     /* Size in bytes of this structure      */
  void  *DeviceId; /* Identifier of the file system device */
}
fd32_unmount_t;
/* Returns 0 on success */


/****************************************************************************/
/* GETATTR - Get attributes and time stamps of a file                       */
/****************************************************************************/
typedef struct fd32_getattr
{
  DWORD  Size;   /* Size in bytes of this structure */
  void  *FileId; /* Identifier of the open file     */
  void  *Attr;   /* OUT: Attributes structure       */
}
fd32_getattr_t;
/* Returns 0 on success */


/****************************************************************************/
/* SETATTR - Set attributes and time stamps of a file                       */
/****************************************************************************/
typedef struct fd32_setattr
{
  DWORD  Size;   /* Size in bytes of this structure */
  void  *FileId; /* Identifier of the open file     */
  void  *Attr;   /* New attributes structure        */
}
fd32_setattr_t;
/* Returns 0 on success */


/****************************************************************************/
/* REOPENDIR - Reopen a directory to continue a DOS-style search service    */
/****************************************************************************/
typedef struct fd32_reopendir
{
  DWORD  Size;        /* Size in bytes of this structure            */
  void  *DeviceId;    /* Identifier of the file system device       */
  void  *DtaReserved; /* Pointer to the 8 reserved bytes in the DTA */
  void  *DirId;       /* OUT: Identifier of the open directory      */
}
fd32_reopendir_t;
/* Returns 0 on success */


/****************************************************************************/
/* GETFSINFO - Get file system informations                                 */
/****************************************************************************/
typedef struct fd32_getfsinfo
{
  DWORD  Size;     /* Size in bytes of this structure      */
  void  *DeviceId; /* Identifier of the file system device */
  void  *FSInfo;   /* OUT: File system info structure      */
}
fd32_getfsinfo_t;
/* Returns 0 on success */


/****************************************************************************/
/* GETFSFREE - Get free space on file system                                */
/****************************************************************************/
typedef struct fd32_getfsfree
{
  DWORD  Size;        /* Size in bytes of this structure           */
  void  *DeviceId;    /* Identifier of the file system device      */
  DWORD  SecPerClus;  /* OUT: Sectors per allocation unit          */
  DWORD  BytesPerSec; /* OUT: Bytes per sector                     */
  DWORD  AvailClus;   /* OUT: Number of available allocation units */
  DWORD  TotalClus;   /* OUT: Total number of allocation units     */
}
fd32_getfsfree_t;
/* Returns 0 on success */


/****************************************************************************/
/* BLOCKWRITE - Write a block of data with random access                    */
/****************************************************************************/
typedef struct fd32_blockwrite
{
  DWORD  Size;      /* Size in bytes of this structure       */
  void  *DeviceId;  /* Identifier of the device              */
  DWORD  Start;     /* Number of the first block to transfer */
  void  *Buffer;    /* Pointer to the transfer buffer        */
  DWORD  NumBlocks; /* Number of blocks to transfer          */
}
fd32_blockwrite_t;


/****************************************************************************/
/* BLOCKREAD - Read a block of data with random access                      */
/****************************************************************************/
typedef struct fd32_blockread
{
  DWORD  Size;      /* Size in bytes of this structure       */
  void  *DeviceId;  /* Identifier of the device              */
  DWORD  Start;     /* Number of the first block to transfer */
  void  *Buffer;    /* Pointer to the transfer buffer        */
  DWORD  NumBlocks; /* Number of blocks to transfer          */
}
fd32_blockread_t;


/****************************************************************************/
/* BLOCKINFO - Retrieve informations on a block device                      */
/****************************************************************************/
typedef struct fd32_blockinfo
{
  DWORD  Size;        /* Size in bytes of this structure       */
  void  *DeviceId;    /* Identifier of the device              */
  DWORD  BlockSize;   /* OUT: Size in bytes of a block         */
  DWORD  TotalBlocks; /* OUT: Number of blocks in the device   */
  DWORD  Type;        /* OUT: Block device type                */
  DWORD  MultiBootId; /* OUT: The MultiBoot boot device number */
}
fd32_blockinfo_t;
/* Type is defined as the following bit mask:                  */
/*  bits  3-0 : device type for DOS drive letter assignment;   */
/*  bits 11-4 : partition identifier if device is a partition, */
/*              zero if device is not a partition;             */
/*  bits 31-12: reserved for future use.                       */
/* Mask and values for bits 2-0 of the device type: */
#define FD32_BITYP(x) (x & 0x00F)
enum
{
  FD32_BIGEN = 0, /* Generic block device              */
  FD32_BIACT = 1, /* Active or first primary partition */
  FD32_BILOG = 2, /* Logical drive or removable drive  */
  FD32_BIPRI = 3, /* Other primary partition           */
  FD32_BIFLO = 4, /* Floppy drive                      */
  FD32_BICD  = 5  /* CD or DVD drive                   */
};
/* Mask for bits 11-4 of the device type (with shift in bits 7-0): */
#define FD32_BIPAR(x) ((x & 0xFF0) >> 4)


/****************************************************************************/
/* MEDIACHANGE - Report change-line status of a device                      */
/****************************************************************************/
typedef struct fd32_mediachange
{
  DWORD  Size;     /* Size in bytes of this structure      */
  void  *DeviceId; /* Identifier of the device             */
}
fd32_mediachange_t;


/****************************************************************************/
/* ISMOUNTED - Check if a file system is mounted on device                  */
/****************************************************************************/
typedef struct fd32_ismounted
{
  DWORD           Size;     /* Size in bytes of this structure         */
  void           *DeviceId; /* Identifier of the device                */
  fd32_request_t *fsreq;    /* OUT: Request function of the FS driver  */
  void           *FSDevId;  /* OUT: Device identifier of the FS device */
}
fd32_ismounted_t;


/***************************/
/* DEVICES ENGINE SERVICES */
/***************************/
int fd32_dev_get       (unsigned         Handle,
                        fd32_request_t **request,
                        void           **DeviceId,
                        char            *Name,
                        int              MaxName);
int fd32_dev_search    (const char      *Name);
int fd32_dev_first     (void);
int fd32_dev_last      (void);
int fd32_dev_next      (unsigned         Handle);
int fd32_dev_prev      (unsigned         Handle);
int fd32_dev_register  (fd32_request_t  *request,
                        void            *DeviceId,
                        const char      *Name);
int fd32_dev_unregister(unsigned         Handle);
int fd32_dev_replace   (unsigned         Handle,
                        fd32_request_t  *request,
                        void            *DeviceId);


#endif /* #ifndef __FD32_DEV_H */


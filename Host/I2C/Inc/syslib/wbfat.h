#ifndef _WBFAT_H_
#define _WBFAT_H_

#ifdef ECOS
#include "drv_api.h"
#endif

#define MAX_PATH_LEN            	520		/* maximum length of full path, counted by 
										   	   character, note that one character may be 
										       compsoed of two bytes */
#define MAX_FILE_NAME_LEN       	514


/*
 * Declared for pre-referenced data strucutres
 */
struct storage_driver_S;
struct physical_disk_S;
struct partition_S;
struct logical_disk_S;
struct fs_op_S;
struct file_op_S;
struct file_S;	
struct file_find_S;	
struct file_stat_S;

#define STORAGE_DRIVER_T		struct storage_driver_S
#define PDISK_T					struct physical_disk_S
#define PARTITION_T				struct partition_S
#define LDISK_T 				struct logical_disk_S
#define DISK_OP_T				struct disk_op_S
#define FILE_OP_T				struct file_op_S
#define FILE_T					struct file_S
#define FILE_FIND_T				struct file_find_S	
#define FILE_STAT_T				struct file_stat_S

/*
 *  Include other header files
 */
#include "wbfat_fat.h"


/*================================================= Error Code Definitions ==*/

/*--- ERROR CODEs ---*/
#define FS_OK              			0

/* GENERAL ERRORs */									   		
#define ERR_FILE_EOF                -1		/* end of file */
#define ERR_GENERAL_FILE_ERROR      -2   	/* general file error */
#define ERR_NO_FREE_MEMORY          -5		/* no free memory */
#define ERR_NO_FREE_BUFFER          -6		/* no available sector buffers */
#define ERR_NOT_SUPPORTED			-7		/* feature or function was not supported */
#define ERR_UNKNOWN_OP_CODE       	-8		/* unrecognized operation code */
#define ERR_INTERNAL_ERROR			-9		/* file system internal error */
#define ERR_SYSTEM_LOCK				-10		/* file system locked by ScanDisk or Defragment */

/* FILE ERRORs */									   		
#define ERR_FILE_NOT_FOUND          -20		/* file not found */
#define ERR_FILE_INVALID_NAME       -21		/* invalid file name */
#define ERR_FILE_INVLAID_HANDLE     -22		/* invalid file hFile */
#define ERR_FILE_IS_DIRECTORY       -24     /* try to open a directory file */
#define ERR_FILE_IS_NOT_DIRECTORY   -25		/* is not a directory file */
#define ERR_FILE_CREATE_NEW         -26     /* can not create new entry    */
#define ERR_FILE_OPEN_MAX_LIMIT		-27		/* has opened too many files */
#define ERR_FILE_EXIST				-28		/* file already exist */
#define ERR_FILE_INVALID_OP			-29		/* invalid file operation */
#define ERR_FILE_INVALID_ATTR		-30		/* invalid file attribute */
#define ERR_FILE_INVALID_TIME		-31		/* invalid time specified */
#define ERR_FILE_TRUNC_UNDER		-32		/* truncate file underflow, size < pos */  
#define ERR_FILE_NO_MORE			-33		/* No really an error, used to identify end of file enumeration of a directory */
#define ERR_FILE_IS_CORRUPT			-34		/* file is corrupt */

/* PATH ERRORs */											
#define ERR_PATH_INVALID            -61		/* path name was invalid */
#define ERR_PATH_TOO_LONG           -62		/* path too long */
#define ERR_PATH_NOT_FOUND          -63		/* path not found */

/* DRIVE ERRORs */											
#define ERR_DRIVE_NOT_FOUND         -81		/* drive not found, the disk may have been unmounted */
#define ERR_DRIVE_INVALID_NUMBER    -82		/* invalid drive number */
#define ERR_DRIVE_NO_FREE_SLOT      -83		/* can not mount more drive */

/* DIRECTORY ERRORS */											
#define ERR_DIR_BUILD_EXIST         -93     /* try to build an already exist directory */
#define ERR_DIR_REMOVE_MISS         -94     /* try to remove a nonexistent directory */
#define ERR_DIR_REMOVE_ROOT         -95     /* try to remoe root directory */
#define ERR_DIR_REMOVE_NOT_EMPTY    -96     /* try to remove a non-empty directory */
#define ERR_DIR_DIFFERENT_DRIVE     -98     /* specified files on different drive */
#define ERR_DIR_ROOT_FULL           -99     /* root directory full */
#define ERR_DIR_SET_SIZE			-101	/* try to set file size of a directory */

/* ACCESS ERRORs */										
#define ERR_READ_VIOLATE            -131    /* user has no read privilege */
#define ERR_WRITE_VIOLATE			-132	/* user has no write privilege */
#define ERR_ACCESS_VIOLATE			-133	/* can not access */
#define ERR_READ_ONLY            	-136    /* try to write a read-only file */
#define ERR_WRITE_CAP				-137	/* try to write file which was opened with read-only */

/* DISK ERRORs */										
#define ERR_NO_DISK_MOUNT			-150	/* there's no any disk mounted */
#define ERR_DISK_CHANGE_DIRTY       -152 	/* disk change, buffer is dirty */
#define ERR_DISK_REMOVED            -154    /* portable disk has been removed */
#define ERR_DISK_WRITE_PROTECT      -156	/* disk is write-protected */
#define ERR_DISK_FULL               -158	/* disk is full */
#define ERR_DISK_BAD_PARTITION      -162	/* bad partition */
#define ERR_DISK_UNKNOWN_PARTITION	-164	/* unknow or not supported partition type */
#define ERR_DISK_UNFORMAT			-165	/* partition was not formatted */
#define ERR_DISK_UNKNOWN_FORMAT     -166	/* unknown disk format */
#define ERR_DISK_BAD_BPB			-168	/* bad BPB, disk may not be formatted */
#define ERR_DISK_IO					-170	/* disk I/O failure */
#define ERR_DISK_IO_TIMEOUT			-171	/* disk I/O time-out */
#define ERR_DISK_FAT_BAD_CLUS		-172	/* bad cluster number in FAT table */
#define ERR_DISK_IO_BUSY			-173	/* I/O device is busy writing, must retry. direct-write mode only */
#define ERR_DISK_INVALID_PARM		-174	/* invalid parameter */
#define ERR_DISK_CANNOT_LOCK		-175	/* cannot lock disk, the disk was in-use or locked by other one */

/* FILE SEEK ERRORs */											
#define ERR_SEEK_SET_EXCEED         -210    /* file seek set exceed end-of-file */
#define ERR_ACCESS_SEEK_WRITE       -211    /* try to seek a file which was opened for written */

/* OTHER ERRORs */										
#define ERR_FILE_SYSTEM_NOT_INIT    -301	/* file system was not initialized */
#define ERR_ILLEGAL_ATTR_CHANGE     -302    /* illegal file attribute change */

/*============================================= disk and file system types ==*/
#define FILE_SYSTEM_FAT12       		12
#define FILE_SYSTEM_FAT16       		16
#define FILE_SYSTEM_FAT32       		32
#define FILE_SYSTEM_NTFS        		64
#define FILE_SYSTEM_ISO9660     		99

#define DISK_TYPE_HARD_DISK     		0x0001
#define DISK_TYPE_RAM_DISK      		0x0002
#define DISK_TYPE_CDR           		0x0004
#define DISK_TYPE_SMART_MEDIA           0x0008
#define DISK_TYPE_CF           			0x0010
#define DISK_TYPE_SD_MMC       			0x0020
#define DISK_TYPE_FM_CARD    			0x2000		/* A bit OR */
#define DISK_TYPE_USB_DEVICE    		0x4000		/* A bit OR */
#define DISK_TYPE_DMA_MODE              0x8000		/* A bit OR */

#define PARTITION_TYPE_UNKNOWN			0x00
#define PARTITION_TYPE_FAT12			0x01
#define PARTITION_TYPE_FAT16_OLD		0x04
#define PARTITION_TYPE_EXTENDED_DOS		0x05
#define PARTITION_TYPE_FAT16			0x06
#define PARTITION_TYPE_NTFS				0x07
#define PARTITION_TYPE_FAT32			0x0B
#define PARTITION_TYPE_FAT32_LBA		0x0C
#define PARTITION_TYPE_FAT16_LBA		0x0E
#define PARTITION_TYPE_DOS_LBA			0x0F


/*
 *  Storage driver structure - Each type of media should support its own storage
 *  driver. The storage driver control the physical I/O access to a storage device.
 */
typedef struct storage_driver_S 
{                       
	INT (*init)(PDISK_T *);
	INT (*read)(PDISK_T *, UINT32, INT, UINT8 *);
	INT (*write)(PDISK_T *, UINT32, INT, UINT8 *, BOOL);
	INT	(*ioctl)(PDISK_T *, INT, VOID *);
} 
#undef STORAGE_DRIVER_T
	STORAGE_DRIVER_T;

#define BLOCKING_WRITE		0
#define NON_BLOCKING_WRITE	1

/*
 *  Storage I/O control
 */
#define STORAGE_IOCTL_EJECT_DOOR   	1	/* eject door */
#define STORAGE_IOCTL_CLOSE_DOOR   	2   /* close door */
#define STORAGE_IOCTL_LOCK_DOOR   	3   /* lock door */
#define STORAGE_IOCTL_FORMAT   		11  /* format disk */
#define STORAGE_IOCTL_SPEED   		21  /* speed control */
#define STORAGE_IOCTL_POWER_MODE	31  /* power saving control */


typedef struct pt_rec		/* partition record */
{
	UINT8       ucState;			/* Current state of partition */
	UINT8       uStartHead;			/* Beginning of partition head */
	UINT8       ucStartSector;		/* Beginning of partition sector */
	UINT8       ucStartCylinder;	/* Beginning of partition cylinder */
	UINT8       ucPartitionType;	/* Partition type, refer to the subsequent definitions */
	UINT8       ucEndHead;			/* End of partition - head */
	UINT8       ucEndSector;		/* End of partition - sector */
	UINT8       ucEndCylinder;		/* End of partition - cylinder */
	UINT32      uFirstSec;			/* Number of Sectors Between the MBR and the First Sector in the Partition */
	UINT32		uNumOfSecs;			/* Number of Sectors in the Partition */
}	PT_REC_T;


typedef struct physical_disk_S
{
	INT			nDiskType;
	CHAR		szManufacture[32];	/* Human readable string descibing this disk */
	CHAR		szProduct[32];		/* Human readable string descibing this disk */
	CHAR		szSerialNo[64];
	INT			nCylinderNum;
	INT			nHeadNum;
	INT			nSectorNum;
	UINT32		uTotalSectorN;
	INT			nSectorSize;
	UINT32		uDiskSize;			/* disk size in Kbytes */
	INT			nPartitionN;		/* number of partitions */
	PARTITION_T	*ptPartList;		/* partition list */
	
	/* file system internal use */
	PDISK_T		*ptSelf;			
	STORAGE_DRIVER_T *ptDriver;  
	PDISK_T		*ptPDiskAllLink;	/* link for all physical disk */
	VOID		*pvPrivate; 		/* driver-specific client data */
}	
#undef PDISK_T
	PDISK_T;


typedef struct partition_S
{
	/* The following 16 bytes are a direct mapping of paration record */
	UINT8       ucState;			/* Current state of partition */
	UINT8       ucStartHead;		/* Beginning of partition head */
	UINT8       ucStartSector;		/* Beginning of partition sector */
	UINT8       ucStartCylinder;	/* Beginning of partition cylinder */
	UINT8       ucPartitionType;	/* Partition type, refer to the subsequent definitions */
	UINT8       ucEndHead;			/* End of partition - head */
	UINT8       ucEndSector;		/* End of partition - sector */
	UINT8       ucEndCylinder;		/* End of partition - cylinder */
	UINT32      uFirstSec;			/* Number of Sectors Between the MBR and the First Sector in the Partition */
	UINT32		uNumOfSecs;			/* Number of Sectors in the Partition */

	/* Please use the followings */
	INT			nSectorN;			/* sectors per track, -1 means unknown */
	INT			nHeadN;				/* number of heads, -1 means unknown */
	INT			nCylinderN;			/* number of cylinders, -1 means unknown */
	UINT32		uPartRecSecN;		/* logical sector number of the sector where partition record resides */
	UINT32		uStartSecN;			/* Beginning logical sector number of this partition */
	UINT32		uTotalSecN;			/* Total number of sectors in this partition */
	INT			nErrorCode;			/* error on this partition */
	LDISK_T		*ptLDisk;			/* the only on logical disk on this partition, if exist */
	PARTITION_T	*ptNextPart;		/* link to the next partition */
}	
#undef PARTITION_T
	PARTITION_T;



/* Here the disk drive is a logical disk drive */
typedef struct logical_disk_S
{
	PDISK_T		*ptPDisk;			/* the physical disk it was located */
	INT    		nDriveNo;			/* 'A', 'B', ... 'Z' */
	UINT8   	ucFileSysType;
	BOOL	   	bIsDiskRemoved;		/* set is this disk has been removed but not unmounted */
	CHAR    	szVolLabel[16];
	UINT32 		uDiskSize;			/* disk size counted by sectors */
	UINT32		uFreeSpace;			/* free space counted by sectors */
	FAT_INFO_T  tFatInfo;        
	DISK_OP_T  	*ptDiskOP;			/* disk operations */
	FILE_OP_T   *ptFileOP;			/* file operations */
	FILE_T   	*ptFileList;        /* files opened on this disk */
	LDISK_T  	*ptLinkForAllLDisk;	/* link used for all logical disk chain */
}	
#undef LDISK_T
 	LDISK_T;



/*
 *  File system operqations
 *  These service routines are provided by a specific file system. They are used
 *  to fulfill operations on a specific file system. They are system level 
 *  operations, instead of file level operations.
 */
typedef struct disk_op_S
{
	INT (*delfile)(LDISK_T *, CHAR *, CHAR *);	/* delete a file */
	INT (*mkdir)(LDISK_T *, CHAR *, CHAR *);	/* create a new directory */
	INT (*rmdir)(LDISK_T *, CHAR *, CHAR *);	/* remove a existent directory */
	INT (*rename)(LDISK_T *, CHAR *, CHAR *, CHAR *, CHAR *, BOOL);	/* rename file/directory */
	INT (*move)(LDISK_T *, CHAR *, CHAR *, CHAR *, CHAR *, BOOL);	/* move file/directory */
	INT (*volume_label)(LDISK_T *, CHAR *, CHAR *);	/* get/change disk volume label */
	//INT (*format)(LDISK_T *);				/* format this disk */
	//INT (*scan_disk)(LDISK_T *);			/* scan this disk */
	//INT (*defragment)(LDISK_T *);			/* defragment this disk */
}
#undef DISK_OP_T
	DISK_OP_T;


/*
 *  File operations
 *  These service routines are provided by a specific file system. They are used
 *  to fulfill operations on a specific file opened on a file system. They are
 *  file level operations, instead of system level operations.
 */
typedef struct file_op_S
{
	INT (*fopen)(FILE_T *, CHAR *, CHAR *);		/* open a file */
	INT (*fread)(FILE_T *, UINT8 *, INT, INT *);/* read data from file */
	INT (*fwrite)(FILE_T *, UINT8 *, INT, INT*);/* write data to file */
	INT (*fclose)(FILE_T *);              		/* close file */
	INT (*fsizing)(FILE_T *, UINT32);			/* change file size */
	INT (*fseek)(FILE_T *, INT);				/* setting the file position pointer */
	INT (*fget_stat)(FILE_T *, FILE_STAT_T *); 	/* get file status */
	INT (*fset_stat)(FILE_T *, FILE_STAT_T *); 	/* set file status */
	INT (*find_first)(FILE_T *, FILE_FIND_T *);
	INT (*find_next)(FILE_T *, FILE_FIND_T *);
	INT (*find_close)(FILE_T *, FILE_FIND_T *);
	INT (*fioctl)(FILE_T *, INT, VOID *); 		/* file system specific control */
} 
#undef FILE_OP_T
	FILE_OP_T;



typedef struct file_S 
{
#ifdef ECOS
	cyg_handle_t owner;
#endif
	//CHAR		suFileName[MAX_PATH_LEN+2];	/* full path file name */
	CHAR		szFnameAscii[MAX_FILE_NAME_LEN/2];
	UINT32   	uFlag;                   
	LDISK_T  	*ptLDisk;
	FAT_FCB_T  	tFatFcb;              	/* File Control Block used by FAT file system */
	FILE_OP_T   *ptFileOP;				/* file operations */
	FILE_T  	*ptFileAllLink;	       	/* used for global file link */
	FILE_T  	*ptFileDiskLink;      	/* used for file link on disk */
} 
#undef FILE_T
	FILE_T;                    


typedef struct file_find_S 
{
	INT			hFile;
	CHAR		suLongName[MAX_FILE_NAME_LEN];
	CHAR		szShortName[14];
	UINT8    	ucAttrib;
	UINT8    	ucCTimeMS;        	/* create time minisecond */
	UINT8    	ucCTimeSec;         /* create time */
	UINT8    	ucCTimeMin;
	UINT8    	ucCTimeHour;
	UINT8    	ucCDateDay;         /* create date */
	UINT8    	ucCDateMonth;
	UINT8    	ucCDateYear;
	UINT8    	ucLDateDay;         /* last access date */
	UINT8    	ucLDateMonth;
	UINT8    	ucLDateYear;
	UINT8    	ucWTimeSec;			/* write time */
	UINT8    	ucWTimeMin;
	UINT8    	ucWTimeHour;
	UINT8    	ucWDateDay;			/* write date */
	UINT8    	ucWDateMonth;
	UINT8    	ucWDateYear;
	INT			nFileSize;
} 
#undef FILE_FIND_T
	FILE_FIND_T;


typedef struct file_stat_S
{
#ifdef ECOS
	cyg_handle_t owner;
#endif
	UINT32   	uFlag;              /* forwarded from FAL */
	UINT8    	ucAttrib;			/* file attribute */
	UINT8    	ucCTimeMs;        	/* create time minisecond */
	UINT8    	ucCTimeSec;         /* create time */
	UINT8    	ucCTimeMin;
	UINT8    	ucCTimeHour;
	UINT8    	ucCDateDay;         /* create date */
	UINT8    	ucCDateMonth;
	UINT8    	ucCDateYear;
	UINT8    	ucLDateDay;          /* last access date */
	UINT8    	ucLDateMonth;
	UINT8    	ucLDateYear;
	UINT8    	ucWTimeSec;			/* write time */
	UINT8    	ucWTimeMin;
	UINT8    	ucWTimeHour;
	UINT8    	ucWDateDay;			/* write date */
	UINT8    	ucWDateMonth;
	UINT8    	ucWDateYear;
	UINT32		uDev;				/* ptLDisk value to fake Linux dev */
	UINT32		uUnique;				/* a unique value used to fake Linux inode number */
	INT			nFilePos;
	INT   		nFileSize;
}
#undef FILE_STAT_T
	FILE_STAT_T;


struct tm 
{
	INT     nTimeSec;   	/* seconds */
	INT     nTimeMin;       /* minutes */
	INT     nTimeHour;      /* hours */
	INT     nTimeDay;       /* day of the month */
	INT     nTimeMonth;     /* month */
	INT     nTimeYear;      /* year */
	INT     nDayOfWeek;     /* day of the week */
	INT     nDayInYear;     /* day in the year */
	BOOL    bIsDaySaving; 	/* daylight saving time */
};

#ifndef time_t
#define time_t	unsigned long
#endif


/*===================================================== file mode and flag ==*/
/* File open mode */
#define O_RDONLY 		0x0001    	/* Open for read only*/
#define O_WRONLY 		0x0002      /* Open for write only*/
#define O_RDWR   		0x0003      /* Read/write access allowed.*/
#define O_APPEND     	0x0004      /* Seek to eof on each write*/
#define O_CREATE     	0x0008      /* Create the file if it does not exist.*/
#define O_TRUNC      	0x0010      /* Truncate the file if it already exists*/
#define O_EXCL       	0x0020      /* Fail if creating and already exists */
#define O_DIR  			0x0080      /* Open a directory file */
#define O_EXCLUSIVE		0x0200		/* Open a file with exclusive lock */
#define O_NODIRCHK		0x0400		/* no dir/file check */
#define O_FSEEK			0x1000		/* Open with cluster chain cache to enhance file seeking performance */
#define O_DIRECT_READ	0x2000		/* Open with direct write capability */
#define O_DIRECT_WRITE	0x4000		/* Open with direct write capability */

/* operation */
#define OP_READ        	0x0010000   /* can read */
#define OP_WRITE       	0x0020000   /* can write */
#define OP_RDWR        	0x0030000   /* can read/write */
#define OP_DELETE     	0x0100000   /* file deletion */
#define OP_RENAME		0x0200000	/* file rename */
#define OP_DIRTY    	0x0400000   /* file has ever been changed */
#define OP_ROOT			0x0800000	/* is root directory */
#define OP_WAS_DEL		0x2000000	/* file has been deleted by other process */

/* file attributes */
#define FA_RDONLY       0x01        /* Read only attribute */
#define FA_HIDDEN       0x02        /* Hidden file */
#define FA_SYSTEM       0x04        /* System file */
#define FA_LABEL        0x08        /* Volume label */
#define FA_DIR          0x10        /* Directory    */
#define FA_ARCHIVE      0x20        /* Archive */

#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2


/* scan disk stage */
#define SDS_SCAN_FAT		1
#define SDS_BACKUP_FAT		2
#define SDS_SCAN_DIR		3
#define SDS_RECLAIM_CLUS	4
#define SDS_DUP_FAT			5

typedef INT (FS_DW_CB)(UINT8 *);
typedef INT (FS_SD_CB)(INT);


/*===================================================== Exported Functions ==*/
/* WBFILE File System APIs */
extern INT  fsInitFileSystem(VOID);
extern VOID fsInstallIoWriteCallBack(FS_DW_CB *cb_func);

/* Disk operations */
extern INT  fsMountLogicalDisk(LDISK_T *ptLDisk);
extern INT  fsUnmountLogicalDisk(LDISK_T *ptLDisk);
extern INT  fsDiskFreeSpace(INT nDriveNo, UINT32 *puBlockSize, UINT32 *puFreeSize, UINT32 *puDiskSize);
extern PDISK_T  *fsGetFullDiskInfomation(VOID);
extern VOID fsReleaseDiskInformation(PDISK_T *ptPDiskList);
extern INT  fsFormatFlashMemoryCard(PDISK_T *ptPDisk);
extern INT  fsScanDisk(INT nDriveNo, FS_SD_CB *cb_func);
extern VOID fsDiskWriteComplete(UINT8 *pucBuff);

/* File operations */
extern INT  fsOpenFile(CHAR *suFileName, CHAR *szAsciiName, UINT32 uFlag);
extern INT  fsCloseFile(INT hFile);
extern INT  fsReadFile(INT hFile, UINT8 *pucPtr, INT nBytes, INT *pnReadCnt);
extern INT  fsWriteFile(INT hFile, UINT8 *pucBuff, INT nBytes, INT *pnWriteCnt);
extern INT  fsFileSeek(INT hFile, INT nOffset, INT16 usWhence);
extern BOOL	fsIsEOF(INT hFile);
extern INT  fsSetFileSize(INT hFile, CHAR *suFileName, CHAR *szAsciiName, UINT32 nNewSize);
extern INT  fsGetFilePosition(INT hFile, UINT32 *puPos);
extern INT  fsGetFileSize(INT hFile);
extern INT  fsGetFileStatus(INT hFile, CHAR *suFileName, CHAR *szAsciiName, FILE_STAT_T *ptFileStat);
extern INT  fsSetFileAttribute(INT hFile, CHAR *suFileName, CHAR *szAsciiName, UINT8 ucAttrib, FILE_STAT_T *ptFileStat);
extern INT  fsSetFileTime(INT hFile, CHAR *suFileName, CHAR *szAsciiName, UINT8 ucYear, UINT8 ucMonth, UINT8 ucDay, UINT8 ucHour, UINT8 unMin, UINT8 ucSec);
extern INT  fsMergeFile(CHAR *suFileNameA, CHAR *szAsciiNameA, CHAR *suFileNameB, CHAR *szAsciiNameB);
extern INT  fsDeleteFile(CHAR *suFileName, CHAR *szAsciiName);
extern INT  fsRenameFile(CHAR *suOldName, CHAR *szOldAsciiName, CHAR *suNewName, CHAR *szNewAsciiName, BOOL bIsDirectory);
extern INT  fsMoveFile(CHAR *suOldName, CHAR *szOldAsciiName, CHAR *suNewName, CHAR *szNewAsciiName, INT bIsDirectory);
extern INT	fsFindFirst(CHAR *suDirName, CHAR *szAsciiName, FILE_FIND_T *ptFindObj);
extern INT  fsFindNext(FILE_FIND_T *ptFindObj);
extern INT	fsFindClose(FILE_FIND_T *ptFindObj);

/* Directory operations */
extern INT  fsMakeDirectory(CHAR *suDirName, CHAR *szAsciiName);
extern INT  fsRemoveDirectory(CHAR *suDirName, CHAR *szAsciiName);

/* language support */
extern CHAR fsToUpperCase(CHAR chr);
extern INT  fsUnicodeToAscii(VOID *pvUniStr, VOID *pvASCII, BOOL bIsNullTerm);
extern INT  fsAsciiToUnicode(VOID *pvASCII, VOID *pvUniStr, BOOL bIsNullTerm);
extern VOID fsAsciiToUpperCase(VOID *pvASCII);
extern INT  fsAsciiNonCaseCompare(VOID *pvASCII1, VOID *pvASCII2);
extern VOID fsUnicodeToUpperCase(VOID *pvUnicode);
extern INT  fsUnicodeStrLen(VOID *pvUnicode);
extern INT  fsUnicodeNonCaseCompare(VOID *pvUnicode1, VOID *pvUnicode2);
extern INT  fsUnicodeCopyStr(VOID *pvStr1, VOID *pvStr2);
extern INT  fsUnicodeStrCat(VOID *pvUniStr1, VOID *pvUniStr2);
extern VOID fsGetAsciiFileName(VOID *pvUniStr, VOID *pvAscii);

/* Driver supporting routines */
extern INT  fsPhysicalDiskConnected(PDISK_T *ptPDisk);
extern INT  fsPhysicalDiskDisconnected(PDISK_T *ptPDisk);


/* For debug and internal use, not exported funcions */
//extern CHAR *fsFindFirstSlash(CHAR *szFullName);
extern CHAR *fsFindLastSlash(CHAR *suFullName);
extern INT  fsTruncatePath(CHAR *szPath, CHAR *szToken);
extern VOID  fsTrimEndingSpace(CHAR *szStr);
extern FILE_T *fsHandleToDescriptor(INT hFile);
extern INT  fsDescriptorToHandle(FILE_T *ptFile);

extern INT  fsGetDiskInformation(LDISK_T *ptLDiskList, INT nMaxNumber);
extern VOID fsDumpBufferHex(UINT8 *pucBuff, INT nSize);
extern VOID fsDumpSectorHex(INT uSectorNo, UINT8 *pucBuff, INT nSize);
extern INT  fsDumpDiskSector(UINT32 uSectorNo, INT nSecNum);

extern LDISK_T *fsAllocateDisk(VOID);
extern UINT8 *fsAllocateSector(VOID);
extern INT  fsFreeSector(UINT8 *pucSecAddr);

extern CHAR  *fsDebugUniStr(CHAR *suStr);
//extern BOOL fsIsValidLongEntName(CHAR *szDirEntName);
//extern BOOL fsIsValidShortNameChar(CHAR cChar);

extern VOID  lname_to_sname(CHAR *szAsciiName, INT nTildeNum, CHAR *szShortName);

extern INT  fsIOWrite(PDISK_T *ptPDisk, UINT32 uStartSecNo, INT nCount, UINT8 *pucOutBuff, UINT8 *pucOrgBuff, INT bIsDupBuff, INT nSectorType);
extern INT  fsIORead(PDISK_T *ptPDisk, UINT32 uStartSecNo, INT nCount, UINT8 *in_buf, INT nSectorType);

#endif  /* _WBFAT_H_ */

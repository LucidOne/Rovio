#ifndef	_WBFAT_FAT_H_
#define	_WBFAT_FAT_H_


#define	NUM_FAT_BUFFER	   			16
#define SECTOR_BURST_READ_NUM 		4
#define SECTOR_BURST_WRITE_NUM 		4

/* Defined for long filenames */
#define	LFN_MAX_FILENAME			514		/* maximum length of LFN in bytes */

/* Macro to	convert	Cluster	number to logical sector number	*/
#define	ClustNo2SecNo(n) ((UINT32)(n - 2) * ptLDisk->tFatInfo.uBpbSecPerClus + ptLDisk->tFatInfo.uDataSecStart)

#define SECTOR_TYPE_FILE			0
#define SECTOR_TYPE_DIR				1
#define SECTOR_TYPE_FAT				2
#define SECTOR_TYPE_PART			3

typedef	struct sector_buf_S
{
	UINT8		*pucData;			/* cached sector contents */
	LDISK_T		*ptLDisk;			/* LDISK_T * associated	with this sector */
	UINT32		uSectorNo;			/* logical sector number */
	BOOL		bIsDirty;			/* dirty or not */
	INT			nReaderCnt;			/* number of task that is currently access this sector */
#ifdef ECOS
	cyg_handle_t owner;
#endif
} 	SEC_BUFF_T;

/* For FAT and DATA sector check out */
#define READ_ACCESS				0
#define WRITE_ACCESS			1


/*======================================================== FAT information ==*/
typedef struct fat_info_S
{
	/*- The followings are BPB fields commonly used by FAT16/FAT32 -*/
	
	UINT32      uBpbBytesPerSec;    /* BPB[11], size:2 */
	UINT32      uBpbSecPerClus;     /* BPB[13], size:1 */
	UINT16      usBpbRsvdSecCnt;    /* BPB[14], size:2 */
	UINT8       ucBpbNumFATs;       /* BPB[16], size:1 */
	UINT16      usBpbRootEntCnt;    /* BPB[17], size:2 */
	UINT32      uBpbTotalSectors;   /* FAT12/FAT16 - BPB[19], size:2 */
									/* FAT32 - BPB[32], size: 4 */
	UINT8       ucBpbMedia;         /* BPB[21], size:1 */
	UINT32      uBpbFatSize;        /* FAT12/FAT16 - BPB[22], size:2 */
									/* FAT32 - BPB[36], size:4 */
	UINT16      usBpbSecPerTrk;     /* BPB[24], size:2 */
	UINT16      usBpbNumHeads;      /* BPB[26], size:2 */
	UINT32      uBpbHiddSec;        /* BPB[28], size:4 */
	
	UINT32      uBsVolID;           /* FAT12/FAT16 - BPB[39], size:4 */
									/* FAT32 - BPB[67], size:4 */
	CHAR        sBsVolLab[12];      /* FAT12/FAT16 - BPB[43], size:11 */
									/* FAT32 - BPB[71], size:4 */

	UINT32   	uBpbRootClus;       /* BPB[44], size:4 */
	UINT16   	usBpbFsInfo;        /* BPB[48], size:2 */
	
	/*- The following is FAT32 structure start from offset 36 -*/
	
	/* UINT32   uBpbBytesPerSec; */ /* BPB[36], size:4 */
	/* UINT16   usBsDrvNum;  */     /* BPB[40], size:2 */
	/* UINT16   usBpbFsVer; */      /* BPB[42], size:2 */
	/* UINT16   usBpbBkBootSec; */  /* BPB[50], size:2 */
	/* bpb_Reserved */              /* BPB[52], size:12 */
	/* usBsDrvNum */                /* BPB[64], size:1 */
	/* bs_Reserved1 */              /* BPB[65], size:1 */
	/* bs_BootSig */                /* BPB[66], size:1 */

	/*- The followings are proprietary used by our driver -*/
	UINT32      uEndClusMark;    	/* FAT12 = 0xFF7, FAT16 = 0xFFF7, FAT32 = 0x0FFFFFF7 */
	UINT8		ucFatType;			/* TYPE_FAT12/TYPE_FAT16/TYPE_FAT32 */
	UINT32      uSecSzMask;      	/* uBpbBytesPerSec - 1 */
	UINT32      uBytesPerClus;   	/* uBpbBytesPerSec * uBpbSecPerClus */
	UINT32		uLastSecNo;			/* the last physical sector number on this partition */
	UINT32      uLastClusNo;    	/* last cluster number on this partition */
	UINT32      uFatSecStart;    	/* sector number of the first FAT sector */
	UINT32      uDataSecStart;   	/* sector number of first data sector */
	UINT32      uRootDirStartSec;	/* FAT12/16 first sector number of the root directory */
	UINT32		uFsInfoSecNo;		/* FAT32 - the sector number of pucFSInfo sector */
	UINT32		uFreeClusSearchIdx;	/* the next start point to search a free cluster */
	UINT32		uTotalFreeClus;		/* total number of free clusters */
	UINT8		pucFSInfo[512];		/* pucFSInfo sector */
}  	FAT_INFO_T;


#define FAT12_END_CLUS_MARK			0xFF8			
#define FAT16_END_CLUS_MARK			0xFFF8
#define FAT32_END_CLUS_MARK			0x0FFFFFF8

#define FAT12_BAD_CLUS_MARK			0xFF7
#define FAT16_BAD_CLUS_MARK			0xFFF7
#define FAT32_BAD_CLUS_MARK			0x0FFFFFF7


#define TYPE_FAT12					12
#define TYPE_FAT16					16
#define TYPE_FAT32					32


/*================================================ FAT directory structure ==*/
/* 
 * A FAT directory entry size is 32 bytes and sector size is 512 bytes.
 * So, it must be word-aligned.
 */
/* physical file name entry */
typedef struct dir_ent_S
{
	UINT8       pucDirName[8];		/* Offset: 0,  Size: 11 */
	UINT8		pucDirExtName[3];	
	UINT8       ucDirAttrib;		/* Offset: 11, Size: 1  */
	UINT8       ucDirNTRes;			/* Offset: 12, Size: 1  */
	UINT8       ucDirCTimeTenth;	/* Offset: 13, Size: 1  */
	UINT8       pucDirCTime[2];    	/* Offset: 14, Size: 2  */
	UINT8       pucDirCDate[2];     /* Offset: 16, Size: 2  */
	UINT8       pucDirLADate[2];  	/* Offset: 18, Size: 2  */
	UINT8       pucDirFirstClusHi[2];/* Offset: 20, Size: 2  */
	UINT8       pucDirWTime[2];     /* Offset: 22, Size: 2  */
	UINT8       pucDirWDate[2];		/* Offset: 24, Size: 2  */
	UINT8       pucDirFirstClusLo[2];/* Offset: 26, Size: 2  */
	UINT8       pucDirFileSize[4];	/* Offset: 28, Size: 4  */
} 	DIR_ENT_T;


/* physical long file name entry */
typedef struct lfn_ent_S
{
	UINT8       ucLDirOrd;			/* Offset: 0,  Size: 1 */
	UINT8       pucLDirName1[10];   /* Offset: 1,  Size: 10, Unicode Chars 1~5 */
	UINT8       ucLDirAttrib;       /* Offset: 11, Size: 1, always 0x0F */
	UINT8       ucLDirType;         /* Offset: 12, Size: 1 */
	UINT8       ucLDirChksum;       /* Offset: 13, Size: 1, Checksum of 8.3 name */
	UINT8       pucLDirName2[12];   /* Offset: 14, Size: 12, Unicode Chars 6-11 */
	UINT8       pucLDirFirstClusLo[2]; 	/* Offset: 26, Size: 2, always 0  */
	UINT8       pucLDirName3[4];    /* Offset: 28, Size: 4, Unicode Chars 12-13 */
} 	LFN_ENT_T;

/* FAT file attributes */
#define A_NORMAL       	0x00        /* Normal file, no attributes */
#define A_RDONLY       	0x01        /* Read only attribute */
#define A_HIDDEN       	0x02        /* Hidden file */
#define A_SYSTEM       	0x04        /* System file */
#define A_LABEL        	0x08        /* Volume label */
#define A_DIR          	0x10        /* Directory    */
#define A_ARCHIVE       0x20        /* Archive */
#define A_LFN           0x0F		/* (A_RDONLY | A_HIDDEN | A_SYSTEM | A_LABEL) */
#define A_LFN_MASK      0x3F		/* (A_LFN | A_DIR | A_ARCHIVE) */


/*================================================ Run-time data structure ==*/
/*
 *  Each WBFILE has its own FAT configuration on a FAT file system. Refer to the
 *  FILE_T defined in wbfile.h.
 */
typedef struct file_control_block_S
{
	/* For directory entry */	
	//CHAR		suLongName[LFN_MAX_FILENAME];
	CHAR		szShortName[14];
	UINT8       ucDirAttrib;		/* file attribute */
	UINT8	    ucDirCTimeTenth;    /* Millisecond stamp at file creation time */	UINT16      dirCtime;           /* Time file was created */
	UINT16      usDirCDate;         /* Date file was created */
	UINT16      usDirLDate;         /* Last access date */
	UINT16      usDirWTime;			/* Time of last write. Note that file creation is considered a write. */
	UINT16      usDirWDate;			/* Date of last write. Note that file creation is considered a write. */
	
	UINT32		uDirEntSecNo;		/* Number of the sector where this directory entry is located. */
	INT			nDirEntInSecOffset;	/* Directory entry offset of this file in <uDirEntSecNo> sector. */

	/* for run-time file operation */
	BOOL		bIsFat16Root;		/* is FAT12/FAT16 root directory */

	BOOL		bIsRWBuffDirty;		/* read/write buffer is dirty or not */
	UINT8       *pucOrgRWBuff;
	UINT8		*pucRWBuff;			/* file r/w buffer */
	UINT32		uRWBuffMapPos;		/* the file position of the r/w buffer mapped to */
	UINT32		uRWBuffSecNo;		/* the start sector number of r/w buffer */
	INT 		nRWBuffSecCnt;		/* the start sector number of r/w buffer */
	UINT32		uRWBuffSize;		/* the size of read/write buffer */

#ifdef USE_CLUSTER_CHAIN
	UINT32		*pucCCBuff;			/* cluster chain buffer */
	UINT32		uCCBuffSize;		/* cluster chain buffer size */
	UINT32		uCCEntCnt;			/* cluster chain entry count */
	UINT32		uCCDoneFPos;		/* cluster chain done file position */
#endif	

	UINT32  	uFirstClusNo;   	/* first cluster number of this file */
	UINT32		uLastClusNo;
	UINT32  	uCurClusNo;         /* the number of the cluster currently used */
	INT			nFileSize;			/* The total length of this file */
	INT  		nFilePos;            /* current position in file */
}  	FAT_FCB_T;



/*====================================== External reference in library only ==*/
extern DISK_OP_T _fs_tFatDiskOperation;
extern FILE_OP_T _fs_tFatFileOperation;

extern INT  fs_fat_parse_partition(PDISK_T *ptPDisk, PARTITION_T *ptPartition, UINT32 uBpbSecNo);
extern INT  fs_fat_format_partition(PDISK_T *ptPDisk, PARTITION_T *ptPartition, INT nType);

extern INT   fs_fat32_scan_disk(LDISK_T *ptLDisk);
extern INT   fs_fat32_defragment_disk(LDISK_T *ptLDisk);

/* Cache */
extern VOID fs_fat_init_sector_cache(VOID);
extern VOID fs_fat_debug_sector_cache(VOID);

extern INT  fs_fat_check_out_sec(LDISK_T *ptLDisk, UINT32 uSectorNo, INT nAccess, SEC_BUFF_T **ptSecBuff);
extern VOID  fs_fat_check_in_sec(SEC_BUFF_T *ptSecBuff, INT nAccess, BOOL bIsDirty);
extern INT  fs_fat_flush_sector_cache(LDISK_T *ptLDisk);
extern VOID fs_fat_clear_sector_cache(LDISK_T *ptLDisk);

/* FAT32 pucFSInfo sector */
//extern INT  fs_fat_update_fs_info(LDISK_T *ptLDisk);

/* FAT table */
extern INT  fs_read_fat_table(LDISK_T *ptLDisk, UINT32 uClusNo, UINT32 *puEntryValue);
extern INT  fs_write_fat_table(LDISK_T *ptLDisk, UINT32 uClusNo, UINT32 uEntryValue);
extern INT  fs_fat_reclaim_clusters(LDISK_T *ptLDisk, UINT32 uStartClusNo);
extern INT  fs_fat_allocate_cluster(FILE_T *ptFile);

/* directory entry */
extern INT  fs_fat_delete_file(LDISK_T *ptLDisk, CHAR *suFullName, CHAR *szAsciiName, BOOL bIsDirectory);
extern INT  fs_fat_create_file(FILE_T *ptFile, CHAR *suFullName, CHAR *szAsciiName);
extern INT  fs_fat_merge_file(LDISK_T *ptLDisk, CHAR *suFileNameA, CHAR *szAsciiNameA, CHAR *suFileNameB, CHAR *szAsciiNameB);
extern INT  fs_fat_get_next_dir_entry(FILE_T *ptFile, UINT8 *pucBuff, CHAR **szLongName, DIR_ENT_T *ptDirEnt);
extern VOID fs_fat_set_dire_mtime(FAT_FCB_T *ptFcb, DIR_ENT_T *ptDire, INT bIsAlso);
extern VOID fs_fat_set_sdir_name(CHAR *szShortName, DIR_ENT_T *ptDire);
extern VOID fs_fat_get_sdir_name(CHAR *szShortName, DIR_ENT_T *ptDire);
extern UINT8  *fs_fat_get_ldir_name(UINT8 *pucNameBuff, LFN_ENT_T *ptLDire);
extern VOID fs_fat_set_dire_info(FAT_FCB_T *ptFcb, DIR_ENT_T *ptDire);
extern VOID fs_fat_get_dire_info(FAT_FCB_T *ptFcb, DIR_ENT_T *ptDire);
extern INT  fs_fat_search_file(FILE_T *ptFile, CHAR *szFileName);

extern VOID fs_fat_dump_fcb(FAT_FCB_T *ptFcb);

#endif	/* _WBFAT_FAT_H_ */

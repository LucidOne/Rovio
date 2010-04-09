#ifndef _WBFAT_CONFIG_H_
#define _WBFAT_CONFIG_H_

#undef W99702

#define W99702


/*----------------------------------------------------------------*
 *  Customization                                                 *
 *----------------------------------------------------------------*/
#define FAT_ONLY					1

//#define USE_FAT2 
#define HAVE_SCAN_DISK
#ifdef ECOS
#define MAX_OPEN_FILE           	16		/* maximum number of files can be opened at the same time */
#else
#define MAX_OPEN_FILE           	12		/* maximum number of files can be opened at the same time */
#endif
#define USE_CLUSTER_CHAIN
#define NUM_SECTOR_BUFFER			8		/* !!at least 8!! */

//#define RW_BUFF_SIZE				(4*1024)
#define RW_BUFF_SIZE				(4)	//xhchen modified to reduce the memory usage in IPCam !!!!

/*----------------------------------------------------------------*
 *  I/O queue                                                     *
 *----------------------------------------------------------------*/
#define IO_CACHE_WRITE
		/* IO_CACHE_SIZE must larger than 8K and RW_BUFF_SIZE */
#define IO_CACHE_SIZE				(RW_BUFF_SIZE * 4)


/*----------------------------------------------------------------*
 *  debugging output                                              *
 *----------------------------------------------------------------*/
//#define _debug_msg				sysprintf
//#define _info_msg				sysprintf		
//#define _error_msg				sysprintf
#define _debug_msg(...)	
#define _info_msg(...)	
#define _error_msg(...)	
#define sysprintf diag_printf


/*----------------------------------------------------------------*
 *  Library components                                            *
 *----------------------------------------------------------------*/
#define FORMAT_CAP
#define WRITE_CAP
#define DISKINFO_CAP
#define FILE_SEARCH_EXT

#define SEARCH_PATTERN_MAX		8

/*----------------------------------------------------------------*
 *  W99702                                                        *
 *----------------------------------------------------------------*/
#ifdef W99702

#ifdef ECOS
#define sysGetTicks(TIMER0)	cyg_current_time()
#define WBFAT_LOCK()		//cyg_scheduler_lock()
#define WBFAT_UNLOCK()		//cyg_scheduler_unlock()
#endif

//#define flush_cache()		sysFlushCache(D_CACHE)
#define flush_cache()		
#define CACHE_BIT			0x10000000

#define GET16_L(bptr,n)   	(bptr[n] | (bptr[n+1] << 8))
#define GET32_L(bptr,n)   	(bptr[n] | (bptr[n+1] << 8) | (bptr[n+2] << 16) | (bptr[n+3] << 24))
#define PUT16_L(bptr,n,val)	bptr[n] = val & 0xFF;				\
							bptr[n+1] = (val >> 8) & 0xFF;
#define PUT32_L(bptr,n,val)	bptr[n] = val & 0xFF;				\
							bptr[n+1] = (val >> 8) & 0xFF;		\
							bptr[n+2] = (val >> 16) & 0xFF;		\
							bptr[n+3] = (val >> 24) & 0xFF;

#define GET16_B(bptr,n)   	((bptr[n]) << 8 | bptr[n+1])
#define GET32_B(bptr,n)   	((bptr[n] << 24) | (bptr[n+1] << 16) | (bptr[n+2] << 8) | bptr[n+3])
#define PUT16_B(bptr,n,val)	bptr[n+1] = val & 0xFF;				\
							bptr[n] = (val >> 8) & 0xFF;
#define PUT32_B(bptr,n,val)	bptr[n+3] = val & 0xFF;				\
							bptr[n+2] = (val >> 8) & 0xFF;		\
							bptr[n+1] = (val >> 16) & 0xFF;		\
							bptr[n] = (val >> 24) & 0xFF;
#define	min(x,y)			(((x) <	(y)) ? (x) : (y))
#define	MIN(x,y)			(((x) <	(y)) ? (x) : (y))
#define	max(x,y)			(((x) >	(y)) ? (x) : (y))
#define	MAX(x,y)			(((x) >	(y)) ? (x) : (y))
#endif

extern VOID *fmiGetpDisk(UINT32 uCard);

#endif	/* _WBFAT_CONFIG_H_ */
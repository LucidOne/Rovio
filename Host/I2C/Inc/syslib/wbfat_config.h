#ifndef _WBFAT_CONFIG_H_
#define _WBFAT_CONFIG_H_

#ifndef W99702
#define W99702
#endif

//#define ECOS

#define FAT_ONLY					1

#define MAX_OPEN_FILE           	12		/* maximum number of files can be opened at the same time */

#define USE_DIRECT_RW
#define USE_CLUSTER_CHAIN

#define USE_WQUEUE					1
#define RW_BUFF_SIZE				(4*1024)
#define IO_WQ_MAX					4

//#define IO_READ_CACHE				0
//#define IO_WRITE_CACHE			0

#define NUM_SECTOR_BUFFER			4		/* number of sectors for dynamic allocate */

//#define IO_WRITE_QUEUE_MAX_MEM	(64 * 1024)
//#define IO_READ_QUEUE_MAX_MEM		(64 * 1024)

//#define _debug_msg				sysPrintf
//#define _info_msg					sysPrintf		
//#define _error_msg				sysPrintf
#define _debug_msg(...)	
#define _info_msg(...)	
#define _error_msg(...)	


/*
 * For W99702
 */
#ifdef W99702

#define flush_cache()		sysFlushCache(D_CACHE)
//#define flush_cache()		

#define CACHE_BIT			0x10000000

#ifndef ECOS
//#define printf				sysPrintf
extern void	sysPrintf(char * str, ...);
#endif

#define fsAllocMemory(x,y,z) malloc(x)
#define fsFreeMemory(x,y,z)	 free(x)
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


#endif	/* _WBFAT_CONFIG_H_ */
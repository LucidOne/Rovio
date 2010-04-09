#ifndef __BMP_TYPES_H__
#define __BMP_TYPES_H__


#define BI_RGB			0L
#define BI_RLE8			1L
#define BI_RLE4			2L
#define BI_BITFIELDS	3L

#define BMP_OS2_HEADER_INFO_SIZE	12
#define BMP_WIN_HEADER_INFO_SIZE	40
#define BMP_PALETTE_PIXEL_SIZE		3
#define BMP1_MAX_PALETTE			2
#define BMP4_MAX_PALETTE			16
#define BMP8_MAX_PALETTE			256

/* The Minix ACK compiler cannot pack a structure so we do it the hardway */
typedef unsigned char	BMP_BYTE;
typedef unsigned char	BMP_WORD[2];
typedef unsigned char	BMP_LONG[4];
#define	BMP_CASTWORD	*(unsigned short *)&
#define	BMP_CASTLONG	*(unsigned long *)&
#define BMP_MKWORD(ac2) (((unsigned short)((unsigned char *)(ac2))[0]) | (((unsigned short)((unsigned char *)(ac2))[1]) << 8))
#define BMP_MKLONG(ac4) \
	(	  ((unsigned long)((unsigned char *)(ac4))[0]) \
		| (((unsigned long)((unsigned char *)(ac4))[1]) << 8) \
		| (((unsigned long)((unsigned char *)(ac4))[2]) << 16) \
		| (((unsigned long)((unsigned char *)(ac4))[3]) << 24) )
#define BMP_CHWORD(word, ac2) \
	do \
	{ \
		((unsigned char *) ac2)[1] = (unsigned char) ((unsigned short) (word) >> 8); \
		((unsigned char *) ac2)[0] = (unsigned char) ((unsigned short) (word)); \
	} while (0)
#define BMP_CHLONG(dword, ac4) \
	do \
	{ \
		((unsigned char *) ac4)[3] = (unsigned char) ((unsigned long) (dword) >> 24); \
		((unsigned char *) ac4)[2] = (unsigned char) ((unsigned long) (dword) >> 16); \
		((unsigned char *) ac4)[1] = (unsigned char) ((unsigned long) (dword) >> 8); \
		((unsigned char *) ac4)[0] = (unsigned char) ((unsigned long) (dword)); \
	} while (0)

typedef unsigned char   BMP_PK4DUMMY[2];

/* windows style*/
typedef struct {
	/* BITMAPFILEHEADER*/
	BMP_BYTE	bType[2];
	BMP_LONG	lSize;
	BMP_WORD	wReserved1;
	BMP_WORD	wReserved2;
	BMP_LONG	lOffBits;
	/* BITMAPINFOHEADER*/
	BMP_LONG	lInfoSize;
	BMP_LONG	lWidth;
	BMP_LONG	lHeight;
	BMP_WORD	wPlanes;
	BMP_WORD	wBitCount;
	BMP_LONG	lCompression;
	BMP_LONG	lSizeImage;
	BMP_LONG	lXpelsPerMeter;
	BMP_LONG	lYpelsPerMeter;
	BMP_LONG	lClrUsed;
	BMP_LONG	lClrImportant;
	BMP_PK4DUMMY pkdummy;
} BMP_HEAD_WIN_T;

/* os/2 style*/
typedef struct {
	/* BITMAPFILEHEADER*/
	BMP_BYTE	bType[2];
	BMP_LONG	lSize;
	BMP_WORD	wReserved1;
	BMP_WORD	wReserved2;
	BMP_LONG	lOffBits;
	/* BITMAPCOREHEADER*/
	BMP_LONG	lInfoSize;
	BMP_WORD	wWidth;
	BMP_WORD	wHeight;
	BMP_WORD	wPlanes;
	BMP_WORD	wBitCount;
	BMP_PK4DUMMY pkdummy;
} BMP_HEAD_OS2_T;


typedef enum
{
	BMP_TYPE_WIN,
	BMP_TYPE_OS2,
	BMP_TYPE_UNKNOWN
} BMP_TYPE_E;


typedef struct
{
	unsigned long uHeaderSize;
	unsigned long uOffset;
	unsigned long uWidth;
	unsigned long uHeight;
	unsigned long uCompression;
	unsigned short usBitCount;
	unsigned short usPalSize;
	int nIsOS2;
	BMP_TYPE_E eBmpType;
} BMP_INFO_T;


typedef enum
{
	BMP_ST_OS2_HEADER,
	BMP_ST_WIN_HEADER,
	BMP_ST_PALLETE,
	BMP_ST_OFFSET_BEFORE_BODY,
	BMP_ST_BODY_PAL_BMP,
	BMP_ST_BODY_RLE_BMP,
	BMP_ST_BODY_TRUE_BMP,
	BMP_ST_EXPECT_OVER,
	BMP_ST_OVER,
	BMP_ST_ERROR
} BMP_STATUS_E;

#endif

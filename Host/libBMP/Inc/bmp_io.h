#ifndef __BMP_IO_H__
#define __BMP_IO_H__

#include "./ininc/bmp_types.h"
#include "softpipe.h"

#define	BMP_IS_OS2			0x0100
#define BMP_IS_COMPRESSION	0x0200
#define BMP_BITDEPTH_MASK	0x00FF
typedef enum
{
	BMP_FMT_OS2_1			= BMP_IS_OS2 | (BMP_BITDEPTH_MASK & 1),
	BMP_FMT_OS2_4			= BMP_IS_OS2 | (BMP_BITDEPTH_MASK & 4),
	BMP_FMT_OS2_8			= BMP_IS_OS2 | (BMP_BITDEPTH_MASK & 8),
	BMP_FMT_OS2_16			= BMP_IS_OS2 | (BMP_BITDEPTH_MASK & 16),
	BMP_FMT_OS2_24			= BMP_IS_OS2 | (BMP_BITDEPTH_MASK & 24),
	BMP_FMT_OS2_32			= BMP_IS_OS2 | (BMP_BITDEPTH_MASK & 32),
	BMP_FMT_WIN_1			= (BMP_BITDEPTH_MASK & 1),
	BMP_FMT_WIN_4			= (BMP_BITDEPTH_MASK & 4),
	BMP_FMT_WIN_RLE4		= BMP_IS_COMPRESSION | (BMP_BITDEPTH_MASK & 4),
	BMP_FMT_WIN_8			= (BMP_BITDEPTH_MASK & 8),
	BMP_FMT_WIN_RLE8		= BMP_IS_COMPRESSION | (BMP_BITDEPTH_MASK & 8),
	BMP_FMT_WIN_16			= (BMP_BITDEPTH_MASK & 16),
	BMP_FMT_WIN_BITFIELD16	= BMP_IS_COMPRESSION | (BMP_BITDEPTH_MASK & 16),
	BMP_FMT_WIN_24			= (BMP_BITDEPTH_MASK & 24),
	BMP_FMT_WIN_32			= (BMP_BITDEPTH_MASK & 32),
	BMP_FMT_WIN_BITFIELD32	= BMP_IS_COMPRESSION | (BMP_BITDEPTH_MASK & 32)
} BMP_FMT_E;


typedef struct tagBMP2RAW_SOURCE_PIPE_T
{
	SOFT_PIPE_T		pipe;
	BMP_INFO_T		bmpInfo;
	BMP_STATUS_E	eStatus;
	unsigned int	uLenRecv;

	int (*pfnOnHeader) (struct tagBMP2RAW_SOURCE_PIPE_T *pBMP);
	void (*pfnOnError) (struct tagBMP2RAW_SOURCE_PIPE_T *pBMP);

	/* Buffer for saving pallette. Before pallette is saved, the
	   buffer is used to save the bmp header temporarily. */
	union
	{
		struct
		{
			unsigned int	uBufferLen;
			unsigned char	aucBuffer[BMP_PALETTE_PIXEL_SIZE * BMP8_MAX_PALETTE];
		} sPallette;
		struct
		{
			/* For bmp with color depth >= 16 bit. */
			unsigned long	uBitFields[3];
			unsigned char	ucBitFields_Count[3];
		} sBitField;
	} uPallette;

	/* Parameters for reading bmp data. */
	struct
	{
		union
		{
			struct
			{
				int				nRLEAction;
				unsigned int	uCmd_Count;
				unsigned char	ucFillPixel;
				unsigned char	aucCmd[2];
			} sRleBMP;
			struct
			{
				unsigned long	uPixel_PerByte;
				unsigned long	uPixel_Stub;
				unsigned char	ucBitMask;
			} sPalBMP;
			struct
			{
				/* Buffer for reading bmp data. */
				unsigned char	aucPixel[4];
				unsigned int	uPixel_Count;
				unsigned long	uBytes_PerPixel;
			} sTrueBMP;
		} uParam;

		unsigned long x;
		unsigned long y;
		unsigned long uPixel_X;
		unsigned long uLine_Total;
		unsigned long uLine_Full;
		unsigned long uLine_Stub;

		/* Buffer for save the bmp data. */
#define RGB_BUFFER_SIZE	64	/* RGB_BUFFER_SIZE can be any value greater than 1. */
		unsigned char	aucRGB[3 * RGB_BUFFER_SIZE];
		unsigned int	uRGB_Count;

	} sParam;
} BMP2RAW_SOURCE_PIPE_T;


/* For converting BMP to raw image data. */
typedef struct tagBMP2RAW_SRC_T BMP2RAW_SRC_T;
struct tagBMP2RAW_SRC_T
{
	int (*fnRead) (BMP2RAW_SRC_T *pSrc, void *pBuf, int nLen);
};

typedef struct tagBMP2RAW_DES_T BMP2RAW_DES_T;
struct tagBMP2RAW_DES_T
{
	int (*fnWrite) (BMP2RAW_DES_T *pDes, unsigned char ucR, unsigned char ucG, unsigned char ucB);
	unsigned int uWidth;
	unsigned int uHeight;
};

/* For convert raw image data to BMP. */
typedef struct tagRAW2BMP_SRC_T RAW2BMP_SRC_T;
struct tagRAW2BMP_SRC_T
{
	int (*fnRead) (RAW2BMP_SRC_T *pSrc, unsigned char *pucR, unsigned char *pucG, unsigned char *pucB);
	unsigned int uWidth;
	unsigned int uHeight;
};

typedef struct tagRAW2BMP_DES_T RAW2BMP_DES_T;
struct tagRAW2BMP_DES_T
{
	int (*fnWrite) (RAW2BMP_DES_T *pDes, const void *pBuf, int nLen);
};



/* Read BMP to raw image data. */
int bmpRead (BMP2RAW_SRC_T *pSrc, BMP2RAW_DES_T *pDes);
/* Write raw image data to BMP. */
int bmpWrite (RAW2BMP_SRC_T *pSrc, RAW2BMP_DES_T *pDes, BMP_FMT_E eDesFormat,
			  unsigned int uBitField_R,
			  unsigned int uBitField_G,
			  unsigned int uBitField_B);


void bmpSourcePipe_Init (BMP2RAW_SOURCE_PIPE_T *pBMP,
						 int (*pfnOnHeader) (BMP2RAW_SOURCE_PIPE_T *pBMP),
						 void (*pfnOnError) (BMP2RAW_SOURCE_PIPE_T *pBMP)
						 );
int bmpSourcePipe_GetSize (BMP2RAW_SOURCE_PIPE_T *pBMP,
						   unsigned int *puWidth,
						   unsigned int *puHeight);
#endif

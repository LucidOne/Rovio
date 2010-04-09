#ifndef __FONT_H__
#define __FONT_H__

#include "wb_syslib_addon.h"

#define SUPPORT_FONT_ENG_11X11
//#define SUPPORT_FONT_GBK_11X11
//#define SUPPORT_FONT_BIG5_11x11
//#define SUPPORT_FONT_BIG5_SHARE_GBK_11X11
//#define SUPPORT_FONT_UNICODE_SHARE_GBK_11X11





/* The dependence. */
#ifdef SUPPORT_FONT_UNICODE_SHARE_GBK_11X11
#	define SUPPORT_FONT_GBK_11X11
#endif
#ifdef SUPPORT_FONT_BIG5_SHARE_GBK_11X11
#	define SUPPORT_FONT_GBK_11X11
#endif
#ifdef SUPPORT_FONT_GBK_11X11
#	define SUPPORT_FONT_ENG_11X11
#endif
#ifdef SUPPORT_FONT_BIG5_11x11
#	define SUPPORT_FONT_ENG_11X11
#endif


#include "stdio.h"
#include "assert.h"
#include "./font_types.h"
#include "./code_convert.h"




typedef struct
{
	EF_USHORT usIndex;
	EF_UCHAR ucValue1;
	EF_UCHAR ucValue2;
} FONT_INDEX_T;


typedef struct tagFONT_T FONT_T;
struct tagFONT_T
{
	/* Get the bitmap address of single byte character. */
	const EF_UCHAR *(*GetMapAddr1)(const FONT_T *pFont, EF_UCHAR ucChar);
	/* Get the bitmap address of double bytes character. */
	const EF_UCHAR *(*GetMapAddr2)(const FONT_T *pFont, EF_UCHAR ucChar1, EF_UCHAR ucChar2, EF_UCHAR aucConvert[4], EF_INT *pnLenConvert, EF_INT *pnLenUsed);

	/* Font map definitions. */
	const FONT_INDEX_T *pFontIndex1;//Index of font map for single byte character.
	EF_INT nIndexSize1;				//Size of buffer "pFontIndex1".
	const EF_UCHAR *pucFontMap1;		//Font map for single byte character.
	EF_INT nMapSize1;					//Size of buffer "pucFontMap1".
	EF_INT nMapWidthBit1;				//Bit length of lines.
	EF_INT nMapHeightBit1;				//Bit length of cols.

	const FONT_INDEX_T *pFontIndex2;//Index of font map for double bytes character.
	EF_INT nIndexSize2;				//Size of buffer "pFontIndex2".
	const EF_UCHAR *pucFontMap2;		//Font map for double bytes character.
	EF_INT nMapSize2;					//Size of buffer "pucFontMap2".
	EF_INT nMapWidthBit2;				//Bit length of lines.
	EF_INT nMapHeightBit2;				//Bit length of cols.
	EF_INT nBitWidth2;
	EF_INT nBitHeight2;
};


typedef struct
{
	EF_INT nLeft;
	EF_INT nTop;
	EF_INT nWidth;
	EF_INT nHeight;
} FONT_RECT_T;

typedef struct
{
	EF_INT nWidth;
	EF_INT nHeight;
} FONT_SIZE_T;


int fontIndexCompare(const void *pKey1, const void *pKey2);
const void *bsearch_floor(const void *key, const void *base, unsigned int nmemb,
              unsigned int size, int (*compar)(const void *, const void *));


/* Virtual function used to draw a pixel on some device. */
typedef void (*FONT_DRAW_PIXEL_FUN)(void *pDevice,
									EF_BOOL bIsDraw,
									EF_INT nPosX,
									EF_INT nPosY);

typedef struct
{
	const FONT_T *pFontType;		//Font definiton.
	void *pDrawDevice;				//Target device to be draw.
	FONT_DRAW_PIXEL_FUN fnDrawPixel;//Draw function.
} FONT_DEVICE_T;


/* Set font attribute, including font type, draw method, draw device. */
void fontSetFont(FONT_DEVICE_T *pFontDevice,
				 const FONT_T *pFontType,
				 FONT_DRAW_PIXEL_FUN fnDrawPixel,
				 void *pDrawDevice);
/* Get the size of a string with length "nLen". */
void fontGetSizeEx(FONT_DEVICE_T *pFontDevice,
				   const EF_UCHAR *pucStr,
				   EF_INT nLen,
				   FONT_SIZE_T *pSize);
/* Get the size of a string. */
void fontGetSize(FONT_DEVICE_T *pFontDevice,
				 const EF_UCHAR *pucStr,
				 FONT_SIZE_T *pSize);
/* Print a string with length "nLen". */
void fontPrintEx(FONT_DEVICE_T *pFontDevice,
				 const EF_UCHAR *pucStr,
				 EF_INT nLen,
				 const FONT_RECT_T *pRange,
				 FONT_SIZE_T *pSizeUsed);
/* Print a string. */
void fontPrint(FONT_DEVICE_T *pFontDevice,
			   const EF_UCHAR *pucStr,
			   const FONT_RECT_T *pRange,
			   FONT_SIZE_T *pSizeUsed);


EF_USHORT BIG5_to_GB(EF_USHORT usCode);




#ifdef SUPPORT_FONT_ENG_11X11
#	include "./data_eng_11x11.h"
#endif
#ifdef SUPPORT_FONT_GBK_11X11
#	include "./data_gbk_11x11.h"
#endif
#ifdef SUPPORT_FONT_BIG5_11x11
#	include "./data_big5_11x11.h"
#endif
#ifdef SUPPORT_FONT_BIG5_SHARE_GBK_11X11
#	include "./data_big5togbk.h"
#endif
#ifdef SUPPORT_FONT_UNICODE_SHARE_GBK_11X11
#	include "./data_unicodetogbk.h"
#endif


#endif


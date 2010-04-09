#include "../../../Libs/Platform/Inc/Platform.h"
#include "../../../Libs/WString/Inc/WString.h"
#include "../../../Libs/SoftPipe/include/softpipe.h"
#include "../../../Libs/HIC/Inc/HIC.h"
#include "../../../Libs/LCM/Inc/LCM.h"
#include "../../../Libs/CmdSet/Inc/CmdSet.h"
#include "../../../Libs/BMP/include/bmp_io.h"


#include "../Inc/OSD.h"

typedef struct
{
	BMP2RAW_SRC_T src;
	CONST UCHAR *cpucBMP;
	UINT32 uBMP_Len;
	UINT32 uBMP_Len_OK;
} MY_BMP2RAW_SRC_T;

typedef struct
{
	BMP2RAW_DES_T des;
	UCHAR *pucRGB565;
	UINT32 uRGB565_Len;
	UINT32 uX;
	UINT32 uY;
} MY_BMP2RAW_DES_T;


static int MyRead (BMP2RAW_SRC_T *pSrc, void *pBuf, int nLen)
{
	MY_BMP2RAW_SRC_T *pMySrc = (MY_BMP2RAW_SRC_T *) pSrc;
	UINT32 nLen_Copy = pMySrc->uBMP_Len - pMySrc->uBMP_Len_OK;
	if (nLen_Copy > nLen) nLen_Copy = nLen;
	memcpy (pBuf, pMySrc->cpucBMP + pMySrc->uBMP_Len_OK, nLen);
	pMySrc->uBMP_Len_OK += nLen;

	return nLen;
}

static int MyWrite (BMP2RAW_DES_T *pDes, unsigned char ucR, unsigned char ucG, unsigned char ucB)
{
	MY_BMP2RAW_DES_T *pMyDes = (MY_BMP2RAW_DES_T *) pDes;
	if (pMyDes->pucRGB565 == NULL)
	{
		pMyDes->uRGB565_Len = (pDes->uWidth * pDes->uHeight * 2);
		pMyDes->pucRGB565	= (UCHAR *) sysMalloc (pMyDes->uRGB565_Len);
		if (pMyDes->pucRGB565 == NULL)
			return -1;
	}

	if (pMyDes->uX < pDes->uWidth && pMyDes->uY < pDes->uHeight)
	{
		USHORT usRGB565 = (((USHORT) ucR & 0xF8) << 8)
			| (((USHORT) ucG & 0xFC) << 3)
			| (((USHORT) ucB & 0xF8) >> 3);
		UINT32 uPos = (pDes->uHeight - 1 - pMyDes->uY) * pDes->uWidth + pMyDes->uX;
		pMyDes->pucRGB565[uPos * 2 + 0] = (UCHAR) (usRGB565);
		pMyDes->pucRGB565[uPos * 2 + 1] = (UCHAR) (usRGB565 >> 8);

		pMyDes->uX++;
		if (pMyDes->uX >= pDes->uWidth)
		{
			pMyDes->uX = 0;
			pMyDes->uY++;
		}

		return 0;
	}
	else
	{
		return -1;
	}
}


BOOL osdLoadBMP2RGB565 (CONST UCHAR *cpucBMP, UINT32 uBMP_Len,
						UCHAR **ppucRGB565, UINT32 *puRGB565_Len,
						UINT32 *puWidth, UINT32 *puHeight)
{
	MY_BMP2RAW_SRC_T bmpSrc;
	MY_BMP2RAW_DES_T bmpDes;

	if (cpucBMP == NULL || ppucRGB565 == NULL)
		return FALSE;

	bmpSrc.src.fnRead		= MyRead;
	bmpSrc.cpucBMP			= cpucBMP;
	bmpSrc.uBMP_Len			= uBMP_Len;
	bmpSrc.uBMP_Len_OK		= 0;

	bmpDes.des.fnWrite		= MyWrite;
	bmpDes.des.uWidth		= 0;
	bmpDes.des.uHeight		= 0;
	bmpDes.pucRGB565		= NULL;
	bmpDes.uRGB565_Len		= 0;
	bmpDes.uX				= 0;
	bmpDes.uY				= 0;

	if (bmpRead (&bmpSrc.src, &bmpDes.des) != 0)
	{
		if (bmpDes.pucRGB565 != NULL)
		{
			sysFree (bmpDes.pucRGB565);
			bmpDes.pucRGB565 = NULL;
		}
		return FALSE;
	}
	else
	{
		*ppucRGB565 = bmpDes.pucRGB565;
		if (puRGB565_Len != NULL)
			*puRGB565_Len = bmpDes.uRGB565_Len;
		if (puWidth != NULL)
			*puWidth = bmpDes.des.uWidth;
		if (puHeight != NULL)
			*puHeight = bmpDes.des.uHeight;
		return TRUE;
	}
}


void osdDisplay (CONST UCHAR *cpucBMP, UINT32 uLength)
{
	UCHAR *pucRGB565;
	UINT32 uRGB565_Len;
	UINT32 uWidth;
	UINT32 uHeight;

	/* Load BMP file to RGB565 raw data. */
	if (! osdLoadBMP2RGB565 (cpucBMP, uLength,
						&pucRGB565, &uRGB565_Len, &uWidth, &uHeight))
	{
		sysPrintf ("No BMP loaded.\n");
		return;
	}

	sysPrintf ("BMP width: %d, length: %d, RGB565 size: %d\n", uWidth, uHeight, uRGB565_Len);

#if 1
	wbvWriteOSD (0, 0, uWidth, uHeight,
				 pucRGB565,
				 uRGB565_Len);
#else
	wbcSendOSDPattern_Method1 (pucRGB565, uRGB565_Len, 0);
	wbcSendOSDPattern_Flush ();
#endif
	sysFree (pucRGB565);
}




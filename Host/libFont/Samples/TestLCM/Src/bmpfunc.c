#include "fontbypass.h"

char g_BackGroundBMP[] = 
{
#include "./bg.h"
};

/* Fill source data to "pBuf". */
int MyRead (BMP2RAW_SRC_T *pSrc, void *pBuf, int nLen)
{
	MyBMP2RAW_SRC_T *pMySrc = (MyBMP2RAW_SRC_T *) pSrc;

	if (nLen < pMySrc->nSrcLen - pMySrc->nSrcLenOK)
	{
		memcpy (pBuf, (void *) (pMySrc->pcSrcBuffer + pMySrc->nSrcLenOK), nLen);
		pMySrc->nSrcLenOK += nLen;
		return nLen;
	}
	else
		return -1;
}

/* Write RGB data to "pDes". */
int MyWrite (BMP2RAW_DES_T *pDes, unsigned char ucR, unsigned char ucG, unsigned char ucB)
{
	MyBMP2RAW_DES_T *pMyDes = (MyBMP2RAW_DES_T *) pDes;

	if (pMyDes->pcDesBuffer == NULL && pDes->uWidth > 0 && pDes->uHeight > 0)
	{
		pMyDes->nDesLenOK = 0;
		pMyDes->nDesLen = pDes->uWidth * pDes->uHeight * 3;
		pMyDes->pcDesBuffer = (char *) malloc (pMyDes->nDesLen);
	}

	if (pMyDes->pcDesBuffer == NULL)
		return -1;

	if (pMyDes->nDesLenOK + 3 <= pMyDes->nDesLen)
	{
		pMyDes->pcDesBuffer[pMyDes->nDesLenOK++] = ucR;
		pMyDes->pcDesBuffer[pMyDes->nDesLenOK++] = ucG;
		pMyDes->pcDesBuffer[pMyDes->nDesLenOK++] = ucB;
		return 0;
	}
	else
		return -1;
}




int bmpconv(char *pf, unsigned char *pBuf, int iWidth, int iHeight)
{
	MyBMP2RAW_SRC_T src;
	MyBMP2RAW_DES_T des;

	/* Prepare bmp source handler. */
	{
		/*
		struct stat st;
		FILE *fp;
		if (stat (pf, &st) != 0)
		{
			printf ("Can not get file size.\n");
			return FALSE;
		}
		if ((fp = fopen (pf, "rb")) == NULL)
		{
			printf ("Can not open file: %s\n", pf);
			return FALSE;
		}
		*/

		src.src.fnRead	= MyRead;
//		src.pcSrcBuffer	= (char *) malloc (st.st_size);
		src.pcSrcBuffer	= g_BackGroundBMP;
//		src.nSrcLen		= st.st_size;
		src.nSrcLen		= sizeof(g_BackGroundBMP);
		src.nSrcLenOK	= 0;
//        fread (src.pcSrcBuffer, 1, src.nSrcLen, fp);
		//printf ("%d %d\n", src.nSrcLen, fread (src.pcSrcBuffer, 1, src.nSrcLen, fp));
//		fclose (fp);
	}

	/* Prepare bmp target handler. */
	{
		des.des.fnWrite	= MyWrite;
		des.des.uWidth	= 0;
		des.des.uHeight	= 0;
		des.pcDesBuffer	= NULL;
		des.nDesLen		= 0;
		des.nDesLenOK	= 0;
	}

	/* Convert bmp to raw data.
		The raw data:
			width (4 bytes), height (4 bytes), RGB, RGB, RGB, RBG, RGB, ...
		Each RGB use 3 bytes.
	*/
	bmpRead ((BMP2RAW_SRC_T *) &src, (BMP2RAW_DES_T *) &des);
	
	/*
	if (src.pcSrcBuffer != NULL)
		free (src.pcSrcBuffer);
	*/

	if (des.pcDesBuffer != NULL)
	{
#ifdef FONT_DEBUG
		printf ("width=%d\n", des.des.uWidth);
		printf ("height=%d\n", des.des.uHeight);
#endif
        {
            int i;
            short r, g, b, mix;
            int x = 0, y = iHeight-1;
            
            for(i=0; (i<des.nDesLen) && (y>=0); i+=3)
            {
                r = des.pcDesBuffer[i];
                g = des.pcDesBuffer[i+1];
                b = des.pcDesBuffer[i+2];
                mix = (((r>>3)&0x1f)<<11) | (((g>>2)&0x3f)<<5) | ((b>>3)&0x1f);
                
                *(short*)(buf_video+(y*iWidth+x)*2) = mix;
                x ++;
                if(x >= iWidth)
                {
                    x= 0;
                    y--;
                }
            }
        }
		free (des.pcDesBuffer);
	}
    else
    {
        printf("Convert failed\n");
        return FALSE;
    }

	return TRUE;
}



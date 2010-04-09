#include "cmp_common.h"
#include <sys/stat.h>
#include "JpegCmp.h"


struct rectangleinfo{
	int 	x1;
	int 	y1;
	int 	x2;
	int 	y2;

};
char *extract (unsigned char *inbuf, int inbuflen, struct rectangleinfo * rectangleinfo, int *outlength);
int djpeg_ex (char *inbuf, int inbuflen, unsigned char **ppout);
int cmp(unsigned char *bufp1, unsigned char *bufp2, int blksize, int sensibility);

BOOL JpegExt2Bmp(char *pcInBuf, int iInBufLen, char **ppcOutBuf, int *piOutBufLen, RECT_T *pRect, unsigned short usResX, unsigned short usResY)
{
	char *pcSmallJpegBuf;
	int iSmallJpegBufLen;
	struct rectangleinfo rect;
	int iRt_djpeg_ex;

	if (pcInBuf == NULL || ppcOutBuf == NULL || iInBufLen <= 0) return FALSE;

	if (pRect->iLeft >= 10000 || pRect->iLeft < 0
		|| pRect->iRight >= 10000 || pRect->iRight < 0
		|| pRect->iLeft >= pRect->iRight
		|| pRect->iTop >= 10000 || pRect->iTop < 0
		|| pRect->iBottom >= 10000 || pRect->iBottom < 0
		|| pRect->iTop >= pRect->iBottom)
	{
		pcSmallJpegBuf = NULL;
	}
	else
	{
		rect.x1 = pRect->iLeft * usResX / 10000;
		rect.x2 = pRect->iRight * usResX / 10000;
		rect.y1 = pRect->iTop * usResY / 10000;
		rect.y2 = pRect->iBottom * usResY / 10000;
		pcSmallJpegBuf = extract(pcInBuf, iInBufLen, &rect, &iSmallJpegBufLen);
		if (pcSmallJpegBuf == NULL)
			PTE;
	}

	if (pcSmallJpegBuf == NULL)
		iRt_djpeg_ex = djpeg_ex(pcInBuf, iInBufLen, (unsigned char **)ppcOutBuf);
	else
	{
		iRt_djpeg_ex = djpeg_ex(pcSmallJpegBuf, iSmallJpegBufLen, (unsigned char **)ppcOutBuf);
		free(pcSmallJpegBuf);
	}

	if (iRt_djpeg_ex == 0)
	{
		if (piOutBufLen != NULL)
			*piOutBufLen = (int)((*ppcOutBuf)[0]) * (int)((*ppcOutBuf)[1]) + 2;
		return TRUE;
	}
	else
		return FALSE;
}

/* Return:
	-2 分辨率不同或不支持的分辨率，不能比较
	-1 其他原因不能比较
	0  图像相同
	1  图像不同
*/
int CompareBmp(char *pcBmp1, char *pcBmp2, unsigned short usResX, unsigned short usResY, int iSensitivity)
{
	int scale;

	if (pcBmp1 == NULL || pcBmp2 == NULL) return -2;

	printf ("Resolution: %d %d\n", (int) usResX, (int) usResY);

	if (usResX == 640 && usResY == 480)
		scale = 8;
	else if (usResX == 352 && usResY == 288)
		scale = 6;
	else if (usResX == 320 && usResY == 240)
		scale = 5;
	else if (usResX == 176 && usResY == 144)
		scale = 4;
	else
		return -2;

	if (iSensitivity < 0) iSensitivity = 0;
	if (iSensitivity > 2) iSensitivity = 2;
	iSensitivity++;

	switch (compare_image(usResX / 8, usResY / 8, (unsigned char *)pcBmp1 + 1, (unsigned char *)pcBmp2 + 1, scale, iSensitivity))
	{
	case 0:
		return 0;
	case 1:
		return 1;
	default:
		return -1;
	}

	return -1;
}


char *ReadFile (const char *pcPath, int *pnLen)
{
	struct stat st;
	if (stat (pcPath, &st) != 0)
		return NULL;
	else
	{
		FILE *fp = fopen (pcPath, "rb");
		if (fp == NULL)
			return NULL;
		else
		{
			char *rt = (char *) malloc (st.st_size);
			if (rt != NULL)
				printf ("Read len: %d\n", fread (rt, 1, st.st_size, fp));
			fclose (fp);

			if (pnLen != NULL)
				*pnLen = st.st_size;
			return rt;
		}
	}
}


void Usage (const char *argv0)
{
	printf ("%s <jpg1> <jpg2> <width> <height>\n", argv0);
}

int main (int argc, char **argv)
{
	char *bmp1;
	char *bmp2;
	int bmplen1;
	int bmplen2;
	
	int len1;
	int len2;
	RECT_T rect;
	
	unsigned short usResX, usResY;
	int cmp_result;
	
	char *jpg1;
	char *jpg2;

	if (argc != 5)
	{
		Usage (argv[0]);
		exit (0);
	}

	usResX = atoi (argv[3]);
	usResY = atoi (argv[4]);
		
	jpg1 = ReadFile (argv[1], &len1);
	jpg2 = ReadFile (argv[2], &len2);


	printf ("Jpg len: %d %d\n", len1, len2);
	
	rect.iLeft = 0;
	rect.iTop = 0;
	rect.iRight = 0;
	rect.iBottom = 0;
	JpegExt2Bmp (jpg1, len1, &bmp1, &bmplen1, &rect, usResX, usResY);
	JpegExt2Bmp (jpg2, len2, &bmp2, &bmplen2, &rect, usResX, usResY);

	printf ("Bmp len: %d %d\n", bmplen1, bmplen2);
	printf ("Bmp1: %d %d\n", (int) bmp1[0], (int) bmp1[1]);
	printf ("Bmp2: %d %d\n", (int) bmp2[0], (int) bmp2[1]);
		
	cmp_result = CompareBmp (bmp1, bmp2, usResX, usResY, 0);
	printf ("Compare result: %d\n", cmp_result);

	return 0;
}


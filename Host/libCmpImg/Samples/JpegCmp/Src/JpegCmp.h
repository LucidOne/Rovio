#ifndef JPEGCMP_H
#define JPEGCMP_H


#define BOOL char
#define TRUE 1
#define FALSE 0

typedef struct
{
	int iLeft;
	int iTop;
	int iRight;
	int iBottom;
} RECT_T;


BOOL JpegExt2Bmp(char *pcInBuf, int iInBufLen, char **ppcOutBuf, int *piOutBufLen, RECT_T *pRect, unsigned short usResX, unsigned short usResY);
int CompareBmp(char *pcBmp1, char *pcBmp2, unsigned short usResX, unsigned short usResY, int iSensitivity);
unsigned char GetResType(unsigned short usResX, unsigned short usResY);


#endif


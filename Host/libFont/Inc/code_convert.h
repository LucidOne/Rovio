#ifndef __CODE_CONVERT_H__
#define __CODE_CONVERT_H__



int CP936_to_UNICODE (EF_UCHAR ucS0, EF_UCHAR ucS1, EF_UCHAR *pucT0, EF_UCHAR *pucT1);
int CP950_to_UNICODE (EF_UCHAR ucS0, EF_UCHAR ucS1, EF_UCHAR *pucT0, EF_UCHAR *pucT1);

int ConvertString_CP950_to_UNICODE (const EF_UCHAR *pucSource, EF_USHORT *pwTarget, size_t szLength);
int ConvertString_CP936_to_UNICODE (const EF_UCHAR *pucSource, EF_USHORT *pwTarget, size_t szLength);
int ConvertString_ASCII_to_UNICODE (const EF_UCHAR *pucSource, EF_USHORT *pwTarget, size_t szLength);

#endif

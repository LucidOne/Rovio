#ifndef __LCMBYPASS_H__
#define __LCMBYPASS_H__


#define LEN_LCM_BUFFER (128*160*2)

VOID lcmbpInitLCD(INT nLCD);
BOOL lcmbpBitBltRGB565(INT nLCD, CONST VOID *pBuffer, INT nBufferLen);
BOOL lcmbpBitBltRGB888(INT nLCD, CONST VOID *pBuffer, INT nBufferLen);


#endif

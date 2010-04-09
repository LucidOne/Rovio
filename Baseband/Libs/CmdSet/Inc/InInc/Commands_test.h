#ifndef __COMMANDS_H__
#define __COMMANDS_H__



BOOL wbtGetFirmwareVersion (UCHAR * pucMajorVer, UCHAR * pucMinorVer);
BOOL wbtSetValue (UINT32 uValue);
BOOL wbtUpdateValue (UINT32 uValue, UINT32 *puOldValue);
BOOL wbtWriteBuffer (VOID *pBuffer, UINT32 uLen);
BOOL wbtReadBuffer (VOID *pBuffer, UINT32 uLen);
BOOL wbtSetFileInfo (CONST CHAR *cpcFilePath, UINT32 uSize);
BOOL wbtGetFileInfo (CHAR *pcFilePath, UINT32 uMaxPathLen, UINT32 *puSize);


#endif

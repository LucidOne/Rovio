#include "../../Platform/Inc/Platform.h"
#include "../../SoftPipe/include/softpipe.h"
#include "../../HIC/Inc/HIC.h"
#include "../Inc/InInc/Commands_test.h"



#if 0
/* Enable the following macro to dump the command names,
   Not all compiler supports __FUNCTION__!! */

/* Test __FUNCTION__ macro. */
static void TestFunctionMacro (void)
{
#ifndef __FUNCTION__
#define __FUNCTION__ __func__
#endif
}


static BOOL
(*__wbhicCmd_I) (UCHAR ucCommand,
                 UCHAR ucSubCommand,
                 UCHAR ucParam1,
                 UCHAR ucParam2,
                 UCHAR ucParam3,
                 UCHAR ucParam4) = &wbhicCmd_I;
#define wbhicCmd_I (sysPrintf ("Call: %s\n", __FUNCTION__), *__wbhicCmd_I)


static BOOL
(*__wbhicCmd_II) (UCHAR ucCommand,
                  UCHAR ucSubCommand,
                  UCHAR ucParam1,
                  UCHAR ucParam2,
                  UCHAR ucParam3,
                  UCHAR ucParam4,
                  UCHAR * pucRespon1,
                  UCHAR * pucRespon2,
                  UCHAR * pucRespon3,
                  UCHAR * pucRespon4) = &wbhicCmd_II;
#define wbhicCmd_II (sysPrintf ("Call: %s\n", __FUNCTION__), *__wbhicCmd_II)

static BOOL
(*__wbhicCmd_III_Write) (UCHAR ucCommand,
                         UCHAR ucSubCommand,
                         VOID *pSourceBuffer,
                         UINT32 uBufferType,
                         UINT32 *puBufferWritten) = &wbhicCmd_III_Write;
#define wbhicCmd_III_Write (sysPrintf ("Call: %s\n", __FUNCTION__), *__wbhicCmd_III_Write)


static BOOL
(*____wbhicCmd_III_Write_Len) (UCHAR ucCommand,
                               UCHAR ucSubCommand,
                               UINT32 uBufferLength) = &__wbhicCmd_III_Write_Len;
#define __wbhicCmd_III_Write_Len (sysPrintf ("Call: %s\n", __FUNCTION__), *____wbhicCmd_III_Write_Len)


static BOOL
(*__wbhicCmd_III_Read_Len ) (UCHAR ucCommand,
                             UCHAR ucSubCommand,
                             UCHAR ucParam1,
                             UCHAR ucParam2,
                             UCHAR ucParam3,
                             UCHAR ucParam4,
                             UINT32 *puBufferLength) = &wbhicCmd_III_Read_Len;
#define wbhicCmd_III_Read_Len (sysPrintf ("Call: %s\n", __FUNCTION__), *__wbhicCmd_III_Read_Len)

static BOOL
(*__wbhicCmd_IV) (UCHAR ucCommand,
                  UCHAR ucSubCommand,
                  UINT32 uParamNum,
                  ...) = &wbhicCmd_IV;
#define wbhicCmd_IV (sysPrintf ("Call: %s\n", __FUNCTION__), *__wbhicCmd_IV)

static BOOL
(*__wbhicCmd_V) (UCHAR ucCommand,
                 UCHAR ucSubCommand,
                 UINT32 uParamNum,
                 ...) = &wbhicCmd_V;
#define wbhicCmd_V (sysPrintf ("Call: %s\n", __FUNCTION__), *__wbhicCmd_V)

#endif





BOOL wbtGetFirmwareVersion (UCHAR * pucMajorVer, UCHAR * pucMinorVer)
{
    return wbhicCmd_II (0x19, 0x00,
                        0x00, 0x00, 0x00, 0x00,
                        pucMajorVer, pucMinorVer, NULL, NULL);
}


BOOL wbtSetValue (UINT32 uValue)
{
	UCHAR aucValue[4];
	
	UINT32_2_UCHAR (uValue, aucValue);
	return wbhicCmd_I (0x19, 0x01,
					   aucValue[3], aucValue[2], aucValue[1], aucValue[0]);
}


BOOL wbtUpdateValue (UINT32 uValue, UINT32 *puOldValue)
{
	UCHAR aucValue[4];
	UCHAR aucOldValue[4];
	
	UINT32_2_UCHAR (uValue, aucValue);
	if (!wbhicCmd_II (0x19, 0x02,
					 aucValue[3], aucValue[2], aucValue[1], aucValue[0],
					 &aucOldValue[3],
					 &aucOldValue[2],
					 &aucOldValue[1],
					 &aucOldValue[0]))
		return FALSE;
	
	if (puOldValue != NULL)
		UCHAR_2_UINT32 (aucOldValue, *puOldValue);
	
	return TRUE;
}


BOOL wbtWriteBuffer (VOID *pBuffer, UINT32 uLen)
{
	return wbhicCmd_III_Write (0x19, 0x03,
							   pBuffer, uLen,
							   NULL);
}

BOOL wbtReadBuffer (VOID *pBuffer, UINT32 uLen)
{
	BOOL bReturn;
	UCHAR aucLen[4];
	
	sysLockWbc ();
	
	UINT32_2_UCHAR (uLen, aucLen);
	bReturn = wbhicCmd_III_Read_Len (0x19, 0x04,
									 aucLen[3],
									 aucLen[2],
									 aucLen[1],
									 aucLen[0],
									 NULL);
	if (!bReturn)
		goto lOut;

	bReturn = wbhicReadFIFO (pBuffer, uLen, NULL);
	
lOut:
	sysUnlockWbc ();
	return bReturn;
}



BOOL wbtSetFileInfo (CONST CHAR *cpcFilePath, UINT32 uSize)
{
	UCHAR aucSize[4];
	
	UINT32_2_UCHAR (uSize, aucSize);
	return wbhicCmd_IV (0x19, 0x05,
						(UINT32) 2,	//Number of parameters
						(VOID *) cpcFilePath,
						(UINT32) (strlen (cpcFilePath) + 1),
						(VOID *) aucSize,
						(UINT32) sizeof (aucSize)
						);
}



BOOL wbtGetFileInfo (CHAR *pcFilePath, UINT32 uMaxPathLen, UINT32 *puSize)
{
	UCHAR aucSize[4];
	
	
	if (!wbhicCmd_V (0x19, 0x06,
					 (UINT32) 0,	//Number of parameters
					 (UINT32) 2,	//Number of responses
					 (VOID *) pcFilePath,
					 uMaxPathLen,
					 NULL,
					 (VOID *) aucSize,
					 (UINT32) sizeof (aucSize),
					 NULL))
		return FALSE;
		
	if (puSize != NULL)
		UCHAR_2_UINT32 (aucSize, *puSize);
	
	return TRUE;
}


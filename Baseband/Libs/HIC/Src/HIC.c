/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2005 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/

/****************************************************************************
 *
 * FILENAME
 *		$RCSfile: HIC.c,v $
 *
 * VERSION
 *		$Revision: 1.5 $
 *
 * DESCRIPTION
 *		W99702 hardware control interface.
 *
 * HISTORY
 *     $Log: HIC.c,v $
 *     Revision 1.5  2006/03/18 15:12:10  xhchen
 *     Test MP4 loopback by BB, OK.
 *     Add checksum for FIFO in protocol command.
 *
 *     Revision 1.4  2006/03/17 15:15:04  xhchen
 *     Test MP4 loopback by BB, almost OK.
 *
 *     Revision 1.3  2006/02/11 14:23:38  xhchen
 *     1. Add functions to get the CPU circles used by the scheduler
 *        (CPU circles used by timer isr).
 *     2. Move hic code from old message-task mode to multi-thread
 *        mode.
 *     3. Fix a bug in BB_Demo when dump error message.
 *
 *     Revision 1.2  2006/01/19 05:01:30  xhchen
 *     Move to new dir.
 *
 *     Revision 1.1  2006/01/17 09:42:12  xhchen
 *     Add B.B. testing applications.
 *
 *     Revision 1.1.2.14  2002/01/19 15:49:21  xhchen
 *     Fixed JPEG playback bugs.
 *     Do not stop preview at the time of comic snapshot.
 *
 *     Revision 1.1.2.13  2005/09/28 10:11:01  xhchen
 *     wpackage supports W99702 and W99802 now.
 *     Use zipped firmware.
 *
 *     Revision 1.1.2.12  2005/09/13 08:41:48  xhchen
 *     1. Mass storage demo added to FullDemo.
 *     2. wbhPlaybackJPEG/wbhPlaybackFile keeps the width/height proportion.
 *     3. Add wbSetLastError/wbGetLastError.
 *
 *     Revision 1.1.2.11  2005/09/07 03:32:06  xhchen
 *     1. Update command set API to WBCM_702_0.82pre14.doc.
 *     2. Add the function to playback audio by sending audio bitstream.
 *     3. Add "wbhicRescue()" to clear BUSY, DRQ an ERROR.
 *     4. Merget WYSun's testing code to "Samples/FlowTest".
 *
 *     Revision 1.1.2.10  2005/08/30 04:14:39  xhchen
 *     Makefile and create_project.js support searching dependence header file automatically.
 *     Add ywsun's filebrowser demo (for virtual com).
 *     Add audio test in "FlowTest".
 *
 *     Revision 1.1.2.9  2005/07/21 07:52:42  xhchen
 *     ...
 *
 *     Revision 1.1.2.8  2005/07/20 09:18:58  xhchen
 *     1. soft pipe: remove fnTrData, use fnOnData only.
 *     2. Movie record: check status after record stopped,
 *        add parameter to set volume, bitrate and size when recording.
 *        Fix a bug when recording movie: OSD did not properly been shown.
 *
 *     Revision 1.1.2.7  2005/07/15 08:20:07  xhchen
 *     Use PIPE to transfer FIFO instead of callback functions.
 *
 *     Revision 1.1.2.6  2005/06/24 11:19:19  xhchen
 *     Fixed big endian problem.
 *     FIFO can use a function as it's R/W data now.
 *
 *     Revision 1.1.2.5  2005/05/16 08:33:27  xhchen
 *         1. Add HIC debug function EnableHICDebug(BOOL bEnable). When HIC debug is on, almost all the HIC I/O operation will be printed to console.
 *     	2. Command set API according to "WBCM_702_0.8.pdf".
 *         3. Add code to check card before playback/record.
 *     	4. MP3 playback window:
 *     	   a. Add a spectrum chart on the window
 *     	   b. Display ID3 tags (artist, album & genre) according to the "Language" settings.
 *     	   c. Add code to setting equalizer by users.
 *     	   d. Continue to play next audio file after current file is over.
 *
 *     Revision 1.1.2.4  2005/01/11 09:42:01  xhchen
 *     Bug fixed: FIFO write in 16bit mode.
 *     Verify YHTu's new firmware.
 *
 *     Revision 1.1.2.3  2004/12/22 09:46:01  xhchen
 *     ...
 *
 *
 * REMARK
 *     None
 *
 **************************************************************************/



#include "../../Platform/Inc/Platform.h"
#include "../../SoftPipe/include/softpipe.h"
#include "../Inc/HIC.h"




/****************************************************************************
 *
 * FUNCTION
 *		wbhicInit
 *
 * DESCRIPTION
 *		HIC initialization.
 *		change HIC access mode (index/address mode, bitwidth).
 *
 * INPUTS
 *		None
 *
 * OUTPUTS
 *		None
 *
 * RETURN
 *		TRUE, success.
 *		FALSE, failed.
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
BOOL wbhicInit(VOID)
{
	// At the beginning of the Power-On of W99702,
	// it is always 8-bit INDEX mode, use registers CF_I_xxx.

	BOOL bOK;
	
	// Pseudo write when DRQ is set.
	if ((CF_I_STATUS & HICST_DRQ) == HICST_DRQ)
	{
		UCHAR ucDummy;
		sysPrintf ("HIC STATUS(%02x): DRQ is on, begin to clear DRQ.\n", (int) CF_I_STATUS);
		CF_I_ADDR0 = 0;
		CF_I_ADDR1 = 0;
		CF_I_ADDR2 = 0;
		CF_I_ADDR3 = 0;
		CF_I_COMMAND = HICCMD_SINGLE_DATA_READ;
		sysPrintf ("HIC STATUS(%02x)\n", (int) CF_I_STATUS);
		WB_COND_WAITFOR (((CF_I_STATUS & HICST_DRQ) == HICST_DRQ), CF_TIMEOUT_MSEC, bOK);
		if (!bOK)
		{
			D_ERROR("CF_I_STATUS\n");
			wbSetLastError (HIC_ERR_ST_NO_DRQ);
			return FALSE;
		}
		ucDummy = CF_I_DATA;
		ucDummy = CF_I_DATA;
		ucDummy = CF_I_DATA;
		ucDummy = CF_I_DATA;
		sysPrintf ("HIC STATUS(%02x)\n", (int) CF_I_STATUS);
	}

	//	REG:	REG_HICFCR (0x7FF01000)
	//	i8:		0x2001
	//	i16:	0x2101
	//	i9:		0x2201
	//	i18:	0x2301
	//	a8:		0x0001
	//	a16:	0x0101
	//	a9:		0x0201
	//	a18:	0x0301

#if defined(_HIC_MODE_I8_)
#	define HIC_MODE_VALUE 0x2001
#elif defined(_HIC_MODE_I16_)
#	define HIC_MODE_VALUE 0x2101
#elif defined(_HIC_MODE_I9_)
#	define HIC_MODE_VALUE 0x2201
#elif defined(_HIC_MODE_I18_)
#	define HIC_MODE_VALUE 0x2301
#elif defined(_HIC_MODE_A8_)
#	define HIC_MODE_VALUE 0x0001
#elif defined(_HIC_MODE_A16_)
#	define HIC_MODE_VALUE 0x0101
#elif defined(_HIC_MODE_A9_)
#	define HIC_MODE_VALUE 0x0201
#elif defined(_HIC_MODE_A18_)
#	define HIC_MODE_VALUE 0x0301
#else
#	error "No HIC mode defined."
#endif

	// wait for BUSY
	WB_COND_WAITFOR (((CF_I_STATUS & HICST_BUSY) == 0), CF_TIMEOUT_MSEC, bOK);
	if (!bOK)
	{
		D_ERROR("CF_I_STATUS\n");
		wbSetLastError (HIC_ERR_ST_BUSY);
		return FALSE;
	}

	CF_I_ADDR0 = (UCHAR)(REG_HICFCR & 0x000000FF);
	CF_I_ADDR1 = (UCHAR)((REG_HICFCR & 0x0000FF00) >> 8);
	CF_I_ADDR2 = (UCHAR)((REG_HICFCR & 0x00FF0000) >> 16);
	CF_I_ADDR3 = (UCHAR)((REG_HICFCR & 0xFF000000) >> 24);
	CF_I_COMMAND = HICCMD_SINGLE_DATA_WRITE;

	// wait for DReq
	WB_COND_WAITFOR (((CF_I_STATUS & HICST_DRQ) == HICST_DRQ), CF_TIMEOUT_MSEC, bOK);
	if (!bOK)
	{
		D_ERROR("CF_I_STATUS\n");
		wbSetLastError (HIC_ERR_ST_NO_DRQ);
		return FALSE;
	}

	//
	CF_I_DATA = (UCHAR)(HIC_MODE_VALUE & 0x000000FF);
	CF_I_DATA = (UCHAR)((HIC_MODE_VALUE & 0x0000FF00) >> 8);
	CF_I_DATA = (UCHAR)((HIC_MODE_VALUE & 0x00FF0000) >> 16);
	CF_I_DATA = (UCHAR)((HIC_MODE_VALUE & 0xFF000000) >> 24);


	// !!! Because, lines after here must use
	//		mode-independent register definitions:
	//		CF_xxxx
	//sysMSleep(200);
	{
		UINT32 uValue = 0xFFFFFFFF;
		if (!wbhicSingleRead(REG_PDID, &uValue))
			return FALSE;
		sysPrintf("Read ID: %08x\n", uValue);

	}

	return TRUE;
}




static BOOL wbhicSingleRead_NoLock(UINT32 uAddress, UINT32 *puValue)
{
	UCHAR pucValue[4];

	if (! __wbhicRegRead(FALSE,
						 HICCMD_SINGLE_DATA_READ,
						 uAddress,
						 sizeof(pucValue),
						 (VOID *) pucValue))
		return FALSE;

	if (puValue != NULL)
		UCHAR_2_UINT32 (pucValue, *puValue);

	return TRUE;
}

static BOOL wbhicSingleWrite_NoLock (UINT32 uAddress, UINT32 uValue)
{
	UCHAR pucValue[4];
	UINT32_2_UCHAR (uValue, pucValue);
	return __wbhicRegWrite(FALSE,
						   HICCMD_SINGLE_DATA_WRITE,
						   uAddress,
						   sizeof(pucValue),
						   (VOID *) pucValue);
}

static BOOL wbhicBurstRead_NoLock (UINT32 uAddress, UINT32 uBufferType, VOID *pTargetBuffer)
{
	return __wbhicRegRead (FALSE,
						   HICCMD_BURST_DATA_READ,
						   uAddress,
						   uBufferType,
						   pTargetBuffer);
}


BOOL wbhicRescue (VOID)
{
	UINT32 uValue;
	UINT32 uLine;
	UCHAR *pucFileName;
#define DEBUG_INFO_ADDR		0x40
#define DEBUG_INFO_SIZE		0x20
#define DEBUG_ERROR_ADDR	0x60
#define DEBUG_ERROR_SIZE	0x80
#define MAX(a,b) ((a) > (b) ? (a) : (b))
	UCHAR aucTmp[MAX(MAX(DEBUG_INFO_SIZE, DEBUG_ERROR_SIZE), sizeof (UINT32))];

	sysPrintf ("Rescue HIC from STATUS: %02x (%02x %02x %02x %02x %02x)\n",
		(INT) CF_STATUS,
		(INT) CF_REGE,
		(INT) CF_REGD,
		(INT) CF_REGC,
		(INT) CF_REGB,
		(INT) CF_REGA);
	/* <1> "Attempt" to clear BUSY or DRQ by a dummy reading. */
	CF_ADDR0 = 0;
	CF_ADDR1 = 0;
	CF_ADDR2 = 0;
	CF_ADDR3 = 0;
	CF_COMMAND = HICCMD_SINGLE_DATA_READ;
	wbhicReadFIFO (aucTmp, sizeof (UINT32), NULL);
	sysPrintf ("HIC STATUS after dummy read: %x\n", (INT) CF_STATUS);

	/* <2> Clear ERR bit by write REG_HICSR. */
	wbhicSingleRead_NoLock (REG_HICSR, &uValue);
	wbhicSingleWrite_NoLock (REG_HICSR, uValue & ~HICST_ERROR);
	sysPrintf ("HIC STATUS after clear ERR bit: %x\n", (INT) CF_STATUS);

	/* <3> Print debug information: in which file and line the firmware runs. */
	wbhicBurstRead_NoLock (DEBUG_INFO_ADDR, DEBUG_INFO_SIZE, (VOID *) aucTmp);
	UCHAR_2_UINT32 (aucTmp, uLine);
	pucFileName = aucTmp + sizeof (UINT32);
	sysPrintf ("Firmware: %s,%d\n", pucFileName, uLine);
	
	/* <4> Print debug information: the error information recorded by firmware. */
	wbhicBurstRead_NoLock (DEBUG_ERROR_ADDR, DEBUG_ERROR_SIZE, (VOID *) aucTmp);
	UCHAR_2_UINT32 (aucTmp, uValue);
	UCHAR_2_UINT32 (aucTmp + sizeof (UINT32), uLine);
	pucFileName = aucTmp + sizeof (UINT32) * 2;
	sysPrintf ("Firmware last error: %08x, [%s,%d]\n", uValue, pucFileName, uLine);
	sysPrintf ("  %s\n", pucFileName + strlen ((const char *) pucFileName) + 1);

	return TRUE;
}


static INT32 g_nErrorCode = 0;
INT32 __wbSetLastError (INT32 nErrorCode, const char *cpFile, int nLine)
{
	INT32 nErrorCode_Old = g_nErrorCode;
	g_nErrorCode = nErrorCode;
	if (cpFile != NULL)
		sysPrintf ("Set err: %x %x (%s,%d)\n", nErrorCode, nErrorCode_Old, cpFile, nLine);
	return nErrorCode_Old;
}

INT32 __wbGetLastError (VOID)
{
	return __wbSetLastError (0, NULL, 0);
}



#ifdef HIC_DEBUG
int g_iHIC_DEBUG_FIFO	= 0;
int g_iHIC_DEBUG_CMD	= 0;
void EnableHICDebugFIFO (BOOL bEnable)
{
	g_iHIC_DEBUG_FIFO = bEnable ? 1 : 0;
}
void EnableHICDebugCMD (BOOL bEnable)
{
	g_iHIC_DEBUG_CMD = bEnable ? 1 : 0;
}
#endif


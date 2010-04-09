/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2005 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/

/****************************************************************************
 *
 * FILENAME
 *		$RCSfile: ModeDataTransfer.c,v $
 *
 * VERSION
 *		$Revision: 1.2 $
 *
 * DESCRIPTION
 *		Functions for HIC data transfer mode.
 *
 * HISTORY
 *     $Log: ModeDataTransfer.c,v $
 *     Revision 1.2  2006/03/18 15:12:10  xhchen
 *     Test MP4 loopback by BB, OK.
 *     Add checksum for FIFO in protocol command.
 *
 *     Revision 1.1  2006/01/17 09:42:12  xhchen
 *     Add B.B. testing applications.
 *
 *     Revision 1.1.2.14  2005/09/28 10:11:01  xhchen
 *     wpackage supports W99702 and W99802 now.
 *     Use zipped firmware.
 *
 *     Revision 1.1.2.13  2005/09/13 08:41:48  xhchen
 *     1. Mass storage demo added to FullDemo.
 *     2. wbhPlaybackJPEG/wbhPlaybackFile keeps the width/height proportion.
 *     3. Add wbSetLastError/wbGetLastError.
 *
 *     Revision 1.1.2.12  2005/08/30 04:14:39  xhchen
 *     Makefile and create_project.js support searching dependence header file automatically.
 *     Add ywsun's filebrowser demo (for virtual com).
 *     Add audio test in "FlowTest".
 *
 *     Revision 1.1.2.11  2005/07/21 07:52:42  xhchen
 *     ...
 *
 *     Revision 1.1.2.10  2005/07/20 09:18:58  xhchen
 *     1. soft pipe: remove fnTrData, use fnOnData only.
 *     2. Movie record: check status after record stopped,
 *        add parameter to set volume, bitrate and size when recording.
 *        Fix a bug when recording movie: OSD did not properly been shown.
 *
 *     Revision 1.1.2.9  2005/07/15 08:20:07  xhchen
 *     Use PIPE to transfer FIFO instead of callback functions.
 *
 *     Revision 1.1.2.8  2005/06/24 11:19:19  xhchen
 *     Fixed big endian problem.
 *     FIFO can use a function as it's R/W data now.
 *
 *     Revision 1.1.2.7  2005/05/10 10:14:32  xhchen
 *     Stable for W99702B.
 *
 *     Revision 1.1.2.6  2005/04/01 02:14:58  xhchen
 *     More effective FIFO I/O OK.
 *
 *     Revision 1.1.2.5  2005/03/31 12:35:37  xhchen
 *     Some FIFO I/O method added for comparing performance.
 *
 *     Revision 1.1.2.4  2005/01/07 08:41:34  xhchen
 *     Winbond Camera Module High Level API first version released.
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




#ifdef HIC_DEBUG
#	define PRINT_DEBUG_DCMD_WRITE_REG(r_f, r_e, r_d, r_c, r_b, r_a, r_9, r_6, addr, len) \
		do \
		{ \
			if (g_iHIC_DEBUG_CMD) \
			{ \
				sysPrintf ("(%08d) DCMD WR REG[FEDCBA96] (%08x %08x): %02x %02x %02x %02x %02x %02x %02x %02x\n", __LINE__, \
					addr, len, \
					(int) (r_f), (int) (r_e), (int) (r_d), (int) (r_c), (int) (r_b), (int) (r_a), (int) (r_9), (int) (r_6)); \
			} \
		} while (0)
#else
#	define PRINT_DEBUG_DCMD_WRITE_REG(r_f, r_e, r_d, r_c, r_b, r_a, r_9, r_6, addr, len)
#endif



/****************************************************************************
 *
 * FUNCTION
 *		__wbhicRegRead
 *
 * DESCRIPTION
 *		Read W99702 memory/registers.
 *
 * INPUTS
 *		ucCommand: Command described by CF_REGF
 *		uAddress: Memory/register starting address
 *		uBufferType: The input buffer's type. If its value is not
 *                   "WBHIC_FUNC_IO", it's used as the buffer length.
 *                   (for compatibility)
 *
 * OUTPUTS
 *		pTargetBuffer: If "uBufferType" is "WBHIC_FUNC_IO", it points to a
 *                     structure WBHIC_FIFO_FUNCTGT_PARAM_T, otherwise it's
 *                     a buffer.
 *
 * RETURN
 *		TRUE, success.
 *		FALSE, failed.
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
BOOL __wbhicRegRead(BOOL bLock,
					UCHAR ucCommand,
					UINT32 uAddress,
					UINT32 uBufferType,
					VOID *pTargetBuffer)
{
	BOOL bReturn;
	UINT32 uLen;
	UINT32 uCount;
	D_TRACE(0xF0D0, "__wbhicRegRead\n");

#	ifdef _HIC_CACHE_ON_
	uAddress|=0x10000000;
#	endif

	if (bLock)
	{
		sysLockWbc ();
	}
	
	/* The buffer length. */
	if (uBufferType == WBHIC_FIFO_PIPE_IO)
	{
		WBHIC_FIXLEN_PIPE_T *pPipeTgt =
			(WBHIC_FIXLEN_PIPE_T *) pTargetBuffer;
		uLen = pPipeTgt->uLength;
	}
	else
		uLen = uBufferType;

	//ASSERT(uLen % 4 == 0);

#if defined(_HIC_MODE_A16_) || defined(_HIC_MODE_A18_) || defined(_HIC_MODE_I16_) || defined(_HIC_MODE_I18_)
	uCount = (uLen + 1) / 2;
#elif defined(_HIC_MODE_A8_) || defined(_HIC_MODE_A9_) || defined(_HIC_MODE_I8_) || defined(_HIC_MODE_I9_)
	uCount = uLen;
#else
#	error "No HIC access mode defined."
#endif

	// wait for CMDBusy cleared
	WB_COND_WAITFOR (((CF_STATUS & HICST_BUSY) == 0x00), CF_TIMEOUT_MSEC, bReturn);
	if (!bReturn)
	{
		D_ERROR("__wbhicRegRead: CMDBusy!");
		wbSetLastError (HIC_ERR_ST_BUSY);
		goto lOut;
	}

	CF_ADDR0 = (UCHAR)(uAddress & 0x000000FF);			/* data address [7:0] */
 	CF_ADDR1 = (UCHAR)((uAddress & 0x0000FF00) >> 8);	/* data address [15:8] */
 	CF_ADDR2 = (UCHAR)((uAddress & 0x00FF0000) >> 16);	/* data address [23:16] */
 	CF_ADDR3 = (UCHAR)((uAddress & 0xFF000000) >> 24);	/* data address [31:24] */
 	CF_LEN0 =  (UCHAR)(uCount & 0x000000FF);			/* transfer count [7:0] */
	CF_LEN1 = (UCHAR)((uCount & 0x0000FF00) >> 8); 		/* transfer count [15:8] */
 	CF_LEN2 = (UCHAR)((uCount & 0x00FF0000) >> 16);		/* transfer count [23:16] */

	CF_COMMAND = ucCommand;

	PRINT_DEBUG_DCMD_WRITE_REG (ucCommand,
		(UCHAR)((uCount & 0x0000FF00) >> 8),
		(UCHAR)(uCount & 0x000000FF),
		(UCHAR)((uAddress & 0xFF000000) >> 24),
		(UCHAR)((uAddress & 0x00FF0000) >> 16),
		(UCHAR)((uAddress & 0x0000FF00) >> 8),
		(UCHAR)(uAddress & 0x000000FF),
		(UCHAR)((uCount & 0x00FF0000) >> 16),
		uAddress,
		uLen);

	bReturn = wbhicReadFIFO(pTargetBuffer, uBufferType, NULL);
	
lOut:
	if (bLock)
	{
		sysUnlockWbc ();
	}
	return bReturn;	
}



/****************************************************************************
 *
 * FUNCTION
 *		__wbhicRegWrite
 *
 * DESCRIPTION
 *		Write W99702 memory/registers.
 *
 * INPUTS
 *		ucCommand: Command described by CF_REGF
 *		uAddress: Memory/register starting address
 *		uBufferType: The input buffer's type. If its value is not
 *                   "WBHIC_FUNC_IO", it's used as the buffer length.
 *                   (for compatibility)
 *		pSourceBuffer: If "uBufferType" is "WBHIC_FUNC_IO", it points to a
 *                     structure WBHIC_FIFO_FUNCSRC_PARAM_T, otherwise it's
 *                     a buffer.
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
BOOL __wbhicRegWrite(BOOL bLock,
					 UCHAR ucCommand,
					 UINT32 uAddress,
					 UINT32 uBufferType,
					 VOID *pSourceBuffer)
{
	BOOL bReturn;
	UINT32 uLen;
	UINT32 uCount;

	D_TRACE( 0xF0D3, "__wbhicRegWrite\n");

#	ifdef _HIC_CACHE_ON_
	uAddress|=0x10000000;
#	endif

	if (bLock)
	{
		sysLockWbc ();
	}
	
	/* The buffer length. */
	if (uBufferType == WBHIC_FIFO_PIPE_IO)
	{
		WBHIC_FIXLEN_PIPE_T *pPipeSrc =
			(WBHIC_FIXLEN_PIPE_T *) pSourceBuffer;
		uLen = pPipeSrc->uLength;
	}
	else
		uLen = uBufferType;


	//ASSERT(uLen % 4 == 0);

#if defined(_HIC_MODE_A16_) || defined(_HIC_MODE_A18_) || defined(_HIC_MODE_I16_) || defined(_HIC_MODE_I18_)
	uCount = (uLen + 1) / 2;
#elif defined(_HIC_MODE_A8_) || defined(_HIC_MODE_A9_) || defined(_HIC_MODE_I8_) || defined(_HIC_MODE_I9_)
	uCount = uLen;
#else
#	error "No HIC access mode defined."
#endif

	// wait for CMDBusy cleared
	WB_COND_WAITFOR (((CF_STATUS & HICST_BUSY) == 0x00), CF_TIMEOUT_MSEC, bReturn);
	if (!bReturn)
	{
		D_ERROR("wbhicBurstWrite: CMDBusy!");
		wbSetLastError (HIC_ERR_ST_BUSY);
		goto lOut;
	}

 	CF_ADDR0 = (UCHAR)(uAddress & 0x000000FF);			/* data address [7:0] */
 	CF_ADDR1 = (UCHAR)((uAddress & 0x0000FF00) >> 8);	/* data address [15:8] */
 	CF_ADDR2 = (UCHAR)((uAddress & 0x00FF0000) >> 16);	/* data address [23:16] */
 	CF_ADDR3 = (UCHAR)((uAddress & 0xFF000000) >> 24);	/* data address [31:24] */
 	CF_LEN0 = (UCHAR)(uCount & 0x000000FF);				/* transfer count [7:0] */
	CF_LEN1 = (UCHAR)((uCount & 0x0000FF00) >> 8);  	/* transfer count [15:8] */
 	CF_LEN2 = (UCHAR)((uCount & 0x00FF0000) >> 16);		/* transfer count [23:16] */

	CF_COMMAND = ucCommand;

	PRINT_DEBUG_DCMD_WRITE_REG (ucCommand,
		(UCHAR)((uCount & 0x0000FF00) >> 8),
		(UCHAR)(uCount & 0x000000FF),
		(UCHAR)((uAddress & 0xFF000000) >> 24),
		(UCHAR)((uAddress & 0x00FF0000) >> 16),
		(UCHAR)((uAddress & 0x0000FF00) >> 8),
		(UCHAR)(uAddress & 0x000000FF),
		(UCHAR)((uCount & 0x00FF0000) >> 16),
		uAddress,
		uLen);

	bReturn = wbhicWriteFIFO(pSourceBuffer, uBufferType, NULL);

lOut:
	if (bLock)
	{
		sysUnlockWbc ();
	}
	return bReturn;	
}


/****************************************************************************
 *
 * FUNCTION
 *		wbhicSingleRead
 *
 * DESCRIPTION
 *		Read single W99702 memory/registers.
 *
 * INPUTS
 *		uAddress: Memory/register starting address
 *
 * OUTPUTS
 *		puValue: The value read.
 *
 * RETURN
 *		TRUE, success.
 *		FALSE, failed.
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
BOOL wbhicSingleRead(UINT32 uAddress, UINT32 *puValue)
{
	UCHAR pucValue[4];

	if (! __wbhicRegRead(TRUE,
						 HICCMD_SINGLE_DATA_READ,
						 uAddress,
						 sizeof(pucValue),
						 (VOID *) pucValue))
		return FALSE;

	if (puValue != NULL)
		UCHAR_2_UINT32 (pucValue, *puValue);

	return TRUE;
}



/****************************************************************************
 *
 * FUNCTION
 *		wbhicSingleWrite
 *
 * DESCRIPTION
 *		Write single W99702 memory/registers.
 *
 * INPUTS
 *		uAddress: Memory/register starting address
 *		uValue: The value to be written
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
BOOL wbhicSingleWrite(UINT32 uAddress, UINT32 uValue)
{
	UCHAR pucValue[4];
	UINT32_2_UCHAR (uValue, pucValue);
	return __wbhicRegWrite(TRUE,
						   HICCMD_SINGLE_DATA_WRITE,
						   uAddress,
						   sizeof(pucValue),
						   (VOID *) pucValue);
}



/****************************************************************************
 *
 * FUNCTION
 *		wbhicSpecialWrite
 *
 * DESCRIPTION
 *		Write special W99702 memory/registers.
 *
 * INPUTS
 *		uAddress: Memory/register starting address
 *		uValue: The value to be written
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
BOOL wbhicSpecialWrite(UINT32 uAddress, UINT32 uValue)
{
	UCHAR pucValue[4];
	UINT32_2_UCHAR (uValue, pucValue);
	return __wbhicRegWrite(TRUE,
						   HICCMD_SPECIAL_DATA_WRITE,
						   uAddress,
						   sizeof(pucValue),
						   (VOID *) pucValue);
}



/****************************************************************************
 *
 * FUNCTION
 *		wbhicBurstRead
 *
 * DESCRIPTION
 *		Burst read W99702 memory/registers.
 *
 * INPUTS
 *		uAddress: Memory/register starting address
 *		uBufferType: The input buffer's type. If its value is not
 *                   "WBHIC_FUNC_IO", it's used as the buffer length.
 *                   (for compatibility)
 *
 * OUTPUTS
 *		pTargetBuffer: If "uBufferType" is "WBHIC_FUNC_IO", it points to a
 *                     structure WBHIC_FIFO_FUNCTGT_PARAM_T, otherwise it's
 *                     a buffer.
 *
 * RETURN
 *		TRUE, success.
 *		FALSE, failed.
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
BOOL wbhicBurstRead (UINT32 uAddress, UINT32 uBufferType, VOID *pTargetBuffer)
{
	return __wbhicRegRead (TRUE,
						   HICCMD_BURST_DATA_READ,
						   uAddress,
						   uBufferType,
						   pTargetBuffer);
}



/****************************************************************************
 *
 * FUNCTION
 *		wbhicBurstWrite
 *
 * DESCRIPTION
 *		Burst write W99702 memory/registers.
 *
 * INPUTS
 *		uAddress: Memory/register starting address
 *		uBufferType: The input buffer's type. If its value is not
 *                   "WBHIC_FUNC_IO", it's used as the buffer length.
 *                   (for compatibility)
 *		pSourceBuffer: If "uBufferType" is "WBHIC_FUNC_IO", it points to a
 *                     structure WBHIC_FIFO_FUNCSRC_PARAM_T, otherwise it's
 *                     a buffer.
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
BOOL wbhicBurstWrite (UINT32 uAddress, UINT32 uBufferType, VOID *pSourceBuffer)
{
	return __wbhicRegWrite (TRUE,
							HICCMD_BURST_DATA_WRITE,
							uAddress,
							uBufferType,
							pSourceBuffer);
}


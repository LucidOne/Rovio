
/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2005 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/

/****************************************************************************
 *
 * FILENAME
 *		$RCSfile: FIFO16.c,v $
 *
 * VERSION
 *		$Revision: 1.5 $
 *
 * DESCRIPTION
 *		W99702 hardware control interface: FIFO read/write.
 *
 * HISTORY
 *     $Log: FIFO16.c,v $
 *     Revision 1.5  2006/03/19 12:10:38  xhchen
 *     Add the function to change video size and z-index
 *     for the demo.
 *     Two bugs still in checking:
 *     a) why MP4 encode/decode sometimes fails.
 *     b) wby HIC sometimes transfers wrong data.
 *
 *     Revision 1.4  2006/03/18 15:12:10  xhchen
 *     Test MP4 loopback by BB, OK.
 *     Add checksum for FIFO in protocol command.
 *
 *     Revision 1.3  2006/02/22 14:05:58  xhchen
 *     1. Serious bug fixed: old thread code (tt_reg_load_in_svc) may
 *        use too large stack, which may cause a fatal error.
 *     2. Move the allocate function (for large buffer) to a single file.
 *     3. Add capture thread. "capture thread" captures video, converts
 *        video by VPE and draws video by LCM.
 *     4. Test encode/decode amr, OK.
 *     5. Add recursive mutex (a lock that can not be locked by the
 *        thread that owns the lock itself.)
 *     6. Test video capture, OK. (dsplib, i2clib)
 *
 *     Revision 1.2  2006/02/14 08:56:48  xhchen
 *     1. Protocol commands support OSD is OK.
 *     2. Bug fixed: memory may be lost if system clock is from crystal.
 *        (why?)
 *     3. Bug fixed: if vpost clock is enabled after HIC interrupt is
 *        received, vpost can not refresh LCM. (why?)
 *
 *     Revision 1.1  2006/01/17 09:42:12  xhchen
 *     Add B.B. testing applications.
 *
 *     Revision 1.1.2.16  2005/09/13 08:41:48  xhchen
 *     1. Mass storage demo added to FullDemo.
 *     2. wbhPlaybackJPEG/wbhPlaybackFile keeps the width/height proportion.
 *     3. Add wbSetLastError/wbGetLastError.
 *
 *     Revision 1.1.2.15  2005/08/30 04:14:39  xhchen
 *     Makefile and create_project.js support searching dependence header file automatically.
 *     Add ywsun's filebrowser demo (for virtual com).
 *     Add audio test in "FlowTest".
 *
 *     Revision 1.1.2.14  2005/08/25 05:41:14  xhchen
 *     Add Virtual com to FullDemo.
 *     Begin to add FlowTest, a testing application almost for all functions.
 *
 *     Revision 1.1.2.13  2005/07/21 07:52:42  xhchen
 *     ...
 *
 *     Revision 1.1.2.12  2005/07/20 09:18:58  xhchen
 *     1. soft pipe: remove fnTrData, use fnOnData only.
 *     2. Movie record: check status after record stopped,
 *        add parameter to set volume, bitrate and size when recording.
 *        Fix a bug when recording movie: OSD did not properly been shown.
 *
 *     Revision 1.1.2.11  2005/07/15 08:20:07  xhchen
 *     Use PIPE to transfer FIFO instead of callback functions.
 *
 *     Revision 1.1.2.10  2005/06/29 10:12:10  xhchen
 *     ...
 *
 *     Revision 1.1.2.9  2005/06/24 11:19:19  xhchen
 *     Fixed big endian problem.
 *     FIFO can use a function as it's R/W data now.
 *
 *     Revision 1.1.2.8  2005/05/16 08:33:27  xhchen
 *         1. Add HIC debug function EnableHICDebug(BOOL bEnable). When HIC debug is on, almost all the HIC I/O operation will be printed to console.
 *     	2. Command set API according to "WBCM_702_0.8.pdf".
 *         3. Add code to check card before playback/record.
 *     	4. MP3 playback window:
 *     	   a. Add a spectrum chart on the window
 *     	   b. Display ID3 tags (artist, album & genre) according to the "Language" settings.
 *     	   c. Add code to setting equalizer by users.
 *     	   d. Continue to play next audio file after current file is over.
 *
 *     Revision 1.1.2.7  2005/05/12 03:36:48  xhchen
 *     Fix LoadFromMemoryCard bugs.
 *     Add 3GP demo.
 *
 *     Revision 1.1.2.6  2005/05/10 10:14:32  xhchen
 *     Stable for W99702B.
 *
 *     Revision 1.1.2.5  2005/04/01 02:14:58  xhchen
 *     More effective FIFO I/O OK.
 *
 *     Revision 1.1.2.4  2005/03/31 12:35:37  xhchen
 *     Some FIFO I/O method added for comparing performance.
 *
 *     Revision 1.1.2.3  2005/03/31 04:12:28  xhchen
 *     Speed up FIFO I/O.
 *
 *     Revision 1.1.2.2  2005/01/14 06:05:42  xhchen
 *     2nd High level API released.
 *
 *     Revision 1.1.2.1  2005/01/11 09:42:01  xhchen
 *     Bug fixed: FIFO write in 16bit mode.
 *     Verify YHTu's new firmware.
 *
 *
 * REMARK
 *     None
 *
 **************************************************************************/



#include "../../Platform/Inc/Platform.h"
#include "../../SoftPipe/include/softpipe.h"
#include "../Inc/HIC.h"


#if defined(_HIC_MODE_A16_) || defined(_HIC_MODE_A18_) || defined (_HIC_MODE_I16_) || defined(_HIC_MODE_I18_)



#ifndef DISABLE_GDMA
#include "../../DMA/Inc/DMA.h"
typedef struct
{
	SYS_WAIT_OBJ_T wait;
	WBHIC_FIFO_T *pwbhicFIFO;
	UINT32 uAddress;
	UINT32 uBlocks;
	UINT32 auBuffer[SIZE_FIFO / sizeof (UINT32)];
} WBHIC_DMA_T;

#define NON_CACHE(addr) ((UINT32) (addr) | 0x80000000)
static WBHIC_DMA_T *g_pDMA = NULL;



void wbhicPartReadFIFO_StartDMA (WBHIC_DMA_T *pDMA);
void wbhicPartReadFIFO_OnDMA ()
{
	memcpy ((void *) g_pDMA->uAddress, (const void *) NON_CACHE (g_pDMA->auBuffer), SIZE_FIFO);

	g_pDMA->uBlocks--;
	g_pDMA->uAddress += SIZE_FIFO;
	wbhicPartReadFIFO_StartDMA (g_pDMA);
}


void wbhicPartReadFIFO_StartDMA (WBHIC_DMA_T *pDMA)
{
	if (pDMA->uBlocks > 0)
	{
		g_pDMA = pDMA;
		
		while (! ((CF_STATUS & HICST_DRQ) == HICST_DRQ));
				
		dmaMemcpy ((UINT32) &CF_REG8, NON_CACHE (pDMA->auBuffer),
			2, SIZE_FIFO / 2,
			0, 1,
			wbhicPartReadFIFO_OnDMA, NULL);
	}
	else
	{
		sysWakeup (&pDMA->wait);
	}
}




void wbhicPartWriteFIFO_StartDMA (WBHIC_DMA_T *pDMA);
void wbhicPartWriteFIFO_OnDMA ()
{
	g_pDMA->uBlocks--;
	g_pDMA->uAddress += SIZE_FIFO;
	wbhicPartWriteFIFO_StartDMA (g_pDMA);
}


void wbhicPartWriteFIFO_StartDMA (WBHIC_DMA_T *pDMA)
{
	if (pDMA->uBlocks > 0)
	{
		g_pDMA = pDMA;
		
		while (! ((CF_STATUS & HICST_DRQ) == HICST_DRQ));

		memcpy ((void *) NON_CACHE (pDMA->auBuffer), (const void *) pDMA->uAddress, SIZE_FIFO);
		dmaMemcpy (NON_CACHE (pDMA->auBuffer), (UINT32) &CF_REG8,
			2, SIZE_FIFO / 2,
			1, 0,
			wbhicPartWriteFIFO_OnDMA, NULL);
	}
	else
	{
		sysWakeup (&pDMA->wait);
	}
}


#endif




#ifdef HIC_DEBUG
#	define PRINT_DEBUG_FIFO16_REG(operation, value_low, value_high) \
		do \
		{ \
			if (g_iHIC_DEBUG_FIFO) \
			{ \
				sysPrintf ("(%08d) %s FIFO16: %02x %02x\n", __LINE__, operation, (int) (value_low), (int) (value_high)); \
			} \
		} while (0)
#	define PRINT_DEBUG_FIFO16_WAIT_DRQ \
		do \
		{ \
			if (g_iHIC_DEBUG_FIFO) \
			{ \
				sysPrintf ("(%08d) WAIT DRQ FIFO16\n", __LINE__); \
			} \
		} while (0)
#else
#	define PRINT_DEBUG_FIFO16_REG(operation, value_low, value_high)
#	define PRINT_DEBUG_FIFO16_WAIT_DRQ
#endif



void wbhicFIFO_CalcBlocks (WBHIC_FIFO_T *pwbhicFIFO,
						   UINT32 uAdd,
						   UINT32 *puBlock_Before,
						   UINT32 *puBlocks,
						   UINT32 *puBlock_After);



/****************************************************************************
 *
 * FUNCTION
 *		wbhicPartReadFIFO_Begin
 *
 * DESCRIPTION
 *		Begin to read W99702 FIFO step by step.
 *
 * INPUTS
 *		uSizeTotal: Total buffer size.
 *
 * OUTPUTS
 *		pwbhicFIFO: The read handle.
 *
 * RETURN
 *		None
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
VOID wbhicPartReadFIFO_Begin(WBHIC_FIFO_T *pwbhicFIFO,
							 UINT32 uSizeTotal)
{
	pwbhicFIFO->uSizeTotal = uSizeTotal + (uSizeTotal & 0x01);
	pwbhicFIFO->uSizeOK = 0;
}



/****************************************************************************
 *
 * FUNCTION
 *		wbhicPartReadFIFO_End
 *
 * DESCRIPTION
 *		Finish to read W99702 FIFO and close the handle.
 *
 * INPUTS
 *		pwbhicFIFO: The read handle.
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
BOOL wbhicPartReadFIFO_End(WBHIC_FIFO_T *pwbhicFIFO)
{
	UCHAR uc;
	//Read the FIFO data if some data has not been read out.
	while (pwbhicFIFO->uSizeOK < pwbhicFIFO->uSizeTotal)
	{
		if (!wbhicPartReadFIFO (pwbhicFIFO,
								&uc,
								sizeof(uc),
								NULL))
			return FALSE;
	}
	return TRUE;
}



/****************************************************************************
 *
 * FUNCTION
 *		wbhicPartReadFIFO
 *
 * DESCRIPTION
 *		Partly read data from W99702 FIFO in 16-bit mode.
 *
 * INPUTS
 *		pwbhicFIFO: The read handle.
 *		pucBuffer: Buffer address.
 *		uBufferSize: Buffer size.
 *
 * OUTPUTS
 *		puSizeRead: The size successfully read.
 *
 * RETURN
 *		TRUE, success.
 *		FALSE, failed.
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
BOOL wbhicPartReadFIFO (WBHIC_FIFO_T *pwbhicFIFO,
						UCHAR *pucBuffer,
						UINT32 uBufferSize,
						UINT32 *puSizeRead)
{
	UINT32 uBlock_Before;	//Bytes before full "SIZE_FIFO" blocks.
	UINT32 uBlocks;			//Number of full "SIZE_FIFO" blocks.
	UINT32 uBlock_After;	//Bytes after full "SIZE_FIFO" blocks.
	UINT32 uBlock_Before2;
	UINT32 uBlock_After2;
	UINT32 uSizeRead;
	UINT32 i;
	BOOL bOK = TRUE;
	UCHAR *puc = pucBuffer;

	wbhicFIFO_CalcBlocks (pwbhicFIFO, uBufferSize, &uBlock_Before, &uBlocks, &uBlock_After);

	//Pack a byte if there's one byte not read in previous call of "wbhicPartReadFIFO_??".
	if ((pwbhicFIFO->uSizeOK & 1) != 0 && uBlock_Before > 0)
	{
		*(puc++) = (UCHAR) (pwbhicFIFO->usHalf >> 8);
		uBlock_Before--;
	}

	//Read bytes before writing full "SIZE_FIFO" blocks.
	uBlock_Before2 = (uBlock_Before & (UINT32) -2);
	for (i = uBlock_Before2 / 2; i > 0; i--)
	{
		USHORT_2_UCHAR (CF_REG8_W, puc);
		PRINT_DEBUG_FIFO16_REG ("RD", *puc, *(puc + 1));
		puc += 2;
	}

	if (uBlock_Before2 != uBlock_Before)
	{//Save the last character for next call of "wbhicPartReadFIFO_??".
		pwbhicFIFO->usHalf = CF_REG8_W;
		PRINT_DEBUG_FIFO16_REG ("RD", ((UCHAR) pwbhicFIFO->usHalf), (pwbhicFIFO->usHalf >> 8));
		*(puc++) = (UCHAR) pwbhicFIFO->usHalf;
	}


#ifndef DISABLE_GDMA
	{
		WBHIC_DMA_T DMA;
		DMA.pwbhicFIFO	= pwbhicFIFO;
		DMA.uAddress	= (UINT32) puc;
		DMA.uBlocks		= uBlocks;
		sysInitWaitObj (&DMA.wait);
		
		wbhicPartReadFIFO_StartDMA (&DMA);
		sysWait (&DMA.wait, 60000);
		puc = (UCHAR *) DMA.uAddress;
	}
#else
	//Read full "SIZE_FIFO" blocks.
	for (i = uBlocks; i > 0; i--)
	{
		UINT32 j;
		PRINT_DEBUG_FIFO16_WAIT_DRQ;
		WB_COND_WAITFOR (((CF_STATUS & HICST_DRQ) == HICST_DRQ), CF_TIMEOUT_MSEC, bOK);
		PRINT_DEBUG_FIFO16_REG ("ST", CF_STATUS, 0);
		if (!bOK)
		{
			wbSetLastError (HIC_ERR_ST_NO_DRQ);
			break;
		}

		for (j = SIZE_FIFO; j > 0; j -= 2)
		{
			USHORT_2_UCHAR (CF_REG8_W, puc);
			PRINT_DEBUG_FIFO16_REG ("RD", *puc, *(puc + 1));
			puc += 2;
		}
	}
#endif

	//Read blocks after full "SIZE_FIFO" blocks.
	if (uBlock_After > 0 && bOK)
	{
		PRINT_DEBUG_FIFO16_WAIT_DRQ;
		WB_COND_WAITFOR (((CF_STATUS & HICST_DRQ) == HICST_DRQ), CF_TIMEOUT_MSEC, bOK);
		PRINT_DEBUG_FIFO16_REG ("ST", CF_STATUS, 0);

		if (bOK)
		{
			uBlock_After2 = (uBlock_After & (UINT32) -2);
			for (i = uBlock_After2; i > 0; i -= 2)
			{
				USHORT_2_UCHAR (CF_REG8_W, puc);
				PRINT_DEBUG_FIFO16_REG ("RD", *puc, *(puc + 1));
				puc += 2;
			}
			if (uBlock_After2 != uBlock_After)
			{
				pwbhicFIFO->usHalf = CF_REG8_W;
				PRINT_DEBUG_FIFO16_REG ("RD", ((UCHAR) pwbhicFIFO->usHalf), (pwbhicFIFO->usHalf >> 8));
				*(puc++) = (UCHAR) pwbhicFIFO->usHalf;
			}
		}
		else
		{
			wbSetLastError (HIC_ERR_ST_NO_DRQ);
		}
	}

	uSizeRead = puc - pucBuffer;
	pwbhicFIFO->uSizeOK += uSizeRead;

	if (puSizeRead != NULL)
		*puSizeRead = uSizeRead;

	return bOK;
}




/****************************************************************************
 *
 * FUNCTION
 *		wbhicPartWriteFIFO_Begin
 *
 * DESCRIPTION
 *		Begin to write W99702 FIFO step by step.
 *
 * INPUTS
 *		uSizeTotal: Total buffer size.
 *
 * OUTPUTS
 *		pwbhicFIFO: The written handle.
 *
 * RETURN
 *		None
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
VOID wbhicPartWriteFIFO_Begin(WBHIC_FIFO_T *pwbhicFIFO,
							  UINT32 uSizeTotal)
{
	pwbhicFIFO->uSizeTotal = uSizeTotal + (uSizeTotal & 0x01);
	pwbhicFIFO->uSizeOK = 0;
}


/****************************************************************************
 *
 * FUNCTION
 *		wbhicPartWriteFIFO_End
 *
 * DESCRIPTION
 *		Finish to write W99702 FIFO and close the handle.
 *
 * INPUTS
 *		pwbhicFIFO: The written handle.
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
BOOL wbhicPartWriteFIFO_End(WBHIC_FIFO_T *pwbhicFIFO)
{
	UCHAR uc = 0x00;
	//Write the FIFO data if some data has not been written.
	while (pwbhicFIFO->uSizeOK < pwbhicFIFO->uSizeTotal)
	{
		if (!wbhicPartWriteFIFO (pwbhicFIFO,
								 &uc,
								 sizeof(uc),
								 NULL))
			return FALSE;
	}

	if ((pwbhicFIFO->uSizeOK & 1) != 0)
	{//Write the last odd index char.
		if (!wbhicPartWriteFIFO (pwbhicFIFO,
								 &uc,
								 sizeof(uc),
								 NULL))
			return FALSE;
	}

	return TRUE;
}


/****************************************************************************
 *
 * FUNCTION
 *		wbhicPartWriteFIFO
 *
 * DESCRIPTION
 *		Partly write data to W99702 FIFO in 16-bit mode.
 *
 * INPUTS
 *		pwbhicFIFO: The written handle.
 *		pcucBuffer: Buffer address.
 *		uBufferSize: Buffer size.
 *
 * OUTPUTS
 *		puSizeWritten: Successfully write size.
 *
 * RETURN
 *		TRUE, success.
 *		FALSE, failed.
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
BOOL wbhicPartWriteFIFO (WBHIC_FIFO_T *pwbhicFIFO,
						 CONST UCHAR *pcucBuffer,
						 UINT32 uBufferSize,
								UINT32 *puSizeWritten)
{
	UINT32 uBlock_Before;	//Bytes before full "SIZE_FIFO" blocks.
	UINT32 uBlocks;			//Number of full "SIZE_FIFO" blocks.
	UINT32 uBlock_After;	//Bytes after full "SIZE_FIFO" blocks.
	UINT32 uBlock_Before2;
	UINT32 uBlock_After2;
	UINT32 uSizeWrite;
	UINT32 i;
	BOOL bOK = TRUE;
	CONST UCHAR *pcuc = pcucBuffer;

	wbhicFIFO_CalcBlocks (pwbhicFIFO, uBufferSize, &uBlock_Before, &uBlocks, &uBlock_After);

	//Pack a byte if there's one byte not written in previous call of "wbhicPartWriteFIFO_??".
	if ((pwbhicFIFO->uSizeOK & 1) != 0 && uBlock_Before > 0)
	{
		pwbhicFIFO->usHalf = (pwbhicFIFO->usHalf & (USHORT) 0x00FF)
			| (((USHORT) *(pcuc++)) << 8);
		CF_REG8_W = pwbhicFIFO->usHalf;
		PRINT_DEBUG_FIFO16_REG ("WR", ((UCHAR) pwbhicFIFO->usHalf), (pwbhicFIFO->usHalf >> 8));
		uBlock_Before--;
	}

	//Write bytes before writing full "SIZE_FIFO" blocks.
	uBlock_Before2 = (uBlock_Before & (UINT32) -2);
	for (i = uBlock_Before2; i > 0; i -= 2)
	{
		UCHAR_2_USHORT (pcuc, CF_REG8_W);
		PRINT_DEBUG_FIFO16_REG ("WR", *pcuc, *(pcuc + 1));
		pcuc += 2;
	}


	if (uBlock_Before2 != uBlock_Before)
	{//Save the last character for next call of "wbhicPartWriteFIFO_??".
		pwbhicFIFO->usHalf = (USHORT) *(pcuc++);
	}


#ifndef DISABLE_GDMA
	{
		WBHIC_DMA_T DMA;
		DMA.pwbhicFIFO	= pwbhicFIFO;
		DMA.uAddress	= (UINT32) pcuc;
		DMA.uBlocks		= uBlocks;
		sysInitWaitObj (&DMA.wait);
		
		wbhicPartWriteFIFO_StartDMA (&DMA);
		sysWait (&DMA.wait, 60000);
		pcuc = (CONST UCHAR *) DMA.uAddress;
	}
#else
	//Write full "SIZE_FIFO" blocks.
	for (i = uBlocks; i > 0; i--)
	{
		UINT32 j;
		PRINT_DEBUG_FIFO16_WAIT_DRQ;
		WB_COND_WAITFOR (((CF_STATUS & HICST_DRQ) == HICST_DRQ), CF_TIMEOUT_MSEC, bOK);
		PRINT_DEBUG_FIFO16_REG ("ST", CF_STATUS, 0);
		if (!bOK)
		{
			wbSetLastError (HIC_ERR_ST_NO_DRQ);
			break;
		}

		for (j = SIZE_FIFO; j > 0; j -= 2)
		{
			UCHAR_2_USHORT (pcuc, CF_REG8_W);
			PRINT_DEBUG_FIFO16_REG ("WR", *pcuc, *(pcuc + 1));
			pcuc += 2;
		}
	}
#endif

	//Write blocks after full "SIZE_FIFO" blocks.
	if (uBlock_After > 0 && bOK)
	{
		PRINT_DEBUG_FIFO16_WAIT_DRQ;
		WB_COND_WAITFOR (((CF_STATUS & HICST_DRQ) == HICST_DRQ), CF_TIMEOUT_MSEC, bOK);
		PRINT_DEBUG_FIFO16_REG ("ST", CF_STATUS, 0);

		if (bOK)
		{
			uBlock_After2 = (uBlock_After & (UINT32) -2);
			for (i = uBlock_After2; i > 0; i -= 2)
			{
				UCHAR_2_USHORT (pcuc, CF_REG8_W);
				PRINT_DEBUG_FIFO16_REG ("WR", *pcuc, *(pcuc + 1));
				pcuc += 2;
			}
			if (uBlock_After2 != uBlock_After)
			{
				pwbhicFIFO->usHalf = (USHORT) *(pcuc++);
			}
		}
		else
		{
			wbSetLastError (HIC_ERR_ST_NO_DRQ);
		}
		
	}

	uSizeWrite = pcuc - pcucBuffer;
	pwbhicFIFO->uSizeOK += uSizeWrite;

	if (puSizeWritten != NULL)
		*puSizeWritten = uSizeWrite;

	return bOK;
}



#endif

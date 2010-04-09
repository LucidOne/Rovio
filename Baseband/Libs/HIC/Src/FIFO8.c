/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2005 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/

/****************************************************************************
 *
 * FILENAME
 *		$RCSfile: FIFO8.c,v $
 *
 * VERSION
 *		$Revision: 1.2 $
 *
 * DESCRIPTION
 *		W99702 hardware control interface: FIFO read/write.
 *
 * HISTORY
 *     $Log: FIFO8.c,v $
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
 *     Revision 1.1.2.8  2005/07/04 11:04:36  xhchen
 *     FullDemo:
 *     	Add image effect.
 *     JPEG -> BMP:
 *     	Fix a bug in calculating BMP line length.
 *     	Use RGB565 instread YUV422 when converting JPG.
 *     	Add parameters to set a resolution for target BMP.
 *     OSD:
 *     	"Burst writing" for OSD is OK now. In CmpAPI.c
 *     	#define BURST_WRITE_OSD		//Write OSD with burst write
 *     	#undef BURST_WRITE_OSD		//Write OSD with command
 *     JPEG capture / Burst capture:
 *     	Add the code for reading JPEG to buffer and playback JPEG in buffer.
 *     	See Samples/JPEGCapture/Src/main.c
 *     		Samples/BurstCapture/Src/main.c
 *     LCM Bypass:
 *     	Change the date source to RGB565 to demonstrate bypass 16-bit to 18-bit by hardware.
 *
 *     Revision 1.1.2.7  2005/06/29 10:12:10  xhchen
 *     ...
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
 *     Revision 1.1.2.4  2005/04/01 02:14:58  xhchen
 *     More effective FIFO I/O OK.
 *
 *     Revision 1.1.2.3  2005/03/31 12:35:37  xhchen
 *     Some FIFO I/O method added for comparing performance.
 *
 *     Revision 1.1.2.2  2005/03/31 04:12:28  xhchen
 *     Speed up FIFO I/O.
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


#if defined(_HIC_MODE_A8_) || defined(_HIC_MODE_A9_) || defined(_HIC_MODE_I8_) || defined(_HIC_MODE_I9_)


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
			1, SIZE_FIFO,
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
			1, SIZE_FIFO,
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
#	define PRINT_DEBUG_FIFO8_REG(operation, value) \
		do \
		{ \
			if (g_iHIC_DEBUG_FIFO) \
			{ \
				sysPrintf ("(%08d) %s FIFO8: %02x\n", __LINE__, operation, (int) (value)); \
			} \
		} while (0)
#	define PRINT_DEBUG_FIFO8_WAIT_DRQ \
		do \
		{ \
			if (g_iHIC_DEBUG_FIFO) \
			{ \
				sysPrintf ("(%08d) WAIT DRQ FIFO8\n", __LINE__); \
			} \
		} while (0)
#else
#	define PRINT_DEBUG_FIFO8_REG(operation, value)
#	define PRINT_DEBUG_FIFO8_WAIT_DRQ
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
	pwbhicFIFO->uSizeTotal = uSizeTotal;
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
 *		Partly read data from W99702 FIFO in 8-bit mode.
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
	UINT32 uSizeRead;
	UINT32 i;
	BOOL bOK = TRUE;
	UCHAR *puc = pucBuffer;

	wbhicFIFO_CalcBlocks (pwbhicFIFO, uBufferSize, &uBlock_Before, &uBlocks, &uBlock_After);

	//Read bytes before writing full "SIZE_FIFO" blocks.
	for (i = uBlock_Before; i > 0; i--)
	{
		*(puc++) = CF_REG8;
		PRINT_DEBUG_FIFO8_REG ("RD", *(puc - 1));
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
		PRINT_DEBUG_FIFO8_WAIT_DRQ;
		WB_COND_WAITFOR (((CF_STATUS & HICST_DRQ) == HICST_DRQ), CF_TIMEOUT_MSEC, bOK);
		PRINT_DEBUG_FIFO8_REG ("ST", CF_STATUS);
		if (!bOK)
		{
			wbSetLastError (HIC_ERR_ST_NO_DRQ);
			break;
		}

		for (j = SIZE_FIFO; j > 0; j--)
		{
			*(puc++) = CF_REG8;
			PRINT_DEBUG_FIFO8_REG ("RD", *(puc - 1));
		}
	}
#endif

	//Read blocks after full "SIZE_FIFO" blocks.
	if (uBlock_After > 0 && bOK)
	{
		PRINT_DEBUG_FIFO8_WAIT_DRQ;
		WB_COND_WAITFOR (((CF_STATUS & HICST_DRQ) == HICST_DRQ), CF_TIMEOUT_MSEC, bOK);
		PRINT_DEBUG_FIFO8_REG ("ST", CF_STATUS);

		if (bOK)
		{
			for (i = uBlock_After; i > 0; i--)
			{
				*(puc++) = CF_REG8;
				PRINT_DEBUG_FIFO8_REG ("RD", *(puc - 1));
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
	pwbhicFIFO->uSizeTotal = uSizeTotal;
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

	return TRUE;
}



/****************************************************************************
 *
 * FUNCTION
 *		wbhicPartWriteFIFO
 *
 * DESCRIPTION
 *		Partly write data to W99702 FIFO in 8-bit mode.
 *
 * INPUTS
 *		pwbhicFIFO: The written handle.
 *		pucBuffer: Buffer address.
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
	UINT32 uSizeWrite;
	UINT32 i;
	BOOL bOK = TRUE;
	CONST UCHAR *pcuc = pcucBuffer;

	wbhicFIFO_CalcBlocks (pwbhicFIFO, uBufferSize, &uBlock_Before, &uBlocks, &uBlock_After);


	//Write bytes before writing full "SIZE_FIFO" blocks.
	for (i = uBlock_Before; i > 0; i--)
	{
		CF_REG8 = *(pcuc++);
		PRINT_DEBUG_FIFO8_REG ("WR", *(pcuc - 1));
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
		PRINT_DEBUG_FIFO8_WAIT_DRQ;
		WB_COND_WAITFOR (((CF_STATUS & HICST_DRQ) == HICST_DRQ), CF_TIMEOUT_MSEC, bOK);
		PRINT_DEBUG_FIFO8_REG ("ST", CF_STATUS);
		if (!bOK)
		{
			wbSetLastError (HIC_ERR_ST_NO_DRQ);
			break;
		}

		for (j = SIZE_FIFO; j > 0; j--)
		{
			CF_REG8 = *(pcuc++);
			PRINT_DEBUG_FIFO8_REG ("WR", *(pcuc - 1));
		}
	}
#endif

	//Write blocks after full "SIZE_FIFO" blocks.
	if (uBlock_After > 0 && bOK)
	{
		PRINT_DEBUG_FIFO8_WAIT_DRQ;
		WB_COND_WAITFOR (((CF_STATUS & HICST_DRQ) == HICST_DRQ), CF_TIMEOUT_MSEC, bOK);
		PRINT_DEBUG_FIFO8_REG ("ST", CF_STATUS);

		if (bOK)
		{
			for (i = uBlock_After; i > 0; i--)
			{
				CF_REG8 = *(pcuc++);
				PRINT_DEBUG_FIFO8_REG ("WR", *(pcuc - 1));
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

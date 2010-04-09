
/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2005 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/

/****************************************************************************
 *
 * FILENAME
 *		$RCSfile: FIFO.c,v $
 *
 * VERSION
 *		$Revision: 1.4 $
 *
 * DESCRIPTION
 *		W99702 hardware control interface: FIFO read/write.
 *
 * HISTORY
 *     $Log: FIFO.c,v $
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
 *     Revision 1.1.2.3  2005/08/25 05:41:14  xhchen
 *     Add Virtual com to FullDemo.
 *     Begin to add FlowTest, a testing application almost for all functions.
 *
 *     Revision 1.1.2.2  2005/07/20 09:18:58  xhchen
 *     1. soft pipe: remove fnTrData, use fnOnData only.
 *     2. Movie record: check status after record stopped,
 *        add parameter to set volume, bitrate and size when recording.
 *        Fix a bug when recording movie: OSD did not properly been shown.
 *
 *     Revision 1.1.2.1  2005/07/15 08:20:07  xhchen
 *     Use PIPE to transfer FIFO instead of callback functions.
 *
 *
 * REMARK
 *     None
 *
 **************************************************************************/



#include "../../Platform/Inc/Platform.h"
#include "../../SoftPipe/include/softpipe.h"
#include "../Inc/HIC.h"





void wbhicFIFO_CalcBlocks (WBHIC_FIFO_T *pwbhicFIFO,
						   UINT32 uAdd,
						   UINT32 *puBlock_Before,
						   UINT32 *puBlocks,
						   UINT32 *puBlock_After)
{
	UINT32 uToAdd;
	UINT32 uBlock_First;

	/* if (uSizeTotal == 0), we consider as no total size limitation.*/
	if (pwbhicFIFO->uSizeTotal == 0)
		uToAdd = uAdd;
	else
	{
		uToAdd = pwbhicFIFO->uSizeTotal	- pwbhicFIFO->uSizeOK;
		if (uToAdd > uAdd)
			uToAdd = uAdd;
	}


	uBlock_First = (SIZE_FIFO - pwbhicFIFO->uSizeOK) & (SIZE_FIFO - 1);
	if (uBlock_First >= uToAdd)
	{
		*puBlock_Before = uToAdd;
		*puBlocks = 0;
		*puBlock_After = 0;
	}
	else
	{
		*puBlock_Before = uBlock_First;
		uToAdd -= uBlock_First;
		*puBlocks = uToAdd / SIZE_FIFO;
		*puBlock_After = uToAdd - *puBlocks * SIZE_FIFO;
	}
}



/****************************************************************************
 *
 * FUNCTION
 *		wbhicWriteFIFO
 *
 * DESCRIPTION
 *		Write data to W99702 FIFO.
 *
 * INPUTS
 *		pSourceBuffer: If "uBufferType" is "WBHIC_FUNC_IO", it points to a
 *                     structure WBHIC_FIFO_FUNCSRC_PARAM_T, otherwise it's
 *                     a buffer.
 *		uBufferType: The input buffer's type. If its value is not
 *                   "WBHIC_FUNC_IO", it's used as the buffer length.
 *                   (for compatibility)
 *
 * OUTPUTS
 *		puSizeWritten: Successfully written size.
 *
 * RETURN
 *		TRUE, success.
 *		FALSE, failed.
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
BOOL wbhicWriteFIFO (VOID *pSourceBuffer,
					 UINT32 uBufferType,
					 UINT32 *puSizeWritten)
{
	WBHIC_FIFO_PIPE_T wbhicFIFO_PIPE;
	UINT32 uBufferLen;
	BOOL bRt;

	if (uBufferType == WBHIC_FIFO_PIPE_IO)
	{
		WBHIC_FIXLEN_PIPE_T *pPipeSrc = (WBHIC_FIXLEN_PIPE_T *) pSourceBuffer;
		uBufferLen = pPipeSrc->uLength;
	}
	else
		uBufferLen = uBufferType;

	wbhicPipeWriteFIFO_Init (&wbhicFIFO_PIPE, uBufferLen);

	if (uBufferType == WBHIC_FIFO_PIPE_IO)
	{
		WBHIC_FIXLEN_PIPE_T *pPipeSrc = (WBHIC_FIXLEN_PIPE_T *) pSourceBuffer;
		spConnect (&pPipeSrc->pipe, &wbhicFIFO_PIPE.pipe);
		spTransfer (&pPipeSrc->pipe);
		bRt = (spGetLenRecv (&wbhicFIFO_PIPE.pipe) == pPipeSrc->uLength ? TRUE : FALSE);
	}
	else
	{
		CONST UCHAR *pcucBuffer = (CONST UCHAR *) pSourceBuffer;
		bRt = wbhicPartWriteFIFO (&wbhicFIFO_PIPE.wbhicFIFO,
								  pcucBuffer,
								  uBufferLen,
								  puSizeWritten);
	}
	if (!bRt)
		return FALSE;

	wbhicPipeWriteFIFO_Uninit (&wbhicFIFO_PIPE);

	return TRUE;
}



/****************************************************************************
 *
 * FUNCTION
 *		wbhicReadFIFO
 *
 * DESCRIPTION
 *		Read data from W99702 FIFO.
 *
 * INPUTS
 *		pTargetBuffer: If "uBufferType" is "WBHIC_FUNC_IO", it points to a
 *                     structure WBHIC_FIFO_FUNCTGT_PARAM_T, otherwise it's
 *                     a buffer.
 *		uBufferType: The input buffer's type. If its value is not
 *                   "WBHIC_FUNC_IO", it's used as the buffer length.
 *                   (for compatibility)
 *
 * OUTPUTS
 *		puSizeRead: The size successfully read.
 *
 * RETURN
 *		TRUE, success.
 *		FALSE, failed.
 *
 * REMARK
 *		To call "wbhicReadFIFO(buffer, length, NULL)"
 *		is equal to:
 *			WBHIC_FIFO_T wbhicFIFO;
 *			wbhicPartReadFIFO_Begin(&wbhicFIFO, length);
 *			wbhicPartReadFIFO(&wbhicFIFO, buffer, length, NULL);
 *			wbhicPartReadFIFO_End(&wbhicFIFO);
 *
 ***************************************************************************/
BOOL wbhicReadFIFO (VOID *pTargetBuffer,
					UINT32 uBufferType,
					UINT32 *puSizeRead)
{
	WBHIC_FIFO_PIPE_T wbhicFIFO_PIPE;
	UINT32 uBufferLen;
	BOOL bRt;

	if (uBufferType == WBHIC_FIFO_PIPE_IO)
	{
		WBHIC_FIXLEN_PIPE_T *pPipeTgt = (WBHIC_FIXLEN_PIPE_T *) pTargetBuffer;
		uBufferLen = pPipeTgt->uLength;
	}
	else
		uBufferLen = uBufferType;


	wbhicPipeReadFIFO_Init (&wbhicFIFO_PIPE, uBufferLen);

	if (uBufferType == WBHIC_FIFO_PIPE_IO)
	{
		WBHIC_FIXLEN_PIPE_T *pPipeTgt = (WBHIC_FIXLEN_PIPE_T *) pTargetBuffer;
		wbhicPipeReadFIFO_SetLength (&wbhicFIFO_PIPE, uBufferLen);
		spConnect (&wbhicFIFO_PIPE.pipe, &pPipeTgt->pipe);
		spTransfer (&pPipeTgt->pipe);
		bRt = (spGetLenSent (&wbhicFIFO_PIPE.pipe) == uBufferLen ? TRUE : FALSE);
	}
	else
	{
		UCHAR *pucBuffer = (UCHAR *) pTargetBuffer;

		bRt = wbhicPartReadFIFO (&wbhicFIFO_PIPE.wbhicFIFO,
								 pucBuffer,
								 uBufferLen,
								 puSizeRead);
	}
	if (!bRt)
		return FALSE;

	wbhicPipeReadFIFO_Uninit (&wbhicFIFO_PIPE);

	return TRUE;
}





static unsigned int
wbhicFIFOWrite_PipeOnData (SOFT_PIPE_T *pspThis, const void *pData, unsigned int uLen)
{
	WBHIC_FIFO_PIPE_T *pPipe = GetParentAddr (pspThis, WBHIC_FIFO_PIPE_T, pipe);
	UINT32 uSizeWritten;
	wbhicPartWriteFIFO (&pPipe->wbhicFIFO,
						(CONST UCHAR *) pData,
						(UINT32) uLen,
						&uSizeWritten);
	return uSizeWritten;
}

VOID wbhicPipeWriteFIFO_Init (WBHIC_FIFO_PIPE_T *pPipe, UINT32 uSizeTotal)
{
	spInit (&pPipe->pipe);
	pPipe->pipe.fnOnData = wbhicFIFOWrite_PipeOnData;
	wbhicPartWriteFIFO_Begin (&pPipe->wbhicFIFO, uSizeTotal);
}

VOID wbhicPipeWriteFIFO_Uninit (WBHIC_FIFO_PIPE_T *pPipe)
{
	wbhicPartWriteFIFO_End (&pPipe->wbhicFIFO);
}


static unsigned int
wbhicFIFORead_PipeOnData (SOFT_PIPE_T *pspThis, const void *pData, unsigned int uLen)
{
	WBHIC_FIFO_PIPE_T *pPipe = GetParentAddr (pspThis, WBHIC_FIFO_PIPE_T, pipe);
	UINT32 uLenOK;

	uLen = pPipe->uLength_ToRead;
	for (uLenOK = 0; uLenOK < uLen; )
	{
		UCHAR ac[32];
		UINT32 uLenRead;
		UINT32 uLenThis = uLen - uLenOK;
		if (uLenThis > sizeof (ac))
			uLenThis = sizeof (ac);

		if (!wbhicPartReadFIFO (&pPipe->wbhicFIFO,
								ac,
								(UINT32) uLenThis,
								&uLenRead))
			break;

		spSend (pspThis, (const void *) ac, uLenRead);
		uLenOK += uLenRead;
	}
	return uLenOK;
}

VOID wbhicPipeReadFIFO_Init (WBHIC_FIFO_PIPE_T *pPipe, UINT32 uSizeTotal)
{
	spInit (&pPipe->pipe);
	pPipe->pipe.fnOnData	= wbhicFIFORead_PipeOnData;
	wbhicPartReadFIFO_Begin (&pPipe->wbhicFIFO, uSizeTotal);
}

VOID wbhicPipeReadFIFO_Uninit (WBHIC_FIFO_PIPE_T *pPipe)
{
	wbhicPartReadFIFO_End (&pPipe->wbhicFIFO);
}

VOID wbhicPipeReadFIFO_SetLength (WBHIC_FIFO_PIPE_T *pPipe, UINT32 uLength)
{
	pPipe->uLength_ToRead = uLength;
}

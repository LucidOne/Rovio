/****************************************************************************
 *                                                                                    
 * Copyright (c) 2006 - 2006 Winbond Electronics Corp. All rights reserved.           
 *                                                                                    
 ***************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *     jpeglib.c
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     Encode a YUV raw data into the JPEG bitstream.
 *     Decode a jpeg bitstream into the YUV raw data.
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *
 *
 * HISTORY
 *     16/10/06		 Ver 1.0 Created by NS25 Jzhou0
 *
 * REMARK
 *     None
 **************************************************************************/

#ifndef ECOS
#include <stdio.h>
#include <string.h>
#include <time.h>
#else
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "wb_syslib_addon.h"
#endif
#include "w99702_reg.h"
#include "jpeglib.h"

volatile UINT32 _jpeg_EncodeOk				= FALSE;
volatile UINT32 _jpeg_DecodeOk				= FALSE;
volatile UINT32 _jpeg_DecodeError			= FALSE;
volatile UINT32 _jpeg_EncodeOnTheFlyError	= FALSE;

static UINT32 (*_jpeg_EncodeCompleteInterruptCallbackFunction)(); /* A pointer to a callback function called by ISR */
static UINT32 (*_jpeg_DecodeCompleteInterruptCallbackFunction)(); /* A pointer to a callback function called by ISR */
static UINT32 (*_jpeg_DecodeErrorInterruptCallbackFunction)(); /* A pointer to a callback function called by ISR */
static UINT32 (*_jpeg_EncodeOnTheFlyErrorCallbackFunction)(); /* A pointer to a callback function called by ISR */
static UINT32 (*_jpeg_EncodeSWOnTheFlyWaitCallbackFunction)(); /* A pointer to a callback function called by ISR */

/* Install one of four types of call-back function */
void jpegSetIRQHandler(JPEG_INSTALL_CALL_BACK_FUNCTION_E eCallBackFunctionType, UINT32 (*puCallbackfunction)())
{
	switch(eCallBackFunctionType)
	{
	case C_JPEG_CALLBACK_ENCODE_COMPLETE_INTERRUPT:
		_jpeg_EncodeCompleteInterruptCallbackFunction = puCallbackfunction;
		break;
	case C_JPEG_CALLBACK_DECODE_COMPLETE_INTERRUPT:
		_jpeg_DecodeCompleteInterruptCallbackFunction = puCallbackfunction;
		break;
	case C_JPEG_CALLBACK_DECODE_ERROR_INTERRUPT:
		_jpeg_DecodeErrorInterruptCallbackFunction = puCallbackfunction;
		break;
	case C_JPEG_CALLBACK_ENCODE_ONTHEFLY_ERROR_INTERRUPT:
		_jpeg_EncodeOnTheFlyErrorCallbackFunction = puCallbackfunction;
		break;
	case C_JPEG_CALLBACK_ENCODE_SWONTHEFLY_WAIT_INTERRUPT:
		_jpeg_EncodeSWOnTheFlyWaitCallbackFunction = puCallbackfunction;
		break;
	}
}

/* Interrupt Service Routine for JPEG CODEC */
#ifdef ECOS
UINT32 g_IntrStatus;
#endif

#ifndef ECOS
void jpegIntHandler(void)
#else
cyg_uint32 jpegIntHandler(cyg_vector_t vector, cyg_addrword_t data)
#endif
{
	UINT32 interruptStatus = inpw(REG_JINTCR);
#ifdef ECOS
	cyg_interrupt_mask(vector);
	
	g_IntrStatus = interruptStatus;
	
#endif

	/* It's Encode Complete Interrupt */
	if(interruptStatus & 0x00000008)
	{
		_jpeg_EncodeOk = TRUE;

#ifndef ECOS
		/* Run JPEG Encode Complete Interrupt Call-back function */
		if(_jpeg_EncodeCompleteInterruptCallbackFunction)
			(*_jpeg_EncodeCompleteInterruptCallbackFunction)();
#endif

		/* Write one to clear this interrupt status */
		outpw(REG_JINTCR, (interruptStatus & ~0x11) | 0x00000008);

		/* Clear Memory On-the-Fly Access For Encode bit */
		outpw(REG_JMACR, inpw(REG_JMACR) & ~0x80);
	}
	/* It's Decode Complete Interrupt */
	else if(interruptStatus & 0x00000004)
	{
		_jpeg_DecodeOk = TRUE;

#ifndef ECOS
		/* Run JPEG Decode Complete Interrupt Call-back function */
		if(_jpeg_DecodeCompleteInterruptCallbackFunction)
			(*_jpeg_DecodeCompleteInterruptCallbackFunction)();
#endif

		/* Write one to clear this interrupt status */
		outpw(REG_JINTCR, interruptStatus |	0x00000004);
	}
	/* It's Decode Error Interrupt */
	else if(interruptStatus & 0x00000002)
	{
		_jpeg_DecodeError = TRUE;

#ifndef ECOS
		/* Run JPEG Decode Error Interrupt Call-back function */
		if(_jpeg_DecodeErrorInterruptCallbackFunction)
			(*_jpeg_DecodeErrorInterruptCallbackFunction)();
#endif

		/* Write one to clear this interrupt status */
		outpw(REG_JINTCR, interruptStatus |	0x00000002);
	}
	/* It's Encode (On-The-Fly) Error Interrupt */
	else if(interruptStatus & 0x00000001)
	{
		_jpeg_EncodeOnTheFlyError = TRUE;

#ifndef ECOS
		/* Run JPEG Encode (On-The-Fly) Error Interrupt Call-back function */
		if(_jpeg_EncodeOnTheFlyErrorCallbackFunction)
			(*_jpeg_EncodeOnTheFlyErrorCallbackFunction)();
#endif

		/* Write one to clear this interrupt status */
		outpw(REG_JINTCR, (interruptStatus & ~0x11) | 0x00000001);

		/* Clear Memory On-the-Fly Access For Encode bit */
		outpw(REG_JMACR, inpw(REG_JMACR) & ~0x80);
	}
	/* It's Encode (sofeware On-The-Fly) Wait Interrupt */
	else if(interruptStatus & 0x00000100)
	{
#ifndef ECOS
		/* Run JPEG Encode (sofeware On-The-Fly) Wait Interrupt Call-back function */
		if(_jpeg_EncodeSWOnTheFlyWaitCallbackFunction)
			(*_jpeg_EncodeSWOnTheFlyWaitCallbackFunction)();
#endif

		F_JPEG_RESUME;
		/* Write one to clear this interrupt status */
		outpw(REG_JINTCR, interruptStatus | 0x00000100);
	}
#ifdef ECOS
	cyg_interrupt_acknowledge(vector);
	return CYG_ISR_CALL_DSR;
#endif
}

#ifdef ECOS
void jpegIntHandler_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
	UINT32 interruptStatus = g_IntrStatus;
	
	if(interruptStatus & 0x00000008)
	{
		/* Run JPEG Encode Complete Interrupt Call-back function */
		if(_jpeg_EncodeCompleteInterruptCallbackFunction)
			(*_jpeg_EncodeCompleteInterruptCallbackFunction)();
	}
	/* It's Decode Complete Interrupt */
	else if(interruptStatus & 0x00000004)
	{
		/* Run JPEG Decode Complete Interrupt Call-back function */
		if(_jpeg_DecodeCompleteInterruptCallbackFunction)
			(*_jpeg_DecodeCompleteInterruptCallbackFunction)();
	}
	/* It's Decode Error Interrupt */
	else if(interruptStatus & 0x00000002)
	{
		/* Run JPEG Decode Error Interrupt Call-back function */
		if(_jpeg_DecodeErrorInterruptCallbackFunction)
			(*_jpeg_DecodeErrorInterruptCallbackFunction)();
	}
	/* It's Encode (On-The-Fly) Error Interrupt */
	else if(interruptStatus & 0x00000001)
	{
		/* Run JPEG Encode (On-The-Fly) Error Interrupt Call-back function */
		if(_jpeg_EncodeOnTheFlyErrorCallbackFunction)
			(*_jpeg_EncodeOnTheFlyErrorCallbackFunction)();
	}
	else if(interruptStatus & 0x00000100)
	{
		/* Run JPEG Encode (sofeware On-The-Fly) Wait Interrupt Call-back function */
		if(_jpeg_EncodeSWOnTheFlyWaitCallbackFunction)
			(*_jpeg_EncodeSWOnTheFlyWaitCallbackFunction)();
	}

	
	cyg_interrupt_acknowledge(vector);
	cyg_interrupt_unmask(vector);
}
#endif

/* Initialization for encoder or decoder */
void jpegInitializeEngine(void)
{
    /* Reset JPEG engine */
    F_JPEG_RESET;

	/* Set the global interrupt */
	//sysInstallISR(IRQ_LEVEL_1, IRQ_JPEG, jpegIntHandler);

	/* Enable the interrupt for JPEG engine */
#ifndef ECOS
	sysEnableInterrupt(IRQ_JPEG);
#else
	//cyg_interrupt_unmask(IRQ_JPEG);
#endif
}

void jpegSetMemoryAddress(JPEG_YUV_BUFFER_E eYUVBuffer, JPEG_BITSTREAM_BUFFER_E eBitstreamBuffer,
							 UINT32 uYStartAddress, UINT32 uUStartAddress, UINT32 uVStartAddress, 
							 UINT32 uJpegBitstreamStartAddress)
{
	if(eYUVBuffer == C_JPEG_YUV_BUFFER_0)
	{
		outpw(REG_JYADDR0, uYStartAddress);    
		outpw(REG_JUADDR0, uUStartAddress); 
		outpw(REG_JVADDR0, uVStartAddress);
	}
	else
	{
		outpw(REG_JYADDR1, uYStartAddress);    
		outpw(REG_JUADDR1, uUStartAddress); 
		outpw(REG_JVADDR1, uVStartAddress);
	}

	if(eBitstreamBuffer == C_JPEG_BITSTREAM_BUFFER_0)
		outpw(REG_JIOADDR0, uJpegBitstreamStartAddress);
	else
		outpw(REG_JIOADDR1, uJpegBitstreamStartAddress);    
}

UINT32 jpegEncSetBasicOption(JPEG_ENCODE_SETTING_T *tEncodeSetting)
{
	int regJMCR = 0, regJITCR = 0, regJHEADER = 0;
	JPEG_IMAGE_T *pImgSource = &(tEncodeSetting->imgSource);
	JPEG_IMAGE_T *pImgPrimary = &(tEncodeSetting->imgPrimary);

	if(pImgSource->eImageFormat != C_JPEG_YUV420 &&
	   pImgSource->eImageFormat != C_JPEG_YUV422 &&
	   pImgSource->eImageFormat != C_JPEG_GRAY)
	   return ERR_JPEG_ENC_IMGFORMAT;

	/* set encode mode */
	regJMCR = 1 << 7;

	/* set image format */
	if(pImgSource->eImageFormat != C_JPEG_GRAY)
		regJMCR |= pImgSource->eImageFormat << 3;
	else
		regJITCR |= 1 << 6;

	/* set QTAB and HTAB header format */
	regJITCR |= tEncodeSetting->eTablesHeaderFormat;
	/* set quantization table number */
	regJITCR |= tEncodeSetting->eQTabNumber << 3;

	if(tEncodeSetting->eEncodeObject == C_JPEG_ENC_OBJECT_BOTH)
	{
		JPEG_IMAGE_T *pImgThumbnail = &(tEncodeSetting->imgThumbnail);
		int uTWDSF = tEncodeSetting->uTWidthDScaleF, uTHDSF = tEncodeSetting->uTHeightDScaleF;

		/* Check the input parameters */
		if(uTWDSF > 64)
			return ERR_JPEG_ENC_TSCALEDOWN;
		if(uTHDSF > 64)
			return ERR_JPEG_ENC_TSCALEDOWN;
		if(tEncodeSetting->uTOffset % 4)
			return ERR_JPEG_ENC_TOFFSET;

		/* Enable encoding thumbnail image */
		regJMCR |= 1 << 4;

		/* set Thumbnail Image*/
		if(uTWDSF > 1 || uTHDSF > 1)
		{
			int regJTSCALD = 0, width = 0, height = 0;

			width = pImgSource->uImageWidth / uTWDSF;
			height = pImgSource->uImageHeight / uTHDSF;
			if(width < 32)
				return ERR_JPEG_ENC_TSCALEDOWN;
			if(height < 32)
				return ERR_JPEG_ENC_TSCALEDOWN;

			/* TSCALX_F = uWidthDownScaleFactor/2 - 1 and TSCALY_F = uHeightDownScaleFactor - 1 */
			if(uTWDSF > 1)
				regJTSCALD |= (0x00008000 | ((uTWDSF / 2 - 1) << 8));
			/* Do the vertical down-scaling */
			if(uTHDSF > 1)
				regJTSCALD |= (uTHDSF - 1);
			outpw(REG_JTSCALD, regJTSCALD);

			pImgThumbnail->uImageWidth = width;
			pImgThumbnail->uImageHeight = height;
		}
		else
		{
			pImgThumbnail->uImageWidth = pImgSource->uImageWidth;
			pImgThumbnail->uImageHeight = pImgSource->uImageHeight;
		}
		pImgThumbnail->eImageFormat = pImgSource->eImageFormat;
		pImgThumbnail->ImgAddress.JpegAddress.uStartAddress = 
						tEncodeSetting->uJpegBitstreamStartAddress + tEncodeSetting->uTOffset;

		/* Set thumbnail image width/height */
		outpw(REG_JTHBWH, (0xFFFF & pImgThumbnail->uImageHeight) << 16 | 
						  (0xFFFF & pImgThumbnail->uImageWidth));

		/* Set Primary/Thumbnail Starting Address Offset Size */
		outpw(REG_JOFFSET, tEncodeSetting->uTOffset);

		/* Primary JPEG Bit-stream Include Quantization-Table and Huffman-Table */
		regJHEADER |= tEncodeSetting->uTIncludeHeader;
		/* JPEG Encode Thumbanil Restart Interval */
		if(tEncodeSetting->uTIncludeHeader & C_JPEG_ENC_T_RINTERVAL_INCLUDE)
			outpw(REG_JTRST, 0xFFFF & tEncodeSetting->uTRestartInterval);
	}

	/* Enable encoding primary image */
	regJMCR |= 1 << 5;

	pImgPrimary->eImageFormat = pImgSource->eImageFormat;
	pImgPrimary->ImgAddress.JpegAddress.uStartAddress = tEncodeSetting->uJpegBitstreamStartAddress;
	pImgPrimary->uImageWidth = pImgSource->uImageWidth;
	pImgPrimary->uImageHeight = pImgSource->uImageHeight;
	outpw(REG_JPRIWH, (0xFFFF & pImgPrimary->uImageHeight) << 16 | 
					  (0xFFFF & pImgPrimary->uImageWidth));

	regJHEADER |= tEncodeSetting->uPIncludeHeader;
	if(tEncodeSetting->uPIncludeHeader & C_JPEG_ENC_P_RINTERVAL_INCLUDE)
		outpw(REG_JPRST, 0xFFFF & tEncodeSetting->uPRestartInterval);

	outpw(REG_JMCR, regJMCR);
	outpw(REG_JITCR, regJITCR);
	outpw(REG_JHEADER, regJHEADER);
	outpw(REG_JYSTRIDE, tEncodeSetting->uYStride);
	outpw(REG_JUSTRIDE, tEncodeSetting->uUStride);
	outpw(REG_JVSTRIDE, tEncodeSetting->uVStride);       

	return Successful;
}

/*
    Set memory address mode. H/W provides the following four modes:
    1. Still image encode mode, the encoded bit-stream is always placed into buffer-0
    2. Still image encode mode, the output dual-buffer is controlled by register REG_JDBCR[0]
    3. Continuous image encode mode, the encoded bit-stream is placed into the continuous memory address
    4. Continuous image encode mode, the offset of memory address for adjacent images is fixed
    	and is specified by JPEG Encode Bit-stream Frame Stride Register (F_STRIDE)
*/
void jpegEncSetMemoryAccessMode(JPEG_ENC_MEMORY_ACCESS_MODE_E eMemoryAccessMode, UINT32 uContinueModeSameSizeBufferSize)
{
	/* Set Memory Address Mode */
	outpw(REG_JMACR, (inpw(REG_JMACR) & ~0x3) | eMemoryAccessMode);

	if(eMemoryAccessMode == C_JPEG_CONTINUE_SAMESIZE)
		outpw(REG_JFSTRIDE, uContinueModeSameSizeBufferSize);
}

/* Set the two input and two output buffers.*/
void jpegEncSetDualBuffer(JPEG_YUV_BUFFER_E eInputBuffer, JPEG_BITSTREAM_BUFFER_E eOutputBuffer)
{
	/* Dual buffering enable, and Select Input Buffer 0 and Output Buffer 0 */
	outpw(REG_JDBCR, 0x00000080 | eInputBuffer << 4 | eOutputBuffer);      
}

/* Write Quantization-tables into the registers. */
void jpegEncWriteQTable(JPEG_QTAB_NUMBER_E eQTabNumber, UINT8 **ucQTable)
{
	int i, j, uQNumber, qTableAddress[] = {REG_JQTAB0, REG_JQTAB1, REG_JQTAB2};

	if(eQTabNumber == C_JPEG_TWO_QTAB)
		uQNumber = 2;
	else
		uQNumber = 3;

	/* Output the Quantization-table to the HW register */
	for(i=0; i<uQNumber; i++)
	{
		for(j=0; j<64; j+=4)
		{
			outpw(qTableAddress[i] + j, *((UINT32 *)(ucQTable[i] + j)));
			while(F_JPEG_IS_QT_BUSY);
		}
	}
}

/*
	Down-Scaling function for encoding procedure.
	uWidthDownScaleFactor = 1/2, 1/4, 1/6, ... , 1/64
	uHeightDownScaleFactor = 1/1, 1/2, 1/3, ... , 1/64
*/
UINT32 jpegEncDownScalePImage(JPEG_ENCODE_SETTING_T *tEncodeSetting, UINT32 uWidthDScaleF, UINT32 uHeightDScaleF)
{
	JPEG_IMAGE_T *pImgSource = &(tEncodeSetting->imgSource);
	JPEG_IMAGE_T *pImgPrimary = &(tEncodeSetting->imgPrimary);

	/* Check the input parameters */
	if(uWidthDScaleF > 64)
		return ERR_JPEG_ENC_PSCALEDOWN;

	if(uHeightDScaleF > 64)
		return ERR_JPEG_ENC_PSCALEDOWN;

	/* Enable the down-scaling function and set Primary Hoirzontal and Vertical Saling-Down Factor */
	if(uWidthDScaleF > 1 || uHeightDScaleF > 1)
	{
		int regJPSCALD = 0, Width = 0, Height = 0;

		/* Get the width and height of primary image after down-scaling */
		Width = pImgSource->uImageWidth / uWidthDScaleF;
		Height = pImgSource->uImageHeight / uHeightDScaleF;
		if(Width < 32)
			return ERR_JPEG_ENC_PSCALEDOWN;
		if(Height < 32)
			return ERR_JPEG_ENC_PSCALEDOWN;

		/* Primary Image Vertical Down-Scaling. PSCALX_F = uWidthDownScaleFactor/2 - 1 and
		   PSCALY_F = uHeightDownScaleFactor - 1 */
		if(uWidthDScaleF > 1)
			regJPSCALD |= (0x00008000 | ((uWidthDScaleF / 2 - 1) << 8));
		if(uHeightDScaleF > 1)
			regJPSCALD |= (uHeightDScaleF - 1);
		outpw(REG_JPSCALD, regJPSCALD);

		pImgPrimary->uImageWidth = Width;
		pImgPrimary->uImageHeight = Height;
		/* Reassign Primary Encode Image Width and Primary Encode Image Height */
		outpw(REG_JPRIWH, (0xFFFF & pImgPrimary->uImageHeight) << 16 |
						  (0xFFFF & pImgPrimary->uImageWidth));
	}

	return Successful;
}

/* Up-Scaling function for encoding procedure. */
UINT32 jpegEncUpScalePImage(JPEG_ENCODE_SETTING_T *tEncodeSetting,
							JPEG_UPSCALE_METHOD_E eHorUpScaleM, JPEG_UPSCALE_METHOD_E eVerUpScaleM,
							UINT32 uWidthAfterUpScale, UINT32 uHeightAfterUpScale)
{
	JPEG_IMAGE_T *pImgSource = &(tEncodeSetting->imgSource);
	JPEG_IMAGE_T *pImgPrimary = &(tEncodeSetting->imgPrimary);
	UINT32 regJPSCALU = 0, Width = pImgSource->uImageWidth, Height = pImgSource->uImageHeight;

	/* Turn on up-scaling function */
	if(uWidthAfterUpScale > Width)
	{
		Width = uWidthAfterUpScale;
		regJPSCALU |= 0x00000080;
	}
	if(uHeightAfterUpScale > Height)
	{
		Height = uHeightAfterUpScale;
		regJPSCALU |= 0x00000040;
	}

	/* Set horizontal and vertical up-scaling methods */
	if(eHorUpScaleM == C_JPEG_INTERPOLATION)
		regJPSCALU |= 0x00000020;
	if(eVerUpScaleM == C_JPEG_INTERPOLATION)
		regJPSCALU |= 0x00000010;

	/* Write to the register REG_JPSCALU */
	outpw(REG_JPSCALU, (inpw(REG_JPSCALU) & ~0xF0) | regJPSCALU);

	/* Assign the width and height of source primary image in the register REG_JSRCWH */
	outpw(REG_JSRCWH, (0xFFFF & pImgSource->uImageHeight) << 16 |
					  (0xFFFF & pImgSource->uImageWidth));

	pImgPrimary->uImageWidth = Width;
	pImgPrimary->uImageHeight = Height;
	/* Assign the width and height of primary image after up-scaling in the register REG_JPRIWH */
	outpw(REG_JPRIWH, (0xFFFF & pImgPrimary->uImageHeight) << 16 |
					  (0xFFFF & pImgPrimary->uImageWidth));

	return Successful;
}

/* Reserve buffer in JPEG bit stream for software application. */
UINT32 jpegEncResBufForSW(UINT32 uReservedBufferSize)
{
	/* The value of reserved size must greater than zero, 
	be multiple of 2 but can't be multiple of 4.
	The actual byte counts reserved in bit-stream is equal to 
	(RES_SIZE - 2). */
	if(uReservedBufferSize < 2)
		return ERR_JPEG_ENC_RESBUFSIZE;

	if(uReservedBufferSize % 2)
		return ERR_JPEG_ENC_RESBUFSIZE;

	if(uReservedBufferSize % 4 == 0)
		return ERR_JPEG_ENC_RESBUFSIZE;

	/* Enable Reserve Buffer Size */
	outpw(REG_JPSCALU, inpw(REG_JPSCALU) | 1 << 2);

	outpw(REG_JRESERVE, 0xFFFF & uReservedBufferSize);

    return Successful;
}

/* Enable/disable encoding JPEG bit stream with on-the-fly mode and set number of lines. */
void jpegEncSetMemoryOnTheFly(BOOL bMemoryOnTheFly, JPEG_ENC_ONTHEFLY_LINE_NUMBER_E eOnTheFlyLineNumber)
{
	if(bMemoryOnTheFly)
		outpw(REG_JMACR, (inpw(REG_JMACR) & ~0x70) | 0x00000080 | eOnTheFlyLineNumber);
    else
		outpw(REG_JMACR, inpw(REG_JMACR) & ~0x00000080);
}

/* Enable/disable encoding JPEG bit stream with on-the-fly mode and set number of lines. */
UINT32 jpegEncSetMemorySWOnTheFly(BOOL bMemorySWOnTheFly, UINT32 bufsize)
{
	if(bMemorySWOnTheFly)
	{
		if(bufsize < 2048)
			return ERR_JPEG_ENC_SWONTHEFLY;
		if(bufsize/2048 > 0x3FF)
			return ERR_JPEG_ENC_SWONTHEFLY;

		outpw(REG_JMACR, (inpw(REG_JMACR) & ~0x3FF00) | 0x00000004 | bufsize/2048 << 8);
	}
    else
		outpw(REG_JMACR, inpw(REG_JMACR) & ~0x00000004);

	return Successful;
}

void jpegEncStartProcess(JPEG_ENC_MEMORY_ACCESS_MODE_E eMemoryAccessMode)
{
	_jpeg_EncodeOk = FALSE;
	_jpeg_EncodeOnTheFlyError = FALSE;

	if(eMemoryAccessMode == C_JPEG_STILL_FIXBUFFER || eMemoryAccessMode == C_JPEG_STILL_DUAL_BUFFER)
		F_JPEG_TRIGGER_ENGINE;
	else
		F_JPEG_START_ENGINE;
}

/* Wait the Encode Complete Interrupt or timeout */
UINT32 jpegEncWaitProcess(UINT32 difference)
{
#ifndef ECOS
	UINT32 uStartTime = sysGetTicks(TIMER1);
#else
	UINT32 uStartTime = cyg_current_time();
#endif

	/* Wait the Encode Compete Interrupt or timeout */
#ifndef ECOS
	while( !_jpeg_EncodeOk && !_jpeg_EncodeOnTheFlyError && difference > (sysGetTicks(TIMER1) - uStartTime) );
#else
	while( !_jpeg_EncodeOk && !_jpeg_EncodeOnTheFlyError && difference > (cyg_current_time() - uStartTime) );
#endif

	if(_jpeg_EncodeOnTheFlyError)
		return ERR_JPEG_ENC_ONTHEFLY;

	if(!_jpeg_EncodeOk)
		return ERR_JPEG_ENC_TIMEOUT;

	return Successful;
}

UINT32 jpegDecSetBasicOption(JPEG_DECODE_SETTING_T *tDecodeSetting)
{
	int regJMCR = 0, regJITCR = 0;

	regJITCR |= tDecodeSetting->eDecodeImgSel;
	regJITCR |= tDecodeSetting->eDecodeErrorAction;
	regJITCR |= tDecodeSetting->eDecodeHTabSource;

	if(tDecodeSetting->eScaling == C_JPEG_DEC_DOWN_SCALING)
	{
		if(tDecodeSetting->uWidthDScaleF > 16)
			return ERR_JPEG_DEC_SCALEDOWN;
		if(tDecodeSetting->uHeightDScaleF > 16)
			return ERR_JPEG_DEC_SCALEDOWN;

		outpw(REG_JPSCALD, 0x00008000 | ((tDecodeSetting->uWidthDScaleF)/2 - 1) << 8 |
										(tDecodeSetting->uHeightDScaleF - 1));
	}
	else if(tDecodeSetting->eScaling == C_JPEG_DEC_UP_SCALING)
	{
		int regJPSCALU = 0;

		/* Turn on both horizontal and vertical up-scaling */
		regJPSCALU |= 0x000000C0;
		if(tDecodeSetting->eHorUpScaleM == C_JPEG_INTERPOLATION)
			regJPSCALU |= 0x00000020;
		if(tDecodeSetting->eVerUpScaleM == C_JPEG_INTERPOLATION)
			regJPSCALU |= 0x00000010;

		outpw(REG_JPSCALU, (inpw(REG_JPSCALU) & ~0xF0) | regJPSCALU);
	}

	if(tDecodeSetting->bIsSetWindow)
	{
		/* JPEG Window Decode Enable */
		regJMCR |= 1 << 6;

		/* Set decode window */
		outpw(REG_JWINDEC0, tDecodeSetting->uHeightStart << 16 | tDecodeSetting->uWidthStart);
		outpw(REG_JWINDEC1, tDecodeSetting->uHeightEnd << 16 | tDecodeSetting->uWidthEnd);
		outpw(REG_JWINDEC2, (tDecodeSetting->uWidthEnd - tDecodeSetting->uWidthStart + 1) * 16);
	}

	outpw(REG_JMCR, regJMCR);
	outpw(REG_JITCR, regJITCR);

	return Successful;
}

void jpegDecSpecialColEffect(JPEG_DEC_COLOR_EFFECT_E eSpecialColorEffect, UINT32 uColorPattern)
{
	/* Set type of the special color effect */
	outpw(REG_JDCOLC, eSpecialColorEffect);

	/* Decode Color Pattern */
	outpw(REG_JDCOLP, uColorPattern);
}

void jpegDecStartProcess()
{
	_jpeg_DecodeOk = FALSE;
	_jpeg_DecodeError = FALSE;
	
	F_JPEG_TRIGGER_ENGINE;
}

/* Wait the Encode Complete Interrupt or timeout */
UINT32 jpegDecWaitProcess(UINT32 difference)
{
#ifndef ECOS
	UINT32 uStartTime = sysGetTicks(TIMER1);
#else
	UINT32 uStartTime = cyg_current_time();
#endif

	/* Wait the Encode Compete Interrupt or timeout */
#ifndef ECOS
	while( !_jpeg_DecodeOk && !_jpeg_DecodeError && difference > (sysGetTicks(TIMER1) - uStartTime) );
#else
	while( !_jpeg_DecodeOk && !_jpeg_DecodeError && difference > (cyg_current_time() - uStartTime) );
#endif

	if(_jpeg_DecodeError)
		return ERR_JPEG_DEC_PROCESS;

	if(!_jpeg_DecodeOk)
		return ERR_JPEG_DEC_TIMEOUT;

	return Successful;
}

UINT32 jpegDecGetImage(JPEG_DECODE_SETTING_T *tDecodeSetting)
{
	JPEG_IMAGE_T *pImgOutput = &(tDecodeSetting->imgOutput);

	pImgOutput->uImageWidth = inpw(REG_JDECWH) & 0x0000FFFF;
	pImgOutput->uImageHeight = inpw(REG_JDECWH) >> 16;

	/* Down-scaling */
	if(tDecodeSetting->eScaling == C_JPEG_DEC_DOWN_SCALING)
	{
		pImgOutput->uImageWidth /= tDecodeSetting->uWidthDScaleF;
		pImgOutput->uImageHeight /= tDecodeSetting->uHeightDScaleF;

		if(pImgOutput->uImageWidth < 32)
			return ERR_JPEG_DEC_SCALEDOWN;
		if(pImgOutput->uImageWidth % 4)
			return ERR_JPEG_DEC_SCALEDOWN;
		if(pImgOutput->uImageHeight < 32)
			return ERR_JPEG_DEC_SCALEDOWN;
	}
	/* Up-scaling */
	else if(tDecodeSetting->eScaling == C_JPEG_DEC_UP_SCALING)
	{
		pImgOutput->uImageWidth <<= 1;
		pImgOutput->uImageHeight <<= 1;
	}

	if(tDecodeSetting->bIsSetWindow)
	{
		pImgOutput->uImageWidth = (tDecodeSetting->uWidthEnd - tDecodeSetting->uWidthStart + 1) << 4;
		pImgOutput->uImageHeight = (tDecodeSetting->uHeightEnd - tDecodeSetting->uHeightStart + 1) << 4;
	}

	pImgOutput->eImageFormat = (inpw(REG_JITCR) & 0x00000300) >> 8;

	return Successful;
}
/****************************************************************************
 *
 * Copyright (c) 2006 - 2006 Winbond Electronics Corp. All rights reserved.
 *
 ***************************************************************************/
 
/****************************************************************************
 *
 * FILENAME
 *     jpeglib.h
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     The header file for 702 JPEG library.
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *     None
 *
 * HISTORY
 *     16/10/06		 Ver 1.0 Created by NS25 Jzhou0
 *
 * REMARK
 *     None
 **************************************************************************/
#ifndef _JPEGLIB_H_
#define _JPEGLIB_H_

#include "wblib.h"


/* const define */
#define ERR_JPEG_ENC_IMGFORMAT							(JPEG_ERR_ID|0x01)
#define ERR_JPEG_ENC_TSCALEDOWN							(JPEG_ERR_ID|0x02)
#define ERR_JPEG_ENC_PSCALEDOWN							(JPEG_ERR_ID|0x03)
#define ERR_JPEG_ENC_TOFFSET							(JPEG_ERR_ID|0x04)
#define ERR_JPEG_ENC_RESBUFSIZE							(JPEG_ERR_ID|0x05)
#define ERR_JPEG_ENC_ONTHEFLY							(JPEG_ERR_ID|0x06)
#define ERR_JPEG_ENC_SWONTHEFLY							(JPEG_ERR_ID|0x07)
#define ERR_JPEG_ENC_TIMEOUT							(JPEG_ERR_ID|0x08)
#define ERR_JPEG_DEC_SCALEDOWN							(JPEG_ERR_ID|0x09)
#define ERR_JPEG_DEC_PROCESS							(JPEG_ERR_ID|0x0a)
#define ERR_JPEG_DEC_TIMEOUT							(JPEG_ERR_ID|0x0b)

#define MAX_CONTINUE_IMAGE 100

#define C_JPEG_ENC_P_JFIF_INCLUDE						(1<<7)
#define C_JPEG_ENC_P_HTAB_INCLUDE						(1<<6)
#define C_JPEG_ENC_P_QTAB_INCLUDE						(1<<5)
#define C_JPEG_ENC_P_RINTERVAL_INCLUDE					(1<<4)
#define C_JPEG_ENC_T_JFIF_INCLUDE						(1<<3)
#define C_JPEG_ENC_T_HTAB_INCLUDE						(1<<2)
#define C_JPEG_ENC_T_QTAB_INCLUDE						(1<<1)
#define C_JPEG_ENC_T_RINTERVAL_INCLUDE					(1<<0)


/* typedefs */
typedef enum {C_JPEG_CALLBACK_ENCODE_COMPLETE_INTERRUPT, C_JPEG_CALLBACK_DECODE_COMPLETE_INTERRUPT,
			  C_JPEG_CALLBACK_DECODE_ERROR_INTERRUPT, C_JPEG_CALLBACK_ENCODE_ONTHEFLY_ERROR_INTERRUPT,
			  C_JPEG_CALLBACK_ENCODE_SWONTHEFLY_WAIT_INTERRUPT
			 } JPEG_INSTALL_CALL_BACK_FUNCTION_E;

typedef enum {C_JPEG_YUV420, C_JPEG_YUV422, C_JPEG_YUV444, C_JPEG_GRAY, C_JPEG_JPEG} JPEG_IMAGEFORMAT_E;

typedef enum {C_JPEG_TWO_QTAB, C_JPEG_THREE_QTAB} JPEG_QTAB_NUMBER_E;

typedef enum {C_JPEG_ENC_OBJECT_P, C_JPEG_ENC_OBJECT_BOTH} JPEG_ENC_OBJECT_E;

typedef enum {C_JPEG_ENC_GENERAL						= 0,
			  C_JPEG_ENC_EXIF							= 1 << 7} JPEG_ENC_QTAB_HTAB_HFORMAT_E;

typedef enum {C_JPEG_DEC_P_IMAGE						= 0 ,
			  C_JPEG_DEC_T_IMAGE						= 1 << 4} JPEG_DEC_IMAGESEL_E;

typedef enum {C_JPEG_DEC_ERROR_ABORT					= 0,
			  C_JPEG_DEC_ERROR_CONTINUE					= 1 << 1 } JPEG_DEC_ERROR_ACTION_E;

typedef enum {C_JPEG_DEC_DEFHTAB						= 0,
			  C_JPEG_DEC_USERHTAB						= 1 } JPEG_DEC_HTAB_SOURCE_E;

typedef enum {C_JPEG_DEC_NO_SCALING, C_JPEG_DEC_UP_SCALING, C_JPEG_DEC_DOWN_SCALING} JPEG_DEC_SCALING_E;

typedef enum {C_JPEG_INTER_OUTPUTWAIT_ENABLE			= 1 << 12,
			  C_JPEG_INTER_PENDING						= 1 << 10,
			  C_JPEG_INTER_OUTPUTWAIT_STATUS			= 1 << 8,
			  C_JPEG_ENC_INTER_COMPLETE_ENABLE			= 1 << 7,
			  C_JPEG_DEC_INTER_COMPLETE_ENABLE			= 1 << 6,
			  C_JPEG_DEC_INTER_ERROR_ENABLE				= 1 << 5,
			  C_JPEG_ENC_INTER_ONTHEFLY_ERROR_ENABLE	= 1 << 4,
			  C_JPEG_ENC_INTER_COMPLETE_STATUS			= 1 << 3,
			  C_JPEG_DEC_INTER_COMPLETE_STATUS			= 1 << 2,
			  C_JPEG_DEC_INTER_ERROR_STATUS				= 1 << 1,
			  C_JPEG_ENC_INTER_ONTHEFLY_ERROR_STATUS	= 1} JPEG_INTERRUPT_E;

typedef enum {C_JPEG_DEC_SOLARIZE						= 1 << 2,
			  C_JPEG_DEC_NEGATIVE						= 1 << 1,
			  C_JPEG_DEC_SEPIA							= 1} JPEG_DEC_COLOR_EFFECT_E;

typedef enum {C_JPEG_ENC_ONTHEFLY_16_LINES				= 0,
			  C_JPEG_ENC_ONTHEFLY_32_LINES				= 1 << 4,
			  C_JPEG_ENC_ONTHEFLY_64_LINES				= 2 << 4,
			  C_JPEG_ENC_ONTHEFLY_128_LINES				= 3 << 4,
			  C_JPEG_ENC_ONTHEFLY_256_LINES				= 4 << 4,
			  C_JPEG_ENC_ONTHEFLY_512_LINES				= 5 << 4,
			  C_JPEG_ENC_ONTHEFLY_1024_LINES			= 6 << 4} JPEG_ENC_ONTHEFLY_LINE_NUMBER_E;

typedef enum {C_JPEG_STILL_FIXBUFFER					= 0,
			  C_JPEG_STILL_DUAL_BUFFER					= 1,
			  C_JPEG_CONTINUE_MEMORY					= 2,
			  C_JPEG_CONTINUE_SAMESIZE					= 3} JPEG_ENC_MEMORY_ACCESS_MODE_E;

typedef enum {C_JPEG_YUV_BUFFER_0, C_JPEG_YUV_BUFFER_1} JPEG_YUV_BUFFER_E;

typedef enum {C_JPEG_BITSTREAM_BUFFER_0, C_JPEG_BITSTREAM_BUFFER_1} JPEG_BITSTREAM_BUFFER_E;

typedef enum {C_JPEG_DUPLICATION, C_JPEG_INTERPOLATION} JPEG_UPSCALE_METHOD_E;

/* Data structures */
typedef struct
{
	JPEG_IMAGEFORMAT_E					eImageFormat;
	UINT32								uImageWidth,
										uImageHeight;
	union{
		struct{									
			UINT32						uYStartAddress,
										uUStartAddress,
										uVStartAddress;
		} YUVAddress;
		struct{
			UINT32						uStartAddress,
										uImgSize;
		} JpegAddress;
	} ImgAddress;
} JPEG_IMAGE_T;

/* encode data structures */
typedef struct 
{
	JPEG_ENC_OBJECT_E					eEncodeObject;
	JPEG_ENC_QTAB_HTAB_HFORMAT_E		eTablesHeaderFormat;
	JPEG_QTAB_NUMBER_E					eQTabNumber;

	/* source image */
	JPEG_IMAGE_T						imgSource;
	UINT32								uYStride,
										uUStride,
										uVStride;

	/* Set primary image for output */
	JPEG_IMAGE_T						imgPrimary;
	UINT32								uPIncludeHeader,
										uPRestartInterval;

	/* Set thumbnail image for output */
	JPEG_IMAGE_T						imgThumbnail;
	UINT32								uTIncludeHeader,
										uTRestartInterval,
										uTOffset,
										uTWidthDScaleF,
										uTHeightDScaleF;

	/* Set IO buffers */
	UINT32								uJpegBitstreamStartAddress;
} JPEG_ENCODE_SETTING_T;

/* decode data structures */
typedef struct
{
	JPEG_DEC_IMAGESEL_E					eDecodeImgSel;
	JPEG_DEC_ERROR_ACTION_E				eDecodeErrorAction;
	JPEG_DEC_HTAB_SOURCE_E				eDecodeHTabSource;

	JPEG_DEC_SCALING_E					eScaling;
	UINT32								uWidthDScaleF,
										uHeightDScaleF;
	JPEG_UPSCALE_METHOD_E				eHorUpScaleM,
										eVerUpScaleM;

	BOOL								bIsSetWindow;
	UINT32								uWidthStart,
										uWidthEnd,
										uHeightStart,
										uHeightEnd;
	/* source image */
	UINT32								uJpegBitstreamStartAddress;

	/* output image */
	JPEG_IMAGE_T						imgOutput;
} JPEG_DECODE_SETTING_T;


/* Macro functions */
#define F_JPEG_RESUME\
	(outpw(REG_JMCR, inpw(REG_JMCR) | 1 << 8))
#define F_JPEG_IS_QT_BUSY\
	(inpw(REG_JMCR) & 1 << 2 )
#define F_JPEG_RESET\
	do{\
		outpw(REG_JMCR,0x00000002);\
		outpw(REG_JMCR,0x00000000);\
		outpw(REG_JHEADER,0x00000000);\
		outpw(REG_JITCR,0x00000000);\
		outpw(REG_JPRIQC, 0x000000F4);\
		outpw(REG_JTHBQC, 0x000000F4);\
		outpw(REG_JPRIWH,0x00000000);\
		outpw(REG_JTHBWH,0x00000000);\
		outpw(REG_JPRST, 0x00000004);\
		outpw(REG_JTRST, 0x00000004);\
		outpw(REG_JDECWH,0x00000000);\
		outpw(REG_JINTCR,0x00000000);\
		outpw(REG_JDCOLC,0x00000000);\
		outpw(REG_JDCOLP,0x00000000);\
		outpw(REG_JTEST,0x00000000);\
		outpw(REG_JWINDEC0,0x00000000);\
		outpw(REG_JWINDEC1,0x00000000);\
		outpw(REG_JWINDEC2,0x00000000);\
		outpw(REG_JMACR,0x00000000);\
		outpw(REG_JPSCALU,0x00000000);\
		outpw(REG_JPSCALD,0x00000000);\
		outpw(REG_JTSCALD,0x00000000);\
		outpw(REG_JDBCR,0x00000000);\
		outpw(REG_JRESERVE,0x00000000);\
		outpw(REG_JOFFSET,0x00000000);\
		outpw(REG_JFSTRIDE,0x00000000);\
		outpw(REG_JYADDR0,0x00000000);\
		outpw(REG_JUADDR0,0x00000000);\
		outpw(REG_JVADDR0,0x00000000);\
		outpw(REG_JYADDR1,0x00000000);\
		outpw(REG_JUADDR1,0x00000000);\
		outpw(REG_JVADDR1,0x00000000);\
		outpw(REG_JYSTRIDE,0x00000000);\
		outpw(REG_JUSTRIDE,0x00000000);\
		outpw(REG_JVSTRIDE,0x00000000);\
		outpw(REG_JIOADDR0,0x00000000);\
		outpw(REG_JIOADDR1,0x00000000);\
		outpw(REG_JPRI_SIZE,0x00000000);\
		outpw(REG_JTHB_SIZE,0x00000000);\
		outpw(REG_JSRCWH,0x00000000);\
	}while (0)
#define F_JPEG_TRIGGER_ENGINE\
	do{outpw(REG_JMCR, inpw(REG_JMCR) | 0x00000001);outpw(REG_JMCR, inpw(REG_JMCR) & ~0x00000001);}while (0)
#define F_JPEG_START_ENGINE\
	do{outpw(REG_JMCR, inpw(REG_JMCR) | 0x00000001);}while (0)
#define F_JPEG_STOP_ENGINE\
	do{outpw(REG_JMCR, inpw(REG_JMCR) & ~0x00000001);}while (0)

#define F_JPEG_ADJUST_PQTAB(uQTableAdjustment, uQTableScalingControl)\
	(outpw(REG_JPRIQC, 0xFF & ((uQTableAdjustment) << 4 | (uQTableScalingControl))))

#define F_JPEG_ADJUST_TQTAB(uQTableAdjustment, uQTableScalingControl)\
	(outpw(REG_JTHBQC, 0xFF & ((uQTableAdjustment) << 4 | (uQTableScalingControl))))

#define F_JPEG_ENABLE_AND_CLEAR_INTERRUPT(uInterrupt)\
	(outpw(REG_JINTCR, (uInterrupt) | ((uInterrupt) >> 4 & 0x0000000F)))
#define F_JPEG_GET_P_IMAGESIZE		inpw(REG_JPRI_SIZE)
#define F_JPEG_GET_T_IMAGESIZE		inpw(REG_JTHB_SIZE)

/* functions */
void jpegSetIRQHandler(JPEG_INSTALL_CALL_BACK_FUNCTION_E eCallBackFunctionType, UINT32 (*puCallbackfunction)());
#ifndef ECOS
void jpegIntHandler(void);
#else
extern UINT32 g_IntrStatus;

cyg_uint32 jpegIntHandler(cyg_vector_t vector, cyg_addrword_t data);
void jpegIntHandler_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);
#endif
void jpegInitializeEngine(void);
void jpegSetMemoryAddress(JPEG_YUV_BUFFER_E eYUVBuffer, JPEG_BITSTREAM_BUFFER_E eBitstreamBuffer,
							 UINT32 uYStartAddress, UINT32 uUStartAddress, UINT32 uVStartAddress, 
							 UINT32 uJpegBitstreamStartAddress);
/* for encode */
UINT32 jpegEncSetBasicOption(JPEG_ENCODE_SETTING_T *tEncodeSetting);
void jpegEncSetMemoryAccessMode(JPEG_ENC_MEMORY_ACCESS_MODE_E eMemoryAccessMode, UINT32 uContinueModeSameSizeBufferSize);
void jpegEncSetDualBuffer(JPEG_YUV_BUFFER_E eInputBuffer, JPEG_BITSTREAM_BUFFER_E eOutputBuffer);
void jpegEncWriteQTable(JPEG_QTAB_NUMBER_E eQTabNumber, UINT8 **ucQTable);
UINT32 jpegEncDownScalePImage(JPEG_ENCODE_SETTING_T *tEncodeSetting, UINT32 uWidthDScaleF, UINT32 uHeightDScaleF);
UINT32 jpegEncUpScalePImage(JPEG_ENCODE_SETTING_T *tEncodeSetting,
							JPEG_UPSCALE_METHOD_E eHorUpScaleM, JPEG_UPSCALE_METHOD_E eVerUpScaleM,
							UINT32 uWidthAfterUpScale, UINT32 uHeightAfterUpScale);
UINT32 jpegEncResBufForSW(UINT32 uReservedBufferSize);
void jpegEncSetMemoryOnTheFly(BOOL bMemoryOnTheFly, JPEG_ENC_ONTHEFLY_LINE_NUMBER_E eOnTheFlyLineNumber);
UINT32 jpegEncSetMemorySWOnTheFly(BOOL bMemorySWOnTheFly, UINT32 bufsize);
void jpegEncStartProcess(JPEG_ENC_MEMORY_ACCESS_MODE_E eMemoryAccessMode);
UINT32 jpegEncWaitProcess(UINT32 difference);
/* for decode */
UINT32 jpegDecSetBasicOption(JPEG_DECODE_SETTING_T *tDecodeSetting);
void jpegDecSpecialColEffect(JPEG_DEC_COLOR_EFFECT_E eSpecialColorEffect, UINT32 eColorPattern);
void jpegDecStartProcess(void);
UINT32 jpegDecWaitProcess(UINT32 difference);
UINT32 jpegDecGetImage(JPEG_DECODE_SETTING_T *tDecodeSetting);

#endif /* End of _JPEGLIB_H_ */

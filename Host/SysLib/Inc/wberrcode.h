/****************************************************************************
 *                                                                          *
 * Copyright (c) 2005 - 2007 Winbond Electronics Corp. All rights reserved. *
 *                                                                          *
 ***************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *     wberrcode.h
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *	   The Error Codes returned by WB S/W library are defined as the following
 *     format:
 *              0xFFFFXX## (XX : Module ID, ##:Error Number)     
 *
 *     The Module IDs for each S/W library are defined in this file. The error
 *     numbers are defined by S/W library according to its requirement. 
 *     
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *     None
 *
 * HISTORY
 *     05/05/2005		 Ver 1.0 Created by PC30 MNCheng
 *
 * REMARK
 *     None
 **************************************************************************/
#ifndef _WBERRCODE_H
#define _WBERRCODE_H

/* Error Code's Module ID */
#define FMI_ERR_ID		0xFFFF0100	/* FMI library ID */
#define ADO_ERR_ID		0xFFFF0200	/* Audio library ID */
#define USB_ERR_ID		0xFFFF0300	/* USB Device library ID */
#define LCM_ERR_ID		0xFFFF0400	/* LCM library ID */
#define DSP_ERR_ID		0xFFFF0500	/* Sensor DSP library ID */
#define CAP_ERR_ID		0xFFFF0600	/* Video Capture library ID */
#define MP4_ERR_ID		0xFFFF0700	/* MPEG-4 library ID */
#define GE_ERR_ID		0xFFFF0800	/* 2D graphics library ID */
#define VPE_ERR_ID		0xFFFF0900	/* VPE library ID */
#define SYSLIB_ERR_ID	0xFFFF0A00	/* System library ID */
#define JPEG_ERR_ID		0xFFFF0C00	/* JPEG library ID */
#define HIC_ERR_ID		0xFFFF1000	/* HIC I/F ID */

#define MFL_ERR_ID		0xFFFF8000	/* Media File library ID */
#define FAT_ERR_ID		0xFFFF8200	/* FAT file system library ID */
#define AACE_ERR_ID		0xFFFF8400	/* AAC encoder ID */
#define AACD_ERR_ID		0xFFFF8600	/* AAC/AAC+ decoder ID */
#define AMRNE_ERR_ID	0xFFFF8800	/* AMR NB encoder ID */
#define AMRND_ERR_ID	0xFFFF8A00	/* AMR NB decoder ID */
#define AMRWD_ERR_ID	0xFFFF8C00	/* AMR WB decoder ID */
#define MP3_ERR_ID		0xFFFF9000	/* MP3 decoder ID */
#define WMA_ERR_ID		0xFFFF9200	/* WMA decoder ID */
#define ADPCM_ERR_ID	0xFFFF9400	/* ADPCM encoder/decoder ID */

#define FONT_ERR_ID     0xFFFFA000  /* FONT library ID */
#define FILTER_ERR_ID   0xFFFFA200  /* Image Filter library ID */
#define PBRIDGE_ERR_ID	0xFFFFA400	/* PictBridge ID */

/* Macros for handing error code */
#define WBAPI_RESULT_IS_ERROR(value)	((value) < 0) ? 1 : 0		/* 1:error, 0:non-err */
#define WBAPI_GET_ERROR_ID(value)		((value) & 0x0000FF00) >> 8 /* get Module ID of error code */
#define WBAPI_GET_ERROR_NUMBER(value)	((value) & 0x000000FF) 		/* get Error Number of error code */


/* Error Number defined by XXX library */


#endif /* _WBERRCODE_H */

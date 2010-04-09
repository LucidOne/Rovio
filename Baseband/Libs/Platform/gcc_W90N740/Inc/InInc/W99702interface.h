/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2005 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *     W99702reg.h
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     W99702 registers' definition.
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *     None
 *
 * HISTORY
 *     11/05/04		 Ver 1.0 Porting from "LeTian's code"
 *
 * REMARK
 *     None
 **************************************************************************/
 





#ifndef __W99702REG_H__
#define __W99702REG_H__





/* W99702 CompactFlash Device related defines */

#define AHB_IO_BASE		0xFFF00000
#define EXT0CON		0x1018

#define EBIO0_BA			0xFC000000
#define EXTIO_BANK			(AHB_IO_BASE + EXT0CON)
#define EXTIO_BASE_ADDR		((EBIO0_BA & 0x7FFC0000) << 1)
#define EXTIO_SIZE			(0x0 << 16)		/* 256KB-0, 512KB-0x01 */
#define EXTIO_ADRS			(0x1 << 15)		/* byte address alignment */
#define EXTIO_BUS_WIDTH_8	0x01			/*  8 bit */
#define EXTIO_BUS_WIDTH_16	0x02			/* 16 bit */
#define EXTIO_BUS_WIDTH_32	0x03			/* 32 bit */
#define EXTIO_tACC			(0x3 << 11)	//(0x8 << 11)
#define EXTIO_tCOH			(0x7 <<  8)
#define EXTIO_tACS			(0x4 <<  5)
#define EXTIO_tCOS			(0x4 <<  2)


#define nIRQ0 2


#endif

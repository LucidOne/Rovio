/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2005 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/

/****************************************************************************
 *
 * FILENAME
 *     Platform.h
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     Compatibale header for various baseband platform.
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *     None
 *
 * HISTORY
 *     11/05/04		 Ver 1.0 Created by PC34 xhchen
 *
 * REMARK
 *     None
 **************************************************************************/



#ifndef __PLATFORM_H__
#define __PLATFORM_H__



#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>


#include "./InInc/wbtypes.h"
#include "./InInc/W99702interface.h"
#include "./InInc/KeyInput.h"
#include "./InInc/W90N740.h"
#include "./InInc/irq.h"
#include "./InInc/W90N740_registers.h"



/* To make a porting for other platform,
   replace the following marios with your own.
   The check the functions like "sysXXX", if it is called by the demo
   or by other libs, it's porting version must also be given.
 */



/* In the following code, use (reg) << 2.
   The reason may be an odd address can not be accessed by pointer
   with "UINT16*" type on some platform ???
   The old remarks in Le.Tian's code:
   	// note: A0 of cotulla is floating
 */
/* Index mode access interface.
   Index mode use A3, A2, A0
		CE#	LCM_ce_			Chip enable to access LCM or to HIC
		A3	Bypass_			0: LCM/Melody access, 1: HIC access
		A2	Melody_ce_		Chip enable to access Melody chip through bypass
		A1	Undefined
		A0	Data/Index_		0:	Index register	1: Data register
	"reg" (0 ~ 15), is used to specify A3 ~ A0
 */
#define PLATFROM_DEFINE_REG_INDEX8(reg)		(*(volatile UCHAR *)((EBIO0_BA) + ((reg) << 2)))
#define PLATFROM_DEFINE_REG_INDEX16(reg)	(*(volatile UINT16 *)((EBIO0_BA) + ((reg) << 2)))
#define PLATFROM_DEFINE_REG_INDEX32(reg)	(*(volatile UINT32 *)((EBIO0_BA) + ((reg) << 2)))

/* 16 registers in address mode, reg: 0 ~ 15. */
#define PLATFROM_DEFINE_REG_A8(reg)		(*(volatile UCHAR *)(EBIO0_BA + ( (reg) << 2 )))
#define PLATFROM_DEFINE_REG_A16(reg)		(*(volatile UINT16 *)(EBIO0_BA + ( (reg) << 2 )))
#define PLATFROM_DEFINE_REG_A32(reg)		(*(volatile UINT32 *)(EBIO0_BA + ( (reg) << 2 )))


/* Functions */
#define FIQ_LEVEL_0                0
#define IRQ_LEVEL_1                1
#define IRQ_LEVEL_2                2
#define IRQ_LEVEL_3                3
#define IRQ_LEVEL_4                4
#define IRQ_LEVEL_5                5
#define IRQ_LEVEL_6                6
#define IRQ_LEVEL_7                7

#define LOW_LEVEL_SENSITIVE        0x00
#define HIGH_LEVEL_SENSITIVE       0x40
#define NEGATIVE_EDGE_TRIGGER      0x80
#define POSITIVE_EDGE_TRIGGER      0xC0

typedef struct
{
	BOOL volatile bWait;
} PLATFORM_WAIT_OBJ_T;

#endif

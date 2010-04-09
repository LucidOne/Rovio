/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2005 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/

/****************************************************************************
 *
 * FILENAME
 *		$RCSfile: HICAddr.h,v $
 *
 * VERSION
 *		$Revision: 1.1 $
 *
 * DESCRIPTION
 *		W99702 hardware control interface.
 *
 * DATA STRUCTURES
 * 		Not list here.
 *
 * FUNCTIONS
 *		Not list here.
 *
 * HISTORY
 *     $Log: HICAddr.h,v $
 *     Revision 1.1  2006/01/17 09:42:12  xhchen
 *     Add B.B. testing applications.
 *
 *     Revision 1.1.2.3  2005/07/21 07:46:57  xhchen
 *     LCM bypass in index mode is OK.
 *
 *     Revision 1.1.2.2  2004/12/22 09:46:01  xhchen
 *     ...
 *
 *
 * REMARK
 *     None
 *
 **************************************************************************/



#ifndef __HICADDR_H__
#define __HICADDR_H__


//Registers.
#define CF_A_REG0		REG_A8(0x00)
#define CF_A_REG1		REG_A8(0x01)
#define CF_A_REG2		REG_A8(0x02)
#define CF_A_REG3		REG_A8(0x03)
#define CF_A_REG4		REG_A8(0x04)
#define CF_A_REG5		REG_A8(0x05)
#define CF_A_REG6		REG_A8(0x06)
#define CF_A_REG7		REG_A8(0x07)
#define CF_A_REG8		REG_A8(0x08)
#define CF_A_REG9		REG_A8(0x09)
#define CF_A_REGA		REG_A8(0x0A)
#define CF_A_REGB		REG_A8(0x0B)
#define CF_A_REGC		REG_A8(0x0C)
#define CF_A_REGD		REG_A8(0x0D)
#define CF_A_REGE		REG_A8(0x0E)
#define CF_A_REGF		REG_A8(0x0F)


#define CF_A_REG0_W		REG_A16(0x00)
#define CF_A_REG1_W		REG_A16(0x01)
#define CF_A_REG2_W		REG_A16(0x02)
#define CF_A_REG3_W		REG_A16(0x03)
#define CF_A_REG4_W		REG_A16(0x04)
#define CF_A_REG5_W		REG_A16(0x05)
#define CF_A_REG6_W		REG_A16(0x06)
#define CF_A_REG7_W		REG_A16(0x07)
#define CF_A_REG8_W		REG_A16(0x08)
#define CF_A_REG9_W		REG_A16(0x09)
#define CF_A_REGA_W		REG_A16(0x0A)
#define CF_A_REGB_W		REG_A16(0x0B)
#define CF_A_REGC_W		REG_A16(0x0C)
#define CF_A_REGD_W		REG_A16(0x0D)
#define CF_A_REGE_W		REG_A16(0x0E)
#define CF_A_REGF_W		REG_A16(0x0F)




//Register alias name.

// General registers in 8bit mode.
#define CF_A_DATA		CF_A_REG8	/* Data Port */
#define CF_A_COMMAND	CF_A_REGF	/* write for Command Port */
#define CF_A_STATUS		CF_A_REGF	/* read for Status Port */
// Parameter
#define CF_A_PARAM0		CF_A_REG9	/* Parameter [ 7: 0] */
#define CF_A_PARAM1		CF_A_REGA	/* Parameter [15: 8] */
#define CF_A_PARAM2		CF_A_REGB	/* Parameter [23:16] */
#define CF_A_PARAM3		CF_A_REGC	/* Parameter [31:24] */
#define CF_A_PARAM4		CF_A_REGD	/* Parameter [39:32] */
#define CF_A_PARAM5		CF_A_REGE	/* Parameter [47:40] */
// Address
#define CF_A_ADDR0		CF_A_REG9	/* Address [ 7: 0] */
#define CF_A_ADDR1		CF_A_REGA	/* Address [15: 8] */
#define CF_A_ADDR2		CF_A_REGB	/* Address [23:16] */
#define CF_A_ADDR3		CF_A_REGC	/* Address [31:24] */
// Length
#define CF_A_LEN0		CF_A_REGD	/* Transfer Count [ 7: 0] */
#define CF_A_LEN1		CF_A_REGE	/* Transfer Count [15: 8] */
#define CF_A_LEN2		CF_A_REG6	/* Transfer Count [23:16] */

//16 bits registers alias name not list here.

#endif

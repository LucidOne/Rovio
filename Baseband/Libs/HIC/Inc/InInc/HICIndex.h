/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2005 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/

/****************************************************************************
 *
 * FILENAME
 *		$RCSfile: HICIndex.h,v $
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
 *     $Log: HICIndex.h,v $
 *     Revision 1.1  2006/01/17 09:42:12  xhchen
 *     Add B.B. testing applications.
 *
 *     Revision 1.1.2.5  2005/07/21 07:46:57  xhchen
 *     LCM bypass in index mode is OK.
 *
 *     Revision 1.1.2.4  2005/07/15 08:20:07  xhchen
 *     Use PIPE to transfer FIFO instead of callback functions.
 *
 *     Revision 1.1.2.3  2005/03/11 09:26:23  xhchen
 *     Check in after a long period of time because the harddisk on 10.130.249.103 can not be used for some unknown reason.
 *
 *     Revision 1.1.2.2  2004/12/22 09:46:01  xhchen
 *     ...
 *
 *
 * REMARK
 *     None
 *
 **************************************************************************/



#ifndef __HICINDEX_H__
#define __HICINDEX_H__


/* 16 registers in index mode, reg: 0 ~ 15. */
#define REG_HIC_INDEX8_ADDR		REG_INDEX8(0x08)
#define REG_HIC_INDEX8_DATA		REG_INDEX8(0x09)
#define REG_HIC_INDEX16_ADDR	REG_INDEX16(0x08)
#define REG_HIC_INDEX16_DATA	REG_INDEX16(0x09)
#define REG_HIC_INDEX32_ADDR	REG_INDEX32(0x08)
#define REG_HIC_INDEX32_DATA	REG_INDEX32(0x09)
#define REG_LCM_INDEX8_ADDR		REG_INDEX8(0x04)
#define REG_LCM_INDEX8_DATA		REG_INDEX8(0x05)
#define REG_LCM_INDEX16_ADDR	REG_INDEX16(0x04)
#define REG_LCM_INDEX16_DATA	REG_INDEX16(0x05)
#define REG_LCM_INDEX32_ADDR	REG_INDEX32(0x04)
#define REG_LCM_INDEX32_DATA	REG_INDEX32(0x05)


#define REG_I8(reg)		(*((REG_HIC_INDEX8_ADDR) = (reg), &(REG_HIC_INDEX8_DATA)))
#define REG_I16(reg)	(*((REG_HIC_INDEX16_ADDR) = (reg), &(REG_HIC_INDEX16_DATA)))
#define REG_I32(reg)	(*((REG_HIC_INDEX32_ADDR) = (reg), &(REG_HIC_INDEX32_DATA)))

/* Note:
	To improve performance, use pre-definitions to access registers.
  The limitation:
	In index mode, you can assign REG0 with
		REG_I8(0) = 0x34;	// REG_HIC_INDEX8_ADDR = 0, REG_HIC_INDEX8_DATA = 0x34;
	But you can not write:
		REG_I8(0) = REG_I8(1);
	Becauset the actual assignment steps were:
		REG_HIC_INDEX8_ADDR = 0
		REG_HIC_INDEX8_ADDR = 1
		tmp = REG_HIC_INDEX8_DATA
		REG_HIC_INDEX8_DATA = tmp
	The correct steps should be:
		REG_HIC_INDEX8_ADDR = 0
		tmp = REG_HIC_INDEX8_DATA
		REG_HIC_INDEX8_ADDR = 1
		REG_HIC_INDEX8_DATA = tmp

	To resolve this problem, use two functions instead:
		VOID REG_I8_outp(UCHAR ucReg, UCHAR ucValue)
		{
			REG_HIC_INDEX8_ADDR = ucReg;
			REG_HIC_INDEX8_DATA = ucValue;
		}

		UCHAR REG_I8_inp(UCHAR ucReg)
		{
			REG_HIC_INDEX8_ADDR = ucReg;
			return REG_HIC_INDEX8_DATA;
		}

		Then if want to assign reg 1 to reg 0, write:
			REG_I8_outp(0, REG_I8_inp(1));

		While in the macro version, we have to write:
			UCHAR ucTmp;
			ucTmp = REG_I8(1);
			REG_I8(0) = ucTmp.

		Fuctions may be much more slower than micros. However,
	add a "inline" before the functions if available.
 */



//Registers.
#define CF_I_REG0		REG_I8(0x00)
#define CF_I_REG1		REG_I8(0x01)
#define CF_I_REG2		REG_I8(0x02)
#define CF_I_REG3		REG_I8(0x03)
#define CF_I_REG4		REG_I8(0x04)
#define CF_I_REG5		REG_I8(0x05)
#define CF_I_REG6		REG_I8(0x06)
#define CF_I_REG7		REG_I8(0x07)
#define CF_I_REG8		REG_I8(0x08)
#define CF_I_REG9		REG_I8(0x09)
#define CF_I_REGA		REG_I8(0x0A)
#define CF_I_REGB		REG_I8(0x0B)
#define CF_I_REGC		REG_I8(0x0C)
#define CF_I_REGD		REG_I8(0x0D)
#define CF_I_REGE		REG_I8(0x0E)
#define CF_I_REGF		REG_I8(0x0F)


#define CF_I_REG0_W		REG_I16(0x00)
#define CF_I_REG1_W		REG_I16(0x01)
#define CF_I_REG2_W		REG_I16(0x02)
#define CF_I_REG3_W		REG_I16(0x03)
#define CF_I_REG4_W		REG_I16(0x04)
#define CF_I_REG5_W		REG_I16(0x05)
#define CF_I_REG6_W		REG_I16(0x06)
#define CF_I_REG7_W		REG_I16(0x07)
#define CF_I_REG8_W		REG_I16(0x08)
#define CF_I_REG9_W		REG_I16(0x09)
#define CF_I_REGA_W		REG_I16(0x0A)
#define CF_I_REGB_W		REG_I16(0x0B)
#define CF_I_REGC_W		REG_I16(0x0C)
#define CF_I_REGD_W		REG_I16(0x0D)
#define CF_I_REGE_W		REG_I16(0x0E)
#define CF_I_REGF_W		REG_I16(0x0F)




//Register alias name.

// General registers in 8bit mode.
#define CF_I_DATA		CF_I_REG8	/* Data Port */
#define CF_I_COMMAND	CF_I_REGF	/* write for Command Port */
#define CF_I_STATUS		CF_I_REGF	/* read for Status Port */
// Parameter
#define CF_I_PARAM0		CF_I_REG9	/* Parameter [ 7: 0] */
#define CF_I_PARAM1		CF_I_REGA	/* Parameter [15: 8] */
#define CF_I_PARAM2		CF_I_REGB	/* Parameter [23:16] */
#define CF_I_PARAM3		CF_I_REGC	/* Parameter [31:24] */
#define CF_I_PARAM4		CF_I_REGD	/* Parameter [39:32] */
#define CF_I_PARAM5		CF_I_REGE	/* Parameter [47:40] */
// Address
#define CF_I_ADDR0		CF_I_REG9	/* Address [ 7: 0] */
#define CF_I_ADDR1		CF_I_REGA	/* Address [15: 8] */
#define CF_I_ADDR2		CF_I_REGB	/* Address [23:16] */
#define CF_I_ADDR3		CF_I_REGC	/* Address [31:24] */
// Length
#define CF_I_LEN0		CF_I_REGD	/* Transfer Count [ 7: 0] */
#define CF_I_LEN1		CF_I_REGE	/* Transfer Count [15: 8] */
#define CF_I_LEN2		CF_I_REG6	/* Transfer Count [23:16] */

//16 bits registers alias name not list here.

#endif

/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2005 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/

/****************************************************************************
 *
 * FILENAME
 *		$RCSfile: HICIndep.h,v $
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
 *     $Log: HICIndep.h,v $
 *     Revision 1.1  2006/01/17 09:42:12  xhchen
 *     Add B.B. testing applications.
 *
 *     Revision 1.1.2.7  2005/08/30 04:14:39  xhchen
 *     Makefile and create_project.js support searching dependence header file automatically.
 *     Add ywsun's filebrowser demo (for virtual com).
 *     Add audio test in "FlowTest".
 *
 *     Revision 1.1.2.6  2005/08/16 11:04:06  xhchen
 *     BMP2JPG.
 *     Powersaving.
 *
 *     Revision 1.1.2.5  2005/07/21 07:46:57  xhchen
 *     LCM bypass in index mode is OK.
 *
 *     Revision 1.1.2.4  2005/07/15 08:20:07  xhchen
 *     Use PIPE to transfer FIFO instead of callback functions.
 *
 *     Revision 1.1.2.3  2005/07/04 11:04:36  xhchen
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
 *     Revision 1.1.2.2  2004/12/22 09:46:01  xhchen
 *     ...
 *
 *
 * REMARK
 *     None
 *
 **************************************************************************/



#ifndef __HICINDEP_H__
#define __HICINDEP_H__



#if defined(_HIC_MODE_I8_) || defined(_HIC_MODE_I16_) || defined(_HIC_MODE_I9_) || defined(_HIC_MODE_I18_)
#	define REG_8(x)	REG_I8(x)
#	define REG_16(x)	REG_I16(x)
#	define REG_32(x)	REG_I32(x)
#elif defined(_HIC_MODE_A8_) || defined(_HIC_MODE_A16_) || defined(_HIC_MODE_A9_) || defined(_HIC_MODE_A18_)
#	define REG_8(x)	REG_A8(x)
#	define REG_16(x)	REG_A16(x)
#	define REG_32(x)	REG_A32(x)
#else
#	error "No HIC mode defined."
#endif


//Registers.
#define CF_REG0		REG_8(0x00)
#define CF_REG1		REG_8(0x01)
#define CF_REG2		REG_8(0x02)
#define CF_REG3		REG_8(0x03)
#define CF_REG4		REG_8(0x04)
#define CF_REG5		REG_8(0x05)
#define CF_REG6		REG_8(0x06)
#define CF_REG7		REG_8(0x07)
#define CF_REG8		REG_8(0x08)
#define CF_REG9		REG_8(0x09)
#define CF_REGA		REG_8(0x0A)
#define CF_REGB		REG_8(0x0B)
#define CF_REGC		REG_8(0x0C)
#define CF_REGD		REG_8(0x0D)
#define CF_REGE		REG_8(0x0E)
#define CF_REGF		REG_8(0x0F)


#define CF_REG0_W	REG_16(0x00)
#define CF_REG1_W	REG_16(0x01)
#define CF_REG2_W	REG_16(0x02)
#define CF_REG3_W	REG_16(0x03)
#define CF_REG4_W	REG_16(0x04)
#define CF_REG5_W	REG_16(0x05)
#define CF_REG6_W	REG_16(0x06)
#define CF_REG7_W	REG_16(0x07)
#define CF_REG8_W	REG_16(0x08)
#define CF_REG9_W	REG_16(0x09)
#define CF_REGA_W	REG_16(0x0A)
#define CF_REGB_W	REG_16(0x0B)
#define CF_REGC_W	REG_16(0x0C)
#define CF_REGD_W	REG_16(0x0D)
#define CF_REGE_W	REG_16(0x0E)
#define CF_REGF_W	REG_16(0x0F)


#define CF_REG0_D	REG_32(0x00)
#define CF_REG1_D	REG_32(0x01)
#define CF_REG2_D	REG_32(0x02)
#define CF_REG3_D	REG_32(0x03)
#define CF_REG4_D	REG_32(0x04)
#define CF_REG5_D	REG_32(0x05)
#define CF_REG6_D	REG_32(0x06)
#define CF_REG7_D	REG_32(0x07)
#define CF_REG8_D	REG_32(0x08)
#define CF_REG9_D	REG_32(0x09)
#define CF_REGA_D	REG_32(0x0A)
#define CF_REGB_D	REG_32(0x0B)
#define CF_REGC_D	REG_32(0x0C)
#define CF_REGD_D	REG_32(0x0D)
#define CF_REGE_D	REG_32(0x0E)
#define CF_REGF_D	REG_32(0x0F)


//Register alias name.

// General registers in 8bit mode
#define CF_DATA		CF_REG8	/* Data Port */
#define CF_COMMAND	CF_REGF	/* write for Command Port */
#define CF_STATUS	CF_REGF	/* read for Status Port */
// Parameter
#define CF_PARAM0	CF_REG9	/* Parameter [ 7: 0] */
#define CF_PARAM1	CF_REGA	/* Parameter [15: 8] */
#define CF_PARAM2	CF_REGB	/* Parameter [23:16] */
#define CF_PARAM3	CF_REGC	/* Parameter [31:24] */
#define CF_PARAM4	CF_REGD	/* Parameter [39:32] */
#define CF_PARAM5	CF_REGE	/* Parameter [47:40] */
// Address
#define CF_ADDR0	CF_REG9	/* Address [ 7: 0] */
#define CF_ADDR1	CF_REGA	/* Address [15: 8] */
#define CF_ADDR2	CF_REGB	/* Address [23:16] */
#define CF_ADDR3	CF_REGC	/* Address [31:24] */
// Length
#define CF_LEN0		CF_REGD	/* Transfer Count [ 7: 0] */
#define CF_LEN1		CF_REGE	/* Transfer Count [15: 8] */
#define CF_LEN2		CF_REG6	/* Transfer Count [23:16] */
// Moledy Bypass Mode
#define CF_MLDYR	CF_REG4	/* Melody Bypass Mode, A[0] = 0 */
#define CF_MLDYW	CF_REG5	/* Melody Bypass Mode, A[0] = 1 */
// Power saving
#define CF_POWERCON	CF_REG7	/* Power Control Port */
#define CF_GPIOOE	CF_REG9	/* HGPIO Output Enable Control Port */
#define CF_GPIODATA	CF_REGA	/* HGPIO Pins Output Data Port */
#define CF_GPIOIS	CF_REGB	/* HGPIO Pins Input Status Port */
#define CF_GPIOIE	CF_REGC	/* HGPIO Interrupt Enable Control Port */
#define CF_GPIOPE	CF_REGD	/* HGPIO Pull-up & Pull-down Control Port */



// General registers in 16bit mode
#define CF_DATA_W		CF_REG8_W	/* Data Port */
#define CF_COMMAND_W	CF_REGF_W	/* write for Command Port */
#define CF_STATUS_W		CF_REGF_W	/* read for Status Port */
// Parameter
#define CF_PARAM0_W		CF_REG9_W	/* Parameter [ 7: 0] */
#define CF_PARAM1_W		CF_REGA_W	/* Parameter [15: 8] */
#define CF_PARAM2_W		CF_REGB_W	/* Parameter [23:16] */
#define CF_PARAM3_W		CF_REGC_W	/* Parameter [31:24] */
#define CF_PARAM4_W		CF_REGD_W	/* Parameter [39:32] */
#define CF_PARAM5_W		CF_REGE_W	/* Parameter [47:40] */
// Address
#define CF_ADDR0_W		CF_REG9_W	/* Address [ 7: 0] */
#define CF_ADDR1_W		CF_REGA_W	/* Address [15: 8] */
#define CF_ADDR2_W		CF_REGB_W	/* Address [23:16] */
#define CF_ADDR3_W		CF_REGC_W	/* Address [31:24] */
// Length
#define CF_LEN0_W		CF_REGD_W	/* Transfer Count [ 7: 0] */
#define CF_LEN1_W		CF_REGE_W	/* Transfer Count [15: 8] */
#define CF_LEN2_W		CF_REG6_W	/* Transfer Count [23:16] */
// Moledy Bypass Mode
#define CF_MLDYR_W		CF_REG4_W	/* Melody Bypass Mode, A[0] = 0 */
#define CF_MLDYW_W		CF_REG5_W	/* Melody Bypass Mode, A[0] = 1 */
// Power saving
#define CF_POWERCON_W	CF_REG7		/* Power Control Port */
#define CF_GPIOOE_W		CF_REG9		/* HGPIO Output Enable Control Port */
#define CF_GPIODATA_W	CF_REGA		/* HGPIO Pins Output Data Port */
#define CF_GPIOIS_W		CF_REGB		/* HGPIO Pins Input Status Port */
#define CF_GPIOIE_W		CF_REGC		/* HGPIO Interrupt Enable Control Port */
#define CF_GPIOPE_W		CF_REGD		/* HGPIO Pull-up & Pull-down Control Port */




// LCM Bypass Mode
#if defined(_HIC_MODE_I8_) || defined(_HIC_MODE_I16_) || defined(_HIC_MODE_I9_) || defined(_HIC_MODE_I18_)
#	define CF_LCM0_0		REG_LCM_INDEX8_ADDR	/* LCM0 Bypass Mode, A[0] = 0, A[3] = 0*/
#	define CF_LCM0_1		REG_LCM_INDEX8_DATA	/* LCM0 Bypass Mode, A[0] = 1, A[3] = 0 */
#	define CF_LCM1_0		REG_LCM_INDEX8_ADDR	/* LCM0 Bypass Mode, A[0] = 0, A[3] = 0 */
#	define CF_LCM1_1		REG_LCM_INDEX8_DATA	/* LCM0 Bypass Mode, A[0] = 1, A[3] = 0 */

#	define CF_LCM0_0_W		REG_LCM_INDEX16_ADDR	/* LCM0 Bypass Mode, A[0] = 0, A[3] = 0 */
#	define CF_LCM0_1_W		REG_LCM_INDEX16_DATA	/* LCM0 Bypass Mode, A[0] = 1, A[3] = 0 */
#	define CF_LCM1_0_W		REG_LCM_INDEX16_ADDR	/* LCM0 Bypass Mode, A[0] = 0, A[3] = 0 */
#	define CF_LCM1_1_W		REG_LCM_INDEX16_DATA	/* LCM0 Bypass Mode, A[0] = 1, A[3] = 0 */

#	define CF_LCM0_0_D		REG_LCM_INDEX32_ADDR	/* LCM0 Bypass Mode, A[0] = 0, A[3] = 0 */
#	define CF_LCM0_1_D		REG_LCM_INDEX32_DATA	/* LCM0 Bypass Mode, A[0] = 1, A[3] = 0 */
#	define CF_LCM1_0_D		REG_LCM_INDEX32_ADDR	/* LCM0 Bypass Mode, A[0] = 0, A[3] = 0 */
#	define CF_LCM1_1_D		REG_LCM_INDEX32_DATA	/* LCM0 Bypass Mode, A[0] = 1, A[3] = 0 */
#elif defined(_HIC_MODE_A8_) || defined(_HIC_MODE_A16_) || defined(_HIC_MODE_A9_) || defined(_HIC_MODE_A18_)
#	define CF_LCM0_0	CF_REG0	/* LCM0 Bypass Mode, A[0] = 0 */
#	define CF_LCM0_1	CF_REG1	/* LCM0 Bypass Mode, A[0] = 1 */
#	define CF_LCM1_0	CF_REG2	/* LCM0 Bypass Mode, A[0] = 0 */
#	define CF_LCM1_1	CF_REG3	/* LCM0 Bypass Mode, A[0] = 1 */

#	define CF_LCM0_0_W		CF_REG0_W	/* LCM0 Bypass Mode, A[0] = 0 */
#	define CF_LCM0_1_W		CF_REG1_W	/* LCM0 Bypass Mode, A[0] = 1 */
#	define CF_LCM1_0_W		CF_REG2_W	/* LCM0 Bypass Mode, A[0] = 0 */
#	define CF_LCM1_1_W		CF_REG3_W	/* LCM0 Bypass Mode, A[0] = 1 */

#	define CF_LCM0_0_D		CF_REG0_D	/* LCM0 Bypass Mode, A[0] = 0 */
#	define CF_LCM0_1_D		CF_REG1_D	/* LCM0 Bypass Mode, A[0] = 1 */
#	define CF_LCM1_0_D		CF_REG2_D	/* LCM0 Bypass Mode, A[0] = 0 */
#	define CF_LCM1_1_D		CF_REG3_D	/* LCM0 Bypass Mode, A[0] = 1 */
#endif

#endif

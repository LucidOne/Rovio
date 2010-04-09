/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2005 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/

/****************************************************************************
 *
 * FILENAME
 *		$RCSfile: ModeLCMBypass.c,v $
 *
 * VERSION
 *		$Revision: 1.1 $
 *
 * DESCRIPTION
 *		Functions for HIC LCM bypass mode.
 *
 * HISTORY
 *     $Log: ModeLCMBypass.c,v $
 *     Revision 1.1  2006/01/17 09:42:12  xhchen
 *     Add B.B. testing applications.
 *
 *     Revision 1.1.2.17  2005/12/27 01:57:43  xhchen
 *     Fixed an OSD bug.
 *
 *     Revision 1.1.2.16  2002/01/10 22:19:27  xhchen
 *     1. wpackage: Flush cache after firmware is unzipped.
 *     2. Support LCM initialization after downloading firmware.
 *     3. Close some of engine clocks before downloading firmware.
 *     4. Update "Libs/CmdSet/Inc/InInc/FM_VCOMCA.h", the previous
 *        FM_VCOMCA.h has something errors.
 *
 *     Revision 1.1.2.15  2002/01/09 18:57:09  xhchen
 *     FullDemo: break on errors in movie playback.
 *
 *     Revision 1.1.2.14  2005/09/19 10:40:02  xhchen
 *     1. Set to LCM bypass mode by writing REG_MISCR on starting up.
 *     2. Add menu to "Delete dir" in file browser.
 *
 *     Revision 1.1.2.13  2005/09/07 03:32:06  xhchen
 *     1. Update command set API to WBCM_702_0.82pre14.doc.
 *     2. Add the function to playback audio by sending audio bitstream.
 *     3. Add "wbhicRescue()" to clear BUSY, DRQ an ERROR.
 *     4. Merget WYSun's testing code to "Samples/FlowTest".
 *
 *     Revision 1.1.2.12  2005/08/30 04:14:39  xhchen
 *     Makefile and create_project.js support searching dependence header file automatically.
 *     Add ywsun's filebrowser demo (for virtual com).
 *     Add audio test in "FlowTest".
 *
 *     Revision 1.1.2.11  2005/08/25 05:41:14  xhchen
 *     Add Virtual com to FullDemo.
 *     Begin to add FlowTest, a testing application almost for all functions.
 *
 *     Revision 1.1.2.10  2005/08/16 11:04:06  xhchen
 *     BMP2JPG.
 *     Powersaving.
 *
 *     Revision 1.1.2.9  2005/07/21 07:52:42  xhchen
 *     ...
 *
 *     Revision 1.1.2.8  2005/07/15 08:20:08  xhchen
 *     Use PIPE to transfer FIFO instead of callback functions.
 *
 *     Revision 1.1.2.7  2005/07/04 11:04:36  xhchen
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
 *     Revision 1.1.2.6  2005/06/24 11:19:19  xhchen
 *     Fixed big endian problem.
 *     FIFO can use a function as it's R/W data now.
 *
 *     Revision 1.1.2.5  2005/06/01 07:43:24  xhchen
 *     Comic and burst added.
 *
 *     Revision 1.1.2.4  2005/02/02 07:42:38  xhchen
 *     Microwindows porting is OK.
 *
 *     Revision 1.1.2.3  2004/12/22 09:46:01  xhchen
 *     ...
 *
 *
 * REMARK
 *     None
 *
 **************************************************************************/



#include "../../Platform/Inc/Platform.h"
#include "../../SoftPipe/include/softpipe.h"
#include "../Inc/HIC.h"


#ifdef _LCM_16BIT

/****************************************************************************
 *
 * FUNCTION
 *		wbhicLCDWriteAddr
 *
 * DESCRIPTION
 *		Write LCD register address.
 *
 * INPUTS
 *		nLCD: LCD index, 0 for LCD0 and 1 for LCD1
 *		usCmd: Register address
 *
 * OUTPUTS
 *		None
 *
 * RETURN
 *		None
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
VOID wbhicLCDWriteAddr(INT nLCD, USHORT usCmd)
{
	ASSERT(nLCD == 0 || nLCD == 1);

	if (nLCD == 0)
	{
		CF_LCM0_0_W = usCmd;
	}
	else if (nLCD == 1)
	{
		CF_LCM1_0_W = usCmd;
	}
}




/****************************************************************************
 *
 * FUNCTION
 *		wbhicLCDWriteAddr
 *
 * DESCRIPTION
 *		Write LCD register data.
 *
 * INPUTS
 *		nLCD: LCD index, 0 for LCD0 and 1 for LCD1
 *		usCmd: Register data
 *
 * OUTPUTS
 *		None
 *
 * RETURN
 *		None
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
VOID wbhicLCDWriteReg(INT nLCD, USHORT usCmd)
{
	ASSERT(nLCD == 0 || nLCD == 1);
	if (nLCD == 0)
	{
		CF_LCM0_1_W = usCmd;
	}
	else if (nLCD == 1)
	{
		CF_LCM1_1_W = usCmd;
	}
}



/****************************************************************************
 *
 * FUNCTION
 *		wbhicLCDWriteRGB565
 *
 * DESCRIPTION
 *		Draw a RGB565 pixel on LCD.
 *
 * INPUTS
 *		nLCD: LCD index, 0 for LCD0 and 1 for LCD1
 *		usColor: The pixel
 *
 * OUTPUTS
 *		None
 *
 * RETURN
 *		None
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
VOID wbhicLCDWriteRGB565(INT nLCD, USHORT usColor)
{
	wbhicLCDWriteReg(nLCD, usColor);
}


/****************************************************************************
 *
 * FUNCTION
 *		wbhicLCDWriteRGB888
 *
 * DESCRIPTION
 *		Draw a RGB888 pixel on LCD.
 *
 * INPUTS
 *		nLCD: LCD index, 0 for LCD0 and 1 for LCD1
 *		ucR: Red in the pixel
 *		ucG: Green in the pixel
 *		ucB: Blue in the pixel
 *
 * OUTPUTS
 *		None
 *
 * RETURN
 *		None
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
VOID wbhicLCDWriteRGB888(INT nLCD, UCHAR ucR, UCHAR ucG, UCHAR ucB)
{
	USHORT usColor = (USHORT) ucB >> 3;
	usColor |= (USHORT) ucG >> 2 << 5;
	usColor |= (USHORT) ucR >> 3 << 11;

	wbhicLCDWriteReg (nLCD, usColor);
}

#endif



#ifdef _LCM_18BIT
/* See description in _LCM_16BIT. */
VOID wbhicLCDWriteAddr(INT nLCD, USHORT usCmd)
{
	UINT32 uCmdShift;
	ASSERT(nLCD == 0 || nLCD == 1);

	//usCmd = (usCmd >> 8) | (usCmd << 8);

	uCmdShift = (usCmd & 0xFF00);
	uCmdShift <<= 2;
	uCmdShift |= ((usCmd & 0x00FF) << 1);

	if (nLCD == 0)
	{
		CF_LCM0_0_D = uCmdShift;
	}
	else if (nLCD == 1)
	{
		CF_LCM1_0_D = uCmdShift;
	}
}


/* See description in _LCM_16BIT. */
VOID wbhicLCDWriteReg(INT nLCD, USHORT usCmd)
{
	UINT32 uCmdShift;
	ASSERT(nLCD == 0 || nLCD == 1);

	//usCmd = (usCmd >> 8) | (usCmd << 8);

	uCmdShift = (usCmd & 0xFF00);
	uCmdShift <<= 2;
	uCmdShift |= ((usCmd & 0x00FF) << 1);

	if (nLCD == 0)
	{
		CF_LCM0_1_D = uCmdShift;
	}
	else if (nLCD == 1)
	{
		CF_LCM1_1_D = uCmdShift;
	}
}


/* See description in _LCM_16BIT. */
VOID wbhicLCDWriteRGB565(INT nLCD, USHORT usColor)
{
	UINT32 uDataH = ((usColor & 0xF800) >> 7) | ((usColor & 0x0700) >> 8);
	UINT32 uDataL = (usColor & 0xFF) << 1;

	UINT32 uData = (uDataH << 9) | uDataL;

	if (nLCD == 0)
	{
		CF_LCM0_1_D = uData;
	}
	else if (nLCD == 1)
	{
		CF_LCM1_1_D = uData;
	}
}


VOID wbhicLCDWriteRGB888(INT nLCD, UCHAR ucR, UCHAR ucG, UCHAR ucB)
{
	UINT32 uDataH = (((UINT32) ucR) >> 2 << 3) | (((UINT32) ucG) >> 5);
	UINT32 uDataL = (((UINT32) ucG) >> 2 << 6) | (((UINT32) ucB) >> 2);

	UINT32 uData = (uDataH << 9) | uDataL;

	if (nLCD == 0)
	{
		CF_LCM0_1_D = uData;
	}
	else if (nLCD == 1)
	{
		CF_LCM1_1_D = uData;
	}
}


#endif


#ifdef _LCM_18BIT_HW16
/* See description in _LCM_16BIT. */
VOID wbhicLCDWriteAddr(INT nLCD, USHORT usCmd)
{
	ASSERT(nLCD == 0 || nLCD == 1);

	if (nLCD == 0)
	{
		CF_LCM0_0_W = usCmd;
	}
	else if (nLCD == 1)
	{
		CF_LCM1_0_W = usCmd;
	}
}


/* See description in _LCM_16BIT. */
VOID wbhicLCDWriteReg(INT nLCD, USHORT usCmd)
{
	ASSERT(nLCD == 0 || nLCD == 1);

	if (nLCD == 0)
	{
		CF_LCM0_1_W = usCmd;
	}
	else if (nLCD == 1)
	{
		CF_LCM1_1_W = usCmd;
	}
}


/* See description in _LCM_16BIT. */
VOID wbhicLCDWriteRGB565(INT nLCD, USHORT usColor)
{
	wbhicLCDWriteReg(nLCD, usColor);
}


VOID wbhicLCDWriteRGB888(INT nLCD, UCHAR ucR, UCHAR ucG, UCHAR ucB)
{
	UINT32 uDataH = (((UINT32) ucR) >> 2 << 3) | (((UINT32) ucG) >> 5);
	UINT32 uDataL = (((UINT32) ucG) >> 2 << 6) | (((UINT32) ucB) >> 2);

	UINT32 uData = (uDataH << 9) | uDataL;

	if (nLCD == 0)
	{
		CF_LCM0_1_D = uData;
	}
	else if (nLCD == 1)
	{
		CF_LCM1_1_D = uData;
	}
}



/* Set the way of hardware transfer for LCD data in normal power mode. */
BOOL wbhicLCDDFTrans_PowerNormal (UINT32 uDF_TRANS)
{
	UINT32 uValue;
	UINT32 uNewValue;

	uNewValue = (uDF_TRANS << 16);

	if (! wbhicSingleRead (REG_HICFCR, &uValue))
		return FALSE;

	if ((uValue & 0x00030000) == uNewValue)
		return TRUE;

	uValue &= 0xFFFCFFFF;
	uValue |= uNewValue;
	if (! wbhicSingleWrite (REG_HICFCR, uValue))
		return FALSE;

	return TRUE;
}


/* Set the way of hardware transfer for LCD data in power saving mode. */
BOOL wbhicLCDDFTrans_PowerSaving (UINT32 uDF_TRANS)
{
	UCHAR ucValue;
	UCHAR ucNewValue;

	ucNewValue = (UCHAR) (uDF_TRANS << 6);

	ucValue = CF_POWERCON;
	if ((ucValue & 0xC0) == ucNewValue)
		return TRUE;

	ucValue &= 0x3F;
	ucValue |= ucNewValue;
	CF_POWERCON = ucValue;

	return TRUE;
}


/* The function should be:
		wbhicLCDDFTrans_PowerNormal
	or	wbhicLCDDFTrans_PowerSaving
*/
static BOOL (*g_fnwbhicLCDDFTrans) (UINT32 uDF_TRANS) = wbhicLCDDFTrans_PowerNormal;
VOID wbhicLCDDFTrans_UsePowerSaving (BOOL bPowerSaving)
{
	if (bPowerSaving)
		g_fnwbhicLCDDFTrans = wbhicLCDDFTrans_PowerSaving;
	else
		g_fnwbhicLCDDFTrans = wbhicLCDDFTrans_PowerNormal;
}


BOOL wbhicLCDDFTrans_NONE(VOID)
{
	return (*g_fnwbhicLCDDFTrans) (0x00);
}


BOOL wbhicLCDDFTrans_IR(VOID)
{
	return (*g_fnwbhicLCDDFTrans) (0x02);
}


BOOL wbhicLCDDFTrans_IMG(VOID)
{
	return (*g_fnwbhicLCDDFTrans) (0x03);
}

#endif



#ifdef _LCM_9BIT
/* See description in _LCM_16BIT. */
VOID wbhicLCDWriteAddr(INT nLCD, USHORT usCmd)
{
	USHORT usCmdH, usCmdL;
	usCmdH = (usCmd >> 8) << 1;
	usCmdL = (usCmd & 0xFF) << 1;

	ASSERT(nLCD == 0 || nLCD == 1);

	if (nLCD == 0)
	{
		CF_LCM0_0_W = usCmdH;
		CF_LCM0_0_W = usCmdL;
	}
	else if (nLCD == 1)
	{
		CF_LCM1_0_W = usCmdH;
		CF_LCM1_0_W = usCmdL;
	}
}


/* See description in _LCM_16BIT. */
VOID wbhicLCDWriteReg(INT nLCD, USHORT usCmd)
{
	USHORT usCmdH, usCmdL;
	usCmdH = (usCmd >> 8) << 1;
	usCmdL = (usCmd & 0xFF) << 1;

	ASSERT(nLCD == 0 || nLCD == 1);

	if (nLCD == 0)
	{
		CF_LCM0_1_W = usCmdH;
		CF_LCM0_1_W = usCmdL;
	}
	else if (nLCD == 1)
	{
		CF_LCM1_1_W = usCmdH;
		CF_LCM1_1_W = usCmdL;
	}
}


/* See description in _LCM_16BIT. */
VOID wbhicLCDWriteRGB565(INT nLCD, USHORT usColor)
{
	USHORT usDataH, usDataL;
	usDataH = ((usColor & 0xF800) >> 7) | ((usColor & 0x0700) >> 8);
	usDataL = (usColor & 0xFF) << 1;

	if (nLCD == 0)
	{
		CF_LCM0_1_W = usDataH;
		CF_LCM0_1_W = usDataL;
	}
	else if (nLCD == 1)
	{
		CF_LCM1_1_W = usDataH;
		CF_LCM1_1_W = usDataL;
	}
}


/* See description in _LCM_16BIT. */
VOID wbhicLCDWriteRGB888(INT nLCD, UCHAR ucR, UCHAR ucG, UCHAR ucB)
{
	USHORT usDataH, usDataL;
	usDataH = (((USHORT) ucR) >> 2 << 3) | (((USHORT) ucG) >> 5);
	usDataL = (((USHORT) ucG) >> 2 << 6) | (((USHORT) ucB) >> 2);

	if (nLCD == 0)
	{
		CF_LCM0_1_W = usDataH;
		CF_LCM0_1_W = usDataL;
	}
	else if (nLCD == 1)
	{
		CF_LCM1_1_W = usDataH;
		CF_LCM1_1_W = usDataL;
	}
}

#endif


/****************************************************************************
 *
 * FUNCTION
 *		wbhicLCDWrite
 *
 * DESCRIPTION
 *		Write LCD register.
 *
 * INPUTS
 *		nLCD: LCD index, 0 for LCD0 and 1 for LCD1
 *		usAddr: Register address
 *		usReg: Register value
 *
 * OUTPUTS
 *		None
 *
 * RETURN
 *		None
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
VOID wbhicLCDWrite(INT nLCD, USHORT usAddr, USHORT usReg)
{
	wbhicLCDWriteAddr(nLCD, usAddr);
	wbhicLCDWriteReg(nLCD, usReg);
}


#if defined(_HIC_MODE_I8_) || defined(_HIC_MODE_I16_) || defined(_HIC_MODE_I9_) || defined(_HIC_MODE_I18_)
/****************************************************************************
 *
 * FUNCTION
 *		wbhicLCDIndexSelectLCD
 *
 * DESCRIPTION
 *		Select which LCD is to be used. (for index mode)
 *
 * INPUTS
 *		nLCD: LCD index, 0 for LCD0 and 1 for LCD1
 *
 * RETURN
 *		TRUE: success
 *		FALSE: failed
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
BOOL wbhicLCDIndexSelectLCD(INT nLCD)
{
	UINT32 uValue;
	nLCD = (nLCD == 0 ? 0 : 1);

	if (!wbhicSingleRead (REG_HICFCR, &uValue))
		return FALSE;

	if ((uValue & 0x00001000) == (UINT32) nLCD)
		return TRUE;

	uValue &= 0xFFFFEFFF;
	uValue |= (UINT32) nLCD;
	if (! wbhicSingleWrite (REG_HICFCR, uValue))
		return FALSE;

	return TRUE;
}
#endif


/****************************************************************************
 *
 * FUNCTION
 *		wbhicInitLCDController
 *
 * DESCRIPTION
 *		Init W99702's LCD controller.
 *
 * OUTPUTS
 *		pRegs: registers that's changed for LCM bypass
 *
 * RETURN
 *		TRUE: success
 *		FALSE: failed
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
BOOL wbhicInitLCDController (WBHIC_LCD_REGS *pRegs)
{
	UINT32 uValue;

	/* <1> Enable video clock and HCLK2 clock in W99702 chip. */
	if (!wbhicSingleRead(REG_CLKCON, &uValue))
		return FALSE;
	if (pRegs != NULL)
		pRegs->u_REG_CLKCON = uValue;

	if (!wbhicSingleWrite(REG_CLKCON, uValue | VAL_VCLK_EN | VAL_HCLK2_EN))
		return FALSE;


	/* <2> Set LCM access bitwidth. */
	
	if (pRegs != NULL
		&& !wbhicSingleRead(REG_LCM_DCCS, &pRegs->u_REG_LCM_DCCS))
			return FALSE;
	if (!wbhicSingleWrite(REG_LCM_DCCS, 0x08))
		return FALSE;

	if (pRegs != NULL
		&& !wbhicSingleRead(REG_LCM_DEV_CTRL, &pRegs->u_REG_LCM_DEV_CTRL))
			return FALSE;
	if (!
#if defined (_LCM_16BIT)
		//wbhicSingleWrite(REG_LCM_DEV_CTRL, 0x030000e0)	/* Should be 0x010000e0? */
		wbhicSingleWrite(REG_LCM_DEV_CTRL, 0x050000e0)
#elif defined (_LCM_9BIT)
		wbhicSingleWrite(REG_LCM_DEV_CTRL, 0x020000e0)
#elif defined (_LCM_18BIT)
		wbhicSingleWrite(REG_LCM_DEV_CTRL, 0x060000e0)
#elif defined (_LCM_18BIT_HW16)
		wbhicSingleWrite(REG_LCM_DEV_CTRL, 0x060000e0)
#else
#	error "No LCD access bitwidth defined!"
#endif
		)
		return FALSE;

	/* <3> Set to LCM bypass mode. */
	if (pRegs != NULL
		&& !wbhicSingleRead(REG_MISCR, &pRegs->u_REG_MISCR))
			return FALSE;
	if (!wbhicSingleWrite (REG_MISCR, 0x02))
		return FALSE;

	return TRUE;
}


/****************************************************************************
 *
 * FUNCTION
 *		wbhicRestoreLCDController
 *
 * DESCRIPTION
 *		Init W99702's LCD controller.
 *
 * INPUTS
 *		pRegs: the registers to be restored.
 *
 * RETURN
 *		TRUE: success
 *		FALSE: failed
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
BOOL wbhicRestoreLCDController (CONST WBHIC_LCD_REGS *pRegs)
{
	if (pRegs == NULL)
		return FALSE;

	if (!wbhicSingleWrite (REG_MISCR, pRegs->u_REG_MISCR))
		return FALSE;
	if (!wbhicSingleWrite (REG_LCM_DEV_CTRL, pRegs->u_REG_LCM_DEV_CTRL))
		return FALSE;
	if (!wbhicSingleWrite (REG_LCM_DCCS, pRegs->u_REG_LCM_DCCS))
		return FALSE;
	if (!wbhicSingleWrite (REG_CLKCON, pRegs->u_REG_CLKCON))
		return FALSE;
	return TRUE;
}

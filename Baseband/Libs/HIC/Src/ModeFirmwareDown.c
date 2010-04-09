/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2005 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *		$RCSfile: ModeFirmwareDown.c,v $
 *
 * VERSION
 *		$Revision: 1.1 $
 *
 * DESCRIPTION
 *		Functions for HIC firmware download mode.
 *
 * HISTORY
 *     $Log: ModeFirmwareDown.c,v $
 *     Revision 1.1  2006/01/17 09:42:12  xhchen
 *     Add B.B. testing applications.
 *
 *     Revision 1.1.2.23  2002/01/16 18:43:29  xhchen
 *     1. Add wbhPlaybackMovie_StartPause.
 *     2. Add wbcGetMaxZoomRatio.
 *     3. Adjust JPG playback size.
 *
 *     Revision 1.1.2.22  2002/01/10 22:19:27  xhchen
 *     1. wpackage: Flush cache after firmware is unzipped.
 *     2. Support LCM initialization after downloading firmware.
 *     3. Close some of engine clocks before downloading firmware.
 *     4. Update "Libs/CmdSet/Inc/InInc/FM_VCOMCA.h", the previous
 *        FM_VCOMCA.h has something errors.
 *
 *     Revision 1.1.2.21  2005/09/29 05:40:08  xhchen
 *     To improve the data moving performance,
 *     Move CLOCK settings from wpackage.c to wb_init.s
 *
 *     Revision 1.1.2.20  2005/09/28 10:11:01  xhchen
 *     wpackage supports W99702 and W99802 now.
 *     Use zipped firmware.
 *
 *     Revision 1.1.2.19  2005/09/15 11:08:46  xhchen
 *     Update firmware.
 *     PCCam added.
 *
 *     Revision 1.1.2.18  2005/09/13 08:41:48  xhchen
 *     1. Mass storage demo added to FullDemo.
 *     2. wbhPlaybackJPEG/wbhPlaybackFile keeps the width/height proportion.
 *     3. Add wbSetLastError/wbGetLastError.
 *
 *     Revision 1.1.2.17  2005/09/07 03:32:06  xhchen
 *     1. Update command set API to WBCM_702_0.82pre14.doc.
 *     2. Add the function to playback audio by sending audio bitstream.
 *     3. Add "wbhicRescue()" to clear BUSY, DRQ an ERROR.
 *     4. Merget WYSun's testing code to "Samples/FlowTest".
 *
 *     Revision 1.1.2.16  2005/09/01 10:03:58  xhchen
 *     Add OSD and suspend test to FlowTest.
 *
 *     Revision 1.1.2.15  2005/08/25 05:41:14  xhchen
 *     Add Virtual com to FullDemo.
 *     Begin to add FlowTest, a testing application almost for all functions.
 *
 *     Revision 1.1.2.14  2005/07/21 07:46:57  xhchen
 *     LCM bypass in index mode is OK.
 *
 *     Revision 1.1.2.13  2005/07/20 09:18:58  xhchen
 *     1. soft pipe: remove fnTrData, use fnOnData only.
 *     2. Movie record: check status after record stopped,
 *        add parameter to set volume, bitrate and size when recording.
 *        Fix a bug when recording movie: OSD did not properly been shown.
 *
 *     Revision 1.1.2.12  2005/07/15 08:20:08  xhchen
 *     Use PIPE to transfer FIFO instead of callback functions.
 *
 *     Revision 1.1.2.11  2005/06/24 11:19:19  xhchen
 *     Fixed big endian problem.
 *     FIFO can use a function as it's R/W data now.
 *
 *     Revision 1.1.2.10  2005/06/01 07:43:24  xhchen
 *     Comic and burst added.
 *
 *     Revision 1.1.2.9  2005/05/25 06:27:28  xhchen
 *     Move clock setting code to the beginning of downloading firmware.
 *
 *     Revision 1.1.2.8  2005/05/10 10:14:32  xhchen
 *     Stable for W99702B.
 *
 *     Revision 1.1.2.7  2005/04/19 02:17:30  xhchen
 *     Fix font bugs.
 *     Use new firmware for new W99702 chip.
 *
 *     Revision 1.1.2.6  2005/03/31 12:35:37  xhchen
 *     Some FIFO I/O method added for comparing performance.
 *
 *     Revision 1.1.2.5  2005/03/30 07:57:47  xhchen
 *     1st Release on 03302005.
 *
 *     Revision 1.1.2.4  2005/03/11 09:26:23  xhchen
 *     Check in after a long period of time because the harddisk on 10.130.249.103 can not be used for some unknown reason.
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


BOOL wbhicDownloadFirmwareEx (CONST UCHAR *pucBinCode,
							  UINT32 uTargetSize,
							  UINT32 uTargetAddr,
							  BOOL bVerify,
							  BOOL bSetBusyOnBoot)
{
	sysPrintf ("Time (Stop CPU):       %d\n", sysGetTickCount ());

	//<1> Halt CPU
	if (!wbhicSingleWrite(REG_CPUCR, CPUCR_CPURST))
	{
		D_ERROR ("");
		return FALSE;
	}

	//<2> Set clock.
	{
		UINT32 uValue;
		if (!wbhicSingleRead (REG_CLKCON, &uValue))
		{
			D_ERROR ("");
			return FALSE;
		}
		/*
		 * VCLK and HCLK2 is for video (LCM), do not change it.
		 * others engines's clock is to be closed for safety.
		 */
		uValue &= VAL_VCLK_EN | VAL_HCLK2_EN | VAL_CLK_RESERVED;
		/* Make sure that CPUCLK and APBCLK is enabled. */
		uValue |= VAL_CPUCLK_EN | VAL_APBCLK_EN;
		sysPrintf ("uValue = %08x\n", uValue);
		if (!wbhicSingleWrite (REG_CLKCON, uValue))
		{
			D_ERROR ("");
			return FALSE;
		}
	}
	if (!wbhicSingleWrite (REG_UPLLCON, 0x16220))
	{
		D_ERROR ("");
		return FALSE;
	}
	if (!wbhicSingleWrite (REG_CLKSEL, 0x304))
	{
		D_ERROR ("");
		return FALSE;
	}

	//<3> disable AIC
	if (!wbhicSingleWrite(REG_AIC_MDCR, 0xFFFFFFFF))
	{
		D_ERROR ("");
		return FALSE;
	}
	
	//<4> Insert a jump instruction at address 0
	if (uTargetAddr != 0)
	{
		/* After reset CPU by writing REG_CPUCR,
		   PC always points to address 0.
		   Some firmware may not be designed to execute from address 0.
		   To execute the firmware, we must write a JUMP instruction at address 0:
		   		B uTargetAddress
		*/
		UINT32 u_JUMP_CODE = (0xEA000000 | (((uTargetAddr >> 2) - 2) & 0x00FFFFFF));
		if (!wbhicSingleWrite (0, u_JUMP_CODE))
		{
			D_ERROR ("");
			return FALSE;
		}
	}
		
	//<5> BurstWrite to download
	if (!wbhicBurstWrite(uTargetAddr, uTargetSize, (VOID *) pucBinCode))
	{
		D_ERROR ("");
		return FALSE;
	}
	// SingleWrite to download
	//UINT32 i;
	//for (i = 0; i < (uTargetSize + 3) / 4; i += 4)
	//{
	//	UINT32 uCode;
	//	UCHAR_2_UINT32 (pucBinCode + i, uCode);
	//	wbhicSingleWrite(i, uCode);
	//}
	

	//<6> Verify the firmware by reading.
	if (bVerify)
	{
		UCHAR aucBinCode[128];
		UINT32 i;
		
		for (i = 0; i < uTargetSize; i += sizeof (aucBinCode))
		{
			UINT32 uThisSize = uTargetSize - i;
			if (uThisSize > sizeof (aucBinCode))
				uThisSize = sizeof (aucBinCode);

			if (!wbhicBurstRead(uTargetAddr + i, uThisSize, aucBinCode))
			{
				D_ERROR ("");
				return FALSE;
			}

			if (memcmp(pucBinCode + i, aucBinCode, uThisSize) != 0)
			{
				UINT32 j;		
				for (j = 0; j < uThisSize; j++)
				{
					if (pucBinCode[i] != aucBinCode[j])
						sysPrintf("DATA %x: %x %x\n", i + j, (int) pucBinCode[i + j], (int) aucBinCode[j]);
				}
				sysPrintf ("Compare failed, data not match.\n");
				return FALSE;
			}
		}
		sysPrintf("Compare... OK\n");
	}


	//<7> RESET and REBOOT
	if (!bSetBusyOnBoot)
	{
		if (!wbhicSingleWrite (REG_CPUCR, 0))
		{
			D_ERROR ("");
			return FALSE;
		}
	}
	else
	{
		BOOL bOK;
		/* wbhicSpecialWrite write register and set HIC ERROR bit,
		   while wbhicSingleWrite do not set HIC ERROR bit. */
		if (!wbhicSpecialWrite(REG_CPUCR, 0))
		{
			D_ERROR ("");
			return FALSE;
		}

		sysPrintf ("Time (Start CPU):      %d\n", sysGetTickCount ());
		WB_COND_WAITFOR (((CF_STATUS & HICST_BUSY) == 0x00), CF_TIMEOUT_MSEC, bOK);
		if (!bOK)
		{
			D_ERROR("BUSY bit not cleared.\n");
			wbSetLastError (HIC_ERR_ST_BUSY);
			wbhicRescue ();
			return FALSE;
		}
		
		sysPrintf ("Time (BUSY cleared):   %d\n", sysGetTickCount ());
	}
	
	sysPrintf("Download firmware OK.\n");
	return TRUE;
}
							  
							  



BOOL wbhicDownloadFirmware(CONST UCHAR *pucBinCode,
						   UINT32 uTargetSize)
{

#define FIRMWARE_ADDR	0x000000
//#define FIRMWARE_ADDR	0x120000
//#define FIRMWARE_ADDR	0x4C0000

	return wbhicDownloadFirmwareEx (pucBinCode, uTargetSize, FIRMWARE_ADDR, FALSE, TRUE);
}








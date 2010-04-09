/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2005 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/

/****************************************************************************
 *
 * FILENAME
 *		$RCSfile: HIC.h,v $
 *
 * VERSION
 *		$Revision: 1.4 $
 *
 * DESCRIPTION
 *		W99702 hardware control interface.
 *
 * DATA STRUCTURES
 * 		Not list here.
 *
 * FUNCTIONS
 *		None
 *
 * HISTORY
 *     $Log: HIC.h,v $
 *     Revision 1.4  2006/03/18 15:12:10  xhchen
 *     Test MP4 loopback by BB, OK.
 *     Add checksum for FIFO in protocol command.
 *
 *     Revision 1.3  2006/02/22 14:05:58  xhchen
 *     1. Serious bug fixed: old thread code (tt_reg_load_in_svc) may
 *        use too large stack, which may cause a fatal error.
 *     2. Move the allocate function (for large buffer) to a single file.
 *     3. Add capture thread. "capture thread" captures video, converts
 *        video by VPE and draws video by LCM.
 *     4. Test encode/decode amr, OK.
 *     5. Add recursive mutex (a lock that can not be locked by the
 *        thread that owns the lock itself.)
 *     6. Test video capture, OK. (dsplib, i2clib)
 *
 *     Revision 1.2  2006/02/14 08:56:48  xhchen
 *     1. Protocol commands support OSD is OK.
 *     2. Bug fixed: memory may be lost if system clock is from crystal.
 *        (why?)
 *     3. Bug fixed: if vpost clock is enabled after HIC interrupt is
 *        received, vpost can not refresh LCM. (why?)
 *
 *     Revision 1.1  2006/01/17 09:42:11  xhchen
 *     Add B.B. testing applications.
 *
 *     Revision 1.1.2.26  2005/12/27 01:57:43  xhchen
 *     Fixed an OSD bug.
 *
 *     Revision 1.1.2.25  2002/01/31 15:06:39  xhchen
 *     Add media parsing functions to High Level API.
 *
 *     Revision 1.1.2.24  2002/01/10 22:19:27  xhchen
 *     1. wpackage: Flush cache after firmware is unzipped.
 *     2. Support LCM initialization after downloading firmware.
 *     3. Close some of engine clocks before downloading firmware.
 *     4. Update "Libs/CmdSet/Inc/InInc/FM_VCOMCA.h", the previous
 *        FM_VCOMCA.h has something errors.
 *
 *     Revision 1.1.2.23  2002/01/09 18:57:09  xhchen
 *     FullDemo: break on errors in movie playback.
 *
 *     Revision 1.1.2.22  2005/09/13 08:41:48  xhchen
 *     1. Mass storage demo added to FullDemo.
 *     2. wbhPlaybackJPEG/wbhPlaybackFile keeps the width/height proportion.
 *     3. Add wbSetLastError/wbGetLastError.
 *
 *     Revision 1.1.2.21  2005/09/07 03:32:06  xhchen
 *     1. Update command set API to WBCM_702_0.82pre14.doc.
 *     2. Add the function to playback audio by sending audio bitstream.
 *     3. Add "wbhicRescue()" to clear BUSY, DRQ an ERROR.
 *     4. Merget WYSun's testing code to "Samples/FlowTest".
 *
 *     Revision 1.1.2.20  2005/08/25 05:41:14  xhchen
 *     Add Virtual com to FullDemo.
 *     Begin to add FlowTest, a testing application almost for all functions.
 *
 *     Revision 1.1.2.19  2005/08/16 11:04:06  xhchen
 *     BMP2JPG.
 *     Powersaving.
 *
 *     Revision 1.1.2.18  2005/07/22 10:29:07  xhchen
 *     Nested find OK.
 *
 *     Revision 1.1.2.17  2005/07/21 07:46:57  xhchen
 *     LCM bypass in index mode is OK.
 *
 *     Revision 1.1.2.16  2005/07/20 09:18:58  xhchen
 *     1. soft pipe: remove fnTrData, use fnOnData only.
 *     2. Movie record: check status after record stopped,
 *        add parameter to set volume, bitrate and size when recording.
 *        Fix a bug when recording movie: OSD did not properly been shown.
 *
 *     Revision 1.1.2.15  2005/07/15 08:20:07  xhchen
 *     Use PIPE to transfer FIFO instead of callback functions.
 *
 *     Revision 1.1.2.14  2005/07/04 11:04:36  xhchen
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
 *     Revision 1.1.2.13  2005/06/24 11:19:19  xhchen
 *     Fixed big endian problem.
 *     FIFO can use a function as it's R/W data now.
 *
 *     Revision 1.1.2.12  2005/06/01 07:43:24  xhchen
 *     Comic and burst added.
 *
 *     Revision 1.1.2.11  2005/05/16 08:33:27  xhchen
 *         1. Add HIC debug function EnableHICDebug(BOOL bEnable). When HIC debug is on, almost all the HIC I/O operation will be printed to console.
 *     	2. Command set API according to "WBCM_702_0.8.pdf".
 *         3. Add code to check card before playback/record.
 *     	4. MP3 playback window:
 *     	   a. Add a spectrum chart on the window
 *     	   b. Display ID3 tags (artist, album & genre) according to the "Language" settings.
 *     	   c. Add code to setting equalizer by users.
 *     	   d. Continue to play next audio file after current file is over.
 *
 *     Revision 1.1.2.10  2005/05/12 06:39:51  xhchen
 *     Update to CamMod_MT9M111_WM8978_050510_3.bin.
 *
 *     Revision 1.1.2.9  2005/05/12 03:36:48  xhchen
 *     Fix LoadFromMemoryCard bugs.
 *     Add 3GP demo.
 *
 *     Revision 1.1.2.8  2005/05/10 10:14:32  xhchen
 *     Stable for W99702B.
 *
 *     Revision 1.1.2.7  2005/04/01 02:14:58  xhchen
 *     More effective FIFO I/O OK.
 *
 *     Revision 1.1.2.6  2005/03/31 12:35:37  xhchen
 *     Some FIFO I/O method added for comparing performance.
 *
 *     Revision 1.1.2.5  2005/03/11 09:26:23  xhchen
 *     Check in after a long period of time because the harddisk on 10.130.249.103 can not be used for some unknown reason.
 *
 *     Revision 1.1.2.4  2005/02/02 07:42:38  xhchen
 *     Microwindows porting is OK.
 *
 *     Revision 1.1.2.3  2005/01/11 09:42:01  xhchen
 *     Bug fixed: FIFO write in 16bit mode.
 *     Verify YHTu's new firmware.
 *
 *     Revision 1.1.2.2  2004/12/22 09:46:01  xhchen
 *     ...
 *
 *
 * REMARK
 *     None
 *
 **************************************************************************/



#ifndef __HIC_H__
#define __HIC_H__

#define DISABLE_GDMA

//#define _HIC_MODE_I8_
//#define _HIC_MODE_I16_
//#define _HIC_MODE_I9_
//#define _HIC_MODE_I18_
//#define _HIC_MODE_A8_
#define _HIC_MODE_A16_
//#define _HIC_MODE_A9_
//#define _HIC_MODE_A18_


//#define _LCM_16BIT
//#define _LCM_18BIT
//#define _LCM_9BIT
#define _LCM_18BIT_HW16


#define CF_TIMEOUT_MSEC				2000000UL


#include "./InInc/HICIndex.h"
#include "./InInc/HICAddr.h"
#include "./InInc/HICIndep.h"
#include "./InInc/w99702_reg.h"
#include "./InInc/wberrcode.h"
#include "./InInc/HICErrorCode.h"
#include "./InInc/HICLocal.h"
#include "./InInc/HIC_FIFO.h"






typedef struct
{
	UINT32 uSizeTotal;
	UINT32 uSizeOK;
	USHORT usHalf;
} WBHIC_FIFO_T;


typedef struct
{
	SOFT_PIPE_T		pipe;
	WBHIC_FIFO_T	wbhicFIFO;
	UINT32			uLength_ToRead;
} WBHIC_FIFO_PIPE_T;

typedef struct
{
	UINT32 u_REG_CLKCON;
	UINT32 u_REG_LCM_DCCS;
	UINT32 u_REG_LCM_DEV_CTRL;
	UINT32 u_REG_MISCR;
} WBHIC_LCD_REGS;	/* Registers need to be changed before LCM bypass. */


#define GetParentAddr(pMe,tParent,eMyName) \
	(tParent *)((char *)(pMe) - (int)&((tParent *)0)->eMyName)



/* HIC initialization, change HIC access mode (index/address mode, bitwidth). */
BOOL wbhicInit(VOID);

/* Clear BUSY,DRQ & ERROR bit in HIC status register. */
BOOL wbhicRescue (VOID);

/* Is in power saving mode? */
BOOL wbhicIsPowerSavingMode (VOID);
/* Power saving mode. */
BOOL wbhicEnterPowerSavingMode (VOID);
/* Normal power mode. */
BOOL wbhicEnterPowerNormalMode (VOID);


/* HIC FIFO data I/O.

	Basically, data can be transferred with "wbhicReadFIFO" or "wbhicWriteFIFO".


	The limitation is data must be transferred in one function call, because
"wbhicWriteFIFO" (or "wbhicReadFIFO") writes 128 bytes data for each writing cycle
internally. The following code is illegal:

		UCHAR pucBuf1[200];
		UCHAR pucBuf2[200];
		....
		wbhicWriteFIFO(pucBuf1, 200, NULL);
		wbhicWriteFIFO(pucBuf2, 200, NULL);


	The correct way:

		UCHAR pucBuf1[200];
		UCHAR pucBuf2[200];
		UCHAR pucBuf[400];
		....
		memcpy(pucBuf, pucBuf1, 200);
		memcpy(pucBuf + 200, pucBuf2, 200);
		wbhicWriteFIFO(pucBuf, sizeof(pucBuf), NULL);


	Besides copying data to a continous buffer, another resolution is to use
"wbhicPartWriteXXXX" functions.

		UCHAR pucBuf1[200];
		UCHAR pucBuf2[200];
		WBHIC_FIFO_T wbhicFIFO;
		....
		wbhicPartWriteFIFO_Begin(&wbhicFIFO, 400);
		wbhicPartWriteFIFO(&wbhicFIFO, pucBuf1, 200, NULL);
		wbhicPartWriteFIFO(&wbhicFIFO, pucBuf2, 200, NULL);
		wbhicPartWriteFIFO_End(&wbhicFIFO);


	Note that for writing a data buffer, wbhicWriteFIFO(buffer, length, NULL) is equal to:

		wbhicPartWriteFIFO_Begin(&wbhicFIFO, length);
		wbhicPartWriteFIFO(&wbhicFIFO, buffer, length, NULL);
		wbhicPartWriteFIFO_End(&wbhicFIFO);

*/
BOOL wbhicReadFIFO(VOID *pTargetBuffer, UINT32 uBufferType, UINT32 *puSizeRead);
BOOL wbhicWriteFIFO(VOID *pSourceBuffer, UINT32 uBufferType, UINT32 *puSizeWritten);


VOID wbhicPartReadFIFO_Begin(WBHIC_FIFO_T *pwbhicFIFO,
							 UINT32 uSizeTotal);
BOOL wbhicPartReadFIFO(WBHIC_FIFO_T *pwbhicFIFO,
					   UCHAR *pucBuffer,
					   UINT32 uBufferSize,
					   UINT32 *puSizeRead);
BOOL wbhicPartReadFIFO_End(WBHIC_FIFO_T *pwbhicFIFO);


VOID wbhicPartWriteFIFO_Begin(WBHIC_FIFO_T *pwbhicFIFO,
							  UINT32 uSizeTotal);
BOOL wbhicPartWriteFIFO(WBHIC_FIFO_T *pwbhicFIFO,
						CONST UCHAR *pcucBuffer,
						UINT32 uBufferSize,
						UINT32 *puSizeWritten);
BOOL wbhicPartWriteFIFO_End(WBHIC_FIFO_T *pwbhicFIFO);


VOID wbhicPipeWriteFIFO_Init (WBHIC_FIFO_PIPE_T *pPipe, UINT32 uSizeTotal);
VOID wbhicPipeWriteFIFO_Uninit (WBHIC_FIFO_PIPE_T *pPipe);
VOID wbhicPipeReadFIFO_Init (WBHIC_FIFO_PIPE_T *pPipe, UINT32 uSizeTotal);
VOID wbhicPipeReadFIFO_Uninit (WBHIC_FIFO_PIPE_T *pPipe);
VOID wbhicPipeReadFIFO_SetLength (WBHIC_FIFO_PIPE_T *pPipe, UINT32 uLength);




/* Functions in data transfer mode. */
BOOL wbhicSingleRead (UINT32 uAddress, UINT32 *puValue);
BOOL wbhicSingleWrite (UINT32 uAddress, UINT32 uValue);
BOOL wbhicSpecialWrite (UINT32 uAddress, UINT32 uValue);
BOOL wbhicBurstRead (UINT32 uAddress, UINT32 uBufferType, VOID *pTargetBuffer);
BOOL wbhicBurstWrite (UINT32 uAddress, UINT32 uBufferType, VOID *pSourceBuffer);

/* Functions in firmware downloading mode. */
BOOL wbhicDownloadFirmwareEx (CONST UCHAR *pucBinCode,	/* Firmware */
							  UINT32 uTargetSize,		/* Firmware size. */
							  UINT32 uTargetAddr,		/* Firmware execute address. */
							  BOOL bVerify,				/* Verify after firmware is written to dram. */
							  BOOL bSetBusyOnBoot		/* Set HIC BUSY flag on booting. */
							  );
BOOL wbhicDownloadFirmware(CONST UCHAR *pucBinCode,
						   UINT32 uTargetSize);


/* Functions in LCM bypass mode. */
#ifdef _LCM_18BIT_HW16
VOID wbhicLCDDFTrans_UsePowerSaving (BOOL bPowerSaving);
BOOL wbhicLCDDFTrans_NONE(VOID);
BOOL wbhicLCDDFTrans_IR(VOID);
BOOL wbhicLCDDFTrans_IMG(VOID);
#endif
BOOL wbhicInitLCDController (WBHIC_LCD_REGS *pRegs);
BOOL wbhicRestoreLCDController (CONST WBHIC_LCD_REGS *pRegs);
VOID wbhicLCDWriteAddr(INT nLCD, USHORT usCmd);
VOID wbhicLCDWriteReg(INT nLCD, USHORT usCmd);
VOID wbhicLCDWrite(INT nLCD, USHORT usAddr, USHORT usReg);
VOID wbhicLCDWriteRGB565(INT nLCD, USHORT usColor);
VOID wbhicLCDWriteRGB888(INT nLCD, UCHAR ucR, UCHAR ucG, UCHAR ucB);
#if defined(_HIC_MODE_I8_) || defined(_HIC_MODE_I16_) || defined(_HIC_MODE_I9_) || defined(_HIC_MODE_I18_)
BOOL wbhicLCDIndexSelectLCD(INT nLCD);
#endif



typedef struct
{
	SOFT_PIPE_T pipe;
	UINT32 uLength;
} WBHIC_FIXLEN_PIPE_T;
#define WBHIC_FIFO_PIPE_IO ((UINT32)-2)

/* Type I: Command without waiting response. */
BOOL wbhicCmd_I(UCHAR ucCommand,	//CF_REGF
				UCHAR ucSubCommand,	//CF_REGE
				UCHAR ucParam1,		//CF_REGD
				UCHAR ucParam2,		//CF_REGC
				UCHAR ucParam3,		//CF_REGB
				UCHAR ucParam4		//CF_REGA
				);
/* Type II: Command with response. */
BOOL wbhicCmd_II(UCHAR ucCommand,	//CF_REGF
				 UCHAR ucSubCommand,//CF_REGE
				 UCHAR ucParam1,	//CF_REGD
				 UCHAR ucParam2,	//CF_REGC
				 UCHAR ucParam3,	//CF_REGB
				 UCHAR ucParam4,	//CF_REGA
				 UCHAR *pucRespon1,	//CF_REGC
				 UCHAR *pucRespon2,	//CF_REGB
				 UCHAR *pucRespon3,	//CF_REGA
				 UCHAR *pucRespon4	//CF_REG9
				 );
/* Type III: Command with mass data writing. */
BOOL wbhicCmd_III_Write_Len(UCHAR ucCommand,	//CF_REGF
						UCHAR ucSubCommand,		//CF_REGE
						UINT32 uBufferLength	//Length of data buffer
						);
BOOL wbhicCmd_III_Write (UCHAR ucCommand,			//CF_REGF
					 	 UCHAR ucSubCommand,		//CF_REGE
						 VOID *pSourceBuffer,			//Data buffer
                         UINT32 uBufferType,		//Type of data buffer
						 UINT32 *puBufferWritten	//The length written
						 );
/* Type III: Command with mass data reading.
	(Step1, send command and read the length.)
 */
BOOL wbhicCmd_III_Read_Len(UCHAR ucCommand,		//CF_REGF
						  UCHAR ucSubCommand,	//CF_REGE
						  UCHAR ucParam1,		//CF_REGD
						  UCHAR ucParam2,		//CF_REGC
						  UCHAR ucParam3,		//CF_REGB
						  UCHAR ucParam4,		//CF_REGA
						  UINT32 *puBufferLength//Length of data buffer
						  );

/* Type IV: Command Parameters in data port. */
BOOL wbhicCmd_IV (UCHAR ucCommand,      //CF_REGF
                  UCHAR ucSubCommand,   //CF_REGE
                  UINT32 uParamNum,	    //Number of parameters
                  ...					//Pointer of param1(VOID *)
										//Type or length of param1(UINT32)
										//Pointer of param2(VOID *)
										//Type or length of param2(UINT32)
										//...
    );
/* Type V: Command response in data port.
 */
BOOL wbhicCmd_V (UCHAR ucCommand,	//CF_REGF
                 UCHAR ucSubCommand,//CF_REGE
	             UINT32 uParamNum,	//Number of parameters
       		     ...				//Pointer of param1(VOID *)
			                		//Type or length of param1(UINT32)
    			            		//Pointer of param2(VOID *)
        	    		    		//Type or length of param2(UINT32)
        	    		    		//...
        	    		    		//Number of responses.(UINT32)
        	    		    		//Pointer of response1(VOID *)
        	    		    		//Type of max length of response1(UINT32)
        	    		    		//Real length of response1(UINT32 *)
        	    		    		//Pointer of response2(VOID *)
        	    		    		//Type of max length of response2(UINT32)
        	    		    		//Real length of response2(UINT32 *)
        	    		    		//...
	);

BOOL __wbhicCmd_III_Write_Len (UCHAR ucCommand,   //CF_REGF
                               UCHAR ucSubCommand,        //CF_REGE
                               UINT32 uBufferLength       //Length of data buffer
                               );




/* Convert USHORT to UCHAR.*/
#define USHORT_2_UCHAR(usIn,pucOut) \
do \
{ \
	USHORT us_alias_name_of_usIn = (USHORT) (usIn); \
	UCHAR *puc_alias_name_of_pucOut = (UCHAR *) (pucOut); \
    puc_alias_name_of_pucOut[1] = (UCHAR) (us_alias_name_of_usIn >> 8); \
    puc_alias_name_of_pucOut[0] = (UCHAR) us_alias_name_of_usIn; \
} \
while (0)

/* Convert UCHAR to USHORT. */
#define UCHAR_2_USHORT(pcucIn,usOut) \
do \
{ \
	CONST UCHAR *pcuc_alias_name_of_pcucIn = (CONST UCHAR *) (pcucIn); \
	(usOut) = ((USHORT) pcuc_alias_name_of_pcucIn[0]) \
			| (((USHORT) pcuc_alias_name_of_pcucIn[1]) << 8); \
} \
while (0)

/* Convert UINT32 to UCHAR. */
#define UINT32_2_UCHAR(uIn,pucOut) \
do \
{ \
	UINT32 u_alias_name_of_uIn_ = (UINT32) (uIn); \
	UCHAR *puc_alias_name_of_pucOut = (UCHAR *) (pucOut); \
	puc_alias_name_of_pucOut[3] = (UCHAR) (u_alias_name_of_uIn_ >> 24); \
	puc_alias_name_of_pucOut[2] = (UCHAR) (u_alias_name_of_uIn_ >> 16); \
	puc_alias_name_of_pucOut[1] = (UCHAR) (u_alias_name_of_uIn_ >> 8); \
	puc_alias_name_of_pucOut[0] = (UCHAR) u_alias_name_of_uIn_; \
} \
while (0)

/* Convert UCHAR to UINT32. */
#define UCHAR_2_UINT32(pcucIn,uOut) \
do \
{ \
	CONST UCHAR *pcuc_alias_name_of_pcucIn = (CONST UCHAR *) (pcucIn); \
	(uOut) = ((UINT32) pcuc_alias_name_of_pcucIn[0]) \
			| (((UINT32) pcuc_alias_name_of_pcucIn[1]) << 8) \
			| (((UINT32) pcuc_alias_name_of_pcucIn[2]) << 16) \
			| (((UINT32) pcuc_alias_name_of_pcucIn[3]) << 24); \
} \
while (0)


/* Error control */
INT32 __wbSetLastError (INT32 nErrorCode, const char *cpFile, int nLine);
INT32 __wbGetLastError (VOID);
#if 1
#	define wbSetLastError(nErrorCode) __wbSetLastError ((nErrorCode), __FILE__, __LINE__)
#else
#	define wbSetLastError(nErrorCode) __wbSetLastError ((nErrorCode), NULL, 0)
#endif
#	define wbGetLastError __wbGetLastError



#define HIC_DEBUG
#ifdef HIC_DEBUG
extern int g_iHIC_DEBUG_FIFO;
extern int g_iHIC_DEBUG_CMD;
void EnableHICDebugFIFO (BOOL bEnable);
void EnableHICDebugCMD (BOOL bEnable);
#endif

#endif

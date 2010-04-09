/****************************************************************************
 * 
 * FILENAME
 *     W99702_Audio.c
 *
 * VERSION
 *     1.0 
 *
 * DESCRIPTION
 *
 *
 *
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *
 *
 *     
 * HISTORY
 *     2004.07.27		Created by Yung-Chang Huang
 *
 *
 * REMARK
 *     None
 *
 *
 **************************************************************************/
#ifdef ECOS
#include "stdio.h"
#include "stdlib.h"
#include "drv_api.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "wbio.h"
#include "wblib.h"
#include "W99702_Audio_Regs.h"
#include "W99702_Audio.h"
#include "AC97.h"



UINT32 _uAuPlayBuffAddr=0x200000, _uAuPlayBuffSize=4096;
UINT32 _uAuRecBuffAddr=0x200000, _uAuRecBuffSize=4096;

AU_DEV_E _ePlayDev = AU_DEV_AC97; 
AU_DEV_E _eRecDev = AU_DEV_AC97;

AU_I2S_MCLK_TYPE_E _eI2SMclkType = I2S_MCLK_TYPE_NORMAL;
AU_I2C_TYPE_T _eI2SType = {	TRUE,	5,	4};//default setting is V1.7 demo board

BOOL _bOutputPathEnable[OUT_PATH_SP+1]= {TRUE,TRUE};
BOOL _bInputPathEnable[IN_PATH_LINEIN2+1] = {TRUE,TRUE,FALSE,FALSE};



BOOL _bIsLeftCH = TRUE;

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      audioEnable                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Enable and select W99702 playback and record device.             */
/*                                                                       */
/* INPUTS                                                                */
/*      ePlayDev    playback device (AU_DEV_AC97, AU_DEV_UDA1345TS,            */
/*                  	AU_DEV_MA3, AU_DEV_MA5, AU_DEV_W5691, AU_DEV_DAC)*/
/*		eRecDev		record device (AU_DEV_AC97, AU_DEV_UDA1345TS, AU_DEV_ADC)  */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
INT  audioEnable(AU_DEV_E ePlayDev, AU_DEV_E eRecDev)
{
	printf("ePlayDev=0x%x eRecDev=0x%x AU_DEV_AC97=0x%x\n", ePlayDev, eRecDev, AU_DEV_AC97);
#ifndef HAVE_AC97
	if ((ePlayDev == AU_DEV_AC97) || (eRecDev == AU_DEV_AC97))
		return ERR_AC97_NO_DEVICE;
#endif		

#ifndef HAVE_UDA1345TS
	if ((ePlayDev == AU_DEV_UDA1345TS) || (eRecDev == AU_DEV_UDA1345TS))
		return ERR_UDA1345TS_NO_DEVICE;
#endif

#ifndef HAVE_IIS
	if ((ePlayDev == AU_DEV_IIS) || (eRecDev == AU_DEV_IIS))
		return ERR_IIS_NO_DEVICE;
#endif

#ifndef HAVE_WM8753
	if ((ePlayDev == AU_DEV_WM8753) || (eRecDev == AU_DEV_WM8753))
		return ERR_WM8753_NO_DEVICE;
#endif

#ifndef HAVE_WM8978
	if ((ePlayDev == AU_DEV_WM8978) || (eRecDev == AU_DEV_WM8978))
		return ERR_WM8978_NO_DEVICE;
#endif

#ifndef HAVE_AK4569
	if ((ePlayDev == AU_DEV_AK4569) || (eRecDev == AU_DEV_AK4569))
		return ERR_AK4569_NO_DEVICE;
#endif

#ifndef HAVE_WM8751
	if (ePlayDev == AU_DEV_WM8751)
		return ERR_WM8751_NO_DEVICE;
#endif


#ifndef HAVE_MA3
	if (ePlayDev == AU_DEV_MA3)
		return ERR_MA3_NO_DEVICE;
#endif		

#ifndef HAVE_MA5
	if (ePlayDev == AU_DEV_MA5)
		return ERR_MA5_NO_DEVICE;
#endif		

#ifndef HAVE_MA5I
	if (ePlayDev == AU_DEV_MA5I)
		return ERR_MA5I_NO_DEVICE;
#endif		

#ifndef HAVE_MA5SI
	if (ePlayDev == AU_DEV_MA5SI)
		return ERR_MA5SI_NO_DEVICE;
#endif

#ifndef HAVE_W5691
	if (ePlayDev == AU_DEV_W5691)
		return ERR_W5691_NO_DEVICE;
#endif

#ifndef HAVE_W56964
	if (ePlayDev == AU_DEV_W56964)
		return ERR_W56964_NO_DEVICE;
#endif

#ifndef HAVE_ADDA
	if (ePlayDev == AU_DEV_DAC)
		return ERR_DAC_NO_DEVICE;
#endif		

#ifndef HAVE_ADDA
	if (eRecDev == AU_DEV_ADC)
		return ERR_ADC_NO_DEVICE;
#endif		

#ifndef HAVE_TIAIC31
	if (ePlayDev == AU_DEV_TIAIC31 || eRecDev == AU_DEV_TIAIC31)
		return ERR_TIAIC31_NO_DEVICE;
#endif

#ifndef HAVE_WM8731
	if ((ePlayDev == AU_DEV_WM8731) || (eRecDev == AU_DEV_WM8731))
		return ERR_WM8731_NO_DEVICE;
#endif

#ifndef HAVE_PSM711A
	if ((ePlayDev == AU_DEV_PSM711A))
		return ERR_NO_DEVICE;
#endif

#ifndef HAVE_WM8750
	if ((ePlayDev == AU_DEV_WM8750) || (eRecDev == AU_DEV_WM8750))
		return ERR_NO_DEVICE;
#endif

	_ePlayDev = ePlayDev;
	_eRecDev = eRecDev;
	
	/* Enable audio clock */
	outpw(REG_CLKCON, inpw(REG_CLKCON) | 0x10000000);
	
	/* Config GPIO multi-function control */
	outpw(REG_PADC0, inpw(REG_PADC0) | 0x20000);
	
	return 0;
}


INT  audioDisable()
{
	/* Disable audio clock */
	outpw(REG_CLKCON, inpw(REG_CLKCON) & ~0x10000000);

	/* Config GPIO multi-function control */
	outpw(REG_PADC0, inpw(REG_PADC0) & ~0x20000);
	
	return 0;
}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      audioSetPlayBuff                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Set playback buffer address and size. This buffer will be used   */
/*      by W99702 audio controller as playback DMA access source. All    */
/*      subsequent play callbacks will request half of this buffer size. */
/*                                                                       */
/* INPUTS                                                                */
/*      uPlayBuffAddr  Playback buffer address                          */
/*      dwPlayBuddSize  Playback buffer size                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
INT  audioSetPlayBuff(UINT32 uPlayBuffAddr, UINT32 uPlayBuffSize)
{
	if ((uPlayBuffAddr%32 != 0) || (uPlayBuffSize%32 != 0) || (uPlayBuffAddr & 0x10000000) ==0)
		return ERR_AU_MEMORY_ALIGN;
	_uAuPlayBuffSize = uPlayBuffSize;
	_uAuPlayBuffAddr = uPlayBuffAddr;
	return 0;
	
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      audioSetRecBuff                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Set record buffer address and size. This buffer will be used     */
/*      by W99702 audio controller as record DMA target. All subsequrnt  */
/*      record callbacks will request half of this buffer size.          */
/*                                                                       */
/* INPUTS                                                                */
/*      uRecBuffAddr  Record buffer address                             */
/*      dwRecBuddSize  Record buffer size                                */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
INT  audioSetRecBuff(UINT32 uRecBuffAddr, UINT32 uRecBuffSize)
{
	if ((uRecBuffAddr%32 != 0) || (uRecBuffSize%32 != 0) || (uRecBuffAddr & 0x10000000) ==0)
		return ERR_AU_MEMORY_ALIGN;
	_uAuRecBuffAddr = uRecBuffAddr;
	_uAuRecBuffSize = uRecBuffSize;
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      audioStartPlay                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Start W99702 audio playback.                                     */
/*                                                                       */
/* INPUTS                                                                */
/*      fnCallBack     client program provided callback function. The audio */
/*                  driver will call back to get next block of PCM data  */
/*      nSamplingRate  the playback sampling rate. Supported sampling    */
/*                  rate are 48000, 44100, 32000, 24000, 22050, 16000,   */
/*                  11025, and 8000 Hz                                   */
/*      nChannels	number of playback nChannels                          */
/*					1: single channel, otherwise: double nChannels        */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
INT  audioStartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels)
{
#ifdef HAVE_AC97
	if (_ePlayDev == AU_DEV_AC97)
		return ac97StartPlay(fnCallBack, nSamplingRate, nChannels);
#endif	

#ifdef HAVE_UDA1345TS
	if (_ePlayDev == AU_DEV_UDA1345TS)
		return uda1345tsStartPlay(fnCallBack, nSamplingRate, nChannels, IIS_FORMAT);
#endif	

#ifdef HAVE_IIS
	if (_ePlayDev == AU_DEV_IIS)
		return i2sStartPlay(fnCallBack, nSamplingRate, nChannels, IIS_FORMAT);
#endif	

#ifdef HAVE_WM8753
	if (_ePlayDev == AU_DEV_WM8753)
		return wm8753StartPlay(fnCallBack, nSamplingRate, nChannels, IIS_FORMAT);
#endif	

#ifdef HAVE_WM8751
	if (_ePlayDev == AU_DEV_WM8751)
		return wm8751StartPlay(fnCallBack, nSamplingRate, nChannels, IIS_FORMAT);
#endif	

#ifdef HAVE_WM8978
	if (_ePlayDev == AU_DEV_WM8978)
		return wm8978StartPlay(fnCallBack, nSamplingRate, nChannels, IIS_FORMAT);
#endif	

#ifdef HAVE_AK4569
	if (_ePlayDev == AU_DEV_AK4569)
		return ak4569StartPlay(fnCallBack, nSamplingRate, nChannels, IIS_FORMAT);
#endif	

#ifdef HAVE_MA5I
	if (_ePlayDev == AU_DEV_MA5I)
		return ma5iStartPlay(fnCallBack, nSamplingRate, nChannels, IIS_FORMAT);
#endif			

#ifdef HAVE_MA5SI
	if (_ePlayDev == AU_DEV_MA5SI)
		return ma5siStartPlay(fnCallBack, nSamplingRate, nChannels, IIS_FORMAT);
#endif

#ifdef HAVE_MA3
	if (_ePlayDev == AU_DEV_MA3)
		return ma3StartPlay(fnCallBack, nSamplingRate, nChannels);
#endif								

#ifdef HAVE_MA5
	if (_ePlayDev == AU_DEV_MA5)
		return ma5StartPlay(fnCallBack, nSamplingRate, nChannels);
#endif								

#ifdef HAVE_W56964
	if (_ePlayDev == AU_DEV_W56964)
		return w56964StartPlay(fnCallBack,	nSamplingRate, nChannels, 0);
#endif

#ifdef HAVE_W5691
	if (_ePlayDev == AU_DEV_W5691)
		return w5691StartPlay(fnCallBack,	nSamplingRate, nChannels, 0);
#endif

#ifdef HAVE_ADDA
	if (_ePlayDev == AU_DEV_DAC)
		return dacStartPlay(fnCallBack, nSamplingRate, nChannels);
#endif

#ifdef HAVE_TIAIC31
	if (_ePlayDev == AU_DEV_TIAIC31)
		return tiaic31StartPlay(fnCallBack, nSamplingRate, nChannels, IIS_FORMAT);
#endif

#ifdef HAVE_WM8731
	if (_ePlayDev == AU_DEV_WM8731)
		return wm8731StartPlay(fnCallBack, nSamplingRate, nChannels, IIS_FORMAT);
#endif

#ifdef HAVE_PSM711A
	if (_ePlayDev == AU_DEV_PSM711A)
		return psm711aStartPlay(fnCallBack, nSamplingRate, nChannels, IIS_FORMAT);
#endif

#ifdef HAVE_WM8750
	if (_ePlayDev == AU_DEV_WM8750)
		return wm8750StartPlay(fnCallBack, nSamplingRate, nChannels, IIS_FORMAT);
#endif


	return ERR_NO_DEVICE;
}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      audioStopPlay                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Stop W99702 audio playback immdediately.                         */
/*                                                                       */
/* INPUTS                                                                */
/*      None    	                                                     */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
INT  audioStopPlay()
{
#ifdef HAVE_AC97
	if (_ePlayDev == AU_DEV_AC97)
		return ac97StopPlay();
#endif	

#ifdef HAVE_UDA1345TS
	if (_ePlayDev == AU_DEV_UDA1345TS)
		return uda1345tsStopPlay();
#endif	

#ifdef HAVE_IIS
	if (_ePlayDev == AU_DEV_IIS)
		return i2sStopPlay();
#endif	

#ifdef HAVE_WM8753
	if (_ePlayDev == AU_DEV_WM8753)
		return wm8753StopPlay();
#endif	

#ifdef HAVE_WM8978
	if (_ePlayDev == AU_DEV_WM8978)
		return wm8978StopPlay();
#endif	

#ifdef HAVE_AK4569
	if (_ePlayDev == AU_DEV_AK4569)
		return ak4569StopPlay();
#endif	

#ifdef HAVE_WM8751
	if (_ePlayDev == AU_DEV_WM8751)
		return wm8751StopPlay();
#endif	

#ifdef HAVE_MA3
	if (_ePlayDev == AU_DEV_MA3)
		return ma3StopPlay();
#endif								

#ifdef HAVE_MA5
	if (_ePlayDev == AU_DEV_MA5)
		return ma5StopPlay();
#endif								

#ifdef HAVE_MA5I
	if (_ePlayDev == AU_DEV_MA5I)
		return ma5iStopPlay();
#endif			

#ifdef HAVE_MA5SI
	if (_ePlayDev == AU_DEV_MA5SI)
		return ma5siStopPlay();
#endif

#ifdef HAVE_W5691
	if (_ePlayDev == AU_DEV_W5691)
		w5691StopPlay();
#endif

#ifdef HAVE_W56964
	if (_ePlayDev == AU_DEV_W56964)
		return w56964StopPlay();
#endif

#ifdef HAVE_ADDA
	if (_ePlayDev == AU_DEV_DAC)
		return dacStopPlay();
#endif

#ifdef HAVE_TIAIC31
	if (_ePlayDev == AU_DEV_TIAIC31)
		return tiaic31StopPlay();
#endif

#ifdef HAVE_WM8731
	if (_ePlayDev == AU_DEV_WM8731)
		return wm8731StopPlay();
#endif

#ifdef HAVE_PSM711A
	if (_ePlayDev == AU_DEV_PSM711A)
		return psm711aStopPlay();
#endif

#ifdef HAVE_WM8750
	if (_ePlayDev == AU_DEV_WM8750)
		return wm8750StopPlay();
#endif


	return ERR_NO_DEVICE;
}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      audioStartRecord                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Start W99702 audio record.                                       */
/*                                                                       */
/* INPUTS                                                                */
/*      fnCallBack     client program provided callback function. The audio */
/*                  driver will call back to deliver the newly recorded  */
/*                  block of PCM data                                    */
/*      nSamplingRate  the record sampling rate. Supported sampling      */
/*                  rate are 48000, 44100, 32000, 24000, 22050, 16000,   */
/*                  11025, and 8000 Hz                                   */
/*      nChannels	number of record nChannels                            */
/*					1: single channel, otherwise: double nChannels        */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
INT  audioStartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels)
{
#ifdef HAVE_AC97
	if (_eRecDev == AU_DEV_AC97)
		return ac97StartRecord(fnCallBack, nSamplingRate, nChannels);
#endif	

#ifdef HAVE_UDA1345TS
	if (_eRecDev == AU_DEV_UDA1345TS)
		return uda1345tsStartRecord(fnCallBack, nSamplingRate, nChannels, IIS_FORMAT);
#endif

#ifdef HAVE_IIS
	if (_eRecDev == AU_DEV_IIS)
		return i2sStartRecord(fnCallBack, nSamplingRate, nChannels, IIS_FORMAT);
#endif

#ifdef HAVE_WM8753
	if (_eRecDev == AU_DEV_WM8753)
		return wm8753StartRecord(fnCallBack, nSamplingRate, nChannels, IIS_FORMAT);
#endif	

#ifdef HAVE_WM8978
	if (_eRecDev == AU_DEV_WM8978)
		return wm8978StartRecord(fnCallBack, nSamplingRate, nChannels, IIS_FORMAT);
#endif	

#ifdef HAVE_AK4569
	if (_eRecDev == AU_DEV_AK4569)
		return ak4569StartRecord(fnCallBack, nSamplingRate, nChannels, IIS_FORMAT);
#endif	

#ifdef HAVE_ADDA
	if (_eRecDev == AU_DEV_ADC)
		return adcStartRecord(fnCallBack, nSamplingRate);
#endif

#ifdef HAVE_TIAIC31
	if (_eRecDev == AU_DEV_TIAIC31)
		return tiaic31StartRecord(fnCallBack, nSamplingRate, nChannels, IIS_FORMAT);
#endif		

#ifdef HAVE_WM8731
	if (_eRecDev == AU_DEV_WM8731)
		return wm8731StartRecord(fnCallBack, nSamplingRate, nChannels, IIS_FORMAT);
#endif

#ifdef HAVE_WM8750
	if (_eRecDev == AU_DEV_WM8750)
		return wm8750StartRecord(fnCallBack, nSamplingRate, nChannels, IIS_FORMAT);
#endif
	return ERR_NO_DEVICE;
}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      audioStopRecord                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Stop W99702 audio record immediately.                            */
/*                                                                       */
/* INPUTS                                                                */
/*      None    	                                                     */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
INT  audioStopRecord()
{
#ifdef HAVE_AC97
	if (_eRecDev == AU_DEV_AC97)
		return ac97StopRecord();
#endif	

#ifdef HAVE_UDA1345TS
	if (_eRecDev == AU_DEV_UDA1345TS)
		return uda1345tsStopRecord();
#endif

#ifdef HAVE_IIS
	if (_eRecDev == AU_DEV_IIS)
		return i2sStopRecord();
#endif





#ifdef HAVE_WM8753
	if (_eRecDev == AU_DEV_WM8753)
		return wm8753StopRecord();
#endif	

#ifdef HAVE_WM8978
	if (_eRecDev == AU_DEV_WM8978)
		return wm8978StopRecord();
#endif	

#ifdef HAVE_AK4569
	if (_eRecDev == AU_DEV_AK4569)
		return ak4569StopRecord();
#endif	

#ifdef HAVE_ADDA
	if (_eRecDev == AU_DEV_ADC)
		return adcStopRecord();
#endif

#ifdef HAVE_TIAIC31
	if (_eRecDev == AU_DEV_TIAIC31)
		return tiaic31StopRecord();
#endif

#ifdef HAVE_WM8731
	if (_eRecDev == AU_DEV_WM8731)
		return wm8731StopRecord();
#endif

#ifdef HAVE_WM8750
	if (_eRecDev == AU_DEV_WM8750)
		return wm8750StopRecord();
#endif	

	return ERR_NO_DEVICE;
}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      audioSetPlayVolume                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Set W99702 audio left and right channel play volume.             */
/*                                                                       */
/* INPUTS                                                                */
/*      ucLeftVol    play volume of left channel                          */
/*      ucRightVol   play volume of left channel                          */
/*                  0:  mute                                             */
/*                  1:  minimal volume                                   */
/*                  31: maxmum volume                                    */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
INT  audioSetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol)
{
#ifdef HAVE_AC97
	if (_ePlayDev == AU_DEV_AC97)
		return ac97SetPlayVolume(ucLeftVol, ucRightVol);
#endif	

#ifdef HAVE_UDA1345TS
	if (_ePlayDev == AU_DEV_UDA1345TS)
		return uda1345tsSetPlayVolume(ucLeftVol,ucRightVol);
#endif	

#ifdef HAVE_WM8753
	if (_ePlayDev == AU_DEV_WM8753)
		return wm8753SetPlayVolume(ucLeftVol,ucRightVol);
#endif	

#ifdef HAVE_AK4569
	if (_ePlayDev == AU_DEV_AK4569)
		return ak4569SetPlayVolume(ucLeftVol,ucRightVol);
#endif	

#ifdef HAVE_WM8751
	if (_ePlayDev == AU_DEV_WM8751)
		return wm8751SetPlayVolume(ucLeftVol,ucRightVol);
#endif	

#ifdef HAVE_WM8978
	if (_ePlayDev == AU_DEV_WM8978)
		return wm8978SetPlayVolume(ucLeftVol,ucRightVol);
#endif	

#ifdef HAVE_MA3
	if (_ePlayDev == AU_DEV_MA3)
		return ma3SetPlayVolume(ucLeftVol, ucRightVol);
#endif								

#ifdef HAVE_MA5
	if (_ePlayDev == AU_DEV_MA5)
		return ma5SetPlayVolume(ucLeftVol, ucRightVol);
#endif			
					
#ifdef HAVE_MA5I
	if (_ePlayDev == AU_DEV_MA5I)
		return ma5iSetPlayVolume(ucLeftVol, ucRightVol);
#endif			

#ifdef HAVE_MA5SI
	if (_ePlayDev == AU_DEV_MA5SI)
		return ma5siSetPlayVolume(ucLeftVol, ucRightVol);
#endif

#ifdef HAVE_W5691
	if (_ePlayDev == AU_DEV_W5691)
		return w5691SetPlayVolume(ucLeftVol, ucRightVol);
#endif

#ifdef HAVE_W56964
	if (_ePlayDev == AU_DEV_W56964)
		return w56964SetPlayVolume(ucLeftVol, ucRightVol);
#endif

#ifdef HAVE_TIAIC31
	if (_ePlayDev == AU_DEV_TIAIC31)
		return tiaic31SetPlayVolume(ucLeftVol, ucRightVol);		
#endif								

#ifdef HAVE_WM8731
	if (_ePlayDev == AU_DEV_WM8731)
		return wm8731SetPlayVolume(ucLeftVol, ucRightVol);		
#endif

#ifdef HAVE_WM8750
	if (_ePlayDev == AU_DEV_WM8750)
		return wm8750SetPlayVolume(ucLeftVol, ucRightVol);
#endif



	return ERR_NO_DEVICE;
}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      audioSetRecordVolume                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Set AUDIO left and right channel record volume.                  */
/*                                                                       */
/* INPUTS                                                                */
/*      ucLeftVol    record volume of left channel                        */
/*      ucRightVol   record volume of left channel                        */
/*                  0:  mute                                             */
/*                  1:  minimal volume                                   */
/*                  31: maxmum volume                                    */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
INT  audioSetRecordVolume(UINT8 ucLeftVol, UINT8 ucRightVol)
{
#ifdef HAVE_AC97
	if (_eRecDev == AU_DEV_AC97)
		return ac97SetRecordVolume(ucLeftVol, ucRightVol);
#endif	

#ifdef HAVE_WM8753
	if (_eRecDev == AU_DEV_WM8753)
		return wm8753SetRecordVolume(ucLeftVol,ucRightVol);
#endif	

#ifdef HAVE_AK4569
	if (_eRecDev == AU_DEV_AK4569)
		return ak4569SetRecordVolume(ucLeftVol);
#endif	

#ifdef HAVE_WM8978
	if (_eRecDev == AU_DEV_WM8978)
		return wm8978SetRecordVolume(ucLeftVol,ucRightVol);
#endif	

#ifdef HAVE_ADDA
	if (_eRecDev == AU_DEV_ADC)
		return adcSetRecordVolume(ucLeftVol);
#endif			

#ifdef HAVE_TIAIC31
	if (_eRecDev == AU_DEV_TIAIC31)
		return tiaic31SetRecordVolume(ucLeftVol, ucRightVol);		
#endif	

#ifdef HAVE_WM8731
	if (_eRecDev == AU_DEV_WM8731)
		return wm8731SetRecordVolume(ucLeftVol, ucRightVol);		
#endif

#ifdef HAVE_WM8750
	if (_eRecDev == AU_DEV_WM8750)
		return wm8750SetRecordVolume(ucLeftVol, ucRightVol);
#endif
			
	return ERR_NO_DEVICE;					
}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      audioWriteCmd		                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Write command to external audio codec.		                     */
/*                                                                       */
/* INPUTS                                                                */
/*      regAddr    register address of external codec                    */
/*      data   	   data for writing to regAddr                           */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
INT audioWriteData(UINT16 regAddr, UINT16 data)
{
#ifdef HAVE_TIAIC31
	if (_ePlayDev == AU_DEV_TIAIC31 || _eRecDev == AU_DEV_TIAIC31)
		return TIAIC31_I2C_Write_Data(regAddr,data);
#endif

#ifdef HAVE_WM8978
	if (_ePlayDev == AU_DEV_WM8978)
		return TwoWire_Write_Data(regAddr,data);
#endif	

#ifdef HAVE_WM8731
	if (_ePlayDev == AU_DEV_WM8731)
		return TwoWire_Write_Data(regAddr,data);
#endif


	return ERR_NO_DEVICE;
	
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      audioReadCmd		                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Read regsiter data from external audio codec.		             */
/*                                                                       */
/* INPUTS                                                                */
/*      regAddr    register address of external codec                    */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      INT				Success                                          */
/*		Ohterwise		error											 */
/*                                                                       */
/*************************************************************************/
INT audioReadData(UINT8 regAddr)
{

#ifdef HAVE_TIAIC31
	if (_ePlayDev == AU_DEV_TIAIC31 || _eRecDev == AU_DEV_TIAIC31)
		return TIAIC31_I2C_Read_Data(regAddr);
#endif

	return ERR_NO_DEVICE;
	
}

INT audioSelectOutputPath(AUDIO_OUTPUTPATH_E eOutput,BOOL bEnable)
{

	_bOutputPathEnable[eOutput]=bEnable;
#ifdef HAVE_WM8978
	if (_ePlayDev == AU_DEV_WM8978)
		return wm8978SelectOutputPath();
#endif

	return ERR_NO_DEVICE;
}
INT audioSelectInputPath(AUDIO_INPUTPATH_E eInput,BOOL bEnable)
{
	_bInputPathEnable[eInput]=bEnable;
#ifdef HAVE_WM8978
	if (_eRecDev == AU_DEV_WM8978)
		return wm8978SelectInputPath();
#endif

	return ERR_NO_DEVICE;
}
INT audioBypassEnable()
{
#ifdef HAVE_WM8978
	if (_eRecDev == AU_DEV_WM8978)
		return WM8978_Bypass_Enable();
#endif

	return ERR_NO_DEVICE;
}
INT audioBypassDisable()
{
#ifdef HAVE_WM8978
	if (_eRecDev == AU_DEV_WM8978)
		return WM8978_Bypass_Disable();
#endif

	return ERR_NO_DEVICE;
}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      audioSetPureI2SMCLKType		                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Set the MCLK output type when the device AU_DEV_IIS is selected. */
/*                                                                       */
/* INPUTS                                                                */
/*      eI2SMclkType    one of three MCLK output types	                 */
/*						I2S_MCLK_TYPE_NORMAL:MCLK=384*fs or 256*fs		 */
/*						I2S_MCLK_TYPE_ZTE:MCLK=APLL						 */
/*						I2S_MCLK_TYPE_BIRD:MCLK=256*fs					 */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0				Success                                          */
/*		Ohterwise		error											 */
/*                                                                       */
/*************************************************************************/
INT audioSetPureI2SMCLKType(AU_I2S_MCLK_TYPE_E eI2SMclkType)
{
	_eI2SMclkType = eI2SMclkType;
	return 0;
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      audioSetI2CType		                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Set the control interface of external audio codec	             */
/*                                                                       */
/* INPUTS                                                                */
/*      eI2CType	Use GPIO as I2C or not and select the pin num.       */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0			Success            		                             */
/*		Ohterwise		error											 */
/*                                                                       */
/*************************************************************************/
INT audioSetI2CType(AU_I2C_TYPE_T eI2CType)
{

	_eI2SType = eI2CType;
	return 0;
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      audioConfigMono		                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      When the audio data input is mono, select the channel is		 */
/*		left or right	             									 */
/*                                                                       */
/* INPUTS                                                                */
/*      bIsLeftCH	Select left channel or right channel		         */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0				Success            		                         */
/*		Ohterwise		error											 */
/*                                                                       */
/*************************************************************************/
INT	audioRecConfigMono(BOOL bIsLeftCH)
{
	_bIsLeftCH = bIsLeftCH;
	return 0;
}






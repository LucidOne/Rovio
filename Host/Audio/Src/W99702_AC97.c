/****************************************************************************
 * 
 * FILENAME
 *     AC97.c
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
 *     2004.06.01		Created by Yung-Chang Huang
 *     2004.09.02       Ver 1.0 Modify for W99702 coding standard
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
#include "string.h"
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

#ifdef HAVE_AC97

static AUDIO_T	_tAC97;

#define	AC97_ACTIVE				0x1
#define	AC97_PLAY_ACTIVE		0x2
#define AC97_REC_ACTIVE			0x4

static BOOL	_bAC97Active = 0;
static volatile BOOL	_bPlayDmaToggle, _bRecDmaToggle;

extern BOOL _bIsLeftCH;

static void Delay(int nCnt)
{
	volatile int  loop;
	for (loop=0; loop<nCnt; loop++);
}


static UINT16  ac97_read_register(INT nIdx)
{
	volatile INT	nWait;
	
	/* set the R_WB bit and write register index */
	outpw(REG_ACTL_ACOS1, 0x80 | nIdx);
	
	/* set the valid frame bit and valid slots */
	outpw(REG_ACTL_ACOS0, 0x11);

	Delay(100);
	
	/* polling the AC_R_FINISH */
	for (nWait = 0; nWait < 0x10000; nWait++)
	{
		if (inpw(REG_ACTL_ACCON) & AC_R_FINISH)
			break;
	}
	if (nWait == 0x10000)
	{
		_error_msg("ac97_read_register time out!\n");
		//_debug_msg("REG_ACTL_ACOS0=%x,  REG_ACTL_ACOS1=%x,  REG_ACTL_ACIS0=%x,  REG_ACTL_ACIS1=%x\n", inpw(REG_ACTL_ACOS0), inpw(REG_ACTL_ACOS1), inpw(REG_ACTL_ACIS0), inpw(REG_ACTL_ACIS1));
	}

	outpw(REG_ACTL_ACOS0, 0);
		
	//if (inpw(REG_ACTL_ACIS0) & 0x0C != 0x0C)
	//	_debug_msg("ac97_read_register - valid slot was not set, 0x%x\n", inpw(REG_ACTL_ACIS0));
		
	if (inpw(REG_ACTL_ACIS1) >> 2 != nIdx)
	{	_debug_msg("ac97_read_register - R_INDEX of REG_ACTL_ACIS1 not match!, 0x%x\n", inpw(REG_ACTL_ACIS1));	}

	Delay(100);		
	return (inpw(REG_ACTL_ACIS2) & 0xFFFF);
}


static INT  ac97_write_register(INT nIdx, UINT16 sValue)
{
	volatile INT	nWait;
	
	//_debug_smg("ac97_write_register - index = 0x%x, value = 0x%x\n", nIdx, sValue);

	/* clear the R_WB bit and write register index */
	outpw(REG_ACTL_ACOS1, nIdx);
	
	/* write register value */
	outpw(REG_ACTL_ACOS2, sValue);
	
	/* set the valid frame bit and valid slots */
	outpw(REG_ACTL_ACOS0, 0x13);
	
	Delay(100);

	/* polling the AC_W_FINISH */
	for (nWait = 0; nWait < 0x10000; nWait++)
	{
		if (!(inpw(REG_ACTL_ACCON) & AC_W_FINISH))
			break;
	}
	
	outpw(REG_ACTL_ACOS0, 0);

	//if (inpw(REG_ACTL_ACIS0) & 0x08 != 0x08)
	//	_debug_msg("                   ! valid slot was not set, 0x%x\n", inpw(REG_ACTL_ACIS0));
		
	if (ac97_read_register(nIdx) != sValue)
	{	_debug_msg("ac97_write_register, nIdx=0x%x, mismatch, 0x%x must be 0x%x\n", nIdx, ac97_read_register(nIdx), sValue); }
	
	return 0;
}


#if 0
static void ac97_read_all_registers()
{
	_debug_msg("AC97_RESET            = 0x%04x\r\n", ac97_read_register(AC97_RESET));
	_debug_msg("AC97_MASTER_VOLUME    = 0x%04x\r\n", ac97_read_register(AC97_MASTER_VOLUME));
	_debug_msg("AC97_AUX_OUT_VOLUME   = 0x%04x\r\n", ac97_read_register(AC97_AUX_OUT_VOLUME));
	_debug_msg("AC97_MONO_VOLUME      = 0x%04x\r\n", ac97_read_register(AC97_MONO_VOLUME));
	_debug_msg("AC97_MASTER_TONE      = 0x%04x\r\n", ac97_read_register(AC97_MASTER_TONE));
	_debug_msg("AC97_PC_BEEP_VOLUME   = 0x%04x\r\n", ac97_read_register(AC97_PC_BEEP_VOLUME));
	_debug_msg("AC97_PHONE_VOLUME     = 0x%04x\r\n", ac97_read_register(AC97_PHONE_VOLUME));
	_debug_msg("AC97_MIC_VOLUME       = 0x%04x\r\n", ac97_read_register(AC97_MIC_VOLUME));
	_debug_msg("AC97_LINE_IN_VOLUME   = 0x%04x\n", ac97_read_register(AC97_LINE_IN_VOLUME));
	_debug_msg("AC97_CD_VOLUME        = 0x%04x\n", ac97_read_register(AC97_CD_VOLUME));
	_debug_msg("AC97_VIDEO_VOLUME     = 0x%04x\n", ac97_read_register(AC97_VIDEO_VOLUME));
	_debug_msg("AC97_AUX_IN_VOLUME    = 0x%04x\n", ac97_read_register(AC97_AUX_IN_VOLUME));
	_debug_msg("AC97_PCM_OUT_VOLUME   = 0x%04x\n", ac97_read_register(AC97_PCM_OUT_VOLUME));
	_debug_msg("AC97_RECORD_SELECT    = 0x%04x\n", ac97_read_register(AC97_RECORD_SELECT));
	_debug_msg("AC97_RECORD_GAIN      = 0x%04x\n", ac97_read_register(AC97_RECORD_GAIN));
	_debug_msg("AC97_RECORD_GAIN_MIC  = 0x%04x\n", ac97_read_register(AC97_RECORD_GAIN_MIC));
	_debug_msg("AC97_GENERAL_PURPOSE  = 0x%04x\n", ac97_read_register(AC97_GENERAL_PURPOSE));
	_debug_msg("AC97_3D_CONTROL       = 0x%04x\n", ac97_read_register(AC97_3D_CONTROL));
	_debug_msg("AC97_AUDIO_INT_PAGING = 0x%04x\n", ac97_read_register(AC97_AUDIO_INT_PAGING));
	_debug_msg("AC97_POWERDOWN_CTRL   = 0x%04x\n", ac97_read_register(AC97_POWERDOWN_CTRL));
	_debug_msg("AC97_FRONT_DAC_RATE   = 0x%04x\n", ac97_read_register(AC97_FRONT_DAC_RATE));
}
#endif


#ifdef ECOS
cyg_uint32  ac97_play_isr(cyg_vector_t vector, cyg_addrword_t data)
#else
static void  ac97_play_isr()
#endif
{
	outpw(REG_ACTL_CON, inpw(REG_ACTL_CON) | T_DMA_IRQ);
	
	if (_tAC97.bPlayLastBlock)
	{
		outpw(REG_ACTL_PSR, P_DMA_MIDDLE_IRQ | P_DMA_END_IRQ);
		ac97StopPlay();
	}

	if (_bPlayDmaToggle == 0)
	{
		if (inpw(REG_ACTL_PSR) & P_DMA_MIDDLE_IRQ)
			outpw(REG_ACTL_PSR, P_DMA_MIDDLE_IRQ);
		else
			{ _error_msg("ac97_play_isr - miss middle!\n");	}
		_bPlayDmaToggle = 1;

		_tAC97.bPlayLastBlock = _tAC97.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, 
										_uAuPlayBuffSize/2);
	}
	else
	{
		if (inpw(REG_ACTL_PSR) & P_DMA_END_IRQ)
			outpw(REG_ACTL_PSR, P_DMA_END_IRQ);
		else
			{ _error_msg("ac97_play_isr - miss end!\n");	}
		_bPlayDmaToggle = 0;

		_tAC97.bPlayLastBlock = _tAC97.fnPlayCallBack((UINT8 *)(_uAuPlayBuffAddr + _uAuPlayBuffSize / 2), 
									_uAuPlayBuffSize/2);
	}
#ifdef ECOS
	return CYG_ISR_HANDLED;
#endif	
}		



#ifdef ECOS
cyg_uint32  ac97_rec_isr(cyg_vector_t vector, cyg_addrword_t data)
#else
static void  ac97_rec_isr()
#endif
{
	outpw(REG_ACTL_CON, inpw(REG_ACTL_CON) | R_DMA_IRQ);

	if (_bRecDmaToggle == 0)
	{
		if (inpw(REG_ACTL_RSR) & R_DMA_MIDDLE_IRQ)
			outpw(REG_ACTL_RSR, R_DMA_MIDDLE_IRQ);
		else
			{ _error_msg("ac97_rec_isr - miss middle!\n");	}
		_bRecDmaToggle = 1;

		_tAC97.fnRecCallBack((UINT8 *)_uAuRecBuffAddr, _uAuRecBuffSize/2);
	}
	else
	{
		if (inpw(REG_ACTL_RSR) & R_DMA_END_IRQ)
			outpw(REG_ACTL_RSR, R_DMA_END_IRQ);
		else
			{ _error_msg("ac97_rec_isr - miss end!\n");	}
		_bRecDmaToggle = 0;

		_tAC97.fnRecCallBack((UINT8 *)(_uAuRecBuffAddr + _uAuRecBuffSize / 2), 
									_uAuRecBuffSize/2);
	}
#ifdef ECOS
	return CYG_ISR_HANDLED;
#endif	
}


static INT  ac97_init()
{
	outpw(REG_GPIOA_OE, (inpw(REG_GPIOA_OE) & 0xFFFFC0FF) | 0x2700);

	/* disable pull-high/low */
	outpw(REG_GPIOA_PE, 0x3F00);

	Delay(1000);

	_tAC97.sPlayVolume = 0x0808;
	_tAC97.sRecVolume = 0x0808;

	/* enable audio controller and AC-link interface */
	outpw(REG_ACTL_CON, IIS_AC_PIN_SEL | AUDIO_EN | ACLINK_EN | PFIFO_EN | RFIFO_EN | T_DMA_IRQ | R_DMA_IRQ | DMA_EN);
	Delay(1000);

	/* reset Audio Controller */
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | ACTL_RESET_BIT);
	Delay(100);
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~ACTL_RESET_BIT);
	Delay(100);
	
	/* reset AC-link interface */
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | AC_RESET);
	Delay(1000);
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~AC_RESET);
	Delay(1000);

	/* cold reset AC 97 */
	outpw(REG_ACTL_ACCON, inpw(REG_ACTL_ACCON) | AC_C_RES);
	Delay(1000);
	outpw(REG_ACTL_ACCON, inpw(REG_ACTL_ACCON) & ~AC_C_RES);
	Delay(1000);

	if (!(inpw(REG_ACTL_ACIS0) & 0x10))
	{
		_debug_msg("Error - AC97 codec ready was not set, cold reset failed!\n");
		return ERR_AC97_CODEC_RESET;
	}

	Delay(100);
	//ac97_read_all_registers();
	
	/* set volumes */
	ac97_write_register(AC97_MASTER_VOLUME, 0x0000);
	ac97_write_register(AC97_MONO_VOLUME, 0x000f);
	//ac97_write_register(AC97_MASTER_TONE, 0x0303);
	ac97_write_register(AC97_MIC_VOLUME, 0x8000);
	//ac97_write_register(AC97_LINE_IN_VOLUME, 0x0707);
	//ac97_write_register(AC97_AUX_IN_VOLUME, 0x0707);
	ac97_write_register(AC97_PCM_OUT_VOLUME, _tAC97.sPlayVolume);
	
	ac97_write_register(AC97_RECORD_SELECT, 0);	/* record select MIC in */
	ac97_write_register(AC97_RECORD_GAIN, 0x0404);
	ac97_write_register(AC97_RECORD_GAIN_MIC, 0x0004);
	ac97_write_register(AC97_GENERAL_PURPOSE, 0);

	return 0;	
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      ac97SetPlayVolume                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Set AC97 left and right channel play volume.                     */
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
INT  ac97SetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol)
{
	//INT		nStatus;

#if 0	
	if (_bAC97Active == 0)
	{
		nStatus = ac97_init();
		if (nStatus < 0)
			return nStatus;	
	}
#endif
	
	if (ucLeftVol == 0)
		ucLeftVol = 0x80;
	else
		ucLeftVol = 32 - (ucLeftVol & 0x1f);
		
	if (ucRightVol == 0)
		ucRightVol = 0x80;
	else
		ucRightVol = 32 - (ucRightVol & 0x1f);
		
	_tAC97.sPlayVolume = (ucLeftVol << 8) | ucRightVol;
	ac97_write_register(AC97_PCM_OUT_VOLUME, _tAC97.sPlayVolume);

	if (_bAC97Active & AC97_PLAY_ACTIVE)
		outpw(REG_ACTL_ACOS0, 0x1f);
	else
		outpw(REG_ACTL_ACOS0, 0);
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      ac97SetRecordVolume                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Set AC97 left and right channel record volume.                   */
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
INT  ac97SetRecordVolume(UINT8 ucLeftVol, UINT8 ucRightVol)
{
	//INT		nStatus;
	
#if 0	
	if (_bAC97Active == 0)
	{
		nStatus = ac97_init();
		if (nStatus < 0)
			return nStatus;	
	}
#endif

	if (ucLeftVol == 0)
		ucLeftVol = 0x80;
	else
		ucLeftVol = 32 - (ucLeftVol & 0x1f);
		
	if (ucRightVol == 0)
		ucRightVol = 0x80;
	else
		ucRightVol = 32 - (ucRightVol & 0x1f);

	_tAC97.sRecVolume = (ucLeftVol << 8) | ucRightVol;
	ac97_write_register(AC97_MIC_VOLUME, ucRightVol );
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      ac97StartPlay                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Start AC97 playback.                                             */
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
INT  ac97StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels)
{
	INT	 nStatus;

	if (_bAC97Active & AC97_PLAY_ACTIVE)
		return ERR_AC97_PLAY_ACTIVE;		/* AC97 was playing */
	
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL));
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | PLAY_LEFT_CHNNEL);
	if (nChannels != 1)
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | PLAY_RIGHT_CHNNEL);
		
	if (_bAC97Active == 0)
	{
		nStatus = ac97_init();
		if (nStatus < 0)
			return nStatus;	
	}

	/* Install AC97 play interrupt */
#ifdef ECOS
	cyg_interrupt_disable();
	sysSetInterruptType(AU_PLAY_INT_NUM, LOW_LEVEL_SENSITIVE);
	cyg_interrupt_create(AU_PLAY_INT_NUM, 1, 0, ac97_play_isr, NULL, 
					&_tAC97.int_handle_play, &_tAC97.int_holder_play);
	cyg_interrupt_attach(_tAC97.int_handle_play);
	cyg_interrupt_unmask(AU_PLAY_INT_NUM);
	cyg_interrupt_enable();
#else	
	sysSetInterruptType(AU_PLAY_INT_NUM, LOW_LEVEL_SENSITIVE);
	sysInstallISR(IRQ_LEVEL_1, AU_PLAY_INT_NUM, (PVOID)ac97_play_isr);
    sysSetLocalInterrupt(ENABLE_IRQ);	/* enable CPSR I bit */
//	sysSetAIC2SWMode();					/* Set AIC into SW mode */
	sysEnableInterrupt(AU_PLAY_INT_NUM);
#endif

	/* set play sampling rate */
	if (nSamplingRate != 48000)
	{
		/* enable VRA and set sampling frequency */
		ac97_write_register(AC97_EXT_AUDIO_CTRL, ac97_read_register(AC97_EXT_AUDIO_CTRL)|0x1);
		ac97_write_register(AC97_FRONT_DAC_RATE, nSamplingRate);
	}
	
	_tAC97.fnPlayCallBack = fnCallBack;
	_tAC97.nPlaySamplingRate = nSamplingRate;

	/* set DMA play destination base address */
	outpw(REG_ACTL_PDSTB, _uAuPlayBuffAddr | 0x10000000);
	
	/* set DMA play buffer length */
	outpw(REG_ACTL_PDST_LENGTH, _uAuPlayBuffSize);

	/* call back to fill DMA play buffer */
	_tAC97.bPlayLastBlock = 0;
	_tAC97.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, _uAuPlayBuffSize/2);
	_tAC97.fnPlayCallBack((UINT8 *)(_uAuPlayBuffAddr + _uAuPlayBuffSize/2), 
								_uAuPlayBuffSize/2);
	
	/* start playing */
	_debug_msg("AC97 start playing...\n");
	_bPlayDmaToggle = 0;
	outpw(REG_ACTL_ACOS0, 0x1C);
	outpw(REG_ACTL_PSR, 0x3);
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | AC_PLAY);
	_bAC97Active |= AC97_PLAY_ACTIVE;
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      ac97StopPlay                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Stop AC97 playback immdediately.                                 */
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
INT  ac97StopPlay()
{
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL));

	if (!(_bAC97Active & AC97_PLAY_ACTIVE))
		return ERR_AC97_PLAY_INACTIVE;
	
	_debug_msg("AC97 stop playing\n");

	/* stop playing */
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~AC_PLAY);
	outpw(REG_ACTL_ACOS0, 0);
	
	/* disable audio play interrupt */
#ifdef ECOS
	cyg_interrupt_mask(AU_PLAY_INT_NUM);
	cyg_interrupt_detach(_tAC97.int_handle_play);
#else	
	sysDisableInterrupt(AU_PLAY_INT_NUM);
#endif
	
	_bAC97Active &= ~AC97_PLAY_ACTIVE;
	
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      ac97StartRecord                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Start AC97 record.                                               */
/*                                                                       */
/* INPUTS                                                                */
/*      fnCallBack  client program provided callback function. The audio */
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
INT  ac97StartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels)
{
	INT		nStatus;

	if (_bAC97Active & AC97_REC_ACTIVE)
		return ERR_AC97_REC_ACTIVE;		/* AC97 was recording */
	
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(RECORD_LEFT_CHNNEL | RECORD_RIGHT_CHNNEL));
	if (nChannels==1)
	{
		if (_bIsLeftCH)
			outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | RECORD_LEFT_CHNNEL);
		else
			outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | RECORD_RIGHT_CHNNEL);
	}else{
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | RECORD_RIGHT_CHNNEL | RECORD_LEFT_CHNNEL);
	}
	
	if (_bAC97Active == 0)
	{
		nStatus = ac97_init();
		if (nStatus < 0)
			return nStatus;	
	}

	/* Install AC97 record interrupt */
#ifdef ECOS
	cyg_interrupt_disable();
	sysSetInterruptType(AU_REC_INT_NUM, LOW_LEVEL_SENSITIVE);
	cyg_interrupt_create(AU_REC_INT_NUM, 1, 0, ac97_rec_isr, NULL, 
					&_tAC97.int_handle_rec, &_tAC97.int_holder_rec);
	cyg_interrupt_attach(_tAC97.int_handle_rec);
	cyg_interrupt_unmask(AU_REC_INT_NUM);
	cyg_interrupt_enable();
#else	
	sysSetInterruptType(AU_REC_INT_NUM, LOW_LEVEL_SENSITIVE);
	sysInstallISR(IRQ_LEVEL_1, AU_REC_INT_NUM, (PVOID)ac97_rec_isr);
    sysSetLocalInterrupt(ENABLE_IRQ);	/* enable CPSR I bit */
//	sysSetAIC2SWMode();					/* Set AIC into SW mode */
	sysEnableInterrupt(AU_REC_INT_NUM);
#endif


	/* set record sampling rate */
	if (nSamplingRate != 48000)
	{
		/* enable VRA and set sampling frequency */
		ac97_write_register(AC97_EXT_AUDIO_CTRL, ac97_read_register(AC97_EXT_AUDIO_CTRL)|0x1);
		ac97_write_register(AC97_LR_ADC_RATE, nSamplingRate);
	}
	
	_tAC97.fnRecCallBack = fnCallBack;
	_tAC97.nRecSamplingRate = nSamplingRate;

	/* set DMA record destination base address */
	outpw(REG_ACTL_RDSTB, _uAuRecBuffAddr | 0x10000000);
	
	/* set DMA record buffer length */
	outpw(REG_ACTL_RDST_LENGTH, _uAuRecBuffSize);

	/* start recording */
	_debug_msg("AC97 start recording...\n");
	_bRecDmaToggle = 0;
	outpw(REG_ACTL_RSR, 0x3);
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | AC_RECORD);
	_bAC97Active |= AC97_REC_ACTIVE;

	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      ac97StopRecord                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Stop AC97 record immediately.                                    */
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
INT  ac97StopRecord()
{
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(RECORD_RIGHT_CHNNEL | RECORD_LEFT_CHNNEL));

	if (!(_bAC97Active & AC97_REC_ACTIVE))
		return ERR_AC97_REC_INACTIVE;
	
	_debug_msg("AC97 stop recording\n");

	/* stop recording */
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~AC_RECORD);

	/* disable audio record interrupt */
#ifdef ECOS
	cyg_interrupt_mask(AU_REC_INT_NUM);
	cyg_interrupt_detach(_tAC97.int_handle_rec);
#else	
	sysDisableInterrupt(AU_REC_INT_NUM);
#endif
	
	/* release buffer */
	_bAC97Active &= ~AC97_REC_ACTIVE;
	return 0;
}


#endif	/* HAVE_AC97 */


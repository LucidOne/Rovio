/****************************************************************************
 * 
 * FILENAME
 *     W99702PCM_QUEUE_LEN.c
 *
 * VERSION
 *     0.1 
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
 *     2004.06.07		Created by Yung-Chang Huang
 *
 *
 * REMARK
 *     None
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

#ifdef HAVE_ADDA

static AUDIO_T	_tADDA;

#define	ADDA_ACTIVE				0x1
#define	DAC_PLAY_ACTIVE			0x2
#define ADC_REC_ACTIVE			0x4

BOOL	_bADDA_Active = 0;
static volatile BOOL	_bPlayDmaToggle, _bRecDmaToggle;

extern BOOL	_bIsLeftCH;

static INT  adda_get_sampling_rate(INT s)
{
	switch (s)
	{
		case 48000:	return AD_DA_48000;
		case 44100:	return AD_DA_44100;
		case 32000:	return AD_DA_32000;
		case 24000:	return AD_DA_24000;
		case 22050:	return AD_DA_22050;
		case 16000:	return AD_DA_16000;
		case 11025:	return AD_DA_11025;
		case 8000:	return AD_DA_8000;
		default:	return AD_DA_48000;			
	}
}


static void Delay(int nCnt)
{
	volatile int  loop;
	for (loop=0; loop<nCnt; loop++);
}


#ifdef ECOS
cyg_uint32  dac_play_isr(cyg_vector_t vector, cyg_addrword_t data)
#else
static void dac_play_isr()
#endif
{
	outpw(REG_ACTL_CON, inpw(REG_ACTL_CON) | T_DMA_IRQ);
	
	if (_tADDA.bPlayLastBlock)
	{
		outpw(REG_ACTL_PSR, P_DMA_MIDDLE_IRQ | P_DMA_END_IRQ);
		dacStopPlay();
	}

	if (_bPlayDmaToggle == 0)
	{
		if (inpw(REG_ACTL_PSR) & P_DMA_MIDDLE_IRQ)
			outpw(REG_ACTL_PSR, P_DMA_MIDDLE_IRQ);
		else
			{ _error_msg("dac_play_isr - miss middle!\n");	}
		_bPlayDmaToggle = 1;

		_tADDA.bPlayLastBlock = _tADDA.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, 
													_uAuPlayBuffSize/2);
	}
	else
	{
		if (inpw(REG_ACTL_PSR) & P_DMA_END_IRQ)
			outpw(REG_ACTL_PSR, P_DMA_END_IRQ);
		else
			{ _error_msg("dac_play_isr - miss end!\n");	}
		_bPlayDmaToggle = 0;

		_tADDA.bPlayLastBlock = _tADDA.fnPlayCallBack((UINT8 *)(_uAuPlayBuffAddr + _uAuPlayBuffSize/2), 
									_uAuPlayBuffSize/2);
	}
#ifdef ECOS
	return CYG_ISR_HANDLED;
#endif	
}		


#ifdef ECOS
cyg_uint32  adc_rec_isr(cyg_vector_t vector, cyg_addrword_t data)
#else
static void adc_rec_isr()
#endif
{
	outpw(REG_ACTL_CON, inpw(REG_ACTL_CON) | R_DMA_IRQ);

	if (_bRecDmaToggle == 0)
	{
		if (inpw(REG_ACTL_RSR) & R_DMA_MIDDLE_IRQ){
			outpw(REG_ACTL_RSR, R_DMA_MIDDLE_IRQ);
		}
		else
			{ _error_msg("adc_rec_isr - miss middle!\n");	}
		_bRecDmaToggle = 1;

		_tADDA.fnRecCallBack((UINT8 *)_uAuRecBuffAddr, _uAuRecBuffSize/2);
	}
	else
	{
		if (inpw(REG_ACTL_RSR) & R_DMA_END_IRQ)
			outpw(REG_ACTL_RSR, R_DMA_END_IRQ);
		else
			{ _error_msg("adc_rec_isr - miss end!\n");	}
		_bRecDmaToggle = 0;

		_tADDA.fnRecCallBack((UINT8 *)(_uAuRecBuffAddr + _uAuRecBuffSize/2), 
									_uAuRecBuffSize/2);
	}
#ifdef ECOS
	return CYG_ISR_HANDLED;
#endif	
}


static int adda_init()
{
	/* Enable AD/DA clock */
	outpw(REG_CLKCON, inpw(REG_CLKCON) | 0x40000000);
		
	/* enable audio controller and ADC, DAC interface */
	outpw(REG_ACTL_CON, inpw(REG_ACTL_CON) | AUDIO_EN | AUDCLK_EN | PFIFO_EN | RFIFO_EN | T_DMA_IRQ | R_DMA_IRQ | DMA_EN);
	Delay(100);

	/* reset Audio Controller */
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | ACTL_RESET_BIT);
	Delay(100);
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~ACTL_RESET_BIT);
	Delay(100);

	
	/* reset ADC interface */
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | ADC_RESET);
	Delay(200); 
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~ADC_RESET);
	Delay(100);
	
	
	/* reset DAC interface */
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | DAC_RESET);
	Delay(1000);
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~DAC_RESET);
	Delay(100);
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      dacSetPlayVolume                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Set DAC left and right channel play volume.                      */
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
INT  dacSetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol)
{
	_tADDA.sPlayVolume = (ucLeftVol << 8) | ucRightVol;
	outpw(REG_ACTL_DACON, 0x2788F800 | 
	                 ((_tADDA.sPlayVolume >> 8) << 11) | 
					adda_get_sampling_rate(_tADDA.nPlaySamplingRate));
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      adcSetRecordVolume                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Set ADC single channel record volume.                            */
/*                                                                       */
/* INPUTS                                                                */
/*      ucVolume    record volume                                        */
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
INT  adcSetRecordVolume(UINT8 ucVolume)
{
	_tADDA.sRecVolume = ucVolume;
	if (_tADDA.sRecVolume==0)
		outpw(REG_ACTL_ADCON, 0x2000 | (1<<1) | adda_get_sampling_rate(_tADDA.nRecSamplingRate));
	else
		outpw(REG_ACTL_ADCON, 0x2000 | (_tADDA.sRecVolume << 8) | adda_get_sampling_rate(_tADDA.nRecSamplingRate));
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      dacStartPlay                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Start DAC playback.                                              */
/*                                                                       */
/* INPUTS                                                                */
/*      fnCallBack  client program provided callback function. The audio */
/*                  driver will call back to get next block of PCM data  */
/*      nSamplingRate  the playback sampling rate. Supported sampling    */
/*                  rate are 48000, 44100, 32000, 24000, 22050, 16000,   */
/*                  11025, and 8000 Hz                                   */
/*      nChannels	number of playback nChannels                         */
/*					1: single channel, otherwise: double nChannels       */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
INT  dacStartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels)
{
	INT		nStatus;

	if (_bADDA_Active & DAC_PLAY_ACTIVE)
		return ERR_DAC_PLAY_ACTIVE;		/* DAC was playing */
	
	if (_bADDA_Active == 0)
	{
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | PLAY_LEFT_CHNNEL);
		if (nChannels != 1)
			outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | PLAY_RIGHT_CHNNEL);

		nStatus = adda_init();
		if (nStatus < 0)
			return nStatus;	
	}

	/* Install DAC playback interrupt */
#ifdef ECOS
	cyg_interrupt_disable();
	sysSetInterruptType(AU_PLAY_INT_NUM, LOW_LEVEL_SENSITIVE);
	cyg_interrupt_create(AU_PLAY_INT_NUM, 1, 0, dac_play_isr, NULL, 
					&_tADDA.int_handle_play, &_tADDA.int_holder_play);
	cyg_interrupt_attach(_tADDA.int_handle_play);
	cyg_interrupt_unmask(AU_PLAY_INT_NUM);
	cyg_interrupt_enable();
#else	
	sysSetInterruptType(AU_PLAY_INT_NUM, LOW_LEVEL_SENSITIVE);
	sysInstallISR(IRQ_LEVEL_1, AU_PLAY_INT_NUM, (PVOID)dac_play_isr);
  sysSetLocalInterrupt(ENABLE_IRQ);
	sysEnableInterrupt(AU_PLAY_INT_NUM);
#endif

	_tADDA.fnPlayCallBack = fnCallBack;
	_tADDA.nPlaySamplingRate = nSamplingRate;

	/* set DMA play destination base address */
	outpw(REG_ACTL_PDSTB, _uAuPlayBuffAddr | 0x10000000);
	
	/* set DMA play buffer length */
	outpw(REG_ACTL_PDST_LENGTH, _uAuPlayBuffSize);

	/* call back to fill DMA play buffer */
	_tADDA.bPlayLastBlock = 0;
	_tADDA.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, _uAuPlayBuffSize/2);
	_tADDA.fnPlayCallBack((UINT8 *)(_uAuPlayBuffAddr + _uAuPlayBuffSize/2), 
									_uAuPlayBuffSize);

	outpw(REG_ACTL_DACON, 0x2788F800 | 
	                 ((_tADDA.sPlayVolume >> 8) << 11) | 
					adda_get_sampling_rate(nSamplingRate));

	/* start playing */
	_bPlayDmaToggle = 0;
	outpw(REG_ACTL_PSR, 0x3);
	outpw(REG_ACTL_CON, inpw(REG_ACTL_CON) | DAC_EN);
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | DAC_PLAY);
	_bADDA_Active |= DAC_PLAY_ACTIVE;
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      dacStopPlay                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Stop DAC playback immdediately.                                  */
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
INT  dacStopPlay()
{
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL));
	
	if (!(_bADDA_Active & DAC_PLAY_ACTIVE))
		return 0;

	/* stop playing */
	_debug_msg("DAC stop playing\n");
	outpw(REG_ACTL_CON, inpw(REG_ACTL_CON) & ~DAC_EN);
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~DAC_PLAY);
	outpw(REG_ACTL_PSR, 0x3);
	
	/* disable audio play interrupt */
#ifdef ECOS
	cyg_interrupt_mask(AU_PLAY_INT_NUM);
	cyg_interrupt_detach(_tADDA.int_handle_play);
#else	
	sysDisableInterrupt(AU_PLAY_INT_NUM);
#endif	
	_bADDA_Active &= ~DAC_PLAY_ACTIVE;

	if (_bADDA_Active == 0)
	{
		/* Disable AD/DA clock */
		outpw(REG_CLKCON, inpw(REG_CLKCON) & ~0x40000000);
	}
	
	return 0;
}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      adcStartRecord                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Start ADC record.                                                */
/*                                                                       */
/* INPUTS                                                                */
/*      fnCallBack  client program provided callback function. The audio */
/*                  driver will call back to deliver the newly recorded  */
/*                  block of PCM data                                    */
/*      nSamplingRate  the record sampling rate. Supported sampling      */
/*                  rate are 48000, 44100, 32000, 24000, 22050, 16000,   */
/*                  11025, and 8000 Hz                                   */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
INT  adcStartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate)
{
	INT		nStatus;

	if (_bADDA_Active & ADC_REC_ACTIVE)
		return ERR_ADC_REC_ACTIVE;		/* ADDA was recording */
	
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(RECORD_LEFT_CHNNEL | RECORD_RIGHT_CHNNEL));
	if (_bIsLeftCH)
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | RECORD_LEFT_CHNNEL);
	else
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | RECORD_RIGHT_CHNNEL);
	
	/* set DMA record destination base address */
	outpw(REG_ACTL_RDSTB, _uAuRecBuffAddr | 0x10000000);
	
	/* set DMA record buffer length */
	outpw(REG_ACTL_RDST_LENGTH, _uAuRecBuffSize);
	
	if (_bADDA_Active == 0)
	{
		nStatus = adda_init();
		if (nStatus < 0)
			return nStatus;	
	}




	_tADDA.fnRecCallBack = fnCallBack;
	_tADDA.nRecSamplingRate = nSamplingRate;

#ifdef ECOS
	cyg_interrupt_disable();
	sysSetInterruptType(AU_REC_INT_NUM, LOW_LEVEL_SENSITIVE);
	cyg_interrupt_create(AU_REC_INT_NUM, 1, 0, adc_rec_isr, NULL, 
					&_tADDA.int_handle_rec, &_tADDA.int_holder_rec);
	cyg_interrupt_attach(_tADDA.int_handle_rec);
	cyg_interrupt_unmask(AU_REC_INT_NUM);
	cyg_interrupt_enable();
#else	
	/* Install ADC record interrupt */
	sysSetInterruptType(AU_REC_INT_NUM, LOW_LEVEL_SENSITIVE);
	sysInstallISR(IRQ_LEVEL_1, AU_REC_INT_NUM, (PVOID)adc_rec_isr);
    sysSetLocalInterrupt(ENABLE_IRQ);	/* enable CPSR I bit */
//	sysSetAIC2SWMode();					/* Set AIC into SW mode */
	sysEnableInterrupt(AU_REC_INT_NUM);
#endif

	outpw(REG_ACTL_ADCON, 0x2000 | (_tADDA.sRecVolume << 8) | adda_get_sampling_rate(nSamplingRate));

	/* start recording */
	_debug_msg("ADC start recording...\n");
	_bRecDmaToggle = 0;
	outpw(REG_ACTL_RSR, 0x3);
	outpw(REG_ACTL_CON, inpw(REG_ACTL_CON) | ADC_EN);
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | ADC_RECORD);
	_bADDA_Active |= ADC_REC_ACTIVE;
	
		/* install adc record interrupt */	


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
INT  adcStopRecord()
{
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~RECORD_LEFT_CHNNEL);

	if (!(_bADDA_Active & ADC_REC_ACTIVE))
		return ERR_ADC_REC_INACTIVE;
	
	_debug_msg("ADC stop recording\n");

	/* stop recording */
	outpw(REG_ACTL_CON, inpw(REG_ACTL_CON) & ~ADC_EN);
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~ADC_RECORD);
	outpw(REG_ACTL_RSR, 0x3);
	
	/* disable audio record interrupt */
#ifdef ECOS
	cyg_interrupt_mask(AU_REC_INT_NUM);
	cyg_interrupt_detach(_tADDA.int_handle_rec);
#else	
	sysDisableInterrupt(AU_REC_INT_NUM);
#endif
	
	_bADDA_Active &= ~ADC_REC_ACTIVE;

	if (_bADDA_Active == 0)
	{
		/* Disable AD/DA clock */
		outpw(REG_CLKCON, inpw(REG_CLKCON) & ~0x40000000);
	}
	return 0;
}


#endif	/* HAVE_ADDA */
/****************************************************************************
 * 
 * FILENAME
 *     W99702_IIS.c
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
 *     2004.07.16		Created by Shih-Jen Lu
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

#include "IIS.h"

#ifdef NUCLEUS_LIB
#include "nucleus.h"
#endif


#ifdef HAVE_IIS_D256

static AUDIO_T	_tIIS;

#define	IIS_ACTIVE				0x1
#define	IIS_PLAY_ACTIVE			0x2
#define IIS_REC_ACTIVE			0x4

static BOOL	 _bIISActive = 0;
static UINT32 _uIISCR = 0;

static VOID IIS_Set_Data_Format(INT);
static VOID IIS_Set_Sample_Frequency(INT);

static void Delay(int nCnt)
{
#if 0
	volatile int  loop;
	for (loop=0; loop<nCnt*10; loop++);
#else
	audioUSleep (nCnt * 13 * 10);
#endif
}

#ifdef ECOS
cyg_uint32  iis_play_isr(cyg_vector_t vector, cyg_addrword_t data)
#else
static void  iis_play_isr()
#endif
{
	outpw(REG_ACTL_CON, inpw(REG_ACTL_CON) | T_DMA_IRQ);
	
	if (_tIIS.bPlayLastBlock)
	{
		outpw(REG_ACTL_PSR, P_DMA_MIDDLE_IRQ | P_DMA_END_IRQ);
		i2sStopPlay();
	}

	if (inpw(REG_ACTL_PSR) & P_DMA_MIDDLE_IRQ)
	{
		outpw(REG_ACTL_PSR, P_DMA_MIDDLE_IRQ);
		_tIIS.bPlayLastBlock = _tIIS.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, 
												_uAuPlayBuffSize/2);
	}
	else if (inpw(REG_ACTL_PSR) & P_DMA_END_IRQ)
	{
		outpw(REG_ACTL_PSR, P_DMA_END_IRQ);
		_tIIS.bPlayLastBlock = _tIIS.fnPlayCallBack((UINT8 *)(_uAuPlayBuffAddr + _uAuPlayBuffSize/2), 
									_uAuPlayBuffSize/2);
	}
#ifdef ECOS
	return CYG_ISR_HANDLED;
#endif	
}		



#ifdef ECOS
cyg_uint32  iis_rec_isr(cyg_vector_t vector, cyg_addrword_t data)
#else
static void  iis_rec_isr()
#endif
{
	
	outpw(REG_ACTL_CON, inpw(REG_ACTL_CON) | R_DMA_IRQ);

	if (inpw(REG_ACTL_RSR) & R_DMA_MIDDLE_IRQ)
	{
		outpw(REG_ACTL_RSR, R_DMA_MIDDLE_IRQ);
		_tIIS.fnRecCallBack((UINT8 *)_uAuRecBuffAddr, _uAuRecBuffSize/2);
	}
	else if (inpw(REG_ACTL_RSR) & R_DMA_END_IRQ)
	{
		outpw(REG_ACTL_RSR, R_DMA_END_IRQ);
		_tIIS.fnRecCallBack((UINT8 *)(_uAuRecBuffAddr + _uAuRecBuffSize/2), 
									_uAuRecBuffSize/2);
	}
#ifdef ECOS
	return CYG_ISR_HANDLED;
#endif	
}


static INT  iis_init()
{
	/* reset audio interface */
	outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) | ACTL_RESET_BIT);
	Delay(100);
	outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) & ~ACTL_RESET_BIT);
	Delay(100);
	
	if (!_bADDA_Active)
	{
		outpw(REG_CLKSEL,inpw(REG_CLKSEL) & ~(3<<4) | ((inpw(REG_CLKSEL)&(3<<8))>>4));//ACLK use the same clock source with HCLK2
		Delay(100);
	}
	/* reset IIS interface */
	outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) | IIS_RESET);
	Delay(100);
	outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) & ~IIS_RESET);
	Delay(100);
	/* enable audio controller and IIS interface */
	outpw(REG_ACTL_CON, inpw (REG_ACTL_CON) | AUDCLK_EN | PFIFO_EN | DMA_EN | IIS_EN | AUDIO_EN | RFIFO_EN | T_DMA_IRQ | R_DMA_IRQ);
	if (!_bADDA_Active)
	{
		outpw(REG_CLKSEL,inpw(REG_CLKSEL) & ~(3<<4) | (2<<4));//ACLK use APLL
	}
	
	
	_uIISCR = 0;
	return 0;
}







/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      i2sStartPlay                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Start IIS playback.                                             */
/*                                                                       */
/* INPUTS                                                                */
/*      fnCallBack     client program provided callback function. The audio */
/*                  driver will call back to get next block of PCM data  */
/*      nSamplingRate  the playback sampling rate. Supported sampling    */
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
INT  i2sStartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
								INT nChannels, INT data_format)
{
	INT		nStatus;
	
	if (_bIISActive & IIS_PLAY_ACTIVE)
		return ERR_IIS_PLAY_ACTIVE;		/* IIS was playing */

	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL));
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | PLAY_LEFT_CHNNEL);
	if (nChannels != 1)
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | PLAY_RIGHT_CHNNEL);
	
	if (_bIISActive == 0)
	{
		nStatus = iis_init();
		if (nStatus < 0)
			return nStatus;	
	}
	
	
	
	/* Install IIS play interrupt */
#ifdef ECOS
	cyg_interrupt_disable();
	sysSetInterruptType(AU_PLAY_INT_NUM, LOW_LEVEL_SENSITIVE);
	//cyg_interrupt_configure(AU_PLAY_INT_NUM, 1 , 0);
	cyg_interrupt_create(AU_PLAY_INT_NUM, 1, 0, iis_play_isr, NULL, 
					&_tIIS.int_handle_play, &_tIIS.int_holder_play);
	cyg_interrupt_attach(_tIIS.int_handle_play);
	cyg_interrupt_unmask(AU_PLAY_INT_NUM);
	cyg_interrupt_enable();
#else	
	sysSetInterruptType(AU_PLAY_INT_NUM, LOW_LEVEL_SENSITIVE);
	sysInstallISR(IRQ_LEVEL_1, AU_PLAY_INT_NUM, (PVOID)iis_play_isr);
  sysSetLocalInterrupt(ENABLE_IRQ);	/* enable CPSR I bit */
	sysEnableInterrupt(AU_PLAY_INT_NUM);
#endif

	_tIIS.fnPlayCallBack = fnCallBack;
	_tIIS.nPlaySamplingRate = nSamplingRate;



	/* set play sampling rate and data format */
	IIS_Set_Sample_Frequency(nSamplingRate);
	IIS_Set_Data_Format(data_format);
	
	
	/* set DMA play destination base address */
	outpw(REG_ACTL_PDSTB, _uAuPlayBuffAddr | 0x10000000);
	
	/* set DMA play buffer length */
	outpw(REG_ACTL_PDST_LENGTH, _uAuPlayBuffSize);

	/* call back to fill DMA play buffer */
	_tIIS.bPlayLastBlock = 0;
	_tIIS.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, _uAuPlayBuffSize/2);
	_tIIS.fnPlayCallBack((UINT8 *)(_uAuPlayBuffAddr + _uAuPlayBuffSize/2),
								_uAuPlayBuffSize/2);
	
	/* start playing */
	_debug_msg("IIS start playing...\n");
	outpw(REG_ACTL_PSR, 0x3);
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | IIS_PLAY);
	_bIISActive |= IIS_PLAY_ACTIVE;
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      i2sStopPlay                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Stop IIS playback immdediately.                                 */
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
INT  i2sStopPlay()
{

	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL));

	if (!(_bIISActive & IIS_PLAY_ACTIVE))
		return ERR_IIS_PLAY_INACTIVE;
	
	_debug_msg("IIS stop playing\n");

	/* stop playing */
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~IIS_PLAY);


	/* disable audio play interrupt */
#ifdef ECOS
	cyg_interrupt_mask(AU_PLAY_INT_NUM);
	cyg_interrupt_detach(_tIIS.int_handle_play);
#else	
	sysDisableInterrupt(AU_PLAY_INT_NUM);
#endif
	
	_bIISActive &= ~IIS_PLAY_ACTIVE;
	
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      i2sStartRecord                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Start IIS record.                                               */
/*                                                                       */
/* INPUTS                                                                */
/*      fnCallBack     client program provided callback function. The audio */
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
INT  i2sStartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
							INT nChannels, INT data_format)
{
	INT		nStatus;

	if (_bIISActive & IIS_REC_ACTIVE)
		return ERR_IIS_REC_ACTIVE;		/* IIS was recording */
	
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(RECORD_LEFT_CHNNEL | RECORD_RIGHT_CHNNEL));
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | RECORD_LEFT_CHNNEL);
	if (nChannels != 1)
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | RECORD_RIGHT_CHNNEL);
	
	if (_bIISActive == 0)
	{
		nStatus = iis_init();
		if (nStatus < 0)
			return nStatus;	
	}
	

	/* Install IIS record interrupt */
#ifdef ECOS
	cyg_interrupt_disable();
	sysSetInterruptType(AU_REC_INT_NUM, LOW_LEVEL_SENSITIVE);
	//cyg_interrupt_configure(AU_REC_INT_NUM, 1 , 0);
	cyg_interrupt_create(AU_REC_INT_NUM, 1, 0, iis_rec_isr, NULL, 
					&_tIIS.int_handle_rec, &_tIIS.int_holder_rec);
	cyg_interrupt_attach(_tIIS.int_handle_rec);
	cyg_interrupt_unmask(AU_REC_INT_NUM);
	cyg_interrupt_enable();
#else	
	sysSetInterruptType(AU_REC_INT_NUM, LOW_LEVEL_SENSITIVE);
	sysInstallISR(IRQ_LEVEL_1, AU_REC_INT_NUM, (PVOID)iis_rec_isr);
  sysSetLocalInterrupt(ENABLE_IRQ);	/* enable CPSR I bit */
	sysEnableInterrupt(AU_REC_INT_NUM);
#endif

	_tIIS.fnRecCallBack = fnCallBack;
	_tIIS.nRecSamplingRate = nSamplingRate;

	/* set record sampling rate and data format */
	IIS_Set_Sample_Frequency(nSamplingRate);
	IIS_Set_Data_Format(data_format);
	
	

	/* set DMA record destination base address */
	outpw(REG_ACTL_RDSTB, _uAuRecBuffAddr | 0x10000000);
	
	/* set DMA record buffer length */
	outpw(REG_ACTL_RDST_LENGTH, _uAuRecBuffSize);
	
	/* start recording */
	_debug_msg("IIS start recording...\n");
	outpw(REG_ACTL_RSR, 0x3);
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | IIS_RECORD);
	_bIISActive |= IIS_REC_ACTIVE;
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      i2sStopRecord                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Stop IIS record immediately.                                    */
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
INT  i2sStopRecord()
{
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(RECORD_RIGHT_CHNNEL | RECORD_LEFT_CHNNEL));

	if (!(_bIISActive & IIS_REC_ACTIVE))
		return ERR_IIS_REC_INACTIVE;
	
	_debug_msg("IIS stop recording\n");

	/* stop recording */
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~IIS_RECORD);

	
	/* disable audio record interrupt */
#ifdef ECOS
	cyg_interrupt_mask(AU_REC_INT_NUM);
	cyg_interrupt_detach(_tIIS.int_handle_rec);
#else	
	sysDisableInterrupt(AU_REC_INT_NUM);
#endif
	
	_bIISActive &= ~IIS_REC_ACTIVE;
	return 0;
}
/*----- set data format -----*/
static void IIS_Set_Data_Format(int choose_format){

	switch(choose_format){
		case IIS_FORMAT: _uIISCR = _uIISCR | IIS;
				break;
		case MSB_FORMAT: _uIISCR = _uIISCR | MSB_Justified;
				break;
		default:break;
	}
	outpw(REG_ACTL_IISCON,_uIISCR);
}

/*----- set sample Frequency -----*/
/* APLL setting 8K:24.576Mhz 11.025K:22.579Mhz*/
static void IIS_Set_Sample_Frequency(int choose_sf){

	switch (choose_sf)
	{
		case AU_SAMPLE_RATE_8000:							//8KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_12 | BCLK_32;
			break;
		case AU_SAMPLE_RATE_11025:					//11.025KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_8 | BCLK_32;
			break;
		case AU_SAMPLE_RATE_12000:					//12KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_8 | BCLK_32;
			break;	
		case AU_SAMPLE_RATE_16000:						//16KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_6 | BCLK_32;
			

			break;
		case AU_SAMPLE_RATE_22050:					//22.05KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_4 | BCLK_32;
			break;
		case AU_SAMPLE_RATE_24000:						//24KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_4 | BCLK_32;
			

			break;
		case AU_SAMPLE_RATE_32000:						//32KHz
			_uIISCR = _uIISCR | SCALE_3 | FS_256  | BCLK_32;
			break;
		case AU_SAMPLE_RATE_44100:					//44.1KHz
			_uIISCR = _uIISCR | SCALE_2 | FS_256  | BCLK_32;
			
			break;
		case AU_SAMPLE_RATE_48000:						//48KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_2 | BCLK_32;
			

			break;
		default:break;
	}
	outpw(REG_ACTL_IISCON,_uIISCR);
}



#endif	/* HAVE_IIS */
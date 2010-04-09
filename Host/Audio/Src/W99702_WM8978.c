/****************************************************************************
 * 
 * FILENAME
 *     W99702_WM8978.c
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
#include "WM8978.h"


#ifdef HAVE_WM8978

short _wm8978_reg[58] = {
	0, 
	0, 
	0, 
	0, 
	0x050, 
	0, 
	0x140, 
	0, 
	0, 
	0, 
	0, 
	0x0FF, 
	0x0FF, 
	0, 
	0x100, 
	0x0FF, 
	0x0FF, 
	0, 
	0x12C, 
	0x02C, 
	0x02C, 
	0x02C, 
	0x02C, 
	0,
	0x032, 
	0,
	0, 
	0, 
	0, 
	0, 
	0,
	0, 
	0x038, 
	0x00B,
	0x032,
	0,
	0x008, 
	0x00C, 
	0x093, 
	0x0E9, 
	0,
	0, 
	0,
	0,
	0x033, 
	0x010, 
	0x010, 
	0x100, 
	0x100, 
	0x002, 
	0x001, 
	0x001, 
	0x039, 
	0x039, 
	0x039, 
	0x039, 
	0x001, 
	0x001
};

#define HWPOINT(x)	((CHAR *)inpw(0x7FF03020))

static AUDIO_T	_tIIS = {0};
static int _audio_play_channels = 0;

#define	IIS_ACTIVE				0x1
#define	IIS_PLAY_ACTIVE			0x2
#define IIS_REC_ACTIVE			0x4
#define RETRYCOUNT		10
#define WMDEVID 0

static BOOL	 _bIISActive = 0;
static UINT32 _uIISCR = 0;
static int _headphone_detection_leve = 1; /* headphone/speaker auto detection io level */

/* WM8978 register value */
static INT volatile _nR1val = 0;
static INT volatile _nR2val = 0;
static INT volatile _nR3val = 0;
static INT volatile _nR44val = 0x033;
static INT volatile _nR45val = 0x010;
static INT volatile _nR46val = 0x010;
static INT volatile _nR47val = 0x100;
static INT volatile _nR48val = 0x100;
/* WM8978 register value */



INT WM8978_DAC_Setup(void);
INT WM8978_ADC_Setup(void);

static VOID WM_Set_Sample_Frequency(INT);


#ifdef _3WIRE
void ThreeWire_Write_Data(UINT16);
#define WM_Write_Data ThreeWire_Write_Data
#elif defined(_2WIRE)
INT TwoWire_Write_Data(UINT16,UINT16);
#define WM_Write_Data TwoWire_Write_Data
#endif
static INT WM_Set_DAC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol);
static INT WM_Set_ADC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol);
static VOID IIS_Set_Data_Format(INT);
static VOID IIS_Set_Sample_Frequency(INT);


extern AU_I2C_TYPE_T _eI2SType;
extern BOOL	_bIsLeftCH;
extern BOOL _bOutputPathEnable[];
extern BOOL _bInputPathEnable[];

static void Delay(int nCnt)
{
	volatile int  loop;
	for (loop=0; loop<nCnt*40; loop++);
}

int REG_ACTL_PSR_Play;
int REG_ACTL_PSR_Record;


static volatile int _play_int_cnt = 0;
static volatile int _last_play_int_cnt = 0;
static volatile int _last_play_int_time = 0;
static volatile int _play_int_rate = 0;
static int _ref_play_int_rate = 0;

int audioPlayIntRate(void)
{
	return _play_int_rate;
}

#ifdef ECOS
cyg_uint32  iis_play_isr(cyg_vector_t vector, cyg_addrword_t data)
#else
static void  iis_play_isr()
#endif
{
	int time;
	
#ifdef ECOS
	cyg_interrupt_mask(vector);
#endif

	if(!_play_int_cnt && 0)
	{
		outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) | IIS_RESET);
		Delay(100);
		outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) & ~IIS_RESET);
	}
	
	_play_int_cnt++;
	time = cyg_current_time();
	if( 0 && time - _last_play_int_time > 100)
	{
		_play_int_rate = _play_int_cnt - _last_play_int_cnt;
		_last_play_int_cnt = _play_int_cnt;
		_last_play_int_time = time;

		if((_play_int_rate > _ref_play_int_rate + 3) || (_play_int_rate < _ref_play_int_rate - 3))
		{
			outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) | IIS_RESET);
			Delay(100);
			outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) & ~IIS_RESET);
			//diag_printf("reset IIS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		}
	}
	
	audioPlayCheck(_audio_play_channels);		
	
	outpw(REG_ACTL_CON, inpw(REG_ACTL_CON) | T_DMA_IRQ);

	REG_ACTL_PSR_Play = inpw(REG_ACTL_PSR);
	
	if ( REG_ACTL_PSR_Play & P_DMA_MIDDLE_IRQ)
	{
		outpw(REG_ACTL_PSR, P_DMA_MIDDLE_IRQ);
#ifndef ECOS
		_tIIS.bPlayLastBlock = _tIIS.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, 
												_uAuPlayBuffSize/2);
#endif
	}
	else if ( REG_ACTL_PSR_Play & P_DMA_END_IRQ)
	{
		outpw(REG_ACTL_PSR, P_DMA_END_IRQ);
#ifndef ECOS
		_tIIS.bPlayLastBlock = _tIIS.fnPlayCallBack((UINT8 *)(_uAuPlayBuffAddr + _uAuPlayBuffSize/2), 
									_uAuPlayBuffSize/2);
#endif
	}

#ifdef ECOS
	cyg_interrupt_acknowledge(vector);
	return CYG_ISR_CALL_DSR;
//	return CYG_ISR_HANDLED;
#endif	
}		

#ifdef ECOS
void iis_play_isr_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
	if (REG_ACTL_PSR_Play & P_DMA_MIDDLE_IRQ)
	{
		_tIIS.bPlayLastBlock = _tIIS.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, 
												_uAuPlayBuffSize/2);
	}
	else if (REG_ACTL_PSR_Play & P_DMA_END_IRQ)
	{
		_tIIS.bPlayLastBlock = _tIIS.fnPlayCallBack((UINT8 *)(_uAuPlayBuffAddr + _uAuPlayBuffSize/2), 
									_uAuPlayBuffSize/2);
	}
	cyg_interrupt_acknowledge(vector);
	cyg_interrupt_unmask(vector);
}
#endif

#ifdef ECOS
cyg_uint32  iis_rec_isr(cyg_vector_t vector, cyg_addrword_t data)
#else
static void  iis_rec_isr()
#endif
{
	
#ifdef ECOS
	cyg_interrupt_mask(vector);
#endif
	outpw(REG_ACTL_CON, inpw(REG_ACTL_CON) | R_DMA_IRQ);

#ifdef ECOS
	REG_ACTL_PSR_Record = inpw(REG_ACTL_RSR);
#endif
	if (inpw(REG_ACTL_RSR) & R_DMA_MIDDLE_IRQ)
	{
		outpw(REG_ACTL_RSR, R_DMA_MIDDLE_IRQ);
#ifndef ECOS
		_tIIS.fnRecCallBack((UINT8 *)_uAuRecBuffAddr, _uAuRecBuffSize/2);
#endif
	}
	else if (inpw(REG_ACTL_RSR) & R_DMA_END_IRQ)
	{
		outpw(REG_ACTL_RSR, R_DMA_END_IRQ);
#ifndef ECOS
		_tIIS.fnRecCallBack((UINT8 *)(_uAuRecBuffAddr + _uAuRecBuffSize/2), 
									_uAuRecBuffSize/2);
#endif
	}
#ifdef ECOS
	cyg_interrupt_acknowledge(vector);
	return CYG_ISR_CALL_DSR;
//	return CYG_ISR_HANDLED;
#endif	
}

#ifdef ECOS
void iis_rec_isr_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
	if (REG_ACTL_PSR_Record & R_DMA_MIDDLE_IRQ)
	{
		_tIIS.fnRecCallBack((UINT8 *)_uAuRecBuffAddr, _uAuRecBuffSize/2);
	}
	else if (REG_ACTL_PSR_Record & R_DMA_END_IRQ)
	{
		_tIIS.fnRecCallBack((UINT8 *)(_uAuRecBuffAddr + _uAuRecBuffSize/2), 
									_uAuRecBuffSize/2);
	}
	cyg_interrupt_acknowledge(vector);
	cyg_interrupt_unmask(vector);
}
#endif

INT audioPlayCheck(int nChannels)
{
	unsigned int reg;
	
	reg = PLAY_LEFT_CHNNEL;
	if(nChannels == 2)
		reg |= PLAY_RIGHT_CHNNEL;
	if( (inpw(REG_ACTL_RESET) & (PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL)) != reg)
	{
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL));
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | reg | IIS_RESET);
		outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) & ~(IIS_RESET));
	}	
	
	return 0;
}

INT audioI2SInit()
{
	unsigned int reg;

	if((inpw(REG_ACTL_RESET) & IIS_PLAY) == 0)
	{
		outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) | ACTL_RESET_BIT | IIS_RESET);
		outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) & ~(ACTL_RESET_BIT | IIS_RESET));
	}
	
	reg = PLAY_LEFT_CHNNEL;
	reg |= PLAY_RIGHT_CHNNEL;
	if( inpw(REG_ACTL_RESET) & (PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL) != reg)
	{
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL));
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | reg);
	}	
		
	/* enable audio controller and IIS interface */
	//reg = AUDCLK_EN | PFIFO_EN | DMA_EN | IIS_EN | AUDIO_EN | RFIFO_EN | T_DMA_IRQ | R_DMA_IRQ;
	reg = AUDCLK_EN | PFIFO_EN | DMA_EN;
	if( (inpw (REG_ACTL_CON) & reg) != reg)
		outpw(REG_ACTL_CON, inpw (REG_ACTL_CON) | reg);
	//outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) & ~IIS_RESET);

	outpw(REG_ACTL_CON,inpw(REG_ACTL_CON) | IIS_EN);
	reg = PLAY_LEFT_CHNNEL;
	reg |= PLAY_RIGHT_CHNNEL;
	if( (inpw(REG_ACTL_RESET) & (PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL)) != reg)
	{
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL));
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | reg | IIS_RESET);
		outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) & ~(IIS_RESET));
	}	


	return 0;
	//for(i=0;i<100;i++)
	{
	/* reset audio interface */
	outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) | ACTL_RESET_BIT);
	//Delay(100);
	outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) & ~ACTL_RESET_BIT);
	//Delay(100);
	
	/* reset IIS interface */
	outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) | IIS_RESET);
	//Delay(100);
	outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) & ~IIS_RESET);
	//Delay(100);
	/* enable audio controller and IIS interface */
	//outpw(REG_ACTL_CON, inpw (REG_ACTL_CON) | AUDCLK_EN | PFIFO_EN | DMA_EN | IIS_EN | AUDIO_EN | RFIFO_EN | T_DMA_IRQ | R_DMA_IRQ);
	outpw(REG_ACTL_CON, inpw (REG_ACTL_CON) | IIS_EN);
	//Delay(100);
	outpw(REG_ACTL_CON, inpw (REG_ACTL_CON) | AUDCLK_EN | PFIFO_EN | DMA_EN  | AUDIO_EN | RFIFO_EN | R_DMA_IRQ);
	}
	return 0;
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
		//outpw(REG_CLKSEL,inpw(REG_CLKSEL) & ~(3<<4) | ((inpw(REG_CLKSEL)&(3<<8))>>4));//ACLK use the same clock source with HCLK2
		//Delay(100);
	}
	/* reset IIS interface */
	outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) | IIS_RESET);
	Delay(100);
	outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) & ~IIS_RESET);
	Delay(100);
	/* enable audio controller and IIS interface */
	outpw(REG_ACTL_CON, inpw (REG_ACTL_CON) | AUDCLK_EN | PFIFO_EN | DMA_EN | IIS_EN | AUDIO_EN | RFIFO_EN | T_DMA_IRQ | R_DMA_IRQ);
	Delay(100);
	//outpw(REG_ACTL_CON, inpw (REG_ACTL_CON) | AUDCLK_EN | IIS_EN | AUDIO_EN | RFIFO_EN | R_DMA_IRQ);
	//outpw(REG_ACTL_CON, inpw (REG_ACTL_CON) | DMA_EN);
//outpw(REG_ACTL_CON, inpw (REG_ACTL_CON) | PFIFO_EN);
//outpw(REG_ACTL_CON, inpw (REG_ACTL_CON) | RFIFO_EN);
//outpw(REG_ACTL_CON, inpw (REG_ACTL_CON) | DMA_EN);
//outpw(REG_ACTL_CON, inpw (REG_ACTL_CON) | IIS_EN);
//outpw(REG_ACTL_CON, inpw (REG_ACTL_CON) | T_DMA_IRQ);
//outpw(REG_ACTL_CON, inpw (REG_ACTL_CON) | R_DMA_IRQ);
//outpw(REG_ACTL_CON, inpw (REG_ACTL_CON) | AUDCLK_EN);
//outpw(REG_ACTL_CON, inpw (REG_ACTL_CON) | AUDIO_EN);

	//outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) | IIS_RESET);
	//Delay(1000);
	//outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) & ~IIS_RESET);
	//Delay(100);
	
	//outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) & ~IIS_RESET);

	
	//outpw(REG_ACTL_CON, inpw (REG_ACTL_CON) | IIS_EN);
	//Delay(100);
	//outpw(REG_ACTL_CON, inpw (REG_ACTL_CON) | AUDCLK_EN | PFIFO_EN | DMA_EN  | AUDIO_EN | RFIFO_EN | R_DMA_IRQ);
	if (!_bADDA_Active)
	{
		//outpw(REG_CLKSEL,inpw(REG_CLKSEL) & ~(3<<4) | (2<<4));//ACLK use APLL
	}



	if (_eI2SType.bIsGPIO){
		outpw(REG_GPIO_IE,inpw(REG_GPIO_IE) & ~(_eI2SType.uSDIN | _eI2SType.uSCLK));
		if (_eI2SType.uSDIN==4 || _eI2SType.uSCLK==4)
			sysDisableInterrupt(4);
		if (_eI2SType.uSDIN==5 || _eI2SType.uSCLK==5)
			sysDisableInterrupt(5);
			
	}

	_uIISCR = 0;
	return 0;
}







/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      wm8978StartPlay                                                   */
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

INT  wm8978StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
								INT nChannels, INT data_format)
{
	INT		nStatus;
	int pre_dma_pos,i,j;
	unsigned int reg;

	//CWS: Waiting for play stop (this is for key sound)
	if ( (inpw(REG_ACTL_RESET) & IIS_PLAY) )
	{
		int ticks;
		
		ticks = cyg_current_time();
		while(1)
		{
			if(cyg_current_time() - ticks > 100)
				break;/* time-out */
				
			if((inpw(REG_ACTL_RESET) & IIS_PLAY) == 0)				
				break;
		}
	}


	cyg_scheduler_lock();
	//outpw(REG_AHB_PRI, 0x2);
	//outpw(REG_BBLENG0, 0x4);
	
	_play_int_cnt = 0;
	_last_play_int_cnt = 0;
	_last_play_int_time = cyg_current_time();
	_play_int_rate = 0;
	_ref_play_int_rate = 0;

	_ref_play_int_rate = nSamplingRate*2*2*nChannels / _uAuPlayBuffSize;

	//WM_Write_Data(52,0x181);
	//WM_Write_Data(53,0x181);
	
	_audio_play_channels = nChannels;
	
	if (_bIISActive & IIS_PLAY_ACTIVE)
		return ERR_IIS_PLAY_ACTIVE;		/* IIS was playing */
#if 0 	
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL));
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | PLAY_RIGHT_CHNNEL);
	if (nChannels != 1)
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | PLAY_LEFT_CHNNEL);

	if (_bIISActive == 0)
	{
		nStatus = iis_init();
		if (nStatus < 0)
			return nStatus;
	}
	
#else

	if((inpw(REG_ACTL_RESET) & IIS_PLAY) == 0)
	{
		outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) | ACTL_RESET_BIT | IIS_RESET);
		outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) & ~(ACTL_RESET_BIT | IIS_RESET));
	}
	
	reg = PLAY_LEFT_CHNNEL;
	if(nChannels == 2)
		reg |= PLAY_RIGHT_CHNNEL;
	if( inpw(REG_ACTL_RESET) & (PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL) != reg)
	{
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL));
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | reg);
	}	
		
	/* enable audio controller and IIS interface */
	//reg = AUDCLK_EN | PFIFO_EN | DMA_EN | IIS_EN | AUDIO_EN | RFIFO_EN | T_DMA_IRQ | R_DMA_IRQ;
	reg = AUDCLK_EN | PFIFO_EN | DMA_EN;
	if( (inpw (REG_ACTL_CON) & reg) != reg)
		outpw(REG_ACTL_CON, inpw (REG_ACTL_CON) | reg);
	//outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) & ~IIS_RESET);

	outpw(REG_ACTL_CON,inpw(REG_ACTL_CON) | IIS_EN);
	reg = PLAY_LEFT_CHNNEL;
	if(nChannels == 2)
		reg |= PLAY_RIGHT_CHNNEL;
	if( (inpw(REG_ACTL_RESET) & (PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL)) != reg)
	{
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL));
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | reg | IIS_RESET);
		outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) & ~(IIS_RESET));
	}	
	


#endif
	
	/* Install IIS play interrupt */
	if (fnCallBack)
	{
#ifdef ECOS
		cyg_interrupt_disable();
	//sysSetInterruptType(AU_PLAY_INT_NUM, LOW_LEVEL_SENSITIVE);
		cyg_interrupt_configure(AU_PLAY_INT_NUM, 1 , 0);
		cyg_interrupt_create(AU_PLAY_INT_NUM, 1, 0, iis_play_isr, iis_play_isr_dsr, 
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
		/* call back to fill DMA play buffer */
		_tIIS.bPlayLastBlock = 0;
#if 0	//xhchen, do not call callback in thread, just in dsr
		_tIIS.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, _uAuPlayBuffSize/2);
		_tIIS.fnPlayCallBack((UINT8 *)(_uAuPlayBuffAddr + _uAuPlayBuffSize/2),
								_uAuPlayBuffSize/2);
#else
		memset((UINT8 *)_uAuPlayBuffAddr, 0, _uAuPlayBuffSize);
#endif
	}

	
	_tIIS.nPlaySamplingRate = nSamplingRate;
	
	
	IIS_Set_Sample_Frequency(nSamplingRate);
	IIS_Set_Data_Format(data_format);
	
	
	
	/* set DMA play destination base address */
	outpw(REG_ACTL_PDSTB, _uAuPlayBuffAddr | 0x10000000);
	
	/* set DMA play buffer length */
	outpw(REG_ACTL_PDST_LENGTH, _uAuPlayBuffSize);

	
	
	/* start playing */
	_debug_msg("IIS start playing...\n");
	outpw(REG_ACTL_PSR, 0x3);
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | IIS_PLAY);
	_bIISActive |= IIS_PLAY_ACTIVE;
		
	nStatus = WM8978_DAC_Setup();
	if (nStatus != 0)
		return nStatus;
#if 1	
	if(!(inpw(REG_ACTL_IISCON)&0x000F0000) || ( inpw(REG_CLKDIV0)&0x00F00000 == 0x00F00000 ))
	{
		unsigned int apll, audio_div, mclk_div;
		audio_get_clk_cfg(nSamplingRate, &apll, &audio_div,&mclk_div);
		audio_set_clk(apll, audio_div, mclk_div);
	}
	
	/* reset IIS interface */
	//outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) | IIS_RESET);
	//Delay(100);
	//outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) & ~IIS_RESET);
#endif	


	cyg_scheduler_unlock();

	return 0;
}

static VOID wait_buf_empty(VOID)
{
	int pre_dma_pos,i,j;
	short *lp,*rp,*lp1, *rp1;
	int lcnt, rcnt;
	char lzcr, rzcr;
	short l,r;
	int channels;
	int timeout;
	int _play_io_buf_size,_audio_io_limit;
	
	
	if( inpw(REG_ACTL_RESET) & PLAY_RIGHT_CHNNEL )
		channels = 2;
	else
		channels = 1;

	/* Mute after zero crossing */
	lzcr = rzcr = 0;
	rcnt = lcnt = 0;
	_play_io_buf_size = inpw(REG_ACTL_PDST_LENGTH);
	_audio_io_limit = inpw(REG_ACTL_PDSTB) + _play_io_buf_size;
	
	
	pre_dma_pos = (int)HWPOINT();
	timeout = _play_io_buf_size;
	while(1)
	{
		if(channels == 2)
		{
			if(!lzcr)
			{
				lp = (short *)HWPOINT() + 16;
				if((unsigned int)lp >= (unsigned int)_audio_io_limit)
					lp = lp - _play_io_buf_size/2;
					
				lp1 = lp+2;
				if((unsigned int)lp1 >= (unsigned int)_audio_io_limit)
					lp1 = lp1 - _play_io_buf_size/2;
					
			}

			if(!rzcr) 
			{
				rp = (short *)HWPOINT() + 17;
				if((unsigned int)rp >= (unsigned int)_audio_io_limit)
					rp = rp - _play_io_buf_size/2;
					
				rp1 = rp+2;
				if((unsigned int)rp1 >= (unsigned int)_audio_io_limit)
					rp1 = rp1 - _play_io_buf_size/2;
					
			}
							

			
			if(!lzcr)
			{
				if( ((*lp>>15) != (*lp1>>15)) || (*lp==0) )
				{
					lzcr = 1;
					lcnt = _play_io_buf_size/2;
				}
			}
			else
			{
				if(lcnt)
				{
					*lp = 0;
					lp++;
					if((unsigned int)lp >= (unsigned int)_audio_io_limit)
						lp = lp - _play_io_buf_size/2;
					lcnt--;
				}
			}
			
			if(!rzcr)
			{
				if( ((*rp>>15) != (*rp1>>15)) || (*rp==0) )
				{
					rzcr = 1;
					rcnt = _play_io_buf_size/2;
				}
			}
			else
			{
				if(rcnt)
				{
					*rp = 0;
					rp++;
					if((unsigned int)rp >= (unsigned int)_audio_io_limit)
						rp = rp - _play_io_buf_size/2;
					rcnt--;
				}
			}
			
			if(lzcr&rzcr)
			{
				if( (rcnt==0) && (lcnt==0) )
					break;
			}
		}
		else
		{
			if(!lzcr)
			{
				lp = (short *)HWPOINT() + 16;
				if((unsigned int)lp >= (unsigned int)_audio_io_limit)
					lp = lp - _play_io_buf_size/2;
					
				lp1 = lp+1;
				if((unsigned int)lp1 >= (unsigned int)_audio_io_limit)
					lp1 = lp1 - _play_io_buf_size/2;
					
			}

			if(!lzcr)
			{
				if( ((*lp>>15) != (*lp1>>15)) || (*lp==0) )
				{
					lzcr = 1;
					lcnt = _play_io_buf_size/2;
				}
			}
			else
			{
				if(lcnt)
				{
					*lp = 0;
					lp++;
					if((unsigned int)lp >= (unsigned int)_audio_io_limit)
						lp = lp - _play_io_buf_size/2;
					lcnt--;
				}
			}
			
			if(lzcr)
			{
				if( (lcnt==0) )
					break;
			}
		}		
		
		if( pre_dma_pos != (int)HWPOINT() )
		{
			timeout = timeout - 32;
			if(timeout <= 0) break;
		}
	}
}



INT  wm8978StopPlay()
{
	
	//WM_Write_Data(11,0x180);
	//WM_Write_Data(12,0x180);
	//WM_Write_Data(52,0x181);
	//WM_Write_Data(53,0x181);
	//WM_Set_DAC_Volume(0,0);
	//outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL));
	
	

	int pre_dma_pos,i,j;
	short *p;

	cyg_scheduler_lock();
	
	/* Mute after zero crossing */
	wait_buf_empty();


	audioSetHeadphoneVolume(-56, -56);
	audioSetSpeakerVolume(-56, -56);
	

	if (!(_bIISActive & IIS_PLAY_ACTIVE))
		return ERR_IIS_PLAY_INACTIVE;

	//outpw(REG_ACTL_RESET, (inpw(REG_ACTL_RESET) | IIS_RESET  | ACTL_RESET_BIT) & ~IIS_PLAY);
	/* stop playing */
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~IIS_PLAY);
	outpw(REG_ACTL_CON, inpw(REG_ACTL_CON) & ~IIS_EN);

	_debug_msg("IIS stop playing\n");
	//WM_Write_Data(0x0,0x00);//software reset
	//_nR3val = _nR3val &~0x003;
	//WM_Write_Data(3,_nR3val);
	/* disable audio play interrupt */
#ifdef ECOS
	if(_tIIS.int_handle_play)
	{
		cyg_interrupt_mask(AU_PLAY_INT_NUM);
		cyg_interrupt_detach(_tIIS.int_handle_play);
	}
#else	
	sysDisableInterrupt(AU_PLAY_INT_NUM);
#endif
	
	_bIISActive &= ~IIS_PLAY_ACTIVE;
	
	//audioSetDacVolume(-255, -255);
	//audioSetHeadphoneVolume(-56, -56);
	//audioSetHeadphoneVolume(-56, -56);

#if 1
	//audio_set_clk(0x652F, 0x00F00000, 0x00000000);	
	outpw(REG_CLKDIV0, inpw(REG_CLKDIV0)|0x00F00000);
	outpw(REG_ACTL_IISCON, 0x000F0000);
	
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | IIS_RESET);
	outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) | ACTL_RESET_BIT);
	
#endif


	cyg_scheduler_unlock();


	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      wm8978StartRecord                                                 */
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
INT  wm8978StartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
							INT nChannels, INT data_format)
{
	INT		nStatus;


	//CWS: Waiting for play stop (this is for key sound)
	if ( (inpw(REG_ACTL_RESET) & IIS_PLAY) )
	{
		int ticks;
		
		ticks = cyg_current_time();
		while(1)
		{
			if(cyg_current_time() - ticks > 100)
				break;/* time-out */
				
			if((inpw(REG_ACTL_RESET) & IIS_PLAY) == 0)				
				break;
		}
	}


	audioSetDacVolume(-255, -255); /* avoid beep noise */	

	if (_bIISActive & IIS_REC_ACTIVE){
		_error_msg("IIS is recording\n");
		return ERR_IIS_REC_ACTIVE;		/* IIS was recording */
	}
	
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

	if (_bIISActive == 0)
	{
		nStatus = iis_init();
		if (nStatus < 0)
			return nStatus;	
	}
	
	if (fnCallBack)
	{
	/* Install IIS record interrupt */
#ifdef ECOS
		cyg_interrupt_disable();
		//sysSetInterruptType(AU_REC_INT_NUM, LOW_LEVEL_SENSITIVE);
		cyg_interrupt_configure(AU_REC_INT_NUM, 1 , 0);
		cyg_interrupt_create(AU_REC_INT_NUM, 1, 0, iis_rec_isr, iis_rec_isr_dsr, 
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
	
	}
	_tIIS.nRecSamplingRate = nSamplingRate;


	nStatus = WM8978_ADC_Setup();
	if (nStatus!=0)
		return nStatus;

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
	
	
	if(!(inpw(REG_ACTL_IISCON)&0x000F0000) || ( inpw(REG_CLKDIV0)&0x00F00000 == 0x00F00000 ))
	{
		unsigned int apll, audio_div, mclk_div;
		audio_get_clk_cfg(nSamplingRate, &apll, &audio_div,&mclk_div);
		audio_set_clk(apll, audio_div, mclk_div);
	}
	
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      wm8978StopRecord                                                  */
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
INT  wm8978StopRecord()
{

	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(RECORD_RIGHT_CHNNEL | RECORD_LEFT_CHNNEL));

	if (!(_bIISActive & IIS_REC_ACTIVE))
		return ERR_IIS_REC_INACTIVE;
	
	_debug_msg("IIS stop recording\n");

	/* stop recording */
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~IIS_RECORD);
	
	//WM_Write_Data(0x0,0x00);//software reset
	_nR2val = _nR2val &~0x003;
	WM_Write_Data(2,_nR2val);
	/* disable audio record interrupt */
#ifdef ECOS
	cyg_interrupt_mask(AU_REC_INT_NUM);
    if(	_tIIS.int_handle_rec )
    {
	    cyg_interrupt_detach(_tIIS.int_handle_rec);
	}
#else	
	sysDisableInterrupt(AU_REC_INT_NUM);
#endif
	
	_bIISActive &= ~IIS_REC_ACTIVE;
	
	
	//audio_set_clk(0x652F, 0x00F00000, 0x00000000);	
	//outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | IIS_RESET);
	//outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) | ACTL_RESET_BIT);
	
	return 0;
}

int audioResetHold()
{
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | AC_RESET);
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | IIS_RESET);
	outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) | ACTL_RESET_BIT);
	return 0;
}

int audioResetRelease()
{
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~AC_RESET);
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~IIS_RESET);
	outpw(REG_ACTL_RESET,inpw(REG_ACTL_RESET) & ~ACTL_RESET_BIT);
	
	
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
#ifdef ECOS
/*----- set sample Frequency -----*/
/* APLL setting 8K:12.288Mhz 32K:8.192Mhz 11.025K:11.289Mhz*/
static void IIS_Set_Sample_Frequency(int choose_sf){

	switch (choose_sf)
	{
		case AU_SAMPLE_RATE_8000:							//8KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_6 | BCLK_32;
			break;
		case AU_SAMPLE_RATE_11025:					//11.025KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_4 | BCLK_32;
			break;
		case AU_SAMPLE_RATE_12000:					//12KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_4 | BCLK_32;
			break;	
		case AU_SAMPLE_RATE_16000:						//16KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_3 | BCLK_32;
			

			break;
		case AU_SAMPLE_RATE_22050:					//22.05KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_2 | BCLK_32;
			break;
		case AU_SAMPLE_RATE_24000:						//24KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_2 | BCLK_32;
			

			break;
		case AU_SAMPLE_RATE_32000:						//32KHz
			_uIISCR = _uIISCR | SCALE_1 | FS_256  | BCLK_32;
			break;
		case AU_SAMPLE_RATE_44100:					//44.1KHz
			_uIISCR = _uIISCR | SCALE_1 | FS_256  | BCLK_32;
			
			break;
		case AU_SAMPLE_RATE_48000:						//48KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_1 | BCLK_32;
			

			break;
		default:break;
	}
	outpw(REG_ACTL_IISCON,_uIISCR);
}
#else
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
#endif
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      wm8978SetPlayVolume                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Set i2S left and right channel play volume.                      */
/*                                                                       */
/* INPUTS                                                                */
/*      ucLeftVol    play volume of left channel                         */
/*      ucRightVol   play volume of left channel                         */
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
INT  wm8978SetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol)  //0~31
{

	if (ucLeftVol>31) ucLeftVol=31;
	if (ucRightVol>31) ucRightVol=31;
	_tIIS.sPlayVolume = (ucLeftVol << 8) | ucRightVol;

	WM_Set_DAC_Volume((_tIIS.sPlayVolume & 0xff00)>>8 , _tIIS.sPlayVolume & 0x00ff);
	
	return 0;
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      wm8978SetRecordVolume                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Set i2S left and right channel record volume.                      */
/*                                                                       */
/* INPUTS                                                                */
/*      ucLeftVol    record volume of left channel                         */
/*      ucRightVol   record volume of left channel                         */
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
INT  wm8978SetRecordVolume(UINT8 ucLeftVol, UINT8 ucRightVol)  //0~31
{
	if (ucLeftVol>31)
		ucLeftVol=31;
	if (ucRightVol>31)
		ucRightVol=31;
	_tIIS.sRecVolume = (ucLeftVol << 8) | ucRightVol;
	WM_Set_ADC_Volume((_tIIS.sRecVolume & 0xff00) >> 8, _tIIS.sRecVolume & 0x00ff);

	return 0;
}


/*---------- for WM8978	functions group----------*/
//(MIN)  0 ~ (MAX) 255
#define VOLUME_STEP	3		/* in dB */
#define VOLUME_MAX_VALUE	64

static int _headphone_speaker_balance = 0;
static int _volume_offset = 0;

int audioSetHeadphoneSpeakerBalance(int balance)
{
	if(balance < 0) balance = 0;
	if(balance > 0x3F ) balance = 0x3F;
	
	_headphone_speaker_balance = balance;
	_volume_offset = balance;
	
	return 0;

}

/* -57dB ~ 6dB, one step is 1dB */
int audioSetSpeakerVolume(int volume_left, int volume_right)
{
	if(volume_left > 6)  volume_left  = 6;
	if(volume_right > 6) volume_right = 6;
	if(volume_left < -57)   volume_left  = -57;
	if(volume_right < -57)  volume_right = -57;

	WM_Write_Data(54,0x180 | volume_left + 57);
	WM_Write_Data(55,0x180 | volume_right+ 57);
	
	return volume_left;
}


/* -57dB ~ 6dB, one step is 1dB */
int audioSetHeadphoneVolume(int volume_left, int volume_right)
{
	if(volume_left > 6)  volume_left  = 6;
	if(volume_right > 6) volume_right = 6;
	if(volume_left < -57)   volume_left  = -57;
	if(volume_right < -57)  volume_right = -57;

	WM_Write_Data(52,0x180 | volume_left + 57);
	WM_Write_Data(53,0x180 | volume_right+ 57);
	
	return volume_left;
}

/*-127dB ~ 0dB, one step is 0.5dB */
int audioSetDacVolume(int volume_left, int volume_right)
{
	if(volume_left  > 0)  volume_left  = 0;
	if(volume_right > 0) volume_right = 0;
	if(volume_left  < -255)   volume_left  = -255;
	if(volume_right < -255)  volume_right = -255;

	WM_Write_Data(11,0x100 | 255 + volume_left);
	WM_Write_Data(12,0x100 | 255 + volume_right);
	
	return volume_left;
}

static INT WM_Set_DAC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol){	//0~31
	/* R11:DACOUTL R12:DACOUTR, R52:HPOUTL R53:HPOUTR, R54:SPOUTL R55:SPOUTR*/
	
#if 1
	int spk_vol_left, spk_vol_right;
	int hp_vol_left, hp_vol_right;
	int dac_vol_left, dac_vol_right;
	
	dac_vol_left = 0;
	dac_vol_right= 0;
	
	hp_vol_left = ucLeftVol*2 - 56;
	hp_vol_right= ucRightVol*2 - 56;
	spk_vol_left = ucLeftVol*2 - 56 + _headphone_speaker_balance;
	spk_vol_right= ucRightVol*2 - 56 + _headphone_speaker_balance;

	/* Avoid saturation by limit the volume up to 0 dB */
	if(hp_vol_left > 0) hp_vol_left = 0;
	if(hp_vol_right > 0) hp_vol_right = 0;
	if(spk_vol_left > 0) spk_vol_left = 0;
	if(spk_vol_right > 0) spk_vol_right = 0;
	
	if(ucLeftVol < 10)ucLeftVol - 56;


	audioSetDacVolume(dac_vol_left, dac_vol_right);
	audioSetHeadphoneVolume(hp_vol_left, hp_vol_right);
	audioSetSpeakerVolume(spk_vol_left, spk_vol_right);

	/* Mute */
	if(ucLeftVol == 0)
	{
		WM_Write_Data(52, 0x140);	
		WM_Write_Data(54, 0x140);	
	}
	if(ucLeftVol == 0)
	{
		WM_Write_Data(53, 0x140);		
		WM_Write_Data(55, 0x140);	
	}

#elif 0
	int volume_left, volume_right;
	
	/* Off the volume setting accordint to perception issue. */
	if(ucLeftVol) ucLeftVol += 33;
	if(ucRightVol) ucRightVol += 33;

	/* Limit the setting range */
	if(ucLeftVol > VOLUME_MAX_VALUE) ucLeftVol = VOLUME_MAX_VALUE;
	if(((int)ucLeftVol) < (int)0) ucLeftVol = 0;
	if(ucRightVol > VOLUME_MAX_VALUE) ucRightVol = VOLUME_MAX_VALUE;
	if(((int)ucRightVol) < (int)0) ucRightVol = 0;
	
	/* Calculate the real volume in dB */
	volume_left  = (ucLeftVol - VOLUME_MAX_VALUE) * VOLUME_STEP; /* volume in dB */
	volume_right = (ucRightVol - VOLUME_MAX_VALUE) * VOLUME_STEP; /* volume in dB */
	if(volume_left < -127) volume_left = -127;
	if(volume_right < -127) volume_right = -127;

	/* Setting left volume */		
	printf("volume_left:%d\n",volume_left);
	if(ucLeftVol)
	{
		if(volume_left <= 0)
		{
			int reg,i,data;
			WM_Write_Data(11,0x100 | 0xFF + volume_left*2 - 34 /* - 20*/);

			reg = _wm8978_reg[52];
			data = 0x180 | (0x3F - _headphone_speaker_balance);
			if( reg > data )
			{
				for(i=reg;i >= data;i--) WM_Write_Data(52, i);
			}
			else
			{
				for(i=reg;i <= data;i++) WM_Write_Data(52, i);
			}
			
			//WM_Write_Data(52,0x180 | (0x3F - _headphone_speaker_balance));
			WM_Write_Data(54,0x180 | 0x3F);
		}
		else	
		{
			WM_Write_Data(11,0x100 | 0xFF);
			WM_Write_Data(52,0x180 | (0x3F - _headphone_speaker_balance));
			WM_Write_Data(54,0x180 | 0x3F);
		}
	}

	/* Setting right volume */		
	if(ucRightVol)
	{
		if(volume_right <= 0)
		{
			int reg,i,data;
			WM_Write_Data(12,0x100 | 0xFF + volume_right*2 - 34/* - 20*/);
			
			reg = _wm8978_reg[53];
			data = 0x180 | (0x3F - _headphone_speaker_balance);
			if( reg > data )
			{
				for(i=reg;i >= data;i--) WM_Write_Data(53, i);
			}
			else
			{
				for(i=reg;i <= data;i++) WM_Write_Data(53, i);
			}
			
			//WM_Write_Data(53,0x180 | (0x3F - _headphone_speaker_balance));
			WM_Write_Data(55,0x180 | 0x3F);
		}
		else	
		{
			WM_Write_Data(12,0x100 | 0xFF);
			WM_Write_Data(53,0x180 | (0x3F - _headphone_speaker_balance));
			WM_Write_Data(55,0x180 | 0x3F);
		}
	}

	if(ucLeftVol  == 0)
	{
		
		WM_Write_Data(11, 0x100); /* Left digital mute */
		//WM_Write_Data(52,0x180 | 0x00);
		//WM_Write_Data(54,0x180 | 0x00);
	}
	if(ucRightVol == 0)
	{
		WM_Write_Data(12, 0x100); /* Right digital mute */
		//WM_Write_Data(52,0x180 | 0x00);
		//WM_Write_Data(55,0x180 | 0x00);
	}
	
	
#else
	if (ucLeftVol!=0)
		data = (1<<8) | (ucLeftVol*3 + 162);
	else
		data = (1<<8);
	WM_Write_Data(11,data);
	
	if (ucRightVol!=0)
		data = (1<<8) | (ucRightVol*3 + 162);
	else
		data = (1<<8);
	WM_Write_Data(12,data);
	
	if (ucLeftVol!=0)	//0~63
		data = (1<<8) | (1<<7) | (ucLeftVol + 32);
	else
		data = (1<<8)| (1<<7);
	WM_Write_Data(52,data);
	
	if (ucRightVol!=0)
		data = (1<<8)| (1<<7) | (ucRightVol + 32);
	else
		data = (1<<8)| (1<<7);
	WM_Write_Data(53,data);
	
	if (ucLeftVol!=0)
		data = (1<<8)| (1<<7) | (ucLeftVol + 32);
	else
		data = (1<<8)| (1<<7);
	WM_Write_Data(54,data);
		
	if (ucRightVol!=0)
		data = (1<<8)| (1<<7) | (ucRightVol + 32);
	else
		data = (1<<8| (1<<7));
	WM_Write_Data(55,data);
#endif
	
	return 0;
}	

static INT WM_Set_ADC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol){
	UINT16 data;
	if (ucLeftVol!=0)
		data = (1<<8) | (ucLeftVol*3 + 162);
	else
		data = (1<<8);
	WM_Write_Data(15,data);
	
	if (ucRightVol!=0)
		data = (1<<8) | (ucRightVol*3 + 162);
	else
		data = (1<<8);
	WM_Write_Data(16,data);
	
	return 0;
}

	
/* for verification board */
#ifdef _3WIRE
static void ThreeWire_Write_Data(UINT16 data)
{
	int i;

	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~(WM_SCLK | WM_SDIN | WM_CSB) );
	
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | WM_CSB);//cs high
	Delay(1);
	for (i=15;i>=0;i--){
		if  ((1<<i)&data){//data is 1
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~WM_SCLK | WM_SDIN);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | WM_SCLK | WM_SDIN);//sclk high
			Delay(0x1);
			
		}	
		else{
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~WM_SDIN & ~WM_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~WM_SDIN | WM_SCLK);//sclk high
			Delay(0x1);
		}
	}
	
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~WM_CSB);//cs low
	Delay(1);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | WM_CSB);//cs high, for latch
}
#endif

#ifdef _2WIRE

static void reset_gpio(int WM_SCLK, int WM_SDIN)
{
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | (WM_SCLK | WM_SDIN));
	outpw(REG_GPIO_PE,inpw(REG_GPIO_PE) | (WM_SCLK | WM_SDIN) );
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | (WM_SCLK | WM_SDIN));
}

/* for demo board */
/* for demo board */
int TwoWire_Write_Data(UINT16 regAddr, UINT16 data)
{
	INT i,retrycount=RETRYCOUNT;
	UINT8 bDevID;
	UINT16 cmdData;
	INT32 WM_SCLK,WM_SDIN;
	INT status = 0;

	/* To avoid I2C conflict*/
	I2CRDWRBegin();

	_wm8978_reg[regAddr] = data;
	

	WM_SCLK = 1<<_eI2SType.uSCLK;
	WM_SDIN = 1<<_eI2SType.uSDIN;
	
	cmdData = (regAddr<<9) | data;
	

	if( WMDEVID == 0)
	{
		bDevID = 0x34;
	}
	if( WMDEVID == 1)
	{
		bDevID =0x36;
	}
	
_WMI2CRETRY:	
	if (_eI2SType.bIsGPIO)
	{
		outpw(REG_GPIO_PE,inpw(REG_GPIO_PE) & ~(WM_SCLK | WM_SDIN) );
		outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~(WM_SCLK | WM_SDIN));
		Delay(2);
		/* START */
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | WM_SDIN | WM_SCLK );//SCLK high SDIN high
		Delay(1);
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE)  & ~WM_SDIN | WM_SCLK );//SDIN low
  	
		Delay(2);
		/* Send Device ID depends on CSB status*/
		for (i=7;i>=0;i--)
		{
			if (bDevID&(1<<i))
			{
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~WM_SCLK );//SCLK low
				Delay(1);
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~WM_SCLK | WM_SDIN);//SCLK low;data ready
				Delay(2);
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | WM_SCLK | WM_SDIN);//SCLK high;data hold
				Delay(3);
			}
			else
			{
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~WM_SCLK );//SCLK low
				Delay(1);
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~WM_SCLK & ~WM_SDIN);//SCLK low
				Delay(2);
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~WM_SDIN | WM_SCLK );//SCLK high
				Delay(3);
				
			}	
		}
#if 1		
		/* clock pulse for receive ACK */
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~WM_SCLK);//SCLK low
		Delay(3);
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | WM_SCLK);//SCLK high
		Delay(3);
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | WM_SDIN);
		Delay(0x10);
		if ((inpw(REG_GPIO_STS) & WM_SDIN)!=0){
		    if (retrycount-- > 0)
		    	goto _WMI2CRETRY;
		    else{
		    	_debug_msg("Reg %d write 0x%x ACK1 receive fail\n",regAddr,data);
		    	reset_gpio(WM_SCLK,WM_SDIN);
		    	status = ERR_2WIRE_NO_ACK;
				goto i2c_lexit;
		    }
	    }
#endif
		
			
		/* Send control byte 1 */
		for (i=15;i>=8;i--)
		{
			if (cmdData&(1<<i))
			{
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~WM_SCLK );//SCLK low
				Delay(1);
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~WM_SCLK | WM_SDIN);//SCLK low;data ready
				Delay(2);
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | WM_SCLK | WM_SDIN);//SCLK high
				Delay(3);
				
			}
			else
			{
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~WM_SCLK );//SCLK low
				Delay(1);
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~WM_SCLK & ~WM_SDIN);//SCLK low
				Delay(2);
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~WM_SDIN | WM_SCLK );//SCLK high
				Delay(3);
				
			}	
		}
#if 1		
		/* clock pulse for receive ACK */
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~WM_SCLK);//SCLK low
		Delay(3);
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | WM_SCLK);//SCLK high
		Delay(3);
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | WM_SDIN);
		Delay(0x10);
		if ((inpw(REG_GPIO_STS) & WM_SDIN)!=0){
		    if (retrycount-- > 0)
		    	goto _WMI2CRETRY;
		    else{
		    	_debug_msg("Reg %d write 0x%x ACK2 receive fail\n",regAddr,data);
		    	reset_gpio(WM_SCLK,WM_SDIN);
		    	status = ERR_2WIRE_NO_ACK;
		    	goto i2c_lexit;
		    }
	    }
#endif		
 	
  	
  	
		
		/* Send control byte 2 */
		for (i=7;i>=0;i--)
		{
			if (cmdData&(1<<i))
			{
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~WM_SCLK );//SCLK low
				Delay(1);
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~WM_SCLK | WM_SDIN);//SCLK low;data ready
				Delay(2);
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | WM_SCLK | WM_SDIN);//SCLK high
				Delay(3);
			}
			else
			{
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~WM_SCLK );//SCLK low
				Delay(1);
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~WM_SCLK & ~WM_SDIN);//SCLK low
				Delay(2);
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~WM_SDIN | WM_SCLK );//SCLK high
				Delay(3);
					
			}	
		}
		
#if 1		
		/* clock pulse for receive ACK */
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~WM_SCLK);//SCLK low
		Delay(3);
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | WM_SCLK);//SCLK high
		Delay(3);
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | WM_SDIN);
		Delay(0x10);
		if ((inpw(REG_GPIO_STS) & WM_SDIN)!=0){
		    if (retrycount-- > 0)
		    	goto _WMI2CRETRY;
		    else{
		    	_debug_msg("Reg %d write 0x%x ACK3 receive fail\n",regAddr,data);
		    	reset_gpio(WM_SCLK,WM_SDIN);
		    	status = ERR_2WIRE_NO_ACK;
		    	goto i2c_lexit;
		    }
	    }
#endif	
 	
		
		/* stop */
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~WM_SCLK);//SCLK low
		Delay(3);
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | WM_SCLK);//SCLK high
		Delay(2);
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | WM_SDIN);//SDIN high
		Delay(2);
		
    	reset_gpio(WM_SCLK,WM_SDIN);
  	
	}
	else
	{
		outpw(REG_PADC0, inpw(REG_PADC0) | (7<<1));
	
		/* START */
		outpw(REG_SerialBusCR,READSTATUS() | WM_SDIN);//SDIN high
		outpw(REG_SerialBusCR,READSTATUS() | WM_SCLK);//SCLK high
		Delay(1);
		outpw(REG_SerialBusCR,READSTATUS() & ~WM_SDIN);//SDIN low
		Delay(2);
		/* Send Device ID depends on CSB status*/
		for (i=7;i>=0;i--)
		{
			if (bDevID&(1<<i))
			{
				outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK );//SCLK low
				Delay(1);
				outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK | WM_SDIN);//SCLK low;data ready
				Delay(1);
				outpw(REG_SerialBusCR,READSTATUS() | WM_SCLK | WM_SDIN);//SCLK high
				Delay(3);
			}
			else
			{
				outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK );//SCLK low
				Delay(1);
				outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK & ~WM_SDIN);//SCLK low
				Delay(1);
				outpw(REG_SerialBusCR,READSTATUS() & ~WM_SDIN | WM_SCLK );//SCLK high
				Delay(3);
				
			}	
		}
		/* clock pulse for receive ACK */
		outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK);//SCLK low
		Delay(2);
		outpw(REG_SerialBusCR,READSTATUS() | WM_SCLK);//SCLK high
		Delay(2);
		if (READSTATUS() & WM_SDIN){
		    if (retrycount-- > 0)
		    	goto _WMI2CRETRY;
		    else{
		    	_debug_msg("Reg %d write 0x%x ACK1 receive fail\n",regAddr,data);
		    	status = ERR_2WIRE_NO_ACK;
		    	goto i2c_lexit;
		    }
	    }
		
			
		/* Send control byte 1 */
		for (i=15;i>=8;i--)
		{
			if (cmdData&(1<<i))
			{
				outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK );//SCLK low
				Delay(1);
				outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK | WM_SDIN);//SCLK low;data ready
				Delay(1);
				outpw(REG_SerialBusCR,READSTATUS() | WM_SCLK | WM_SDIN);//SCLK high
				Delay(3);
				
			}
			else
			{
				outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK );//SCLK low
				Delay(1);
				outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK & ~WM_SDIN);//SCLK low
				Delay(1);
				outpw(REG_SerialBusCR,READSTATUS() & ~WM_SDIN | WM_SCLK );//SCLK high
				Delay(3);
				
			}	
		}
		/* clock pulse for receive ACK */
		outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK);//SCLK low
		Delay(2);
		outpw(REG_SerialBusCR,READSTATUS() | WM_SCLK);//SCLK high
		Delay(2);
		if (READSTATUS() & WM_SDIN){
		    if (retrycount-- > 0)
		    	goto _WMI2CRETRY;
		    else{
		    	_debug_msg("Reg %d write 0x%x ACK2 receive fail\n",regAddr,data);
		    	status = ERR_2WIRE_NO_ACK;
		    	goto i2c_lexit;
		    }
	    }
  	
		
		/* Send control byte 2 */
		for (i=7;i>=0;i--)
		{
			if (cmdData&(1<<i))
			{
				outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK );//SCLK low
				Delay(1);
				outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK | WM_SDIN);//SCLK low;data ready
				Delay(1);
				outpw(REG_SerialBusCR,READSTATUS() | WM_SCLK | WM_SDIN);//SCLK high
				Delay(3);
			}
			else
			{
				outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK );//SCLK low
				Delay(1);
				outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK & ~WM_SDIN);//SCLK low
				Delay(1);
				outpw(REG_SerialBusCR,READSTATUS() & ~WM_SDIN | WM_SCLK );//SCLK high
				Delay(3);
					
			}	
		}
		/* clock pulse for receive ACK */
		outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK);//SCLK low
		Delay(2);
		outpw(REG_SerialBusCR,READSTATUS() | WM_SCLK);//SCLK high
		Delay(2);
		if (READSTATUS() & WM_SDIN){
		    if (retrycount-- > 0)
		    	goto _WMI2CRETRY;
		    else{
		    	_debug_msg("Reg %d write 0x%x ACK3 receive fail\n",regAddr,data);
		    	status = ERR_2WIRE_NO_ACK;
		    	goto i2c_lexit;
		    }
	    }
		
		
		/* stop */
		outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK);//SCLK low
		Delay(2);
		outpw(REG_SerialBusCR,READSTATUS() | WM_SCLK);//SCLK high
		Delay(2);
		outpw(REG_SerialBusCR,READSTATUS() | WM_SDIN);//SDIN high
		Delay(2);
		
  	
		//outpw(REG_PADC0, inpw(REG_PADC0) & ~(7<<1));
		
	}	

i2c_lexit:	

	/* To avoid I2C conflict*/
	I2CRDWREnd();
	
	return status;

}
#endif /* #ifdef _2WIRE */ 
#ifdef _3WIRE	/* for verification board */
UINT8 ThreeWire_Read_Status(UINT8 ucID)
{
	UINT8 data=0;
	int i;
	WM_Write_Data(R28 | 0x02);//set GP1= WM_SDOUT
	WM_Write_Data(R24 | 0x01 | (ucID<<1));//read enable; read register select 

	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~WM_CSB);//cs low
	for (i=15;i>=0;i--){
		outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~WM_SCLK);//sclk low
		Delay(0x1);
		if (i<=7){
			if (inpw(REG_GPIO_DAT) & WM_DO)
				data = data | 1<<i;
		}
		outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | WM_SCLK);//sclk high
	}
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | WM_CSB);//cs high
	return data;
	
	
}
#endif

int WM8978_reset(void)
{
	WM_Write_Data(0 , 0);
	Delay(0x1000);
	return 0;
}

int WM8978_R1_Init(int R)
{
	INT nRetValue = 0;

	R &= 0x3;

	_nR1val = _nR1val | R;
	nRetValue = WM_Write_Data(1,_nR1val);//R1 BIASEN BUFIOEN VMIDSEL=	;
	//Delay(0x100);

	//nRetValue = WM_Write_Data(52,0x140);
	//nRetValue = WM_Write_Data(53,0x140);
	
	//nRetValue = WM_Write_Data(10,0x044);
	
	//nRetValue = WM_Write_Data(1,0xFF);
	//Delay(0x100);
		
	//for(i=0;i<1;i++)
	//{
	//_nR1val = _nR1val & ~0x008;
	//nRetValue = WM_Write_Data(1,_nR1val);//R1 BIASEN BUFIOEN VMIDSEL=	;
	//Delay(0x10);
	_nR1val = _nR1val | 0x00C;
	nRetValue = WM_Write_Data(1,_nR1val);//R1 BIASEN BUFIOEN VMIDSEL=	;
	//Delay(0x10);
	
	//}
	//Delay(0x100);
	
	return _nR1val;
}

INT WM8978_DAC_Setup()
{
	INT nRetValue = 0;
	
	//WM_Write_Data(0x0,0x00);//software reset
	//WM_Set_DAC_Volume(0,0);//r11 r12 set volume
	//WM_Write_Data(49,0x003);//R49 VROI
	//Delay(0x100);
	
	_nR3val = _nR3val | 0x00C;//R3 RMIXEN LMIXEN DACENR DACENL
	//WM_Write_Data(3,_nR3val);
	//Delay(0x10000);
	
	_nR3val = _nR3val | 0x003;//R3 RMIXEN LMIXEN DACENR DACENL
	WM_Write_Data(3,_nR3val);
	//Delay(0x10000);	


#if 0
	_nR1val = _nR1val | 0x004;
	nRetValue = WM_Write_Data(1,_nR1val);//R1 BIASEN BUFIOEN VMIDSEL=	;
	Delay(0x100);
	_nR1val = _nR1val | 0x008;
	nRetValue = WM_Write_Data(1,_nR1val);//R1 BIASEN BUFIOEN VMIDSEL=	;
	Delay(0x100);
#endif	

	/*
	_nR1val = _nR1val | 0x005;
	nRetValue = WM_Write_Data(1,_nR1val);//R1 BIASEN BUFIOEN VMIDSEL=	;
	Delay(0x10000);
	_nR1val = _nR1val | 0x008;
	nRetValue = WM_Write_Data(1,_nR1val);//R1 BUFIOEN VMIDSEL=	;
	Delay(0x30000);*/
	//WM_Write_Data(49,0x003);//R49 VROI
	//Delay(0x20000);
	//WM_Write_Data(2,0x180);//R2 power up ROUT1 LOUT1
	//WM_Write_Data(3,0x06F);//R3 LOUT2EN ROUT2EN RMIXEN LMIXEN DACENR DACENL
	
	wm8978SelectOutputPath();
	
	WM_Write_Data(4,0x010);//R4 select audio format(I2S format) and word length (16bits)
	WM_Write_Data(5,0x000);//R5 companding ctrl and loop back mode (all disable)
	WM_Write_Data(6,0x000);//R6 clock Gen ctrl(slave mode)
	//WM_Write_Data(13,1|1<<5);//L:HP H:SP
	//WM_Write_Data(13,1<<1|1<<4);//H:HP L:SP
#if 0
	WM_Write_Data(9,1<<6);//Jack Detection Enable, /* Note: Can't work on PMP board. If enable , sound will very small */
	if(_headphone_detection_leve == 1)
	{
		WM_Write_Data(13,1<<1|1<<4);//H:HP L:SP
	}
	else if ( _headphone_detection_leve == 0 ) 
	{
		WM_Write_Data(13,1|1<<5);//L:HP H:SP
	}
	else
	{
		WM_Write_Data(9,0);//jack detection off
	}
#endif	
	
	WM_Write_Data(10,0x008);//R10 DAC control (softmute disable, oversample select 64x (lowest power) )
	//WM_Write_Data(24,0x032);//R24 DAC limiter 1
	//WM_Write_Data(25,0x07f);//R25 DAC limiter 2
	WM_Write_Data(43,0x010);//For differential speaker
	//WM_Write_Data(50,_nR50val);//R50 DACL2LMIX
	//WM_Write_Data(51,_nR51val);//R51 DACR2RMIX
	WM_Write_Data(50,0x1);//R50 DACL2LMIX
	WM_Write_Data(51,0x1);//R51 DACR2RMIX
	WM_Write_Data(49,0x002);
	//WM_Set_DAC_Volume((_tIIS.sPlayVolume & 0xff00)>>8 , _tIIS.sPlayVolume & 0x00ff);
	
	WM_Set_Sample_Frequency(_tIIS.nPlaySamplingRate);//R7 set sampling rate

	return nRetValue;
}


INT WM8978_Init(void)
{
	INT nRetValue = 0;
	
	//WM_Write_Data(0x0,0x00);//software reset
	//WM_Set_DAC_Volume(0,0);//r11 r12 set volume
	//WM_Write_Data(49,0x003);//R49 VROI
	//Delay(0x100);
	
	_nR3val = _nR3val | 0x00C;//R3 RMIXEN LMIXEN DACENR DACENL
	
	_nR3val = _nR3val | 0x003;//R3 RMIXEN LMIXEN DACENR DACENL

	if (_bOutputPathEnable[OUT_PATH_SP])
		_nR3val = _nR3val | 0x060;//R3 LOUT2EN ROUT2EN
	else
		_nR3val = _nR3val &~0x060;
		
	WM_Write_Data(3,_nR3val);

	
	if (_bOutputPathEnable[OUT_PATH_HP])
		_nR2val = _nR2val |  0x180;//R2 power up ROUT1 LOUT1
	else
		_nR2val = _nR2val & ~0x180;
	
	_nR2val = _nR2val | 0x030;//BOOST Enable !!( It would effect the record volume )
	nRetValue = WM_Write_Data(2,_nR2val);

#if 0
	if(_headphone_detection_leve == 1)
	{
		WM_Write_Data(13,1<<1|1<<4);//H:HP L:SP
	}
	else if ( _headphone_detection_leve == 0 ) 
	{
		WM_Write_Data(13,1|1<<5);//L:HP H:SP
	}
	else
	{
		WM_Write_Data(9,0);//jack detection off
	}
#endif	

	return nRetValue;
}



INT WM8978_ADC_Setup()
{
	INT nRetValue = 0;

	//_nR1val = _nR1val | 0x008; //R1 BIASEN BUFIOEN VMIDSEL=	;
	
	_nR2val = _nR2val | 0x003;//R2 power up ADCR ADCL
	nRetValue = WM_Write_Data(2,_nR2val);
	WM_Write_Data(4,0x010);//R4 select audio format(I2S format) and word length (16bits)
	WM_Write_Data(5,0x000);//R5 companding ctrl and loop back mode (all disable)
	WM_Write_Data(6,0x000);//R6 clock Gen ctrl(slave mode)
	WM_Set_ADC_Volume((_tIIS.sRecVolume & 0xff00) >> 8, _tIIS.sRecVolume & 0x00ff);
	WM_Write_Data(35,0x008);//R35 enable noise gate
	//WM_Write_Data(44,0x033);//R44 RIN2INPPGA RIP2INPPGA LIN2INPPGA LIP2INPPGA
	wm8978SelectInputPath();
	//WM_Write_Data(45,0x11a);//R45 Left INP PGA gain ctrl
	//WM_Write_Data(46,0x11a);//R46 Right INP PGA gain ctrl
	//WM_Write_Data(47,0x007);//R47 PGABoostLEable 
	//WM_Write_Data(48,0x007);//R48 PGABoostREable 
	//WM_Write_Data(49,0x002);

	//WM_Write_Data(18,0x000);
	//WM_Write_Data(19,0x003);
	//WM_Write_Data(20,0x004);
	//WM_Write_Data(21,0x003);
	//WM_Write_Data(22,0x000);
	
	//WM_Write_Data(14,0x008);
	
	return nRetValue;
}
#if 0
INT WM8978_ALL_Setup()
{
	INT nRetValue = 0;

	//WM_Write_Data(0x0,0x00);//software reset
	WM_Set_DAC_Volume(0,0);//set volume
	Delay(0x10000);
	nRetValue = WM_Write_Data(1,0x01D);//R1 MICBEN BIASEN BUFIOEN VMIDSEL=	;
	Delay(0x30000);
	WM_Write_Data(49,0x003);//R49 VROI
	Delay(0x20000);
	//WM_Write_Data(2,0x180);//R2 power up ROUT1 LOUT1
	//Delay(10000);
	WM_Write_Data(2,0x1BF);//R2 power up ROUT1 LOUT1 BOOSTENR BOOSTENL INPGAR INPGAL ADCR ADCL
	WM_Write_Data(3,0x06F);//R3 LOUT2EN ROUT2EN RMIXEN LMIXEN DACENR DACENL
	WM_Write_Data(4,0x010);//R4 select audio format(I2S format) and word length (16bits)
	WM_Write_Data(5,0x000);//R5 companding ctrl and loop back mode (all disable)
	WM_Write_Data(6,0x000);//R6 clock Gen ctrl(slave mode)
	//WM_Write_Data(9,1<<6);//Jack Detection Enable, /* Note: Can't work on PMP board. If enable , sound will very small */
	WM_Write_Data(10,0x000);//R10 DAC control (softmute disable, oversample select 64x (lowest power) )
	WM_Write_Data(13,1|1<<5);//L:HP H:SP for Jack Detection
	WM_Write_Data(14,0x100);//R14 ADC control
	WM_Set_ADC_Volume((_tIIS.sRecVolume & 0xff00) >> 8, _tIIS.sRecVolume & 0x00ff);
	WM_Write_Data(24,0x032);//R24 DAC limiter 1
	WM_Write_Data(25,0x07f);//R25 DAC limiter 2
	WM_Write_Data(32 , 0x038);//ALC disable -12dB~+35.25dB
	WM_Write_Data(33 , 0x00B);//
	WM_Write_Data(34 , 0x032);
	WM_Write_Data(35,0x000);//R35 disable noise gate 
	WM_Write_Data(43,0x030);//For differential speaker
	//WM_Write_Data(44,0x077);//R44 R2_2INPPGA RIN2INPPGA RIP2INPPGA L2_2INPPGA LIN2INPPGA LIP2INPPGA
	SetInputPath();
	//WM_Write_Data(45,0x11a);//R45 Left INP PGA gain ctrl
	//WM_Write_Data(46,0x11a);//R46 Right INP PGA gain ctrl
	//WM_Write_Data(47,0x177);//R47 PGABoostLEable L2_2_BoostVol AUXL_2_BoostVol
	//WM_Write_Data(48,0x177);//R48 PGABoostREable R2_2_BoostVol AUXR_2_BoostVol
	WM_Write_Data(50,0x1);//R50 DACL2LMIX
	WM_Write_Data(51,0x1);//R51 DACR2RMIX
	WM_Write_Data(49 ,  0x002);
	WM_Set_DAC_Volume((_tIIS.sPlayVolume & 0xff00)>>8 , _tIIS.sPlayVolume & 0x00ff);
	WM_Set_Sample_Frequency(_tIIS.nPlaySamplingRate);//R7 set sampling rate


	return nRetValue;
}
#endif

INT audioDisableDAC()
{
	_nR3val = _nR3val & (~0x3UL);
	WM_Write_Data(3,_nR3val);
	return _nR3val;
}

INT audioEnableDAC()
{
	_nR3val = _nR3val | 0x3UL;
	WM_Write_Data(3,_nR3val);
	return _nR3val;
}

INT audioDisableADC()
{
	_nR2val = _nR2val & (~0x3UL);
	WM_Write_Data(2,_nR2val);
	return _nR2val;
}

INT audioEnableADC()
{
	_nR2val = _nR2val | 0x3UL;
	WM_Write_Data(2,_nR2val);
	return _nR2val;
}


INT WM8978_Bypass_Enable()
{
	INT nRetValue = 0;
	

	WM_Write_Data(52,0x181);
	WM_Write_Data(53,0x181);
	
	///WM_Set_DAC_Volume(0,0);//r11 r12 set volume
	///Delay(0x100);

	///_nR1val = _nR1val | 0x008 |  0x1;
	///nRetValue = WM_Write_Data(1,_nR1val);//R1 BIASEN BUFIOEN VMIDSEL=	;
	
	///Delay(0x300);
	///WM_Write_Data(49,0x002);//R49 VROI
	///Delay(0x200);
	//WM_Write_Data(2,0x1B3);//R2 power up ROUT1 LOUT1 0x1bc
	//WM_Write_Data(3,0x06C);//R3 LOUT2EN ROUT2EN RMIXEN LMIXEN DACENR DACENL
	///_nR2val = _nR2val | 0x030;
	///_nR3val = _nR3val | 0x00C;//R3 RMIXEN LMIXEN
	wm8978SelectOutputPath();
	//WM_Write_Data(5,0x000);//R5 companding ctrl and loop back mode (all disable)
	//WM_Write_Data(13,1|1<<5);//L:HP H:SP
	//WM_Write_Data(13,1<<1|1<<4);//H:HP L:SP
#if 0
	WM_Write_Data(9,1<<6);//Jack Detection Enable, /* Note: Can't work on PMP board. If enable , sound will very small */

	if(_headphone_detection_leve == 1)
	{
		WM_Write_Data(13,1<<1|1<<4);//H:HP L:SP
	}
	else if(_headphone_detection_leve == 0)
	{
		WM_Write_Data(13,1|1<<5);//L:HP H:SP
	}
	else
	{
		WM_Write_Data(9,0);
	}
#endif	
	///WM_Write_Data(43,0x030);//For differential speaker
	
	//WM_Write_Data(44,0x033);//R44 RIN2INPPGA RIP2INPPGA LIN2INPPGA LIP2INPPGA
	//WM_Write_Data(45,0x11a);//R45 Left INP PGA gain ctrl
	//WM_Write_Data(46,0x11a);//R46 Right INP PGA gain ctrl
	
	//WM_Write_Data(45,1<<6);
	//WM_Write_Data(46,1<<6);
	//WM_Write_Data(47,0x007);//R47 L2_2_BoostVol
	//WM_Write_Data(48,0x007);//R48 R2_2_BoostVol
	wm8978SelectInputPath();
	//_nR50val = _nR50val | 0x1E;
	//_nR51val = _nR51val | 0x1E;
	
	WM_Write_Data(50,0x116/*0x13*/);//R50 BYPL2LMIX
	WM_Write_Data(51,0x116/*0x13*/);//R51 BYPR2RMIX
	///WM_Write_Data(49,0x002);
	///WM_Set_DAC_Volume((_tIIS.sPlayVolume & 0xff00)>>8 , _tIIS.sPlayVolume & 0x00ff);
	///WM_Write_Data(7,0x001);
	
	WM_Write_Data(52,0x180 | 0x39);
	WM_Write_Data(53,0x180 | 0x39);
	
	//audioDisableDAC();
	//audioEnableADC();
	
	
	return nRetValue;

}
INT WM8978_Bypass_Disable()
{
	INT nRetValue = 0;

	//WM_Set_DAC_Volume(0,0);
	//Delay(0x100);
	//WM_Write_Data(52,0x181);
	//WM_Write_Data(53,0x181);
	nRetValue = WM_Write_Data(50,0x01);//R50 BYPL2LMIX
	nRetValue = WM_Write_Data(51,0x01);//R51 BYPR2RMIX
	//WM_Write_Data(52,0x180 | 0x39);
	//WM_Write_Data(53,0x180 | 0x39);

	//audioEnableDAC();
	//audioDisableADC();
	
	return nRetValue;
}
INT wm8978SelectOutputPath()
{
	INT nRetValue=0;
	
	//audioSetHeadphoneVolume(-56, -56);
	//WM_Write_Data(52,0x140);
	//WM_Write_Data(53,0x140);

	if (_bOutputPathEnable[OUT_PATH_SP])
		_nR3val = _nR3val | 0x060;//R3 LOUT2EN ROUT2EN
	else
		_nR3val = _nR3val &~0x060;
		
	WM_Write_Data(3,_nR3val);

	
	if (_bOutputPathEnable[OUT_PATH_HP])
		_nR2val = _nR2val |  0x180;//R2 power up ROUT1 LOUT1
	else
		_nR2val = _nR2val & ~0x180;
	
	nRetValue = WM_Write_Data(2,_nR2val);
		
	return nRetValue;
}
INT wm8978SelectInputPath()
{
	INT nRetValue=0;
	_nR47val = _nR47val | 0x100;
	_nR48val = _nR48val | 0x100;
	
	if (_bInputPathEnable[IN_PATH_MICL])
	{
		if ((_nR1val & 0x010)==0)
		{
			_nR1val = _nR1val | 0x010;//Enable MICBIAS
			WM_Write_Data(1,_nR1val);
		}
		
		_nR2val = _nR2val | 0x004;//Enable INPPGAL
		_nR44val = _nR44val | 0x003;
		_nR45val = _nR45val | 0x13f;
		_nR47val = _nR47val | 0x100;
		WM_Write_Data(2,_nR2val);
		WM_Write_Data(44,_nR44val);
		WM_Write_Data(45,_nR45val);
		WM_Write_Data(47,_nR47val);
	}
	else
	{
		_nR2val = _nR2val &~0x004;
		_nR44val = _nR44val &~0x003;
		WM_Write_Data(2,_nR2val);
		WM_Write_Data(44,_nR44val);
	}
	
	if (_bInputPathEnable[IN_PATH_MICR])
	{
		if ((_nR1val & 0x010)==0)
		{
			_nR1val = _nR1val | 0x010;//Enable MICBIAS
			WM_Write_Data(1,_nR1val);
		}
		
		_nR2val = _nR2val | 0x008;//Enable INPPGAR
		_nR44val = _nR44val | 0x030;
		_nR46val = _nR46val | 0x13f;
		_nR48val = _nR48val | 0x100;
		WM_Write_Data(2,_nR2val);
		WM_Write_Data(44,_nR44val);
		WM_Write_Data(46,_nR46val);
		WM_Write_Data(48,_nR48val);
	}
	else
	{
		_nR2val = _nR2val & ~0x008;//disable INPPGAR
		_nR44val = _nR44val &~0x030;
		WM_Write_Data(2,_nR2val);
		WM_Write_Data(44,_nR44val);
	}
		
	if (_bInputPathEnable[IN_PATH_LINEIN1])
	{
		_nR47val = _nR47val | 0x040;
		_nR48val = _nR48val | 0x040;
		WM_Write_Data(47,_nR47val);
		WM_Write_Data(48,_nR48val);
	}
	else
	{
		_nR47val = _nR47val &~0x070;
		_nR48val = _nR48val &~0x070;
		WM_Write_Data(47,_nR47val);
		WM_Write_Data(48,_nR48val);
	}
	
	if (_bInputPathEnable[IN_PATH_LINEIN2])
	{
		_nR47val = _nR47val | 0x007;
		_nR48val = _nR48val | 0x007;
		WM_Write_Data(47,_nR47val);
		WM_Write_Data(48,_nR48val);
	}
	else
	{
		_nR47val = _nR47val &~0x007;
		_nR48val = _nR48val &~0x007;
		WM_Write_Data(47,_nR47val);
		WM_Write_Data(48,_nR48val);
	}
	
	return nRetValue;
}



static void WM_Set_Sample_Frequency(INT uSample_Rate)
{   
	UINT16 data=0;
	switch (uSample_Rate)
	{
		case AU_SAMPLE_RATE_8000:							//8KHz
			data = WM_8000 ;
			break;
		case AU_SAMPLE_RATE_11025:					//11.025KHz
			data = WM_11025 ;
    
			break;
		case AU_SAMPLE_RATE_16000:						//16KHz
			data = WM_16000 ;
    
			break;
		case AU_SAMPLE_RATE_22050:					//22.05KHz
			data = WM_22050;
    
			break;
		case AU_SAMPLE_RATE_24000:						//24KHz
			data = WM_24000;

			break;
		case AU_SAMPLE_RATE_32000:						//32KHz
			data = WM_32000 ;

			break;
		case AU_SAMPLE_RATE_44100:					//44.1KHz
			data = WM_44100 ;

			break;
		case AU_SAMPLE_RATE_48000:						//48KHz
			data = WM_48000;

			break;
		default:break;
	}
	WM_Write_Data(7,data|1);//bit 1 for JACK DETECT
}



/*---------- end of WM8978's functions group ----------*/

void audioHeadphoneDetectionLevel(int level)
{

	if(level == 1)
	{
		_headphone_detection_leve = 1;
		WM_Write_Data(9,1<<6); //Jack Detection Enable.
		WM_Write_Data(13,1<<1|1<<4);//H:HP L:SP
	}
	else if(level == 0)
	{
		_headphone_detection_leve = 0;
		WM_Write_Data(9,1<<6); //Jack Detection Enable.
		WM_Write_Data(13,1|1<<5);//L:HP H:SP
	}
	else
	{
		_headphone_detection_leve = 2;
		WM_Write_Data(9,0);//jack detection off
	}
	

}


// for ECOS only
#ifdef ECOS
INT wm8978WriteReg(UINT16 data)
{
	int addr, value;
	
	addr = (data >> 9);
	value = data & 0x1FF;
	
	return TwoWire_Write_Data(addr, value);
}
#endif
#endif	/* HAVE_IIS */

int audioGetSampleRate(void)
{
	return _tIIS.nPlaySamplingRate;
}

int audioGetChannels(void)
{
	return _audio_play_channels;
}

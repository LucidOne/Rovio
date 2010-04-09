/****************************************************************************
 * 
 * FILENAME
 *     W99702_UDA1345TS.c
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

#ifdef HAVE_UDA1345TS

static AUDIO_T	_tIIS;

#define	IIS_ACTIVE				0x1
#define	IIS_PLAY_ACTIVE			0x2
#define IIS_REC_ACTIVE			0x4

static BOOL	 _bIISActive = 0;
static UINT32 _uIISCR = 0;


VOID L3_Set_Data(UINT8);
VOID L3_Set_Status(UINT8);
UINT8 L3_Set_Sample_Frequency(INT);
UINT8 L3_Set_Data_Format(INT);
static VOID IIS_Set_Data_Format(INT);
static VOID IIS_Set_Sample_Frequency(INT);

extern BOOL	_bIsLeftCH;

static void Delay(int nCnt)
{
	volatile int  loop;
	for (loop=0; loop<nCnt*10; loop++);
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
		

	/* use GPIO 17 18 19 as L3 interface control pin */
	outpw(REG_GPIO_OE,inpw(REG_GPIO_OE) & ~(1 << L3MODE_GPIO_NUM | 1<< L3CLOCK_GPIO_NUM | 1<< L3DATA_GPIO_NUM));
	/* set volume, dsp power up and no de-emph.no mute of external codec, from L3 */
	

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
INT  uda1345tsStartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
								INT nChannels, INT data_format)
{
	INT		nStatus,L3status;
	
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

	L3_Set_Data(EX_DAC_On);

	/* set play sampling rate and data format */
	L3status = L3_Set_Sample_Frequency(nSamplingRate);
	L3status = L3status | L3_Set_Data_Format(data_format);
	L3_Set_Status(L3status);

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
INT  uda1345tsStopPlay()
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
INT  uda1345tsStartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
							INT nChannels, INT data_format)
{
	INT		nStatus,L3status;

	if (_bIISActive & IIS_REC_ACTIVE)
		return ERR_IIS_REC_ACTIVE;		/* IIS was recording */
	
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

	L3_Set_Data(EX_ADC_On);

	/* set record sampling rate and data format */
	L3status = L3_Set_Sample_Frequency(nSamplingRate);
	L3status = L3status | L3_Set_Data_Format(data_format);
	L3_Set_Status(L3status);

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
INT  uda1345tsStopRecord()
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

/*----------- for UDA1345TS functions group ----------*/

/* L3 Address mode */
void L3_Address_Mode(volatile UINT8 L3Data){
	int i;
	
	for (i=0;i<8;i++)
	{
		
		outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~(1<<L3MODE_GPIO_NUM) | (1<<L3CLOCK_GPIO_NUM) );
		Delay(10);
		if ((1 << i) & L3Data)
		{
			outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~(1<<L3MODE_GPIO_NUM) & ~(1<<L3CLOCK_GPIO_NUM) | (1<<L3DATA_GPIO_NUM));
		}	
		else 
		{	
			outpw(REG_GPIO_DAT,	inpw(REG_GPIO_DAT) & ~(1<<L3MODE_GPIO_NUM) & ~(1<<L3CLOCK_GPIO_NUM) & ~(1<<L3DATA_GPIO_NUM));
		}	
		Delay(10);
	}
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~(1<<L3MODE_GPIO_NUM) | (1 << L3CLOCK_GPIO_NUM));
}

/* L3 data transfer mode */
void L3_Data_Transfer_Mode(volatile UINT8 L3Data){
	int i;
	
	for (i=0;i<8;i++)
	{
		outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | (1 << L3MODE_GPIO_NUM) | (1 << L3CLOCK_GPIO_NUM) );
		Delay(10);
		if ((1 << i) & L3Data)
		{
			outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~(1<<L3CLOCK_GPIO_NUM) | (1<<L3MODE_GPIO_NUM) | (1<<L3DATA_GPIO_NUM));
		}	
		else 
		{
			outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~(1<<L3CLOCK_GPIO_NUM) & ~(1<<L3DATA_GPIO_NUM) | (1<<L3MODE_GPIO_NUM));
		}
		Delay(10);
	}
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | (1<<L3MODE_GPIO_NUM) | (1<<L3CLOCK_GPIO_NUM));
}

void L3_Set_Data(volatile UINT8 data){

	UINT8 volatile L3Data = 0;
	
	
	/*----- Address Mode -----*/
	L3Data = L3Data | EX_1345ADDR | EX_DATA; //set the DATA(volumn,de-emphasis, mute, and power control)
	L3_Address_Mode(L3Data);	
		
	/*----- Data Transfer Mode -----*/
	L3Data = 0x0;						//set the maximum volumn
	L3_Data_Transfer_Mode(L3Data);
	
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~(1<<L3MODE_GPIO_NUM) | (1<<L3CLOCK_GPIO_NUM));//transfer gap
	
	L3Data = data;						//power control turn on the DAC or ADC
	L3_Data_Transfer_Mode(L3Data);
	
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~(1<<L3MODE_GPIO_NUM) | (1<<L3CLOCK_GPIO_NUM));//transfer gap
	
	L3Data = 0x80;						
	L3_Data_Transfer_Mode(L3Data);
	
	
}

/*----- Set L3 nStatus -----*/
VOID L3_Set_Status(UINT8 nStatus){
	
	volatile UINT8 L3Data = 0;
	/*----- Address Mode -----*/
	L3Data = L3Data | EX_1345ADDR | EX_STATUS;	//set the STATUS(system clock frequency, data input format and DC filter)
	L3_Address_Mode(L3Data);	
	
	/*----- Data Transfer Mode -----*/
	L3_Data_Transfer_Mode(nStatus);
	
}



/*----- Set sampling frequency -----*/
UINT8 L3_Set_Sample_Frequency(int choose_sf){
	UINT8 nStatus=0;
	switch (choose_sf)
	{
		case AU_SAMPLE_RATE_8000:							//8KHz
			nStatus = EX_384FS;
			break;
		case AU_SAMPLE_RATE_11025:					//11.025KHz
			nStatus = EX_384FS;
			break;
		case AU_SAMPLE_RATE_16000:						//16KHz
			nStatus = EX_384FS;
			break;
		case AU_SAMPLE_RATE_22050:					//22.05KHz
			nStatus = EX_256FS;
			break;
		case AU_SAMPLE_RATE_24000:						//24KHz
			nStatus = EX_256FS;
			break;
		case AU_SAMPLE_RATE_32000:						//32KHz
			nStatus = EX_384FS;
			break;
		case AU_SAMPLE_RATE_44100:					//44.1KHz
			nStatus = EX_384FS;
			break;
		case AU_SAMPLE_RATE_48000:						//48KHz
			nStatus = EX_256FS;
			break;
		default:break;
	}
	return nStatus;
}

/*----- set data format -----*/
UINT8 L3_Set_Data_Format(INT choose_format){
	UINT8 nStatus=0;
	switch(choose_format){
		case IIS_FORMAT: nStatus = EX_IIS;
				break;
		case MSB_FORMAT: nStatus = EX_MSB;
				break;
		default:break;
	}
	return nStatus;
}

/*----------- End of UDA1345TS's functions group ----------*/

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
static void IIS_Set_Sample_Frequency(int choose_sf){

	switch (choose_sf)
	{
		case AU_SAMPLE_RATE_8000:							//8KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_4 | BCLK_48;//APLL=12.288Mhz
			//_uIISCR = _uIISCR | FS_256 | SCALE_8 | BCLK_48;//APLL=24.576Mhz
			break;
		case AU_SAMPLE_RATE_11025:					//11.025KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_4 | BCLK_48;//APLL=16.934Mhz
			//_uIISCR = _uIISCR | FS_256 | SCALE_8 | BCLK_32;//APLL=22.579
			break;
		case AU_SAMPLE_RATE_12000:					//12KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_4 | BCLK_32;
			//_uIISCR = _uIISCR | FS_256 | SCALE_8 | BCLK_32;
			break;	
		case AU_SAMPLE_RATE_16000:						//16KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_2 | BCLK_48;
			//_uIISCR = _uIISCR | FS_256 | SCALE_4 | BCLK_48;

			break;
		case AU_SAMPLE_RATE_22050:					//22.05KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_2 | BCLK_48;
			//_uIISCR = _uIISCR | FS_256 | SCALE_4 | BCLK_32;
			break;
		case AU_SAMPLE_RATE_24000:						//24KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_2 | BCLK_32;
			//_uIISCR = _uIISCR | FS_256 | SCALE_4 | BCLK_32;

			break;
		case AU_SAMPLE_RATE_32000:						//32KHz
			_uIISCR = _uIISCR | SCALE_1 | FS_256  | BCLK_48;
			//_uIISCR = _uIISCR | SCALE_2 | FS_256  | BCLK_48;
			break;
		case AU_SAMPLE_RATE_44100:					//44.1KHz
			_uIISCR = _uIISCR | SCALE_1 | FS_256  | BCLK_48;
			//_uIISCR = _uIISCR | FS_256 | SCALE_2 | BCLK_32;
			break;
		case AU_SAMPLE_RATE_48000:						//48KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_1 | BCLK_32;
			//_uIISCR = _uIISCR | FS_256 | SCALE_2 | BCLK_32;

			break;
		default:break;
	}
	outpw(REG_ACTL_IISCON,_uIISCR);
}
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      i2sSetPlayVolume                                                 */
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
INT  uda1345tsSetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol)  //0~31
{
	if (ucLeftVol>31)
		ucLeftVol=31;
	if (ucRightVol>31)
		ucRightVol=31;
	
		_tIIS.sPlayVolume = 0x3F - ucLeftVol*2;
		/*----- Address Mode -----*/
		L3_Address_Mode(EX_1345ADDR | EX_DATA);//set the DATA(volumn,de-emphasis, mute, and power control)
		/*----- Data Transfer Mode -----*/
		L3_Data_Transfer_Mode((UINT8)_tIIS.sPlayVolume);
	


	
	return 0;
}







#endif	/* HAVE_IIS */
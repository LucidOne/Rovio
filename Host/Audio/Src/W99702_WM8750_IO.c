/****************************************************************************
 * 
 * FILENAME
 *     W99702_WM8750.c
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
//#include "WM8750.h"
#define _2WIRE
#define READSTATUS() ((inpw(REG_SerialBusCR)&0x38)>>3)


#ifdef HAVE_WM8750



static AUDIO_T	_tIIS;

#define	IIS_ACTIVE				0x1
#define	IIS_PLAY_ACTIVE			0x2
#define IIS_REC_ACTIVE			0x4
#define RETRYCOUNT		10
#define WMDEVID 0

static BOOL	 _bIISActive = 0;
static BOOL	 _bIsSetPlayVol = TRUE;
static BOOL	 _bIsSetRecVol = TRUE;
static UINT32 _uIISCR = 0;


INT WM8750_DAC_Setup(void);
INT WM8750_ADC_Setup(void);

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

static void Delay(int nCnt)
{
	volatile int  loop;
	for (loop=0; loop<nCnt*20; loop++);
}

#ifdef ECOS
cyg_uint32  iis_play_isr(cyg_vector_t vector, cyg_addrword_t data)
#else
static void  iis_play_isr()
#endif
{
	outpw(REG_ACTL_CON, inpw(REG_ACTL_CON) | T_DMA_IRQ);
	if (_bIsSetPlayVol)
	{
		WM_Set_DAC_Volume((_tIIS.sPlayVolume & 0xff00)>>8 , _tIIS.sPlayVolume & 0x00ff);
		_bIsSetPlayVol = FALSE;
	}
	if (_tIIS.bPlayLastBlock)
	{
		outpw(REG_ACTL_PSR, P_DMA_MIDDLE_IRQ | P_DMA_END_IRQ);
		wm8750StopPlay();
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
	if(_bIsSetRecVol)
	{
		WM_Set_ADC_Volume((_tIIS.sRecVolume & 0xff00) >> 8, _tIIS.sRecVolume & 0x00ff);
		_bIsSetRecVol = FALSE;
	}
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



	if (_eI2SType.bIsGPIO){
		outpw(REG_GPIO_IE,inpw(REG_GPIO_IE) & ~(_eI2SType.uSDIN | _eI2SType.uSCLK));
		if (_eI2SType.uSDIN==4 || _eI2SType.uSCLK==4)
			sysDisableInterrupt(4);
		if (_eI2SType.uSDIN==5 || _eI2SType.uSCLK==5)
			sysDisableInterrupt(5);
			
	}
		
	WM_Write_Data(15,0x0);//software reset codec

	_uIISCR = 0;
	return 0;
}







/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      wm8750StartPlay                                                   */
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
INT  wm8750StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
								INT nChannels, INT data_format)
{
	INT		nStatus;
	//INT		nTime0;
	//UINT32	uHWAdd;
	
	if (_bIISActive & IIS_PLAY_ACTIVE)
		return ERR_IIS_PLAY_ACTIVE;		/* IIS was playing */
	
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


	IIS_Set_Sample_Frequency(nSamplingRate);
	IIS_Set_Data_Format(data_format);
	
	nStatus = WM8750_DAC_Setup();
	if (nStatus != 0)
		return nStatus;
	
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
	//outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | IIS_PLAY);
	_bIISActive |= IIS_PLAY_ACTIVE;
		
	
	
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      wm8750StopPlay                                                    */
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
INT  wm8750StopPlay()
{

	
	WM_Set_DAC_Volume(0,0);
	
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL));

	if (!(_bIISActive & IIS_PLAY_ACTIVE))
		return ERR_IIS_PLAY_INACTIVE;
	
	/* stop playing */
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~IIS_PLAY);

	_debug_msg("IIS stop playing\n");
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
/*      wm8750StartRecord                                                 */
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
INT  wm8750StartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
							INT nChannels, INT data_format)
{
	INT		nStatus;
	

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


	nStatus = WM8750_ADC_Setup();
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
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      wm8750StopRecord                                                  */
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
INT  wm8750StopRecord()
{

	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(RECORD_RIGHT_CHNNEL | RECORD_LEFT_CHNNEL));

	if (!(_bIISActive & IIS_REC_ACTIVE))
		return ERR_IIS_REC_INACTIVE;
	
	_debug_msg("IIS stop recording\n");

	/* stop recording */
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~IIS_RECORD);
	
	WM_Write_Data(0x0,0x00);//software reset
	
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
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      wm8750SetPlayVolume                                              */
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
INT  wm8750SetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol)  //0~31
{
	INT nRetValue = 0;
	if (ucLeftVol>31)
		ucLeftVol=31;
	if (ucRightVol>31)
		ucRightVol=31;
	_tIIS.sPlayVolume = (ucLeftVol << 8) | ucRightVol;
	_bIsSetPlayVol = TRUE;
	//nRetValue = WM_Set_DAC_Volume((_tIIS.sPlayVolume & 0xff00)>>8 , _tIIS.sPlayVolume & 0x00ff);
	if (nRetValue!=0){
		return nRetValue;
	}
	return 0;
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      wm8750SetRecordVolume                                                 */
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
INT  wm8750SetRecordVolume(UINT8 ucLeftVol, UINT8 ucRightVol)  //0~31
{
	if (ucLeftVol>31)
		ucLeftVol=31;
	if (ucRightVol>31)
		ucRightVol=31;
	_tIIS.sRecVolume = (ucLeftVol << 8) | ucRightVol;
	_bIsSetRecVol = TRUE;
	//WM_Set_ADC_Volume((_tIIS.sRecVolume & 0xff00) >> 8, _tIIS.sRecVolume & 0x00ff);

	return 0;
}



/*---------- for WM8750	functions group----------*/
//(MIN)  0 ~ (MAX) 255
static INT WM_Set_DAC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol){	//0~31
	
	UINT16 data;
	if (ucLeftVol!=0)
		data = (1<<8) | (ucLeftVol + 96);
	else
		data = (1<<8);
	WM_Write_Data(2,data);//LOUT1 vol
	
	if (ucRightVol!=0)
		data = (1<<8) | (ucRightVol + 96);
	else
		data = (1<<8);
	WM_Write_Data(3,data);//ROUT1 vol
	
	if (ucLeftVol!=0)	//0~63
		data = (1<<8) | (ucLeftVol*3 + 162);
	else
		data = (1<<8);
	WM_Write_Data(10,data);//LDAC vol
	
	if (ucRightVol!=0)
		data = (1<<8) | (ucRightVol*3 + 162);
	else
		data = (1<<8);
	WM_Write_Data(11,data);//RDAC vol
	
	
	
	return 0;
}	
static INT WM_Set_ADC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol){
	UINT16 data;
	
	if (ucLeftVol!=0)
		data = (1<<8) | (ucLeftVol*2 + 1);
	else
		data = (1<<8);
	WM_Write_Data(0,data);//LPGA vol
	
	if (ucRightVol!=0)
		data = (1<<8) | (ucRightVol*2 + 1);
	else
		data = (1<<8);
	WM_Write_Data(1,data);//RPGA vol
	
	if (ucLeftVol!=0)
		data = (1<<8) | (ucLeftVol*8 + 7);
	else
		data = (1<<8);
	WM_Write_Data(21,data);//LADC vol
	
	if (ucRightVol!=0)
		data = (1<<8) | (ucRightVol*8 + 7);
	else
		data = (1<<8);
	WM_Write_Data(22,data);//RADC vol
	

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
/* for demo board */
int TwoWire_Write_Data(UINT16 regAddr, UINT16 data)
{
	INT i,retrycount=RETRYCOUNT;
	UINT8 bDevID;
	UINT16 cmdData;
	INT32 WM_SCLK,WM_SDIN;


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
		outpw(REG_GPIO_PE,inpw(REG_GPIO_PE) |  (WM_SCLK | WM_SDIN) );
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
		    	return ERR_2WIRE_NO_ACK;
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
		    	return ERR_2WIRE_NO_ACK;
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
		    	return ERR_2WIRE_NO_ACK;
		    }
	    }
#endif	
 	
  	
		
		/* stop */
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | WM_SCLK);//SCLK high
		Delay(2);
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | WM_SDIN);//SDIN high
		Delay(2);
		
  	
		
		return 0;
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
		    	return ERR_2WIRE_NO_ACK;
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
		    	return ERR_2WIRE_NO_ACK;
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
		    	return ERR_2WIRE_NO_ACK;
		    }
	    }
		
		
		/* stop */
		outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK);//SCLK low
		Delay(2);
		outpw(REG_SerialBusCR,READSTATUS() | WM_SCLK);//SCLK high
		Delay(2);
		outpw(REG_SerialBusCR,READSTATUS() | WM_SDIN);//SDIN high
		Delay(2);
		
  	
		outpw(REG_PADC0, inpw(REG_PADC0) & ~(7<<1));
		
		return 0;
	}	

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
int WM8750_DAC_Setup(){
	INT nRetValue = 0;

	WM_Write_Data(15,0x0);//software reset codec
	nRetValue = WM_Write_Data(25,0xc0);//VMID:50Kom; VREF power up
	WM_Write_Data(26,0x180);//LRDAC power up
	WM_Write_Data(26,0x1e0);//LRDAC power up, LROUT1 power up
	WM_Write_Data(34,0x100 | 1<<7);//LD2LO LMIXSEL
	WM_Write_Data(37,0x100 | 1<<7);//RD2RO 
	WM_Set_DAC_Volume(20,20);//r11 r12 set volume

	WM_Write_Data(5,0x01);//disable DAC-mute
	WM_Write_Data(7,0x02);//word length 16bits, I2S format, slave mode, 
	WM_Write_Data(12,0x87);//Bass control
	WM_Write_Data(13,0x7);//Treble control
	
	//WM_Write_Data(32,0x30);//LINPUT1,29dB boost
	//WM_Write_Data(33,0x30);//RINPUT1,29dB boost
	
		
	return nRetValue;
}

int WM8750_ADC_Setup(){
	INT nRetValue = 0;
	
	WM_Write_Data(15,0x0);//software reset codec
	nRetValue = WM_Write_Data(25,0xfe);//VMID:5Kom; VREF power up
	//WM_Write_Data(26,0x1e0);//LRDAC power up, LROUT1 power up
	
	WM_Write_Data(32,0x30);//LINPUT1,29dB boost
	WM_Write_Data(33,0x30);//RINPUT1,29dB boost
	WM_Write_Data(34,0x100 | (1<<7));//LMIX
	//WM_Write_Data(36,0x00);//RMIX
	WM_Write_Data(37,0x103 | (1<<7));//RMIX
	WM_Write_Data(7,0x02);//word length 16bits, I2S format, slave mode, 
	WM_Write_Data(27,0x00);
	
	
	return nRetValue;
}



/*---------- end of WM8750's functions group ----------*/

#endif	/* HAVE_IIS */
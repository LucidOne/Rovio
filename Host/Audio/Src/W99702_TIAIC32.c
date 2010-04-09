/****************************************************************************
 * 
 * FILENAME
 *     W99702_TIAIC32.c
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
 *     2005.06.14		Created by Shih-Jen Lu
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
#include "TIAIC32.h"


#ifdef HAVE_TIAIC32



static AUDIO_T	_tIIS;

#define	IIS_ACTIVE				0x1
#define	IIS_PLAY_ACTIVE			0x2
#define IIS_REC_ACTIVE			0x4

static BOOL	 _bIISActive = 0;
static UINT32 _uIISCR = 0;


extern BOOL _bADDA_Active;

INT TIAIC32_DAC_Setup(void);
void TIAIC32_ADC_Setup(void);

static void I2C_Write_Data(UINT8 regAddr, UINT8 data);

static INT TI_Set_DAC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol);
static INT TI_Set_ADC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol);




static VOID IIS_Set_Data_Format(INT);
static VOID IIS_Set_Sample_Frequency(INT);

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
	
	if (_tIIS.bPlayLastBlock)
	{
		outpw(REG_ACTL_PSR, P_DMA_MIDDLE_IRQ | P_DMA_END_IRQ);
		tiaic32StopPlay();
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

	
	/* for verification board */
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~(TI_SCLK | TI_SDA) );
	
	/** end of for verification board **/

	
	//ThreeWire_Write_Data(0x0000);//R0 software reset;all registers reture to default value
	//Delay(1000);
	


	_uIISCR = 0;
	return 0;
}







/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      tiaic32StartPlay                                                   */
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
INT  tiaic32StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
								INT nChannels, INT data_format)
{
	INT		nStatus;

	
	if (_bIISActive & IIS_PLAY_ACTIVE)
		return ERR_IIS_PLAY_ACTIVE;		/* IIS was playing */
	
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
//	sysSetAIC2SWMode();					/* Set AIC into SW mode */
	sysEnableInterrupt(AU_PLAY_INT_NUM);
#endif

	_tIIS.fnPlayCallBack = fnCallBack;
	_tIIS.nPlaySamplingRate = nSamplingRate;


	IIS_Set_Sample_Frequency(nSamplingRate);
	IIS_Set_Data_Format(data_format);
	
	nStatus = TIAIC32_DAC_Setup();
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
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | IIS_PLAY);
	_bIISActive |= IIS_PLAY_ACTIVE;
	
	
	
		
	
	
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      tiaic32StopPlay                                                    */
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
INT  tiaic32StopPlay()
{
	//UINT32	uAddress;
	
	TI_Set_DAC_Volume(0,0);
	
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL));

	if (!(_bIISActive & IIS_PLAY_ACTIVE))
		return ERR_IIS_PLAY_INACTIVE;
	
	/* stop playing */
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~IIS_PLAY);
	_debug_msg("IIS stop playing\n");
	//ThreeWire_Write_Data(0x0000);//R0 software reset;all registers reture to default value
	
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
/*      tiaic32StartRecord                                                 */
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
INT  tiaic32StartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
							INT nChannels, INT data_format)
{
	INT		nStatus;

	if (_bIISActive & IIS_REC_ACTIVE){
		_error_msg("IIS is recording\n");
		return ERR_IIS_REC_ACTIVE;		/* IIS was recording */
	}
	
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | RECORD_RIGHT_CHNNEL);
	if (nChannels != 1)
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | RECORD_LEFT_CHNNEL);
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
//	sysSetAIC2SWMode();					/* Set AIC into SW mode */
	sysEnableInterrupt(AU_REC_INT_NUM);
#endif

	_tIIS.fnRecCallBack = fnCallBack;
	_tIIS.nRecSamplingRate = nSamplingRate;


	TIAIC32_ADC_Setup();

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
/*      tiaic32StopRecord                                                  */
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
INT  tiaic32StopRecord()
{
	UINT32	uAddress;
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(RECORD_RIGHT_CHNNEL | RECORD_LEFT_CHNNEL));

	if (!(_bIISActive & IIS_REC_ACTIVE))
		return ERR_IIS_REC_INACTIVE;
	
	_debug_msg("IIS stop recording\n");

	/* stop recording */
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~IIS_RECORD);
	
	/* clear the register */
	for (uAddress=REG_ACTL_CON;uAddress<=REG_ACTL_IISCON;uAddress=uAddress+4)
	{
		outpw(uAddress,0x0);
	}
	
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
static void IIS_Set_Sample_Frequency(int choose_sf){

	switch (choose_sf)
	{
		case AU_SAMPLE_RATE_8000:							//8KHz
			//_uIISCR = _uIISCR | FS_384 | SCALE_4 | BCLK_32;
			_uIISCR = _uIISCR | FS_384 | SCALE_8 | BCLK_32;
			break;
		case AU_SAMPLE_RATE_11025:					//11.025KHz
			//_uIISCR = _uIISCR | FS_384 | SCALE_4 | BCLK_32;
			_uIISCR = _uIISCR | FS_256 | SCALE_8 | BCLK_32;
			break;
		case AU_SAMPLE_RATE_16000:						//16KHz
			//_uIISCR = _uIISCR | FS_384 | SCALE_2 | BCLK_32;
			_uIISCR = _uIISCR | FS_384 | SCALE_4 | BCLK_32;

			break;
		case AU_SAMPLE_RATE_22050:					//22.05KHz
			//_uIISCR = _uIISCR | FS_256 | SCALE_3 | BCLK_32;
			_uIISCR = _uIISCR | FS_256 | SCALE_4 | BCLK_32;
			break;
		case AU_SAMPLE_RATE_24000:						//24KHz
			//_uIISCR = _uIISCR | FS_256 | SCALE_2 | BCLK_32;
			_uIISCR = _uIISCR | FS_256 | SCALE_4 | BCLK_32;

			break;
		case AU_SAMPLE_RATE_32000:						//32KHz
			//_uIISCR = _uIISCR | SCALE_1 | FS_384  | BCLK_32;
			_uIISCR = _uIISCR | SCALE_2 | FS_384  | BCLK_32;
			break;
		case AU_SAMPLE_RATE_44100:					//44.1KHz
			//_uIISCR = _uIISCR | SCALE_1 | FS_384  | BCLK_32;
			_uIISCR = _uIISCR | FS_256 | SCALE_2 | BCLK_32;
			break;
		case AU_SAMPLE_RATE_48000:						//48KHz
			//_uIISCR = _uIISCR | FS_256 | SCALE_1 | BCLK_32;
			_uIISCR = _uIISCR | FS_256 | SCALE_2 | BCLK_32;

			break;
		default:break;
	}
	outpw(REG_ACTL_IISCON,_uIISCR);
}
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      tiaic32SetPlayVolume                                              */
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
INT  tiaic32SetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol)  //0~31
{
	INT nRetValue = 0;
	if (ucLeftVol>31)
		ucLeftVol=31;
	if (ucRightVol>31)
		ucRightVol=31;
	_tIIS.sPlayVolume = (ucLeftVol << 8) | ucRightVol;
	nRetValue = TI_Set_DAC_Volume(ucLeftVol,ucRightVol);
	if (nRetValue!=0){
		return nRetValue;
	}
	return 0;
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      tiaic32SetRecordVolume                                                 */
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
INT  tiaic32SetRecordVolume(UINT8 ucLeftVol, UINT8 ucRightVol)  //0~31
{
	if (ucLeftVol>31)
		ucLeftVol=31;
	if (ucRightVol>31)
		ucRightVol=31;
	_tIIS.sRecVolume = (ucLeftVol << 8) | ucRightVol;
	TI_Set_ADC_Volume(ucLeftVol,ucRightVol);

	return 0;
}



/*---------- for TIAIC32	functions group----------*/
//(MAX)  0 ~ (MIN) 127
static INT TI_Set_DAC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol){	//0~31


	
	/* 43:ldac vol */
	UINT16 data;
	if (ucLeftVol!=0)
		data = 127 - ucLeftVol*4;
	else
		data = 0x7f;
	I2C_Write_Data(43,data);
	
	if (ucRightVol!=0)
		data = 127 - ucRightVol*4;
	else
		data = 0x7f;
	I2C_Write_Data(44,data);
	
	if (ucLeftVol!=0)	//0~63
		data = (1<<7) | (127 - ucLeftVol*4);
	else
		data = 0xff;
	I2C_Write_Data(47,data);
	
	if (ucRightVol!=0)
		data = (1<<7) | (127 - ucRightVol*4);
	else
		data = 0xff;
	I2C_Write_Data(64,data);
	
	
	
	return 0;
}	
static INT TI_Set_ADC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol){
	
	

	return 0;
}

#ifdef _I2C

static void I2C_Write_Data(UINT8 regAddr, UINT8 data)
{
	int i;

	UINT8	devID = 0x30;
	
	/*==== start condition ====*/
	outpw(REG_SerialBusCR,READSTATUS() | TI_SCLK | TI_SDA);//SCLK high, SDA high
	Delay(0x1);
	outpw(REG_SerialBusCR,READSTATUS() & ~TI_SDA | TI_SCLK );//SCLK high, SDA low
	Delay(0x1);
	
	/*==== send 7-bits device address ====*/
	for (i=7;i>=0;i--){
		if  ((1<<i)&devID){
			outpw(REG_SerialBusCR,READSTATUS() & ~TI_SCLK | TI_SDA);//sclk low
			Delay(0x1);
			outpw(REG_SerialBusCR,READSTATUS() | TI_SCLK | TI_SDA);//sclk high
			Delay(0x1);
		}	
		else{
			outpw(REG_SerialBusCR,READSTATUS() & ~TI_SDA & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_SerialBusCR,READSTATUS() & ~TI_SDA | TI_SCLK);//sclk high
			Delay(0x1);
			
		}
	}
	
	/*toggle for ACK*/
	outpw(REG_SerialBusCR,READSTATUS() & ~TI_SCLK);
	Delay(0x1);
	outpw(REG_SerialBusCR,READSTATUS() | TI_SCLK);
	Delay(0x1);
	
	/*==== send 8-bits register address ====*/
	for (i=7;i>=0;i--){
		if  ((1<<i)&regAddr){//data is 1
			outpw(REG_SerialBusCR,READSTATUS() & ~TI_SCLK | TI_SDA);//sclk low
			Delay(0x1);
			outpw(REG_SerialBusCR,READSTATUS() | TI_SCLK | TI_SDA);//sclk high
			Delay(0x1);
		}	
		else{
			outpw(REG_SerialBusCR,READSTATUS() & ~TI_SDA & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_SerialBusCR,READSTATUS() & ~TI_SDA | TI_SCLK);//sclk high
			Delay(0x1);
			
		}
	}
	
	/*toggle for ACK*/
	outpw(REG_SerialBusCR,READSTATUS() & ~TI_SCLK);
	Delay(0x1);
	outpw(REG_SerialBusCR,READSTATUS() | TI_SCLK);
	Delay(0x1);
	
	/*==== send 8-bits register data ====*/
	for (i=7;i>=0;i--){
		if  ((1<<i)&data){//data is 1
			outpw(REG_SerialBusCR,READSTATUS() & ~TI_SCLK | TI_SDA);//sclk low
			Delay(0x1);
			outpw(REG_SerialBusCR,READSTATUS() | TI_SCLK | TI_SDA);//sclk high
			Delay(0x1);
		}	
		else{
			outpw(REG_SerialBusCR,READSTATUS() & ~TI_SDA & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_SerialBusCR,READSTATUS() & ~TI_SDA | TI_SCLK);//sclk high
			Delay(0x1);
			
		}
	}
	
	/*toggle for ACK*/
	outpw(REG_SerialBusCR,READSTATUS() & ~TI_SCLK);
	Delay(0x1);
	outpw(REG_SerialBusCR,READSTATUS() | TI_SCLK);
	Delay(0x1);
	
	/*==== stop condition ====*/
	outpw(REG_SerialBusCR,READSTATUS() &~TI_SDA | TI_SCLK  );//SCLK high,SDA low
	Delay(0x1);
	outpw(REG_SerialBusCR,READSTATUS() | TI_SDA | TI_SCLK  );//SCLK high,SDA low
	Delay(0x1);
	
	
}

#else
	
static void I2C_Write_Data(UINT8 regAddr, UINT8 data)
{
	int i;

	UINT8	devID = 0x30;
	
	/*==== start condition ====*/
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK | TI_SDA);//SCLK high, SDA high
	Delay(0x1);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SDA | TI_SCLK );//SCLK high, SDA low
	Delay(0x1);
	
	/*==== send 7-bits device address ====*/
	for (i=7;i>=0;i--){
		if  ((1<<i)&devID){
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK | TI_SDA);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK | TI_SDA);//sclk high
			Delay(0x1);
		}	
		else{
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SDA & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SDA | TI_SCLK);//sclk high
			Delay(0x1);
			
		}
	}
	
	/*toggle for ACK*/
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);
	Delay(0x1);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK);
	Delay(0x1);
	
	/*==== send 8-bits register address ====*/
	for (i=7;i>=0;i--){
		if  ((1<<i)&regAddr){//data is 1
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK | TI_SDA);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK | TI_SDA);//sclk high
			Delay(0x1);
		}	
		else{
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SDA & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SDA | TI_SCLK);//sclk high
			Delay(0x1);
			
		}
	}
	
	/*toggle for ACK*/
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);
	Delay(0x1);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK);
	Delay(0x1);
	
	/*==== send 8-bits register data ====*/
	for (i=7;i>=0;i--){
		if  ((1<<i)&data){//data is 1
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK | TI_SDA);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK | TI_SDA);//sclk high
			Delay(0x1);
		}	
		else{
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SDA & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SDA | TI_SCLK);//sclk high
			Delay(0x1);
			
		}
	}
	
	/*toggle for ACK*/
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);
	Delay(0x1);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK);
	Delay(0x1);
	
	/*==== stop condition ====*/
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) &~TI_SDA | TI_SCLK  );//SCLK high,SDA low
	Delay(0x1);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SDA | TI_SCLK  );//SCLK high,SDA low
	Delay(0x1);
	
	
}

#endif

INT TIAIC32_DAC_Setup(){
	INT nRetValue = 0;
	I2C_Write_Data(1,0x80);//self clearing software reset
	I2C_Write_Data(7,0xA);//codec datapath set up
	I2C_Write_Data(8,0x0);//set slave mode 3D disable(enable:1<<2)
	I2C_Write_Data(9,0x0);//I2S mode, word length=16bits
	I2C_Write_Data(37,0xC0);//LDAC power up, RDAC power up
	I2C_Write_Data(41,0x0);//LDAC output to DAC_L1, RDAC output to DAC_R1
	I2C_Write_Data(42,0x34);//power-on time 1ms,Ramp-up time 1ms
	
	
	nRetValue = TI_Set_DAC_Volume(0,0);//r11 r12 set volume
	I2C_Write_Data(51,1<<3);//HPLOUT Ouput level control
	I2C_Write_Data(65,1<<3);//HPROUT Ouput level control
	
	
	return 0;

}
void TIAIC32_ADC_Setup(){
	
	
}   
	

/*---------- end of TIAIC32's functions group ----------*/

#endif	/* HAVE_IIS */
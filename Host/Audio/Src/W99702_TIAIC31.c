/****************************************************************************
 * 
 * FILENAME
 *     W99702_TIAIC31.c
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
#include "TIAIC31.h"


#ifdef HAVE_TIAIC31



static AUDIO_T	_tIIS;

#define	IIS_ACTIVE				0x1
#define	IIS_PLAY_ACTIVE			0x2
#define IIS_REC_ACTIVE			0x4

static BOOL	 _bIISActive = 0;
static UINT32 _uIISCR = 0;
static INT volatile _nRETRYCOUNT = 10;
static BOOL  _bSetDACVol = 0;
static BOOL	 _bSetADCVol = 0;
static BOOL	 _bSetOPDev = 0;

static OP_DEV_E _eOPDEVICE = OP_DEV_HP;
#define RETRYCOUNT1 20
#define PLLDISABLE 0

extern BOOL _bADDA_Active;
extern BOOL	_bIsLeftCH;

INT TIAIC31_DAC_Setup(void);
INT TIAIC31_ADC_Setup(void);

INT TIAIC31_I2C_Write_Data(UINT8 regAddr, UINT8 data);
INT TIAIC31_I2C_Read_Data(UINT8 regAddr);

static INT TI_Set_DAC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol);
static INT TI_Set_ADC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol);




static VOID IIS_Set_Data_Format(INT);
static VOID IIS_Set_Sample_Frequency(INT);

static void Delay(int nCnt)
{
	volatile int  loop;
	for (loop=0; loop<nCnt*100; loop++);
}

#ifdef ECOS
cyg_uint32  iis_play_isr(cyg_vector_t vector, cyg_addrword_t data)
#else
static void  iis_play_isr()
#endif
{
	outpw(REG_ACTL_CON, inpw(REG_ACTL_CON) | T_DMA_IRQ);
	
	if (_bSetDACVol){
		TI_Set_DAC_Volume((_tIIS.sPlayVolume>>8)&0xff,_tIIS.sPlayVolume&0xff);
		_bSetDACVol = 0;
	}
	if (_bSetOPDev) {
		TI_Set_DAC_Volume((_tIIS.sPlayVolume>>8)&0xff,_tIIS.sPlayVolume&0xff);
		_bSetOPDev = 0;
	}
	if (_tIIS.bPlayLastBlock)
	{
		outpw(REG_ACTL_PSR, P_DMA_MIDDLE_IRQ | P_DMA_END_IRQ);
		tiaic31StopPlay();
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
	
	if (_bSetADCVol){
		TI_Set_ADC_Volume((_tIIS.sRecVolume>>8)&0xff,_tIIS.sRecVolume&0xff);
		_bSetADCVol = 0;
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

	//outpw(REG_GPIO_PE,inpw(REG_GPIO_PE) | 0x22 );

	_uIISCR = 0;
	return 0;
}







/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      tiaic31StartPlay                                                   */
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
INT  tiaic31StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
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
	
	nStatus = TIAIC31_DAC_Setup();
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
/*      tiaic31StopPlay                                                    */
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
INT  tiaic31StopPlay()
{
	
	TI_Set_DAC_Volume(0,0);
	
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL));

	if (!(_bIISActive & IIS_PLAY_ACTIVE))
		return ERR_IIS_PLAY_INACTIVE;
	
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
/*      tiaic31StartRecord                                                 */
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
INT  tiaic31StartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
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
//	sysSetAIC2SWMode();					/* Set AIC into SW mode */
	sysEnableInterrupt(AU_REC_INT_NUM);
#endif

	_tIIS.fnRecCallBack = fnCallBack;
	_tIIS.nRecSamplingRate = nSamplingRate;


	nStatus = TIAIC31_ADC_Setup();
	if (nStatus != 0)
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
/*      tiaic31StopRecord                                                  */
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
INT  tiaic31StopRecord()
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
		case AU_SAMPLE_RATE_12000:						//12KHz
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
		case AU_SAMPLE_RATE_44100:					//44.1KHz,MCLK=16.934Mhz
			_uIISCR = _uIISCR | SCALE_1 | FS_256  | BCLK_48;
			//_uIISCR = _uIISCR | FS_256 | SCALE_2 | BCLK_32;
			break;
		case AU_SAMPLE_RATE_48000:						//48KHz
			_uIISCR = _uIISCR | FS_256 | SCALE_1 | BCLK_32;
			//_uIISCR = _uIISCR | FS_256 | SCALE_2 | BCLK_32;

			break;
		default:break;
	}
	_uIISCR = _uIISCR | (1<<8);
	outpw(REG_ACTL_IISCON, _uIISCR);
}
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      tiaic31SetPlayVolume                                              */
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
INT  tiaic31SetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol)  //0~31
{

	if (ucLeftVol>31)
		ucLeftVol=31;
	if (ucRightVol>31)
		ucRightVol=31;
	_tIIS.sPlayVolume = (ucLeftVol << 8) | ucRightVol;
	_bSetDACVol = 1;
#if 0
	nRetValue = TI_Set_DAC_Volume(ucLeftVol,ucRightVol);
	if (nRetValue!=0){
		return nRetValue;
	}
#endif	
	return 0;
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      tiaic31SetRecordVolume                                                 */
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
INT  tiaic31SetRecordVolume(UINT8 ucLeftVol, UINT8 ucRightVol)  //0~31
{
	if (ucLeftVol>31)
		ucLeftVol=31;
	if (ucRightVol>31)
		ucRightVol=31;
	_tIIS.sRecVolume = (ucLeftVol << 8) | ucRightVol;
	_bSetADCVol = 1;

	return 0;
}



/*---------- for TIAIC31	functions group----------*/
//(MAX)  0 ~ (MIN) 127
static INT TI_Set_DAC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol){	//0~31


	
	/* R43:LDAC vol */
	UINT16 data;
	if (ucLeftVol!=0)
		data = 124 - ucLeftVol*4;
	else
		data = 0x7f;
	TIAIC31_I2C_Write_Data(43,data);
	
	if (TIAIC31_I2C_Read_Data(43)!=data)
	{
		_error_msg("Register 43 R/W error\n");	
		_bSetDACVol = 1;
	}
	/* R44:RDAC vol */
	if (ucRightVol!=0)
		data = 124 - ucRightVol*4;
	else
		data = 0x7f;
	TIAIC31_I2C_Write_Data(44,data);
	if (TIAIC31_I2C_Read_Data(44)!=data)
	{
		_error_msg("Register 44 R/W error\n");	
		_bSetDACVol = 1;
	}
	if (_eOPDEVICE == OP_DEV_HP){//DAC_L1 to HPLOUT;DAC_R1 to HPROUT
		TIAIC31_I2C_Write_Data(51,0x9f);//HPLOUT Ouput un-mute
		TIAIC31_I2C_Write_Data(65,0x9f);//HPROUT Ouput un-mute
		TIAIC31_I2C_Write_Data(86,0x0);//LEFT_LOP/M Ouput mute
		TIAIC31_I2C_Write_Data(93,0x0);//RIGHT_LOP/M Ouput mute
		if (ucLeftVol!=0)	//0~63
			data = (1<<7) | (124 - ucLeftVol*4);
		else
			data = 0xff;
		TIAIC31_I2C_Write_Data(47,data | (1<<7));
		if (TIAIC31_I2C_Read_Data(47)!=(data | (1<<7)))
		{
			_error_msg("Register 47 R/W error\n");	
			_bSetDACVol = 1;
		}
		if (ucRightVol!=0)
			data = (1<<7) | (124 - ucRightVol*4);
		else
			data = 0xff;
		
		TIAIC31_I2C_Write_Data(64,data | (1<<7));
		if (TIAIC31_I2C_Read_Data(64)!=(data | (1<<7)))
		{
			_error_msg("Register 64 R/W error\n");	
			_bSetDACVol = 1;
		}
	
		TIAIC31_I2C_Write_Data(82,0x0);
		if (TIAIC31_I2C_Read_Data(82)!=0x0){
			_error_msg("Register 82 R/W error\n");	
			_bSetDACVol = 1;
		}
		TIAIC31_I2C_Write_Data(92,0x0);
		if (TIAIC31_I2C_Read_Data(92)!=0x0){
			_error_msg("Register 92 R/W error\n");	
			_bSetDACVol = 1;
		}
	}else if (_eOPDEVICE == OP_DEV_SP){//DAC_L1 to HPLCOM;DAC_R1 to HPRCOM
		TIAIC31_I2C_Write_Data(86,0x9f);//LEFT_LOP/M Ouput un-mute
		TIAIC31_I2C_Write_Data(93,0x9f);//RIGHT_LOP/M Ouput un-mute
		TIAIC31_I2C_Write_Data(51,0x0);//HPLOUT mute
		TIAIC31_I2C_Write_Data(65,0x0);//HPROUT mute
		if (ucLeftVol!=0)	//0~63
			data = (1<<7) | (124 - ucLeftVol*4 );
		else
			data = 0xff;
		TIAIC31_I2C_Write_Data(82,data | (1<<7));//DAC_L1 to LEFT_LOP/M Volume
		if (TIAIC31_I2C_Read_Data(82)!=(data | (1<<7)))
		{
			_error_msg("Register 82 R/W error\n");	
			_bSetDACVol = 1;
		}
		if (ucRightVol!=0)
			data = (1<<7) | (124 - ucRightVol*4);
		else
			data = 0xff;
		TIAIC31_I2C_Write_Data(92,data | (1<<7));//DAC_R1 to RIGHT_LOP/M Volume
		if (TIAIC31_I2C_Read_Data(92)!=(data | (1<<7)))
		{
			_error_msg("Register 92 R/W error\n");	
			_bSetDACVol = 1;
		}
		
		TIAIC31_I2C_Write_Data(47,0x0);
		if (TIAIC31_I2C_Read_Data(47)!=0x0){
			_error_msg("Register 47 R/W error\n");	
			_bSetDACVol = 1;
		}
		TIAIC31_I2C_Write_Data(64,0x0);
		if (TIAIC31_I2C_Read_Data(64)!=0x0){
			_error_msg("Register 64 R/W error\n");	
			_bSetDACVol = 1;
		}
	}
	
	_error_msg("status = 0x%x\n",TIAIC31_I2C_Read_Data(94));
	_error_msg("status = 0x%x\n",TIAIC31_I2C_Read_Data(95));

	return 0;
}	
static INT TI_Set_ADC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol){
	
	UINT16 data;
	
	if (ucLeftVol!=0)
		data = ucLeftVol*4;
	else
		data = 0x0;
	TIAIC31_I2C_Write_Data(15,data);
	
	if (TIAIC31_I2C_Read_Data(15)!=data)
	{
		_error_msg("Register 15 R/W error\n");	
		_bSetDACVol = 1;
	}
		
	if (ucRightVol!=0)
		data = ucRightVol*4;
	else
		data = 0x0;
	TIAIC31_I2C_Write_Data(16,data);
	
	if (TIAIC31_I2C_Read_Data(16)!=data)
	{
		_error_msg("Register 16 R/W error\n");	
		_bSetDACVol = 1;
	}
	
	return 0;
}

#ifdef _I2C	
INT TIAIC31_I2C_Write_Data(UINT8 regAddr, UINT8 data)
{
	int i;

	UINT8	devID = 0x30;
	
	outpw(REG_PADC0, inpw(REG_PADC0) | (7<<1));
	
	
	/*==== start condition ====*/
	outpw(REG_SerialBusCR,READSTATUS() | TI_SCLK | TI_SDA);//SCLK high, SDA high
	Delay(0x2);
	outpw(REG_SerialBusCR,READSTATUS() & ~TI_SDA | TI_SCLK );//SCLK high, SDA low
	Delay(0x2);
	
	/*==== send 7-bits device address ====*/
	for (i=7;i>=0;i--){
		if  ((1<<i)&devID){
			outpw(REG_SerialBusCR,READSTATUS() & ~TI_SCLK | TI_SDA);//sclk low
			Delay(0x2);
			outpw(REG_SerialBusCR,READSTATUS() | TI_SCLK | TI_SDA);//sclk high
			Delay(0x2);
		}
		else{
			outpw(REG_SerialBusCR,READSTATUS() & ~TI_SDA & ~TI_SCLK);//sclk low
			Delay(0x2);
			outpw(REG_SerialBusCR,READSTATUS() & ~TI_SDA | TI_SCLK);//sclk high
			Delay(0x2);
			
		}
	}
	
	/*toggle for ACK*/
	outpw(REG_SerialBusCR,READSTATUS() & ~TI_SCLK);
	Delay(0x2);
	outpw(REG_SerialBusCR,READSTATUS() | TI_SCLK);
	Delay(0x2);
	
	/*==== send 8-bits register address ====*/
	for (i=7;i>=0;i--){
		if  ((1<<i)&regAddr){//data is 1
			outpw(REG_SerialBusCR,READSTATUS() & ~TI_SCLK | TI_SDA);//sclk low
			Delay(0x2);
			outpw(REG_SerialBusCR,READSTATUS() | TI_SCLK | TI_SDA);//sclk high
			Delay(0x2);
		}	
		else{
			outpw(REG_SerialBusCR,READSTATUS() & ~TI_SDA & ~TI_SCLK);//sclk low
			Delay(0x2);
			outpw(REG_SerialBusCR,READSTATUS() & ~TI_SDA | TI_SCLK);//sclk high
			Delay(0x2);
			
		}
	}
	
	/*toggle for ACK*/
	outpw(REG_SerialBusCR,READSTATUS() & ~TI_SCLK);
	Delay(0x2);
	outpw(REG_SerialBusCR,READSTATUS() | TI_SCLK);
	Delay(0x2);
	
	/*==== send 8-bits register data ====*/
	for (i=7;i>=0;i--){
		if  ((1<<i)&data){//data is 1
			outpw(REG_SerialBusCR,READSTATUS() & ~TI_SCLK | TI_SDA);//sclk low
			Delay(0x2);
			outpw(REG_SerialBusCR,READSTATUS() | TI_SCLK | TI_SDA);//sclk high
			Delay(0x2);
		}	
		else{
			outpw(REG_SerialBusCR,READSTATUS() & ~TI_SDA & ~TI_SCLK);//sclk low
			Delay(0x2);
			outpw(REG_SerialBusCR,READSTATUS() & ~TI_SDA | TI_SCLK);//sclk high
			Delay(0x2);
			
		}
	}
	
	/*toggle for ACK*/
	outpw(REG_SerialBusCR,READSTATUS() & ~TI_SCLK);
	Delay(0x2);
	outpw(REG_SerialBusCR,READSTATUS() | TI_SCLK);
	Delay(0x2);
	
	/*==== stop condition ====*/
	outpw(REG_SerialBusCR,READSTATUS() &~TI_SDA | TI_SCLK  );//SCLK high,SDA low
	Delay(0x2);
	outpw(REG_SerialBusCR,READSTATUS() | TI_SDA | TI_SCLK  );//SCLK high,SDA high
	Delay(0x2);
	
	return 0;
}
#else
INT TIAIC31_I2C_Write_Data(UINT8 regAddr, UINT8 data)
{

	int i;

	UINT8	devID = 0x30;
	
retry:	
	//outpw(REG_PADC0, inpw(REG_PADC0) & ~(1<<9) );
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~(TI_SCLK | TI_SDA) );	
	
	/*==== start condition ====*/
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK | TI_SDA);//SCLK high, SDA high
	Delay(0x2);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SDA | TI_SCLK );//SCLK high, SDA low
	Delay(0x2);
	
	/*==== send 7-bits device address and 1 bit for write operation====*/
	for (i=7;i>=0;i--){
		if  ((1<<i)&devID){
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK | TI_SDA);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK | TI_SDA);//sclk high
			Delay(0x2);
		}	
		else{
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK & ~TI_SDA);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SDA | TI_SCLK);//sclk high
			Delay(0x2);
			
		}
	}
	
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | TI_SDA);//for receive ACK
	/*toggle for ACK*/
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);
	Delay(0x2);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK);
	Delay(0x2);
#if 1	
	if ((inpw(REG_GPIO_STS) & TI_SDA)!=0){
		_debug_msg("1.TIAIC31 does not ACK!%d\n",_nRETRYCOUNT);
		if (_nRETRYCOUNT-- > 0)
			goto retry;
		else{
			_debug_msg("TIAIC31 reg %d write 0x%x is timeout\n",regAddr,data);
			_nRETRYCOUNT = 10;
			return -1;
		}
	}
#endif
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~TI_SDA);//
	
	/*==== send 8-bits register address ====*/
	for (i=7;i>=0;i--){
		if  ((1<<i)&regAddr){//data is 1
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK | TI_SDA);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK | TI_SDA);//sclk high
			Delay(0x2);
		}	
		else{
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK & ~TI_SDA);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SDA | TI_SCLK);//sclk high
			Delay(0x2);
			
		}
	}
	
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | TI_SDA);//for receive ACK
	/*toggle for ACK*/
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);
	Delay(0x2);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK);
	Delay(0x2);
#if 1	
	if ((inpw(REG_GPIO_STS) & TI_SDA)!=0){
		_debug_msg("2.TIAIC31 does not ACK!%d\n",_nRETRYCOUNT);
		if (_nRETRYCOUNT-- > 0)
			goto retry;
		else{
			_debug_msg("TIAIC31 reg %d write 0x%x is timeout\n",regAddr,data);
			_nRETRYCOUNT = 10;
			return -1;
		}
	}
#endif
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~TI_SDA);//
	
	/*==== send 8-bits register data ====*/
	for (i=7;i>=0;i--){
		if  ((1<<i)&data){//data is 1
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK | TI_SDA);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK | TI_SDA);//sclk high
			Delay(0x2);
		}	
		else{
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK & ~TI_SDA);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SDA | TI_SCLK);//sclk high
			Delay(0x2);
			
		}
	}
	
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | TI_SDA);//for receive ACK
	/*toggle for ACK*/
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);
	Delay(0x2);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK);
	Delay(0x2);
#if 1	
	if ((inpw(REG_GPIO_STS) & TI_SDA)!=0){
		_debug_msg("3.TIAIC31 does not ACK!%d\n",_nRETRYCOUNT);
		if (_nRETRYCOUNT-- > 0)
			goto retry;
		else{
			_debug_msg("TIAIC31 reg %d write 0x%x is timeout\n",regAddr,data);
			_nRETRYCOUNT = 10;
			return -1;
		}
	}
#endif
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~TI_SDA);//
	
	/*==== stop condition ====*/
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SDA & ~TI_SCLK  );//SCLK low,SDA low
	Delay(0x2);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK);//SCLK high SDA low
	Delay(0x2);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SDA);//SCLK high,SDA high
	Delay(0x2);
	
	_nRETRYCOUNT = 10;
	
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | (TI_SCLK | TI_SDA) );
	return 0;
}
#endif

INT TIAIC31_I2C_Read_Data(UINT8 regAddr)
{

	int i;

	UINT8	devID = 0x30,recData=0;
	

	//outpw(REG_PADC0, inpw(REG_PADC0) & ~(1<<9) );
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~(TI_SCLK | TI_SDA) );	
retry:	
	/*==== start condition ====*/
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK | TI_SDA);//SCLK high, SDA high
	Delay(0x2);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SDA | TI_SCLK );//SCLK high, SDA low
	Delay(0x2);
	
	/*==== send 7-bits device address and 1 bit for write operation====*/
	for (i=7;i>=0;i--){
		if  ((1<<i)&devID){
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK | TI_SDA);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK | TI_SDA);//sclk high
			Delay(0x2);
		}	
		else{
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK & ~TI_SDA);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SDA | TI_SCLK);//sclk high
			Delay(0x2);
			
		}
	}
	
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | TI_SDA);//for receive ACK
	/*toggle for ACK*/
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);
	Delay(0x2);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK);
	Delay(0x2);
#if 1	
	if ((inpw(REG_GPIO_STS) & TI_SDA)!=0){
		_debug_msg("1.TIAIC31 does not ACK!%d\n",_nRETRYCOUNT);
		if (_nRETRYCOUNT-- > 0)
			goto retry;
		else{
			_debug_msg("TIAIC31 reg %d read is timeout\n",regAddr);
			_nRETRYCOUNT = 10;
			return -1;
		}
	}
#endif
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~TI_SDA);//
	
	/*==== send 8-bits register address ====*/
	for (i=7;i>=0;i--){
		if  ((1<<i)&regAddr){//data is 1
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK | TI_SDA);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK | TI_SDA);//sclk high
			Delay(0x2);
		}	
		else{
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK & ~TI_SDA);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SDA | TI_SCLK);//sclk high
			Delay(0x2);
			
		}
	}
	
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | TI_SDA);//for receive ACK
	/*toggle for ACK*/
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);
	Delay(0x2);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK);
	Delay(0x2);
#if 1	
	if ((inpw(REG_GPIO_STS) & TI_SDA)!=0){
		_debug_msg("2.TIAIC31 does not ACK!%d\n",_nRETRYCOUNT);
		if (_nRETRYCOUNT-- > 0)
			goto retry;
		else{
			_debug_msg("TIAIC31 reg %d read is timeout\n",regAddr);
			_nRETRYCOUNT = 10;
			return -1;
		}
	}
#endif
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~TI_SDA);//
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK &~TI_SDA);//SCLK low sda low
	Delay(0x1);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK | TI_SDA);//SCLK low sda high
	Delay(0x1);
		
	/*=== repeat start ===*/
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK | TI_SDA);//SCLK high, SDA high
	Delay(0x1);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SDA | TI_SCLK );//SCLK high, SDA low
	Delay(0x1);
	
	
	/*==== send 7-bits device address and 1 bit for read operation====*/
	for (i=7;i>=0;i--){
		if  ((1<<i)&(devID|0x1)){
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK | TI_SDA);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK | TI_SDA);//sclk high
			Delay(0x2);
		}	
		else{
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK & ~TI_SDA);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SDA | TI_SCLK);//sclk high
			Delay(0x2);
			
		}
	}
	
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | TI_SDA);//for receive ACK
	/*toggle for ACK*/
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);
	Delay(0x2);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK);
	Delay(0x2);
#if 1	
	if ((inpw(REG_GPIO_STS) & TI_SDA)!=0){
		_debug_msg("3.TIAIC31 does not ACK!%d\n",_nRETRYCOUNT);
		if (_nRETRYCOUNT-- > 0)
			goto retry;
		else{
			_debug_msg("TIAIC31 reg %d read is timeout\n",regAddr);
			_nRETRYCOUNT = 10;
			return -1;
		}
	}
#endif		
	
	/*==== receive 8-bits register data ====*/
	for (i=7;i>=0;i--){
		outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);//sclk low
		Delay(0x2);
		if ((inpw(REG_GPIO_STS) & TI_SDA)!=0){//data is 1
			recData = recData | (1<<i);
		}
		outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK);//sclk high
		Delay(0x2);
	}
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~TI_SDA);//set SDA to output mode
	
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);//sclk low
	Delay(0x1);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK | TI_SDA);//sclk low
	Delay(0x1);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK | TI_SDA);//sclk high
	Delay(0x2);
	
				
	/*==== stop condition ====*/
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK | TI_SDA  );//SCLK low,SDA high
	Delay(0x1);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SDA & ~TI_SCLK  );//SCLK low,SDA low
	Delay(0x2);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK);//SCLK high SDA low
	Delay(0x2);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SDA);//SCLK high,SDA high
	Delay(0x2);
	
	_nRETRYCOUNT = 10;
	
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | (TI_SCLK | TI_SDA) );
	
	return recData;
}


static void TIAIC31_Set_Sample_Frequency(int choose_sf){
	INT data;
	
#if PLLDISABLE
	if ((choose_sf%AU_SAMPLE_RATE_8000==0) || (choose_sf%AU_SAMPLE_RATE_12000 ==0)){//Q=2 
		TIAIC31_I2C_Write_Data(7,0xa);//codec datapath set up,FSref=48khz
		TIAIC31_I2C_Write_Data(3,0x10);
		
		if (TIAIC31_I2C_Read_Data(7)!=0xa)
		{
			_error_msg("Register 7 R/W error\n");	
	
		}
		if (TIAIC31_I2C_Read_Data(3)!=0x10)
		{
			_error_msg("Register 3 R/W error\n");	
	
		}
		
	}
	else if (choose_sf%AU_SAMPLE_RATE_11025==0){//Q=3
		TIAIC31_I2C_Write_Data(7,0x8a);//codec datapath set up,FSref=44.1khz
		TIAIC31_I2C_Write_Data(3,0x18);
		if (TIAIC31_I2C_Read_Data(7)!=0x8a)
			_error_msg("Register R/W error\n");	
		if (TIAIC31_I2C_Read_Data(3)!=0x18)
			_error_msg("Register R/W error\n");	

	}
#else
	if ((choose_sf%AU_SAMPLE_RATE_8000==0) || (choose_sf%AU_SAMPLE_RATE_12000 ==0)){//P=1,R=1,J=8,D=0
		TIAIC31_I2C_Write_Data(3,0x81);//P
		TIAIC31_I2C_Write_Data(4,(0x8<<2));//J
		TIAIC31_I2C_Write_Data(5,0x0);//D 
		TIAIC31_I2C_Write_Data(6,0x0);//D
		TIAIC31_I2C_Write_Data(11,0x1);//R
		TIAIC31_I2C_Write_Data(7,0xa);//codec datapath set up,FSref=48khz
		if (TIAIC31_I2C_Read_Data(7)!=0xa)
			_error_msg("Register R/W error\n");	
	}else if (choose_sf%AU_SAMPLE_RATE_11025==0){//P=1,R=1,J=5,D=3333
		TIAIC31_I2C_Write_Data(3,0x81);//P
		TIAIC31_I2C_Write_Data(4,(0x5<<2));//J
		TIAIC31_I2C_Write_Data(5,0x34);//D 
		TIAIC31_I2C_Write_Data(6,0x14);//D
		TIAIC31_I2C_Write_Data(11,0x1);//R
		TIAIC31_I2C_Write_Data(7,0x8a);//codec datapath set up,FSref=44.1khz
		if (TIAIC31_I2C_Read_Data(7)!=0x8a)
			_error_msg("Register R/W error\n");	
	}
#endif	
	switch (choose_sf)
	{
		case AU_SAMPLE_RATE_8000:							//8KHz
			
			data = 0xaa;//FSref/6
			break;
		case AU_SAMPLE_RATE_11025:					//11.025KHz
			data = 0x66;//FSref/4
			break;
		case AU_SAMPLE_RATE_12000:						//12KHz
			data = 0x66;//FSref/4
			break;	
		case AU_SAMPLE_RATE_16000:						//16KHz
			data = 0x44;//FSref/3

			break;
		case AU_SAMPLE_RATE_22050:					//22.05KHz
			data = 0x22;//FSref/2
			break;
		case AU_SAMPLE_RATE_24000:						//24KHz
			data = 0x22;//FSref/2

			break;
		case AU_SAMPLE_RATE_32000:						//32KHz
			data = 0x11;//FSref/1.5
			break;
		case AU_SAMPLE_RATE_44100:					//44.1KHz
			data = 0x00;//FSref/1
			break;
		case AU_SAMPLE_RATE_48000:						//48KHz
			data = 0x00;;//FSref/1

			break;
		default:break;
	}
	TIAIC31_I2C_Write_Data(0x2,data);
	if (TIAIC31_I2C_Read_Data(2)!=data)
	{
		_error_msg("Register 2 R/W error\n");	

	}

	
	
}
INT DACReadbackCheck(){
	if (TIAIC31_I2C_Read_Data(102)!=0x2)
	{
		_error_msg("Register 102 R/W error\n");
		return -1;
	}
#if PLLDISABLE
	if (TIAIC31_I2C_Read_Data(101)!=0x1)
	{
		_error_msg("Register 101 R/W error\n");	
		return -1;
	}
#else	
	if (TIAIC31_I2C_Read_Data(101)!=0x0)
	{
		_error_msg("Register 101 R/W error\n");	
		return -1;
	}
#endif	
	if (TIAIC31_I2C_Read_Data(8)!=0x0)
	{
		_error_msg("Register 8 R/W error\n");	
		return -1;
	}
	if (TIAIC31_I2C_Read_Data(9)!=0x7)
	{
		_error_msg("Register 9 R/W error\n");	
		return -1;
	}
		if (TIAIC31_I2C_Read_Data(37)!=0xc0)
	{
		_error_msg("Register 37 R/W error\n");	
		return -1;
	}	
	if (TIAIC31_I2C_Read_Data(41)!=0x00)
	{
		_error_msg("Register 41 R/W error\n");	
		return -1;
	}
#if 0	
	if (TIAIC31_I2C_Read_Data(51)!=0x9f)
	{
		_error_msg("Register 51 R/W error\n");	
		return -1;
	}
	if (TIAIC31_I2C_Read_Data(65)!=0x9f)
	{
		_error_msg("Register 65 R/W error\n");	
		return -1;
	}	
#endif	
	return 0;
		
}

INT ADCReadbackCheck(){
	if (TIAIC31_I2C_Read_Data(102)!=0x2)
	{
		_error_msg("Register 102 R/W error\n");
		return -1;
	}
#if PLLDISABLE
	if (TIAIC31_I2C_Read_Data(101)!=0x1)
	{
		_error_msg("Register 101 R/W error\n");	
		return -1;
	}
#else	
	if (TIAIC31_I2C_Read_Data(101)!=0x0)
	{
		_error_msg("Register 101 R/W error\n");	
		return -1;
	}
#endif	
	if (TIAIC31_I2C_Read_Data(8)!=0x0)
	{
		_error_msg("Register 8 R/W error\n");	
		return -1;
	}
	if (TIAIC31_I2C_Read_Data(9)!=0x7)
	{
		_error_msg("Register 9 R/W error\n");	
		return -1;
	}
		if (TIAIC31_I2C_Read_Data(17)!=0x0f)
	{
		_error_msg("Register 17 R/W error\n");	
		return -1;
	}	
	if (TIAIC31_I2C_Read_Data(18)!=0xf0)
	{
		_error_msg("Register 18 R/W error\n");	
		return -1;
	}
	if (TIAIC31_I2C_Read_Data(19)!=0x04)
	{
		_error_msg("Register 19 R/W error\n");	
		return -1;
	}
	if (TIAIC31_I2C_Read_Data(22)!=0x04)
	{
		_error_msg("Register 65 R/W error\n");	
		return -1;
	}	
	if ((TIAIC31_I2C_Read_Data(25)&0xc0)!=0x80)
	{
		_error_msg("Register 25 R/W error 0x%x\n",TIAIC31_I2C_Read_Data(25));	
		return -1;
	}
	if (TIAIC31_I2C_Read_Data(26)!=0x0)
	{
		_error_msg("Register 26 R/W error\n");	
		return -1;
	}
	if (TIAIC31_I2C_Read_Data(29)!=0x00)
	{
		_error_msg("Register 29 R/W error\n");	
		return -1;
	}
	
	
	return 0;
		
}

INT TIAIC31_DAC_Setup(){

	INT nTimeOutCount;


	TIAIC31_I2C_Write_Data(0,0x0);//select page 0
	TIAIC31_I2C_Write_Data(1,0x80);//self clearing software reset
	while((TIAIC31_I2C_Read_Data(1)&0x8)!=0);
	Delay(0x10000);

	for (nTimeOutCount=RETRYCOUNT1;nTimeOutCount>0;nTimeOutCount--){
		if (TIAIC31_I2C_Read_Data(102)!=0x2)
			TIAIC31_I2C_Write_Data(102,0x2);//CLKDIV_IN use MCLK ;  N=2
		else
			break;
	}
	if (nTimeOutCount == 0){
		_error_msg("Register R/W error!!\n");
		return -1;
	}
				
#if PLLDISABLE
	TIAIC31_I2C_Write_Data(101,0x1);//CODEC_CLKIN use CLKDIV_OUT;
#else
	TIAIC31_I2C_Write_Data(101,0x0);//CODEC_CLKIN use PLLDIV_OUT;
#endif
	
 	
	TIAIC31_Set_Sample_Frequency(_tIIS.nPlaySamplingRate);
	
	TIAIC31_I2C_Write_Data(8,0x00);//set slave mode 3D disable(enable:1<<2);
	
	TIAIC31_I2C_Write_Data(9,0x07);//I2S mode, word length=16bits;
	
	//while(TIAIC31_I2C_Read_Data(10)!=0x0)TIAIC31_I2C_Write_Data(10,0x0);//;Audio serial data word offset control
	
	//while(TIAIC31_I2C_Read_Data(12)!=0xf)TIAIC31_I2C_Write_Data(12,0x0f);//audio digital filter;
	
	
	TIAIC31_I2C_Write_Data(37,0xc0);//LDAC power up, RDAC power up;

	TIAIC31_I2C_Write_Data(41,0x0);//LDAC output to DAC_L1, RDAC output to DAC_R1;
	//while(TIAIC31_I2C_Read_Data(42)!=0x34)	TIAIC31_I2C_Write_Data(42,0x34);//for pop reduction power-on time 1ms,Ramp-up time 1ms;

	if (_eOPDEVICE == OP_DEV_HP){
		TIAIC31_I2C_Write_Data(51,0x9f);//HPLOUT Ouput un-mute
		TIAIC31_I2C_Write_Data(65,0x9f);//HPROUT Ouput un-mute
		TIAIC31_I2C_Write_Data(86,0x0);//Line Ouput mute
		TIAIC31_I2C_Write_Data(93,0x0);//HPROUT Ouput mute
	}else if(_eOPDEVICE == OP_DEV_SP) {
		TIAIC31_I2C_Write_Data(86,0x9f);//Line Ouput un-mute
		TIAIC31_I2C_Write_Data(93,0x9f);//HPROUT Ouput un-mute
		TIAIC31_I2C_Write_Data(51,0x0);//HPLOUT mute
		TIAIC31_I2C_Write_Data(65,0x0);//HPROUT mute
	}
	
	
	
	//while(TIAIC31_I2C_Read_Data(82)!=0x80)TIAIC31_I2C_Write_Data(82,0x80);;//DAC_L1 to LEFT_LOP/M
	
	//while(TIAIC31_I2C_Read_Data(92)!=0x80)TIAIC31_I2C_Write_Data(92,0x80);;//DAC_R1 to RIGHT_LOP/M
	
	
	//while(TIAIC31_I2C_Read_Data(86)!=0x9b)TIAIC31_I2C_Write_Data(86,0x9b);;//LEFT_LOP/M output level
	
	//while(TIAIC31_I2C_Read_Data(93)!=0x9b)TIAIC31_I2C_Write_Data(93,0x9b);;//LEFT_LOP/M output level
	
	
	

	_error_msg("status = 0x%x\n",TIAIC31_I2C_Read_Data(94));
	_error_msg("status = 0x%x\n",TIAIC31_I2C_Read_Data(95));
	
	TI_Set_DAC_Volume(30,30);//r43 r44 r47 r64 set volume
	
	return DACReadbackCheck();
	


}
INT TIAIC31_ADC_Setup(){
	INT nTimeOutCount;


	TIAIC31_I2C_Write_Data(0,0x0);//select page 0
	TIAIC31_I2C_Write_Data(1,0x80);//self clearing software reset
	while((TIAIC31_I2C_Read_Data(1)&0x8)!=0);
	Delay(0x10000);

	for (nTimeOutCount=RETRYCOUNT1;nTimeOutCount>0;nTimeOutCount--){
		if (TIAIC31_I2C_Read_Data(102)!=0x2)
			TIAIC31_I2C_Write_Data(102,0x2);//use MCLK
		else
			break;
	}
	if (nTimeOutCount == 0){
		_error_msg("Register R/W error!!\n");
		return -1;
	}
	
#if PLLDISABLE
	TIAIC31_I2C_Write_Data(101,0x1);//CODEC_CLKIN use CLKDIV_OUT;
#else
	TIAIC31_I2C_Write_Data(101,0x0);//CODEC_CLKIN use PLLDIV_OUT;
#endif
	
	TIAIC31_Set_Sample_Frequency(_tIIS.nPlaySamplingRate);
	TIAIC31_I2C_Write_Data(8,0x00);//set slave mode 3D disable(enable:1<<2);
	TIAIC31_I2C_Write_Data(9,0x07);//I2S mode, word length=16bits;Re-Sync 
	TI_Set_ADC_Volume(30,30);//r15 r16 set gain
	TIAIC31_I2C_Write_Data(17,0x0f);//IN2L to Left ADC PGA
	TIAIC31_I2C_Write_Data(18,0xf0);//IN2R to Right ADC PGA
	TIAIC31_I2C_Write_Data(19,0x04);//Left ADC power up;IN1L to Left ADC PGA 
	TIAIC31_I2C_Write_Data(21,0x00);//IN1R to Left ADC PGA
	TIAIC31_I2C_Write_Data(22,0x04);//Right ADC power up;IN1R to Left ADC PGA 
	TIAIC31_I2C_Write_Data(25,0x80);//MICBIAS output is powered to 2.5V
	TIAIC31_I2C_Write_Data(26,0x80);//Left AGC disable
	TIAIC31_I2C_Write_Data(29,0x80);//right AGC disable
	
	_error_msg("status = 0x%x\n",TIAIC31_I2C_Read_Data(36));
	
	return ADCReadbackCheck();
	
	return 0;
}   
INT TIAIC31_SetOutputDevice(OP_DEV_E opdev){
	_eOPDEVICE = opdev;
	_bSetOPDev = 1;
	return 0;
	
}
OP_DEV_E TIAIC31_GetOutputDevice(){
	return _eOPDEVICE;
}
	

/*---------- end of TIAIC31's functions group ----------*/

#endif	/* HAVE_IIS */
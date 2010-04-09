/****************************************************************************
 * 
 * FILENAME
 *     W99702_TIDAC26.c
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
#include "TIDAC26.h"


#ifdef HAVE_TIDAC26



static AUDIO_T	_tIIS;

#define	IIS_ACTIVE				0x1
#define	IIS_PLAY_ACTIVE			0x2
#define IIS_REC_ACTIVE			0x4

static BOOL	 _bIISActive = 0;
static UINT32 _uIISCR = 0;
static BOOL _gPowerUp = 0;

extern BOOL _bADDA_Active;

INT TIDAC26_DAC_Setup(void);
void TIDAC26_ADC_Setup(void);
static void TI_Set_Sample_Frequency(INT);
#ifdef _3WIRE
static void ThreeWire_Write_Data(UINT16);
#elif defined(_2WIRE)
static INT TwoWire_Write_Data(UINT16);
#define ThreeWire_Write_Data TwoWire_Write_Data
#endif
static INT TI_Set_DAC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol);
static INT TI_Set_ADC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol);

#define TIDEVID 0


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
		tidac26StopPlay();
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
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~(TI_SS | TI_MOSI | TI_SCLK) );
	
	/** end of for verification board **/

	
	//ThreeWire_Write_Data(0x0000);//R0 software reset;all registers reture to default value
	//Delay(1000);
	


	_uIISCR = 0;
	return 0;
}







/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      tidac26StartPlay                                                   */
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
INT  tidac26StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
								INT nChannels, INT data_format)
{
	INT		nStatus,nTime0;
	UINT32	uHWAdd;
	
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
	
	nStatus = TIDAC26_DAC_Setup();
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
	uHWAdd = inpw(0x7ff03020);
	while(1){
		if (sysGetTicks(TIMER0) - nTime0 >= 100)
		{
			nTime0 = sysGetTicks(TIMER0);
			if (inpw(0x7ff03020)==uHWAdd)
				return -1;
			else
				break;
			
		}
	}
	
		
	
	
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      tidac26StopPlay                                                    */
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
INT  tidac26StopPlay()
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
/*      tidac26StartRecord                                                 */
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
INT  tidac26StartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
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


	TIDAC26_ADC_Setup();

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
/*      tidac26StopRecord                                                  */
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
INT  tidac26StopRecord()
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
/*      tidac26SetPlayVolume                                              */
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
INT  tidac26SetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol)  //0~31
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
/*      tidac26SetRecordVolume                                                 */
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
INT  tidac26SetRecordVolume(UINT8 ucLeftVol, UINT8 ucRightVol)  //0~31
{
	if (ucLeftVol>31)
		ucLeftVol=31;
	if (ucRightVol>31)
		ucRightVol=31;
	_tIIS.sRecVolume = (ucLeftVol << 8) | ucRightVol;
	TI_Set_ADC_Volume(ucLeftVol,ucRightVol);

	return 0;
}



/*---------- for TIDAC26	functions group----------*/
//(MIN)  0 ~ (MAX) 255
static INT TI_Set_DAC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol){	//0~31
	INT nTimeOutCount;
	INT nRetValue = 0;
	
	/* R11:DACOUTL R12:DACOUTR, R52:HPOUTL R53:HPOUTR, R54:SPOUTL R55:SPOUTR */
	UINT16 data;
	if (ucLeftVol!=0)
		data = R11 | (1<<8) | (ucLeftVol*3 + 162);
	else
		data = R11 | (1<<8);
	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(data);
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}
	
	
	if (ucRightVol!=0)
		data = R12 | (1<<8) | (ucRightVol*3 + 162);
	else
		data = R12 | (1<<8);
	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(data);
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}
	
	if (ucLeftVol!=0)	//0~63
		data = R52 | (1<<8) | (1<<7) | (ucLeftVol + 32);
	else
		data = R52 | (1<<8)| (1<<7);
	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(data);
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}
	
	if (ucRightVol!=0)
		data = R53 | (1<<8)| (1<<7) | (ucRightVol + 32);
	else
		data = R53 | (1<<8)| (1<<7);
	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(data);
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}
	
	if (ucLeftVol!=0)
		data = R54 | (1<<8)| (1<<7) | (ucLeftVol + 32);
	else
		data = R54 | (1<<8)| (1<<7);
	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(data);
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}
	
	if (ucRightVol!=0)
		data = R55 | (1<<8)| (1<<7) | (ucRightVol + 32);
	else
		data = R55 | (1<<8| (1<<7));
	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(data);
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}
	return 0;
}	
static INT TI_Set_ADC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol){
	UINT16 data;
	if (ucLeftVol!=0)
		data = R15 | (1<<8) | (ucLeftVol*3 + 162);
	else
		data = R15 | (1<<8);
	ThreeWire_Write_Data(data);
	
	if (ucRightVol!=0)
		data = R16 | (1<<8) | (ucRightVol*3 + 162);
	else
		data = R16 | (1<<8);
	ThreeWire_Write_Data(data);
	

	return 0;
}
	
static void ThreeWire_Write_Data(UINT16 pg, UINT16 addr, UINT16 data)
{
	int i;
	UINT16	cmdword = 0;
	
	cmdword = pg<<11 | addr<<5; 
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SS);//SS high
	Delay(1);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SS);//SS low
	/*==== send command word ====*/	
	for (i=15;i>=0;i--){
		if  ((1<<i)&cmdword){//data is 1
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK | TI_MOSI);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK | TI_MOSI);//sclk high
			Delay(0x1);
			
		}	
		else{
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_MOSI & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_MOSI | TI_SCLK);//sclk high
			Delay(0x1);
		}
	}
	/*==== send data ====*/	
	for (i=15;i>=0;i--){
		if  ((1<<i)&data){//data is 1
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK | TI_MOSI);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK | TI_MOSI);//sclk high
			Delay(0x1);
			
		}	
		else{
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_MOSI & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_MOSI | TI_SCLK);//sclk high
			Delay(0x1);
		}
	}
	
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SS);//cs low
	Delay(1);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SS);//cs high, for latch
}

static UINT16 ThreeWire_Read_Data(UINT16 pg, UINT16 addr)
{
	int i;
	UINT16	cmdword = 0;
	UINT16	recvData = 0;
	
	cmdword = pg<<11 | addr<<5 | 1<<15; 
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SS);//SS high
	Delay(1);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SS);//SS low
	/*==== send command word ====*/	
	for (i=15;i>=0;i--){
		if  ((1<<i)&cmdword){//data is 1
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK | TI_MOSI);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK | TI_MOSI);//sclk high
			Delay(0x1);
			
		}	
		else{
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_MOSI & ~TI_SCLK);//sclk low
			Delay(0x1);
			outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_MOSI | TI_SCLK);//sclk high
			Delay(0x1);
		}
	}
	/*==== receive data ====*/	
	for (i=15;i>=0;i--){
		outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SCLK);//sclk low
		if  (inpw(REG_GPIO_DAT) & TI_MISO){//data is 1
			recvData = recvData | 1<<i;
		}	
		outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SCLK);//sclk high
	}
	
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~TI_SS);//SS low
	Delay(1);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | TI_SS);//SS high, for latch
	
	return recvData;
}


INT TIDAC26_DAC_Setup(){
	
	ThreeWire_Write_Data(1,0x4,0xBB00);//software reset
	nRetValue = TI_Set_DAC_Volume(0,0);//r11 r12 set volume
	if (nRetValue!=0)
		return nRetValue;
	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(R1 | 0x00D);//R1 MICBEN BIASEN BUFIOEN VMIDSEL	;
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}
	if (_gPowerUp)
		Delay(1000);
	else
		Delay(0x30000);
	
	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(R49 | 0x003);//R49 VROI
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}	

	if (_gPowerUp)
		Delay(1000);
	else
		Delay(0x30000);
	
	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(R2 | 0x180);//R2 power up ROUT1 LOUT1 
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}	
	
	if (_gPowerUp)
		Delay(1000);		
	else
		Delay(0x30000);

	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(R3 | 0x00F);//R3 RMIXEN LMIXEN DACENR DACENL
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}
	
	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(R50 | 0x0021 | (7<<6));//R50 left channel output mixer control (AUXL2LMIX, DACL2LMIX)
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}
	
	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(R51 | 0x0021 | (7<<6));//R51 right channel output mixer control (AUXR2RMIX, DACR2RMIX)
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}
	
	/***** Note :W99702 only support 16bits word length, some sampling rates will fail if word length>16bits ******/
	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(0x0810);//R4 select audio format(I2S format) and word length (16bits)
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}

	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(0x0A00);//R5 companding ctrl and loop back mode (all disable)
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}
	
	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(0x0C00);//R6 clock Gen ctrl(slave mode)
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}
	
	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(0x1000);//R8 GPIO stuff
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}
	
	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(0x1200);//R9 Jack detect control
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}
	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(0x1400);//R10 DAC control (softmute disable, oversample select 64x (lowest power) )
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}	
		
	
	
	/* 5-band graphic equaliser applied to ADC and DAC path, together with 3D enhance */
	ThreeWire_Write_Data(0x252C);//R18 EQ1
	ThreeWire_Write_Data(0x262C);//R19 EQ2
	ThreeWire_Write_Data(0x282C);//R20 EQ3
	ThreeWire_Write_Data(0x2A2C);//R21 EQ4
	ThreeWire_Write_Data(0x2C2C);//R22 EQ5
	
	/* DAC output limiter */
	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(0x3032);//R24 DAC limiter 1
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}
	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(0x3200);//R25 DAC limiter 2
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}
	
	
	
	
	ThreeWire_Write_Data(0x5200);//R41 3D depth
	
	for (nTimeOutCount=100;nTimeOutCount>=0;nTimeOutCount--){
		nRetValue = ThreeWire_Write_Data(0x6202);//R49 output mixer control
		if (nRetValue == 0)
			break;
	}
	if (nTimeOutCount == 0){
		return nRetValue;
	}
	//ThreeWire_Write_Data(0x7001);//R56 out3 mixer control (LDAC2OUT3)
	//ThreeWire_Write_Data(0x7201);//R57 out4 mixer control (RDAC2OUT4)
	_gPowerUp = 1;

	TI_Set_Sample_Frequency(_tIIS.nPlaySamplingRate);//R7 set sampling rate
	
	return 0;

}
void TIDAC26_ADC_Setup(){
	
	//ThreeWire_Write_Data(0x0000);//R0 reset
	ThreeWire_Write_Data(R1 | 0x01D);//VMID=75K om,BIASEN,MICBEN
	ThreeWire_Write_Data(R2 | 0x3F);//BOOSTENL,BOOSTENR,INPGAENR,INPGAENL,ADCENR,ADCENL


	ThreeWire_Write_Data(R4 | 0x010);//audio interface: I2S format,16bits word length
	ThreeWire_Write_Data(R6| 0x000);//R6 clock Gen ctrl(slave mode)
	/* for line in, output check*/
	//ThreeWire_Write_Data(R3 | 0x00C);//RMIXEN,LMIXEN
	//ThreeWire_Write_Data(R50 | (1<<5) | (7<<6));//R50 left channel output mixer control (AUXL2LMIX) 
	//ThreeWire_Write_Data(R51 | (1<<5) | (7<<6));//R51 right channel output mixer control (AUXR2RMIX)
	ThreeWire_Write_Data(R52 | 0x38 | (3<<7));//left head phone vol
	ThreeWire_Write_Data(R53 | 0x38 | (3<<7));//right head phone vol
	TI_Set_Sample_Frequency(_tIIS.nRecSamplingRate);//R7 set sampling rate
	/* for line in */
	

	ThreeWire_Write_Data(R14 | 0x100);//
	TI_Set_ADC_Volume(31,31);//r15 r16 set record volume
	
	/* ALC control */
	ThreeWire_Write_Data(R32 |  0x038);//ALC enable -12dB~+35.25dB
	ThreeWire_Write_Data(R33 |  0x00B);//
	ThreeWire_Write_Data(R34 |  0x032);
	/* ALC control */
	
	ThreeWire_Write_Data(R35 |  0x008);//disable noise gate
	


	ThreeWire_Write_Data(R44 |  0x033);//input control LIN2INPPGA, LIP2INPPGA, RIN2INPPGA, RIP2INPPGA, MICBV=0.9*AVDD
	/* PGA gain volume */
	ThreeWire_Write_Data(R45 |  0x13f);
	ThreeWire_Write_Data(R46 |  0x13f);
	
	ThreeWire_Write_Data(R47 | (1<<8) | 0x7);
	ThreeWire_Write_Data(R48 | (1<<8) | 0x7);
	ThreeWire_Write_Data(R49 |  0x002);
	
	_gPowerUp = 0;
	
}   
	
static void TI_Set_Sample_Frequency(INT uSample_Rate)
{   
	UINT16 data=0;
	switch (uSample_Rate)
	{
		case AU_SAMPLE_RATE_8000:							//8KHz
			data = R7 | TI_8000 ;
			break;
		case AU_SAMPLE_RATE_11025:					//11.025KHz
			data = R7 | TI_11025 ;
    
			break;
		case AU_SAMPLE_RATE_16000:						//16KHz
			data = R7 | TI_16000 ;
    
			break;
		case AU_SAMPLE_RATE_22050:					//22.05KHz
			data = R7 | TI_22050;
    
			break;
		case AU_SAMPLE_RATE_24000:						//24KHz
			data = R7 | TI_24000;

			break;
		case AU_SAMPLE_RATE_32000:						//32KHz
			data = R7 | TI_32000 ;

			break;
		case AU_SAMPLE_RATE_44100:					//44.1KHz
			data = R7 | TI_44100 ;

			break;
		case AU_SAMPLE_RATE_48000:						//48KHz
			data = R7 | TI_48000;

			break;
		default:break;
	}
	ThreeWire_Write_Data(data);
}
/*---------- end of TIDAC26's functions group ----------*/

#endif	/* HAVE_IIS */
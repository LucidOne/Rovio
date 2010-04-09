/****************************************************************************
 * 
 * FILENAME
 *     W99702_MA5SI.c
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
 *     2005.01.07		Created by Shih-Jen Lu
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
#include "YMU783.h"



#ifdef HAVE_MA5SI



static AUDIO_T	_tIIS;

#define	IIS_ACTIVE				0x1
#define	IIS_PLAY_ACTIVE			0x2
#define IIS_REC_ACTIVE			0x4

static BOOL	 _bIISActive = 0;
static BOOL	_bChangeVolume = 0;
static UINT32 _uIISCR = 0;


static VOID IIS_Set_Data_Format(INT);
static VOID IIS_Set_Sample_Frequency(INT);
VOID MA5Si_DAC_Setup(VOID);
VOID MA5Si_DAC_Stop(VOID);
static UINT32 HWInitialize(VOID);
static VOID InitRegisters(VOID);
static INT32 machdep_PowerManagement(UINT8);
static INT32 MaDevDrv_VerifyRegisters(VOID);

static void Delay(int nCnt)
{
	volatile int  loop;
	for (loop=0; loop<nCnt; loop++);
}
static INT read_intermediate_reg(INT nRegId)
{
	INT nTimeOutCount;
	outpw(REG_ACTL_M80ADDR, nRegId);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | R_IF11_ACT);

	for(nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
		if ((inpw(REG_ACTL_M80CON) & R_IF11_ACT) == 0 )
			break;
	}
	
	if (nTimeOutCount == 10000){
		_error_msg("read_intermediate_reg - M80 read intermediate register timeout\n");
		return ERR_M80_R_INTERMEDIATEREG_TIMEOUT;
	}
	return ((inpw(REG_ACTL_M80SIZE)>>16) & 0xff);
}


static INT write_intermediate_reg(INT nRegId, UINT8 data)
{
	INT nTimeOutCount;
	outpw(REG_ACTL_M80ADDR, nRegId);
	outpw(REG_ACTL_M80SIZE, 0x01);
	outpw(REG_ACTL_M80DATA0, data);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | W_IF12_ACT);
	for(nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
		if ((inpw(REG_ACTL_M80CON) & W_IF12_ACT) == 0 )
			break;
	}
	
	if (nTimeOutCount == 10000){
		_error_msg("write_intermediate_reg - M80 write intermediate register timeout\n");
		return ERR_M80_W_INTERMEDIATEREG_TIMEOUT;
	}
	return 0;
}

static INT read_control_reg(INT nRegNum)
{
	UINT32	 uData32;
	INT nTimeOutCount;
	if (nRegNum < 0x80)
		uData32 = (0x80|nRegNum) << 8;
	else
		uData32 = ((nRegNum & 0x7f) << 8) |
				 (((nRegNum >> 7) & 0x7f) | 0x80) << 16;
	uData32 |= 0x03;					/* state flag */
	outpw(REG_ACTL_M80ADDR, uData32);

	outpw(REG_ACTL_M80DATA0, 0x80);		/* select read buffer #0 */

	outpw(REG_ACTL_M80SIZE, 0x1);      	/* read buffer ID = 0 */

	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | R_IF10_ACT);

	for(nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
		if ((inpw(REG_ACTL_M80CON) & R_IF10_ACT) == 0 )
			break;
	}
	
	if (nTimeOutCount == 10000){
			_error_msg("read_control_reg - M80 read control register timeout\n");
			return ERR_M80_R_CONTROLREG_TIMEOUT;
	}
		
	return ((inpw(REG_ACTL_M80SIZE)>>16) & 0xff);
}


#ifdef ECOS
cyg_uint32  iis_play_isr(cyg_vector_t vector, cyg_addrword_t data)
#else
static void  iis_play_isr()
#endif
{
	outpw(REG_ACTL_CON, inpw(REG_ACTL_CON) | T_DMA_IRQ);
	
	if (_bChangeVolume)
	{
		write_intermediate_reg(MAI_BASIC_SETTING,0x03);
		write_intermediate_reg(MAI3_DA_LVOL,_tIIS.sPlayVolume >> 8);
		write_intermediate_reg(MAI3_DA_RVOL,_tIIS.sPlayVolume & 0xff);
		write_intermediate_reg(MAI_BASIC_SETTING,0x00);
		
		write_intermediate_reg( MAI_ANALOG_HPVOL_L, _tIIS.sPlayVolume >> 8 );
		write_intermediate_reg( MAI_ANALOG_HPVOL_R, _tIIS.sPlayVolume & 0xff);
		write_intermediate_reg( MAI_ANALOG_SPVOL, read_intermediate_reg( MAI_ANALOG_SPVOL) | _tIIS.sPlayVolume & 0xff);
			
	
		_bChangeVolume = 0;
	}
	
	
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

static INT  ma5si_init()
{

	_debug_msg("Reset MA5SI..\n");
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~0x100000);	/* GPIO 20 */
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x100000);
	Delay(100);
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~0x100000);
	Delay(100);
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x100000);
	_debug_msg("complet!\n");
	

	
	
	/* reset Audio Controller */
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | ACTL_RESET_BIT);
	Delay(100);
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~ACTL_RESET_BIT);
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
	outpw(REG_ACTL_CON, inpw (REG_ACTL_CON) | AUDIO_EN | AUDCLK_EN | M80_EN | IIS_EN | PFIFO_EN | T_DMA_IRQ | R_DMA_IRQ | DMA_EN);
	if (!_bADDA_Active)
	{
		outpw(REG_CLKSEL,inpw(REG_CLKSEL) & ~(3<<4) | (2<<4));//ACLK use APLL
	}
	
	/* reset M80 interface */
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | M80_RESET);
	Delay(100);
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~M80_RESET);
	Delay(100);

	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL);
	outpw(REG_ACTL_PDST_LENGTH, 256);
	outpw(REG_ACTL_M80CON, CLK_DIV);
	
	HWInitialize();
	
	/* set volume */
	write_intermediate_reg(MAI_BASIC_SETTING,0x00);
	
	write_intermediate_reg( MAI_ANALOG_EQVOL, 0x1f);
	write_intermediate_reg( MAI_ANALOG_HPVOL_L, 0x10);
	write_intermediate_reg( MAI_ANALOG_HPVOL_R, 0x10);
	write_intermediate_reg( MAI_ANALOG_SPVOL, read_intermediate_reg( MAI_ANALOG_SPVOL) | 0x1f | (1<<6));
		
	
	/* initialize registers */
	//write_intermediate_reg( MAI_EXT_CTRL, 0xBC );
	

	/* Initialize API Balance */
	//write_intermediate_reg( MAI_MASTER_VOLL, ((15 << 4) & 0xF8) | 2);
	//write_intermediate_reg( MAI_MASTER_VOLR, ((15 << 4) & 0xF8) | 2);
#if 0	
	write_intermediate_reg(12,6);
	write_intermediate_reg(11,1);
	Delay(0x100000);
	write_intermediate_reg(11,0);
	Delay(0x100000);
	write_intermediate_reg(11,1);
	Delay(0x100000);
	write_intermediate_reg(11,0);
	Delay(0x100000);
	write_intermediate_reg(11,1);
	Delay(0x100000);
	write_intermediate_reg(11,0);
#endif	
	
	_uIISCR = 0;
	return 0;
}






/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      ma5siStartPlay                                                   */
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
INT  ma5siStartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
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
		nStatus = ma5si_init();
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

	
	//MA5Si DAC set up
	MA5Si_DAC_Setup();
	
	


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
/*      ma5siStopPlay                                                    */
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
INT  ma5siStopPlay()
{

	INT nRetValue;
	MA5Si_DAC_Stop();
	nRetValue = machdep_PowerManagement(PW_DOWN);
	if (nRetValue != 0){
		return nRetValue;
	}
	
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
/*      ma5siSetPlayVolume                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Set ma5si left and right channel play volume.                      */
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
INT  ma5siSetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol)  //0~31
{
	_tIIS.sPlayVolume = (ucLeftVol << 8) | ucRightVol;
	_bChangeVolume = 1;
	return 0;
}
void MA5Si_Set_Sample_Frequency()
{
	UINT8 Src_Rate_L = 0;
	UINT8 Src_Rate_H = 0;
	
	switch (_tIIS.nPlaySamplingRate)
	{
		case AU_SAMPLE_RATE_8000:							//8KHz
			Src_Rate_L = 0x1555%0xFF;
			Src_Rate_H = 0x1555/0xFF;
			break;
		case AU_SAMPLE_RATE_11025:					//11.025KHz
			Src_Rate_L = 0x1D55%0xFF;
			Src_Rate_H = 0x1D55/0xFF;
			break;
		case AU_SAMPLE_RATE_16000:						//16KHz
			Src_Rate_L = 0x2AAA%0xFF;
			Src_Rate_H = 0x2AAA/0xFF;
			break;
		case AU_SAMPLE_RATE_22050:					//22.05KHz
			Src_Rate_L = 0x3A00%0xFF;
			Src_Rate_H = 0x3A00/0xFF;
			break;
		case AU_SAMPLE_RATE_24000:						//24KHz
			Src_Rate_L = 0x4000%0xFF;
			Src_Rate_H = 0x4000/0xFF;
			break;
		case AU_SAMPLE_RATE_32000:						//32KHz
			Src_Rate_L = 0x5555%0xFF;
			Src_Rate_H = 0x5555/0xFF;
			break;
		case AU_SAMPLE_RATE_44100:					//44.1KHz
			Src_Rate_L = 0x74E0%0xFF;
			Src_Rate_H = 0x74E0/0xFF;
			break;
		case AU_SAMPLE_RATE_48000:						//48KHz
			Src_Rate_L = 0x7E00%0xFF;
			Src_Rate_H = 0x7E00/0xFF;
			break;
		default:break;
	}
	/* set BANK bits of REG_ID #4 basic setting register to '4' */
	write_intermediate_reg(MAI_BASIC_SETTING,0x04);
	write_intermediate_reg(22,Src_Rate_H);
	write_intermediate_reg(23,Src_Rate_L);
	
	/* set BANK bits of REG_ID #4 basic setting register to '0' */
	write_intermediate_reg(MAI_BASIC_SETTING,0x00);
	
}

	

VOID MA5Si_DAC_Setup()
{
	


	/* set slave mode */
	write_intermediate_reg(MAI_BASIC_SETTING,0x03);
	write_intermediate_reg(MAI3_DAC_CTRL1, 0x00);

	MA5Si_Set_Sample_Frequency();
	
	/* set BANK bits of REG_ID #4 basic setting register to '3' */
	write_intermediate_reg(MAI_BASIC_SETTING,0x03);
	
	/* set audio data format */
	write_intermediate_reg(MAI3_DAC_CTRL2, MAB_DACCTRL_IIS);
			
	
	/* slave mode */
	/* set DPA0 = 0 */
	write_intermediate_reg(MAI3_POWER_MANAGEMENT_DIGITAL_AUDIO,	MAB_PWM_DPA2 | MAB_PWM_DPA1);
	
	
	write_intermediate_reg(MAI3_DAVOL, 0x0f);
	/* set the mixer */
	/* TXOUT */
	write_intermediate_reg(MAI3_TXOUT_MIXER,MAB_MIXER_TX_RXMIX);//TXOUT from RXIN
	
	
	
	/* HPOUT */
	write_intermediate_reg(MAI3_HPOUT_MIXER, (MAB_MIXER_HR_DAMIX | MAB_MIXER_HL_DAMIX));//HPOUT from DAC
	
	
	/* SPOUT */
	write_intermediate_reg(MAI3_SPOUT_MIXER,MAB_MIXER_SP_DAMIX);//SPOUT from DAC

	/* set volume for Digital Audio */
	//ma5siSetPlayVolume(_tIIS.sPlayVolume >> 8,_tIIS.sPlayVolume & 0xff);
	write_intermediate_reg(MAI3_DA_LVOL,0x10);
	write_intermediate_reg(MAI3_DA_RVOL,0x10);
	
	write_intermediate_reg(35,3);//bass
	write_intermediate_reg(33,0x1E);//bank #3 TXOUT VOL
	write_intermediate_reg(31,0x1E);//bank #3 RXIN VOL
	
}
VOID MA5Si_DAC_Stop()
{
	/* OFF */
	UINT8 bRegData;
	
	write_intermediate_reg(MAI_BASIC_SETTING,0x03);
	write_intermediate_reg(MAI3_DAVOL, 0x00);
	
	/* set DPA1 to 1 */
	bRegData = read_intermediate_reg(MAI3_POWER_MANAGEMENT_DIGITAL_AUDIO);
	write_intermediate_reg(MAI3_POWER_MANAGEMENT_DIGITAL_AUDIO,bRegData | MAB_PWM_DPA1);
	
	/* set DPA0 to 1 */
	bRegData = read_intermediate_reg(MAI3_POWER_MANAGEMENT_DIGITAL_AUDIO);
	write_intermediate_reg(MAI3_POWER_MANAGEMENT_DIGITAL_AUDIO,bRegData | MAB_PWM_DPA0);
	
	
																
	/* set BANK bits of REG_ID #4 basic setting register to '0' */
	write_intermediate_reg(MAI_BASIC_SETTING,0x00);
	
}

static UINT32 HWInitialize()
{
	UINT32	uResult;					/* result of function */

	_debug_msg("Hardware Initialization\n");


	/* Initialize the uninitialized registers by software reset */
	InitRegisters();

	/* Set the PLL. */
	write_intermediate_reg( MAI_PLL_SETTING_1, MA_ADJUST1_VALUE  | 0x80);
	write_intermediate_reg( MAI_PLL_SETTING_2, MA_ADJUST2_VALUE );
	
	//_debug_msg("PLL 1: %d\n",read_intermediate_reg(MAI_PLL_SETTING_1));
	//_debug_msg("PLL 2: %d\n",read_intermediate_reg(MAI_PLL_SETTING_2));

	/* Disable power down mode. */
	uResult = machdep_PowerManagement( HW_INIT );

	/* error  */
	if ( uResult != 0 )
	{
		return uResult;
	}


	write_intermediate_reg( MAI_IRQ_CONTROL_1, 
					MAB_IRQCTRL_ETM2 | MAB_IRQCTRL_ESIRQ1 | MAB_IRQCTRL_ESIRQ0  );

	write_intermediate_reg( MAI_IRQ_CONTROL_2, MAB_IRQCTRL_ESIRQ4 | MAB_IRQCTRL_ESIRQ3 | MAB_IRQCTRL_ESIRQ2  );

	/* set stream buffer size and irq point */
	/* buff_size=1024, irq_point=1/4 */
	write_intermediate_reg( MAI_STM_IRQ_CTRL, 0x10 );

	return 0;
}

static VOID InitRegisters( )
{
	UINT32	dCount;

	static const UINT8 bIndex[10] = { 7, 8, 9, 10, 11, 12, 13, 14, 15, 63 };

	static const UINT8 bIndex3[23] = { 27, 28, 29, 30, 31, 32, 33, 34, 35, 
	                                  21, 22, 23, 24, 25, 26, 
	                                  10, 11, 12, 18, 19, 20, 16, 17 };
	static const UINT8 bDefault3[23] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	                                     0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 
	                                     0x00, 0x00, 0x0F, 0x03, 0x60, 0x82, 0x87, 0x1F };


	_debug_msg("InitRegisters:\n");

	/* set BANK bits of REG_ID #4 basic setting register to '0' */

	//write_intermediate_reg(MAI_BASIC_SETTING,0x00);
	//_debug_msg("reg 4 = %d\n",	read_intermediate_reg(MAI_BASIC_SETTING));

	for ( dCount = 0; dCount < 10; dCount++ )
	{
		/* set register index to REG_ID */
		write_intermediate_reg(bIndex[dCount],0);
	}
#if 0 
	for ( dCount = 0; dCount < 10; dCount++ )
	{
		/* set register index to REG_ID */
		_debug_msg("reg %d : %d\n",bIndex[dCount],read_intermediate_reg(bIndex[dCount]));
	}
#endif		
	/* set BANK bits of REG_ID #4 basic setting register to '3' */
	
	write_intermediate_reg(MAI_BASIC_SETTING,0x03);
	
	for ( dCount = 0; dCount < 23; dCount++ )
	{
		write_intermediate_reg( bIndex3[dCount], bDefault3[dCount]);
	}

	/* set BANK bits of REG_ID #4 basic setting register to '0' */
	write_intermediate_reg( MAI_BASIC_SETTING, 0x00 );
	
}

static INT32 machdep_PowerManagement( UINT8 bMode )
{
	INT32	sdResult;
	UINT32	dCount;
	UINT8	bRegData;


	sdResult= 0;

	switch ( bMode )
	{
	case HW_INIT:

		/* sequence of hardware initialize when normal */

		/* set BANK bits of REG_ID #4 basic setting register to '0' */
		write_intermediate_reg( MAI_BASIC_SETTING ,0x00);
		

		/* 4:set DP0 bit of REG_ID #5 power management (D) setting register to '0' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_DIGITAL, MAB_PWM_DP5 | MAB_PWM_DP4 | MAB_PWM_DP2 | MAB_PWM_DP1);
		

		/* wait 2ms */
		Delay( 2 * 1000 );

		/* 5:set PLLPD and AP0 bits of REG_ID #6 power management (A) setting register to '0' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_ANALOG, MAB_PWM_AP4 | MAB_PWM_AP3 |
														MAB_PWM_AP2 | MAB_PWM_AP1);
		

		/* wait 10ms */
		Delay( 10 * 1000 );


		/* check VREF_RDY bit of Bank #3 REG_ID #37 */
		
		/* 6:set BANK bits of REG_ID #4 basic setting register to '3' */
		write_intermediate_reg( MAI_BASIC_SETTING,0x03);
		for ( dCount = 0; dCount < MA_RESET_RETRYCOUNT; dCount++ )
		{	
			//_debug_msg("check VREF_RDY\n");	
			/* set specified reg# */
			bRegData = read_intermediate_reg( MAI3_ANALOG_STATUS);
			if ( bRegData & MAB_VREF_RDY )	break;
			Delay( 10000 );
		}
		
		/* error handler */
		if ( dCount >= MA_RESET_RETRYCOUNT )
		{
			_debug_msg("VREF_RDY error\n");
			return ERR_MA5SI_VREF_RDY;
			/* continue process for power management */
		}
		
		/* set BANK bits of REG_ID #4 basic setting register to '0' */
		write_intermediate_reg( MAI_BASIC_SETTING,0x00);
		



		/* set DP1 bit of REG_ID #5 power management (D) setting register to '0' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_DIGITAL, MAB_PWM_DP5 | MAB_PWM_DP4 | MAB_PWM_DP2);
		

		/* 7:set DP2 bit of REG_ID #5 power management (D) setting register to '0' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_DIGITAL, MAB_PWM_DP5 | MAB_PWM_DP4 );
		
		/* 7:set DP4 bit of REG_ID #5 power management (D) setting register to '0' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_DIGITAL, MAB_PWM_DP5);
		

		for ( dCount = 0; dCount < MA_RESET_RETRYCOUNT; dCount++ )
		{
			/* 8:set RST bit of REG_ID #4 basic setting register to '1' */
			write_intermediate_reg( MAI_BASIC_SETTING,MAB_RST);
			
			/* set RST bit of REG_ID #4 basic setting register to '0' */
			write_intermediate_reg( MAI_BASIC_SETTING,0x00);
						
			Delay( 41700 );


			/* verify the initialized registers by software reset */
			sdResult = MaDevDrv_VerifyRegisters();

			if ( sdResult == 0 ) break;
		}

		/* error handler */
		if ( dCount >= MA_RESET_RETRYCOUNT )
		{
			_debug_msg("software reset error\n");
			return ERR_MA5SI_SOFTRESET;
			/* continue process for power management */
		}

		/* 12:set DP5 bit of REG_ID #5 power management (D) setting register to '0' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_DIGITAL,0x00 );
		

		/* MA-5Si */

		/* MA-5si core */
		
		/* step 13:As necessary, set AP* bit of REG_ID #6 power management (A) setting register to '0' */
		/* set AP0 to 0 */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_ANALOG, MAB_PWM_AP4 |MAB_PWM_AP3| MAB_PWM_AP2 | MAB_PWM_AP1 );
		/* bank 3 */
		write_intermediate_reg( MAI_BASIC_SETTING,0x03);
		write_intermediate_reg( MAI_BASIC_SETTING,0x03);
		for ( dCount = 0; dCount < MA_RESET_RETRYCOUNT; dCount++ )
		{	
			_debug_msg("check VREF_RDY\n");	
			/* set specified reg# */
			bRegData = read_intermediate_reg( MAI3_ANALOG_STATUS);
			if ( bRegData & MAB_VREF_RDY )	break;
			Delay( 10000 );
		}
		
		/* error handler */
		if ( dCount >= MA_RESET_RETRYCOUNT )
		{
			_debug_msg("VREF_RDY error\n");
			return ERR_MA5SI_VREF_RDY;
			/* continue process for power management */
		}
		
		
		write_intermediate_reg( MAI_BASIC_SETTING,0x03);//Bank #3
		/* TXOUT */ 
		/* set AP6 bits of REG_ID #17 power management (A) setting register to '0' */
		write_intermediate_reg( MAI3_POWER_MANAGEMENT_ANALOG, MAB_PWM_AP9 | MAB_PWM_AP8 
																		| MAB_PWM_AP5 );
		
		/* RXIN */																
		/* set AP9	bits of REG_ID #17 power management (A) setting register to '0' */
		write_intermediate_reg( MAI3_POWER_MANAGEMENT_ANALOG, MAB_PWM_AP8 | MAB_PWM_AP5 );
		Delay(1000);
		write_intermediate_reg( 31, 0x0F);
		
		write_intermediate_reg( MAI_BASIC_SETTING,0x00);//Bank #0
		/* Digital audiio section */
		/* set AP3 bits of REG_ID #6 power management (A) setting register to '0' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_ANALOG, MAB_PWM_AP4 | MAB_PWM_AP2 | MAB_PWM_AP1);
				
		/* SPOUT */
		/* set AP1 bits of REG_ID #6 power management (A) setting register to '0' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_ANALOG, MAB_PWM_AP4 | MAB_PWM_AP2 );
		
		/* wait 10us */
		Delay( 10 * 1000 );

		/* set AP2 bit of REG_ID #6 power management (A) setting register to '0' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_ANALOG , MAB_PWM_AP4);
		//write_intermediate_reg( MAI_POWER_MANAGEMENT_ANALOG , MAB_PWM_AP4L | MAB_PWM_AP3);

		/* HPOUT */
		/* set MONO bit of REG_ID #8 analog setting register(the minimum volume) */
		write_intermediate_reg( MAI_ANALOG_HPVOL_L, 0x00);
		
		
		/* set AP4 bit of REG_ID #6 power management (A) setting register to '0' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_ANALOG , 0x00);
		//write_intermediate_reg( MAI_POWER_MANAGEMENT_ANALOG , MAB_PWM_AP3);
		
		/* sleep 200ms */
		Delay( 200*1000 );

		/* check HP_RDY bits of Bank #3 REG_ID #37 */
		/* set BANK bits of REG_ID #4 basic setting register to '3' */
		write_intermediate_reg( MAI_BASIC_SETTING,0x03);
		for ( dCount = 0; dCount < MA_RESET_RETRYCOUNT; dCount++ )
		{	
			_debug_msg("check HP_RDY\n");	
			/* set specified reg# */
			bRegData = read_intermediate_reg( MAI3_ANALOG_STATUS);
			if ( bRegData & MAB_HP_RDY )	break;
			Delay( 10000 );
		}
		
		/* error handler */
		if ( dCount >= MA_RESET_RETRYCOUNT )
		{
			_debug_msg("HP_RDY error\n");
			return ERR_MA5SI_HP_RDY;
			/* continue process for power management */
		}
		
		/* set BANK bits of REG_ID #4 basic setting register to '0' */
		write_intermediate_reg( MAI_BASIC_SETTING , 0x00);
		


		/* normal */

		break;

	case PW_DOWN:

		/* sequence of power down (analog section)*/
		/* SPOUT off */
		/* set BANK bits of REG_ID #4 basic setting register to '0' */
		write_intermediate_reg( MAI_BASIC_SETTING , 0x00);
		write_intermediate_reg(MAI_ANALOG_EQVOL,0x00);
		write_intermediate_reg(MAI_ANALOG_HPVOL_L,0x00);// HPOUT MUTE
		write_intermediate_reg(MAI_ANALOG_HPVOL_R,0x00);
		write_intermediate_reg(MAI_ANALOG_SPVOL,0x00);// SPOUT MUTE
		
		
		/* set AP1~4 bits of REG_ID #6 power management (A) setting register to '1' */
		bRegData = read_intermediate_reg(MAI_POWER_MANAGEMENT_ANALOG);
		write_intermediate_reg( MAI_POWER_MANAGEMENT_ANALOG, bRegData | MAB_PWM_AP4 | MAB_PWM_AP3 
									| MAB_PWM_AP2 | MAB_PWM_AP1);
		
		
		/* sleep 250ms */
		Delay( 250*1000 );
		
		
		/* check HP_STBY bits of Bank #3 REG_ID #37 */
		/* set BANK bits of REG_ID #4 basic setting register to '3' */
		write_intermediate_reg( MAI_BASIC_SETTING, 0x03 );
		for ( dCount = 0; dCount < MA_RESET_RETRYCOUNT; dCount++ )
		{	
			_debug_msg("check HP_STBY\n");	
			/* set specified reg# */
			bRegData = read_intermediate_reg( MAI3_ANALOG_STATUS);
			if ( bRegData & MAB_HP_STBY )	break;
			Delay( 10000 );
		}
		
		/* error handler */
		if ( dCount >= MA_RESET_RETRYCOUNT )
		{
			_debug_msg("HP_RDY error\n");
			return ERR_MA5SI_HP_STBY;
			/* continue process for power management */
		}
		
		
		write_intermediate_reg(MAI3_SPOUT_MIXER,0x00);
		write_intermediate_reg(MAI3_HPOUT_MIXER,0x00);
		
		
		/* set AP0 and PLLPD bits of REG_ID #6 power management (A) setting register to '1' */
		/* set BANK bits of REG_ID #4 basic setting register to '0' */
		write_intermediate_reg( MAI_BASIC_SETTING,0x00);
		bRegData = read_intermediate_reg(MAI_POWER_MANAGEMENT_ANALOG);
		write_intermediate_reg( MAI_POWER_MANAGEMENT_ANALOG, bRegData | MAB_PWM_PLLPD | MAB_PWM_AP0);
		break;

	default:
		sdResult = ERR_MA5SI_ERROR;
		break;
	}

	return sdResult;
}

static INT32 MaDevDrv_VerifyRegisters( void )
{
	INT32	sdResult;
	UINT32	i;

	sdResult = 0;

	for( i=192 ; i<192+16 ; i++ )
	{
		sdResult = read_control_reg( i );
		if( (sdResult&0xff)!=0x60 ) return ERR_MA5SI_SOFTRESET;
	}
	for( i=208 ; i<208+16 ; i++ )
	{
		sdResult = read_control_reg( i );
		if( (sdResult&0xff)!=0x3c ) return ERR_MA5SI_SOFTRESET;
	}
	return 0;
}


#endif	/* HAVE_MA5I */
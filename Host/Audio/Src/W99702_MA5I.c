/****************************************************************************
 * 
 * FILENAME
 *     W99702_MA5I.c
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
#include "YMU782.h"



#ifdef HAVE_MA5I



static AUDIO_T	_tIIS;

#define	IIS_ACTIVE				0x1
#define	IIS_PLAY_ACTIVE			0x2
#define IIS_REC_ACTIVE			0x4

static BOOL	 _bIISActive = 0;
static BOOL	_bChangeVolume = 0;
static UINT32 _uIISCR = 0;


static VOID IIS_Set_Data_Format(INT);
static VOID IIS_Set_Sample_Frequency(INT);
INT MA5i_DAC_Setup(VOID);
VOID MA5i_DAC_Stop(VOID);
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
		write_intermediate_reg( MAI_ANALOG_HPVOL_L, _tIIS.sPlayVolume >> 8);
		write_intermediate_reg( MAI_ANALOG_HPVOL_R, _tIIS.sPlayVolume & 0xff);
		write_intermediate_reg( MAI_ANALOG_SPVOL, read_intermediate_reg( MAI_ANALOG_SPVOL) | _tIIS.sPlayVolume & 0xff);
		write_intermediate_reg(MAI_BASIC_SETTING,0x00);
		_bChangeVolume = 0;
	}
	
	
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

static INT  ma5i_init()
{

	
	_debug_msg("Reset MA5I..\n");
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~0x100000);	/* GPIO 20 */
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x100000);
	Delay(100);
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~0x100000);
	Delay(100);
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x100000);
	_debug_msg("reset complet!\n");
	
	
	
	/* reset Audio Controller */
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

	outpw(REG_ACTL_M80CON, CLK_DIV);
	
	HWInitialize();
	
	/* set volume */
	write_intermediate_reg(MAI_BASIC_SETTING,0x00);
	
	write_intermediate_reg( MAI_ANALOG_EQVOL, 0x1f);
	write_intermediate_reg( MAI_ANALOG_HPVOL_L, 0x10);
	write_intermediate_reg( MAI_ANALOG_HPVOL_R, 0x10);
	write_intermediate_reg( MAI_ANALOG_SPVOL, read_intermediate_reg( MAI_ANALOG_SPVOL) | 0x1f | (1<<6));
	write_intermediate_reg( 41, (0x1E<<3) | 2);
	
	
	
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
/*      ma5iStartPlay                                                   */
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
INT  ma5iStartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
								INT nChannels, INT data_format)
{
	INT		nStatus;
	INT		nRetValue;
	
	if (_bIISActive & IIS_PLAY_ACTIVE)
	
		return ERR_IIS_PLAY_ACTIVE;		/* IIS was playing */
	
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~(PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL));
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | PLAY_LEFT_CHNNEL);
	if (nChannels != 1)
		outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | PLAY_RIGHT_CHNNEL);
	
	if (_bIISActive == 0)
	{
		nStatus = ma5i_init();
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

	
	//MA5i DAC set up
	nRetValue = MA5i_DAC_Setup();
	if (nRetValue != 0)
		return nRetValue;
	
	//_debug_msg ("HP L VOL %d\n",read_intermediate_reg( MAI_ANALOG_HPVOL_L));
	//_debug_msg ("HP R VOL %d\n",read_intermediate_reg( MAI_ANALOG_HPVOL_R));


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
	
	
	
	write_intermediate_reg(MAI_BASIC_SETTING,0x03);
	write_intermediate_reg(MAI_BASIC_SETTING,0x00);
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      ma5iStopPlay                                                    */
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
INT  ma5iStopPlay()
{


	MA5i_DAC_Stop();
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
				_uIISCR = _uIISCR | FS_384 | SCALE_4 | BCLK_32;//APLL=12.288Mhz
				break;
			case AU_SAMPLE_RATE_11025:					//11.025KHz
				_uIISCR = _uIISCR | FS_256 | SCALE_4 | BCLK_48;//APLL=16.934Mhz
				break;
			case AU_SAMPLE_RATE_12000:					//12KHz
				_uIISCR = _uIISCR | FS_256 | SCALE_4 | BCLK_32;
				break;	
			case AU_SAMPLE_RATE_16000:						//16KHz
				_uIISCR = _uIISCR | FS_256 | SCALE_2 | BCLK_48;
				break;
			case AU_SAMPLE_RATE_22050:					//22.05KHz
				_uIISCR = _uIISCR | FS_256 | SCALE_2 | BCLK_48;
				break;
			case AU_SAMPLE_RATE_24000:						//24KHz
				_uIISCR = _uIISCR | FS_256 | SCALE_2 | BCLK_32;
				break;
			case AU_SAMPLE_RATE_32000:						//32KHz
				_uIISCR = _uIISCR | SCALE_1 | FS_256  | BCLK_48;
				break;
			case AU_SAMPLE_RATE_44100:					//44.1KHz
				_uIISCR = _uIISCR | SCALE_1 | FS_256  | BCLK_48;
				break;
			case AU_SAMPLE_RATE_48000:						//48KHz
				_uIISCR = _uIISCR | FS_256 | SCALE_1 | BCLK_32 ;
				break;
			default:break;
		}
	outpw(REG_ACTL_IISCON,_uIISCR);
}
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      ma5iSetPlayVolume                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Set ma5i left and right channel play volume.                      */
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
INT  ma5iSetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol)  //0~31
{
	_tIIS.sPlayVolume = (ucLeftVol << 8) | ucRightVol;
	_bChangeVolume = 1;
	return 0;
}
void MA5i_Set_Sample_Frequency()
{
	unsigned char reg21data = 0;
	reg21data = MAB_DACCTRL_BCLK32;
	switch (_tIIS.nPlaySamplingRate)
	{
		case AU_SAMPLE_RATE_8000:							//8KHz
			reg21data = MAB_DACCTRL_BCLK48;//BCLK "ONLY" can be 48 times fs when the sampling rate is bellow 11.025kMhz
			reg21data = reg21data | MAB_DACCTRL_44100;
			break;
		case AU_SAMPLE_RATE_11025:					//11.025KHz
			reg21data = reg21data | MAB_DACCTRL_11025;

			break;
		case AU_SAMPLE_RATE_12000:
			reg21data = reg21data | MAB_DACCTRL_12000;
			break;
		case AU_SAMPLE_RATE_16000:						//16KHz
			reg21data = reg21data | MAB_DACCTRL_16000;

			break;
		case AU_SAMPLE_RATE_22050:					//22.05KHz
			reg21data = reg21data | MAB_DACCTRL_22050;

			break;
		case AU_SAMPLE_RATE_24000:						//24KHz
			reg21data = reg21data | MAB_DACCTRL_24000;

			break;
		case AU_SAMPLE_RATE_32000:						//32KHz
			reg21data = reg21data | MAB_DACCTRL_32000;

			break;
		case AU_SAMPLE_RATE_44100:					//44.1KHz
			reg21data = reg21data | MAB_DACCTRL_44100;

			break;
		case AU_SAMPLE_RATE_48000:						//48KHz
			reg21data = reg21data | MAB_DACCTRL_48000;

			break;
		default:break;
	}
	/* set BANK bits of REG_ID #4 basic setting register to '3' */
	write_intermediate_reg(MAI_BASIC_SETTING,0x03);
	
	write_intermediate_reg(MAI3_DAC_CTRL1, reg21data);
	
	/* set BANK bits of REG_ID #4 basic setting register to '0' */
	write_intermediate_reg(MAI_BASIC_SETTING,0x00);
	
}

	

INT MA5i_DAC_Setup()
{
	
	UINT8 regdata;
	INT nTimeOutCount;

	/* clear power down mode of the digital audio section */
	MA5i_Set_Sample_Frequency();
	
	/* set BANK bits of REG_ID #4 basic setting register to '3' */
	write_intermediate_reg(MAI_BASIC_SETTING,0x03);
	
	/* set audio data format */
	write_intermediate_reg(MAI3_DAC_CTRL2, MAB_DACCTRL_IIS);
			
	/* set ADJUST */
	/* slave mode */
	
	write_intermediate_reg(MAI3_DAC_ADJUST5,0x80);//automatic setting

	/* set DPA0 = 0 */
	write_intermediate_reg(MAI3_POWER_MANAGEMENT_DIGITAL_AUDIO,MAB_PWM_PLLDAPD |
																MAB_PWM_DPA2 | MAB_PWM_DPA1);
	
	
	/* wait 2ms */
	Delay( 2 * 1000 );
	
	/* set PLLDAPD = 0 */
	write_intermediate_reg(MAI3_POWER_MANAGEMENT_DIGITAL_AUDIO,	MAB_PWM_DPA2 | MAB_PWM_DPA1);

	/* wait 10ms */
	Delay( 10 * 1000  );

	/* set DPA1 = 0*/
	write_intermediate_reg(MAI3_POWER_MANAGEMENT_DIGITAL_AUDIO,	MAB_PWM_DPA2 );
	/* set DPA2 = 0*/
	write_intermediate_reg(MAI3_POWER_MANAGEMENT_DIGITAL_AUDIO,	0x00 );


	/* set AP7=0, if not set */
	write_intermediate_reg(MAI3_POWER_MANAGEMENT_ANALOG,read_intermediate_reg(MAI3_POWER_MANAGEMENT_ANALOG) & ~MAB_PWM_AP7);
	//write_intermediate_reg(MAI3_POWER_MANAGEMENT_ANALOG,read_intermediate_reg(MAI3_POWER_MANAGEMENT_ANALOG) & ~(MAB_PWM_AP9 | MAB_PWM_AP6));

	/* set the mixer */
	/* TXOUT */
	//write_intermediate_reg(MAI3_TXOUT_MIXER,MAB_MIXER_TX_RXMIX);
	write_intermediate_reg(MAI3_TXOUT_MIXER,MAB_MIXER_TX_DAMIX);
	
	
	/* HPOUT */
	//write_intermediate_reg(MAI3_HPOUT_MIXER, (1<<6 | 1<<2));
	write_intermediate_reg(MAI3_HPOUT_MIXER, (MAB_MIXER_HR_DAMIX | MAB_MIXER_HL_DAMIX));
	
	
	/* SPOUT */
	write_intermediate_reg(MAI3_SPOUT_MIXER,MAB_MIXER_SP_DAMIX);

	/* set volume for Digital Audio */
	//ma5iSetPlayVolume(_tIIS.sPlayVolume >> 8,_tIIS.sPlayVolume & 0xff);
	write_intermediate_reg(MAI3_DA_LVOL,0x10);
	write_intermediate_reg(MAI3_DA_RVOL,0x10);
	write_intermediate_reg(35,3);//bass
	write_intermediate_reg(33,0x1E);//bank #3 TXOUT VOL
	write_intermediate_reg(31,0x1E);//bank #3 RXIN VOL
	

	//write_intermediate_reg(MAI_BASIC_SETTING,0x03);
	
	


	/* set DAMEMCLR */
	//regdata = read_intermediate_reg(MAI3_DAC_CTRL1);
	
	write_intermediate_reg(MAI3_DAC_CTRL1,read_intermediate_reg(MAI3_DAC_CTRL1) | MAB_DACCTRL_DAMEMCLR);
	
	/* wait for the MAB_DACCTRL_DAMEMCLR bit clear */
	/* need timeout */
	
	for (nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
		regdata = read_intermediate_reg(MAI3_DAC_CTRL1);
		if (( regdata & MAB_DACCTRL_DAMEMCLR ) == 0)
			break;
	}
	
	if (nTimeOutCount==10000){
		_error_msg("MA5i_DAC_Setup - wait DAC memory clear timeout\n");
		return ERR_MA5I_MEMCLR;
	}
		
	/* set DADSPSTART */
	write_intermediate_reg(MAI3_DAC_CTRL2, read_intermediate_reg(MAI3_DAC_CTRL2) | MAB_DACCTRL_DADSPSTART);
	
	
	return 0;
	
	
}
VOID MA5i_DAC_Stop()
{
	/* OFF */
	UINT8 regdata;

	/* set volume for the digital audio */
	ma5iSetPlayVolume(0,0);

	/* set BANK bits of REG_ID #4 basic setting register to '3' */
	write_intermediate_reg(MAI_BASIC_SETTING,0x03);
	/* set the mixer */
	/* HPOUT */
	regdata = read_intermediate_reg(MAI3_HPOUT_MIXER);
	write_intermediate_reg(MAI3_HPOUT_MIXER,regdata & ~(MAB_MIXER_HR_DAMIX | MAB_MIXER_HL_DAMIX));
		
	/* SPOUT */
	regdata = read_intermediate_reg(MAI3_SPOUT_MIXER);
	write_intermediate_reg(MAI3_SPOUT_MIXER,regdata & ~MAB_MIXER_SP_DAMIX);
	
	/* read bank3 #17 */
	regdata = read_intermediate_reg(MAI3_POWER_MANAGEMENT_ANALOG);
	write_intermediate_reg(MAI3_SPOUT_MIXER,regdata | MAB_PWM_AP7);
		
	/* set DADSPSTART */
	write_intermediate_reg(MAI3_DAC_CTRL2,MAB_DACCTRL_IIS);
	
	/* wait 2Fs */
	Delay( 41700 );
	
	/* set DPA2 */
	write_intermediate_reg(MAI3_POWER_MANAGEMENT_DIGITAL_AUDIO,MAB_PWM_DPA2);
	
	/* wait 1Fs */
	Delay( 21000 );
	
	/* set DPA1 */
	write_intermediate_reg(MAI3_POWER_MANAGEMENT_DIGITAL_AUDIO,MAB_PWM_DPA1);

	/* set PLLPDA */
	write_intermediate_reg(MAI3_POWER_MANAGEMENT_DIGITAL_AUDIO, MAB_PWM_DPA1 | MAB_PWM_DPA2 
																| MAB_PWM_PLLDAPD );
	
	/* set DPA0 */
	write_intermediate_reg(MAI3_POWER_MANAGEMENT_DIGITAL_AUDIO, MAB_PWM_DPA1 | MAB_PWM_DPA2
																| MAB_PWM_PLLDAPD | MAB_PWM_DPA0);
																
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
	write_intermediate_reg( MAI_PLL_SETTING_1, MA_ADJUST1_VALUE );
	write_intermediate_reg( MAI_PLL_SETTING_2, MA_ADJUST2_VALUE );
	
	_debug_msg("PLL 1: %d\n",read_intermediate_reg(MAI_PLL_SETTING_1));
	_debug_msg("PLL 2: %d\n",read_intermediate_reg(MAI_PLL_SETTING_2));

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
	INT nTimeOutCount;

	sdResult= 0;

	switch ( bMode )
	{
	case HW_INIT:

		/* sequence of hardware initialize when normal */

		/* set BANK bits of REG_ID #4 basic setting register to '0' */
		write_intermediate_reg( MAI_BASIC_SETTING ,0x00);
		

		/* 4:set DP0 bit of REG_ID #5 power management (D) setting register to '0' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_DIGITAL, MAB_PWM_DP3 | MAB_PWM_DP2 | MAB_PWM_DP1);
		

		/* wait 2ms */
		Delay( 2 * 1000 );

		/* 5:set PLLPD and AP0 bits of REG_ID #6 power management (A) setting register to '0' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_ANALOG, MAB_PWM_AP4L | MAB_PWM_AP3 |
														MAB_PWM_AP2 | MAB_PWM_AP1);
		

		/* wait 10ms */
		Delay( 10 * 1000 );


		/* check VREF_RDY bit of Bank #3 REG_ID #37 */
		
		/* 6:set BANK bits of REG_ID #4 basic setting register to '3' */
		write_intermediate_reg( MAI_BASIC_SETTING,0x03);
		
		
		_debug_msg("wait for VREF_RDY!\n");
		for (nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
			bRegData = read_intermediate_reg( MAI3_ANALOG_STATUS);
			if (( bRegData & MAB_VREF_RDY ) != 0)
				break;
		}
		
		if (nTimeOutCount == 10000){
			_error_msg("machdep_PowerManagement - wait for VREF_RDY timeout\n");
			return ERR_MA5I_VREF_RDY;
		}
				
		/* set BANK bits of REG_ID #4 basic setting register to '0' */
		write_intermediate_reg( MAI_BASIC_SETTING,0x00);
		



		/* set DP1 bit of REG_ID #5 power management (D) setting register to '0' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_DIGITAL, MAB_PWM_DP3 | MAB_PWM_DP2);
		

		/* 7:set DP2 bit of REG_ID #5 power management (D) setting register to '0' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_DIGITAL, MAB_PWM_DP3 );
		
#if 1
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
			return ERR_MA5I_SOFTRESET;
			/* continue process for power management */
		}
#endif
		/* 12:set DP3 bit of REG_ID #5 power management (D) setting register to '0' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_DIGITAL,0x00 );
		

		/* MA-5i */

		/* MA-5 core */
		
		/* step 13:As necessary, set AP* bit of REG_ID #6 power management (A) setting register to '0' */
		/* set AP0 to 0 */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_ANALOG, MAB_PWM_AP4L |MAB_PWM_AP3| MAB_PWM_AP2 | MAB_PWM_AP1 );
		/* bank 3 */
		write_intermediate_reg( MAI_BASIC_SETTING,0x03);
		_debug_msg("wait for VREF_RDY!\n");
		
		for (nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
			bRegData = read_intermediate_reg( MAI3_ANALOG_STATUS);
			if (( bRegData & MAB_VREF_RDY ) != 0)
				break;
		}
		
		if (nTimeOutCount == 10000){
			_error_msg("machdep_PowerManagement - wait for VREF_RDY timeout\n");
			return ERR_MA5I_VREF_RDY;
		}
						
		/* bank 3 */
		
		write_intermediate_reg( MAI_BASIC_SETTING,0x03);//Bank #3
		write_intermediate_reg( MAI3_POWER_MANAGEMENT_ANALOG, MAB_PWM_AP9 | MAB_PWM_AP8 
																		|  MAB_PWM_AP5 );
		
		write_intermediate_reg( MAI_BASIC_SETTING,0x00);//Bank #0
		/* set AP3 bits of REG_ID #6 power management (A) setting register to '0' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_ANALOG, MAB_PWM_AP4L | MAB_PWM_AP2 | MAB_PWM_AP1);
		//write_intermediate_reg( MAI_POWER_MANAGEMENT_ANALOG, MAB_PWM_AP4L | MAB_PWM_AP3 | MAB_PWM_AP2 | MAB_PWM_AP1);
		
		/* SPOUT */
		/* set AP1 bits of REG_ID #6 power management (A) setting register to '0' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_ANALOG, MAB_PWM_AP4L | MAB_PWM_AP2 );
		//write_intermediate_reg( MAI_POWER_MANAGEMENT_ANALOG, MAB_PWM_AP4L | MAB_PWM_AP2 | MAB_PWM_AP3);
		

		/* wait 10us */
		Delay( 10 * 1000 );

		/* set AP2 bit of REG_ID #6 power management (A) setting register to '0' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_ANALOG , MAB_PWM_AP4L);
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
		write_intermediate_reg( MAI_BASIC_SETTING,0x03 );
		
		for (nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
			bRegData = read_intermediate_reg( MAI3_ANALOG_STATUS);
			if (( bRegData & MAB_HP_RDY ) != 0)
				break;
		}
		
		if (nTimeOutCount == 10000){
			_error_msg("machdep_PowerManagement - wait for HP_RDY timeout\n");
			return ERR_MA5I_HP_RDY;
		}
		
		/* set BANK bits of REG_ID #4 basic setting register to '0' */
		write_intermediate_reg( MAI_BASIC_SETTING , 0x00);
		


		/* normal */

		break;

	case PW_DOWN:

		/* sequence of power down (synthesizer section)*/

		/* set BANK bits of REG_ID #4 basic setting register to '0' */
		write_intermediate_reg( MAI_BASIC_SETTING , 0x00);
		


		/* set DP3 bit of REG_ID #5 power management (D) setting register to '1' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_DIGITAL, MAB_PWM_DP3 );
		
		/* set DP2 bit of REG_ID #5 power management (D) setting register to '1' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_DIGITAL, MAB_PWM_DP3 | MAB_PWM_DP2 );

		/* set DP1 bit of REG_ID #5 power management (D) setting register to '1' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_DIGITAL, MAB_PWM_DP3 | MAB_PWM_DP2 | MAB_PWM_DP1 );

		/* MA-5i */

		/* set AP1, AP2, AP3 and AP4bits of REG_ID #6 power management (A) setting register to '1' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_ANALOG, MAB_PWM_AP4L | MAB_PWM_AP3 | MAB_PWM_AP2 | MAB_PWM_AP1 );
		
		
		/* sleep 250ms */
		Delay( 250*1000 );
		
		
		/* check HP_STBY bits of Bank #3 REG_ID #37 */

		/* set BANK bits of REG_ID #4 basic setting register to '3' */
		write_intermediate_reg( MAI_BASIC_SETTING, 0x03 );
		
		for (nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
			bRegData = read_intermediate_reg( MAI3_ANALOG_STATUS);
			if (( bRegData & MAB_HP_STBY ) != 0)
				break;
		}
		
		if (nTimeOutCount == 10000){
			_error_msg("machdep_PowerManagement - wait for HP_STBY timeout\n");
			return ERR_MA5I_HP_STBY;
		}
			
		/* set BANK bits of REG_ID #4 basic setting register to '0' */
		write_intermediate_reg( MAI_BASIC_SETTING,0x00);
		

		/* set AP0 and PLLPD bits of REG_ID #6 power management (A) setting register to '1' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_ANALOG, MAB_PWM_PLLPD | MAB_PWM_AP4L | MAB_PWM_AP3 
									| MAB_PWM_AP2 | MAB_PWM_AP1 | MAB_PWM_AP0);
		



		/* set DP0 bit of REG_ID #5 power management (D) setting register to '1' */
		write_intermediate_reg( MAI_POWER_MANAGEMENT_DIGITAL,  MAB_PWM_DP3 | MAB_PWM_DP2 | MAB_PWM_DP1 | MAB_PWM_DP0);
		

		break;

	default:
		sdResult = ERR_MA5I_ERROR;
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
		if( (sdResult&0xff)!=0x60 ) return ERR_MA5I_SOFTRESET;
	}
	for( i=208 ; i<208+16 ; i++ )
	{
		sdResult = read_control_reg( i );
		if( (sdResult&0xff)!=0x3c ) return ERR_MA5I_SOFTRESET;
	}
	return 0;
}


#endif	/* HAVE_MA5I */
/****************************************************************************
 * 
 * FILENAME
 *     W99702_W56964I.c
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
 *     2005.11.11		Created by Shih-Jen Lu
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

#include "wblib.h"
#include "W99702_Audio_Regs.h"
#include "W99702_Audio.h"

#include "IIS.h"
//#include "W56964I.h"

#ifdef HAVE_W56964I

#define	IIS_ACTIVE				0x1
#define	IIS_PLAY_ACTIVE			0x2
#define IIS_REC_ACTIVE			0x4

static AUDIO_T	_tIIS;
static BOOL	 _bIISActive = 0;
static UINT32 _uIISCR = 0;

#define RETRYCOUNT 100

static VOID IIS_Set_Data_Format(INT);
static VOID IIS_Set_Sample_Frequency(INT);
static INT w56964i_DAC_Setup(VOID);

static void Delay(int nCnt)
{
	volatile int  loop;
	for (loop=0; loop<nCnt; loop++);
}
static INT read_intermediate_reg(INT nRegId)
{
	INT volatile nTimeOutCount;
	outpw(REG_ACTL_M80ADDR, nRegId);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | R_IF11_ACT);

	
	for(nTimeOutCount=RETRYCOUNT;nTimeOutCount>0;nTimeOutCount--){
		if ((inpw(REG_ACTL_M80CON) & R_IF11_ACT) == 0 )
			break;
	}
	
	if (nTimeOutCount == 0){
			_error_msg("read_intermediate_reg - M80 read intermediate register timeout\n");
			return ERR_M80_R_INTERMEDIATEREG_TIMEOUT;
	}

	return ((inpw(REG_ACTL_M80SIZE)>>16) & 0xff);
}

static INT write_intermediate_reg(INT nRegId, UINT8 data)
{
	INT volatile nTimeOutCount;
	outpw(REG_ACTL_M80ADDR, nRegId);
	outpw(REG_ACTL_M80SIZE, 0x1);
	outpw(REG_ACTL_M80DATA0, data);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | W_IF12_ACT);
	for(nTimeOutCount=RETRYCOUNT;nTimeOutCount>0;nTimeOutCount--){
		if ((inpw(REG_ACTL_M80CON) & W_IF12_ACT) == 0 )
			break;
	}
	
	if (nTimeOutCount == 10000){
			_error_msg("write_intermediate_reg - M80 write intermediate register timeout\n");
			return ERR_M80_W_INTERMEDIATEREG_TIMEOUT;
	}
	return 0;
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



static INT  w56964i_init()
{

	
	_debug_msg("Reset W56964I..\n");
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
	
	
	

		

	
	_uIISCR = 0;
	return 0;
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      w56964iStartPlay                                                   */
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
INT  w56964iStartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
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
		nStatus = w56964i_init();
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

	
	//w56964i DAC set up
	nRetValue = w56964i_DAC_Setup();
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


static INT w56964i_DAC_Setup()
{
	INT data;
	INT i;
	//write_intermediate_reg(0x0d,0x0f);
	for (i=0;i<0x3f;i++){
		data = read_intermediate_reg(i);
		_debug_msg("test 0x%x\n",data);
	}
	
	
	
	return 0;
	
}



#endif	/* HAVE_W56964I */
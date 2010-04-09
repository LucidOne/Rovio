/****************************************************************************
 * 
 * FILENAME
 *     W99702_WM8751.c
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
#include "WM8751.h"


#ifdef HAVE_WM8751



static AUDIO_T	_tIIS;

#define	IIS_ACTIVE				0x1
#define	IIS_PLAY_ACTIVE			0x2
#define IIS_REC_ACTIVE			0x4

static BOOL	 _bIISActive = 0;
static UINT32 _uIISCR = 0;

extern AU_DEV_E _ePlayDev;
extern AU_DEV_E _eRecDev;


void WM8751_DAC_Setup(void);


#ifdef _3WIRE
static void ThreeWire_Write_Data(UINT16);
#elif defined(_2WIRE)
static void TwoWire_Write_Data(UINT16);
#define ThreeWire_Write_Data TwoWire_Write_Data
#endif
static INT WM_Set_DAC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol);


#define WMDEVID 0


static VOID IIS_Set_Data_Format(INT);
static VOID IIS_Set_Sample_Frequency(INT);

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
		wm8751StopPlay();
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
	
	

	
	#ifdef _2WIRE
	/* for demo board */
	outpw(REG_PADC0, inpw(REG_PADC0) | (7<<1));
	/** end of for demo board **/
	#endif
	
	#ifdef _3WIRE 
	/* for verification board */
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~(WM_SCLK | WM_SDIN | WM_CSB) );
	//outpw(REG_PADC0, inpw(REG_PADC0) | 0x6);
	/** end of for verification board **/
	#endif
	
	ThreeWire_Write_Data(R15 | 0x000);//R15 write 0 to reset all registers to their default state
	Delay(100);

	_uIISCR = 0;
	return 0;
}







/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      wm8751StartPlay                                                   */
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
INT  wm8751StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
								INT nChannels, INT data_format)
{
	INT		nStatus;
	
	if (_bIISActive & IIS_PLAY_ACTIVE)
	
		return ERR_IIS_PLAY_ACTIVE;		/* IIS was playing */
	
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~ PLAY_LEFT_CHNNEL & ~ PLAY_RIGHT_CHNNEL);
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
//	sysSetAIC2SWMode();					/* Set AIC into SW mode */
	sysEnableInterrupt(AU_PLAY_INT_NUM);
#endif

	_tIIS.fnPlayCallBack = fnCallBack;
	_tIIS.nPlaySamplingRate = nSamplingRate;


	
	WM8751_DAC_Setup();

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
	_debug_msg("WM8751 start playing...\n");
	outpw(REG_ACTL_PSR, 0x3);
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | IIS_PLAY);
	_bIISActive |= IIS_PLAY_ACTIVE;
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      wm8751StopPlay                                                    */
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
INT  wm8751StopPlay()
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
/*      wm8751SetPlayVolume                                                 */
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
INT  wm8751SetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol)  //0~31
{
	if (ucLeftVol>31)
		ucLeftVol=31;
	if (ucRightVol>31)
		ucRightVol=31;
	
	_tIIS.sPlayVolume = (ucLeftVol << 8) | ucRightVol;
	WM_Set_DAC_Volume(ucLeftVol,ucRightVol);
	


	
	return 0;
}





/*---------- for WM8751	functions group----------*/
//(MIN)  0 ~ (MAX) 255
static INT WM_Set_DAC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol){	//0~31
	UINT16 data;
	if (ucLeftVol!=0)
		data = R2 | (1<<8) | (ucLeftVol*3 + 34);
	else
		data = R2 | (1<<8);
	ThreeWire_Write_Data(data);
	
	if (ucRightVol!=0)
		data = R3 | (1<<8) | (ucRightVol*3 + 34);
	else
		data = R3 | (1<<8);
	ThreeWire_Write_Data(data);
	
	if (ucLeftVol!=0)
		data = R40 | (1<<8) | (ucLeftVol*3 + 34);
	else
		data = R40 | (1<<8);
	ThreeWire_Write_Data(data);
	
	if (ucRightVol!=0)
		data = R41 | (1<<8) | (ucRightVol*3 + 34);
	else
		data = R41 | (1<<8);
	ThreeWire_Write_Data(data);
	
	if (ucLeftVol!=0)
		data = R10 | (1<<8) | (ucLeftVol*3 + 162);
	else
		data = R10 | (1<<8);
	ThreeWire_Write_Data(data);
	
	if (ucRightVol!=0)
		data = R11 | (1<<8) | (ucRightVol*3 + 162);
	else
		data = R11 | (1<<8);
	ThreeWire_Write_Data(data);
	


	return 0;
}	

	
/* for verification board */
#ifdef _3WIRE
static void ThreeWire_Write_Data(UINT16 data)
{
	int i;
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
void TwoWire_Write_Data(UINT16 data)
{
	INT i;
	UINT8 bDevID;
	
	if( WMDEVID == 0)
	{
		
		bDevID = 0x34;
	}
	if( WMDEVID == 1)
	{
		
		bDevID =0x36;
	}
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
	/* clok pulse for receive ACK */
	outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK);//SCLK low
	Delay(2);
	outpw(REG_SerialBusCR,READSTATUS() | WM_SCLK);//SCLK high
	Delay(2);
	outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK);//SCLK low
	Delay(2);
	
	//for ( i =0; i < 512; i++); // wait for ACK byte
	
	/* Send control dyte 1 */
	for (i=15;i>=8;i--)
	{
		if (data&(1<<i))
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
	/* clok pulse for receive ACK */
	outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK);//SCLK low
	Delay(2);
	outpw(REG_SerialBusCR,READSTATUS() | WM_SCLK);//SCLK high
	Delay(2);
	outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK);//SCLK low
	Delay(2);
	//for ( i =0; i < 512; i++); // wait for ACK byte
	
	/* Send control byte 2 */
	for (i=7;i>=0;i--)
	{
		if (data&(1<<i))
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
	/* clok pulse for receive ACK */
	outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK);//SCLK low
	Delay(2);
	outpw(REG_SerialBusCR,READSTATUS() | WM_SCLK);//SCLK high
	Delay(2);
	outpw(REG_SerialBusCR,READSTATUS() & ~WM_SCLK);//SCLK low
	Delay(2);
	
	/* stop */
	outpw(REG_SerialBusCR,READSTATUS() | WM_SCLK);//SCLK high
	Delay(2);
	outpw(REG_SerialBusCR,READSTATUS() | WM_SDIN);//SDIN high
	Delay(2);
	
	//for ( i =0; i < 512; i++); // wait for ACK byte

	
}
#endif
#if 0
#ifdef _3WIRE	/* for verification board */
UINT8 ThreeWire_Read_Status(UINT8 ucID)
{
	UINT8 data=0;
	int i;
	ThreeWire_Write_Data(R28 | 0x02);//set GP1= WM_SDOUT
	ThreeWire_Write_Data(R24 | 0x01 | (ucID<<1));//read enable; read register select 

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
#endif

void WM8751_DAC_Setup(){

	
	ThreeWire_Write_Data(R25 | 0x0C0);//R25 power up VMID[50kohm],VREF;diable master clock 
	ThreeWire_Write_Data(R26 | 0x180);//power up DACL & DACR
	ThreeWire_Write_Data(R26 | 0x1F8);//power up DACL/R,L/ROUT1(head phone),L/ROUT2(speaker);disable MOUT OUT3
	ThreeWire_Write_Data(R1 | 0x97);
	ThreeWire_Write_Data(R4 | 0x000);
	ThreeWire_Write_Data(R5 | 0x000);//DAC control disable DAC soft mute
	ThreeWire_Write_Data(R6 | 0x000);
	ThreeWire_Write_Data(R7 | 0x012);//audio interface select I2S format, slave mode, 16bits word length
	ThreeWire_Write_Data(R8 | 0x000);
	ThreeWire_Write_Data(R9 | 0x000);
	WM_Set_DAC_Volume(10,10);
	ThreeWire_Write_Data(R12 | 0x007);//bass
	ThreeWire_Write_Data(R13 | 0x007);//treble
	ThreeWire_Write_Data(R16 | 0x000);
	ThreeWire_Write_Data(R17 | 0x07b);
	ThreeWire_Write_Data(R18 | 0x000);
	ThreeWire_Write_Data(R19 | 0x032);
	ThreeWire_Write_Data(R20 | 0x000);
	ThreeWire_Write_Data(R21 | 0x0c3);
	ThreeWire_Write_Data(R22 | 0x0c3);
	ThreeWire_Write_Data(R23 | 0x0c0);
	ThreeWire_Write_Data(R24 | 0x000);
	ThreeWire_Write_Data(R27 | 0x000);//VREF to analogue output R=1.5 om
	ThreeWire_Write_Data(R31 | 0x000);
	ThreeWire_Write_Data(R32 | 0x000);
	ThreeWire_Write_Data(R33 | 0x000);
	ThreeWire_Write_Data(R34 | 0x180);//left mix(1) enable LD2LO,LI2LO,LI2LOVOL=MAX 
	ThreeWire_Write_Data(R35 | 0x050);//left mix(2) disable RD2LO MI2LO
	ThreeWire_Write_Data(R36 | 0x050);//right mix(2) disable LD2RO MI2RO
	ThreeWire_Write_Data(R37 | 0x180);//right mix(1) enable RD2RO RI2RO RI2ROVOL=MAX
	ThreeWire_Write_Data(R38 | 0x050);
	ThreeWire_Write_Data(R39 | 0x050);
	ThreeWire_Write_Data(R42 | 0x079);
	
	
	
	
	
	
}

	

/*---------- end of WM8751's functions group ----------*/

#endif	/* HAVE_IIS */
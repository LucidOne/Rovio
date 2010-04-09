/****************************************************************************
 * 
 * FILENAME
 *     W99702_PSM711A.c
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
 *     2004.11.21		Created by Shih-Jen Lu
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



#ifdef HAVE_PSM711A



static AUDIO_T	_tIIS;
static INT32 _SCLK,_SDIN;


#define READSTATUS() ((inpw(REG_SerialBusCR)&0x38)>>3)
#define	IIS_ACTIVE				0x1
#define	IIS_PLAY_ACTIVE			0x2
#define RETRYCOUNT		10


static BOOL	 _bIISActive = 0;
static UINT32 _uIISCR = 0;


static INT PSM711A_DAC_Setup(void);
INT PSM711A_I2C_2BYTE_RW_Data(UINT8,UINT8*,UINT32,BOOL);


static INT PSM_Set_DAC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol);

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
	
	if (_tIIS.bPlayLastBlock)
	{
		outpw(REG_ACTL_PSR, P_DMA_MIDDLE_IRQ | P_DMA_END_IRQ);
		psm711aStopPlay();
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
/*      psm711aStartPlay                                                   */
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
INT  psm711aStartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, 
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
	
	
	
	
	PSM711A_DAC_Setup();
	
	
	
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
		
	
	
	//if (nStatus != 0)
	//	return nStatus;
		
		/* Install IIS play interrupt */

	
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      psm711aStopPlay                                                    */
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
INT  psm711aStopPlay()
{

	PSM_Set_DAC_Volume(0,0);
	
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
	_uIISCR = _uIISCR | (1<<8) | (1<<9);
	outpw(REG_ACTL_IISCON,_uIISCR );
}
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      psm711aSetPlayVolume                                              */
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
INT  psm711aSetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol)  //0~31
{
	INT nRetValue = 0;
	if (ucLeftVol>31)
		ucLeftVol=31;
	if (ucRightVol>31)
		ucRightVol=31;
	_tIIS.sPlayVolume = (ucLeftVol << 8) | ucRightVol;
	nRetValue = PSM_Set_DAC_Volume(ucLeftVol,ucRightVol);
	if (nRetValue!=0){
		return nRetValue;
	}
	return 0;
}



/*---------- for PSM711A	functions group----------*/
//(MIN)  0 ~ (MAX) 255
static INT PSM_Set_DAC_Volume(UINT8 ucLeftVol, UINT8 ucRightVol){	//0~31
	UINT8 uBuff[2];
	
	uBuff[0]=0x00;
	if (((ucLeftVol+ucRightVol)/2) != 0)
		uBuff[1] = 0xBD - (((ucLeftVol+ucRightVol)/2)*6);
	else
		uBuff[1] = 0xBD;
	
	PSM711A_I2C_2BYTE_RW_Data(0x10,uBuff,2,1);//master volume
	
	if (ucLeftVol!=0)
	    uBuff[0] = 0xBD - ucLeftVol*6;
	else
	    uBuff[0] = 0xBD;
	    
	if (ucRightVol!=0)
	    uBuff[1] =  ((0xBD - ucRightVol*6)<<8);
	else
	    uBuff[1] =  0xBD;
	PSM711A_I2C_2BYTE_RW_Data(0x11,uBuff,2,1);//left and right channel volume
	
	uBuff[0]=0x00;
	if (((ucLeftVol+ucRightVol)/2) != 0)
	    uBuff[1] = 0x18 - ucLeftVol + 7;
	else
	    uBuff[1] = 0x18;
	PSM711A_I2C_2BYTE_RW_Data(0x97,uBuff,2,1);//headphone volume control
	    
    return 0;
}	

static VOID I2C_Write_One_Byte(UINT8 data){
	INT bitmask;
	
		for (bitmask=7;bitmask>=0;bitmask--)
		{
			if (_eI2SType.bIsGPIO)
			{
				if (data&(1<<bitmask))
				{
					outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~_SCLK );//_SCLK low
					Delay(1);
					outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~_SCLK | _SDIN);//_SCLK low;data ready
					Delay(2);
					outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | _SCLK | _SDIN);//_SCLK high;data hold
					Delay(3);
				}
				else
				{
					outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~_SCLK );//_SCLK low
					Delay(1);
					outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~_SCLK & ~_SDIN);//_SCLK low
					Delay(2);
					outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~_SDIN | _SCLK );//_SCLK high
					Delay(3);
					
				}
			}
			else
			{
				if (data&(1<<bitmask))
				{
					outpw(REG_SerialBusCR,READSTATUS() & ~_SCLK );//_SCLK low
					Delay(1);
					outpw(REG_SerialBusCR,READSTATUS() & ~_SCLK | _SDIN);//_SCLK low;data ready
					Delay(1);
					outpw(REG_SerialBusCR,READSTATUS() | _SCLK | _SDIN);//_SCLK high
					Delay(3);
				}
				else
				{
					outpw(REG_SerialBusCR,READSTATUS() & ~_SCLK );//_SCLK low
					Delay(1);
					outpw(REG_SerialBusCR,READSTATUS() & ~_SCLK & ~_SDIN);//_SCLK low
					Delay(1);
					outpw(REG_SerialBusCR,READSTATUS() & ~_SDIN | _SCLK );//_SCLK high
					Delay(3);
					
				}
			}	
		}
	
}
static VOID I2C_Read_One_Byte(UINT8 *data){
	INT bitmask;
	/* Recieve one data byte */
	for (bitmask=7;bitmask>=0;bitmask--)
	{
		if (_eI2SType.bIsGPIO)
		{
			outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~_SCLK );//_SCLK low
			Delay(1);
			if (inpw(REG_GPIO_DAT)&_SDIN)
				*data|= (1<<bitmask);
			outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | _SCLK);//_SCLK high
			Delay(3);
		}
		else
		{
			outpw(REG_SerialBusCR,READSTATUS() & ~_SCLK );//_SCLK low
			Delay(1);
			if (READSTATUS() & _SDIN)
				*data|=(1<<bitmask);
			outpw(REG_SerialBusCR,READSTATUS() | _SCLK);//_SCLK high
			Delay(3);
		}	
	}
	
	
}


/* for demo board */
int PSM711A_I2C_2BYTE_RW_Data(UINT8 regAddr, UINT8 *pdata,UINT32 ulength,BOOL bwrite)
{
    
	    
	INT retrycount=RETRYCOUNT,index;
	UINT8 bDevID;
	
	
	if (regAddr>=0xA0)
	    return ERR_2WIRE_NO_ACK;
	
	_SCLK = 1<<_eI2SType.uSCLK;
	_SDIN = 1<<_eI2SType.uSDIN;
	bDevID = 0x70 ;
	
_PSMI2CRETRY:		
	if (_eI2SType.bIsGPIO)
	{
		//outpw(REG_GPIO_PE,inpw(REG_GPIO_PE) |  (_SCLK | _SDIN) );
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | _SDIN | _SCLK );//_SCLK high _SDIN high
		outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~(_SCLK | _SDIN));
		Delay(2);
		/* START */
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | _SDIN | _SCLK );//_SCLK high _SDIN high
		Delay(1);
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE)  & ~_SDIN | _SCLK );//_SDIN low
  		Delay(2);
		/* 7bit slave device address + 1bit R/W */
		I2C_Write_One_Byte(bDevID);
								
		/* clock pulse for receive ACK */
		
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~_SCLK);//_SCLK low
		Delay(3);
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | _SCLK);//_SCLK high
		Delay(3);
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | _SDIN);
		Delay(0x1);
		if ((inpw(REG_GPIO_STS) & _SDIN)!=0){
		    if (retrycount-- > 0)
		    	goto _PSMI2CRETRY;
		    else{
		    	_debug_msg("write Reg %d  ACK1 receive fail\n",regAddr);
		    	return ERR_2WIRE_NO_ACK;
		    }
	    }
  	
		
			
		/* Send register address */
		I2C_Write_One_Byte(regAddr);

		
		
		/* clock pulse for receive ACK */
		
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~_SCLK);//_SCLK low
		Delay(3);
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | _SCLK);//_SCLK high
		Delay(3);
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | _SDIN);
		Delay(1);
		//Delay(0x10);
		if ((inpw(REG_GPIO_STS) & _SDIN)!=0){
		    if (retrycount-- > 0)
		    	goto _PSMI2CRETRY;
		    else{
		    	_debug_msg("write Reg %d  0x%x ACK2 receive fail\n",regAddr);
		    	return ERR_2WIRE_NO_ACK;
		    }
	    }
  	
  		if (bwrite)
  		{
			for (index=0;index<ulength;index++)
			{
				/* Send data byte */
				I2C_Write_One_Byte(pdata[index]);
				
				/* clock pulse for receive ACK */
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~_SCLK);//_SCLK low
				Delay(3);
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | _SCLK);//_SCLK high
				Delay(3);
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | _SDIN);
				Delay(1);
				if ((inpw(REG_GPIO_STS) & _SDIN)!=0){
				    if (retrycount-- > 0)
				    	goto _PSMI2CRETRY;
				    else{
				    	_debug_msg("write Reg %d  0x%x ACK receive fail\n",regAddr);
				    	return ERR_2WIRE_NO_ACK;
				    }
	    		}
  	    	
  	    	}
  	    }else
  	    {
  	    	/* stop */
			outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | _SCLK);//_SCLK high
			Delay(2);
			outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | _SDIN);//_SDIN high
			Delay(2);
			/* START */
			outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | _SDIN | _SCLK );//_SCLK high _SDIN high
			Delay(1);
			outpw(REG_GPIO_OE, inpw(REG_GPIO_OE)  & ~_SDIN | _SCLK );//_SDIN low
  			Delay(2);
  			/* 7bit slave device address + 1bit R/W */
			I2C_Write_One_Byte(bDevID|1);
			
			/* clock pulse for receive ACK */
			outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~_SCLK);//_SCLK low
			Delay(3);
			outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | _SCLK);//_SCLK high
			Delay(3);
			outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | _SDIN);
			Delay(1);
			if ((inpw(REG_GPIO_STS) & _SDIN)!=0){
			    if (retrycount-- > 0)
			    	goto _PSMI2CRETRY;
			    else{
			    	_debug_msg("write Reg %d ACK1 receive fail\n",regAddr);
			    	return ERR_2WIRE_NO_ACK;
			    }
	    	}
  			outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | _SDIN);//
  			for (index=0;index<ulength;index++)
			{
				pdata[index]=0;
				
				I2C_Read_One_Byte(&pdata[index]);
				
				/* clock pulse for receive ACK */
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~_SCLK);//_SCLK low
				Delay(3);
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | _SCLK);//_SCLK high
				Delay(3);
				outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | _SDIN);
				Delay(1);
				if ((inpw(REG_GPIO_STS) & _SDIN)!=0){
				    if (retrycount-- > 0)
				    	goto _PSMI2CRETRY;
				    else{
				    	_debug_msg("write Reg %d  ACK receive fail\n",regAddr);
				    	return ERR_2WIRE_NO_ACK;
				    }
	    		}
  	    	
  	    	}
			
  	    }
		
		
		/* stop */
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~_SCLK &~_SDIN);//_SCLK high
		Delay(1);
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | _SCLK);//_SCLK high
		Delay(2);
		
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | _SDIN);//_SDIN high
		Delay(2);
		
  	
		
		return 0;
	}
	else
	{
		outpw(REG_PADC0, inpw(REG_PADC0) | (7<<1));
	
		/* START */
		outpw(REG_SerialBusCR,READSTATUS() | _SDIN);//_SDIN high
		outpw(REG_SerialBusCR,READSTATUS() | _SCLK);//_SCLK high
		Delay(1);
		outpw(REG_SerialBusCR,READSTATUS() & ~_SDIN);//_SDIN low
		Delay(2);
		/* Send Device ID depends on CSB status*/
		I2C_Write_One_Byte(bDevID);
		/* clock pulse for receive ACK */
		outpw(REG_SerialBusCR,READSTATUS() & ~_SCLK);//_SCLK low
		Delay(2);
		outpw(REG_SerialBusCR,READSTATUS() | _SCLK);//_SCLK high
		Delay(2);
		if (READSTATUS() & _SDIN)
		{
		    if (retrycount-- > 0)
		    	goto _PSMI2CRETRY;
		    else{
		    	_debug_msg("write Reg %d ACK1 receive fail\n",regAddr);
		    	return ERR_2WIRE_NO_ACK;
		    }
	    }
		
			
		/* Send register address */
		I2C_Write_One_Byte(regAddr);
		
		/* clock pulse for receive ACK */
		outpw(REG_SerialBusCR,READSTATUS() & ~_SCLK);//_SCLK low
		Delay(2);
		outpw(REG_SerialBusCR,READSTATUS() | _SCLK);//_SCLK high
		Delay(2);
		if (READSTATUS() & _SDIN)
		{
		    if (retrycount-- > 0)
		    	goto _PSMI2CRETRY;
		    else{
		    	_debug_msg("write Reg %d ACK2 receive fail\n",regAddr);
		    	return ERR_2WIRE_NO_ACK;
		    }
	    }
  	
  		if (bwrite)
  		{
			for (index=0;index<ulength;index++)
			{
				/* Send one data byte  */
				I2C_Write_One_Byte(pdata[index]);
				
				/* clock pulse for receive ACK */
				outpw(REG_SerialBusCR,READSTATUS() & ~_SCLK);//_SCLK low
				Delay(2);
				outpw(REG_SerialBusCR,READSTATUS() | _SCLK);//_SCLK high
				Delay(2);
				if (READSTATUS() & _SDIN)
				{
				    if (retrycount-- > 0)
				    	goto _PSMI2CRETRY;
				    else{
				    	_debug_msg("write Reg %d  ACK3 receive fail\n",regAddr);
				    	return ERR_2WIRE_NO_ACK;
				    }
	    		}
			}
		}else
		{
			/* stop */
			outpw(REG_SerialBusCR,READSTATUS() & ~_SCLK);//_SCLK low
			Delay(2);
			outpw(REG_SerialBusCR,READSTATUS() | _SCLK);//_SCLK high
			Delay(2);
			outpw(REG_SerialBusCR,READSTATUS() | _SDIN);//_SDIN high
			Delay(2);
			/* START */
			outpw(REG_SerialBusCR,READSTATUS() | _SDIN);//_SDIN high
			outpw(REG_SerialBusCR,READSTATUS() | _SCLK);//_SCLK high
			Delay(1);
			outpw(REG_SerialBusCR,READSTATUS() & ~_SDIN);//_SDIN low
			Delay(2);
			/* Send Device ID depends on CSB status*/
			I2C_Write_One_Byte(bDevID|1);
			
			/* clock pulse for receive ACK */
			outpw(REG_SerialBusCR,READSTATUS() & ~_SCLK);//_SCLK low
			Delay(2);
			outpw(REG_SerialBusCR,READSTATUS() | _SCLK);//_SCLK high
			Delay(2);
			if (READSTATUS() & _SDIN)
			{
			    if (retrycount-- > 0)
			    	goto _PSMI2CRETRY;
			    else{
			    	_debug_msg("write Reg %d ACK1 receive fail\n",regAddr);
			    	return ERR_2WIRE_NO_ACK;
			    }
	    	}
	    	for (index=0;index<ulength;index++)
			{
				/* Recieve one data byte  */
				I2C_Read_One_Byte(&pdata[index]);
				/* clock pulse for receive ACK */
				outpw(REG_SerialBusCR,READSTATUS() & ~_SCLK);//_SCLK low
				Delay(2);
				outpw(REG_SerialBusCR,READSTATUS() | _SCLK);//_SCLK high
				Delay(2);
				if (READSTATUS() & _SDIN)
				{
				    if (retrycount-- > 0)
				    	goto _PSMI2CRETRY;
				    else{
				    	_debug_msg("write Reg %d ACK3 receive fail\n",regAddr);
				    	return ERR_2WIRE_NO_ACK;
				    }
	    		}
			}
	    	
			
		}
	    
		/* stop */
		outpw(REG_SerialBusCR,READSTATUS() & ~_SCLK);//_SCLK low
		Delay(2);
		outpw(REG_SerialBusCR,READSTATUS() | _SCLK);//_SCLK high
		Delay(2);
		outpw(REG_SerialBusCR,READSTATUS() | _SDIN);//_SDIN high
		Delay(2);
		
  	
		outpw(REG_PADC0, inpw(REG_PADC0) & ~(7<<1));
		
		return 0;
	}

}


INT PSM711A_DAC_Setup(){
    UINT8 ucBuffer[3];
    
    /* set GPIO 8,9 output mode */
	outpw(REG_GPIO_OE,inpw(REG_GPIO_OE) & ~(1<<16 | 1<<17)  );//
	
	/* reset PSM711A */
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | 1<<16);
	Delay(100);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~(1<<16));
	Delay(100);
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) | 1<<16);
	
	/* disable PWD_DN */
	outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT) & ~(1<<17));
	
    if ((_tIIS.nPlaySamplingRate%8000==0) || (_tIIS.nPlaySamplingRate%12000==0))
    {
    	ucBuffer[0]=0x12;
    	ucBuffer[1]=0x8b;
    	PSM711A_I2C_2BYTE_RW_Data(0x8f,ucBuffer,2,1);//PLL_FREQUENCY_SEL(0x8f)
    }else if (_tIIS.nPlaySamplingRate%11025==0)
    {
    	ucBuffer[0]=0x11;
    	ucBuffer[1]=0x8b;
    	PSM711A_I2C_2BYTE_RW_Data(0x8f,ucBuffer,2,1);//PLL_FREQUENCY_SEL(0x8f)
    }    
    ucBuffer[0]=0x00;
    ucBuffer[1]=0x00;
    PSM711A_I2C_2BYTE_RW_Data(0x00,ucBuffer,2,1);//PWM_ON_OFF(0x00)
    ucBuffer[0]=0x01;
    ucBuffer[1]=0xec;
    PSM711A_I2C_2BYTE_RW_Data(0x02,ucBuffer,2,1);//IN1_CONTROL(0x02)
    ucBuffer[0]=0x00;
    ucBuffer[1]=0x11;
    PSM711A_I2C_2BYTE_RW_Data(0x07,ucBuffer,2,1);//IN_MIX_CON(0x07)

    ucBuffer[0]=0x00;
    ucBuffer[1]=0x33;
    PSM711A_I2C_2BYTE_RW_Data(0x10,ucBuffer,2,1);//MASTER_VOLUME(0x10)

  	ucBuffer[0]=0x00;
    ucBuffer[1]=0x00;
    PSM711A_I2C_2BYTE_RW_Data(0x17,ucBuffer,2,1);//GLOBAL_MUTE(0x17)
    ucBuffer[0]=0x00;
    ucBuffer[1]=0x22;
    PSM711A_I2C_2BYTE_RW_Data(0x81,ucBuffer,2,1);//PWM_LIMIT(0x81)
  	
	return 0;
}



/*---------- end of PSM711A's functions group ----------*/

#endif	/* HAVE_IIS */
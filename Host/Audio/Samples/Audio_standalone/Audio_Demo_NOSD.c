/****************************************************************************
 * 
 * FILENAME
 *     Audio_Demo.c
 *
 * VERSION
 *     1.0 
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
 *     2004.08.04		Created by Yung-Chang Huang
 *
 *
 * REMARK
 *     None
 *
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wbio.h"
#include "wblib.h"
#include "w99702_reg.h"
#include "W99702_Audio.h"
//#include "wbfat.h"
//#include "wb_fmi.h"

//#define TEST_AC97
//#define TEST_I2S
//#define TEST_W56964
//#define TEST_W5691
//#define TEST_ADDA
//#define TEST_MA3
//#define TEST_MA5
//#define TEST_MA5I
//#define TEST_WM8753
#define TEST_WM8978
//#define TEST_WM8751
//#define TEST_TIAIC31

#define _DEMO_VER 0x17

UINT32 PCM_QUEUE_LEN		=	(512*1024);

//extern _MA3_COUNTER;
//#define sysPrintf(...)
#define printf sysPrintf


//static UINT8  *_pucPcmQueue = (UINT8 *)(0x180000 | 0x10000000);
static UINT8  _pucPcmQueue [] = {
	#include "44.1k.dat"
	//#include "8k.dat"
};

volatile UINT32 _uPcmQHead, _uPcmQTail;
int isnotfinish = 1;
/*UINT8 _sine_wave[192] = 
{
    0x00, 0x00, 0x00, 0x00, 0xb5, 0x10, 0xb5, 0x10, 0x21, 0x21, 0x21, 0x21, 0xfc, 0x30, 0xfc, 0x30, 
    0x00, 0x40, 0x00, 0x40, 0xec, 0x4d, 0xec, 0x4d, 0x82, 0x5a, 0x82, 0x5a, 0x8d, 0x65, 0x8d, 0x65, 
    0xda, 0x6e, 0xda, 0x6e, 0x42, 0x76, 0x42, 0x76, 0xa3, 0x7b, 0xa3, 0x7b, 0xe8, 0x7e, 0xe8, 0x7e, 
    0xff, 0x7f, 0xff, 0x7f, 0xe8, 0x7e, 0xe8, 0x7e, 0xa3, 0x7b, 0xa3, 0x7b, 0x42, 0x76, 0x42, 0x76, 
    0xda, 0x6e, 0xda, 0x6e, 0x8d, 0x65, 0x8d, 0x65, 0x82, 0x5a, 0x82, 0x5a, 0xec, 0x4d, 0xec, 0x4d, 
    0x00, 0x40, 0x00, 0x40, 0xfc, 0x30, 0xfc, 0x30, 0x21, 0x21, 0x21, 0x21, 0xb5, 0x10, 0xb5, 0x10, 
    0x00, 0x00, 0x00, 0x00, 0x4b, 0xef, 0x4b, 0xef, 0xdf, 0xde, 0xdf, 0xde, 0x04, 0xcf, 0x04, 0xcf, 
    0x00, 0xc0, 0x00, 0xc0, 0x14, 0xb2, 0x14, 0xb2, 0x7e, 0xa5, 0x7e, 0xa5, 0x73, 0x9a, 0x73, 0x9a, 
    0x26, 0x91, 0x26, 0x91, 0xbe, 0x89, 0xbe, 0x89, 0x5d, 0x84, 0x5d, 0x84, 0x18, 0x81, 0x18, 0x81, 
    0x01, 0x80, 0x01, 0x80, 0x18, 0x81, 0x18, 0x81, 0x5d, 0x84, 0x5d, 0x84, 0xbe, 0x89, 0xbe, 0x89, 
    0x26, 0x91, 0x26, 0x91, 0x73, 0x9a, 0x73, 0x9a, 0x7e, 0xa5, 0x7e, 0xa5, 0x14, 0xb2, 0x14, 0xb2, 
    0x00, 0xc0, 0x00, 0xc0, 0x04, 0xcf, 0x04, 0xcf, 0xdf, 0xde, 0xdf, 0xde, 0x4b, 0xef, 0x4b, 0xef
};	*/


INT  play_callback(UINT8 *pucBuff, UINT32 uDataLen)
{
	INT		nLen;
	
//sysInvalidCache();
#if 0	/* force play sine wave */
	INT		idx;
	for (idx = 0; idx < 50; idx++)
	{
		memcpy(pucBuff, _sine_wave, 192);
		pucBuff += 192;
	}
	return;
#endif

#ifdef TEST_MA3
	for (idx = 0; idx < uDataLen; idx++)
	{
		pucBuff[idx] = _pucPcmQueue[_uPcmQHead+1];   /* left channel */
		pucBuff[idx+uDataLen] = _pucPcmQueue[_uPcmQHead+3];   /* right channel */
		_uPcmQHead += 4;
	}
	_uPcmQHead %= PCM_QUEUE_LEN;
	return 0;
#endif

#ifdef TEST_MA5
	for (idx = 0; idx < uDataLen; idx+=2)
	{
		pucBuff[idx] = _pucPcmQueue[_uPcmQHead];
		pucBuff[idx+1] = _pucPcmQueue[_uPcmQHead+1];
		pucBuff[uDataLen+idx] = _pucPcmQueue[_uPcmQHead+2];
		pucBuff[uDataLen+idx+1] = _pucPcmQueue[_uPcmQHead+3];
		_uPcmQHead = (_uPcmQHead+4) % PCM_QUEUE_LEN;
	}
	return 0;
#endif
	nLen = PCM_QUEUE_LEN - _uPcmQHead;
	if (nLen==uDataLen)
			isnotfinish=0;
	if (nLen >= uDataLen)
	{
		memcpy(pucBuff, &_pucPcmQueue[_uPcmQHead], uDataLen);
		_uPcmQHead = (_uPcmQHead + uDataLen) % PCM_QUEUE_LEN;
	}
	else
	{
		memcpy(pucBuff, &_pucPcmQueue[_uPcmQHead], nLen);
		memcpy(&pucBuff[nLen], _pucPcmQueue, uDataLen - nLen);
		_uPcmQHead = uDataLen - nLen;
		isnotfinish=0;
	}

	
	return 0;
}


INT  record_callback(UINT8 *pucBuff, UINT32 uDataLen)
{
	INT		nLen;

	nLen = PCM_QUEUE_LEN - _uPcmQTail;
	if (nLen >= uDataLen)
	{
		memcpy(&_pucPcmQueue[_uPcmQTail], pucBuff, uDataLen);
		_uPcmQTail = (_uPcmQTail + uDataLen) % PCM_QUEUE_LEN;
	}
	else
	{
		memcpy(&_pucPcmQueue[_uPcmQTail], pucBuff, nLen);
		memcpy(_pucPcmQueue, &pucBuff[nLen], uDataLen - nLen);
		_uPcmQTail = uDataLen - nLen;
	}
	return 0;
}


void audio_demo(int choose, int channels)
{
	INT		nTime0,nVol=15;

	_uPcmQHead = _uPcmQTail = 0;

#if defined(TEST_TIAIC31)||defined( TEST_I2S ) || defined(TEST_MA5I) || defined(TEST_WM8753) || defined(TEST_WM8751)
	if (choose%11025==0){
		//16.934Mhz
		outpw(REG_APLLCON, 0x642d);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x300000);
		//outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x200000);
		//outpw(REG_APLLCON, 0x651A);//for bird
		//outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x200000);//for bird
	}
	else{
		//12.288Mhz
		outpw(REG_APLLCON, 0x6529);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x300000);
		//outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x100000);
		//outpw(REG_APLLCON, 0x6922);//for bird
		//outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x100000);//for bird
	}
#endif

#if defined (TEST_WM8978)
	if (choose%8000==0 || choose%12000==0){//24.576Mhz
		outpw(REG_APLLCON, 0x6529);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x100000);
	}
	else if (choose%11025==0){//22.579Mhz
		outpw(REG_APLLCON, 0x642d);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x200000);
	}
#endif
	

	printf("starting audio demo\n");
	audioSetPlayVolume(15,15);
	audioStartPlay(play_callback, choose, channels);
	
	

	nTime0 = sysGetTicks(TIMER1);
	while (1)
	{
		if (sysGetTicks(TIMER1) - nTime0 >= 100)
		{
			nTime0 = sysGetTicks(TIMER1);
			printf("H=%d, T=%d\n", _uPcmQHead/1024, _uPcmQTail/1024);

			
		}
		if (inpb(REG_COM_LSR) & 0x01)			/* check key press */
		{
			CHAR  cmd = inpb(REG_COM_RX);
			if (cmd=='1')
			{
				if (--nVol<=0)
					nVol=0;
				audioSetPlayVolume(nVol,nVol);
				printf("vol=%d\n",nVol);
			}else if (cmd=='2')
			{
				if (++nVol>=31)
					nVol=31;
				audioSetPlayVolume(nVol,nVol);
				printf("vol=%d\n",nVol);
			}
		}	

	}

	
	audioStopPlay();
	isnotfinish=1;
}




int main()
{
	INT			nLoop;
    WB_UART_T 	uart;
    AU_I2C_TYPE_T	i2cType;

	PCM_QUEUE_LEN = sizeof (_pucPcmQueue);

	sysInvalidCache();
//    sysEnableCache(CACHE_WRITE_THROUGH);
    sysEnableCache(CACHE_WRITE_BACK);

//	fsInitFileSystem();
//	fmiSetFMIReferenceClock(12000);
//	fmiInitDevice();


	outpw(REG_HICSR, inpw(REG_HICSR) & 0xFFFFFF7F);
	
	outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFFFFFFFC) | 0x1); /* APB 2:1 */
	outpw(REG_CLKDIV1, 0x00015555);		/* AHB2 2:1 */
	outpw(REG_SDTIME0, 0x80005829);		/* set DRAM timing */
	
	//outpw(REG_UPLLCON, 0x4541);		/* 156 MHz */
	//outpw(REG_UPLLCON, 0x453C);		/* 144 MHz */
	//outpw(REG_UPLLCON, 0x4521);		/* 132 MHz */
	//outpw(REG_UPLLCON, 0x4214);		/* 120 MHz */
	outpw(REG_UPLLCON, 0x6550);			/* 96 MHz */
	//outpw(REG_UPLLCON, 0x4A25);			/* 96 MHz for bird */

#ifdef TEST_AC97
	/* 24.576 MHz */
	outpw(REG_APLLCON, 0x6529);
	outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x100000);
#endif	
	
#if defined(TEST_MA5) || defined(TEST_MA3) || defined(TEST_W56964) || defined(TEST_MA5I) || defined(TEST_W5691)
	outpw(REG_LCM_DEV_CTRL, 0);		/* set VPOST/M80 arbitor */
	outpw(REG_LCM_DCCS, 0x8);		/* switch VPOST output enable */
	outpw(REG_LCM_DEV_CTRL, 0xE0);	/* set VPOST control to mcu type */
	outpw(REG_MISCR, 0x0);			/* set bridge mode-0 */
#endif

	/* 
	 * CLK select: SYSTEM_S from UPLL, AUDIO_S from APLL 
	 * SENSOR_S and VIDEO_S from crystal in 
	 */
	outpw(REG_CLKSEL, 0x364);
	
	for (nLoop = 0; nLoop < 0x1000; nLoop++)
		;

    uart.uiFreq = 12000000;
    //uart.uiFreq = 26000000;//for bird
    uart.uiBaudrate = 57600;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    sysInitializeUART(&uart);

	//fmiSetFMIReferenceClock(48000);
	//fmiSetFMIReferenceClock(12000);
//	fmiInitSDDevice();
//	Action_DIR("C:\\");

#ifdef TEST_ADDA
	audioEnable(AU_DEV_IIS, AU_DEV_ADC);
#endif	
#ifdef TEST_AC97
	audioEnable(AU_DEV_AC97, AU_DEV_AC97);
#endif	
#ifdef TEST_I2S
	audioEnable(AU_DEV_IIS, AU_DEV_IIS);
	//audioEnable(AU_DEV_WM8753, AU_DEV_WM8753);
#endif	
#ifdef TEST_WM8753
	audioEnable(AU_DEV_WM8753, AU_DEV_WM8753);
#endif
#ifdef TEST_WM8751
	audioEnable(AU_DEV_WM8751, AU_DEV_WM8751);
#endif

#ifdef TEST_WM8978
	audioEnable(AU_DEV_WM8978, AU_DEV_WM8978);
	//audioEnable(AU_DEV_WM8750, AU_DEV_WM8750);
#endif	

#ifdef TEST_TIAIC31
	audioEnable(AU_DEV_TIAIC31, AU_DEV_TIAIC31);
#endif

#ifdef TEST_W56964
	audioEnable(AU_DEV_W56964, AU_DEV_W56964);
#endif	

#ifdef TEST_W5691
	audioEnable(AU_DEV_W5691, AU_DEV_W5691);
#endif

#ifdef TEST_MA3
	audioEnable(AU_DEV_MA3, AU_DEV_MA3);
#endif	
#ifdef TEST_MA5
	audioEnable(AU_DEV_MA5, AU_DEV_MA5);
#endif	
#ifdef TEST_MA5I
	audioEnable(AU_DEV_MA5I, AU_DEV_MA5I);
#endif	

#if _DEMO_VER >= 0x17
	i2cType.bIsGPIO = TRUE;
	i2cType.uSDIN = 5;
	i2cType.uSCLK = 4;
#else
	i2cType.bIsGPIO = FALSE;
	i2cType.uSDIN = 1;
	i2cType.uSCLK = 0;
#endif

	audioSetI2CType(i2cType);
	audioRecConfigMono(0);

	//audioSetPlayBuff(0x100000, 8192);
	//audioSetRecBuff(0x110000, 8192);
	audioSetPlayBuff(0x10100000, 4096);
	audioSetRecBuff(0x100c0000, 4096);

	sysSetTimerReferenceClock (TIMER1, 12000000);
	//sysSetTimerReferenceClock (TIMER1, 26000000);//for bird
	sysStartTimer(TIMER1, 100, PERIODIC_MODE);
    audio_demo(AU_SAMPLE_RATE_44100,2);//ok

}



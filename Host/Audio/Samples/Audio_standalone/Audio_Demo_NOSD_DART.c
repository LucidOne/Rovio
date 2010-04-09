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
#include "wbfat.h"
#include "wb_fmi.h"


UINT32 PCM_QUEUE_LEN		=	(512*1024);


//#define printf sysPrintf



static UINT8  _pucPcmQueue [] = {
	#include "8k.dat"
};

volatile UINT32 _uPcmQHead, _uPcmQTail;
int isnotfinish = 1;



INT  play_callback(UINT8 *pucBuff, UINT32 uDataLen)
{
	INT		nLen;
	INT		idx;


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
	INT		hFile;
	INT		nReadBytes;
	INT		nTime0;
	INT 	nErr;

	_uPcmQHead = _uPcmQTail = 0;

	if (choose%11025==0){
		/* 16.934Mhz */
		//outpw(REG_APLLCON, 0x642D);
		//outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x300000);
		/* 22.579 MHz */
		outpw(REG_APLLCON, 0x652f);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x200000);
		//outpw(REG_APLLCON, 0x642d);
		//outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x200000);
	}
	else{
		/* 12.288Mhz */
		//outpw(REG_APLLCON, 0x6529);
		//outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x300000);
		/* 24.576 MHz */
		outpw(REG_APLLCON, 0x6730);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x100000);
		//outpw(REG_APLLCON, 0x6529);
		//outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x100000);
	}

	printf("starting audio demo\n");
	
	switch(choose){
	case 48000:audioStartPlay(play_callback, AU_SAMPLE_RATE_48000, channels);break;
	case 44100:audioStartPlay(play_callback, AU_SAMPLE_RATE_44100, channels);break;
	case 32000:audioStartPlay(play_callback, AU_SAMPLE_RATE_32000, channels);break;
	case 24000:audioStartPlay(play_callback, AU_SAMPLE_RATE_24000, channels);break;
	case 22050:audioStartPlay(play_callback, AU_SAMPLE_RATE_22050, channels);break;
	case 16000:audioStartPlay(play_callback, AU_SAMPLE_RATE_16000, channels);break;
	case 11025:audioStartPlay(play_callback, AU_SAMPLE_RATE_11025, channels);break;
	case 8000:audioStartPlay(play_callback, AU_SAMPLE_RATE_8000, channels);break;
	}

	audioSetPlayVolume(31,31);
#ifndef TEST_AC97
	//audioSetPlayVolume(31,31);
	//audioSetRecordVolume(30,30);
#endif	

	nTime0 = sysGetTicks(TIMER1);
	while (1)
	{
		if (sysGetTicks(TIMER1) - nTime0 >= 100)
		{
			nTime0 = sysGetTicks(TIMER1);
			printf("H=%d, T=%d\n", _uPcmQHead/1024, _uPcmQTail/1024);

			
		}

	}

	
	audioStopPlay();
	isnotfinish=1;
}


int main()
{
	INT			nLoop;
    WB_UART_T 	uart;

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
	
	outpw(REG_CLKDIV0, 0x31104000); /* APB 2:1 */
	outpw(REG_CLKDIV1, 0x00505555);		/* AHB2 2:1 */
	
	
	//outpw(REG_UPLLCON, 0x4541);		/* 156 MHz */
	//outpw(REG_UPLLCON, 0x453C);		/* 144 MHz */
	//outpw(REG_UPLLCON, 0x4521);		/* 132 MHz */
	//outpw(REG_UPLLCON, 0x4214);		/* 120 MHz */
	//outpw(REG_UPLLCON, 0x472f);			/* 96 MHz */
	outpw(REG_UPLLCON, 0x6550);			/* 96 MHz */
	





	/* 
	 * CLK select: SYSTEM_S from UPLL, AUDIO_S from APLL 
	 * SENSOR_S and VIDEO_S from crystal in 
	 */
	 for (nLoop = 0; nLoop < 0x1000; nLoop++)
		;
	outpw(REG_CLKSEL, 0x364);
	
	

    uart.uiFreq = 14318000;
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


	audioEnable(AU_DEV_AK4569, AU_DEV_AK4569);



	//audioSetPlayBuff(0x100000, 8192);
	//audioSetRecBuff(0x110000, 8192);
	audioSetPlayBuff(0x10100000, 4096);
	audioSetRecBuff(0x100c0000, 4096);

	sysSetTimerReferenceClock (TIMER1, 14318000);
	sysSetAIC2SWMode();
	sysStartTimer(TIMER1, 100, PERIODIC_MODE);
    

	audio_demo(8000,2);//ok

}



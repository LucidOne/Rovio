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
#include "resultcode.h"

UINT32 PCM_QUEUE_LEN		=	(256*1024);



#define printf sysPrintf
#define sysPrintf(...)



static UINT8  *_pucPcmQueue = (UINT8 *)(0x180000 | 0x10000000);


volatile UINT32 _uPcmQHead, _uPcmQTail;
int isnotfinish = 1;
void record_to_file(INT choose);

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
	INT8	bAsciiFileName [20];
	INT8	bUnicodeFileName [40];
	INT 	nRetValue;

	_uPcmQHead = _uPcmQTail = 0;

	switch(choose){
	case 48000:
	strcpy( bAsciiFileName,"C:\\48k.pcm");
	fsAsciiToUnicode(bAsciiFileName, bUnicodeFileName, TRUE);
	hFile = fsOpenFile(bUnicodeFileName, NULL, OP_READ);break;
	
	case 44100:
	
	strcpy(bAsciiFileName , "C:\\44.1k.pcm");
	fsAsciiToUnicode(bAsciiFileName, bUnicodeFileName, TRUE);
	hFile = fsOpenFile(bUnicodeFileName, NULL, OP_READ);break;
	
	
	
	case 32000:
	strcpy(bAsciiFileName , "C:\\32k.pcm");
	fsAsciiToUnicode(bAsciiFileName, bUnicodeFileName, TRUE);
	hFile = fsOpenFile(bUnicodeFileName, NULL, OP_READ);break;
	
	
	
	case 24000:
	strcpy(bAsciiFileName , "C:\\24k.pcm");
	fsAsciiToUnicode(bAsciiFileName, bUnicodeFileName, TRUE);
	hFile = fsOpenFile(bUnicodeFileName, NULL, OP_READ);break;
	
	
	
	case 22050:
	strcpy(bAsciiFileName , "C:\\22.05k.pcm");
	fsAsciiToUnicode(bAsciiFileName, bUnicodeFileName, TRUE);
	hFile = fsOpenFile(bUnicodeFileName, NULL, OP_READ);break;
	
	
	case 16000:
	
	strcpy(bAsciiFileName , "C:\\16k.pcm");
	fsAsciiToUnicode(bAsciiFileName, bUnicodeFileName, TRUE);
	hFile = fsOpenFile(bUnicodeFileName, NULL, OP_READ);break;
	
	case 11025:
	strcpy(bAsciiFileName , "C:\\11.025k.pcm");
	fsAsciiToUnicode(bAsciiFileName, bUnicodeFileName, TRUE);
	hFile = fsOpenFile(bUnicodeFileName, NULL, OP_READ);break;
	
	
	case 8000:
	strcpy(bAsciiFileName , "C:\\8k.pcm");
	fsAsciiToUnicode(bAsciiFileName, bUnicodeFileName, TRUE);
	hFile = fsOpenFile(bUnicodeFileName, NULL, OP_READ);break;
	
	
	}
	
	if (hFile < 0)
	{
		printf("Can't open playback file%d!\n",hFile);
		return;
	}

	if (choose%11025==0){
		/* 16.934Mhz */
		//outpw(REG_APLLCON, 0x642D);
		//outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x300000);
		/* 22.579 MHz */
		outpw(REG_APLLCON, 0x652f);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x200000);
	}
	else{
		/* 12.288Mhz */
		//outpw(REG_APLLCON, 0x6529);
		//outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x300000);
		/* 24.576 MHz */
		outpw(REG_APLLCON, 0x6730);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x100000);
	}


	/* read PCM data to PCM buffer */
	printf("Read %d KB to PCM data buffer...\n", PCM_QUEUE_LEN/1024);
	
	nErr = fsReadFile(hFile, _pucPcmQueue, 256*1024, &nReadBytes);
	if (nErr<0)
		printf("err num=%d\n",nErr);
	//memset(_pucPcmQueue,0,PCM_QUEUE_LEN);

	printf("OK\n");
	
	switch(choose){
	case 48000:nRetValue = audioStartPlay(play_callback, AU_SAMPLE_RATE_48000, channels);break;
	case 44100:nRetValue = audioStartPlay(play_callback, AU_SAMPLE_RATE_44100, channels);break;
	case 32000:nRetValue = audioStartPlay(play_callback, AU_SAMPLE_RATE_32000, channels);break;
	case 24000:nRetValue = audioStartPlay(play_callback, AU_SAMPLE_RATE_24000, channels);break;
	case 22050:nRetValue = audioStartPlay(play_callback, AU_SAMPLE_RATE_22050, channels);break;
	case 16000:nRetValue = audioStartPlay(play_callback, AU_SAMPLE_RATE_16000, channels);break;
	case 11025:nRetValue = audioStartPlay(play_callback, AU_SAMPLE_RATE_11025, channels);break;
	case 8000:nRetValue = audioStartPlay(play_callback, AU_SAMPLE_RATE_8000, channels);break;
	}
	


	nRetValue = audioSetPlayVolume(15,15);
	
	
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

		if ((_uPcmQTail > _uPcmQHead) ||
			((_uPcmQTail < _uPcmQHead) && (_uPcmQHead - _uPcmQTail > 8192)))
		{
			fsReadFile(hFile, &_pucPcmQueue[_uPcmQTail], 8192, &nReadBytes);
			
			if (nReadBytes > 0)
				_uPcmQTail = (_uPcmQTail + nReadBytes) % PCM_QUEUE_LEN;
			//else
				//break;
		}

	}
	fsCloseFile(hFile);
	
	audioStopPlay();
	isnotfinish=1;
}


void  audio_loopback_demo(INT choose,INT ch)
{
	INT		nTime0;

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
	}	printf("Start record...\n");
	
	while(1){
		audioStartRecord(record_callback, choose, ch);
		audioSetRecordVolume(31,31);
	

		nTime0 = sysGetTicks(TIMER1);
		while (_uPcmQTail <= PCM_QUEUE_LEN - 4096){
			if (sysGetTicks(TIMER1) - nTime0 >= 100)
			{
				nTime0 = sysGetTicks(TIMER1);
				printf("H=%d, T=%d\n", _uPcmQHead/1024, _uPcmQTail/1024);
			}
		}
		audioStopRecord();

		printf("Start play...\n");
		audioStartPlay(play_callback, choose, ch);
		audioSetPlayVolume(20,20);

		while (_uPcmQHead <= PCM_QUEUE_LEN - 4096)
		{
			if (sysGetTicks(TIMER1) - nTime0 >= 100)
			{
				nTime0 = sysGetTicks(TIMER1);
				printf("H=%d, T=%d\n", _uPcmQHead/1024, _uPcmQTail/1024);
			}
		}
		audioStopPlay();
		_uPcmQTail = _uPcmQHead = 0;
	}
}



void record_to_file(INT choose)
{
	INT		hFile;
	INT		nWriteBytes, nTotalBytes = 0;
	INT		nTime0;
	INT8	bAsciiFileName[20] = "C:\\record.pcm";
	INT8	bUnicodeFileName[40];
	INT		ntick0,temp;
	_uPcmQHead = _uPcmQTail = 0;
	fsAsciiToUnicode(bAsciiFileName, bUnicodeFileName, TRUE);
	hFile = fsOpenFile(bUnicodeFileName,NULL, O_CREATE);
	if (hFile < 0)
	{
		printf("Can't open playback file%d!\n",hFile);
		return;
	}
	printf("Start record...\n");
#if 0	
#if defined( TEST_I2S ) || defined (TEST_WM8978) || defined (TEST_UDA1345TS)
	if (choose%11025==0){
		/* 16.934Mhz */
		outpw(REG_APLLCON, 0x642D);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x300000);
	}
	else{
		/* 12.288Mhz */
		outpw(REG_APLLCON, 0x6529);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x300000);
	}
#endif
#endif	
	audioStartRecord(record_callback, choose, 1);
	ntick0 = sysGetTicks(TIMER1);
	audioSetRecordVolume(30,30);
	
	nTime0 = sysGetTicks(TIMER1);
	
	while (1)
	{
		if (sysGetTicks(TIMER1) - nTime0 >= 100)
		{
			nTime0 = sysGetTicks(TIMER1);
			printf("H=%d, T=%d\n", _uPcmQHead/1024, _uPcmQTail/1024);

			
		}
		
		if ((_uPcmQHead > _uPcmQTail) ||
			((_uPcmQHead < _uPcmQTail) && (_uPcmQTail - _uPcmQHead > 8192)))
		{
			fsWriteFile(hFile, &_pucPcmQueue[_uPcmQHead], 8192, &nWriteBytes);
			
			if (nWriteBytes > 0)
				_uPcmQHead = (_uPcmQHead + nWriteBytes) % PCM_QUEUE_LEN;
			else
				break;
				
			nTotalBytes += nWriteBytes;
			temp = sysGetTicks(TIMER1);
			if (temp - ntick0  >= 30000)
				break;
		}
	}
	fsCloseFile(hFile);
}

int main()
{
	INT nRetValue = 0,nLoop;
	WB_UART_T 	uart;

	
	sysInvalidCache();
    sysEnableCache(CACHE_WRITE_BACK);

	/* 
	 * CLK select: SYSTEM_S from UPLL, AUDIO_S from APLL 
	 * SENSOR_S and VIDEO_S from crystal in 
	 */

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
	
	audioEnable(AU_DEV_AK4569, AU_DEV_AK4569);


	
	//audioSetPlayBuff(0x100000, 8192);
	//audioSetRecBuff(0x110000, 8192);
	nRetValue = audioSetPlayBuff(0x1005a720, 4096);
	nRetValue = audioSetRecBuff(0x100c0000, 4096);

	

	sysSetTimerReferenceClock (TIMER1, 14318000);
	sysStartTimer(TIMER1, 100, PERIODIC_MODE);
    



//	audio_demo(48000,2);//ok
//	audio_demo(8000,1);//ok
//	audio_demo(8000,2);//ok
//    audio_demo(44100,2);//ok
//    audio_demo(32000,2);//ok
//
//
//
  //audio_demo(24000,2);//ok
//  audio_demo(22050,2);//ok
//
// audio_demo(16000,2);//ok
//
//	audio_demo(11025,2);//ok
//    
//  audio_demo(8000,2);//ok
//	audio_demo(44100,2);//ok
///	audio_loopback_demo(AU_SAMPLE_RATE_48000,1);
//	audio_loopback_demo(AU_SAMPLE_RATE_44100,1);
//	audio_loopback_demo(AU_SAMPLE_RATE_32000,1);
//	audio_loopback_demo(AU_SAMPLE_RATE_24000,1);
//	audio_loopback_demo(AU_SAMPLE_RATE_22050,1);
//	audio_loopback_demo(AU_SAMPLE_RATE_16000,1);
//	audio_loopback_demo(AU_SAMPLE_RATE_11025,1);
	audio_loopback_demo(AU_SAMPLE_RATE_8000,2);

//   record_to_file(AU_SAMPLE_RATE_24000);
}



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

#define USEUART 1
#define _USE_SD 1
//#define TEST_AC97
//#define TEST_I2S
//#define TEST_UDA1345TS
//#define TEST_TIAIC31

//#define TEST_W56964
//#define TEST_ADDA
//#define TEST_MA3
//#define TEST_MA5
//#define TEST_MA5I
//#define TEST_MA5SI
//#define TEST_WM8753
#define TEST_WM8978
//#define TEST_AK4569
//#define TEST_WM8751
//#define TEST_W56964I

#define _DEMO_VER 0x17

UINT32 PCM_QUEUE_LEN		=	(256*1024);



#if USEUART
#define printf sysPrintf
//#define sysPrintf(...)
#endif



static UINT8  *_pucPcmQueue = (UINT8 *)(0x180000 | 0x10000000);
//static UINT8  _TempQueue[256*1024];
//static UINT8  _pucPcmQueue [] = {
//	#include "8k.dat"
//};

volatile UINT32 _uPcmQHead, _uPcmQTail;

void record_to_file(INT choose,INT);
void play_from_file(INT choose,INT);
/*
UINT8 _sine_wave[192] = 
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
};	
*/

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


void audio_demo(int choose, int channels, int format)
{
	INT		hFile;
	INT		nReadBytes;
	INT		nTime0;
	INT 	nErr;
	INT8	bAsciiFileName [20];
	INT8	bUnicodeFileName [40];
	INT		ntemp,nVol=12 ;
	INT 	nSPToggle=0;
	INT 	nHPToggle=0;

	_uPcmQHead = _uPcmQTail = 0;

	switch(choose){
	case 48000:
	strcpy( bAsciiFileName,"C:\\48k.pcm");break;
	case 44100:
	strcpy(bAsciiFileName , "C:\\44.1k.pcm");break;
	case 32000:
	strcpy(bAsciiFileName , "C:\\32k.pcm");break;
	case 24000:
	strcpy(bAsciiFileName , "C:\\24k.pcm");break;
	case 22050:
	strcpy(bAsciiFileName , "C:\\22.05k.pcm");break;
	case 16000:
	strcpy(bAsciiFileName , "C:\\16k.pcm");break;
	case 12000:
	strcpy(bAsciiFileName , "C:\\12k.pcm");break;
	case 11025:
	strcpy(bAsciiFileName , "C:\\11.025k.pcm");break;
	case 8000:
	strcpy(bAsciiFileName , "C:\\8k.pcm");break;
	}
	fsAsciiToUnicode(bAsciiFileName, bUnicodeFileName, TRUE);
	hFile = fsOpenFile(bUnicodeFileName, NULL, OP_READ);
	if (hFile < 0)
	{
		printf("Can't open playback file0x%x!\n",hFile);
		//return;
	}

#if defined(TEST_W56964I) || defined( TEST_I2S ) || defined(TEST_MA5I) || defined(TEST_WM8753) || defined(TEST_WM8751) || defined(TEST_MA5SI)  || defined(TEST_UDA1345TS) || defined(TEST_TIAIC31)
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

#if defined (TEST_WM8978)
	/* for eCos version audio lib*/
#if 0	
	if (choose==32000)//8.192Mhz
	{
		outpw(REG_APLLCON, 0x6529);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x500000);
	}
	else if (choose%8000==0 || choose%12000==0){//12.288Mhz
		outpw(REG_APLLCON, 0x6529);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x300000);
	}
	else if (choose%11025==0){//11.289Mhz
		outpw(REG_APLLCON, 0x652f);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x400000);
	}
#else
	if (choose%8000==0 || choose%12000==0){//24.576Mhz
		outpw(REG_APLLCON, 0x6529);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x100000);
	}
	else if (choose%11025==0){//22.579Mhz
		outpw(REG_APLLCON, 0x642d);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x200000);
	}
#endif	
#endif

	/* read PCM data to PCM buffer */
	printf("Read %d KB to PCM data buffer...\n", PCM_QUEUE_LEN/1024);
	
	nErr = fsReadFile(hFile, _pucPcmQueue, 256*1024, &nReadBytes);
	if (nErr<0)
		printf("err num=%d\n",nErr);
	//memset(_pucPcmQueue,0,PCM_QUEUE_LEN);

	printf("OK\n");
	
	audioStartPlay(play_callback, choose, channels);
	//tiaic31StartPlay(play_callback, choose, channels,format);
		
	audioSetPlayVolume(nVol,nVol);
	
	
		
	ntemp = nTime0 = sysGetTicks(TIMER1);
	
	
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
			if (sysGetTicks(TIMER1) - ntemp >=1900)
				break;
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
			}else if (cmd=='3')
			{
				if (nSPToggle)
				{
					audioSelectOutputPath(OUT_PATH_SP,TRUE);
					nSPToggle = 0;
				}else
				{
					audioSelectOutputPath(OUT_PATH_SP,FALSE);
					nSPToggle = 1;
				}
				
					
			}
			else if (cmd=='4')
			{
				if (nHPToggle)
				{
					audioSelectOutputPath(OUT_PATH_HP,TRUE);
					nHPToggle = 0;
				}else
				{
					audioSelectOutputPath(OUT_PATH_HP,FALSE);
					nHPToggle = 1;
				}
			}
		}
		

	}
	fsCloseFile(hFile);
	
	audioStopPlay();

}

void audio_PMP_record_demo()
{
	INT 	nMICLToggle=0;
	INT 	nLINEIN1Toggle=0;
	INT 	nLINEIN2Toggle=0;
	INT 	nSPToggle=0;
	INT 	nHPToggle=0;
	

	while(1)
	{
		INT8 choose;
		
		printf("1.Enable bypass\n");
		printf("2.Disable bypass\n");
		printf("3.Start record 10sec PCM to file\n");
		printf("4.Play from file\n");
		printf("5.En/Disable Path\n");
#if USEUART    	
   		choose = sysGetChar();
#else
		scanf("%c",&choose);
#endif
		switch (choose)
		{
			case '1':audioSetPlayVolume(20,20);
					//WM8978_Bypass_Enable();
					audioBypassEnable();
					break;
			case '2'://WM8978_Bypass_Disable();
					audioBypassDisable();
					break;
			case '3':record_to_file(8000,1);
					break;
			case '4':play_from_file(8000,1);
					break;
			case '5':
					{
						
						
						printf("select output path\n");
						printf("1.Speaker:%d\n",nSPToggle);
						printf("2.Head phone:%d\n",nHPToggle);
#if USEUART 		   	
   						choose = sysGetChar();
#else       		
						scanf("%c",&choose);
#endif      		
						if (choose == '1')
						{
							if (nSPToggle)
								nSPToggle = 0;
							else
								nSPToggle = 1;
							audioSelectOutputPath(OUT_PATH_SP,nSPToggle);
						}
						if (choose == '2')
						{
							if (nHPToggle)
								nHPToggle = 0;
							else
								nHPToggle = 1;
							audioSelectOutputPath(OUT_PATH_HP,nHPToggle);
						}
						
					}
					{
						
						
						printf("select input path\n");
						printf("1.MicL IN:%d\n",nMICLToggle);
						printf("2.FM IN:%d\n",nLINEIN1Toggle);
						printf("3.LINE IN:%d\n",nLINEIN2Toggle);
#if USEUART 		   	
   						choose = sysGetChar();
#else       		
						scanf("%c",&choose);
#endif      		
						if (choose=='1')
						{
							if (nMICLToggle)
								nMICLToggle = 0;
							else
								nMICLToggle = 1;
							audioSelectInputPath(IN_PATH_MICL,nMICLToggle);
						}
						if (choose=='2')
						{
							if (nLINEIN1Toggle)
								nLINEIN1Toggle = 0;
							else
								nLINEIN1Toggle = 1;
							audioSelectInputPath(IN_PATH_LINEIN1,nLINEIN1Toggle);
						}
						if (choose=='3')
						{
							if (nLINEIN2Toggle)
								nLINEIN2Toggle = 0;
							else
								nLINEIN2Toggle = 1;
							audioSelectInputPath(IN_PATH_LINEIN2,nLINEIN2Toggle);
						}
						
						
						
					}
					break;
			default: break;	
						
		}
	}
	
}
void  audio_loopback_demo(INT choose,INT ch,INT format)
{
	INT		nTime0,nTime1,nVol=20;

	_uPcmQHead = _uPcmQTail = 0;


#if defined( TEST_I2S ) || defined(TEST_UDA1345TS)
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

#if defined (TEST_WM8978)
#if 0
	if (choose==32000)//8.192Mhz
	{
		outpw(REG_APLLCON, 0x6529);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x500000);
	}
	else if (choose%8000==0 || choose%12000==0){//12.288Mhz
		outpw(REG_APLLCON, 0x6529);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x300000);
	}
	else if (choose%11025==0){//11.289Mhz
		outpw(REG_APLLCON, 0x652f);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x400000);
	}
#else

	if (choose%8000==0 || choose%12000==0){//24.576Mhz
		outpw(REG_APLLCON, 0x6529);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x100000);
	}
	else if (choose%11025==0){//22.579Mhz
		outpw(REG_APLLCON, 0x642d);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x200000);
	}
#endif
#endif

#if 1
while(1){
	_uPcmQTail = _uPcmQHead = 0;
	printf("Start record...\n");
	audioStartRecord(record_callback, choose, ch);
	audioSetRecordVolume(nVol,nVol);



	nTime1 = nTime0 = sysGetTicks(TIMER1);

	while (_uPcmQTail <  PCM_QUEUE_LEN - 4096)
	{
		if (inpb(REG_COM_LSR) & 0x01)			/* check key press */
		{
			CHAR  cmd = inpb(REG_COM_RX);
			if (cmd=='1')
			{
				if (--nVol<=0)
					nVol=0;
				audioSetRecordVolume(nVol,nVol);
				printf("vol=%d\n",nVol);
			}else if (cmd=='2')
			{
				if (++nVol>=31)
					nVol=31;
				audioSetRecordVolume(nVol,nVol);
				printf("vol=%d\n",nVol);
			}
		}	
		if (sysGetTicks(TIMER1) - nTime0 >= 100)
		{
			nTime0 = sysGetTicks(TIMER1);
			printf("H=%d, T=%d\n", _uPcmQHead/1024, _uPcmQTail/1024);
		}
	}
	

	audioStopRecord();




	printf("Start play...\n");
	audioStartPlay(play_callback, choose, ch);
	audioSetPlayVolume(30,30);

	nTime1 = nTime0 = sysGetTicks(TIMER1);
	
	while (_uPcmQHead < PCM_QUEUE_LEN - 4096 )
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
#endif
#if 1
	audioStartRecord(record_callback, choose, ch);
	audioSetRecordVolume(31,31);
	
	
	
	while (_uPcmQTail- _uPcmQHead <= 1024);
	
	audioStartPlay(play_callback, choose, ch);
	audioSetPlayVolume(20,20);
	

	//while(audioWriteData(50,0x1)!=0);
	//while(audioWriteData(51,0x1)!=0);


	
	
	while(1){
		if (sysGetTicks(TIMER1) - nTime0 >= 100)
		{
			nTime0 = sysGetTicks(TIMER1);
			printf("H=%d, T=%d\n", _uPcmQHead/1024, _uPcmQTail/1024);
		}
	}
#endif

}



void record_to_file(INT choose,INT ch)
{
	INT		hFile;
	INT		nWriteBytes, nTotalBytes = 0;
	INT		nTime0;
	INT8	bAsciiFileName[20] = "c:\\record.pcm";
	INT8	bUnicodeFileName[40];
	INT		ntick0;

	_uPcmQHead = _uPcmQTail = 0;
	fsAsciiToUnicode(bAsciiFileName, bUnicodeFileName, TRUE);
	hFile = fsOpenFile(bUnicodeFileName,NULL, O_CREATE);

	if (hFile < 0)
	{
		printf("Can't open playback file%d!\n",hFile);
		return;
	}
	printf("Start record...\n");

#if defined( TEST_I2S ) || defined (TEST_UDA1345TS)
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

#if defined (TEST_WM8978)
#if 0
	if (choose==32000)//8.192Mhz
	{
		outpw(REG_APLLCON, 0x6529);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x500000);
	}
	else if (choose%8000==0 || choose%12000==0){//12.288Mhz
		outpw(REG_APLLCON, 0x6529);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x300000);
	}
	else if (choose%11025==0){//11.289Mhz
		outpw(REG_APLLCON, 0x652f);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x400000);
	}
#else

	if (choose%8000==0 || choose%12000==0){//24.576Mhz
		outpw(REG_APLLCON, 0x6529);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x100000);
	}
	else if (choose%11025==0){//22.579Mhz
		outpw(REG_APLLCON, 0x642d);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x200000);
	}
#endif
#endif
	audioRecConfigMono(1);
	audioStartRecord(record_callback, choose, ch);
	
	//audioSetRecordVolume(31,31);
	nTime0 = ntick0 = sysGetTicks(TIMER1);
	
	while (1)
	{
		if (sysGetTicks(TIMER1) - nTime0 >= 100)
		{
			nTime0 = sysGetTicks(TIMER1);
			printf("H=%d, T=%d\n", _uPcmQHead/1024, _uPcmQTail/1024);

			
		}
		
		if ((_uPcmQHead > _uPcmQTail) ||
			((_uPcmQHead < _uPcmQTail) && (_uPcmQTail - _uPcmQHead > 64*1024)))
		{

			fsWriteFile(hFile, &_pucPcmQueue[_uPcmQHead], 64*1024, &nWriteBytes);
			
			if (nWriteBytes > 0)
				_uPcmQHead = (_uPcmQHead + nWriteBytes) % PCM_QUEUE_LEN;
			else
				break;
				
			

			if (sysGetTicks(TIMER1) - ntick0  >= 1000)
				break;
		}
	}
	fsCloseFile(hFile);
	audioStopRecord();
}

void play_from_file(INT choose,INT ch)
{
	INT		hFile;
	INT		nReadBytes;
	INT		nTime0;
	INT 	nErr;
	INT8	bAsciiFileName [20];
	INT8	bUnicodeFileName [40];
	INT		ntemp,nVol=20 ;
	INT 	nSPToggle=0;
	INT 	nHPToggle=0;

	_uPcmQHead = _uPcmQTail = 0;

	strcpy(bAsciiFileName , "c:\\record.pcm");

	fsAsciiToUnicode(bAsciiFileName, bUnicodeFileName, TRUE);
	hFile = fsOpenFile(bUnicodeFileName, NULL, OP_READ);
	if (hFile < 0)
	{
		printf("Can't open playback file0x%x!\n",hFile);
		//return;
	}
#if defined (TEST_WM8978)
	/* for eCos version audio lib*/
#if 0	
	if (choose==32000)//8.192Mhz
	{
		outpw(REG_APLLCON, 0x6529);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x500000);
	}
	else if (choose%8000==0 || choose%12000==0){//12.288Mhz
		outpw(REG_APLLCON, 0x6529);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x300000);
	}
	else if (choose%11025==0){//11.289Mhz
		outpw(REG_APLLCON, 0x652f);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x400000);
	}
#else
	if (choose%8000==0 || choose%12000==0){//24.576Mhz
		outpw(REG_APLLCON, 0x6529);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x100000);
	}
	else if (choose%11025==0){//22.579Mhz
		outpw(REG_APLLCON, 0x642d);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x200000);
	}
#endif	
#endif

	/* read PCM data to PCM buffer */
	printf("Read %d KB to PCM data buffer...\n", PCM_QUEUE_LEN/1024);
	
	nErr = fsReadFile(hFile, _pucPcmQueue, 256*1024, &nReadBytes);
	if (nErr<0)
		printf("err num=%d\n",nErr);
	//memset(_pucPcmQueue,0,PCM_QUEUE_LEN);

	printf("OK\n");
	
	audioStartPlay(play_callback, choose, ch);
	//tiaic31StartPlay(play_callback, choose, channels,format);
		
	audioSetPlayVolume(nVol,nVol);
	
	
		
	ntemp = nTime0 = sysGetTicks(TIMER1);
	
	
	while (1)
	{
		if (sysGetTicks(TIMER1) - nTime0 >= 100)
		{
			nTime0 = sysGetTicks(TIMER1);
			printf("H=%d, T=%d\n", _uPcmQHead/1024, _uPcmQTail/1024);
			
		}

		if ((_uPcmQTail > _uPcmQHead) ||
			((_uPcmQTail < _uPcmQHead) && (_uPcmQHead - _uPcmQTail > 64*1024)))
		{
			fsReadFile(hFile, &_pucPcmQueue[_uPcmQTail], 64*1024, &nReadBytes);
			
			if (nReadBytes > 0)
				_uPcmQTail = (_uPcmQTail + nReadBytes) % PCM_QUEUE_LEN;
			if (sysGetTicks(TIMER1) - ntemp >=1000)
				break;
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
			}else if (cmd=='3')
			{
				if (nSPToggle)
				{
					audioSelectOutputPath(OUT_PATH_SP,TRUE);
					nSPToggle = 0;
				}else
				{
					audioSelectOutputPath(OUT_PATH_SP,FALSE);
					nSPToggle = 1;
				}
				
					
			}
			else if (cmd=='4')
			{
				if (nHPToggle)
				{
					audioSelectOutputPath(OUT_PATH_HP,TRUE);
					nHPToggle = 0;
				}else
				{
					audioSelectOutputPath(OUT_PATH_HP,FALSE);
					nHPToggle = 1;
				}
			}
		}
		

	}
	fsCloseFile(hFile);
	
	audioStopPlay();

}

static INT  Action_DIR(CHAR *szPath)
{
	INT				i, nStatus;
	CHAR			szMainName[12], szExtName[8], *pcPtr;
	FILE_FIND_T  	tFileInfo;
	UINT32 			uBlockSize, uFreeSize, uDiskSize;

	nStatus = fsFindFirst(szPath, "*.*",&tFileInfo);
	if (nStatus < 0)
		return nStatus;

	do 
	{
		pcPtr = tFileInfo.szShortName;
		if ((tFileInfo.ucAttrib & A_DIR) && 
			(!strcmp(pcPtr, ".") || !strcmp(pcPtr, "..")))
			strcat(tFileInfo.szShortName, ".");

		memset(szMainName, 0x20, 9);
		szMainName[8] = 0;
		memset(szExtName, 0x20, 4);
		szExtName[3] = 0;
		i = 0;
		while (*pcPtr && (*pcPtr != '.'))
			szMainName[i++] = *pcPtr++;
		if (*pcPtr++)
		{
			i = 0;
			while (*pcPtr)
				szExtName[i++] = *pcPtr++;
		}

		if (tFileInfo.ucAttrib & A_DIR)
			printf("%s %s      <DIR>  %02d-%02d-%04d  %02d:%02d  %s\n",
						szMainName, szExtName, 
						tFileInfo.ucWDateMonth, tFileInfo.ucWDateDay, (tFileInfo.ucWDateYear+80)%100 ,
						tFileInfo.ucWTimeHour, tFileInfo.ucWTimeMin);
		else
			printf("%s %s %10d  %02d-%02d-%04d  %02d:%02d  %s\n",
						szMainName, szExtName, (UINT32)tFileInfo.nFileSize,
						tFileInfo.ucWDateMonth, tFileInfo.ucWDateDay, (tFileInfo.ucWDateYear+80)%100 ,
						tFileInfo.ucWTimeHour, tFileInfo.ucWTimeMin);
	   
	} while (!fsFindNext(&tFileInfo));
	
	fsFindClose(&tFileInfo);
	
	fsDiskFreeSpace('C', &uBlockSize, &uFreeSize, &uDiskSize);
	
	printf("Disk Size: %d Kbytes, Free Space: %d KBytes\n", uDiskSize, uFreeSize);

	return 0;
}


int main()
{

	FMI_CARD_DETECT_T card;
	AU_I2C_TYPE_T	i2cType;
	
	//_pucPcmQueue = _TempQueue;
	
	outpw(REG_HICSR, inpw(REG_HICSR) & 0xFFFFFF7F);
	
//	PCM_QUEUE_LEN = sizeof (_pucPcmQueue);
	wb702_STS_READY();
	/* start timer0 for FMI init */
	sysConfiguration();
	
	
#if 1	// demo
	card.uCard = FMI_SD_CARD;					// card type
#if _DEMO_VER >= 0x17
	card.uGPIO = 15;					// card detect GPIO pin V1.7
#else 
	card.uGPIO = 4;					// card detect GPIO pin
#endif
	card.uWriteProtect = 16;					// card detect GPIO pin
	card.uInsert = 0;				// 0/1 which one is insert
	card.nPowerPin = 12;				// card power pin, -1 means no power pin
	card.bIsTFlashCard = FALSE;
#endif
#if 0	// Module
	card.uCard = FMI_SD_CARD;		// card type
	card.uGPIO = 17;				// card detect GPIO pin
	card.uWriteProtect = 6;			// card detect GPIO pin
	card.uInsert = 0;				// 0/1 which one is insert
	card.nPowerPin = -1;			// card power pin, -1 means no power pin
	card.bIsTFlashCard = FALSE;
#endif
	
#if _USE_SD	
	fmiSetCardDetection(&card);
	fsInitFileSystem();
	fmiSetFMIReferenceClock(48000);
	fmiInitDevice();
	fmiInitSDDevice();
#endif
	
#ifdef TEST_AC97
	/* 24.576 MHz */
	outpw(REG_APLLCON, 0x6529);
	outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x100000);
#endif	
	
#if defined(TEST_W56964I) || defined(TEST_MA5) || defined(TEST_MA3) || defined(TEST_W56964) || defined(TEST_MA5I) || defined(TEST_MA5SI)
	outpw(REG_LCM_DEV_CTRL, 0);		/* set VPOST/M80 arbitor */
	outpw(REG_LCM_DCCS, 0x8);		/* switch VPOST output enable */
	outpw(REG_LCM_DEV_CTRL, 0xE0);	/* set VPOST control to mcu type */
	outpw(REG_MISCR, 0x0);			/* set bridge mode-0 */
#endif

	/* 
	 * CLK select: SYSTEM_S from UPLL, AUDIO_S from APLL 
	 * SENSOR_S and VIDEO_S from crystal in 
	 */

	//fmiSetFMIReferenceClock(48000);
	//fmiSetFMIReferenceClock(12000);
	
	

#ifdef TEST_ADDA
	audioEnable(AU_DEV_WM8978, AU_DEV_ADC);
#endif	
#ifdef TEST_AC97
	audioEnable(AU_DEV_AC97, AU_DEV_AC97);
#endif	
#ifdef TEST_I2S
	audioEnable(AU_DEV_IIS, AU_DEV_IIS);
	audioSetPureI2SMCLKType(I2S_MCLK_TYPE_NORMAL);
#endif
#ifdef TEST_UDA1345TS
	audioEnable(AU_DEV_UDA1345TS, AU_DEV_UDA1345TS);
#endif

#ifdef TEST_TIAIC31
	audioEnable(AU_DEV_TIAIC31, AU_DEV_TIAIC31);
	//audioEnable(AU_DEV_IIS_TIAIC31, AU_DEV_IIS_TIAIC31);
#endif	

#ifdef TEST_WM8753
	audioEnable(AU_DEV_WM8753, AU_DEV_WM8753);
#endif
#ifdef TEST_WM8751
	audioEnable(AU_DEV_WM8751, AU_DEV_WM8751);
#endif

#ifdef TEST_WM8978
	audioEnable(AU_DEV_WM8978, AU_DEV_WM8978);
	//audioEnable(AU_DEV_WM8978, AU_DEV_ADC);
	//audioEnable(AU_DEV_WM8750, AU_DEV_WM8750);
#endif	

#ifdef TEST_W56964
	audioEnable(AU_DEV_W56964, AU_DEV_W56964);
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
#ifdef TEST_W56964I
	audioEnable(AU_DEV_W56964I, AU_DEV_W56964I);
#endif
#ifdef TEST_MA5SI
	audioEnable(AU_DEV_MA5SI, AU_DEV_MA5SI);
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
	
	//audioSetOutputPath(TRUE,FALSE);
	
	//audioSetPlayBuff(0x100000, 8192);
	//audioSetRecBuff(0x110000, 8192);
	audioSetPlayBuff(0x10050000, 8192);
	audioSetRecBuff(0x100c0000, 8192);

	

	sysSetTimerReferenceClock (TIMER1, 12000000);
	//sysSetTimerReferenceClock (TIMER1, 26000000);//for bird
	sysStartTimer(TIMER1, 100, PERIODIC_MODE);


#if 0
    {
    	INT SR,channel,format;
    	INT8 choose;
repeat:    	
    	printf("select data format:\n");
    	printf("1.I2S format\n");
    	printf("2.MSB Justified format\n");
#if USEUART    	
    	choose = sysGetChar();
#else
		scanf("%c",&choose);
#endif
		switch(choose)
		{
			case '1':
				format = IIS_FORMAT;
				break;
			case '2':
				format = MSB_FORMAT;
				break;
			default:
				break;
		}
		
		printf("select num of channel:\n");
    	printf("1.mono\n");
    	printf("2.stereo\n");
#if USEUART    	
    	choose = sysGetChar();
#else
		scanf("%c",&choose);
#endif
		switch(choose)
		{
			case '1':
				channel = 1;
				break;
			case '2':
				channel = 2;
				break;
			default:
				break;
		}
		
		printf("select sampling rate:\n");
    	printf("1.8k\n");
    	printf("2.11.025k\n");
    	printf("3.12k\n");
    	printf("4.16k\n");
    	printf("5.22.05k\n");
    	printf("6.24k\n");
    	printf("7.32k\n");
    	printf("8.44.1k\n");
    	printf("9.48k\n");
#if USEUART    	
    	choose = sysGetChar();
#else
		scanf("%c",&choose);
#endif
		switch(choose)
		{
			case '1':
				SR = AU_SAMPLE_RATE_8000;break;
			case '2':
				SR = AU_SAMPLE_RATE_11025;break;
			case '3':
				SR = AU_SAMPLE_RATE_12000;break;
			case '4':
				SR = AU_SAMPLE_RATE_16000;break;
			case '5':
				SR = AU_SAMPLE_RATE_22050;break;				
			case '6':
				SR = AU_SAMPLE_RATE_24000;break;
			case '7':
				SR = AU_SAMPLE_RATE_32000;break;
			case '8':
				SR = AU_SAMPLE_RATE_44100;break;				
			case '9':
				SR = AU_SAMPLE_RATE_48000;break;
			default:
				break;
		}
		
		//audio_demo(SR,channel,format);//ok
		audio_loopback_demo(SR,channel,format);
		goto repeat;
		
		
		//record_to_file(8000);
    }
#endif	
	//audio_PMP_record_demo();
	
	while(1)
 		audio_demo(AU_SAMPLE_RATE_44100,2,IIS_FORMAT);
	//audio_loopback_demo(8000,1,IIS_FORMAT);
	//audioSetPlayVolume(15,15);
	//WM8978_FMBypass_Setup();
	//while(1);

///	audio_loopback_demo(AU_SAMPLE_RATE_48000,1);
//	audio_loopback_demo(AU_SAMPLE_RATE_44100,1);
//	audio_loopback_demo(AU_SAMPLE_RATE_32000,1);
//	audio_loopback_demo(AU_SAMPLE_RATE_24000,1);
//	audio_loopback_demo(AU_SAMPLE_RATE_22050,1);
//	audio_loopback_demo(AU_SAMPLE_RATE_16000,1);
//	audio_loopback_demo(AU_SAMPLE_RATE_11025,1);
//	audio_loopback_demo(AU_SAMPLE_RATE_8000,1);

//   record_to_file(AU_SAMPLE_RATE_24000);
}



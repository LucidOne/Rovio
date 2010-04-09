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

//#define TEST_AC97
#define TEST_I2S
//#define TEST_W56964
//#define TEST_ADDA
//#define TEST_MA3
//#define TEST_MA5
//#define TEST_MA5I
//#define TEST_MA5SI
//#define TEST_WM8753
//#define TEST_WM8978
//#define TEST_WM8751


UINT32 PCM_QUEUE_LEN		=	(256*1024);

//extern _MA3_COUNTER;

//#define printf sysPrintf
//#define sysPrintf(...)

static UINT8  *_pucPcmQueue = (UINT8 *)(0x180000 | 0x10000000);
//static UINT8  _pucPcmQueue [] = {
//	#include "8k.dat"
//};

volatile UINT32 _uPcmQHead, _uPcmQTail;
int isnotfinish = 1;
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


INT  play_callback(UINT8 *pucBuff, UINT32 uDataLen)
{
	INT		nLen;

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

void error_handler(INT nRetValue){
	if (WBAPI_RESULT_IS_ERROR(nRetValue)){
		printf("error ID:%x, error code:%x\n",WBAPI_GET_ERROR_ID(nRetValue),WBAPI_GET_ERROR_NUMBER(nRetValue));
		wb702_STS_ERR(nRetValue);
		exit(-1);
	}else
		return;
}




void  I2S_loopback_demo(INT choose,INT ch)
{
	INT		nTime0;
	INT   nTempData;
	INT	  nRetValue;
	
	nRetValue = audioEnable(AU_DEV_IIS, AU_DEV_IIS);

	_uPcmQHead = _uPcmQTail = 0;

	
	
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



	nRetValue = audioStartRecord(record_callback, choose, ch);
	nTime0 = sysGetTicks(TIMER1);
	nTempData = _uPcmQTail;
	while((sysGetTicks(TIMER1) - nTime0) <= 50 );
	if (_uPcmQTail==nTempData)

	error_handler(-1);
		
	nRetValue = audioStartPlay(play_callback, choose, ch);
	nTime0 = sysGetTicks(TIMER1);
	nTempData = _uPcmQHead;
	while((sysGetTicks(TIMER1) - nTime0) <= 50 );
	if (_uPcmQHead==nTempData)
		error_handler(-1);
	nRetValue = audioStopRecord();
	nRetValue = audioStopPlay();

	
	return;
}


void  I2S_ADC_loopback_demo(INT choose,INT ch)
{
	INT		nTime0;
	INT   nTempData;
	INT	  nRetValue;
	
	nRetValue = audioEnable(AU_DEV_IIS, AU_DEV_ADC);


	_uPcmQHead = _uPcmQTail = 0;

	
	
	if (choose%11025==0){
		/* 22.579Mhz */
		outpw(REG_APLLCON, 0x642d);
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x200000);
	}
	else{
		/* 24.567Mhz */
		outpw(REG_APLLCON, 0x6529);//24.576Mhz
		outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x100000);
	}

	nRetValue = audioStartRecord(record_callback, choose, ch);
	nTime0 = sysGetTicks(TIMER1);
	nTempData = _uPcmQTail;
	while((sysGetTicks(TIMER1) - nTime0) <= 50 );
	if (_uPcmQTail==nTempData)
		error_handler(-1);


	nRetValue = audioStartPlay(play_callback, choose , ch);
	
	nTime0 = sysGetTicks(TIMER1);
	nTempData = _uPcmQHead;
	while((sysGetTicks(TIMER1) - nTime0) <= 50 );
	if (_uPcmQHead==nTempData)
		error_handler(-1);
	nRetValue = audioStopRecord();
	nRetValue = audioStopPlay();
	
	return;
}





int main()
{
	INT nRetValue = 0,nTime0;
	wb702_STS_READY();
	/* start timer0 for FMI init */
	sysConfiguration();
		
	nRetValue = audioSetPlayBuff(0x1005a724, 2048);
	error_handler(nRetValue);
	nRetValue = audioSetRecBuff(0x100c0000, 2048);
	error_handler(nRetValue);

	sysSetTimerReferenceClock (TIMER1, 12000000);
	sysStartTimer(TIMER1, 100, PERIODIC_MODE);
    



	nTime0 = sysGetTicks(TIMER1);
	wb702_STS_RUNNING();
	I2S_loopback_demo(AU_SAMPLE_RATE_8000,1);
	I2S_ADC_loopback_demo(AU_SAMPLE_RATE_32000,1);
	wb702_STS_OK();
	printf("period = %d\n",sysGetTicks(TIMER1) - nTime0);



}



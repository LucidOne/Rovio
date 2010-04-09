/****************************************************************************
 *                                                                          *
 * Copyright (c) 2004 - 2006 Winbond Electronics Corp. All rights reserved. *
 *                                                                          *
 ***************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *    
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *     None
 *
 * HISTORY
 *     09/01/04		 Ver 1.0 Created by PC33 WTCheng
 *
 * REMARK
 *     None
 **************************************************************************/
/* This pragram code must called frame by frame */
#define _ECOS_

#ifdef _ECOS_
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cyg/hal/hal_arch.h"           // CYGNUM_HAL_STACK_SIZE_TYPICAL
#include "cyg/kernel/kapi.h"
#include "cyg/infra/testcase.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "wbio.h"
#include "wblib.h"
#include "wbfat.h"
#include "wb_fmi.h"
#include "W99702_Audio.h"



//#include "AMRNBLib.h"
#include "mode.h"

#define amrSERIAL_FRAMESIZE (1+MAX_SERIAL_SIZE+5)
#define amrAMRMaxSize 32
#define amrL_ENERGYHIST 60
#define amrL_CBGAINHIST 7
#define amrM            10  
#define amrPHDGAINMEMSIZE 5
#define amrDTX_HIST_SIZE 8
#define amrL_SUBFR      40 
#define amrPIT_MAX       143 
#define amrL_INTERPOL    (10+1)
#define amrL_FRAME      160


#define NO_DATA							-111 //bitsteam is invalid                                             

#define NumOfFrame 150 // total frame

#define _WT            // make sure the amrDecode() is correct with audio, fmi, wbfat AP applyed on bard.

#define NUM_OF_INBUFSIZE	(160*2)
#define PCM_QUEUE_LEN		(2048*1024)


static __align(4) UINT8  _pucPcmQueue[PCM_QUEUE_LEN];
static volatile UINT32 _uPcmQHead, _uPcmQTail;
//static volatile BOOL  _bIsStop = 0;
static INT32	_hFileHandle;
#ifdef _WT
static INT32	_hFileHandleR;
#endif
__align(4) static INT16 sD_in[amrAMRMaxSize], sD_out[amrL_FRAME];

INT  play_callback(UINT8 *pucBuff, UINT32 uDataLen)
{
	INT		nLen;

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
#ifdef _ECOS_
static VOID amr_decoder_from_AC97(cyg_addrword_t data)
#else
static VOID amr_decoder_from_AC97()
#endif
{	
  	INT				nLen;
  	//UINT32			uInputSamples, uMaxOutputBytes;
    INT32			uWriteLen,uSeekNum;
    UINT8 ucCh[6];
    //UINT8 ucMagicNum[]={'#','!','A','M','R','\n'};
    //enum amrMode mode;   
  	INT16 sStatus;
  	INT16 sSize;
    //UINT8 *ucPtrOut;
    INT16 i;
  	UINT8 ucMode8;
 	enum Mode nMode;      
 	UINT8 *ucPtrIn;
 	//UINT32 uPos1,uPos2;
 	
	_hFileHandle = fsOpenFile((char*)L"C:\\7.amr", "7.amr",O_RDONLY);
	if(_hFileHandle<0)
	{
	   printf("Can't not open file, message is %d\n",_hFileHandle);
	}
#ifdef _WT
	_hFileHandleR = fsOpenFile("C:\\0922ecosAMRD.pcm", O_CREATE);
	if (_hFileHandleR < 0)
	{
		printf("Can't open INP file!\n[%d]\n",_hFileHandleR);
		return;
	}	
#endif		

	fsReadFile(_hFileHandle,ucCh, 6, &uWriteLen);
	if(uWriteLen!=6)
	   printf("not read the magic number\n");
 
	for(i=0;i<6;i++)
    	printf("%c\n",ucCh[i]);
        
    _uPcmQHead = _uPcmQTail = 0;
		
   /*-----------------------------------------------------------------------*
    * Initialization of decoder                                             *
    *-----------------------------------------------------------------------*/
    sStatus=amrInitDecode();
	
	
	audioStartPlay(play_callback, 8000, 1);
	
	//while (_uPcmQTail == 0)
	//	;

    while(1)
    {
		if(i>100)
		  break;
		//else
		//  printf("i=%d\n",i);
		fsReadFile(_hFileHandle,&ucMode8, 1, &uWriteLen);	// read mode 	
		if(uWriteLen<1)
		   break;
		uSeekNum=fsFileSeek(_hFileHandle,-1,SEEK_CUR);		        // restore the org address        
        //printf("seek %d\n",uSeekNum);        
        
        nMode = (ucMode8 >> 3) & 0x0F;
        //printf("nMode=%d\n",nMode);
        sSize=amrSizeOfEncodeOut(nMode);  // sSize is byte units.   	
    	
    	ucPtrIn=(UINT8*)sD_in;
		fsReadFile(_hFileHandle,ucPtrIn, (INT32)sSize, &uWriteLen);
		sStatus = amrDecode(sD_in,sD_out);
     	if(sStatus<0)
     	{
         	if(sStatus==NO_DATA) 
           		continue ;
         	printf("error number %d and abort \n",sStatus);
         	break;
    	}    	

		nLen = PCM_QUEUE_LEN - _uPcmQTail;
		if (nLen >= (amrL_FRAME*sizeof(INT16)))
		{
			memcpy(&_pucPcmQueue[_uPcmQTail], (UINT8 *)sD_out, amrL_FRAME*sizeof(INT16));
			_uPcmQTail += (amrL_FRAME*sizeof(INT16));
		}
		else
		{
			memcpy(&_pucPcmQueue[_uPcmQTail], (UINT8 *)sD_out, nLen);
			memcpy(_pucPcmQueue, (char *)((UINT32)sD_out + nLen), (amrL_FRAME*sizeof(INT16)) - nLen);
			_uPcmQTail = (amrL_FRAME*sizeof(INT16)) - nLen;
		}
     	
#ifdef _WT     	
	    fsWriteFile(_hFileHandleR,(UINT8*) sD_out, amrL_FRAME*sizeof(INT16), &uWriteLen);
	    //printf("memW (%d)\n",uWriteLen);
#endif	    		
		if (_uPcmQTail >= PCM_QUEUE_LEN - (amrL_FRAME*2))
			audioStartPlay(play_callback, AU_SAMPLE_RATE_8000, 1);
			
	}		
			
						


       
      
	printf("play end\n");
	fsCloseFile(_hFileHandle);
	fsCloseFile(_hFileHandleR);	
}

#ifdef _ECOS_
__align(4) static UINT8 stack_encoder[4096];
#endif

int main()
{
#ifdef _ECOS_
	cyg_handle_t	handle_decoder;
	cyg_thread		thread_decoder;
#endif

	sysEnableCache(CACHE_WRITE_BACK);

	fsInitFileSystem();
	fmiInitDevice();
	fmiInitSDDevice();
	
	audioSelectDevice(AU_DEV_AC97, AU_DEV_AC97);
	//audioSelectDevice(AU_DEV_IIS, AU_DEV_IIS);
	audioSetPlayBuff(0x100000, 4096);
	//audioSetRecBuff(0x110000, 4096);

#ifdef _ECOS_    
    CYG_TEST_INIT();
 	cyg_thread_create(10, amr_decoder_from_AC97, 0, "amr decoder_from_AC97",
        				(void *)stack_encoder, 4096, 
        				&handle_decoder, &thread_decoder);
    cyg_thread_resume(handle_decoder);
    CYG_TEST_PASS_FINISH("Thread 0 OK");
#else
	amr_decoder_from_AC97();
#endif
}
/****************************************************************************
 *                                                                            
 * Copyright (c) 2005 - 2006 Winbond Electronics Corp. All rights reserved.   
 *
 * 
 * FILENAME
 *     demo.c
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
 *     2004.10.15		Created by wen-shuo chang
 *
 *
 * REMARK
 *     None
 *
 **************************************************************************/
#include "stdio.h"
#include "drv_api.h"
#include "ima_adpcm.h"
#include "wblib.h"
#include "w99702_audio.h"
#include "w99702_reg.h"
#include "st.h"
#include "wav.h"
#include "mad.h"
#include "wbfat.h"

#define dbg_printf(...)		printf(__VA_ARGS__)

//#define AUDIO_BUF_CHUNK		(1152*2)
#define AUDIO_BUF_CHUNK		(IMA_BLOCK_SIZE)
#define AUDIO_REC_BUF_SIZE	(AUDIO_BUF_CHUNK * 4)	/* NOTE: audio record buffer size / 4 must be even number */
#define AUDIO_PLAY_BUF_SIZE	(AUDIO_BUF_CHUNK * 4)   /* NOTE: audio play buffer size / 4 must be even number */
#define AUDIO_BUF_SIZE		(AUDIO_BUF_CHUNK * 4)	/* NOTE: it should be AUDIO_BUF_CHUNK alignment */

static __align(32) CHAR record_buf[AUDIO_REC_BUF_SIZE];
static __align(32) CHAR play_buf[AUDIO_PLAY_BUF_SIZE];

#define DEMO_DURATION	100000000L

static CHAR audio_buf[AUDIO_BUF_SIZE];
static CHAR * pRec = audio_buf;
static CHAR * pPlay = audio_buf;

int nByteCnt;

void audio_buf_init(void)
{
	pRec = audio_buf;
	pPlay = audio_buf;
}

//--------------------------------------------------------------------------------------------
/* Encoding the audio data from AUDIO Interface and Decoding it to AUDIO Interface */

INT demo1rec_callback(UINT8 *pucBuff, UINT32 uDataLen)
{
	INT len;
	INT sampleperblock;
	INT blocksize;
	CHAR packet[IMA_PACKET_SIZE];
	INT packet_size;

	sampleperblock = imaadpcmSamplePerBlock();
	blocksize = sampleperblock * 2; /* singal channel, 16bits */


	/* rounding ring buffer */
	if( pRec >= &audio_buf[AUDIO_BUF_SIZE] )	 pRec = audio_buf;

	len = uDataLen;
	while( len >= blocksize )
	{
		/* ima adpcm encode a block */
		imaadpcmBlockEnc((CHAR *)pucBuff, blocksize,  packet, &packet_size); /* IMA ADPCM ENCODE API */
		
		/* update pucBuff pointer */
		pucBuff += blocksize;
		
		/* ima adpcm decode a block */
		imaadpcmBlockDec(packet, IMA_PACKET_SIZE, pRec, &blocksize); /* IMA ADPCM DECODE API */
		
		/* update recode pointer */
		pRec += blocksize;
		
		len -= blocksize;
	}
	
	return 0;
}

INT  demo1play_callback(UINT8 *pucBuff, UINT32 uDataLen)
{
	/* Recorded data not ready or buffer full */
	if( pRec == pPlay ) return 0;

	/* rounding ring buffer */
	if( pPlay >= &audio_buf[AUDIO_BUF_SIZE] ) pPlay = audio_buf;
	
	/* Move recorded data into audio buffer */
	memcpy(pucBuff, pPlay, uDataLen);

	/* update play buffer point */
	pPlay += uDataLen;

	return 0;
}


void ima_adpcm_codec_demo1(void)
{
	INT nch;
	INT cnt;


	/* audio buffer initiazation */
	audio_buf_init();
	
	/* IMA ADPCM Codec initialization */
	nch = 1; /* single channel */
	imaadpcmInit(nch, 8000); /* IMA ADPCM INITIALIZATION API */
		
	
	/* Starting record */
	audioStartRecord(demo1rec_callback, AU_SAMPLE_RATE_8000, nch);
	dbg_printf("Start recording ...\n");
 
	/* Starting play */
	audioStartPlay(demo1play_callback, AU_SAMPLE_RATE_8000, nch);
	dbg_printf("Start playing ...\n");
	
	cnt = DEMO_DURATION;
	while(--cnt!=0)
	{
		if( (cnt&0xFFF) == 0 )dbg_printf(".");
	}
	dbg_printf("\n");

	
	/* stop play */	
	audioStopPlay();
	dbg_printf("Stop record\n");
			
	/* stop record */	
	audioStopRecord();
	dbg_printf("Stop play\n");
	
}


//--------------------------------------------------------------------------------------------
/* Encoding the audio data from PCM file to IMA ADPCM WAV file */

void ima_adpcm_codec_demo2(void)
{
    INT n;
    INT sampleperblock;
    INT blocksize;
    CHAR *buf;
	CHAR packet[IMA_PACKET_SIZE];
	INT packet_size;
    INT len = 0;
    INT sample_rate;
	INT fpr, fpw;
	INT nch;
#define  FILENAME "meet_to_late_mono1s.pcm"
#define  OUT_FILE "ima_out.wav"
	st_signalinfo_t sinfo;
	extern struct wavstuff	wavhead;

	/* Open PCM file for input audio data */
	fpr = fsOpenFile((char*)L"c:\\meet_to_late_mono1s.pcm", FILENAME, O_RDONLY);
	if(!fpr)
	{
		dbg_printf("Open read file error\n");
		return;
	}
	/* Open WAV file for write IMA ADPCM data */
	fpw = fsOpenFile((char*)L"c:\\ima_out.wav", OUT_FILE, O_CREATE);
	if(!fpw)
	{
		fsCloseFile(fpr);
		dbg_printf("Open write file error\n");
		return;
	}

	/* IMA ADPCM Codec initialization */
	nch = 1; /* single channel */
	sample_rate = 44100; /* 44.1 kHz sampling rate */
	imaadpcmInit(nch, sample_rate);/* IMA ADPCM INITIALIZATION API */

	sampleperblock = imaadpcmSamplePerBlock();
	blocksize = sampleperblock * 2; /* singal channel, 16bits. One block size for input (in bytes) */
	
	sinfo.rate = sample_rate; /* 44.1 kHz sampling rate */
	sinfo.channels = nch; /* number of channel */
	sinfo.encoding = ST_ENCODING_IMA_ADPCM; /* Audio data format for WAV file */
	sinfo.size = ST_SIZE_BYTE; /* data unit, in byte */
	sinfo.fp = fpw;	/* Audio file pointer for WAV file */

	/* Initialize the length information of WAV header */	
	wavhead.numSamples = 0;
	wavhead.dataLength = 0;
	
	/* Generate the WAV header into WAV file */
	wavwritehdr(&sinfo ,0);
	
	
    while(1) {
    	buf = (CHAR *)&audio_buf;
    	
    	/* Keep the other PCM data from previous encoding loop */
		if( len ) memcpy(buf, buf + AUDIO_BUF_SIZE-len, len);

		/* Read PCM data form file into audio buffer (fill the audio buffer) */			
		fsReadFile(fpr, (UINT8 *)buf + len, (AUDIO_BUF_SIZE - len ), &n);
		
		/* Accessing error */
		if ( n < 0 ) {
	    	dbg_printf("input file");
	    	return;
		}
		
		/* PCM file empty. Stop reading */
		if ( n == 0 ) break; 

		/* Update the data length in audio buffer */
		len += n;
		
		while (len >= blocksize) {
			/* ima adpcm encode a block */
			imaadpcmBlockEnc(buf, blocksize,  packet, &packet_size); /* IMA ADPCM ENCODE API */

			/* Writing the IMA ADPCM packet into WAV file */
			fsWriteFile(sinfo.fp, (UINT8*)packet, packet_size, &nByteCnt);
			
			/* Update the header information for WAV file */
			wavhead.numSamples += sampleperblock;
			wavhead.dataLength += packet_size;
			
			/* Update the un-encoding data length in audio buffer */
			len -= blocksize;
			/* Update the pointer of the un-encoding data in audio buffer */
			buf += blocksize;
	    }
    }

	/* ima adpcm encode the other data in audio buffer */
	if( len )
	{
		/* Prepare the other audio data */
		memcpy(audio_buf, buf, len);
		/* Append with zero for a block */
		memset(audio_buf + len, blocksize-len, 0);
		/* Encoding a block */
		imaadpcmBlockEnc(audio_buf, blocksize,  packet, &packet_size); /* IMA ADPCM ENCODE API */
		/* Writing the IMA ADPCM packet into WAV file */
		fsWriteFile(sinfo.fp, (UINT8*)packet, packet_size, &nByteCnt);
		/* Update the header information for WAV file */
		wavhead.numSamples += sampleperblock;
		wavhead.dataLength += packet_size;
	}

	/* Update the header with new data length information */
	if (fsFileSeek(sinfo.fp, 0, SEEK_SET) == 0)
		wavwritehdr(&sinfo ,1);
	else
		dbg_printf("Can't rewind output file to rewrite .wav header.\n");

	/* Close the WAV file */
	fsCloseFile(fpw);
	
	/* Close the PCM file */
	fsCloseFile(fpr);
	return;	
}


//--------------------------------------------------------------------------------------------
/* Decoding the audio data from WAV file to PCM file */

void ima_adpcm_codec_demo3(void)
{
    INT n;
    INT packet_size;
    CHAR *buf;
	CHAR pcm_buf[IMA_BLOCK_SIZE];
	INT pcm_size;
    INT len = 0;
    INT sample_rate;
	INT fpr, fpw;
	INT nch;
	CHAR * filename = "ima_out.wav";
	CHAR * out_file = "ima_out.pcm";
	st_signalinfo_t sinfo;

	/* Open WAF file for input IMA ADPCM coding data */
	fpr = fsOpenFile((char*)L"c:\\ima_out.wav", filename, O_RDONLY);
	if(!fpr)
	{
		dbg_printf("Open read file error\n");
		return;
	}
	/* Open PCM file for write decoded PCM data */
	fpw = fsOpenFile((char*)L"c:\\ima_out.pcm", out_file, O_CREATE);
	if(!fpw)
	{
		fsCloseFile(fpr);
		dbg_printf("Open write file error\n");
		return;
	}

	/* Read audio information from WAV header, and check if supported. */
	if(checkwavfile(fpr, &sinfo) == ST_EOF)
	{
		dbg_printf("We don't support this kind of encoding wav file\n");
		fsCloseFile(fpr);
		fsCloseFile(fpw);
		return;
	}
	/* If checkwavfile success, the fpr is pointing to the data now */

	/* IMA ADPCM Codec initialization */
	nch = sinfo.channels; /* number of channel */
	sample_rate = sinfo.rate; /* sampling rate in Hz */
	imaadpcmInit(nch, sample_rate); /* IMA ADPCM INITIALIZATION API */

	packet_size = IMA_PACKET_SIZE;

    while(1) {
    	/* Keep the other IMA ADPCM data from previous encoding loop */
		if( len ) memcpy(audio_buf, buf, len);

    	buf = (CHAR *)&audio_buf;

		/* Read IMA ADPCM data form file into audio buffer (fill the audio buffer) */			
		fsReadFile(fpr, (UINT8 *)buf + len, (AUDIO_BUF_SIZE - len ), &n);
		
		/* Accessing error */
		if ( n < 0 ) {
	    	dbg_printf("input file");
	    	return;
		}
		
		/* PCM file empty. Stop reading */
		if ( n == 0 ) break; 

		/* Update the data length in audio buffer */
		len += n;

		while (len >= packet_size) {
			/* ima adpcm encode a block */
			imaadpcmBlockDec(buf, packet_size,  pcm_buf, &pcm_size); /* IMA ADPCM DECODE API */

			/* Writing the PCM data into PCM file */
			fsWriteFile(fpw, (UINT8*)pcm_buf, pcm_size, &nByteCnt);
			
			/* Update the un-encoding data length in audio buffer */
			len -= packet_size;
			
			/* Update the pointer of the un-decoding data in audio buffer */
			buf += packet_size;
	    }
    }

	/* decode the other data in audio buffer */
	if( len )
	{
		/* Prepare the other audio data */
		memcpy(audio_buf, buf, len);
		/* Append with zero for a block */
		memset(audio_buf + len, packet_size-len, 0);
		/* Decoding a block */
		imaadpcmBlockDec(audio_buf, packet_size,  pcm_buf, &pcm_size); /* IMA ADPCM DECODE API */
		/* Writing the PCM data into PCM file */
		fsWriteFile(fpw, (UINT8*)pcm_buf, pcm_size, &nByteCnt);
	}
	
	/* Close the PCM file */
	fsCloseFile(fpw);
	
	/* Cloce the WAV file */
	fsCloseFile(fpr);
	
	printf("decode IMA_ADPCM finished\n");
	return;	
}


//--------------------------------------------------------------------------------------------
/* AUDIO Interface demo (for test) */

INT demo0rec_callback(UINT8 *pucBuff, UINT32 uDataLen)
{
	//dbg_printf("rec callback...\n");

	/* rounding ring buffer */
	if( pRec >= &audio_buf[AUDIO_BUF_SIZE] )	 pRec = audio_buf;
	
	/* Move recorded data into audio buffer */
	memcpy(pRec, pucBuff, uDataLen);
	
	/* update record buffer point */
	pRec += uDataLen;
	
	return 0;
}

INT  demo0play_callback(UINT8 *pucBuff, UINT32 uDataLen)
{
	/* Recorded data not ready or buffer full */
	if( pRec == pPlay ) return 0;

	/* rounding ring buffer */
	if( pPlay >= &audio_buf[AUDIO_BUF_SIZE] ) pPlay = audio_buf;
	
	/* Move recorded data into audio buffer */
	memcpy(pucBuff, pPlay, uDataLen);

	/* update play buffer point */
	pPlay += uDataLen;

	return 0;
}


void audio_loopback_demo0(void)
{
	INT cnt;

	/* audio buffer initiazation */
	audio_buf_init();
		
	/* Starting record */
	audioStartRecord(demo0rec_callback, AU_SAMPLE_RATE_8000, 1);
	dbg_printf("Start recording ...\n");

	/* Starting play */
	audioStartPlay(demo0play_callback, AU_SAMPLE_RATE_8000, 1);
	dbg_printf("Start playing ...\n");
	
	dbg_printf("\n\npress 'q' to Quit ...\n");
	
	
	cnt = DEMO_DURATION;
	while(--cnt!=0)
	{
		if( (cnt&0xFFF) == 0 )dbg_printf(".");
	}
	dbg_printf("\n");

	
	/* stop play */	
	audioStopPlay();
	dbg_printf("Stop record\n");
			
	/* stop record */	
	audioStopRecord();
	dbg_printf("Stop play\n");
	
}


void InitClk()
{
	int volatile	nLoop, tmp;
	
	//sysDisableCache();
	//sysInvalidCache();

	tmp = inpw(REG_CLKCON);
	outpw(REG_CLKSEL, 0x00000104);	// clock source from crystal
    outpw(REG_CLKCON, 0x02C01000);


	outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFFFFFFFC) | 0x1); /* APB 2:1 */
	outpw(REG_CLKDIV1, 0x00015555);		/* AHB2 2:1 */
	outpw(REG_SDTIME0, 0x80005829);		/* set DRAM timing */
	
	//outpw(REG_UPLLCON, 0x4541);		/* 156 MHz */
	//outpw(REG_UPLLCON, 0x453C);		/* 144 MHz */
	//outpw(REG_UPLLCON, 0x4521);		/* 132 MHz */
	//outpw(REG_UPLLCON, 0x4214);		/* 120 MHz */
	outpw(REG_UPLLCON, 0x6550);			/* 96 MHz */

#if 0	//44.1k 22.05K
	/* programming APLL to be 16.934 MHz */
	outpw(REG_APLLCON, 0x642D);
	outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x300000);
#else	//48k 32k 24k
	/* 12.288 MHz */
	outpw(REG_APLLCON, 0x6529);
	outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xFF0FFFFF) | 0x300000);
#endif	
	
	for (nLoop = 0; nLoop < 0x1000; nLoop++)
		;
	/* 
	 * CLK select: SYSTEM_S from UPLL, AUDIO_S from APLL 
	 * SENSOR_S and VIDEO_S from crystal in 
	 */
	outpw(REG_CLKSEL, 0x364);
	
	for (nLoop = 0; nLoop < 0x1000; nLoop++)
		;
	
	outpw(REG_CLKCON, tmp);
	outpw(REG_CLKCON, inpw(REG_CLKCON) | 0x53003090);
	
	for (nLoop = 0; nLoop < 0x1000; nLoop++)
		;
	
	fsInitFileSystem();
	fmiInitDevice();
	fmiSetFMIReferenceClock(96000);
	fmiInitSDDevice();
}

//--------------------------------------------------------------------------------------------
int main(void)
{
	int n;
	
	InitClk();
	
	audioEnable(AU_DEV_WM8978, AU_DEV_WM8978);
	/* Setting record buffer */
	audioSetRecBuff((INT)record_buf, AUDIO_REC_BUF_SIZE);
	/* Setting play buffer */
	audioSetPlayBuff((INT)play_buf, AUDIO_PLAY_BUF_SIZE);
	
	while(1)
	{
		dbg_printf("**********************\n");
		dbg_printf(" IMA ADPCM Codec Demo\n");
		dbg_printf("**********************\n");
		dbg_printf("(0) AUDIO Interface loop back demo.\n");
		dbg_printf("(1) Encoding from AUDIO Interface and decoding it to AUDIO Interface.\n");
		dbg_printf("(2) Encoding from PCM file to IMA ADPCM WAV file.\n");
		dbg_printf("(3) Decoding from IMA ADPCM WAV file to PCM file .\n");
		dbg_printf("Please select:");
		scanf("%d",&n);
		printf("n=%d\n",n);
		if( n == 0 )
			audio_loopback_demo0();
		if( n == 1 )
			ima_adpcm_codec_demo1();
		else if( n == 2 )
			ima_adpcm_codec_demo2();
		else if( n == 3 )
			ima_adpcm_codec_demo3();
		else
			dbg_printf("Please select a vaild option\n");
	}
	return 0;
}
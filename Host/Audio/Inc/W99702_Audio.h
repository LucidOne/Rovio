/**************************************************************************************************
 *                                                                          
 * Copyright (c) 2004 - 2007 Winbond Electronics Corp. All rights reserved.      
 *                                                                         
 * FILENAME
 *     AC97.h
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     This file contains the register map of AC97 audio codec
 *
 * HISTORY
 *     02/09/2004		 Ver 1.0 Created by PC30 YCHuang
 *
 * REMARK
 *     None
 *     
 *************************************************************************************************/

#ifndef _W99702_AUDIO_H_
#define _W99702_AUDIO_H_
#include "wberrcode.h"

typedef INT (AU_CB_FUNC_T)(UINT8 *, UINT32);


typedef struct au_i2c_type_t
{
	BOOL	bIsGPIO;	//True:GPIO; False:fast serial bus
	INT32	uSDIN;			// serial data input pin
	INT32	uSCLK;	// serial clock pin
} AU_I2C_TYPE_T;

typedef struct audio_t
{
	AU_CB_FUNC_T 	*fnPlayCallBack;
	AU_CB_FUNC_T 	*fnRecCallBack;
	INT				nPlaySamplingRate;
	INT				nRecSamplingRate;
	BOOL			bPlayLastBlock;
	UINT16			sPlayVolume;
	UINT16			sRecVolume;
#ifdef ECOS
	cyg_handle_t	int_handle_play;
	cyg_handle_t	int_handle_rec;
	cyg_interrupt	int_holder_play;
	cyg_interrupt	int_holder_rec;
#endif	
}	AUDIO_T;

typedef enum audio_outputpath_e
{
	OUT_PATH_HP,				//Head phone
	OUT_PATH_SP					//Speaker
} AUDIO_OUTPUTPATH_E;

typedef enum audio_inputpath_e
{
	IN_PATH_MICL,				//left micro phone
	IN_PATH_MICR,				//right micro phone
	IN_PATH_LINEIN1,			//
	IN_PATH_LINEIN2 			//
} AUDIO_INPUTPATH_E;

typedef enum au_dev_e
{
	AU_DEV_AC97,
	AU_DEV_IIS,
	AU_DEV_UDA1345TS,
	AU_DEV_ADC,
	AU_DEV_DAC,
	AU_DEV_MA3,
	AU_DEV_MA5,
	AU_DEV_W5691,
	AU_DEV_WM8753,
	AU_DEV_WM8751,
	AU_DEV_WM8978,
	AU_DEV_MA5I,
	AU_DEV_MA5SI,
	AU_DEV_W56964,
	AU_DEV_AK4569,
	AU_DEV_TIAIC31,
	AU_DEV_WM8731,
	AU_DEV_PSM711A,
	AU_DEV_WM8750
} AU_DEV_E;

typedef enum AU_I2S_MCLK_TYPE_E
{
	I2S_MCLK_TYPE_NORMAL,
	I2S_MCLK_TYPE_256FS,
	I2S_MCLK_TYPE_APLL

} AU_I2S_MCLK_TYPE_E;

#define AU_SAMPLE_RATE_48000	48000
#define AU_SAMPLE_RATE_44100	44100
#define AU_SAMPLE_RATE_32000	32000
#define AU_SAMPLE_RATE_24000	24000
#define AU_SAMPLE_RATE_22050	22050
#define AU_SAMPLE_RATE_16000	16000
#define AU_SAMPLE_RATE_12000	12000
#define AU_SAMPLE_RATE_11025	11025
#define AU_SAMPLE_RATE_8000		8000

#define MSB_FORMAT	1
#define IIS_FORMAT  2	

/* Error Code */
#define ERR_AU_GENERAL_ERROR	  			(ADO_ERR_ID | 0x10)
#define ERR_AU_NO_MEMORY		    		(ADO_ERR_ID | 0x11) 	/* memory allocate failure */
#define ERR_AU_ILL_BUFF_SIZE	  			(ADO_ERR_ID | 0x12) 	/* illegal callback buffer size */
#define ERR_NO_DEVICE						(ADO_ERR_ID | 0x13)		/* audio device not available */
#define ERR_AU_MEMORY_ALIGN					(ADO_ERR_ID | 0x14)		/* audio buffer length or address is not 32 byte alignment */
#define ERR_AC97_CODEC_RESET	  			(ADO_ERR_ID | 0x20) 	/* AC97 codec reset failed */
#define ERR_AC97_PLAY_ACTIVE	  			(ADO_ERR_ID | 0x21) 	/* AC97 playback has been activated */
#define ERR_AC97_REC_ACTIVE		  			(ADO_ERR_ID | 0x22)  	/* AC97 record has been activated */
#define ERR_AC97_NO_DEVICE		  			(ADO_ERR_ID | 0x23) 	/* have no AC97 codec on board */
#define ERR_AC97_PLAY_INACTIVE	  			(ADO_ERR_ID | 0x24) 	/* AC97 playback is inactive */
#define ERR_AC97_REC_INACTIVE		  		(ADO_ERR_ID | 0x25)  	/* AC97 record is inactive */

#define ERR_MA3_PLAY_ACTIVE		  			(ADO_ERR_ID | 0x30)  	/* MA3 playback has been activated */
#define ERR_MA3_NO_DEVICE		    		(ADO_ERR_ID | 0x31)  	/* have no MA3 chip on board */
#define ERR_MA5_PLAY_ACTIVE		  			(ADO_ERR_ID | 0x40)  	/* MA5 playback has been activated */
#define ERR_MA5_NO_DEVICE		    		(ADO_ERR_ID | 0x41)  	/* have no MA5 chip on board */
#define ERR_MA5I_NO_DEVICE		  			(ADO_ERR_ID | 0x50)  	/* have no MA-5i chip on board */
#define ERR_MA5I_ERROR						(ADO_ERR_ID | 0x51)		/* MA-5i power manager parameter error  */
#define ERR_MA5I_SOFTRESET					(ADO_ERR_ID | 0x52)		/* error of soft reset for MA-5i	*/
#define ERR_MA5I_MEMCLR						(ADO_ERR_ID | 0x53)		/* MA-5i wait memory clear timeout  */
#define ERR_MA5I_VREF_RDY					(ADO_ERR_ID | 0x54)		/* MA-5i wait VREF_RDY timeout  */
#define ERR_MA5I_HP_RDY						(ADO_ERR_ID | 0x55)		/* MA-5i wait HP_RDY timeout  */
#define ERR_MA5I_HP_STBY					(ADO_ERR_ID | 0x56)		/* MA-5i wait HP_STBY timeout  */

                                			
#define ERR_MA5SI_NO_DEVICE		  			(ADO_ERR_ID | 0x60)  	/* have no MA5-Si chip on board */
#define ERR_MA5SI_ERROR						(ADO_ERR_ID | 0x61)		/* MA-5Si power manager parameter error  */
#define ERR_MA5SI_SOFTRESET					(ADO_ERR_ID | 0x62)		/* error of soft reset for MA-5i	*/
#define ERR_MA5SI_MEMCLR					(ADO_ERR_ID | 0x63)		/* MA-5Si wait memory clear timeout  */
#define ERR_MA5SI_VREF_RDY					(ADO_ERR_ID | 0x64)		/* MA-5Si wait VREF_RDY timeout  */
#define ERR_MA5SI_HP_RDY					(ADO_ERR_ID | 0x65)		/* MA-5Si wait HP_RDY timeout  */
#define ERR_MA5SI_HP_STBY					(ADO_ERR_ID | 0x65)		/* MA-5Si wait HP_STBY timeout  */

#define ERR_DAC_PLAY_ACTIVE		  			(ADO_ERR_ID | 0x70) 	/* DAC playback has been activated */
#define ERR_DAC_NO_DEVICE		    		(ADO_ERR_ID | 0x71) 	/* DAC is not available */
#define ERR_ADC_REC_ACTIVE		  			(ADO_ERR_ID | 0x72)   	/* ADC record has been activated */
#define ERR_ADC_NO_DEVICE	 	    		(ADO_ERR_ID | 0x73) 	/* ADC is not available */
#define ERR_ADC_REC_INACTIVE				(ADO_ERR_ID | 0x74) 	/* ADC is inactive */

#define ERR_IIS_PLAY_ACTIVE		  			(ADO_ERR_ID | 0x80)  	/* IIS playback has been activated */
#define ERR_IIS_REC_ACTIVE		  			(ADO_ERR_ID | 0x81)  	/* IIS record has been activated */
#define ERR_IIS_NO_DEVICE		    		(ADO_ERR_ID | 0x82)  	/* has no IIS on board */
#define ERR_IIS_PLAY_INACTIVE	  			(ADO_ERR_ID | 0x83)  	/* IIS playback is inactive */
#define ERR_IIS_REC_INACTIVE	  			(ADO_ERR_ID | 0x84)  	/* IIS record is inactive */
#define ERR_UDA1345TS_NO_DEVICE		   		(ADO_ERR_ID | 0x85)  	/* has no UDA1345TS on board */

#define ERR_WM8753_NO_DEVICE	  			(ADO_ERR_ID | 0x90)  	/* has no wm8753 codec on board */
#define ERR_AK4569_NO_DEVICE				(ADO_ERR_ID | 0x91)  	/* has no AK4569 codec on board */

#define ERR_W5691_PLAY_ACTIVE	  			(ADO_ERR_ID | 0xA0)  	/* W5691 playback has been activated */
#define ERR_W5691_NO_DEVICE		  			(ADO_ERR_ID | 0xA1)  	/* Have no W5691 chip on board */
#define ERR_W5691_STOP_TIMEOUT  			(ADO_ERR_ID | 0xA2)  	/* wait W5691 uC enter stop mode timeout */

#define ERR_W56964_PLAY_ACTIVE  			(ADO_ERR_ID | 0xB0)  	/* W56964 playback has been activated */
#define ERR_W56964_NO_DEVICE	  			(ADO_ERR_ID | 0xB1)  	/* Have no W56964 chip on board */
#define ERR_W56964_STOP_TIMEOUT				(ADO_ERR_ID | 0xB2) 	/* wait W56964 uC enter stop mode timeout */
#define ERR_W56964_RESET_TIMEOUT			(ADO_ERR_ID | 0xB3) 	/* wait W56964 reset timeout */
#define	ERR_W56964_ERROR_ARGUMENT			(ADO_ERR_ID | 0xB4) 	/* error of function argument */
#define	ERR_W56964_ERROR_REGRW				(ADO_ERR_ID | 0xB5) 	/* error of register read/write */


#define ERR_WM8978_NO_DEVICE	  			(ADO_ERR_ID | 0xC0)  	/* Have no WM8978 chip on board */
#define ERR_2WIRE_NO_ACK					(ADO_ERR_ID | 0xC1)  	/* 2-wire write command receive ack fail */


#define ERR_WM8751_NO_DEVICE	  			(ADO_ERR_ID | 0xD0)  	/* Have no WM8751 chip on board */
#define ERR_TIAIC31_NO_DEVICE				(ADO_ERR_ID | 0xD1)  	/* Have no TIAIC31 chip on board */
#define ERR_WM8731_NO_DEVICE				(ADO_ERR_ID | 0xD2)  	/* Have no WM8751 chip on board */

#define ERR_M80_R_INTERMEDIATEREG_TIMEOUT	(ADO_ERR_ID | 0xE0)		/* M80 interface write intermediate register timeout */
#define ERR_M80_W_INTERMEDIATEREG_TIMEOUT	(ADO_ERR_ID | 0xE1)		/* M80 interface read intermediate register timeout */
#define ERR_M80_R_CONTROLREG_TIMEOUT		(ADO_ERR_ID | 0xE2)		/* M80 interface read control register timeout */
#define ERR_M80_W_CONTROLREG_TIMEOUT		(ADO_ERR_ID | 0xE3)		/* M80 interface write control register timeout */
#define ERR_M80_W_RAM_TIMEOUT				(ADO_ERR_ID | 0xE4)		/* M80 interface write for ROM/RAM timeout */
#define ERR_M80_X86_PCM_TRANS_TIMEOUT		(ADO_ERR_ID | 0xE5)		/* M80 interface PCM transfer timeout */





extern INT  audioEnable(AU_DEV_E ePlayDev, AU_DEV_E eRecDev);
extern INT  audioDisable(VOID);
extern INT  audioSetPlayBuff(UINT32 uPlayBuffAddr, UINT32 uPlayBuffSize);
extern INT	audioStartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels);
extern INT  audioStopPlay(VOID);
extern INT  audioSetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol);
extern INT  audioSetRecBuff(UINT32 uRecBuffAddr, UINT32 uRecBuffSize);
extern INT  audioStartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels);
extern INT  audioStopRecord(VOID);
extern INT  audioSetRecordVolume(UINT8 ucLeftVol, UINT8 ucRightVol);
extern INT  audioWriteData(UINT16 regAddr, UINT16 data);
extern INT  audioReadData(UINT8 regAddr);
extern INT  audioSetPureI2SMCLKType(AU_I2S_MCLK_TYPE_E eI2SMclkType);
extern INT  audioSetI2CType(AU_I2C_TYPE_T eI2CType);
extern INT	audioRecConfigMono(BOOL bIsLeftCH);
extern INT  audioSelectOutputPath(AUDIO_OUTPUTPATH_E eOutput,BOOL bEnable);
extern INT  audioSelectInputPath(AUDIO_INPUTPATH_E eInput,BOOL bEnable);

extern INT  ac97StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels);
extern INT  ac97StopPlay(VOID);
extern INT  ac97StartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels);
extern INT  ac97StopRecord(VOID);
extern INT  ac97SetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol);
extern INT  ac97SetRecordVolume(UINT8 ucLeftVol, UINT8 ucRightVol);

extern INT  ma3StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels);
extern INT  ma3StopPlay(VOID);
extern INT  ma3SetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol);

extern INT  ma5StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels);
extern INT  ma5StopPlay(VOID);
extern INT  ma5SetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol);

extern INT  ma5iStartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT data_format);
extern INT  ma5iStopPlay(VOID);
extern INT  ma5iSetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol);

extern INT  ma5siStartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT data_format);
extern INT  ma5siStopPlay(VOID);
extern INT  ma5siSetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol);

extern INT  dacStartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels);
extern INT  dacStopPlay(VOID);
extern INT  adcStartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate);
extern INT  adcStopRecord(VOID);
extern INT  dacSetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol);
extern INT  adcSetRecordVolume(UINT8 ucVolume);

extern INT  i2sStartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT data_format);
extern INT  i2sStopPlay(VOID);
extern INT  i2sStartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT data_format);
extern INT  i2sStopRecord(VOID);

extern INT  uda1345tsStartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT data_format);
extern INT	uda1345tsStopPlay(VOID);
extern INT  uda1345tsStartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT data_format);
extern INT	uda1345tsStopRecord(VOID);
extern INT	uda1345tsSetPlayVolume(UINT8 ucLeftVol,UINT8 ucRightVol);

extern INT  wm8753StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT data_format);
extern INT  wm8753StopPlay(VOID);
extern INT  wm8753StartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT data_format);
extern INT  wm8753StopRecord(VOID);
extern INT  wm8753SetPlayVolume(UINT8 ucLeftVol,UINT8 ucRightVol);
extern INT  wm8753SetRecordVolume(UINT8 ucLeftVol,UINT8 ucRightVol);

extern INT  wm8978StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT data_format);
extern INT  wm8978StopPlay(VOID);
extern INT  wm8978StartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT data_format);
extern INT  wm8978StopRecord(VOID);
extern INT  wm8978SetPlayVolume(UINT8 ucLeftVol,UINT8 ucRightVol);
extern INT  wm8978SetRecordVolume(UINT8 ucLeftVol,UINT8 ucRightVol);
extern INT  wm8978SelectOutputPath(VOID);
extern INT  wm8978SelectInputPath(VOID);
extern INT 	TwoWire_Write_Data(UINT16,UINT16);
extern INT  WM8978_Bypass_Enable(VOID);
extern INT  WM8978_Bypass_Disable(VOID);

extern INT  ak4569StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT data_format);
extern INT  ak4569StopPlay(VOID);
extern INT  ak4569StartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT data_format);
extern INT  ak4569StopRecord(VOID);
extern INT  ak4569SetPlayVolume(UINT8 ucLeftVol,UINT8 ucRightVol);
extern INT  ak4569SetRecordVolume(UINT8 ucLeftVol);

extern INT  wm8751StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT data_format);
extern INT  wm8751StopPlay(VOID);
extern INT  wm8751SetPlayVolume(UINT8 ucLeftVol,UINT8 ucRightVol);

extern INT  w56964StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT pcm_format);
extern INT  w56964StopPlay(VOID);
extern INT  w56964SetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol);

extern INT  w5691StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT pcm_format);
extern INT  w5691StopPlay(VOID);
extern INT  w5691SetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol);

extern INT  tiaic31StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT data_format);
extern INT  tiaic31StopPlay(VOID);
extern INT  tiaic31StartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT data_format);
extern INT  tiaic31StopRecord(VOID);
extern INT  tiaic31SetPlayVolume(UINT8 ucLeftVol,UINT8 ucRightVol);
extern INT  tiaic31SetRecordVolume(UINT8 ucLeftVol, UINT8 ucRightVol);

extern INT  TIAIC31_I2C_Write_Data(UINT8 regAddr, UINT8 data);
extern INT  TIAIC31_I2C_Read_Data(UINT8 regAddr);


extern INT  wm8731StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT data_format);
extern INT  wm8731StopPlay(VOID);
extern INT  wm8731StartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT data_format);
extern INT  wm8731StopRecord(VOID);
extern INT  wm8731SetPlayVolume(UINT8 ucLeftVol,UINT8 ucRightVol);
extern INT  wm8731SetRecordVolume(UINT8 ucLeftVol,UINT8 ucRightVol);

extern INT  psm711aStartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT pcm_format);
extern INT  psm711aStopPlay(VOID);
extern INT  psm711aSetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol);

extern INT  wm8750StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT data_format);
extern INT  wm8750StopPlay(VOID);
extern INT  wm8750StartRecord(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels, INT data_format);
extern INT  wm8750StopRecord(VOID);
extern INT  wm8750SetPlayVolume(UINT8 ucLeftVol,UINT8 ucRightVol);
extern INT  wm8750SetRecordVolume(UINT8 ucLeftVol,UINT8 ucRightVol);



#endif	/* _W99702_AUDIO_H_ */

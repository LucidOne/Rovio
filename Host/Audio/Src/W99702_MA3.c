/****************************************************************************
 * 
 * FILENAME
 *     W99702_MA3.c
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
 *     2004.03.12		Created by Yung-Chang Huang
 *
 *
 * REMARK
 *     None
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
#include "YMU762.h"

#ifdef HAVE_MA3

#define SPEAKER

#define TONE_DATA_ADDR_LEFT		0x5c00
#define WT_PCM_LEFT_ADDR		0x5c20
#define WT_LEFT_START_ADDR_H	0x5c
#define WT_LEFT_START_ADDR_L	0x20
#define WT_LEFT_LOOP_START_H	0
#define WT_LEFT_LOOP_START_L	0
#define WT_LEFT_LOOP_END_H		0x4
#define WT_LEFT_LOOP_END_L		0x00

#define TONE_DATA_ADDR_RIGHT	0x5700
#define WT_PCM_RIGHT_ADDR		0x5720
#define WT_RIGHT_START_ADDR_H	0x57
#define WT_RIGHT_START_ADDR_L	0x20
#define WT_RIGHT_LOOP_START_H	0
#define WT_RIGHT_LOOP_START_L	0
#define WT_RIGHT_LOOP_END_H		0x4
#define WT_RIGHT_LOOP_END_L		0x00

static UINT8 _Tone_Setting_Left[] = 
{
	0x02, 	/* PANPOT=0x00, STM=1, PE=0 */
	0x03,	/* LFO=1.8Hz, PANOFF=0, MODE=8-bit(2's comp)PCM */
	0x00,	/* SR=0, XOF=0, SUS=0 */
	0xf0,	/* RR=0xf, DR=0 */
	0xf0,	/* AR=0xf, SL=0 */
	0x00,	/* TL=0 */
	0x00,	/* DAM=0, EAM=0, EVB=0, DVB=0 */
	WT_LEFT_START_ADDR_H,
	WT_LEFT_START_ADDR_L,
	WT_LEFT_LOOP_START_H,
	WT_LEFT_LOOP_START_L,
	WT_LEFT_LOOP_END_H,
	WT_LEFT_LOOP_END_L
};


static UINT8 _Tone_Setting_Right[] = 
{
	0xfa, 	/* PANPOT=0x1f, STM=1, PE=0 */
	0x03,	/* LFO=1.8Hz, PANOFF=0, MODE=8-bit(2's comp)PCM */
	0x00,	/* SR=0, XOF=0, SUS=0 */
	0xf0,	/* RR=0xf, DR=0 */
	0xf0,	/* AR=0xf, SL=0 */
	0x00,	/* TL=0 */
	0x00,	/* DAM=0, EAM=0, EVB=0, DVB=0 */
	WT_RIGHT_START_ADDR_H,
	WT_RIGHT_START_ADDR_L,
	WT_RIGHT_LOOP_START_H,
	WT_RIGHT_LOOP_START_L,
	WT_RIGHT_LOOP_END_H,
	WT_RIGHT_LOOP_END_L
};


static AUDIO_T	_tMA3;
static BOOL		_bSingleChannel = 0;
static BOOL		_bMA3Playing = 0;
static BOOL		_bChangeVolume = 0;
static UINT32 	_uSoftwarePG;

static void Delay(int nCnt)
{
	volatile int  loop;
	for (loop=0; loop<nCnt; loop++);
}


#if 0
static UINT8 read_intermediate_reg(INT nRegId)
{
	outpw(REG_ACTL_M80ADDR, nRegId);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | R_IF11_ACT);

	while (inpw(REG_ACTL_M80CON) & R_IF11_ACT)
		;
	return ((inpw(REG_ACTL_M80SIZE)>>16) & 0xff);
}
#endif


static INT write_intermediate_reg(INT nRegId, UINT8 data)
{
	INT nTimeOutCount;
	outpw(REG_ACTL_M80ADDR, nRegId);
	outpw(REG_ACTL_M80SIZE, 0x01);
	outpw(REG_ACTL_M80DATA0, data);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | W_IF12_ACT);
	for(nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
		if ((inpw(REG_ACTL_M80CON) & W_IF12_ACT) == 0 )
			break;
	}
	
	if (nTimeOutCount == 10000){
			_error_msg("write_intermediate_reg - M80 write intermediate register timeout\n");
			return ERR_M80_W_INTERMEDIATEREG_TIMEOUT;
	}
	return 0;
}


static INT read_control_reg(INT nRegNum)
{
	UINT32	 uData32;
	INT	nTimeOutCount;
	if (nRegNum < 0x80)
		uData32 = (0x80|nRegNum) << 8;
	else
		uData32 = ((nRegNum & 0x7f) << 8) |
				 (((nRegNum >> 7) & 0x7f) | 0x80) << 16;
	uData32 |= 0x03;					/* state flag */
	outpw(REG_ACTL_M80ADDR, uData32);

	outpw(REG_ACTL_M80DATA0, 0x82);		/* select read buffer #2 */
	
	outpw(REG_ACTL_M80SIZE, 0x03);      /* read ID =3, indicate read buffer #2 */

	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | R_IF10_ACT);

	for(nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
		if ((inpw(REG_ACTL_M80CON) & R_IF10_ACT) == 0 )
			break;
	}
	
	if (nTimeOutCount == 10000){
			_error_msg("read_control_reg - M80 read control register timeout\n");
			return ERR_M80_R_CONTROLREG_TIMEOUT;
	}

	return ((inpw(REG_ACTL_M80SIZE)>>16) & 0xff);
}


static INT  write_control_reg(INT nRegNum, UINT8 data)
{
	UINT32	 uData32;
	INT	nTimeOutCount;
	
retry:
	if (nRegNum < 0x80)
		uData32 = (0x80|nRegNum) << 8;
	else
		uData32 = ((nRegNum & 0x7f) << 8) |
				 (((nRegNum >> 7) & 0x7f) | 0x80) << 16;
	uData32 |= 0x02;					/* state flag */
	outpw(REG_ACTL_M80ADDR, uData32);
	
	outpw(REG_ACTL_M80DATA0, (0x80|data));

	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | W_IF10_ACT);
	
	for(nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
		if ((inpw(REG_ACTL_M80CON) & W_IF10_ACT) == 0 )
			break;
	}
	if (nTimeOutCount == 10000){
		_error_msg("write_control_reg - M80 write control register timeout\n");
		return ERR_M80_W_CONTROLREG_TIMEOUT;
	}
			
	if (read_control_reg(nRegNum) != data)
	{
		_error_msg("write_control_reg - nRegNum = %d, is 0x%x, must be = 0x%x\n", nRegNum, read_control_reg(nRegNum), data);
		goto retry;
	}
	return 0;
}


static INT  write_control_reg_word(INT nRegNum, UINT8 data1, UINT8 data2)
{
	UINT32	 uData32;
	INT nTimeOutCount;
	if (nRegNum < 0x80)
		uData32 = (0x80|nRegNum) << 8;
	else
		uData32 = ((nRegNum & 0x7f) << 8) |
				 (((nRegNum >> 7) & 0x7f) | 0x80) << 16;
	uData32 |= 0x02;					/* state flag */
	outpw(REG_ACTL_M80ADDR, uData32);

	outpw(REG_ACTL_M80DATA0, ((data2 << 8) | data1) | 0x8000);

	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | W_IF10_ACT);
	
	for(nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
		if ((inpw(REG_ACTL_M80CON) & W_IF10_ACT) == 0 )
			break;
	}
	if (nTimeOutCount == 10000){
			_error_msg("write_control_reg_word - M80 write control register timeout\n");
			return ERR_M80_W_CONTROLREG_TIMEOUT;
	}
	return 0;
}


static INT read_data_from_RAM(INT addr)
{
	UINT32	 uData32;
	INT nTimeOutCount;
	uData32 = ((addr & 0x7f) << 8) | (((addr >> 7) & 0x7f) << 16) |
			 ((((addr >> 14) & 0x7f) | 0x80) << 24);
	uData32 |= 0x03;					/* state flag */

	outpw(REG_ACTL_M80ADDR, uData32);

	outpw(REG_ACTL_M80DATA0, 0x82);		/* select read buffer #0 */

	outpw(REG_ACTL_M80SIZE, 3);        	/* read buffer ID = 2 */

	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | R_IF10_ACT);
	
	for(nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
		if ((inpw(REG_ACTL_M80CON) & R_IF10_ACT) == 0 )
			break;
	}
	
	if (nTimeOutCount == 10000){
		_error_msg("read_data_from_RAM - M80 read control register timeout\n");
		return ERR_M80_R_CONTROLREG_TIMEOUT;
	}
		
	return ((inpw(REG_ACTL_M80SIZE) >> 16) & 0xff);
}


static INT write_tone_data_to_RAM(UINT32 addr, UINT8 *buff)
{
	UINT32	 uData32;
	INT nTimeOutCount;

	uData32 = ((addr & 0x7f) << 8) | (((addr >> 7) & 0x7f) << 16) |
			 ((((addr >> 14) & 0x7f) | 0x80) << 24);
	uData32 |= 0x02;					/* state flag */
	outpw(REG_ACTL_M80ADDR, uData32);
	
	outpw(REG_ACTL_M80SIZE, 0x8D);
	
	uData32 = buff[0] | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24);
	outpw(REG_ACTL_M80DATA0, uData32);
	uData32 = buff[4] | (buff[5] << 8) | (buff[6] << 16) | (buff[7] << 24);
	outpw(REG_ACTL_M80DATA1, uData32);
	uData32 = buff[8] | (buff[9] << 8) | (buff[10] << 16) | (buff[11] << 24);
	outpw(REG_ACTL_M80DATA2, uData32);
	outpw(REG_ACTL_M80DATA3, buff[12]);
	
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | W_IF11_ACT);
	
	for(nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
		if ((inpw(REG_ACTL_M80CON) & W_IF11_ACT) == 0 )
			break;
	}
	
	if (nTimeOutCount == 10000){
			_error_msg("write_tone_data_to_RAM - M80 write for ROM/RAM timeout\n");
			return ERR_M80_W_RAM_TIMEOUT;
	}
		
	return 0;
}



static INT  ma3_start_dma(UINT32 uLeftAddr, UINT32 uRightAddr, INT nSize)
{
	UINT32	uData32;
	INT nTimeOutCount;

	for(nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
		if ((inpw(REG_ACTL_M80CON) & (X86_PCM_TRANS | BUSY)) == 0 )
			break;
	}
	
	if (nTimeOutCount == 10000){
		_error_msg("ma3_start_dma - M80 X86_PCM_TRANS timeout\n");
		return ERR_M80_X86_PCM_TRANS_TIMEOUT;
	}
	
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~M80_PLAY);

	uData32 = ((uLeftAddr & 0x7f) << 8) | (((uLeftAddr >> 7) & 0x7f) << 16) |
			 ((((uLeftAddr >> 14) & 0x7f) | 0x80) << 24);
	uData32 |= 0x82;					/* state flag */
	outpw(REG_ACTL_M80ADDR, uData32);
	
	uData32 = (uRightAddr & 0x7f) | (((uRightAddr >> 7) & 0x7f) << 8) |
			 ((((uRightAddr >> 14) & 0x7f) | 0x80) << 16);
	outpw(REG_ACTL_M80SRADDR, uData32);

	if (nSize < 128)
		outpw(REG_ACTL_M80SIZE, nSize | 0x80);
	else
		outpw(REG_ACTL_M80SIZE, (nSize & 0x7f) | ((((nSize >> 7) & 0x7f) | 0x80) << 8));
	
	/* set DMA play buffer length */
	outpw(REG_ACTL_PDST_LENGTH, nSize * 2);

	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | M80_PLAY);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | X86_PCM_TRANS);
	
	return 0;
}


static INT  ma3_dma_isr()
{
	int  pg, pg0, pg1, write_cnt;
	INT nTimeOutCount;

	for(nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
		if ((inpw(REG_ACTL_M80CON) & (X86_PCM_TRANS | BUSY)) == 0 )
			break;
	}
	
	if (nTimeOutCount == 10000){
		_error_msg("ma3_start_dma - M80 X86_PCM_TRANS timeout\n");
		return ERR_M80_X86_PCM_TRANS_TIMEOUT;
	}
	
	
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~M80_PLAY);

	if (_bChangeVolume)
	{
		write_intermediate_reg(IMD_REG_ANALOG_SETTING2, _tMA3.sPlayVolume >> 8);
		write_intermediate_reg(IMD_REG_ANALOG_SETTING3, _tMA3.sPlayVolume & 0xff);
#ifdef SPEAKER
		write_intermediate_reg(IMD_REG_ANALOG_SETTING4, _tMA3.sPlayVolume >> 8);
#endif
		_bChangeVolume = 0;
	}

	pg0 = (read_control_reg(337) & 0x1f) << 5;
	pg1 = (read_control_reg(336) & 0x1f) << 5;

	write_intermediate_reg(0x80 | IMD_REG_INT_FLAG, 0x30);	/* clear interrupt */
	
	/* clear GPIO interrupt status */
	outpw(REG_GPIO_IS, 1<<USED_GPIO_NUM);
	outpw(REG_GPIO_IS, 0);

	write_intermediate_reg(0x80 | IMD_REG_INT_FLAG, 0x30);	/* clear interrupt */

	if (pg0 > pg1)
		pg = pg1;
	else
		pg = pg0;
		
	//_debug_msg("PG = %d\n", pg);
	

	if (_uSoftwarePG == pg)
	{
	}
	else if (_uSoftwarePG < pg)
	{
		write_cnt = pg - _uSoftwarePG;
		_tMA3.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, write_cnt);
		ma3_start_dma(WT_PCM_LEFT_ADDR+_uSoftwarePG, WT_PCM_RIGHT_ADDR+_uSoftwarePG, write_cnt);
		_uSoftwarePG = pg;
	}
	else
	{
		write_cnt = 1024 - _uSoftwarePG;
		_tMA3.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, write_cnt);
		ma3_start_dma(WT_PCM_LEFT_ADDR+_uSoftwarePG, WT_PCM_RIGHT_ADDR+_uSoftwarePG, write_cnt);
		_uSoftwarePG = 0;

		if (pg > 0)
		{
			_tMA3.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, pg);
			ma3_start_dma(WT_PCM_LEFT_ADDR, WT_PCM_RIGHT_ADDR, pg);
			_uSoftwarePG = pg;
		}
	}
	return 0;
}


static INT  ma3_init(INT rate)
{
	if (_tMA3.sPlayVolume == 0)
		_tMA3.sPlayVolume = 0x1010;

	/* enable audio controller and M80 interface */
	outpw(REG_ACTL_RESET, PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL);
	Delay(1000);
	outpw(REG_ACTL_CON, AUDIO_EN | AUDCLK_EN | M80_EN | PFIFO_EN | RFIFO_EN | T_DMA_IRQ | R_DMA_IRQ | DMA_EN);
	
	/* reset Audio Controller */
	outpw(REG_ACTL_RESET, ACTL_RESET_BIT);
	Delay(100);
	outpw(REG_ACTL_RESET, 0);
	Delay(100);
	
	/* reset M80 interface */
	outpw(REG_ACTL_RESET,  M80_RESET);
	Delay(100);
	outpw(REG_ACTL_RESET, 0);
	Delay(100);

	if (_bSingleChannel)
		outpw(REG_ACTL_RESET, PLAY_LEFT_CHNNEL);
	else
		outpw(REG_ACTL_RESET, PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL);

	outpw(REG_ACTL_PDST_LENGTH, 256);
	outpw(REG_ACTL_M80CON, CLK_DIV);
	
	/* software reset, wait at least 300ns */
	write_intermediate_reg(IMD_REG_BASIC_SETTING, BS_RST);
	Delay(3000);
	write_intermediate_reg(IMD_REG_BASIC_SETTING, 0);
	Delay(200);

	/* Configure PLL */
	write_intermediate_reg(IMD_REG_BASIC_SETTING, 1);	/* select bank 1 */
	Delay(1000);
	write_intermediate_reg(IMD_REG_PLL_SETTING1, 10);	/* 12.288 MHz */
	//write_intermediate_reg(IMD_REG_PLL_SETTING1, 18);   /* 18.432 MHz */
	
	if (rate == 48000)
		write_intermediate_reg(IMD_REG_PLL_SETTING2, 45);
	else if (rate == 44100)
		write_intermediate_reg(IMD_REG_PLL_SETTING2, 41);
		//write_intermediate_reg(IMD_REG_PLL_SETTING2, 50);	/* 18.432 MHz */
	else if (rate == 32000)
		write_intermediate_reg(IMD_REG_PLL_SETTING2, 30);
	else if (rate == 24000)
		write_intermediate_reg(IMD_REG_PLL_SETTING2, 22);
	else if (rate == 22050)
		write_intermediate_reg(IMD_REG_PLL_SETTING2, 21);
	else if (rate == 16000)
		write_intermediate_reg(IMD_REG_PLL_SETTING2, 15);
	else if (rate == 11025)
		write_intermediate_reg(IMD_REG_PLL_SETTING2, 10);
	else if (rate == 8000)
		write_intermediate_reg(IMD_REG_PLL_SETTING2, 8);
	else
		write_intermediate_reg(IMD_REG_PLL_SETTING2, 45);

	Delay(1000);
	write_intermediate_reg(IMD_REG_BASIC_SETTING, 0);	/* select bank 0 */
	
	/* set DP0 to 0 */
	Delay(1000);
	write_intermediate_reg(IMD_REG_PWR_DIGITAL, 0x0E);
	
	/* wait 2ms, set PLLPD and AP0 to 0 */
	Delay(2000);
	write_intermediate_reg(IMD_REG_PWR_ANALOG, 0x3E);
	
	/* Wait at least 10ms and then set DP1 to 0 */
	Delay(20000);
	write_intermediate_reg(IMD_REG_PWR_DIGITAL, 0x0C);

	/* set DP2 to 0 */
	Delay(100);
	write_intermediate_reg(IMD_REG_PWR_DIGITAL, 0x08);

	/* software reset, wait at least 300ns */
	Delay(100);
	write_intermediate_reg(IMD_REG_BASIC_SETTING, BS_RST);
	Delay(300);
	write_intermediate_reg(IMD_REG_BASIC_SETTING, 0);
	
	/* wait 20.83us and set DP3 to 0 */
	Delay(20);
	write_intermediate_reg(IMD_REG_PWR_DIGITAL, 0);

	/* set AP3, and AP4 to 0 */
	Delay(100);
	write_intermediate_reg(IMD_REG_PWR_ANALOG, 0x6);

#ifdef SPEAKER
	/* set AP1 to 0 */
	write_intermediate_reg(IMD_REG_PWR_ANALOG, 0x4);
	/* after 10us or more, set AP2 to 0 */
	Delay(20);
	write_intermediate_reg(IMD_REG_PWR_ANALOG, 0x0);
#endif	

	/* set EQ amplifier */
	write_intermediate_reg(IMD_REG_ANALOG_SETTING1, 0x1f);
	
	/* set headphone left channel volume */
	write_intermediate_reg(IMD_REG_ANALOG_SETTING2, _tMA3.sPlayVolume >> 8);

	/* set headphone right channel volume */
	write_intermediate_reg(IMD_REG_ANALOG_SETTING3, _tMA3.sPlayVolume & 0xff);
	
	/* set speaker amplifier */
#ifdef SPEAKER
	write_intermediate_reg(IMD_REG_ANALOG_SETTING4, 0x13);
#else
	write_intermediate_reg(IMD_REG_ANALOG_SETTING4, 0);		/* mute */
#endif	
	
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      ma3StartPlay                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Start MA3 playback.                                              */
/*                                                                       */
/* INPUTS                                                                */
/*      fnCallBack  client program provided callback function. The audio */
/*                  driver will call back to get next block of PCM data  */
/*      nSamplingRate  the playback sampling rate. Supported sampling    */
/*                  rate are 48000, 44100, 32000, 24000, 22050, 16000,   */
/*                  11025, and 8000 Hz                                   */
/*      nChannels	number of playback nChannels                         */
/*					1: single channel, otherwise: double nChannels       */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
INT  ma3StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels)
{
  UINT32		loop;
	UINT8		byte;
	INT nTimeOutCount;

	if (nChannels == 1)
		_bSingleChannel = 1;
	else
		_bSingleChannel = 0;
		
	if (_bMA3Playing)
		return ERR_MA3_PLAY_ACTIVE;

	/* reset MA-3 */
	_debug_msg("Reset MA3..");
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~0x100000);	/* GPIO 20 */
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x100000);
	Delay(0x10000);
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~0x100000);
	Delay(0x10000);
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x100000);
	_debug_msg("complete!\n");
	
	ma3_init(nSamplingRate);
	_debug_msg("MA-3 initialized!\n");

	_bMA3Playing = 1;

	_tMA3.nPlaySamplingRate = nSamplingRate;
	_tMA3.fnPlayCallBack = fnCallBack;

	/* set DMA play destination base address */
	outpw(REG_ACTL_PDSTB, _uAuPlayBuffAddr | 0x10000000);

	/* ChVol setting */
	write_control_reg(144, 0x7C);
	write_control_reg(145, 0x7C);
	
	/* Panpot */
	write_control_reg(160, 0x00);	/* channel 0, Lch=0dB, Rch=Mute */ 
	write_control_reg(161, 0x7c);	/* channel 1, Lch=Mute, Rch=0dB */ 
	
	/* Pitch-bend */
	write_control_reg_word(192, 0x08, 0);	
	write_control_reg_word(194, 0x08, 0);	

	/* time setting, timer0 */
	write_control_reg(338, 9);			/* unit for timer is 16 fs */
	//write_control_reg(339, 1);		
	write_control_reg(340, 16);			/* 256 fs */
	write_control_reg(341, 0);

	/* Sequencer setting */
	//write_control_reg_word(350, 0x1b, 0);	
	//write_control_reg(352, 0x04);	/* 256 bytes or less */
	//write_control_reg(353, 0x01);	/* Start the sequencer */

	/* interrupt setting, enable timer0 interrupt */
	write_control_reg(354, 0x10);	

	/* Volume interpolation setting */
	write_control_reg(355, 0x3f);	

	_debug_msg("Writing tone data left - ");
	for (loop = 0; loop < 13; loop++)
	{	_debug_msg("%02x ", _Tone_Setting_Left[loop]);	}
	_debug_msg("\n");
	write_tone_data_to_RAM(TONE_DATA_ADDR_LEFT, _Tone_Setting_Left);
	Delay(10000);

	if (!_bSingleChannel)
	{
		_debug_msg("Writing tone data right - ");
		for (loop = 0; loop < 13; loop++)
		{	_debug_msg("%02x ", _Tone_Setting_Right[loop]);	}
		_debug_msg("\n");
		write_tone_data_to_RAM(TONE_DATA_ADDR_RIGHT, _Tone_Setting_Right);
		Delay(10000);
	}
	
	_debug_msg("Read back left tone data  - \n");
	for (loop = 0; loop < 13; loop++)
	{
		byte = read_data_from_RAM(TONE_DATA_ADDR_LEFT+loop);
		_debug_msg("%02x ", byte);
		if (byte != _Tone_Setting_Left[loop])
		{
			//_debug_msg("\nData mismatch!!\n");
			//exit(0);
		}
	}
	_debug_msg("\n");

	if (!_bSingleChannel)
	{
		_debug_msg("Read back right tone data  - \n");
		for (loop = 0; loop < 13; loop++)
		{
			byte = read_data_from_RAM(TONE_DATA_ADDR_RIGHT+loop);
			_debug_msg("%02x ", byte);
			if (byte != _Tone_Setting_Right[loop])
			{
				//_debug_msg("\nData mismatch!!\n");
				//exit(0);
			}
		}
		_debug_msg("\n");
	}

	/* write 8-bit PCM data to RAM */
	_tMA3.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, 1024);	
	ma3_start_dma(WT_PCM_LEFT_ADDR, WT_PCM_RIGHT_ADDR, 1024);
	
	for(nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
		if ((inpw(REG_ACTL_M80CON) & (X86_PCM_TRANS | BUSY)) == 0 )
			break;
	}
	
	if (nTimeOutCount == 10000){
			_error_msg("ma3_start_dma - M80 X86_PCM_TRANS timeout\n");
			return ERR_M80_X86_PCM_TRANS_TIMEOUT;
	}

	_uSoftwarePG = 0;
		
	_debug_msg("Start playing PCM data...\n");

	/* WT voice 7 setting */
	write_control_reg_word(138, (TONE_DATA_ADDR_LEFT>>8)&0x7f, (TONE_DATA_ADDR_LEFT>>1)&0x7f);
	write_control_reg(140, 0x7c);
	write_control_reg_word(141, 0x05, 0x0);
	write_control_reg(143, 0x40);	/* KeyOn, channel 0 */

	/* WT voice 6 setting */
	if (!_bSingleChannel)
	{
		write_control_reg_word(132, (TONE_DATA_ADDR_RIGHT>>8)&0x7f, (TONE_DATA_ADDR_RIGHT>>1)&0x7f);
		write_control_reg(134, 0x7c);
		write_control_reg_word(135, 0x05, 0x0);
		write_control_reg(137, 0x41);	/* KeyOn, channel 1 */
	}
	
	write_intermediate_reg(IMD_REG_INT_FLAG, 0x30);	/* clear interrupt */
	write_control_reg(341, 0x01);					/* Start timer 0 */
	write_intermediate_reg(0x80 | IMD_REG_INT_FLAG, 0);	/* enable interrupt */

	/* set interrupt trigger type */
	sysSetInterruptType(GPIO_INT_NUM, LOW_LEVEL_SENSITIVE);
	sysInstallISR(IRQ_LEVEL_1, GPIO_INT_NUM, (PVOID)ma3_dma_isr);
    sysSetLocalInterrupt(ENABLE_IRQ);
	sysEnableInterrupt(GPIO_INT_NUM);
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | (1<<USED_GPIO_NUM));

	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      ma3StopPlay                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Stop MA3 playback immdediately.                                 */
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
INT  ma3StopPlay()
{
	INT nTimeOutCount;
	
	sysDisableInterrupt(GPIO_INT_NUM);
	
	for(nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
		if ((inpw(REG_ACTL_M80CON) & (X86_PCM_TRANS | BUSY)) == 0 )
			break;
	}
	
	if (nTimeOutCount == 10000){
			_error_msg("ma3_start_dma - M80 X86_PCM_TRANS timeout\n");
			return ERR_M80_X86_PCM_TRANS_TIMEOUT;
	}
	
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~M80_PLAY);

 	/* software reset, wait at least 300ns */
	write_intermediate_reg(IMD_REG_BASIC_SETTING, BS_RST);
	Delay(3000);
	write_intermediate_reg(IMD_REG_BASIC_SETTING, 0);
	Delay(200);

	_bMA3Playing = 0;
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      ma3SetPlayVolume                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Set MA3 left and right channel play volume.                      */
/*                                                                       */
/* INPUTS                                                                */
/*      ucLeftVol    play volume of left channel                          */
/*      ucRightVol   play volume of left channel                          */
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
INT  ma3SetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol)
{
	_tMA3.sPlayVolume = (ucLeftVol << 8) | ucRightVol;
	_bChangeVolume = 1;
	return 0;
}


#endif	/* HAVE_MA3 */
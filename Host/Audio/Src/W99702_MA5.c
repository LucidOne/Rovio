/****************************************************************************
 * 
 * FILENAME
 *     W99702_MA5.c
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
 *     2004.06.03		Created by Yung-Chang Huang
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
#include "YMU765.h"

#ifdef HAVE_MA5

#define PCM16
#define SPEAKER

#define TONE_DATA_ADDR_LEFT		0x5c00
#define WT_PCM_LEFT_ADDR		0x4000
#define WT_LEFT_START_ADDR_H	0x40
#define WT_LEFT_START_ADDR_L	0x00
#define WT_LEFT_LOOP_START_H	0
#define WT_LEFT_LOOP_START_L	0
#define WT_LEFT_LOOP_END_H		0x4
#define WT_LEFT_LOOP_END_L		0x00
 
#define TONE_DATA_ADDR_RIGHT	0x5800
#define WT_PCM_RIGHT_ADDR		0x4800
#define WT_RIGHT_START_ADDR_H	0x48
#define WT_RIGHT_START_ADDR_L	0x00
#define WT_RIGHT_LOOP_START_H	0
#define WT_RIGHT_LOOP_START_L	0
#define WT_RIGHT_LOOP_END_H		0x4
#define WT_RIGHT_LOOP_END_L		0x00

UINT8 _Tone_Setting_Left[] = 
{
	0x02, 	/* PANPOT=0x00, STM=1, PE=0 */
#ifdef PCM16
	0x01,	/* LFO=1.8Hz, PANOFF=0, MODE=16-bit(2's comp)PCM */
#else	
	0x03,	/* LFO=1.8Hz, PANOFF=0, MODE=8-bit(2's comp)PCM */
#endif
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

UINT8 _Tone_Setting_Right[] = 
{
	0xfa, 	/* PANPOT=0x1f, STM=1, PE=0 */
#ifdef PCM16
	0x01,	/* LFO=1.8Hz, PANOFF=0, MODE=16-bit(2's comp)PCM */
#else	
	0x03,	/* LFO=1.8Hz, PANOFF=0, MODE=8-bit(2's comp)PCM */
#endif
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


static AUDIO_T	_tMA5;
static BOOL		_bMA5Playing = 0;
static BOOL		_bSingleChannel = 0;
static BOOL		_bChangeVolume = 0;
static UINT32 	_uSoftwarePG;


static void Delay(int nCnt)
{
	volatile int  loop;
	for (loop=0; loop<nCnt*10; loop++);
}



static INT read_intermediate_reg(INT nRegId)
{
	INT nTimeOutCount;
	outpw(REG_ACTL_M80ADDR, nRegId);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | R_IF11_ACT);

	
	for(nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
		if ((inpw(REG_ACTL_M80CON) & R_IF11_ACT) == 0 )
			break;
	}
	
	if (nTimeOutCount == 10000){
			_error_msg("read_intermediate_reg - M80 read intermediate register timeout\n");
			return ERR_M80_R_INTERMEDIATEREG_TIMEOUT;
	}

	return ((inpw(REG_ACTL_M80SIZE)>>16) & 0xff);
}


static INT write_intermediate_reg(INT nRegId, UINT8 data)
{
	INT nTimeOutCount;
	outpw(REG_ACTL_M80ADDR, nRegId);
	outpw(REG_ACTL_M80SIZE, 0x1);
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
	INT nTimeOutCount;
	if (nRegNum < 0x80)
		uData32 = (0x80|nRegNum) << 8;
	else
		uData32 = ((nRegNum & 0x7f) << 8) |
				 (((nRegNum >> 7) & 0x7f) | 0x80) << 16;
	uData32 |= 0x03;					/* state flag */
	outpw(REG_ACTL_M80ADDR, uData32);

	outpw(REG_ACTL_M80DATA0, 0x80);		/* select read buffer #0 */

	outpw(REG_ACTL_M80SIZE, 0x1);      	/* read buffer ID = 0 */

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


static INT write_control_reg(INT nRegNum, UINT8 data)
{
	UINT32	 uData32;
	INT nTimeOutCount;
retry:
	if (nRegNum < 0x80)
		uData32 = (0x80|nRegNum) << 8;
	else
		uData32 = ((nRegNum & 0x7f) << 8) |
				 (((nRegNum >> 7) & 0x7f) | 0x80) << 16;
	uData32 |= 0x2;					/* state flag */
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
		_debug_msg("write_control_reg - nRegNum = %d, is 0x%x, must be = 0x%x\n", nRegNum, read_control_reg(nRegNum), data);
		goto retry;
	}
	return 0;
}


static INT write_control_reg_word(INT nRegNum, UINT8 data1, UINT8 data2)
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
	uData32 |= 0x3;					/* state flag */

	outpw(REG_ACTL_M80ADDR, uData32);

	outpw(REG_ACTL_M80DATA0, 0x80);		/* select read buffer #0 */

	outpw(REG_ACTL_M80SIZE, 1);        	/* read buffer ID = 0 */

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


static INT  ma5_start_dma(UINT32 left_addr, UINT32 right_addr, INT size)
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

	uData32 = ((left_addr & 0x7f) << 8) | (((left_addr >> 7) & 0x7f) << 16) |
			 ((((left_addr >> 14) & 0x7f) | 0x80) << 24);
	uData32 |= 0x82;					/* state flag */
	outpw(REG_ACTL_M80ADDR, uData32);
	
	uData32 = (right_addr & 0x7f) | (((right_addr >> 7) & 0x7f) << 8) |
			 ((((right_addr >> 14) & 0x7f) | 0x80) << 16);
	outpw(REG_ACTL_M80SRADDR, uData32);

	if (size < 128)
		outpw(REG_ACTL_M80SIZE, size | 0x80);
	else
		outpw(REG_ACTL_M80SIZE, (size & 0x7f) | ((((size >> 7) & 0x7f) | 0x80) << 8));
	
	/* set DMA play buffer length */
	outpw(REG_ACTL_PDST_LENGTH, size * 2);

	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) | M80_PLAY);
	outpw(REG_ACTL_M80CON, inpw(REG_ACTL_M80CON) | X86_PCM_TRANS);
	
	return 0;
}


static INT ma5_dma_isr()
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
		write_intermediate_reg(IMD_HPVOL_L, _tMA5.sPlayVolume >> 8);
		write_intermediate_reg(IMD_HPVOL_R, _tMA5.sPlayVolume & 0xff);
#ifdef SPEAKER
		write_intermediate_reg(IMD_SPVOL, _tMA5.sPlayVolume >> 8);
#endif
		_bChangeVolume = 0;
	}

	pg0 = (read_intermediate_reg(25) & 0x3f) << 4;
	pg1 = (read_intermediate_reg(26) & 0x3f) << 4;
	
	write_intermediate_reg(0x80 | IMD_INT_FLAG1, 0x03);	/* clear interrupt */
	outpw(REG_GPIO_IS, 1<<USED_GPIO_NUM);
	outpw(REG_GPIO_IS, 0);

	if (pg0 > pg1)
		pg = pg1;
	else
		pg = pg0;

#ifdef PCM16
	pg = pg * 2;
#endif

	//sysPrintf("gp = %d\n", pg);
	if (_uSoftwarePG == pg)
	{
	}
	else if (_uSoftwarePG < pg)
	{
		write_cnt = pg - _uSoftwarePG;
		_tMA5.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, write_cnt);
		ma5_start_dma(WT_PCM_LEFT_ADDR+_uSoftwarePG, WT_PCM_RIGHT_ADDR+_uSoftwarePG, write_cnt);
		_uSoftwarePG = pg;
	}
	else
	{
#ifdef PCM16
		write_cnt = 2048 - _uSoftwarePG;
#else
		write_cnt = 1024 - _uSoftwarePG;
#endif		
		_tMA5.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, write_cnt);
		ma5_start_dma(WT_PCM_LEFT_ADDR+_uSoftwarePG, WT_PCM_RIGHT_ADDR+_uSoftwarePG, write_cnt);
		_uSoftwarePG = 0;

		if (pg > 0)
		{
			_tMA5.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, pg);
			ma5_start_dma(WT_PCM_LEFT_ADDR, WT_PCM_RIGHT_ADDR, pg);
			_uSoftwarePG = pg;
		}
	}
	return 0;
}


INT  ma5_init(INT rate)
{
	INT		idx;

	if (_tMA5.sPlayVolume == 0)
		_tMA5.sPlayVolume = 0x1010;

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

	outpw(REG_ACTL_RESET, PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL);
	outpw(REG_ACTL_PDST_LENGTH, 256);
	outpw(REG_ACTL_M80CON, CLK_DIV);
	
	/* write 0x00 to #7~#15, #63 */
	for (idx = 7; idx <= 15; idx++)	
		write_intermediate_reg(idx, 0);
		

	write_intermediate_reg(63, 0);
	
	

	/* Configure PLL */
	write_intermediate_reg(IMD_PLL_SETTING1, 12);
	
	if (rate == 48000)
		write_intermediate_reg(IMD_PLL_SETTING2, 72);
	else if (rate == 44100)
		write_intermediate_reg(IMD_PLL_SETTING2, 66);
	else if (rate == 32000)
		write_intermediate_reg(IMD_PLL_SETTING2, 48);
	else if (rate == 24000)
		write_intermediate_reg(IMD_PLL_SETTING2, 36);
	else if (rate == 22050)
		write_intermediate_reg(IMD_PLL_SETTING2, 33);
	else if (rate == 16000)
		write_intermediate_reg(IMD_PLL_SETTING2, 24);
	else if (rate == 12000)
		write_intermediate_reg(IMD_PLL_SETTING2, 18);
	else if (rate == 11025)
		write_intermediate_reg(IMD_PLL_SETTING2, 16);
	else if (rate == 8000)
		write_intermediate_reg(IMD_PLL_SETTING2, 12);
	else
		write_intermediate_reg(IMD_PLL_SETTING2, 72);
	Delay(10000);
	

	sysPrintf("PLL1=%d\n", read_intermediate_reg(IMD_PLL_SETTING1));
	sysPrintf("PLL2=%d\n", read_intermediate_reg(IMD_PLL_SETTING2));
	
	/* set DP0 to 0 */
	write_intermediate_reg(IMD_PWR_DIGITAL, 0x0E);
	
	/* wait 2ms, set PLLPD and AP0 to 0 */
	Delay(2000);
	write_intermediate_reg(IMD_PWR_ANALOG, 0x3E);
	
	/* Wait at least 10ms and then set DP1 to 0 */
	Delay(20000);
	write_intermediate_reg(IMD_PWR_DIGITAL, 0x0C);

	/* set DP2 to 0 */
	Delay(100);
	write_intermediate_reg(IMD_PWR_DIGITAL, 0x08);

	/* software reset, wait at least 300ns */
reset:
	_debug_msg("Software reset...\n");
	Delay(100);
	write_intermediate_reg(IMD_BASIC_SETTING, BS_RST);
	Delay(2000);
	write_intermediate_reg(IMD_BASIC_SETTING, 0);
	Delay(3000);

	if ((read_control_reg(208) != 0x3C) ||
		(read_control_reg(209) != 0x3C) ||
		(read_control_reg(210) != 0x3C) ||
		(read_control_reg(223) != 0x3C))
		goto reset;
		
	_debug_msg("reset complete!\n");

	/* wait 41.7us and set DP3 to 0 */
	Delay(5000);
	write_intermediate_reg(IMD_PWR_DIGITAL, 0);

	/* set AP3, and AP4 to 0 */
	write_intermediate_reg(IMD_PWR_ANALOG, 0x6);

#ifdef SPEAKER
	/* set AP1 to 0 */
	write_intermediate_reg(IMD_PWR_ANALOG, 0x4);
	
	/* after 10us or more, set AP2 to 0 */
	Delay(2000);
	write_intermediate_reg(IMD_PWR_ANALOG, 0x0);
#endif	

	/* set EQ amplifier */
	write_intermediate_reg(IMD_EQVAL, 0);
	
	/* set headphone left channel volume */
	write_intermediate_reg(IMD_HPVOL_L, _tMA5.sPlayVolume >> 8);

	/* set headphone right channel volume */
	write_intermediate_reg(IMD_HPVOL_R, _tMA5.sPlayVolume & 0xff);
	
	/* set speaker amplifier */
#ifdef SPEAKER
	write_intermediate_reg(IMD_SPVOL, _tMA5.sPlayVolume >> 8);
#else
	write_intermediate_reg(IMD_REG_ANALOG_SETTING4, 0);		/* mute */
#endif	

	write_intermediate_reg(IMD_MASTER_VOL_L, 0xf1);
	write_intermediate_reg(IMD_MASTER_VOL_R, 0xf1);
	write_intermediate_reg(IMD_STM_VOL, 0x78);

	/* configure sequencer */
	write_intermediate_reg(35, 0);
	write_intermediate_reg(36, 10 << 2);		/* 100/192 fs */
	write_intermediate_reg(38, 1);		

	/* configure timer0 */	
	write_intermediate_reg(46, 9);		/* unit for timer is 16 fs */
	write_intermediate_reg(47, 16);	/* 16 x unit time = 256 fs */
	//write_intermediate_reg(48, 0x1);	/* start timer0 */		

	/* Synthesizer mode setting */
	write_intermediate_reg(IMD_SYNTH_MODE, 0x02);	/* set WT as stream playback mode */

	/* Stream playback interrupt setting */
#ifdef PCM16
	write_intermediate_reg(IMD_STREAM_INT, 0x20);	/* size=1024 bytes, 1/2 generate interrupt */
#else
	write_intermediate_reg(IMD_STREAM_INT, 0x10);	/* size=1024 bytes, 1/2 generate interrupt */
#endif	
	return 0;
}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      ma5StartPlay                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Start MA5 playback.                                             */
/*                                                                       */
/* INPUTS                                                                */
/*      fnCallBack     client program provided callback function. The audio */
/*                  driver will call back to get next block of PCM data  */
/*      nSamplingRate  the playback sampling rate. Supported sampling    */
/*                  rate are 48000, 44100, 32000, 24000, 22050, 16000,   */
/*                  11025, and 8000 Hz                                   */
/*      nChannels	number of playback nChannels                          */
/*					1: single channel, otherwise: double nChannels        */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
INT  ma5StartPlay(AU_CB_FUNC_T *fnCallBack, INT nSamplingRate, INT nChannels)
{
  UINT32		loop;
	UINT8		byte;
	INT nTimeOutCount;

	if (_bMA5Playing)
		return ERR_MA5_PLAY_ACTIVE;
	
	if (nChannels == 1)
		_bSingleChannel = 1;
	else
		_bSingleChannel = 0;

	/* reset MA-5 */
	_debug_msg("Reset MA5..");
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~0x100000);	/* GPIO 20 */
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x100000);
	Delay(0x10000);
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~0x100000);
	Delay(0x10000);
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x100000);
	_debug_msg("complet!\n");
	
	ma5_init(nSamplingRate);
	_debug_msg("MA-5 initialized!\n");

	_bMA5Playing = 1;
	_tMA5.nPlaySamplingRate = nSamplingRate;
	_tMA5.fnPlayCallBack = fnCallBack;

	/* set DMA play destination base address */
	outpw(REG_ACTL_PDSTB, _uAuPlayBuffAddr | 0x10000000);

	/* ChVol setting */
	write_control_reg(192, 0x7C);
	write_control_reg(193, 0x7C);
	
	/* Panpot */
	write_control_reg(208, 0x00);	/* channel 0, Lch=0dB, Rch=Mute */ 
	write_control_reg(209, 0x7c);	/* channel 1, Lch=Mute, Rch=0dB */ 
	
	/* Pitch-bend */
	//write_control_reg_word(240, 0x10, 0);	/* 2.0 */
	//write_control_reg_word(242, 0x10, 0);	/* 2.0 */
	write_control_reg_word(240, 0x08, 0);	/* 1.0 */
	write_control_reg_word(242, 0x08, 0);	/* 1.0 */
	//write_control_reg_word(240, 0x07, 0x7E);	/* 0.999 */
	//write_control_reg_word(242, 0x07, 0x7E);	/* 0.999 */
	//write_control_reg_word(240, 0x04, 0);		/* 0.5 */
	//write_control_reg_word(242, 0x04, 0);		/* 0.5 */

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
	
	write_intermediate_reg(12,6);
	/* write 16-bit PCM data to RAM */
#ifdef PCM16	
	_tMA5.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, 2048);	
	ma5_start_dma(WT_PCM_LEFT_ADDR, WT_PCM_RIGHT_ADDR, 2048);
#else
	_tMA5.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, 1024);	
	ma5_start_dma(WT_PCM_LEFT_ADDR, WT_PCM_RIGHT_ADDR, 1024);
#endif	
	

	for(nTimeOutCount=0;nTimeOutCount<10000;nTimeOutCount++){
		if ((inpw(REG_ACTL_M80CON) & (X86_PCM_TRANS | BUSY)) == 0 )
			break;
	}
	
	if (nTimeOutCount == 10000){
			_error_msg("ma3_start_dma - M80 X86_PCM_TRANS timeout\n");
			return ERR_M80_X86_PCM_TRANS_TIMEOUT;
	}
	outpw(REG_ACTL_RESET, inpw(REG_ACTL_RESET) & ~M80_PLAY);
	
	_debug_msg("Start playing PCM data...\n");

	/* WT voice #25 setting */
	write_control_reg_word(422, (TONE_DATA_ADDR_LEFT>>8)&0x7f, (TONE_DATA_ADDR_LEFT>>1)&0x7f);
	write_control_reg(424, 0x7c);
	write_control_reg_word(425, 0x28, 0x00);
	write_control_reg(427, 0x40);	/* KeyOn, channel 0 */

	/* WT voice #26 setting */
	if (!_bSingleChannel)
	{
		write_control_reg_word(428, (TONE_DATA_ADDR_RIGHT>>8)&0x7f, (TONE_DATA_ADDR_RIGHT>>1)&0x7f);
		write_control_reg(430, 0x7c);
		write_control_reg_word(431, 0x28, 0x00);
		write_control_reg(433, 0x41);	/* KeyOn, channel 1 */
	}

	/* enable stream playback interrupt interrupt */
	write_intermediate_reg(IMD_INT_MASK1, 0x03);
	write_intermediate_reg(0x80 | IMD_INT_FLAG1, 0x3);	/* enable interrupt */

	/* Install ISR */
	sysSetInterruptType(GPIO_INT_NUM, LOW_LEVEL_SENSITIVE);
	sysInstallISR(IRQ_LEVEL_1, GPIO_INT_NUM, (PVOID)ma5_dma_isr);
    sysSetLocalInterrupt(ENABLE_IRQ);
	sysEnableInterrupt(GPIO_INT_NUM);
	outpw(REG_GPIO_DEBOUNCE, inpw(REG_GPIO_DEBOUNCE) | (1<<USED_GPIO_NUM));
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | (1<<USED_GPIO_NUM));
	
	
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      ma5StopPlay                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Stop MA5 playback immdediately.                                 */
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
INT  ma5StopPlay()
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
	write_intermediate_reg(IMD_BASIC_SETTING, BS_RST);
	Delay(2000);
	write_intermediate_reg(IMD_BASIC_SETTING, 0);
	Delay(300);

	/* release buffer */
	_bMA5Playing = 0;
	return 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      ma5SetPlayVolume                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Set MA5 left and right channel play volume.                      */
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
INT  ma5SetPlayVolume(UINT8 ucLeftVol, UINT8 ucRightVol)
{
	_tMA5.sPlayVolume = (ucLeftVol << 8) | ucRightVol;
	_bChangeVolume = 1;
	return 0;
}


#endif	/* HAVE_MA5 */
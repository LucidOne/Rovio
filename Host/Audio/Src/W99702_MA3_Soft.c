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
#define WT_LEFT_LOOP_END_H		0x3
#define WT_LEFT_LOOP_END_L		0x00

#define TONE_DATA_ADDR_RIGHT	0x5800
#define WT_PCM_RIGHT_ADDR		0x5820
#define WT_RIGHT_START_ADDR_H	0x58
#define WT_RIGHT_START_ADDR_L	0x20
#define WT_RIGHT_LOOP_START_H	0
#define WT_RIGHT_LOOP_START_L	0
#define WT_RIGHT_LOOP_END_H		0x3
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


static UINT8 soft_read_state_flag_reg()
{
	UINT8			data8;
	//volatile INT  	loop;

	outpw(REG_ACTL_M80DATA0, SOFT_CON_REQ | SOFT_CON_W | SOFT_CON_R);
	//while (!(inpw(ACTL_M80CON) & BUSY))
	//	;
	
	//for (loop=0; loop<3; loop++);
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_A0 | SOFT_CON_REQ | SOFT_CON_W));
	//for (loop=0; loop<10; loop++);
	data8 =	inpw(REG_ACTL_M80DATA0) & 0xff;

	outpw(REG_ACTL_M80DATA0, SOFT_CON_CS | SOFT_CON_REL | SOFT_CON_W | SOFT_CON_R);
	//for (loop=0; loop<2; loop++);
	return data8;
}


static void soft_write_state_flag_reg(UINT8 data)
{
	UINT32			data32;
	//volatile INT  	loop;

	outpw(REG_ACTL_M80DATA0, SOFT_CON_REQ | SOFT_CON_W | SOFT_CON_R);
	//while (!(inpw(ACTL_M80CON) & BUSY))
	//	;
	
	data32 = SOFT_CON_A0 | SOFT_CON_REQ | SOFT_CON_R;
	//for (loop=0; loop<3; loop++);
	outpw(REG_ACTL_M80DATA0, data32 | data);
	//for (loop=0; loop<10; loop++);
	
	outpw(REG_ACTL_M80DATA0, SOFT_CON_CS | SOFT_CON_REL | SOFT_CON_W | SOFT_CON_R);
	//for (loop=0; loop<2; loop++);
}


static UINT8 soft_read_data_reg()
{
	UINT8			data8;
	//volatile INT  	loop;

	outpw(REG_ACTL_M80DATA0, SOFT_CON_REQ | SOFT_CON_W | SOFT_CON_R);
	//while (!(inpw(ACTL_M80CON) & BUSY))
	//	;
	
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_A1 | SOFT_CON_REQ | SOFT_CON_W));
	//for (loop=0; loop<3; loop++);
	data8 = inpw(REG_ACTL_M80DATA0) & 0xff;

	outpw(REG_ACTL_M80DATA0, SOFT_CON_CS | SOFT_CON_REL | SOFT_CON_W | SOFT_CON_R);
	//for (loop=0; loop<2; loop++);
	return data8;
}


static void soft_write_data_reg(UINT8 data)
{
	//volatile INT  	loop;

	outpw(REG_ACTL_M80DATA0, SOFT_CON_REQ | SOFT_CON_W | SOFT_CON_R);
	//while (!(inpw(ACTL_M80CON) & BUSY))
	//	;
	
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_A1 | SOFT_CON_REQ | SOFT_CON_R));
	//for (loop=0; loop<2; loop++);
	outpw(REG_ACTL_M80DATA0, SOFT_CON_A1 | SOFT_CON_REQ | SOFT_CON_R | data);
	//for (loop=0; loop<10; loop++);
	
	outpw(REG_ACTL_M80DATA0, SOFT_CON_CS | SOFT_CON_REL | SOFT_CON_W | SOFT_CON_R);
	//for (loop=0; loop<2; loop++);
}


#if 0
static UINT8 soft_read_intermediate_reg(INT reg_id)
{
	soft_write_state_flag_reg(reg_id);
	return soft_read_data_reg();
}
#endif


static void soft_write_intermediate_reg(INT reg_id, UINT8 data)
{
	soft_write_state_flag_reg(reg_id);
	soft_write_data_reg(data);
}


static int soft_WaitImmediateFifoEmpty()
{
	volatile UINT32	retry = 0;
	do 
	{
		if (soft_read_state_flag_reg() & ST_EMP_W)
			break;

		if ( retry++ > 0x10000 )
		{
			printf("soft_WaitImmediateFifoEmpty - timeout! (%x)\n", soft_read_state_flag_reg());
			return -1;
		}
	} while (1);
	return 0;
}



static UINT8  soft_read_control_reg(INT reg_num)
{
	volatile UINT32	 retry = 0;

	if (soft_WaitImmediateFifoEmpty() < 0)
		return 0;
	
	if (reg_num < 0x80)
	{
		soft_write_state_flag_reg(IMD_REG_IR_SETTING);
		soft_write_data_reg(0x80|reg_num);
		soft_write_data_reg(0x80);		/* Radr==0, select buffer #0 */
	}
	else
	{
		soft_write_state_flag_reg(IMD_REG_IR_SETTING);
		soft_write_data_reg( reg_num & 0x7F );
		soft_write_data_reg( 0x80 | ((reg_num >> 7) & 0x7f) );
		soft_write_data_reg(0x80);		/* Radr==0, select buffer #0 */
	}
	
	/* wait for data ready in read buffer #0 */
	for (retry = 0; retry < 0x1000; retry++)
	{
		if ((soft_read_state_flag_reg() & (ST_BUSY|ST_VALID_R0)) == ST_VALID_R0)
			break;
	}
	if (retry >= 0x1000)
	{	
		printf("soft_read_control_reg - retry time-out (2), 0x%x\n", soft_read_state_flag_reg());
		return 0;
	}
	
	/* read data from buffer #0 */
	soft_write_state_flag_reg(ST_VALID_R0|IMD_REG_READ_DATA0);
	return soft_read_data_reg();
}


static void  soft_write_control_reg(INT reg_num, UINT8 data)
{
retry:

	if (soft_WaitImmediateFifoEmpty() < 0)
		return;
	
	if (reg_num < 0x80)
	{
		soft_write_state_flag_reg(IMD_REG_IW_SETTING);
		soft_write_data_reg(0x80|reg_num);
	}
	else
	{
		soft_write_state_flag_reg(IMD_REG_IW_SETTING);
		soft_write_data_reg( reg_num & 0x7F );
		soft_write_data_reg( 0x80 | ((reg_num >> 7) & 0x7f) );
	}
	
	soft_write_data_reg(0x80|data);
	
	if (soft_read_control_reg(reg_num) != data)
	{
		printf("soft_write_control_reg - reg_num = %d, is 0x%x, must be = 0x%x\n", reg_num, soft_read_control_reg(reg_num), data);
		goto retry;
	}
}




static void  soft_write_control_reg_word(INT reg_num, UINT8 data1, UINT8 data2)
{
	if (soft_WaitImmediateFifoEmpty() < 0)
		return;
	
	if (reg_num < 0x80)
	{
		soft_write_state_flag_reg(IMD_REG_IW_SETTING);
		soft_write_data_reg(0x80|reg_num);
	}
	else
	{
		soft_write_state_flag_reg(IMD_REG_IW_SETTING);
		soft_write_data_reg( reg_num & 0x7F );
		soft_write_data_reg( 0x80 | ((reg_num >> 7) & 0x7f) );
	}
	
	soft_write_data_reg(data1);
	soft_write_data_reg(0x80|data2);
}



static UINT8  soft_read_data_from_RAM(INT addr)
{
	UINT8	reg;
	volatile UINT32	 retry = 0;

	if (soft_WaitImmediateFifoEmpty() < 0)
		return 0;
	
	soft_write_state_flag_reg(IMD_REG_IR_SETTING);
	soft_write_data_reg( addr & 0x7F );
	soft_write_data_reg( (addr >> 7) & 0x7f );
	soft_write_data_reg( 0x80 | ((addr >> 14) & 0x7f) );
	soft_write_data_reg(0x80);		/* Radr==0, select buffer #0 */
	
	/* wait for data ready in read buffer #1 */
	for (retry = 0; retry < 0x1000; retry++)
	{
		if ((soft_read_state_flag_reg() & (ST_BUSY|ST_VALID_R0)) == ST_VALID_R0)
			break;
	}
	if (retry >= 0x1000)
	{
		printf("soft_read_data_from_RAM - retry time-out (2), 0x%x\n", soft_read_state_flag_reg());
		return 0;
	}
	
	/* read data from buffer #0 */
	soft_write_state_flag_reg(IMD_REG_READ_DATA0);
	reg = soft_read_data_reg();
	soft_write_state_flag_reg(ST_VALID_R0);
	return reg;
}



static void  soft_write_data_to_RAM(UINT32 addr, UINT16 size, UINT8 *buff)
{
	volatile UINT32	 i, write_size;

	if (soft_WaitImmediateFifoEmpty() < 0)
		return;
	
	soft_write_state_flag_reg(IMD_REG_IW_SETTING);

	/* address section */
	soft_write_data_reg( addr & 0x7F );
	soft_write_data_reg( (addr >> 7) & 0x7f );
	soft_write_data_reg( 0x80 | ((addr >> 14) & 0x7f) );
	
	/* data section */
	if (size < 0x80)
	{
		soft_write_data_reg(0x80|size);
	}
	else
	{
		soft_write_data_reg( size & 0x7F );
		soft_write_data_reg( 0x80 | ((size >> 7) & 0x7f) );
	}
	
	while (size > 0)
	{
		if (soft_WaitImmediateFifoEmpty() < 0)
			return;

		write_size = (size < 64) ? size : 64;
		
		for (i = 0; i < write_size; i++, buff++)
			soft_write_data_reg(*buff);
			
		size -= write_size;
	}
}



static void  ma3_dma_isr()
{
	int  pg, pg0, pg1, write_cnt;

	if (_bChangeVolume)
	{
		soft_write_intermediate_reg(IMD_REG_ANALOG_SETTING2, _tMA3.sPlayVolume >> 8);
		soft_write_intermediate_reg(IMD_REG_ANALOG_SETTING3, _tMA3.sPlayVolume & 0xff);
#ifdef SPEAKER
		soft_write_intermediate_reg(IMD_REG_ANALOG_SETTING4, _tMA3.sPlayVolume >> 8);
#endif
		_bChangeVolume = 0;
	}

	pg0 = (soft_read_control_reg(337) & 0x1f) << 5;
	pg1 = (soft_read_control_reg(336) & 0x1f) << 5;

	soft_write_intermediate_reg(0x80 | IMD_REG_INT_FLAG, 0x30);	/* clear interrupt */
	
	/* clear GPIO interrupt status */
	outpw(REG_GPIO_IS, 1<<USED_GPIO_NUM);
	//outpw(REG_GPIO_IS, 0);

	soft_write_intermediate_reg(0x80 | IMD_REG_INT_FLAG, 0x30);	/* clear interrupt */

	if (pg0 > pg1)
		pg = pg1;
	else
		pg = pg0;
		
	_debug_msg("PG = %d, %d\n", pg0, pg1);

	if (_uSoftwarePG == pg)
	{
	}
	else if (_uSoftwarePG < pg)
	{
		write_cnt = pg - _uSoftwarePG;
		_tMA3.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, write_cnt);
		soft_write_data_to_RAM(WT_PCM_LEFT_ADDR+_uSoftwarePG, write_cnt, (UINT8 *)_uAuPlayBuffAddr);
		soft_write_data_to_RAM(WT_PCM_RIGHT_ADDR+_uSoftwarePG, write_cnt, (UINT8 *)_uAuPlayBuffAddr+write_cnt);
		_uSoftwarePG = pg;
	}
	else
	{
		write_cnt = 768 - _uSoftwarePG;
		_tMA3.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, write_cnt);
		soft_write_data_to_RAM(WT_PCM_LEFT_ADDR+_uSoftwarePG, write_cnt, (UINT8 *)_uAuPlayBuffAddr);
		soft_write_data_to_RAM(WT_PCM_RIGHT_ADDR+_uSoftwarePG, write_cnt, (UINT8 *)_uAuPlayBuffAddr+write_cnt);
		_uSoftwarePG = 0;

		if (pg > 0)
		{
			_tMA3.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, pg);
			soft_write_data_to_RAM(WT_PCM_LEFT_ADDR, pg, (UINT8 *)_uAuPlayBuffAddr);
			soft_write_data_to_RAM(WT_PCM_RIGHT_ADDR, pg, (UINT8 *)_uAuPlayBuffAddr+pg);
			_uSoftwarePG = pg;
		}
	}

	soft_write_intermediate_reg(0x80 | IMD_REG_INT_FLAG, 0);	/* enable interrupt */
}


static INT  ma3_init(INT rate)
{
	if (_tMA3.sPlayVolume == 0)
		_tMA3.sPlayVolume = 0x1010;

	/* enable audio controller and M80 interface */
	outpw(REG_ACTL_CON, AUDIO_EN | AUDCLK_EN | M80_EN | PFIFO_EN | RFIFO_EN | T_DMA_IRQ | R_DMA_IRQ | DMA_EN);
	outpw(REG_ACTL_RESET, PLAY_RIGHT_CHNNEL | PLAY_LEFT_CHNNEL);
	
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
	
	/* SOFTWARE mode */
	outpw(REG_ACTL_M80CON, SOFT_CON);
	outpw(REG_ACTL_M80DATA0, (SOFT_CON_CS | SOFT_CON_REL | SOFT_CON_W | SOFT_CON_R) << 8);

	/* software reset, wait at least 300ns */
	soft_write_intermediate_reg(IMD_REG_BASIC_SETTING, BS_RST);
	Delay(3000);
	soft_write_intermediate_reg(IMD_REG_BASIC_SETTING, 0);
	Delay(200);

	/* Configure PLL */
	soft_write_intermediate_reg(IMD_REG_BASIC_SETTING, 1);	/* select bank 1 */
	Delay(1000);
	//soft_write_intermediate_reg(IMD_REG_PLL_SETTING1, 10);	/* 12.288 MHz */
	soft_write_intermediate_reg(IMD_REG_PLL_SETTING1, 18);   /* 18.432 MHz */
	
	if (rate == 48000)
		soft_write_intermediate_reg(IMD_REG_PLL_SETTING2, 45);
	else if (rate == 44100)
		//soft_write_intermediate_reg(IMD_REG_PLL_SETTING2, 41);
		soft_write_intermediate_reg(IMD_REG_PLL_SETTING2, 50);	/* 18.432 MHz */
	else if (rate == 32000)
		soft_write_intermediate_reg(IMD_REG_PLL_SETTING2, 30);
	else if (rate == 24000)
		soft_write_intermediate_reg(IMD_REG_PLL_SETTING2, 22);
	else if (rate == 22050)
		soft_write_intermediate_reg(IMD_REG_PLL_SETTING2, 21);
	else if (rate == 16000)
		soft_write_intermediate_reg(IMD_REG_PLL_SETTING2, 15);
	else if (rate == 11025)
		soft_write_intermediate_reg(IMD_REG_PLL_SETTING2, 10);
	else if (rate == 8000)
		soft_write_intermediate_reg(IMD_REG_PLL_SETTING2, 8);
	else
		soft_write_intermediate_reg(IMD_REG_PLL_SETTING2, 45);

	Delay(1000);
	soft_write_intermediate_reg(IMD_REG_BASIC_SETTING, 0);	/* select bank 0 */
	
	/* set DP0 to 0 */
	Delay(1000);
	soft_write_intermediate_reg(IMD_REG_PWR_DIGITAL, 0x0E);
	
	/* wait 2ms, set PLLPD and AP0 to 0 */
	Delay(2000);
	soft_write_intermediate_reg(IMD_REG_PWR_ANALOG, 0x3E);
	
	/* Wait at least 10ms and then set DP1 to 0 */
	Delay(20000);
	soft_write_intermediate_reg(IMD_REG_PWR_DIGITAL, 0x0C);

	/* set DP2 to 0 */
	Delay(100);
	soft_write_intermediate_reg(IMD_REG_PWR_DIGITAL, 0x08);

	/* software reset, wait at least 300ns */
	Delay(100);
	soft_write_intermediate_reg(IMD_REG_BASIC_SETTING, BS_RST);
	Delay(300);
	soft_write_intermediate_reg(IMD_REG_BASIC_SETTING, 0);
	
	/* wait 20.83us and set DP3 to 0 */
	Delay(20);
	soft_write_intermediate_reg(IMD_REG_PWR_DIGITAL, 0);

	/* set AP3, and AP4 to 0 */
	Delay(100);
	soft_write_intermediate_reg(IMD_REG_PWR_ANALOG, 0x6);

#ifdef SPEAKER
	/* set AP1 to 0 */
	soft_write_intermediate_reg(IMD_REG_PWR_ANALOG, 0x4);
	/* after 10us or more, set AP2 to 0 */
	Delay(20);
	soft_write_intermediate_reg(IMD_REG_PWR_ANALOG, 0x0);
#endif	

	/* set EQ amplifier */
	soft_write_intermediate_reg(IMD_REG_ANALOG_SETTING1, 0x1f);
	
	/* set headphone left channel volume */
	soft_write_intermediate_reg(IMD_REG_ANALOG_SETTING2, _tMA3.sPlayVolume >> 8);

	/* set headphone right channel volume */
	soft_write_intermediate_reg(IMD_REG_ANALOG_SETTING3, _tMA3.sPlayVolume & 0xff);
	
	/* set speaker amplifier */
#ifdef SPEAKER
	soft_write_intermediate_reg(IMD_REG_ANALOG_SETTING4, 0x13);
#else
	soft_write_intermediate_reg(IMD_REG_ANALOG_SETTING4, 0);		/* mute */
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

	if (nChannels == 1)
		_bSingleChannel = 1;
	else
		_bSingleChannel = 0;
		
	if (_bMA3Playing)
		return ERR_MA3_PLAY_ACTIVE;
	
	ma3_init(nSamplingRate);
	_debug_msg("MA-3 initialized!\n");

	/* set interrupt trigger type */
	sysSetInterruptType(GPIO_INT_NUM, LOW_LEVEL_SENSITIVE);
	sysInstallISR(IRQ_LEVEL_1, GPIO_INT_NUM, (PVOID)ma3_dma_isr);
    sysSetLocalInterrupt(ENABLE_IRQ);
	sysEnableInterrupt(GPIO_INT_NUM);
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | (1<<USED_GPIO_NUM));

	_bMA3Playing = 1;

	_tMA3.nPlaySamplingRate = nSamplingRate;
	_tMA3.fnPlayCallBack = fnCallBack;

	/* set DMA play destination base address */
	outpw(REG_ACTL_PDSTB, _uAuPlayBuffAddr | 0x10000000);

	/* ChVol setting */
	soft_write_control_reg(144, 0x7C);
	soft_write_control_reg(145, 0x7C);
	
	/* Panpot */
	soft_write_control_reg(160, 0x00);	/* channel 0, Lch=0dB, Rch=Mute */ 
	soft_write_control_reg(161, 0x7c);	/* channel 1, Lch=Mute, Rch=0dB */ 
	
	/* Pitch-bend */
	soft_write_control_reg_word(192, 0x08, 0);	
	soft_write_control_reg_word(194, 0x08, 0);	

	/* time setting, timer0 */
	soft_write_control_reg(338, 9);			/* unit for timer is 16 fs */
	//soft_write_control_reg(339, 1);		
	soft_write_control_reg(340, 16);			/* 256 fs */
	soft_write_control_reg(341, 0);

	/* Sequencer setting */
	//soft_write_control_reg_word(350, 0x1b, 0);	
	//soft_write_control_reg(352, 0x04);	/* 256 bytes or less */
	//soft_write_control_reg(353, 0x01);	/* Start the sequencer */

	/* interrupt setting, enable timer0 interrupt */
	soft_write_control_reg(354, 0x10);	

	/* Volume interpolation setting */
	soft_write_control_reg(355, 0x3f);	

	_debug_msg("Writing tone data left - ");
	for (loop = 0; loop < 13; loop++)
	{	_debug_msg("%02x ", _Tone_Setting_Left[loop]);	}
	_debug_msg("\n");
	soft_write_data_to_RAM(TONE_DATA_ADDR_LEFT, 13, _Tone_Setting_Left);
	Delay(10000);

	if (!_bSingleChannel)
	{
		_debug_msg("Writing tone data right - ");
		for (loop = 0; loop < 13; loop++)
		{	_debug_msg("%02x ", _Tone_Setting_Right[loop]);	}
		_debug_msg("\n");
		soft_write_data_to_RAM(TONE_DATA_ADDR_RIGHT, 13, _Tone_Setting_Right);
		Delay(10000);
	}
	
	_debug_msg("Read back left tone data  - \n");
	for (loop = 0; loop < 13; loop++)
	{
		byte = soft_read_data_from_RAM(TONE_DATA_ADDR_LEFT+loop);
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
			byte = soft_read_data_from_RAM(TONE_DATA_ADDR_RIGHT+loop);
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
	_tMA3.fnPlayCallBack((UINT8 *)_uAuPlayBuffAddr, 768);	
	soft_write_data_to_RAM(TONE_DATA_ADDR_LEFT, 768, (UINT8 *)_uAuPlayBuffAddr);
	soft_write_data_to_RAM(TONE_DATA_ADDR_RIGHT, 768, (UINT8 *)_uAuPlayBuffAddr+768);

	_uSoftwarePG = 0;
		
	_debug_msg("Start playing PCM data...\n");

	/* WT voice 7 setting */
	soft_write_control_reg_word(138, (TONE_DATA_ADDR_LEFT>>8)&0x7f, (TONE_DATA_ADDR_LEFT>>1)&0x7f);
	soft_write_control_reg(140, 0x7c);
	soft_write_control_reg_word(141, 0x05, 0x0);
	soft_write_control_reg(143, 0x40);	/* KeyOn, channel 0 */

	/* WT voice 6 setting */
	if (!_bSingleChannel)
	{
		soft_write_control_reg_word(132, (TONE_DATA_ADDR_RIGHT>>8)&0x7f, (TONE_DATA_ADDR_RIGHT>>1)&0x7f);
		soft_write_control_reg(134, 0x7c);
		soft_write_control_reg_word(135, 0x05, 0x0);
		soft_write_control_reg(137, 0x41);	/* KeyOn, channel 1 */
	}
	
	soft_write_intermediate_reg(IMD_REG_INT_FLAG, 0x30);	/* clear interrupt */
	soft_write_control_reg(341, 0x01);	/* Start timer 0 */
	soft_write_intermediate_reg(0x80 | IMD_REG_INT_FLAG, 0);	/* enable interrupt */
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
	sysDisableInterrupt(GPIO_INT_NUM);

 	/* software reset, wait at least 300ns */
	soft_write_intermediate_reg(IMD_REG_BASIC_SETTING, BS_RST);
	Delay(3000);
	soft_write_intermediate_reg(IMD_REG_BASIC_SETTING, 0);
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
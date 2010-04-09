/**************************************************************************************************
 *                                                                          
 * Copyright (c) 2004 - 2007 Winbond Electronics Corp. All rights reserved.      
 *                                                                         
 * FILENAME
 *     IIS.h
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     This file contains the register map of IIS audio interface
 *
 * HISTORY
 *     02/09/2004		 Ver 1.0 Created by PC31 SJLu
 *
 * REMARK
 *     None
 *     
 *************************************************************************************************/
#ifndef _W5691_H_
#define _W5691_H_

enum E_CommandType
{
	// Simple command
	eCOMMAND_POWER_UP,
	eCOMMAND_POWER_UP_ANALOG,
	eCOMMAND_POWER_DOWN,
	eCOMMAND_CLEAR_MELODY_BUFFER,
	eCOMMAND_CLEAR_SPEECH_BUFFER,
	eCOMMAND_SETUP_MELODY_INTERRUPT,
	eCOMMAND_SETUP_SPEECH_INTERRUPT,
	eCOMMAND_ENABLE_MELODY_INTERRUPT,
	eCOMMAND_ENABLE_SPEECH_INTERRUPT,
	eCOMMAND_DISABLE_MELODY_INTERRUPT,
	eCOMMAND_DISABLE_SPEECH_INTERRUPT,
	eCOMMAND_START_MELODY_SYNTHESIZER,
	eCOMMAND_STOP_MELODY_SYNTHESIZER,
	eCOMMAND_PAUSE_MELODY_SYNTHESIZER,
	eCOMMAND_RESUME_MELODY_SYNTHESIZER,
	eCOMMAND_SPEECH_END_OF_SEQUENCE,
	eCOMMAND_STOP_SPEECH_SYNTHESIZER,
	eCOMMAND_PAUSE_SPEECH_SYNTHESIZER,
	eCOMMAND_RESUME_SPEECH_SYNTHESIZER,

	// command with data
	eCOMMAND_SEND_COMMAND_DATA		= 0x0100,
	eCOMMAND_SEND_MELODY_DATA,
	eCOMMAND_SEND_SPEECH_DATA,
	eCOMMAND_SET_EQ_VOLUME,
	eCOMMAND_SET_LED_ON_OFF_IN_POWER_DOWN_MODE,
	eCOMMAND_SET_MOTOR_ON_OFF_IN_POWER_DOWN_MODE,

	// command for require data
	eCOMMAND_GET_INTERRUPT_STATUS	= 0x1000,
	eCOMMAND_GET_INTERRUPT_ENABLE,
	eCOMMAND_GET_FIFO_STATUS,
	eCOMMAND_GET_X_BUFFER_DATA,
	eCOMMAND_GET_Y_BUFFER_DATA,
	eCOMMAND_GET_X_BUFFER_DATA_NO_WAIT,
	eCOMMAND_GET_Y_BUFFER_DATA_NO_WAIT,
	eCOMMAND_GET_MELODY_CLOCK_COUNT
};



#define MW_NO_ERROR					(0)		/* no error */
#define MW_ERROR_ARGUMENT			(-2)	/* error of arguments */

/* PLL output clock = CLKI / PLL_ADJUST_N * (PLL_ADJUST_M + 4) */
#define	W5691_CLKI			15000000
#define	W5691_PLL_ADJUST_N	14
#define	W5691_PLL_ADJUST_M	43

//#define	W5691_PLL_ADJUST_N	4
//#define	W5691_PLL_ADJUST_M	46

//#define	W5691_PLL_ADJUST_N	4
//#define	W5691_PLL_ADJUST_M	60

#define	W5691_SYSTEM_CLOCK	6310100	/* 6.3101MHz */
//#define	W569_SYSTEM_CLOCK	8000000	/* 8MHz */

#define	SPK_VOL_REG_VDS			0


#define	MELODY_FIFO_SIZE		256
#define	MELODY_FIFO_FILL_SIZE	128
#define	SPEECH_FIFO_SIZE		256
#define	SPEECH_FIFO_FILL_SIZE	192

#define 	eSPEECH_FORMAT_SIGN_8BIT_PCM	0x00
#define 	eSPEECH_FORMAT_SIGN_16BIT_PCM	0x01
#define 	eSPEECH_FORMAT_YAMAHA_ADPCM		0x02
#define 	eSPEECH_FORMAT_APM				0x03
#define 	eSPEECH_FORMAT_LP8				0x04
#define 	eSPEECH_FORMAT_AD4				0x05
#define 	eSPEECH_FORMAT_MD5				0x06
#define 	eSPEECH_FORMAT_MD6				0x07

#define 	eSPEECH_MS_STEREO				0x20
#define 	eSPEECH_FCH_PFIFO				0x00

#define 	eSPEECH_FORMAT_UNSIGN_8BIT_PCM	0x80
#define 	eSPEECH_FORMAT_UNSIGN_16BIT_PCM	0x81

#define 	eSPEECH_FORMAT_UNKNOWN			-1







#define WTB_ROM_START_ADDR		0x800000

#define CMD_COM_IRQE			0x80



#define CMD_STA_BUSY			0x01	
#define CMD_STA_S_EMP			0x02
#define CMD_STA_X_RDY			0x04
#define CMD_STA_S_FUL			0x08
#define CMD_STA_P_EMP			0x10
#define CMD_STA_P_FUL			0x20
#define CMD_STA_G_EMP			0x40
#define CMD_STA_Y_RDY			0x80

#define	CMD_COM_ID_SEQ_FIFO			0x00	// Write
#define	CMD_COM_ID_X_BUF			0x00	// Read
#define	CMD_COM_ID_WAKE_UP			0x01	// R/W
#define	CMD_COM_ID_CLK_CTRL			0x02	// R/W
#define	CMD_COM_ID_FIFO_RT_REG		0x03	// Write
#define	CMD_COM_ID_Y_BUF			0x03	// Read
#define	CMD_COM_ID_INT_ST_REG		0x04	// R/W
#define	CMD_COM_ID_SPE_FIFO			0x05	// Write
#define CMD_COM_ID_FIFO_STAT_REG	0x05	// Read
#define CMD_COM_ID_FIFO_ST_REG		0x06	// R/W
#define	CMD_COM_ID_OP_CTRL			0x07	// R/W
#define	CMD_COM_ID_GEN_FIFO			0x08	// Write
#define	CMD_COM_ID_SPK_VOL_REG		0x09	// R/W
#define	CMD_COM_ID_PLL_ST_REG_I		0x0a	// R/W
#define	CMD_COM_ID_PLL_ST_REG_II	0x0b	// R/W
#define	CMD_COM_ID_ANALOG_CTRL		0x0c	// R/W
#define	CMD_COM_ID_INT_EN_REG		0x0d	// R/W
#define CMD_COM_ID_EQ_VOL_REG		0x0e	// R/W
#define CMD_COM_ID_HEADPHONE_VOL_L_REG		0x0f	// R/W
#define CMD_COM_ID_HEADPHONE_VOL_R_REG		0x10	// R/W
#define COD_COM_ID_PLL_CHARGE_PUMP_REG		0X11	// R/W

#define IRQ_STA_REG_FIS0		0x01
#define IRQ_STA_REG_SFIS		0x02
#define IRQ_STA_REG_FIS2		0x04
#define IRQ_STA_REG_FIS3		0x08
#define IRQ_STA_REG_TBIS		0x10
#define IRQ_STA_REG_TAIS		0x20
#define IRQ_STA_REG_PFIS		0x40
#define IRQ_STA_REG_FIS1		0x80

#define IRQ_EN_REG_FIE0			0x01
#define IRQ_EN_REG_SFIE			0x02
#define IRQ_EN_REG_FIE2			0x04
#define IRQ_EN_REG_FIE3			0x08
#define IRQ_EN_REG_TBIE			0x10
#define IRQ_EN_REG_TAIE			0x20
#define IRQ_EN_REG_PFIE			0x40
#define IRQ_EN_REG_FIE1			0x80

#define	WAKE_UP_REG_INT			0x01
#define	WAKE_UP_REG_DP0			0x02

#define FIFO_RT_REG_RST			0x01
#define FIFO_RT_REG_SFRT		0x02
#define FIFO_RT_REG_GFRT		0x04
#define FIFO_RT_REG_PFRT		0x08

#define	CLK_CTRL_REG_DP1		0x01
#define CLK_CTRL_REG_AP2		0x02

#define	ANALOG_CTRL_REG_AP7		0x01	// W56940 & W56964
#define ANALOG_CTRL_REG_AP1		0x04
#define	ANALOG_CTRL_REG_AP0		0x08
#define	ANALOG_CTRL_REG_AP3		0x10	// W56940 & W56964
#define	ANALOG_CTRL_REG_AP4		0x20	// W56940 & W56964
#define	ANALOG_CTRL_REG_AP5		0x40	// W56940 & W56964
#define	ANALOG_CTRL_REG_AP6		0x80	// W56940 & W56964

#define	FIFO_STAT_REG_SLET		0x01
#define	FIFO_STAT_REG_PLET		0x02



#define FIFO_ST_REG_SDIRQ_0		0x00
#define FIFO_ST_REG_SDIRQ_32	0x01
#define FIFO_ST_REG_SDIRQ_64	0x02
#define FIFO_ST_REG_SDIRQ_96	0x03
#define FIFO_ST_REG_SDIRQ_128	0x04
#define FIFO_ST_REG_SDIRQ_160	0x05
#define FIFO_ST_REG_SDIRQ_192	0x06
#define FIFO_ST_REG_SDIRQ_224	0x07

#define FIFO_ST_REG_SDIRQ_2_0	0x00
#define FIFO_ST_REG_SDIRQ_2_64	0x01
#define FIFO_ST_REG_SDIRQ_2_128	0x02
#define FIFO_ST_REG_SDIRQ_2_192	0x03
#define FIFO_ST_REG_SDIRQ_2_256	0x04
#define FIFO_ST_REG_SDIRQ_2_320	0x05
#define FIFO_ST_REG_SDIRQ_2_384	0x06
#define FIFO_ST_REG_SDIRQ_2_448	0x07

#define FIFO_ST_REG_CASD		0x08

#define FIFO_ST_REG_PDIRQ_0		0x00
#define FIFO_ST_REG_PDIRQ_32	0x10
#define FIFO_ST_REG_PDIRQ_64	0x20
#define FIFO_ST_REG_PDIRQ_96	0x30
#define FIFO_ST_REG_PDIRQ_128	0x40
#define FIFO_ST_REG_PDIRQ_160	0x50
#define FIFO_ST_REG_PDIRQ_192	0x60
#define FIFO_ST_REG_PDIRQ_224	0x70

#define OPC_REG_OPT0		0x01
#define OPC_REG_OPT1		0x02
#define OPC_REG_OPT2		0x04
#define OPC_REG_OPT3		0x08

#endif	/* _W5691_H_ */



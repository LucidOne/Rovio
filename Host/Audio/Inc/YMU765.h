#ifndef _YMU765_H_
#define _YMU765_H_

#define IMD_REG_READ_DATA0		1   	/* #1 read data #0 */
#define IMD_REG_READ_DATA1		2   	/* #2 read data #1 */
#define IMD_REG_READ_DATA2		3   	/* #3 read data #2 */

/* Intermediate registers */
#define IMD_INT_FLAG1			0  		/* #0 Interruption flag (1) */
#define IMD_DELAYED_WRITE		1   	/* #1 Delayed write register */
#define IMD_INSTANT_WRITE		2   	/* #2 Instantaneous write setting */
#define IMD_INSTANT_READ		3   	/* #3 Instantaneous read setting */
#define IMD_BASIC_SETTING		4   	/* #4 Basic setting */

/* bank 0 */
#define IMD_PWR_DIGITAL			5   	/* #5 power management setting (digital) */
#define IMD_PWR_ANALOG			6   	/* #6 power management setting (analog) */
#define IMD_EQVAL				7   	/* #7 analog setting (EQVOL) */
#define IMD_HPVOL_L				8   	/* #8 analog setting (HPVOL_L, Mono) */
#define IMD_HPVOL_R				9   	/* #9 analog setting (HPVOL_R, Mono) */
#define IMD_SPVOL				10  	/* #10 analog setting (SPVOL, VSEL) */
#define IMD_INT_FLAG2			16  	/* #16 Interruption flag (2) */
#define IMD_INT_MASK1			17  	/* #17 Interruption enable / disable setting */
#define IMD_INT_MASK2			18  	/* #18 Interruption enable / disable setting */
#define IMD_STREAM_PG0			25  	/* #25 Stream playback potion read-out */
#define IMD_STREAM_PG1			26  	/* #26 Stream playback potion read-out */
#define IMD_TIMER_CNT0			27  	/* #27 Timer0 count read-out */
#define IMD_TIMER_CNT1			28  	/* #28 Timer1 count read-out */
#define IMD_TIMER_CNT2			29  	/* #29 Timer2 count read-out */
#define IMD_PLL_SETTING1		30  	/* #30 PLL setting */
#define IMD_PLL_SETTING2		31  	/* #31 PLL setting */
#define IMD_SEQ_CNT_H			32  	/* #32 Sequencer control */
#define IMD_SEQ_CNT_M			33  	/* #33 Sequencer control */
#define IMD_SEQ_CNT_L			34  	/* #34 Sequencer control */
#define IMD_MASTER_VOL_L		39  	/* #39 Master volume left */
#define IMD_MASTER_VOL_R		40  	/* #40 Master volume right */
#define IMD_FM_VOL				41  	/* #41 FM synthesizer volume */
#define IMD_WT_VOL				42  	/* #42 Wave Table synthesizer volume */
#define IMD_STM_VOL				43  	/* #43 STM synthesizer volume */
#define IMD_SYNTH_MODE			56		/* #56 Synthesizer mode setting */
#define IMD_STREAM_INT			57		/* #57 Stream playback interrupt setting */
#define IMD_STREAM_INT2			58		/* #58 Stream playback interrupt setting 2*/


/* Control registers */

/* Bit definition of state flag register */
#define ST_IRQ					0x80
#define ST_BUSY					0x80
#define ST_VALID_R				0x40
#define ST_FULL_DW				0x08
#define ST_FULL_W				0x04
#define ST_EMP_DW				0x02
#define ST_EMP_W				0x01

/* Bit definition of basic setting register */
#define BS_RST					0x80
#define BS_R_FIFO_1				0x40
#define BS_R_FIFO_0				0x20
#define BS_R_SEQ_1				0x10
#define BS_R_SEQ_0				0x08



#endif	/* _YMU765_H_ */



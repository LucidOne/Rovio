#ifndef _YMU762_H_
#define _YMU762_H_


/* Intermediate registers */
#define IMD_REG_INT_FLAG		0   	/* #0 interrupt flag */
#define IMD_REG_DW_SETTING		1   	/* #1 Delayed write setting */
#define IMD_REG_IW_SETTING		2   	/* #2 Instantaneous write setting */
#define IMD_REG_IR_SETTING		3   	/* #3 Instantaneous read setting */
#define IMD_REG_BASIC_SETTING	4   	/* #4 Basic setting */

#define IMD_REG_READ_DATA0		1   	/* #1 read data #0 */
#define IMD_REG_READ_DATA1		2   	/* #2 read data #1 */
#define IMD_REG_READ_DATA2		3   	/* #3 read data #2 */
/* bank 0 */
#define IMD_REG_PWR_DIGITAL		5   	/* #5 power management setting (digital) */
#define IMD_REG_PWR_ANALOG		6   	/* #6 power management setting (analog) */
#define IMD_REG_ANALOG_SETTING1	7   	/* #7 analog setting (EQVOL) */
#define IMD_REG_ANALOG_SETTING2	8   	/* #8 analog setting (HPVOL_L, Mono) */
#define IMD_REG_ANALOG_SETTING3	9   	/* #9 analog setting (HPVOL_R) */
#define IMD_REG_ANALOG_SETTING4	10  	/* #10 analog setting (SPVOL, VSEL) */
#define IMD_REG_LED_SETTING1	11  	/* #11 LED setting(1) */
#define IMD_REG_LED_SETTING2	12  	/* #12 LED setting(2) */
#define IMD_REG_MOTOR_SETTING1	13  	/* #13 MOTOR setting(1) */
#define IMD_REG_MOTOR_SETTING2	14  	/* #14 MOTOR setting(2) */
/* bank 1 */
#define IMD_REG_PLL_SETTING1	5		/* #5 PLL setting register(1) */
#define IMD_REG_PLL_SETTING2	6		/* #6 PLL setting register(2) */

/* Control registers */

/* Bit definition of state flag register */
#define ST_IRQ					0x80
#define ST_BUSY					0x80
#define ST_VALID_R2				0x40
#define ST_VALID_R1				0x20
#define ST_VALID_R0				0x10
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



#endif	/* _YMU762_H_ */



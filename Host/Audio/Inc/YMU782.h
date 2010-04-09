#ifndef _YMU782_H_
#define _YMU782_H_


/*******************************************************************

	MA-5 Register Definition

	[Naming Rules]

	  MAI_xxx : Intermediate Registers Bank#0
	  MAI3_xx : Intermediate Registers Bank#3
	  MAC_xxx : Control Registers
	  MAB_xxx : Miscelleneous bit definition

********************************************************************/


#define MAI_BASIC_SETTING				(4)
/*
        7       6       5       4       3       2       1       0
	+-------+-------+-------+-------+-------+-------+-------+-------+
	|  RST  |R_FIFO1|R_FIFO0|R_SEQ#1|R_SEQ#0|  "0"  |    B A N K    |
	+-------+-------+-------+-------+-------+-------+-------+-------+
*/
#define	MAB_RST							(0x80)
#define	MAB_RFIFO1						(0x40)
#define	MAB_RFIFO0						(0x20)
#define	MAB_RSEQ1						(0x10)
#define	MAB_RSEQ0						(0x08)


#define MAI_POWER_MANAGEMENT_DIGITAL	(5)
/*
        7       6       5       4       3       2       1       0
	+-------+-------+-------+-------+-------+-------+-------+-------+
	|  "0"  |  "0"  |  "0"  |  "0"  |   D3  |   D2  |   D1  |   D0  |
	+-------+-------+-------+-------+-------+-------+-------+-------+
*/
#define MAB_PWM_DP3						(0x08)
#define MAB_PWM_DP2						(0x04)
#define MAB_PWM_DP1						(0x02)
#define MAB_PWM_DP0						(0x01)

#define MAI_POWER_MANAGEMENT_ANALOG		(6)
/*
        7       6       5       4       3       2       1       0
	+-------+-------+-------+-------+-------+-------+-------+-------+
	| PLLPD |  "0"  |  AP4R |  AP4L |  AP3  |  AP2  |  AP1  |  AP0  |
	+-------+-------+-------+-------+-------+-------+-------+-------+
*/
#define MAB_PWM_PLLPD					(0x80)
#define MAB_PWM_AP4R					(0x20)
#define MAB_PWM_AP4L					(0x10)
#define MAB_PWM_AP3						(0x08)
#define MAB_PWM_AP2						(0x04)
#define MAB_PWM_AP1						(0x02)
#define MAB_PWM_AP0						(0x01)



#define MAI_ANALOG_EQVOL				(7)
/*
        7       6       5       4       3       2       1       0
	+-------+-------+-------+-------+-------+-------+-------+-------+
	|  "0"  |  "0"  |  "0"  |                 EQVOL                 |
	+-------+-------+-------+-------+-------+-------+-------+-------+
*/

#define MAI_ANALOG_HPVOL_L				(8)
/*
        7       6       5       4       3       2       1       0
	+-------+-------+-------+-------+-------+-------+-------+-------+
	| MONO  |  "0"  |  "0"  |                HPVOLL                 |
	+-------+-------+-------+-------+-------+-------+-------+-------+
*/
#define MAI_ANALOG_HPVOL_R				(9)
/*
        7       6       5       4       3       2       1       0
	+-------+-------+-------+-------+-------+-------+-------+-------+
	|  "0"  |  "0"  |  "0"  |                HPVOLR                 |
	+-------+-------+-------+-------+-------+-------+-------+-------+
*/

#define MAI_ANALOG_SPVOL				(10)
/*
        7       6       5       4       3       2       1       0
	+-------+-------+-------+-------+-------+-------+-------+-------+
	| VSEL2 | VSEL1 |  "0"  |                SPVOL                  |
	+-------+-------+-------+-------+-------+-------+-------+-------+
*/


#define MAI_IRQ_CONTROL_1				(17)
/*
        7       6       5       4       3       2       1       0
	+-------+-------+-------+-------+-------+-------+-------+-------+
	| EFIFO | ETM#2 | ETM#1 | ETM#0 |ESIRQ#1|ESIRQ#0|ESTM#1 |ESTM#0 |
	+-------+-------+-------+-------+-------+-------+-------+-------+
*/
#define	MAB_IRQCTRL_EFIFO				(0x80)
#define	MAB_IRQCTRL_ETM2				(0x40)
#define	MAB_IRQCTRL_ETM1				(0x20)
#define	MAB_IRQCTRL_ETM0				(0x10)
#define	MAB_IRQCTRL_ESIRQ1				(0x08)
#define	MAB_IRQCTRL_ESIRQ0				(0x04)
#define	MAB_IRQCTRL_ESTM1				(0x02)
#define	MAB_IRQCTRL_ESTM0				(0x01)

#define MAI_IRQ_CONTROL_2				(18)
/*
        7       6       5       4       3       2       1       0
	+-------+-------+-------+-------+-------+-------+-------+-------+
	|  "0"  |  "0"  |ESIRQ#5|ESIRQ#4|ESIRQ#3|ESIRQ#2|  "0"  |  "0"  |
	+-------+-------+-------+-------+-------+-------+-------+-------+
*/
#define	MAB_IRQCTRL_ESIRQ5				(0x20)
#define	MAB_IRQCTRL_ESIRQ4				(0x10)
#define	MAB_IRQCTRL_ESIRQ3				(0x08)
#define	MAB_IRQCTRL_ESIRQ2				(0x04)



#define MAI_PLL_SETTING_1				(30)
/*
        7       6       5       4       3       2       1       0
	+-------+-------+-------+-------+-------+-------+-------+-------+
	| CKSEL |  "0"  |  "0"  |                ADJUST1                |
	+-------+-------+-------+-------+-------+-------+-------+-------+
*/
#define MAI_PLL_SETTING_2				(31)
/*
        7       6       5       4       3       2       1       0
	+-------+-------+-------+-------+-------+-------+-------+-------+
	|  "0"  |                        ADJUST2                        |
	+-------+-------+-------+-------+-------+-------+-------+-------+
*/
#define MAI_MASTER_VOLL					(39)
#define MAI_MASTER_VOLR					(40)

#define	MAI_EXT_CTRL					(55)
/*
        7       6       5       4       3       2       1       0
	+-------+-------+-------+-------+-------+-------+-------+-------+
	|     SV_MUTE   |SV_CHV |SV_PAN | MVSEL | NOP2E | FINT  | DADJT |
	+-------+-------+-------+-------+-------+-------+-------+-------+
*/
#define	MAB_EXTCTRL_SVMUTE				(0xC0)
#define	MAB_EXTCTRL_SVCHV				(0x20)
#define	MAB_EXTCTRL_SVPAN				(0x10)
#define	MAB_EXTCTRL_MVSEL				(0x08)
#define	MAB_EXTCTRL_NOP2E				(0x04)
#define	MAB_EXTCTRL_FINT				(0x02)
#define	MAB_EXTCTRL_DADJT				(0x01)

#define	MAI_STM_IRQ_CTRL				(57)
/*
        7       6       5       4       3       2       1       0
	+-------+-------+-------+-------+-------+-------+-------+-------+
	|  "0"  |  "0"  |      SIZE     |  "0"  |  "0"  |  IRQ POINT    |
	+-------+-------+-------+-------+-------+-------+-------+-------+
*/
/*=====  Bank #3  =====*/


#define	MAI3_POWER_MANAGEMENT_DIGITAL_AUDIO	(16)
/*
        7       6       5       4       3       2       1       0
	+--------+-------+-------+-------+-------+-------+-------+-------+
	|PLLDAPD |  "0"  |  "0"  |  "0"  |  "0"  |  DPA2 | DPA1  | DPA0  |
	+--------+-------+-------+-------+-------+-------+-------+-------+
*/
#define	MAB_PWM_PLLDAPD					(1<<7)
#define	MAB_PWM_DPA2					(1<<2)
#define	MAB_PWM_DPA1					(1<<1)
#define	MAB_PWM_DPA0					(1)

#define	MAI3_POWER_MANAGEMENT_ANALOG	(17)
/*
        7       6       5       4       3       2       1       0
	+-------+-------+-------+-------+-------+-------+-------+-------+
	|  "0"  |  "0"  |  "0"  |  AP9  |  AP8  |  AP7  |  AP6  |  AP5  |
	+-------+-------+-------+-------+-------+-------+-------+-------+
*/
#define	MAB_PWM_AP9						(0x10)
#define	MAB_PWM_AP8						(0x08)
#define	MAB_PWM_AP7						(0x04)
#define	MAB_PWM_AP6						(0x02)
#define	MAB_PWM_AP5						(0x01)





#define	MAI3_DAC_ADJUST3	(18)
#define	MAI3_DAC_ADJUST4	(19)
#define	MAI3_DAC_ADJUST5	(20)



#define	MAI3_DAC_CTRL1	(21)
/*
        7        6       5       4       3       2       1       0
	+--------+-------+-------+-------+-------+-------+-------+-------+
	|DAMEMCLR|   MS  |     BCKFS     |               FS              |
	+--------+-------+-------+-------+-------+-------+-------+-------+
*/
#define	MAB_DACCTRL_DAMEMCLR					(1<<7)
#define MAB_DACCTRL_BCLK32						(2<<4)
#define MAB_DACCTRL_BCLK48						(1<<4)
#define	MAB_DACCTRL_48000						(0)
#define	MAB_DACCTRL_44100						(1)
#define	MAB_DACCTRL_32000						(2)
#define	MAB_DACCTRL_24000						(4)
#define	MAB_DACCTRL_22050						(5)
#define	MAB_DACCTRL_16000						(6)
#define	MAB_DACCTRL_12000						(8)
#define	MAB_DACCTRL_11025						(9)
#define	MAB_DACCTRL_8000						(10)

#define	MAI3_DAC_CTRL2	(22)
/*
        7          6       5       4       3       2       1       0
	+----------+-------+-------+-------+-------+-------+-------+-------+
	|DADSPSTART|  "0"  |  "0"  |  "0"  |  "0"  |  "0"  |      MODE	   |
	+----------+-------+-------+-------+-------+-------+-------+-------+
*/
#define	MAB_DACCTRL_MSB							(0)
#define	MAB_DACCTRL_IIS							(1)
#define	MAB_DACCTRL_MODE2						(2)
#define MAB_DACCTRL_DADSPSTART					(1<<7)

#define	MAI3_TXOUT_MIXER	(23)
/*
        7          6       5       4       3         2        1       0
	+----------+-------+-------+-------+---------+---------+--------+--------+
	|  "0"     |  "0"  |  "0"  |  "0"  |TX_EXMIX |TX_RXMIX|TX_DAMIX|TX_FMMIX|
	+----------+-------+-------+-------+---------+---------+--------+--------+
*/
#define	MAB_MIXER_TX_EXMIX							(1<<3)
#define	MAB_MIXER_TX_RXMIX							(1<<2)
#define	MAB_MIXER_TX_DAMIX							(1<<1)
#define	MAB_MIXER_TX_FMMIX							(1)

#define	MAI3_HPOUT_MIXER	(25)
/*
        7         6         5       4         3       2        1        0
	+--------+---------+--------+--------+--------+--------+--------+--------+
	|HR_EXMIX|HR_RXMIX |HR_DAMIX|HR_FMMIX|HL_EXMIX|HL_RXMIX|HL_DAMIX|HL_FMMIX|
	+--------+---------+--------+--------+--------+--------+----*---+--------+
*/
#define	MAB_MIXER_HR_DAMIX							(1<<5)
#define	MAB_MIXER_HL_DAMIX							(1<<1)

#define	MAI3_SPOUT_MIXER	(26)
/*
        7         6         5       4         3       2        1        0
	+--------+---------+--------+--------+--------+--------+--------+--------+
	|  "0"   |    "0"  |   "0"  |   "0"  |SP_EXMIX|SP_RXMIX|SP_DAMIX|SP_FMMIX|
	+--------+---------+--------+--------+--------+--------+----*---+--------+
*/
#define	MAB_MIXER_SP_DAMIX							(1<<1)

#define	MAI3_DA_LVOL	(27)
#define	MAI3_DA_RVOL	(28)

#define	MAI3_ANALOG_STATUS				(37)
/*
        7       6       5       4       3       2       1       0
	+-------+-------+-------+-------+-------+-------+-------+-------+
	|  "0"  |  "0"  |  "0"  |EX_STBY| EX_RDY|HP_STBY| HP_RDY|VREF_RDY|
	+-------+-------+-------+-------+-------+-------+-------+-------+
*/
#define	MAB_EX_STBY						(0x10)
#define	MAB_EX_RDY						(0x08)
#define	MAB_HP_STBY						(0x04)
#define	MAB_HP_RDY						(0x02)
#define	MAB_VREF_RDY					(0x01)

#define	MA_ADJUST1_VALUE	(10)	/* register bank 0, B_Address #30 CKSEL & ADJUST1 */
#define	MA_ADJUST2_VALUE	(60)	/* register bank 0, B_Address #31 ADJUST2 */
								/* Y=(CLKI/ADJUST1), 750KHZ< Y <1.25MHZ, Y*ADJUST2 =: 73.728MHZ */
/*******/
			
#define MA_RESET_RETRYCOUNT				(1000)




#define HW_INIT 0
#define PW_DOWN 1

#endif	/* _YMU782_H_ */



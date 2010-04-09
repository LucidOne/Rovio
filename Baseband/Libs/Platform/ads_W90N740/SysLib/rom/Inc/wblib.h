/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2002 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *     WBLIB.h
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     This file contains W90N740 low level library APIs.
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *     None
 *
 * HISTORY
 *     06/06/02		 Ver 1.0 Created by PC30 HPChen
 *
 * REMARK
 *     None
 **************************************************************************/
#ifndef _WBLIB_H
#define _WBLIB_H

#include "wbio.h"

//-- function return value
#define	   Successful  0
#define	   Fail        1

#if W90N740_INTEGRATOR_BOARD

#define LM_BASE 		0xC0000000
#define AHB_IO_BASE		0x7FF00000
#define APB_IO_BASE		0xCFF80000

/* ----- Define	the LM registers ----- */
#define	LM_OSC1		(LM_BASE+0x0000000)		/* Oscillator divisor register 1 */
#define	LM_OSC2		(LM_BASE+0x0000004)		/* Oscillator divisor register 2 */
#define	LM_LOCK		(LM_BASE+0x0000008)		/* Oscillator lock register */

#define SYSTEM_CLOCK  10000000  /* 10MHz */

#else

#define AHB_IO_BASE		0xFFF00000
#define APB_IO_BASE		0xFFF80000

#define SYSTEM_CLOCK  15000000  /* 15MHz */

#endif

#define AIC_IO_OFFSET		0x82000
#define TIMER_IO_OFFSET		0x81000
#define UART_IO_OFFSET		0x80000


/* ----- Define	the external I/O controller registers ----- */
#define EXT0CON		0x1018
#define EXT1CON		0x101C
#define EXT2CON		0x1020
#define EXT3CON		0x1024


/* ----- Define	the interrupt controller registers ----- */
#define AIC_SCR1	(AIC_IO_OFFSET+0x04)		/* Source control register 1 */
#define AIC_SCR2	(AIC_IO_OFFSET+0x08)		/* Source control register 2 */
#define AIC_SCR3	(AIC_IO_OFFSET+0x0C)		/* Source control register 3 */
#define AIC_SCR4	(AIC_IO_OFFSET+0x10)		/* Source control register 4 */
#define AIC_SCR5	(AIC_IO_OFFSET+0x14)		/* Source control register 5 */
#define AIC_SCR6	(AIC_IO_OFFSET+0x18)		/* Source control register 6 */
#define AIC_SCR7	(AIC_IO_OFFSET+0x1C)		/* Source control register 7 (timer0) */
#define AIC_SCR8	(AIC_IO_OFFSET+0x20)		/* Source control register 8 (timer1) */
#define AIC_SCR9	(AIC_IO_OFFSET+0x24)		/* Source control register 9 */
#define AIC_SCR10	(AIC_IO_OFFSET+0x28)		/* Source control register 10 */
#define AIC_SCR11	(AIC_IO_OFFSET+0x2C)		/* Source control register 11 */
#define AIC_SCR12	(AIC_IO_OFFSET+0x30)		/* Source control register 12 */
#define AIC_SCR13	(AIC_IO_OFFSET+0x34)		/* Source control register 13 */
#define AIC_SCR14	(AIC_IO_OFFSET+0x38)		/* Source control register 14 */
#define AIC_SCR15	(AIC_IO_OFFSET+0x3C)		/* Source control register 15 */
#define AIC_SCR16	(AIC_IO_OFFSET+0x40)		/* Source control register 16 */
#define AIC_SCR17	(AIC_IO_OFFSET+0x44)		/* Source control register 17 */
#define AIC_SCR18	(AIC_IO_OFFSET+0x48)		/* Source control register 18 */

#define AIC_IRSR	(AIC_IO_OFFSET+0x100)		/* Interrupt raw status register */
#define AIC_IASR	(AIC_IO_OFFSET+0x104)		/* Interrupt active status register */
#define AIC_ISR		(AIC_IO_OFFSET+0x108)		/* Interrupt status register */
#define AIC_IPER	(AIC_IO_OFFSET+0x10C)		/* Interrupt priority encoding register */
#define AIC_ISNR	(AIC_IO_OFFSET+0x110)		/* Interrupt source number register */
#define AIC_IMR		(AIC_IO_OFFSET+0x114)		/* Interrupt mask register */
#define AIC_OISR	(AIC_IO_OFFSET+0x118)		/* Output interrupt status register */

#define AIC_MECR	(AIC_IO_OFFSET+0x120)		/* Mask enable command register */
#define AIC_MDCR	(AIC_IO_OFFSET+0x124)		/* Mask disable command register */

#define AIC_SSCR	(AIC_IO_OFFSET+0x128)		/* Source set command register */
#define AIC_SCCR	(AIC_IO_OFFSET+0x12C)		/* Source clear command register */
#define AIC_EOSCR	(AIC_IO_OFFSET+0x130)		/* End of service command register */

#define AIC_TEST    (AIC_IO_OFFSET+0x200)     /* ICE/Debug mode register */


/*----- Define the Timer registers -----*/
#define	TIMER_TCR0		(TIMER_IO_OFFSET+0x0)
#define	TIMER_TCR1		(TIMER_IO_OFFSET+0x04)
#define	TIMER_TICR0		(TIMER_IO_OFFSET+0x08)
#define	TIMER_TICR1		(TIMER_IO_OFFSET+0x0C)
#define	TIMER_TDR0		(TIMER_IO_OFFSET+0x10)
#define	TIMER_TDR1		(TIMER_IO_OFFSET+0x14)
#define	TIMER_TISR		(TIMER_IO_OFFSET+0x18)
#define TIMER_WTCR		(TIMER_IO_OFFSET+0x1C)


/*----- Define the UART registers -----*/
#define UART_TX		(UART_IO_OFFSET+0x0)		/* (W) TX buffer */
#define UART_RX		(UART_IO_OFFSET+0x0)		/* (R) RX buffer */
#define UART_LSB	(UART_IO_OFFSET+0x0)		/* Divisor latch LSB */
#define UART_MSB	(UART_IO_OFFSET+0x04)		/* Divisor latch MSB */
#define UART_IER	(UART_IO_OFFSET+0x04)		/* Interrupt enable register */
#define UART_IIR	(UART_IO_OFFSET+0x08)		/* (R) Interrupt ident. register */
#define UART_FCR	(UART_IO_OFFSET+0x08)		/* (W) FIFO control register */
#define UART_LCR	(UART_IO_OFFSET+0x0C)		/* Line control register */
#define UART_MCR	(UART_IO_OFFSET+0x10)		/* Modem control register */
#define	UART_LSR	(UART_IO_OFFSET+0x14)		/* (R) Line status register */
#define UART_MSR	(UART_IO_OFFSET+0x18)		/* (R) Modem status register */
#define	UART_TOR	(UART_IO_OFFSET+0x1C)		/* (R) Time out register */


/* Define constants for use external IO in service parameters.  */
#define EXT0			0
#define EXT1			1
#define EXT2			2
#define EXT3			3

#define SIZE_256K		4
#define SIZE_512K		5
#define SIZE_1M			6
#define SIZE_2M			7
#define SIZE_4M			8
#define SIZE_8M			9
#define SIZE_16M		10
#define SIZE_32M		11

#define BUS_DISABLE		12
#define BUS_BIT_8		13
#define BUS_BIT_16		14
#define BUS_BIT_32		15

/* Define constants for use timer in service parameters.  */
#define TIMER0            0
#define TIMER1            1

#define ONE_SHOT_MODE     0
#define PERIODIC_MODE     1
#define TOGGLE_MODE       2

#define HALF_MINUTES      0
#define ONE_MINUTES       1
#define TWO_MINUTES       2
#define FOUR_MINUTES      3

/* Define constants for use UART in service parameters.  */
#define WB_DATA_BITS_5    0x00
#define WB_DATA_BITS_6    0x01
#define WB_DATA_BITS_7    0x02
#define WB_DATA_BITS_8    0x03

#define WB_STOP_BITS_1    0x00
#define WB_STOP_BITS_2    0x04

#define WB_PARITY_NONE    0x00
#define WB_PARITY_ODD     0x00
#define WB_PARITY_EVEN    0x10

//#define WB_DTR_Low        0x01
//#define WB_RTS_Low        0x02
//#define WB_MODEM_En       0x08

#define LEVEL_1_BYTE      0x00
#define LEVEL_4_BYTES     0x40
#define LEVEL_8_BYTES     0x80
#define LEVEL_14_BYTES    0xC0

/* Define constants for use AIC in service parameters.  */
#define WB_SWI                     0
#define WB_D_ABORT                 1
#define WB_I_ABORT                 2
#define WB_UNDEFINE                3

#define FIQ_LEVEL_0                0
#define IRQ_LEVEL_1                1
#define IRQ_LEVEL_2                2
#define IRQ_LEVEL_3                3
#define IRQ_LEVEL_4                4
#define IRQ_LEVEL_5                5
#define IRQ_LEVEL_6                6
#define IRQ_LEVEL_7                7

#define ENABLE_ALL_INTERRUPTS      0
#define DISABLE_ALL_INTERRUPTS     1

#define LOW_LEVEL_SENSITIVE        0x00
#define HIGH_LEVEL_SENSITIVE       0x40
#define NEGATIVE_EDGE_TRIGGER      0x80
#define POSITIVE_EDGE_TRIGGER      0xC0

#define ENABLE_IRQ                 0x7F
#define ENABLE_FIQ                 0xBF
#define ENABLE_FIQ_IRQ             0x3F
#define DISABLE_IRQ                0x80
#define DISABLE_FIQ                0x40
#define DISABLE_FIQ_IRQ            0xC0


/* Define UART initialization data structure */
typedef struct UART_INIT_STRUCT
{
    UINT32       freq;
    UINT32       baud_rate;
    UINT8        data_bits;
    UINT8        stop_bits;
    UINT8        parity;
    UINT8        rx_trigger_level;
} WB_UART;


/* UART return value */
#define WB_INVALID_PARITY       -1
#define WB_INVALID_DATA_BITS    -2
#define WB_INVALID_STOP_BITS    -3
#define WB_INVALID_BAUD         -4

/* Define system library Timer functions */
UINT32 WB_GetTicks (INT32 timeNo);
INT32 WB_ResetTicks (INT32 timeNo);
INT32 WB_SetTimerReferenceClock (UINT32 timeNo, UINT32 clockRate);
INT32 WB_StartTimer (INT32 timeNo, UINT32 ticksPerSecond, INT32 opMode);
INT32 WB_StopTimer (INT32 timeNo);
VOID WB_ClearWatchDogTimerCount (VOID);
VOID WB_ClearWatchDogTimerInterruptStatus(VOID);
VOID WB_DisableWatchDogTimer (VOID);
VOID WB_DisableWatchDogTimerReset(VOID);
VOID WB_EnableWatchDogTimer (VOID);
VOID WB_EnableWatchDogTimerReset(VOID);
PVOID WB_InstallWatchDogTimerISR (INT32 intTypeLevel, PVOID pNewISR);
INT32 WB_SetWatchDogTimerInterval (INT32 wdtInterval);
INT32 WB_SetTimerEvent(UINT32 timeNo, UINT32 timeTick, PVOID pFun);
VOID WB_ClearTimerEvent(UINT32 timeNo, UINT32 timeEventNo);

/* Define system library UART functions */
INT8 WB_GetChar (VOID);
INT32 WB_InitializeUART (WB_UART *uart);
VOID WB_Printf (PINT8 str,...);
VOID WB_PutChar (UINT8 ch);

/* Define system library AIC functions */
INT32 WB_DisableInterrupt (UINT32 intNo);
INT32 WB_EnableInterrupt (UINT32 intNo);
PVOID WB_InstallExceptionHandler (INT32 exceptType, PVOID pNewHandler);
PVOID WB_InstallFiqHandler (PVOID pNewISR);
PVOID WB_InstallIrqHandler (PVOID pNewISR);
PVOID WB_InstallISR (INT32 intTypeLevel, INT32 intNo, PVOID pNewISR);
INT32 WB_SetGlobalInterrupt (INT32 intState);
INT32 WB_SetInterruptPriorityLevel (UINT32 intNo, UINT32 intLevel);
INT32 WB_SetInterruptType (UINT32 intNo, UINT32 intSourceType);
INT32 WB_SetLocalInterrupt (INT32 intState);
INT32 WB_SetAIC2SWMode(void);

/* Define system library External IO functions */
VOID WB_SetExternalIO(INT extNo, UINT32 extBaseAddr, UINT32 extSize, INT extBusWidth);
VOID WB_SetExternalIOTiming1(INT extNo, INT tACC, INT tACS);
VOID WB_SetExternalIOTiming2(INT extNo, INT tCOH, INT tCOS);

#endif  /* _WBLIB_H */


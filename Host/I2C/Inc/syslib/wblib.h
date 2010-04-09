/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2003 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *     WBLIB.h
 *
 * VERSION
 *     1.1
 *
 * DESCRIPTION
 *     This file contains W99702 low level library APIs.
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *     None
 *
 * HISTORY
 *     10/13/03		 Ver 1.0 Created by PC30 HPChen
 *     08/31/04      Ver 1.1 Modified by PC31 HPChen
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

//#define APB_SYSTEM_CLOCK  25000000  /* 25MHz */
#define APB_SYSTEM_CLOCK  14318000

/* Define the vector numbers associated with each interrupt */
typedef enum int_source_e
{
	IRQ_WDT=1,		// Watch Dog Timer
	IRQ_EXT0,		// External Interrupt 0 (GPIO4)
	IRQ_EXT1,		// External Interrupt 1 (GPIO5)
	IRQ_EXT2,		// External Interrupt 2 (GPIO6)
	IRQ_EXT3,		// External Interrupt 3 (GPIO7)
	IRQ_ADOP,		// Audio Interface record interrupt
	IRQ_ADOR,		// Audio Interface playback interrupt
	IRQ_DSP,		// Sensor DSP
	IRQ_VCE,		// Video Capture Engine
	IRQ_HIC,		// Host Interface
	IRQ_LCM,		// Display Controller
	IRQ_UART1,		// UART1 (High Speed)
	IRQ_UART0,		// UART0
	IRQ_TIMER1,		// Timer1
	IRQ_TIMER0,		// Timer0
	IRQ_JPEG,		// JPEG Video Codec
	IRQ_MPEG,		// MPEG-4 Video Codec
	IRQ_2DGE,		// 2-D Graphics Engine
	IRQ_VPE,		// Video Processing Engine
	IRQ_ME,			// Motion Estimation Engine
	IRQ_FMI,		// Flash Memory Interface
	IRQ_USB,		// USB interrupt
	IRQ_GPIO,		// GPIO interrupt
	IRQ_USI,		// universal serial interface interrupt
	IRQ_FI2C,		// Fast I2C interface interrupt
	IRQ_PWR			// System Wake-up interrupt
} INT_SOURCE_E;


/* Define constants for use timer in service parameters.  */
#define TIMER0            0
#define TIMER1            1

#define ONE_SHOT_MODE     2
#define PERIODIC_MODE     3
#define TOGGLE_MODE       4

#define ONE_HALF_SECS     5
#define FIVE_SECS         6
#define TEN_SECS          7
#define TWENTY_SECS       8

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


/* Define Cache type  */
#define CACHE_WRITE_BACK		0
#define CACHE_WRITE_THROUGH		1

/* Define constants for use Cache in service parameters.  */
#define CACHE_4M		2
#define CACHE_8M		3
#define CACHE_16M		4
#define CACHE_32M		5
#define I_CACHE			6
#define D_CACHE			7
#define I_D_CACHE		8

/* Define UART initialization data structure */
typedef struct UART_INIT_STRUCT
{
    UINT32       uiFreq;
    UINT32       uiBaudrate;
    UINT8        uiDataBits;
    UINT8        uiStopBits;
    UINT8        uiParity;
    UINT8        uiRxTriggerLevel;
} WB_UART_T;


/* UART return value */
#define WB_INVALID_PARITY       -1
#define WB_INVALID_DATA_BITS    -2
#define WB_INVALID_STOP_BITS    -3
#define WB_INVALID_BAUD         -4

/* Define system library Timer functions */
UINT32	sysGetTicks (INT32 nTimeNo);
INT32	sysResetTicks (INT32 nTimeNo);
INT32	sysSetTimerReferenceClock (INT32 nTimeNo, UINT32 uClockRate);
INT32	sysStartTimer (INT32 nTimeNo, UINT32 uTicksPerSecond, INT32 nOpMode);
INT32	sysStopTimer (INT32 nTimeNo);
VOID	sysClearWatchDogTimerCount (VOID);
VOID	sysClearWatchDogTimerInterruptStatus(VOID);
VOID	sysDisableWatchDogTimer (VOID);
VOID	sysDisableWatchDogTimerReset(VOID);
VOID	sysEnableWatchDogTimer (VOID);
VOID	sysEnableWatchDogTimerReset(VOID);
PVOID	sysInstallWatchDogTimerISR (INT32 nIntTypeLevel, PVOID pvNewISR);
INT32	sysSetWatchDogTimerInterval (INT32 nWdtInterval);
INT32	sysSetTimerEvent(INT32 nTimeNo, UINT32 uTimeTick, PVOID pvFun);
VOID	sysClearTimerEvent(INT32 nTimeNo, UINT32 uTimeEventNo);

/* Define system library UART functions */
INT8	sysGetChar (VOID);
INT32	sysInitializeUART (WB_UART_T *uart);
VOID	sysPrintf (PINT8 pcStr,...);
VOID	sysPutChar (UINT8 ucCh);

/* Define system library AIC functions */
INT32	sysDisableInterrupt (INT_SOURCE_E eIntNo);
INT32	sysEnableInterrupt (INT_SOURCE_E eIntNo);
PVOID	sysInstallExceptionHandler (INT32 nExceptType, PVOID pvNewHandler);
PVOID	sysInstallFiqHandler (PVOID pvNewISR);
PVOID	sysInstallIrqHandler (PVOID pvNewISR);
PVOID	sysInstallISR (INT32 nIntTypeLevel, INT_SOURCE_E eIntNo, PVOID pvNewISR);
INT32	sysSetGlobalInterrupt (INT32 nIntState);
INT32	sysSetInterruptPriorityLevel (INT_SOURCE_E eIntNo, UINT32 uIntLevel);
INT32	sysSetInterruptType (INT_SOURCE_E eIntNo, UINT32 uIntSourceType);
INT32	sysSetLocalInterrupt (INT32 nIntState);
INT32	sysSetAIC2SWMode(VOID);

/* Define system library Cache functions */
VOID	sysEnableCache(UINT32 uCacheOpMode);
VOID	sysDisableCache(VOID);
VOID	sysFlushCache(INT32 nCacheType);
BOOL	sysCacheState(VOID);
VOID	sysInvalidCache(VOID);
#endif  /* _WBLIB_H */


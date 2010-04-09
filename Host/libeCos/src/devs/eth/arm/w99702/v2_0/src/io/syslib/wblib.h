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

//#include "wbio.h"
#include "pkgconf/system.h"
#include "cyg/error/codes.h"
#include "cyg/infra/cyg_type.h"  // Common type definitions and support
                                 // including endian-ness
//#include "wberrcode.h"
#include "std.h"

//-- function return value
#define	   Successful  0
#define	   Fail        -1

#define SYSLIB_NO_ERR			0						/* No error */

/* UART return value */
#define WB_INVALID_PARITY		(SYSLIB_ERR_ID|0x01)	/* UART invalid parity */
#define WB_INVALID_DATA_BITS	(SYSLIB_ERR_ID|0x02)	/* UART invalid data bits */
#define WB_INVALID_STOP_BITS	(SYSLIB_ERR_ID|0x03)	/* UART invalid stop bits */
#define WB_INVALID_BAUD			(SYSLIB_ERR_ID|0x04)	/* UART invalid baud rate */

/* AIC return value */
#define SYSLIB_AIC_ERR_INT		(SYSLIB_ERR_ID|0x10)	/* error INT source of AIC */

//#define APB_SYSTEM_CLOCK  25000000  /* 25MHz */
#define APB_SYSTEM_CLOCK  12000000

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


typedef struct datetime_t
{
	cyg_uint32	year;
	cyg_uint32	mon;
	cyg_uint32	day;
	cyg_uint32	hour;
	cyg_uint32	min;
	cyg_uint32	sec;
} DateTime_T;

/* Define constants for use timer in service parameters.  */
#define TIMER0            0
#define TIMER1            1

#define ONE_SHOT_MODE     0
#define PERIODIC_MODE     1
#define TOGGLE_MODE       2

#define ONE_HALF_SECS     0
#define FIVE_SECS         1
#define TEN_SECS          2
#define TWENTY_SECS       3

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

/* The parameters for sysSetInterruptPriorityLevel() and 
   sysInstallISR() use */
#define FIQ_LEVEL_0                0
#define IRQ_LEVEL_1                1
#define IRQ_LEVEL_2                2
#define IRQ_LEVEL_3                3
#define IRQ_LEVEL_4                4
#define IRQ_LEVEL_5                5
#define IRQ_LEVEL_6                6
#define IRQ_LEVEL_7                7

/* The parameters for sysSetGlobalInterrupt() use */
#define ENABLE_ALL_INTERRUPTS      0
#define DISABLE_ALL_INTERRUPTS     1

/* The parameters for sysSetInterruptType() use */
#define LOW_LEVEL_SENSITIVE        0x00
#define HIGH_LEVEL_SENSITIVE       0x40
#define NEGATIVE_EDGE_TRIGGER      0x80
#define POSITIVE_EDGE_TRIGGER      0xC0

/* The parameters for sysSetLocalInterrupt() use */
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
#define CACHE_8M		3
#define CACHE_16M		4
#define CACHE_32M		5
#define I_CACHE			6
#define D_CACHE			7
#define I_D_CACHE		8

/* Define UART initialization data structure */
typedef struct UART_INIT_STRUCT
{
    cyg_uint32       uiFreq;
    cyg_uint32       uiBaudrate;
    cyg_uint8        uiDataBits;
    cyg_uint8        uiStopBits;
    cyg_uint8        uiParity;
    cyg_uint8        uiRxTriggerLevel;
} WB_UART_T;


/* Define system library Timer functions */
cyg_uint32	sysGetTicks (cyg_int32 nTimeNo);
cyg_int32	sysResetTicks (cyg_int32 nTimeNo);
cyg_int32	sysUpdateTickCount(cyg_int32 nTimeNo, cyg_uint32 uCount);
cyg_int32	sysSetTimerReferenceClock (cyg_int32 nTimeNo, cyg_uint32 uClockRate);
cyg_int32	sysStartTimer (cyg_int32 nTimeNo, cyg_uint32 uTicksPerSecond, cyg_int32 nOpMode);
cyg_int32	sysStopTimer (cyg_int32 nTimeNo);
VOID	sysClearWatchDogTimerCount (VOID);
VOID	sysClearWatchDogTimerInterruptStatus(VOID);
VOID	sysDisableWatchDogTimer (VOID);
VOID	sysDisableWatchDogTimerReset(VOID);
VOID	sysEnableWatchDogTimer (VOID);
VOID	sysEnableWatchDogTimerReset(VOID);
PVOID	sysInstallWatchDogTimerISR (cyg_int32 nIntTypeLevel, PVOID pvNewISR);
cyg_int32	sysSetWatchDogTimerInterval (cyg_int32 nWdtInterval);
cyg_int32	sysSetTimerEvent(cyg_int32 nTimeNo, cyg_uint32 uTimeTick, PVOID pvFun);
VOID	sysClearTimerEvent(cyg_int32 nTimeNo, cyg_uint32 uTimeEventNo);
VOID	sysSetLocalTime(DateTime_T ltime);
VOID	sysGetCurrentTime(DateTime_T *curTime);
VOID	sysDelay(cyg_uint32 uTicks);

/* Define system library UART functions */
//cyg_int8	sysGetChar (VOID);
//cyg_int32	sysInitializeUART (WB_UART_T *uart);
//VOID	sysPrintf (PINT8 pcStr,...);
//VOID	sysprintf (PINT8 pcStr,...);
//VOID	sysPutChar (cyg_uint8 ucCh);

/* Define system library AIC functions */
cyg_int32	sysDisableInterrupt (INT_SOURCE_E eIntNo);
cyg_int32	sysEnableInterrupt (INT_SOURCE_E eIntNo);
PVOID	sysInstallExceptionHandler (cyg_int32 nExceptType, PVOID pvNewHandler);
PVOID	sysInstallFiqHandler (PVOID pvNewISR);
PVOID	sysInstallIrqHandler (PVOID pvNewISR);
PVOID	sysInstallISR (cyg_int32 nIntTypeLevel, INT_SOURCE_E eIntNo, PVOID pvNewISR);
cyg_int32	sysSetGlobalInterrupt (cyg_int32 nIntState);
cyg_int32	sysSetInterruptPriorityLevel (INT_SOURCE_E eIntNo, cyg_uint32 uIntLevel);
cyg_int32	sysSetInterruptType (INT_SOURCE_E eIntNo, cyg_uint32 uIntSourceType);
cyg_int32	sysSetLocalInterrupt (cyg_int32 nIntState);
cyg_int32	sysSetAIC2SWMode(VOID);
BOOL	sysGetIBitState(VOID);

/* Define system library Cache functions */
VOID	sysEnableCache(cyg_uint32 uCacheOpMode);
VOID	sysDisableCache(VOID);
VOID	sysFlushCache(cyg_int32 nCacheType);
BOOL	sysCacheState(VOID);
VOID	sysInvalidCache(VOID);

#endif  /* _WBLIB_H */


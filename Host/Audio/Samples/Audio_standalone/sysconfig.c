/******************************************************************************
 *                                                                            
 * Copyright (c) 2005 - 2006 Winbond Electronics Corp. All rights reserved.   
 *
 * 
 * FILENAME : sysconfig.c
 *
 * VERSION  : 0.9
 *
 * DESCRIPTION : 
 *               Give a function sysConfiguration() to set up the cache, PLLs,
 *               clock source, Timer 0 and UART.
 *
 * HISTORY
 *               03/30/2005      ver 0.9 Created by PC30 MNCheng
 *
 ******************************************************************************/
#include <stdio.h>
#include "wbtypes.h"
#include "wbio.h"
#include "wblib.h"
#include "w99702_reg.h"

/* 
  Cache Setting :
     Turn on one of the define to enable cache in write-back or write-through mode.
     The cache is disabled if both defines are turned off
*/
#define WBCONF_CACHE_WRITE_BACK			1
//#define WBCONF_CACHE_WRITE_THROUGH		1


/* 
  APLL Setting :
     Turn on one of the define to set up APLL output frequency
     The APLL is turned off, if none is turned on.
*/     
//#define WBCONF_APLL_48MHZ			1
//#define WBCONF_APLL_60MHZ			1
//#define WBCONF_APLL_72MHZ			1
//#define WBCONF_APLL_96MHZ			1
//#define WBCONF_APLL_120MHZ		1
//#define WBCONF_APLL_144MHZ		1
//#define WBCONF_APLL_156MHZ		1


/* 
  UPLL Setting :
     Turn on one of the define to set up UPLL output frequency
     The UPLL is turned off, if none is turned on.
*/    
//#define WBCONF_UPLL_48MHZ			1
//#define WBCONF_UPLL_60MHZ			1
//#define WBCONF_UPLL_72MHZ			1
#define WBCONF_UPLL_96MHZ			1
//#define WBCONF_UPLL_120MHZ		1
//#define WBCONF_UPLL_144MHZ		1
//#define WBCONF_UPLL_156MHZ		1


/* 
  UART Setting :
     Turn on the following define, if you want to use debug console
*/
#define WBCONF_USE_UART		1


void sysConfiguration()
{
  unsigned int volatile nLoop;

#ifdef WBCONF_USE_UART
  WB_UART_T	my_uart;
#endif

  /* Diable all interrupt for safity */
  sysSetGlobalInterrupt(DISABLE_ALL_INTERRUPTS); /* write 0xFFFFFFFF to AIC_MDCR */
  
  /* Cache Setting */
  sysDisableCache();
  sysInvalidCache();
#ifdef WBCONF_CACHE_WRITE_BACK  
  sysEnableCache(CACHE_WRITE_BACK);
#endif  
#ifdef WBCONF_CACHE_WRITE_THROUGH
  sysEnableCache(CACHE_WRITE_THROUGH);
#endif  

  /* Clock controller */
  outpw(REG_CLKSEL,  0x104);      /* system clock source is from external clock */  
  outpw(REG_CLKCON,  0x02C01000); /* Enable CPU and APB clock */  
  outpw(REG_APLLCON, 0xE220);     /* Turn off APLL */
  outpw(REG_UPLLCON, 0xE220);	  /* Turn off UPLL */
  
  /* APLL setting */
#ifdef WBCONF_APLL_48MHZ  
  outpw(REG_APLLCON, 0x6210);
#endif  
#ifdef WBCONF_APLL_60MHZ  
  outpw(REG_APLLCON, 0x6214);
#endif  
#ifdef WBCONF_APLL_72MHZ  
  outpw(REG_APLLCON, 0x6218);
#endif  
#ifdef WBCONF_APLL_96MHZ  
  outpw(REG_APLLCON, 0x6550);
#endif
#ifdef WBCONF_APLL_120MHZ  
  outpw(REG_APLLCON, 0x4214);
#endif     
#ifdef WBCONF_APLL_144MHZ  
  outpw(REG_APLLCON, 0x453C);
#endif  
#ifdef WBCONF_APLL_156MHZ  
  outpw(REG_APLLCON, 0x421A);
#endif  
  
  /* UPLL setting */
#ifdef WBCONF_UPLL_48MHZ  
  outpw(REG_UPLLCON, 0x6210);
#endif  
#ifdef WBCONF_UPLL_60MHZ  
  outpw(REG_UPLLCON, 0x6214);
#endif  
#ifdef WBCONF_UPLL_72MHZ  
  outpw(REG_UPLLCON, 0x6218);
#endif  
#ifdef WBCONF_UPLL_96MHZ  
  outpw(REG_UPLLCON, 0x6550);
#endif
#ifdef WBCONF_UPLL_120MHZ  
  outpw(REG_UPLLCON, 0x4214);
#endif     
#ifdef WBCONF_UPLL_144MHZ  
  outpw(REG_UPLLCON, 0x453C);
#endif  
#ifdef WBCONF_UPLL_156MHZ  
  outpw(REG_UPLLCON, 0x421A);
#endif  
  
  /* Clock setting */
  outpw(REG_CLKDIV0, 0x01104001); /* PCLK/2 */
  outpw(REG_CLKDIV1, 0x00015555); /* HCLK2/2 */  

  /* delay loop for PLL stable */
  for (nLoop=0; nLoop<100000; nLoop++);

  /* select clock from PLL */
  outpw(REG_CLKSEL,  0x364);
  //outpw(REG_CLKSEL,  0x110);

  /* enable engine clock */
  //outpw(REG_CLKCON,  0x1B003090);
  outpw(REG_CLKCON,  0x7F0F7CD6);
  
  /* Start Timer channel 0 */
  sysSetTimerReferenceClock(TIMER0, 12000000); /* reference clock */
  sysStartTimer(TIMER0, 100, PERIODIC_MODE);   /* 100 ticks per second */
     
  /* UART setting */
#ifdef WBCONF_USE_UART	
	my_uart.uiFreq     = 12000000; /* 12MHZ */
	//my_uart.uiFreq = 26000000;//for bird
	my_uart.uiBaudrate = 57600;
	my_uart.uiDataBits = WB_DATA_BITS_8;
	my_uart.uiStopBits = WB_STOP_BITS_1;
	my_uart.uiParity   = WB_PARITY_NONE;
	my_uart.uiRxTriggerLevel = LEVEL_8_BYTES;
	
	sysInitializeUART(&my_uart);
#endif	          
      
} /* end sysConfiguration */



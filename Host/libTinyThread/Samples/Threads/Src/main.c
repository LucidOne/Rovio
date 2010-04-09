#include <stdlib.h>

#include "../../../../libTinyThread/Inc/tt_thread.h"
#include "../Inc/clk.h"
#include "../Inc/pll.h"



#define OPT_XIN_CLOCK			12000000
#define OPT_UPLL_OUT_CLOCK		(108 * 1000 * 1000)

int sysInit (void)
{
	/* <1> Enable flash. */
	sysFlushCache (I_D_CACHE);
	sysDisableCache ();
	sysEnableCache (CACHE_WRITE_BACK);

	/* <2> Disable almost all of the engines for safety. */
	outpw (REG_CLKCON, CPUCLK_EN | APBCLK_EN | HCLK2_EN);
	outpw (REG_CLKSEL, SYSTEM_CLK_FROM_XTAL);
	outpw (REG_APLLCON, PLL_OUT_OFF);
	outpw (REG_UPLLCON, PLL_OUT_OFF);
	
	/* <3> Initialize UART. */
	{
		WB_UART_T uart;
		
		TURN_ON_UART_CLOCK;
		uart.uiFreq = OPT_XIN_CLOCK;
		uart.uiBaudrate = 57600;
		uart.uiDataBits = WB_DATA_BITS_8;
		uart.uiStopBits = WB_STOP_BITS_1;
		uart.uiParity = WB_PARITY_NONE;
		uart.uiRxTriggerLevel = LEVEL_1_BYTE;
		sysInitializeUART(&uart);
		sysSafePrintf ("UART ok\n");
	}


	/* <4> Set PLL. */
	// SDRAM MCLK automatically on/off
	outpw(REG_SDICON, inpw(REG_SDICON) & 0xBFFFFFFF);
	// Force MPEG4 transfer length as 8 bytes
	outpw(REG_BBLENG1, (inpw(REG_BBLENG1) & 0xFFFFFFF8) | 0x2);
	
	// CLKDIV0
	// Bit 1~0 : APB = HCLK1/?  -------------------------+
	// Bit15~8 : VPOST = PLL/?  -----------------------+ |
	// Bit19~16: USB = PLL/?    ---------------------+ | |
	// Bit23~20: Audio = PLL/?  --------------------+| | |
	// Bit27~24: Sensor = PLL/? -------------------+|| | |
	// Bit31~28: System = PLL/? ------------------+||| | |
	                                         //   |||| | | 
	                                         //   ||||++ |
	outpw (REG_CLKDIV0, inpw(REG_CLKDIV0)&0x0fc|0x01322302);
	outpw (REG_UPLLCON, pllEvalPLLCON (OPT_XIN_CLOCK, OPT_UPLL_OUT_CLOCK));
	
	                 //  +-------- Bit31~28 : HCLK (from system)
	                 //  |+------- Bit27~24 : CPU (from HCLK)
	                 //  ||+------ Bit23~20 : HCLK1 (from HCLK, must less or equal to HCLK and CPU)
	                 //  |||+----- Bit19~16 : HCLK2 (from HCLK)
	                 //  ||||+---- Bit15~12 : MPEG4, JPEG (from HCLK2)
	                 //  |||||+--- Bit11~8  : 2D, VPE (from HCLK2)
	                 //  ||||||+-- Bit 7~6  : DSP (from HCLK2)
	                 //  |||||||+- Bit 5~0  : Reserved.
	outpw(REG_CLKDIV1, 0x00104045);
	outpw (REG_CLKSEL, SYSTEM_CLK_FROM_UPLL);

	/* <5> Initialize TinyThread to support multi-thread. */
	tt_init (OPT_XIN_CLOCK, OPT_UPLL_OUT_CLOCK);

	return 0;
}



/* 32K stack size for thread1. */
UINT32 g_thread1_buffer[TT_THREAD_BUFFER_SIZE(32*1024)/sizeof(UINT32)];
/* 32K stack size for thread2. */
UINT32 g_thread2_buffer[TT_THREAD_BUFFER_SIZE(32*1024)/sizeof(UINT32)];


void thread_entry (void *arg)
{
	TT_THREAD_T *the_thread = tt_get_current_thread ();
	
	while (1)
	{
		sysSafePrintf ("Run thread %s with argument (%s)\n", the_thread->name, arg);
	}
}

int main ()
{
	/* Initialize uart, clock, thread, etc.. */
	sysInit ();
	
	/* Create thread 1.
	   Be sure that "g_thread1_buffer" is available when thread is running. */
	tt_create_thread ("thread_1", 0,
		g_thread1_buffer, sizeof (g_thread1_buffer),
		thread_entry, "In thread 1");

	/* Create thread 2.
	   Be sure that "g_thread2_buffer" is available when thread is running. */
	tt_create_thread ("thread_2", 0,
		g_thread2_buffer, sizeof (g_thread2_buffer),
		thread_entry, "In thread 2");
	
	while (1);
}

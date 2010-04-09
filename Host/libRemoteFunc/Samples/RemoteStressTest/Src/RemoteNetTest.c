#include "../../../../SysLib/Inc/wb_syslib_addon.h"
#include "../../../../libHIC_Host/Inc/hic_host.h"
#include "../../../libFunc_Through_HIC/Inc/FTH_Int.h"
#include "../../../libRemoteFunc/Inc/RemoteFunc.h"
#include "../../../libRemoteFunc/Inc/RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteNetTest.h"


char g_LargeData[] =
{
#include "largedata.h"
};

#pragma arm section zidata = "non_init"
__align (32) CHAR g_RemoteNet_Buf[MEM_ALIGN_SIZE(RNT_BUFFER_LEN)];
__align (32) CHAR g_RemoteNet_Buf1[MAX_THREADS][MEM_ALIGN_SIZE(RNT_BUFFER_LEN)];
#pragma arm section zidata

__align (32) char thread_stack[MAX_THREADS][MEM_ALIGN_SIZE(STACK_SIZE)];
cyg_handle_t thread_handle[MAX_THREADS];
cyg_thread thread[MAX_THREADS];


int sysInit (void);

int main(void)
{
	/* <1> Enable flash. */
	sysFlushCache (I_D_CACHE);
	sysDisableCache ();
	sysEnableCache (CACHE_WRITE_BACK);
	sysInit();
	printf("REG_APLLCON=0x%x REG_UPLLCON=0x%x\n", inpw(REG_APLLCON), inpw(REG_UPLLCON));
	printf("REG_CLKCON=0x%x REG_CLKSEL=0x%x\n", inpw(REG_CLKCON), inpw(REG_CLKSEL));
	printf("REG_CLKDIV0=0x%x REG_CLKDIV1=0x%x\n", inpw(REG_CLKDIV0), inpw(REG_CLKDIV1));
	printf("REG_TICR0=%d\n", inpw(REG_TICR0));
	FTH_Init();
	
	test_speed_entry();
	return 0;
}


int sysInit (void)
{
	/* <0> Save threads debug entry. */
#ifndef ECOS
	{
		extern UCHAR ASM_THREADS_DUMP_FUN[];	//A buffer in wb_init.s
		UINT32 uThreadDebugFun_Addr = (UINT32) &tt_dump_threads;
		memcpy (ASM_THREADS_DUMP_FUN, &uThreadDebugFun_Addr, sizeof (UINT32));
	}
#endif
	
	/* <1> Enable flash. */
	sysFlushCache (I_D_CACHE);
	sysDisableCache ();
	sysEnableCache (CACHE_WRITE_BACK);

	/* <2> Disable almost all of the engines for safety. */
	outpw (REG_CLKCON, inpw (REG_CLKCON) | CPUCLK_EN | APBCLK_EN | HCLK2_EN);
	outpw (REG_CLKSEL, SYSTEM_CLK_FROM_XTAL);
	outpw (REG_APLLCON, PLL_OUT_OFF);
	outpw (REG_UPLLCON, PLL_OUT_OFF);
	
	/* <3> Initialize UART. */
	/*
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
		printf ("UART ok\n");
	}
	*/

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
//	outpw(REG_CLKDIV0, inpw(REG_CLKDIV0)&0x0fc|0x01322302);	//(ok for 108)
#ifndef ECOS
	outpw(REG_CLKDIV0, inpw(REG_CLKDIV0)&0x0fc|0x25362401);	//(ok for 336(112))
#else
	//outpw(REG_CLKDIV0, inpw(REG_CLKDIV0)&0xf00000fc|0x05362401);	//can't modify system clock
	outpw(REG_CLKDIV0, inpw(REG_CLKDIV0)&0x0fc|0x25362401);	//(ok for 336(112))
#endif
//	outpw(REG_CLKDIV0, inpw(REG_CLKDIV0)&0x0fc|0x03331802);	//(ok for 144)

	//Adjust SDRAM
	printf("setting apll to 0x%x, upll to 0x%x\n", pllEvalPLLCON (OPT_XIN_CLOCK, OPT_APLL_OUT_CLOCK), pllEvalPLLCON (OPT_XIN_CLOCK, OPT_UPLL_OUT_CLOCK));
	outpw(REG_APLLCON, pllEvalPLLCON (OPT_XIN_CLOCK, OPT_APLL_OUT_CLOCK));
	outpw(REG_UPLLCON, pllEvalPLLCON (OPT_XIN_CLOCK, OPT_UPLL_OUT_CLOCK));

	                 //  +-------- Bit31~28 : HCLK (from system)
	                 //  |+------- Bit27~24 : CPU (from HCLK)
	                 //  ||+------ Bit23~20 : HCLK1 (from HCLK, must less or equal to HCLK and CPU)
	                 //  |||+----- Bit19~16 : HCLK2 (from HCLK)
	                 //  ||||+---- Bit15~12 : MPEG4, JPEG (from HCLK2)
	                 //  |||||+--- Bit11~8  : 2D, VPE (from HCLK2)
	                 //  ||||||+-- Bit 7~6  : DSP (from HCLK2)
	                 //  |||||||+- Bit 5~0  : Reserved.
//	outpw(REG_CLKDIV1, 0x00105045);	//(ok for 108)
	outpw(REG_CLKDIV1, 0x00105045);	//(ok for 112)
//	outpw(REG_CLKDIV1, 0x00110005);
#ifndef ECOS	//don't need to modify system clock source
	outpw(REG_CLKSEL, SYSTEM_CLK_FROM_UPLL);
#endif
printf("before system clock\n");
	outpw(REG_CLKSEL, SYSTEM_CLK_FROM_UPLL);
printf("system clock ok\n");

	/* <5> Initialize TinyThread to support multi-thread. */
#ifndef ECOS
	tt_init (OPT_XIN_CLOCK, OPT_UPLL_OUT_CLOCK);
#else
	//tt_init (OPT_XIN_CLOCK, OPT_UPLL_OUT_CLOCK);
	printf("reg_tcr0=0x%x reg_ticr0=0x%x\n", inpw(REG_TCR0), inpw(REG_TICR0));
	printf("reg_tcr1=0x%x reg_ticr1=0x%x\n", inpw(REG_TCR1), inpw(REG_TICR1));
#endif

	/* <6> Set other engine clocks. */

	/* Set vpost clock:
	Why I can NOT call it after HIC is transferred. */
	outpw (REG_CLKSEL, inpw (REG_CLKSEL) | VIDEO_CLK_FROM_UPLL);
	tt_msleep (30);
	TURN_ON_VPOST_CLOCK;
	tt_msleep (30);

	/* Set audio clock. */
	outpw (REG_CLKSEL, inpw (REG_CLKSEL) | AUDIO_CLK_FROM_APLL);
	tt_msleep (30);
	TURN_ON_AUDIO_CLOCK;
	tt_msleep (30);
	
	/* Set sensor clock. */
	outpw (REG_CLKSEL, inpw (REG_CLKSEL) | SENSOR_CLK_FROM_UPLL);
	//tt_msleep (30);
	//TURN_ON_DSP_CLOCK;
	//tt_msleep (30);
	//TURN_ON_CAP_CLOCK;

	/* Set MP4 clock. */
	//TURN_ON_MPEG4_CLOCK;
	//tt_msleep (30);
	
	/* Set VPE and 2D clock. */
	TURN_ON_VPE_CLOCK;
	tt_msleep (30);
	TURN_ON_GFX_CLOCK;
	tt_msleep (30);
	
	/* Set USB clock */
#if 0	
	writew(REG_SYS_CFG, readw(REG_SYS_CFG)&0xffffffbf);
	outpw (REG_CLKSEL, inpw (REG_CLKSEL) | USB_CLK_FROM_UPLL);
	tt_msleep (30);
	TURN_ON_USB_CLOCK;
	outpw(REG_GPIO_PE,inpw(REG_GPIO_PE)|0x00000200);//GPIO9,disable pull-up or pull-down  
#endif
	
	return 0;
}



#include "../Inc/inc.h"
#include "../../powerctrl/Inc/702clk.h"
const char *g_apcRevision[] =
{
#include "../../../ChangeLog.txt"
};

#define TT_TICKS_PER_SECOND 50

int tt_init(UINT32 timer_rreq, UINT32 cpu_freq)
{
   UINT32 _mTicr, _mTcr;
   
   _mTcr = 0x60000000 | (PERIODIC_MODE << 27);
   _mTicr = timer_rreq / TT_TICKS_PER_SECOND;
           outpw(REG_TICR0, _mTicr);
           outpw(REG_TCR0, _mTcr);

	outpw(REG_CLKCON, inpw(REG_CLKCON) | TCLK_EN);
	return 0;
}

int VideoPhoneInitClock (void)
{
	static BOOL bInited = FALSE;
	
	
	if (bInited)
		return;
	bInited = TRUE;

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
	
	/* <2> Set clock. */
	{
		w99702_clk_t clkIPCam;
		clkIPCam.clkcon =	ACLK_EN | VCLK_EN | CPUCLK_EN | HCLK2_EN
							| _2DCLK_EN | VPECLK_EN
							| FMICLK_EN | APBCLK_EN
							| TCLK_EN | UR1CLK_EN | UR2CLK_EN;
		clkIPCam.apllcon =	pllEvalPLLCON( OPT_XIN_CLOCK, OPT_APLL_OUT_CLOCK );
		clkIPCam.upllcon =	pllEvalPLLCON( OPT_XIN_CLOCK, OPT_UPLL_OUT_CLOCK );
		clkIPCam.clksel =	SYSTEM_CLK_FROM_UPLL | AUDIO_CLK_FROM_APLL | VIDEO_CLK_FROM_UPLL | SENSOR_CLK_FROM_UPLL;
		// CLKDIV0            +------------- Bit31~28: System = PLL/?
		//                    |+------------ Bit27~24: Sensor = PLL/?
		//                    ||+----------- Bit23~20: Audio = PLL/?
		//                    |||+---------- Bit19~16: USB = PLL/?
		//                    |||| +-------- Bit15~8 : VPOST = PLL/?
		//                    |||| | +------ Bit 1~0 : APB = HCLK1/?
		//                    ||||++ |

		clkIPCam.clkdiv0 =	0x00720202;
		// CLKDIV1            +------------- Bit31~28 : HCLK (from system)
		//                    |+------------ Bit27~24 : CPU (from HCLK)
		//                    ||+----------- Bit23~20 : HCLK1 (from HCLK, must less or equal to HCLK and CPU)
		//                    |||+---------- Bit19~16 : HCLK2 (from HCLK)
		//                    ||||+--------- Bit15~12 : MPEG4, JPEG (from HCLK2)
		//                    |||||+-------- Bit11~8  : 2D, VPE (from HCLK2)
		//                    ||||||+------- Bit 7~6  : DSP (from HCLK2)
		//                    |||||||+------ Bit 5~0  : Reserved.
		//                    ||||||||
		
		clkIPCam.clkdiv1 =	0x00110015;
		clkIPCam.sdcon =	0x130080FF;
#if CONFIG_MEMORY_SIZE == 16	
		clkIPCam.sdtime0 =	inpw(REG_SDTIME0);
#elif CONFIG_MEMORY_SIZE == 8
		clkIPCam.sdtime0 =	0xF8006948;
#else
#	error "No memory size defined"
#endif
		
		pwr_set_clk(&clkIPCam); 
	}

	//{ static int i = 0; while ( 1 ) diag_printf ( "Hello ======= %d\n", i++ ); }


	/* <3> ........ */
	// SDRAM MCLK automatically on/off
	//outpw(REG_SDICON, inpw(REG_SDICON) & 0xBFFFFFFF);
	// Force MPEG4 transfer length as 8 bytes
	outpw(REG_BBLENG1, (inpw(REG_BBLENG1) & 0xFFFFFFF8) | 0x2);
	
	/* <5> Initialize TinyThread to support multi-thread. */
#ifndef ECOS
	tt_init (OPT_XIN_CLOCK, OPT_UPLL_OUT_CLOCK);
#else
	//tt_init (OPT_XIN_CLOCK, OPT_UPLL_OUT_CLOCK);
	printf("reg_tcr0=0x%x reg_ticr0=0x%x\n", inpw(REG_TCR0), inpw(REG_TICR0));
	printf("reg_tcr1=0x%x reg_ticr1=0x%x\n", inpw(REG_TCR1), inpw(REG_TICR1));
#endif

	/* <4> Set other engine clocks. */
	/* Set VPE and 2D clock. */
	TURN_ON_VPE_CLOCK;
	tt_msleep (30);
	TURN_ON_GFX_CLOCK;
	tt_msleep (30);


	
	/* Set USB Clock */
#if 1	
	writew(REG_SYS_CFG, readw(REG_SYS_CFG)&0xffffffbf);
	outpw (REG_CLKSEL, inpw (REG_CLKSEL) | USB_CLK_FROM_UPLL);
	TURN_ON_USB_CLOCK;
	outpw(REG_GPIO_PE,inpw(REG_GPIO_PE)|0x00000200); /* GPIO9,disable pull-up or pull-down */
#endif
	
	diag_printf( "Set clock OK\n" );
	
	return 0;
}

#ifndef ECOS
int main ()
#else
int VideoPhoneInit(void)
#endif
{
	int rt;
	
	VideoPhoneInitClock ();
	
	
	printf("REG_APLLCON=0x%x REG_UPLLCON=0x%x\n", inpw(REG_APLLCON), inpw(REG_UPLLCON));
	printf("REG_CLKCON=0x%x REG_CLKSEL=0x%x\n", inpw(REG_CLKCON), inpw(REG_CLKSEL));
	printf("REG_CLKDIV0=0x%x REG_CLKDIV1=0x%x\n", inpw(REG_CLKDIV0), inpw(REG_CLKDIV1));
	printf("REG_TICR0=%d\n", inpw(REG_TICR0));

	bufInit ();
	
	vjpegInit();
	vdInit ();
	printf("before vpe init\n");
	vvpeInit ();
	printf("vpe init ok\n");
	
	vgfxInit ();
	
	printf("before lcm init\n");
	vlcmInit ();
	vmp4Init ();
	
#ifdef ECOS
	printf("iothread init\n");
	iothread_Init();
#endif
	
	printf("vcpt init\n");
	vcptInitThread ();

	printf("vau init\n");
	rt = vauInit ();
	printf("vau init %d\n", rt);
	
	vinfoInit();
	
	printf("all ok\n");
#ifndef ECOS
	hicInitThread ();
#endif
	return 0;
}


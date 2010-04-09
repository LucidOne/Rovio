#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd57_56_SuspendModule (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	/* Stop audio. */
	vauUninit ();
	/* Stop catpure. */
	vcptUninitThread ();

	sysDisableIRQ ();
	hicSetCmdReady (pHicThread);
	//tt_msleep (100);
	sysStopTimer (TIMER0);
	
	outpw (REG_CLKCON, CPUCLK_EN | APBCLK_EN | HCLK2_EN);
	outpw (REG_CLKSEL, SYSTEM_CLK_FROM_XTAL);
	outpw (REG_APLLCON, PLL_OUT_OFF);
	outpw (REG_UPLLCON, PLL_OUT_OFF);
	sysEnableIRQ ();
}

#endif

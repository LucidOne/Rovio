#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "wblib.h"
#include "w99702_reg.h"
#include "wbspi.h"
#include "702clk.h"
//#include "pll.h"
#include "clk.h"
#include "cyg/kernel/kapi.h"
static BOOL g_bDisableDebugMsg = FALSE;

char buf[] = 
{
	#include "image1.h"
};
char readbuf[100*1024];

void hi_uart_write( const char *buf, size_t size )
{
	size_t i;
	
	g_bDisableDebugMsg = TRUE;
	for ( i = 0; i < size; i++ )
		hal_if_diag_write_char( buf[i] );
}

void hi_uart_read( const char *buf, size_t size )
{
	size_t i;
	for ( i = 0; i < size; i++ )
		hal_if_diag_read_char( &buf[i] );
}

void uart_write( const char *buf, size_t size )
{
#if 1
	if (!g_bDisableDebugMsg)
	{
		size_t i;
	
		for ( i = 0; i < size; i++ )
			hal_if_diag_write_char( buf[i] );
	}
#endif
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

		clkIPCam.clkdiv0 =	0x00720902;
		// CLKDIV1            +------------- Bit31~28 : HCLK (from system)
		//                    |+------------ Bit27~24 : CPU (from HCLK)
		//                    ||+----------- Bit23~20 : HCLK1 (from HCLK, must less or equal to HCLK and CPU)
		//                    |||+---------- Bit19~16 : HCLK2 (from HCLK)
		//                    ||||+--------- Bit15~12 : MPEG4, JPEG (from HCLK2)
		//                    |||||+-------- Bit11~8  : 2D, VPE (from HCLK2)
		//                    ||||||+------- Bit 7~6  : DSP (from HCLK2)
		//                    |||||||+------ Bit 5~0  : Reserved.
		//                    ||||||||
		
		clkIPCam.clkdiv1 =	0x00115415;
		clkIPCam.sdcon =	0x130080FF;
		clkIPCam.sdtime0 =	0xF8006948;
		
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

	diag_printf( "Set clock OK\n" );
	
	return 0;
}

cyg_handle_t	int_handle_keypad;
cyg_interrupt	int_holder_keypad;
static cyg_tick_count_t	g_StartTime;
static cyg_tick_count_t	g_EndTime;
#define KEY_PRESSED		0
#define KEY_RELEASED	1
#define KEY_REPEATED	2
static INT g_KeyStatus = KEY_RELEASED;

void KeyPad_Int_ISR(void)
{
	#if 0	
	UINT32 code1, code2;
	diag_printf("state: %08x\n", inpw(REG_GPIO_IS));
	
		outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT)&~0x100);
		code1 = inpw(REG_GPIO_STS);

		outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT)|0x100);
		code2 = inpw(REG_GPIO_STS);
		if ((((code1 >> 13) & 0x1) == 0) || (((code2 >> 13) & 0x1) == 0))
		{
			g_KeyStatus = KEY_PRESSED;
			g_StartTime =  cyg_current_time();	
			diag_printf("**********pressed\n");
		
		}
		else if(g_KeyStatus == KEY_PRESSED)
		{
					g_KeyStatus = KEY_RELEASED;
			g_EndTime =  cyg_current_time();	
			diag_printf("*************************released\n");
			if((g_EndTime - g_StartTime) >= 500)
						diag_printf("reset\n");	

		}
		
		
		if ((((code1 >> 13) & 0x1) != 0) || (((code2 >> 13) & 0x1) != 0) || (g_KeyStatus != KEY_PRESSED))
		{
				outpw(REG_GPIO_IS, inpw(REG_GPIO_IS) &(0x2000) );
				return;
		}
		#endif

	switch(g_KeyStatus)
	{
		case KEY_RELEASED:
			g_KeyStatus = KEY_PRESSED;
			g_StartTime =  cyg_current_time();	
			diag_printf("**********pressed\n");
			break;
		case KEY_PRESSED:
			g_KeyStatus = KEY_RELEASED;
			g_EndTime =  cyg_current_time();	
			diag_printf("*************************released\n");
			if((g_EndTime - g_StartTime) >= 500)
			{
				diag_printf("reset\n");	
				/*		
				usiMyWrite(950*1024,sizeof(buf),(UINT8*)buf);
				//memset(buf,0,sizeof(buf));
				usiMyRead(950*1024,sizeof(buf),(UINT8*)readbuf);
				if((memcmp(buf,readbuf,sizeof(buf))) != 0)
					diag_printf("************read error***************\n");
				else
					diag_printf("read right\n");
					*/
			}
			break;
		case KEY_REPEATED:	
			diag_printf("################repeated\n");
			break;
		default:
			;		
	}
	outpw(REG_GPIO_IS, inpw(REG_GPIO_IS) &(0x2000) );
}	
	
cyg_uint32 KeyPad_INT_Handler(cyg_vector_t vector, cyg_addrword_t data)
{
   /* clear GPIO interrupt state */
   //diag_printf("state: %08x\n", inpw(REG_GPIO_IS));
    KeyPad_Int_ISR();
	cyg_interrupt_acknowledge(vector);
    return CYG_ISR_HANDLED;
}

void readGPIO(void)
{
	UINT32 code1, code2;
	/* Install interrupt handler */
    cyg_interrupt_create(IRQ_GPIO, 1, 0, KeyPad_INT_Handler, NULL, &int_handle_keypad, &int_holder_keypad);
    cyg_interrupt_attach(int_handle_keypad);
    cyg_interrupt_unmask(IRQ_GPIO);
    cyg_interrupt_enable();
  #if 1 
	//cyg_tick_count_t time;
    outpw(REG_GPIO_OE,inpw(REG_GPIO_OE)|0x00002000);//10/11/13/15/17/19  input;
	//cyg_thread_delay(1);
    outpw(REG_GPIO_OE,inpw(REG_GPIO_OE)&(~0x00000200));//8/9  output;
	//cyg_thread_delay(1);
    outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT)&(~0x00000200)); // 8 low, 9 low
	//cyg_thread_delay(1);
    outpw(REG_GPIO_IE,inpw(REG_GPIO_IE)&(~0x00000200)|0x00002000);//enable 10/11/13/15/17/19 DMA interrupt

	//cyg_thread_delay(1);
#else
	outpw(REG_GPIO_IS, inpw(REG_GPIO_IS) &(0x2000) );

	/* Install interrupt handler 
    cyg_interrupt_create(IRQ_GPIO, 1, 0, Reset_INT_Handler, NULL, &int_handle_reset, &int_holder_reset);
    cyg_interrupt_attach(int_handle_reset);
    cyg_interrupt_unmask(IRQ_GPIO);
    cyg_interrupt_enable();*/
    
	//cyg_tick_count_t time;
    outpw(REG_GPIO_OE,inpw(REG_GPIO_OE)|0x00002000);//13  input;
	//cyg_thread_delay(1);
  //  outpw(REG_GPIO_OE,inpw(REG_GPIO_OE)&(~0x00000200));//8  output;
	//cyg_thread_delay(1);
 //   outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT)&(~0x00000200)); // 8 low
	//cyg_thread_delay(1);
    outpw(REG_GPIO_IE,inpw(REG_GPIO_IE)|0x00002000);//enable 13 DMA interrupt

#endif
	diag_printf("state: %08x\n", inpw(REG_GPIO_IS));

   	while(0)
	{
		

		outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT)&~0x100);
		code1 = inpw(REG_GPIO_STS);

		outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT)|0x100);
		code2 = inpw(REG_GPIO_STS);
		/*
		if ((((code1 >> 13) & 0x1) == 0) && (((code2 >> 13) & 0x1) == 0))
		{
			 time = cyg_current_time();
		}*/
		diag_printf("state: %08x-%08x\n", code1, code2);
	}
}


int main(void)
{
	//char buf[26];
	int len = sizeof(buf);
	int i;
	sysInit();
	usiInitDevice(144);
	usiWriteEnable();
	memset(readbuf,0,10*1024);
	diag_printf("sizeof buf %d\n",sizeof(buf));
readGPIO();
#if 1
	//usiEraseAll();
	while(0)
	{
		//usiEraseSector(0,sizeof(buf));
		usiMyWrite(950*1024,sizeof(buf),(UINT8*)buf);
		//memset(buf,0,sizeof(buf));

		usiMyRead(950*1024,sizeof(buf),(UINT8*)readbuf);

		if((memcmp(buf,readbuf,sizeof(buf))) != 0)
			diag_printf("************read error***************\n");
		else
			diag_printf("read right\n");
			
		
	cyg_thread_delay(50);

			
	}
#endif

	return 0;
}
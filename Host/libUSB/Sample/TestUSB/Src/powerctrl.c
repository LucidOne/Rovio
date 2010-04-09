#include "wbio.h"
#include "wblib.h"
#include "w99702_reg.h"
#include "702clk.h"

//#define dbg_printf	diag_printf
#define dbg_printf

#if 0
#define TUBE_BASE	(int *)0x500000
static int *tube = TUBE_BASE;
# define MEMPRINT(x)	*tube++ = x
# define MEMCLEAN()	memset(tube, 0, 64)
#else
# define MEMPRINT(x)
# define MEMCLEAN()
#endif

static unsigned int _delay_cnt = 0;
#define MAX_DELAY_LOOPS	0x6000
#define MIN_DELAY_LOOPS 0x400
#define DELAY_LOOPS	1000
//#define DELAY(x)	{volatile int delay;delay = (x); while(delay--);}

#define HAL_DISABLE_INTERRUPTS(_old_)           \
    __asm \
     {                              \
        MRS _old_,CPSR;                          \
        MRS r4,CPSR;                          \
        orr r4,r4,0xC0 ;                     \
        MSR CPSR_cf,r4                           \
     }


#define HAL_RESTORE_INTERRUPTS(_old_)           \
    __asm \
     {                              \
        MRS r3,CPSR;                          \
        and r4,_old_,0xC0;                      \
        bic r3,r3,0xC0;                      \
        orr r3,r3,r4;                         \
        MSR CPSR_cf,r3                           \
     }



static int current_clk_profile = 0;
extern void asm_set_clk(w99702_clk_t *cfg);

w99702_clk_t clk_profile[] = {

	/* Clock setting profile of menu operation mode */
	{0x1BC63094, 0x0000652F, 0x00006534, 0x0000032C, 0x00F00203, 0x01105455, 0x130080FF, 0xB8002424},
	/* Clock setting profile of menu power saving mode */
	{0x1BC63094, 0x0000652F, 0x00006534, 0x0000032C, 0x00F00F03, 0x01105455, 0x130080FF, 0xB8002424}, 
	/* Clock setting profile of mp3 play operation mode */
	{0x1BC63094, 0x0000652F, 0x00006534, 0x0000032C, 0x00400203, 0x01105455, 0x130080FF, 0xB8002424}, 
	/* Clock setting profile of mp3 play power saving mode */
	{0x1B003094, 0x0000652F, 0x00006534, 0x00000320, 0x00400F03, 0x0110AAAA, 0x130080FF, 0xB8002424}, 
	/* Clock setting profile of aac play operation mode */
	{0x1BC63094, 0x0000652F, 0x0000632D, 0x0000032C, 0x00400303, 0x01105455, 0x130080FF, 0xB8004624}, 
	/* Clock setting profile of aac play power saving mode */
	{0x1B003094, 0x0000652F, 0x0000632D, 0x00000320, 0x00400F03, 0x0110AAAA, 0x130080FF, 0xB8004624}, 
	/* Clock setting profile of aac plus play operation mode */
	{0x1BC63094, 0x0000652F, 0x00006534, 0x0000032C, 0x00400203, 0x00105455, 0x130080FF, 0xB8002424}, 
	/* Clock setting profile of aac plus play power saving mode */
	{0x1B003094, 0x0000652F, 0x00006534, 0x00000320, 0x00400F03, 0x0010AAAA, 0x130080FF, 0xB8002424}, 
	/* Clock setting profile of enhanced aac plus play operation mode */
	{0x1BC63094, 0x0000652F, 0x0000632D, 0x0000032C, 0x00400303, 0x00105455, 0x130080FF, 0xB8004624}, 
	/* Clock setting profile of enhanced aac plus play power saving mode */
	{0x1B003094, 0x0000652F, 0x0000632D, 0x00000320, 0x00400F03, 0x0010AAAA, 0x130080FF, 0xB8004624}, 
	/* Clock setting profile of wma play operation mode */
	{0x1BC63094, 0x0000652F, 0x0000632D, 0x0000032C, 0x00400303, 0x01105455, 0x130080FF, 0xB8004624}, 
	/* Clock setting profile of wma play power saving mode */
	{0x1B003094, 0x0000652F, 0x0000632D, 0x00000320, 0x00400F03, 0x0110AAAA, 0x130080FF, 0xB8004624}, 
	/* Clock setting profile of mp4 play low operation speed mode */
	{0x1BC63494, 0x0000652F, 0x00004212, 0x0000032C, 0x00400303, 0x00105455, 0x130080FF, 0xB8004624},
	/* Clock setting profile of mp4 play high operation speed mode */
	{0x1BC63494, 0x0000652F, 0x0000442D, 0x0000032C, 0x00400303, 0x00105455, 0x130080FF, 0xF8006948}, 
	/* Clock setting profile of 3gp rec low operation speed mode */
	{0x3BC734D4, 0x0000652F, 0x0000632D, 0x000003EC, 0x04400303, 0x00105455, 0x130080FF, 0xB8004624}, 
	/* Clock setting profile of 3gp rec high operation speed mode */
	{0x3BC734D4, 0x0000652F, 0x0000442D, 0x000003EC, 0x04400503, 0x00105455, 0x130080FF, 0xF8006948},
	/* Clock setting profile of image viewer low operation mode */
	{0x1BC63094, 0x0000652F, 0x0000442D, 0x0000032C, 0x00F00503, 0x11105555, 0x130080FF, 0xF8006948}, 
	/* Clock setting profile of image viewer high operation mode */
	{0x1BC63094, 0x0000652F, 0x0000442D, 0x0000032C, 0x00400503, 0x00105555, 0x130080FF, 0xF8006948}, 
	/* Clock setting profile of JPEG capture operation speed mode */
	{0x1BC63094, 0x0000652F, 0x00004212, 0x000003CC, 0x04F00303, 0x0110A9AA, 0x130080FF, 0xB8002424}, 


	/* Clock setting profile of menu operation mode */
	{0x1BC63094, 0x0000652F, 0x0000442D, 0x0000032C, 0x0F400403, 0x00305555, 0x130080FF, 0xF8006948},
	/* Clock setting profile of menu power saving mode */
	{0x1BC63094, 0x0000652F, 0x0000442D, 0x0000032C, 0x0F400403, 0x00305555, 0x130080FF, 0xF8006948},
	/* Clock setting profile of mp3 play operation mode */
	{0x1BC63094, 0x0000652F, 0x0000442D, 0x0000032C, 0x0F400403, 0x00305555, 0x130080FF, 0xF8006948},
	/* Clock setting profile of mp3 play power saving mode */
	{0x1BC63094, 0x0000652F, 0x0000442D, 0x0000032C, 0x0F400403, 0x00305555, 0x130080FF, 0xF8006948},
	/* Clock setting profile of aac play operation mode */
	{0x1BC63094, 0x0000652F, 0x0000442D, 0x0000032C, 0x0F400403, 0x00305555, 0x130080FF, 0xF8006948},
	/* Clock setting profile of aac play power saving mode */
	{0x1BC63094, 0x0000652F, 0x0000442D, 0x0000032C, 0x0F400403, 0x00305555, 0x130080FF, 0xF8006948},
	/* Clock setting profile of aac plus play operation mode */
	{0x1BC63094, 0x0000652F, 0x0000442D, 0x0000032C, 0x0F400403, 0x00305555, 0x130080FF, 0xF8006948},
	/* Clock setting profile of aac plus play power saving mode */
	{0x1BC63094, 0x0000652F, 0x0000442D, 0x0000032C, 0x0F400403, 0x00305555, 0x130080FF, 0xF8006948},
	/* Clock setting profile of enhanced aac plus play operation mode */
	{0x1BC63094, 0x0000652F, 0x0000442D, 0x0000032C, 0x0F400403, 0x00305555, 0x130080FF, 0xF8006948},
	/* Clock setting profile of enhanced aac plus play power saving mode */
	{0x1BC63094, 0x0000652F, 0x0000442D, 0x0000032C, 0x0F400403, 0x00305555, 0x130080FF, 0xF8006948},
	/* Clock setting profile of wma play operation mode */
	{0x1BC63094, 0x0000652F, 0x0000442D, 0x0000032C, 0x0F400403, 0x00305555, 0x130080FF, 0xF8006948},
	/* Clock setting profile of wma play power saving mode */
	{0x1BC63094, 0x0000652F, 0x0000442D, 0x0000032C, 0x0F400403, 0x00305555, 0x130080FF, 0xF8006948},
	/* Clock setting profile of mp4 play low operation speed mode */
	{0x1BC63494, 0x0000652F, 0x0000442D, 0x0000032C, 0x0F400403, 0x00305555, 0x130080FF, 0xF8006948},
	/* Clock setting profile of mp4 play high operation speed mode */
	{0x1BC63494, 0x0000652F, 0x0000442D, 0x0000032C, 0x0F400403, 0x00305555, 0x130080FF, 0xF8006948},
	/* Clock setting profile of 3gp rec low operation speed mode */
	{0x3BC734D4, 0x0000652F, 0x0000442D, 0x000003EC, 0x05400403, 0x00305555, 0x510081ff, 0x80005848}, 
	/* Clock setting profile of 3gp rec high operation speed mode */
	{0x3BC734D4, 0x0000652F, 0x0000442D, 0x000003EC, 0x05400403, 0x00305555, 0x510081ff, 0x80005848}, 
	/* Clock setting profile of image viewer low operation speed mode */
	{0x1BC63094, 0x0000652F, 0x0000442D, 0x0000032C, 0x0F400403, 0x00305555, 0x130080FF, 0xF8006948},
	/* Clock setting profile of image viewer high operation speed mode */
	{0x1BC63094, 0x0000652F, 0x0000442D, 0x0000032C, 0x0F400403, 0x00305555, 0x130080FF, 0xF8006948},
	/* Clock setting profile of JPEG capture operation speed mode */
	{0x1BC63094, 0x0000652F, 0x0000442D, 0x000003CC, 0x05400403, 0x00305555, 0x130080FF, 0xF8006948},

	////////////////////

	/* Clock setting profile of menu operation mode */
	{0x1BC63090, 0x0000652F, 0x00006534, 0x0000032C, 0x00F00203, 0x01105455, 0x130080FF, 0xB8002424},
	/* Clock setting profile of menu power saving mode */
	{0x1BC63090, 0x0000652F, 0x00006534, 0x0000032C, 0x00F00F03, 0x01105455, 0x130080FF, 0xB8002424}, 
	/* Clock setting profile of mp3 play operation mode */
	{0x1BC63090, 0x0000652F, 0x00006550, 0x0000032C, 0x00400303, 0x01105455, 0x130080FF, 0xB8004624}, 
	/* Clock setting profile of mp3 play power saving mode */
	{0x1B003090, 0x0000652F, 0x00006550, 0x00000320, 0x00400F03, 0x0110AAAA, 0x130080FF, 0xB8004624}, 
	/* Clock setting profile of aac play operation mode */
	{0x1BC63090, 0x0000652F, 0x00006550, 0x0000032C, 0x00400303, 0x01105455, 0x130080FF, 0xB8004624}, 
	/* Clock setting profile of aac play power saving mode */
	{0x1B003090, 0x0000652F, 0x00006550, 0x00000320, 0x00400F03, 0x0110AAAA, 0x130080FF, 0xB8004624}, 
	/* Clock setting profile of aac plus play operation mode */
	{0x1BC63090, 0x0000652F, 0x00006550, 0x0000032C, 0x00400303, 0x00105455, 0x130080FF, 0xB8004624}, 
	/* Clock setting profile of aac plus play power saving mode */
	{0x1B003090, 0x0000652F, 0x00006550, 0x00000320, 0x00400F03, 0x0010AAAA, 0x130080FF, 0xB8004624}, 
	/* Clock setting profile of enhanced aac plus play operation mode */
	{0x1BC63090, 0x0000652F, 0x00006550, 0x0000032C, 0x00400303, 0x00105455, 0x130080FF, 0xB8004624}, 
	/* Clock setting profile of enhanced aac plus play power saving mode */
	{0x1B003090, 0x0000652F, 0x00006550, 0x00000320, 0x00400F03, 0x0010AAAA, 0x130080FF, 0xB8004624}, 
	/* Clock setting profile of wma play operation mode */
	{0x1BC63090, 0x0000652F, 0x00006550, 0x0000032C, 0x00400303, 0x01105455, 0x130080FF, 0xB8004624}, 
	/* Clock setting profile of wma play power saving mode */
	{0x1B003090, 0x0000652F, 0x00006550, 0x00000320, 0x00400F03, 0x0110AAAA, 0x130080FF, 0xB8004624}, 
	/* Clock setting profile of mp4 play low operation speed mode */
	{0x1BC63490, 0x0000652F, 0x00004212, 0x0000032C, 0x00400303, 0x0010A9AA, 0x130080FF, 0xB8004624},
	/* Clock setting profile of mp4 play high operation speed mode */
	{0x1BC63490, 0x0000652F, 0x0000442D, 0x0000032C, 0x00400303, 0x0010A9AA, 0x130080FF, 0xF8006948}, 
	/* Clock setting profile of 3gp rec low operation speed mode */
	{0x3BC734D0, 0x0000652F, 0x0000632D, 0x000003EC, 0x04400303, 0x00105455, 0x130080FF, 0xB8004624}, 
	/* Clock setting profile of 3gp rec high operation speed mode */
	{0x3BC734D0, 0x0000652F, 0x0000442D, 0x000003EC, 0x04400503, 0x00105455, 0x130080FF, 0xF8006948},
	/* Clock setting profile of image viewer low operation mode */
	{0x1BC63090, 0x0000652F, 0x0000442D, 0x0000032C, 0x00F00503, 0x11105555, 0x130080FF, 0xF8006948}, 
	/* Clock setting profile of image viewer high operation mode */
	{0x1BC63090, 0x0000652F, 0x0000442D, 0x0000032C, 0x00400503, 0x00105555, 0x130080FF, 0xF8006948}, 
	/* Clock setting profile of JPEG capture operation speed mode */
	{0x1BC63090, 0x0000652F, 0x00004212, 0x000003CC, 0x04F00303, 0x0110A9AA, 0x130080FF, 0xB8002424}, 

};


static void DELAY(int timeout)
{
	/* NOTE: We assue the timer 0 is ready  */
	int init_cnt, timer_init;
	int cur_cnt;
	int max_cnt;

	init_cnt = inpw(REG_TDR0);
	timer_init = inpw(REG_TICR0);
	
	max_cnt = 0;
	do
	{
		
		cur_cnt = inpw(REG_TDR0);
		if(cur_cnt > init_cnt)
			cur_cnt = init_cnt + (timer_init - cur_cnt);
		else
			cur_cnt = init_cnt - cur_cnt;
		
		/* NOTE: the timeout value must be smaller than init_cnt */
		if(cur_cnt >= timeout)
			break;

	}while(max_cnt++ < MAX_DELAY_LOOPS);
	
	dbg_printf("current system speed:%4d loops/us\n",max_cnt);

}


#if 0
static void pwr_disable_vpost(void)
{
	int cnt;
    if ((readw(REG_LCM_DCCS)&0x00000002)==0x00000002)
    {
		outpw(REG_LCM_DCCS, inpw(REG_LCM_DCCS) & ~(1<<3));
		cnt = 50000;
		while( (inpw(REG_LCM_DCCS)&(1<<25)) && cnt--);
		
        writew(REG_LCM_DCCS,readw(REG_LCM_DCCS)&0xfffffff7); //display_out-disable
    }
}
#endif

void pwr_set_clk(w99702_clk_t *cfg)
{
	unsigned int reg,bak,bak_audio, bak_mp4dec;
	unsigned int clkcon, clksel, clkdiv0;
	volatile int delay;
	
MEMCLEAN();
	
MEMPRINT(1);		

	/* if UPLL is enabled, switch the PLL to APLL first */
	bak = 0; bak_audio = 0; bak_mp4dec = 0;
	reg = inpw(REG_CLKSEL);
	
	
	/* Set the audio clock */	
	clkcon = (cfg->clkcon & ((1UL<<30))) | (inpw(REG_CLKCON)&(~(1<<30)));
	clksel = (cfg->clksel&(0x30UL)) | (inpw(REG_CLKSEL)&(~0x30UL));
	clkdiv0 = (cfg->clkdiv0 & 0x00F00000UL) | (inpw(REG_CLKDIV0) & 0xFF0FFFFFUL);
	outpw(REG_CLKCON, inpw(REG_CLKCON) & (~(1UL<<30)) );
	DELAY(DELAY_LOOPS);
	outpw(REG_CLKSEL, clksel);
	outpw(REG_CLKDIV0, clkdiv0);
	outpw(REG_APLLCON, cfg->apllcon);
	outpw(REG_CLKCON, clkcon);
	DELAY(DELAY_LOOPS);
	
	
	/* Reserve the audio clock */	
	if(inpw(REG_APLLCON)!=0xE220)
	{
		cfg->apllcon = inpw(REG_APLLCON);
		cfg->clkdiv0 = (cfg->clkdiv0 & 0xFF0FFFFFUL) | (inpw(REG_CLKDIV0) & 0x00F00000UL);
	}




MEMPRINT(2);           
	/* Set the USB clock */
	clkcon = (cfg->clkcon & ((1UL<<26))) | (inpw(REG_CLKCON)&(~(1<<26)));
	clksel = (cfg->clksel&(0x3UL)) | (inpw(REG_CLKSEL)&(~0x3UL));
	clkdiv0 = (cfg->clkdiv0 & 0x000F0000UL) | (inpw(REG_CLKDIV0) & 0xFFF0FFFFUL);
	outpw(REG_CLKCON, inpw(REG_CLKCON) & (~(1UL<<26)) );
	DELAY(DELAY_LOOPS);
	outpw(REG_CLKSEL, clksel);
	outpw(REG_CLKDIV0, clkdiv0);
	outpw(REG_CLKCON, clkcon);
	DELAY(DELAY_LOOPS);

MEMPRINT(2);		
	
	/* Reserve the USB clock */
	cfg->clkcon = (cfg->clkcon & (~(1UL<<26))) | (inpw(REG_CLKCON)&(1<<26));
	cfg->clksel = (cfg->clksel&(~0x3UL)) | (inpw(REG_CLKSEL)&0x3);
	cfg->clkdiv0 = (cfg->clkdiv0 & 0xFFF0FFFFUL) | (inpw(REG_CLKDIV0) & 0x000F0000UL);

MEMPRINT(3);		
	
	if( 	inpw(REG_UPLLCON) == cfg->upllcon && 
			(inpw(REG_CLKDIV0)&0xF0000000UL) == (cfg->clkdiv0&0xF0000000UL) &&
			(inpw(REG_CLKDIV1)&0xF0000000UL) == (cfg->clkdiv1&0xF0000000UL) )
	{
		/* Just change dividor when UPLL & HCLK is not changed */

MEMPRINT(4);		
			
		/* select the clock source */
		if(inpw(REG_CLKSEL) != cfg->clksel)
		{
			outpw(REG_CLKSEL, cfg->clksel);
			DELAY(DELAY_LOOPS);
		}

MEMPRINT(5);		
		
		clkcon = inpw(REG_CLKCON);	
		if(clkcon != cfg->clkcon)
		{
			/* Enable IP's step by step */		
			clkcon |= (cfg->clkcon&0xF0000000);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
			clkcon |= (cfg->clkcon&0x00F00000);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
			clkcon |= (cfg->clkcon&0x000F0000);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
			clkcon |= (cfg->clkcon&0x00000F00);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
		    clkcon |= (cfg->clkcon&0x000000F0);
		    outpw(REG_CLKCON, clkcon);
		    DELAY(DELAY_LOOPS);
		    clkcon |= (cfg->clkcon&0x0000000F);
		    outpw(REG_CLKCON, clkcon);
		    DELAY(DELAY_LOOPS);
		}

MEMPRINT(6);		
		
		/* setting the dividor */		
		if(inpw(REG_CLKDIV0) != cfg->clkdiv0)
		{
			outpw(REG_CLKDIV0, cfg->clkdiv0);
			DELAY(DELAY_LOOPS);
		}

MEMPRINT(7);		


		if(inpw(REG_CLKDIV1) != cfg->clkdiv1)
		{
			outpw(REG_CLKDIV1, cfg->clkdiv1);
			DELAY(DELAY_LOOPS);
		}

MEMPRINT(8);		

		/* setting SDRAM control */
		if(inpw(REG_SDICON) != cfg->sdcon)
		{
			outpw(REG_SDICON, cfg->sdcon);
			DELAY(DELAY_LOOPS);
		}

MEMPRINT(9);		

		/* setting SDRAM timing */
		if(inpw(REG_SDTIME0) != cfg->sdtime0)
		{
			outpw(REG_SDTIME0, cfg->sdtime0);
			DELAY(DELAY_LOOPS);
		}
		
MEMPRINT(10);		
		
	}
	else if( inpw(REG_UPLLCON) == cfg->upllcon )
	{
		/* change dividor and SDRAM timing when only UPLL fixed */
		/* Force the SDRAM timing to slowest */
		outpw(REG_SDTIME0, 0xF8006948);
		DELAY(DELAY_LOOPS);

MEMPRINT(11);		
		
		/* select the clock source */
		if(inpw(REG_CLKSEL) != cfg->clksel)
		{
			outpw(REG_CLKSEL, cfg->clksel);
			DELAY(DELAY_LOOPS);
		}
		
MEMPRINT(12);
		
		clkcon = inpw(REG_CLKCON);
		if( clkcon != cfg->clkcon)
		{
			/* Enable IP's step by step */		
			clkcon |= (cfg->clkcon&0xF0000000);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
			clkcon |= (cfg->clkcon&0x00F00000);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
			clkcon |= (cfg->clkcon&0x000F0000);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
			clkcon |= (cfg->clkcon&0x00000F00);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
			clkcon |= (cfg->clkcon&0x000000F0);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
			clkcon |= (cfg->clkcon&0x0000000F);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
		}
		
MEMPRINT(13);		
		
		/* setting the dividor */		
		if(inpw(REG_CLKDIV0) != cfg->clkdiv0)
		{
			outpw(REG_CLKDIV0, cfg->clkdiv0);
			DELAY(DELAY_LOOPS);
		}
		
MEMPRINT(14);
		
		if(inpw(REG_CLKDIV1) != cfg->clkdiv1)
		{
			outpw(REG_CLKDIV1, cfg->clkdiv1);
			DELAY(DELAY_LOOPS);
		}
		
MEMPRINT(15);		
		
		/* setting SDRAM control */
		if(inpw(REG_SDICON) != cfg->sdcon)
		{
			outpw(REG_SDICON, cfg->sdcon);
			DELAY(DELAY_LOOPS);
		}
		
MEMPRINT(16);		
		
		/* setting SDRAM timing */
		if(inpw(REG_SDTIME0) != cfg->sdtime0)
		{
			outpw(REG_SDTIME0, cfg->sdtime0);
			DELAY(DELAY_LOOPS);
		}
		
MEMPRINT(17);		
	
	}
	else
	{
		/* Force the SDRAM timing to slowest */
		outpw(REG_SDTIME0, 0xF8006948);
		DELAY(DELAY_LOOPS);

MEMPRINT(18);

		/* close some IP to make system stable */
		clkcon = cfg->clkcon & 0x1F07F0FF;
		outpw(REG_CLKCON , clkcon);
		
MEMPRINT(19);		
		/* Reserve the audio clock */	
		if(inpw(REG_APLLCON)==0xE220)
		{
			outpw(REG_APLLCON, cfg->apllcon);
			DELAY(DELAY_LOOPS);
		}

		/* force the clock source to crystal for futur clock change */
		outpw(REG_CLKSEL , 0x0000022C);
		/* Keep Video clock & system clock as high as possible (clkdiv = 0) */
		//outpw(REG_CLKDIV0, inpw(REG_CLKDIV0)&0xFFFF00FF);
		//outpw(REG_CLKDIV1, inpw(REG_CLKDIV1)&0x0000FFFF);
		
		/* change the UPLL first, and wait for stable */
		outpw(REG_UPLLCON, cfg->upllcon);
		DELAY(DELAY_LOOPS);


MEMPRINT(25);		
		
		/* setting the dividor */		
		outpw(REG_CLKDIV0, cfg->clkdiv0);
		outpw(REG_CLKDIV1, cfg->clkdiv1);


MEMPRINT(20);
		
		/* select the clock source to UPLL */
		outpw(REG_CLKSEL, cfg->clksel);
		DELAY(DELAY_LOOPS);
		
MEMPRINT(21);
		
		/* Enable IP's step by step */		
		clkcon |= (cfg->clkcon&0xE0000000);
		outpw(REG_CLKCON, clkcon);
		DELAY(DELAY_LOOPS);
		
MEMPRINT(22);
		
		clkcon |= (cfg->clkcon&0x00F00000);
		outpw(REG_CLKCON, clkcon);
		DELAY(DELAY_LOOPS);
		
MEMPRINT(23);
		
		clkcon |= (cfg->clkcon&0x000F0000);
		outpw(REG_CLKCON, clkcon);
		DELAY(DELAY_LOOPS);
		
MEMPRINT(24);		
		
		clkcon |= (cfg->clkcon&0x00000F00);
		outpw(REG_CLKCON, clkcon);
		DELAY(DELAY_LOOPS);

		
		
MEMPRINT(26);
		
		/* setting SDRAM control */
		outpw(REG_SDICON, cfg->sdcon);
		DELAY(DELAY_LOOPS);
		
MEMPRINT(27);
		
		/* setting SDRAM timing */
		outpw(REG_SDTIME0, cfg->sdtime0);
		DELAY(DELAY_LOOPS);
		
MEMPRINT(28);		
	}	
	
}

#if 0

void pwr_set_clk2(w99702_clk_t *cfg)
{
	unsigned int reg,bak,bak_audio, bak_mp4dec;

	/* if UPLL is enabled, switch the PLL to APLL first */
	bak = 0; bak_audio = 0; bak_mp4dec = 0;
	reg = inpw(REG_CLKSEL);
	
	/* Reserve the audio clock */	
	if(inpw(REG_APLLCON)!=0xE220)
	{
		cfg->apllcon = inpw(REG_APLLCON);
		cfg->clkdiv0 = (cfg->clkdiv0 & 0xFF0FFFFFUL) | (inpw(REG_CLKDIV0) & 0x00F00000UL);
	}
	
	/* Reserve the USB clock */
	cfg->clkcon = (cfg->clkcon & (~(1UL<<26))) | (inpw(REG_CLKCON)&(1<<26));
	cfg->clksel = (cfg->clksel&(~0x3UL)) | (inpw(REG_CLKSEL)&0x3);
	cfg->clkdiv0 = (cfg->clkdiv0 & 0xFFF0FFFFUL) | (inpw(REG_CLKDIV0) & 0x000F0000UL);
	
	if( (reg & 0x300) == 0x300 && (cfg->upllcon!=inpw(REG_UPLLCON)))
	{
		/* close Audio */
		//reg = inpw(REG_CLKCON);
		//bak_audio = reg & 0x10000000UL;
		//reg = reg & (~0x10000000UL);
		//outpw(REG_CLKCON, reg);
		
		/* close MP4 dec */
		//reg = inpw(REG_CLKCON);
		//bak_mp4dec = reg & 0x400UL;
		//reg = reg & (~0x400UL);
		//outpw(REG_CLKCON, reg);
		
		/* backup APLLCON setting */		
		bak = inpw(REG_APLLCON); 
		
		/* UPLL setting -> APLL setting */
		reg = inpw(REG_UPLLCON);
		outpw(REG_APLLCON, reg);

		//delay = 0x400;	while(delay--);
					  
		/* switch system clock to APLL */			   	
		reg = inpw(REG_CLKSEL);
		reg = reg & (~0x300);
		reg = reg | (0x200);
		outpw(REG_CLKSEL, reg);
									   	
		//delay = 0x400;	while(delay--);
	}				   

				   
	asm_set_clk(cfg);
	
	if(bak)
	{
		volatile int delay;
		/* Restore the APLL setting */						
		outpw(REG_APLLCON, bak);

		/* restore mp4 clock setting */
		//reg = inpw(REG_CLKCON);
		//reg = reg | bak_mp4dec;
		//outpw(REG_CLKCON, reg);
						
		/* restore Audio clock setting */
		//reg = inpw(REG_CLKCON);
		//reg = reg | bak_audio;
		//outpw(REG_CLKCON, reg);
		delay = 0x1000;	while(delay--);
	}
	
	//delay = 0x400;	while(delay--);
	
}

#endif

void pwr_set_clk_profile(int profile)
{
	int i, ticks;
	int old;
	
	if(profile > sizeof(clk_profile)/sizeof(w99702_clk_t)) return;	
	
	cyg_scheduler_lock();
	//HAL_DISABLE_INTERRUPTS(old);
	sysDisableCache();
	sysInvalidCache();
	sysFlushCache(I_D_CACHE);
	sysEnableCache(CACHE_WRITE_BACK);

	
	//outpw(0x3FFF001C, inpw(0x3FFF001C)|0xC);
	//outpw(0x7FF00108, (inpw(0x7FF00108)&(~3))|0x2);

	pwr_set_clk(&clk_profile[profile]);
	
	


	current_clk_profile = profile;	

	//HAL_RESTORE_INTERRUPTS(old);
	cyg_scheduler_unlock();
	
}

int pwr_get_current_clk_profile(void)
{
	return current_clk_profile;
}

/* this function return the next profile according to the pre-defined state-machine */
int get_next_clk_profile(void)
{
	switch(current_clk_profile)
	{
		case CLK_MENU_OP:
			return CLK_MENU_OP;
			break;
		case CLK_MENU_SAVE:
			return CLK_MENU_OP;
			break;
		case CLK_MP3_PLAY_OP:
			return CLK_MENU_OP;
			break;
		case CLK_MP3_PLAY_SAVE:
			return CLK_MP3_PLAY_OP;
			break;
		case CLK_MP4_PLAY_OP_LO:
		case CLK_MP4_PLAY_OP_HI:
			return CLK_MENU_OP;
			break;
		default:
			return CLK_MENU_OP;
	}
}

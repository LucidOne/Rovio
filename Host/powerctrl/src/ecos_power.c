#include "wbio.h"
#include "wblib.h"
#include "w99702_reg.h"
#include "702clk.h"
#include "../../VideoPhone/Inc/clk.h"
#ifdef ECOS
#include "cyg/kernel/kapi.h"
#include "cyg/infra/diag.h"
#endif



static void __pwr_delay(int count)
{
	int volatile uTick;
	for (uTick = 0; uTick < count; ++uTick);
}


void pwr_enable_mcu_irq()
{
	outpw(REG_GPIO_IE, (inpw(REG_GPIO_IE)|0x00000400));
}

void pwr_disable_mcu_irq()
{
	outpw(REG_GPIO_IE, (inpw(REG_GPIO_IE)&~0x00000400));
}



void pwr_enable_wifi_irq()
{
	outpw(REG_GPIO_IE, (inpw(REG_GPIO_IE)|0x00000010));
}

void pwr_disable_wifi_irq()
{
	outpw(REG_GPIO_IE, (inpw(REG_GPIO_IE)&~0x00000010));
}


void thread_join(cyg_handle_t* handle, cyg_thread* thread, cyg_thread_info* info);

static volatile BOOL	g_pwr_no_irq = TRUE;
static volatile BOOL	g_pwr_thread_killed = FALSE;
static cyg_handle_t		pwr_pd_handle = NULL;
static cyg_thread		pwr_pd_thread;
static cyg_thread_info	pwr_pd_info;
static UINT32			pwr_pd_stack[1024];
void pwr_system_power_down(cyg_addrword_t arg)
{
	volatile UINT loop;
	
	pwr_enable_mcu_irq();
	pwr_enable_wifi_irq();
		
	//------------------------------------------------------------------------------------------
	// Enable PowerDown mode
	//------------------------------------------------------------------------------------------
	outpw(REG_PWRCON, 0x04); 
	
	//------------------------------------------------------------------------------------------
	// !!! NOTE !!!
	// The loop must be a cached variable
	// The assignment of loop must be before entering SDRAM self-refresh mode  
	//------------------------------------------------------------------------------------------
	loop=10000;
	outpw(REG_SDICON, (inpw(REG_SDICON) | 0x20000000)); /* set SDRAM self-refrensh */

	//------------------------------------------------------------------------------------------
	// System will be blocked within somewhere the while loop and 
	// waiting for the wake-up source (interrupts)
	//------------------------------------------------------------------------------------------
	while(loop--);
	
	pwr_disable_mcu_irq();
	pwr_disable_wifi_irq();	
}


void pwr_stop_suspend_irq()
{
	g_pwr_no_irq = FALSE;

	if (pwr_pd_handle != NULL && !g_pwr_thread_killed)
	{
		cyg_thread_set_priority(pwr_pd_handle, 99);
		diag_printf("About to kill suspend thread\n");
		g_pwr_thread_killed = TRUE;
	}
}

void pwr_mcu_irq()
{
	diag_printf("In MCU irq\n");
	pwr_stop_suspend_irq();
	pwr_disable_mcu_irq();
}


void pwr_usb_irq()
{
	pwr_stop_suspend_irq();
}

void pwr_wifi_irq()
{
	pwr_stop_suspend_irq();
	pwr_disable_wifi_irq();
}



void pwr_system_power_saving()
{

	UINT32 uAPLL, uUPLL, uCLKSEL, uCLKCON;
	UINT32 uAIC_IMR;


	//------------------------------------------------------------------------------------------
    // Store the APLL and UPLL setting
	//------------------------------------------------------------------------------------------
	uAPLL = inpw( REG_APLLCON );
	uUPLL = inpw( REG_UPLLCON );
	uCLKSEL = inpw( REG_CLKSEL );
	uCLKCON = inpw( REG_CLKCON );
	

	//------------------------------------------------------------------------------------------
    // Disable local interrupt
	//------------------------------------------------------------------------------------------	
	//cyg_interrupt_disable();
	//------------------------------------------------------------------------------------------
    // Backup AIC mask state
	//------------------------------------------------------------------------------------------
	uAIC_IMR = inpw( REG_AIC_IMR );
	//------------------------------------------------------------------------------------------
    // Disable All interrupts except USB & FMI
	//------------------------------------------------------------------------------------------
	outpw( REG_AIC_MDCR, 0x003FFFFE ); /* disable AIC bit 26 for system wake up interrupt */
	outpw( REG_AIC_MECR, IRQ_USB | IRQ_FMI );


	//------------------------------------------------------------------------------------------
    // XIN for SYSTEM CLOCK
	//------------------------------------------------------------------------------------------
	outpw( REG_CLKSEL, 0x100 );
	
	//------------------------------------------------------------------------------------------
	// set PLLs into power down mode
	//------------------------------------------------------------------------------------------
	outpw( REG_APLLCON, (uAPLL | 0x8000) );
	outpw( REG_UPLLCON, (uUPLL | 0x8000) );

	//------------------------------------------------------------------------------------------
    // Disable most engine's power
	//------------------------------------------------------------------------------------------
	outpw( REG_CLKCON, (inpw( REG_CLKCON ) & ~(SECLK_EN | DSPCLK_EN | DACCLK_EN | ACLK_EN)));
	__pwr_delay(800);
	TURN_OFF_FMI_CLOCK;
	__pwr_delay(800);


#if 0
//******如果wakeup的source是?HIC bus?，才需要enable HIC auto-wakeup function//	
	//------------------------------------------------------------------------------------------
    // HIC is also allowed to be one of the wake-up sources.
	//------------------------------------------------------------------------------------------
	if( bHicAsWakeupSource )
	{
    	// Enable the interrupt of HIC as the wake-up source
    	//outpw(REG_HICFCR, (inpw(REG_HICFCR) | bAWAKEEN)); /* enable HIC auto-wakeup function */	
    	outpw(REG_HICFCR, (inpw(REG_HICFCR) | 0x4000)); /* enable HIC auto-wakeup function */	
	}
//******如果wakeup的source是?HIC bus?，才需要enable HIC auto-wakeup function//
#endif
	
	pwr_disable_mcu_irq();
	pwr_disable_wifi_irq();
	
	if (g_pwr_no_irq)
	{
		pwr_pd_handle = NULL;
		g_pwr_thread_killed = FALSE;
		cyg_thread_create(
			0,
			pwr_system_power_down,
			0,
			"power_down",
			pwr_pd_stack,
			sizeof(pwr_pd_stack),
			&pwr_pd_handle,
			&pwr_pd_thread);
	
		if (pwr_pd_handle != NULL)
		{
			cyg_thread_resume(pwr_pd_handle);
			while (!g_pwr_thread_killed);
			cyg_thread_kill (pwr_pd_handle);
			thread_join(&pwr_pd_handle, &pwr_pd_thread, &pwr_pd_info);
			pwr_pd_handle = NULL;
			g_pwr_thread_killed = FALSE;
		}
	}
	
	pwr_disable_mcu_irq();
	pwr_disable_wifi_irq();

	
	//------------------------------------------------------------------------------------------
    // Restore CLKCON
	//------------------------------------------------------------------------------------------
	outpw( REG_CLKCON, uCLKCON );
	
	
	//------------------------------------------------------------------------------------------
    // Restore APLL and UPLL   	
	//------------------------------------------------------------------------------------------
	outpw( REG_APLLCON, uAPLL );
	outpw( REG_UPLLCON, uUPLL );
	
	//------------------------------------------------------------------------------------------
    // Restore other Clock Settings 	
	//------------------------------------------------------------------------------------------
	outpw( REG_CLKSEL, uCLKSEL );

	//------------------------------------------------------------------------------------------
    // Restore interrupt
	//------------------------------------------------------------------------------------------
	outpw( REG_AIC_MECR, ( uAIC_IMR & 0x003FFFFE ) );
	//------------------------------------------------------------------------------------------
    // Enable local interrupt
	//------------------------------------------------------------------------------------------
	//cyg_interrupt_enable();

}




void wifiResetHostSleepFlag(void);
BOOL wifiIsHostSleep(void);

/* Force all threads into power saving mode */
void pwr_power_saving()
{
	BOOL bAudioEnabled;
	UINT32 uLED_GPIO;

	diag_printf("Set PS mode FALSE\n");
	
	
	pwr_disable_mcu_irq();
	pwr_disable_wifi_irq();
	g_pwr_no_irq = TRUE;
	pwr_enable_mcu_irq();
	
	SetWlanPSMode("wlan0", FALSE);
	SetWlanHostSleepCfg("wlan0", 5);
	
   	wifiSetHostSleepFlag(FALSE);
	pwr_enable_wifi_irq();


	
	//cyg_interrupt_disable(); while(1);
	//SetWlanDeepSleepMode("wlan0", TRUE);
	//vcptEnterPowerSaving();

	diag_printf("Before enter pwr_system_power_saving()\n");
	tt_msleep(800);

	if (wifiIsHostSleep())
	{
		diag_printf("Enter pwr_system_power_saving()\n");
		
		prdLock();
	
		bAudioEnabled = wb702LockAndDisableAudio();
		vcptEnterPowerSaving();

		nsSuspend();
		mcuLock();
		mcuSuspend();
	
		uLED_GPIO = ledSuspend();
		
		diag_printf("==>Enter pwr_system_power_saving()\n");
		pwr_system_power_saving();
		diag_printf("==>Enter pwr_system_power_saving()\n");

		ledWakeup(uLED_GPIO);
		
		mcuWakeup();
		mcuUnlock();
		nsWakeup();
		
		vcptLeavePowerSaving();
		wb702UnlockAndRestorePreviousAudio(bAudioEnabled);
		
		prdUnlock();
	
		diag_printf("Leave pwr_system_power_saving()\n");
	}
	//else
	//	tt_msleep(4500);
	
	SetWlanHostWakeUpCfg("wlan0");
	SetWlanPSMode("wlan0", TRUE);
	diag_printf("Set PS mode TRUE\n");

}



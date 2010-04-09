#include "stdio.h"
#include "stdlib.h"
#include "Cdefine.h"
#include "string.h"
#include "stddef.h"
#include "wb_syslib_addon.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "errno.h"
#include "C_list.h"
#include "wbtypes.h"
#include "sys/types.h"
#include "time.h"
#include "cyg/kernel/kapi.h"

#include "wblib.h"
#include "w99702_reg.h"
#include "vp_usb.h"
UINT32 volatile _fmi_SDPreState;


UINT8 volatile USB_Power_Flag;
extern UINT8 volatile USBModeFlag;


UINT8 MY_Read_GPIO7(void)
{
    outpw(REG_GPIO_OE,inpw(REG_GPIO_OE)|0x00000080);
    if ((inpw(REG_GPIO_STS)&0x00000080)==0x00000080)
        return 1;
    else 
        return 0;
}



void GPIO_ISR_USBPower(void)
{
	UINT32 volatile nLoop;
    USB_Power_Flag = MY_Read_GPIO7();
    for(nLoop=0; nLoop < 0x100000; nLoop++);//jhe modified 10000->100000 for videoplay check usb
    USB_Power_Flag = MY_Read_GPIO7();

    
    if (USB_Power_Flag==1)
    {
        outpw(REG_USB_CTL,(inpw(REG_USB_CTL)|0x00000001));//D+ high 
    }
    else
    {
        USBModeFlag=0;
        outpw(REG_USB_CTL,(inpw(REG_USB_CTL)&0xfffffffe));//D+ low
    }
}

static int volatile g_nGPIO_id;
cyg_uint32 GpioOtherInterruptPinManage(cyg_vector_t vector, cyg_addrword_t data)
{
 //GPIO_Int_ISR();
 //KeyPad_Int_ISR();
// add USB interrupt hander server;
#if 1
    UINT32 id;
    id = inpw(REG_GPIO_IS);
    g_nGPIO_id = id;
    
    diag_printf("GPIO:%08x\n", id);
    if (id&0x00000080)
        GPIO_ISR_USBPower();
   	if (id&0x00002000)
    	Reset_Int_ISR_MP4EVB_Board();
    outpw(REG_GPIO_IS,id);
#endif
   return 0;
}

void gpio_dsr_handle(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
	//usb has been removed, we should abord all the usbread and write
	//reset usb
//	diag_printf("Receive: GPIO %08x\n", g_nGPIO_id);
	if (g_nGPIO_id & 0x00000080)
	{
    	if (USBModeFlag == 0)
    	{
       		// since everytime usb unplug will cause a GPIO interrutp, I use this change to turn on usb
			// interrupt, and notify usbread to wakeup;
			USBExceptionHandle();
    	}
    	// clear this bits
    	g_nGPIO_id = g_nGPIO_id & (~0x00000080);
    	
    	
    	pwr_usb_irq();
    }
    
	if (g_nGPIO_id&0x00080000)
		Reset_Int_ISR_IPCam_Board();
	
	/* GPIO 4, WiFi wakeup
	   GPIO 10, MCU wakeup */
	if (g_nGPIO_id&0x00000010)
		pwr_wifi_irq();	
	
	if (g_nGPIO_id&0x00000400)
		pwr_mcu_irq();
		
   //diag_printf("GPIO---\n");

}

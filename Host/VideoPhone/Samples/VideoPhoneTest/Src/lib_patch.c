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
#include "../../Src/USBConfig/vp_gpio.h"

#include "wblib.h"
#include "w99702_reg.h"
#include "../../Src/USBConfig/vp_gpio.h"
#include "../../Src/USBConfig/vp_usb.h"
UINT32 volatile _fmi_SDPreState;


extern UINT8 volatile USB_Power_Flag;


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
    outpw(REG_GPIO_IS,0x00000080);
}


cyg_uint32 GpioOtherInterruptPinManage(cyg_vector_t vector, cyg_addrword_t data)
{
 //GPIO_Int_ISR();
 //KeyPad_Int_ISR();
// add USB interrupt hander server;
#if 1
    UINT32 id;
    id = inpw(REG_GPIO_IS);
    if (id&0x00000080)
        GPIO_ISR_USBPower();
#endif
   return 0;
}
 
void gpio_dsr_handle(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
}
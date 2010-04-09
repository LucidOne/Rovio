
#include "stdio.h"
#include "stdlib.h"
//#include "Cdefine.h"
#include "string.h"
#include "stddef.h"
#include "wb_syslib_addon.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "errno.h"
//#include "C_list.h"
#include "wbtypes.h"
#include "sys/types.h"
#include "time.h"
#include "cyg/kernel/kapi.h"

int volatile _fmi_SDPreState;

 
cyg_uint32 GpioOtherInterruptPinManage(cyg_vector_t vector, cyg_addrword_t data)
{
 //GPIO_Int_ISR();
 //KeyPad_Int_ISR();
 return 0;
}
 
void gpio_dsr_handle(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
}
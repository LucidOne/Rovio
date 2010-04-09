#include "../../Inc/Platform.h"


#define    GPIO_config         (*(unsigned int*) 0xfff83000)
#define    GPIO_dir               (*(unsigned int*) 0xfff83004) 
#define    GPIO_out_data      (*(unsigned int*) 0xfff83008) 
#define    GPIO_in_data        (*(unsigned int*) 0xfff8300c)


VOID sysResetLcd (VOID)
{
	/* Set LCD reset pin: High -> Low -> High.
		This step may be changed according to baseband. */
		
	GPIO_config |= 0x00000100; 	//Disable gpio 8
	
	GPIO_dir |= 0x00000100;		//set gpio 8 as output, others unchange.
	GPIO_out_data |= 0x00000100;					//gpio 8 output data high
	
	GPIO_config &= 0xFFFFFEFF;	//set gpio 8 enable

	GPIO_out_data |= 0x00000100;					//gpio 8 output data high
	sysMSleep(200);
	
	GPIO_out_data &= 0xfffffeff;					//gpio 8 output data low
	sysMSleep(200);

	GPIO_out_data |= 0x00000100;					//gpio 8 output data high
	sysMSleep(200);
	sysPrintf("Reset LCD\n");
}



VOID sysResetWbc (VOID)
{
	/* Set W99702 reset pin: High -> Low -> High.
		This step may be changed according to baseband. */
		
	GPIO_config |= 0x00000200; 	//Disable gpio
	
	GPIO_dir |= 0x00000200;		//set gpio 9 as output, others unchange.
	GPIO_out_data |= 0x00000200;					//gpio 9 output data high
	
	GPIO_config &= 0xFFFFFDFF;	//set gpio 9 enable

	GPIO_out_data |= 0x00000200;					//gpio 9 output data high
	sysMSleep(200);
	
	GPIO_out_data &= 0xFFFFFDFF;					//gpio 9 output data low
	sysMSleep(200);

	GPIO_out_data |= 0x00000200;					//gpio 9 output data high
	sysMSleep(200);
	sysPrintf("Reset W99702\n");
}





#define PSR_MODE_MASK	0x1F
#define PSR_MODE_IRQ	0x12
static __inline BOOL WB_IsInIRQ (void)
{
	int tmp;
	__asm__ __volatile__(
	" MRS %0, CPSR	\n"
	: "=r"(tmp)
	);
	tmp &= PSR_MODE_MASK;
	
	if (tmp == PSR_MODE_IRQ)
		return TRUE;
	else
		return FALSE;
}


VOID sysLockWbc (VOID)
{
	if (!WB_IsInIRQ ())
		sysDisableWbcIsr ();
}


VOID sysUnlockWbc (VOID)
{
	if (!WB_IsInIRQ ())
		sysEnableWbcIsr ();
}



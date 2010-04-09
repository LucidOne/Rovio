#include "../Inc/CommonDef.h"

cyg_handle_t	int_handle_reset;
cyg_interrupt	int_holder_reset;
static cyg_tick_count_t	g_StartTime;
static cyg_tick_count_t	g_EndTime;
#define KEY_PRESSED		0
#define KEY_RELEASED	1
static INT g_KeyStatus = KEY_RELEASED;

void Reset_Int_ISR_IPCam_Board(void)
{
	UINT32 n8051_CMD = inpw(REG_HICBUF);
	if (n8051_CMD == 0x01)
	{/* Shutdown */
		diag_printf("ACK 8051: %08x\n", n8051_CMD);
	
		/* ACK */
		diag_printf("Write 0x7ff01104 = 0x05\n");
		outpw(REG_HICBUF + 0x4, 0x05);	/* HICBUF1 ACK */
		diag_printf("GPIO 3 low\n");
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & 0xFFFFFFF7);		/* Set GPIO3 output */
		outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & 0xFFFFFFF7);	/* Set GPIO3 low */
		{ int volatile i; for (i = 0; i < 50000; i++); }
		diag_printf("GPIO 3 high\n");
		outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x00000008);	/* Set GPIO3 high */
		{ int volatile i; for (i = 0; i < 50000; i++); }


		/* Shutdown */
		diag_printf("Write 0x7ff01104 = 0x01\n");
		outpw(REG_HICBUF + 0x4, 0x01);	/* HICBUF1 0x01 */
		diag_printf("GPIO 3 low\n");
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & 0xFFFFFFF7);		/* Set GPIO3 output */
		outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & 0xFFFFFFF7);	/* Set GPIO3 low */
		{ int volatile i; for (i = 0; i < 50000; i++); }
		diag_printf("GPIO 3 high\n");
		outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x00000008);	/* Set GPIO3 high */
		{ int volatile i; for (i = 0; i < 50000; i++); }
	}
	else if (n8051_CMD == 0x03)
	{/* Reboot */
		diag_printf("Write 0x7ff01104 = 0x02\n");
		outpw(REG_HICBUF + 0x4, 0x02);	/* HICBUF1 0x02 */
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & 0xFFFFFFF7);		/* Set GPIO3 output */
		outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & 0xFFFFFFF7);	/* Set GPIO3 low */
		{ int volatile i; for (i = 0; i < 50000; i++); }
		outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x00000008);	/* Set GPIO3 high */
		{ int volatile i; for (i = 0; i < 50000; i++); }		
	}
}



void Reset_Int_ISR_MP4EVB_Board(void)
{
#if defined IPCAM_CONFIG_MP4_EVB_VER_0
/*
	UINT32 code1, code2;
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT)&~0x100);
	code1 = inpw(REG_GPIO_STS);

	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT)|0x100);
	code2 = inpw(REG_GPIO_STS);
	diag_printf("state: %08x-%08x\n", code1, code2);
	if ((((code1 >> 13) & 0x1) != 0) && (((code2 >> 13) & 0x1) != 0)) //GPIO13
	{
			 outpw(REG_GPIO_IS, inpw(REG_GPIO_IS) &(0x2000) );
			 return;
	}
*/	
	//cyg_interrupt_disable();
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
			if((g_EndTime - g_StartTime) >= 500) //Reset time is 5s
			{
				diag_printf("Set factory default\n");	
#if 0
				if (ReadFactoryDefault(&g_ConfigParam))
				{
					__WriteFlashMemory(&g_ConfigParam, FALSE, FALSE);
					diag_printf("Set factory default OK\n");		
				}
				else
					diag_printf("Set factory default error\n");	
#else
				//InitDefaultParam(&g_ConfigParam);			
				//WriteFlashMemory(&g_ConfigParam);
				g_ConfigParam.ulCheckSum = GetConfigCheckSum(&g_ConfigParam) + 1;
				__WriteFlashMemory(&g_ConfigParam, FALSE, FALSE);
						
				WebCameraSIGTERM(0);
				W99802Reboot();
#endif	
			}
			break;
		default:
			;		
	}
	outpw(REG_GPIO_IS, inpw(REG_GPIO_IS) &(0x2000) );
//	cyg_interrupt_enable();
#endif
}	
	
cyg_uint32 Reset_INT_Handler(cyg_vector_t vector, cyg_addrword_t data)
{
   /* clear GPIO interrupt state */
   //diag_printf("state: %08x\n", inpw(REG_GPIO_IS));
    Reset_Int_ISR_MP4EVB_Board();
	cyg_interrupt_acknowledge(vector);
    return CYG_ISR_HANDLED;
}

void InitResetGPIO(void)
{

	diag_printf("state: %08x\n", inpw(REG_GPIO_IS));
#if defined IPCAM_CONFIG_MP4_EVB_VER_0
	/* For old MP4-EVB board */
	outpw(REG_GPIO_IS, inpw(REG_GPIO_IS) &(0x2000) );
#elif defined IPCAM_CONFIG_IP_CAM_VER_0 || defined IPCAM_CONFIG_IP_CAM_VER_1 || defined IPCAM_CONFIG_IP_CAM_VER_2 || defined IPCAM_CONFIG_IP_CAM_VER_3 || defined IPCAM_CONFIG_MP4_EVB_VER_1
#else
#	error "No hardware config defined!"
#endif	

	
	/* For new IPCam board */
	outpw(REG_GPIO_IS, inpw(REG_GPIO_IS) &(0x80000) );

	diag_printf("state: %08x\n", inpw(REG_GPIO_IS));

	/* Install interrupt handler 
    cyg_interrupt_create(IRQ_GPIO, 1, 0, Reset_INT_Handler, NULL, &int_handle_reset, &int_holder_reset);
    cyg_interrupt_attach(int_handle_reset);
    cyg_interrupt_unmask(IRQ_GPIO);
    cyg_interrupt_enable();*/
    
#if defined IPCAM_CONFIG_MP4_EVB_VER_0
    outpw(REG_GPIO_OE,inpw(REG_GPIO_OE)|0x00002000);//GPIO 13 input;
	cyg_thread_delay(1);
#elif defined IPCAM_CONFIG_IP_CAM_VER_0 || defined IPCAM_CONFIG_IP_CAM_VER_1 || defined IPCAM_CONFIG_IP_CAM_VER_2 || defined IPCAM_CONFIG_IP_CAM_VER_3 || defined IPCAM_CONFIG_MP4_EVB_VER_1
#else
#	error "No hardware config defined!"
#endif

    outpw(REG_GPIO_OE,inpw(REG_GPIO_OE)&(~0x00000200));//9  output;
	cyg_thread_delay(1);
    outpw(REG_GPIO_DAT,inpw(REG_GPIO_DAT)&(~0x00000200)); // 9 low
	cyg_thread_delay(1);

    outpw(REG_GPIO_IE,inpw(REG_GPIO_IE)&(~0x00000200));	//disable GPIO 9 interrupt
	cyg_thread_delay(1);
	
#if defined IPCAM_CONFIG_MP4_EVB_VER_0
    outpw(REG_GPIO_IE,inpw(REG_GPIO_IE)|0x00002000);	//enable GPIO 13 interrupt
#elif defined IPCAM_CONFIG_IP_CAM_VER_0 || defined IPCAM_CONFIG_IP_CAM_VER_1 || defined IPCAM_CONFIG_IP_CAM_VER_2 || defined IPCAM_CONFIG_IP_CAM_VER_3 || defined IPCAM_CONFIG_MP4_EVB_VER_1
    outpw(REG_GPIO_IE,inpw(REG_GPIO_IE)&(~0x00002000));	//disable GPIO 13 interrupt
#else
#	error "No hardware config defined!"
#endif
	cyg_thread_delay(1);
	
    outpw(REG_GPIO_IE,inpw(REG_GPIO_IE)|0x00080000);
    cyg_thread_delay(1);
}

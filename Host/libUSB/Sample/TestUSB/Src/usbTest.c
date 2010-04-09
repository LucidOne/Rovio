#include "string.h"
#include "wblib.h"
#include "w99702_reg.h"
#include "cyg/kernel/kapi.h"
#include "cyg/infra/diag.h"

#include "stddef.h"
#include "wb_syslib_addon.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "errno.h"
#include "wbtypes.h"
#include "sys/types.h"
#include "time.h"
#include "cyg/kernel/kapi.h"

#include "wblib.h"
#include "w99702_reg.h"
#include "vp_usb.h"
#include "702clk.h"
#include "../Inc/clk.h"


#define _UNIT_TEST

#ifdef _UNIT_TEST
	#define VCOM_THREAD_STACK_SIZE 4*1024

	int read_thread_stack_1[ VCOM_THREAD_STACK_SIZE ];
	cyg_handle_t read_thread_handle_1;	
	cyg_thread read_thread_obj_1;
	
	int read_thread_stack_2[ VCOM_THREAD_STACK_SIZE ];
	cyg_handle_t read_thread_handle_2;	
	cyg_thread read_thread_obj_2;
	
	int read_thread_stack_3[ VCOM_THREAD_STACK_SIZE ];
	cyg_handle_t read_thread_handle_3;	
	cyg_thread read_thread_obj_3;
	
	int write_thread_stack_1[ VCOM_THREAD_STACK_SIZE ];
	cyg_handle_t write_thread_handle_1;	
	cyg_thread write_thread_obj_1;
	
	
	int write_thread_stack_2[ VCOM_THREAD_STACK_SIZE ];
	cyg_handle_t write_thread_handle_2;	
	cyg_thread write_thread_obj_2;
	
	
	int read_write_thread_stack[ VCOM_THREAD_STACK_SIZE ];
	cyg_handle_t read_write_thread_handle;	
	cyg_thread read_write_thread_obj;
	
	
	int write_read_thread_stack[ VCOM_THREAD_STACK_SIZE ];
	cyg_handle_t write_read_thread_handle;	
	cyg_thread write_read_thread_obj;	
	
	
static void test(void);
static void test1(void);
static void test2(void);
static void test3(void);
static void test4(void);
static void test5(void);
static void test6(void);
static void test7(void);
static void write_read_thread(cyg_addrword_t index) ;
static void read_write_thread(cyg_addrword_t index) ;
static void write_thread_1(cyg_addrword_t index);
static void write_thread_2(cyg_addrword_t index);
static void read_thread_1(cyg_addrword_t index);
static void read_thread_2(cyg_addrword_t index);
static void read_thread_3(cyg_addrword_t index);


	

	
	
#endif

static void GPIO_ISR_USBPower(void);
cyg_uint32 GPIOHandler(cyg_vector_t vector, cyg_addrword_t data) ;


static BOOL g_bDisableDebugMsg = FALSE;
UINT8 volatile USB_Power_Flag;
extern UINT8 volatile USBModeFlag;

static cyg_handle_t		int_handle_gpio;   
static cyg_interrupt	int_older_handle_gpio;


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

	/* <4> Set other engine clocks. */
	/* Set VPE and 2D clock. */
	TURN_ON_VPE_CLOCK;
	cyg_thread_delay(30);
	TURN_ON_GFX_CLOCK;
	cyg_thread_delay(30);


	
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

		size_t i;
	
		for ( i = 0; i < size; i++ )
			hal_if_diag_write_char( buf[i] );
}
static void W99802Reboot()
{
	unsigned int iValue;
	iValue = *((volatile unsigned int *)0x7FF00008);
	iValue |= 1;
	*((volatile unsigned int *)0x7FF00008) = iValue;
}


static void GPIOHandleDSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
	return;
}

void GPIOInit() 
{
    cyg_interrupt_disable();
    
    /* Usually GPIO interrupt is shareed by mang device, we cann't create the interrupt arbitrarely 
     * but we can used GpioOtherInterruptPinManage() to handle it
     */
    cyg_interrupt_create(IRQ_GPIO, 1, 0, &GPIOHandler,    
						 &GPIOHandleDSR, &int_handle_gpio,                  
						 &int_older_handle_gpio);                  
    cyg_interrupt_attach(int_handle_gpio);   
    cyg_interrupt_unmask(IRQ_GPIO);
    cyg_interrupt_enable();

}

cyg_uint32 GPIOHandler(cyg_vector_t vector, cyg_addrword_t data) 
{
    UINT32 id;
    //cyg_interrupt_mask(vector);
    id = inpw(REG_GPIO_IS);
    if (id&0x00000080)
        GPIO_ISR_USBPower();
    cyg_interrupt_acknowledge(vector);
   // cyg_interrupt_unmask(vector);
    return CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;

}





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





int main() {
	//
	unsigned int lenth = 1;
	char ch;
	char buff[1024];
	int counter = 0;
	char* id = "Winbond yanyaojie\r\nO";
	char* version = "1.2.3.4\r\nO";
	char* data = "0x12345678 0x34567800 0x56780000 0x78000000\r\n0x87654321 0x12345678 0x34567812 0x56781234\r\n";
	int i = 0;
	char reply[2];
	reply[0] = 'O';
	reply[1] = 0x0d;
	cyg_interrupt_disable();
	cyg_interrupt_disable();
	cyg_interrupt_disable();
	
	cyg_interrupt_enable();
	sysInit();
	//cyg_interrupt_mask(IRQ_TIMER0);
	GPIOInit();
	// initial usb for vcom
	USBInitForVCom();
	test();
}


#ifdef _UNIT_TEST
int usbrev(char* buff, int size)
{
	char head[4] = {0};
	int headlen = 4;
	int size_avail = 0;
	USBRead((PCHAR)head, (PUINT32)&headlen);
	size_avail = head[0] + (head[1] << 8) + (head[2] << 16) + (head[3] <<24);
	if ( size < size_avail)
	{
		size_avail = size;
	}
	USBRead((PCHAR)buff, (PUINT32)&size_avail);
	return size_avail;
}
int usbsnd(char* buff, int size)
{
	char head[4] = {0};
	int headlen = 4;
	char* tmp = buff;
	int package = 1023;
	int remain = size;
	
	head[0] = size & 0xff;
	head[1] = (size >> 8) & 0xff;
	head[2] = (size >> 16) & 0xff;
	head[3] = (size >> 24) & 0xff;
	USBWrite((PCHAR)head, (PUINT32)&headlen);
#if 0
	while ( remain > 0 )
	{
		if ( remain < package )
		{
			package = remain;
		}
		USBWrite((PCHAR)tmp, (PUINT32)&package); 
		tmp = tmp + package;
		remain = remain  - package;
	}
#endif
	USBWrite((PCHAR)buff, (PUINT32)&size);	
}

void busy_thread(cyg_addrword_t index)
{
	int times = 0;
	while(1) 
	{
		diag_printf("busy thread [%d] times\n", times++);
		cyg_thread_delay(10);
		
	}
	
}

void test_new_thread()
{
	cyg_thread_create(10, busy_thread,
 		0,     
		"test thread",      
		&read_thread_stack_1,      
		VCOM_THREAD_STACK_SIZE,      
		&read_thread_handle_1,      
		&read_thread_obj_1); 
	cyg_thread_resume(read_thread_handle_1);
}
char buff[1024*1024];
void test()
{
	int ret = 0;
	int i = 0;
	int times = 0;
	//test_new_thread();
	while(1)
	{
		ret = usbrev(buff, 1024*1024);
		diag_printf("This is the [%d] times, revceive [%d] chars\n", times++, ret);
		usbsnd(buff, ret);
	#if 0
		for ( i = 0; i < ret; i++)
		{
			if ( i%16 == 0 && i != 0 )
			{
				diag_printf("\n");
			}
			diag_printf("%4d",buff[i]);
		}
		diag_printf("\n");
	#endif
	}
}
/* this case test the boundary for USBRead() */
void test1(void) {
	char a[1024 + 1];
	int i =0;
	unsigned int len;
	int ret;
	len = 1024 - 1; //max lenth is 1024-1;
	printf("begine to test1\n");
	ret = USBRead(a, &len);
	if (ret == -1) {
		printf("Test1 faild\n");
		return;
	}
	a[1023] = 0;
	printf("Cleint has write 1023 charactors %s\n", a);
	len = 1024;
	ret = USBRead(a, &len);
	if (ret == -1) {
		printf("the number of read characyers more than 1023, not surport\n");
		//but we can call read many times to read longer string
		for(i =0; i<1024; i++ ) {
			len = 1;
			USBRead(&a[i], &len);
			if (i == 1022) {
				printf("this may be a bug\n");
				printf("this may be a bug\n");
				printf("this may be a bug\n");
				printf("this may be a bug\n");
			}
		}
		a[1024] = 0;
		printf("Client write 1024 chararters %s\n", a);
	}
	len = 0;
	ret = USBRead(a, &len);
	if (ret == -1) {
		printf("Invalid length\n");
	}
	
	len = -1;
	ret = USBRead(a, &len);
	if (ret == -1) {
		printf("Invalid length\n");
	}
	
	printf("test1 succesully\n");

}
/* this case tests USBWrite()
 * to test this case, you should run a client application in windows 
 */
void test2()
{
	char buff[1026];
	int i;
	int ret;
	unsigned int lenth = 1024;
	printf("begine to test2\n");
	for(i = 0; i < 1026; i++) {
		buff[i] = 'H';
	}
	ret = USBWrite(buff,&lenth);
	if (ret == -1) {
		printf("test2 faild \n");
		return ;
	}
	lenth = 1026;

	ret = USBWrite(buff,&lenth);
	if (ret == -1) {
		printf("The write lenth is too long, we can call USBWrite for many times \n");
		lenth =  1;
		for (i = 0; i <1026 ; i++ ) {
			USBWrite(&buff[i], &lenth);
		}
	}
	lenth = -1;
	ret = USBWrite(buff,&lenth);
	if (ret == -1) {
		printf("Invalid lenth \n");
	}
	
	
	lenth = 0;
	ret = USBWrite(buff,&lenth);
	if (ret == -1){
		printf("invalid lenth\n");
	}
	printf("test2 succesully\n");
	
	
	
}
/* this case tests USBPrintf()
 * open a HyperTerminal to test the case
 */
void test3()
{
	char buff[1026];
	int i = 0;
	printf("begine to test3\n");
	for(i = 0; i < 1026; i++) {
		buff[i] = 'H';
	}
	//write 1026 characters to device
	
	buff[1026] = 0;
	USBPrintf("%s",buff);
	
	//write 1024 characters to device
	buff[1024] = 0;
	USBPrintf("%s",buff);
	
	//write 1023 characters to device
	buff[1023] = 0;
	USBPrintf("%s",buff);
	printf("test3 sucessfully");
	
}
/* simultaneity read many thread 
 * to test this case, you should run a client application in windows 
 */
void test4(){
	
	/* create 3 read thread */
	printf("begine to test4\n");
	cyg_thread_create(10, read_thread_1,
 		0,     
		"read 1",      
		&read_thread_stack_1,      
		VCOM_THREAD_STACK_SIZE,      
		&read_thread_handle_1,      
		&read_thread_obj_1); 
		
		cyg_thread_create(10, read_thread_2,
 		0,     
		"read 2",      
		&read_thread_stack_2,      
		VCOM_THREAD_STACK_SIZE,      
		&read_thread_handle_2,      
		&read_thread_obj_2); 
		
		cyg_thread_create(10, read_thread_3,
 		0,     
		"read 3",      
		&read_thread_stack_3,      
		VCOM_THREAD_STACK_SIZE,      
		&read_thread_handle_3,      
		&read_thread_obj_3); 
		  
	
  cyg_thread_resume(read_thread_handle_1);
  cyg_thread_resume(read_thread_handle_2);      	    
  cyg_thread_resume(read_thread_handle_3);	

}

/*simultaneity many write thread 
 *  to test this case, you should run a client application in windows 
 */
void test5() {
		printf("begine to test5\n");
		/* create 2 write thread */
		cyg_thread_create(10, write_thread_1,
 		0,     
		"write 1",      
		&write_thread_stack_1,      
		VCOM_THREAD_STACK_SIZE,      
		&write_thread_handle_1,      
		&write_thread_obj_1); 
		
		cyg_thread_create(10, write_thread_2,
 		0,     
		"write 2",      
		&write_thread_stack_2,      
		VCOM_THREAD_STACK_SIZE,      
		&write_thread_handle_2,      
		&write_thread_obj_2); 
		
	  	cyg_thread_resume(write_thread_handle_1);
  		cyg_thread_resume(write_thread_handle_2); 
	

}
/* read and write simultaneitily 
 *  to test this case, you should run a client application in windows 
 */
void test6() {

		printf("begine to test6\n");
		cyg_thread_create(10, read_write_thread,
 		0,     
		"read_write",      
		&read_write_thread_stack,      
		VCOM_THREAD_STACK_SIZE,      
		&read_write_thread_handle,      
		&read_write_thread_obj); 
		
		cyg_thread_create(10, write_read_thread,
 		0,     
		"write_read",      
		&write_read_thread_stack,      
		VCOM_THREAD_STACK_SIZE,      
		&write_read_thread_handle,      
		&write_read_thread_obj); 
		
		cyg_thread_resume(read_write_thread_handle);
  		cyg_thread_resume(write_read_thread_handle); 


}
void read_thread_1(cyg_addrword_t index)
{
	unsigned int lenth = 2;
	char buff[10];
	
	while(1) {
		USBRead(buff, &lenth);
		buff[2] = 0;
		printf("Thread 1 read [%s]\n", buff);
		cyg_thread_delay(10);
		
	}
	
}

void read_thread_2(cyg_addrword_t index)
{
	unsigned int lenth = 2;
	char buff[10];
	
	while(1) {
		USBRead(buff, &lenth);
		buff[2] = 0;
		printf("Thread 2 read [%s]\n", buff);
		cyg_thread_delay(10);
		
	}
	
}

void read_thread_3(cyg_addrword_t index)
{
	unsigned int lenth = 2;
	char buff[10];
	
	while(1) {
		USBRead(buff, &lenth);
		buff[2] = 0;
		printf("Thread 3 read [%s]\n", buff);
		cyg_thread_delay(10);
		
	}
	
}


void write_thread_1(cyg_addrword_t index)
{

	while(1) {

		USBPrintf("this is thread 1 write: 11111111111111111111111\n");
		cyg_thread_delay(10);
		
	}
	
}

void write_thread_2(cyg_addrword_t index)
{
	
	while(1) {
		USBPrintf("this is thread 2 write: 22222222222222222222222\n");
		cyg_thread_delay(10);
		
	}
	
}

void read_write_thread(cyg_addrword_t index) 
{
	unsigned int lenth = 20;
	char buff[21];
	while(1) {
		USBRead(buff, &lenth);
		buff[20] = 0;
		printf("Read from client: %s\n", buff);
		
	}

}

void write_read_thread(cyg_addrword_t index) 
{
	unsigned int lenth = 13;
	char* buff = "Thanks Client";
	while(1) {
		USBWrite(buff, &lenth);
		cyg_thread_delay(100);
		
	}

}




#endif

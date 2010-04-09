#include "CommonDef.h"
#include "memmgmt.h"
#include "702clk.h"

#include "cyg\io\devtab.h"
#include "cyg\io\serial.h"

#if defined IPCAM_CONFIG_IP_CAM_VER_1 || defined IPCAM_CONFIG_IP_CAM_VER_2
#include "ns_api.h"
#endif

//----------------------------------------------------------------------------
// Lower priority number means higher priority

#define TEST_UART_RECEIVER_PRIORITY 6


//----------------------------------------------------------------------------
static void UartWriteTrigger(char uTestCase, char uValue)
{
	if(uTestCase == 1)
		outpw(REG_COM_TX, uValue);
	else
		outpw(REG_HS_COM_TX, uValue);
}
 

cyg_handle_t er_find_thread()
{
	cyg_uint16 id = 0;
	for (id = 0; id < 64; ++id)
	{
		cyg_handle_t handle = cyg_thread_find(id);
		cyg_thread_info info;
		if (handle && cyg_thread_get_info(handle, id, &info))
		{
			if (strcmp(info.name, "ns_nav") == 0)
				return handle;
		}
	}
	
	return NULL;
}

//cplu++
cyg_handle_t period_find_thread()
{
	cyg_uint16 id = 0;
	for (id = 0; id < 64; ++id)
	{
		cyg_handle_t handle = cyg_thread_find(id);
		cyg_thread_info info;
		if (handle && cyg_thread_get_info(handle, id, &info))
		{
			if (strcmp(info.name, "period") == 0)
				return handle;
		}
	}
	
	return NULL;
}

void er_pause_thread()
{
	cyg_handle_t ns_thread_handle = er_find_thread();
	if (ns_thread_handle)
		cyg_thread_suspend( ns_thread_handle );
}

//cplu++
void period_pause_thread()
{
	cyg_handle_t period_thread_handle = period_find_thread();
	if (period_thread_handle)
		cyg_thread_suspend( period_thread_handle );
}


void er_resume_thread()
{
	cyg_handle_t ns_thread_handle = er_find_thread();
	if (ns_thread_handle)
		cyg_thread_resume(ns_thread_handle);
}

//cplu++
void period_resume_thread()
{
	cyg_handle_t period_thread_handle = period_find_thread();
	if (period_thread_handle)
		cyg_thread_resume(period_thread_handle);
}


int tsUartTestHandler(ICTL_HANDLE_T* ctrlHandle,char ** valueList, int paraNum, char *pcResponse,size_t szMaxResponse) 
{
	char cNo = 0x90;
	char cUartTestCase = 0;
	volatile char cRx=0;
	char i=0;
	char j=0;
	char bRetryCnt=0; // Tim
	//int len=0;

	cyg_serial_info_t get_ns_serial_info;
	cyg_serial_info_t get_mcu_serial_info;
	Cyg_ErrNo res;
	
	//extern cyg_io_handle_t	ns_uart; 
	//extern cyg_io_handle_t	mcu_uart;
	cyg_io_handle_t	ns_uart; 
	cyg_io_handle_t	mcu_uart;
	//extern void er_pause_thread();
	//extern void er_resume_thread();

   	BOOL bUartSuppressed= uart_is_sys_msg_suppressed();

   	cyg_handle_t hCurrentThread = cyg_thread_self();
   	cyg_priority_t nOldPriority = cyg_thread_get_priority( hCurrentThread );



	static cyg_serial_info_t	ser_setup_19200 = {  CYGNUM_SERIAL_BAUD_19200,
										CYGNUM_SERIAL_STOP_1,  
										CYGNUM_SERIAL_PARITY_NONE,
										CYGNUM_SERIAL_WORD_LENGTH_8,
										0 };
	
	static cyg_serial_info_t	ser_setup_9600 = {  CYGNUM_SERIAL_BAUD_9600,
										CYGNUM_SERIAL_STOP_1,  
										CYGNUM_SERIAL_PARITY_NONE,
										CYGNUM_SERIAL_WORD_LENGTH_8,
										0 };
	
	static cyg_serial_info_t	ser_setup_1200 = {  CYGNUM_SERIAL_BAUD_1200,
										CYGNUM_SERIAL_STOP_1,  
										CYGNUM_SERIAL_PARITY_NONE,
										CYGNUM_SERIAL_WORD_LENGTH_8,
										0 };
	cyg_uint32			sers_len;

	// suspend NS thread	
	er_pause_thread();
	period_pause_thread();//cplu0421
	
//cplu---	cyg_thread_set_priority( hCurrentThread, TEST_UART_RECEIVER_PRIORITY );

    // lookup "dev/ser0" and "/dev/ser1"
    //#define NS_SERIAL_PORT           "/dev/ser0"
    //#define MCU_SERIAL_PORT          "/dev/ser1"
   	if( ENOERR!=cyg_io_lookup( "/dev/ser0", &mcu_uart ) )
   	{
       	strcpy(pcResponse, "FAIL (No Ser0)");
   	    return -1;
   	}        
   	if( ENOERR!=cyg_io_lookup( "/dev/ser1", &ns_uart ) )
   	{
       	strcpy(pcResponse, "FAIL (No Ser1)");
       	return -1;
   	}

   	

	// get ns_uart baud rate
   	sers_len = sizeof(get_ns_serial_info);
    	res = cyg_io_get_config(ns_uart, CYG_IO_GET_CONFIG_SERIAL_INFO, &get_ns_serial_info, &sers_len);

	// get mcu_uart baud rate
   	//len = sizeof(get_mcu_serial_info);
    //res = cyg_io_get_config(mcu_uart, CYG_IO_GET_CONFIG_SERIAL_INFO, &get_mcu_serial_info, &len);

	// set baudrate, parity and stuff like that
	//sers_len = sizeof(ser_setup_19200);
	//cyg_io_set_config( ns_uart, CYG_IO_SET_CONFIG_SERIAL_INFO, (void*)&ser_setup_19200, &sers_len );

	// set baudrate, parity and stuff like that
	if(get_ns_serial_info.baud==CYGNUM_SERIAL_BAUD_19200)
	{
		sers_len = sizeof(ser_setup_19200);
		cyg_io_set_config( mcu_uart, CYG_IO_SET_CONFIG_SERIAL_INFO, (void*)&ser_setup_19200, &sers_len );
	}
	else  if(get_ns_serial_info.baud==CYGNUM_SERIAL_BAUD_1200)
	{
		sers_len = sizeof(ser_setup_1200);
		cyg_io_set_config( mcu_uart, CYG_IO_SET_CONFIG_SERIAL_INFO, (void*)&ser_setup_1200, &sers_len );
	}
	else
	{
		sers_len = sizeof(ser_setup_1200);
		cyg_io_set_config( ns_uart, CYG_IO_SET_CONFIG_SERIAL_INFO, (void*)&ser_setup_1200, &sers_len );			
		cyg_io_set_config( mcu_uart, CYG_IO_SET_CONFIG_SERIAL_INFO, (void*)&ser_setup_1200, &sers_len );
	}

	uart_suppress_sys_msg(TRUE);

	if(strcmp( valueList[0], "1" )== 0)
		cUartTestCase = 1;
	else
		cUartTestCase = 0;		
	
repeat://Tim
	UartWriteTrigger(cUartTestCase, cNo);
	cyg_thread_delay(2);

	for(i=0;i<10;i++)
	{
		for(j=0;j<5;j++)
		{
			if(cUartTestCase==1)
				cRx = (char)inpw(REG_HS_COM_RX);
			else
				cRx = (char)inpw(REG_COM_RX);				
			 if(cRx==cNo)
			{
				cNo++;
				UartWriteTrigger(cUartTestCase, cNo);
				cyg_thread_delay(1);
				break;
			}
		}
	}

	//Tim start
	if(cNo==0x9A)
	{
		if(cUartTestCase==1)
		 	strcpy(pcResponse, "OK (Ser0->Ser1)");
		else
		 	strcpy(pcResponse, "OK (Ser1->Ser0)");
	}
	else
	{
		cNo = 0x90;
		bRetryCnt++;
		if(bRetryCnt<=5)
			goto repeat;
		
		if(cUartTestCase==1)
		 	strcpy(pcResponse, "FAIL (Ser0->Ser1)");
		else
		 	strcpy(pcResponse, "FAIL (Ser1->Ser0)");		
	}
		
	cNo = 0x90;
	//Tim end

// restore mcu baud rate
	sers_len = sizeof(ser_setup_9600);
	cyg_io_set_config( mcu_uart, CYG_IO_SET_CONFIG_SERIAL_INFO, (void*)&ser_setup_9600, &sers_len );
	
//cplu---	cyg_thread_set_priority( hCurrentThread, nOldPriority );
	er_resume_thread();
	period_resume_thread();
	
//cplu---	uart_suppress_sys_msg( bUartSuppressed );
	return -1;
}


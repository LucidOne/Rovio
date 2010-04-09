#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stddef.h"
#include "wb_syslib_addon.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "errno.h"
#include "wbtypes.h"
#include "sys/types.h"
#include "time.h"
#include "cyg/kernel/kapi.h"
#include "cyg/io/devtab.h"
#include "cyg/infra/diag.h"
#include "serialio.h"

void uart_init_setting_block_mode(const char *uart)
{
	cyg_io_handle_t uart_handle;
	if( cyg_io_lookup( uart, &uart_handle ) == ENOERR )
    {
    	cyg_uint32 len, blocking;
        len = sizeof(blocking);
		blocking = 1;					// indicates blocking reads
		cyg_io_set_config( uart_handle, 
                           CYG_IO_SET_CONFIG_SERIAL_READ_BLOCKING, 
                           (void*)&blocking, 
                           &len );

        len = sizeof(blocking);
		blocking = 1;					// indicates blocking reads
		cyg_io_set_config( uart_handle, 
                           CYG_IO_SET_CONFIG_SERIAL_WRITE_BLOCKING, 
                           (void*)&blocking, 
                           &len );
	}
}

int main(void)
{

	cyg_interrupt_disable();
	cyg_interrupt_disable();
	cyg_interrupt_disable();
	
	cyg_interrupt_enable();

 	uart_init_setting_block_mode("/dev/ser0");
 	uart_init_setting_block_mode("/dev/ser1");

	printf("Hello, world!\n");
	return 0;
	
}

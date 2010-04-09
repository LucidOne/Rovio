#include "CommonDef.h"
#include "memmgmt.h"
#include "702clk.h" 
#include "uart_wrapper.h"
#include "cyg/io/io.h"
#include "cyg/io/config_keys.h"

static BOOL g_bDisableDebugMsg = (CONFIG_USE_UART_MSG ? FALSE : TRUE);


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


void hi_uart_write( const char *buf, size_t size )
{
#if 1	//output to high speed uart
	int fd = open( "/dev/ser1", O_WRONLY );
	if (fd >= 0)
	{
		write(fd, buf, size);
		close(fd);
	}
#else
	size_t i;
	
	g_bDisableDebugMsg = TRUE;
	for ( i = 0; i < size; i++ )
		hal_if_diag_write_char( buf[i] );
#endif
}

int hi_uart_read( char *buf, size_t size )
{
#if 1	//input from high speed uart
	size_t i = 0;
	int fd = open( "/dev/ser1", O_RDONLY );
	if (fd >= 0)
	{
		fd_set read_set;
		FD_ZERO(&read_set);
			
		for (i = 0; i < size; ++i)
		{
			int rt = -1;
			struct timeval tv;
			FD_SET(fd, &read_set);
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			if (select(fd + 1, &read_set, NULL, NULL, &tv) == 1)
				rt = read(fd, &buf[i], 1);
			if (rt <= 0)
#if 1	//xhchen - MCU debug
			{
				void hi_uart_log_stop(void);
				hi_uart_log_stop();
				break;
			}
#else				
				break;
#endif
				
			FD_CLR(fd, &read_set);
		}
		close(fd);
	}
	return (i == 0 ? - 1: i);
#else
	size_t i;
	for ( i = 0; i < size; i++ )
		hal_if_diag_read_char( &buf[i] );
#endif
}

void uart_suppress_sys_msg( BOOL suppress )
{
	g_bDisableDebugMsg = suppress;
}

BOOL uart_is_sys_msg_suppressed( )
{
	return g_bDisableDebugMsg;
}


#if 0
void uart_read_raw( char *buf, size_t size )
{
	size_t i;
	for ( i = 0; i < size; i++ )
		hal_if_diag_read_char( &buf[i] );
}

void uart_write_raw( const char *buf, size_t size )
{
	size_t i;
	for ( i = 0; i < size; i++ )
		hal_if_diag_write_char( buf[i] );
}

void uart_write( const char *buf, size_t size )
{
	if (!g_bDisableDebugMsg && CONFIG_USE_UART_MSG)
	{
		size_t i;
	
		for ( i = 0; i < size; i++ )
			hal_if_diag_write_char( buf[i] );
	}

	//vcom_write(buf, size);
}
#endif


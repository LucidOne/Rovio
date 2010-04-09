#ifndef __UART_WRAPPER__
#define __UART_WRAPPER__

/* Set UART to block mode */
void uart_init_setting_block_mode(const char *uart);
/* Read hi-UART */
int hi_uart_read( char *buf, size_t size );
/* Write hi-UART */
void hi_uart_write( const char *buf, size_t size );

/* Enable or disable system message walk on through UART */
void uart_suppress_sys_msg( BOOL suppress );
/* Read UART */
void uart_read_raw( char *buf, size_t size );
/* Write UART */
void uart_write_raw( const char *buf, size_t size );

#endif


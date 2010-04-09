#include "stdlib.h"
#include "wbtypes.h"

/* Writes tty infor to virtual com */
void vcom_write(char *buf, size_t size)
{
}

/* Log read operation on HI speed UART */
char hi_uart_log_read(int blog, char ch)
{
	return ch;
}

/* Log write operation on HI speed UART */
char hi_uart_log_write(int blog, char ch)
{
	return ch;
}


/* Callbacks for eth driver receives eapol package. */
void eapol_on_drv_recv(const char *buf, int len)
{
}


/* Determine if UART message is supressed or not. */
BOOL uart_is_sys_msg_suppressed()
{
	return FALSE;
}



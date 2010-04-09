//==========================================================================
//
//      io/serial/arm/tcp_serial.c
//
//      WINBOND W90702 Serial I/O Interface Module (interrupt driven)
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    xhchen
// Contributors: nuvoTon
// Date:         7/7/2009
// Purpose:      TCP Serial I/O module (poll driven)
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include "pkgconf/system.h"
#include "pkgconf/io.h"
#include "pkgconf/io_serial.h"
#include "cyg/io/io.h"
#include "cyg/hal/hal_intr.h"
#include "cyg/io/devtab.h"
#include "cyg/io/serial.h"
#include "cyg/infra/diag.h"


#include "sys/stat.h"
#include "fcntl.h"
#include "errno.h"
#include "sys/types.h"
#include "time.h"
#include "cyg/kernel/kapi.h"
#include "network.h"
#include "netinet/if_ether.h"
#include "cyg/io/eth/netdev.h"
#include "net/wireless.h"
#include "stdarg.h"
#include "assert.h"
#include "net/if.h"
#include "inet.h"

//#define CYGPKG_IO_SERIAL_TCP_POLLED_MODE
#define CYGPKG_IO_SERIAL_TCP_NEW_THREAD

#ifdef CYGPKG_IO_SERIAL_ARM_W99702
#include "tcp_serial.h"
#include "ppot.h"


enum TCP_SERIAL_CTRL_CMD
{
	TCP_SERIAL__DISCONNECT	= 1,
	TCP_SERIAL__CONNECT		= 2
};


/* Queue used in tcp serial buffer */

#define TQ_SIZE(q) sizeof((q)->buf)

#define TQ_VALID_LEN(q) \
	((q)->tail >= (q)->head ? (q)->tail - (q)->head : (q)->tail + TQ_SIZE(q) - (q)->head )

#define TQ_FREE_LEN(q) \
	(TQ_SIZE(q) - TQ_VALID_LEN(q) - 1)

#define TQ_VALID_TAIL_LEN(q) \
	((q)->tail >= (q)->head ? TQ_VALID_LEN(q) : TQ_SIZE(q) - (q)->head)

#define TQ_FREE_TAIL_LEN(q) \
	((q)->tail >= (q)->head ? TQ_SIZE(q) - (q)->tail - ((q)->head == 0 ? 1 : 0 ) : TQ_FREE_LEN(q))
		

/* n must < TQ_SIZE(q) */
#define TQ_NEXT_HEAD(q, n) \
	((q)->head + (n) >= TQ_SIZE(q) ? (q)->head + (n) - TQ_SIZE(q) : (q)->head + (n))

#define TQ_NEXT_TAIL(q, n) \
	((q)->tail + (n) >= TQ_SIZE(q) ? (q)->tail + (n) - TQ_SIZE(q) : (q)->tail + (n))

#define TQ_IS_FULL(q) (TQ_NEXT_TAIL(q, 1) == (q)->head)

#define TQ_IS_NULL(q) ((q)->head == (q)->tail)




typedef struct
{
	int head;
	int tail;
	char buf[1024];
} tq_buf;

typedef struct tcp_serial_info {
    //CYG_ADDRWORD   base;
    //CYG_WORD       int_num;
    //cyg_interrupt  serial_interrupt;
    //cyg_handle_t   serial_interrupt_handle;
    cyg_mutex_t		mutex;
    cyg_sem_t		sem_send;
    cyg_sem_t		sem_recv;
    enum TCP_SERIAL_CTRL_CMD ctrl_cmd;
    
    cyg_bool		tx_enable;
    
    cyg_mutex_t		buf_mutex;
	tq_buf			r_buf;
    cyg_cond_t		r_full;
    cyg_cond_t		r_null;
    //cyg_sem_t		r_produce;
    //cyg_sem_t		r_consume;
    
    int				w_check0;
    int				w_check1;
    int				w_checksize0;
    int				w_checksize1;
    tq_buf			w_buf;
    cyg_cond_t		w_full;
    cyg_cond_t		w_null;
    //cyg_sem_t		w_produce;
    //cyg_sem_t		w_consume;
    
    //unsigned int	ip_addr;
    char			server[128];
	unsigned short	port;
	int				fd;
	bool			exit_flag;
} tcp_serial_info;

static bool tcp_serial_init(struct cyg_devtab_entry *tab);
static bool tcp_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo tcp_serial_lookup(struct cyg_devtab_entry **tab, 
                                   struct cyg_devtab_entry *sub_tab,
                                   const char *name);
static unsigned char tcp_serial_getc(serial_channel *chan);
static Cyg_ErrNo tcp_serial_set_config(serial_channel *chan, cyg_uint32 key,
					      const void *xbuf, cyg_uint32 *len);
static void tcp_serial_start_xmit(serial_channel *chan);
static void tcp_serial_stop_xmit(serial_channel *chan);


#ifndef CYGPKG_IO_SERIAL_TCP_POLLED_MODE
static void       
tcp_serial_thread(cyg_addrword_t data);
static cyg_handle_t g_tcp_serial_handle;
static cyg_thread g_tcp_serial_thread;
static unsigned long g_tcp_serial_thread_stack[16*1024/sizeof(unsigned long)];
#endif

void tcp_serial_send_cmd(enum TCP_SERIAL_CTRL_CMD ctrl_cmd);
enum TCP_SERIAL_CTRL_CMD tcp_serial_recv_cmd(tcp_serial_info *tcp_chan);
enum TCP_SERIAL_CTRL_CMD tcp_serial_try_recv_cmd(tcp_serial_info *tcp_chan);



static SERIAL_FUNS(tcp_serial_funs, 
                   tcp_serial_putc, 
                   tcp_serial_getc,
                   tcp_serial_set_config,
                   tcp_serial_start_xmit,
                   tcp_serial_stop_xmit
    );


#ifdef CYGPKG_IO_SERIAL_TCP_SERIAL0
static tcp_serial_info tcp_serial_info0;

//#undef CYGNUM_IO_SERIAL_TCP_SERIAL0_BUFSIZE
//#define CYGNUM_IO_SERIAL_TCP_SERIAL0_BUFSIZE 64

#if CYGNUM_IO_SERIAL_TCP_SERIAL0_BUFSIZE > 0
static unsigned char tcp_serial_out_buf0[CYGNUM_IO_SERIAL_TCP_SERIAL0_BUFSIZE];
static unsigned char tcp_serial_in_buf0[CYGNUM_IO_SERIAL_TCP_SERIAL0_BUFSIZE];

static serial_channel tcp_serial_channel0 = {
    &tcp_serial_funs,
    &cyg_io_serial_callbacks,
    &tcp_serial_info0,
    CYG_SERIAL_INFO_INIT(CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_TCP_SERIAL0_BAUD), CYG_SERIAL_STOP_DEFAULT, CYG_SERIAL_PARITY_DEFAULT, CYG_SERIAL_WORD_LENGTH_DEFAULT, CYG_SERIAL_FLAGS_DEFAULT),
    false,
    CBUF_INIT(&tcp_serial_out_buf0[0], sizeof(tcp_serial_out_buf0)),
    CBUF_INIT(&tcp_serial_in_buf0[0],  sizeof(tcp_serial_in_buf0))
};

#else
static SERIAL_CHANNEL(tcp_serial_channel0,
                      tcp_serial_funs, 
                      tcp_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_TCP_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

#pragma arm section rwdata = "devtab"
DEVTAB_ENTRY(tcp_serial_io0, 
             CYGDAT_IO_SERIAL_TCP_SERIAL0_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             tcp_serial_init, 
             tcp_serial_lookup,     // Serial driver may need initializing
             &tcp_serial_channel0
    );
#pragma arm section rwdata

#endif //  CYGPKG_IO_SERIAL_TCP_SERIAL0




bool fd_is_readable(int fd)
{
	struct timeval tv;
	fd_set rset;
	
	FD_ZERO(&rset);
	FD_SET(fd, &rset);
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	
	if (select(fd + 1, &rset, NULL, NULL, &tv) == 1)
	{
		if (FD_ISSET(fd, &rset))
			return true;
	}
	
	return false;
}

bool fd_is_writable(int fd)
{
	struct timeval tv;
	fd_set wset;
	
	FD_ZERO(&wset);
	FD_SET(fd, &wset);
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	
	if (select(fd + 1, NULL, &wset, NULL, &tv) == 1)
	{
		if (FD_ISSET(fd, &wset))
			return true;
	}
	
	return false;
}


static char tcp_serial_read(void *priv)
{
//#ifdef CYGPKG_IO_SERIAL_TCP_NEW_THREAD
#if 0
	char ch;
	bool is_full;
	tcp_serial_info *tcp_chan = (tcp_serial_info *)priv;
	
	diag_printf("tcp_serial_read\n");
	cyg_mutex_lock(&tcp_chan->buf_mutex);
	
	while (TQ_IS_NULL(&tcp_chan->r_buf))
		cyg_cond_wait(&tcp_chan->r_null);
	
	is_full = TQ_IS_FULL(&tcp_chan->r_buf);

	ch = tcp_chan->r_buf.buf[tcp_chan->r_buf.head];
	tcp_chan->r_buf.head = TQ_NEXT_HEAD(&tcp_chan->r_buf, 1);
	
	if (is_full)
		cyg_cond_signal(&tcp_chan->r_full);
	
	cyg_mutex_unlock(&tcp_chan->buf_mutex);

	diag_printf("tcp_serial_read=%02x\n", (int)(unsigned char)ch);
	return ch;
#else
	char ch = 0;
	tcp_serial_info *tcp_chan = (tcp_serial_info *)priv;
	//diag_printf("tcp_serial_read\n");
	
	if (tcp_chan->fd >= 0)
	{
		int read_len = read(tcp_chan->fd, (void*)&ch, sizeof(ch));
		diag_printf("R:%02x\n", (int)ch);
		
		if (read_len == 0)
			tcp_chan->exit_flag = true;
		else if (read_len < 0)
		{
			if (errno != EAGAIN)
				tcp_chan->exit_flag = true;
			else
				diag_printf("Failed to read-------------\n");
		}
	}
	
	return ch;
#endif
}


static char tcp_serial_write(void *priv, char ch)
{
#ifdef CYGPKG_IO_SERIAL_TCP_NEW_THREAD
	int next_pos;
	bool is_null;
	tcp_serial_info *tcp_chan = (tcp_serial_info *)priv;
	
	//diag_printf("tcp_serial_write=%02x\n", (int)(unsigned char)ch);
	cyg_mutex_lock(&tcp_chan->buf_mutex);
	
	while(TQ_IS_FULL(&tcp_chan->w_buf))
		cyg_cond_wait(&tcp_chan->w_full);
	
	is_null = TQ_IS_NULL(&tcp_chan->w_buf);
	
	tcp_chan->w_buf.buf[tcp_chan->w_buf.tail] = ch;
	tcp_chan->w_buf.tail = TQ_NEXT_TAIL(&tcp_chan->w_buf, 1);
	
	tcp_chan->w_check0 += (unsigned char)ch;
	tcp_chan->w_checksize0++;
	
	if (is_null)
		cyg_cond_signal(&tcp_chan->w_null);
	
	cyg_mutex_unlock(&tcp_chan->buf_mutex);
	//diag_printf("tcp_serial_write\n");
	
#else
	tcp_serial_info *tcp_chan = (tcp_serial_info *)priv;
	//diag_printf("tcp_serial_write:%d\n", (int)ch);
	
	if (tcp_chan->fd >= 0)
	{
		//diag_printf("W:%02x\n", (int)ch);
		write(tcp_chan->fd, (void*)&ch, sizeof(ch));
	}
	return ch;
#endif
}


static bool tcp_serial_tx_ready(void *priv)
{
#ifdef CYGPKG_IO_SERIAL_TCP_NEW_THREAD
	bool is_full;
	tcp_serial_info *tcp_chan = (tcp_serial_info *)priv;
	
	if (tcp_chan->fd < 0)
		return false;
			
	cyg_mutex_lock(&tcp_chan->buf_mutex);	
	is_full = TQ_IS_FULL(&tcp_chan->w_buf);
	cyg_mutex_unlock(&tcp_chan->buf_mutex);
	
	return (is_full ? false : true);
#else
	tcp_serial_info *tcp_chan = (tcp_serial_info *)priv;
	
	struct timeval tv;
	fd_set wset;
	
	//diag_printf("tcp_serial_tx_ready\n");
	
	if (tcp_chan->fd < 0)
		return false;
	
	FD_ZERO(&wset);
	FD_SET(tcp_chan->fd, &wset);
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	
	if (select(tcp_chan->fd + 1, NULL, &wset, NULL, &tv) == 1)
	{
		if (FD_ISSET(tcp_chan->fd, &wset))
			return true;
	}
	
	return false;
#endif
}


static bool tcp_serial_rx_data(void *priv)
{
//#ifdef CYGPKG_IO_SERIAL_TCP_NEW_THREAD
#if 0
	bool is_null;
	tcp_serial_info *tcp_chan = (tcp_serial_info *)priv;
	
	cyg_mutex_lock(&tcp_chan->buf_mutex);
	is_null = TQ_IS_NULL(&tcp_chan->r_buf);
	cyg_mutex_unlock(&tcp_chan->buf_mutex);
	
	return (is_null ? false : true);
#else
	tcp_serial_info *tcp_chan = (tcp_serial_info *)priv;
	
	struct timeval tv;
	fd_set rset;
	
	//diag_printf("tcp_serial_rx_data\n");
	
	
	if (tcp_chan->fd < 0)
		return false;
	
	FD_ZERO(&rset);
	FD_SET(tcp_chan->fd, &rset);
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	
	if (select(tcp_chan->fd + 1, &rset, NULL, NULL, &tv) == 1)
	{
		if (FD_ISSET(tcp_chan->fd, &rset))
			return true;
	}
	
	return false;
#endif
}



#define GET_CHAR(p)			tcp_serial_read(p)
#define PUT_CHAR(p, c)		tcp_serial_write(p,c)
#define TX_READY(p)			tcp_serial_tx_ready(p)
#define RX_DATA(p)			tcp_serial_rx_data(p)


// debugging help
static int chars_rx = 0 ;
static int chars_tx = 0 ;

// Internal function to actually configure the hardware to desired baud rate, etc.
static bool
tcp_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    tcp_serial_info *tcp_chan = (tcp_serial_info *)chan->dev_priv;
    //unsigned int ip_addr = tcp_chan->ip_addr;
    const char *server = tcp_chan->server;
    unsigned int port = tcp_chan->port;
    int fd = tcp_chan->fd;

	//xh: add code to configure TCP connection here.
	//disable_uart_tx_interrupt
	//disable_uart_rx_interrupt
	
	//enable_uart_rx_interrupt
	//enable_uart_tx_interrupt
		
    // success
    return true;
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
tcp_serial_init(struct cyg_devtab_entry *tab)
{
#if 1
    serial_channel *chan = (serial_channel *)tab->priv;
    tcp_serial_info *tcp_chan = (tcp_serial_info *)chan->dev_priv;

	diag_printf("%x %x %x\n%x %x %x\n", tab, chan, tcp_chan,
	
		&tcp_serial_info0, &tcp_serial_io0, &tcp_serial_channel0);

	tcp_chan->w_check0 = 0;
	tcp_chan->w_check1 = 0;
	tcp_chan->w_checksize0 = 0;
	tcp_chan->w_checksize1 = 0;
	
    tcp_chan->r_buf.head = 0;
    tcp_chan->r_buf.tail = 0;
    cyg_mutex_init(&tcp_chan->buf_mutex);
    cyg_cond_init(&tcp_chan->r_full, &tcp_chan->buf_mutex);
    cyg_cond_init(&tcp_chan->r_null, &tcp_chan->buf_mutex);
    //cyg_semaphore_init(&tcp_chan->r_produce, 1);
    //cyg_semaphore_init(&tcp_chan->r_consume, 0);
    
    tcp_chan->w_buf.head = 0;
    tcp_chan->w_buf.tail = 0;
    cyg_cond_init(&tcp_chan->w_full, &tcp_chan->buf_mutex);
    cyg_cond_init(&tcp_chan->w_null, &tcp_chan->buf_mutex);
    //cyg_semaphore_init(&tcp_chan->w_produce, 1);
    //cyg_semaphore_init(&tcp_chan->w_consume, 0);



	cyg_mutex_init(&tcp_chan->mutex);
	cyg_semaphore_init(&tcp_chan->sem_send, 1);	
	cyg_semaphore_init(&tcp_chan->sem_recv, 0);	
	tcp_chan->ctrl_cmd = 0;
	
    tcp_chan->server[0] = '\0';
    tcp_chan->port = 0;
    tcp_chan->fd = -1;
	//diag_printf("TCP emu SERIAL init - dev: %s,%d\n", tcp_chan->server, tcp_chan->port);

    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices

#ifndef CYGPKG_IO_SERIAL_TCP_POLLED_MODE
	cyg_thread_create(10, &tcp_serial_thread, (cyg_addrword_t)chan, "tcp_serial", g_tcp_serial_thread_stack,
		sizeof(g_tcp_serial_thread_stack), &g_tcp_serial_handle, &g_tcp_serial_thread);			
	cyg_thread_resume(g_tcp_serial_handle);
#endif
	
	
	tcp_serial_config_port(chan, &chan->config, true);
#endif
    return true;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
tcp_serial_lookup(struct cyg_devtab_entry **tab, 
                  struct cyg_devtab_entry *sub_tab,
                  const char *name)
{
    serial_channel *chan = (serial_channel *)(*tab)->priv;
    
    diag_printf("tcp_serial_lookup\n");
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    return ENOERR;
}


// Send a character to the device output buffer.
// Return 'true' if character is sent to device
static inline bool
tcp_serial_putc(serial_channel *chan, unsigned char c)
{
    tcp_serial_info *tcp_chan = (tcp_serial_info *)chan->dev_priv;

	//diag_printf("putc:%02x\n", (int)c);
    if(!TX_READY( tcp_chan ))
    	return false;

    PUT_CHAR(tcp_chan, c);
    return true;
}

// Fetch a character from the device input buffer, waiting if necessary
static inline unsigned char 
tcp_serial_getc(serial_channel *chan)
{
    unsigned char c;
    tcp_serial_info *tcp_chan = (tcp_serial_info *)chan->dev_priv;
    unsigned int status ;

	//diag_printf("gutc->\n", (int)c);
	while(!RX_DATA(tcp_chan));	// Wait for char

    c = GET_CHAR(tcp_chan) ;
    
    //diag_printf("gutc:%02x\n", (int)c);
    return c;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
tcp_serial_set_config(serial_channel *chan, cyg_uint32 key, const void *xbuf,
                      cyg_uint32 *len)
{
    tcp_serial_info *tcp_chan = (tcp_serial_info *)chan->dev_priv;

    switch (key) {
    case CYG_IO_SET_CONFIG_SERIAL_INFO:
      {
        cyg_serial_info_t *config = (cyg_serial_info_t *)xbuf;
        if ( *len < sizeof(cyg_serial_info_t) ) {
            return -EINVAL;
        }
        *len = sizeof(cyg_serial_info_t);

        if ( true != tcp_serial_config_port(chan, config, false) )
            return -EINVAL;
      }
      break;
    default:
        return -EINVAL;
    }
    return ENOERR;
}

// Enable the transmitter on the device
static void
tcp_serial_start_xmit(serial_channel *chan)
{
#ifndef CYGPKG_IO_SERIAL_TCP_POLLED_MODE
    tcp_serial_info *tcp_chan = (tcp_serial_info *)chan->dev_priv;
	
	tcp_chan->tx_enable = true;

	(chan->callbacks->xmt_char)(chan);
#endif 
}

// Disable the transmitter on the device
static void 
tcp_serial_stop_xmit(serial_channel *chan)
{
#ifndef CYGPKG_IO_SERIAL_TCP_POLLED_MODE
    tcp_serial_info *tcp_chan = (tcp_serial_info *)chan->dev_priv;

    tcp_chan->tx_enable = false;
#endif
}




// Serial I/O - io alarm handler
static void       
tcp_serial_thread(cyg_addrword_t data)
{
	char			server[128];
	unsigned short	port;
	
    serial_channel *chan = (serial_channel *)data;
    tcp_serial_info *tcp_chan = (tcp_serial_info *)chan->dev_priv;

#if 0    
    (chan->callbacks->rcv_char)(chan, GET_CHAR(tcp_chan));
    if (tcp_chan->tx_enable)
		(chan->callbacks->xmt_char)(chan);//serial_xmt_char
#else

	while(1)
	{
		struct hostent* phostent;
		struct sockaddr_in sin;
		enum TCP_SERIAL_CTRL_CMD ctrl_cmd;
		
		//Wait start
		ctrl_cmd = tcp_serial_recv_cmd(tcp_chan);
		if (ctrl_cmd == TCP_SERIAL__CONNECT)
			;
		else if (ctrl_cmd == TCP_SERIAL__DISCONNECT)
			continue;
		else
			continue;

		
l_connect:
		/* get server address and port */
		cyg_mutex_lock(&tcp_chan->mutex);	
		if(tcp_chan->server == NULL || tcp_chan->server[0] == '\0' || tcp_chan->port == 0)
		{
			cyg_mutex_unlock(&tcp_chan->mutex);
			continue;
		}
		strncpy(server, tcp_chan->server, sizeof(server));
		port = tcp_chan->port;
		cyg_mutex_unlock(&tcp_chan->mutex);
		
		
		diag_printf("PPOT: resolve host: %s\n", server);
		/* Resove host */
		phostent = gethostbyname(server);
		if (phostent == (struct hostent *)NULL
			|| phostent->h_addr_list == NULL
			|| *(phostent->h_addr_list) == 0)
		{
			diag_printf("Can not resolve host for TCP serial device: %s\n", server);
			continue;
		}
		
		
		/* try commands */
		ctrl_cmd = tcp_serial_try_recv_cmd(tcp_chan);
		if (ctrl_cmd == TCP_SERIAL__CONNECT)
			goto l_connect;
		else if (ctrl_cmd == TCP_SERIAL__DISCONNECT)
			continue;
		

		/* Create socket */
		tcp_chan->fd = socket(AF_INET, SOCK_STREAM, 0);
		if (tcp_chan->fd < 0)
		{
			diag_printf("Can not create socket for TCP serial device\n");
			continue;
		}
		
		/* Connect socket */
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sin.sin_addr.s_addr = *(unsigned long *)*(phostent->h_addr_list);
		
		diag_printf("PPOT: connect server:%x\n", sin.sin_addr.s_addr);
		connect(tcp_chan->fd, (struct sockaddr *)&sin, sizeof(struct sockaddr));
		
//#ifdef CYGPKG_IO_SERIAL_TCP_NEW_THREAD	
#if 0
		{
			int flag = 1;
			if (setsockopt(tcp_chan->fd, IPPROTO_TCP, 0x01/*TCP_NODELAY*/, (void *) &flag,
				sizeof(flag)) == -1
				
				|| ioctl(tcp_chan->fd,FIONBIO,&flag) == -1)
			{
				diag_printf("Can not set socket to nodelay mode!\n");
				close(tcp_chan->fd);
				continue;
			}
		}
#endif
		
		cyg_mutex_lock(&tcp_chan->buf_mutex);
		tcp_chan->exit_flag = false;
		//tcp_chan->r_buf.head = 0;
		//tcp_chan->r_buf.tail = 0;
		//tcp_chan->w_buf.head = 0;
		//tcp_chan->w_buf.tail = 0;
		cyg_mutex_unlock(&tcp_chan->buf_mutex);
		
		while(1)
		{
			int length;
			bool is_null;
			bool is_full;
			
			enum TCP_SERIAL_CTRL_CMD ctrl_cmd = tcp_serial_try_recv_cmd(tcp_chan);
			/* try commands */
			ctrl_cmd = tcp_serial_try_recv_cmd(tcp_chan);
			if (ctrl_cmd == TCP_SERIAL__CONNECT)
			{
				diag_printf("PPOT: connect new server\n");
				close(tcp_chan->fd);
				tcp_chan->fd = -1;
				goto l_connect;
			}
			else if (ctrl_cmd == TCP_SERIAL__DISCONNECT)
			{
				diag_printf("PPOT: Disconnect\n");
				break;
			}
			
//#ifdef CYGPKG_IO_SERIAL_TCP_NEW_THREAD
#if 0
			//diag_printf("Before buf_mutex\n");
			cyg_mutex_lock(&tcp_chan->buf_mutex);
			//diag_printf("After buf_mutex\n");
			
			is_null = TQ_IS_NULL(&tcp_chan->r_buf);
			length = TQ_FREE_TAIL_LEN(&tcp_chan->r_buf);
			
			if (length > 0 && fd_is_readable(tcp_chan->fd))
			{
				int read_len;
				
				diag_printf("read length=%d\n", length);
				read_len = read(tcp_chan->fd, tcp_chan->r_buf.buf + tcp_chan->r_buf.tail, length);
				diag_printf("read=%d,is_full=%d\n", read_len, (int)is_null);
				if (read_len > 0)
				{
					int i; for(i = 0; i < read_len; ++i)
						diag_printf("read=%02x\n", tcp_chan->r_buf.buf[tcp_chan->r_buf.tail + i]);
						
					//tcp_chan->r_buf.tail = TQ_NEXT_TAIL(&tcp_chan->r_buf, read_len);
					
					//if (is_null)
					//	cyg_cond_signal(&tcp_chan->r_null);
					
					for(i = 0; i < read_len; ++i)
						(chan->callbacks->rcv_char)(chan, tcp_chan->r_buf.buf[tcp_chan->r_buf.tail + i]);
					
				}
			}
			
			
			is_full = TQ_IS_FULL(&tcp_chan->w_buf);
			length = TQ_VALID_TAIL_LEN(&tcp_chan->w_buf);
			
			if (length > 0 && fd_is_writable(tcp_chan->fd))
			{
				int write_len;
				diag_printf("write length=%d, head=%d\n", length, tcp_chan->w_buf.head);
				write_len = write(tcp_chan->fd, tcp_chan->w_buf.buf + tcp_chan->w_buf.head, length);
				diag_printf("write=%d,is_full=%d\n", write_len, (int)is_full);
				if (write_len > 0)
				{
					int i; for(i = 0; i < write_len; ++i)
						diag_printf("write=%02x\n", tcp_chan->w_buf.buf[tcp_chan->r_buf.head + i]);
				
					tcp_chan->w_buf.head = TQ_NEXT_HEAD(&tcp_chan->w_buf, write_len);
					
					if (is_full)
						cyg_cond_signal(&tcp_chan->w_full);
				}
			}
	
			cyg_mutex_unlock(&tcp_chan->buf_mutex);

			//if (RX_DATA(tcp_chan))
			//	(chan->callbacks->rcv_char)(chan, GET_CHAR(tcp_chan));
			if (tcp_chan->tx_enable)
				(chan->callbacks->xmt_char)(chan);//serial_xmt_char

#elif 1
{
			char rt_buf[1024];
			char wt_buf[1024];
			bool transferred;
			
//			retry:
//diag_printf("Befor tcp_chan->tx_enable=%d\n", (int)tcp_chan->tx_enable);						
			//if (tcp_chan->tx_enable)
				(chan->callbacks->xmt_char)(chan);//serial_xmt_char
				
//diag_printf("Befor lock\n");			
			transferred = false;
			cyg_mutex_lock(&tcp_chan->buf_mutex);
			
			is_full = TQ_IS_FULL(&tcp_chan->w_buf);
			length = TQ_VALID_LEN(&tcp_chan->w_buf);
			if (length >= 2)
			{
				int lastpos;
				if (tcp_chan->w_buf.tail == 0)
					lastpos = TQ_SIZE(&tcp_chan->w_buf) - 1;
				else
					lastpos = tcp_chan->w_buf.tail - 1;

//diag_printf("is_full=%d,len=%d,tailch=%02x\n", (int)is_full, (int)length, (int)tcp_chan->w_buf.buf[lastpos]);
				
				if (!is_full && tcp_chan->w_buf.buf[lastpos] != (char)0x7E)
				{
					//cyg_mutex_unlock(&tcp_chan->buf_mutex);
					//goto retry;
				}
				else
				{
					int length1, length2;
//diag_printf("line %d\n", __LINE__);					
					length1 = TQ_VALID_TAIL_LEN(&tcp_chan->w_buf);
					memcpy(wt_buf, tcp_chan->w_buf.buf + tcp_chan->w_buf.head, length1);
					tcp_chan->w_buf.head = TQ_NEXT_HEAD(&tcp_chan->w_buf, length1);
//diag_printf("line %d\n", __LINE__);					
					length2 = TQ_VALID_TAIL_LEN(&tcp_chan->w_buf);
					memcpy(wt_buf + length1, tcp_chan->w_buf.buf + tcp_chan->w_buf.head, length2);
					tcp_chan->w_buf.head = TQ_NEXT_HEAD(&tcp_chan->w_buf, length2);
//diag_printf("line %d\n", __LINE__);					
					write(tcp_chan->fd, wt_buf, length1 + length2);
//diag_printf("line %d\n", __LINE__);

					{int i = 0;for(i = 0;i < length1 + length2;++i)
					{
						tcp_chan->w_check1 += (unsigned char)wt_buf[i];
						tcp_chan->w_checksize1++;
					}}
					if(tcp_chan->w_check0 != tcp_chan->w_check1
						|| tcp_chan->w_checksize0 != tcp_chan->w_checksize1)
					{
						diag_printf("write check sum error\n");
						while(1);
					}	
					

					if(0)
					{int i = 0;for(i = 0;i < length1 + length2;++i)
						diag_printf("W:%02x\n", (int)(unsigned char)wt_buf[i]);
					}
					
					transferred = true;
//diag_printf("line %d\n", __LINE__);					
				}
			}
			cyg_mutex_unlock(&tcp_chan->buf_mutex);

			if(!transferred)
				(chan->callbacks->xmt_char)(chan);//serial_xmt_char
				

//diag_printf("Before read\n");			
			if (RX_DATA(tcp_chan))
			{
				int ret = read(tcp_chan->fd, rt_buf, sizeof(rt_buf));
//("Read=%d\n", ret);
				if (ret > 0)
				{
					int i;
					if (0)
					{int i = 0;for(i = 0;i < ret;++i)
						diag_printf("R:%02x\n", (int)(unsigned char)rt_buf[i]);
					}

					for( i = 0; i < ret; ++i)
						(chan->callbacks->rcv_char)(chan, rt_buf[i]);	
				}
				transferred = true;
			}
			
			if(!transferred)
			{
				cyg_thread_delay(0);
				cyg_thread_yield();
			}
//diag_printf("Over read\n");						
}
#elif 1
			//diag_printf("Before buf_mutex\n");
			//cyg_mutex_lock(&tcp_chan->buf_mutex);
			//diag_printf("After buf_mutex\n");

			if (tcp_chan->tx_enable)
				(chan->callbacks->xmt_char)(chan);//serial_xmt_char

			
			cyg_mutex_lock(&tcp_chan->buf_mutex);
			
			is_full = TQ_IS_FULL(&tcp_chan->w_buf);
			length = TQ_VALID_TAIL_LEN(&tcp_chan->w_buf);
			
			if (!tcp_chan->exit_flag && length > 0 && fd_is_writable(tcp_chan->fd))
			{
				int write_len;
				//diag_printf("write length=%d, head=%d\n", length, tcp_chan->w_buf.head);
				write_len = write(tcp_chan->fd, tcp_chan->w_buf.buf + tcp_chan->w_buf.head, length);
				//diag_printf("write=%d,is_full=%d\n", write_len, (int)is_full);
				if (write_len > 0)
				{
					int i; for(i = 0; i < write_len; ++i)
						diag_printf("W=%02x\n", tcp_chan->w_buf.buf[tcp_chan->w_buf.head + i]);
				
					tcp_chan->w_buf.head = TQ_NEXT_HEAD(&tcp_chan->w_buf, write_len);
					
					if (is_full)
						cyg_cond_signal(&tcp_chan->w_full);
				}
				else if (write_len == 0)
				{
					tcp_chan->exit_flag = true;
				}
				else if (write_len < 0)
				{
					if (errno != EAGAIN)
						tcp_chan->exit_flag = true;
				}
			}
	
			cyg_mutex_unlock(&tcp_chan->buf_mutex);
			

#if 0
			is_null = TQ_IS_NULL(&tcp_chan->r_buf);
			length = TQ_FREE_TAIL_LEN(&tcp_chan->r_buf);
			
			if (length > 0 /*&& fd_is_readable(tcp_chan->fd)*/)
			{
				int read_len;
				
				diag_printf("read length=%d\n", length);
				read_len = read(tcp_chan->fd, tcp_chan->r_buf.buf + tcp_chan->r_buf.tail, 1);
				diag_printf("read=%d,is_full=%d\n", read_len, (int)is_null);
				if (read_len > 0)
				{
					int i; for(i = 0; i < read_len; ++i)
						diag_printf("read=%02x\n", tcp_chan->r_buf.buf[tcp_chan->r_buf.tail + i]);
						
					//tcp_chan->r_buf.tail = TQ_NEXT_TAIL(&tcp_chan->r_buf, read_len);
					
					//if (is_null)
					//	cyg_cond_signal(&tcp_chan->r_null);
					
					for(i = 0; i < read_len; ++i)
						(chan->callbacks->rcv_char)(chan, tcp_chan->r_buf.buf[tcp_chan->r_buf.tail + i]);
					
				}
			}
#else			
			//if (RX_DATA(tcp_chan))
				(chan->callbacks->rcv_char)(chan, GET_CHAR(tcp_chan));
#endif			
			

#else
			if (fd_is_readable(tcp_chan->fd))
				(chan->callbacks->rcv_char)(chan, GET_CHAR(tcp_chan));
			if (tcp_chan->tx_enable)
				(chan->callbacks->xmt_char)(chan);//serial_xmt_char
#endif
			
			if (tcp_chan->exit_flag)
			{
				diag_printf("Connection dropped\n");
				break;
			}
			
		}
		
		close(tcp_chan->fd);
		tcp_chan->fd = -1;		
	}

#endif
    
}



void tcp_serial_send_cmd(enum TCP_SERIAL_CTRL_CMD ctrl_cmd)
{
#if 0
	static int first = 0;
	if (first == 0)
	{
		first = 1;
	cyg_thread_create(10, &tcp_serial_thread, (cyg_addrword_t)&tcp_serial_info0, "tcp_serial", g_tcp_serial_thread_stack,
		sizeof(g_tcp_serial_thread_stack), &g_tcp_serial_handle, &g_tcp_serial_thread);			
	cyg_thread_resume(g_tcp_serial_handle);
	}
#endif


	cyg_semaphore_wait(&tcp_serial_info0.sem_send);
	
	cyg_mutex_lock(&tcp_serial_info0.mutex);	
	tcp_serial_info0.ctrl_cmd = ctrl_cmd;
	cyg_mutex_unlock(&tcp_serial_info0.mutex);
	
	cyg_semaphore_post(&tcp_serial_info0.sem_recv);
}


enum TCP_SERIAL_CTRL_CMD tcp_serial_recv_cmd(tcp_serial_info *tcp_chan)
{
	enum TCP_SERIAL_CTRL_CMD ctrl_cmd;
	
	cyg_semaphore_wait(&tcp_chan->sem_recv);
	
	cyg_mutex_lock(&tcp_chan->mutex);	
	ctrl_cmd = tcp_chan->ctrl_cmd;	
	cyg_mutex_unlock(&tcp_chan->mutex);
	
	cyg_semaphore_post(&tcp_chan->sem_send);
	return ctrl_cmd;
}


enum TCP_SERIAL_CTRL_CMD tcp_serial_try_recv_cmd(tcp_serial_info *tcp_chan)
{
	enum TCP_SERIAL_CTRL_CMD ctrl_cmd;
	
	if (tcp_chan->ctrl_cmd == 0)
		return 0;

	if (cyg_semaphore_trywait(&tcp_chan->sem_recv) == false)
		return 0;
	
	cyg_mutex_lock(&tcp_chan->mutex);	
	ctrl_cmd = tcp_chan->ctrl_cmd;	
	cyg_mutex_unlock(&tcp_chan->mutex);
	
	cyg_semaphore_post(&tcp_chan->sem_send);
	return ctrl_cmd;
}



void ppot_disconnect()
{
	tcp_serial_send_cmd(TCP_SERIAL__DISCONNECT);
}

void ppot_connect(const char *server, int port)
{
	cyg_mutex_lock(&tcp_serial_info0.mutex);
	strncpy(tcp_serial_info0.server, server, sizeof(tcp_serial_info0.server));
	tcp_serial_info0.port = port;
	cyg_mutex_unlock(&tcp_serial_info0.mutex);
	
	tcp_serial_send_cmd(TCP_SERIAL__CONNECT);	
}


#endif

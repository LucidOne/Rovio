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



#ifdef CYGPKG_IO_SERIAL_ARM_W99702
#include "tcp_serial.h"
#include "ppp.h"
#include "ppot.h"
#include "tt_thread.h"

enum TCP_SERIAL_CTRL_CMD
{
	TCP_SERIAL__DISCONNECT	= 1,
	TCP_SERIAL__CONNECT		= 2,
	TCP_SERIAL__START_WRITE	= 3,
	TCP_SERIAL__ERROR_DISCONNECT = 4
};

enum TCP_PPP_STATE_E
{
	TCP_PPP__OFFLINE,
	TCP_PPP__TRY_ONLINE,
	TCP_PPP__ONLINE,
	TCP_PPP__WAIT_RECONNECT
};

//#define TCP_SERIAL_RECONNECT_TIMEOUT_SEC	8
#define PPP_MAX_SERVERS						10


typedef struct tcp_serial_info {
    cyg_mutex_t		mutex;
    int				cmd_fd;
    
    cyg_bool		tx_enable;
    
    TT_RMUTEX_T		buf_mutex;
    TT_COND_T		w_full;
    TT_COND_T		w_data;
    char			w_buf[1024];
    size_t			w_buf_len;
    
    char			servers[PPP_MAX_SERVERS][64];
	unsigned short	ports[PPP_MAX_SERVERS];
	unsigned int	current_server;
	unsigned int	first_server;
	unsigned int	try_count;
	char			user[MAXNAMELEN];       /* Our name for authenticating ourselves */
    char			passwd[MAXSECRETLEN];   /* Password for PAP and secret for CHAP */

	int				fd;
	bool			reconnect_flag;
	bool			exit_flag;
	
	cyg_ppp_handle_t	ppp_handle;
	enum TCP_PPP_STATE_E		ppp_state;
	//bool				ppp_try_online;
	//bool				ppp_online;
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



static void       
tcp_serial_thread(cyg_addrword_t data);
static cyg_handle_t g_tcp_serial_handle;
static cyg_thread g_tcp_serial_thread;
static unsigned long g_tcp_serial_thread_stack[14*1024/sizeof(unsigned long)];
static void       
tcp_serial_thread1(cyg_addrword_t data);
static cyg_handle_t g_tcp_serial_handle1;
static cyg_thread g_tcp_serial_thread1;
static unsigned long g_tcp_serial_thread_stack1[2*1024/sizeof(unsigned long)];



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



static char tcp_serial_read(void *priv)
{
	char ch = 0;
	tcp_serial_info *tcp_chan = (tcp_serial_info *)priv;
	//diag_printf("tcp_serial_read\n");
	
	if (tcp_chan->fd >= 0)
	{
		int read_len = read(tcp_chan->fd, (void*)&ch, sizeof(ch));
		//ag_printf("R:%02x\n", (int)ch);
		
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
}


static char tcp_serial_write(void *priv, char ch)
{
	bool write_flag = false;
	tcp_serial_info *tcp_chan = (tcp_serial_info *)priv;
	
	tt_rmutex_lock(&tcp_chan->buf_mutex);
	
	//diag_printf("before while tcp_chan->w_buf_len =%d\n", tcp_chan->w_buf_len);
	while(tcp_chan->w_buf_len >= sizeof(tcp_chan->w_buf))
	{
		//Write the buffer!
		//tcp_serial_send_cmd(TCP_SERIAL__START_WRITE);
		tt_cond_signal(&tcp_chan->w_data);
	
		//diag_printf("before tcp_chan->w_buf_len =%d\n", tcp_chan->w_buf_len);
		tt_cond_wait(&tcp_chan->w_full);
//		diag_printf("after tcp_chan->w_buf_len =%d\n", tcp_chan->w_buf_len);
	}
	tcp_chan->w_buf[tcp_chan->w_buf_len++] = ch;

	if(ch == (char)0x7E || tcp_chan->w_buf_len >= sizeof(tcp_chan->w_buf))
		write_flag = true;
	
	tt_rmutex_unlock(&tcp_chan->buf_mutex);


	if (write_flag)
	{
		/* Start to write */
		//tcp_serial_send_cmd(TCP_SERIAL__START_WRITE);
//		diag_printf("before tcp_chan->w_buf_len2 =%d, id=%d\n", tcp_chan->w_buf_len, ((cyg_thread *)cyg_thread_self())->unique_id);
		tt_cond_signal(&tcp_chan->w_data);
//		diag_printf("after tcp_chan->w_buf_len2 =%d\n", tcp_chan->w_buf_len);
	}
	
	return ch;
}


static bool tcp_serial_tx_ready(void *priv)
{
	bool is_full;
	tcp_serial_info *tcp_chan = (tcp_serial_info *)priv;
	
	if (tcp_chan->fd < 0)
		return false;
			
	tt_rmutex_lock(&tcp_chan->buf_mutex);	
	is_full = ( tcp_chan->w_buf_len >= sizeof(tcp_chan->w_buf) ? true : false );
	tt_rmutex_unlock(&tcp_chan->buf_mutex);
	
	return (is_full ? false : true);
}


static bool tcp_serial_rx_data(void *priv)
{
	tcp_serial_info *tcp_chan = (tcp_serial_info *)priv;
	
	struct timeval tv;
	fd_set rset;
	
//	diag_printf("tcp_serial_rx_data\n");
	
	
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
}



#define GET_CHAR(p)			tcp_serial_read(p)
#define PUT_CHAR(p, c)		tcp_serial_write(p,c)
#define TX_READY(p)			tcp_serial_tx_ready(p)
#define RX_DATA(p)			tcp_serial_rx_data(p)



// Internal function to actually configure the hardware to desired baud rate, etc.
static bool
tcp_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    tcp_serial_info *tcp_chan = (tcp_serial_info *)chan->dev_priv;
    //unsigned int ip_addr = tcp_chan->ip_addr;
    //const char *server = tcp_chan->server;
    //unsigned int port = tcp_chan->port;
    //int fd = tcp_chan->fd;

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
	int i;
    serial_channel *chan = (serial_channel *)tab->priv;
    tcp_serial_info *tcp_chan = (tcp_serial_info *)chan->dev_priv;

	//ag_printf("%x %x %x\n%x %x %x\n", tab, chan, tcp_chan,
	//tcp_serial_info0, &tcp_serial_io0, &tcp_serial_channel0);

	
    tt_rmutex_init(&tcp_chan->buf_mutex);
    tt_cond_init(&tcp_chan->w_full, &tcp_chan->buf_mutex);
    tt_cond_init(&tcp_chan->w_data, &tcp_chan->buf_mutex);
    tcp_chan->w_buf_len = 0;

    

	cyg_mutex_init(&tcp_chan->mutex);
	
	tcp_chan->cmd_fd = -1;
	for (i = 0; i < PPP_MAX_SERVERS; ++i)
	{
	    tcp_chan->servers[i][0] = '\0';
	    tcp_chan->ports[i] = 0;
	}
    tcp_chan->fd = -1;
	diag_printf("TCP emu SERIAL init - dev\n");

    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices

	tcp_chan->try_count = 0;
	tcp_chan->exit_flag = false;
	tcp_chan->reconnect_flag = false;
	
	tcp_serial_config_port(chan, &chan->config, true);
	tcp_chan->ppp_state = TCP_PPP__OFFLINE;
	//tcp_chan->ppp_try_online = false;
	//tcp_chan->ppp_online = false;
	tcp_chan->ppp_handle = NULL;
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
    
//    diag_printf("tcp_serial_lookup\n");
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    return ENOERR;
}


// Send a character to the device output buffer.
// Return 'true' if character is sent to device
static inline bool
tcp_serial_putc(serial_channel *chan, unsigned char c)
{
	bool ret;
    tcp_serial_info *tcp_chan = (tcp_serial_info *)chan->dev_priv;

	tt_rmutex_lock(&tcp_chan->buf_mutex);

	if (tcp_chan->exit_flag)
	{
		ret = true;	//Drop !
	}
	else
	{
		//diag_printf("putc:%02x\n", (int)c);
    	if(!TX_READY( tcp_chan ))
    		ret = false;
	    else
    	{
	    	PUT_CHAR(tcp_chan, c);
	    	ret = true;
		}
	}
    
    tt_rmutex_unlock(&tcp_chan->buf_mutex);
    
    return ret;
}

// Fetch a character from the device input buffer, waiting if necessary
static inline unsigned char 
tcp_serial_getc(serial_channel *chan)
{
    unsigned char c;
    tcp_serial_info *tcp_chan = (tcp_serial_info *)chan->dev_priv;
    unsigned int status ;

//	diag_printf("gutc->\n", (int)c);
	while(!RX_DATA(tcp_chan));	// Wait for char

    c = GET_CHAR(tcp_chan) ;
    
//    diag_printf("gutc:%02x\n", (int)c);
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
    tcp_serial_info *tcp_chan = (tcp_serial_info *)chan->dev_priv;
	
	tcp_chan->tx_enable = true;

	(chan->callbacks->xmt_char)(chan);
}

// Disable the transmitter on the device
static void 
tcp_serial_stop_xmit(serial_channel *chan)
{
    tcp_serial_info *tcp_chan = (tcp_serial_info *)chan->dev_priv;

    tcp_chan->tx_enable = false;
}


static void
tcp_serial_close_fd(tcp_serial_info *tcp_chan)
{
	if (tcp_chan->fd >= 0)
	{
		tt_rmutex_lock(&tcp_chan->buf_mutex);
		if (tcp_chan->fd >= 0)
		{
			close(tcp_chan->fd);
			tcp_chan->exit_flag = true;
			tcp_chan->w_buf_len = 0;
			tcp_chan->fd = -1;
		}
		tt_rmutex_unlock(&tcp_chan->buf_mutex);
	}
}


// Serial I/O - io alarm handler
#if 1
static void
tcp_serial_thread1(cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    tcp_serial_info *tcp_chan = (tcp_serial_info *)chan->dev_priv;

	tt_rmutex_lock(&tcp_chan->buf_mutex);
	while(1)
	{
//		diag_printf("check if there's data to be sent = %d\n", tcp_chan->w_buf_len);
		if(tcp_chan->w_buf_len > 0)
			tcp_serial_send_cmd(TCP_SERIAL__START_WRITE);
		tt_cond_wait(&tcp_chan->w_data);
	}
	tt_rmutex_unlock(&tcp_chan->buf_mutex);	
}

static void
tcp_serial_thread(cyg_addrword_t data)
{
	bool has_first_written = false;
	//bool is_connecting = false;
	
	int max_fd = 0;
	struct fd_set rd_set;
	struct fd_set wt_set;
    serial_channel *chan = (serial_channel *)data;
    tcp_serial_info *tcp_chan = (tcp_serial_info *)chan->dev_priv;


	FD_ZERO(&rd_set);
	FD_ZERO(&wt_set);
	
//	diag_printf("in tcp_serial_thread, cmd_fd = %d\n", tcp_chan->cmd_fd);
	if (tcp_chan->cmd_fd >= 0)
	{
//		diag_printf("add fd %d to fdset\n", tcp_chan->cmd_fd);
		FD_SET(tcp_chan->cmd_fd, &rd_set);
		max_fd = tcp_chan->cmd_fd;
	}
	
	
	while(1)
	{
		int rt;
		bool transferred = false;
		struct timeval tv;
		//tv.tv_sec = TCP_SERIAL_RECONNECT_TIMEOUT_SEC;
		tv.tv_sec = rand() % (1 << tcp_chan->try_count);
		if(tv.tv_sec < 1)
			tv.tv_sec = 1;
		tv.tv_usec = 0;
//		diag_printf("before select: %d %d %d\n", max_fd, tcp_chan->cmd_fd, 
//			(int)(FD_ISSET(tcp_chan->cmd_fd, &rd_set)));
		rt = select(max_fd + 1, &rd_set, &wt_set, NULL, &tv);
//		diag_printf("after select\n");
		
		if (rt < 0)
		{
			FD_ZERO(&rd_set);
			FD_ZERO(&wt_set);
		}
		
		if (rt <= 0)
		{
			if (tcp_chan->reconnect_flag)
			{
				diag_printf("Try to connect again...\n");
				tcp_serial_send_cmd(TCP_SERIAL__CONNECT);
			}
		}
		tcp_chan->reconnect_flag = false;
		
		
		/* Check read status */
		if (/*is_connecting*/ tcp_chan->fd >= 0)
		{
			if(!FD_ISSET(tcp_chan->fd, &rd_set))
			{
				if (has_first_written)
					FD_SET(tcp_chan->fd, &rd_set);
			}
			else
			{
				char rd_buf[sizeof(tcp_chan->w_buf)];
				/* Read tcp data here. */
				int ret;
				
//				diag_printf("checked read\n");
				ret = read(tcp_chan->fd, rd_buf, sizeof(rd_buf));
				//("Read=%d\n", ret);
				if (ret > 0)
				{
					int i;
					if (0)
					{int i = 0;for(i = 0;i < ret;++i)
						diag_printf("R:%02x\n", (int)(unsigned char)rd_buf[i]);
					}

					for( i = 0; i < ret; ++i)
					{
						(chan->callbacks->rcv_char)(chan, rd_buf[i]);	
					}
					
					transferred = true;
				}
				if ((ret < 0 && errno != EAGAIN) || ret == 0)
				{
					diag_printf("PPP read error =%d, disconnect ..\n", ret);
					/* Close on error */
					FD_CLR(tcp_chan->fd, &rd_set);
					FD_CLR(tcp_chan->fd, &wt_set);
					max_fd = tcp_chan->cmd_fd;
					tcp_serial_close_fd(tcp_chan);
					//is_connecting = false;
					
					tcp_serial_send_cmd(TCP_SERIAL__ERROR_DISCONNECT);
				}
			}
		}
		
		
		
		/* Check write status */
		if (/*is_connecting*/tcp_chan->fd >= 0)
		{
			char wt_buf[1024];
			size_t wt_len = 0;
			
			tt_rmutex_lock(&tcp_chan->buf_mutex);

			if (FD_ISSET(tcp_chan->fd, &wt_set))
			{
				const char *end;
				if(!has_first_written)
				{
					has_first_written = true;
					FD_SET(tcp_chan->fd, &rd_set);
				}
				
//				diag_printf("checked write\n");
				
				/* Write tcp data here. */
				if (tcp_chan->w_buf_len <= 0)
					;	//Do nothing
				else if(1)
				{
					wt_len = tcp_chan->w_buf_len;
					memcpy(wt_buf, tcp_chan->w_buf, wt_len);
				}
				else
				{
					if(tcp_chan->w_buf_len >= sizeof(tcp_chan->w_buf))
						end = tcp_chan->w_buf + sizeof(tcp_chan->w_buf) - 3;	//Eat all except the tailing 2 bytes.
					else
					{
						end = memchr(tcp_chan->w_buf + 1, (char)0x7E, tcp_chan->w_buf_len - 1);
						if (end == NULL)
							end = tcp_chan->w_buf + tcp_chan->w_buf_len - 1;
					}

					wt_len = end - tcp_chan->w_buf + 1;
					memcpy(wt_buf, tcp_chan->w_buf, wt_len);					
				}

			}
			
			tt_rmutex_unlock(&tcp_chan->buf_mutex);
					
			//Write TCP data
retry:
			if(wt_len > 0)
			{
				int ret, err;
				ret = write(tcp_chan->fd, wt_buf, wt_len);
				err = errno;
//				diag_printf("real write: %d %d\n", ret, err);

				if (ret > 0)
				{
					transferred = true;
				}
				else if ((ret < 0 && err != EAGAIN) || ret == 0)
				{
					diag_printf("PPP read error =%d, disconnect ..\n", ret);
					/* Close on error */
					FD_CLR(tcp_chan->fd, &rd_set);
					FD_CLR(tcp_chan->fd, &wt_set);
					max_fd = tcp_chan->cmd_fd;
					tcp_serial_close_fd(tcp_chan);
					//is_connecting = false;
					
					tcp_serial_send_cmd(TCP_SERIAL__ERROR_DISCONNECT);
				}
				else if (ret < 0 && err == EAGAIN)
				{
					//cyg_thread_delay(0);
					//cyg_thread_yield();		
					//goto retry;
				}
			}

			tt_rmutex_lock(&tcp_chan->buf_mutex);
//			diag_printf("tcp_chan->w_buf_len=%d, wt_len=%d\n", tcp_chan->w_buf_len, wt_len);
			tcp_chan->w_buf_len -= wt_len;
			memmove(tcp_chan->w_buf, tcp_chan->w_buf + wt_len, tcp_chan->w_buf_len);
					
			tt_cond_signal(&tcp_chan->w_full);
			/* Write status is cleared to wait driver write cmd TCP_SERIAL__START_WRITE. */
			if (tcp_chan->w_buf_len > 0)
			{
//				diag_printf("set wt_set\n");
				FD_SET(tcp_chan->fd, &wt_set);
			}
			else
			{
//				diag_printf("clear wt_set\n");
				FD_CLR(tcp_chan->fd, &wt_set);
			}
			tt_rmutex_unlock(&tcp_chan->buf_mutex);
		}
				
		
		if (!FD_ISSET(tcp_chan->cmd_fd, &rd_set))
		{
			FD_SET(tcp_chan->cmd_fd, &rd_set);
		}
		else
		{
			/* Process command */
			enum TCP_SERIAL_CTRL_CMD ctrl_cmd = tcp_serial_recv_cmd(tcp_chan);
			if (ctrl_cmd == TCP_SERIAL__CONNECT && /*!is_connecting*/tcp_chan->fd < 0)
			{
				int i;
				char			server[64];
				unsigned short	port = 0;
				struct hostent* phostent;
				struct sockaddr_in sin;				
			
				/* Begin to connect */
				tcp_chan->ppp_state = TCP_PPP__TRY_ONLINE;
				
				/* get server address and port */
				cyg_mutex_lock(&tcp_chan->mutex);
				if (++tcp_chan->current_server >= PPP_MAX_SERVERS)
					tcp_chan->current_server = 0;
				/* Get the valid server */
				for (i = 0; i < PPP_MAX_SERVERS; ++i)
				{
					unsigned int index = i + tcp_chan->current_server;
					if (index >= PPP_MAX_SERVERS)
						index -= PPP_MAX_SERVERS;
					
					if (index == tcp_chan->first_server)
					{
						if(tcp_chan->try_count < 6)
							tcp_chan->try_count++;
						else
							tcp_chan->try_count = 6;
					}
						
					if (tcp_chan->servers[index] != NULL && tcp_chan->servers[index] != '\0' && tcp_chan->ports[index] != 0)
					{
						tcp_chan->current_server = index;
						strncpy(server, tcp_chan->servers[index], sizeof(server));
						port = tcp_chan->ports[index];					
						break;
					}
				}
				cyg_mutex_unlock(&tcp_chan->mutex);		
				
				/* Resolve host */
				if (port > 0)
				{
					diag_printf("PPOT: resolve host: %s\n", server);
					/* Resove host */
					phostent = gethostbyname(server);
					if (phostent == (struct hostent *)NULL
						|| phostent->h_addr_list == NULL
						|| *(phostent->h_addr_list) == 0)
					{
						diag_printf("Can not resolve host for TCP serial device: %s\n", server);
						port = 0;	//Set port as a failed flag
					}
					else
					{
						diag_printf("Resolved PPP server: %x\n", *(unsigned long *)*(phostent->h_addr_list));
					}
				}
				
				/* Connect */
				if (port > 0)
				{
					/* Create socket */
					tcp_chan->fd = socket(AF_INET, SOCK_STREAM, 0);
					if (tcp_chan->fd < 0)
					{
						diag_printf("Can not create socket for TCP serial device\n");
					}
					else
					{
						/* Clear write buffer */
						tcp_chan->w_buf_len = 0;
						
						/* Connect socket */
						sin.sin_family = AF_INET;
						sin.sin_port = htons(port);
						sin.sin_addr.s_addr = *(unsigned long *)*(phostent->h_addr_list);
		
						diag_printf("PPOT: connect server:%x\n", sin.sin_addr.s_addr);
						if (connect(tcp_chan->fd, (struct sockaddr *)&sin, sizeof(struct sockaddr)) < 0)
						{
							diag_printf("PPOT: connect server:%x, failed\n", sin.sin_addr.s_addr);
							tcp_serial_close_fd(tcp_chan);
							diag_printf("After close: fd = %d\n", tcp_chan->fd);
						}
						else
						{
							cyg_ppp_options_t options;

							diag_printf("PPOT: connected server:%x\n", sin.sin_addr.s_addr);
							
#if 1
	/* Connect */	
	cyg_ppp_options_init(&options);

	strcpy(options.user, tcp_chan->user);
	strcpy(options.passwd, tcp_chan->passwd);
	//strcpy(options.user, "xiaohui");
	//strcpy(options.passwd, "0gogorovio0");
	//strcpy(options.user, "blah");
	//strcpy(options.passwd, "blahing");
				
	options.debug = 0;
	options.modem = 1;
	options.default_route = 0;
	options.flowctl = CYG_PPP_FLOWCTL_NONE;
	options.refuse_chap = 0;
	options.idle_time_limit = 3600*24*1000;

	tcp_chan->ppp_handle = cyg_ppp_up( "/dev/sert0", &options );
	diag_printf("PPP thread start...\n");
#endif
							//tcp_chan->ppp_online = true;
							tcp_chan->ppp_state = TCP_PPP__ONLINE;
						}


						//is_connecting = true;
						has_first_written = false;
					}
				}
				
				diag_printf("After connect: fd = %d, ppp_state = %d\n", tcp_chan->fd, (int)tcp_chan->ppp_state);
				if (tcp_chan->fd < 0)
				{
					/* Connect failed */
					tcp_serial_send_cmd(TCP_SERIAL__ERROR_DISCONNECT);
				}			
			}
			else if (ctrl_cmd == TCP_SERIAL__DISCONNECT /*&& is_connecting*/
				|| ctrl_cmd == TCP_SERIAL__ERROR_DISCONNECT )
			{
				/* Begin to disconnect */
				if (tcp_chan->fd >= 0)
				{
					FD_CLR(tcp_chan->fd, &rd_set);
					FD_CLR(tcp_chan->fd, &wt_set);
					max_fd = tcp_chan->cmd_fd;
					tcp_serial_close_fd(tcp_chan);
				}

				if(tcp_chan->ppp_handle != NULL)
				{
					// Bring PPP link down
					cyg_ppp_down( tcp_chan->ppp_handle );
					// Wait for connection to go down.
					cyg_ppp_wait_down( tcp_chan->ppp_handle );	
					tcp_chan->ppp_handle = NULL;
				}
				
				//tcp_chan->ppp_online = false;

				//is_connecting = false;
				if (ctrl_cmd == TCP_SERIAL__DISCONNECT)
				{
					tcp_chan->ppp_state = TCP_PPP__OFFLINE;
					tcp_chan->exit_flag = false;
					tcp_chan->reconnect_flag = false;
				}
				
				/* Retry connect here */
				if (ctrl_cmd == TCP_SERIAL__ERROR_DISCONNECT)
				{
					if(tcp_chan->ppp_state == TCP_PPP__TRY_ONLINE
						|| tcp_chan->ppp_state == TCP_PPP__ONLINE
						|| tcp_chan->ppp_state == TCP_PPP__WAIT_RECONNECT)
					{
						tcp_chan->ppp_state = TCP_PPP__WAIT_RECONNECT;
						tcp_chan->exit_flag = false;
						tcp_chan->reconnect_flag = true;

						diag_printf("PPP will connect in %d seconds....\n", (1 << tcp_chan->try_count));
					}
				}
			}
			else if (ctrl_cmd == TCP_SERIAL__START_WRITE && /*is_connecting*/ tcp_chan->fd >= 0)
			{
				/* Add fd to rd_set and then select */
				if (tcp_chan->fd >= 0)
				{
					FD_SET(tcp_chan->fd, &wt_set);
					if (max_fd < tcp_chan->fd)
					{
						max_fd = tcp_chan->fd;
//						diag_printf("set max_fd\n");
					}
				}
			}
		}
		


		
		//if (!transferred)
		if (/*is_connecting*/tcp_chan->fd >= 0)
			(chan->callbacks->xmt_char)(chan);//serial_xmt_char
	}
	
}

#endif


void tcp_serial_send_cmd(enum TCP_SERIAL_CTRL_CMD ctrl_cmd)
{
	int cmd = ctrl_cmd;

	if (tcp_serial_info0.cmd_fd < 0)
	{
		tcp_serial_info0.cmd_fd = pipe_create(NULL);
		diag_printf("pipe fd for ppp is=%d\n", tcp_serial_info0.cmd_fd);
		if(tcp_serial_info0.cmd_fd < 0)
		{
			diag_printf("Can not create pipe cmd fd for tcp serial driver\n");
			while(1);
		}

		cyg_thread_create(10, &tcp_serial_thread1, (cyg_addrword_t)&tcp_serial_channel0, "tcp_serial1", g_tcp_serial_thread_stack1,
			sizeof(g_tcp_serial_thread_stack1), &g_tcp_serial_handle1, &g_tcp_serial_thread1);			
		cyg_thread_resume(g_tcp_serial_handle1);

		cyg_thread_create(10, &tcp_serial_thread, (cyg_addrword_t)&tcp_serial_channel0, "tcp_serial", g_tcp_serial_thread_stack,
			sizeof(g_tcp_serial_thread_stack), &g_tcp_serial_handle, &g_tcp_serial_thread);			
		cyg_thread_resume(g_tcp_serial_handle);
		
//		diag_printf("ppp thread id:%d %d\n", (int)g_tcp_serial_thread1.unique_id, (int)g_tcp_serial_thread.unique_id);
	}
	

//	diag_printf("before write data to fd %d\n", tcp_serial_info0.cmd_fd);
	write(tcp_serial_info0.cmd_fd, &cmd, sizeof(cmd));
//	diag_printf("after write data to fd %d\n", tcp_serial_info0.cmd_fd);
}


enum TCP_SERIAL_CTRL_CMD tcp_serial_recv_cmd(tcp_serial_info *tcp_chan)
{
	int cmd;
	read(tcp_serial_info0.cmd_fd, &cmd, sizeof(cmd));
	return (enum TCP_SERIAL_CTRL_CMD)cmd;
}


void ppot_disconnect()
{
	//if(!tcp_serial_info0.ppp_try_online)
	//	return;

	//cyg_mutex_lock(&tcp_serial_info0.mutex);
	//tcp_serial_info0.ppp_try_online = false;
	//cyg_mutex_unlock(&tcp_serial_info0.mutex);

	tcp_serial_send_cmd(TCP_SERIAL__DISCONNECT);
	
	//while(tcp_serial_info0.ppp_try_online)
	//	cyg_thread_delay(100);
}

bool ppot_is_connecting()
{
	if(tcp_serial_info0.ppp_state == TCP_PPP__TRY_ONLINE)
		return true;
	else
		return false;
	//return tcp_serial_info0.ppp_try_online;
}

bool ppot_is_online()
{
	if(tcp_serial_info0.ppp_state == TCP_PPP__ONLINE)
		return true;
	else
		return false;
	//return tcp_serial_info0.ppp_online;
}


bool ppot_connect(const char * const*servers, const int *ports, int num, const char *username, const char *password)
{
	//bool ppp_try_online;
	enum TCP_PPP_STATE_E ppp_state;
	
	/* Check the parameters */
	if (servers == NULL
		|| ports == NULL
		|| username == NULL
		|| password == NULL
		)
		return false;
		
	ppot_disconnect();
	
	/* Check if it's already connect. */
	cyg_mutex_lock(&tcp_serial_info0.mutex);
	//ppp_try_online = tcp_serial_info0.ppp_try_online;
	//tcp_serial_info0.ppp_try_online = true;	
	ppp_state = tcp_serial_info0.ppp_state;
	tcp_serial_info0.ppp_state = TCP_PPP__TRY_ONLINE;	
	cyg_mutex_unlock(&tcp_serial_info0.mutex);
	
	//if(ppp_try_online)
	//	return false;
	if(ppp_state == TCP_PPP__ONLINE
		|| ppp_state == TCP_PPP__TRY_ONLINE
		|| ppp_state == TCP_PPP__WAIT_RECONNECT)
		return false;
	
	cyg_mutex_lock(&tcp_serial_info0.mutex);
	{
		int i;
		for (i = 0; i < PPP_MAX_SERVERS && i < num; ++i)
		{
			strncpy(tcp_serial_info0.servers[i], servers[i], sizeof(tcp_serial_info0.servers[i]));
			tcp_serial_info0.ports[i] = ports[i];
		}
		for (; i < PPP_MAX_SERVERS; ++i )
		{
			tcp_serial_info0.servers[i][0] = '\0';
			tcp_serial_info0.ports[i] = 0;
		}
		tcp_serial_info0.current_server = rand() % PPP_MAX_SERVERS;
		tcp_serial_info0.first_server = (tcp_serial_info0.current_server + 1) % PPP_MAX_SERVERS;
		tcp_serial_info0.try_count = 0;
	}
	strncpy(tcp_serial_info0.user, username, sizeof(tcp_serial_info0.user));
	strncpy(tcp_serial_info0.passwd, password, sizeof(tcp_serial_info0.passwd));
	tcp_serial_info0.user[sizeof(tcp_serial_info0.user) - 1] = '\0';
	tcp_serial_info0.passwd[sizeof(tcp_serial_info0.passwd) - 1] = '\0';	
	
	cyg_mutex_unlock(&tcp_serial_info0.mutex);
	

#if 0
	/* Connect */	
	cyg_ppp_options_init(&options);

	strcpy(options.user, username);
	strcpy(options.passwd, password);
	//strcpy(options.user, "xiaohui");
	//strcpy(options.passwd, "0gogorovio0");
	//strcpy(options.user, "blah");
	//strcpy(options.passwd, "blahing");
				
	options.debug = 0;
	options.modem = 1;
	options.default_route = 0;
	options.flowctl = CYG_PPP_FLOWCTL_NONE;
	options.refuse_chap = 0;
	options.idle_time_limit = 3600*24*1000;

	tcp_serial_info0.ppp_handle = cyg_ppp_up( "/dev/sert0", &options );
	diag_printf("PPP thread start...\n");
#endif

	tcp_serial_send_cmd(TCP_SERIAL__CONNECT);
	return true;	
}


#endif

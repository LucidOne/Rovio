

#include "wb_syslib_addon.h"
#include "clk.h"
#include "pll.h"

#include "errno.h"
#include "fcntl.h"
#include "limits.h"				/* OPEN_MAX */
#include "stdlib.h"				/* malloc, free, etc. */
#include "stdio.h"				/* stdin, stdout, stderr */
#include "string.h"				/* strdup */
#include "time.h"				/* localtime, time */


#include "unistd.h"
#include "sys/ioctl.h"
#include "sys/types.h"			/* socket, bind, accept */
#include "sys/stat.h"			/* open */
#include "wb_syslib_addon.h"
#include "wbfat.h"
#include "sys/select.h"
#include "netinet/in.h"			/* sockaddr_in, sockaddr */
#include "arpa/inet.h"			/* inet_ntoa */
#include "sys/time.h"			/* select */
#include "sys/types.h"			/* socket, bind, accept */
#include "sys/socket.h"			/* socket, bind, accept, setsockopt, */
#include "net/if.h"
#include "network.h"
#include "pkgconf/libc_startup.h"
#include "netinet/if_ether.h"
#include "cyg/io/eth/netdev.h"
#include "net/wireless.h"

#include "netinet/in.h"
#include "netinet/tcp.h"
#include "netdb.h"
#include "CommonDef.h"




#define THREAD_STACK_SIZE (16*1024)
#define WIRELESS_TEST_SERVER_BEGIN_PORT 3000
#define MAX_INT_NUM	(1*1024)
#define MAX_FD_NUM	3

typedef enum
{
	CLIENT_WRITE,
	CLIENT_READ
} CLIENT_STATE;
 

typedef struct
{
	int fd;				/* file descriptor */
	time_t last_io_time;/* last io time */
	CLIENT_STATE state;	/* read state or write state. */
	int total_length;	/* total buffer length to be read or write. */
	int io_length;		/* length that has already been read or write. */
	
	struct
	{
		int number;
		int digit[MAX_INT_NUM];
	} buffer;
} CLIENT_T;


typedef struct {
	// For data thread
	cyg_handle_t	datathread_handle;
	cyg_thread		datathread_thread;
	char			datathread_stack[THREAD_STACK_SIZE];
	int				fd_listen;
	CLIENT_T fd_array[MAX_FD_NUM];
}WIRELESS_TEST_T;





#define MAX_IO_BUFFER (50000)

int my_read( int fd, char *buffer, size_t size )
{
	if ( size > MAX_IO_BUFFER - 4096 )
		size = 	MAX_IO_BUFFER - 4096;
	return read( fd, buffer, size );
}


int my_write( int fd, const char *buffer, size_t size )
{
	if ( size > MAX_IO_BUFFER - 4096 )
		size = 	MAX_IO_BUFFER - 4096;
	return write( fd, buffer, size );
}




int create_listen_socket( int port )
{
	int fd;
	int sock_opt;
	struct sockaddr_in r_sa;
	
	r_sa.sin_addr.s_addr	= htonl(INADDR_ANY);
	r_sa.sin_family			= AF_INET;
	r_sa.sin_port			= htons(port);
	diag_printf( "create_listen_socket: addr=%s port %d\n", 
		inet_ntoa( r_sa.sin_addr ),
		ntohs( r_sa.sin_port ) );

	
	if ( ( fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP ) ) == -1 )
	{
		diag_printf( "create_listen_socket: socket error!!!!!!\n" );
		goto error_create;
	}
	diag_printf( "create_listen_socket: create socket success...\n" );
	
	
	if (1)
	{
		int value = 1;
		if ( ioctl( fd, FIONBIO, &value ) == -1 )
		{
			diag_printf( "create_listen_socket: ioctl error!\n" );
			goto error_fd;
		}
	}
	
	if ( ( setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, (void *) &sock_opt,
		sizeof(sock_opt))) == -1 )
	{
		diag_printf( "setsockopt error\n" );
		goto error_fd;
	}

	if ( bind( fd, (struct sockaddr*)&r_sa, sizeof(r_sa)) == -1 )
	{
		diag_printf("create_listen_socket: bind error!!!!!!\n" );
		goto error_fd;
	}
	diag_printf( "create_listen_socket: bind success...\n" );

	if ( listen( fd, 10 ) == -1 )
	{
		diag_printf( "create_listen_socket: listen error\n" );
		goto error_fd;
	}
	diag_printf( "create_listen_socket: listen success...\n" );

	return fd;

error_fd:
	close( fd );
	fd = -1;
error_create:
	return fd;
}



int accept_client( int fd_listen )
{
	int sockbufsize;
	int fd;						/* socket */
	int sock_opt;
	struct sockaddr_in remote_addr;		/* address */
	int remote_addrlen = sizeof(remote_addr);

	diag_printf( "begin to accept\n");
	fd = accept( fd_listen, (struct sockaddr *) &remote_addr, (size_t*)&remote_addrlen );
	if ( fd == -1 )
	{
		diag_printf( "accept_client: accept error\n" );
		goto error_accept;
	}
	
	sock_opt = 1;
	if ( ( setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, (void *) &sock_opt,
		sizeof(sock_opt))) == -1 )
	{
		diag_printf( "accept_client: setsockopt(SO_REUSEADDR) error\n" );
		goto error_fd;
	}	

	if(0)
	{
		int value = 1;
		if ( ioctl( fd, FIONBIO, &value ) == -1 )
		{
			diag_printf( "accept_client: ioctl(FIONBIO) error!\n" );
			goto error_fd;
		}
	}
	
	
	sockbufsize = MAX_IO_BUFFER;
	if ( setsockopt( fd, SOL_SOCKET, SO_SNDBUF, (void *) &sockbufsize,
		sizeof(sockbufsize) ) == -1 )
	{
		diag_printf( "accept_client: setsockopt(SO_SNDBUF) error\n" );
	}
	sockbufsize = MAX_IO_BUFFER;
	if ( setsockopt( fd, SOL_SOCKET, SO_RCVBUF, (void *) &sockbufsize,
		sizeof(sockbufsize) ) == -1 )
	{
		diag_printf( "accept_client: setsockopt(SO_RCVBUF) error\n" );
	}


	
	sock_opt = 1;
	if ( setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, (void *) &sock_opt,
			sizeof(sock_opt) ) == -1 )
	{
		diag_printf( "accept_client: setsockopt(TCP_NODELAY) error\n" );
		goto error_fd;
	}
	
	return fd;
		
error_fd:
	close( fd );
	fd = -1;
error_accept:
	return fd;
}





void client_init_for_write( CLIENT_T *client )
{
	int i;

	client->state			= CLIENT_WRITE;
	client->buffer.number	= rand( ) % MAX_INT_NUM;
	client->total_length	= sizeof( client->buffer.number ) + client->buffer.number * sizeof( int );
	client->io_length	= 0;

	for ( i = 0; i < client->buffer.number; i++ )
		client->buffer.digit[i] = i;
	
	//diag_printf( "Send %d number of integers.\n", client->buffer.number );
	
}


void client_init_for_read( CLIENT_T *client )
{
	client->state		= CLIENT_READ;
	client->io_length	= 0;
}


void client_io_time_reset( CLIENT_T *client )
{
	client->last_io_time = time( 0 );
}


BOOL client_io_time_is_expired( CLIENT_T *client )
{
	if ( time( 0 ) - client->last_io_time > 8 )
		return TRUE;
	else
		return FALSE;
}


void client_init( CLIENT_T *client, int fd )
{
	client->fd = fd;
	client->total_length = 0;
	client->io_length = 0;
	client_io_time_reset( client );

	client_init_for_write( client );
}


void client_uninit( CLIENT_T *client )
{
	diag_printf( "client closed\n" );
	close( client->fd );
	client->fd = -1;
}


void do_client_write( CLIENT_T *client )
{
	/* Write the integers to client. */
	if ( client->io_length < client->total_length )
	{
		int length_write;
		int length =  client->total_length - client->io_length;
		//diag_printf( "About write %d bytes at position %d\n", length, client->io_length );
		errno = 0;
		length_write = my_write( client->fd, (char *)&client->buffer + client->io_length, length );
		if ( length_write < 0 )
		{
			if ( errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR )
			{
				diag_printf( "Write return %d with errno = %d\n", length_write, errno );
				return;			/* request blocked at the pipe level, but keep going */
			}
			else
			{
				diag_printf( "error in writing client\n" );
				client_uninit( client );
				return;
			}
		}
		else if (length_write == 0 )
		{
			diag_printf( "Close by peer\n");
			client_uninit( client );
			return;
		}
		else 
		{
			//diag_printf( "Acturally send %d bytes\n", length_write );
			client->io_length += length_write;
		}
	}
	
	if ( client->io_length >= client->total_length )
	{
		//diag_printf( "write finished\n" );
		//client_uninit( client );
		client_init_for_read( client );
	}
}


void do_client_read( CLIENT_T *client )
{
	/* Read the integers number firstly. */
	if ( client->io_length < sizeof( int ) )
	{
		int length_read;
		int length = sizeof( int ) - client->io_length;
		//diag_printf( "About read %d bytes at position %d\n", length, client->io_length );
		errno = 0;
		length_read = my_read( client->fd, (char *)&client->buffer + client->io_length, length );
		if ( length_read < 0 )
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR )
			{
				diag_printf( "Read return %d with errno = %d\n", length_read, errno );
				return;			/* request blocked at the pipe level, but keep going */
			}
			else
			{
				diag_printf( "error in reading client\n" );
				client_uninit( client );
				return;
			}
		}
		else if (length_read == 0 )
		{
			diag_printf( "Close by peer\n");
			client_uninit( client );
			return;
		}
		else
		{
			//diag_printf( "Acturally read %d bytes\n", length_read );
			client->io_length += length_read;
		}

		if ( client->io_length >= sizeof( int ) )
		{
			if ( client->buffer.number > MAX_INT_NUM )
			{
				diag_printf( "Too long client data\n" );
				client_uninit( client );
				return;
			}
			client->total_length = sizeof( client->buffer.number ) + client->buffer.number * sizeof( int );
		}	
	}
	/* Read the integers. */
	else if ( client->io_length < client->total_length )
	{
		int length_read;
		int length = client->total_length - client->io_length;
		
		//diag_printf( "About read %d bytes at position %d\n", length, client->io_length );
		length_read = my_read( client->fd, (char *)&client->buffer + client->io_length, length );
		if ( length_read < 0 )
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR )
				return;			/* request blocked at the pipe level, but keep going */
			else
			{
				diag_printf( "error in reading client\n" );
				client_uninit( client );
				return;
			}
		}
		else if (length_read == 0 )
		{
			diag_printf( "Close by peer\n");
			client_uninit( client );
			return;
		}
		else
		{
			//diag_printf( "Acturally read %d bytes\n", length_read );
			client->io_length += length_read;
		}
	}
	
	if ( client->io_length >= client->total_length )
	{
		int i;
		//diag_printf( "Read finished\n" );
		
		/* Check the read buffer. */
		for ( i = 0; 0 && i < client->buffer.number; i++ )
		{
			if ( client->buffer.digit[i] != i )
			{
				diag_printf( "The read buffer is not correct!\n" );
				client_uninit( client );
				while( 1 );	//Block here
			}
		}
	
		
		
		client_init_for_write( client );
	}
}



void do_client( CLIENT_T *client )
{
	switch ( client->state )
	{
		case CLIENT_WRITE:
			do_client_write( client );
			break;
		case CLIENT_READ:
			do_client_read( client );
			break;
		default:;
			diag_printf( "Unknown client status\n" );
	}
}

void WirelessTest_Test_Entry( cyg_addrword_t arg )
{
	int i;
	int fd_listen;
	int fd_max;
	fd_set r_set;
	fd_set w_set;
	
	WIRELESS_TEST_T *thread_cfg = (WIRELESS_TEST_T *)arg;
	srand( time( 0 ) );
	
	
	/* Clear the fd_array. */
	for ( i = 0; i < sizeof( thread_cfg->fd_array ) / sizeof( thread_cfg->fd_array[0] ); i++ )
	{
		thread_cfg->fd_array[i].fd			= -1;
	}
	
	/* The listening socket. */
	fd_listen = thread_cfg->fd_listen;
		
	/* Do select loop */
	while ( 1 )
	{
		FD_ZERO( &r_set );
		FD_ZERO( &w_set );
	
		/* Update fd_set */
		FD_SET( fd_listen, &r_set );
		fd_max = fd_listen;
		for ( i = 0; i < sizeof( thread_cfg->fd_array ) / sizeof( thread_cfg->fd_array[0] ); i++ )
		{
			if ( thread_cfg->fd_array[i].fd != -1 )
			{
				if ( thread_cfg->fd_array[i].state == CLIENT_WRITE )
				{
					if ( client_io_time_is_expired( &thread_cfg->fd_array[i] ) )
					{
diag_printf( "connection timeout, clean client\n" );						
						client_uninit( &thread_cfg->fd_array[i] );
					}
					else
					{
//diag_printf( "Add write set, fd = %d, state = %d\n", thread_cfg->fd_array[i].fd, thread_cfg->fd_array[i].state );
						FD_SET( thread_cfg->fd_array[i].fd, &w_set );
					}
				}
				else if ( thread_cfg->fd_array[i].state == CLIENT_READ )
				{
					if ( client_io_time_is_expired( &thread_cfg->fd_array[i] ) )
					{
diag_printf( "connection timeout, clean client\n" );						
						client_uninit( &thread_cfg->fd_array[i] );
					}
					else
					{
//diag_printf( "Add read set, fd = %d, state = %d\n", thread_cfg->fd_array[i].fd, thread_cfg->fd_array[i].state );
						FD_SET( thread_cfg->fd_array[i].fd, &r_set );
					}
				}
	
				if ( fd_max < thread_cfg->fd_array[i].fd )
					fd_max = thread_cfg->fd_array[i].fd;
			}
		}
		
		
	
		if ( ( select(fd_max + 1, &r_set, &w_set, NULL, NULL ) ) == -1 )
		{
			diag_printf( "select error\n" );
			continue;
		}
		
		/* Process each clients */
		for ( i = 0; i < sizeof( thread_cfg->fd_array ) / sizeof( thread_cfg->fd_array[0] ); i++ )
		{
			if ( thread_cfg->fd_array[i].fd != -1 )
			{
				if ( FD_ISSET( thread_cfg->fd_array[i].fd, &w_set )
				  || FD_ISSET( thread_cfg->fd_array[i].fd, &r_set) )
				{
					client_io_time_reset( &thread_cfg->fd_array[i] );
					do_client( &thread_cfg->fd_array[i] );
				}
			}
		}
		
		/* Check listening fd. */
		if ( FD_ISSET( fd_listen, &r_set ) )
		{
			int fd = accept_client( fd_listen );
			for ( i = 0; i < sizeof( thread_cfg->fd_array ) / sizeof( thread_cfg->fd_array[0] ); i++ )
			{
				if ( thread_cfg->fd_array[i].fd == -1 )
					break;
			}
			
			if ( i < sizeof( thread_cfg->fd_array ) / sizeof( thread_cfg->fd_array[0] ) )
			{
				client_init( &thread_cfg->fd_array[i], fd );
				diag_printf( "Init thread_cfg->fd_array[%d].fd = %d %d\n", i, fd, thread_cfg->fd_array[i].fd);
			}
			else
			{
				close( fd );
				diag_printf( "No fd slot available!\n" );
			}
		}
		
		
		
	}
}





static WIRELESS_TEST_T *g_pWirelessTest = NULL;
static BOOL TestNet_ThreadInit(unsigned short usPort)
{
	if (g_pWirelessTest != NULL)
		goto error_alread_inited;
		
	g_pWirelessTest = (WIRELESS_TEST_T *) malloc(sizeof(WIRELESS_TEST_T));
	if (g_pWirelessTest == NULL)
		goto error_malloc;

	/* Create listening socket. */
	g_pWirelessTest->fd_listen = create_listen_socket( usPort );
	if ( g_pWirelessTest->fd_listen == -1 )
		goto error_listen;
	
	cyg_thread_create(PTD_PRIORITY-3, WirelessTest_Test_Entry, (cyg_addrword_t)g_pWirelessTest, "test_net", 
					g_pWirelessTest->datathread_stack, sizeof(g_pWirelessTest->datathread_stack), 
					&g_pWirelessTest->datathread_handle, &g_pWirelessTest->datathread_thread);

	cyg_thread_resume(g_pWirelessTest->datathread_handle);
	

	return TRUE;
error_listen:
	free(g_pWirelessTest);
error_malloc:
error_alread_inited:
	return FALSE;
}


static BOOL TestNet_ThreadUninit(void)
{
	cyg_thread_info infoTestNet;
	if (g_pWirelessTest == NULL)
		return FALSE;
	
	thread_join(&g_pWirelessTest->datathread_handle,&g_pWirelessTest->datathread_thread,&infoTestNet);
	free(g_pWirelessTest);
	g_pWirelessTest = NULL;
}





int Config_EnableTestNet(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	unsigned short usPort;
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
	case CA_CONFIG:
		usPort = httpGetLong(pParamList, "port");
		if (usPort >= 100 && TestNet_ThreadInit(usPort))
			AddHttpValue(pReturnXML, "Result", "true");
		else
			AddHttpValue(pReturnXML, "Result", "false");
		return 0;
	}
	return -1;
}

#ifndef __TEST_TCP_SOCKET_H__
#define __TEST_TCP_SOCKET_H__

typedef struct
{
	int		fd;
	int		iport;
	char	*pbuf;
}TEST_TCP_SOCKET_DATA_T;

void test_tcp_socket(char *pcBuf, int iBufLen);
void test_tcp_create_socket(char *pcBuf, int iBufLen);
void test_tcp_bind_socket(char *pcBuf, int iBufLen);
void test_tcp_listen_socket(char *pcBuf, int iBufLen);
void test_tcp_accept_socket(void);
void test_tcp_socket_server(cyg_addrword_t pnetdata);
void test_tcp_socket_client(cyg_addrword_t pnetdata);

#endif

#ifndef __TEST_TCP_SOCKET_H__
#define __TEST_TCP_SOCKET_H__

typedef struct
{
	int		fd;
	int		iport;
	char	*pbuf;
}TEST_TCP_SOCKET_DATA_T;

void test_socket(char *pcBuf, int iBufLen);
void test_create_socket(char *pcBuf, int iBufLen);
void test_bind_socket(char *pcBuf, int iBufLen);
void test_listen_socket(char *pcBuf, int iBufLen);
void test_accept_socket(char *pcBuf, int iBufLen);

#endif

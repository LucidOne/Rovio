#ifndef __TEST_RECV_H__
#define __TEST_RECV_H__

typedef struct
{
	int		fd;
	int		iport;
	char	*pbuf;
	char	*precvbuf;
}TEST_RECV_DATA_T;

void test_recv(void);
void test_recv_entry(void);
void test_recv_server(cyg_addrword_t pnetdata);


#endif

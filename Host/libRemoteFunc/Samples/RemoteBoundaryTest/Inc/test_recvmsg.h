#ifndef __TEST_RECVMSG_H__
#define __TEST_RECVMSG_H__

typedef struct
{
	int		fd;
	int		iport;
	char	*pbuf;
	char	*precvmsgbuf;
}TEST_RECVMSG_DATA_T;

void test_recvmsg(void);
void test_recvmsg_entry(void);
void test_recvmsg_server(cyg_addrword_t pnetdata);
void test_recvmsg_client(cyg_addrword_t pnetdata);


#endif

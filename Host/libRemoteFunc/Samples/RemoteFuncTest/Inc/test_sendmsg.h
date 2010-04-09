#ifndef __TEST_SENDMSG_H__
#define __TEST_SENDMSG_H__

typedef struct
{
	int		fd;
	int		iport;
	char	*pbuf;
}TEST_SENDMSG_DATA_T;

void test_sendmsg(void);
void test_sendmsg_entry(void);
void test_sendmsg_server(cyg_addrword_t pnetdata);


#endif

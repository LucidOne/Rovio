#ifndef __TEST_NETFCNTL_H__
#define __TEST_NETFCNTL_H__

typedef struct
{
	int		fd;
	int		iport;
	char	*pbuf;
	char	*pnetfcntlbuf;
}TEST_NETFCNTL_DATA_T;

void test_netfcntl(void);
void test_netfcntl_entry(void);
void test_netfcntl_server(cyg_addrword_t pnetdata);


#endif

#ifndef __TEST_NETWRITE_H__
#define __TEST_NETWRITE_H__

typedef struct
{
	int		fd;
	int		iport;
	char	*pbuf;
	char	*pwritebuf;
}TEST_NETWRITE_DATA_T;

void test_netwrite(void);
void test_netwrite_entry(void);
void test_netwrite_server(cyg_addrword_t pnetdata);
void test_netwrite_client(cyg_addrword_t pnetdata);


#endif

#ifndef __TEST_NETREAD_H__
#define __TEST_NETREAD_H__

typedef struct
{
	int		fd;
	int		iport;
	char	*pbuf;
	char	*preadbuf;
}TEST_NETREAD_DATA_T;

void test_netread(void);
void test_netread_entry(void);
void test_netread_server(cyg_addrword_t pnetdata);


#endif

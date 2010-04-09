#ifndef __TEST_NETSELECT_H__
#define __TEST_NETSELECT_H__

typedef struct
{
	int		fd;
	int		iport;
	char	*pbuf;
	char	*pnetselectbuf;
}TEST_NETSELECT_DATA_T;

void test_netselect(void);
void test_netselect_entry(void);
void test_netselect_server(cyg_addrword_t pnetdata);


#endif

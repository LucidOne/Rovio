#ifndef __TEST_GETSOCKNAME_H__
#define __TEST_GETSOCKNAME_H__

typedef struct
{
	int		fd;
	int		iport;
	char	*pbuf;
	char	*pgetsocknamebuf;
}TEST_GETSOCKNAME_DATA_T;

void test_getsockname(void);
void test_getsockname_entry(void);
void test_getsockname_server(cyg_addrword_t pnetdata);
void test_getsockname_client(cyg_addrword_t pnetdata);


#endif

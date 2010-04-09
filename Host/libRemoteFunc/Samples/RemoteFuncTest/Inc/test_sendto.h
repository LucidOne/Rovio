#ifndef __TEST_SENDTO_H__
#define __TEST_SENDTO_H__

typedef struct
{
	int		fd;
	int		iport;
	char	*pbuf;
	char	*psendtobuf;
}TEST_SENDTO_DATA_T;

void test_sendto(void);
void test_sendto_entry(void);
void test_sendto_server(cyg_addrword_t pnetdata);


#endif

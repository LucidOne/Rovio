#ifndef __TEST_SEND_H__
#define __TEST_SEND_H__

typedef struct
{
	int		fd;
	int		iport;
	char	*pbuf;
	char	*psendbuf;
}TEST_SEND_DATA_T;

void test_send(void);
void test_send_entry(void);
void test_send_server(cyg_addrword_t pnetdata);


#endif

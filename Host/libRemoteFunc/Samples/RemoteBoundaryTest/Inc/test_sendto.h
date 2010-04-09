#ifndef __TEST_SENDTO_H__
#define __TEST_SENDTO_H__

typedef struct
{
	int		fd;
	int		iport;
	char	*pbuf;
	char	*psendtobuf;
}TEST_SENDTO_DATA_T;

void test_sendto(char *pBuf, int iBufLen);
void test_sendto_entry(char *pBuf, int iBufLen);


#endif

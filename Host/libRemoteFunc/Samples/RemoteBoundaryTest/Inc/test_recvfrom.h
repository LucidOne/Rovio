#ifndef __TEST_RECVFROM_H__
#define __TEST_RECVFROM_H__

typedef struct
{
	int		fd;
	int		iport;
	char	*pbuf;
	char	*precvfrombuf;
}TEST_RECVFROM_DATA_T;

void test_recvfrom(char *pBuf, int iBufLen);
void test_recvfrom_entry(char *pBuf, int iBufLen);


#endif

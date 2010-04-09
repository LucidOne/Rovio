#ifndef __TEST_GETHOSTBYNAME_H__
#define __TEST_GETHOSTBYNAME_H__

void test_gethost(char *pcBuf, int iBufLen);
void test_gethostbyname(char *pcBuf, int iBufLen);
void test_gethostbyaddr(char *pcBuf, int iBufLen);
void test_gethostname(char *pcBuf, int iBufLen);
void test_uname(char *pcBuf, int iBufLen);

void test_getservbyname(char *pcBuf, int iBufLen);
void test_getservbyport(char *pcBuf, int iBufLen);

#endif

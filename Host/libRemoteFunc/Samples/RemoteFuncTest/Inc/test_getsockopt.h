#ifndef __TEST_GETSOCKOPT_H__
#define __TEST_GETSOCKOPT_H__

void test_getsockopt(char *pBuf, int pBufLen);
void test_getsockopt_entry(int fd, char *pBuf, int iBufLen);
void test_setsockopt_entry(int fd, char *pBuf, int iBufLen);
void test_setsockopt_reuseaddr(char *pBuf, int iBufLen);
int set_reuseaddr(int fd, char *pBuf, int iBufLen);

#endif

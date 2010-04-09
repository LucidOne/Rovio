#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "test_remotefunc.h"

void *test_sendto_client(void *parg);

void test_sendto_entry(void)
{
    pthread_t sendto_client[TEST_SENDTO_CLIENT_NUM];
    int thread_arg[TEST_SENDTO_CLIENT_NUM];
    int i;
    
    for(i = 0; i < TEST_SENDTO_CLIENT_NUM; i++)
    {
        thread_arg[i] = TEST_SENDTO_SERVER_BEGIN_PORT + i;
        pthread_create(&sendto_client[i], 0, &test_sendto_client, &thread_arg[i]);
    }
    
    for(i = 0; i < TEST_SENDTO_CLIENT_NUM; i++)
    {
        pthread_join(sendto_client[i], NULL);
    }
}

void *test_sendto_client(void *parg)
{
    int s, i, recvlen;
    struct sockaddr_in sa = {0}, r_sa = {0};
	unsigned int r_sa_l = sizeof(r_sa);
    char preadbuf[TEST_SENDTO_MSG_LEN];
    int port = *(int*)parg;
    
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(IPPORT_USERRESERVED + port);
    
    if((s = socket(AF_INET, SOCK_DGRAM, PF_UNSPEC)) == -1)
    {
        printf("socket error\n");
        return NULL;
    }
    printf("socket success...\n");
    
    set_reuseaddr(s);
    if(bind(s, (struct sockaddr*)&sa, sizeof(sa)) == -1)
    {
        printf("bind error\n");
        close(s);
        return NULL;
    }
    printf("bind success...\n");
    
	for(i = 0; i < TEST_SENDTO_READ_TIMES; i++)
	{
		recvlen = recvfrom(s, preadbuf, TEST_SENDTO_MSG_LEN, 0, (struct sockaddr*)&r_sa, &r_sa_l);
		if(recvlen < 0)
		{
			printf("netread error(%d)\n", errno);
			break;
		}
		if(recvlen == 0)
		{
			printf("connection broken\n");
			break;
		}
		if(strcmp(preadbuf, MYREQUEST) != 0)
		{
			printf("read content error\n");
			printf("error content %d(\"%s\")\n", recvlen, preadbuf);
            close(s);
            return NULL;
		}
	}
    printf("read all ok\n");
    
    close(s);
    return NULL;
 }

int main(int argc, char *argv[])
{
    test_sendto_entry();
    
    return 0;
}

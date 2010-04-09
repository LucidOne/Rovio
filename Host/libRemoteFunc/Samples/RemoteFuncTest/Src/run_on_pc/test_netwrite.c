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

void *test_netwrite_client(void *parg);

void test_netwrite_entry(void)
{
    pthread_t netwrite_client[TEST_NETWRITE_CLIENT_NUM];
    int thread_arg[TEST_NETWRITE_CLIENT_NUM];
    int i;
    
    for(i = 0; i < TEST_NETWRITE_CLIENT_NUM; i++)
    {
        thread_arg[i] = TEST_NETWRITE_SERVER_BEGIN_PORT + i;
        pthread_create(&netwrite_client[i], 0, &test_netwrite_client, &thread_arg[i]);
    }
    
    for(i = 0; i < TEST_NETWRITE_CLIENT_NUM; i++)
    {
        pthread_join(netwrite_client[i], NULL);
    }
}

void *test_netwrite_client(void *parg)
{
    int s, new_s, readlen;
    struct sockaddr_in sa = {0}, r_sa = {0};
	int r_sa_l = sizeof(r_sa);
    char preadbuf[TEST_NETWRITE_MSG_LEN];
    int port = *(int*)parg;
    
    if(inet_aton(TEST_NETWRITE_SERVER_ADDR, &sa.sin_addr) == 0)
    {
        printf("inet_aton error\n");
        return NULL;
    }
    sa.sin_family = AF_INET;
    sa.sin_port = htons(IPPORT_USERRESERVED + port);
    
    if((s = socket(AF_INET, SOCK_STREAM, PF_UNSPEC)) == -1)
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
    
	if(listen(s, 10) == -1)
	{
		printf("listen error\n");
		close(s);
		return NULL;
	}
    printf("listen success...\n");
	
	if((new_s = accept(s, (struct sockaddr*)&r_sa, (size_t*)&r_sa_l)) == -1)
	{
		printf("accept error\n");
		close(s);
		return NULL;
	}
    printf("accept success...\n");
	
	while(1)
	{
		readlen = read(new_s, preadbuf, TEST_NETWRITE_MSG_LEN);
		if(readlen < 0)
		{
			printf("read error(%d)\n", errno);
			break;
		}
		if(readlen == 0)
		{
			printf("connection broken\n");
			break;
		}
		if(strcmp(preadbuf, MYREQUEST) != 0)
		{
			printf("read content error\n");
			printf("error content %d(\"%s\")\n", readlen, preadbuf);
		}
	}
    printf("read all ok\n");
    
    close(new_s);
    close(s);
    return NULL;
 }

int main(int argc, char *argv[])
{
    test_netwrite_entry();
    
    return 0;
}

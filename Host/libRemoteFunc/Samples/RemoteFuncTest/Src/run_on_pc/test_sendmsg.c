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

void *test_sendmsg_client(void *parg);

void test_sendmsg_entry(void)
{
    pthread_t sendmsg_client[TEST_SENDMSG_CLIENT_NUM];
    int thread_arg[TEST_SENDMSG_CLIENT_NUM];
    int i;
    
    for(i = 0; i < TEST_SENDMSG_CLIENT_NUM; i++)
    {
        thread_arg[i] = TEST_SENDMSG_SERVER_BEGIN_PORT + i;
        pthread_create(&sendmsg_client[i], 0, &test_sendmsg_client, &thread_arg[i]);
    }
    
    for(i = 0; i < TEST_SENDMSG_CLIENT_NUM; i++)
    {
        pthread_join(sendmsg_client[i], NULL);
    }
}

void *test_sendmsg_client(void *parg)
{
    int s, new_s, recvlen;
    struct sockaddr_in sa = {0}, r_sa = {0};
	int r_sa_l = sizeof(r_sa);
    char preadbuf[TEST_SENDMSG_MSG_LEN];
    int port = *(int*)parg;
    
    if(inet_aton(TEST_SENDMSG_SERVER_ADDR, &sa.sin_addr) == 0)
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
		recvlen = recv(new_s, preadbuf, TEST_NETREAD_MSG_LEN, 0);
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
        if(recvlen > 0)
        {
            if(strcmp(preadbuf, MYREQUEST) != 0)
            {
                printf("read content error\n");
                printf("error content %d(\"%s\")\n", recvlen, preadbuf);
                close(new_s);
                close(s);
                return NULL;
            }
        }
	}
    printf("read all ok\n");
    
    close(new_s);
    close(s);
    return NULL;
 }

int main(int argc, char *argv[])
{
    test_sendmsg_entry();
    
    return 0;
}

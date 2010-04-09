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

#include "test_speed.h"

int InitBuffer(void);
int FreeBuffer(void);
void *test_speed_client(void *parg);

char *pcReadBuf[TEST_SPEED_CLIENT_NUM];

int InitBuffer(void)
{
    int i;
    for(i = 0; i < TEST_SPEED_CLIENT_NUM; i++)
    {
        pcReadBuf[i] = malloc(TEST_SPEED_MSG_LEN);
        if(pcReadBuf[i] == NULL)
        {
            FreeBuffer();
            return -1;
        }
    }
    return 0;
}

int FreeBuffer(void)
{
    int i;
    for(i = 0; i < TEST_SPEED_CLIENT_NUM; i++)
    {
        if(pcReadBuf[i] != NULL) free(pcReadBuf[i]);
    }
    return 0;
}

void test_speed_entry(void)
{
    pthread_t speed_client[TEST_SPEED_CLIENT_NUM];
    TEST_SPEED_DATA_T thread_arg[TEST_SPEED_CLIENT_NUM];
    int i;
    
    if(InitBuffer() == -1)
        return;
    
    for(i = 0; i < TEST_SPEED_CLIENT_NUM; i++)
    {
        thread_arg[i].port = TEST_SPEED_SERVER_BEGIN_PORT + i;
        thread_arg[i].preadbuf = pcReadBuf[i];
        pthread_create(&speed_client[i], 0, &test_speed_client, &thread_arg[i]);
    }
    
    for(i = 0; i < TEST_SPEED_CLIENT_NUM; i++)
    {
        pthread_join(speed_client[i], NULL);
    }
    
    FreeBuffer();
}

void *test_speed_client(void *parg)
{
    int s, readlen;
    struct sockaddr_in sa = {0}, r_sa = {0};
    int port = ((TEST_SPEED_DATA_T*)parg)->port;
    char *preadbuf = ((TEST_SPEED_DATA_T*)parg)->preadbuf;
    
    if(inet_aton(TEST_SPEED_SERVER_ADDR, &r_sa.sin_addr) == 0)
    {
        printf("inet_aton error\n");
        return NULL;
    }
    r_sa.sin_family = AF_INET;
    r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
    //printf("sending to %s : %d\n", inet_ntoa(*((struct in_addr*)hp->h_addr)), r_sa.sin_port);
    
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(IPPORT_USERRESERVED + port);
    
    if((s = socket(AF_INET, SOCK_STREAM, PF_UNSPEC)) == -1)
    {
        printf("socket error\n");
        return NULL;
    }
    printf("socket success...\n");
    
    if(bind(s, (struct sockaddr*)&sa, sizeof(sa)) == -1)
    {
        printf("bind error\n");
        return NULL;
    }
    printf("bind success...\n");
    
    if(connect(s, (struct sockaddr*)&r_sa, sizeof(sa)) == -1)
    {
        printf("connect error\n");
        return NULL;
    }
    printf("connect success...\n");
    
	while(1)
	{
		readlen = read(s, preadbuf, TEST_SPEED_MSG_LEN);
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
	}
    printf("read all ok\n");
    
    close(s);
    return NULL;
 }

int main(int argc, char *argv[])
{
    test_speed_entry();
    
    return 0;
}

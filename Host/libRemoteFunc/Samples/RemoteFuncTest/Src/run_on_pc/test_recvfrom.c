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

void *test_recvfrom_client(void *parg);

void test_recvfrom_entry(void)
{
    pthread_t recvfrom_client[TEST_RECVFROM_CLIENT_NUM];
    int thread_arg[TEST_RECVFROM_CLIENT_NUM];
    int i;
    
    for(i = 0; i < TEST_RECVFROM_CLIENT_NUM; i++)
    {
        thread_arg[i] = TEST_RECVFROM_SERVER_BEGIN_PORT + i;
        pthread_create(&recvfrom_client[i], 0, &test_recvfrom_client, &thread_arg[i]);
    }
    
    for(i = 0; i < TEST_RECVFROM_CLIENT_NUM; i++)
    {
        pthread_join(recvfrom_client[i], NULL);
    }
}

void *test_recvfrom_client(void *parg)
{
    int s, i, len;
    struct sockaddr_in r_sa = {0};
    int port = *(int*)parg;
    char msg[TEST_RECVFROM_MSG_LEN];
    
    if(inet_aton(TEST_RECVFROM_SERVER_ADDR, &r_sa.sin_addr) == 0)
    {
        printf("inet_aton error\n");
        return NULL;
    }
    r_sa.sin_family = AF_INET;
    r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
    //printf("sendto to %s : %d\n", inet_ntoa(r_sa.sin_addr), ntohs(r_sa.sin_port));
    
    if((s = socket(AF_INET, SOCK_DGRAM, PF_UNSPEC)) == -1)
    {
        printf("socket error\n");
        return NULL;
    }
    printf("socket success...\n");
    
    for(i = 0; i < TEST_RECVFROM_WRITE_TIMES; i++)
    {
        len = sprintf(msg, "%s", MYREQUEST);
        len++;
        if((len = sendto(s, msg, len, 0, (struct sockaddr*)&r_sa, sizeof(r_sa))) == -1)
        {
            printf("sendto error\n");
            break;
        }
    }
    printf("sendto all ok\n");
    
    close(s);
    return NULL;
 }

int main(int argc, char *argv[])
{
    test_recvfrom_entry();
    
    return 0;
}

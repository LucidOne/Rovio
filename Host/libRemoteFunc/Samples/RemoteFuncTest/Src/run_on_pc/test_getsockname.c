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

#include "test_remotefunc.h"

void test_getsockname_entry(void)
{
    int s, readlen;
    struct sockaddr_in sa = {0}, r_sa = {0};
    char preadbuf[TEST_NETREAD_MSG_LEN];
    int port = TEST_GETPEERNAME_SERVER_PORT;
    
    if(inet_aton(TEST_GETSOCKNAME_SERVER_ADDR, &r_sa.sin_addr) == 0)
    {
        printf("inet_aton error\n");
        return;
    }
    r_sa.sin_family = AF_INET;
    r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
    
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(IPPORT_USERRESERVED + port);
    
    if((s = socket(AF_INET, SOCK_STREAM, PF_UNSPEC)) == -1)
    {
        printf("socket error\n");
        return;
    }
    printf("socket success...\n");
    
    set_reuseaddr(s);
    if(bind(s, (struct sockaddr*)&sa, sizeof(sa)) == -1)
    {
        printf("bind error\n");
        return;
    }
    printf("bind success...\n");
    
    if(connect(s, (struct sockaddr*)&r_sa, sizeof(sa)) == -1)
    {
        printf("connect error\n");
        return;
    }
    printf("connect success...\n");
    
    readlen = read(s, preadbuf, TEST_NETREAD_MSG_LEN);
    if(readlen < 0)
    {
        printf("read error(%d)\n", errno);
        close(s);
        return;
    }
    if(readlen == 0)
    {
        printf("connection broken\n");
        close(s);
        return;
    }
    
    close(s);
    return;
 }

int main(int argc, char *argv[])
{
    test_getsockname_entry();
    
    return 0;
}

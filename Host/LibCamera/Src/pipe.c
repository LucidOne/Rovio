#include "CommonDef.h"

#if 1
int socket_pipe(int fd[2], int port, char* buf, int buflen)
{
	int pipe_fd;
	
	pipe_fd = pipe_create(NULL);
	
	if(pipe_fd < 0)
	{
		diag_printf("Pipe create error\n");
		return -1;
	}
	
	fd[0] = pipe_fd;
	fd[1] = pipe_fd;
	return 0;
}

#else
int socket_pipe(int fd[2], int port, char* buf, int buflen)
{
    struct sockaddr_in socket_addr;
    int addrlen, socket_fd;
    
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(port);
#ifndef WLAN    
    socket_addr.sin_addr.s_addr = inet_addr("127.0.0.1", buf, buflen);
#else
    socket_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    socket_addr.sin_len = sizeof(socket_addr);
#endif
    bzero(&(socket_addr.sin_zero), 8);
#ifndef WLAN    
    if( (socket_fd = socket(AF_INET, SOCK_STREAM, 0, buf, buflen)) < 0)
#else
    if( (socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
#endif
    {
        printf("socket create error\n");
        return -1;
    }
    
	printf("ip1 %s port1 %d\n",inet_ntoa(socket_addr.sin_addr), ntohs(socket_addr.sin_port));
#ifndef WLAN        
    if(bind(socket_fd, (struct sockaddr*)&socket_addr, sizeof(struct sockaddr), buf, buflen) < 0)
#else
    if(bind(socket_fd, (struct sockaddr*)&socket_addr, sizeof(struct sockaddr)) < 0)
#endif
    {
        printf("bind socker fd error\n");
#ifndef WLAN
        netclose(socket_fd, buf, buflen);
#else
        close(socket_fd);
#endif
        return -1;
    }
    
#ifndef WLAN    
    if(listen(socket_fd, 5, buf, buflen) < 0)
#else
    if(listen(socket_fd, 5) < 0)
#endif
    {
        printf("listen socket fd error\n");
#ifndef WLAN
        netclose(socket_fd, buf, buflen);
#else
        close(socket_fd);
#endif
        return -1;
    }
    
#ifndef WLAN    
    if((fd[1] = socket(AF_INET, SOCK_STREAM, 0, buf, buflen)) < 0)
#else
    if((fd[1] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
#endif
    {
        printf("socket create error\n");
#ifndef WLAN
        netclose(socket_fd, buf, buflen);
#else
        close(socket_fd);
#endif
        return -1;
    }
#ifndef WLAN
    
    if(connect(fd[1], (struct sockaddr*)&socket_addr, sizeof(struct sockaddr), buf, buflen) < 0)
#else
    if(connect(fd[1], (struct sockaddr*)&socket_addr, sizeof(struct sockaddr)) < 0)
#endif
    {
        printf("connect server error\n");
        return -1;
    }
    
    addrlen = sizeof(struct sockaddr);
 #ifndef WLAN
   if((fd[0] = accept(socket_fd, (struct sockaddr*)&socket_addr, (size_t*)&addrlen, buf, buflen)) < 0)
#else
   if((fd[0] = accept(socket_fd, (struct sockaddr*)&socket_addr, (size_t*)&addrlen)) < 0)
#endif
    {
        printf("accept client error\n");
        return -1;
    }
    
#ifndef WLAN
        netclose(socket_fd, buf, buflen);
#else
        close(socket_fd);
#endif
    return 0;
}
#endif
        
#ifndef PIPE_H
#define PIPE_H

int socket_pipe(int fd[2], int port, char* buf, int buflen);

#endif
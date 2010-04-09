#ifndef __REMOTE_NET_H__
#define __REMOTE_NET_H__

#include "errno.h"
#include "fcntl.h"
#include "limits.h"				/* OPEN_MAX */
#include "stdlib.h"				/* malloc, free, etc. */
#include "stdio.h"				/* stdin, stdout, stderr */
#include "string.h"				/* strdup */
#include "time.h"				/* localtime, time */


#include "unistd.h"
//#include "sys/time.h"			/* select */
#include "sys/types.h"			/* socket, bind, accept */
//#include "sys/socket.h"			/* socket, bind, accept, setsockopt, */
//#include "netinet/in.h"			/* sockaddr_in, sockaddr */
//#include "arpa/inet.h"			/* inet_ntoa */
#include "sys/stat.h"			/* open */

#include "mysocket.h"
#include "myin.h"
#include "mytcp.h"
//#include "myfcntl.h"
#include "mysockios.h"
#include "myioctls.h"

/* Address to accept any incoming messages. */
#define	INADDR_ANY		((unsigned long int) 0x00000000)

/* Address to send to all hosts. */
#define	INADDR_BROADCAST	((unsigned long int) 0xffffffff)

/* Address indicating an error return. */
#define	INADDR_NONE		((unsigned long int) 0xffffffff)

/* Network number for local host loopback. */
#define	IN_LOOPBACKNET		127

/* Address to loopback in software to local host.  */
#define	INADDR_LOOPBACK		0x7f000001	/* 127.0.0.1   */
#define	IN_LOOPBACK(a)		((((long int) (a)) & 0xff000000) == 0x7f000000)

/* Defines for Multicast INADDR */
#define INADDR_UNSPEC_GROUP   	0xe0000000U	/* 224.0.0.0   */
#define INADDR_ALLHOSTS_GROUP 	0xe0000001U	/* 224.0.0.1   */
#define INADDR_ALLRTRS_GROUP    0xe0000002U	/* 224.0.0.2 */
#define INADDR_MAX_LOCAL_GROUP  0xe00000ffU	/* 224.0.0.255 */


enum
  {
    IPPORT_ECHO = 7,		/* Echo service.  */
    IPPORT_DISCARD = 9,		/* Discard transmissions service.  */
    IPPORT_SYSTAT = 11,		/* System status service.  */
    IPPORT_DAYTIME = 13,	/* Time of day service.  */
    IPPORT_NETSTAT = 15,	/* Network status service.  */
    IPPORT_FTP = 21,		/* File Transfer Protocol.  */
    IPPORT_TELNET = 23,		/* Telnet protocol.  */
    IPPORT_SMTP = 25,		/* Simple Mail Transfer Protocol.  */
    IPPORT_TIMESERVER = 37,	/* Timeserver service.  */
    IPPORT_NAMESERVER = 42,	/* Domain Name Service.  */
    IPPORT_WHOIS = 43,		/* Internet Whois service.  */
    IPPORT_MTP = 57,

    IPPORT_TFTP = 69,		/* Trivial File Transfer Protocol.  */
    IPPORT_RJE = 77,
    IPPORT_FINGER = 79,		/* Finger service.  */
    IPPORT_TTYLINK = 87,
    IPPORT_SUPDUP = 95,		/* SUPDUP protocol.  */


    IPPORT_EXECSERVER = 512,	/* execd service.  */
    IPPORT_LOGINSERVER = 513,	/* rlogind service.  */
    IPPORT_CMDSERVER = 514,
    IPPORT_EFSSERVER = 520,

    /* UDP ports.  */
    IPPORT_BIFFUDP = 512,
    IPPORT_WHOSERVER = 513,
    IPPORT_ROUTESERVER = 520,

    /* Ports less than this value are reserved for privileged processes.  */
    IPPORT_RESERVED = 1024,

    /* Ports greater this value are reserved for (non-privileged) servers.  */
    IPPORT_USERRESERVED = 5000
  };


/* Internet address. */

struct sockaddr {
	sa_family_tt	sa_family;	/* address family, AF_xxx	*/
	char		sa_data[14];	/* 14 bytes of protocol address	*/
};

/* myif.h need struct sockaddr */
#include "myif.h"
#include "myroute.h"

#undef __NFDBITS
#define __NFDBITS	(8 * sizeof(unsigned long))

#undef __FD_SETSIZE
#define __FD_SETSIZE	1024

#undef __FDSET_LONGS
#define __FDSET_LONGS	(__FD_SETSIZE/__NFDBITS)

typedef struct {
	unsigned long fds_bits [__FDSET_LONGS];
} __kernel_fd_set;


typedef long			__kernel_time_t;
typedef long			__kernel_suseconds_t;


struct pollfd {
	int fd;
	short events;
	short revents;
};

#define _UTS_NAMESIZE 16
#define _UTS_NODESIZE 256
struct utsname {
	char sysname[_UTS_NAMESIZE];
	char nodename[_UTS_NODESIZE];
	char release[_UTS_NAMESIZE];
	char version[_UTS_NAMESIZE];
	char machine[_UTS_NAMESIZE];
};

struct servent {
	char *	s_name;
	char **	s_aliases;
	int		s_port;
	char *	proto;
};

struct hostent {
	char	h_name[256];
	char	h_aliases[256];
	int		h_addrtype;
	int		h_length;
	char	h_addr_list0[4];
	char	h_addr_list1[4];
	char	h_addr_list2[4];
	char	h_addr_list3[4];
};
#define h_addr h_addr_list0;

#undef	__FD_SET
#define __FD_SET(fd, fdsetp) \
		(((fd_set *)fdsetp)->fds_bits[fd >> 5] |= (1<<(fd & 31)))

#undef	__FD_CLR
#define __FD_CLR(fd, fdsetp) \
		(((fd_set *)fdsetp)->fds_bits[fd >> 5] &= ~(1<<(fd & 31)))

#undef	__FD_ISSET
#define __FD_ISSET(fd, fdsetp) \
		((((fd_set *)fdsetp)->fds_bits[fd >> 5] & (1<<(fd & 31))) != 0)

#undef	__FD_ZERO
#define __FD_ZERO(fdsetp) \
		(memset (fdsetp, 0, sizeof (*(fd_set *)fdsetp)))

#if 0
#define FD_SETSIZE		__FD_SETSIZE
#define FD_SET(fd,fdsetp)	__FD_SET(fd,fdsetp)
#define FD_CLR(fd,fdsetp)	__FD_CLR(fd,fdsetp)
#define FD_ISSET(fd,fdsetp)	__FD_ISSET(fd,fdsetp)
#define FD_ZERO(fdsetp)		__FD_ZERO(fdsetp)
#endif


in_addr_t inet_addr (const char *straddr, char *pBuf, int iBufLen);
int inet_aton(const char *straddr, struct in_addr *addrp, char *pBuf, int iBufLen);
char *inet_ntoa(struct in_addr inaddr, char *pBuf, int iBufLen);
int socket(int domain, int type, int protocol, char *pBuf, int iBufLen);
int bind(int sockfd, const struct sockaddr *my_addr, int addrlen, char *pBuf, int iBufLen);
int listen(int sockfd, int backlog, char *pBuf, int iBufLen);
int accept(int sockfd, struct sockaddr *client_addr, size_t *addrlen, char *pBuf, int iBufLen);
int connect(int sockfd, const struct sockaddr *serv_addr, size_t addrlen, char *pBuf, int iBufLen);
int netclose(int sockfd, char *pBuf, int iBufLen);
int netread(int filedes, void *buff, int nbytes, char *pBuf, int iBufLen);
int netwrite(int filedes, void *buff, int nbytes, char *pBuf, int iBufLen);
int send(int sockfd, const void *msg, int len, int flags, char *pBuf, int iBufLen);
int recv(int sockfd, void *buf, int len, unsigned int flags, char *pBuf, int iBufLen);
int sendto(int sockfd, void *mes, int len, int flags, struct sockaddr *toaddr, int addrlen, char *pBuf, int iBufLen);
int recvfrom(int sockfd, void *buff, int len, int flags, struct sockaddr *fromaddr, int *addrlen, char *pBuf, int iBufLen);
int netselect(int maxfd, fd_set *readset, fd_set *writeset, fd_set *exceptset, const struct timeval *timeout, char *pBuf, int iBufLen);
//int poll(struct pollfd *fdarray, uint nfds, int timeout, char *pBuf, int iBufLen);
int uname(struct utsname *name,  char *pBuf, int iBufLen);
int gethostname(char *name, int len, char *pBuf, int iBufLen);
struct servent *getservbyname(const char *servname, const char *protoname, char *pBuf, int iBufLen);
struct servent *getservbyport(int port, const char *protoname, char *pBuf, int iBufLen);
int getsockname(int sockfd, struct sockaddr *addr, int *len, char *pBuf, int iBufLen);
int getpeername(int sockfd, struct sockaddr *addr, int *len, char *pBuf, int iBufLen);
struct hostent *gethostbyaddr(const char *addr, size_t len, int type, char *pBuf, int iBufLen);
struct hostent *gethostbyname(const char *name, char *pBuf, int iBufLen);
int setsockopt(int sockfd, int level, int optname, void *optval, int optlen, char *pBuf, int iBufLen);
int getsockopt(int sockfd, int level, int optname, void *optval, int *optlen, char *pBuf, int iBufLen);
int netfcntl(int fd, int cmd, long setval, char *pBuf, int iBufLen);
int sendmsg(int sockfd, struct msghdr *msg, int flags, char *pBuf, int iBufLen);
int recvmsg(int sockfd, struct msghdr *msg, int flags, char *pBuf, int iBufLen);
int netioctl(int fd, int request, void *arg, int isize, char *pBuf, int iBufLen);
int netioctl_withbuf(int fd, int request, void *arg, int isize, char *pBuf, int iBufLen);

int wb740reboot(char *pBuf, int iBufLen);
unsigned long wb740getgateway(const char *pcInterface, char *pBuf, int iBufLen);

int N702_printf(const char* src, char *des, int deslen, char* pBuf, int iBufLen);

#endif

//==========================================================================
//
//      sys/kern/sockio.c
//
//     Socket interface to Fileio subsystem
//
//==========================================================================
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    nickg
// Contributors: nickg
// Date:         2000-06-06
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

/*
 * Copyright (c) 1982, 1986, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
//==========================================================================
#ifdef CYGPKG_NET_BLUEZ_STACK

#include "pkgconf/system.h"
//#include <pkgconf/net.h>
#include "pkgconf/io_fileio.h"

#include "linux/types.h"

#include "cyg/io/file.h"

#include "cyg/fileio/fileio.h"
#include "cyg/fileio/sockio.h"

#include "net/sock.h"
#include "asm/poll.h"
#include "stdlib.h"
#include "linux/net.h"
#include "asm/uaccess.h"
#include "linux/uio.h"

/*
 *	Support routines. Move socket addresses back and forth across the kernel/user
 *	divide and look after the messy bits.
 */

#define MAX_SOCK_ADDR	128		/* 108 for Unix domain - 
					   16 for IP, 16 for IPX,
					   24 for IPv6,
					   about 80 for AX.25 
					   must be at least one bigger than
					   the AF_UNIX size (see net/unix/af_unix.c
					   :unix_mkname()).  
					 */

//==========================================================================
// Forward definitions

static int     bluez_init( cyg_nstab_entry *nste );
static int     bluez_socket( cyg_nstab_entry *nste, int domain, int type,
		       int protocol, cyg_file *file );

static int bluez_bind      ( cyg_file *fp, const sockaddr *sa, socklen_t len );
static int bluez_connect   ( cyg_file *fp, const sockaddr *sa, socklen_t len );
static int bluez_accept    ( cyg_file *fp, cyg_file *new_fp,
                           struct sockaddr *name, socklen_t *anamelen );
static int bluez_listen    ( cyg_file *fp, int len );
static int bluez_getname   ( cyg_file *fp, sockaddr *sa, socklen_t *len, int peer );
static int bluez_shutdown  ( cyg_file *fp, int flags );
static int bluez_getsockopt( cyg_file *fp, int level, int optname,
                           void *optval, socklen_t *optlen);
static int bluez_setsockopt( cyg_file *fp, int level, int optname,
                           const void *optval, socklen_t optlen);
static int bluez_sendmsg   ( cyg_file *fp, const struct msghdr *m,
                               int flags, ssize_t *retsize );
static int bluez_recvmsg   ( cyg_file *fp, struct msghdr *m,
                               socklen_t *namelen, ssize_t *retsize );


// File operations
static int bluez_read      (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio);
static int bluez_write     (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio);
static int bluez_lseek     (struct CYG_FILE_TAG *fp, off_t *pos, int whence );
static int bluez_ioctl     (struct CYG_FILE_TAG *fp, CYG_ADDRWORD com,
                                CYG_ADDRWORD data);
static int bluez_select    (struct CYG_FILE_TAG *fp, int which, CYG_ADDRWORD info);
static int bluez_fsync     (struct CYG_FILE_TAG *fp, int mode );        
static int bluez_close     (struct CYG_FILE_TAG *fp);
static int bluez_fstat     (struct CYG_FILE_TAG *fp, struct stat *buf );
static int bluez_getinfo   (struct CYG_FILE_TAG *fp, int key, void *buf, int len );
static int bluez_setinfo   (struct CYG_FILE_TAG *fp, int key, void *buf, int len );

static int
bsd_recvit(cyg_file *fp, struct msghdr *mp, socklen_t *namelenp, ssize_t *retsize);
static int
bsd_sendit(cyg_file *fp, const struct msghdr *mp, int flags, ssize_t *retsize);


//==========================================================================
// Table entrys

#pragma arm section rwdata = "nstab"
NSTAB_ENTRY( bluez_nste, 0,
             "bluez",
             "",
             0,
             bluez_init,
             bluez_socket);
#pragma arm section rwdata

struct cyg_sock_ops bluez_sockops =
{
    bluez_bind,
    bluez_connect,
    bluez_accept,
    bluez_listen,
    bluez_getname,
    bluez_shutdown,
    bluez_getsockopt,
    bluez_setsockopt,
    bluez_sendmsg,
    bluez_recvmsg
};

cyg_fileops bluez_sock_fileops =
{
    bluez_read,
    bluez_write,
    bluez_lseek,
    bluez_ioctl,
    bluez_select,
    bluez_fsync,
    bluez_close,
    bluez_fstat,
    bluez_getinfo,
    bluez_setinfo    
};

//==========================================================================
// NStab functions



// -------------------------------------------------------------------------

static int     bluez_init( cyg_nstab_entry *nste )
{
    // Initialization already handled via constructor
    
    return ENOERR;
}

// -------------------------------------------------------------------------

static int     bluez_socket( cyg_nstab_entry *nste, int domain, int type,
		       int protocol, cyg_file *file )
{
    int error = 0;
    struct socket *so = NULL;
//diag_printf("sizeof(*so) = %d\n",sizeof(*so));
    error = sock_create(domain, type, protocol, &so);
    
    if(!so)
		return -ENOMEM;	

    if( error == ENOERR )
    {
        file->f_flag   |= CYG_FREAD|CYG_FWRITE;
        file->f_type    = CYG_FILE_TYPE_SOCKET;
        file->f_ops     = &bluez_sock_fileops;
        file->f_offset  = 0;
        file->f_data    = (CYG_ADDRWORD)so;
        file->f_xops    = (CYG_ADDRWORD)&bluez_sockops;
    }
    
    return error;
}


//==========================================================================
// Sockops functions

// -------------------------------------------------------------------------

int move_addr_to_kernel(void *uaddr, int ulen, void *kaddr)
{
	if(ulen<0||ulen>MAX_SOCK_ADDR)
		return -EINVAL;
	if(ulen==0)
		return 0;
	if(copy_from_user(kaddr,uaddr,ulen))
		return -EFAULT;
	return 0;
}

/**
 *	move_addr_to_user	-	copy an address to user space
 *	@kaddr: kernel space address
 *	@klen: length of address in kernel
 *	@uaddr: user space address
 *	@ulen: pointer to user length field
 *
 *	The value pointed to by ulen on entry is the buffer length available.
 *	This is overwritten with the buffer space used. -EINVAL is returned
 *	if an overlong buffer is specified or a negative buffer size. -EFAULT
 *	is returned if either the buffer or the length field are not
 *	accessible.
 *	After copying the data up to the limit the user specifies, the true
 *	length of the data is written over the length limit the user
 *	specified. Zero is returned for a success.
 */
 
int move_addr_to_user(void *kaddr, int klen, void *uaddr, int *ulen)
{
	int err;
	int len;

	//if((err=get_user(len, ulen)))
	//	return err;
	
	len = *ulen;
	
	if(len>klen)
		len=klen;
	if(len<0 || len> MAX_SOCK_ADDR)
		return -EINVAL;
	if(len)
	{
		if(copy_to_user(uaddr,kaddr,len))
			return -EFAULT;
	}
	/*
	 *	"fromlen shall refer to the value before truncation.."
	 *			1003.1g
	 */
	klen = *ulen;
	return 0;//__put_user(klen, ulen);
}

static int bluez_bind(cyg_file *fp, const sockaddr *sa, socklen_t len )
{
    struct socket *sock = (struct socket *)fp->f_data;
	char address[MAX_SOCK_ADDR];
	int err = -EINVAL;

	if(sock != NULL)
	{
		if((err=move_addr_to_kernel((void*)sa,len,address))>=0)
		{
#if 0
			diag_printf("%x %x %x %x %x %x %x %x %x %x\n",address[0],address[1],address[2],address[3],
				address[4],address[5],address[6],address[7],address[8],address[9]);
#endif
			err = sock->ops->bind(sock, (struct sockaddr *)address, len);
		}
		//sockfd_put(sock);
	}			
	return err;

}

// -------------------------------------------------------------------------

static int bluez_connect( cyg_file *fp, const sockaddr *sa, socklen_t len )
{
    register struct socket *sock;
    char address[MAX_SOCK_ADDR];
	int err = -EINVAL;

    sock = (struct socket *)fp->f_data;

	if (!sock)
		goto out;
	err = move_addr_to_kernel((void*)sa, len, address);
	if (err < 0)
		goto out_put;
	
	err = sock->ops->connect(sock, (struct sockaddr *) address, len,
				 fp->f_flag);
out_put:
	sockfd_put(sock);
out:
	return err;

}

// -------------------------------------------------------------------------

static int bluez_accept(cyg_file *fp, cyg_file *new_fp,
                           struct sockaddr *upeer_sockaddr, socklen_t *upeer_addrlen )
{
	struct socket *sock, *newsock;
	int err = -EINVAL, len;
	char address[MAX_SOCK_ADDR];

	sock = (struct socket *)fp->f_data;
	if (!sock)
		goto out;

	err = -EMFILE;
	if (!(newsock = sock_alloc())) 
		goto out_put;
	
	newsock->type = sock->type;
	newsock->ops = sock->ops;

	err = sock->ops->accept(sock, newsock, fp->f_flag);
	if (err < 0)
		goto out_release;

	if (upeer_sockaddr) {
		if(newsock->ops->getname(newsock, (struct sockaddr *)address, &len, 2)<0) {
			err = ECONNABORTED;
			goto out_release;
		}
		err = move_addr_to_user(address, len, upeer_sockaddr, (int*)upeer_addrlen);
		if (err < 0)
			goto out_release;
	}

	/* File flags are not inherited via accept() unlike another OSes. */

	new_fp->f_type      = CYG_FILE_TYPE_SOCKET;
    new_fp->f_flag     |= CYG_FREAD|CYG_FWRITE;
    new_fp->f_offset    = 0;
    new_fp->f_ops       = &bluez_sock_fileops;
    new_fp->f_data      = (CYG_ADDRWORD)newsock;
    new_fp->f_xops      = (CYG_ADDRWORD)&bluez_sockops;
    
//printf("accept newsock=%x",newsock);
	
out_put:
		sockfd_put(sock);
out:
	return err;

out_release:
	sock_release(newsock);
	goto out_put;

}

// -------------------------------------------------------------------------

static int bluez_listen( cyg_file *fp, int backlog )
{
	struct socket *sock;
	int err = -EINVAL;
	
	sock = (struct socket *)fp->f_data;
	
	if (sock != NULL) {
		if ((unsigned) backlog > SOMAXCONN)
			backlog = SOMAXCONN;
		err=sock->ops->listen(sock, backlog);
		sockfd_put(sock);
	}
	return err;
    //return (solisten((struct socket *)fp->f_data, backlog));
}

// -------------------------------------------------------------------------

#define PEERADDR	1
static int bluez_getname( cyg_file *fp, sockaddr *asa, socklen_t *alen, int peer )
{
    //socklen_t len = 0;
    struct socket *sock;
	char address[MAX_SOCK_ADDR];
	int len, err = -EINVAL;
    
    sock = (struct socket *)fp->f_data;
    
    if(peer == PEERADDR)
    {
		if (sock!=NULL)
		{
			err = sock->ops->getname(sock, (struct sockaddr *)address, &len, 1);
			if (!err)
				err=move_addr_to_user(address,len, asa, (int*)alen);
			sockfd_put(sock);
		}
		return err;
    }
    else
    {
		if (!sock)
			goto out;
		err = sock->ops->getname(sock, (struct sockaddr *)address, &len, 0);
		if (err)
			goto out_put;
		err = move_addr_to_user(address, len, asa, (int*)alen);

	out_put:
		sockfd_put(sock);
	out:
		return err;
    }
}

// -------------------------------------------------------------------------

static int bluez_shutdown  ( cyg_file *fp, int how )
{
	int err = -EINVAL;
	struct socket *sock;
	
	sock = (struct socket *)fp->f_data;
	
	if (sock!=NULL)
	{
		err=sock->ops->shutdown(sock, how);
		sockfd_put(sock);
	}
	return err;

  //  return (soshutdown((struct socket *)fp->f_data, how));
}

// -------------------------------------------------------------------------

static int bluez_getsockopt( cyg_file *fp, int level, int optname,
                           void *optval, socklen_t *optlen)
{
	int err = -EINVAL;
	struct socket *sock;
	sock = (struct socket *)fp->f_data;

	if (sock != NULL)
	{
//		if (level == SOL_SOCKET)
//			err=sock_getsockopt(sock,level,optname,optval,optlen);
//		else
			err=sock->ops->getsockopt(sock, level, optname, optval, (int *)optlen);
		sockfd_put(sock);
	}
	return err;

}

// -------------------------------------------------------------------------

static int bluez_setsockopt( cyg_file *fp, int level, int optname,
                           const void *optval, socklen_t optlen)
{

	int err = -EINVAL;
	struct socket *sock;

	sock = (struct socket *)fp->f_data;

	if (optlen < 0)
		return -EINVAL;
			
	if (sock != NULL)
	{
//		if (level == SOL_SOCKET)
//			err=sock_setsockopt(sock,level,optname,optval,optlen);
//		else
			err=sock->ops->setsockopt(sock, level, optname, (char *)optval, optlen);
		sockfd_put(sock);
	}
	return err;
}

// -------------------------------------------------------------------------

static int bluez_sendmsg( cyg_file *fp, const struct msghdr *m, int flags, ssize_t *retsize )
{
	struct socket *sock;
	char address[MAX_SOCK_ADDR];
	struct iovec iovstack[UIO_FASTIOV], *iov = iovstack;
	unsigned char ctl[sizeof(struct cmsghdr) + 20];	/* 20 is size of ipv6_pktinfo */
	unsigned char *ctl_buf = ctl;
	struct msghdr msg_sys;
	int err, ctl_len, iov_size, total_len = 0;
	int i;
	
	sock = (struct socket*)fp->f_data;
	if(!sock)
		return -EINVAL;
	
	err = -EFAULT;
	if (copy_from_user(&msg_sys,m,sizeof(struct msghdr)))
		goto out; 
	
	err = -EINVAL;
	if (msg_sys.msg_iovlen > UIO_MAXIOV)
		goto out_put;

	/* Check whether to allocate the iovec area*/
	err = -ENOMEM;
	iov_size = msg_sys.msg_iovlen * sizeof(struct iovec);
	if (msg_sys.msg_iovlen > UIO_FASTIOV) {
		iov = sock_kmalloc(sock->sk, iov_size, 0);
		if (!iov)
			goto out_put;
	}	
	
	
	/* This will also move the address data into kernel space */
	err = verify_iovec(&msg_sys, iov, address, VERIFY_READ);
	if (err < 0) 
		goto out_freeiov;
	
	for (i = 0 ; i < msg_sys.msg_iovlen ; i++) {
		ssize_t tmp = total_len;
		ssize_t len = (ssize_t) iov[i].iov_len;
		if (len < 0)	/* size_t not fitting an ssize_t .. */
			goto out_freeiov;
		total_len += len;
		if (total_len < tmp) /* maths overflow on the ssize_t */
			goto out_freeiov;
	}

	err = -ENOBUFS;

	if (msg_sys.msg_controllen > INT_MAX)
		goto out_freeiov;
	ctl_len = msg_sys.msg_controllen; 
	if (ctl_len) 
	{
		if (ctl_len > sizeof(ctl))
		{
			ctl_buf = sock_kmalloc(sock->sk, ctl_len, 0);
			if (ctl_buf == NULL) 
				goto out_freeiov;
		}
		err = -EFAULT;
		if (copy_from_user(ctl_buf, msg_sys.msg_control, ctl_len))
			goto out_freectl;
		msg_sys.msg_control = ctl_buf;
	}
	msg_sys.msg_flags = flags;

	if (fp->f_flag & O_NONBLOCK)
		msg_sys.msg_flags |= MSG_DONTWAIT;
//diag_printf("sendmsg len = %d\n",total_len);
	err = sock_sendmsg(sock, &msg_sys, total_len);
	if(err < 0)
		goto out_freectl;
	
	*retsize = err;
	err = ENOERR;

out_freectl:
	if (ctl_buf != ctl)    
		sock_kfree_s(sock->sk, ctl_buf, ctl_len);
out_freeiov:
	if (iov != iovstack)
		sock_kfree_s(sock->sk, iov, iov_size);
out_put:
	sockfd_put(sock);
out:       
	return err;

//    return bsd_sendit( fp, m, flags, retsize);
}

// -------------------------------------------------------------------------

static int bluez_recvmsg( cyg_file *fp, struct msghdr *msg, socklen_t *namelen, ssize_t *retsize )
{
	struct socket *sock;
	struct iovec iovstack[UIO_FASTIOV];
	struct iovec *iov=iovstack;
	struct msghdr msg_sys;
	unsigned long cmsg_ptr;
	int err, iov_size, total_len = 0, len;
	int i;

	/* kernel mode address */
	char addr[MAX_SOCK_ADDR];

	/* user mode address pointers */
	struct sockaddr *uaddr;
	int *uaddr_len;
	
	err=-EFAULT;
	if (copy_from_user(&msg_sys,msg,sizeof(struct msghdr)))
		goto out;

	sock = (struct socket*)fp->f_data;
	if (!sock)
		goto out;

	err = -EINVAL;
	if (msg_sys.msg_iovlen > UIO_MAXIOV)
		goto out_put;
	
	/* Check whether to allocate the iovec area*/
	err = -ENOMEM;
	iov_size = msg_sys.msg_iovlen * sizeof(struct iovec);
	if (msg_sys.msg_iovlen > UIO_FASTIOV) {
		iov = sock_kmalloc(sock->sk, iov_size, 0);
		if (!iov)
			goto out_put;
	}

	/*
	 *	Save the user-mode address (verify_iovec will change the
	 *	kernel msghdr to use the kernel address space)
	 */
	 
	uaddr = msg_sys.msg_name;
	uaddr_len = &msg->msg_namelen;
	err = verify_iovec(&msg_sys, iov, addr, VERIFY_WRITE);
	if (err < 0)
		goto out_freeiov;

	for (i = 0 ; i < msg_sys.msg_iovlen ; i++) {
		ssize_t tmp = total_len;
		ssize_t len = (ssize_t) iov[i].iov_len;
		if (len < 0)	/* size_t not fitting an ssize_t .. */
			goto out_freeiov;
		total_len += len;
		if (total_len < tmp) /* maths overflow on the ssize_t */
			goto out_freeiov;
	}

	cmsg_ptr = (unsigned long)msg_sys.msg_control;
	msg_sys.msg_flags = 0;
	
	if (fp->f_flag & O_NONBLOCK)
		msg->msg_flags |= MSG_DONTWAIT;
	
	err = sock_recvmsg(sock, &msg_sys, total_len, msg->msg_flags);
	if (err < 0)
		goto out_freeiov;
	
	*retsize = err;
	err = ENOERR;

	if (uaddr != NULL && msg_sys.msg_namelen) {
		err = move_addr_to_user(addr, msg_sys.msg_namelen, uaddr, uaddr_len);
		if (err < 0)
			goto out_freeiov;
	}
	msg->msg_flags = msg_sys.msg_flags;
	//err = __put_user(msg_sys.msg_flags, &msg->msg_flags);
	//if (err)
	//	goto out_freeiov;
	//err = __put_user((unsigned long)msg_sys.msg_control-cmsg_ptr, 
//							 &msg->msg_controllen);
	//if (err)
	//	goto out_freeiov;
	msg->msg_controllen = (cyg_uint32)msg_sys.msg_control-cmsg_ptr;

out_freeiov:
	if (iov != iovstack)
		sock_kfree_s(sock->sk, iov, iov_size);
out_put:
	sockfd_put(sock);
out:
	return err;
    //return bsd_recvit( fp, m, namelen, retsize);
}

//==========================================================================
// File system call functions

// -------------------------------------------------------------------------

static int bluez_read(struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio)
{
//    return (soreceive((struct socket *)fp->f_data, (struct mbuf **)0,
//                      uio, (struct mbuf **)0, (struct mbuf **)0, (int *)0));

	int ret,i;
	struct socket *sock = (struct socket *)fp->f_data;
	int resid = uio->uio_resid;
	struct iovec iovstack[UIO_FASTIOV];//8
	struct iovec *iov=iovstack;
	struct iovec * vector = uio->uio_iov;
	int count = uio->uio_iovcnt;
	struct msghdr msg_sys;

	if(!sock)
		return -EINVAL;
	/*
	 * First get the "struct iovec" from user memory and
	 * verify all the pointers
	 */
	ret = 0;
	if (!count)
		goto out_nofree;
	ret = -EINVAL;
	if (count > UIO_MAXIOV)
		goto out_nofree;

	if (count > UIO_FASTIOV) {
		ret = -ENOMEM;
		iov = malloc(count*sizeof(struct iovec));
		if (!iov)
			goto out_nofree;
	}
	ret = -EFAULT;
	if (copy_from_user(iov, vector, count*sizeof(*vector)))
		goto out;

	// We successfully read some data, update the node's access time
	// and update the file offset and transfer residue.
	
	ret = 0;
	i = 0;
	
//	while(count--)
	{
		msg_sys.msg_name = NULL;
		msg_sys.msg_namelen = 0;
		msg_sys.msg_iov = iov;
		msg_sys.msg_iovlen = 1;
		msg_sys.msg_control =0;
		msg_sys.msg_flags = 0;
		
		if (fp->f_flag & O_NONBLOCK)
			msg_sys.msg_flags |= MSG_DONTWAIT;

		ret = sock_recvmsg(sock, &msg_sys, resid, msg_sys.msg_flags);//iov[i++].iov_len);
		if(ret > 0)
			resid -= ret;
	}

out:
	if (iov != iovstack)
		free(iov);
out_nofree:
	uio->uio_resid = resid;
	if(ret < 0)
		return ret;
	else
		return ENOERR;

	//fp->f_offset = pos;

}

// -------------------------------------------------------------------------

static int bluez_write(struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio)
{
//	return (sosend((struct socket *)fp->f_data, (struct mbuf *)0,
//		uio, (struct mbuf *)0, (struct mbuf *)0, 0));
	int tot_len;
	struct iovec iovstack[UIO_FASTIOV];
	struct iovec *iov=iovstack;
	int ret, i;
	struct iovec * vector = uio->uio_iov;
	struct socket *sock = (struct socket *)fp->f_data;
	int count = uio->uio_iovcnt;
	struct msghdr msg_sys;
	int resid = uio->uio_resid;
	
	if(!sock)
		return -EINVAL;
	/*
	 * First get the "struct iovec" from user memory and
	 * verify all the pointers
	 */
	ret = 0;
	if (!count)
		goto out_nofree;
	ret = -EINVAL;
	if (count > UIO_MAXIOV)
		goto out_nofree;

	if (count > UIO_FASTIOV) {
		ret = -ENOMEM;
		iov = malloc(count*sizeof(struct iovec));
		if (!iov)
			goto out_nofree;
	}
	ret = -EFAULT;
	if (copy_from_user(iov, vector, count*sizeof(*vector)))
		goto out;

	/*
	 * Single unix specification:
	 * We should -EINVAL if an element length is not >= 0 and fitting an ssize_t
	 * The total length is fitting an ssize_t
	 *
	 * Be careful here because iov_len is a size_t not an ssize_t
	 */
	 
	ret = 0;
	//i = 0;
	
//	while(count--)
	{
		msg_sys.msg_name = NULL;
		msg_sys.msg_namelen = 0;
		msg_sys.msg_iov = iov;
		msg_sys.msg_iovlen = 1;
		msg_sys.msg_control =0;
		msg_sys.msg_flags = 0;
		
		if (fp->f_flag & O_NONBLOCK)
			msg_sys.msg_flags |= MSG_DONTWAIT;

		ret = sock_sendmsg(sock, &msg_sys, resid);//iov[i++].iov_len);

		if(ret > 0)
			resid -= ret;
	}

out:
	if (iov != iovstack)
		free(iov);
out_nofree:
	uio->uio_resid = resid;
	if(ret < 0)
		return ret;
	else
		return ENOERR;

}

// -------------------------------------------------------------------------

static int bluez_lseek(struct CYG_FILE_TAG *fp, off_t *pos, int whence )
{
    return -ESPIPE;
}

// -------------------------------------------------------------------------

static int bluez_ioctl(struct CYG_FILE_TAG *fp, CYG_ADDRWORD cmd,
                                CYG_ADDRWORD data)
{
	register struct socket *so = (struct socket *)fp->f_data;
	int err;
	
	if(!so)
		return -EINVAL;
	err = so->ops->ioctl(so, cmd, data);
	return err;

}

// -------------------------------------------------------------------------
#define POLLIN_SET (POLLRDNORM | POLLRDBAND | POLLIN | POLLHUP | POLLERR)
#define POLLOUT_SET (POLLWRBAND | POLLWRNORM | POLLOUT | POLLERR)
#define POLLEX_SET (POLLPRI)

int selrwwakeup(struct sock *sk)
{
	int mask = 0;
	struct socket *so = sk->socket;
	
	if(!so || !so->ops)
		return;
	
	mask = so->ops->poll(NULL, so);
//printf("selrwwakeup mask=%x,so=%x,sk=%x,%d,%d \n",mask,so,sk,so->read_flag_sel,so->write_flag_sel);
	if(so->read_flag_sel && (mask&POLLIN_SET))
	{
		so->read_flag_sel = 0;
		cyg_selwakeup(&so->read_sb_sel);
	}
	else if(so->write_flag_sel && (mask&POLLOUT_SET))
	{
		so->write_flag_sel = 0;
		cyg_selwakeup(&so->write_sb_sel);
	}
}

static int bluez_select(struct CYG_FILE_TAG *fp, int which, CYG_ADDRWORD info)
{
    register struct socket *so = (struct socket *)fp->f_data;
    int mask;

    //register int s = splsoftnet();
	if(!so || !so->sk)
		return -EINVAL;
	
	mask = so->ops->poll(fp, so);
	
	//printf("bluez_select so=%x,sk=%x,mask=%x,which=%d\n",so,so->sk,mask,which);
    switch (which) {

    case FREAD:
        if(mask & POLLIN_SET){
            return (1);
        }
        cyg_selrecord(info, &so->read_sb_sel);
        so->read_flag_sel = 1;
        //so->so_rcv.sb_flags |= SB_SEL;
        break;

    case FWRITE:
        if(mask & POLLOUT_SET){
            return (1);
        }
        cyg_selrecord(info, &so->write_sb_sel);
        so->write_flag_sel = 1;
        break;

    case 0:
#if 0
        if (so->so_oobmark || (so->so_state & SS_RCVATMARK)) {
            splx(s);
            return (1);
        }
        cyg_selrecord(info, &so->so_rcv.sb_sel);
        so->so_rcv.sb_flags |= SB_SEL;
#endif
        break;
    }
    //splx(s);

    return ENOERR;
}

// -------------------------------------------------------------------------

static int bluez_fsync(struct CYG_FILE_TAG *fp, int mode )
{
    // FIXME: call some sort of flush IOCTL?
    return 0;
}

// -------------------------------------------------------------------------

static int bluez_close(struct CYG_FILE_TAG *fp)
{
    int error = 0;

    	/*
	 *	It was possible the inode is NULL we were 
	 *	closing an unfinished socket. 
	 */

	sock_release((struct socket*)fp->f_data);
//diag_printf("fp->f_data=%x\n",fp->f_data);
    fp->f_data = 0;
    return (error);
}

// -------------------------------------------------------------------------

static int bluez_fstat(struct CYG_FILE_TAG *fp, struct stat *buf )
{
    register struct socket *so = (struct socket *)fp->f_data;

    bzero((caddr_t)buf, sizeof (*buf));

    // Mark socket as a fifo for now. We need to add socket types to
    // sys/stat.h.
    buf->st_mode = __stat_mode_FIFO;

}

// -------------------------------------------------------------------------

static int bluez_getinfo(struct CYG_FILE_TAG *fp, int key, void *buf, int len )
{
    return -ENOSYS;
}

// -------------------------------------------------------------------------

static int bluez_setinfo(struct CYG_FILE_TAG *fp, int key, void *buf, int len )
{
    return -ENOSYS;
}



//==========================================================================
// Select support

// -------------------------------------------------------------------------
// This function is called by the lower layers to record the
// fact that a particular 'select' event is being requested.
//
#if 0
void        
selrecord(void *selector, struct selinfo *info)
{
    // Unused by this implementation
}

// -------------------------------------------------------------------------
// This function is called to indicate that a 'select' event
// may have occurred.
//

void    
selwakeup(struct selinfo *info)
{
    cyg_selwakeup( info );
}

//==========================================================================
// Misc support functions

int
sockargs(mp, buf, buflen, type)
	struct mbuf **mp;
	caddr_t buf;
	socklen_t buflen;
	int type;
{
#if 0
	register struct sockaddr *sa;
	register struct mbuf *m;
	int error;

	if (buflen > MLEN) {
#ifdef COMPAT_OLDSOCK
		if (type == MT_SONAME && buflen <= 112)
			buflen = MLEN;		/* unix domain compat. hack */
		else
#endif
		return (EINVAL);
	}
	m = m_get(M_WAIT, type);
	if (m == NULL)
		return (ENOBUFS);
	m->m_len = buflen;
	error = copyin(buf, mtod(m, caddr_t), buflen);
	if (error) {
		(void) m_free(m);
		return (error);
	}
	*mp = m;
	if (type == MT_SONAME) {
		sa = mtod(m, struct sockaddr *);

#if defined(COMPAT_OLDSOCK) && BYTE_ORDER != BIG_ENDIAN
		if (sa->sa_family == 0 && sa->sa_len < AF_MAX)
			sa->sa_family = sa->sa_len;
#endif
		sa->sa_len = buflen;
	}
#endif
	return (0);
}

#endif
#if 0
// -------------------------------------------------------------------------
// bsd_recvit()
// Support for message reception. This is a lightly edited version of the
// recvit() function is uipc_syscalls.c.

static int
bsd_recvit(cyg_file *fp, struct msghdr *mp, socklen_t *namelenp, ssize_t *retsize)
{
        struct uio auio;
	register struct iovec *iov;
	register int i;
	size_t len;
	int error;
	struct mbuf *from = 0, *control = 0;
//diag_printf("bsd_recvit\n");
	auio.uio_iov = mp->msg_iov;
	auio.uio_iovcnt = mp->msg_iovlen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_rw = UIO_READ;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = 0;
	iov = mp->msg_iov;
	for (i = 0; i < mp->msg_iovlen; i++, iov++) {
		/* Don't allow sum > SSIZE_MAX */
		if (iov->iov_len > SSIZE_MAX ||
		    (auio.uio_resid += iov->iov_len) > SSIZE_MAX)
			return (EINVAL);
	}

	len = auio.uio_resid;
	error = soreceive((struct socket *)fp->f_data, &from, &auio,
			  NULL, mp->msg_control ? &control : NULL,
			  &mp->msg_flags);
//diag_printf("bsd_recvit error = %d\n");
	if (error) {
		if (auio.uio_resid != len && 
                    (error == EINTR || error == EWOULDBLOCK))
			error = 0;
	}
	if (error)
		goto out;
	*retsize = len - auio.uio_resid;
	if (mp->msg_name) {
		len = mp->msg_namelen;
		if (len <= 0 || from == 0)
			len = 0;
		else {
		        /* save sa_len before it is destroyed by MSG_COMPAT */
			if (len > from->m_len)
				len = from->m_len;
			/* else if len < from->m_len ??? */
#ifdef COMPAT_OLDSOCK
			if (mp->msg_flags & MSG_COMPAT)
				mtod(from, struct osockaddr *)->sa_family =
				    mtod(from, struct sockaddr *)->sa_family;
#endif
			error = copyout(mtod(from, caddr_t),
			    (caddr_t)mp->msg_name, (unsigned)len);
			if (error)
				goto out;
		}
		mp->msg_namelen = len;
		if (namelenp ) {
                        *namelenp = len;
#ifdef COMPAT_OLDSOCK
			if (mp->msg_flags & MSG_COMPAT)
				error = 0;	/* old recvfrom didn't check */
			else
#endif
			goto out;
		}
	}
	if (mp->msg_control) {
#ifdef COMPAT_OLDSOCK
		/*
		 * We assume that old recvmsg calls won't receive access
		 * rights and other control info, esp. as control info
		 * is always optional and those options didn't exist in 4.3.
		 * If we receive rights, trim the cmsghdr; anything else
		 * is tossed.
		 */
		if (control && mp->msg_flags & MSG_COMPAT) {
			if (mtod(control, struct cmsghdr *)->cmsg_level !=
			    SOL_SOCKET ||
			    mtod(control, struct cmsghdr *)->cmsg_type !=
			    SCM_RIGHTS) {
				mp->msg_controllen = 0;
				goto out;
			}
			control->m_len -= sizeof (struct cmsghdr);
			control->m_data += sizeof (struct cmsghdr);
		}
#endif
		len = mp->msg_controllen;
		if (len <= 0 || control == 0)
			len = 0;
		else {
			struct mbuf *m = control;
			caddr_t p = (caddr_t)mp->msg_control;

			do {
				i = m->m_len;
				if (len < i) {
					mp->msg_flags |= MSG_CTRUNC;
					i = len;
				}
				error = copyout(mtod(m, caddr_t), p,
				    (unsigned)i);
				if (m->m_next)
					i = ALIGN(i);
				p += i;
				len -= i;
				if (error != 0 || len <= 0)
					break;
			} while ((m = m->m_next) != NULL);
			len = p - (caddr_t)mp->msg_control;
		}
		mp->msg_controllen = len;
	}
out:
	if (from)
		m_freem(from);
	if (control)
		m_freem(control);
	return (error);
}

// -------------------------------------------------------------------------
// sendit()
// Support for message transmission. This is a lightly edited version of the
// synonymous function is uipc_syscalls.c.

static int
bsd_sendit(cyg_file *fp, const struct msghdr *mp, int flags, ssize_t *retsize)
{
	struct uio auio;
	register struct iovec *iov;
	register int i;
	struct mbuf *to, *control;
	int len, error;
	
	auio.uio_iov = mp->msg_iov;
	auio.uio_iovcnt = mp->msg_iovlen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_rw = UIO_WRITE;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = 0;
	iov = mp->msg_iov;
	for (i = 0; i < mp->msg_iovlen; i++, iov++) {
		/* Don't allow sum > SSIZE_MAX */
		if (iov->iov_len > SSIZE_MAX ||
		    (auio.uio_resid += iov->iov_len) > SSIZE_MAX)
			return (EINVAL);
	}
	if (mp->msg_name) {
		error = sockargs(&to, mp->msg_name, mp->msg_namelen,
				 MT_SONAME);
		if (error)
			return (error);
	} else
		to = 0;
	if (mp->msg_control) {
		if (mp->msg_controllen < sizeof(struct cmsghdr)
#ifdef COMPAT_OLDSOCK
		    && mp->msg_flags != MSG_COMPAT
#endif
		) {
			error = EINVAL;
			goto bad;
		}
		error = sockargs(&control, mp->msg_control,
				 mp->msg_controllen, MT_CONTROL);
		if (error)
			goto bad;
#ifdef COMPAT_OLDSOCK
		if (mp->msg_flags == MSG_COMPAT) {
			register struct cmsghdr *cm;

			M_PREPEND(control, sizeof(*cm), M_WAIT);
			if (control == 0) {
				error = ENOBUFS;
				goto bad;
			} else {
				cm = mtod(control, struct cmsghdr *);
				cm->cmsg_len = control->m_len;
				cm->cmsg_level = SOL_SOCKET;
				cm->cmsg_type = SCM_RIGHTS;
			}
		}
#endif
	} else
		control = 0;

	len = auio.uio_resid;
	error = sosend((struct socket *)fp->f_data, to, &auio,
		       NULL, control, flags);
	if (error) {
		if (auio.uio_resid != len && 
                    (error == EINTR || error == EWOULDBLOCK))
			error = 0;
#ifndef __ECOS
		if (error == EPIPE)
			psignal(p, SIGPIPE);
#endif
	}
	if (error == 0)
		*retsize = len - auio.uio_resid;
bad:
	if (to)
		m_freem(to);
	return (error);
}
#endif


#endif
//==========================================================================
// End of sockio.c

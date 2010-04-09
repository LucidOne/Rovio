/* 
   BlueZ - Bluetooth protocol stack for Linux
   Copyright (C) 2000-2001 Qualcomm Incorporated

   Written 2000,2001 by Maxim Krasnyansky <maxk@qualcomm.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 as
   published by the Free Software Foundation;

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.
   IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) AND AUTHOR(S) BE LIABLE FOR ANY
   CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES 
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN 
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF 
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ALL LIABILITY, INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PATENTS, 
   COPYRIGHTS, TRADEMARKS OR OTHER RIGHTS, RELATING TO USE OF THIS 
   SOFTWARE IS DISCLAIMED.
*/

/*
 * BlueZ Bluetooth address family and sockets.
 *
 * $Id: af_bluetooth.c,v 1.8 2002/07/22 20:32:54 maxk Exp $
 */
#ifdef CYGPKG_NET_BLUEZ_STACK

#define VERSION "2.3"

#include "pkgconf/system.h"
#include "pkgconf/net.h"
#include "pkgconf/io_fileio.h"
#include "cyg/infra/cyg_type.h"
#include "cyg/kernel/kapi.h"

#include "linux/types.h"

#include "cyg/io/file.h"

#include "cyg/fileio/fileio.h"
#include "cyg/fileio/sockio.h"

//#include <sys/param.h>

#include "linux/spinlock.h"
#include "asm/atomic.h"
#include "linux/socket.h"
//#include <sys/socketvar.h>

//#include "net/if.h"
//#include "net/route.h"
#include "asm/uaccess.h"
#include "net/checksum.h"
#include "net/sock.h"
//#include <linux/uio.h>
#include "linux/list.h"
//#include <linux/skbuff.h>

#include "linux/notifier.h"
#include "net/bluetooth/bluetooth.h"

#define BUG_TRAP(x) if (!(x)) { diag_printf("KERNEL: assertion (" #x ") failed at " __FILE__ "(%d)\n", __LINE__); }

static struct net_proto_family *net_families[NPROTO];

static int wait_for_packet(struct sock * sk, int *err, long *timeo_p);
static inline int connection_based(struct sock *sk);

/*
 *	Statistics counters of the socket lists
 */
#define	NR_CPUS	1
static union {
	int	counter;
	//char	__pad[SMP_CACHE_BYTES];
} sockets_in_use[NR_CPUS];// __cacheline_aligned = {{0}};


#ifndef AF_BLUETOOTH_DEBUG
#undef  BT_DBG
#define BT_DBG( A... )
#endif

/* Bluetooth sockets */
#define BLUEZ_MAX_PROTO	6
static struct net_proto_family *bluez_proto[BLUEZ_MAX_PROTO];

int bluez_sock_register(int proto, struct net_proto_family *ops)
{
	if (proto >= BLUEZ_MAX_PROTO)
		return -EINVAL;

	if (bluez_proto[proto])
		return -EEXIST;

	bluez_proto[proto] = ops;
	return 0;
}

int bluez_sock_unregister(int proto)
{
	if (proto >= BLUEZ_MAX_PROTO)
		return -EINVAL;

	if (!bluez_proto[proto])
		return -ENOENT;

	bluez_proto[proto] = NULL;
	return 0;
}

static int bluez_sock_create(struct socket *sock, int proto)
{
	if (proto >= BLUEZ_MAX_PROTO)
		return -EINVAL;

	if (!bluez_proto[proto])
		return -ENOENT;

	return bluez_proto[proto]->create(sock, proto);
}

void bluez_sock_init(struct socket *sock, struct sock *sk)
{ 
	sock_init_data(sock, sk);
	
	INIT_LIST_HEAD(&bluez_pi(sk)->accept_q);
	
}

void bluez_sock_link(struct bluez_sock_list *l, struct sock *sk)
{
	write_lock_bh(&l->lock);
	sk->next = l->head;
	l->head = sk;
	sock_hold(sk);
	write_unlock_bh(&l->lock);
}

void bluez_sock_unlink(struct bluez_sock_list *l, struct sock *sk)
{
	struct sock **skp;

	write_lock_bh(&l->lock);
	for (skp = &l->head; *skp; skp = &((*skp)->next)) {
		if (*skp == sk) {
			*skp = sk->next;
			__sock_put(sk);
			break;
		}
	}
	write_unlock_bh(&l->lock);
}

void bluez_accept_enqueue(struct sock *parent, struct sock *sk)
{
	BT_DBG("parent %p, sk %p", parent, sk);

	sock_hold(sk);
	list_add_tail(&bluez_pi(sk)->accept_q, &bluez_pi(parent)->accept_q);
	bluez_pi(sk)->parent = parent;
	parent->ack_backlog++;
}

static void bluez_accept_unlink(struct sock *sk)
{
	BT_DBG("sk %p state %d", sk, sk->state);

	list_del_init(&bluez_pi(sk)->accept_q);
	bluez_pi(sk)->parent->ack_backlog--;
	bluez_pi(sk)->parent = NULL;
	sock_put(sk);
}

struct sock *bluez_accept_dequeue(struct sock *parent, struct socket *newsock)
{
	struct list_head *p, *n;
	struct bluez_pinfo *pi;
	struct sock *sk;
	
	BT_DBG("parent %p", parent);

	list_for_each_safe(p, n, &bluez_pi(parent)->accept_q) {
		pi = list_entry(p, struct bluez_pinfo, accept_q);
		sk = bluez_sk(pi);
		
		lock_sock(sk);
		if (sk->state == BT_CLOSED) {
			release_sock(sk);
			bluez_accept_unlink(sk);
			continue;
		}
		
		if (sk->state == BT_CONNECTED || !newsock) {
//			diag_printf("accept unlink sk\n");
			bluez_accept_unlink(sk);
			if (newsock)
				sock_graft(sk, newsock);
			release_sock(sk);
			return sk;
		}
		release_sock(sk);
	}
	return NULL;
}

int bluez_sock_recvmsg(struct socket *sock, struct msghdr *msg, int len, int flags, struct scm_cookie *scm)
{
	int noblock = flags & MSG_DONTWAIT;
	struct sock *sk = sock->sk;
	struct sk_buff *skb;
	int copied, err;

	BT_DBG("sock %p sk %p len %d", sock, sk, len);

	if (flags & (MSG_OOB))
		return -EOPNOTSUPP;

	if (!(skb = skb_recv_datagram(sk, flags, noblock, &err))) {
		if (sk->shutdown & RCV_SHUTDOWN)
			return 0;
		return err;
	}

	msg->msg_namelen = 0;

	copied = skb->len;
	if (len < copied) {
		msg->msg_flags |= MSG_TRUNC;
		copied = len;
	}

	skb->h.raw = skb->data;
	err = skb_copy_datagram_iovec(skb, 0, msg->msg_iov, copied);

	skb_free_datagram(sk, skb);

	return err ? err : copied;
}

cyg_uint32 bluez_sock_poll(struct CYG_FILE_TAG * file, struct socket *sock/*, poll_table *wait*/)
{
	struct sock *sk = sock->sk;
	unsigned int mask = 0;
	cyg_tick_count_t sleep_time;

	BT_DBG("sock %p, sk %p", sock, sk);

	//poll_wait(file, sk->sleep, wait);

	//cyg_semaphore_init(&sk->sem, 0);
#if 0
	if (timo) {
        sleep_time = cyg_current_time() + timo;
        if (!cyg_semaphore_timed_wait(&sk->sem, sleep_time)) {
            if( cyg_current_time() >= sleep_time )
                res = -ETIMEDOUT;
            else
                res = -EINTR;
        }
    } else {
#endif
#if 0
        if (!cyg_semaphore_wait(&sk->sem) ) {
            //res = EINTR;
        }
    //}
#endif
	if (sk->err || !skb_queue_empty(&sk->error_queue))
		mask |= POLLERR;

	if (sk->shutdown == SHUTDOWN_MASK)
		mask |= POLLHUP;

	if (!skb_queue_empty(&sk->receive_queue) || 
			!list_empty(&bluez_pi(sk)->accept_q) ||
			(sk->shutdown & RCV_SHUTDOWN))
		mask |= POLLIN | POLLRDNORM;

	if (sk->state == BT_CLOSED)
		mask |= POLLHUP;

	if (sk->state == BT_CONNECT ||
			sk->state == BT_CONNECT2 ||
			sk->state == BT_CONFIG)
		return mask;

	if (sock_writeable(sk))
		mask |= POLLOUT | POLLWRNORM | POLLWRBAND;
	else
		set_bit(SOCK_ASYNC_NOSPACE, &sk->socket->flags);

	return mask;
}
int testme()
{}
int testok()
{}
int bluez_sock_wait_state(struct sock *sk, int state, unsigned long timeo)
{
	int err = 0;
	cyg_tick_count_t sleep_time;

	printf("wait state sk %p timeo=%d,&sem=%x,self=%x", sk, timeo, &sk->sem,cyg_thread_self());

	while (sk->state != state) {
		//set_current_state(TASK_INTERRUPTIBLE);

		if (!timeo) {
			err = -EAGAIN;
			break;
		}

		release_sock(sk);
		//timeo = schedule_timeout(timeo);
		//cyg_semaphore_init(&sk->sem, 0);
		if (timeo) {
        	sleep_time = cyg_current_time() + timeo;
        	if(sk->state != state)
        	{
	        	if(!cyg_semaphore_timed_wait(&sk->sem, sleep_time))
    	    	{
        			if(cyg_current_time() > sleep_time)
        				err = -ETIMEDOUT;
        			else
        				err = -EINTR;
        		}
        	}
       	}
        testok();
        timeo = 0;
    	

		lock_sock(sk);

		if (sk->err) {
			err = sock_error(sk);
			break;
		}
	}
	printf("sk->state=%d,state=%d,time=%lld,sleep_time=%lld",sk->state,state,cyg_current_time(),sleep_time);
	//set_current_state(TASK_RUNNING);

	return err;
}

struct net_proto_family bluez_sock_family_ops =
{
	PF_BLUETOOTH, bluez_sock_create
};


int bluez_init(void)
{
	//BT_INFO
	printf("BlueZ Core ver %s Copyright (C) 2000,2001 Qualcomm Inc\n",
		 VERSION);
	//BT_INFO
	printf("Written 2000,2001 by Maxim Krasnyansky <maxk@qualcomm.com>\n");

	sock_register(&bluez_sock_family_ops);
	
	/* Init l2cap */
	l2cap_init();
	
	/* Init rfcomm */
	rfcomm_init();

	/* Init HCI Core */
	hci_core_init();

	/* Init sockets */
	hci_sock_init();

	return 0;
}

void bluez_cleanup(void)
{
	/* Release socket */
	hci_sock_cleanup();

	/* Release core */
	hci_core_cleanup();

	sock_unregister(PF_BLUETOOTH);

	//remove_proc_entry("bluetooth", NULL);
}
/*
 *	This function is called by a protocol handler that wants to
 *	advertise its address family, and have it linked into the
 *	SOCKET module.
 */

int sock_register(struct net_proto_family *ops)
{
	int err;

	if (ops->family >= NPROTO) {
		diag_printf("protocol %d >= NPROTO(%d)\n", ops->family, NPROTO);
		return -ENOBUFS;
	}
	//net_family_write_lock();
	err = EEXIST;
	if (net_families[ops->family] == NULL) {
		net_families[ops->family]=ops;
		err = 0;
	}
	//net_family_write_unlock();
	return err;
}

/*
 *	This function is called by a protocol handler that wants to
 *	remove its address family, and have it unlinked from the
 *	SOCKET module.
 */

int sock_unregister(int family)
{
	if (family < 0 || family >= NPROTO)
		return -1;

//	net_family_write_lock();
	net_families[family]=NULL;
//	net_family_write_unlock();
	return 0;
}


struct sk_buff *skb_recv_datagram(struct sock *sk, unsigned flags, int noblock, int *err)
{
	int error;
	struct sk_buff *skb;
	long timeo;

	/* Caller is allowed not to check sk->err before skb_recv_datagram() */
	error = sock_error(sk);
	if (error)
		goto no_packet;

	timeo = sock_rcvtimeo(sk, noblock);

	do {
		/* Again only user level code calls this function, so nothing interrupt level
		   will suddenly eat the receive_queue.

		   Look at current nfs client by the way...
		   However, this function was corrent in any case. 8)
		 */
		if (flags & MSG_PEEK)
		{
			cyg_addrword_t cpu_flags;

			cyg_spinlock_spin_intsave(&sk->receive_queue.lock, &cpu_flags);
			skb = skb_peek(&sk->receive_queue);
			if(skb!=NULL)
				atomic_inc(&skb->users);
			cyg_spinlock_clear_intsave(&sk->receive_queue.lock, cpu_flags);
		} else
			skb = skb_dequeue(&sk->receive_queue);

		if (skb)
			return skb;

		/* User doesn't want to wait */
		error = -EAGAIN;
		if (!timeo)
			goto no_packet;

	} while (wait_for_packet(sk, err, &timeo) == 0);

	return NULL;

no_packet:
	*err = error;
	return NULL;
}

static int wait_for_packet(struct sock * sk, int *err, long *timeo_p)
{
	int error;
	cyg_tick_count_t sleep_time;

	DECLARE_WAITQUEUE(wait, current);

	//__set_current_state(TASK_INTERRUPTIBLE);
	//add_wait_queue_exclusive(sk->sleep, &wait);
	printf("wait_for_packet,sk = %x ",sk);

	/* Socket errors? */
	error = sock_error(sk);
	if (error)
		goto out_err;

	if (!skb_queue_empty(&sk->receive_queue))
		goto ready;

	/* Socket shut down? */
	if (sk->shutdown & RCV_SHUTDOWN)
		goto out_noerr;

	/* Sequenced packets can come disconnected. If so we report the problem */
	error = -ENOTCONN;
	if(connection_based(sk) && !(sk->state==TCP_ESTABLISHED || sk->state==TCP_LISTEN))
		goto out_err;

	//error = cyg_tsleep(sk, 0, NULL, *timeo_p);
	//cyg_semaphore_init(&sk->sem, 0);
	sleep_time = cyg_current_time() + *timeo_p;
	if(!cyg_semaphore_timed_wait(&sk->sem, sleep_time))
	{
		*timeo_p = 0;
		if(cyg_current_time() > sleep_time)
		{
        	error = -ETIMEDOUT;
        	goto out_err;
        }
        else
        {
        	error = -EINTR;
        	goto interrupted;
        }
	}
	else
		*timeo_p = 0;
		
ready:
	//current->state = TASK_RUNNING;
	//remove_wait_queue(sk->sleep, &wait);
	return 0;

interrupted:
	error = sock_intr_errno(*timeo_p);
out_err:
	*err = error;
out:
	//current->state = TASK_RUNNING;
	//remove_wait_queue(sk->sleep, &wait);
	return error;
out_noerr:
	*err = 0;
	error = 1;
	goto out;
}


int sock_create(int family, int type, int protocol, struct socket **res)
{
	int i;
	struct socket *sock;
	/*
	 *	Check protocol is in range
	 */
	if (family < 0 || family >= NPROTO)
		return -EAFNOSUPPORT;
	if (type < 0 || type >= SOCK_MAX)
		return -EINVAL;

	//net_family_read_lock();
	if (net_families[family] == NULL) {
		i = -EAFNOSUPPORT;
		goto out;
	}

/*
 *	Allocate the socket and allow the family to set things up. if
 *	the protocol is 0, the family is instructed to select an appropriate
 *	default.
 */
	if (!(sock = sock_alloc())) 
	{
		diag_printf("socket: no more sockets\n");
		i = -ENFILE;		/* Not exactly a match, but its the
					   closest posix thing */
		goto out;
	}

	sock->type  = type;

	if ((i = net_families[family]->create(sock, protocol)) < 0) 
	{
		sock_release(sock);
		goto out;
	}

	*res = sock;

out:
	//net_family_read_unlock();
	return i;
}
int sock_sendmsg(struct socket *sock, struct msghdr *msg, int size)
{
	int err = 0;
	//struct scm_cookie scm;

	//err = scm_send(sock, msg, &scm);
	//if (err >= 0) {
	err = sock->ops->sendmsg(sock, msg, size, 0);
		//scm_destroy(&scm);
	//}
	return err;
}

int sock_recvmsg(struct socket *sock, struct msghdr *msg, int size, int flags)
{
	//struct scm_cookie scm;

	//memset(&scm, 0, sizeof(scm));

	size = sock->ops->recvmsg(sock, msg, size, flags, 0);
	//if (size >= 0)
	//	scm_recv(sock, msg, &scm, flags);

	return size;
}

int sock_getsockopt(struct socket *sock, int level, int optname,
		    char *optval, int *optlen)
{
	struct sock *sk = sock->sk;
	
	union
	{
  		int val;
  		struct linger ling;
		struct timeval tm;
	} v;
	
	unsigned int lv=sizeof(int),len;
  	
  	if(get_user((int*)&len,optlen))
  		return -EFAULT;
	if(len < 0)
		return -EINVAL;
		
  	switch(optname) 
  	{
#if 0
		case SO_DEBUG:		
			v.val = sk->debug;
			break;
		
		case SO_DONTROUTE:
			v.val = sk->localroute;
			break;
		
		case SO_BROADCAST:
			v.val= sk->broadcast;
			break;

		case SO_SNDBUF:
			v.val=sk->sndbuf;
			break;
		
		case SO_RCVBUF:
			v.val =sk->rcvbuf;
			break;

		case SO_REUSEADDR:
			v.val = sk->reuse;
			break;

		case SO_KEEPALIVE:
			v.val = sk->keepopen;
			break;

		case SO_TYPE:
			v.val = sk->type;		  		
			break;

		case SO_ERROR:
			v.val = -sock_error(sk);
			if(v.val==0)
				v.val=xchg(&sk->err_soft,0);
			break;

		case SO_OOBINLINE:
			v.val = sk->urginline;
			break;
	
		case SO_NO_CHECK:
			v.val = sk->no_check;
			break;

		case SO_PRIORITY:
			v.val = sk->priority;
			break;
#endif
		case SO_LINGER:	
			lv=sizeof(v.ling);
			v.ling.l_onoff=sk->linger;
 			v.ling.l_linger=sk->lingertime/HZ;
			break;

#if 0					
		case SO_BSDCOMPAT:
			v.val = sk->bsdism;
			break;

		case SO_TIMESTAMP:
			v.val = sk->rcvtstamp;
			break;

		case SO_RCVTIMEO:
			lv=sizeof(struct timeval);
			if (sk->rcvtimeo == MAX_SCHEDULE_TIMEOUT) {
				v.tm.tv_sec = 0;
				v.tm.tv_usec = 0;
			} else {
				v.tm.tv_sec = sk->rcvtimeo/HZ;
				v.tm.tv_usec = ((sk->rcvtimeo%HZ)*1000)/HZ;
			}
			break;

		case SO_SNDTIMEO:
			lv=sizeof(struct timeval);
			if (sk->sndtimeo == MAX_SCHEDULE_TIMEOUT) {
				v.tm.tv_sec = 0;
				v.tm.tv_usec = 0;
			} else {
				v.tm.tv_sec = sk->sndtimeo/HZ;
				v.tm.tv_usec = ((sk->sndtimeo%HZ)*1000)/HZ;
			}
			break;

		case SO_RCVLOWAT:
			v.val = sk->rcvlowat;
			break;

		case SO_SNDLOWAT:
			v.val=1;
			break; 

		case SO_PASSCRED:
			v.val = sock->passcred;
			break;

		case SO_PEERCRED:
			if (len > sizeof(sk->peercred))
				len = sizeof(sk->peercred);
			if (copy_to_user(optval, &sk->peercred, len))
				return -EFAULT;
			goto lenout;

		case SO_PEERNAME:
		{
			char address[128];

			if (sock->ops->getname(sock, (struct sockaddr *)address, &lv, 2))
				return -ENOTCONN;
			if (lv < len)
				return -EINVAL;
			if(copy_to_user((void*)optval, address, len))
				return -EFAULT;
			goto lenout;
		}

		/* Dubious BSD thing... Probably nobody even uses it, but
		 * the UNIX standard wants it for whatever reason... -DaveM
		 */
		case SO_ACCEPTCONN:
			v.val = (sk->state == TCP_LISTEN);
			break;
#endif
		default:
			return(-ENOPROTOOPT);
	}
	if (len > lv)
		len = lv;
	if (copy_to_user(optval, &v, len))
		return -EFAULT;
lenout:
  	if (put_user(len, optlen))
  		return -EFAULT;
  	return 0;
}

int sock_setsockopt(struct socket *sock, int level, int optname,
		    char *optval, int optlen)
{
	struct sock *sk=sock->sk;

	int val;
	int valbool;
	struct linger ling;
	int ret = 0;
	
	/*
	 *	Options without arguments
	 */

#ifdef SO_DONTLINGER		/* Compatibility item... */
	switch(optname)
	{
		case SO_DONTLINGER:
			sk->linger=0;
			return 0;
	}
#endif	
		
  	if(optlen<sizeof(int))
  		return(-EINVAL);
  	
	if (get_user(&val, (int *)optval))
		return -EFAULT;
	
  	valbool = val?1:0;

	lock_sock(sk);

  	switch(optname) 
  	{
#if 0
		case SO_DEBUG:	
			if(val && !capable(CAP_NET_ADMIN))
			{
				ret = EACCES;
			}
			else
				sk->debug=valbool;
			break;
		case SO_REUSEADDR:
			sk->reuse = valbool;
			break;
		case SO_TYPE:
		case SO_ERROR:
			ret = -ENOPROTOOPT;
		  	break;
		case SO_DONTROUTE:
			sk->localroute=valbool;
			break;
		case SO_BROADCAST:
			sk->broadcast=valbool;
			break;
		case SO_SNDBUF:
			/* Don't error on this BSD doesn't and if you think
			   about it this is right. Otherwise apps have to
			   play 'guess the biggest size' games. RCVBUF/SNDBUF
			   are treated in BSD as hints */
			   
			if (val > sysctl_wmem_max)
				val = sysctl_wmem_max;

			sk->userlocks |= SOCK_SNDBUF_LOCK;
			if ((val * 2) < SOCK_MIN_SNDBUF)
				sk->sndbuf = SOCK_MIN_SNDBUF;
			else
				sk->sndbuf = (val * 2);

			/*
			 *	Wake up sending tasks if we
			 *	upped the value.
			 */
			sk->write_space(sk);
			break;

		case SO_RCVBUF:
			/* Don't error on this BSD doesn't and if you think
			   about it this is right. Otherwise apps have to
			   play 'guess the biggest size' games. RCVBUF/SNDBUF
			   are treated in BSD as hints */
			  
			if (val > sysctl_rmem_max)
				val = sysctl_rmem_max;

			sk->userlocks |= SOCK_RCVBUF_LOCK;
			/* FIXME: is this lower bound the right one? */
			if ((val * 2) < SOCK_MIN_RCVBUF)
				sk->rcvbuf = SOCK_MIN_RCVBUF;
			else
				sk->rcvbuf = (val * 2);
			break;

		case SO_KEEPALIVE:
			sk->keepopen = valbool;
			break;

	 	case SO_OOBINLINE:
			sk->urginline = valbool;
			break;

	 	case SO_NO_CHECK:
			sk->no_check = valbool;
			break;

		case SO_PRIORITY:
			if ((val >= 0 && val <= 6) || capable(CAP_NET_ADMIN)) 
				sk->priority = val;
			else
				ret = EPERM;
			break;
#endif
		case SO_LINGER:
			if(optlen<sizeof(ling)) {
				ret = -EINVAL;	/* 1003.1g */
				break;
			}
			if (copy_from_user(&ling,optval,sizeof(ling))) {
				ret = -EFAULT;
				break;
			}
			if(ling.l_onoff==0) {
				sk->linger=0;
			} else {
#if (BITS_PER_LONG == 32)
				if (ling.l_linger >= MAX_SCHEDULE_TIMEOUT/HZ)
					sk->lingertime=MAX_SCHEDULE_TIMEOUT;
				else
#endif
					sk->lingertime=ling.l_linger*HZ;
				sk->linger=1;
			}
			break;
#if 0
		case SO_BSDCOMPAT:
			sk->bsdism = valbool;
			break;

		case SO_PASSCRED:
			sock->passcred = valbool;
			break;

		case SO_TIMESTAMP:
			sk->rcvtstamp = valbool;
			break;

		case SO_RCVLOWAT:
			if (val < 0)
				val = INT_MAX;
			sk->rcvlowat = val ? : 1;
			break;

		case SO_RCVTIMEO:
			ret = sock_set_timeout(&sk->rcvtimeo, optval, optlen);
			break;

		case SO_SNDTIMEO:
			ret = sock_set_timeout(&sk->sndtimeo, optval, optlen);
			break;

#ifdef CONFIG_NETDEVICES
		case SO_BINDTODEVICE:
		{
			char devname[IFNAMSIZ]; 

			/* Sorry... */ 
			if (!capable(CAP_NET_RAW)) {
				ret = -EPERM;
				break;
			}

			/* Bind this socket to a particular device like "eth0",
			 * as specified in the passed interface name. If the
			 * name is "" or the option length is zero the socket 
			 * is not bound. 
			 */ 

			if (!valbool) {
				sk->bound_dev_if = 0;
			} else {
				if (optlen > IFNAMSIZ) 
					optlen = IFNAMSIZ; 
				if (copy_from_user(devname, optval, optlen)) {
					ret = -EFAULT;
					break;
				}

				/* Remove any cached route for this socket. */
				sk_dst_reset(sk);

				if (devname[0] == '\0') {
					sk->bound_dev_if = 0;
				} else {
					struct net_device *dev = dev_get_by_name(devname);
					if (!dev) {
						ret = -ENODEV;
						break;
					}
					sk->bound_dev_if = dev->ifindex;
					dev_put(dev);
				}
			}
			break;
		}
#endif

#endif//clyu
		/* We implement the SO_SNDLOWAT etc to
		   not be settable (1003.1g 5.3) */
		default:
		  	ret = -ENOPROTOOPT;
			break;
  	}
	release_sock(sk);
	return ret;
}

void sock_release(struct socket *sock)
{
	if (sock->ops) 
		sock->ops->release(sock);

	if (sock->fasync_list)
		diag_printf("sock_release: fasync list not empty!\n");

	sockets_in_use[0].counter--;
	//if (!sock->file) {
//		iput(sock->inode);
	//	return;
	//}
	//sock->file=NULL;
	free(sock);
}
/**
 *	sock_alloc	-	allocate a socket
 *	
 *	Allocate a new inode and socket object. The two are bound together
 *	and initialised. The socket is then returned. If we are out of inodes
 *	NULL is returned.
 */

struct socket *sock_alloc(void)
{
	struct socket * sock;

	sock = malloc(sizeof(*sock));
	if(!sock)
		return NULL;

	//init_waitqueue_head(&sock->wait);
	sock->fasync_list = NULL;
	sock->state = SS_UNCONNECTED;
	sock->flags = 0;
	sock->ops = NULL;
	sock->sk = NULL;
	sock->file = NULL;
	cyg_selinit(&sock->read_sb_sel);
    cyg_selinit(&sock->write_sb_sel);
    sock->read_flag_sel = 0;
    sock->write_flag_sel = 0;
//printf("sock_create=%x,%x ",sock,__builtin_return_address(0));
	sockets_in_use[0].counter++;
	return sock;
}

rwlock_t notifier_lock = RW_LOCK_UNLOCKED();

/**
 *	notifier_chain_register	- Add notifier to a notifier chain
 *	@list: Pointer to root list pointer
 *	@n: New entry in notifier chain
 *
 *	Adds a notifier to a notifier chain.
 *
 *	Currently always returns zero.
 */
 
int notifier_chain_register(struct notifier_block **list, struct notifier_block *n)
{
	write_lock(&notifier_lock);
	while(*list)
	{
		if(n->priority > (*list)->priority)
			break;
		list= &((*list)->next);
	}
	n->next = *list;
	*list=n;
	write_unlock(&notifier_lock);
	return 0;
}

/**
 *	notifier_chain_unregister - Remove notifier from a notifier chain
 *	@nl: Pointer to root list pointer
 *	@n: New entry in notifier chain
 *
 *	Removes a notifier from a notifier chain.
 *
 *	Returns zero on success, or %-ENOENT on failure.
 */
 
int notifier_chain_unregister(struct notifier_block **nl, struct notifier_block *n)
{
	write_lock(&notifier_lock);
	while((*nl)!=NULL)
	{
		if((*nl)==n)
		{
			*nl=n->next;
			write_unlock(&notifier_lock);
			return 0;
		}
		nl=&((*nl)->next);
	}
	write_unlock(&notifier_lock);
	return -ENOENT;
}

/**
 *	notifier_call_chain - Call functions in a notifier chain
 *	@n: Pointer to root pointer of notifier chain
 *	@val: Value passed unmodified to notifier function
 *	@v: Pointer passed unmodified to notifier function
 *
 *	Calls each function in a notifier chain in turn.
 *
 *	If the return value of the notifier can be and'd
 *	with %NOTIFY_STOP_MASK, then notifier_call_chain
 *	will return immediately, with the return value of
 *	the notifier function which halted execution.
 *	Otherwise, the return value is the return value
 *	of the last notifier function called.
 */
 
int notifier_call_chain(struct notifier_block **n, unsigned long val, void *v)
{
	int ret=NOTIFY_DONE;
	struct notifier_block *nb = *n;

	while(nb)
	{
		ret=nb->notifier_call(nb,val,v);
		if(ret&NOTIFY_STOP_MASK)
		{
			return ret;
		}
		nb=nb->next;
	}
	return ret;
}

int do_gettimeofday(struct timeval *tv)
{
    cyg_tick_count_t cur_time;
    cur_time = cyg_current_time();
    tv->tv_sec = cur_time / 100;
    tv->tv_usec = (cur_time % 100) * 10000;
}

/*
 *	Copy a datagram to an iovec.
 *	Note: the iovec is modified during the copy.
 */
int skb_copy_datagram_iovec(const struct sk_buff *skb, int offset, struct iovec *to,
			    int len)
{
	int i, copy;
	int start = skb->len - skb->data_len;

	/* Copy header. */
	if ((copy = start-offset) > 0) {
		if (copy > len)
			copy = len;
		if (memcpy_toiovec(to, skb->data + offset, copy))
			goto fault;
		if ((len -= copy) == 0)
			return 0;
		offset += copy;
	}

	/* Copy paged appendix. Hmm... why does this look so complicated? */
	for (i=0; i<skb_shinfo(skb)->nr_frags; i++) {
		int end;

		BUG_TRAP(start <= offset+len);

		end = start + skb_shinfo(skb)->frags[i].size;
		if ((copy = end-offset) > 0) {
			int err;
			cyg_uint8  *vaddr;
			skb_frag_t *frag = &skb_shinfo(skb)->frags[i];
			//struct page *page = frag->page;

			if (copy > len)
				copy = len;
			vaddr = (cyg_uint8  *)frag->page;//kmap(page);
			err = memcpy_toiovec(to, vaddr + frag->page_offset +
					     offset-start, copy);
			//kunmap(page);
			if (err)
				goto fault;
			if (!(len -= copy))
				return 0;
			offset += copy;
		}
		start = end;
	}

	if (skb_shinfo(skb)->frag_list) {
		struct sk_buff *list;

		for (list = skb_shinfo(skb)->frag_list; list; list=list->next) {
			int end;

			BUG_TRAP(start <= offset+len);

			end = start + list->len;
			if ((copy = end-offset) > 0) {
				if (copy > len)
					copy = len;
				if (skb_copy_datagram_iovec(list, offset-start, to, copy))
					goto fault;
				if ((len -= copy) == 0)
					return 0;
				offset += copy;
			}
			start = end;
		}
	}
	if (len == 0)
		return 0;

fault:
	return -EFAULT;
}

void skb_free_datagram(struct sock * sk, struct sk_buff *skb)
{
	kfree_skb(skb);
}
/*
 *	Is a socket 'connection oriented' ?
 */
 
static inline int connection_based(struct sock *sk)
{
	return (sk->type==SOCK_SEQPACKET || sk->type==SOCK_STREAM);
}

int put_cmsg(struct msghdr * msg, int level, int type, int len, void *data)
{
	struct cmsghdr *cm = (struct cmsghdr*)msg->msg_control;
	struct cmsghdr cmhdr;
	int cmlen = CMSG_LEN(len);
	int err;

	if (cm==NULL || msg->msg_controllen < sizeof(*cm)) {
		msg->msg_flags |= MSG_CTRUNC;
		return 0; /* XXX: return error? check spec. */
	}
	if (msg->msg_controllen < cmlen) {
		msg->msg_flags |= MSG_CTRUNC;
		cmlen = msg->msg_controllen;
	}
	cmhdr.cmsg_level = level;
	cmhdr.cmsg_type = type;
	cmhdr.cmsg_len = cmlen;

	err = -EFAULT;
	if (copy_to_user(cm, &cmhdr, sizeof cmhdr))
		goto out; 
	if (copy_to_user(CMSG_DATA(cm), data, cmlen - sizeof(struct cmsghdr)))
		goto out;
	cmlen = CMSG_SPACE(len);
	(char*)msg->msg_control += cmlen;
	msg->msg_controllen -= cmlen;
	err = 0;
out:
	return err;
}

/*
 *	Datagram poll: Again totally generic. This also handles
 *	sequenced packet sockets providing the socket receive queue
 *	is only ever holding data ready to receive.
 *
 *	Note: when you _don't_ use this routine for this protocol,
 *	and you use a different write policy from sock_writeable()
 *	then please supply your own write_space callback.
 */

unsigned int datagram_poll(struct CYG_FILE_TAG * file, struct socket *sock/*, poll_table *wait*/)
{
	struct sock *sk = sock->sk;
	unsigned int mask;

	//poll_wait(file, sk->sleep, wait);
//	cyg_semaphore_init(&sk->sem, 0);
#if 0
	if (timo) {
        sleep_time = cyg_current_time() + timo;
        if (!cyg_semaphore_timed_wait(&sk->sem, sleep_time)) {
            if( cyg_current_time() >= sleep_time )
                res = ETIMEDOUT;
            else
                res = EINTR;
        }
    } else {
#endif
#if 0
        if (!cyg_semaphore_wait(&sk->sem) ) {
            //return EINTR;
        }
#endif
    //}
	mask = 0;

	/* exceptional events? */
	if (sk->err || !skb_queue_empty(&sk->error_queue))
		mask |= POLLERR;
	if (sk->shutdown == SHUTDOWN_MASK)
		mask |= POLLHUP;

	/* readable? */
	if (!skb_queue_empty(&sk->receive_queue) || (sk->shutdown&RCV_SHUTDOWN))
		mask |= POLLIN | POLLRDNORM;

	/* Connection-based need to check for termination and startup */
	if (connection_based(sk)) {
		if (sk->state==TCP_CLOSE)
			mask |= POLLHUP;
		/* connection hasn't started yet? */
		if (sk->state == TCP_SYN_SENT)
			return mask;
	}

	/* writable? */
	if (sock_writeable(sk))
		mask |= POLLOUT | POLLWRNORM | POLLWRBAND;
	else
		set_bit(SOCK_ASYNC_NOSPACE, &sk->socket->flags);

	return mask;
}

extern void sockfd_put(struct socket *sock)
{
	//fput(sock->file);
}
#endif

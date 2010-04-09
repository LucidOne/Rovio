#ifndef __LINUX_NETLINK_H
#define __LINUX_NETLINK_H

#ifndef IFF_LOWER_UP
#define IFF_LOWER_UP   0x10000         /* driver signals L1 up         */
#endif
#ifndef IFF_DORMANT
#define IFF_DORMANT    0x20000         /* driver signals dormant       */
#endif

#ifndef IFLA_IFNAME
#define IFLA_IFNAME 3
#endif
#ifndef IFLA_WIRELESS
#define IFLA_WIRELESS 11
#endif
#ifndef IFLA_OPERSTATE
#define IFLA_OPERSTATE 16
#endif
#ifndef IFLA_LINKMODE
#define IFLA_LINKMODE 17
#define IF_OPER_DORMANT 5
#define IF_OPER_UP 6
#endif

#define RTMGRP_LINK 1
#define RTM_BASE 0x10
#define RTM_NEWLINK (RTM_BASE + 0)
#define RTM_DELLINK (RTM_BASE + 1)
#define RTM_SETLINK (RTM_BASE + 3)



#define NETLINK_ROUTE		0	/* Routing/device hook				*/
#define NETLINK_SKIP		1	/* Reserved for ENskip  			*/
#define NETLINK_USERSOCK	2	/* Reserved for user mode socket protocols 	*/
#define NETLINK_FIREWALL	3	/* Firewalling hook				*/
#define NETLINK_TCPDIAG		4	/* TCP socket monitoring			*/
#define NETLINK_NFLOG		5	/* netfilter/iptables ULOG */
#define NETLINK_ARPD		8
#define NETLINK_ROUTE6		11	/* af_inet6 route comm channel */
#define NETLINK_IP6_FW		13
#define NETLINK_DNRTMSG		14	/* DECnet routing messages */
#define NETLINK_TAPBASE		16	/* 16 to 31 are ethertap */

#define MAX_LINKS 32		

struct sockaddr_nl
{
	sa_family_t	nl_family;	/* AF_NETLINK	*/
	unsigned short	nl_pad;		/* zero		*/
	cyg_uint32		nl_pid;		/* process pid	*/
   	cyg_uint32		nl_groups;	/* multicast groups mask */
};

struct nlmsghdr
{
	cyg_uint32		nlmsg_len;	/* Length of message including header */
	cyg_uint16		nlmsg_type;	/* Message content */
	cyg_uint16		nlmsg_flags;	/* Additional flags */
	cyg_uint32		nlmsg_seq;	/* Sequence number */
	cyg_uint32		nlmsg_pid;	/* Sending process PID */
};

struct ifinfomsg
{
	unsigned char ifi_family;
	unsigned char __ifi_pad;
	unsigned short ifi_type;
	int ifi_index;
	unsigned ifi_flags;
	unsigned ifi_change;
};

struct rtattr
{
	unsigned short rta_len;
	unsigned short rta_type;
};

/* Flags values */

#define NLM_F_REQUEST		1	/* It is request message. 	*/
#define NLM_F_MULTI		2	/* Multipart message, terminated by NLMSG_DONE */
#define NLM_F_ACK		4	/* Reply with ack, with zero or error code */
#define NLM_F_ECHO		8	/* Echo this request 		*/

/* Modifiers to GET request */
#define NLM_F_ROOT	0x100	/* specify tree	root	*/
#define NLM_F_MATCH	0x200	/* return all matching	*/
#define NLM_F_ATOMIC	0x400	/* atomic GET		*/
#define NLM_F_DUMP	(NLM_F_ROOT|NLM_F_MATCH)

/* Modifiers to NEW request */
#define NLM_F_REPLACE	0x100	/* Override existing		*/
#define NLM_F_EXCL	0x200	/* Do not touch, if it exists	*/
#define NLM_F_CREATE	0x400	/* Create, if it does not exist	*/
#define NLM_F_APPEND	0x800	/* Add to end of list		*/

/*
   4.4BSD ADD		NLM_F_CREATE|NLM_F_EXCL
   4.4BSD CHANGE	NLM_F_REPLACE

   True CHANGE		NLM_F_CREATE|NLM_F_REPLACE
   Append		NLM_F_CREATE
   Check		NLM_F_EXCL
 */

#define NLMSG_ALIGNTO	4
#define NLMSG_ALIGN(len) ( ((len)+NLMSG_ALIGNTO-1) & ~(NLMSG_ALIGNTO-1) )
#define NLMSG_LENGTH(len) ((len)+NLMSG_ALIGN(sizeof(struct nlmsghdr)))
#define NLMSG_SPACE(len) NLMSG_ALIGN(NLMSG_LENGTH(len))
#define NLMSG_DATA(nlh)  ((void*)(((char*)nlh) + NLMSG_LENGTH(0)))
#define NLMSG_NEXT(nlh,len)	 ((len) -= NLMSG_ALIGN((nlh)->nlmsg_len), \
				  (struct nlmsghdr*)(((char*)(nlh)) + NLMSG_ALIGN((nlh)->nlmsg_len)))
#define NLMSG_OK(nlh,len) ((len) > 0 && (nlh)->nlmsg_len >= sizeof(struct nlmsghdr) && \
			   (nlh)->nlmsg_len <= (len))
#define NLMSG_PAYLOAD(nlh,len) ((nlh)->nlmsg_len - NLMSG_SPACE((len)))

#define NLMSG_NOOP		0x1	/* Nothing.		*/
#define NLMSG_ERROR		0x2	/* Error		*/
#define NLMSG_DONE		0x3	/* End of a dump	*/
#define NLMSG_OVERRUN		0x4	/* Data lost		*/

#define RTA_ALIGNTO 4
#define RTA_ALIGN(len) (((len) + RTA_ALIGNTO - 1) & ~(RTA_ALIGNTO - 1))
#define RTA_OK(rta,len) \
((len) > 0 && (rta)->rta_len >= sizeof(struct rtattr) && \
(rta)->rta_len <= (len))
#define RTA_NEXT(rta,attrlen) \
((attrlen) -= RTA_ALIGN((rta)->rta_len), \
(struct rtattr *) (((char *)(rta)) + RTA_ALIGN((rta)->rta_len)))
#define RTA_LENGTH(len) (RTA_ALIGN(sizeof(struct rtattr)) + (len))
#define RTA_DATA(rta) ((void *) (((char *) (rta)) + RTA_LENGTH(0)))

struct nlmsgerr
{
	int		error;
	struct nlmsghdr msg;
};

#define NET_MAJOR 36		/* Major 36 is reserved for networking 						*/

#ifdef __ECOS

struct ucred {
	cyg_uint32	pid;
	cyg_uint32	uid;
	cyg_uint32	gid;
};


struct netlink_skb_parms
{
	struct ucred		creds;		/* Skb credentials	*/
	cyg_uint32			pid;
	cyg_uint32			groups;
	cyg_uint32			dst_pid;
	cyg_uint32			dst_groups;
	cyg_uint32		eff_cap;
};

#define NETLINK_CB(skb)		(*(struct netlink_skb_parms*)&((skb)->cb))
#define NETLINK_CREDS(skb)	(&NETLINK_CB((skb)).creds)


//extern int netlink_attach(int unit, int (*function)(int,struct sk_buff *skb));
//extern void netlink_detach(int unit);
//extern int netlink_post(int unit, struct sk_buff *skb);
//extern int init_netlink(void);
//extern struct sock *netlink_kernel_create(int unit, void (*input)(struct sock *sk, int len));
//extern void netlink_ack(struct sk_buff *in_skb, struct nlmsghdr *nlh, int err);
//extern int netlink_unicast(struct sock *ssk, struct sk_buff *skb, cyg_uint32 pid, int nonblock);
//extern void netlink_broadcast(struct sock *ssk, struct sk_buff *skb, cyg_uint32 pid, cyg_uint32 group, int allocation);
//extern void netlink_set_err(struct sock *ssk, cyg_uint32 pid, cyg_uint32 group, int code);
//extern int netlink_register_notifier(struct notifier_block *nb);
//extern int netlink_unregister_notifier(struct notifier_block *nb);

/*
 *	skb should fit one page. This choice is good for headerless malloc.
 *
 *      FIXME: What is the best size for SLAB???? --ANK
 */
#define NLMSG_GOODSIZE PAGE_SIZE//(PAGE_SIZE - ((sizeof(struct sk_buff)+0xF)&~0xF))


struct netlink_callback
{
	//struct sk_buff	*skb;
	struct mbuf *m;
	struct nlmsghdr	*nlh;
	int		(*dump)(struct mbuf *m, struct netlink_callback *cb);
	int		(*done)(struct netlink_callback *cb);
	int		family;
	long		args[4];
};

struct netlink_notify
{
	int pid;
	int protocol;
};

static __inline__ struct nlmsghdr *
__nlmsg_put(struct mbuf *m, cyg_uint32 pid, cyg_uint32 seq, int type, int len)
{
	struct nlmsghdr *nlh;
	int size = NLMSG_LENGTH(len);

	nlh = mtod(m, struct nlmsghdr*);//(struct nlmsghdr*)skb_put(m, NLMSG_ALIGN(size));
	nlh->nlmsg_type = type;
	nlh->nlmsg_len = size;
	nlh->nlmsg_flags = 0;
	nlh->nlmsg_pid = pid;
	nlh->nlmsg_seq = seq;
	return nlh;
}

#define NLMSG_PUT(skb, pid, seq, type, len) \
({ if (skb_tailroom(skb) < (int)NLMSG_SPACE(len)) goto nlmsg_failure; \
   __nlmsg_put(skb, pid, seq, type, len); })

#if 0
extern int netlink_dump_start(struct sock *ssk, struct sk_buff *skb,
			      struct nlmsghdr *nlh,
			      int (*dump)(struct sk_buff *skb, struct netlink_callback*),
			      int (*done)(struct netlink_callback*));
#endif

#define NL_NONROOT_RECV 0x1
#define NL_NONROOT_SEND 0x2
extern void netlink_set_nonroot(int protocol, unsigned flag);


#endif /* __ECOS */

#endif	/* __LINUX_NETLINK_H */

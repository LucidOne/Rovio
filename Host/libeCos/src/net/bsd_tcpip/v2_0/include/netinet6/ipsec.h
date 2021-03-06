//==========================================================================
//
//      include/netinet6/ipsec.h
//
//==========================================================================
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD, 
// FreeBSD or other sources, and are covered by the appropriate
// copyright disclaimers included herein.
//
// Portions created by Red Hat are
// Copyright (C) 2002 Red Hat, Inc. All Rights Reserved.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================

/*	$KAME: ipsec.h,v 1.53 2001/11/20 08:32:38 itojun Exp $	*/

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * IPsec controller part.
 */

#ifndef _NETINET6_IPSEC_H_
#define _NETINET6_IPSEC_H_

#if defined(_KERNEL) && !defined(_LKM) && !defined(KLD_MODULE)
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
#include "opt_inet.h"
#include "opt_inet6.h"
#include "opt_ipsec.h"
#endif
#if defined(__NetBSD__)
#include "opt_inet.h"
#include "opt_ipsec.h"
#endif
#endif

#include <net/pfkeyv2.h>
#include <netkey/keydb.h>

#ifdef _KERNEL

/*
 * Security Policy Index
 * Ensure that both address families in the "src" and "dst" are same.
 * When the value of the ul_proto is ICMPv6, the port field in "src"
 * specifies ICMPv6 type, and the port field in "dst" specifies ICMPv6 code.
 */
struct secpolicyindex {
	u_int8_t dir;			/* direction of packet flow, see blow */
	struct sockaddr_storage src;	/* IP src address for SP */
	struct sockaddr_storage dst;	/* IP dst address for SP */
	u_int8_t prefs;			/* prefix length in bits for src */
	u_int8_t prefd;			/* prefix length in bits for dst */
	u_int16_t ul_proto;		/* upper layer Protocol */
#ifdef notyet
	uid_t uids;
	uid_t uidd;
	gid_t gids;
	gid_t gidd;
#endif
};

/* Security Policy Data Base */
struct secpolicy {
	LIST_ENTRY(secpolicy) chain;

	int refcnt;			/* reference count */
	struct secpolicyindex spidx;	/* selector */
	u_int32_t id;			/* It's unique number on the system. */
	u_int state;			/* 0: dead, others: alive */
#define IPSEC_SPSTATE_DEAD	0
#define IPSEC_SPSTATE_ALIVE	1

	u_int policy;		/* DISCARD, NONE or IPSEC, see keyv2.h */
	struct ipsecrequest *req;
				/* pointer to the ipsec request tree, */
				/* if policy == IPSEC else this value == NULL.*/

	/*
	 * lifetime handler.
	 * the policy can be used without limitiation if both lifetime and
	 * validtime are zero.
	 * "lifetime" is passed by sadb_lifetime.sadb_lifetime_addtime.
	 * "validtime" is passed by sadb_lifetime.sadb_lifetime_usetime.
	 */
	long created;		/* time created the policy */
	long lastused;		/* updated every when kernel sends a packet */
	long lifetime;		/* duration of the lifetime of this policy */
	long validtime;		/* duration this policy is valid without use */
};

/* Request for IPsec */
struct ifnet;
struct ipsecrequest {
	struct ipsecrequest *next;
				/* pointer to next structure */
				/* If NULL, it means the end of chain. */
	struct secasindex saidx;/* hint for search proper SA */
				/* if __ss_len == 0 then no address specified.*/
	u_int level;		/* IPsec level defined below. */

	struct secasvar *sav;	/* place holder of SA for use */
	struct secpolicy *sp;	/* back pointer to SP */

	struct ifnet *tunifp;	/* interface for tunnelling */
};

/* security policy in PCB */
struct inpcbpolicy {
	struct secpolicy *sp_in;
	struct secpolicy *sp_out;
	int priv;			/* privileged socket ? */

	/* cached policy */
	/* XXX 3 == IPSEC_DIR_MAX */
	struct secpolicy *cache[3];
	struct secpolicyindex cacheidx[3];
	int cachegen[3]; 	/* cache generation #, the time we filled it */
	int cacheflags;	
#define IPSEC_PCBSP_CONNECTED	1
};

/* SP acquiring list table. */
struct secspacq {
	LIST_ENTRY(secspacq) chain;

	struct secpolicyindex spidx;

	long created;		/* for lifetime */
	int count;		/* for lifetime */
	/* XXX: here is mbuf place holder to be sent ? */
};

struct ipsecaux {
	struct socket *so;
	int hdrs;	/* # of ipsec headers */

	struct secpolicy *sp;
	struct ipsecrequest *req;
};
#endif /* _KERNEL */

/* according to IANA assignment, port 0x0000 and proto 0xff are reserved. */
#define IPSEC_PORT_ANY		0
#define IPSEC_ULPROTO_ANY	255
#define IPSEC_PROTO_ANY		255

/* mode of security protocol */
/* NOTE: DON'T use IPSEC_MODE_ANY at SPD.  It's only use in SAD */
#define	IPSEC_MODE_ANY		0	/* i.e. wildcard. */
#define	IPSEC_MODE_TRANSPORT	1
#define	IPSEC_MODE_TUNNEL	2

/*
 * Direction of security policy.
 * NOTE: Since INVALID is used just as flag.
 * The other are used for loop counter too.
 */
#define IPSEC_DIR_ANY		0
#define IPSEC_DIR_INBOUND	1
#define IPSEC_DIR_OUTBOUND	2
#define IPSEC_DIR_MAX		3
#define IPSEC_DIR_INVALID	4

/* Policy level */
/*
 * IPSEC, ENTRUST and BYPASS are allowed for setsockopt() in PCB,
 * DISCARD, IPSEC and NONE are allowed for setkey() in SPD.
 * DISCARD and NONE are allowed for system default.
 */
#define IPSEC_POLICY_DISCARD	0	/* discarding packet */
#define IPSEC_POLICY_NONE	1	/* through IPsec engine */
#define IPSEC_POLICY_IPSEC	2	/* do IPsec */
#define IPSEC_POLICY_ENTRUST	3	/* consulting SPD if present. */
#define IPSEC_POLICY_BYPASS	4	/* only for privileged socket. */

/* Security protocol level */
#define	IPSEC_LEVEL_DEFAULT	0	/* reference to system default */
#define	IPSEC_LEVEL_USE		1	/* use SA if present. */
#define	IPSEC_LEVEL_REQUIRE	2	/* require SA. */
#define	IPSEC_LEVEL_UNIQUE	3	/* unique SA. */

#define IPSEC_MANUAL_REQID_MAX	0x3fff
				/*
				 * if security policy level == unique, this id
				 * indicate to a relative SA for use, else is
				 * zero.
				 * 1 - 0x3fff are reserved for manual keying.
				 * 0 are reserved for above reason.  Others is
				 * for kernel use.
				 * Note that this id doesn't identify SA
				 * by only itself.
				 */
#define IPSEC_REPLAYWSIZE  32

/* statistics for ipsec processing */
struct ipsecstat {
	u_quad_t in_success;  /* succeeded inbound process */
	u_quad_t in_polvio;
			/* security policy violation for inbound process */
	u_quad_t in_nosa;     /* inbound SA is unavailable */
	u_quad_t in_inval;    /* inbound processing failed due to EINVAL */
	u_quad_t in_nomem;    /* inbound processing failed due to ENOBUFS */
	u_quad_t in_badspi;   /* failed getting a SPI */
	u_quad_t in_ahreplay; /* AH replay check failed */
	u_quad_t in_espreplay; /* ESP replay check failed */
	u_quad_t in_ahauthsucc; /* AH authentication success */
	u_quad_t in_ahauthfail; /* AH authentication failure */
	u_quad_t in_espauthsucc; /* ESP authentication success */
	u_quad_t in_espauthfail; /* ESP authentication failure */
	u_quad_t in_esphist[256];
	u_quad_t in_ahhist[256];
	u_quad_t in_comphist[256];
	u_quad_t out_success; /* succeeded outbound process */
	u_quad_t out_polvio;
			/* security policy violation for outbound process */
	u_quad_t out_nosa;    /* outbound SA is unavailable */
	u_quad_t out_inval;   /* outbound process failed due to EINVAL */
	u_quad_t out_nomem;    /* inbound processing failed due to ENOBUFS */
	u_quad_t out_noroute; /* there is no route */
	u_quad_t out_esphist[256];
	u_quad_t out_ahhist[256];
	u_quad_t out_comphist[256];

	u_quad_t spdcachelookup;
	u_quad_t spdcachemiss;
};

/*
 * Definitions for IPsec & Key sysctl operations.
 */
/*
 * Names for IPsec & Key sysctl objects
 */
#define IPSECCTL_STATS			1	/* stats */
#define IPSECCTL_DEF_POLICY		2
#define IPSECCTL_DEF_ESP_TRANSLEV	3	/* int; ESP transport mode */
#define IPSECCTL_DEF_ESP_NETLEV		4	/* int; ESP tunnel mode */
#define IPSECCTL_DEF_AH_TRANSLEV	5	/* int; AH transport mode */
#define IPSECCTL_DEF_AH_NETLEV		6	/* int; AH tunnel mode */
#if 0	/* obsolete, do not reuse */
#define IPSECCTL_INBOUND_CALL_IKE	7
#endif
#define	IPSECCTL_AH_CLEARTOS		8
#define	IPSECCTL_AH_OFFSETMASK		9
#define	IPSECCTL_DFBIT			10
#define	IPSECCTL_ECN			11
#define	IPSECCTL_DEBUG			12
#define	IPSECCTL_ESP_RANDPAD		13
#define IPSECCTL_MAXID			14

#define IPSECCTL_NAMES { \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "def_policy", CTLTYPE_INT }, \
	{ "esp_trans_deflev", CTLTYPE_INT }, \
	{ "esp_net_deflev", CTLTYPE_INT }, \
	{ "ah_trans_deflev", CTLTYPE_INT }, \
	{ "ah_net_deflev", CTLTYPE_INT }, \
	{ 0, 0 }, \
	{ "ah_cleartos", CTLTYPE_INT }, \
	{ "ah_offsetmask", CTLTYPE_INT }, \
	{ "dfbit", CTLTYPE_INT }, \
	{ "ecn", CTLTYPE_INT }, \
	{ "debug", CTLTYPE_INT }, \
	{ "esp_randpad", CTLTYPE_INT }, \
}

#define IPSEC6CTL_NAMES { \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "def_policy", CTLTYPE_INT }, \
	{ "esp_trans_deflev", CTLTYPE_INT }, \
	{ "esp_net_deflev", CTLTYPE_INT }, \
	{ "ah_trans_deflev", CTLTYPE_INT }, \
	{ "ah_net_deflev", CTLTYPE_INT }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "ecn", CTLTYPE_INT }, \
	{ "debug", CTLTYPE_INT }, \
	{ "esp_randpad", CTLTYPE_INT }, \
}

#ifdef __bsdi__
#define IPSECCTL_VARS { \
	0, \
	0, \
	&ip4_def_policy.policy, \
	&ip4_esp_trans_deflev, \
	&ip4_esp_net_deflev, \
	&ip4_ah_trans_deflev, \
	&ip4_ah_net_deflev, \
	0, \
	&ip4_ah_cleartos, \
	&ip4_ah_offsetmask, \
	&ip4_ipsec_dfbit, \
	&ip4_ipsec_ecn, \
	&ipsec_debug, \
	&ip4_esp_randpad, \
}

#define IPSEC6CTL_VARS { \
	0, \
	0, \
	&ip6_def_policy.policy, \
	&ip6_esp_trans_deflev, \
	&ip6_esp_net_deflev, \
	&ip6_ah_trans_deflev, \
	&ip6_ah_net_deflev, \
	0, \
	0, \
	0, \
	0, \
	&ip6_ipsec_ecn, \
	&ipsec_debug, \
	&ip6_esp_randpad, \
}
#endif

#ifdef _KERNEL
struct ipsec_output_state {
	struct mbuf *m;
	struct route *ro;
	struct sockaddr *dst;
	int encap;
};

struct ipsec_history {
	int ih_proto;
	u_int32_t ih_spi;
};

extern int ipsec_debug;
extern int ipsec_tunnel_device;

#ifdef INET
extern struct ipsecstat ipsecstat;
extern struct secpolicy ip4_def_policy;
extern int ip4_esp_trans_deflev;
extern int ip4_esp_net_deflev;
extern int ip4_ah_trans_deflev;
extern int ip4_ah_net_deflev;
extern int ip4_ah_cleartos;
extern int ip4_ah_offsetmask;
extern int ip4_ipsec_dfbit;
extern int ip4_ipsec_ecn;
extern int ip4_esp_randpad;
#endif

#ifdef INET6
extern struct ipsecstat ipsec6stat;
extern struct secpolicy ip6_def_policy;
extern int ip6_esp_trans_deflev;
extern int ip6_esp_net_deflev;
extern int ip6_ah_trans_deflev;
extern int ip6_ah_net_deflev;
extern int ip6_ipsec_ecn;
extern int ip6_esp_randpad;
#endif

#define ipseclog(x)	do { if (ipsec_debug) log x; } while (0)

extern int ipsec_pcbconn __P((struct inpcbpolicy *));
extern int ipsec_pcbdisconn __P((struct inpcbpolicy *));
extern int ipsec_invalpcbcacheall __P((void));

extern struct secpolicy *ipsec4_getpolicybysock
	__P((struct mbuf *, u_int, struct socket *, int *));
extern struct secpolicy *ipsec4_getpolicybyaddr
	__P((struct mbuf *, u_int, int, int *));

#ifdef INET6
extern struct secpolicy *ipsec6_getpolicybysock
	__P((struct mbuf *, u_int, struct socket *, int *));
extern struct secpolicy *ipsec6_getpolicybyaddr
	__P((struct mbuf *, u_int, int, int *));
#endif /* INET6 */

struct inpcb;
#ifdef INET6
struct in6pcb;
#endif
extern int ipsec_init_policy __P((struct socket *so, struct inpcbpolicy **));
extern int ipsec_copy_policy
	__P((struct inpcbpolicy *, struct inpcbpolicy *));
extern u_int ipsec_get_reqlevel __P((struct ipsecrequest *));

extern int ipsec4_set_policy __P((struct inpcb *inp, int optname,
	caddr_t request, size_t len, int priv));
extern int ipsec4_get_policy __P((struct inpcb *inpcb, caddr_t request,
	size_t len, struct mbuf **mp));
extern int ipsec4_delete_pcbpolicy __P((struct inpcb *));
extern int ipsec4_in_reject_so __P((struct mbuf *, struct socket *));
extern int ipsec4_in_reject __P((struct mbuf *, struct inpcb *));

#ifdef INET6
extern int ipsec6_in_reject_so __P((struct mbuf *, struct socket *));
#if (defined(__FreeBSD__) && __FreeBSD__ >= 3) || (defined(__bsdi__) && _BSDI_VERSION >= 199802)
extern int ipsec6_delete_pcbpolicy __P((struct inpcb *));
extern int ipsec6_set_policy __P((struct inpcb *inp, int optname,
	caddr_t request, size_t len, int priv));
extern int ipsec6_get_policy
	__P((struct inpcb *inp, caddr_t request, size_t len, struct mbuf **mp));
extern int ipsec6_in_reject __P((struct mbuf *, struct inpcb *));
#else
extern int ipsec6_delete_pcbpolicy __P((struct in6pcb *));
extern int ipsec6_set_policy __P((struct in6pcb *in6p, int optname,
	caddr_t request, size_t len, int priv));
extern int ipsec6_get_policy __P((struct in6pcb *in6p, caddr_t request,
	size_t len, struct mbuf **mp));
extern int ipsec6_in_reject __P((struct mbuf *, struct in6pcb *));
#endif
#endif /* INET6 */

struct secas;
struct tcpcb;
struct tcp6cb;
extern int ipsec_chkreplay __P((u_int32_t, struct secasvar *));
extern int ipsec_updatereplay __P((u_int32_t, struct secasvar *));

extern size_t ipsec4_hdrsiz __P((struct mbuf *, u_int, struct inpcb *));
#if (defined(__FreeBSD__) && __FreeBSD__ == 3) || (defined(__bsdi__) && _BSDI_VERSION >= 199802)
extern size_t ipsec_hdrsiz_tcp __P((struct tcpcb *, int));
#elif  (defined(__FreeBSD__) && __FreeBSD__ >= 4)
extern size_t ipsec_hdrsiz_tcp __P((struct tcpcb *));
#else
extern size_t ipsec4_hdrsiz_tcp __P((struct tcpcb *));
#endif
#ifdef INET6
#if (defined(__FreeBSD__) && __FreeBSD__ >= 3) || (defined(__bsdi__) && _BSDI_VERSION >= 199802)
extern size_t ipsec6_hdrsiz __P((struct mbuf *, u_int, struct inpcb *));
#else
extern size_t ipsec6_hdrsiz __P((struct mbuf *, u_int, struct in6pcb *));
#if defined(__NetBSD__) && !defined(TCP6)
extern size_t ipsec6_hdrsiz_tcp __P((struct tcpcb *));
#else
extern size_t ipsec6_hdrsiz_tcp __P((struct tcp6cb *));
#endif
#endif
#endif

struct ip;
#ifdef INET6
struct ip6_hdr;
#endif
extern const char *ipsec4_logpacketstr __P((struct ip *, u_int32_t));
#ifdef INET6
extern const char *ipsec6_logpacketstr __P((struct ip6_hdr *, u_int32_t));
#endif
extern const char *ipsec_logsastr __P((struct secasvar *));

extern void ipsec_dumpmbuf __P((struct mbuf *));

extern int ipsec4_output __P((struct ipsec_output_state *, struct secpolicy *,
	int));
#ifdef INET6
extern int ipsec6_output_trans __P((struct ipsec_output_state *, u_char *,
	struct mbuf *, struct secpolicy *, int, int *));
extern int ipsec6_output_tunnel __P((struct ipsec_output_state *,
	struct secpolicy *, int));
#endif
extern int ipsec4_tunnel_validate __P((struct mbuf *, int, u_int,
	struct secasvar *));
#ifdef INET6
extern int ipsec6_tunnel_validate __P((struct mbuf *, int, u_int,
	struct secasvar *));
#endif
extern struct mbuf *ipsec_copypkt __P((struct mbuf *));
extern void ipsec_delaux __P((struct mbuf *));
extern int ipsec_setsocket __P((struct mbuf *, struct socket *));
extern struct socket *ipsec_getsocket __P((struct mbuf *));
extern int ipsec_addhist __P((struct mbuf *, int, u_int32_t)); 
extern int ipsec_getnhist __P((struct mbuf *));
extern struct ipsec_history *ipsec_gethist __P((struct mbuf *, int *));
extern void ipsec_clearhist __P((struct mbuf *));

#if defined(__bsdi__) || defined(__NetBSD__)
extern int ipsec_sysctl __P((int *, u_int, void *, size_t *, void *, size_t));
extern int ipsec6_sysctl __P((int *, u_int, void *, size_t *, void *, size_t));
#endif /* __bsdi__ || __NetBSD__ */

#endif /* _KERNEL */

#ifndef _KERNEL
extern caddr_t ipsec_set_policy __P((char *, int));
extern int ipsec_get_policylen __P((caddr_t));
extern char *ipsec_dump_policy __P((caddr_t, char *));

extern const char *ipsec_strerror __P((void));
#endif /* !_KERNEL */

#endif /* _NETINET6_IPSEC_H_ */

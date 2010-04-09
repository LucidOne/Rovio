/*
 * This file implement the Wireless Extensions APIs.
 *
 * Authors :	Jean Tourrilhes - HPL - <jt@hpl.hp.com>
 * Copyright (c) 1997-2006 Jean Tourrilhes, All Rights Reserved.
 *
 * (As all part of the Linux kernel, this file is GPL)
 */

/************************** DOCUMENTATION **************************/
/*
 * API definition :
 * --------------
 * See <linux/wireless.h> for details of the APIs and the rest.
 *
 * History :
 * -------
 *
 * v1 - 5.12.01 - Jean II
 *	o Created this file.
 *
 * v2 - 13.12.01 - Jean II
 *	o Move /proc/net/wireless stuff from net/core/dev.c to here
 *	o Make Wireless Extension IOCTLs go through here
 *	o Added iw_handler handling ;-)
 *	o Added standard ioctl description
 *	o Initial dumb commit strategy based on orinoco.c
 *
 * v3 - 19.12.01 - Jean II
 *	o Make sure we don't go out of standard_ioctl[] in ioctl_standard_call
 *	o Add event dispatcher function
 *	o Add event description
 *	o Propagate events as rtnetlink IFLA_WIRELESS option
 *	o Generate event on selected SET requests
 *
 * v4 - 18.04.02 - Jean II
 *	o Fix stupid off by one in iw_ioctl_description : IW_ESSID_MAX_SIZE + 1
 *
 * v5 - 21.06.02 - Jean II
 *	o Add IW_PRIV_TYPE_ADDR in priv_type_size (+cleanup)
 *	o Reshuffle IW_HEADER_TYPE_XXX to map IW_PRIV_TYPE_XXX changes
 *	o Add IWEVCUSTOM for driver specific event/scanning token
 *	o Turn on WE_STRICT_WRITE by default + kernel warning
 *	o Fix WE_STRICT_WRITE in ioctl_export_private() (32 => iw_num)
 *	o Fix off-by-one in test (extra_size <= IFNAMSIZ)
 *
 * v6 - 9.01.03 - Jean II
 *	o Add common spy support : iw_handler_set_spy(), wireless_spy_update()
 *	o Add enhanced spy support : iw_handler_set_thrspy() and event.
 *	o Add WIRELESS_EXT version display in /proc/net/wireless
 *
 * v6 - 18.06.04 - Jean II
 *	o Change get_spydata() method for added safety
 *	o Remove spy #ifdef, they are always on -> cleaner code
 *	o Allow any size GET request if user specifies length > max
 *		and if request has IW_DESCR_FLAG_NOMAX flag or is SIOCGIWPRIV
 *	o Start migrating get_wireless_stats to struct iw_handler_def
 *	o Add wmb() in iw_handler_set_spy() for non-coherent archs/cpus
 * Based on patch from Pavel Roskin <proski@gnu.org> :
 *	o Fix kernel data leak to user space in private handler handling
 *
 * v7 - 18.3.05 - Jean II
 *	o Remove (struct iw_point *)->pointer from events and streams
 *	o Remove spy_offset from struct iw_handler_def
 *	o Start deprecating dev->get_wireless_stats, output a warning
 *	o If IW_QUAL_DBM is set, show dBm values in /proc/net/wireless
 *	o Don't loose INVALID/DBM flags when clearing UPDATED flags (iwstats)
 *
 * v8 - 17.02.06 - Jean II
 *	o RtNetlink requests support (SET/GET)
 */

/***************************** INCLUDES *****************************/

#include "pkgconf/system.h"
#include "cyg/error/codes.h"
#include "cyg/infra/cyg_type.h"  // Common type definitions and support
                                 // including endian-ness
#include "cyg/infra/diag.h"
#include "cyg/io/eth/netdev.h"
#include "cyg/io/eth/eth_drv.h"
#include "cyg/io/eth/eth_drv_stats.h"
#include "cyg/hal/hal_intr.h"
#include "net/wireless.h"
#include "net/iw_handler.h"
#include    "wlan_defs.h"
#include    "wlan_thread.h"
#include    "wlan_types.h"
#include "sys/malloc.h"
#include "sys/socket.h"
#include "sys/mbuf.h"
#include "net/netlink.h"
#include "asm/uaccess.h"

//#include    "wlan_wmm.h"
#include    "wlan_11d.h"

//#include    "os_timers.h"

#include    "host.h"
#include    "hostcmd.h"

#include    "wlan_scan.h"
#include    "wlan_join.h"

#include    "wlan_dev.h"

//#define WE_EVENT_DEBUG

#define	WE_EVENT_RTNETLINK


/************************* GLOBAL VARIABLES *************************/
/*
 * You should not use global variables, because of re-entrancy.
 * On our case, it's only const, so it's OK...
 */
/*
 * Meta-data about all the standard Wireless Extension request we
 * know about.
 */
static const struct iw_ioctl_description standard_ioctl[] = {
	/*[SIOCSIWCOMMIT	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_NULL,
	},
	/*[SIOCGIWNAME	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_CHAR,
		0, 0, 0, 0,
		/*.flags		= */IW_DESCR_FLAG_DUMP,
	},
	/*[SIOCSIWNWID	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_PARAM,
		0, 0, 0, 0,
		/*.flags		= */IW_DESCR_FLAG_EVENT,
	},
	/*[SIOCGIWNWID	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_PARAM,
		0, 0, 0, 0,
		/*.flags		= */IW_DESCR_FLAG_DUMP,
	},
	/*[SIOCSIWFREQ	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_FREQ,
		0, 0, 0, 0,
		/*.flags		= */IW_DESCR_FLAG_EVENT,
	},
	/*[SIOCGIWFREQ	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_FREQ,
		0, 0, 0, 0,
		/*.flags		= */IW_DESCR_FLAG_DUMP,
	},
	/*[SIOCSIWMODE	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_UINT,
		0, 0, 0, 0,
		/*.flags		= */IW_DESCR_FLAG_EVENT,
	},
	/*[SIOCGIWMODE	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_UINT,
		0, 0, 0, 0,
		/*.flags		= */IW_DESCR_FLAG_DUMP,
	},
	/*[SIOCSIWSENS	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_PARAM,
	},
	/*[SIOCGIWSENS	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_PARAM,
	},
	/*[SIOCSIWRANGE	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_NULL,
	},
	/*[SIOCGIWRANGE	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */1,
		0,
		/*.max_tokens	= */sizeof(struct iw_range),
		/*.flags		= */IW_DESCR_FLAG_DUMP,
	},
	/*[SIOCSIWPRIV	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_NULL,
	},
	/*[SIOCGIWPRIV	- SIOCIWFIRST] = */{ /* (handled directly by us) */
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */sizeof(struct iw_priv_args),
		0,
		/*.max_tokens	= */16,
		/*.flags		= */IW_DESCR_FLAG_NOMAX,
	},
	/*[SIOCSIWSTATS	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_NULL,
	},
	/*[SIOCGIWSTATS	- SIOCIWFIRST] = */{ /* (handled directly by us) */
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */1,
		0,
		/*.max_tokens	= */sizeof(struct iw_statistics),
		/*.flags		= */IW_DESCR_FLAG_DUMP,
	},
	/*[SIOCSIWSPY	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */sizeof(struct sockaddr),
		0,
		/*.max_tokens	= */IW_MAX_SPY,
	},
	/*[SIOCGIWSPY	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */sizeof(struct sockaddr) +
				  sizeof(struct iw_quality),
		0,
		/*.max_tokens	= */IW_MAX_SPY,
	},
	/*[SIOCSIWTHRSPY	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */sizeof(struct iw_thrspy),
		/*.min_tokens	= */1,
		/*.max_tokens	= */1,
	},
	/*[SIOCGIWTHRSPY	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */sizeof(struct iw_thrspy),
		/*.min_tokens	= */1,
		/*.max_tokens	= */1,
	},
	/*[SIOCSIWAP	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_ADDR,
	},
	/*[SIOCGIWAP	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_ADDR,
		0, 0, 0, 0,
		/*.flags		= */IW_DESCR_FLAG_DUMP,
	},
	/*[SIOCSIWMLME	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */1,
		/*.min_tokens	= */sizeof(struct iw_mlme),
		/*.max_tokens	= */sizeof(struct iw_mlme),
	},
	/*[SIOCGIWAPLIST	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */sizeof(struct sockaddr) +
				  sizeof(struct iw_quality),
		0,
		/*.max_tokens	= */IW_MAX_AP,
		/*.flags		= */IW_DESCR_FLAG_NOMAX,
	},
	/*[SIOCSIWSCAN	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */1,
		/*.min_tokens	= */0,
		/*.max_tokens	= */sizeof(struct iw_scan_req),
	},
	/*[SIOCGIWSCAN	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */1,
		0,
		/*.max_tokens	= */IW_SCAN_MAX_DATA,
		/*.flags		= */IW_DESCR_FLAG_NOMAX,
	},
	/*[SIOCSIWESSID	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */1,
		0,
		/*.max_tokens	= */IW_ESSID_MAX_SIZE + 1,
		/*.flags		= */IW_DESCR_FLAG_EVENT,
	},
	/*[SIOCGIWESSID	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */1,
		0,
		/*.max_tokens	= */IW_ESSID_MAX_SIZE + 1,
		/*.flags		= */IW_DESCR_FLAG_DUMP,
	},
	/*[SIOCSIWNICKN	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */1,
		0,
		/*.max_tokens	= */IW_ESSID_MAX_SIZE + 1,
	},
	/*[SIOCGIWNICKN	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */1,
		0,
		/*.max_tokens	= */IW_ESSID_MAX_SIZE + 1,
	},{},{},
	/*[SIOCSIWRATE	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_PARAM,
	},
	/*[SIOCGIWRATE	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_PARAM,
	},
	/*[SIOCSIWRTS	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_PARAM,
	},
	/*[SIOCGIWRTS	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_PARAM,
	},
	/*[SIOCSIWFRAG	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_PARAM,
	},
	/*[SIOCGIWFRAG	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_PARAM,
	},
	/*[SIOCSIWTXPOW	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_PARAM,
	},
	/*[SIOCGIWTXPOW	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_PARAM,
	},
	/*[SIOCSIWRETRY	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_PARAM,
	},
	/*[SIOCGIWRETRY	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_PARAM,
	},
	/*[SIOCSIWENCODE	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */1,
		0,
		/*.max_tokens	= */IW_ENCODING_TOKEN_MAX,
		/*.flags		= */IW_DESCR_FLAG_EVENT | IW_DESCR_FLAG_RESTRICT,
	},
	/*[SIOCGIWENCODE	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */1,
		0,
		/*.max_tokens	= */IW_ENCODING_TOKEN_MAX,
		/*.flags		= */IW_DESCR_FLAG_DUMP | IW_DESCR_FLAG_RESTRICT,
	},
	/*[SIOCSIWPOWER	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_PARAM,
	},
	/*0x8B2D*//*[SIOCGIWPOWER	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_PARAM,
	},{},{},
	/*0x8B30*//*[SIOCSIWGENIE	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */1,
		0,
		/*.max_tokens	= */IW_GENERIC_IE_MAX,
	},
	/*[SIOCGIWGENIE	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */1,
		0,
		/*.max_tokens	= */IW_GENERIC_IE_MAX,
	},
	/*[SIOCSIWAUTH	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_PARAM,
	},
	/*[SIOCGIWAUTH	- SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_PARAM,
	},
	/*[SIOCSIWENCODEEXT - SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */1,
		/*.min_tokens	= */sizeof(struct iw_encode_ext),
		/*.max_tokens	= */sizeof(struct iw_encode_ext) +
				  IW_ENCODING_TOKEN_MAX,
	},
	/*[SIOCGIWENCODEEXT - SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */1,
		/*.min_tokens	= */sizeof(struct iw_encode_ext),
		/*.max_tokens	= */sizeof(struct iw_encode_ext) +
				  IW_ENCODING_TOKEN_MAX,
	},
	/*[SIOCSIWPMKSA - SIOCIWFIRST] = */{
		/*.header_type	= */IW_HEADER_TYPE_POINT,
		0,
		/*.token_size	= */1,
		/*.min_tokens	= */sizeof(struct iw_pmksa),
		/*.max_tokens	= */sizeof(struct iw_pmksa),
	},
};
static const int standard_ioctl_num = (sizeof(standard_ioctl) /
				       sizeof(struct iw_ioctl_description));

/*
 * Meta-data about all the additional standard Wireless Extension events
 * we know about.
 */
static const struct iw_ioctl_description standard_event[] = {
	/*[IWEVTXDROP	- IWEVFIRST] = */{
		/*header_type*/IW_HEADER_TYPE_ADDR,
	},
	/*[IWEVQUAL	- IWEVFIRST] = */{
		IW_HEADER_TYPE_QUAL,
	},
	/*[IWEVCUSTOM	- IWEVFIRST] = */{
		IW_HEADER_TYPE_POINT,
		0,
		1,
		0,
		IW_CUSTOM_MAX,
	},
	/*[IWEVREGISTERED	- IWEVFIRST] = */{
		IW_HEADER_TYPE_ADDR,
	},
	/*[IWEVEXPIRED	- IWEVFIRST] = */{
		IW_HEADER_TYPE_ADDR, 
	},
	/*[IWEVGENIE	- IWEVFIRST] = */{
		IW_HEADER_TYPE_POINT,
		0, 1, 0,
		IW_GENERIC_IE_MAX,
	},
	/*[IWEVMICHAELMICFAILURE	- IWEVFIRST] = */{
		IW_HEADER_TYPE_POINT, 
		0, 1, 0,
		sizeof(struct iw_michaelmicfailure),
	},
	/*[IWEVASSOCREQIE	- IWEVFIRST] = */{
		IW_HEADER_TYPE_POINT,
		0, 1, 0,
		IW_GENERIC_IE_MAX,
	},
	/*[IWEVASSOCRESPIE	- IWEVFIRST] = */{
		IW_HEADER_TYPE_POINT,
		0, 1, 0,
		IW_GENERIC_IE_MAX,
	},
	/*[IWEVPMKIDCAND	- IWEVFIRST] = */{
		IW_HEADER_TYPE_POINT,
		0, 1, 0,
		sizeof(struct iw_pmkid_cand),
	},
};
static const int standard_event_num = (sizeof(standard_event) /
				       sizeof(struct iw_ioctl_description));

/* Size (in bytes) of the various private data types */
static const char priv_type_size[] = {
	0,				/* IW_PRIV_TYPE_NONE */
	1,				/* IW_PRIV_TYPE_BYTE */
	1,				/* IW_PRIV_TYPE_CHAR */
	0,				/* Not defined */
	sizeof(cyg_uint32),			/* IW_PRIV_TYPE_INT */
	sizeof(struct iw_freq),		/* IW_PRIV_TYPE_FLOAT */
	sizeof(struct sockaddr),	/* IW_PRIV_TYPE_ADDR */
	0,				/* Not defined */
};

/* Size (in bytes) of various events */
static const int event_type_size[] = {
	IW_EV_LCP_LEN,			/* IW_HEADER_TYPE_NULL */
	0,
	IW_EV_CHAR_LEN,			/* IW_HEADER_TYPE_CHAR */
	0,
	IW_EV_UINT_LEN,			/* IW_HEADER_TYPE_UINT */
	IW_EV_FREQ_LEN,			/* IW_HEADER_TYPE_FREQ */
	IW_EV_ADDR_LEN,			/* IW_HEADER_TYPE_ADDR */
	0,
	IW_EV_POINT_LEN + IW_EV_POINT_OFF,		/* Without variable payload */
	IW_EV_PARAM_LEN,		/* IW_HEADER_TYPE_PARAM */
	IW_EV_QUAL_LEN,			/* IW_HEADER_TYPE_QUAL */
};


static inline struct iw_statistics *get_wireless_stats(struct eth_drv_sc *sc);

/************************* EVENT PROCESSING *************************/
/*
 * Process events generated by the wireless layer or the driver.
 * Most often, the event will be propagated through rtnetlink
 */

#ifdef WE_EVENT_RTNETLINK
typedef struct {
	struct nlmsghdr hdr;
	struct ifinfomsg ifinfo;
	char opts[256];
} netlink_t;
/* ---------------------------------------------------------------- */
/*
 * Fill a rtnetlink message with our event data.
 * Note that we propage only the specified event and don't dump the
 * current wireless config. Dumping the wireless config is far too
 * expensive (for each parameter, the driver need to query the hardware).
 */
static inline int rtnetlink_fill_iwinfo(netlink_t *	skb,
					struct cyg_netdevtab_entry *tab,
					int			type,
					char *			event,
					int			event_len)
{
	struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
	struct ifnet *ifp = &sc->sc_arpcom.ac_if;
	wlan_private *wlan_priv = (wlan_private *)sc->driver_private;
	struct rtattr *rta;

	int size = NLMSG_LENGTH(sizeof(struct ifinfomsg));
	
	if (NLMSG_ALIGN(size) + RTA_LENGTH(event_len) >= sizeof(netlink_t) )
	{
		diag_printf("Too long size: %d\n", event_len);
		cyg_interrupt_disable();
		while(1);
		return -1;
	}
	
	diag_printf("Fill: %x, %d\n", skb, sizeof(netlink_t));

	skb->hdr.nlmsg_type = type;
	skb->hdr.nlmsg_len = size;
	skb->hdr.nlmsg_flags = 0;
	skb->hdr.nlmsg_pid = 0;
	skb->hdr.nlmsg_seq = 0;
		
	diag_printf("nlh=%x %x\n", &skb->hdr, skb->hdr.nlmsg_len);	
	
	
	skb->ifinfo.ifi_family = AF_UNSPEC;
	skb->ifinfo.__ifi_pad = 0;
	skb->ifinfo.ifi_type = ifp->if_type;
	skb->ifinfo.ifi_index = ifp->if_index;
	skb->ifinfo.ifi_flags = ifp->if_flags;
	skb->ifinfo.ifi_change = 0;	/* Wireless changes don't affect those flags */

	diag_printf("r = %x\n", &skb->ifinfo);
	
	diag_printf("mlmsg_len=%d, %d\n", skb->hdr.nlmsg_len, NLMSG_ALIGN(skb->hdr.nlmsg_len));
	
	/* Add the wireless events in the netlink packet */
	//RTA_PUT(skb, IFLA_WIRELESS, event_len, event);
	rta = (struct rtattr *)
		((char *) skb + NLMSG_ALIGN(skb->hdr.nlmsg_len));
	rta->rta_type = IFLA_WIRELESS;
	rta->rta_len = RTA_LENGTH(event_len);
	
	diag_printf("rta = %x, %x, end=%x\n", rta, RTA_DATA(rta), (char *)RTA_DATA(rta) + event_len);
	memcpy(RTA_DATA(rta), event, event_len);
	
	
	skb->hdr.nlmsg_len = NLMSG_ALIGN(size) +
		RTA_LENGTH(event_len);
	return skb->hdr.nlmsg_len;

/*
nlmsg_failure:
rtattr_failure:
	skb_trim(skb, b - skb->data);
	return -1;
*/
}

//#define NLMSG_GOODSIZE 4096//(PAGE_SIZE - ((sizeof(struct sk_buff)+0xF)&~0xF))
//#define NETLINK_CB(skb)		(*(struct netlink_skb_parms*)&((skb)->cb))
int wpa_kernel_driver_wext_send_link_event(const char *buffer, int len);


/* ---------------------------------------------------------------- */
/*
 * Create and broadcast and send it on the standard rtnetlink socket
 * This is a pure clone rtmsg_ifinfo() in net/core/rtnetlink.c
 * Andrzej Krzysztofowicz mandated that I used a IFLA_XXX field
 * within a RTM_NEWLINK event.
 */
static inline void rtmsg_iwinfo(struct cyg_netdevtab_entry *tab,
				char *			event,
				int			event_len)
{
	struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
	wlan_private *wlan_priv = (wlan_private *)sc->driver_private;
	netlink_t *skb;
	int size = sizeof(netlink_t);	//NLMSG_GOODSIZE;

	MALLOC(skb, netlink_t*, size, 0, M_NOWAIT);
	if (!skb)
		return;

#if 1
	if (rtnetlink_fill_iwinfo(skb, tab, RTM_NEWLINK,
				  event, event_len) < 0) {
		FREE(skb, 0);
		return;
	}
	
	
	wpa_kernel_driver_wext_send_link_event((const char *)skb, skb->hdr.nlmsg_len);
	//NETLINK_CB(skb).dst_group = RTNLGRP_LINK;
	//netlink_broadcast(rtnl, skb, 0, RTNLGRP_LINK, GFP_ATOMIC);
#endif

	FREE(skb, 0);
	return;	
}
#endif	/* WE_EVENT_RTNETLINK */

/* ---------------------------------------------------------------- */
/*
 * Main event dispatcher. Called from other parts and drivers.
 * Send the event on the appropriate channels.
 * May be called from interrupt context.
 */
void wireless_send_event(struct cyg_netdevtab_entry *tab,
			 unsigned int		cmd,
			 union iwreq_data *	wrqu,
			 char *			extra)
{
	struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
	wlan_private *wlan_priv = (wlan_private *)sc->driver_private;
	
	const struct iw_ioctl_description *	descr = NULL;
	int extra_len = 0;
	struct iw_event  *event;		/* Mallocated whole event */
	int event_len;				/* Its size */
	int hdr_len;				/* Size of the event header */
	int wrqu_off = 0;			/* Offset in wrqu */
	/* Don't "optimise" the following variable, it will crash */
	unsigned	cmd_index;		/* *MUST* be unsigned */

	/* Get the description of the Event */
	if((cmd & ~IOC_DIRMASK) <= (SIOCIWLAST & ~IOC_DIRMASK)) {
		cmd_index = (cmd - SIOCIWFIRST) & ~IOC_DIRMASK;
		if(cmd_index < standard_ioctl_num)
			descr = &(standard_ioctl[cmd_index]);
	} else {
		cmd_index = cmd - IWEVFIRST;
		if(cmd_index < standard_event_num)
			descr = &(standard_event[cmd_index]);
	}
	/* Don't accept unknown events */
	if(descr == NULL) {
		/* Note : we don't return an error to the driver, because
		 * the driver would not know what to do about it. It can't
		 * return an error to the user, because the event is not
		 * initiated by a user request.
		 * The best the driver could do is to log an error message.
		 * We will do it ourselves instead...
		 */
	  	diag_printf("%s (WE) : Invalid/Unknown Wireless Event (0x%04X)\n",
		       sc->dev_name, cmd);
		return;
	}
#ifdef WE_EVENT_DEBUG
	diag_printf("%s (WE) : Got event 0x%04X\n",
	       sc->dev_name, cmd);
	diag_printf("%s (WE) : Header type : %d, Token type : %d, size : %d, token : %d\n", sc->dev_name, descr->header_type, descr->token_type, descr->token_size, descr->max_tokens);
#endif	/* WE_EVENT_DEBUG */

	/* Check extra parameters and set extra_len */
	if(descr->header_type == IW_HEADER_TYPE_POINT) {
		/* Check if number of token fits within bounds */
		if(wrqu->data.length > descr->max_tokens) {
		  	diag_printf("%s (WE) : Wireless Event too big (%d)\n", sc->dev_name, wrqu->data.length);
			return;
		}
		if(wrqu->data.length < descr->min_tokens) {
		  	diag_printf("%s (WE) : Wireless Event too small (%d)\n", sc->dev_name, wrqu->data.length);
			return;
		}
		/* Calculate extra_len - extra is NULL for restricted events */
		if(extra != NULL)
			extra_len = wrqu->data.length * descr->token_size;
		/* Always at an offset in wrqu */
		wrqu_off = IW_EV_POINT_OFF;
#ifdef WE_EVENT_DEBUG
		diag_printf("%s (WE) : Event 0x%04X, tokens %d, extra_len %d\n", sc->dev_name, cmd, wrqu->data.length, extra_len);
#endif	/* WE_EVENT_DEBUG */
	}

	/* Total length of the event */
	hdr_len = event_type_size[descr->header_type];
	event_len = hdr_len + extra_len;

#ifdef WE_EVENT_DEBUG
	diag_printf("%s (WE) : Event 0x%04X, hdr_len %d, wrqu_off %d, event_len %d\n", sc->dev_name, cmd, hdr_len, wrqu_off, event_len);
#endif	/* WE_EVENT_DEBUG */

	/* Create temporary buffer to hold the event */
	MALLOC(event, struct iw_event *, event_len, 0, M_NOWAIT);
	if(event == NULL)
		return;

	/* Fill event */
	event->len = event_len;
	event->cmd = cmd;
	memcpy(&event->u, ((char *) wrqu) + wrqu_off, hdr_len - IW_EV_LCP_LEN);
	if(extra != NULL)
		memcpy(((char *) event) + hdr_len, extra, extra_len);

#ifdef WE_EVENT_RTNETLINK
	/* Send via the RtNetlink event channel */
	rtmsg_iwinfo(tab, (char *) event, event_len);
#endif	/* WE_EVENT_RTNETLINK */

	/* Cleanup */
	FREE(event, 0);

	return;		/* Always success, I guess ;-) */
}

/* ---------------------------------------------------------------- */
/*
 * Call the commit handler in the driver
 * (if exist and if conditions are right)
 *
 * Note : our current commit strategy is currently pretty dumb,
 * but we will be able to improve on that...
 * The goal is to try to agreagate as many changes as possible
 * before doing the commit. Drivers that will define a commit handler
 * are usually those that need a reset after changing parameters, so
 * we want to minimise the number of reset.
 * A cool idea is to use a timer : at each "set" command, we re-set the
 * timer, when the timer eventually fires, we call the driver.
 * Hopefully, more on that later.
 *
 * Also, I'm waiting to see how many people will complain about the
 * netif_running(dev) test. I'm open on that one...
 * Hopefully, the driver will remember to do a commit in "open()" ;-)
 */
static inline int call_commit_handler(struct eth_drv_sc *sc)
{
	if(/*(netif_running(sc)) &&*/
	   (sc->wireless_handlers->standard[0] != NULL)) {
		/* Call the commit handler on the driver */
		return sc->wireless_handlers->standard[0]((void*)sc, NULL,
							   NULL, NULL);
	} else
		return 0;		/* Command completed successfully */
}

/* ---------------------------------------------------------------- */
/*
 * Number of private arguments
 */
static inline int get_priv_size(cyg_uint16	args)
{
	int	num = args & IW_PRIV_SIZE_MASK;
	int	type = (args & IW_PRIV_TYPE_MASK) >> 12;

	return num * priv_type_size[type];
}


/* ---------------------------------------------------------------- */
/*
 *	Allow programatic access to /proc/net/wireless even if /proc
 *	doesn't exist... Also more efficient...
 */
static inline int dev_iwstats(struct eth_drv_sc *sc, caddr_t data)
{
	/* Get stats from the driver */
	struct iw_statistics *stats;

	stats = get_wireless_stats(sc);
	if (stats != (struct iw_statistics *) NULL) {
		struct iwreq *	wrq = (struct iwreq *)data;

		/* Copy statistics to the user buffer */
		if(copy_to_user(wrq->u.data.pointer, stats,
				sizeof(struct iw_statistics)))
			return -EFAULT;

		/* Check if we need to clear the update flag */
		if(wrq->u.data.flags != 0)
			stats->qual.updated = 0;
		return 0;
	} else
		return -EOPNOTSUPP;
}

/* ---------------------------------------------------------------- */
/*
 * Return the driver handler associated with a specific Wireless Extension.
 * Called from various place, so make sure it remains efficient.
 */
static inline iw_handler get_handler(struct eth_drv_sc *sc, 
				     unsigned int cmd)
{
	/* Don't "optimise" the following variable, it will crash */
	unsigned int	index;		/* *MUST* be unsigned */

	/* Check if we have some wireless handlers defined */
	if(sc->wireless_handlers == NULL)
		return NULL;
/*
	diag_printf("cmd=%x,SIOCIWFIRST=%x,SIOCIWFIRSTPRIV=%x\n", cmd, SIOCIWFIRST, SIOCIWFIRSTPRIV);
	diag_printf("index1=%x,%x\n",((cmd - SIOCIWFIRST) & ~IOC_DIRMASK), sc->wireless_handlers->num_standard);
	diag_printf("index2=%x,%x\n",((cmd - SIOCIWFIRSTPRIV) & ~IOC_DIRMASK), sc->wireless_handlers->num_private);
*/
	/* Try as a standard command */
	index = (cmd - SIOCIWFIRST) & ~IOC_DIRMASK;
	if(index < sc->wireless_handlers->num_standard)
		return sc->wireless_handlers->standard[index];

	/* Try as a private command */
	index = (cmd - SIOCIWFIRSTPRIV) & ~IOC_DIRMASK;
	if(index < sc->wireless_handlers->num_private)
		return sc->wireless_handlers->private[index];

	/* Not found */
	return NULL;
}


static inline struct iw_statistics *get_wireless_stats(struct eth_drv_sc *sc)
{
	return (sc->get_wireless_stats ?
		sc->get_wireless_stats(sc) :
		(struct iw_statistics *) NULL);
	/* In the future, get_wireless_stats may move from 'struct net_device'
	 * to 'struct iw_handler_def', to de-bloat struct net_device.
	 * Definitely worse a thought... */
}


/* ---------------------------------------------------------------- */
/*
 * Export the driver private handler definition
 * They will be picked up by tools like iwpriv...
 */
static inline int ioctl_export_private(struct eth_drv_sc *sc,
				       struct ifreq *		ifr)
{
	struct iwreq *				iwr = (struct iwreq *) ifr;

	/* Check if the driver has something to export */
	if((sc->wireless_handlers->num_private_args == 0) ||
	   (sc->wireless_handlers->private_args == NULL))
		return -EOPNOTSUPP;

	/* Check NULL pointer */
	if(iwr->u.data.pointer == NULL)
		return -EFAULT;
#ifdef WE_STRICT_WRITE
	/* Check if there is enough buffer up there */
	if(iwr->u.data.length < (SIOCIWLASTPRIV - SIOCIWFIRSTPRIV + 1))
		return -E2BIG;
#endif	/* WE_STRICT_WRITE */

	/* Set the number of available ioctls. */
	iwr->u.data.length = sc->wireless_handlers->num_private_args;

	/* Copy structure to the user buffer. */
	if (copy_to_user(iwr->u.data.pointer,
			 sc->wireless_handlers->private_args,
			 sizeof(struct iw_priv_args) * iwr->u.data.length))
		return -EFAULT;

	return 0;
}

/* ---------------------------------------------------------------- */
/*
 * Wrapper to call a standard Wireless Extension handler.
 * We do various checks and also take care of moving data between
 * user space and kernel space.
 */
static inline int ioctl_standard_call(struct eth_drv_sc *sc,
				      struct ifreq *		ifr,
				      unsigned int		cmd,
				      iw_handler		handler)
{
	struct iwreq *				iwr = (struct iwreq *) ifr;
	const struct iw_ioctl_description *	descr;
	struct iw_request_info			info;
	int					ret = -EINVAL;

	/* Get the description of the IOCTL */
	if(((cmd - SIOCIWFIRST) & ~IOC_DIRMASK) >= standard_ioctl_num)
		return -EOPNOTSUPP;
	descr = &(standard_ioctl[(cmd - SIOCIWFIRST) & ~IOC_DIRMASK]);

#ifdef WE_IOCTL_DEBUG
	printk(KERN_DEBUG "%s (WE) : Found standard handler for 0x%04X\n",
	       ifr->ifr_name, cmd);
	printk(KERN_DEBUG "%s (WE) : Header type : %d, Token type : %d, size : %d, token : %d\n", dev->name, descr->header_type, descr->token_type, descr->token_size, descr->max_tokens);
#endif	/* WE_IOCTL_DEBUG */

	/* Prepare the call */
	info.cmd = cmd;
	info.flags = 0;

	/* Check if we have a pointer to user space data or not */
	if(descr->header_type != IW_HEADER_TYPE_POINT) {

		/* No extra arguments. Trivial to handle */
		ret = handler((void*)sc, &info, &(iwr->u), NULL);

#ifdef WE_SET_EVENT
		/* Generate an event to notify listeners of the change */
		if((descr->flags & IW_DESCR_FLAG_EVENT) &&
		   ((ret == 0) || (ret == -EIWCOMMIT)))
			wireless_send_event(sc, cmd, &(iwr->u), NULL);
#endif	/* WE_SET_EVENT */
	} else {
		char *	extra;
		int	err;

		/* Check what user space is giving us */
		if(IW_IS_SET(cmd)) {
			/* Check NULL pointer */
			if((iwr->u.data.pointer == NULL) &&
			   (iwr->u.data.length != 0))
				return -EFAULT;
			/* Check if number of token fits within bounds */
			if(iwr->u.data.length > descr->max_tokens)
				return -E2BIG;
			if(iwr->u.data.length < descr->min_tokens)
				return -EINVAL;
		} else {
			/* Check NULL pointer */
			if(iwr->u.data.pointer == NULL)
				return -EFAULT;
#ifdef WE_STRICT_WRITE
			/* Check if there is enough buffer up there */
			if(iwr->u.data.length < descr->max_tokens)
				return -E2BIG;
#endif	/* WE_STRICT_WRITE */
		}

#ifdef WE_IOCTL_DEBUG
		diag_printf(KERN_DEBUG "%s (WE) : Malloc %d bytes\n",
		       sc->name, descr->max_tokens * descr->token_size);
#endif	/* WE_IOCTL_DEBUG */

		/* Always allocate for max space. Easier, and won't last
		 * long... */
		MALLOC(extra, char*, descr->max_tokens * descr->token_size, 0, M_NOWAIT);
		if (extra == NULL) {
			return -ENOMEM;
		}

		/* If it is a SET, get all the extra data in here */
		if(IW_IS_SET(cmd) && (iwr->u.data.length != 0)) {
			err = copy_from_user(extra, iwr->u.data.pointer,
					     iwr->u.data.length *
					     descr->token_size);
			if (err) {
				FREE(extra, 0);
				return -EFAULT;
			}
#ifdef WE_IOCTL_DEBUG
			diag_printf(KERN_DEBUG "%s (WE) : Got %d bytes\n",
			       sc->name,
			       iwr->u.data.length * descr->token_size);
#endif	/* WE_IOCTL_DEBUG */
		}

		/* Call the handler */
		ret = handler((void*)sc, &info, &(iwr->u), extra);

		/* If we have something to return to the user */
		if (!ret && IW_IS_GET(cmd)) {
			err = copy_to_user(iwr->u.data.pointer, extra,
					   iwr->u.data.length *
					   descr->token_size);
			if (err)
				ret =  -EFAULT;				   
#ifdef WE_IOCTL_DEBUG
			diag_printf(KERN_DEBUG "%s (WE) : Wrote %d bytes\n",
			       sc->name,
			       iwr->u.data.length * descr->token_size);
#endif	/* WE_IOCTL_DEBUG */
		}

#ifdef WE_SET_EVENT
		/* Generate an event to notify listeners of the change */
		if((descr->flags & IW_DESCR_FLAG_EVENT) &&
		   ((ret == 0) || (ret == -EIWCOMMIT))) {
			if(descr->flags & IW_DESCR_FLAG_RESTRICT)
				/* If the event is restricted, don't
				 * export the payload */
				wireless_send_event(sc, cmd, &(iwr->u), NULL);
			else
				wireless_send_event(sc, cmd, &(iwr->u),
						    extra);
		}
#endif	/* WE_SET_EVENT */

		/* Cleanup - I told you it wasn't that long ;-) */
		FREE(extra, 0);
	}

	/* Call commit handler if needed and defined */
	if(ret == -EIWCOMMIT)
		ret = call_commit_handler(sc);

	/* Here, we will generate the appropriate event if needed */

	return ret;
}

/* ---------------------------------------------------------------- */
/*
 * Wrapper to call a private Wireless Extension handler.
 * We do various checks and also take care of moving data between
 * user space and kernel space.
 * It's not as nice and slimline as the standard wrapper. The cause
 * is struct iw_priv_args, which was not really designed for the
 * job we are going here.
 *
 * IMPORTANT : This function prevent to set and get data on the same
 * IOCTL and enforce the SET/GET convention. Not doing it would be
 * far too hairy...
 * If you need to set and get data at the same time, please don't use
 * a iw_handler but process it in your ioctl handler (i.e. use the
 * old driver API).
 */
static inline int ioctl_private_call(struct eth_drv_sc *sc,
				     struct ifreq *		ifr,
				     unsigned int		cmd,
				     iw_handler		handler)
{
	struct iwreq *			iwr = (struct iwreq *) ifr;
	struct iw_priv_args *		descr = NULL;
	struct iw_request_info		info;
	int				extra_size = 0;
	int				i;
	int				ret = -EINVAL;

	/* Get the description of the IOCTL */
	for(i = 0; i < sc->wireless_handlers->num_private_args; i++)
		if(cmd == sc->wireless_handlers->private_args[i].cmd) {
			descr = &(sc->wireless_handlers->private_args[i]);
			break;
		}

#ifdef WE_IOCTL_DEBUG
	diag_printf(KERN_DEBUG "%s (WE) : Found private handler for 0x%04X\n",
	       ifr->ifr_name, cmd);
	if(descr) {
		diag_printf(KERN_DEBUG "%s (WE) : Name %s, set %X, get %X\n",
		       sc->name, descr->name,
		       descr->set_args, descr->get_args);
	}
#endif	/* WE_IOCTL_DEBUG */

	/* Compute the size of the set/get arguments */
	if(descr != NULL) {
		if(IW_IS_SET(cmd)) {
			/* Size of set arguments */
			extra_size = get_priv_size(descr->set_args);

			/* Does it fits in iwr ? */
			if((descr->set_args & IW_PRIV_SIZE_FIXED) &&
			   (extra_size < IFNAMSIZ))
				extra_size = 0;
		} else {
			/* Size of set arguments */
			extra_size = get_priv_size(descr->get_args);

			/* Does it fits in iwr ? */
			if((descr->get_args & IW_PRIV_SIZE_FIXED) &&
			   (extra_size < IFNAMSIZ))
				extra_size = 0;
		}
	}

	/* Prepare the call */
	info.cmd = cmd;
	info.flags = 0;

	/* Check if we have a pointer to user space data or not. */
	if(extra_size == 0) {
		/* No extra arguments. Trivial to handle */
		ret = handler((void*)sc, &info, &(iwr->u), (char *) &(iwr->u));
	} else {
		char *	extra;
		int	err;

		/* Check what user space is giving us */
		if(IW_IS_SET(cmd)) {
			/* Check NULL pointer */
			if((iwr->u.data.pointer == NULL) &&
			   (iwr->u.data.length != 0))
				return -EFAULT;

			/* Does it fits within bounds ? */
			if(iwr->u.data.length > (descr->set_args &
						 IW_PRIV_SIZE_MASK))
				return -E2BIG;
		} else {
			/* Check NULL pointer */
			if(iwr->u.data.pointer == NULL)
				return -EFAULT;
		}

#ifdef WE_IOCTL_DEBUG
		diag_printf(KERN_DEBUG "%s (WE) : Malloc %d bytes\n",
		       sc->name, extra_size);
#endif	/* WE_IOCTL_DEBUG */

		/* Always allocate for max space. Easier, and won't last
		 * long... */
		MALLOC(extra, char *, extra_size, 0, M_NOWAIT);
		if (extra == NULL) {
			return -ENOMEM;
		}

		/* If it is a SET, get all the extra data in here */
		if(IW_IS_SET(cmd) && (iwr->u.data.length != 0)) {
			err = copy_from_user(extra, iwr->u.data.pointer,
					     extra_size);
			if (err) {
				FREE(extra, 0);
				return -EFAULT;
			}
#ifdef WE_IOCTL_DEBUG
			diag_printf(KERN_DEBUG "%s (WE) : Got %d elem\n",
			       dev->name, iwr->u.data.length);
#endif	/* WE_IOCTL_DEBUG */
		}

		/* Call the handler */
		ret = handler((void*)sc, &info, &(iwr->u), extra);

		/* If we have something to return to the user */
		if (!ret && IW_IS_GET(cmd)) {
			err = copy_to_user(iwr->u.data.pointer, extra,
					   extra_size);
			if (err)
				ret =  -EFAULT;				   
#ifdef WE_IOCTL_DEBUG
			diag_printf(KERN_DEBUG "%s (WE) : Wrote %d elem\n",
			       dev->name, iwr->u.data.length);
#endif	/* WE_IOCTL_DEBUG */
		}

		/* Cleanup - I told you it wasn't that long ;-) */
		FREE(extra, 0);
	}


	/* Call commit handler if needed and defined */
	if(ret == -EIWCOMMIT)
		ret = call_commit_handler(sc);

	return ret;
}


/* ---------------------------------------------------------------- */
/*
 * Main IOCTl dispatcher. Called from the main networking code
 * (dev_ioctl() in net/core/dev.c).
 * Check the type of IOCTL and call the appropriate wrapper...
 */
//int wireless_process_ioctl(struct ifreq *ifr, unsigned int cmd)
int wireless_process_ioctl(struct ifnet *ifp, u_long cmd, caddr_t data)
{
//	struct net_device *dev;
	struct eth_drv_sc *sc = ifp->if_softc;
	iw_handler	handler;

	/* Permissions are already checked in dev_ioctl() before calling us.
	 * The copy_to/from_user() of ifr is also dealt with in there */

	/* Make sure the device exist */
	//if ((dev = __dev_get_by_name(ifr->ifr_name)) == NULL)
	//	return -ENODEV;

	/* A bunch of special cases, then the generic case...
	 * Note that 'cmd' is already filtered in dev_ioctl() with
	 * (cmd >= SIOCIWFIRST && cmd <= SIOCIWLAST) */
	switch(cmd) 
	{
		case SIOCGIWSTATS:
			/* Get Wireless Stats */
			return dev_iwstats(sc, data);

		case SIOCGIWPRIV:
			/* Check if we have some wireless handlers defined */
			if(sc->wireless_handlers != NULL) {
				/* We export to user space the definition of
				 * the private handler ourselves */
				return ioctl_export_private(sc, (struct ifreq *	)data);
			}
			// ## Fall-through for old API ##
		default:
			/* Generic IOCTL */
			/* Basic check */
			//if (!netif_device_present(dev))
			//	return -ENODEV;
			/* New driver API : try to find the handler */
			handler = get_handler(sc, cmd);
			if(handler != NULL) {
				/* Standard and private are not the same */
				if((cmd & ~IOC_DIRMASK) < (SIOCIWFIRSTPRIV & ~IOC_DIRMASK))
					return ioctl_standard_call(sc,
								   (struct ifreq *	)data,
								   cmd,
								   handler);
				else
					return ioctl_private_call(sc,
								  (struct ifreq *	)data,
								  cmd,
								  handler);
			}
			/* Old driver API : call driver ioctl handler */
			if(sc->funs->control)
				return sc->funs->control(sc, cmd, data, 0);
			
			
			return -EOPNOTSUPP;
	}
	/* Not reached */
	return -EINVAL;
}

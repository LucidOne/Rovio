/*
 * wpa_supplicant/hostapd - Default include files
 * Copyright (c) 2005-2006, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 *
 * This header file is included into all C files so that commonly used header
 * files can be selected with OS specific #ifdefs in one place instead of
 * having to have OS/C library specific selection in many files.
 */

#ifndef INCLUDES_H
#define INCLUDES_H

/* Include possible build time configuration before including anything else */
#include "custom_config.h"
#include "build_config.h"

#include "stdlib.h"
#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#ifndef _WIN32_WCE
#ifndef CONFIG_TI_COMPILER
#include "signal.h"
#include "sys/types.h"
#endif /* CONFIG_TI_COMPILER */
#include "errno.h"
#endif /* _WIN32_WCE */
#include "ctype.h"
#include "time.h"


#include "network.h"
#include "cyg/io/eth/netdev.h"
#include "cyg/io/pipelib.h"
#include "net/if.h"
#if 1	//xh, copied from linux/socket.h
/* Supported address families. */
#define AF_NETLINK	16

/* Protocol families, same as address families. */
#define PF_NETLINK	AF_NETLINK

#endif




#ifndef CONFIG_TI_COMPILER
#ifndef _MSC_VER
#include "unistd.h"
#endif /* _MSC_VER */
#endif /* CONFIG_TI_COMPILER */

#ifndef CONFIG_NATIVE_WINDOWS
#ifndef CONFIG_TI_COMPILER
//#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#ifndef __vxworks
//#include "sys/uio.h"
//#include "sys/time.h"
#endif /* __vxworks */
#endif /* CONFIG_TI_COMPILER */
#endif /* CONFIG_NATIVE_WINDOWS */


int fix_size_read(int fd, void *ptr, int size);
int fix_size_write(int fd, const void *ptr, int size);

#define RECONF_EVENT_RELOAD	0
#define RECONF_EVENT_SETIP	1

enum SET_WIFI_STATE_E
{
	SET_WIFI__NO_INIT,
	SET_WIFI__TRYING,
	SET_WIFI__OK
};

enum SET_IP_STATE_E
{
	SET_IP__NO_INIT,
	SET_IP__DHCP_TRYING,
	SET_IP__DHCP_FAILED,
	SET_IP__DHCP_OK,
	SET_IP__STATIC_IP_TRYING,
	SET_IP__STATIC_IP_FAILED,
	SET_IP__STATIC_IP_OK
};


void wsp_reconfig(char reconf_msg);


void wsp_set_network_managed(const char *ssid, const char *key);
void wsp_set_network_adhoc(const char *ssid, const char *key, int channel);
void wsp_set_network_ip(void);
void wsp_wait_network_ip(int timeout_ticks);

void wsp_init_interface(const char *interface);	/* Todo by user. */
void wsp_set_wireless_config(void);
void wsp_set_ip_addr(const char *interface, enum SET_IP_STATE_E *ip_state);	/* Todo by user. */
void wsp_connecting_callback(void);	/* Todo by user. */
void wsp_connected_callback(void);	/* Todo by user. */
void wsp_get_config_state(enum SET_WIFI_STATE_E *wifi_state, enum SET_IP_STATE_E *ip_state);


#endif /* INCLUDES_H */

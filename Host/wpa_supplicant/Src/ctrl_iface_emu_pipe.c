/*
 * WPA Supplicant / UDP socket -based control interface
 * Copyright (c) 2004-2005, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"

#include "common.h"
#include "eloop.h"
#include "config.h"
#include "eapol_sm.h"
#include "wpa_supplicant_i.h"
#include "ctrl_iface.h"
#include "wpa_ctrl.h"


#define COOKIE_LEN 8

/* Per-interface ctrl_iface */

/**
 * struct wpa_ctrl_dst - Internal data structure of control interface monitors
 *
 * This structure is used to store information about registered control
 * interface monitors into struct wpa_supplicant. This data is private to
 * ctrl_iface_udp.c and should not be touched directly from other files.
 */
struct wpa_ctrl_dst {
	struct wpa_ctrl_dst *next;
	int pipe_index;
	int debug_level;
	int errors;
};


struct ctrl_iface_priv {
	struct wpa_supplicant *wpa_s;
	int fd;
	struct wpa_ctrl_dst *ctrl_dst;
	u8 cookie[COOKIE_LEN];
};


static void wpa_supplicant_ctrl_iface_send(struct ctrl_iface_priv *priv,
					   int level, const char *buf,
					   size_t len);


static int wpa_supplicant_ctrl_iface_attach(struct ctrl_iface_priv *priv, int pipe_index)
{
	struct wpa_ctrl_dst *dst;

	dst = os_zalloc(sizeof(*dst));
	if (dst == NULL)
		return -1;
		
	dst->pipe_index = pipe_index;
	dst->debug_level = MSG_INFO;
	dst->next = priv->ctrl_dst;
	priv->ctrl_dst = dst;
	wpa_printf(MSG_DEBUG, "CTRL_IFACE monitor attached: /dev/pipe%d", pipe_index);
	return 0;
}


static int wpa_supplicant_ctrl_iface_detach(struct ctrl_iface_priv *priv, int pipe_index)
{
	struct wpa_ctrl_dst *dst, *prev = NULL;

	dst = priv->ctrl_dst;
	while (dst) {
		if (dst->pipe_index == pipe_index) {
			if (prev == NULL)
				priv->ctrl_dst = dst->next;
			else
				prev->next = dst->next;
			os_free(dst);
			wpa_printf(MSG_DEBUG, "CTRL_IFACE monitor detached: /dev/pipe%d ", pipe_index);
			return 0;
		}
		prev = dst;
		dst = dst->next;
	}
	return -1;
}


static int wpa_supplicant_ctrl_iface_level(struct ctrl_iface_priv *priv,
					   int pipe_index,
					   char *level)
{
	struct wpa_ctrl_dst *dst;

	wpa_printf(MSG_DEBUG, "CTRL_IFACE LEVEL %s", level);

	dst = priv->ctrl_dst;
	while (dst) {
		if (dst->pipe_index == pipe_index) {
			wpa_printf(MSG_DEBUG, "CTRL_IFACE changed monitor "
				   "level /dev/pipe%d", pipe_index);
			dst->debug_level = atoi(level);
			return 0;
		}
		dst = dst->next;
	}

	return -1;
}


static char *
wpa_supplicant_ctrl_iface_get_cookie(struct ctrl_iface_priv *priv,
				     size_t *reply_len)
{
	char *reply;
	reply = os_malloc(7 + 2 * COOKIE_LEN + 1);
	if (reply == NULL) {
		*reply_len = 1;
		return NULL;
	}

	os_memcpy(reply, "COOKIE=", 7);
	wpa_snprintf_hex(reply + 7, 2 * COOKIE_LEN + 1,
			 priv->cookie, COOKIE_LEN);

	*reply_len = 7 + 2 * COOKIE_LEN;
	return reply;
}


static void wpa_supplicant_ctrl_iface_receive(int fd, void *eloop_ctx,
					      void *fd_ctx)
{
	struct wpa_supplicant *wpa_s = eloop_ctx;
	struct ctrl_iface_priv *priv = fd_ctx;
	char buf[256], *pos;
	int len;
	char *reply = NULL;
	size_t reply_len = 0;
	int new_attached = 0;
	u8 cookie[COOKIE_LEN];

	fix_size_read(fd, &len, sizeof(len));
	if (len >= sizeof(buf))
		len = sizeof(buf) - 1;
	fix_size_read(fd, buf, len);

	buf[len] = '\0';

	if (os_strcmp(buf, "GET_COOKIE") == 0) {
		reply = wpa_supplicant_ctrl_iface_get_cookie(priv, &reply_len);
		goto done;
	}

	/*
	 * Require that the client includes a prefix with the 'cookie' value
	 * fetched with GET_COOKIE command. This is used to verify that the
	 * client has access to a bidirectional link over UDP in order to
	 * avoid attacks using forged localhost IP address even if the OS does
	 * not block such frames from remote destinations.
	 */
	if (os_strncmp(buf, "COOKIE=", 7) != 0) {
		wpa_printf(MSG_DEBUG, "CTLR: No cookie in the request - "
			   "drop request");
		return;
	}

	if (hexstr2bin(buf + 7, cookie, COOKIE_LEN) < 0) {
		wpa_printf(MSG_DEBUG, "CTLR: Invalid cookie format in the "
			   "request - drop request");
		return;
	}

	if (os_memcmp(cookie, priv->cookie, COOKIE_LEN) != 0) {
		wpa_printf(MSG_DEBUG, "CTLR: Invalid cookie in the request - "
			   "drop request");
		return;
	}

	pos = buf + 7 + 2 * COOKIE_LEN;
	while (*pos == ' ')
		pos++;

	if (os_strcmp(pos, "ATTACH") == 0) {
		if (wpa_supplicant_ctrl_iface_attach(priv, priv->ctrl_dst->pipe_index))
			reply_len = 1;
		else {
			new_attached = 1;
			reply_len = 2;
		}
	} else if (os_strcmp(pos, "DETACH") == 0) {
		if (wpa_supplicant_ctrl_iface_detach(priv, priv->ctrl_dst->pipe_index))
			reply_len = 1;
		else
			reply_len = 2;
	} else if (os_strncmp(pos, "LEVEL ", 6) == 0) {
		if (wpa_supplicant_ctrl_iface_level(priv, priv->ctrl_dst->pipe_index,
						    pos + 6))
			reply_len = 1;
		else
			reply_len = 2;
	} else {
		reply = wpa_supplicant_ctrl_iface_process(wpa_s, pos,
							  &reply_len);
	}

 done:
	if (reply) {
		fix_size_write(fd, &reply_len, sizeof(reply_len));
		fix_size_write(fd, reply, reply_len);
		os_free(reply);
	} else if (reply_len == 1) {
		reply_len = 5;
		fix_size_write(fd, &reply_len, sizeof(reply_len));
		fix_size_write(fd, "FAIL\n", reply_len);
	} else if (reply_len == 2) {
		reply_len = 3;
		fix_size_write(fd, &reply_len, sizeof(reply_len));
		fix_size_write(fd, "OK\n", reply_len);
	}

	if (new_attached)
		eapol_sm_notify_ctrl_attached(wpa_s->eapol);
}


static void wpa_supplicant_ctrl_iface_msg_cb(void *ctx, int level,
					     const char *txt, size_t len)
{
	struct wpa_supplicant *wpa_s = ctx;
	if (wpa_s == NULL || wpa_s->ctrl_iface == NULL)
		return;
	wpa_supplicant_ctrl_iface_send(wpa_s->ctrl_iface, level, txt, len);
}


struct ctrl_iface_priv *
wpa_supplicant_ctrl_iface_init(struct wpa_supplicant *wpa_s)
{
	int pipe_index;
	struct ctrl_iface_priv *priv;

	priv = os_zalloc(sizeof(*priv));
	if (priv == NULL)
		return NULL;
	priv->wpa_s = wpa_s;
	priv->fd = -1;
	os_get_random(priv->cookie, COOKIE_LEN);

	if (wpa_s->conf->ctrl_interface == NULL)
		return priv;

	priv->fd = pipe_create(&pipe_index);
	if (priv->fd < 0) {
		perror("pipe for ctrl_iface");
		goto fail;
	}

	eloop_register_read_sock(priv->fd, wpa_supplicant_ctrl_iface_receive,
				 wpa_s, priv);
	wpa_msg_register_cb(wpa_supplicant_ctrl_iface_msg_cb);

	return priv;

fail:
	if (priv->fd >= 0)
		close(priv->fd);
	os_free(priv);
	return NULL;
}


void wpa_supplicant_ctrl_iface_deinit(struct ctrl_iface_priv *priv)
{
	struct wpa_ctrl_dst *dst, *prev;

	if (priv->fd > -1) {
		eloop_unregister_read_sock(priv->fd);
		if (priv->ctrl_dst) {
			/*
			 * Wait a second before closing the control socket if
			 * there are any attached monitors in order to allow
			 * them to receive any pending messages.
			 */
			wpa_printf(MSG_DEBUG, "CTRL_IFACE wait for attached "
				   "monitors to receive messages");
			os_sleep(1, 0);
		}
		close(priv->fd);
		priv->fd = -1;
	}

	dst = priv->ctrl_dst;
	while (dst) {
		prev = dst;
		dst = dst->next;
		os_free(prev);
	}
	os_free(priv);
}


static void wpa_supplicant_ctrl_iface_send(struct ctrl_iface_priv *priv,
					   int level, const char *buf,
					   size_t len)
{
	struct wpa_ctrl_dst *dst, *next;
	char levelstr[10];
	int idx;
	char *sbuf;
	int llen;

	dst = priv->ctrl_dst;
	if (priv->fd < 0 || dst == NULL)
		return;

	os_snprintf(levelstr, sizeof(levelstr), "<%d>", level);

	llen = os_strlen(levelstr);
	sbuf = os_malloc(llen + len);
	if (sbuf == NULL)
		return;

	os_memcpy(sbuf, levelstr, llen);
	os_memcpy(sbuf + llen, buf, len);

	idx = 0;
	while (dst) {
		next = dst->next;
		if (level >= dst->debug_level) {
			int send_len;
			wpa_printf(MSG_DEBUG, "CTRL_IFACE monitor send /dev/pipe%d",
				   dst->pipe_index);
			
			send_len = llen + len;
			fix_size_write(priv->fd, &send_len, sizeof(send_len));
			if (fix_size_write(priv->fd, sbuf, llen + len) < 0) {
				perror("sendto(CTRL_IFACE monitor)");
				dst->errors++;
				if (dst->errors > 10) {
					wpa_supplicant_ctrl_iface_detach(
						priv, dst->pipe_index);
				}
			} else
				dst->errors = 0;
		}
		idx++;
		dst = next;
	}
	os_free(sbuf);
}


void wpa_supplicant_ctrl_iface_wait(struct ctrl_iface_priv *priv)
{
	wpa_printf(MSG_DEBUG, "CTRL_IFACE - %s - wait for monitor",
		   priv->wpa_s->ifname);
	eloop_wait_for_read_sock(priv->fd);
}


/* Global ctrl_iface */

struct ctrl_iface_global_priv {
	int fd;
	u8 cookie[COOKIE_LEN];
};


static char *
wpa_supplicant_global_get_cookie(struct ctrl_iface_global_priv *priv,
				 size_t *reply_len)
{
	char *reply;
	reply = os_malloc(7 + 2 * COOKIE_LEN + 1);
	if (reply == NULL) {
		*reply_len = 1;
		return NULL;
	}

	os_memcpy(reply, "COOKIE=", 7);
	wpa_snprintf_hex(reply + 7, 2 * COOKIE_LEN + 1,
			 priv->cookie, COOKIE_LEN);

	*reply_len = 7 + 2 * COOKIE_LEN;
	return reply;
}


static void wpa_supplicant_global_ctrl_iface_receive(int fd, void *eloop_ctx,
						     void *fd_ctx)
{
	struct wpa_global *global = eloop_ctx;
	struct ctrl_iface_global_priv *priv = fd_ctx;
	char buf[256], *pos;
	int len;
	char *reply;
	size_t reply_len;
	u8 cookie[COOKIE_LEN];

	fix_size_read(fd, &len, sizeof(len));
	if (len >= sizeof(buf))
		len = sizeof(buf) - 1;
	fix_size_read(fd, buf, len);

	if (len < 0) {
		perror("recvfrom(ctrl_iface)");
		return;
	}
	buf[len] = '\0';

	if (os_strcmp(buf, "GET_COOKIE") == 0) {
		reply = wpa_supplicant_global_get_cookie(priv, &reply_len);
		goto done;
	}

	if (os_strncmp(buf, "COOKIE=", 7) != 0) {
		wpa_printf(MSG_DEBUG, "CTLR: No cookie in the request - "
			   "drop request");
		return;
	}

	if (hexstr2bin(buf + 7, cookie, COOKIE_LEN) < 0) {
		wpa_printf(MSG_DEBUG, "CTLR: Invalid cookie format in the "
			   "request - drop request");
		return;
	}

	if (os_memcmp(cookie, priv->cookie, COOKIE_LEN) != 0) {
		wpa_printf(MSG_DEBUG, "CTLR: Invalid cookie in the request - "
			   "drop request");
		return;
	}

	pos = buf + 7 + 2 * COOKIE_LEN;
	while (*pos == ' ')
		pos++;

	reply = wpa_supplicant_global_ctrl_iface_process(global, pos,
							 &reply_len);

 done:
	if (reply) {
		fix_size_write(fd, &reply_len, sizeof(reply_len));
		fix_size_write(fd, reply, reply_len);
		os_free(reply);
	} else if (reply_len) {
		reply_len = 5;
		fix_size_write(fd, &reply_len, sizeof(reply_len));
		fix_size_write(fd, "FAIL\n", reply_len);
	}
}


struct ctrl_iface_global_priv *
wpa_supplicant_global_ctrl_iface_init(struct wpa_global *global)
{
	int pipe_index;
	struct ctrl_iface_global_priv *priv;

	priv = os_zalloc(sizeof(*priv));
	if (priv == NULL)
		return NULL;
	priv->fd = -1;
	os_get_random(priv->cookie, COOKIE_LEN);

	if (global->params.ctrl_interface == NULL)
		return priv;

	wpa_printf(MSG_DEBUG, "Global control interface '%s'",
		   global->params.ctrl_interface);

	priv->fd = pipe_create(&pipe_index);
	if (priv->fd < 0) {
		perror("socket(PF_INET)");
		goto fail;
	}

	eloop_register_read_sock(priv->fd,
				 wpa_supplicant_global_ctrl_iface_receive,
				 global, priv);

	return priv;

fail:
	if (priv->fd >= 0)
		close(priv->fd);
	os_free(priv);
	return NULL;
}


void
wpa_supplicant_global_ctrl_iface_deinit(struct ctrl_iface_global_priv *priv)
{
	if (priv->fd >= 0) {
		eloop_unregister_read_sock(priv->fd);
		close(priv->fd);
	}
	os_free(priv);
}

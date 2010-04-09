/*
 * WPA Supplicant - driver interaction with Linux Marvell drivers
 * 
 *  Copyright © 2006, Marvell International Ltd.
 *  
 *  All Rights Reserved
 *  
 * 1. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 2. Neither the name of Marvell nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND  CONTRIBUTORS
 *  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR  
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "includes.h"
#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "sys/ioctl.h"
#include "errno.h"


//#include "net/wireless.h"
#include "net/if.h"
#include "common.h"
#include "wireless_copy.h"
#include "driver.h"
#include "driver_wext.h"
#include "eloop.h"
#include "driver_hostap.h"
#include "wpa_supplicant.h"
#include "defs.h"

#define EAP_ALG_NONE    0x01
#define EAP_ALG_LEAP    0x02
#define EAP_ALG_TLS   0x04
#define EAP_ALG_TTLS    0x08
#define EAP_ALG_MD5   0x10

#ifndef AUTH_ALG_NETWORK_EAP
#define AUTH_ALG_NETWORK_EAP  0x08
#endif

typedef unsigned char uchar;

typedef u8 WLAN_802_11_MAC_ADDRESS[ETH_ALEN];
typedef u8 WLAN_802_11_KEY_RSC[8];

struct wpa_driver_marvell_data {
	void  *wext; /* Private data for driver_wext */
	void  *ctx;
	char  ifname[IFNAMSIZ + 1];
	int sock;
	Boolean CounterMeasure;
	Boolean WpaEnabled;
};

struct _WLAN_802_11_KEY {
	ulong Length;   /* Length of this structure */
	ulong KeyIndex;
	ulong KeyLength;  /* Length of key in bytes */
	WLAN_802_11_MAC_ADDRESS BSSID;
	WLAN_802_11_KEY_RSC KeyRSC;
	//u8 KeyMaterial[0];
	u8 KeyMaterial[1];
} __attribute__ ((packed));

typedef struct _WLAN_802_11_KEY WLAN_802_11_KEY, *PWLAN_802_11_KEY;

struct _WLAN_802_11_SSID {
	ulong ssid_len;
	uchar ssid[32];
} __attribute__ ((packed));

typedef struct _WLAN_802_11_SSID WLAN_802_11_SSID, *PWLAN_802_11_SSID;

#define IW_MAX_PRIV_DEF   128
#define IW_PRIV_IOCTL_COUNT (SIOCIWLASTPRIV-SIOCIWFIRSTPRIV+1)

static int Priv_count;
static struct iw_priv_args Priv_args[IW_MAX_PRIV_DEF];

static int get_private_info(const char *ifname)
{
	struct iwreq    iwr;
	int     s, ret = 0;
	struct iw_priv_args *pPriv = Priv_args;

	s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket[PF_INET,SOCK_DGRAM]");
		return -1;
	}

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, ifname, IFNAMSIZ);
	iwr.u.data.pointer = (caddr_t) pPriv;
	iwr.u.data.length = IW_MAX_PRIV_DEF;
	iwr.u.data.flags = 0;

	if (ioctl(s, SIOCGIWPRIV, &iwr) < 0) {
		perror("ioctl[SIOCGIWPRIV]");
		ret = -1;
	} else {
	/* Return the number of private ioctls */
		ret = iwr.u.data.length;
	}

	close(s);

	return ret;
}

static char *priv_ioctl_names[] = {
	"extscan",
	"authalgs",
	"setwpaie",
	"deauth",
	"encryptionmode",
	"reasso-on",
	"reasso-off",
};

static int PRIV_IOCTL[sizeof(priv_ioctl_names)/sizeof(priv_ioctl_names[0])];
static int PRIV_SUBIOCTL[sizeof(priv_ioctl_names)/sizeof(priv_ioctl_names[0])];

#define IOCTL_WLANEXTSCAN		PRIV_IOCTL[0]
#define SUBIOCTL_WLANEXTSCAN		PRIV_SUBIOCTL[0]
#define IOCTL_SET_AP_AUTH_ALGS		PRIV_IOCTL[1]
#define SUBIOCTL_SET_AP_AUTH_ALGS	PRIV_SUBIOCTL[1]
#define IOCTL_SET_WPA_IE		PRIV_IOCTL[2]
#define SUBIOCTL_SET_WPA_IE		PRIV_SUBIOCTL[2]
#define IOCTL_DEAUTH			PRIV_IOCTL[3]
#define SUBIOCTL_DEAUTH			PRIV_SUBIOCTL[3]
#define IOCTL_ENCRYPTION_MODE		PRIV_IOCTL[4]
#define SUBIOCTL_ENCRYPTION_MODE	PRIV_SUBIOCTL[4]
#define IOCTL_REASSO_ON			PRIV_IOCTL[5]
#define SUBIOCTL_REASSO_ON		PRIV_SUBIOCTL[5]
#define IOCTL_REASSO_OFF		PRIV_IOCTL[6]
#define SUBIOCTL_REASSO_OFF		PRIV_SUBIOCTL[6]

#define SIOCDEVPRIVATE      0x89F0  /* to 89FF */

int marvell_get_subioctl_no(int i, int *sub_cmd)
{
	int j;

	if (Priv_args[i].cmd >= SIOCDEVPRIVATE) {
		*sub_cmd = 0;
		return Priv_args[i].cmd;
	}
  	j = -1;

	/* Find the matching *real* ioctl */
	while ((++j < Priv_count) && ((Priv_args[j].name[0] != '\0') ||
		(Priv_args[j].set_args != Priv_args[i].set_args) || 
		(Priv_args[j].get_args != Priv_args[i].get_args)));

  	/* If not found... */
  	if (j == Priv_count) {
		wpa_printf(MSG_DEBUG,
			"%s: Invalid private ioctl definition for: 0x%x\n",
			__func__, Priv_args[i].cmd);
		return -1;
	}

	/* Save sub-ioctl number */
	*sub_cmd = Priv_args[i].cmd;
	/* Return the real IOCTL number */
	return Priv_args[j].cmd;
}

static int marvell_get_ioctl_no(const char *ifname, const char *priv_cmd,
        int *sub_cmd)
{
	int i;

	/* Are there any private ioctls? */
	if (Priv_count <= 0) {
	/* Could skip this message ? */
	wpa_printf(MSG_DEBUG, "%-8.8s  no private ioctls.\n", "eth1");
	} else {
#ifdef MARVELL_DEBUG
		wpa_printf(MSG_DEBUG, "%-8.8s  Available private ioctl :\n", "eth1");
#endif  /* MARVELL_DEBUG */
		for (i = 0; i < Priv_count; i++) {
			if (Priv_args[i].name[0] && !strcmp(Priv_args[i].name, priv_cmd)) {
				return marvell_get_subioctl_no(i, sub_cmd);
			}
		}
	}
	return -1;
}

void marvell_init_ioctl_numbers(const char *ifname)
{
	int i;

	/* Read the private ioctls */
	Priv_count = get_private_info(ifname);

	for (i = 0; i < sizeof(priv_ioctl_names) / sizeof(priv_ioctl_names[0]); i++) {
		PRIV_IOCTL[i] = marvell_get_ioctl_no(ifname, 
		priv_ioctl_names[i], &PRIV_SUBIOCTL[i]);
		wpa_printf(MSG_DEBUG, "IOCTL %s: 0x%x, 0x%x\n",
			priv_ioctl_names[i], PRIV_IOCTL[i], 
			PRIV_SUBIOCTL[i]);
	}
}

static int marvell_ioctl(const char *dev, int cmdno, caddr_t param,
             int len, int show_err)
{
	int   s;
	struct iwreq  iwr;

	s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		return -1;
	}

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, dev, IFNAMSIZ);
	iwr.u.data.pointer = param;
	iwr.u.data.length = len;

	if (show_err == IW_ENCODE_DISABLED)
		iwr.u.data.flags = IW_ENCODE_DISABLED;

	wpa_printf(MSG_DEBUG, "IOCTL: CMD = 0x%X\n", cmdno);

	if (ioctl(s, cmdno, &iwr) < 0) {
		int ret;

		close(s);
		ret = errno;

		if (show_err)
			perror("marvell ioctl");
		return ret;
	}

	close(s);
	return 0;
}

static int wpa_driver_marvell_set_wpa_ie(const char *ifname, 
           const char *wpa_ie, size_t wpa_ie_len)
{
	int ret;

	if (IOCTL_SET_WPA_IE != -1) {
		ret = marvell_ioctl(ifname, IOCTL_SET_WPA_IE,
			(caddr_t) wpa_ie, wpa_ie_len, 1);
		if (ret < 0) {
			wpa_printf(MSG_ERROR, "%s: SET_WPA_IE", __func__);
			return ret;
		}
	} else
	ret = 0;

	return ret;
}

/* marvell2param - function to call get/set ioctl's that use sub-commands */
static int marvell2param(const char *ifname, int cmdno, int param, int value)
{
	struct iwreq  iwr;
	int   s, ret = 0;

	if (cmdno == -1) {
		wpa_printf(MSG_ERROR, "%s: invalid ioctl number 0x%04x",
			__func__, cmdno);
		return 0;
	}
	s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket[PF_INET,SOCK_DGRAM]");
		return -1;
	}
	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, ifname, IFNAMSIZ);
	iwr.u.data.pointer = (caddr_t) & value;
	iwr.u.data.length = 1;
	iwr.u.data.flags = param;

	if (ioctl(s, cmdno, &iwr) < 0) {
		perror("marvell2param ioctl");
		ret = -1;
	}
	close(s);
	return ret;
}

int wpa_driver_marvell_set_wpa(void *priv, int enabled)
{
	int ret = 0;
	struct wpa_driver_marvell_data *drv = priv;

	wpa_printf(MSG_DEBUG, "%s: enabled=%d", __func__, enabled);

	/* Disable WPA IE so next associate will fail */
	if (!enabled && wpa_driver_marvell_set_wpa_ie(drv->ifname, NULL, 0) < 0)
		ret = -1;

	/* Enable Roaming? */
	/* Right now Marvell driver has no support for this */

	/* Privacy Invoked? */

	if (!ret)
		drv->WpaEnabled = enabled ? TRUE : FALSE;

	return ret;
}

static int wpa_driver_marvell_set_key(void *priv, wpa_alg alg,
           const u8 *addr, int key_idx,
           int set_tx, const u8 * seq, size_t seq_len,
           const u8 * key, size_t key_len)
{
	WLAN_802_11_KEY *param;
	u8    *buf;
	size_t    blen;
	int   ret = 0, flag = 0;
	char    *alg_name;
	struct wpa_driver_marvell_data *drv = priv;

	/* Marvell driver does not support setting NULL keys. */

	if (alg == WPA_ALG_NONE) {
		if (!key_len)
			marvell_ioctl(drv->ifname, SIOCSIWENCODE, (caddr_t)key,
				0, IW_ENCODE_DISABLED);
			return 0;
	}

	switch (alg) {
		case WPA_ALG_NONE:
			alg_name = "none";
			break;
		case WPA_ALG_WEP:
			alg_name = "WEP";
			flag = 1;
			break;
		case WPA_ALG_TKIP:
			alg_name = "TKIP";
			break;
		case WPA_ALG_CCMP:
			alg_name = "CCMP";
			break;
		default:
			return -1;
	}

	wpa_printf(MSG_DEBUG, "%s: alg=%s key_idx=%d set_tx=%d seq_len=%d "
		"key_len=%d", __func__, alg_name, key_idx, set_tx,
		seq_len, key_len);

	if (seq_len > 8)
		return -2;

	blen = sizeof(*param) + key_len;
	buf = os_malloc(blen);

	if (buf == NULL)
		return -1;
	memset(buf, 0, blen);

	param = (WLAN_802_11_KEY *) buf;

	/* TODO: In theory, STA in client mode can use five keys; four default
	* keys for receiving (with keyidx 0..3) and one individual key for
	* both transmitting and receiving (keyidx 0) _unicast_ packets. Now,
	* keyidx 0 is reserved for this unicast use and default keys can only
	* use keyidx 1..3 (i.e., default key with keyidx 0 is not supported).
	* This should be fine for more or less all cases, but for completeness
	* sake, the driver could be enhanced to support the missing key. */

	/* Param structure filled here */
	memset((u8 *) param->BSSID, 0xff, ETH_ALEN);

	if (addr) {
		if (!key_idx)
			key_idx = 0x40000000;
		param->KeyIndex = key_idx;
		param->KeyLength = key_len;
		memcpy((u8 *) param->BSSID, addr, ETH_ALEN);
		memcpy((u8 *) &param->KeyRSC, seq, seq_len);

		if (key)
			memcpy(param->KeyMaterial, key, key_len);

		if (marvell_ioctl(drv->ifname, SIOCSIWENCODE, 
			(caddr_t)param, blen, 1)) {
			wpa_printf(MSG_WARNING, "Failed to set encryption.");
			ret = -1;
		}
		if (set_tx && flag) {
			param->KeyLength = 0;
			if (marvell_ioctl(drv->ifname, SIOCSIWENCODE, 
				(caddr_t)param, blen, 1)) {
				wpa_printf(MSG_WARNING, "Failed to "
					"enable transmit key.");
				ret = -1;
			}
		}
	}

	free(buf);

	return ret;
}

static int wpa_driver_marvell_set_countermeasures(void *priv, int enabled)
{
	struct wpa_driver_marvell_data *drv = priv;

	if (!enabled) {
		if (drv->CounterMeasure == TRUE) {
			if (wpa_driver_marvell_set_wpa(priv, 1) < 0) {
				fprintf(stderr, "WPA ON ioctl failed\n");
				return -1;
			}
			drv->CounterMeasure = FALSE;
		}
	} else {
		drv->CounterMeasure = TRUE;
	}

	wpa_printf(MSG_DEBUG, "%s: enabled=%d", __func__, enabled);
	return 0;
}

static int wpa_driver_marvell_set_drop_unencrypted(void *priv, int enabled)
{
	wpa_printf(MSG_DEBUG, "%s: enabled=%d", __func__, enabled);
	return 0;
}

static int wpa_driver_marvell_reset(const char *ifname, int type)
{
	return 0;
}

static int wpa_driver_marvell_mlme(void *priv, const u8 *addr, int cmd,
                 int reason_code)
{
	struct wpa_driver_marvell_data *drv = priv;
	wpa_printf(MSG_DEBUG, "%s", __func__);
	/* 
	* Send the deauth command if the driver supports(new driver) 
	* otherwise send an invalid key to force the driver(old driver)
	* to disconnect 
	*/

	if (SUBIOCTL_DEAUTH != -1)
		marvell2param(drv->ifname, IOCTL_DEAUTH, SUBIOCTL_DEAUTH, 0);
	else if (IOCTL_DEAUTH != -1)
		marvell_ioctl(drv->ifname, IOCTL_DEAUTH, NULL, 0, 1);
	else
		wpa_driver_marvell_set_key(drv, WPA_ALG_NONE, addr, 0,
			0, NULL, 0, NULL, 0xffff);

	if (IOCTL_ENCRYPTION_MODE != -1) {
		marvell2param(drv->ifname, IOCTL_ENCRYPTION_MODE,
		SUBIOCTL_ENCRYPTION_MODE, CIPHER_NONE);
	}

	if (IOCTL_REASSO_ON != -1) {
		wpa_printf(MSG_DEBUG, "marvell: auto association on");
		marvell2param(drv->ifname, IOCTL_REASSO_ON, 
		SUBIOCTL_REASSO_ON, 0);
	}

	if (wpa_driver_marvell_set_wpa(priv, 0) < 0) {
		fprintf(stderr, "Failed to disable WPA" "in the driver.\n");
		return -1;
	}

	wpa_driver_marvell_reset(drv->ifname, 0);

	return 0;
}

static int wpa_driver_marvell_deauthenticate(void *priv, const u8 *addr,
               int reason_code)
{
	wpa_printf(MSG_DEBUG, "%s", __func__);
	return wpa_driver_marvell_mlme(priv, addr, MLME_STA_DEAUTH,
              reason_code);
}

static int wpa_driver_marvell_disassociate(void *priv, const u8 *addr,
                 int reason_code)
{
	wpa_printf(MSG_DEBUG, "%s", __func__);
	return wpa_driver_marvell_mlme(priv, addr, MLME_STA_DISASSOC,
		reason_code);
}

static int wpa_driver_marvell_associate(void *priv, 
      struct wpa_driver_associate_params *params)
{
	int ret = 0;
	int allow_unencrypted_eapol;
	struct wpa_driver_marvell_data *drv = priv;

	wpa_printf(MSG_DEBUG, "%s", __func__);

	if (IOCTL_REASSO_OFF != -1) {
		wpa_printf(MSG_DEBUG, "marvell: auto association off");
		marvell2param(drv->ifname, IOCTL_REASSO_OFF, 
			SUBIOCTL_REASSO_OFF, 0);
	}

#ifdef MARVELL_DEBUG
	diag_printf("pairwise_suite = %d, group_suite = %d, key_mgmt_suite =%d \n", 
		params->pairwise_suite, params->group_suite, 
		params->key_mgmt_suite);
#endif  /* MARVELL_DEBUG */

	//set mode
	if (wpa_driver_wext_set_mode(drv->wext, params->mode) < 0)
		ret = -1;	

	/*
	* As in most cases the cipher is same for pairwise and group 
	* send only the group cipher, this is true for PSK, ENTERPRISE,
	* LEAP. This is a workaround, need to implement a better way.
	*/
   
	if (IOCTL_ENCRYPTION_MODE != -1) {
		marvell2param(drv->ifname, IOCTL_ENCRYPTION_MODE,
			SUBIOCTL_ENCRYPTION_MODE, params->group_suite);
	}

	/* 
	* If the Key mgmt is NONE then disable WPA in the driver, and don't 
	* set the WPA_IE in the driver. 
	*/
	if (params->key_mgmt_suite == KEY_MGMT_802_1X_NO_WPA) {
		if (wpa_driver_marvell_set_wpa(priv, 0) < 0) {
			fprintf(stderr, "Failed to disable WPA in the driver.\n");
			return -1;
		}
	} else {
		if (wpa_driver_marvell_set_wpa_ie(drv->ifname, (char *)params->wpa_ie, 
			params->wpa_ie_len) < 0)
			ret = -1;
	}
	
	if (params->mode == 1 && params->freq)
		wpa_driver_wext_set_freq(drv->wext, params->freq);

	/* Set SSID in Marvell drivers triggers association */
	if (wpa_driver_wext_set_ssid(drv->wext, params->ssid, 
		params->ssid_len) < 0)
		ret = -1;

	/*
	* Allow unencrypted EAPOL messages even if pairwise keys are set when
	* not using WPA. IEEE 802.1X specifies that these frames are not
	* encrypted, but WPA encrypts them when pairwise keys are in use.
	*/
	if (params->key_mgmt_suite == KEY_MGMT_802_1X ||
		params->key_mgmt_suite == KEY_MGMT_PSK)
		allow_unencrypted_eapol = 0;
	else
		allow_unencrypted_eapol = 1;

	return ret;
}

static int wpa_driver_marvell_scan(void *priv, const u8 * ssid,
              size_t ssid_len)
{
	WLAN_802_11_SSID  param;
	int     ret;
	struct wpa_driver_marvell_data *drv = priv;


	if (ssid == NULL) {
		return wpa_driver_wext_scan(drv->wext, ssid, ssid_len);
	}

	if (ssid_len > 32)
		ssid_len = 32;

	memset(&param, 0, sizeof(param));
	param.ssid_len = ssid_len;
	memcpy(param.ssid, ssid, ssid_len);

	ret = marvell_ioctl(drv->ifname, IOCTL_WLANEXTSCAN, 
		(caddr_t)&param, 4, 1);

	if (ret < 0) {
		wpa_printf(MSG_ERROR, "%s: EXTSCAN", __func__);
		return ret;
	}

	/* Not all drivers generate "scan completed" wireless event, so try to
		* read results after a timeout. */
	eloop_register_timeout(3, 0, wpa_driver_wext_scan_timeout, 
		drv->wext, drv->ctx);

	return ret;
}

static int wpa_driver_marvell_set_auth_alg(void *priv, int auth_alg)
{
	int algs = AUTH_ALG_OPEN_SYSTEM;
	int eap_algs = EAP_ALG_NONE;
	int ret = 0;
	struct wpa_driver_marvell_data *drv = priv;

	if (auth_alg & AUTH_ALG_LEAP) {
		eap_algs = EAP_ALG_LEAP;
		/* For WPA should be disabled in the driver */
		if ((ret = wpa_driver_marvell_set_wpa(priv, 0)) < 0) {
			fprintf(stderr,"Failed to disable WPA in the driver.\n");
			return ret;
		}
	}
  
	if (auth_alg & AUTH_ALG_OPEN_SYSTEM) {
		/* OPEN/OPEN+SHARED/OPEN+EAPLEAP/OPEN+SHARED+EAPLEAP */
		algs = AUTH_ALG_OPEN_SYSTEM;
	} else if (auth_alg & AUTH_ALG_SHARED_KEY) {
		/* SHARED/SHARED+EAPLEAP */
		algs = AUTH_ALG_SHARED_KEY;
	} else if (auth_alg & AUTH_ALG_LEAP) {
		/* NETWORKEAP+EAPLEAP */
		algs = AUTH_ALG_NETWORK_EAP;
	}

	wpa_printf(MSG_DEBUG, "Setting Auth Alg to 0x%X\n", algs);

	if (IOCTL_SET_AP_AUTH_ALGS != -1)
		ret = marvell2param(drv->ifname, IOCTL_SET_AP_AUTH_ALGS,
			SUBIOCTL_SET_AP_AUTH_ALGS, algs);
	return ret;
}

int wpa_driver_marvell_get_ssid(void *priv, u8 *ssid)
{
	struct wpa_driver_marvell_data *drv = priv;
	return wpa_driver_wext_get_ssid(drv->wext, ssid);
}

static int wpa_driver_marvell_get_bssid(void *priv, u8 *bssid)
{
	struct wpa_driver_marvell_data *drv = priv;
	return wpa_driver_wext_get_bssid(drv->wext, bssid);
}

static int wpa_driver_marvell_get_scan_results(void *priv,
                struct wpa_scan_result *results,
                size_t max_size)
{
	struct wpa_driver_marvell_data *drv = priv;
	return wpa_driver_wext_get_scan_results(drv->wext, results, max_size);
}

static int wpa_driver_marvell_set_operstate(void *priv, int state)
{
	struct wpa_driver_marvell_data *drv = priv;
	return wpa_driver_wext_set_operstate(drv->wext, state);
}


static int wpa_driver_marvell_set_mode(void *priv, int mode)
{
	struct wpa_driver_marvell_data *drv = priv;
	return wpa_driver_wext_set_mode(drv->wext, mode);
}


static void *wpa_driver_marvell_init(void *ctx, const char *ifname)
{
	struct wpa_driver_marvell_data *drv;

	marvell_init_ioctl_numbers(ifname);

	drv = os_malloc(sizeof(*drv));
	if (drv == NULL)
		return NULL;
	memset(drv, 0, sizeof(*drv));
	drv->wext = wpa_driver_wext_init(ctx, ifname);
	if (drv->wext == NULL) {
		free(drv);
		return NULL;
	}

	drv->ctx = ctx;
	strncpy(drv->ifname, ifname, sizeof(drv->ifname));
	drv->sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (drv->sock < 0) {
		perror("socket");
		wpa_driver_wext_deinit(drv->wext);
		free(drv);
		return NULL;
	}

	return drv;
}

static void wpa_driver_marvell_deinit(void *priv)
{
	struct wpa_driver_marvell_data *drv = priv;

	if (drv->WpaEnabled == TRUE) {
		if (wpa_driver_marvell_set_wpa(priv, 0) < 0) {
			fprintf(stderr, "WPA ON ioctl failed\n");
		}
	}

	wpa_driver_wext_deinit(drv->wext);
	close(drv->sock);
	free(drv);
}

struct wpa_driver_ops wpa_driver_marvell_ops = {
	/* .name */					"marvell",
	/* .desc */					"Marvell Semiconductor Inc.",
	/* .get_bssid */			wpa_driver_marvell_get_bssid,
	/* .get_ssid */				wpa_driver_marvell_get_ssid,
	/* .set_wpa */				wpa_driver_marvell_set_wpa,
	/* .set_key */				wpa_driver_marvell_set_key,
	/* .init */					wpa_driver_marvell_init,
	/* .deinit */				wpa_driver_marvell_deinit,
	/* .set_param */			NULL,
	/* .set_countermeasures */	wpa_driver_marvell_set_countermeasures,	
	/* .set_drop_unencrypted */	wpa_driver_marvell_set_drop_unencrypted,
	/* .scan */					wpa_driver_marvell_scan,
	/* .get_scan_results */		wpa_driver_marvell_get_scan_results,
	/* .deauthenticate */		wpa_driver_marvell_deauthenticate,
	/* .disassociate */			wpa_driver_marvell_disassociate,
	/* .associate */			wpa_driver_marvell_associate,
	/* .set_auth_alg */			wpa_driver_marvell_set_auth_alg,
	/* .add_pmkid */			NULL,
	/* .remove_pmkid */			NULL,
	/* .flush_pmkid */			NULL,
	/* .get_capa */				NULL,
	/* .poll */					NULL,
	/* .get_ifname */			NULL,
	/* .get_mac_addr */			NULL,
	/* .send_eapol */			NULL,
	/* .set_operstate */		wpa_driver_marvell_set_operstate,
	/* .mlme_setprotection */	NULL,
	/* .get_hw_feature_data */	NULL,
	/* .set_channel */			NULL,
	/* .set_ssid */				NULL,
	/* .set_bssid */			NULL,
	/* .send_mlme */			NULL,
	/* .mlme_add_sta */			NULL,
	/* .mlme_remove_sta */		NULL,
	/* .set_mode */				wpa_driver_marvell_set_mode
};

#ifdef MARVELL_DEBUG
void DUMPHEX(char *prompt, u8 * buf, int len)
{
	int i;

	diag_printf("%s: ", prompt);
	for (i = 0; i < len; i++)
		diag_printf("%02x ", *buf++);
	diag_printf("\n");
}
#endif  /* MARVELL_DEBUG */


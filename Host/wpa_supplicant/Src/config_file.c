/*
 * WPA Supplicant / Configuration backend: text file
 * Copyright (c) 2003-2006, Jouni Malinen <jkmaline@cc.hut.fi>
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
 * This file implements a configuration backend for text files. All the
 * configuration information is stored in a text file that uses a format
 * described in the sample configuration file, wpa_supplicant.conf.
 */

#include "includes.h"

#include "common.h"
#include "config.h"
#include "base64.h"
#include "eap_methods.h"


#if 1	//xhchen: default config file


static char CONF_FILE_MANAGED[] = "\r\n"
"ctrl_interface=/var/run/wpa_supplicant\r\n"
"ap_scan=1\r\n";

static char CONF_FILE_MANAGED_WPA[] =
"network={\r\n"
"	ssid=\"%s\"\r\n"
"	scan_ssid=1\r\n"
"	proto=WPA WPA2\r\n"
"	key_mgmt=WPA-PSK\r\n"
"	pairwise=CCMP TKIP\r\n"
"	group=CCMP TKIP WEP104 WEP40\r\n"
"	psk=\"%s\"\r\n"
"}\r\n";

static char CONF_FILE_MANAGED_WEP[] =
"network={\r\n"
"	ssid=\"%s\"\r\n"
"	scan_ssid=1\r\n"
"	key_mgmt=NONE\r\n"
"	wep_key0=%s%s%s\r\n"
"	wep_tx_keyidx=0\r\n"
"}\r\n";

static char CONF_FILE_MANAGED_PLAINTEXT[] =
"network={\r\n"
"	ssid=\"%s\"\r\n"
"	scan_ssid=1\r\n"
"	key_mgmt=NONE\r\n"
"}\r\n";

static char CONF_FILE_ADHOC[] = "\r\n"
"ctrl_interface=/var/run/wpa_supplicant\r\n"
"ap_scan=2\r\n";

static char CONF_FILE_ADHOC_WEP[] =
"network={\r\n"
"	ssid=\"%s\"\r\n"
"	mode=1\r\n"
"	frequency=%d\r\n"
"	scan_ssid=1\r\n"
"	key_mgmt=NONE\r\n"
"	wep_key0=%s%s%s\r\n"
"	wep_tx_keyidx=0\r\n"
"}\r\n";

static char CONF_FILE_ADHOC_NONE[] = 
"network={\r\n"
"	ssid=\"%s\"\r\n"
"	mode=1\r\n"
"	frequency=%d\r\n"
"	scan_ssid=1\r\n"
"	key_mgmt=NONE\r\n"
"}\r\n";


#define CONF_SIZE_MANAGED_WPA	(sizeof(CONF_FILE_MANAGED_WPA) + 64 /* ssid */ + 64 /* password */)
#define CONF_SIZE_MANAGED_WEP	(sizeof(CONF_FILE_MANAGED_WEP) + 64 /* ssid */ + 28 /* password */)
#define CONF_SIZE_MANAGED_PLAINTEXT	(sizeof(CONF_FILE_MANAGED_PLAINTEXT) + 64 /* ssid */)
#define CONF_SIZE_MANAGED		(sizeof(CONF_FILE_MANAGED) + CONF_SIZE_MANAGED_WPA + CONF_SIZE_MANAGED_WEP + CONF_SIZE_MANAGED_PLAINTEXT)

#define CONF_SIZE_ADHOC_WEP		(sizeof(CONF_FILE_ADHOC) + sizeof(CONF_FILE_ADHOC_WEP) + 64 /* ssid */ + 64 /* password */)
#define CONF_SIZE_ADHOC_NONE	(sizeof(CONF_FILE_ADHOC) + sizeof(CONF_FILE_ADHOC_NONE) + 64 /* ssid */)

#define CONF_SIZE_FILE	MAX(CONF_SIZE_MANAGED, MAX(CONF_SIZE_ADHOC_WEP, CONF_SIZE_ADHOC_NONE))


static char g_conf_file_content[CONF_SIZE_FILE] = "\r\n"
"ctrl_interface=/var/run/wpa_supplicant\r\n"
"ap_scan=1\r\n"
"network={\r\n"
"	ssid=\"ipcam\"\r\n"
"	scan_ssid=1\r\n"
"	proto=WPA WPA2\r\n"
"	key_mgmt=WPA-PSK\r\n"
"	pairwise=CCMP TKIP\r\n"
"	group=CCMP TKIP WEP104 WEP40\r\n"
"	psk=\"0000000000\"\r\n"
"}\r\n"
"network={\r\n"
"	ssid=\"ipcam\"\r\n"
"	scan_ssid=1\r\n"
"	key_mgmt=NONE\r\n"
"	wep_key0=0000000000\r\n"
"	wep_tx_keyidx=0\r\n"
"}\r\n"
"network={\r\n"
"	ssid=\"ipcam\"\r\n"
"	scan_ssid=1\r\n"
"	key_mgmt=NONE\r\n"
"}\r\n"
;


void wsp_set_network_managed(const char *ssid, const char *key)
{
	char *pos;
	size_t key_len = strlen(key);
	
	
	pos = g_conf_file_content;
	pos += sprintf(pos, CONF_FILE_MANAGED);
	
	/* WPA configuration */
	if (key_len >= 8 && key_len <= 63)
	{
		pos += sprintf(pos, CONF_FILE_MANAGED_WPA,
			ssid,					/* ssid for WPA */
			key						/* psk for WPA */
			);
	}
	
	/* WEP configuration */
	if ( ( key_len > 0 && key_len <= MAX_WEP_KEY_LEN) || key_len == 26 )
	{
		const char *asc_flag;
		char asc_key = 1;
	
		if (key_len == 10 || key_len == 26)
		{
			size_t i = 0;
			for (i = 0; i < key_len; ++i)
			{
				if (!(key[i] >= '0' && key[i] <= '9')
					&& !(key[i] >= 'A' && key[i] <= 'F')
					&& !(key[i] >= 'a' && key[i] <= 'f'))
					break;
			}
		
			if (i >= key_len)
				asc_key = 0;
		}
	
		asc_flag = (asc_key ? "\"" : "");
		pos += sprintf(pos, CONF_FILE_MANAGED_WEP,
			ssid,					/* ssid for WEP */
			asc_flag, key, asc_flag	/* key for WEP */
			);
	}
	
	/* Plain text configuration */	
	pos += sprintf(pos, CONF_FILE_MANAGED_PLAINTEXT,
		ssid					/* ssid for plain text */
		);
	
	
	//diag_printf("----------------------\n");
	//diag_printf("%s\n", g_conf_file_content);
	wsp_set_wireless_config();
}


void wsp_set_network_adhoc(const char *ssid, const char *key, int channel)
{
	char *pos;
	size_t key_len = strlen(key);
	int frequency;
	
	/* channel */
	if (channel <= 0 || channel > 14)
		channel = 6;
	
	if (channel == 14)
		frequency = 2484;
	else
		frequency = 2412 + (channel - 1) * 5;
	
	
	pos = g_conf_file_content;
	pos += sprintf(pos, CONF_FILE_ADHOC);
	

	/* WEP configuration */
	if ( ( key_len > 0 && key_len <= MAX_WEP_KEY_LEN ) || key_len == 26 )
	{
		const char *asc_flag;
		char asc_key = 1;
		if (key_len == 10 || key_len == 26)
		{
			size_t i = 0;
			for (i = 0; i < key_len; ++i)
			{
				if (!(key[i] >= '0' && key[i] <= '9')
					&& !(key[i] >= 'A' && key[i] <= 'F')
					&& !(key[i] >= 'a' && key[i] <= 'f'))
					break;
			}
		
			if (i >= key_len)
				asc_key = 0;
		}
	
		asc_flag = (asc_key ? "\"" : "");
		pos += sprintf(pos,
			CONF_FILE_ADHOC_WEP,
			ssid,					/* ssid for WEP */
			frequency,
			asc_flag, key, asc_flag	/* key for WEP */
			);
	}
	else
	{
		pos += sprintf(pos,
			CONF_FILE_ADHOC_NONE,
			ssid,					/* ssid for plain text */
			frequency
			);
	}
	
	
	//diag_printf("----------------------\n");
	//diag_printf("%s\n", g_conf_file_content);
	wsp_set_wireless_config();
}



typedef struct
{
	char *buffer;
	size_t size;
	size_t max_size;
	size_t offset;
	char mode;
} BF_FILE;


BF_FILE *bf_fopen(const char *path, const char *mode)
{
	BF_FILE *stream = (BF_FILE *)os_malloc(sizeof(BF_FILE));
	
	if (strcmp(mode, "r") == 0)
	{
		stream->buffer = g_conf_file_content;
		stream->size = strlen(stream->buffer);
		stream->max_size = sizeof(g_conf_file_content);
		stream->offset = 0;
		stream->mode = 'r';
	}
	else
	{
		stream->buffer = g_conf_file_content;
		stream->size = 0;
		stream->max_size = sizeof(g_conf_file_content);
		stream->offset = 0;
		stream->mode = 'w';
	}
	return stream;
}

int bf_fclose(BF_FILE *stream)
{
	if (stream == NULL)
		return -1;
	free(stream);
	return 0;
}


char *bf_fgets(char *s, int size, BF_FILE *stream)
{
	int i;
	char *p;
	
	if (stream->offset >= stream->size)
		return NULL;
	
	p = s;
	for (i = 1; i < size; ++i)
	{
		char c;
		
		if (stream->offset >= stream->size)
			break;

		c = stream->buffer[stream->offset++];
		if (c == '\n')
			break;
			
		*(p++) = c;
	}
	*p = '\0';
	
	return s;
}


int bf_fprintf(BF_FILE *stream, const char *format, ...)
{
	int rt;
	va_list ap;

	va_start(ap, format);
	rt = vsnprintf(stream->buffer + stream->offset, stream->max_size - stream->offset, format, ap);
	va_end(ap);
	
	if (rt >= 0)
		stream->offset += rt;
	
	return rt;
}

#endif

/**
 * wpa_config_get_line - Read the next configuration file line
 * @s: Buffer for the line
 * @size: The buffer length
 * @stream: File stream to read from
 * @line: Pointer to a variable storing the file line number
 * @_pos: Buffer for the pointer to the beginning of data on the text line or
 * %NULL if not needed (returned value used instead)
 * Returns: Pointer to the beginning of data on the text line or %NULL if no
 * more text lines are available.
 *
 * This function reads the next non-empty line from the configuration file and
 * removes comments. The returned string is guaranteed to be null-terminated.
 */
static char * wpa_config_get_line(char *s, int size, BF_FILE *stream, int *line,
				  char **_pos)
{
	char *pos, *end, *sstart;

	while (bf_fgets(s, size, stream)) {
		(*line)++;
		s[size - 1] = '\0';
		pos = s;

		/* Skip white space from the beginning of line. */
		while (*pos == ' ' || *pos == '\t' || *pos == '\r')
			pos++;

		/* Skip comment lines and empty lines */
		if (*pos == '#' || *pos == '\n' || *pos == '\0')
			continue;

		/*
		 * Remove # comments unless they are within a double quoted
		 * string.
		 */
		sstart = os_strchr(pos, '"');
		if (sstart)
			sstart = os_strrchr(sstart + 1, '"');
		if (!sstart)
			sstart = pos;
		end = os_strchr(sstart, '#');
		if (end)
			*end-- = '\0';
		else
			end = pos + os_strlen(pos) - 1;

		/* Remove trailing white space. */
		while (end > pos &&
		       (*end == '\n' || *end == ' ' || *end == '\t' ||
			*end == '\r'))
			*end-- = '\0';

		if (*pos == '\0')
			continue;

		if (_pos)
			*_pos = pos;
		return pos;
	}

	if (_pos)
		*_pos = NULL;
	return NULL;
}


static int wpa_config_validate_network(struct wpa_ssid *ssid, int line)
{
	int errors = 0;

	if (ssid->passphrase) {
		if (ssid->psk_set) {
			wpa_printf(MSG_ERROR, "Line %d: both PSK and "
				   "passphrase configured.", line);
			errors++;
		}
		wpa_config_update_psk(ssid);
	}

	if ((ssid->key_mgmt & WPA_KEY_MGMT_PSK) && !ssid->psk_set) {
		wpa_printf(MSG_ERROR, "Line %d: WPA-PSK accepted for key "
			   "management, but no PSK configured.", line);
		errors++;
	}

	if ((ssid->group_cipher & WPA_CIPHER_CCMP) &&
	    !(ssid->pairwise_cipher & WPA_CIPHER_CCMP) &&
	    !(ssid->pairwise_cipher & WPA_CIPHER_NONE)) {
		/* Group cipher cannot be stronger than the pairwise cipher. */
		wpa_printf(MSG_DEBUG, "Line %d: removed CCMP from group cipher"
			   " list since it was not allowed for pairwise "
			   "cipher", line);
		ssid->group_cipher &= ~WPA_CIPHER_CCMP;
	}

	return errors;
}


static struct wpa_ssid * wpa_config_read_network(BF_FILE *f, int *line, int id)
{
	struct wpa_ssid *ssid;
	int errors = 0, end = 0;
	char buf[256], *pos, *pos2;

	wpa_printf(MSG_MSGDUMP, "Line: %d - start of a new network block",
		   *line);
	ssid = os_zalloc(sizeof(*ssid));
	if (ssid == NULL)
		return NULL;
	ssid->id = id;

	wpa_config_set_network_defaults(ssid);

	while (wpa_config_get_line(buf, sizeof(buf), f, line, &pos)) {
		if (os_strcmp(pos, "}") == 0) {
			end = 1;
			break;
		}

		pos2 = os_strchr(pos, '=');
		if (pos2 == NULL) {
			wpa_printf(MSG_ERROR, "Line %d: Invalid SSID line "
				   "'%s'.", *line, pos);
			errors++;
			continue;
		}

		*pos2++ = '\0';
		if (*pos2 == '"') {
			if (os_strchr(pos2 + 1, '"') == NULL) {
				wpa_printf(MSG_ERROR, "Line %d: invalid "
					   "quotation '%s'.", *line, pos2);
				errors++;
				continue;
			}
		}

		if (wpa_config_set(ssid, pos, pos2, *line) < 0)
			errors++;
	}

	if (!end) {
		wpa_printf(MSG_ERROR, "Line %d: network block was not "
			   "terminated properly.", *line);
		errors++;
	}

	errors += wpa_config_validate_network(ssid, *line);

	if (errors) {
		wpa_config_free_ssid(ssid);
		ssid = NULL;
	}

	return ssid;
}


static struct wpa_config_blob * wpa_config_read_blob(BF_FILE *f, int *line,
						     const char *name)
{
	struct wpa_config_blob *blob;
	char buf[256], *pos;
	unsigned char *encoded = NULL, *nencoded;
	int end = 0;
	size_t encoded_len = 0, len;

	wpa_printf(MSG_MSGDUMP, "Line: %d - start of a new named blob '%s'",
		   *line, name);

	while (wpa_config_get_line(buf, sizeof(buf), f, line, &pos)) {
		if (os_strcmp(pos, "}") == 0) {
			end = 1;
			break;
		}

		len = os_strlen(pos);
		nencoded = os_realloc(encoded, encoded_len + len);
		if (nencoded == NULL) {
			wpa_printf(MSG_ERROR, "Line %d: not enough memory for "
				   "blob", *line);
			os_free(encoded);
			return NULL;
		}
		encoded = nencoded;
		os_memcpy(encoded + encoded_len, pos, len);
		encoded_len += len;
	}

	if (!end) {
		wpa_printf(MSG_ERROR, "Line %d: blob was not terminated "
			   "properly", *line);
		os_free(encoded);
		return NULL;
	}

	blob = os_zalloc(sizeof(*blob));
	if (blob == NULL) {
		os_free(encoded);
		return NULL;
	}
	blob->name = os_strdup(name);
	blob->data = base64_decode(encoded, encoded_len, &blob->len);
	os_free(encoded);

	if (blob->name == NULL || blob->data == NULL) {
		wpa_config_free_blob(blob);
		return NULL;
	}

	return blob;
}


struct wpa_config * wpa_config_read(const char *name)
{
	BF_FILE *f;
	char buf[256], *pos;
	int errors = 0, line = 0;
	struct wpa_ssid *ssid, *tail = NULL, *head = NULL;
	struct wpa_config *config;
	int id = 0;

	config = wpa_config_alloc_empty(NULL, NULL);
	if (config == NULL)
		return NULL;
	wpa_printf(MSG_DEBUG, "Reading configuration file '%s'", name);
	f = bf_fopen(name, "r");
	if (f == NULL) {
		os_free(config);
		return NULL;
	}

	while (wpa_config_get_line(buf, sizeof(buf), f, &line, &pos)) {
		if (os_strcmp(pos, "network={") == 0) {
			ssid = wpa_config_read_network(f, &line, id++);
			if (ssid == NULL) {
				wpa_printf(MSG_ERROR, "Line %d: failed to "
					   "parse network block.", line);
				errors++;
				continue;
			}
			if (head == NULL) {
				head = tail = ssid;
			} else {
				tail->next = ssid;
				tail = ssid;
			}
			if (wpa_config_add_prio_network(config, ssid)) {
				wpa_printf(MSG_ERROR, "Line %d: failed to add "
					   "network block to priority list.",
					   line);
				errors++;
				continue;
			}
		} else if (os_strncmp(pos, "blob-base64-", 12) == 0) {
			char *bname = pos + 12, *name_end;
			struct wpa_config_blob *blob;

			name_end = os_strchr(bname, '=');
			if (name_end == NULL) {
				wpa_printf(MSG_ERROR, "Line %d: no blob name "
					   "terminator", line);
				errors++;
				continue;
			}
			*name_end = '\0';

			blob = wpa_config_read_blob(f, &line, bname);
			if (blob == NULL) {
				wpa_printf(MSG_ERROR, "Line %d: failed to read"
					   " blob %s", line, bname);
				errors++;
				continue;
			}
			wpa_config_set_blob(config, blob);
#ifdef CONFIG_CTRL_IFACE
		} else if (os_strncmp(pos, "ctrl_interface=", 15) == 0) {
			os_free(config->ctrl_interface);
			config->ctrl_interface = os_strdup(pos + 15);
			wpa_printf(MSG_DEBUG, "ctrl_interface='%s'",
				   config->ctrl_interface);
		} else if (os_strncmp(pos, "ctrl_interface_group=", 21) == 0) {
			os_free(config->ctrl_interface_group);
			config->ctrl_interface_group = os_strdup(pos + 21);
			wpa_printf(MSG_DEBUG, "ctrl_interface_group='%s' "
				   "(DEPRECATED)",
				   config->ctrl_interface_group);
#endif /* CONFIG_CTRL_IFACE */
		} else if (os_strncmp(pos, "eapol_version=", 14) == 0) {
			config->eapol_version = atoi(pos + 14);
			if (config->eapol_version < 1 ||
			    config->eapol_version > 2) {
				wpa_printf(MSG_ERROR, "Line %d: Invalid EAPOL "
					   "version (%d): '%s'.",
					   line, config->eapol_version, pos);
				errors++;
				continue;
			}
			wpa_printf(MSG_DEBUG, "eapol_version=%d",
				   config->eapol_version);
		} else if (os_strncmp(pos, "ap_scan=", 8) == 0) {
			config->ap_scan = atoi(pos + 8);
			wpa_printf(MSG_DEBUG, "ap_scan=%d", config->ap_scan);
		} else if (os_strncmp(pos, "fast_reauth=", 12) == 0) {
			config->fast_reauth = atoi(pos + 12);
			wpa_printf(MSG_DEBUG, "fast_reauth=%d",
				   config->fast_reauth);
		} else if (os_strncmp(pos, "opensc_engine_path=", 19) == 0) {
			os_free(config->opensc_engine_path);
			config->opensc_engine_path = os_strdup(pos + 19);
			wpa_printf(MSG_DEBUG, "opensc_engine_path='%s'",
				   config->opensc_engine_path);
		} else if (os_strncmp(pos, "pkcs11_engine_path=", 19) == 0) {
			os_free(config->pkcs11_engine_path);
			config->pkcs11_engine_path = os_strdup(pos + 19);
			wpa_printf(MSG_DEBUG, "pkcs11_engine_path='%s'",
				   config->pkcs11_engine_path);
		} else if (os_strncmp(pos, "pkcs11_module_path=", 19) == 0) {
			os_free(config->pkcs11_module_path);
			config->pkcs11_module_path = os_strdup(pos + 19);
			wpa_printf(MSG_DEBUG, "pkcs11_module_path='%s'",
				   config->pkcs11_module_path);
		} else if (os_strncmp(pos, "driver_param=", 13) == 0) {
			os_free(config->driver_param);
			config->driver_param = os_strdup(pos + 13);
			wpa_printf(MSG_DEBUG, "driver_param='%s'",
				   config->driver_param);
		} else if (os_strncmp(pos, "dot11RSNAConfigPMKLifetime=", 27)
			   == 0) {
			config->dot11RSNAConfigPMKLifetime = atoi(pos + 27);
			wpa_printf(MSG_DEBUG, "dot11RSNAConfigPMKLifetime=%d",
				   config->dot11RSNAConfigPMKLifetime);
		} else if (os_strncmp(pos,
				      "dot11RSNAConfigPMKReauthThreshold=", 34)
			   == 0) {
			config->dot11RSNAConfigPMKReauthThreshold =
				atoi(pos + 34);
			wpa_printf(MSG_DEBUG,
				   "dot11RSNAConfigPMKReauthThreshold=%d",
				   config->dot11RSNAConfigPMKReauthThreshold);
		} else if (os_strncmp(pos, "dot11RSNAConfigSATimeout=", 25) ==
			   0) {
			config->dot11RSNAConfigSATimeout = atoi(pos + 25);
			wpa_printf(MSG_DEBUG, "dot11RSNAConfigSATimeout=%d",
				   config->dot11RSNAConfigSATimeout);
		} else if (os_strncmp(pos, "update_config=", 14) == 0) {
			config->update_config = atoi(pos + 14);
			wpa_printf(MSG_DEBUG, "update_config=%d",
				   config->update_config);
		} else if (os_strncmp(pos, "load_dynamic_eap=", 17) == 0) {
			char *so = pos + 17;
			int ret;
			wpa_printf(MSG_DEBUG, "load_dynamic_eap=%s", so);
			ret = eap_peer_method_load(so);
			if (ret == -2) {
				wpa_printf(MSG_DEBUG, "This EAP type was "
					   "already loaded - not reloading.");
			} else if (ret) {
				wpa_printf(MSG_ERROR, "Line %d: Failed to "
					   "load dynamic EAP method '%s'.",
					   line, so);
				errors++;
			}
		} else {
			wpa_printf(MSG_ERROR, "Line %d: Invalid configuration "
				   "line '%s'.", line, pos);
			errors++;
			continue;
		}
	}

	bf_fclose(f);

	config->ssid = head;
	wpa_config_debug_dump_networks(config);

	if (errors) {
		wpa_config_free(config);
		config = NULL;
		head = NULL;
	}

	return config;
}


static void write_str(BF_FILE *f, const char *field, struct wpa_ssid *ssid)
{
	char *value = wpa_config_get(ssid, field);
	if (value == NULL)
		return;
	bf_fprintf(f, "\t%s=%s\n", field, value);
	os_free(value);
}


static void write_int(BF_FILE *f, const char *field, int value, int def)
{
	if (value == def)
		return;
	bf_fprintf(f, "\t%s=%d\n", field, value);
}


static void write_bssid(BF_FILE *f, struct wpa_ssid *ssid)
{
	char *value = wpa_config_get(ssid, "bssid");
	if (value == NULL)
		return;
	bf_fprintf(f, "\tbssid=%s\n", value);
	os_free(value);
}


static void write_psk(BF_FILE *f, struct wpa_ssid *ssid)
{
	char *value = wpa_config_get(ssid, "psk");
	if (value == NULL)
		return;
	bf_fprintf(f, "\tpsk=%s\n", value);
	os_free(value);
}


static void write_proto(BF_FILE *f, struct wpa_ssid *ssid)
{
	char *value;

	if (ssid->proto == DEFAULT_PROTO)
		return;

	value = wpa_config_get(ssid, "proto");
	if (value == NULL)
		return;
	if (value[0])
		bf_fprintf(f, "\tproto=%s\n", value);
	os_free(value);
}


static void write_key_mgmt(BF_FILE *f, struct wpa_ssid *ssid)
{
	char *value;

	if (ssid->key_mgmt == DEFAULT_KEY_MGMT)
		return;

	value = wpa_config_get(ssid, "key_mgmt");
	if (value == NULL)
		return;
	if (value[0])
		bf_fprintf(f, "\tkey_mgmt=%s\n", value);
	os_free(value);
}


static void write_pairwise(BF_FILE *f, struct wpa_ssid *ssid)
{
	char *value;

	if (ssid->pairwise_cipher == DEFAULT_PAIRWISE)
		return;

	value = wpa_config_get(ssid, "pairwise");
	if (value == NULL)
		return;
	if (value[0])
		bf_fprintf(f, "\tpairwise=%s\n", value);
	os_free(value);
}


static void write_group(BF_FILE *f, struct wpa_ssid *ssid)
{
	char *value;

	if (ssid->group_cipher == DEFAULT_GROUP)
		return;

	value = wpa_config_get(ssid, "group");
	if (value == NULL)
		return;
	if (value[0])
		bf_fprintf(f, "\tgroup=%s\n", value);
	os_free(value);
}


static void write_auth_alg(BF_FILE *f, struct wpa_ssid *ssid)
{
	char *value;

	if (ssid->auth_alg == 0)
		return;

	value = wpa_config_get(ssid, "auth_alg");
	if (value == NULL)
		return;
	if (value[0])
		bf_fprintf(f, "\tauth_alg=%s\n", value);
	os_free(value);
}


#ifdef IEEE8021X_EAPOL
static void write_eap(BF_FILE *f, struct wpa_ssid *ssid)
{
	char *value;

	value = wpa_config_get(ssid, "eap");
	if (value == NULL)
		return;

	if (value[0])
		bf_fprintf(f, "\teap=%s\n", value);
	os_free(value);
}
#endif /* IEEE8021X_EAPOL */


static void write_wep_key(BF_FILE *f, int idx, struct wpa_ssid *ssid)
{
	char field[20], *value;

	os_snprintf(field, sizeof(field), "wep_key%d", idx);
	value = wpa_config_get(ssid, field);
	if (value) {
		bf_fprintf(f, "\t%s=%s\n", field, value);
		os_free(value);
	}
}


static void wpa_config_write_network(BF_FILE *f, struct wpa_ssid *ssid)
{
	int i;

#define STR(t) write_str(f, #t, ssid)
#define INT(t) write_int(f, #t, ssid->t, 0)
#define INT_DEF(t, def) write_int(f, #t, ssid->t, def)

	STR(ssid);
	INT(scan_ssid);
	write_bssid(f, ssid);
	write_psk(f, ssid);
	write_proto(f, ssid);
	write_key_mgmt(f, ssid);
	write_pairwise(f, ssid);
	write_group(f, ssid);
	write_auth_alg(f, ssid);
#ifdef IEEE8021X_EAPOL
	write_eap(f, ssid);
	STR(identity);
	STR(anonymous_identity);
	STR(eappsk);
	STR(nai);
	STR(password);
	STR(ca_cert);
	STR(ca_path);
	STR(client_cert);
	STR(private_key);
	STR(private_key_passwd);
	STR(dh_file);
	STR(subject_match);
	STR(altsubject_match);
	STR(ca_cert2);
	STR(ca_path2);
	STR(client_cert2);
	STR(private_key2);
	STR(private_key2_passwd);
	STR(dh_file2);
	STR(subject_match2);
	STR(altsubject_match2);
	STR(phase1);
	STR(phase2);
	STR(pcsc);
	STR(pin);
	STR(engine_id);
	STR(key_id);
	INT(engine);
	INT_DEF(eapol_flags, DEFAULT_EAPOL_FLAGS);
#endif /* IEEE8021X_EAPOL */
	for (i = 0; i < 4; i++)
		write_wep_key(f, i, ssid);
	INT(wep_tx_keyidx);
	INT(priority);
#ifdef IEEE8021X_EAPOL
	INT_DEF(eap_workaround, DEFAULT_EAP_WORKAROUND);
	STR(pac_file);
	INT_DEF(fragment_size, DEFAULT_FRAGMENT_SIZE);
#endif /* IEEE8021X_EAPOL */
	INT(mode);
	INT(proactive_key_caching);
	INT(disabled);
	INT(peerkey);
#ifdef CONFIG_IEEE80211W
	INT(ieee80211w);
#endif /* CONFIG_IEEE80211W */
	STR(id_str);

#undef STR
#undef INT
#undef INT_DEF
}


static int wpa_config_write_blob(BF_FILE *f, struct wpa_config_blob *blob)
{
	unsigned char *encoded;

	encoded = base64_encode(blob->data, blob->len, NULL);
	if (encoded == NULL)
		return -1;

	bf_fprintf(f, "\nblob-base64-%s={\n%s}\n", blob->name, encoded);
	os_free(encoded);
	return 0;
}


static void wpa_config_write_global(BF_FILE *f, struct wpa_config *config)
{
#ifdef CONFIG_CTRL_IFACE
	if (config->ctrl_interface)
		bf_fprintf(f, "ctrl_interface=%s\n", config->ctrl_interface);
	if (config->ctrl_interface_group)
		bf_fprintf(f, "ctrl_interface_group=%s\n",
			config->ctrl_interface_group);
#endif /* CONFIG_CTRL_IFACE */
	if (config->eapol_version != DEFAULT_EAPOL_VERSION)
		bf_fprintf(f, "eapol_version=%d\n", config->eapol_version);
	if (config->ap_scan != DEFAULT_AP_SCAN)
		bf_fprintf(f, "ap_scan=%d\n", config->ap_scan);
	if (config->fast_reauth != DEFAULT_FAST_REAUTH)
		bf_fprintf(f, "fast_reauth=%d\n", config->fast_reauth);
	if (config->opensc_engine_path)
		bf_fprintf(f, "opensc_engine_path=%s\n",
			config->opensc_engine_path);
	if (config->pkcs11_engine_path)
		bf_fprintf(f, "pkcs11_engine_path=%s\n",
			config->pkcs11_engine_path);
	if (config->pkcs11_module_path)
		bf_fprintf(f, "pkcs11_module_path=%s\n",
			config->pkcs11_module_path);
	if (config->driver_param)
		bf_fprintf(f, "driver_param=%s\n", config->driver_param);
	if (config->dot11RSNAConfigPMKLifetime)
		bf_fprintf(f, "dot11RSNAConfigPMKLifetime=%d\n",
			config->dot11RSNAConfigPMKLifetime);
	if (config->dot11RSNAConfigPMKReauthThreshold)
		bf_fprintf(f, "dot11RSNAConfigPMKReauthThreshold=%d\n",
			config->dot11RSNAConfigPMKReauthThreshold);
	if (config->dot11RSNAConfigSATimeout)
		bf_fprintf(f, "dot11RSNAConfigSATimeout=%d\n",
			config->dot11RSNAConfigSATimeout);
	if (config->update_config)
		bf_fprintf(f, "update_config=%d\n", config->update_config);
}


int wpa_config_write(const char *name, struct wpa_config *config)
{
	BF_FILE *f;
	struct wpa_ssid *ssid;
	struct wpa_config_blob *blob;
	int ret = 0;

	wpa_printf(MSG_DEBUG, "Writing configuration file '%s'", name);

	f = bf_fopen(name, "w");
	if (f == NULL) {
		wpa_printf(MSG_DEBUG, "Failed to open '%s' for writing", name);
		return -1;
	}

	wpa_config_write_global(f, config);

	for (ssid = config->ssid; ssid; ssid = ssid->next) {
		bf_fprintf(f, "\nnetwork={\n");
		wpa_config_write_network(f, ssid);
		bf_fprintf(f, "}\n");
	}

	for (blob = config->blobs; blob; blob = blob->next) {
		ret = wpa_config_write_blob(f, blob);
		if (ret)
			break;
	}

	bf_fclose(f);

	wpa_printf(MSG_DEBUG, "Configuration file '%s' written %ssuccessfully",
		   name, ret ? "un" : "");
	return ret;
}

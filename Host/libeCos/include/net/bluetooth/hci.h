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
 *  $Id: hci.h,v 1.5 2002/06/27 17:29:30 maxk Exp $
 */

#ifndef __HCI_H
#define __HCI_H

#define HCI_MAX_ACL_SIZE	1024
#define HCI_MAX_SCO_SIZE	255
#define HCI_MAX_EVENT_SIZE	260
#define HCI_MAX_FRAME_SIZE	(HCI_MAX_ACL_SIZE + 4)

/* HCI dev events */
#define HCI_DEV_REG	1
#define HCI_DEV_UNREG   2
#define HCI_DEV_UP	3
#define HCI_DEV_DOWN	4
#define HCI_DEV_SUSPEND	5
#define HCI_DEV_RESUME	6

/* HCI device types */
#define HCI_VHCI	0
#define HCI_USB		1
#define HCI_PCCARD	2
#define HCI_UART 	3
#define HCI_RS232 	4
#define HCI_PCI		5

/* HCI device quirks */
enum {
	HCI_QUIRK_RESET_ON_INIT
};

/* HCI device flags */
enum {
	HCI_UP,
	HCI_INIT,
	HCI_RUNNING,

	HCI_PSCAN,
	HCI_ISCAN,
	HCI_AUTH,
	HCI_ENCRYPT,
	HCI_INQUIRY,

	HCI_RAW
};

/* HCI ioctl defines */
#define HCIDEVUP        _IOW('H', 201, int)
#define HCIDEVDOWN      _IOW('H', 202, int)
#define HCIDEVRESET     _IOW('H', 203, int)
#define HCIDEVRESTAT    _IOW('H', 204, int)

#define HCIGETDEVLIST   _IOR('H', 210, int)
#define HCIGETDEVINFO   _IOR('H', 211, int)
#define HCIGETCONNLIST  _IOR('H', 212, int)
#define HCIGETCONNINFO  _IOR('H', 213, int)

#define HCISETRAW       _IOW('H', 220, int)
#define HCISETSCAN      _IOW('H', 221, int)
#define HCISETAUTH      _IOW('H', 222, int)
#define HCISETENCRYPT   _IOW('H', 223, int)
#define HCISETPTYPE     _IOW('H', 224, int)
#define HCISETLINKPOL   _IOW('H', 225, int)
#define HCISETLINKMODE  _IOW('H', 226, int)
#define HCISETACLMTU    _IOW('H', 227, int)
#define HCISETSCOMTU    _IOW('H', 228, int)

#define HCIINQUIRY      _IOR('H', 240, int)

/* HCI timeouts */
#define HCI_CONN_TIMEOUT 	(HZ * 40)
#define HCI_DISCONN_TIMEOUT 	(HZ * 2)
#define HCI_CONN_IDLE_TIMEOUT	(HZ * 60)

/* HCI Packet types */
#define HCI_COMMAND_PKT		0x01
#define HCI_ACLDATA_PKT 	0x02
#define HCI_SCODATA_PKT 	0x03
#define HCI_EVENT_PKT		0x04
#define HCI_UNKNOWN_PKT		0xff

/* HCI Packet types */
#define HCI_DM1 	0x0008
#define HCI_DM3 	0x0400
#define HCI_DM5 	0x4000
#define HCI_DH1 	0x0010
#define HCI_DH3 	0x0800
#define HCI_DH5 	0x8000

#define HCI_HV1		0x0020
#define HCI_HV2		0x0040
#define HCI_HV3		0x0080

#define SCO_PTYPE_MASK	(HCI_HV1 | HCI_HV2 | HCI_HV3)
#define ACL_PTYPE_MASK	(~SCO_PTYPE_MASK)

/* ACL flags */
#define ACL_CONT		0x01
#define ACL_START		0x02
#define ACL_ACTIVE_BCAST	0x04
#define ACL_PICO_BCAST		0x08

/* Baseband links */
#define SCO_LINK	0x00
#define ACL_LINK	0x01

/* LMP features */
#define LMP_3SLOT	0x01
#define LMP_5SLOT	0x02
#define LMP_ENCRYPT	0x04
#define LMP_SOFFSET	0x08
#define LMP_TACCURACY	0x10
#define LMP_RSWITCH	0x20
#define LMP_HOLD	0x40
#define LMP_SNIF	0x80

#define LMP_PARK	0x01
#define LMP_RSSI	0x02
#define LMP_QUALITY	0x04
#define LMP_SCO		0x08
#define LMP_HV2		0x10
#define LMP_HV3		0x20
#define LMP_ULAW	0x40
#define LMP_ALAW	0x80

#define LMP_CVSD	0x01
#define LMP_PSCHEME	0x02
#define LMP_PCONTROL	0x04

/* Link policies */
#define HCI_LP_RSWITCH	0x0001
#define HCI_LP_HOLD	0x0002
#define HCI_LP_SNIFF	0x0004
#define HCI_LP_PARK	0x0008

/* Link mode */
#define HCI_LM_ACCEPT	0x8000
#define HCI_LM_MASTER	0x0001
#define HCI_LM_AUTH	0x0002
#define HCI_LM_ENCRYPT	0x0004
#define HCI_LM_TRUSTED	0x0008
#define HCI_LM_RELIABLE	0x0010

/* -----  HCI Commands ----- */
/* OGF & OCF values */

/* Informational Parameters */
#define OGF_INFO_PARAM	0x04

#define OCF_READ_LOCAL_VERSION	0x0001
typedef __packed struct {
	cyg_uint8  status;
	cyg_uint8  hci_ver;
	cyg_uint16 hci_rev;
	cyg_uint8  lmp_ver;
	cyg_uint16 manufacturer;
	cyg_uint16 lmp_subver;
} read_local_version_rp;//__attribute__ ((packed)) read_local_version_rp;
#define READ_LOCAL_VERSION_RP_SIZE 9

#define OCF_READ_LOCAL_FEATURES	0x0003
typedef __packed struct {
	cyg_uint8 status;
	cyg_uint8 features[8];
} read_local_features_rp;//__attribute__ ((packed)) read_local_features_rp;

#define OCF_READ_BUFFER_SIZE	0x0005
typedef __packed struct {
	cyg_uint8 	status;
	cyg_uint16 	acl_mtu;
	cyg_uint8 	sco_mtu;
	cyg_uint16 	acl_max_pkt;
	cyg_uint16	sco_max_pkt;
}read_buffer_size_rp;// __attribute__ ((packed)) read_buffer_size_rp;

#define OCF_READ_BD_ADDR	0x0009
typedef __packed struct {
	cyg_uint8 status;
	bdaddr_t bdaddr;
} read_bd_addr_rp;//__attribute__ ((packed)) read_bd_addr_rp;

/* Host Controller and Baseband */
#define OGF_HOST_CTL	0x03
#define OCF_RESET		0x0003
#define OCF_READ_AUTH_ENABLE	0x001F
#define OCF_WRITE_AUTH_ENABLE	0x0020
	#define AUTH_DISABLED		0x00
	#define AUTH_ENABLED		0x01

#define OCF_READ_ENCRYPT_MODE	0x0021
#define OCF_WRITE_ENCRYPT_MODE	0x0022
	#define ENCRYPT_DISABLED	0x00
	#define ENCRYPT_P2P		0x01
	#define ENCRYPT_BOTH		0x02

#define OCF_WRITE_CA_TIMEOUT  	0x0016	
#define OCF_WRITE_PG_TIMEOUT  	0x0018

#define OCF_WRITE_SCAN_ENABLE 	0x001A
	#define SCAN_DISABLED		0x00
	#define SCAN_INQUIRY		0x01
	#define SCAN_PAGE		0x02

#define OCF_SET_EVENT_FLT	0x0005
typedef __packed struct {
	cyg_uint8 	flt_type;
	cyg_uint8 	cond_type;
	cyg_uint8 	condition[1];//clyu 0-->1
} set_event_flt_cp;//__attribute__ ((packed)) set_event_flt_cp;
#define SET_EVENT_FLT_CP_SIZE 2

/* Filter types */
#define FLT_CLEAR_ALL	0x00
#define FLT_INQ_RESULT	0x01
#define FLT_CONN_SETUP	0x02

/* CONN_SETUP Condition types */
#define CONN_SETUP_ALLOW_ALL	0x00
#define CONN_SETUP_ALLOW_CLASS	0x01
#define CONN_SETUP_ALLOW_BDADDR	0x02

/* CONN_SETUP Conditions */
#define CONN_SETUP_AUTO_OFF	0x01
#define CONN_SETUP_AUTO_ON	0x02

#define OCF_CHANGE_LOCAL_NAME	0x0013
typedef __packed struct {
	cyg_uint8 	name[248];
}change_local_name_cp ;//__attribute__ ((packed)) change_local_name_cp;
#define CHANGE_LOCAL_NAME_CP_SIZE 248 

#define OCF_READ_LOCAL_NAME	0x0014
typedef __packed struct {
	cyg_uint8	status;
	cyg_uint8 	name[248];
} read_local_name_rp;//__attribute__ ((packed)) read_local_name_rp;
#define READ_LOCAL_NAME_RP_SIZE 249 

#define OCF_READ_CLASS_OF_DEV	0x0023
typedef __packed struct {
	cyg_uint8	status;
	cyg_uint8 	dev_class[3];
} read_class_of_dev_rp;//__attribute__ ((packed)) read_class_of_dev_rp;
#define READ_CLASS_OF_DEV_RP_SIZE 4 

#define OCF_WRITE_CLASS_OF_DEV	0x0024
typedef __packed struct {
	cyg_uint8 	dev_class[3];
} write_class_of_dev_cp;//__attribute__ ((packed)) write_class_of_dev_cp;
#define WRITE_CLASS_OF_DEV_CP_SIZE 3

#define OCF_HOST_BUFFER_SIZE	0x0033
typedef __packed struct {
	cyg_uint16 	acl_mtu;
	cyg_uint8 	sco_mtu;
	cyg_uint16 	acl_max_pkt;
	cyg_uint16	sco_max_pkt;
} host_buffer_size_cp;//__attribute__ ((packed)) host_buffer_size_cp;
#define HOST_BUFFER_SIZE_CP_SIZE 7

/* Link Control */
#define OGF_LINK_CTL	0x01 
#define OCF_CREATE_CONN		0x0005
typedef __packed struct {
	bdaddr_t bdaddr;
	cyg_uint16 	pkt_type;
	cyg_uint8 	pscan_rep_mode;
	cyg_uint8 	pscan_mode;
	cyg_uint16 	clock_offset;
	cyg_uint8 	role_switch;
} create_conn_cp;//__attribute__ ((packed)) create_conn_cp;
#define CREATE_CONN_CP_SIZE 13

#define OCF_ACCEPT_CONN_REQ	0x0009
typedef __packed struct {
	bdaddr_t bdaddr;
	cyg_uint8 	role;
} accept_conn_req_cp;//__attribute__ ((packed)) accept_conn_req_cp;
#define ACCEPT_CONN_REQ_CP_SIZE	7

#define OCF_REJECT_CONN_REQ	0x000a
typedef __packed struct {
	bdaddr_t bdaddr;
	cyg_uint8 	reason;
} reject_conn_req_cp;//__attribute__ ((packed)) reject_conn_req_cp;
#define REJECT_CONN_REQ_CP_SIZE	7

#define OCF_DISCONNECT	0x0006
typedef __packed struct {
	cyg_uint16 	handle;
	cyg_uint8 	reason;
} disconnect_cp;//__attribute__ ((packed)) disconnect_cp;
#define DISCONNECT_CP_SIZE 3

#define OCF_ADD_SCO	0x0007
typedef __packed struct {
	cyg_uint16 	handle;
	cyg_uint16 	pkt_type;
} add_sco_cp;//__attribute__ ((packed)) add_sco_cp;
#define ADD_SCO_CP_SIZE 4

#define OCF_INQUIRY		0x0001
typedef __packed struct {
	cyg_uint8 	lap[3];
	cyg_uint8 	length;
	cyg_uint8	num_rsp;
} inquiry_cp;//__attribute__ ((packed)) inquiry_cp;
#define INQUIRY_CP_SIZE 5

typedef __packed struct {
	cyg_uint8     status;
	bdaddr_t bdaddr;
} __attribute__ ((packed)) status_bdaddr_rp;
#define STATUS_BDADDR_RP_SIZE 7

#define OCF_INQUIRY_CANCEL	0x0002

#define OCF_LINK_KEY_REPLY	0x000B
#define OCF_LINK_KEY_NEG_REPLY	0x000C
typedef __packed struct {
	bdaddr_t bdaddr;
	cyg_uint8     link_key[16];
} __attribute__ ((packed)) link_key_reply_cp;
#define LINK_KEY_REPLY_CP_SIZE 22

#define OCF_PIN_CODE_REPLY	0x000D
#define OCF_PIN_CODE_NEG_REPLY	0x000E
typedef __packed struct {
	bdaddr_t bdaddr;
	cyg_uint8	 pin_len;
	cyg_uint8	 pin_code[16];
} __attribute__ ((packed)) pin_code_reply_cp;
#define PIN_CODE_REPLY_CP_SIZE 23

#define OCF_CHANGE_CONN_PTYPE	0x000F
typedef __packed struct {
	cyg_uint16	 handle;
	cyg_uint16	 pkt_type;
} __attribute__ ((packed)) change_conn_ptype_cp;
#define CHANGE_CONN_PTYPE_CP_SIZE 4

#define OCF_AUTH_REQUESTED	0x0011
typedef __packed struct {
	cyg_uint16	 handle;
} __attribute__ ((packed)) auth_requested_cp;
#define AUTH_REQUESTED_CP_SIZE 2

#define OCF_SET_CONN_ENCRYPT	0x0013
typedef __packed struct {
	cyg_uint16	 handle;
	cyg_uint8	 encrypt;
} __attribute__ ((packed)) set_conn_encrypt_cp;
#define SET_CONN_ENCRYPT_CP_SIZE 3

#define OCF_REMOTE_NAME_REQ	0x0019
typedef __packed struct {
	bdaddr_t bdaddr;
	cyg_uint8     pscan_rep_mode;
	cyg_uint8     pscan_mode;
	cyg_uint16    clock_offset;
} __attribute__ ((packed)) remote_name_req_cp;
#define REMOTE_NAME_REQ_CP_SIZE 10

#define OCF_READ_REMOTE_FEATURES 0x001B
typedef __packed struct {
	cyg_uint16   handle;
} __attribute__ ((packed)) read_remote_features_cp;
#define READ_REMOTE_FEATURES_CP_SIZE 2

#define OCF_READ_REMOTE_VERSION 0x001D
typedef __packed struct {
	cyg_uint16   handle;
} __attribute__ ((packed)) read_remote_version_cp;
#define READ_REMOTE_VERSION_CP_SIZE 2

/* Link Policy */
#define OGF_LINK_POLICY	 0x02   
#define OCF_ROLE_DISCOVERY	0x0009
typedef __packed struct {
	cyg_uint16	handle;
} __attribute__ ((packed)) role_discovery_cp;
#define ROLE_DISCOVERY_CP_SIZE 2
typedef __packed struct {
	cyg_uint8    status;
	cyg_uint16	handle;
	cyg_uint8    role;
} __attribute__ ((packed)) role_discovery_rp;
#define ROLE_DISCOVERY_RP_SIZE 4

#define OCF_READ_LINK_POLICY	0x000C
typedef __packed struct {
	cyg_uint16	handle;
} __attribute__ ((packed)) read_link_policy_cp;
#define READ_LINK_POLICY_CP_SIZE 2
typedef __packed struct {
	cyg_uint8    status;
	cyg_uint16	handle;
	cyg_uint16   policy;
} __attribute__ ((packed)) read_link_policy_rp;
#define READ_LINK_POLICY_RP_SIZE 5

#define OCF_SWITCH_ROLE	0x000B
typedef __packed struct {
	bdaddr_t bdaddr;
	cyg_uint8     role;
} __attribute__ ((packed)) switch_role_cp;
#define SWITCH_ROLE_CP_SIZE 7

#define OCF_WRITE_LINK_POLICY	0x000D
typedef __packed struct {
	cyg_uint16	handle;
	cyg_uint16   policy;
} __attribute__ ((packed)) write_link_policy_cp;
#define WRITE_LINK_POLICY_CP_SIZE 4
typedef __packed struct {
	cyg_uint8    status;
	cyg_uint16	handle;
} __attribute__ ((packed)) write_link_policy_rp;
#define WRITE_LINK_POLICY_RP_SIZE 3

/* Status params */
#define OGF_STATUS_PARAM 	0x05

/* Testing commands */
#define OGF_TESTING_CMD 	0x3e

/* Vendor specific commands */
#define OGF_VENDOR_CMD  	0x3f

/* ---- HCI Events ---- */
#define EVT_INQUIRY_COMPLETE 	0x01

#define EVT_INQUIRY_RESULT 	0x02
typedef __packed struct {
	bdaddr_t	bdaddr;
	cyg_uint8	pscan_rep_mode;
	cyg_uint8	pscan_period_mode;
	cyg_uint8	pscan_mode;
	cyg_uint8	dev_class[3];
	cyg_uint16	clock_offset;
} __attribute__ ((packed)) inquiry_info;
#define INQUIRY_INFO_SIZE 14

#define EVT_INQUIRY_RESULT_WITH_RSSI	0x22
typedef __packed struct {
	bdaddr_t	bdaddr;
	cyg_uint8	pscan_rep_mode;
	cyg_uint8	pscan_period_mode;
	cyg_uint8	dev_class[3];
	cyg_uint16	clock_offset;
	cyg_uint8	rssi;
} __attribute__ ((packed)) inquiry_info_with_rssi;
#define INQUIRY_INFO_WITH_RSSI_SIZE 14

#define EVT_CONN_COMPLETE 	0x03
typedef __packed struct {
	cyg_uint8	status;
	cyg_uint16	handle;
	bdaddr_t	bdaddr;
	cyg_uint8	link_type;
	cyg_uint8	encr_mode;
} __attribute__ ((packed)) evt_conn_complete;
#define EVT_CONN_COMPLETE_SIZE 13

#define EVT_CONN_REQUEST	0x04
typedef __packed struct {
	bdaddr_t 	bdaddr;
	cyg_uint8 		dev_class[3];
	cyg_uint8		link_type;
} __attribute__ ((packed)) evt_conn_request;
#define EVT_CONN_REQUEST_SIZE 10

#define EVT_DISCONN_COMPLETE	0x05
typedef __packed struct {
	cyg_uint8 	status;
	cyg_uint16 	handle;
	cyg_uint8 	reason;
} __attribute__ ((packed)) evt_disconn_complete;
#define EVT_DISCONN_COMPLETE_SIZE 4

#define EVT_AUTH_COMPLETE	0x06
typedef __packed struct {
	cyg_uint8 	status;
	cyg_uint16 	handle;
} __attribute__ ((packed)) evt_auth_complete;
#define EVT_AUTH_COMPLETE_SIZE 3

#define EVT_REMOTE_NAME_REQ_COMPLETE	0x07
typedef __packed struct {
	cyg_uint8 	 status;
	bdaddr_t bdaddr;
	cyg_uint8 	 name[248];
} __attribute__ ((packed)) evt_remote_name_req_complete;
#define EVT_REMOTE_NAME_REQ_COMPLETE_SIZE 255

#define EVT_ENCRYPT_CHANGE	0x08
typedef __packed struct {
	cyg_uint8 	status;
	cyg_uint16 	handle;
	cyg_uint8	encrypt;
} __attribute__ ((packed)) evt_encrypt_change;
#define EVT_ENCRYPT_CHANGE_SIZE 5

#define EVT_QOS_SETUP_COMPLETE 0x0D
typedef __packed struct {
	cyg_uint8	service_type;
	cyg_uint32	token_rate;
	cyg_uint32	peak_bandwidth;
	cyg_uint32	latency;
	cyg_uint32	delay_variation;
} __attribute__ ((packed)) hci_qos;
typedef __packed struct {
	cyg_uint8	status;
	cyg_uint16	handle;
	hci_qos qos;
} __attribute__ ((packed)) evt_qos_setup_complete;
#define EVT_QOS_SETUP_COMPLETE_SIZE 20

#define EVT_CMD_COMPLETE 	0x0e
typedef __packed struct {
	cyg_uint8 	ncmd;
	cyg_uint16 	opcode;
} __attribute__ ((packed)) evt_cmd_complete;
#define EVT_CMD_COMPLETE_SIZE 3

#define EVT_CMD_STATUS 		0x0f
typedef __packed struct {
	cyg_uint8 	status;
	cyg_uint8 	ncmd;
	cyg_uint16 	opcode;
} __attribute__ ((packed)) evt_cmd_status;
#define EVT_CMD_STATUS_SIZE 4

#define EVT_NUM_COMP_PKTS	0x13
typedef __packed struct {
	cyg_uint8 	num_hndl;
	/* variable length part */
} __attribute__ ((packed)) evt_num_comp_pkts;
#define EVT_NUM_COMP_PKTS_SIZE 1

#define EVT_ROLE_CHANGE		0x12
typedef __packed struct {
	cyg_uint8 	 status;
	bdaddr_t bdaddr;
	cyg_uint8     role;
} __attribute__ ((packed)) evt_role_change;
#define EVT_ROLE_CHANGE_SIZE 8

#define EVT_PIN_CODE_REQ        0x16
typedef __packed struct {
	bdaddr_t bdaddr;
} __attribute__ ((packed)) evt_pin_code_req;
#define EVT_PIN_CODE_REQ_SIZE 6

#define EVT_LINK_KEY_REQ        0x17
typedef __packed struct {
	bdaddr_t bdaddr;
} __attribute__ ((packed)) evt_link_key_req;
#define EVT_LINK_KEY_REQ_SIZE 6

#define EVT_LINK_KEY_NOTIFY	0x18
typedef __packed struct {
	bdaddr_t bdaddr;
	cyg_uint8	 link_key[16];
	cyg_uint8	 key_type;
} __attribute__ ((packed)) evt_link_key_notify;
#define EVT_LINK_KEY_NOTIFY_SIZE 23

#define EVT_READ_REMOTE_FEATURES_COMPLETE 0x0B
typedef __packed struct {
	cyg_uint8    status;
	cyg_uint16   handle;
	cyg_uint8    features[8];
} __attribute__ ((packed)) evt_read_remote_features_complete;
#define EVT_READ_REMOTE_FEATURES_COMPLETE_SIZE 11

#define EVT_READ_REMOTE_VERSION_COMPLETE 0x0C
typedef __packed struct {
	cyg_uint8    status;
	cyg_uint16   handle;
	cyg_uint8    lmp_ver;
	cyg_uint16   manufacturer;
	cyg_uint16   lmp_subver;
} __attribute__ ((packed)) evt_read_remote_version_complete;
#define EVT_READ_REMOTE_VERSION_COMPLETE_SIZE 8

/* Internal events generated by BlueZ stack */
#define EVT_STACK_INTERNAL	0xfd
typedef __packed struct {
	cyg_uint16   type;
	cyg_uint8 	data[1];//clyu 0-->1
} __attribute__ ((packed)) evt_stack_internal;
#define EVT_STACK_INTERNAL_SIZE 2

#define EVT_SI_DEVICE  	0x01
typedef __packed struct {
	cyg_uint16   event;
	cyg_uint16 	dev_id;
} __attribute__ ((packed)) evt_si_device;
#define EVT_SI_DEVICE_SIZE 4

#define EVT_SI_SECURITY	0x02
typedef __packed struct {
	cyg_uint16 	event;
	cyg_uint16   proto;
	cyg_uint16   subproto;
	cyg_uint8    incomming;
} __attribute__ ((packed)) evt_si_security;

/* --------  HCI Packet structures  -------- */
#define HCI_TYPE_LEN	1

typedef __packed struct {
	cyg_uint16 	opcode;		/* OCF & OGF */
	cyg_uint8 	plen;
} __attribute__ ((packed))	hci_command_hdr;
#define HCI_COMMAND_HDR_SIZE 	3

typedef __packed struct {
	cyg_uint8 	evt;
	cyg_uint8 	plen;
} __attribute__ ((packed))	hci_event_hdr;
#define HCI_EVENT_HDR_SIZE 	2

typedef __packed struct {
	cyg_uint16 	handle;		/* Handle & Flags(PB, BC) */
	cyg_uint16 	dlen;
} __attribute__ ((packed))	hci_acl_hdr;
#define HCI_ACL_HDR_SIZE 	4

typedef __packed struct {
	cyg_uint16 	handle;
	cyg_uint8 	dlen;
} __attribute__ ((packed))	hci_sco_hdr;
#define HCI_SCO_HDR_SIZE 	3

/* Command opcode pack/unpack */
#define cmd_opcode_pack(ogf, ocf)	(cyg_uint16)((ocf & 0x03ff)|(ogf << 10))
#define cmd_opcode_ogf(op)		(op >> 10)
#define cmd_opcode_ocf(op)		(op & 0x03ff)

/* ACL handle and flags pack/unpack */
#define acl_handle_pack(h, f)	(cyg_uint16)((h & 0x0fff)|(f << 12))
#define acl_handle(h)		(h & 0x0fff)
#define acl_flags(h)		(h >> 12)

/* HCI Socket options */
#define HCI_DATA_DIR	1
#define HCI_FILTER	2
#define HCI_TIME_STAMP	3

/* HCI CMSG flags */
#define HCI_CMSG_DIR	0x0001
#define HCI_CMSG_TSTAMP	0x0002

struct sockaddr_hci {
	u_int8_t       hci_len;
	sa_family_t    hci_family;
	unsigned short hci_dev;
};
#define HCI_DEV_NONE	0xffff

struct hci_filter {
	cyg_uint32 type_mask;
	cyg_uint32 event_mask[2];
	cyg_uint16 opcode;
};

#define HCI_FLT_TYPE_BITS	31
#define HCI_FLT_EVENT_BITS	63
#define HCI_FLT_OGF_BITS	63
#define HCI_FLT_OCF_BITS	127

#if BITS_PER_LONG == 64
static inline void hci_set_bit(int nr, void *addr)
{
	*((cyg_uint32 *) addr + (nr >> 5)) |= ((cyg_uint32) 1 << (nr & 31));
}
static inline int hci_test_bit(int nr, void *addr)
{
	return *((cyg_uint32 *) addr + (nr >> 5)) & ((cyg_uint32) 1 << (nr & 31));
}
#else
#define hci_set_bit	set_bit
#define hci_test_bit	test_bit
#endif

/* Ioctl requests structures */
struct hci_dev_stats {
	cyg_uint32 err_rx;
	cyg_uint32 err_tx;
	cyg_uint32 cmd_tx;
	cyg_uint32 evt_rx;
	cyg_uint32 acl_tx;
	cyg_uint32 acl_rx;
	cyg_uint32 sco_tx;
	cyg_uint32 sco_rx;
	cyg_uint32 byte_rx;
	cyg_uint32 byte_tx;
};

struct hci_dev_info {
	cyg_uint16 dev_id;
	char  name[8];

	bdaddr_t bdaddr;

	cyg_uint32 flags;
	cyg_uint8  type;

	cyg_uint8  features[8];

	cyg_uint32 pkt_type;
	cyg_uint32 link_policy;
	cyg_uint32 link_mode;

	cyg_uint16 acl_mtu;
	cyg_uint16 acl_pkts;
	cyg_uint16 sco_mtu;
	cyg_uint16 sco_pkts;

	struct hci_dev_stats stat;
};

struct hci_conn_info {
	cyg_uint16    handle;
	bdaddr_t bdaddr;
	cyg_uint8	 type;
	cyg_uint8	 out;
	cyg_uint16	 state;
	cyg_uint32	 link_mode;
};

struct hci_dev_req {
	cyg_uint16 dev_id;
	cyg_uint32 dev_opt;
};

struct hci_dev_list_req {
	cyg_uint16  dev_num;
	struct hci_dev_req dev_req[1];	//clyu 0-->1/* hci_dev_req structures */
};

struct hci_conn_list_req {
	cyg_uint16  dev_id;
	cyg_uint16  conn_num;
	struct hci_conn_info conn_info[1];//clyu 0-->1
};

struct hci_conn_info_req {
	bdaddr_t bdaddr;
	cyg_uint8     type;
	struct   hci_conn_info conn_info[1];//clyu 0-->1
};

struct hci_inquiry_req {
	cyg_uint16 dev_id;
	cyg_uint16 flags;
	cyg_uint8  lap[3];
	cyg_uint8  length;
	cyg_uint8  num_rsp;
};
#define IREQ_CACHE_FLUSH 0x0001

struct hci_remotename_req {
	cyg_uint16 dev_id;
	cyg_uint16 flags;
	bdaddr_t bdaddr;
	cyg_uint8  name[248];
};

#endif /* __HCI_H */

/** @file wlan_defs.h
 *  @brief This header file contains global constant/enum definitions,
 *  global variable declaration.
 *
 *  Copyright © Marvell International Ltd. and/or its affiliates, 2003-2006
 */
/*************************************************************
Change log:
	10/11/05: add Doxygen format comments 
	01/11/06: Change compile flag BULVERDE_SDIO to SD to support
	          Monahans/Zylonite
	01/11/06: Add NELEMENTS, BAND_XX defines
	04/10/06: Add hostcmd generic API and power_adapt_cfg_ext command
************************************************************/

#ifndef _WLAN_DEFS_H_
#define _WLAN_DEFS_H_

#include	"os_defs.h"


#define	REASSOCIATION


/** Double-Word(32Bit) Bit definition */
#define DW_BIT_0	0x00000001
#define DW_BIT_1	0x00000002
#define DW_BIT_2	0x00000004
#define DW_BIT_3	0x00000008
#define DW_BIT_4	0x00000010
#define DW_BIT_5	0x00000020
#define DW_BIT_6	0x00000040
#define DW_BIT_7	0x00000080
#define DW_BIT_8	0x00000100
#define DW_BIT_9	0x00000200
#define DW_BIT_10	0x00000400
#define DW_BIT_11       0x00000800
#define DW_BIT_12       0x00001000
#define DW_BIT_13       0x00002000
#define DW_BIT_14       0x00004000
#define DW_BIT_15       0x00008000
#define DW_BIT_16       0x00010000
#define DW_BIT_17       0x00020000
#define DW_BIT_18       0x00040000
#define DW_BIT_19       0x00080000
#define DW_BIT_20       0x00100000
#define DW_BIT_21       0x00200000
#define DW_BIT_22       0x00400000
#define DW_BIT_23       0x00800000
#define DW_BIT_24       0x01000000
#define DW_BIT_25       0x02000000
#define DW_BIT_26       0x04000000
#define DW_BIT_27       0x08000000
#define DW_BIT_28       0x10000000
#define DW_BIT_29       0x20000000
#define DW_BIT_30	0x40000000
#define DW_BIT_31	0x80000000

/** Word (16bit) Bit Definition*/
#define W_BIT_0		0x0001
#define W_BIT_1		0x0002
#define W_BIT_2		0x0004
#define W_BIT_3		0x0008
#define W_BIT_4		0x0010
#define W_BIT_5		0x0020
#define W_BIT_6		0x0040
#define W_BIT_7		0x0080
#define W_BIT_8		0x0100
#define W_BIT_9		0x0200
#define W_BIT_10	0x0400
#define W_BIT_11	0x0800
#define W_BIT_12	0x1000
#define W_BIT_13	0x2000
#define W_BIT_14	0x4000
#define W_BIT_15	0x8000

/** Byte (8Bit) Bit definition*/
#define B_BIT_0		0x01
#define B_BIT_1		0x02
#define B_BIT_2		0x04
#define B_BIT_3		0x08
#define B_BIT_4		0x10
#define B_BIT_5		0x20
#define B_BIT_6		0x40
#define B_BIT_7		0x80

/** Debug Macro definition*/
#ifdef	DEBUG_LEVEL4
#define	PRINTM_INFO(msg...)	diag_printf(msg)
#ifndef DEBUG_LEVEL3
#define	DEBUG_LEVEL3
#endif
#else
#define	PRINTM_INFO(msg...)
#endif

#ifdef	DEBUG_LEVEL3
#define	PRINTM_WARN(msg...)	diag_printf(msg)
#ifndef DEBUG_LEVEL2
#define	DEBUG_LEVEL2
#endif
#else
#define	PRINTM_WARN(msg...)
#endif

#ifdef	DEBUG_LEVEL2
#define	PRINTM_FATAL(msg...)	diag_printf(msg)
#ifndef DEBUG_LEVEL1
#define	DEBUG_LEVEL1
#endif
#else
#define	PRINTM_FATAL(msg...)
#endif

#ifdef	DEBUG_LEVEL1
#define	PRINTM_MSG(msg...)	diag_printf(msg)
#else
#define	PRINTM_MSG(msg...)
#endif

#define	PRINTM(level,msg...)	PRINTM_##level(msg)

#define ASSERT(cond)						\
do {								\
	if (!(cond))						\
		diag_printf("ASSERT: %s, %s:%i\n",		\
		       __FUNCTION__, __FILE__, __LINE__);	\
} while(0)
#define	ENTER()			//diag_printf("Enter: %s, %s:%i\n", __FUNCTION, __FILE, __LINE)
#define	LEAVE()			//diag_printf("Leave: %s, %s:%i\n", __FUNCTION, __FILE, __LINE)

#if defined(DEBUG_LEVEL4) && defined(__ECOS)
static inline void
HEXDUMP(char *prompt, cyg_uint8 * buf, int len)
{
    int i = 0;

    printk(KERN_DEBUG "%s: ", prompt);
    for (i = 1; i <= len; i++) {
        printk("%02x ", (cyg_uint8) * buf);
        buf++;
    }
    printk("\n");
}
#else
#define HEXDUMP(x,y,z)
#endif


#define WLAN_DEBUG_PRINTF wlan_debug_printf
inline int wlan_debug_printf(const char *fmt, ...)
{
	return 0;
}

#ifndef	TRUE
#define TRUE			1
#endif
#ifndef	FALSE
#define	FALSE			0
#endif

#ifndef MIN
#define MIN(a,b)		((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b)		((a) > (b) ? (a) : (b))
#endif

#ifndef NELEMENTS
#define NELEMENTS(x) (sizeof(x)/sizeof(x[0]))
#endif

/** Buffer Constants */

/*	The size of SQ memory PPA, DPA are 8 DWORDs, that keep the physical
*	addresses of TxPD buffers. Station has only 8 TxPD available, Whereas
*	driver has more local TxPDs. Each TxPD on the host memory is associated 
*	with a Tx control node. The driver maintains 8 RxPD descriptors for 
*	station firmware to store Rx packet information.
*
*	Current version of MAC has a 32x6 multicast address buffer.
*
*	802.11b can have up to  14 channels, the driver keeps the
*	BSSID(MAC address) of each APs or Ad hoc stations it has sensed.
*/

#define MRVDRV_SIZE_OF_PPA		0x00000008
#define MRVDRV_SIZE_OF_DPA		0x00000008
#define MRVDRV_NUM_OF_TxPD		0x00000020
#define MRVDRV_MAX_MULTICAST_LIST_SIZE	32
#define MRVDRV_NUM_OF_CMD_BUFFER        10
#define MRVDRV_SIZE_OF_CMD_BUFFER       (2 * 1024)
#define MRVDRV_MAX_CHANNEL_SIZE		14
#define MRVDRV_MAX_BSSID_LIST		64
#define MRVDRV_TIMER_10S		10000
#define MRVDRV_TIMER_5S			5000
#define MRVDRV_TIMER_1S			1000
#define MRVDRV_SNAP_HEADER_LEN          8
#define MRVDRV_ETH_ADDR_LEN             6
#define MRVDRV_ETH_HEADER_SIZE          14

#define	WLAN_UPLD_SIZE			2312
#define DEV_NAME_LEN			32

//clyu
//#define IW_ESSID_MAX_SIZE		128

#ifndef	ETH_ALEN
#define ETH_ALEN			6
#endif

/** Misc constants */
/* This section defines 802.11 specific contants */
#define SDIO_HEADER_LEN		4

#define MRVDRV_MAX_SSID_LENGTH			32
#define MRVDRV_MAX_BSS_DESCRIPTS		16
#define MRVDRV_MAX_REGION_CODE			6

#define MRVDRV_IGNORE_MULTIPLE_DTIM		0xfffe
#define MRVDRV_MIN_MULTIPLE_DTIM		1
#define MRVDRV_MAX_MULTIPLE_DTIM		1
#define MRVDRV_DEFAULT_MULTIPLE_DTIM		1

#define MRVDRV_DEFAULT_LISTEN_INTERVAL		10
#define MRVDRV_DEFAULT_LOCAL_LISTEN_INTERVAL		0

#define	MRVDRV_CHANNELS_PER_SCAN		4
#define	MRVDRV_MAX_CHANNELS_PER_SCAN		14

#define	MRVDRV_CHANNELS_PER_ACTIVE_SCAN		14
#define MRVDRV_MIN_BEACON_INTERVAL		20
#define MRVDRV_MAX_BEACON_INTERVAL		1000
#define MRVDRV_BEACON_INTERVAL			100

/** TxPD Status */

/*	Station firmware use TxPD status field to report final Tx transmit
*	result, Bit masks are used to present combined situations.
*/

#define MRVDRV_TxPD_POWER_MGMT_NULL_PACKET 0x01
#define MRVDRV_TxPD_POWER_MGMT_LAST_PACKET 0x08

/** Tx control node status */

#define MRVDRV_TX_CTRL_NODE_STATUS_IDLE      0x0000

/* Link spped */
#define MRVDRV_LINK_SPEED_1mbps          10000  /* in unit of 100bps */
#define MRVDRV_LINK_SPEED_11mbps         110000

/** RSSI-related defines */
/*	RSSI constants are used to implement 802.11 RSSI threshold 
*	indication. if the Rx packet signal got too weak for 5 consecutive
*	times, miniport driver (driver) will report this event to wrapper
*/

#define MRVDRV_NF_DEFAULT_SCAN_VALUE		(-96)

/** RTS/FRAG related defines */
#define MRVDRV_RTS_MIN_VALUE		0
#define MRVDRV_RTS_MAX_VALUE		2347
#define MRVDRV_FRAG_MIN_VALUE		256
#define MRVDRV_FRAG_MAX_VALUE		2346

/* Fixed IE size is 8 bytes time stamp + 2 bytes beacon interval +
 * 2 bytes cap */
#define MRVL_FIXED_IE_SIZE      12

/* This is for firmware specific length */
#define EXTRA_LEN	36
#define MRVDRV_MAXIMUM_ETH_PACKET_SIZE	1514

#define MRVDRV_ETH_TX_PACKET_BUFFER_SIZE \
	(MRVDRV_MAXIMUM_ETH_PACKET_SIZE + sizeof(TxPD) + EXTRA_LEN)

#define MRVDRV_ETH_RX_PACKET_BUFFER_SIZE \
	(MRVDRV_MAXIMUM_ETH_PACKET_SIZE + sizeof(RxPD) \
	 + MRVDRV_SNAP_HEADER_LEN + EXTRA_LEN)

#define	CMD_F_HOSTCMD		(1 << 0)

/** WEP list macros & data structures */
#define MRVL_KEY_BUFFER_SIZE_IN_BYTE  16

/* to resolve CISCO AP extension */
#define MRVDRV_SCAN_LIST_VAR_IE_SPACE  	256
#define FW_IS_WPA_ENABLED(_adapter) \
		(_adapter->fwCapInfo & FW_CAPINFO_WPA)

#define FW_CAPINFO_WPA  	(1 << 0)
#define WLAN_802_11_AI_REQFI_CAPABILITIES 	1
#define WLAN_802_11_AI_REQFI_LISTENINTERVAL 	2
#define WLAN_802_11_AI_REQFI_CURRENTAPADDRESS 	4

#define WLAN_802_11_AI_RESFI_CAPABILITIES 	1
#define WLAN_802_11_AI_RESFI_STATUSCODE 	2
#define WLAN_802_11_AI_RESFI_ASSOCIATIONID 	4

#define MRVL_NUM_WEP_KEY		4

/** WPA Key LENGTH*/
/* Support 4 keys per key set */
#define MRVL_NUM_WPA_KEY_PER_SET        4
#define MRVL_MAX_WPA_KEY_LENGTH 	32
#define MRVL_MAX_KEY_WPA_KEY_LENGTH     32

#define WPA_AES_KEY_LEN 		16
#define WPA_TKIP_KEY_LEN 		32

/* A few details needed for WEP (Wireless Equivalent Privacy) */
/* 104 bits */
#define MAX_KEY_SIZE		13
/*40 bits RC4 - WEP*/
#define MIN_KEY_SIZE		5
#define MAX_SIZE_ARRA		64

#define RF_ANTENNA_1		0x1
#define RF_ANTENNA_2		0x2
#define RF_ANTENNA_AUTO		0xFFFF

#define HOSTCMD_SUPPORTED_RATES G_SUPPORTED_RATES

#define	BAND_B			(0x01)
#define	BAND_G			(0x02)
#define ALL_802_11_BANDS	(BAND_B | BAND_G)

#define KEY_INFO_ENABLED	0x01

/* For Wireless Extensions */
#define		OID_MRVL_MFG_COMMAND	1

#define SNR_BEACON		0
#define SNR_RXPD		1
#define NF_BEACON		2
#define NF_RXPD			3

/** MACRO DEFINITIONS */
#define CAL_NF(NF)			((cyg_int32)(-(cyg_int32)(NF)))
#define CAL_RSSI(SNR, NF) 		((cyg_int32)((cyg_int32)(SNR) + CAL_NF(NF)))
#define SCAN_RSSI(RSSI)			(0x100 - ((cyg_uint8)(RSSI)))

#define DEFAULT_BCN_AVG_FACTOR		8
#define DEFAULT_DATA_AVG_FACTOR		8
#define AVG_SCALE			100
#define CAL_AVG_SNR_NF(AVG, SNRNF, N)         \
                        (((AVG) == 0) ? ((cyg_uint16)(SNRNF) * AVG_SCALE) : \
                        ((((int)(AVG) * (N -1)) + ((cyg_uint16)(SNRNF) * \
                        AVG_SCALE))  / N))

#define WLAN_STATUS_SUCCESS			(0)
#define WLAN_STATUS_FAILURE			(-1)
#define WLAN_STATUS_NOT_ACCEPTED                (-2)
#define WLAN_STATUS_RECEIVE_FULL                (-3)

#define B_SUPPORTED_RATES		8
#define G_SUPPORTED_RATES		14

#define	WLAN_SUPPORTED_RATES		14
#define WLAN_MAX_SSID_LENGTH		32

#define	MAX_LEDS			8

#define	MAX_POWER_ADAPT_GROUP		5

/* S_SWAP : To swap 2 cyg_uint8 */
#define S_SWAP(a,b) 	do { \
				cyg_uint8  t = SArr[a]; \
				SArr[a] = SArr[b]; SArr[b] = t; \
			} while(0)

/* SWAP: swap cyg_uint8 */
#define SWAP_U8(a,b)	{cyg_uint8 t; t=a; a=b; b=t;}

/* SWAP: swap cyg_uint8 */
#define SWAP_U16(a,b)	{cyg_uint16 t; t=a; a=b; b=t;}

#define wlan_le16_to_cpu(x) x
#define wlan_le32_to_cpu(x) x
#define wlan_le64_to_cpu(x) x
#define wlan_cpu_to_le16(x) x
#define wlan_cpu_to_le32(x) x
#define wlan_cpu_to_le64(x) x

/** Global Varibale Declaration */
typedef struct _wlan_private wlan_private;
typedef struct _wlan_adapter wlan_adapter;
typedef struct _HostCmd_DS_COMMAND HostCmd_DS_COMMAND;

extern cyg_uint32 DSFreqList[15];
extern const char driver_version[];
extern cyg_uint32 DSFreqList[];
extern cyg_uint16 RegionCodeToIndex[MRVDRV_MAX_REGION_CODE];

extern cyg_uint8 WlanDataRates[WLAN_SUPPORTED_RATES];

extern cyg_uint8 SupportedRates[G_SUPPORTED_RATES];

extern cyg_uint8 AdhocRates_G[G_SUPPORTED_RATES];

extern cyg_uint8 AdhocRates_B[4];
extern wlan_private wlanpriv;

#ifdef MFG_CMD_SUPPORT
/* For the mfg command */
typedef struct PkHeader
{
    cyg_uint16 cmd;
    cyg_uint16 len;
    cyg_uint16 seq;
    cyg_uint16 result;
    cyg_uint32 MfgCmd;
} PkHeader;

#define SIOCCFMFG SIOCDEVPRIVATE
#endif /* MFG_CMD_SUPPORT */

/** ENUM definition*/
/** SNRNF_TYPE */
typedef enum _SNRNF_TYPE
{
    TYPE_BEACON = 0,
    TYPE_RXPD,
    MAX_TYPE_B
} SNRNF_TYPE;

/** SNRNF_DATA*/
typedef enum _SNRNF_DATA
{
    TYPE_NOAVG = 0,
    TYPE_AVG,
    MAX_TYPE_AVG
} SNRNF_DATA;

/** WLAN_802_11_AUTH_ALG*/
typedef enum _WLAN_802_11_AUTH_ALG
{
    AUTH_ALG_OPEN_SYSTEM = 1,
    AUTH_ALG_SHARED_KEY = 2,
    AUTH_ALG_NETWORK_EAP = 8
} WLAN_802_11_AUTH_ALG;

/** WLAN_802_1X_AUTH_ALG */
typedef enum _WLAN_802_1X_AUTH_ALG
{
    WLAN_1X_AUTH_ALG_NONE = 1,
    WLAN_1X_AUTH_ALG_LEAP = 2,
    WLAN_1X_AUTH_ALG_TLS = 4,
    WLAN_1X_AUTH_ALG_TTLS = 8,
    WLAN_1X_AUTH_ALG_MD5 = 16
} WLAN_802_1X_AUTH_ALG;

/** WLAN_802_11_ENCRYPTION_MODE */
typedef enum _WLAN_802_11_ENCRYPTION_MODE
{
    CIPHER_NONE,
    CIPHER_WEP40,
    CIPHER_TKIP,
    CIPHER_CCMP,
    CIPHER_WEP104
} WLAN_802_11_ENCRYPTION_MODE;

/** WLAN_802_11_POWER_MODE */
typedef enum _WLAN_802_11_POWER_MODE
{
    Wlan802_11PowerModeCAM,
    Wlan802_11PowerModeMAX_PSP,
    Wlan802_11PowerModeFast_PSP,

    /*not a real mode, defined as an upper bound */
    Wlan802_11PowerModeMax
} WLAN_802_11_POWER_MODE, *PWLAN_802_11_POWER_MODE;

/** PS_STATE */
typedef enum _PS_STATE
{
    PS_STATE_FULL_POWER,
    PS_STATE_AWAKE,
    PS_STATE_PRE_SLEEP,
    PS_STATE_SLEEP
} PS_STATE;

/** DNLD_STATE */
typedef enum _DNLD_STATE
{
    DNLD_RES_RECEIVED,
    DNLD_DATA_SENT,
    DNLD_CMD_SENT
} DNLD_STATE;

/** WLAN_MEDIA_STATE */
typedef enum _WLAN_MEDIA_STATE
{
    WlanMediaStateConnected,
    WlanMediaStateDisconnected
} WLAN_MEDIA_STATE, *PWLAN_MEDIA_STATE;

/** WLAN_802_11_PRIVACY_FILTER */
typedef enum _WLAN_802_11_PRIVACY_FILTER
{
    Wlan802_11PrivFilterAcceptAll,
    Wlan802_11PrivFilter8021xWEP
} WLAN_802_11_PRIVACY_FILTER, *PWLAN_802_11_PRIVACY_FILTER;

/** mv_ms_type */
typedef enum _mv_ms_type
{
    MVMS_DAT = 0,
    MVMS_CMD = 1,
    MVMS_TXDONE = 2,
    MVMS_EVENT
} mv_ms_type;

/* Set of 8 data rates */
typedef cyg_uint8 WLAN_802_11_RATES[WLAN_SUPPORTED_RATES];
typedef cyg_uint8 WLAN_802_11_MAC_ADDRESS[ETH_ALEN];

/* Hardware status codes */
typedef enum _WLAN_HARDWARE_STATUS
{
    WlanHardwareStatusReady,
    WlanHardwareStatusInitializing,
    WlanHardwareStatusReset,
    WlanHardwareStatusClosing,
    WlanHardwareStatusNotReady
} WLAN_HARDWARE_STATUS, *PWLAN_HARDWARE_STATUS;

/** WLAN_802_11_NETWORK_TYPE */
typedef enum _WLAN_802_11_NETWORK_TYPE
{
    Wlan802_11FH,
    Wlan802_11DS,
    /*defined as upper bound */
    Wlan802_11NetworkTypeMax
} WLAN_802_11_NETWORK_TYPE, *PWLAN_802_11_NETWORK_TYPE;

/** WLAN_802_11_NETWORK_INFRASTRUCTURE */
typedef enum _WLAN_802_11_NETWORK_INFRASTRUCTURE
{
    Wlan802_11IBSS,
    Wlan802_11Infrastructure,
    Wlan802_11AutoUnknown,
    /*defined as upper bound */
    Wlan802_11InfrastructureMax
} WLAN_802_11_NETWORK_INFRASTRUCTURE, *PWLAN_802_11_NETWORK_INFRASTRUCTURE;

/** WLAN_802_11_AUTHENTICATION_MODE */
typedef enum _WLAN_802_11_AUTHENTICATION_MODE
{
    Wlan802_11AuthModeOpen = 0x00,
    Wlan802_11AuthModeShared = 0x01,
    Wlan802_11AuthModeNetworkEAP = 0x80
} WLAN_802_11_AUTHENTICATION_MODE, *PWLAN_802_11_AUTHENTICATION_MODE;

/** WLAN_802_11_WEP_STATUS */
typedef enum _WLAN_802_11_WEP_STATUS
{
    Wlan802_11WEPEnabled,
    Wlan802_11WEPDisabled,
    Wlan802_11WEPKeyAbsent,
    Wlan802_11WEPNotSupported, Wlan802_11Encryption2Enabled,
    Wlan802_11Encryption2KeyAbsent,
    Wlan802_11Encryption3Enabled,
    Wlan802_11Encryption3KeyAbsent
} WLAN_802_11_WEP_STATUS, *PWLAN_802_11_WEP_STATUS,
    WLAN_802_11_ENCRYPTION_STATUS, *PWLAN_802_11_ENCRYPTION_STATUS;

/** SNMP_MIB_INDEX_e */
typedef enum _SNMP_MIB_INDEX_e
{
    DesiredBssType_i = 0,
    OpRateSet_i,
    BcnPeriod_i,
    DtimPeriod_i,
    AssocRspTimeOut_i,
    RtsThresh_i,
    ShortRetryLim_i,
    LongRetryLim_i,
    FragThresh_i,
    Dot11D_i,
    Dot11H_i,
    ManufId_i,
    ProdId_i,
    ManufOui_i,
    ManufName_i,
    ManufProdName_i,
    ManufProdVer_i
} SNMP_MIB_INDEX_e;

/** KEY_TYPE_ID */
typedef enum _KEY_TYPE_ID
{
    KEY_TYPE_ID_WEP = 0,
    KEY_TYPE_ID_TKIP,
    KEY_TYPE_ID_AES
} KEY_TYPE_ID;

/** KEY_INFO_WEP*/
typedef enum _KEY_INFO_WEP
{
    KEY_INFO_WEP_DEFAULT_KEY = 0x01
} KEY_INFO_WEP;

/** KEY_INFO_TKIP */
typedef enum _KEY_INFO_TKIP
{
    KEY_INFO_TKIP_MCAST = 0x01,
    KEY_INFO_TKIP_UNICAST = 0x02,
    KEY_INFO_TKIP_ENABLED = 0x04
} KEY_INFO_TKIP;

/** KEY_INFO_AES*/
typedef enum _KEY_INFO_AES
{
    KEY_INFO_AES_MCAST = 0x01,
    KEY_INFO_AES_UNICAST = 0x02,
    KEY_INFO_AES_ENABLED = 0x04
} KEY_INFO_AES;

/** SNMP_MIB_VALUE_e */
typedef enum _SNMP_MIB_VALUE_e
{
    SNMP_MIB_VALUE_INFRA = 1,
    SNMP_MIB_VALUE_ADHOC
} SNMP_MIB_VALUE_e;

#endif /* _WLAN_DEFS_H_ */

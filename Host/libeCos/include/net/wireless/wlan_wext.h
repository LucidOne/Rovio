/** @file wlan_wext.h
 * @brief This file contains definition for IOCTL call.
 *  
 *  Copyright ?Marvell International Ltd. and/or its affiliates, 2003-2006
 */
/********************************************************
Change log:
	10/11/05: Add Doxygen format comments
	12/19/05: Correct a typo in structure _wlan_ioctl_wmm_tspec
	01/11/06: Change compile flag BULVERDE_SDIO to SD to support Monahans/Zylonite
	01/11/06: Conditionalize new scan/join ioctls
	04/10/06: Add hostcmd generic API
	04/18/06: Remove old Subscrive Event and add new Subscribe Event
		  implementation through generic hostcmd API
	06/08/06: Add definitions of custom events
********************************************************/

#ifndef	_WLAN_WEXT_H_
#define	_WLAN_WEXT_H_

#include "include.h"

#define SUBCMD_OFFSET			4
/** PRIVATE CMD ID */
#define	WLANIOCTL			SIOCIWFIRSTPRIV

#define WLANSETWPAIE			(WLANIOCTL + 0)
#define WLANCISDUMP 			(WLANIOCTL + 1)
#ifdef MFG_CMD_SUPPORT
#define	WLANMANFCMD			(WLANIOCTL + 2)
#endif
#define	WLANREGRDWR			(WLANIOCTL + 3)
#define MAX_EEPROM_DATA     			256
#define	WLANHOSTCMD			(WLANIOCTL + 4)

#define WLANHOSTSLEEPCFG		(WLANIOCTL + 6)

#define WLAN_SETINT_GETINT		(WLANIOCTL + 7)
#define WLANNF					1
#define WLANRSSI				2
#define WLANBGSCAN				4
#define WLANENABLE11D				5
#define WLANADHOCGRATE				6
#define WLANSDIOCLOCK				7
#define WLANWMM_ENABLE				8
#define WLANNULLGEN				10
#define WLAN_SUBCMD_SET_PRESCAN			11
#define WLANADHOCCSET				12

#define WLAN_SETNONE_GETNONE	        (WLANIOCTL + 8)
#define WLANDEAUTH                  		1
#define WLANRADIOON                 		2
#define WLANRADIOOFF                		3
#define WLANREMOVEADHOCAES          		4
#define WLANADHOCSTOP               		5
#define WLANCIPHERTEST              		6
#define WLANCRYPTOTEST				7
#ifdef REASSOCIATION
#define WLANREASSOCIATIONAUTO			8
#define WLANREASSOCIATIONUSER			9
#endif /* REASSOCIATION */
#define WLANWLANIDLEON				10
#define WLANWLANIDLEOFF				11

#define WLANGETLOG                  	(WLANIOCTL + 9)
#define WLAN_SETCONF_GETCONF		(WLANIOCTL + 10)

#define BG_SCAN_CONFIG				1
#define WMM_ACK_POLICY              2
#define WMM_PARA_IE                 3
#define WMM_ACK_POLICY_PRIO         4
#define CAL_DATA_EXT_CONFIG         5
#define WLANGET_CAL_DATA_EXT            0
#define WLANSET_CAL_DATA_EXT            1

#define WLANSCAN_TYPE			(WLANIOCTL + 11)

#define WLAN_SET_GET_2K         (WLANIOCTL + 13)
#define WLAN_SET_USER_SCAN              1
#define WLAN_GET_SCAN_TABLE             2
#define WLAN_SET_MRVL_TLV               3
#define WLAN_GET_ASSOC_RSP              4
#define WLAN_ADDTS_REQ                  5
#define WLAN_DELTS_REQ                  6
#define WLAN_QUEUE_CONFIG               7
#define WLAN_QUEUE_STATS                8

#define WLAN_SETNONE_GETONEINT		(WLANIOCTL + 15)
#define WLANGETREGION				1
#define WLAN_GET_LISTEN_INTERVAL		2
#define WLAN_GET_MULTIPLE_DTIM			3
#define WLAN_GET_TX_RATE			4
#define	WLANGETBCNAVG				5

#define WLAN_SETTENCHAR_GETNONE		(WLANIOCTL + 16)

#define WLAN_SETNONE_GETTENCHAR		(WLANIOCTL + 17)

#define WLAN_SETNONE_GETTWELVE_CHAR (WLANIOCTL + 19)
#define WLAN_SUBCMD_GETRXANTENNA    1
#define WLAN_SUBCMD_GETTXANTENNA    2
#define WLAN_GET_TSF                3

#define WLAN_SETWORDCHAR_GETNONE	(WLANIOCTL + 20)
#define WLANSETADHOCAES				1

#define WLAN_SETNONE_GETWORDCHAR	(WLANIOCTL + 21)
#define WLANGETADHOCAES				1
#define WLANVERSION				2

#define WLAN_SETONEINT_GETONEINT	(WLANIOCTL + 23)
#define WLAN_WMM_QOSINFO			2
#define	WLAN_LISTENINTRVL			3
#define WLAN_FW_WAKEUP_METHOD			4
#define WAKEUP_FW_UNCHANGED			0
#define WAKEUP_FW_THRU_INTERFACE		1
#define WAKEUP_FW_THRU_GPIO			2

#define WLAN_TXCONTROL				5
#define WLAN_NULLPKTINTERVAL			6
#define WLAN_ADHOC_AWAKE_PERIOD			8

#define WLAN_SETONEINT_GETNONE		(WLANIOCTL + 24)
#define WLAN_SUBCMD_SETRXANTENNA		1
#define WLAN_SUBCMD_SETTXANTENNA		2
#define WLANSETAUTHALG				5
#define WLANSETENCRYPTIONMODE			6
#define WLANSETREGION				7
#define WLAN_SET_LISTEN_INTERVAL		8

#define WLAN_SET_MULTIPLE_DTIM			9

#define WLANSETBCNAVG				10
#define WLANSETDATAAVG				11

#define WLAN_SET64CHAR_GET64CHAR	(WLANIOCTL + 25)
#define WLANSLEEPPARAMS 			2
#define	WLAN_BCA_TIMESHARE			3
#define WLANSCAN_MODE				6

#define WLAN_GET_ADHOC_STATUS			9

#define WLAN_SET_GEN_IE                 	10
#define WLAN_GET_GEN_IE                 	11
#define WLAN_REASSOCIATE                	12
#define WLAN_WMM_QUEUE_STATUS               	14

#define WLANEXTSCAN			(WLANIOCTL + 26)
#define WLANDEEPSLEEP			(WLANIOCTL + 27)
#define DEEP_SLEEP_ENABLE			1
#define DEEP_SLEEP_DISABLE  			0

#define WLAN_SET_GET_SIXTEEN_INT       (WLANIOCTL + 29)
#define WLAN_TPCCFG                             1
#define WLAN_POWERCFG                           2

#define WLAN_AUTO_FREQ_SET			3
#define WLAN_AUTO_FREQ_GET			4
#define WLAN_LED_GPIO_CTRL			5
#define WLAN_SCANPROBES 			6
#define WLAN_SLEEP_PERIOD			7
#define	WLAN_ADAPT_RATESET			8
#define	WLAN_INACTIVITY_TIMEOUT			9
#define WLANSNR					10
#define WLAN_GET_RATE				11
#define	WLAN_GET_RXINFO				12
#define	WLAN_SET_ATIM_WINDOW			13
#define WLAN_BEACON_INTERVAL			14

#define WLANCMD52RDWR			(WLANIOCTL + 30)
#define WLANCMD53RDWR			(WLANIOCTL + 31)
#define CMD53BUFLEN				32

#define	REG_MAC					0x19
#define	REG_BBP					0x1a
#define	REG_RF					0x1b
#define	REG_EEPROM				0x59

#define	CMD_DISABLED				0
#define	CMD_ENABLED				1
#define	CMD_GET					2
#define SKIP_CMDNUM				4
#define SKIP_TYPE				1
#define SKIP_SIZE				2
#define SKIP_ACTION				2
#define SKIP_TYPE_SIZE			(SKIP_TYPE + SKIP_SIZE)
#define SKIP_TYPE_ACTION		(SKIP_TYPE + SKIP_ACTION)

/* define custom events */
#define CUS_EVT_HWM_CFG_DONE		"HWM_CFG_DONE.indication "
#define CUS_EVT_RSSI_LOW		"EVENT=RSSI_LOW"
#define CUS_EVT_SNR_LOW			"EVENT=SNR_LOW"
#define CUS_EVT_MAX_FAIL		"EVENT=MAX_FAIL"
#define CUS_EVT_RSSI_HIGH		"EVENT=RSSI_HIGH"
#define CUS_EVT_SNR_HIGH		"EVENT=SNR_HIGH"
#define CUS_EVT_MLME_MIC_ERR_UNI	"MLME-MICHAELMICFAILURE.indication unicast "
#define CUS_EVT_MLME_MIC_ERR_MUL	"MLME-MICHAELMICFAILURE.indication multicast "

//JONO
#define CUS_EVT_HS_ACTIVATED		"HS_ACTIVATED "
#define CUS_EVT_HS_DEACTIVATED		"HS_DEACTIVATED "
#define CUS_EVT_HS_GPIO_INT		"HS_GPIO_INT "
#define CUS_EVT_BEACON_RSSI_LOW		"EVENT=BEACON_RSSI_LOW"
#define CUS_EVT_BEACON_SNR_LOW		"EVENT=BEACON_SNR_LOW"
#define CUS_EVT_BEACON_RSSI_HIGH	"EVENT=BEACON_RSSI_HIGH"
#define CUS_EVT_BEACON_SNR_HIGH		"EVENT=BEACON_SNR_HIGH"
//JONO

/** wlan_ioctl */
typedef struct _wlan_ioctl
{
        /** Command ID */
    cyg_uint16 command;
        /** data length */
    cyg_uint16 len;
        /** data pointer */
    cyg_uint8 *data;
} wlan_ioctl;

/** wlan_ioctl_rfantenna */
typedef struct _wlan_ioctl_rfantenna
{
    cyg_uint16 Action;
    cyg_uint16 AntennaMode;
} wlan_ioctl_rfantenna;

/** wlan_ioctl_regrdwr */
typedef struct _wlan_ioctl_regrdwr
{
        /** Which register to access */
    cyg_uint16 WhichReg;
        /** Read or Write */
    cyg_uint16 Action;
    cyg_uint32 Offset;
    cyg_uint16 NOB;
    cyg_uint32 Value;
} wlan_ioctl_regrdwr;

/** wlan_ioctl_cfregrdwr */
typedef struct _wlan_ioctl_cfregrdwr
{
        /** Read or Write */
    cyg_uint8 Action;
        /** register address */
    cyg_uint16 Offset;
        /** register value */
    cyg_uint16 Value;
} wlan_ioctl_cfregrdwr;

/** wlan_ioctl_rdeeprom */
typedef struct _wlan_ioctl_rdeeprom
{
    cyg_uint16 WhichReg;
    cyg_uint16 Action;
    cyg_uint16 Offset;
    cyg_uint16 NOB;
    cyg_uint8 Value;
} wlan_ioctl_rdeeprom;

/** wlan_ioctl_adhoc_key_info */
typedef struct _wlan_ioctl_adhoc_key_info
{
    cyg_uint16 action;
    cyg_uint8 key[16];
    cyg_uint8 tkiptxmickey[16];
    cyg_uint8 tkiprxmickey[16];
} wlan_ioctl_adhoc_key_info;

#ifdef __ECOS
extern struct iw_handler_def wlan_handler_def;
//struct iw_statistics *wlan_get_wireless_stats(struct net_device *dev);
struct iw_statistics *wlan_get_wireless_stats(struct eth_drv_sc *sc);
//int wlan_do_ioctl(struct net_device *dev, struct ifreq *req, int i);
int wlan_do_ioctl(struct eth_drv_sc *sc, struct ifreq *req, int cmd);
#endif

/** sleep_params */
typedef __packed struct _wlan_ioctl_sleep_params_config
{
    cyg_uint16 Action;
    cyg_uint16 Error;
    cyg_uint16 Offset;
    cyg_uint16 StableTime;
    cyg_uint8 CalControl;
    cyg_uint8 ExtSleepClk;
    cyg_uint16 Reserved;
} wlan_ioctl_sleep_params_config,
    *pwlan_ioctl_sleep_params_config;

/** BCA TIME SHARE */
typedef __packed struct _wlan_ioctl_bca_timeshare_config
{
        /** ACT_GET/ACT_SET */
    cyg_uint16 Action;
        /** Type: WLAN, BT */
    cyg_uint16 TrafficType;
        /** Interval: 20msec - 60000msec */
    cyg_uint32 TimeShareInterval;
        /** PTA arbiter time in msec */
    cyg_uint32 BTTime;
} wlan_ioctl_bca_timeshare_config,
    *pwlan_ioctl_bca_timeshare_config;

typedef __packed struct _wlan_ioctl_reassociation_info
{
    cyg_uint8 CurrentBSSID[6];
    cyg_uint8 DesiredBSSID[6];
    char DesiredSSID[IW_ESSID_MAX_SIZE + 1];
} wlan_ioctl_reassociation_info;

#endif /* _WLAN_WEXT_H_ */

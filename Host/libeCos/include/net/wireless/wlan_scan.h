/** @file wlan_scan.h
 *
 *  @brief Interface for the wlan network scan routines
 *
 *  Driver interface functions and type declarations for the scan module
 *    implemented in wlan_scan.c.
 *
 *  Copyright © Marvell International Ltd. and/or its affiliates, 2003-2006
 *
 *  @sa wlan_scan.c
 */
/*************************************************************
Change Log:
    01/11/06: Initial revision. New scan code, relocate related functions

************************************************************/

#ifndef _WLAN_SCAN_H
#define _WLAN_SCAN_H

/**
 *  @brief Maximum number of channels that can be sent in a setuserscan ioctl
 *
 *  @sa wlan_ioctl_user_scan_cfg
 */
#define WLAN_IOCTL_USER_SCAN_CHAN_MAX  50

//! Infrastructure BSS scan type in wlan_scan_cmd_config
#define WLAN_SCAN_BSS_TYPE_BSS         1

//! Adhoc BSS scan type in wlan_scan_cmd_config
#define WLAN_SCAN_BSS_TYPE_IBSS        2

//! Adhoc or Infrastructure BSS scan type in wlan_scan_cmd_config, no filter
#define WLAN_SCAN_BSS_TYPE_ANY         3

/** @brief Maximum buffer space for beacons retrieved from scan responses
 *    4000 has successfully stored up to 40 beacons
 *    6000 has successfully stored the max scan results (max 64)
 */
#define MAX_SCAN_BEACON_BUFFER         6000

/**
 * @brief Buffer pad space for newly allocated beacons/probe responses
 *
 * Beacons are typically 6 bytes longer than an equivalent probe response.
 *  For each scan response stored, allocate an extra byte pad at the end to
 *  allow easy expansion to store a beacon in the same memory a probe reponse
 *  previously contained
 */
#define SCAN_BEACON_ENTRY_PAD          6

/**
 * @brief Structure used internally in the wlan driver to configure a scan.
 *
 * Sent to the command processing module to configure the firmware
 *   scan command prepared by wlan_cmd_802_11_scan.
 *
 * @sa wlan_scan_networks
 *
 */
typedef struct
{
    /**
     *  @brief BSS Type to be sent in the firmware command
     *
     *  Field can be used to restrict the types of networks returned in the
     *    scan.  Valid settings are:
     *
     *   - WLAN_SCAN_BSS_TYPE_BSS  (infrastructure)
     *   - WLAN_SCAN_BSS_TYPE_IBSS (adhoc)
     *   - WLAN_SCAN_BSS_TYPE_ANY  (unrestricted, adhoc and infrastructure)
     */
    cyg_uint8 bssType;

    /**
     *  @brief Specific BSSID used to filter scan results in the firmware
     */
    cyg_uint8 specificBSSID[MRVDRV_ETH_ADDR_LEN];

    /**
     *  @brief Length of TLVs sent in command starting at tlvBuffer
     */
    int tlvBufferLen;

    /**
     *  @brief SSID TLV(s) and ChanList TLVs to be sent in the firmware command
     *
     *  @sa TLV_TYPE_CHANLIST, MrvlIEtypes_ChanListParamSet_t
     *  @sa TLV_TYPE_SSID, MrvlIEtypes_SsIdParamSet_t
     */
    cyg_uint8 tlvBuffer[1];            //!< SSID TLV(s) and ChanList TLVs are stored here
} wlan_scan_cmd_config;

/**
 *  @brief IOCTL channel sub-structure sent in wlan_ioctl_user_scan_cfg
 *
 *  Multiple instances of this structure are included in the IOCTL command
 *   to configure a instance of a scan on the specific channel.
 */
typedef __packed struct
{
    cyg_uint8 chanNumber;              //!< Channel Number to scan
    cyg_uint8 radioType;               //!< Radio type: 'B/G' Band = 0, 'A' Band = 1
    cyg_uint8 scanType;                //!< Scan type: Active = 0, Passive = 1
    cyg_uint16 scanTime;               //!< Scan duration in milliseconds; if 0 default used
} wlan_ioctl_user_scan_chan;

/**
 *  @brief IOCTL input structure to configure an immediate scan cmd to firmware
 *
 *  Used in the setuserscan (WLAN_SET_USER_SCAN) private ioctl.  Specifies
 *   a number of parameters to be used in general for the scan as well
 *   as a channel list (wlan_ioctl_user_scan_chan) for each scan period
 *   desired.
 *
 *  @sa wlan_set_user_scan_ioctl
 */
typedef __packed struct
{

    /**
     *  @brief Flag set to keep the previous scan table intact
     *
     *  If set, the scan results will accumulate, replacing any previous
     *   matched entries for a BSS with the new scan data
     */
    cyg_uint8 keepPreviousScan;        //!< Do not erase the existing scan results

    /**
     *  @brief BSS Type to be sent in the firmware command
     *
     *  Field can be used to restrict the types of networks returned in the
     *    scan.  Valid settings are:
     *
     *   - WLAN_SCAN_BSS_TYPE_BSS  (infrastructure)
     *   - WLAN_SCAN_BSS_TYPE_IBSS (adhoc)
     *   - WLAN_SCAN_BSS_TYPE_ANY  (unrestricted, adhoc and infrastructure)
     */
    cyg_uint8 bssType;

    /**
     *  @brief Configure the number of probe requests for active chan scans
     */
    cyg_uint8 numProbes;

    /**
     *  @brief BSSID filter sent in the firmware command to limit the results
     */
    cyg_uint8 specificBSSID[MRVDRV_ETH_ADDR_LEN];

    /**
     *  @brief SSID filter sent in the firmware command to limit the results
     */
    char specificSSID[MRVDRV_MAX_SSID_LENGTH + 1];

    /**
     *  @brief Variable number (fixed maximum) of channels to scan up
     */
    wlan_ioctl_user_scan_chan chanList[WLAN_IOCTL_USER_SCAN_CHAN_MAX];
} wlan_ioctl_user_scan_cfg;

/**
 *  @brief Sub-structure passed in wlan_ioctl_get_scan_table_entry for each BSS
 *
 *  Fixed field information returned for the scan response in the IOCTL
 *    response.
 */
typedef __packed struct
{
    cyg_uint8 bssid[6];                //!< BSSID of this network
    cyg_uint8 channel;                 //!< Channel this beacon/probe response was detected
    cyg_uint8 rssi;                    //!< RSSI for the received packet
    cyg_uint64 networkTSF;             //!< TSF value from the firmware at packet reception
} wlan_ioctl_get_scan_table_fixed;

/**
 *  @brief Structure passed in the wlan_ioctl_get_scan_table_info for each
 *         BSS returned in the WLAN_GET_SCAN_RESP IOCTL
 *
 *  @sa wlan_get_scan_table_ioctl
 */
typedef __packed struct
{

    /**
     *  @brief Fixed field length included in the response.
     *
     *  Length value is included so future fixed fields can be added to the
     *   response without breaking backwards compatibility.  Use the length
     *   to find the offset for the bssInfoLength field, not a sizeof() calc.
     */
    cyg_uint32 fixedFieldLength;

    /**
     *  @brief Always present, fixed length data fields for the BSS
     */
    wlan_ioctl_get_scan_table_fixed fixedFields;

    /**
     *  @brief Length of the BSS Information (probe resp or beacon) that
     *         follows starting at bssInfoBuffer
     */
    cyg_uint32 bssInfoLength;

    /**
     *  @brief Probe response or beacon scanned for the BSS.
     *
     *  Field layout:
     *   - TSF              8 octets
     *   - Beacon Interval  2 octets
     *   - Capability Info  2 octets
     *
     *   - IEEE Infomation Elements; variable number & length per 802.11 spec
     */
    cyg_uint8 bssInfoBuffer[1];
} wlan_ioctl_get_scan_table_entry;

/**
 *  @brief WLAN_GET_SCAN_RESP private IOCTL struct to retrieve the scan table
 *
 *  @sa wlan_get_scan_table_ioctl
 */
typedef __packed struct
{

    /**
     *  - Zero based scan entry to start retrieval in command request
     *  - Number of scans entires returned in command response
     */
    cyg_uint32 scanNumber;

     /**
      * Buffer marker for multiple wlan_ioctl_get_scan_table_entry structures.
      *   Each struct is padded to the nearest 32 bit boundary.
      */
    cyg_uint8 scan_table_entry_buffer[1];

} wlan_ioctl_get_scan_table_info;

/**
 *  @brief Structure used to store information for each beacon/probe response
 */
typedef struct
{
    WLAN_802_11_MAC_ADDRESS MacAddress;

    WLAN_802_11_SSID Ssid;

    /* WEP encryption requirement */
    cyg_uint32 Privacy;

    /* receive signal strength in dBm */
    WLAN_802_11_RSSI Rssi;

    cyg_uint32 Channel;

    cyg_uint16 BeaconPeriod;

    cyg_uint32 ATIMWindow;

    WLAN_802_11_NETWORK_TYPE NetworkTypeInUse;
    WLAN_802_11_NETWORK_INFRASTRUCTURE InfrastructureMode;
    WLAN_802_11_RATES SupportedRates;
    WMM_PARAMETER_IE wmmIE;

    int extra_ie;

    cyg_uint8 TimeStamp[8];            //!< TSF value included in the beacon/probe response
    IEEEtypes_PhyParamSet_t PhyParamSet;
    IEEEtypes_SsParamSet_t SsParamSet;
    IEEEtypes_CapInfo_t Cap;
    cyg_uint8 DataRates[WLAN_SUPPORTED_RATES];

    cyg_uint64 networkTSF;             //!< TSF timestamp from the current firmware TSF

    IEEEtypes_CountryInfoFullSet_t CountryInfo;

    WPA_SUPPLICANT wpa_supplicant;
    WPA_SUPPLICANT wpa2_supplicant;

    cyg_uint8 *pBeaconBuf;             //!< Pointer to the returned scan response
    cyg_uint32 beaconBufSize;         //!< Length of the stored scan response
    cyg_uint32 beaconBufSizeMax;      //!< Max allocated size for updated scan response

} BSSDescriptor_t;

extern int SSIDcmp(WLAN_802_11_SSID * ssid1, WLAN_802_11_SSID * ssid2);
extern int FindSSIDInList(wlan_adapter * Adapter,
                          WLAN_802_11_SSID * ssid, cyg_uint8 * bssid, int mode);
extern int FindBestSSIDInList(wlan_adapter * Adapter);
extern int FindBSSIDInList(wlan_adapter * Adapter, cyg_uint8 * bssid, int mode);

extern int FindBestNetworkSsid(wlan_private * priv, WLAN_802_11_SSID * pSSID);

extern int SendSpecificSSIDScan(wlan_private * priv,
                                WLAN_802_11_SSID * pRequestedSSID);
extern int SendSpecificBSSIDScan(wlan_private * priv, cyg_uint8 * bssid);

extern cyg_uint8 wlan_scan_radio_to_band(cyg_uint8 scanBand);

extern int wlan_get_scan_table_ioctl(wlan_private * priv, struct iwreq *wrq);
extern int wlan_set_user_scan_ioctl(wlan_private * priv, struct iwreq *wrq);

extern int wlan_associate(wlan_private * priv, BSSDescriptor_t * pBSSDesc);

extern int wlan_cmd_802_11_scan(wlan_private * priv,
                                HostCmd_DS_COMMAND * cmd, void *pdata_buf);

extern int wlan_ret_802_11_scan(wlan_private * priv,
                                HostCmd_DS_COMMAND * resp);

extern int wlan_extscan_ioctl(wlan_private * priv, struct ifreq *req);

extern int sendBgScanQueryCmd(wlan_private * priv);
extern int wlan_bg_scan_enable(wlan_private * priv, BOOLEAN enable);
extern int wlan_do_bg_scan_config_ioctl(wlan_private * priv,
                                        struct ifreq *req);
extern int wlan_cmd_802_11_bg_scan_config(wlan_private * priv,
                                          HostCmd_DS_COMMAND * cmd,
                                          int cmd_action, void *pdata_buf);
extern int wlan_cmd_802_11_bg_scan_query(wlan_private * priv,
                                         HostCmd_DS_COMMAND * cmd);

#ifdef __ECOS
#if 0
extern int wlan_get_scan(struct net_device *dev, struct iw_request_info *info,
                         struct iw_point *dwrq, char *extra);
extern int wlan_set_scan(struct net_device *dev, struct iw_request_info *info,
                         struct iw_param *vwrq, char *extra);
#else
extern int wlan_get_scan(struct eth_drv_sc *sc, struct iw_request_info *info,
                         struct iw_point *dwrq, char *extra);
extern int wlan_set_scan(struct eth_drv_sc *sc, struct iw_request_info *info,
              struct iw_param *vwrq, char *extra);
#endif
#endif

#endif /* _WLAN_SCAN_H */

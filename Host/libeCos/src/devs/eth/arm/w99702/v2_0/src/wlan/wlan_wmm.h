/** @file wlan_wmm.h
 * @brief This file contains related macros, enum, and struct
 * of wmm functionalities
 *
 *  Copyright © Marvell International Ltd. and/or its affiliates, 2003-2006
 */
/****************************************************
Change log:
    09/26/05: add Doxygen format comments 
    04/06/06: Add TSPEC, queue metrics, and MSDU expiry support
****************************************************/

#ifndef __WLAN_WMM_H
#define __WLAN_WMM_H

#include "wlan_types.h"
#include "cyg/io/eth/eth_drv.h"

/** enum of WMM AC_QUEUES */
#define  MAX_AC_QUEUES 4
typedef __packed enum
{
    AC_PRIO_BK,
    AC_PRIO_BE,
    AC_PRIO_VI,
    AC_PRIO_VO
} wlan_wmm_ac_e;

#define WMM_IE_LENGTH                 0x0009
#define WMM_PARA_IE_LENGTH            0x0018
#define WMM_QOS_INFO_OFFSET           0x08
#define WMM_QOS_INFO_UAPSD_BIT        0x80
#define WMM_OUISUBTYPE_IE             0x00
#define WMM_OUISUBTYPE_PARA           0x01
#define WMM_TXOP_LIMIT_UNITS_SHIFT    5

#define WMM_CONFIG_CHANGE_INDICATION  "WMM_CONFIG_CHANGE.indication"

/** Size of a TSPEC.  Used to allocate necessary buffer space in commands */
#define WMM_TSPEC_SIZE              63

/** Extra IE bytes allocated in messages for appended IEs after a TSPEC */
#define WMM_ADDTS_EXTRA_IE_BYTES    256

/** Extra TLV bytes allocated in messages for configuring WMM Queues */
#define WMM_QUEUE_CONFIG_EXTRA_TLV_BYTES 64

/** wlan_ioctl_wmm_para_ie */
typedef __packed struct
{

    /** type */
    cyg_uint8 Type;

    /** action */
    cyg_uint16 Action;

    /** WMM Parameter IE */
    cyg_uint8 Para_IE[26];

} wlan_ioctl_wmm_para_ie;

/** wlan_ioctl_wmm_ack_policy */
typedef __packed struct
{
    cyg_uint8 Type;

    /** 0-ACT_GET, 1-ACT_SET */
    cyg_uint16 Action;

    /** 0-AC_BE, 1-AC_BK, 2-AC_VI, 3-AC_VO */
    cyg_uint8 AC;

    /** 0-WMM_ACK_POLICY_IMM_ACK, 1-WMM_ACK_POLICY_NO_ACK */
    cyg_uint8 AckPolicy;

} wlan_ioctl_wmm_ack_policy;

/** data structure of WMM QoS information */
typedef __packed struct
{
    cyg_uint8 ParaSetCount:4;
    cyg_uint8 Reserved:3;
    cyg_uint8 QosUAPSD:1;
} WMM_QoS_INFO;

typedef __packed struct
{
    cyg_uint8 AIFSN:4;
    cyg_uint8 ACM:1;
    cyg_uint8 ACI:2;
    cyg_uint8 Reserved:1;
} WMM_ACI_AIFSN;

/**  data structure of WMM ECW */
typedef __packed struct
{
    cyg_uint8 ECW_Min:4;
    cyg_uint8 ECW_Max:4;
} WMM_ECW;

/** data structure of WMM AC parameters  */
typedef __packed struct
{
    WMM_ACI_AIFSN ACI_AIFSN;
    WMM_ECW ECW;
    cyg_uint16 Txop_Limit;
} WMM_AC_PARAS;

/** data structure of WMM Info IE  */
typedef __packed struct
{
    /** 221 */
    cyg_uint8 ElementId;
    /** 7 */
    cyg_uint8 Length;
    /** 00:50:f2 (hex) */
    cyg_uint8 Oui[3];
    /** 2 */
    cyg_uint8 OuiType;
    /** 0 */
    cyg_uint8 OuiSubtype;
    /** 1 */
    cyg_uint8 Version;

    WMM_QoS_INFO QoSInfo;

} WMM_INFO_IE;

/** data structure of WMM parameter IE  */
typedef __packed struct
{
    /** 221 */
    cyg_uint8 ElementId;
    /** 24 */
    cyg_uint8 Length;
    /** 00:50:f2 (hex) */
    cyg_uint8 Oui[3];
    /** 2 */
    cyg_uint8 OuiType;
    /** 1 */
    cyg_uint8 OuiSubtype;
    /** 1 */
    cyg_uint8 Version;

    WMM_QoS_INFO QoSInfo;
    cyg_uint8 Reserved;

    /** AC Parameters Record AC_BE */
    WMM_AC_PARAS AC_Paras_BE;
    /** AC Parameters Record AC_BK */
    WMM_AC_PARAS AC_Paras_BK;
    /** AC Parameters Record AC_VI */
    WMM_AC_PARAS AC_Paras_VI;
    /** AC Parameters Record AC_VO */
    WMM_AC_PARAS AC_Paras_VO;
} WMM_PARAMETER_IE;

/** struct of command of WMM ack policy*/
typedef __packed struct
{
    /** 0 - ACT_GET,
        1 - ACT_SET */
    cyg_uint16 Action;

    /** 0 - AC_BE
        1 - AC_BK
        2 - AC_VI
        3 - AC_VO */
    cyg_uint8 AC;

    /** 0 - WMM_ACK_POLICY_IMM_ACK
        1 - WMM_ACK_POLICY_NO_ACK */
    cyg_uint8 AckPolicy;
} HostCmd_DS_WMM_ACK_POLICY;

/**
 *  @brief Firmware command structure to retrieve the firmware WMM status.
 *
 *  Used to retrieve the status of each WMM AC Queue in TLV 
 *    format (MrvlIEtypes_WmmQueueStatus_t) as well as the current WMM
 *    parameter IE advertised by the AP.  
 *  
 *  Used in response to a MACREG_INT_CODE_WMM_STATUS_CHANGE event signalling
 *    a QOS change on one of the ACs or a change in the WMM Parameter in
 *    the Beacon.
 *
 *  TLV based command, byte arrays used for max sizing purpose. There are no 
 *    arguments sent in the command, the TLVs are returned by the firmware.
 */
typedef __packed struct
{
    cyg_uint8 queueStatusTlv[sizeof(MrvlIEtypes_WmmQueueStatus_t) * MAX_AC_QUEUES];
    cyg_uint8 wmmParamTlv[sizeof(WMM_PARAMETER_IE) + 2];

}
HostCmd_DS_WMM_GET_STATUS;

typedef __packed struct
{
    cyg_uint16 PacketAC;
} HostCmd_DS_WMM_PRIO_PKT_AVAIL;

/**
 *  @brief Enumeration for the command result from an ADDTS or DELTS command 
 */
typedef __packed enum
{
    TSPEC_RESULT_SUCCESS = 0,
    TSPEC_RESULT_EXEC_FAILURE = 1,
    TSPEC_RESULT_TIMEOUT = 2,
    TSPEC_RESULT_DATA_INVALID = 3,

} wlan_wmm_tspec_result_e;

/**
 *  @brief IOCTL structure to send an ADDTS request and retrieve the response.
 *
 *  IOCTL structure from the application layer relayed to firmware to 
 *    instigate an ADDTS management frame with an appropriate TSPEC IE as well
 *    as any additional IEs appended in the ADDTS Action frame.
 *
 *  @sa wlan_wmm_addts_req_ioctl
 */
typedef __packed struct
{
    wlan_wmm_tspec_result_e commandResult;
    cyg_uint32 timeout_ms;

    cyg_uint8 ieeeStatusCode;

    cyg_uint8 tspecData[WMM_TSPEC_SIZE];

    cyg_uint8 addtsExtraIEBuf[WMM_ADDTS_EXTRA_IE_BYTES];

} wlan_ioctl_wmm_addts_req_t;

/**
 *  @brief IOCTL structure to send a DELTS request.
 *
 *  IOCTL structure from the application layer relayed to firmware to 
 *    instigate an DELTS management frame with an appropriate TSPEC IE.
 *
 *  @sa wlan_wmm_delts_req_ioctl
 */
typedef __packed struct
{
    wlan_wmm_tspec_result_e commandResult;      //!< Firmware execution result

    cyg_uint8 ieeeReasonCode;          //!< IEEE reason code sent, unused for WMM 

    cyg_uint8 tspecData[WMM_TSPEC_SIZE];       //!< TSPEC to send in the DELTS

} wlan_ioctl_wmm_delts_req_t;

/**
 *  @brief Internal command structure used in executing an ADDTS command.
 *
 *  Relay information between the IOCTL layer and the firmware command and 
 *    command response procedures.
 *
 *  @sa wlan_wmm_addts_req_ioctl
 *  @sa wlan_cmd_wmm_addts_req
 *  @sa wlan_cmdresp_wmm_addts_req
 */
typedef struct
{
    wlan_wmm_tspec_result_e commandResult;
    cyg_uint32 timeout_ms;

    cyg_uint8 dialogToken;
    cyg_uint8 ieeeStatusCode;

    int tspecDataLen;
    cyg_uint8 tspecData[WMM_TSPEC_SIZE];
    cyg_uint8 addtsExtraIEBuf[WMM_ADDTS_EXTRA_IE_BYTES];

} wlan_cmd_wmm_addts_req_t;

/**
 *  @brief Internal command structure used in executing an DELTS command.
 *
 *  Relay information between the IOCTL layer and the firmware command and 
 *    command response procedures.
 *
 *  @sa wlan_wmm_delts_req_ioctl
 *  @sa wlan_cmd_wmm_delts_req
 *  @sa wlan_cmdresp_wmm_delts_req
 */
typedef struct
{
    wlan_wmm_tspec_result_e commandResult;

    cyg_uint8 dialogToken;

    cyg_uint8 ieeeReasonCode;

    int tspecDataLen;
    cyg_uint8 tspecData[WMM_TSPEC_SIZE];

} wlan_cmd_wmm_delts_req_t;

/**
 *  @brief Command structure for the HostCmd_CMD_WMM_ADDTS_REQ firmware command
 *
 */
typedef __packed struct
{
    wlan_wmm_tspec_result_e commandResult;
    cyg_uint32 timeout_ms;

    cyg_uint8 dialogToken;
    cyg_uint8 ieeeStatusCode;
    cyg_uint8 tspecData[WMM_TSPEC_SIZE];
    cyg_uint8 addtsExtraIEBuf[WMM_ADDTS_EXTRA_IE_BYTES];

} HostCmd_DS_WMM_ADDTS_REQ;

/**
 *  @brief Command structure for the HostCmd_CMD_WMM_DELTS_REQ firmware command
 */
typedef __packed struct
{
    wlan_wmm_tspec_result_e commandResult;
    cyg_uint8 dialogToken;
    cyg_uint8 ieeeReasonCode;
    cyg_uint8 tspecData[WMM_TSPEC_SIZE];

} HostCmd_DS_WMM_DELTS_REQ;

/**
 *  @brief Enumeration for the action field in the Queue configure command
 */
typedef __packed enum
{
    WMM_QUEUE_CONFIG_ACTION_GET = 0,
    WMM_QUEUE_CONFIG_ACTION_SET = 1,
    WMM_QUEUE_CONFIG_ACTION_DEFAULT = 2,

    WMM_QUEUE_CONFIG_ACTION_MAX
} wlan_wmm_queue_config_action_e;

/**
 *  @brief Command structure for the HostCmd_CMD_WMM_QUEUE_CONFIG firmware cmd
 *
 *  Set/Get/Default the Queue parameters for a specific AC in the firmware.
 *
 */
typedef __packed struct
{
    wlan_wmm_queue_config_action_e action;      //!< Set, Get, or Default
    wlan_wmm_ac_e accessCategory;       //!< AC_BK(0) to AC_VO(3)

    /** @brief MSDU lifetime expiry per 802.11e
     *
     *   - Ignored if 0 on a set command 
     *   - Set to the 802.11e specified 500 TUs when defaulted
     */
    cyg_uint16 msduLifetimeExpiry;

    cyg_uint8 tlvBuffer[WMM_QUEUE_CONFIG_EXTRA_TLV_BYTES];     //!< Not supported yet

} HostCmd_DS_WMM_QUEUE_CONFIG;

/**
 *  @brief Internal command structure used in executing a queue config command.
 *
 *  Relay information between the IOCTL layer and the firmware command and 
 *    command response procedures.
 *
 *  @sa wlan_wmm_queue_config_ioctl
 *  @sa wlan_cmd_wmm_queue_config
 *  @sa wlan_cmdresp_wmm_queue_config
 */
typedef struct
{
    wlan_wmm_queue_config_action_e action;      //!< Set, Get, or Default
    wlan_wmm_ac_e accessCategory;       //!< AC_BK(0) to AC_VO(3)
    cyg_uint16 msduLifetimeExpiry;     //!< lifetime expiry in TUs

    int tlvBufLen;              //!< Not supported yet
    cyg_uint8 tlvBuffer[WMM_QUEUE_CONFIG_EXTRA_TLV_BYTES];     //!< Not supported yet

} wlan_cmd_wmm_queue_config_t;

/**
 *  @brief IOCTL structure to configure a specific AC Queue's parameters
 *
 *  IOCTL structure from the application layer relayed to firmware to 
 *    get, set, or default the WMM AC queue parameters.
 *
 *  - msduLifetimeExpiry is ignored if set to 0 on a set command
 *
 *  @sa wlan_wmm_queue_config_ioctl
 */
typedef __packed struct
{
    wlan_wmm_queue_config_action_e action;      //!< Set, Get, or Default
    wlan_wmm_ac_e accessCategory;       //!< AC_BK(0) to AC_VO(3)
    cyg_uint16 msduLifetimeExpiry;     //!< lifetime expiry in TUs

    cyg_uint8 supportedRates[10];      //!< Not supported yet

} wlan_ioctl_wmm_queue_config_t;

/**
 *   @brief Enumeration for the action field in the queue stats command
 */
typedef __packed enum
{
    WMM_STATS_ACTION_START = 0,
    WMM_STATS_ACTION_STOP = 1,
    WMM_STATS_ACTION_GET_CLR = 2,

    WMM_STATS_ACTION_MAX
} wlan_wmm_stats_action_e;

/** Number of bins in the histogram for the HostCmd_DS_WMM_QUEUE_STATS */
#define WMM_STATS_PKTS_HIST_BINS  7

/**
 *  @brief Command structure for the HostCmd_CMD_WMM_QUEUE_STATS firmware cmd
 *
 *  Turn statistical collection on/off for a given AC or retrieve the 
 *    accumulated stats for an AC and clear them in the firmware.
 */
typedef __packed struct
{
    wlan_wmm_stats_action_e action;     //!< Start, Stop, or Get 
    wlan_wmm_ac_e accessCategory;       //!< AC_BK(0) to AC_VO(3)

    cyg_uint16 pktCount;               //!< Number of successful packets transmitted
    cyg_uint16 pktLoss;                //!< Packets lost; not included in pktCount
    cyg_uint32 avgQueueDelay;          //!< Average Queue delay in microseconds
    cyg_uint32 avgTxDelay;             //!< Average Transmission delay in microseconds
    cyg_uint32 usedTime;               //!< Calculated medium time 

    /** @brief Queue Delay Histogram; number of packets per queue delay range
     * 
     *  [0] -  0ms <= delay < 5ms
     *  [1] -  5ms <= delay < 10ms
     *  [2] - 10ms <= delay < 20ms
     *  [3] - 20ms <= delay < 30ms
     *  [4] - 30ms <= delay < 40ms
     *  [5] - 40ms <= delay < 50ms
     *  [6] - 50ms <= delay < msduLifetime (TUs)
     */
    cyg_uint16 delayHistogram[WMM_STATS_PKTS_HIST_BINS];

} HostCmd_DS_WMM_QUEUE_STATS;

/**
 *  @brief IOCTL structure to start, stop, and get statistics for a WMM AC
 *
 *  IOCTL structure from the application layer relayed to firmware to 
 *    start or stop statistical collection for a given AC.  Also used to 
 *    retrieve and clear the collected stats on a given AC.
 *
 *  @sa wlan_wmm_queue_stats_ioctl
 */
typedef __packed struct
{
    wlan_wmm_stats_action_e action;     //!< Start, Stop, or Get 
    wlan_wmm_ac_e accessCategory;       //!< AC_BK(0) to AC_VO(3)
    cyg_uint16 pktCount;               //!< Number of successful packets transmitted  
    cyg_uint16 pktLoss;                //!< Packets lost; not included in pktCount    
    cyg_uint32 avgQueueDelay;          //!< Average Queue delay in microseconds
    cyg_uint32 avgTxDelay;             //!< Average Transmission delay in microseconds
    cyg_uint32 usedTime;               //!< Calculated medium time 

    /** @brief Queue Delay Histogram; number of packets per queue delay range
     * 
     *  [0] -  0ms <= delay < 5ms
     *  [1] -  5ms <= delay < 10ms
     *  [2] - 10ms <= delay < 20ms
     *  [3] - 20ms <= delay < 30ms
     *  [4] - 30ms <= delay < 40ms
     *  [5] - 40ms <= delay < 50ms
     *  [6] - 50ms <= delay < msduLifetime (TUs)
     */
    cyg_uint16 delayHistogram[WMM_STATS_PKTS_HIST_BINS];
} wlan_ioctl_wmm_queue_stats_t;

/** 
 *  @brief IOCTL sub structure for a specific WMM AC Status
 */
typedef __packed struct
{
    cyg_uint8 wmmACM;
    cyg_uint8 flowRequired;
    cyg_uint8 flowCreated;
    cyg_uint8 disabled;
} wlan_ioctl_wmm_queue_status_ac_t;

/**
 *  @brief IOCTL structure to retrieve the WMM AC Queue status
 *
 *  IOCTL structure from the application layer to retrieve:
 *     - ACM bit setting for the AC
 *     - Firmware status (flow required, flow created, flow disabled)
 *
 *  @sa wlan_wmm_queue_status_ioctl
 */
typedef __packed struct
{
    wlan_ioctl_wmm_queue_status_ac_t acStatus[MAX_AC_QUEUES];
} wlan_ioctl_wmm_queue_status_t;

/** Firmware status for a specific AC */
typedef __packed struct
{
    cyg_uint8 Disabled;
    cyg_uint8 FlowRequired;
    cyg_uint8 FlowCreated;
} WMM_AC_STATUS;

#ifdef __ECOS
/** struct of WMM DESC */
typedef __packed struct
{
    cyg_uint8 required;
    cyg_uint8 enabled;
    cyg_uint8 fw_notify;
//    struct sk_buff TxSkbQ[MAX_AC_QUEUES];
    cyg_uint8 Para_IE[WMM_PARA_IE_LENGTH];
    cyg_uint8 qosinfo;
    WMM_AC_STATUS acStatus[MAX_AC_QUEUES];
} WMM_DESC;

//extern void wmm_map_and_add_skb(wlan_private * priv, struct sk_buff *);
void
wmm_map_and_add_skb(wlan_private * priv, struct eth_drv_sg *sg_list,
                               int sg_len,
                               int total_len);
#endif

extern int sendWMMStatusChangeCmd(wlan_private * priv);
extern int wmm_lists_empty(wlan_private * priv);
extern void wmm_cleanup_queues(wlan_private * priv);
extern void wmm_process_tx(wlan_private * priv);

extern cyg_uint32 wlan_wmm_process_association_req(wlan_private * priv,
                                            cyg_uint8 ** ppAssocBuf,
                                            WMM_PARAMETER_IE * pWmmIE);

/* 
 *  Functions used in the cmd handling routine
 */
extern int wlan_cmd_wmm_ack_policy(wlan_private * priv,
                                   HostCmd_DS_COMMAND * cmd, void *InfoBuf);
extern int wlan_cmd_wmm_get_status(wlan_private * priv,
                                   HostCmd_DS_COMMAND * cmd, void *InfoBuf);
extern int wlan_cmd_wmm_addts_req(wlan_private * priv,
                                  HostCmd_DS_COMMAND * cmd, void *InfoBuf);
extern int wlan_cmd_wmm_delts_req(wlan_private * priv,
                                  HostCmd_DS_COMMAND * cmd, void *InfoBuf);
extern int wlan_cmd_wmm_queue_config(wlan_private * priv,
                                     HostCmd_DS_COMMAND * cmd, void *InfoBuf);
extern int wlan_cmd_wmm_queue_stats(wlan_private * priv,
                                    HostCmd_DS_COMMAND * cmd, void *InfoBuf);

/* 
 *  Functions used in the cmdresp handling routine
 */
extern int wlan_cmdresp_wmm_ack_policy(wlan_private * priv,
                                       const HostCmd_DS_COMMAND * resp);
extern int wlan_cmdresp_wmm_get_status(wlan_private * priv,
                                       const HostCmd_DS_COMMAND * resp);
extern int wlan_cmdresp_wmm_addts_req(wlan_private * priv,
                                      const HostCmd_DS_COMMAND * resp);
extern int wlan_cmdresp_wmm_delts_req(wlan_private * priv,
                                      const HostCmd_DS_COMMAND * resp);
extern int wlan_cmdresp_wmm_queue_config(wlan_private * priv,
                                         const HostCmd_DS_COMMAND * resp);
extern int wlan_cmdresp_wmm_queue_stats(wlan_private * priv,
                                        const HostCmd_DS_COMMAND * resp);

/* 
 * IOCTLs 
 */
extern int wlan_wmm_ack_policy_ioctl(wlan_private * priv, struct ifreq *req);
extern int wlan_wmm_para_ie_ioctl(wlan_private * priv, struct ifreq *req);
extern int wlan_wmm_enable_ioctl(wlan_private * priv, struct iwreq *wrq);
extern int wlan_wmm_queue_status_ioctl(wlan_private * priv,
                                       struct iwreq *wrq);

extern int wlan_wmm_addts_req_ioctl(wlan_private * priv, struct iwreq *wrq);
extern int wlan_wmm_delts_req_ioctl(wlan_private * priv, struct iwreq *wrq);
extern int wlan_wmm_queue_config_ioctl(wlan_private * priv,
                                       struct iwreq *wrq);
extern int wlan_wmm_queue_stats_ioctl(wlan_private * priv, struct iwreq *wrq);
#endif /* __WLAN_WMM_H */

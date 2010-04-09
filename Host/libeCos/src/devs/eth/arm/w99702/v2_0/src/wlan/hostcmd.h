/** @file hostcmd.h
 * 
 * @brief This file contains the function prototypes, data structure 
 * and defines for all the host/station commands
 *  
 *  Copyright ?Marvell International Ltd. and/or its affiliates, 2003-2006
 */
/********************************************************
Change log:
	10/11/05: Add Doxygen format comments
	01/11/06: Update association struct to reflect IEEE passthrough response
	          Conditionalize new scan/join structures
	04/10/06: Add hostcmd generic API and power_adapt_cfg_ext command
	04/18/06: Remove old Subscrive Event and add new Subscribe Event
		  implementation through generic hostcmd API
	05/03/06: Add auto_tx hostcmd
	05/04/06: Add IBSS coalescing related new hostcmd
********************************************************/

#ifndef __HOSTCMD__H
#define __HOSTCMD__H

#include "linux/list.h"
#include "wlan_wmm.h"

/*  802.11-related definitions */

/** TxPD descriptor */
typedef __packed struct _TxPD
{
        /** Current Tx packet status */
    cyg_uint32 TxStatus;
        /** Tx Control */
    cyg_uint32 TxControl;
    cyg_uint32 TxPacketLocation;
        /** Tx packet length */
    cyg_uint16 TxPacketLength;
        /** First 2 byte of destination MAC address */
    cyg_uint8 TxDestAddrHigh[2];
        /** Last 4 byte of destination MAC address */
    cyg_uint8 TxDestAddrLow[4];
        /** Pkt Priority */
    cyg_uint8 Priority;
        /** Pkt Trasnit Power control*/
    cyg_uint8 PowerMgmt;
        /** Amount of time the packet has been queued in the driver (units = 2ms)*/
    cyg_uint8 PktDelay_2ms;
        /** Reserved */
    cyg_uint8 Reserved1;

}TxPD, *PTxPD;

/** RxPD Descriptor */
typedef __packed struct _RxPD
{
        /** Current Rx packet status */
    cyg_uint16 RxStatus;

        /** SNR */
    cyg_uint8 SNR;

        /** Tx Control */
    cyg_uint8 RxControl;

        /** Pkt Length */
    cyg_uint16 PktLen;

        /** Noise Floor */
    cyg_uint8 NF;

        /** Rx Packet Rate */
    cyg_uint8 RxRate;

        /** Pkt addr*/
    cyg_uint32 PktPtr;

    cyg_uint32 Reserved_1;

        /** Pkt Priority */
    cyg_uint8 Priority;
    cyg_uint8 Reserved[3];

}RxPD, *PRxPD;

#if defined(__ECOS)

/** CmdCtrlNode */
typedef /*__packed*/ struct _CmdCtrlNode
{
    /* CMD link list */
    struct list_head list;

    cyg_uint32 Status;

    /* CMD ID */
    WLAN_OID cmd_oid;

    /*CMD wait option: wait for finish or no wait */
    cyg_uint16 wait_option;

    /* command parameter */
    void *pdata_buf;

    /*command data */
    cyg_uint8 *BufVirtualAddr;

    cyg_uint16 CmdFlags;

    /* wait queue */
    cyg_uint16 CmdWaitQWoken;
//    wait_queue_head_t cmdwait_q __ATTRIB_ALIGN__;

	cyg_flag_t cmdwait_flag_q;
} CmdCtrlNode, *PCmdCtrlNode;

#endif

/** MRVL_WEP_KEY */
typedef __packed struct _MRVL_WEP_KEY
{
    cyg_uint32 Length;
    cyg_uint32 KeyIndex;
    cyg_uint32 KeyLength;
    cyg_uint8 KeyMaterial[MRVL_KEY_BUFFER_SIZE_IN_BYTE];
} MRVL_WEP_KEY, *PMRVL_WEP_KEY;

typedef ULONGLONG WLAN_802_11_KEY_RSC;

/** WLAN_802_11_KEY */
typedef __packed struct _WLAN_802_11_KEY
{
    cyg_uint32 Length;
    cyg_uint32 KeyIndex;
    cyg_uint32 KeyLength;
    WLAN_802_11_MAC_ADDRESS BSSID;
    WLAN_802_11_KEY_RSC KeyRSC;
    cyg_uint8 KeyMaterial[MRVL_MAX_KEY_WPA_KEY_LENGTH];
} WLAN_802_11_KEY, *PWLAN_802_11_KEY;

/** MRVL_WPA_KEY */
typedef struct _MRVL_WPA_KEY
{
    cyg_uint32 KeyIndex;
    cyg_uint32 KeyLength;
    cyg_uint32 KeyRSC;
    cyg_uint8 KeyMaterial[MRVL_MAX_KEY_WPA_KEY_LENGTH];
} MRVL_WPA_KEY, *PMRVL_WPA_KEY;

/** MRVL_WLAN_WPA_KEY */
typedef struct _MRVL_WLAN_WPA_KEY
{
    cyg_uint8 EncryptionKey[16];
    cyg_uint8 MICKey1[8];
    cyg_uint8 MICKey2[8];
} MRVL_WLAN_WPA_KEY, *PMRVL_WLAN_WPA_KEY;

/** IE_WPA */
typedef struct _IE_WPA
{
    cyg_uint8 Elementid;
    cyg_uint8 Len;
    cyg_uint8 oui[4];
    cyg_uint16 version;
} IE_WPA, *PIE_WPA;

/* Received Signal Strength Indication  in dBm*/
typedef LONG WLAN_802_11_RSSI;

/** WLAN_802_11_WEP */
typedef __packed struct _WLAN_802_11_WEP
{
    /* Length of this structure */
    cyg_uint32 Length;

    /* 0 is the per-client key, 1-N are the global keys */
    cyg_uint32 KeyIndex;

    /* length of key in bytes */
    cyg_uint32 KeyLength;

    /* variable length depending on above field */
    cyg_uint8 KeyMaterial[1];
} WLAN_802_11_WEP, *PWLAN_802_11_WEP;

/** WLAN_802_11_SSID */
typedef __packed struct _WLAN_802_11_SSID
{
    /* SSID Length */
    cyg_uint32 SsidLength;

    /* SSID information field */
    cyg_uint8 Ssid[WLAN_MAX_SSID_LENGTH];
} WLAN_802_11_SSID, *PWLAN_802_11_SSID;

typedef struct _WPA_SUPPLICANT
{
    cyg_uint8 Wpa_ie[256];
    cyg_uint8 Wpa_ie_len;
} WPA_SUPPLICANT, *PWPA_SUPPLICANT;

typedef cyg_uint32 WLAN_802_11_FRAGMENTATION_THRESHOLD;
typedef cyg_uint32 WLAN_802_11_RTS_THRESHOLD;
typedef cyg_uint32 WLAN_802_11_ANTENNA;

/** wlan_offset_value */
typedef struct _wlan_offset_value
{
    cyg_uint32 offset;
    cyg_uint32 value;
} wlan_offset_value;

/** WLAN_802_11_FIXED_IEs */
typedef struct _WLAN_802_11_FIXED_IEs
{
    cyg_uint8 Timestamp[8];
    cyg_uint16 BeaconInterval;
    cyg_uint16 Capabilities;
} WLAN_802_11_FIXED_IEs, *PWLAN_802_11_FIXED_IEs;

/** WLAN_802_11_VARIABLE_IEs */
typedef struct _WLAN_802_11_VARIABLE_IEs
{
    cyg_uint8 ElementID;
    cyg_uint8 Length;
    cyg_uint8 data[1];
} WLAN_802_11_VARIABLE_IEs, *PWLAN_802_11_VARIABLE_IEs;

/** WLAN_802_11_AI_RESFI */
typedef struct _WLAN_802_11_AI_RESFI
{
    cyg_uint16 Capabilities;
    cyg_uint16 StatusCode;
    cyg_uint16 AssociationId;
} WLAN_802_11_AI_RESFI, *PWLAN_802_11_AI_RESFI;

/** WLAN_802_11_AI_REQFI */
typedef struct _WLAN_802_11_AI_REQFI
{
    cyg_uint16 Capabilities;
    cyg_uint16 ListenInterval;
    WLAN_802_11_MAC_ADDRESS CurrentAPAddress;
} WLAN_802_11_AI_REQFI, *PWLAN_802_11_AI_REQFI;

/* Define general data structure */
/** HostCmd_DS_GEN */
typedef __packed struct _HostCmd_DS_GEN
{
    cyg_uint16 Command;
    cyg_uint16 Size;
    cyg_uint16 SeqNum;
    cyg_uint16 Result;
} HostCmd_DS_GEN, *PHostCmd_DS_GEN,
    HostCmd_DS_802_11_DEEP_SLEEP, *PHostCmd_DS_802_11_DEEP_SLEEP;

#define S_DS_GEN    sizeof(HostCmd_DS_GEN)
/*
 * Define data structure for HostCmd_CMD_GET_HW_SPEC
 * This structure defines the response for the GET_HW_SPEC command
 */
/** HostCmd_DS_GET_HW_SPEC */
typedef __packed struct _HostCmd_DS_GET_HW_SPEC
{
    /* HW Interface version number */
    cyg_uint16 HWIfVersion;

    /* HW version number */
    cyg_uint16 Version;

    /* Max number of TxPD FW can handle */
    cyg_uint16 NumOfTxPD;

    /* Max no of Multicast address  */
    cyg_uint16 NumOfMCastAdr;

    /* MAC address */
    cyg_uint8 PermanentAddr[6];

    /* Region Code */
    cyg_uint16 RegionCode;

    /* Number of antenna used */
    cyg_uint16 NumberOfAntenna;

    /* FW release number, example 0x1234=1.2.3.4 */
    cyg_uint32 FWReleaseNumber;

    cyg_uint32 Reserved_1;

    cyg_uint32 Reserved_2;

    cyg_uint32 Reserved_3;

    /*FW/HW Capability */
    cyg_uint32 fwCapInfo;
} HostCmd_DS_GET_HW_SPEC, *PHostCmd_DS_GET_HW_SPEC;

/**  HostCmd_CMD_EEPROM_UPDATE */
typedef __packed struct _HostCmd_DS_EEPROM_UPDATE
{
    cyg_uint16 Action;
    cyg_uint32 Value;
} HostCmd_DS_EEPROM_UPDATE, *PHostCmd_DS_EEPROM_UPDATE;

typedef __packed struct _HostCmd_DS_802_11_SUBSCRIBE_EVENT
{
    cyg_uint16 Action;
    cyg_uint16 Events;
} HostCmd_DS_802_11_SUBSCRIBE_EVENT;

/* 
 * This scan handle Country Information IE(802.11d compliant) 
 * Define data structure for HostCmd_CMD_802_11_SCAN 
 */
/** HostCmd_DS_802_11_SCAN */
typedef __packed struct _HostCmd_DS_802_11_SCAN
{
    cyg_uint8 BSSType;
    cyg_uint8 BSSID[ETH_ALEN];
    cyg_uint8 TlvBuffer[1];
    /* MrvlIEtypes_SsIdParamSet_t   SsIdParamSet; 
     * MrvlIEtypes_ChanListParamSet_t       ChanListParamSet;
     * MrvlIEtypes_RatesParamSet_t  OpRateSet; 
     * */
} HostCmd_DS_802_11_SCAN, *PHostCmd_DS_802_11_SCAN;

typedef __packed struct _HostCmd_DS_802_11_SCAN_RSP
{
    cyg_uint16 BSSDescriptSize;
    cyg_uint8 NumberOfSets;
    cyg_uint8 BssDescAndTlvBuffer[1];

} HostCmd_DS_802_11_SCAN_RSP, *PHostCmd_DS_802_11_SCAN_RSP;

/** HostCmd_CMD_802_11_GET_LOG */
typedef __packed struct _HostCmd_DS_802_11_GET_LOG
{
    cyg_uint32 mcasttxframe;
    cyg_uint32 failed;
    cyg_uint32 retry;
    cyg_uint32 multiretry;
    cyg_uint32 framedup;
    cyg_uint32 rtssuccess;
    cyg_uint32 rtsfailure;
    cyg_uint32 ackfailure;
    cyg_uint32 rxfrag;
    cyg_uint32 mcastrxframe;
    cyg_uint32 fcserror;
    cyg_uint32 txframe;
    cyg_uint32 wepundecryptable;
} HostCmd_DS_802_11_GET_LOG, *PHostCmd_DS_802_11_GET_LOG;

/**  HostCmd_CMD_MAC_CONTROL */
typedef __packed struct _HostCmd_DS_MAC_CONTROL
{
    cyg_uint16 Action;
    cyg_uint16 Reserved;
} HostCmd_DS_MAC_CONTROL, *PHostCmd_DS_MAC_CONTROL;

/**  HostCmd_CMD_MAC_MULTICAST_ADR */
typedef __packed struct _HostCmd_DS_MAC_MULTICAST_ADR
{
    cyg_uint16 Action;
    cyg_uint16 NumOfAdrs;
    cyg_uint8 MACList[MRVDRV_ETH_ADDR_LEN * MRVDRV_MAX_MULTICAST_LIST_SIZE];
} HostCmd_DS_MAC_MULTICAST_ADR,
    *PHostCmd_DS_MAC_MULTICAST_ADR;

/** HostCmd_CMD_802_11_AUTHENTICATE */
typedef __packed struct _HostCmd_DS_802_11_AUTHENTICATE
{
    cyg_uint8 MacAddr[ETH_ALEN];
    cyg_uint8 AuthType;
    cyg_uint8 Reserved[10];
} HostCmd_DS_802_11_AUTHENTICATE,
    *PHostCmd_DS_802_11_AUTHENTICATE;

/** HostCmd_RET_802_11_AUTHENTICATE */
typedef __packed struct _HostCmd_DS_802_11_AUTHENTICATE_RSP
{
    cyg_uint8 MacAddr[6];
    cyg_uint8 AuthType;
    cyg_uint8 AuthStatus;
} HostCmd_DS_802_11_AUTHENTICATE_RSP,
    *PHostCmd_DS_802_11_AUTHENTICATE_RSP;

/**  HostCmd_CMD_802_11_DEAUTHENTICATE */
typedef __packed struct _HostCmd_DS_802_11_DEAUTHENTICATE
{
    cyg_uint8 MacAddr[6];
    cyg_uint16 ReasonCode;
} HostCmd_DS_802_11_DEAUTHENTICATE,
    *PHostCmd_DS_802_11_DEAUTHENTICATE;

/** HostCmd_DS_802_11_ASSOCIATE */
typedef __packed struct _HostCmd_DS_802_11_ASSOCIATE
{
    cyg_uint8 PeerStaAddr[6];
    IEEEtypes_CapInfo_t CapInfo;
    cyg_uint16 ListenInterval;
    cyg_uint16 BcnPeriod;
    cyg_uint8 DtimPeriod;

    /*
     *      MrvlIEtypes_SsIdParamSet_t      SsIdParamSet;
     *      MrvlIEtypes_PhyParamSet_t       PhyParamSet;
     *      MrvlIEtypes_SsParamSet_t        SsParamSet;
     *      MrvlIEtypes_RatesParamSet_t     RatesParamSet;
     */
} HostCmd_DS_802_11_ASSOCIATE, *PHostCmd_DS_802_11_ASSOCIATE;

/**  HostCmd_CMD_802_11_DISASSOCIATE */
typedef __packed struct _HostCmd_DS_802_11_DISASSOCIATE
{
    cyg_uint8 DestMacAddr[6];
    cyg_uint16 ReasonCode;
} HostCmd_DS_802_11_DISASSOCIATE,
    *PHostCmd_DS_802_11_DISASSOCIATE;

/** HostCmd_RET_802_11_ASSOCIATE */
typedef __packed struct
{
    IEEEtypes_AssocRsp_t assocRsp;
} HostCmd_DS_802_11_ASSOCIATE_RSP;

/**  HostCmd_RET_802_11_AD_HOC_JOIN */
typedef __packed struct _HostCmd_DS_802_11_AD_HOC_RESULT
{
    cyg_uint8 PAD[3];
    cyg_uint8 BSSID[MRVDRV_ETH_ADDR_LEN];
} HostCmd_DS_802_11_AD_HOC_RESULT,
    *PHostCmd_DS_802_11_AD_HOC_RESULT;

/**  HostCmd_CMD_802_11_SET_WEP */
typedef __packed struct _HostCmd_DS_802_11_SET_WEP
{
    /* ACT_ADD, ACT_REMOVE or ACT_ENABLE  */
    cyg_uint16 Action;

    /* Key Index selected for Tx */
    cyg_uint16 KeyIndex;

    /* 40, 128bit or TXWEP */
    cyg_uint8 WEPTypeForKey1;

    cyg_uint8 WEPTypeForKey2;
    cyg_uint8 WEPTypeForKey3;
    cyg_uint8 WEPTypeForKey4;
    cyg_uint8 WEP1[16];
    cyg_uint8 WEP2[16];
    cyg_uint8 WEP3[16];
    cyg_uint8 WEP4[16];
} HostCmd_DS_802_11_SET_WEP, *PHostCmd_DS_802_11_SET_WEP;

/**  HostCmd_CMD_802_3_GET_STAT */
typedef __packed struct _HostCmd_DS_802_3_GET_STAT
{
    cyg_uint32 XmitOK;
    cyg_uint32 RcvOK;
    cyg_uint32 XmitError;
    cyg_uint32 RcvError;
    cyg_uint32 RcvNoBuffer;
    cyg_uint32 RcvCRCError;
} HostCmd_DS_802_3_GET_STAT, *PHostCmd_DS_802_3_GET_STAT;

/** HostCmd_CMD_802_11_GET_STAT */
typedef __packed struct _HostCmd_DS_802_11_GET_STAT
{
    cyg_uint32 TXFragmentCnt;
    cyg_uint32 MCastTXFrameCnt;
    cyg_uint32 FailedCnt;
    cyg_uint32 RetryCnt;
    cyg_uint32 MultipleRetryCnt;
    cyg_uint32 RTSSuccessCnt;
    cyg_uint32 RTSFailureCnt;
    cyg_uint32 ACKFailureCnt;
    cyg_uint32 FrameDuplicateCnt;
    cyg_uint32 RXFragmentCnt;
    cyg_uint32 MCastRXFrameCnt;
    cyg_uint32 FCSErrorCnt;
    cyg_uint32 BCastTXFrameCnt;
    cyg_uint32 BCastRXFrameCnt;
    cyg_uint32 TXBeacon;
    cyg_uint32 RXBeacon;
    cyg_uint32 WEPUndecryptable;
} HostCmd_DS_802_11_GET_STAT, *PHostCmd_DS_802_11_GET_STAT;

/** HostCmd_DS_802_11_AD_HOC_STOP */
typedef __packed struct _HostCmd_DS_802_11_AD_HOC_STOP
{
	void *pad;
} HostCmd_DS_802_11_AD_HOC_STOP,
    *PHostCmd_DS_802_11_AD_HOC_STOP;

/** HostCmd_DS_802_11_BEACON_STOP */
typedef __packed struct _HostCmd_DS_802_11_BEACON_STOP
{
	void *pad;
} HostCmd_DS_802_11_BEACON_STOP,
    *PHostCmd_DS_802_11_BEACON_STOP;

/**  HostCmd_CMD_802_11_SNMP_MIB */
typedef __packed struct _HostCmd_DS_802_11_SNMP_MIB
{
    cyg_uint16 QueryType;
    cyg_uint16 OID;
    cyg_uint16 BufSize;
    cyg_uint8 Value[128];
} HostCmd_DS_802_11_SNMP_MIB, *PHostCmd_DS_802_11_SNMP_MIB;

/**  HostCmd_CMD_MAC_REG_MAP */
typedef __packed struct _HostCmd_DS_MAC_REG_MAP
{
    cyg_uint16 BufferSize;
    cyg_uint8 RegMap[128];
    cyg_uint16 Reserved;
} HostCmd_DS_MAC_REG_MAP, *PHostCmd_DS_MAC_REG_MAP;

/*  HostCmd_CMD_BBP_REG_MAP */
typedef __packed struct _HostCmd_DS_BBP_REG_MAP
{
    cyg_uint16 BufferSize;
    cyg_uint8 RegMap[128];
    cyg_uint16 Reserved;
} HostCmd_DS_BBP_REG_MAP, *PHostCmd_DS_BBP_REG_MAP;

/** HostCmd_CMD_RF_REG_MAP */
typedef __packed struct _HostCmd_DS_RF_REG_MAP
{
    cyg_uint16 BufferSize;
    cyg_uint8 RegMap[64];
    cyg_uint16 Reserved;
} HostCmd_DS_RF_REG_MAP, *PHostCmd_DS_RF_REG_MAP;

/** HostCmd_CMD_MAC_REG_ACCESS */
typedef __packed struct _HostCmd_DS_MAC_REG_ACCESS
{
    cyg_uint16 Action;
    cyg_uint16 Offset;
    cyg_uint32 Value;
} HostCmd_DS_MAC_REG_ACCESS, *PHostCmd_DS_MAC_REG_ACCESS;

/** HostCmd_CMD_BBP_REG_ACCESS */
typedef __packed struct _HostCmd_DS_BBP_REG_ACCESS
{
    cyg_uint16 Action;
    cyg_uint16 Offset;
    cyg_uint8 Value;
    cyg_uint8 Reserved[3];
} HostCmd_DS_BBP_REG_ACCESS, *PHostCmd_DS_BBP_REG_ACCESS;

/**  HostCmd_CMD_RF_REG_ACCESS */
typedef __packed struct _HostCmd_DS_RF_REG_ACCESS
{
    cyg_uint16 Action;
    cyg_uint16 Offset;
    cyg_uint8 Value;
    cyg_uint8 Reserved[3];
} HostCmd_DS_RF_REG_ACCESS, *PHostCmd_DS_RF_REG_ACCESS;

/** HostCmd_CMD_802_11_RADIO_CONTROL */
typedef __packed struct _HostCmd_DS_802_11_RADIO_CONTROL
{
    cyg_uint16 Action;
    cyg_uint16 Control;
} HostCmd_DS_802_11_RADIO_CONTROL,
    *PHostCmd_DS_802_11_RADIO_CONTROL;

/* HostCmd_DS_802_11_SLEEP_PARAMS */
typedef __packed struct _HostCmd_DS_802_11_SLEEP_PARAMS
{
    /* ACT_GET/ACT_SET */
    cyg_uint16 Action;

    /* Sleep clock error in ppm */
    cyg_uint16 Error;

    /* Wakeup offset in usec */
    cyg_uint16 Offset;

    /* Clock stabilization time in usec */
    cyg_uint16 StableTime;

    /* Control periodic calibration */
    cyg_uint8 CalControl;

    /* Control the use of external sleep clock */
    cyg_uint8 ExternalSleepClk;

    /* Reserved field, should be set to zero */
    cyg_uint16 Reserved;
} HostCmd_DS_802_11_SLEEP_PARAMS,
    *PHostCmd_DS_802_11_SLEEP_PARAMS;

/* HostCmd_DS_802_11_SLEEP_PERIOD */
typedef __packed struct _HostCmd_DS_802_11_SLEEP_PERIOD
{
    /* ACT_GET/ACT_SET */
    cyg_uint16 Action;

    /* Sleep Period in msec */
    cyg_uint16 Period;
} HostCmd_DS_802_11_SLEEP_PERIOD,
    *PHostCmd_DS_802_11_SLEEP_PERIOD;

/* HostCmd_DS_802_11_BCA_TIMESHARE */
typedef __packed struct _HostCmd_DS_802_11_BCA_TIMESHARE
{
    /* ACT_GET/ACT_SET */
    cyg_uint16 Action;

    /* Type: WLAN, BT */
    cyg_uint16 TrafficType;

    /* 20msec - 60000msec */
    cyg_uint32 TimeShareInterval;

    /* PTA arbiter time in msec */
    cyg_uint32 BTTime;
} HostCmd_DS_802_11_BCA_TIMESHARE,
    *PHostCmd_DS_802_11_BCA_TIMESHARE;

/* HostCmd_DS_802_11_INACTIVITY_TIMEOUT */
typedef __packed struct _HostCmd_DS_802_11_INACTIVITY_TIMEOUT
{
    /* ACT_GET/ACT_SET */
    cyg_uint16 Action;

    /* Inactivity timeout in msec */
    cyg_uint16 Timeout;
} HostCmd_DS_802_11_INACTIVITY_TIMEOUT,
    *PHostCmd_DS_802_11_INACTIVITY_TIMEOUT;

/** HostCmd_CMD_802_11_RF_CHANNEL */
typedef __packed struct _HostCmd_DS_802_11_RF_CHANNEL
{
    cyg_uint16 Action;
    cyg_uint16 CurrentChannel;
    cyg_uint16 RFType;
    cyg_uint16 Reserved;
    cyg_uint8 ChannelList[32];
}HostCmd_DS_802_11_RF_CHANNEL,
    *PHostCmd_DS_802_11_RF_CHANNEL;

/**  HostCmd_CMD_802_11_RSSI */
typedef __packed struct _HostCmd_DS_802_11_RSSI
{
    /* weighting factor */
    cyg_uint16 N;

    cyg_uint16 Reserved_0;
    cyg_uint16 Reserved_1;
    cyg_uint16 Reserved_2;
} HostCmd_DS_802_11_RSSI, *PHostCmd_DS_802_11_RSSI;

/** HostCmd_DS_802_11_RSSI_RSP */
typedef __packed struct _HostCmd_DS_802_11_RSSI_RSP
{
    cyg_uint16 SNR;
    cyg_uint16 NoiseFloor;
    cyg_uint16 AvgSNR;
    cyg_uint16 AvgNoiseFloor;
} HostCmd_DS_802_11_RSSI_RSP, *PHostCmd_DS_802_11_RSSI_RSP;

/** HostCmd_DS_802_11_MAC_ADDRESS */
typedef __packed struct _HostCmd_DS_802_11_MAC_ADDRESS
{
    cyg_uint16 Action;
    cyg_uint8 MacAdd[ETH_ALEN];
} HostCmd_DS_802_11_MAC_ADDRESS,
    *PHostCmd_DS_802_11_MAC_ADDRESS;

/** HostCmd_CMD_802_11_RF_TX_POWER */
typedef __packed struct _HostCmd_DS_802_11_RF_TX_POWER
{
    cyg_uint16 Action;
    cyg_uint16 CurrentLevel;
    cyg_uint8 MaxPower;
    cyg_uint8 MinPower;
} HostCmd_DS_802_11_RF_TX_POWER,
    *PHostCmd_DS_802_11_RF_TX_POWER;

/** HostCmd_CMD_802_11_RF_ANTENNA */
typedef __packed struct _HostCmd_DS_802_11_RF_ANTENNA
{
    cyg_uint16 Action;

    /*  Number of antennas or 0xffff(diversity) */
    cyg_uint16 AntennaMode;

} HostCmd_DS_802_11_RF_ANTENNA,
    *PHostCmd_DS_802_11_RF_ANTENNA;

/** HostCmd_CMD_802_11_PS_MODE */
typedef __packed struct _HostCmd_DS_802_11_PS_MODE
{
    cyg_uint16 Action;
    cyg_uint16 NullPktInterval;
    cyg_uint16 MultipleDtim;
    cyg_uint16 Reserved;
    cyg_uint16 LocalListenInterval;
    cyg_uint16 AdhocAwakePeriod;
} HostCmd_DS_802_11_PS_MODE, *PHostCmd_DS_802_11_PS_MODE;

/** PS_CMD_ConfirmSleep */
typedef __packed struct _PS_CMD_ConfirmSleep
{
	cyg_uint8  sdio_header[SDIO_HEADER_LEN];//clyu add
    cyg_uint16 Command;
    cyg_uint16 Size;
    cyg_uint16 SeqNum;
    cyg_uint16 Result;

    cyg_uint16 Action;
    cyg_uint16 Reserved1;
    cyg_uint16 MultipleDtim;
    cyg_uint16 Reserved;
    cyg_uint16 LocalListenInterval;
} PS_CMD_ConfirmSleep, *PPS_CMD_ConfirmSleep;

/** HostCmd_CMD_802_11_FW_WAKEUP_METHOD */
typedef __packed struct _HostCmd_DS_802_11_FW_WAKEUP_METHOD
{
    cyg_uint16 Action;
    cyg_uint16 Method;
} HostCmd_DS_802_11_FW_WAKEUP_METHOD,
    *PHostCmd_DS_802_11_FW_WAKEUP_METHOD;

/** HostCmd_CMD_802_11_DATA_RATE */
typedef __packed struct _HostCmd_DS_802_11_DATA_RATE
{
    cyg_uint16 Action;
    cyg_uint16 Reserverd;
    cyg_uint8 DataRate[HOSTCMD_SUPPORTED_RATES];
} HostCmd_DS_802_11_DATA_RATE, *PHostCmd_DS_802_11_DATA_RATE;

/** HostCmd_DS_802_11_RATE_ADAPT_RATESET */
typedef __packed struct _HostCmd_DS_802_11_RATE_ADAPT_RATESET
{
    cyg_uint16 Action;
    cyg_uint16 EnableHwAuto;
    cyg_uint16 Bitmap;
} HostCmd_DS_802_11_RATE_ADAPT_RATESET,
    *PHostCmd_DS_802_11_RATE_ADAPT_RATESET;

/** HostCmd_DS_802_11_AD_HOC_START*/
typedef __packed struct _HostCmd_DS_802_11_AD_HOC_START
{
    cyg_uint8 SSID[MRVDRV_MAX_SSID_LENGTH];
    cyg_uint8 BSSType;
    cyg_uint16 BeaconPeriod;
    cyg_uint8 DTIMPeriod;
    IEEEtypes_SsParamSet_t SsParamSet;
    IEEEtypes_PhyParamSet_t PhyParamSet;
    cyg_uint16 Reserved1;
    IEEEtypes_CapInfo_t Cap;
    cyg_uint8 DataRate[HOSTCMD_SUPPORTED_RATES];
    cyg_uint8 tlv_memory_size_pad[100];
} HostCmd_DS_802_11_AD_HOC_START,
    *PHostCmd_DS_802_11_AD_HOC_START;

/** AdHoc_BssDesc_t */
typedef __packed struct _AdHoc_BssDesc_t
{
    cyg_uint8 BSSID[6];
    cyg_uint8 SSID[32];
    cyg_uint8 BSSType;
    cyg_uint16 BeaconPeriod;
    cyg_uint8 DTIMPeriod;
    cyg_uint8 TimeStamp[8];
    cyg_uint8 LocalTime[8];
    IEEEtypes_PhyParamSet_t PhyParamSet;
    IEEEtypes_SsParamSet_t SsParamSet;
    IEEEtypes_CapInfo_t Cap;
    cyg_uint8 DataRates[HOSTCMD_SUPPORTED_RATES];

    /* DO NOT ADD ANY FIELDS TO THIS STRUCTURE.      It is used below in the
     *      Adhoc join command and will cause a binary layout mismatch with 
     *      the firmware 
     */
} AdHoc_BssDesc_t;

/** HostCmd_DS_802_11_AD_HOC_JOIN */
typedef __packed struct _HostCmd_DS_802_11_AD_HOC_JOIN
{
    AdHoc_BssDesc_t BssDescriptor;
    cyg_uint16 Reserved1;
    cyg_uint16 Reserved2;

} HostCmd_DS_802_11_AD_HOC_JOIN,
    *PHostCmd_DS_802_11_AD_HOC_JOIN;

/** HostCmd_DS_802_11_ENABLE_RSN */
typedef __packed struct _HostCmd_DS_802_11_ENABLE_RSN
{
    cyg_uint16 Action;
    cyg_uint16 Enable;
} HostCmd_DS_802_11_ENABLE_RSN,
    *PHostCmd_DS_802_11_ENABLE_RSN;

/** HostCmd_DS_802_11_QUERY_TKIP_REPLY_CNTRS */
typedef __packed struct _HostCmd_DS_802_11_QUERY_TKIP_REPLY_CNTRS
{
    cyg_uint16 CmdCode;
    cyg_uint16 Size;
    cyg_uint16 SeqNum;
    cyg_uint16 Result;
    cyg_uint32 NumTkipCntrs;
} HostCmd_DS_802_11_QUERY_TKIP_REPLY_CNTRS,
    *PHostCmd_DS_802_11_QUERY_TKIP_REPLY_CNTRS;

/** HostCmd_DS_802_11_PAIRWISE_TSC */
typedef __packed struct _HostCmd_DS_802_11_PAIRWISE_TSC
{
    cyg_uint16 CmdCode;
    cyg_uint16 Size;
    cyg_uint16 SeqNum;
    cyg_uint16 Result;
    cyg_uint16 Action;
    cyg_uint32 Txlv32;
    cyg_uint16 Txlv16;
} HostCmd_DS_802_11_PAIRWISE_TSC,
    *PHostCmd_DS_802_11_PAIRWISE_TSC;

/** HostCmd_DS_802_11_GROUP_TSC */
typedef __packed struct _HostCmd_DS_802_11_GROUP_TSC
{
    cyg_uint16 CmdCode;
    cyg_uint16 Size;
    cyg_uint16 SeqNum;
    cyg_uint16 Result;
    cyg_uint16 Action;
    cyg_uint32 Txlv32;
    cyg_uint16 Txlv16;
} HostCmd_DS_802_11_GROUP_TSC, *PHostCmd_DS_802_11_GROUP_TSC;

typedef __packed union _KeyInfo_WEP_t
{
    cyg_uint8 Reserved;

    /* bits 1-4: Specifies the index of key */
    cyg_uint8 WepKeyIndex;

    /* bit 0: Specifies that this key is 
     * to be used as the default for TX data packets 
     * */
    cyg_uint8 isWepDefaultKey;
} KeyInfo_WEP_t;

typedef __packed union _KeyInfo_TKIP_t
{
    cyg_uint8 Reserved;

    /* bit 2: Specifies that this key is 
     * enabled and valid to use */
    cyg_uint8 isKeyEnabled;

    /* bit 1: Specifies that this key is
     * to be used as the unicast key */
    cyg_uint8 isUnicastKey;

    /* bit 0: Specifies that this key is 
     * to be used as the multicast key */
    cyg_uint8 isMulticastKey;
} KeyInfo_TKIP_t;

typedef __packed union _KeyInfo_AES_t
{
    cyg_uint8 Reserved;

    /* bit 2: Specifies that this key is
     * enabled and valid to use */
    cyg_uint8 isKeyEnabled;

    /* bit 1: Specifies that this key is
     * to be used as the unicast key */
    cyg_uint8 isUnicastKey;

    /* bit 0: Specifies that this key is 
     * to be used as the multicast key */
    cyg_uint8 isMulticastKey;
} KeyInfo_AES_t;

/** KeyMaterial_TKIP_t */
typedef __packed struct _KeyMaterial_TKIP_t
{
    /* TKIP encryption/decryption key */
    cyg_uint8 TkipKey[16];

    /* TKIP TX MIC Key */
    cyg_uint8 TkipTxMicKey[16];

    /* TKIP RX MIC Key */
    cyg_uint8 TkipRxMicKey[16];
} KeyMaterial_TKIP_t, *PKeyMaterial_TKIP_t;

/** KeyMaterial_AES_t */
typedef __packed struct _KeyMaterial_AES_t
{
    /* AES encryption/decryption key */
    cyg_uint8 AesKey[16];
} KeyMaterial_AES_t, *PKeyMaterial_AES_t;

/** MrvlIEtype_KeyParamSet_t */
typedef __packed struct _MrvlIEtype_KeyParamSet_t
{
    /* Type ID */
    cyg_uint16 Type;

    /* Length of Payload */
    cyg_uint16 Length;

    /* Type of Key: WEP=0, TKIP=1, AES=2 */
    cyg_uint16 KeyTypeId;

    /* Key Control Info specific to a KeyTypeId */
    cyg_uint16 KeyInfo;

    /* Length of key */
    cyg_uint16 KeyLen;

    /* Key material of size KeyLen */
    cyg_uint8 Key[32];
} MrvlIEtype_KeyParamSet_t, *PMrvlIEtype_KeyParamSet_t;

/** HostCmd_DS_802_11_KEY_MATERIAL */
typedef __packed struct _HostCmd_DS_802_11_KEY_MATERIAL
{
    cyg_uint16 Action;

    MrvlIEtype_KeyParamSet_t KeyParamSet;
} HostCmd_DS_802_11_KEY_MATERIAL,
    *PHostCmd_DS_802_11_KEY_MATERIAL;

/** HostCmd_DS_802_11_HOST_SLEEP_CFG */
typedef __packed struct _HostCmd_DS_HOST_802_11_HOST_SLEEP_CFG
{
    /* bit0=1: non-unicast data
     * bit1=1: unicast data
     * bit2=1: mac events
     * bit3=1: magic packet 
     */
    cyg_uint32 conditions;

    cyg_uint8 gpio;

    /* in milliseconds */
    cyg_uint8 gap;
//    cyg_uint16 filter[6]; //JONO
} HostCmd_DS_802_11_HOST_SLEEP_CFG;

/** HostCmd_DS_802_11_CAL_DATA_EXT */
typedef __packed struct _HostCmd_DS_802_11_CAL_DATA_EXT
{
    cyg_uint16 Action;
    cyg_uint16 Revision;
    cyg_uint16 CalDataLen;
    cyg_uint8 CalData[1024];
} HostCmd_DS_802_11_CAL_DATA_EXT,
    *pHostCmd_DS_802_11_CAL_DATA_EXT;

/** HostCmd_DS_802_11_EEPROM_ACCESS */
typedef __packed struct _HostCmd_DS_802_11_EEPROM_ACCESS
{
    cyg_uint16 Action;

    /* multiple 4 */
    cyg_uint16 Offset;
    cyg_uint16 ByteCount;
    cyg_uint8 Value;
} HostCmd_DS_802_11_EEPROM_ACCESS,
    *pHostCmd_DS_802_11_EEPROM_ACCESS;

/** HostCmd_DS_802_11_BG_SCAN_CONFIG */
typedef __packed struct _HostCmd_DS_802_11_BG_SCAN_CONFIG
{
        /** Action */
    cyg_uint16 Action;

        /** Enable */
    /*  0 - Disable 1 - Enable */
    cyg_uint8 Enable;

        /** bssType */
    /*  1 - Infrastructure
     *  2 - IBSS
     *  3 - any 
     */
    cyg_uint8 BssType;

        /** ChannelsPerScan */
    /* No of channels to scan at one scan */
    cyg_uint8 ChannelsPerScan;

    /* 0 - Discard old scan results
     * 1 - Discard new scan results 
     */
    cyg_uint8 DiscardWhenFull;

    cyg_uint16 Reserved;

        /** ScanInterval */
    cyg_uint32 ScanInterval;

        /** StoreCondition */
    /* - SSID Match
     * - Exceed RSSI threshold
     * - SSID Match & Exceed RSSI Threshold 
     * - Always 
     */
    cyg_uint32 StoreCondition;

        /** ReportConditions */
    /* - SSID Match
     * - Exceed RSSI threshold
     * - SSID Match & Exceed RSSIThreshold
     * - Exceed MaxScanResults
     * - Entire channel list scanned once 
     * - Domain Mismatch in country IE 
     */
    cyg_uint32 ReportConditions;

        /** MaxScanResults */
    /* Max scan results that will trigger 
     * a scn completion event */
    cyg_uint16 MaxScanResults;

    /*      attach TLV based parameters as needed, e.g.
     *      MrvlIEtypes_SsIdParamSet_t      SsIdParamSet;
     *      MrvlIEtypes_ChanListParamSet_t  ChanListParamSet;
     *      MrvlIEtypes_NumProbes_t         NumProbes;
     */

} HostCmd_DS_802_11_BG_SCAN_CONFIG,
    *pHostCmd_DS_802_11_BG_SCAN_CONFIG;

/** HostCmd_DS_802_11_BG_SCAN_QUERY */
typedef __packed struct _HostCmd_DS_802_11_BG_SCAN_QUERY
{
    cyg_uint8 Flush;
} HostCmd_DS_802_11_BG_SCAN_QUERY,
    *pHostCmd_DS_802_11_BG_SCAN_QUERY;

/** HostCmd_DS_802_11_BG_SCAN_QUERY_RSP */
typedef __packed struct _HostCmd_DS_802_11_BG_SCAN_QUERY_RSP
{
    cyg_uint32 ReportCondition;
    HostCmd_DS_802_11_SCAN_RSP scanresp;
} HostCmd_DS_802_11_BG_SCAN_QUERY_RSP,
    *PHostCmd_DS_802_11_BG_SCAN_QUERY_RSP;

/** HostCmd_DS_802_11_TPC_CFG */
typedef __packed struct _HostCmd_DS_802_11_TPC_CFG
{
    cyg_uint16 Action;
    cyg_uint8 Enable;
    cyg_int8 P0;
    cyg_int8 P1;
    cyg_int8 P2;
    cyg_uint8 UseSNR;
} HostCmd_DS_802_11_TPC_CFG;

/** HostCmd_DS_802_11_LED_CTRL */
typedef __packed struct _HostCmd_DS_802_11_LED_CTRL
{
    cyg_uint16 Action;
    cyg_uint16 NumLed;
    cyg_uint8 data[256];
} HostCmd_DS_802_11_LED_CTRL;

/*** HostCmd_DS_802_11_PWR_CFG */
typedef __packed struct _HostCmd_DS_802_11_PWR_CFG
{
    cyg_uint16 Action;
    cyg_uint8 Enable;
    cyg_int8 PA_P0;
    cyg_int8 PA_P1;
    cyg_int8 PA_P2;
} HostCmd_DS_802_11_PWR_CFG;

/** HostCmd_DS_802_11_POWER_ADAPT_CFG_EXT */
typedef __packed struct _HostCmd_DS_802_11_POWER_ADAPT_CFG_EXT
{
        /** Action */
    cyg_uint16 Action;                 /* 0 = ACT_GET; 1 = ACT_SET; */
    cyg_uint16 EnablePA;               /* 0 = disable; 1 = enable; */
    MrvlIEtypes_PowerAdapt_Group_t PowerAdaptGroup;
} HostCmd_DS_802_11_POWER_ADAPT_CFG_EXT,
    *PHostCmd_DS_802_11_POWER_ADAPT_CFG_EXT;

/** HostCmd_DS_802_11_AFC */
typedef __packed struct _HostCmd_DS_802_11_AFC
{
    cyg_uint16 afc_auto;
    __packed union
    {
        __packed struct
        {
            cyg_uint16 threshold;
            cyg_uint16 period;
        } auto_mode;

        __packed struct
        {
            cyg_int16 timing_offset;
            cyg_int16 carrier_offset;
        } manual_mode;
    } b;
} HostCmd_DS_802_11_AFC;

#define afc_data    b.data
#define afc_thre    b.auto_mode.threshold
#define afc_period  b.auto_mode.period
#define afc_toff    b.manual_mode.timing_offset
#define afc_foff    b.manual_mode.carrier_offset

typedef __packed struct _HostCmd_DS_802_11_IBSS_Status
{
    cyg_uint16 Action;
    cyg_uint16 Enable;
    cyg_uint8 BSSID[ETH_ALEN];
    cyg_uint16 BeaconInterval;
    cyg_uint16 ATIMWindow;
} HostCmd_DS_802_11_IBSS_Status;

typedef __packed struct _HostCmd_TX_RATE_QUERY
{
    cyg_uint16 TxRate;
} HostCmd_TX_RATE_QUERY, *PHostCmd_TX_RATE_QUERY;

/** HostCmd_DS_802_11_AUTO_TX */
typedef __packed struct _HostCmd_DS_802_11_AUTO_TX
{
        /** Action */
    cyg_uint16 Action;                 /* 0 = ACT_GET; 1 = ACT_SET; */
    MrvlIEtypes_AutoTx_t AutoTx;
} HostCmd_DS_802_11_AUTO_TX, *PHostCmd_DS_802_11_AUTO_TX;

/** HostCmd_MEM_ACCESS */
typedef __packed struct _HostCmd_DS_MEM_ACCESS
{
        /** Action */
    cyg_uint16 Action;                 /* 0 = ACT_GET; 1 = ACT_SET; */
    cyg_uint16 Reserved;
    cyg_uint32 Addr;
    cyg_uint32 Value;
} HostCmd_DS_MEM_ACCESS, *PHostCmd_DS_MEM_ACCESS;

typedef __packed struct
{
    cyg_uint64 TsfValue;
} HostCmd_DS_GET_TSF;

/** _HostCmd_DS_COMMAND*/
__packed struct _HostCmd_DS_COMMAND
{

    /** Command Header */
    cyg_uint16 Command;
    cyg_uint16 Size;
    cyg_uint16 SeqNum;
    cyg_uint16 Result;

    /** Command Body */
    __packed union
    {
        HostCmd_DS_GET_HW_SPEC hwspec;
        HostCmd_DS_802_11_PS_MODE psmode;
        HostCmd_DS_802_11_SCAN scan;
        HostCmd_DS_802_11_SCAN_RSP scanresp;
        HostCmd_DS_MAC_CONTROL macctrl;
        HostCmd_DS_802_11_ASSOCIATE associate;
        HostCmd_DS_802_11_ASSOCIATE_RSP associatersp;
        HostCmd_DS_802_11_DEAUTHENTICATE deauth;
        HostCmd_DS_802_11_SET_WEP wep;
        HostCmd_DS_802_11_AD_HOC_START ads;
        HostCmd_DS_802_11_AD_HOC_RESULT result;
        HostCmd_DS_802_11_GET_LOG glog;
        HostCmd_DS_802_11_AUTHENTICATE auth;
        HostCmd_DS_802_11_AUTHENTICATE_RSP rauth;
        HostCmd_DS_802_11_GET_STAT gstat;
        HostCmd_DS_802_3_GET_STAT gstat_8023;
        HostCmd_DS_802_11_SNMP_MIB smib;
        HostCmd_DS_802_11_RF_TX_POWER txp;
        HostCmd_DS_802_11_RF_ANTENNA rant;
        HostCmd_DS_802_11_DATA_RATE drate;
        HostCmd_DS_802_11_RATE_ADAPT_RATESET rateset;
        HostCmd_DS_MAC_MULTICAST_ADR madr;
        HostCmd_DS_802_11_AD_HOC_JOIN adj;
        HostCmd_DS_802_11_RADIO_CONTROL radio;
        HostCmd_DS_802_11_RF_CHANNEL rfchannel;
        HostCmd_DS_802_11_RSSI rssi;
        HostCmd_DS_802_11_RSSI_RSP rssirsp;
        HostCmd_DS_802_11_DISASSOCIATE dassociate;
        HostCmd_DS_802_11_AD_HOC_STOP adhoc_stop;
        HostCmd_DS_802_11_MAC_ADDRESS macadd;
        HostCmd_DS_802_11_ENABLE_RSN enbrsn;
        HostCmd_DS_802_11_KEY_MATERIAL keymaterial;
        HostCmd_DS_MAC_REG_ACCESS macreg;
        HostCmd_DS_BBP_REG_ACCESS bbpreg;
        HostCmd_DS_RF_REG_ACCESS rfreg;
        HostCmd_DS_802_11_BEACON_STOP beacon_stop;
        HostCmd_DS_802_11_CAL_DATA_EXT caldataext;
        HostCmd_DS_802_11_HOST_SLEEP_CFG hostsleepcfg;
        HostCmd_DS_802_11_EEPROM_ACCESS rdeeprom;

        HostCmd_DS_802_11D_DOMAIN_INFO domaininfo;
        HostCmd_DS_802_11D_DOMAIN_INFO_RSP domaininforesp;
        HostCmd_DS_802_11_BG_SCAN_CONFIG bgscancfg;
        HostCmd_DS_802_11_BG_SCAN_QUERY bgscanquery;
        HostCmd_DS_802_11_BG_SCAN_QUERY_RSP bgscanqueryresp;
        HostCmd_DS_WMM_GET_STATUS getWmmStatus;
        HostCmd_DS_WMM_ACK_POLICY ackpolicy;
        HostCmd_DS_WMM_ADDTS_REQ addTsReq;
        HostCmd_DS_WMM_DELTS_REQ delTsReq;
        HostCmd_DS_WMM_QUEUE_CONFIG queueConfig;
        HostCmd_DS_WMM_QUEUE_STATS queueStats;

        HostCmd_DS_802_11_SLEEP_PARAMS sleep_params;
        HostCmd_DS_802_11_BCA_TIMESHARE bca_timeshare;
        HostCmd_DS_802_11_INACTIVITY_TIMEOUT inactivity_timeout;
        HostCmd_DS_802_11_SLEEP_PERIOD ps_sleeppd;
        HostCmd_DS_802_11_TPC_CFG tpccfg;
        HostCmd_DS_802_11_PWR_CFG pwrcfg;
        HostCmd_DS_802_11_AFC afc;
        HostCmd_DS_802_11_LED_CTRL ledgpio;
        HostCmd_DS_802_11_FW_WAKEUP_METHOD fwwakeupmethod;

        HostCmd_TX_RATE_QUERY txrate;
        HostCmd_DS_GET_TSF gettsf;
        HostCmd_DS_802_11_IBSS_Status ibssCoalescing;
    } params;
};

#endif

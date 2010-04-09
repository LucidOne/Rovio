/** @file wlan_types.h
 *  @brief This header file contains definition for global types
 *
 *  Copyright © Marvell International Ltd. and/or its affiliates, 2003-2006
 */
/*************************************************************
Change log:
	10/11/05: add Doxygen format comments 
	01/11/06: Add IEEE Association response type.  Add TSF TLV information.
	01/31/06: Add support to selectively enabe the FW Scan channel filter
	04/10/06: Add power_adapt_cfg_ext command
	04/18/06: Remove old Subscrive Event and add new Subscribe Event
		  implementation through generic hostcmd API
	05/03/06: Add auto_tx hostcmd
************************************************************/

#ifndef _WLAN_TYPES_
#define _WLAN_TYPES_

/** IEEE Type definitions  */
typedef __packed enum _IEEEtypes_ElementId_e
{
    SSID = 0,
    SUPPORTED_RATES,
    FH_PARAM_SET,
    DS_PARAM_SET,
    CF_PARAM_SET,
    TIM,
    IBSS_PARAM_SET,
    COUNTRY_INFO = 7,

    CHALLENGE_TEXT = 16,

    EXTENDED_SUPPORTED_RATES = 50,

    VENDOR_SPECIFIC_221 = 221,
    WMM_IE = 221,

    WPA_IE = 221,
    WPA2_IE = 48,

    EXTRA_IE = 133
} IEEEtypes_ElementId_e;

#define WMM_OUI_TYPE  2

#define CAPINFO_MASK    (~( W_BIT_15 | W_BIT_14 |               \
                            W_BIT_12 | W_BIT_11 | W_BIT_9) )

typedef __packed struct _IEEEtypes_CapInfo_t
{
    cyg_uint8 Ess:1;
    cyg_uint8 Ibss:1;
    cyg_uint8 CfPollable:1;
    cyg_uint8 CfPollRqst:1;
    cyg_uint8 Privacy:1;
    cyg_uint8 ShortPreamble:1;
    cyg_uint8 Pbcc:1;
    cyg_uint8 ChanAgility:1;
    cyg_uint8 SpectrumMgmt:1;
    cyg_uint8 Rsrvd3:1;
    cyg_uint8 ShortSlotTime:1;
    cyg_uint8 Apsd:1;
    cyg_uint8 Rsvrd2:1;
    cyg_uint8 DSSSOFDM:1;
    cyg_uint8 Rsrvd1:2;
} IEEEtypes_CapInfo_t;

/** IEEEtypes_CfParamSet_t */
typedef __packed struct _IEEEtypes_CfParamSet_t
{
    cyg_uint8 ElementId;
    cyg_uint8 Len;
    cyg_uint8 CfpCnt;
    cyg_uint8 CfpPeriod;
    cyg_uint16 CfpMaxDuration;
    cyg_uint16 CfpDurationRemaining;
} IEEEtypes_CfParamSet_t;

typedef __packed struct IEEEtypes_IbssParamSet_t
{
    cyg_uint8 ElementId;
    cyg_uint8 Len;
    cyg_uint16 AtimWindow;
} IEEEtypes_IbssParamSet_t;

/** IEEEtypes_SsParamSet_t */
typedef __packed union _IEEEtypes_SsParamSet_t
{
    IEEEtypes_CfParamSet_t CfParamSet;
    IEEEtypes_IbssParamSet_t IbssParamSet;
} IEEEtypes_SsParamSet_t;

/** IEEEtypes_FhParamSet_t */
typedef __packed struct _IEEEtypes_FhParamSet_t
{
    cyg_uint8 ElementId;
    cyg_uint8 Len;
    cyg_uint16 DwellTime;
    cyg_uint8 HopSet;
    cyg_uint8 HopPattern;
    cyg_uint8 HopIndex;
} IEEEtypes_FhParamSet_t;

typedef __packed struct _IEEEtypes_DsParamSet_t
{
    cyg_uint8 ElementId;
    cyg_uint8 Len;
    cyg_uint8 CurrentChan;
} IEEEtypes_DsParamSet_t;

/** IEEEtypes_DsParamSet_t */
typedef __packed union IEEEtypes_PhyParamSet_t
{
    IEEEtypes_FhParamSet_t FhParamSet;
    IEEEtypes_DsParamSet_t DsParamSet;
} IEEEtypes_PhyParamSet_t;

typedef cyg_uint16 IEEEtypes_AId_t;
typedef cyg_uint16 IEEEtypes_StatusCode_t;

typedef __packed struct
{
    IEEEtypes_CapInfo_t Capability;
    IEEEtypes_StatusCode_t StatusCode;
    IEEEtypes_AId_t AId;
    cyg_uint8 IEBuffer[1];
} IEEEtypes_AssocRsp_t;

/** TLV  type ID definition */
#define PROPRIETARY_TLV_BASE_ID		0x0100

/* Terminating TLV Type */
#define MRVL_TERMINATE_TLV_ID		0xffff

#define TLV_TYPE_SSID				0x0000
#define TLV_TYPE_RATES				0x0001
#define TLV_TYPE_PHY_FH				0x0002
#define TLV_TYPE_PHY_DS				0x0003
#define TLV_TYPE_CF				    0x0004
#define TLV_TYPE_IBSS				0x0006

#define TLV_TYPE_DOMAIN				0x0007

#define TLV_TYPE_POWER_CAPABILITY	0x0021

#define TLV_TYPE_KEY_MATERIAL       (PROPRIETARY_TLV_BASE_ID + 0)
#define TLV_TYPE_CHANLIST           (PROPRIETARY_TLV_BASE_ID + 1)
#define TLV_TYPE_NUMPROBES          (PROPRIETARY_TLV_BASE_ID + 2)
#define TLV_TYPE_RSSI_LOW           (PROPRIETARY_TLV_BASE_ID + 4)
#define TLV_TYPE_SNR_LOW            (PROPRIETARY_TLV_BASE_ID + 5)
#define TLV_TYPE_FAILCOUNT          (PROPRIETARY_TLV_BASE_ID + 6)
#define TLV_TYPE_BCNMISS            (PROPRIETARY_TLV_BASE_ID + 7)
#define TLV_TYPE_LED_GPIO           (PROPRIETARY_TLV_BASE_ID + 8)
#define TLV_TYPE_LEDBEHAVIOR        (PROPRIETARY_TLV_BASE_ID + 9)
#define TLV_TYPE_PASSTHROUGH        (PROPRIETARY_TLV_BASE_ID + 10)
#define TLV_TYPE_REASSOCAP          (PROPRIETARY_TLV_BASE_ID + 11)
#define TLV_TYPE_POWER_TBL_2_4GHZ   (PROPRIETARY_TLV_BASE_ID + 12)
#define TLV_TYPE_POWER_TBL_5GHZ     (PROPRIETARY_TLV_BASE_ID + 13)
#define TLV_TYPE_BCASTPROBE	    (PROPRIETARY_TLV_BASE_ID + 14)
#define TLV_TYPE_NUMSSID_PROBE	    (PROPRIETARY_TLV_BASE_ID + 15)
#define TLV_TYPE_WMMQSTATUS   	    (PROPRIETARY_TLV_BASE_ID + 16)
#define TLV_TYPE_CRYPTO_DATA	    (PROPRIETARY_TLV_BASE_ID + 17)
#define TLV_TYPE_WILDCARDSSID	    (PROPRIETARY_TLV_BASE_ID + 18)
#define TLV_TYPE_TSFTIMESTAMP	    (PROPRIETARY_TLV_BASE_ID + 19)
#define TLV_TYPE_POWERADAPTCFGEXT   (PROPRIETARY_TLV_BASE_ID + 20)
#define TLV_TYPE_RSSI_HIGH          (PROPRIETARY_TLV_BASE_ID + 22)
#define TLV_TYPE_SNR_HIGH           (PROPRIETARY_TLV_BASE_ID + 23)
#define TLV_TYPE_AUTO_TX	    (PROPRIETARY_TLV_BASE_ID + 24)

/** TLV related data structures*/
/** MrvlIEtypesHeader_t */
typedef __packed struct _MrvlIEtypesHeader
{
    cyg_uint16 Type;
    cyg_uint16 Len;
} MrvlIEtypesHeader_t;

/** MrvlIEtypes_Data_t */
typedef __packed struct _MrvlIEtypes_Data_t
{
    MrvlIEtypesHeader_t Header;
    cyg_uint8 Data[1];
} MrvlIEtypes_Data_t;

/** MrvlIEtypes_RatesParamSet_t */
typedef __packed struct _MrvlIEtypes_RatesParamSet_t
{
    MrvlIEtypesHeader_t Header;
    cyg_uint8 Rates[1];
} MrvlIEtypes_RatesParamSet_t;

/** MrvlIEtypes_SsIdParamSet_t */
typedef __packed struct _MrvlIEtypes_SsIdParamSet_t
{
    MrvlIEtypesHeader_t Header;
    cyg_uint8 SsId[1];
} MrvlIEtypes_SsIdParamSet_t;

/** MrvlIEtypes_WildCardSsIdParamSet_t */
typedef __packed struct _MrvlIEtypes_WildCardSsIdParamSet_t
{
    MrvlIEtypesHeader_t Header;
    cyg_uint8 MaxSsidLength;
    cyg_uint8 SsId[1];
} MrvlIEtypes_WildCardSsIdParamSet_t;

/** ChanScanMode_t */
typedef __packed struct
{
    cyg_uint8 PassiveScan:1;
    cyg_uint8 DisableChanFilt:1;
    cyg_uint8 Reserved_2_7:6;
} ChanScanMode_t;

/** ChanScanParamSet_t */
typedef __packed struct _ChanScanParamSet_t
{
    cyg_uint8 RadioType;
    cyg_uint8 ChanNumber;
    ChanScanMode_t ChanScanMode;
    cyg_uint16 MinScanTime;
    cyg_uint16 MaxScanTime;
} ChanScanParamSet_t;

/** MrvlIEtypes_ChanListParamSet_t */
typedef __packed struct _MrvlIEtypes_ChanListParamSet_t
{
    MrvlIEtypesHeader_t Header;
    ChanScanParamSet_t ChanScanParam[1];
} MrvlIEtypes_ChanListParamSet_t;

/** CfParamSet_t */
typedef __packed struct _CfParamSet_t
{ 
    cyg_uint8 CfpCnt;
    cyg_uint8 CfpPeriod;
    cyg_uint16 CfpMaxDuration;
    cyg_uint16 CfpDurationRemaining;
} CfParamSet_t;

/** IbssParamSet_t */
typedef __packed struct _IbssParamSet_t
{
    cyg_uint16 AtimWindow;
} IbssParamSet_t;

/** MrvlIEtypes_SsParamSet_t */
typedef __packed struct _MrvlIEtypes_SsParamSet_t
{
    MrvlIEtypesHeader_t Header;
    __packed union
    {
        CfParamSet_t CfParamSet[1];
        IbssParamSet_t IbssParamSet[1];
    } cf_ibss;
} MrvlIEtypes_SsParamSet_t;

/** FhParamSet_t */
typedef __packed struct _FhParamSet_t
{
    cyg_uint16 DwellTime;
    cyg_uint8 HopSet;
    cyg_uint8 HopPattern;
    cyg_uint8 HopIndex;
} FhParamSet_t;

/** DsParamSet_t */
typedef __packed struct _DsParamSet_t
{
    cyg_uint8 CurrentChan;
} DsParamSet_t;

/** MrvlIEtypes_PhyParamSet_t */
typedef __packed struct _MrvlIEtypes_PhyParamSet_t
{
    MrvlIEtypesHeader_t Header;
    __packed union
    {
        FhParamSet_t FhParamSet[1];
        DsParamSet_t DsParamSet[1];
    } fh_ds;
} MrvlIEtypes_PhyParamSet_t;

/** MrvlIEtypes_ReassocAp_t */
typedef __packed struct _MrvlIEtypes_ReassocAp_t
{
    MrvlIEtypesHeader_t Header;
    WLAN_802_11_MAC_ADDRESS currentAp;

} MrvlIEtypes_ReassocAp_t;

/** MrvlIEtypes_RsnParamSet_t */
typedef __packed struct _MrvlIEtypes_RsnParamSet_t
{
    MrvlIEtypesHeader_t Header;
    cyg_uint8 RsnIE[1];
} MrvlIEtypes_RsnParamSet_t;

/** MrvlIEtypes_WmmParamSet_t */
typedef __packed struct _MrvlIEtypes_WmmParamSet_t
{
    MrvlIEtypesHeader_t Header;
    cyg_uint8 WmmIE[1];
} MrvlIEtypes_WmmParamSet_t;

typedef __packed struct
{
    MrvlIEtypesHeader_t Header;
    cyg_uint8 QueueIndex;
    cyg_uint8 Disabled;
    cyg_uint8 TriggeredPS;
    cyg_uint8 FlowDirection;
    cyg_uint8 FlowRequired;
    cyg_uint8 FlowCreated;
    cyg_uint32 MediumTime;
} MrvlIEtypes_WmmQueueStatus_t;

typedef __packed struct
{
    MrvlIEtypesHeader_t Header;
    cyg_uint64 tsfTable[1];
} MrvlIEtypes_TsfTimestamp_t;

/**  Local Power Capability */
typedef __packed struct _MrvlIEtypes_PowerCapability_t
{
    MrvlIEtypesHeader_t Header;
    cyg_int8 MinPower;
    cyg_int8 MaxPower;
} MrvlIEtypes_PowerCapability_t;

/** MrvlIEtypes_RssiParamSet_t */
typedef __packed struct _MrvlIEtypes_RssiThreshold_t
{
    MrvlIEtypesHeader_t Header;
    cyg_uint8 RSSIValue;
    cyg_uint8 RSSIFreq;
} MrvlIEtypes_RssiParamSet_t;

/** MrvlIEtypes_SnrThreshold_t */
typedef __packed struct _MrvlIEtypes_SnrThreshold_t
{
    MrvlIEtypesHeader_t Header;
    cyg_uint8 SNRValue;
    cyg_uint8 SNRFreq;
} MrvlIEtypes_SnrThreshold_t;

/** MrvlIEtypes_FailureCount_t */
typedef __packed struct _MrvlIEtypes_FailureCount_t
{
    MrvlIEtypesHeader_t Header;
    cyg_uint8 FailValue;
    cyg_uint8 FailFreq;
} MrvlIEtypes_FailureCount_t;

/** MrvlIEtypes_BeaconsMissed_t */
typedef __packed struct _MrvlIEtypes_BeaconsMissed_t
{
    MrvlIEtypesHeader_t Header;
    cyg_uint8 BeaconMissed;
    cyg_uint8 Reserved;
} MrvlIEtypes_BeaconsMissed_t;

/** MrvlIEtypes_NumProbes_t */
typedef __packed struct _MrvlIEtypes_NumProbes_t
{
    MrvlIEtypesHeader_t Header;
    cyg_uint16 NumProbes;
} MrvlIEtypes_NumProbes_t;

/** MrvlIEtypes_BcastProbe_t */
typedef __packed struct _MrvlIEtypes_BcastProbe_t
{
    MrvlIEtypesHeader_t Header;
    cyg_uint16 BcastProbe;
} MrvlIEtypes_BcastProbe_t;

/** MrvlIEtypes_NumSSIDProbe_t */
typedef __packed struct _MrvlIEtypes_NumSSIDProbe_t
{
    MrvlIEtypesHeader_t Header;
    cyg_uint16 NumSSIDProbe;
} MrvlIEtypes_NumSSIDProbe_t;

typedef __packed struct
{
    cyg_uint8 Led;
    cyg_uint8 Pin;
} Led_Pin;

/** MrvlIEtypes_LedGpio_t */
typedef __packed struct _MrvlIEtypes_LedGpio_t
{
    MrvlIEtypesHeader_t Header;
    Led_Pin LedPin[1];
} MrvlIEtypes_LedGpio_t;

typedef __packed struct _PA_Group_t
{
    cyg_uint16 PowerAdaptLevel;
    cyg_uint16 RateBitmap;
    cyg_uint32 Reserved;
} PA_Group_t;

/** MrvlIEtypes_PA_Group_t */
typedef __packed struct _MrvlIEtypes_PowerAdapt_Group_t
{
    MrvlIEtypesHeader_t Header;
    PA_Group_t PA_Group[MAX_POWER_ADAPT_GROUP];
} MrvlIEtypes_PowerAdapt_Group_t;

typedef __packed struct _AutoTx_MacFrame_t
{
    cyg_uint16 Interval;               /* in seconds */
    cyg_uint8 Priority;                /* User Priority: 0~7, ignored if non-WMM */
    cyg_uint8 Reserved;                /* set to 0 */
    cyg_uint16 FrameLen;               /* Length of MAC frame payload */
    cyg_uint8 DestMacAddr[ETH_ALEN];
    cyg_uint8 SrcMacAddr[ETH_ALEN];
    cyg_uint8 *Payload;
} AutoTx_MacFrame_t;

/** MrvlIEtypes_PA_Group_t */
typedef __packed struct _MrvlIEtypes_AutoTx_t
{
    MrvlIEtypesHeader_t Header;
    AutoTx_MacFrame_t AutoTx_MacFrame;
} MrvlIEtypes_AutoTx_t;

#endif /* _WLAN_TYPES_ */

/** @file wlan_dev.h
 *  @brief This file contains definitions and data structures specific
 *          to Marvell 802.11 NIC. It contains the Device Information
 *          structure wlan_adapter.  
 *
 *  Copyright ?Marvell International Ltd. and/or its affiliates, 2003-2006
 */
/*************************************************************
Change log:
	09/26/05: add Doxygen format comments 
	01/11/06: Conditionalize new scan/join structures.
	04/18/06: Remove old Subscrive Event and add new Subscribe Event
		  implementation through generic hostcmd API
	05/08/06: Remove PermanentAddr from Adapter

 ************************************************************/

#ifndef _WLAN_DEV_H_
#define _WLAN_DEV_H_

#include "cyg/io/eth/eth_drv_stats.h"
#include  "hostcmd.h"

#define	MAX_BSSID_PER_CHANNEL		16

#define MAX_NUM_IN_TX_Q			3

/* For the extended Scan */
#define MAX_EXTENDED_SCAN_BSSID_LIST    MAX_BSSID_PER_CHANNEL * \
						MRVDRV_MAX_CHANNEL_SIZE + 1

typedef struct _PER_CHANNEL_BSSID_LIST_DATA
{
    cyg_uint8 ucStart;
    cyg_uint8 ucNumEntry;
} PER_CHANNEL_BSSID_LIST_DATA, *PPER_CHANNEL_BSSID_LIST_DATA;

typedef struct _MRV_BSSID_IE_LIST
{
    WLAN_802_11_FIXED_IEs FixedIE;
    cyg_uint8 VariableIE[MRVDRV_SCAN_LIST_VAR_IE_SPACE];
} MRV_BSSID_IE_LIST, *PMRV_BSSID_IE_LIST;

#define	MAX_REGION_CHANNEL_NUM	2

/** Chan-Freq-TxPower mapping table*/
typedef struct _CHANNEL_FREQ_POWER
{
        /** Channel Number		*/
    cyg_uint16 Channel;
        /** Frequency of this Channel	*/
    cyg_uint32 Freq;
        /** Max allowed Tx power level	*/
    cyg_uint16 MaxTxPower;
        /** TRUE:channel unsupported;  FLASE:supported*/
    BOOLEAN Unsupported;
} CHANNEL_FREQ_POWER;

/** region-band mapping table*/
typedef struct _REGION_CHANNEL
{
        /** TRUE if this entry is valid		     */
    BOOLEAN Valid;
        /** Region code for US, Japan ...	     */
    cyg_uint8 Region;
        /** Band B/G/A, used for BAND_CONFIG cmd	     */
    cyg_uint8 Band;
        /** Actual No. of elements in the array below */
    cyg_uint8 NrCFP;
        /** chan-freq-txpower mapping table*/
    CHANNEL_FREQ_POWER *CFP;
} REGION_CHANNEL;

typedef struct _wlan_802_11_security_t
{
    BOOLEAN WPAEnabled;
    BOOLEAN WPA2Enabled;
    WLAN_802_11_WEP_STATUS WEPStatus;
    WLAN_802_11_AUTHENTICATION_MODE AuthenticationMode;
    WLAN_802_11_ENCRYPTION_MODE EncryptionMode;
} wlan_802_11_security_t;

/** Current Basic Service Set State Structure */
typedef struct CurrentBSSParams
{

    BSSDescriptor_t BSSDescriptor;
        /** bssid */
    cyg_uint8 bssid[MRVDRV_ETH_ADDR_LEN];
        /** ssid */
    WLAN_802_11_SSID ssid;

        /** band */
    cyg_uint8 band;
        /** channel */
    cyg_uint8 channel;
        /** number of rates supported */
    int NumOfRates;
        /** supported rates*/
    cyg_uint8 DataRates[WLAN_SUPPORTED_RATES];
        /** wmm enable? */
    cyg_uint8 wmm_enabled;
        /** wmm queue priority table*/
    cyg_uint8 wmm_queue_prio[MAX_AC_QUEUES];
        /** uapse enable?*/
    cyg_uint8 wmm_uapsd_enabled;
} CurrentBSSParams;

/** sleep_params */
typedef struct SleepParams
{
    cyg_uint16 sp_error;
    cyg_uint16 sp_offset;
    cyg_uint16 sp_stabletime;
    cyg_uint8 sp_calcontrol;
    cyg_uint8 sp_extsleepclk;
    cyg_uint16 sp_reserved;
} SleepParams;

/** sleep_period */
typedef struct SleepPeriod
{
    cyg_uint16 period;
    cyg_uint16 reserved;
} SleepPeriod;

/** Data structure for the Marvell WLAN device */
typedef struct _wlan_dev
{
        /** device name */
    char name[DEV_NAME_LEN];
        /** card pointer */
    void *card;
        /** IO port */
    cyg_uint32 ioport;
        /** Upload received */
    cyg_uint32 upld_rcv;
        /** Upload type */
    cyg_uint32 upld_typ;
        /** Upload length */
    cyg_uint32 upld_len;
        /** netdev pointer */
    //struct net_device *netdev;
    struct cyg_netdevtab_entry *netdev;
    /* Upload buffer */
    cyg_uint8 upld_buf[WLAN_UPLD_SIZE];
    /* Download sent: 
       bit0 1/0=data_sent/data_tx_done, 
       bit1 1/0=cmd_sent/cmd_tx_done, 
       all other bits reserved 0 */
    cyg_uint8 dnld_sent;

} wlan_dev_t, *pwlan_dev_t;

/** Private structure for the MV device */
struct _wlan_private
{
    int open;

    wlan_adapter *adapter;
    wlan_dev_t wlan_dev;

//    struct net_device_stats stats;
    struct ether_drv_stats  stats;

    struct iw_statistics wstats;
//    struct ether_drv_stats wstats;
 //   struct proc_dir_entry *proc_entry;
 //   struct proc_dir_entry *proc_dev;

        /** thread to service interrupts */
    wlan_thread MainThread;

#ifdef REASSOCIATION
        /** thread to service mac events */
    wlan_thread ReassocThread;
#endif                          /* REASSOCIATION */
	void *priv;
};

/** Wlan Adapter data structure*/
struct _wlan_adapter
{
    /*__align(4)*/ cyg_uint8 TmpTxBuf[WLAN_UPLD_SIZE];
        /** STATUS variables */
    WLAN_HARDWARE_STATUS HardwareStatus;
    cyg_uint32 FWReleaseNumber;
    cyg_uint32 fwCapInfo;
    cyg_uint8 chip_rev;
	cyg_uint8 MainthreadStatus;

        /** Command-related variables */
    cyg_uint16 SeqNum;
    CmdCtrlNode *CmdArray;
        /** Current Command */
    CmdCtrlNode *CurCmd;
    int CurCmdRetCode;

        /** Command Queues */
        /** Free command buffers */
    struct list_head CmdFreeQ;
        /** Pending command buffers */
    struct list_head CmdPendingQ;

        /** Variables brought in from private structure */
    int irq;

        /** Async and Sync Event variables */
    cyg_uint32 IntCounter;
    cyg_uint32 IntCounterSaved;        /* save int for DS/PS */
    cyg_uint32 EventCause;
    cyg_uint8 nodeName[16];            /* nickname */

        /** spin locks */
    //__align(4) spinlock_t QueueSpinLock;

        /** Timers */
    //__align(4) WLAN_DRV_TIMER MrvDrvCommandTimer;
    cyg_bool CommandTimerIsSet;

#ifdef REASSOCIATION
        /**Reassociation timer*/
    cyg_bool TimerIsSet;
    //__align(4) WLAN_DRV_TIMER MrvDrvTimer;
#endif                          /* REASSOCIATION */

        /** Event Queues */
    //wait_queue_head_t ds_awake_q __ATTRIB_ALIGN__;
    //cyg_sem_t ds_awake_q;
    cyg_mutex_t ds_mutex;
	cyg_cond_t ds_cond_q;

    cyg_uint8 HisRegCpy;

#ifdef MFG_CMD_SUPPORT
        /** manf command related cmd variable*/
    cyg_uint32 mfg_cmd_len;
    int mfg_cmd_flag;
    cyg_uint32 mfg_cmd_resp_len;
    cyg_uint8 *mfg_cmd;
    //wait_queue_head_t mfg_cmd_q;
    cyg_mutex_t mfg_mutex;
	cyg_cond_t mfg_cond_q;
#endif

        /** bg scan related variable */
    pHostCmd_DS_802_11_BG_SCAN_CONFIG bgScanConfig;
    cyg_uint32 bgScanConfigSize;

        /** WMM related variable*/
    WMM_DESC wmm;

        /** current ssid/bssid related parameters*/
    CurrentBSSParams CurBssParams;

    WLAN_802_11_NETWORK_INFRASTRUCTURE InfrastructureMode;

    BSSDescriptor_t *pAttemptedBSSDesc;

    WLAN_802_11_SSID AttemptedSSIDBeforeScan;
    WLAN_802_11_SSID PreviousSSID;
    cyg_uint8 PreviousBSSID[MRVDRV_ETH_ADDR_LEN];

    BSSDescriptor_t *ScanTable;
    cyg_uint32 NumInScanTable;

    cyg_uint8 ScanType;
    cyg_uint32 ScanMode;

    cyg_uint16 BeaconPeriod;
    cyg_uint8 AdhocCreate;

        /** Capability Info used in Association, start, join */
    IEEEtypes_CapInfo_t capInfo;

#ifdef REASSOCIATION
        /** Reassociation on and off */
    cyg_bool Reassoc_on;
    //SEMAPHORE ReassocSem;
    cyg_mutex_t	ReassocSem;
#endif                          /* REASSOCIATION */

    cyg_bool ATIMEnabled;

        /** MAC address information */
    cyg_uint8 CurrentAddr[MRVDRV_ETH_ADDR_LEN];
    cyg_uint8 MulticastList[MRVDRV_MAX_MULTICAST_LIST_SIZE]
        [MRVDRV_ETH_ADDR_LEN];
    cyg_uint32 NumOfMulticastMACAddr;

        /** 802.11 statistics */
    HostCmd_DS_802_11_GET_STAT wlan802_11Stat;

    cyg_bool AdHocCreated;
    cyg_bool AdHocFailed;
    cyg_uint16 EnableHwAuto;
    cyg_uint16 RateBitmap;
        /** control G Rates */
    cyg_bool adhoc_grate_enabled;

    WLAN_802_11_ANTENNA TxAntenna;
    WLAN_802_11_ANTENNA RxAntenna;

    cyg_uint8 AdhocChannel;
    WLAN_802_11_FRAGMENTATION_THRESHOLD FragThsd;
    WLAN_802_11_RTS_THRESHOLD RTSThsd;

    cyg_uint32 DataRate;
    cyg_bool Is_DataRate_Auto;

        /** number of association attempts for the current SSID cmd */
    cyg_uint32 m_NumAssociationAttemp;
    cyg_uint16 ListenInterval;
    cyg_uint16 Prescan;
    cyg_uint8 TxRetryCount;

        /** Tx-related variables (for single packet tx) */
    //__align(4) spinlock_t TxSpinLock;
   // struct sk_buff *CurrentTxSkb;
   // struct sk_buff RxSkbQ;
   // struct sk_buff TxSkbQ;
    cyg_uint32 TxSkbNum;
    cyg_bool TxLockFlag;
    cyg_uint16 gen_null_pkg;
   // __align(4) spinlock_t CurrentTxLock;

        /** NIC Operation characteristics */
    cyg_uint32 LinkSpeed;
    cyg_uint16 CurrentPacketFilter;
    cyg_uint32 MediaConnectStatus;
    cyg_uint16 RegionCode;
    cyg_uint16 RegionTableIndex;
    cyg_uint16 TxPowerLevel;
    cyg_uint8 MaxTxPowerLevel;
    cyg_uint8 MinTxPowerLevel;

        /** POWER MANAGEMENT AND PnP SUPPORT */
    cyg_bool SurpriseRemoved;
    cyg_uint16 AtimWindow;

    cyg_uint16 PSMode;                 /* Wlan802_11PowerModeCAM=disable
                                   Wlan802_11PowerModeMAX_PSP=enable */
    cyg_uint16 MultipleDtim;
    cyg_uint32 PSState;
    cyg_bool NeedToWakeup;

    PS_CMD_ConfirmSleep PSConfirmSleep;
    cyg_uint16 LocalListenInterval;
    cyg_uint16 NullPktInterval;
    cyg_uint16 AdhocAwakePeriod;
    cyg_uint16 fwWakeupMethod;
    cyg_bool IsDeepSleep;
        /** Host wakeup parameter */
    cyg_bool bWakeupDevRequired;
    cyg_bool bHostSleepConfigured;
    cyg_uint8 HSCfg_Gap;
    cyg_uint32 WakeupTries;
    cyg_bool HS_Activated; //JONO

        /** Encryption parameter */
    wlan_802_11_security_t SecInfo;

    MRVL_WEP_KEY WepKey[MRVL_NUM_WEP_KEY];
    cyg_uint16 CurrentWepKeyIndex;
    cyg_uint8 mrvlTlvBuffer[256];
    cyg_uint8 mrvlTlvBufferLen;

    cyg_uint8 assocRspBuffer[MRVDRV_ASSOC_RSP_BUF_SIZE];
    int assocRspSize;
    cyg_uint8 genIeBuffer[256];
    cyg_uint8 genIeBufferLen;
    WLAN_802_11_ENCRYPTION_STATUS EncryptionStatus;

    cyg_bool IsGTK_SET;

        /** Encryption Key*/
    cyg_uint8 Wpa_ie[256];
    cyg_uint8 Wpa_ie_len;

    MRVL_WPA_KEY WpaPwkKey, WpaGrpKey;

    HostCmd_DS_802_11_KEY_MATERIAL aeskey;

    /* Advanced Encryption Standard */
    cyg_bool AdhocAESEnabled;
    //__align(4) wait_queue_head_t cmd_EncKey;
    cyg_mutex_t cmdEncKey_mutex;
	cyg_cond_t cmdEncKey_cond_q;

    cyg_uint16 RxAntennaMode;
    cyg_uint16 TxAntennaMode;

        /** Requested Signal Strength*/
    cyg_uint16 bcn_avg_factor;
    cyg_uint16 data_avg_factor;
    cyg_uint16 SNR[MAX_TYPE_B][MAX_TYPE_AVG];
    cyg_uint16 NF[MAX_TYPE_B][MAX_TYPE_AVG];
    cyg_uint8 RSSI[MAX_TYPE_B][MAX_TYPE_AVG];
    cyg_uint8 rawSNR[DEFAULT_DATA_AVG_FACTOR];
    cyg_uint8 rawNF[DEFAULT_DATA_AVG_FACTOR];
    cyg_uint16 nextSNRNF;
    cyg_uint16 numSNRNF;
    cyg_uint32 RxPDAge;
    cyg_uint16 RxPDRate;

    cyg_bool RadioOn;
    cyg_uint32 Preamble;

        /** Blue Tooth Co-existence Arbitration */
    HostCmd_DS_802_11_BCA_TIMESHARE bca_ts;

        /** sleep_params */
    SleepParams sp;

        /** sleep_period (Enhanced Power Save) */
    SleepPeriod sleep_period;

        /** RF calibration data */
    HostCmd_DS_COMMAND caldataext;

#define	MAX_REGION_CHANNEL_NUM	2
        /** Region Channel data */
    REGION_CHANNEL region_channel[MAX_REGION_CHANNEL_NUM];

    REGION_CHANNEL universal_channel[MAX_REGION_CHANNEL_NUM];

        /** 11D and Domain Regulatory Data */
    wlan_802_11d_domain_reg_t DomainReg;
    parsed_region_chan_11d_t parsed_region_chan;

        /** FSM variable for 11d support */
    wlan_802_11d_state_t State11D;
    int reassocAttempt;
    WLAN_802_11_MAC_ADDRESS reassocCurrentAp;
    cyg_uint8 beaconBuffer[MAX_SCAN_BEACON_BUFFER];
    cyg_uint8 *pBeaconBufEnd;

        /**	MISCELLANEOUS */
    /* Card Information Structure */
    cyg_uint8 CisInfoBuf[512];
    cyg_uint16 CisInfoLen;
    cyg_uint8 *pRdeeprom;
    wlan_offset_value OffsetValue;

    //wait_queue_head_t cmd_get_log;
    cyg_mutex_t cmdget_mutex;
	cyg_cond_t cmdget_cond_q;

    HostCmd_DS_802_11_GET_LOG LogMsg;
    cyg_uint16 ScanProbes;

    cyg_uint32 PktTxCtrl;

    cyg_uint8 *helper;
    cyg_uint32 helper_len;
    cyg_uint8 *fmimage;
    cyg_uint32 fmimage_len;
    cyg_uint16 TxRate;

};

#endif /* _WLAN_DEV_H_ */

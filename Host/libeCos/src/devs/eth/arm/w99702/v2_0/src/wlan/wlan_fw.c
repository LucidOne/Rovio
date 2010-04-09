/** @file wlan_fw.c
  * @brief This file contains the initialization for FW
  * and HW 
  * 
  *  Copyright ?Marvell International Ltd. and/or its affiliates, 2003-2006
  */
/********************************************************
Change log:
	09/28/05: Add Doxygen format comments
	01/05/06: Add kernel 2.6.x support	
	01/11/06: Conditionalize new scan/join functions.
	          Cleanup association response handler initialization.
	01/06/05: Add FW file read
	05/08/06: Remove the 2nd GET_HW_SPEC command and TempAddr/PermanentAddr
	06/30/06: replaced MODULE_PARM(name, type) with module_param(name, type, perm)

********************************************************/

#include	"include.h"
#include	"sys/malloc.h"

/********************************************************
		Local Variables
********************************************************/

char *helper_name = NULL;
char *fw_name = NULL;

//module_param(helper_name, charp, 0);
//module_param(fw_name, charp, 0);

#ifdef MFG_CMD_SUPPORT
int mfgmode = 0;
module_param(mfgmode, int, 0);
#endif

/********************************************************
		Global Variables
********************************************************/

/********************************************************
		Local Functions
********************************************************/

void
fw_buffer_free(cyg_uint8 * addr)
{
    FREE(addr, 0);
}



/** 
 *  @brief This function downloads firmware image, gets
 *  HW spec from firmware and set basic parameters to
 *  firmware.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
wlan_setup_station_hw(wlan_private * priv)
{
    int ret = WLAN_STATUS_SUCCESS;
    wlan_adapter *adapter = priv->adapter;
    cyg_uint8 *ptr = NULL;
    cyg_uint32 len = 0;

    ENTER();

    sbi_disable_host_int(priv);

#if 0
    diag_printf("helper:%s fw:%s \n", helper_name, fw_name);
    adapter->fmimage = NULL;
    adapter->fmimage_len = 0;
    adapter->helper = NULL;
    adapter->helper_len = 0;

    if (helper_name != NULL) {
        if (fw_read(helper_name, &ptr, &len) != WLAN_STATUS_FAILURE) {
            adapter->helper = ptr;
            adapter->helper_len = len;
            diag_printf("helper read success:%x len=%x\n",
                   (unsigned int) ptr, len);
        } else {
            diag_printf("helper read fail.\n");
            ret = WLAN_STATUS_FAILURE;
            goto done;
        }
    }

    if (fw_name != NULL) {
        if (fw_read(fw_name, &ptr, &len) != WLAN_STATUS_FAILURE) {
            adapter->fmimage = ptr;
            adapter->fmimage_len = len;
            diag_printf("fw read success:%x len=%x\n", (unsigned int) ptr,
                   len);
        } else {
            diag_printf("fw read fail.\n");
            ret = WLAN_STATUS_FAILURE;
            goto done;
        }
    }
#endif
	HAL_ENABLE_INTERRUPTS();//clyu

    /* Download the helper */
    ret = sbi_prog_helper(priv);

    if (ret) {
        diag_printf("Bootloader in invalid state!\n");
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }
    /* Download the main firmware via the helper firmware */
    if (sbi_prog_firmware_w_helper(priv)) {
        diag_printf("Wlan Firmware download failed!\n");
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    if (adapter->helper != NULL) {
        fw_buffer_free(adapter->helper);
        diag_printf("vfree helper\n");
    }
    if (adapter->fmimage != NULL) {
        fw_buffer_free(adapter->fmimage);
        diag_printf("vfree fmimage\n");
    }

    /* check if the fimware is downloaded successfully or not */
    if (sbi_verify_fw_download(priv)) {
        diag_printf("Firmware failed to be active in time!\n");
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }
#define RF_REG_OFFSET 0x07
#define RF_REG_VALUE  0xc8

    sbi_enable_host_int(priv);

#ifdef MFG_CMD_SUPPORT
    diag_printf("mfgmode=%d\n", mfgmode);
    if (mfgmode == 0) {
#endif

        /*
         * Read MAC address from HW
         */
        memset(adapter->CurrentAddr, 0xff, MRVDRV_ETH_ADDR_LEN);

        ret = PrepareAndSendCommand(priv, HostCmd_CMD_GET_HW_SPEC,
                                    0, HostCmd_OPTION_WAITFORRSP, 0, NULL);

        if (ret) {
            ret = WLAN_STATUS_FAILURE;
            goto done;
        }
		
#if 1
	    //clyu
		if((adapter->CurrentAddr[0] != 0x00) || (adapter->CurrentAddr[1] != 0x01) || (adapter->CurrentAddr[2] != 0x36))
		{
		#if 1
			adapter->CurrentAddr[0] = 0x00;
			adapter->CurrentAddr[1] = 0x00;
			adapter->CurrentAddr[2] = 0x00;
			ret = PrepareAndSendCommand(priv, HostCmd_CMD_802_11_MAC_ADDRESS,
                                HostCmd_ACT_SET,
                                HostCmd_OPTION_WAITFORRSP, 0, NULL);

		#else
			ret = WLAN_STATUS_FAILURE;
		#endif
	        if (ret) {
    	        ret = WLAN_STATUS_FAILURE;
        	    goto done;
	        }
		}
    	//end
#endif
		
        SetMacPacketFilter(priv);

        ret = PrepareAndSendCommand(priv,
                                    HostCmd_CMD_802_11_FW_WAKEUP_METHOD,
                                    HostCmd_ACT_GET,
                                    HostCmd_OPTION_WAITFORRSP, 0,
                                    &priv->adapter->fwWakeupMethod);

        if (ret) {
            ret = WLAN_STATUS_FAILURE;
            goto done;
        }
        else
        	diag_printf("FW_WAKEUP_METHOD %d\n", priv->adapter->fwWakeupMethod);
#ifdef MFG_CMD_SUPPORT
    }
#endif

#ifdef MFG_CMD_SUPPORT
    if (mfgmode == 0) {
#endif
/*   Modify by Cybertan King
		 priv->adapter->RateBitmap = 0x1000;
		ret = PrepareAndSendCommand(priv,
                                    HostCmd_CMD_802_11_RATE_ADAPT_RATESET,
                                    HostCmd_ACT_GEN_SET,
                                    HostCmd_OPTION_WAITFORRSP, 0, NULL);
		
		if (ret) {
            ret = WLAN_STATUS_FAILURE;
            goto done;
        }
        ret = PrepareAndSendCommand(priv,
                                    HostCmd_CMD_802_11_RATE_ADAPT_RATESET,
                                    HostCmd_ACT_GEN_GET,
                                    HostCmd_OPTION_WAITFORRSP, 0, NULL);
        if (ret) {
            ret = WLAN_STATUS_FAILURE;
            goto done;
        }
*/         
              
        priv->adapter->DataRate = 0;
        ret = PrepareAndSendCommand(priv,
                                    HostCmd_CMD_802_11_RF_TX_POWER,
                                    HostCmd_ACT_GEN_GET,
                                    HostCmd_OPTION_WAITFORRSP, 0, NULL);
		
        if (ret) {
            ret = WLAN_STATUS_FAILURE;
            goto done;
        }
	#if 0
		/* Get the supported Data Rates */   //Cybertan King Add
		ret = PrepareAndSendCommand(priv, HostCmd_CMD_802_11_DATA_RATE,
			HostCmd_ACT_GET_TX_RATE,
			HostCmd_OPTION_WAITFORRSP,
			0, NULL);

		if (ret) {
			ret = WLAN_STATUS_FAILURE;
			goto done;
		}
	#endif
        
#ifdef MFG_CMD_SUPPORT
    }
#endif

    ret = WLAN_STATUS_SUCCESS;
  done:
    LEAVE();

    return (ret);
}

/** 
 *  @brief This function initializes timers.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   n/a
 */
static void
init_sync_objects(wlan_private * priv)
{
    wlan_adapter *Adapter = priv->adapter;

#if 0

    InitializeTimer(&Adapter->MrvDrvCommandTimer,
                    MrvDrvCommandTimerFunction, priv);
    Adapter->CommandTimerIsSet = FALSE;

#ifdef REASSOCIATION
    /* Initialize the timer for the reassociation */
    InitializeTimer(&Adapter->MrvDrvTimer, MrvDrvTimerFunction, priv);
    Adapter->TimerIsSet = FALSE;
#endif /* REASSOCIATION */


#else

	WirelessDrvCmdAlarm(priv);
#ifdef REASSOCIATION
	WirelessDrvAlarm(priv);
#endif

	
#endif

    return;
}

/** 
 *  @brief This function allocates buffer for the member of adapter
 *  structure like command buffer and BSSID list.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_allocate_adapter(wlan_private * priv)
{
    cyg_uint32 ulBufSize;
    wlan_adapter *Adapter = priv->adapter;

    BSSDescriptor_t *pTempScanTable;

    /* Allocate buffer to store the BSSID list */
    ulBufSize = sizeof(BSSDescriptor_t) * MRVDRV_MAX_BSSID_LIST;
    MALLOC(pTempScanTable, BSSDescriptor_t *, ulBufSize, 0, M_NOWAIT);
    if(!pTempScanTable)
    {
        wlan_free_adapter(priv);
        return WLAN_STATUS_FAILURE;
    }

    Adapter->ScanTable = (BSSDescriptor_t*)((cyg_uint32)pTempScanTable | NON_CACHE_FLAG);
    memset(Adapter->ScanTable, 0, ulBufSize);

   // spin_lock_init(&Adapter->QueueSpinLock);

    /* Allocate the command buffers */
    AllocateCmdBuffer(priv);

    memset((void*)&Adapter->PSConfirmSleep, 0, sizeof(PS_CMD_ConfirmSleep));
    Adapter->SeqNum++;
    Adapter->PSConfirmSleep.SeqNum = wlan_cpu_to_le16(Adapter->SeqNum);
    Adapter->PSConfirmSleep.Command =
        wlan_cpu_to_le16(HostCmd_CMD_802_11_PS_MODE);
    Adapter->PSConfirmSleep.Size =
        wlan_cpu_to_le16(sizeof(PS_CMD_ConfirmSleep));
    Adapter->PSConfirmSleep.Result = 0;
    Adapter->PSConfirmSleep.Action =
        wlan_cpu_to_le16(HostCmd_SubCmd_Sleep_Confirmed);

    return WLAN_STATUS_SUCCESS;
}

/**
 *  @brief This function initializes the adapter structure
 *  and set default value to the member of adapter.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   n/a
 */
static void
wlan_init_adapter(wlan_private * priv)
{
    wlan_adapter *Adapter = priv->adapter;
    int i;
        /**
	 * Default configuration for BG scan command
	 */
    static cyg_uint8 dfltBgScanCfg[] = {
        0x01, 0x00,             //Action
        0x00,                   //Enable
        0x03,                   //BssType
        0x0E,                   //ChannelsPerScan
        0x00,                   //DiscardWhenFull
        0x00, 0x00,             //Reserved
        0x64, 0x00, 0x00, 0x00, //Scan Interval                         
        0x01, 0x00, 0x00, 0x00, //StoreCondition
        0x01, 0x00, 0x00, 0x00, //ReportConditions
        0x0E, 0x00,             //MaxScanResults

        0x01, 0x01, 0x4d, 0x00, //ChannelList  
        0x00, 0x0b, 0x00, 0x06, 0x00, 0x64, 0x00,
        0x00, 0x01, 0x00, 0x06, 0x00, 0x64, 0x00,
        0x00, 0x02, 0x00, 0x06, 0x00, 0x64, 0x00,
        0x00, 0x03, 0x00, 0x06, 0x00, 0x64, 0x00,
        0x00, 0x04, 0x00, 0x06, 0x00, 0x64, 0x00,
        0x00, 0x05, 0x00, 0x06, 0x00, 0x64, 0x00,
        0x00, 0x06, 0x00, 0x06, 0x00, 0x64, 0x00,
        0x00, 0x07, 0x00, 0x06, 0x00, 0x64, 0x00,
        0x00, 0x08, 0x00, 0x06, 0x00, 0x64, 0x00,
        0x00, 0x09, 0x00, 0x06, 0x00, 0x64, 0x00,
        0x00, 0x0a, 0x00, 0x06, 0x00, 0x64, 0x00
    };

	diag_printf("Before set GPIO for WiFi: %08x %08x\n", inpw(REG_GPIO_OE), inpw(REG_GPIO_DAT));
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & 0xFFFFF7FF);		/* Set GPIO11 output */
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x00000800);	/* Set GPIO11 high */
	diag_printf("Before set GPIO for WiFi: %08x %08x\n", inpw(REG_GPIO_OE), inpw(REG_GPIO_DAT));

    Adapter->ScanProbes = 0;

    Adapter->bcn_avg_factor = DEFAULT_BCN_AVG_FACTOR;
    Adapter->data_avg_factor = DEFAULT_DATA_AVG_FACTOR;

    /* ATIM params */
    Adapter->AtimWindow = 0;
    Adapter->ATIMEnabled = FALSE;

    Adapter->MediaConnectStatus = WlanMediaStateDisconnected;
    Adapter->LinkSpeed = MRVDRV_LINK_SPEED_1mbps;
    memset(Adapter->CurrentAddr, 0xff, MRVDRV_ETH_ADDR_LEN);

    /* Status variables */
    Adapter->HardwareStatus = WlanHardwareStatusInitializing;

    /* scan type */
    Adapter->ScanType = HostCmd_SCAN_TYPE_ACTIVE;

    /* scan mode */
    Adapter->ScanMode = HostCmd_BSS_TYPE_ANY;

    /* 802.11 specific */
    Adapter->SecInfo.WEPStatus = Wlan802_11WEPDisabled;
    for (i = 0; i < sizeof(Adapter->WepKey) / sizeof(Adapter->WepKey[0]); i++)
        memset((void*)&Adapter->WepKey[i], 0, sizeof(MRVL_WEP_KEY));
    Adapter->CurrentWepKeyIndex = 0;
    Adapter->SecInfo.AuthenticationMode = Wlan802_11AuthModeOpen;
    Adapter->SecInfo.EncryptionMode = CIPHER_NONE;
    Adapter->AdhocAESEnabled = FALSE;
    Adapter->InfrastructureMode = Wlan802_11Infrastructure;

    Adapter->NumInScanTable = 0;
    Adapter->pAttemptedBSSDesc = NULL;
#ifdef REASSOCIATION
    //OS_INIT_SEMAPHORE(&Adapter->ReassocSem);
    cyg_mutex_init(&Adapter->ReassocSem);
#endif
    Adapter->pBeaconBufEnd = Adapter->beaconBuffer;

    Adapter->Prescan = CMD_ENABLED;
    Adapter->HisRegCpy |= HIS_TxDnLdRdy;

    memset(&Adapter->CurBssParams, 0, sizeof(Adapter->CurBssParams));

    /* PnP and power profile */
    Adapter->SurpriseRemoved = FALSE;

    Adapter->CurrentPacketFilter =
        HostCmd_ACT_MAC_RX_ON | HostCmd_ACT_MAC_TX_ON;

    Adapter->RadioOn = RADIO_ON;
#ifdef REASSOCIATION
    Adapter->Reassoc_on = TRUE;
#endif /* REASSOCIATION */
    Adapter->TxAntenna = RF_ANTENNA_2;
    Adapter->RxAntenna = RF_ANTENNA_AUTO;

    Adapter->EnableHwAuto = TRUE;
    Adapter->Is_DataRate_Auto = TRUE;
    Adapter->BeaconPeriod = MRVDRV_BEACON_INTERVAL;

    // set default value of capInfo.
#define SHORT_PREAMBLE_ALLOWED		1
    memset((void*)&Adapter->capInfo, 0, sizeof(Adapter->capInfo));
    Adapter->capInfo.ShortPreamble = SHORT_PREAMBLE_ALLOWED;

    Adapter->AdhocChannel = DEFAULT_AD_HOC_CHANNEL;

    Adapter->PSMode = Wlan802_11PowerModeCAM;
    //Adapter->PSMode = Wlan802_11PowerModeMAX_PSP;	//xhchen according CP's suggest
    
    Adapter->MultipleDtim = MRVDRV_DEFAULT_MULTIPLE_DTIM;

    Adapter->ListenInterval = MRVDRV_DEFAULT_LISTEN_INTERVAL;

    Adapter->PSState = PS_STATE_FULL_POWER;
    Adapter->NeedToWakeup = FALSE;
    Adapter->LocalListenInterval = 0;   /* default value in firmware will be used */
    Adapter->fwWakeupMethod = WAKEUP_FW_UNCHANGED;

    Adapter->IsDeepSleep = FALSE;

    Adapter->bWakeupDevRequired = FALSE;
    Adapter->bHostSleepConfigured = FALSE;
    Adapter->WakeupTries = 0;

    Adapter->DataRate = 0;      // Initially indicate the rate as auto 

    Adapter->adhoc_grate_enabled = FALSE;

    Adapter->IntCounter = Adapter->IntCounterSaved = 0;
    memset((void*)&Adapter->wmm, 0, sizeof(WMM_DESC));
//    for (i = 0; i < MAX_AC_QUEUES; i++)
//        INIT_LIST_HEAD((struct list_head *) &Adapter->wmm.TxSkbQ[i]);
    Adapter->gen_null_pkg = TRUE;       /*Enable NULL Pkg generation */

   // INIT_LIST_HEAD((struct list_head *) &Adapter->RxSkbQ);
    Adapter->gen_null_pkg = TRUE;       /*Enable NULL Pkg generation */

   // INIT_LIST_HEAD((struct list_head *) &Adapter->TxSkbQ);
    Adapter->TxSkbNum = 0;
    MALLOC(Adapter->bgScanConfig,pHostCmd_DS_802_11_BG_SCAN_CONFIG ,sizeof(dfltBgScanCfg) , 0, M_NOWAIT);
    memcpy((void*)Adapter->bgScanConfig, dfltBgScanCfg, sizeof(dfltBgScanCfg));
    Adapter->bgScanConfigSize = sizeof(dfltBgScanCfg);

    //init_waitqueue_head(&Adapter->cmd_EncKey);
    cyg_cond_init(&Adapter->cmdEncKey_cond_q, &Adapter->cmdEncKey_mutex);
    

    Adapter->EncryptionStatus = Wlan802_11WEPDisabled;

   // spin_lock_init(&Adapter->CurrentTxLock);

//    Adapter->CurrentTxSkb = NULL;
    Adapter->PktTxCtrl = 0;

    return;
}

/********************************************************
		Global Functions
********************************************************/

/** 
 *  @brief This function initializes firmware
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
wlan_init_fw(wlan_private * priv)
{
    int ret = WLAN_STATUS_SUCCESS;
    wlan_adapter *Adapter = priv->adapter;

    ENTER();

    /* Allocate adapter structure */
    if ((ret = wlan_allocate_adapter(priv)) != WLAN_STATUS_SUCCESS) {
        goto done;
    }

    /* init adapter structure */
    wlan_init_adapter(priv);

    /* init timer etc. */
    init_sync_objects(priv);

    /* download fimrware etc. */
    if ((ret = wlan_setup_station_hw(priv)) != WLAN_STATUS_SUCCESS) {
        Adapter->HardwareStatus = WlanHardwareStatusNotReady;
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }
    /* init 802.11d */
    wlan_init_11d(priv);

    Adapter->HardwareStatus = WlanHardwareStatusReady;
    ret = WLAN_STATUS_SUCCESS;
  done:
    LEAVE();
    return ret;
}

/** 
 *  @brief This function frees the structure of adapter
 *    
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   n/a
 */
void
wlan_free_adapter(wlan_private * priv)
{
    wlan_adapter *Adapter = priv->adapter;

    ENTER();

    if (!Adapter) {
        diag_printf("Why double free adapter?:)\n");
        return;
    }

    diag_printf("Free Command buffer\n");
    FreeCmdBuffer(priv);

    diag_printf("Free CommandTimer\n");
    if (Adapter->CommandTimerIsSet) {
        //CancelTimer(&Adapter->MrvDrvCommandTimer);
        cyg_alarm_disable(cmd_alarm_handle);
        Adapter->CommandTimerIsSet = FALSE;
    }
    //FreeTimer(&Adapter->MrvDrvCommandTimer);
    cyg_alarm_delete(cmd_alarm_handle);
#ifdef REASSOCIATION
    diag_printf("Free MrvDrvTimer\n");
    if (Adapter->TimerIsSet) {
        //CancelTimer(&Adapter->MrvDrvTimer);
        cyg_alarm_disable(reassociation_alarm_handle);
		
        Adapter->TimerIsSet = FALSE;
    }
    //FreeTimer(&Adapter->MrvDrvTimer);
    cyg_alarm_delete(reassociation_alarm_handle);
#endif /* REASSOCIATION */

    if (Adapter->bgScanConfig) {
        FREE(Adapter->bgScanConfig, 0);
        Adapter->bgScanConfig = NULL;
    }

    //OS_FREE_LOCK(&Adapter->CurrentTxLock);
    //OS_FREE_LOCK(&Adapter->QueueSpinLock);

    diag_printf("Free ScanTable\n");
    if (Adapter->ScanTable) {
        FREE(Adapter->ScanTable, 0);
        Adapter->ScanTable = NULL;
    }

    diag_printf("Free Adapter\n");

    /* Free the adapter object itself */
    FREE(Adapter, 0);
    priv->adapter = NULL;
    LEAVE();
}

/** 
 *  @brief This function handles the timeout of command sending.
 *  It will re-send the same command again.
 *  
 *  @param FunctionContext    A pointer to FunctionContext
 *  @return 	   n/a
 */

extern cyg_uint32 _uSDIO_cmd_send;

void
MrvDrvCommandTimerFunction(cyg_handle_t alarm, cyg_addrword_t data)
{
    wlan_private *priv = (wlan_private *) data;
    wlan_adapter *Adapter = priv->adapter;
    CmdCtrlNode *pTempNode;
    ulong flags;

    ENTER();

    diag_printf("MrvDrvCommandTimer fired.\n");
    
    if(_uSDIO_cmd_send)
    {
    	_uSDIO_cmd_send = 0;
    	sdio_interrupt();
    }
    else
    {
    
	    Adapter->CommandTimerIsSet = FALSE;

    	pTempNode = Adapter->CurCmd;

	    if (pTempNode == NULL) {
    	    diag_printf("PTempnode Empty\n");
        	return;
	    }

#if 1	// HPChen
    	//spin_lock_irqsave(&Adapter->QueueSpinLock, flags);
	    OS_INT_DISABLE(flags);
    
    	Adapter->CurCmd = NULL;
    
	    OS_INT_RESTORE(flags);
	   // spin_unlock_irqrestore(&Adapter->QueueSpinLock, flags);
	
	
    	if(!Adapter->SurpriseRemoved)
	    {

		    diag_printf("Re-sending same command as it timeout...!\n");
    		QueueCmd(Adapter, pTempNode, FALSE);
	    }

    	//wake_up_interruptible(&priv->MainThread.waitQ);

		cyg_flag_setbits( &priv->MainThread.waitQ_flag_q, 1 );
	}
#endif

    LEAVE();
    return;
}

#ifdef REASSOCIATION
/** 
 *  @brief This function triggers re-association by waking up
 *  re-assoc thread.
 *  
 *  @param FunctionContext    A pointer to FunctionContext
 *  @return 	   n/a
 */
void
MrvDrvTimerFunction(cyg_handle_t alarm, cyg_addrword_t data)
{
    wlan_private *priv = (wlan_private *) data;
    wlan_adapter *Adapter = priv->adapter;
    //OS_INTERRUPT_SAVE_AREA;

    ENTER();

    diag_printf("MrvDrvTimer fired %x.\n", priv);
    Adapter->TimerIsSet = FALSE;
    if (Adapter->PSState != PS_STATE_FULL_POWER) {
        /* wait until Exit_PS command returns */
        Adapter->TimerIsSet = TRUE;
        //ModTimer(&Adapter->MrvDrvTimer, MRVDRV_TIMER_1S);
		cyg_alarm_initialize(reassociation_alarm_handle, cyg_current_time()+100, 0);

        diag_printf("MrvDrvTimerFunction(PSState=%d) waiting"
               "for Exit_PS done\n", Adapter->PSState);
        LEAVE();
        return;
    }

    diag_printf("Waking Up the Event Thread\n");

    //wake_up_interruptible(&priv->ReassocThread.waitQ);
    
	cyg_flag_setbits( &priv->ReassocThread.waitQ_flag_q, 1 ); 

    LEAVE();
    return;
}
#endif /* REASSOCIATION */

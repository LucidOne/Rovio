/** @file wlan_cmd.c
  *  
  * @brief This file contains the handling of command.
  * it prepares command and sends it to firmware when
  * it is ready.
  * 
  *  Copyright ?Marvell International Ltd. and/or its affiliates, 2003-2006
  * 
  */
/********************************************************
Change log:
    10/04/05: Add Doxygen format comments
    01/05/06: Add kernel 2.6.x support  
    01/11/06: Change compile flag BULVERDE_SDIO to SD to support
              Monahans/Zylonite
    01/11/06: Conditionalize new scan/join structures
    01/31/06: Add support to selectively enabe the FW Scan channel filter
    02/16/06: Clear scan in progress flag when scan command failed and dropped
    04/06/06: Add TSPEC, queue metrics, and MSDU expiry support
    04/18/06: Remove old Subscrive Event and add new Subscribe Event
	      implementation through generic hostcmd API
    05/04/06: Add IBSS coalescing related new hostcmd handling	      
********************************************************/

#include	"include.h"
#include	"sys/malloc.h"

cyg_handle_t cmd_alarm_handle;
static cyg_alarm cmd_alarm;
cyg_handle_t reassociation_alarm_handle;
static cyg_alarm reassociation_alarm;



/********************************************************
		Local Variables
********************************************************/

static cyg_uint16 Commands_Allowed_In_PS[] = {
    HostCmd_CMD_802_11_RSSI,
    HostCmd_CMD_802_11_HOST_SLEEP_CFG, //JONO
    HostCmd_CMD_802_11_HOST_SLEEP_AWAKE_CONFIRM,
};

/********************************************************
		Global Variables
********************************************************/

/********************************************************
		Local Functions
********************************************************/

/** 
 *  @brief This function checks if the commans is allowed
 *  in PS mode not.
 *  
 *  @param Command the command ID
 *  @return 	   TRUE or FALSE
 */
static BOOLEAN
Is_Command_Allowed_In_PS(cyg_uint16 Command)
{
    int count = sizeof(Commands_Allowed_In_PS)
        / sizeof(Commands_Allowed_In_PS[0]);
    int i;

    for (i = 0; i < count; i++) {
        if (Command == wlan_cpu_to_le16(Commands_Allowed_In_PS[i]))
            return TRUE;
    }

    return FALSE;
}

/** 
 *  @brief This function prepares command of get_hw_spec.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @param cmd	   A pointer to HostCmd_DS_COMMAND structure
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_hw_spec(wlan_private * priv, HostCmd_DS_COMMAND * cmd)
{
    HostCmd_DS_GET_HW_SPEC *hwspec = &cmd->params.hwspec;

    ENTER();

    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_GET_HW_SPEC);
    cmd->Size = wlan_cpu_to_le16(sizeof(HostCmd_DS_GET_HW_SPEC) + S_DS_GEN);
    memcpy((void *)hwspec->PermanentAddr, priv->adapter->CurrentAddr, ETH_ALEN);

    LEAVE();
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function prepares command of ps_mode.
 *  
 *  @param priv    	A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @param cmd_action 	the action: GET or SET
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_ps_mode(wlan_private * priv,
                        HostCmd_DS_COMMAND * cmd, cyg_uint16 cmd_action)
{
    HostCmd_DS_802_11_PS_MODE *psm = &cmd->params.psmode;
    cyg_uint16 Action = cmd_action;
    wlan_adapter *Adapter = priv->adapter;

    ENTER();

    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_PS_MODE);
    cmd->Size =
        wlan_cpu_to_le16(sizeof(HostCmd_DS_802_11_PS_MODE) + S_DS_GEN);
    psm->Action = wlan_cpu_to_le16(cmd_action);
    psm->MultipleDtim = 0;
    switch (Action) {
    case HostCmd_SubCmd_Enter_PS:
        WLAN_DEBUG_PRINTF("PS Command:" "SubCode- Enter PS\n");
        WLAN_DEBUG_PRINTF("LocalListenInterval = %d\n",
               Adapter->LocalListenInterval);

        psm->LocalListenInterval =
            wlan_cpu_to_le16(Adapter->LocalListenInterval);
        psm->NullPktInterval = wlan_cpu_to_le16(Adapter->NullPktInterval);
        psm->MultipleDtim = wlan_cpu_to_le16(priv->adapter->MultipleDtim);
        if (priv->adapter->InfrastructureMode == Wlan802_11IBSS)
            psm->AdhocAwakePeriod =
                wlan_cpu_to_le16(priv->adapter->AdhocAwakePeriod);
        break;

    case HostCmd_SubCmd_Exit_PS:
        WLAN_DEBUG_PRINTF("PS Command:" "SubCode- Exit PS\n");
        break;

    case HostCmd_SubCmd_Sleep_Confirmed:
        WLAN_DEBUG_PRINTF("PS Command: SubCode- sleep confirm\n");
        break;

    default:
        break;
    }

    LEAVE();
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function prepares command of fw_wakeup_method.
 *  
 *  @param priv    	A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @param cmd_action 	the action: GET or SET
 *  @param pdata_buf 	A pointer to data buffer
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_fw_wakeup_method(wlan_private * priv,
                                 HostCmd_DS_COMMAND * cmd,
                                 int cmd_action, void *pdata_buf)
{
    HostCmd_DS_802_11_FW_WAKEUP_METHOD *fwwm = &cmd->params.fwwakeupmethod;
    cyg_uint16 action = (cyg_uint16) cmd_action;
    cyg_uint16 method = *((cyg_uint16 *) pdata_buf);

    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_FW_WAKEUP_METHOD);
    cmd->Size =
        wlan_cpu_to_le16(sizeof(HostCmd_DS_802_11_FW_WAKEUP_METHOD) +
                         S_DS_GEN);
    fwwm->Action = wlan_cpu_to_le16(action);
    switch (action) {
    case HostCmd_ACT_SET:
        fwwm->Method = wlan_cpu_to_le16(method);
        break;
    case HostCmd_ACT_GET:
    default:
        fwwm->Method = 0;
        break;
    }

    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function sends the HS_Activated event to the application
 *  
 *  @param priv    	A pointer to wlan_private structure
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
wlan_host_sleep_activated_event(wlan_private * priv)
{
    ENTER();

	priv->adapter->HS_Activated = TRUE; //JONO
#if WIRELESS_EXT > 14
//    send_iwevcustom_event(priv, (cyg_int8 *)CUS_EVT_HWM_CFG_DONE);
    send_iwevcustom_event(priv, (cyg_int8 *)CUS_EVT_HS_ACTIVATED); //JONO
    WLAN_DEBUG_PRINTF("HWM_CFG_DONE event sent\n");
#endif /* WIRELESS_EXT */

    LEAVE();
    return WLAN_STATUS_SUCCESS;
}

/** JONO START
 *  @brief This function sends the HS_DeActivated event to the application
 *  
 *  @param priv    	A pointer to wlan_private structure
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
wlan_host_sleep_deactivated_event(wlan_private * priv)
{
    ENTER();

    priv->adapter->HS_Activated = FALSE;

#if WIRELESS_EXT > 14
    send_iwevcustom_event(priv, (cyg_int8*)CUS_EVT_HS_DEACTIVATED);
    WLAN_DEBUG_PRINTF("HS_DEACTIVATED event sent\n");
#endif /* WIRELESS_EXT */

    LEAVE();
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function sends the HS_GPIO_INT event to the application
 *   
 *  @param priv                 A pointer to wlan_private structure
 *  @return 	   		WLAN_STATUS_SUCCESS --success, otherwise fail 
 */
int
wlan_host_sleep_gpio_int_event(wlan_private * priv)
{
    wlan_adapter *Adapter = priv->adapter;
    int ret = WLAN_STATUS_SUCCESS;

    ENTER();

    if (Adapter->bHostSleepConfigured) {
#if WIRELESS_EXT > 14
        send_iwevcustom_event(priv, (cyg_int8*)CUS_EVT_HS_GPIO_INT);
        WLAN_DEBUG_PRINTF("HS_GPIO_INT event sent\n");
#endif /* WIRELESS_EXT */
    } else {
        WLAN_DEBUG_PRINTF("hs_gpio_int: HS not configured !!!\n");
    }

    LEAVE();

    return ret;
}
//JONO END

/**
 *	This function uses for preparing data before host_sleep_cfg
 *	without return value
 */
static void wlan_get_hs_data(cyg_uint16 * data, cyg_uint32 ip_addr)
{
	data[0] = 0x22;		//data size
	data[1] = 0x00;		//don't remove
	data[2] = 0x00;		//don't remove
	data[3] = 0x03;		//Condition : broadcast + unicast
	data[4] = 0x00;		//don't remove
	data[5] = 0xa005;	//High byte = Gap (160ms) ; Low byte = GPIO (5)
	data[6] = 0x115;	//TlvType : 0x115 (don't change)
	data[7] = 0x10;		//don't remove
/*	
	data[8] = 0x01;		//AddrType : broadcast=1 ; unicast=2 ; multicast=3
	data[9] = 0x608;	//EthType = IPv4: 0x0800
	data[10] = 0xa8c0;	//IP addr1  192.168
	data[11] = 0x040a;	//IP addr2  10.4	
	
	data[12] = 0x02;	//AddrType : broadcast=1 ; unicast=2 ; multicast=3
	data[13] = 0x0008;	//EthType = ARP: 0x0806
	data[14] = 0xffff;	//IP addr1  192.168
	data[15] = 0xffff;	//IP addr2  10.4	
*/	
	
	data[8] = 0x02;		//AddrType : broadcast=1 ; unicast=2 ; multicast=3
	data[9] = 0x0008;	//EthType = IPv4: 0x0800
	data[10] = 0xffff;	//IP addr1  192.168
	data[11] = 0xffff;	//IP addr2  10.4	
	
	data[12] = 0x01;	//AddrType : broadcast=1 ; unicast=2 ; multicast=3
	data[13] = 0x608;	//EthType = ARP: 0x0806
	//data[14] = 0xa8c0;	//IP addr1  192.168
	//data[15] = 0x040a;	//IP addr2  10.4	
	data[14] = ip_addr & 0x0000FFFF;
	data[15] = ip_addr >> 16;
	diag_printf("ip_addr for host sleep config %08x: %08x %08x\n", ip_addr, (cyg_uint32) data[14], (cyg_uint32) data[15]);
}

/**
 *  @brief This function prepares command of host_sleep_cfg.
 *  
 *  @param priv    	A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @param pdata_buf 	A pointer to HostCmd_DS_802_11_HOST_SLEEP_CFG structure
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_host_sleep_cfg(wlan_private * priv,
                               HostCmd_DS_COMMAND * cmd,
                               HostCmd_DS_802_11_HOST_SLEEP_CFG * pdata_buf)
{
#if 1
    HostCmd_DS_802_11_HOST_SLEEP_CFG *phwuc = &cmd->params.hostsleepcfg;

    ENTER();

    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_HOST_SLEEP_CFG);
    cmd->Size = 0x22;
	wlan_get_hs_data((cyg_uint16 *)&cmd->Size, pdata_buf->ip_addr);
    phwuc->conditions = wlan_cpu_to_le32(pdata_buf->conditions);
    phwuc->gpio = pdata_buf->gpio;
    phwuc->gap = pdata_buf->gap;
    //phwuc->ip_addr = pdata_buf->ip_addr;

    LEAVE();
    return WLAN_STATUS_SUCCESS;
#else    
    HostCmd_DS_802_11_HOST_SLEEP_CFG *phwuc = &cmd->params.hostsleepcfg;

    ENTER();

    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_HOST_SLEEP_CFG);
    cmd->Size =
        wlan_cpu_to_le16(sizeof(HostCmd_DS_802_11_HOST_SLEEP_CFG) + S_DS_GEN);
    phwuc->conditions = wlan_cpu_to_le32(pdata_buf->conditions);
    phwuc->gpio = pdata_buf->gpio;
    phwuc->gap = pdata_buf->gap;
//    memcpy((void*)phwuc->filter, (void*)pdata_buf->filter, sizeof(pdata_buf->filter)); //JONO
    
	//WLAN_DEBUG_PRINTF("host sleep filter %x\n",phwuc->filter[3]);//JONO

    LEAVE();
    return WLAN_STATUS_SUCCESS;
#endif    
}

/** 
 *  @brief This function prepares command of inactivity_timeout.
 *  
 *  @param priv    	A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @param cmd_action   Action: GET SET
 *  @param pdata_buf 	A pointer to data buffer
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_inactivity_timeout(wlan_private * priv,
                                   HostCmd_DS_COMMAND * cmd,
                                   cyg_uint16 cmd_action, void *pdata_buf)
{
    cyg_uint16 *timeout = (cyg_uint16 *) pdata_buf;

    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_INACTIVITY_TIMEOUT);
    cmd->Size =
        wlan_cpu_to_le16(sizeof(HostCmd_DS_802_11_INACTIVITY_TIMEOUT) +
                         S_DS_GEN);

    cmd->params.inactivity_timeout.Action = wlan_cpu_to_le16(cmd_action);

    if (cmd_action)
        cmd->params.inactivity_timeout.Timeout = wlan_cpu_to_le16(*timeout);
    else
        cmd->params.inactivity_timeout.Timeout = 0;

    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function prepares command of sleep_period.
 *  
 *  @param priv    		A pointer to wlan_private structure
 *  @param cmd	   		A pointer to HostCmd_DS_COMMAND structure
 *  @param cmd_action 		the action: GET or SET
 *  @param pdata_buf		A pointer to data buffer
 *  @return 	   		WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_sleep_period(wlan_private * priv,
                             HostCmd_DS_COMMAND * cmd,
                             cyg_uint16 cmd_action, void *pdata_buf)
{
    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_SLEEP_PERIOD);
    cmd->Size = wlan_cpu_to_le16(sizeof(HostCmd_DS_802_11_SLEEP_PERIOD) +
                                 S_DS_GEN);
    memmove((void *)&cmd->params.ps_sleeppd, pdata_buf,
            sizeof(HostCmd_DS_802_11_SLEEP_PERIOD));
    cmd->params.ps_sleeppd.Period = //JONO
        wlan_cpu_to_le16(cmd->params.ps_sleeppd.Period); //JONO

    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function prepares command of sleep_params.
 *  
 *  @param priv    		A pointer to wlan_private structure
 *  @param cmd	   		A pointer to HostCmd_DS_COMMAND structure
 *  @param cmd_action 		the action: GET or SET
 *  @return 	   		WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_sleep_params(wlan_private * priv,
                             HostCmd_DS_COMMAND * cmd, cyg_uint16 cmd_action)
{
    wlan_adapter *Adapter = priv->adapter;
    HostCmd_DS_802_11_SLEEP_PARAMS *sp = &cmd->params.sleep_params;

    ENTER();

    cmd->Size = wlan_cpu_to_le16((sizeof(HostCmd_DS_802_11_SLEEP_PARAMS)) +
                                 S_DS_GEN);
    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_SLEEP_PARAMS);

    if (cmd_action == HostCmd_ACT_GEN_GET) {
        memset((void *)&Adapter->sp, 0, sizeof(SleepParams));
        memset((void *)sp, 0, sizeof(HostCmd_DS_802_11_SLEEP_PARAMS));
        sp->Action = wlan_cpu_to_le16(cmd_action);
    } else if (cmd_action == HostCmd_ACT_GEN_SET) {
        sp->Action = wlan_cpu_to_le16(cmd_action);
        sp->Error = wlan_cpu_to_le16(Adapter->sp.sp_error);
        sp->Offset = wlan_cpu_to_le16(Adapter->sp.sp_offset);
        sp->StableTime = wlan_cpu_to_le16(Adapter->sp.sp_stabletime);
        sp->CalControl = (cyg_uint8) Adapter->sp.sp_calcontrol;
        sp->ExternalSleepClk = (cyg_uint8) Adapter->sp.sp_extsleepclk;
        sp->Reserved = wlan_cpu_to_le16(Adapter->sp.sp_reserved);
    }

    LEAVE();
    return WLAN_STATUS_SUCCESS;
}

#define WEP_40_BIT_LEN	5
#define WEP_104_BIT_LEN	13

/** 
 *  @brief This function prepares command of set_wep.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @param cmd_oid   OID: ADD_WEP KEY or REMOVE_WEP KEY
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_set_wep(wlan_private * priv,
                        HostCmd_DS_COMMAND * cmd, cyg_uint32 cmd_oid)
{
    HostCmd_DS_802_11_SET_WEP *wep = &cmd->params.wep;
    wlan_adapter *Adapter = priv->adapter;
    int ret = WLAN_STATUS_SUCCESS;

    ENTER();

    if (cmd_oid == OID_802_11_ADD_WEP) {
        cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_SET_WEP);
        cmd->Size =
            wlan_cpu_to_le16((sizeof(HostCmd_DS_802_11_SET_WEP)) + S_DS_GEN);
        wep->Action = wlan_cpu_to_le16(HostCmd_ACT_ADD);

        /* default tx key index */
        wep->KeyIndex = wlan_cpu_to_le16(Adapter->CurrentWepKeyIndex &
                                         HostCmd_WEP_KEY_INDEX_MASK);

        WLAN_DEBUG_PRINTF("Tx Key Index: %u\n", wep->KeyIndex);

        switch (Adapter->WepKey[0].KeyLength) {
        case WEP_40_BIT_LEN:
            wep->WEPTypeForKey1 = HostCmd_TYPE_WEP_40_BIT;
            memmove((void *)wep->WEP1, (void *)Adapter->WepKey[0].KeyMaterial,
                    Adapter->WepKey[0].KeyLength);
            break;
        case WEP_104_BIT_LEN:
            wep->WEPTypeForKey1 = HostCmd_TYPE_WEP_104_BIT;
            memmove((void *)wep->WEP1, (void *)Adapter->WepKey[0].KeyMaterial,
                    Adapter->WepKey[0].KeyLength);
            break;
        case 0:
            break;
        default:
            WLAN_DEBUG_PRINTF("Key1 Length = %d is incorrect\n",
                   Adapter->WepKey[0].KeyLength);
            ret = WLAN_STATUS_FAILURE;
            goto done;
        }

        switch (Adapter->WepKey[1].KeyLength) {
        case WEP_40_BIT_LEN:
            wep->WEPTypeForKey2 = HostCmd_TYPE_WEP_40_BIT;
            memmove((void *)wep->WEP2, (void *)Adapter->WepKey[1].KeyMaterial,
                    Adapter->WepKey[1].KeyLength);
            break;
        case WEP_104_BIT_LEN:
            wep->WEPTypeForKey2 = HostCmd_TYPE_WEP_104_BIT;
            memmove((void *)wep->WEP2, (void *)Adapter->WepKey[1].KeyMaterial,
                    Adapter->WepKey[1].KeyLength);
            break;
        case 0:
            break;
        default:
            WLAN_DEBUG_PRINTF("Key2 Length = %d is incorrect\n",
                   Adapter->WepKey[1].KeyLength);
            ret = WLAN_STATUS_FAILURE;
            goto done;
        }

        switch (Adapter->WepKey[2].KeyLength) {
        case WEP_40_BIT_LEN:
            wep->WEPTypeForKey3 = HostCmd_TYPE_WEP_40_BIT;
            memmove((void *)wep->WEP3, (void *)Adapter->WepKey[2].KeyMaterial,
                    Adapter->WepKey[2].KeyLength);
            break;
        case WEP_104_BIT_LEN:
            wep->WEPTypeForKey3 = HostCmd_TYPE_WEP_104_BIT;
            memmove((void *)wep->WEP3,(void *) Adapter->WepKey[2].KeyMaterial,
                    Adapter->WepKey[2].KeyLength);
            break;
        case 0:
            break;
        default:
            WLAN_DEBUG_PRINTF("Key3 Length = %d is incorrect\n",
                   Adapter->WepKey[2].KeyLength);
            ret = WLAN_STATUS_FAILURE;
            goto done;
        }

        switch (Adapter->WepKey[3].KeyLength) {
        case WEP_40_BIT_LEN:
            wep->WEPTypeForKey4 = HostCmd_TYPE_WEP_40_BIT;
            memmove((void *)wep->WEP4,(void *)Adapter->WepKey[3].KeyMaterial,
                    Adapter->WepKey[3].KeyLength);
            break;
        case WEP_104_BIT_LEN:
            wep->WEPTypeForKey4 = HostCmd_TYPE_WEP_104_BIT;
            memmove((void *)wep->WEP4, (void *)Adapter->WepKey[3].KeyMaterial,
                    Adapter->WepKey[3].KeyLength);
            break;
        case 0:
            break;
        default:
            WLAN_DEBUG_PRINTF("Key4 Length = %d is incorrect\n",
                   Adapter->WepKey[3].KeyLength);
            ret = WLAN_STATUS_FAILURE;
            goto done;
        }
    } else if (cmd_oid == OID_802_11_REMOVE_WEP) {
        cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_SET_WEP);
        cmd->Size =
            wlan_cpu_to_le16((sizeof(HostCmd_DS_802_11_SET_WEP)) + S_DS_GEN);
        wep->Action = wlan_cpu_to_le16(HostCmd_ACT_REMOVE);

        /* default tx key index */
        wep->KeyIndex = wlan_cpu_to_le16((cyg_uint16) (Adapter->CurrentWepKeyIndex &
                                                (cyg_uint32)
                                                HostCmd_WEP_KEY_INDEX_MASK));
    }

    ret = WLAN_STATUS_SUCCESS;
  done:
    LEAVE();
    return ret;
}

/** 
 *  @brief This function prepares command of enable_rsn.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @param cmd_action 	the action: GET or SET
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_enable_rsn(wlan_private * priv,
                           HostCmd_DS_COMMAND * cmd, cyg_uint16 cmd_action)
{
    HostCmd_DS_802_11_ENABLE_RSN *pEnableRSN = &cmd->params.enbrsn;
    wlan_adapter *Adapter = priv->adapter;

    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_ENABLE_RSN);
    cmd->Size =
        wlan_cpu_to_le16(sizeof(HostCmd_DS_802_11_ENABLE_RSN) + S_DS_GEN);
    pEnableRSN->Action = wlan_cpu_to_le16(cmd_action);
    if (Adapter->SecInfo.WPAEnabled || Adapter->SecInfo.WPA2Enabled) {
        pEnableRSN->Enable = wlan_cpu_to_le16(HostCmd_ENABLE_RSN);
    } else {
        pEnableRSN->Enable = wlan_cpu_to_le16(HostCmd_DISABLE_RSN);
    }

    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function prepares command of key_material.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @param cmd_action 	the action: GET or SET
 *  @param cmd_oid	OID: ENABLE or DISABLE
 *  @param pdata_buf    A pointer to data buffer
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_key_material(wlan_private * priv,
                             HostCmd_DS_COMMAND * cmd,
                             cyg_uint16 cmd_action,
                             WLAN_OID cmd_oid, void *pdata_buf)
{
    HostCmd_DS_802_11_KEY_MATERIAL *pKeyMaterial = &cmd->params.keymaterial;
    PWLAN_802_11_KEY pKey = (PWLAN_802_11_KEY) pdata_buf;
    cyg_uint16 KeyParamSet_len;
    int ret = WLAN_STATUS_SUCCESS;

    ENTER();

    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_KEY_MATERIAL);
    pKeyMaterial->Action = wlan_cpu_to_le16(cmd_action);

    if (cmd_action == HostCmd_ACT_GET) {
        cmd->Size = wlan_cpu_to_le16(2 + S_DS_GEN);
        ret = WLAN_STATUS_SUCCESS;
        goto done;
    }

    memset((void *)&pKeyMaterial->KeyParamSet, 0, sizeof(MrvlIEtype_KeyParamSet_t));

    if (pKey->KeyLength == WPA_AES_KEY_LEN) {
        WLAN_DEBUG_PRINTF("WPA_AES\n");
        pKeyMaterial->KeyParamSet.KeyTypeId =
            wlan_cpu_to_le16(KEY_TYPE_ID_AES);

        if (cmd_oid == (WLAN_OID) KEY_INFO_ENABLED)
            pKeyMaterial->KeyParamSet.KeyInfo =
                wlan_cpu_to_le16(KEY_INFO_AES_ENABLED);
        else
            pKeyMaterial->KeyParamSet.KeyInfo =
                !(wlan_cpu_to_le16(KEY_INFO_AES_ENABLED));

        if (pKey->KeyIndex & 0x40000000)        //AES pairwise key: unicast
            pKeyMaterial->KeyParamSet.KeyInfo |=
                wlan_cpu_to_le16(KEY_INFO_AES_UNICAST);
        else                    //AES group key: multicast
            pKeyMaterial->KeyParamSet.KeyInfo |=
                wlan_cpu_to_le16(KEY_INFO_AES_MCAST);
    } else if (pKey->KeyLength == WPA_TKIP_KEY_LEN) {
        WLAN_DEBUG_PRINTF("WPA_TKIP\n");
        pKeyMaterial->KeyParamSet.KeyTypeId =
            wlan_cpu_to_le16(KEY_TYPE_ID_TKIP);
        pKeyMaterial->KeyParamSet.KeyInfo =
            wlan_cpu_to_le16(KEY_INFO_TKIP_ENABLED);

        if (pKey->KeyIndex & 0x40000000)        //TKIP pairwise key: unicast
            pKeyMaterial->KeyParamSet.KeyInfo |=
                wlan_cpu_to_le16(KEY_INFO_TKIP_UNICAST);
        else                    //TKIP group key: multicast
            pKeyMaterial->KeyParamSet.KeyInfo |=
                wlan_cpu_to_le16(KEY_INFO_TKIP_MCAST);
    }

    if (pKeyMaterial->KeyParamSet.KeyTypeId) {
        pKeyMaterial->KeyParamSet.Type =
            wlan_cpu_to_le16(TLV_TYPE_KEY_MATERIAL);
        pKeyMaterial->KeyParamSet.KeyLen = wlan_cpu_to_le16(pKey->KeyLength);
        memcpy((void *)pKeyMaterial->KeyParamSet.Key,
               (void *)pKey->KeyMaterial, pKey->KeyLength);
        pKeyMaterial->KeyParamSet.Length =
            wlan_cpu_to_le16(pKey->KeyLength + 6);

#define TYPE_LEN_FIELDS_LEN 4
        KeyParamSet_len =
            pKeyMaterial->KeyParamSet.Length + TYPE_LEN_FIELDS_LEN;
#define ACTION_FIELD_LEN 2
        cmd->Size =
            wlan_cpu_to_le16(KeyParamSet_len + ACTION_FIELD_LEN + S_DS_GEN);
    }

    ret = WLAN_STATUS_SUCCESS;
  done:
    LEAVE();
    return ret;
}

/** 
 *  @brief This function prepares command of get_log.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_get_log(wlan_private * priv, HostCmd_DS_COMMAND * cmd)
{
    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_GET_LOG);
    cmd->Size =
        wlan_cpu_to_le16(sizeof(HostCmd_DS_802_11_GET_LOG) + S_DS_GEN);

    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function prepares command of get_stat.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_get_stat(wlan_private * priv, HostCmd_DS_COMMAND * cmd)
{
    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_GET_STAT);
    cmd->Size =
        wlan_cpu_to_le16(sizeof(HostCmd_DS_802_11_GET_STAT) + S_DS_GEN);

    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function prepares command of snmp_mib.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @param cmd_action   the action: GET or SET
 *  @param cmd_oid   	the OID of SNMP MIB
 *  @param pdata_buf	the pointer to data buffer
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_snmp_mib(wlan_private * priv,
                         HostCmd_DS_COMMAND * cmd,
                         int cmd_action, int cmd_oid, void *pdata_buf)
{
    HostCmd_DS_802_11_SNMP_MIB *pSNMPMIB = &cmd->params.smib;
    wlan_adapter *Adapter = priv->adapter;
    cyg_uint8 ucTemp;

    ENTER();

    WLAN_DEBUG_PRINTF("SNMP_CMD: cmd_oid = 0x%x\n", cmd_oid);

    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_SNMP_MIB);
    cmd->Size =
        wlan_cpu_to_le16(sizeof(HostCmd_DS_802_11_SNMP_MIB) + S_DS_GEN);

    switch (cmd_oid) {
    case OID_802_11_INFRASTRUCTURE_MODE:
        pSNMPMIB->QueryType = wlan_cpu_to_le16(HostCmd_ACT_GEN_SET);
        pSNMPMIB->OID = wlan_cpu_to_le16((cyg_uint16) DesiredBssType_i);
        pSNMPMIB->BufSize = sizeof(cyg_uint8);
        if (Adapter->InfrastructureMode == Wlan802_11Infrastructure)
            ucTemp = SNMP_MIB_VALUE_INFRA;
        else
            ucTemp = SNMP_MIB_VALUE_ADHOC;

        memmove((void *)pSNMPMIB->Value, &ucTemp, sizeof(cyg_uint8));

        break;

    case OID_802_11D_ENABLE:
        {
            cyg_uint32 ulTemp;

            pSNMPMIB->OID = wlan_cpu_to_le16((cyg_uint16) Dot11D_i);

            if (cmd_action == HostCmd_ACT_SET) {
                pSNMPMIB->QueryType = HostCmd_ACT_GEN_SET;
                pSNMPMIB->BufSize = sizeof(cyg_uint16);
                ulTemp = *(cyg_uint32 *) pdata_buf;
                *((PUSHORT) (pSNMPMIB->Value)) =
                    wlan_cpu_to_le16((cyg_uint16) ulTemp);
            }
            break;
        }

    case OID_802_11_FRAGMENTATION_THRESHOLD:
        {
            WLAN_802_11_FRAGMENTATION_THRESHOLD ulTemp;

            pSNMPMIB->OID = wlan_cpu_to_le16((cyg_uint16) FragThresh_i);

            if (cmd_action == HostCmd_ACT_GET) {
                pSNMPMIB->QueryType = wlan_cpu_to_le16(HostCmd_ACT_GEN_GET);
            } else if (cmd_action == HostCmd_ACT_SET) {
                pSNMPMIB->QueryType = wlan_cpu_to_le16(HostCmd_ACT_GEN_SET);
                pSNMPMIB->BufSize = wlan_cpu_to_le16(sizeof(cyg_uint16));
                ulTemp = *((WLAN_802_11_FRAGMENTATION_THRESHOLD *)
                           pdata_buf);
                *((PUSHORT) (pSNMPMIB->Value)) =
                    wlan_cpu_to_le16((cyg_uint16) ulTemp);

            }

            break;
        }

    case OID_802_11_RTS_THRESHOLD:
        {

            WLAN_802_11_RTS_THRESHOLD ulTemp;
            pSNMPMIB->OID = wlan_le16_to_cpu((cyg_uint16) RtsThresh_i);

            if (cmd_action == HostCmd_ACT_GET) {
                pSNMPMIB->QueryType = wlan_cpu_to_le16(HostCmd_ACT_GEN_GET);
            } else if (cmd_action == HostCmd_ACT_SET) {
                pSNMPMIB->QueryType = wlan_cpu_to_le16(HostCmd_ACT_GEN_SET);
                pSNMPMIB->BufSize = wlan_cpu_to_le16(sizeof(cyg_uint16));
                ulTemp = *((WLAN_802_11_RTS_THRESHOLD *)
                           pdata_buf);
                *(PUSHORT) (pSNMPMIB->Value) = wlan_cpu_to_le16((cyg_uint16) ulTemp);

            }
            break;
        }
    case OID_802_11_TX_RETRYCOUNT:
        pSNMPMIB->OID = wlan_cpu_to_le16((cyg_uint16) ShortRetryLim_i);

        if (cmd_action == HostCmd_ACT_GET) {
            pSNMPMIB->QueryType = wlan_cpu_to_le16(HostCmd_ACT_GEN_GET);
        } else if (cmd_action == HostCmd_ACT_SET) {
            pSNMPMIB->QueryType = wlan_cpu_to_le16(HostCmd_ACT_GEN_SET);
            pSNMPMIB->BufSize = wlan_cpu_to_le16(sizeof(cyg_uint16));
            *((PUSHORT) (pSNMPMIB->Value)) =
                wlan_cpu_to_le16((cyg_uint16) Adapter->TxRetryCount);
        }

        break;
    default:
        break;
    }

    WLAN_DEBUG_PRINTF(
           "SNMP_CMD: Command=0x%x, Size=0x%x, SeqNum=0x%x, Result=0x%x\n",
           cmd->Command, cmd->Size, cmd->SeqNum, cmd->Result);

    WLAN_DEBUG_PRINTF(
           "SNMP_CMD: Action=0x%x, OID=0x%x, OIDSize=0x%x, Value=0x%x\n",
           pSNMPMIB->QueryType, pSNMPMIB->OID, pSNMPMIB->BufSize,
           *(cyg_uint16 *) pSNMPMIB->Value);

    LEAVE();
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function prepares command of radio_control.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @param cmd_action   the action: GET or SET
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_radio_control(wlan_private * priv,
                              HostCmd_DS_COMMAND * cmd, int cmd_action)
{
    wlan_adapter *Adapter = priv->adapter;
    HostCmd_DS_802_11_RADIO_CONTROL *pRadioControl = &cmd->params.radio;

    ENTER();

    cmd->Size =
        wlan_cpu_to_le16((sizeof(HostCmd_DS_802_11_RADIO_CONTROL)) +
                         S_DS_GEN);
    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_RADIO_CONTROL);

    pRadioControl->Action = wlan_cpu_to_le16(cmd_action);

    switch (Adapter->Preamble) {
    case HostCmd_TYPE_SHORT_PREAMBLE:
        pRadioControl->Control = wlan_cpu_to_le16(SET_SHORT_PREAMBLE);
        break;

    case HostCmd_TYPE_LONG_PREAMBLE:
        pRadioControl->Control = wlan_cpu_to_le16(SET_LONG_PREAMBLE);
        break;

    case HostCmd_TYPE_AUTO_PREAMBLE:
    default:
        pRadioControl->Control = wlan_cpu_to_le16(SET_AUTO_PREAMBLE);
        break;
    }

    if (Adapter->RadioOn)
        pRadioControl->Control |= wlan_cpu_to_le16(TURN_ON_RF);
    else
        pRadioControl->Control &= wlan_cpu_to_le16(~TURN_ON_RF);

    LEAVE();
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function prepares command of bca_timeshare.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @param cmd_action   the action: GET or SET
 *  @param user_bca_ts	A pointer to HostCmd_DS_802_11_BCA_TIMESHARE structure
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_bca_timeshare(wlan_private * priv,
                              HostCmd_DS_COMMAND * cmd,
                              cyg_uint16 cmd_action,
                              HostCmd_DS_802_11_BCA_TIMESHARE * user_bca_ts)
{
    wlan_adapter *Adapter = priv->adapter;
    HostCmd_DS_802_11_BCA_TIMESHARE *bca_ts = &cmd->params.bca_timeshare;

    ENTER();

    cmd->Size = wlan_cpu_to_le16((sizeof(HostCmd_DS_802_11_BCA_TIMESHARE)) +
                                 S_DS_GEN);
    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_BCA_CONFIG_TIMESHARE);

    if (cmd_action == HostCmd_ACT_GEN_GET) {
        memset((void *)&Adapter->bca_ts, 0, sizeof(bca_ts));
        memset((void *)bca_ts, 0, sizeof(HostCmd_DS_802_11_BCA_TIMESHARE));
        bca_ts->Action = wlan_cpu_to_le16(cmd_action);
    } else if (cmd_action == HostCmd_ACT_GEN_SET) {
        bca_ts->Action = wlan_cpu_to_le16(cmd_action);
        bca_ts->TrafficType = wlan_cpu_to_le16(user_bca_ts->TrafficType);
        bca_ts->TimeShareInterval =
            wlan_cpu_to_le32(user_bca_ts->TimeShareInterval);
        bca_ts->BTTime = wlan_cpu_to_le32(user_bca_ts->BTTime);
    }

    LEAVE();
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function prepares command of rf_tx_power.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @param cmd_action   the action: GET or SET
 *  @param pdata_buf	A pointer to data buffer
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_rf_tx_power(wlan_private * priv,
                            HostCmd_DS_COMMAND * cmd,
                            cyg_uint16 cmd_action, void *pdata_buf)
{

    HostCmd_DS_802_11_RF_TX_POWER *pRTP = &cmd->params.txp;

    ENTER();

    cmd->Size =
        wlan_cpu_to_le16((sizeof(HostCmd_DS_802_11_RF_TX_POWER)) + S_DS_GEN);
    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_RF_TX_POWER);
    pRTP->Action = cmd_action;

    WLAN_DEBUG_PRINTF("RF_TX_POWER_CMD: Size:%d Cmd:0x%x Act:%d\n", cmd->Size,
           cmd->Command, pRTP->Action);

    switch (cmd_action) {
    case HostCmd_ACT_GEN_GET:
        pRTP->Action = wlan_cpu_to_le16(HostCmd_ACT_GEN_GET);
        pRTP->CurrentLevel = 0;
        break;

    case HostCmd_ACT_GEN_SET:
        pRTP->Action = wlan_cpu_to_le16(HostCmd_ACT_GEN_SET);
        pRTP->CurrentLevel = wlan_cpu_to_le16(*((cyg_uint16 *) pdata_buf));
        break;
    }
    LEAVE();
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function prepares command of rf_antenna.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @param cmd_action   the action: GET or SET
 *  @param pdata_buf	A pointer to data buffer
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_rf_antenna(wlan_private * priv,
                           HostCmd_DS_COMMAND * cmd,
                           cyg_uint16 cmd_action, void *pdata_buf)
{
    HostCmd_DS_802_11_RF_ANTENNA *rant = &cmd->params.rant;

    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_RF_ANTENNA);
    cmd->Size =
        wlan_cpu_to_le16(sizeof(HostCmd_DS_802_11_RF_ANTENNA) + S_DS_GEN);

    rant->Action = wlan_cpu_to_le16(cmd_action);
    if ((cmd_action == HostCmd_ACT_SET_RX) ||
        (cmd_action == HostCmd_ACT_SET_TX)) {
        rant->AntennaMode =
            wlan_cpu_to_le16((cyg_uint16) (*(WLAN_802_11_ANTENNA *) pdata_buf));
    }

    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function prepares command of rate_adapt_rateset.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @param cmd_action   the action: GET or SET
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_rate_adapt_rateset(wlan_private * priv,
                                   HostCmd_DS_COMMAND * cmd, cyg_uint16 cmd_action)
{
    HostCmd_DS_802_11_RATE_ADAPT_RATESET * rateadapt = &cmd->params.rateset;
    wlan_adapter *Adapter = priv->adapter;

    cmd->Size =
        wlan_cpu_to_le16(sizeof(HostCmd_DS_802_11_RATE_ADAPT_RATESET) +
                         S_DS_GEN);
    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_RATE_ADAPT_RATESET);

    ENTER();

    rateadapt->Action = cmd_action;
    rateadapt->EnableHwAuto = Adapter->EnableHwAuto;
    rateadapt->Bitmap = Adapter->RateBitmap;

    LEAVE();
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function prepares command of mac_multicast_adr.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @param cmd_action   the action: GET or SET
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_mac_multicast_adr(wlan_private * priv,
                           HostCmd_DS_COMMAND * cmd, cyg_uint16 cmd_action)
{
    HostCmd_DS_MAC_MULTICAST_ADR *pMCastAdr = &cmd->params.madr;
    wlan_adapter *Adapter = priv->adapter;

    cmd->Size =
        wlan_cpu_to_le16(sizeof(HostCmd_DS_MAC_MULTICAST_ADR) + S_DS_GEN);
    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_MAC_MULTICAST_ADR);

    pMCastAdr->Action = wlan_cpu_to_le16(cmd_action);
    pMCastAdr->NumOfAdrs =
        wlan_cpu_to_le16((cyg_uint16) Adapter->NumOfMulticastMACAddr);
    memcpy((void *)pMCastAdr->MACList, Adapter->MulticastList,
           Adapter->NumOfMulticastMACAddr * ETH_ALEN);

    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function prepares command of rf_channel.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @param cmd_action   the action: GET or SET
 *  @param pdata_buf	A pointer to data buffer
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_rf_channel(wlan_private * priv,
                           HostCmd_DS_COMMAND * cmd,
                           int option, void *pdata_buf)
{
    HostCmd_DS_802_11_RF_CHANNEL *rfchan = &cmd->params.rfchannel;

    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_RF_CHANNEL);
    cmd->Size = wlan_cpu_to_le16(sizeof(HostCmd_DS_802_11_RF_CHANNEL)
                                 + S_DS_GEN);

    if (option == HostCmd_OPT_802_11_RF_CHANNEL_SET) {
        rfchan->CurrentChannel = wlan_cpu_to_le16(*((cyg_uint16 *) pdata_buf));
    }

    rfchan->Action = wlan_cpu_to_le16(option);

    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function prepares command of rssi.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_rssi(wlan_private * priv, HostCmd_DS_COMMAND * cmd)
{
    wlan_adapter *Adapter = priv->adapter;

    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_RSSI);
    cmd->Size = wlan_cpu_to_le16(sizeof(HostCmd_DS_802_11_RSSI) + S_DS_GEN);
    cmd->params.rssi.N = priv->adapter->bcn_avg_factor;

    /* reset Beacon SNR/NF/RSSI values */
    Adapter->SNR[TYPE_BEACON][TYPE_NOAVG] = 0;
    Adapter->SNR[TYPE_BEACON][TYPE_AVG] = 0;
    Adapter->NF[TYPE_BEACON][TYPE_NOAVG] = 0;
    Adapter->NF[TYPE_BEACON][TYPE_AVG] = 0;
    Adapter->RSSI[TYPE_BEACON][TYPE_NOAVG] = 0;
    Adapter->RSSI[TYPE_BEACON][TYPE_AVG] = 0;

    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function prepares command of reg_access.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @param cmd_action   the action: GET or SET
 *  @param pdata_buf	A pointer to data buffer
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_reg_access(wlan_private * priv,
                    HostCmd_DS_COMMAND * CmdPtr,
                    cyg_uint8 cmd_action, void *pdata_buf)
{
    wlan_offset_value *offval;

    ENTER();

    offval = (wlan_offset_value *) pdata_buf;

    switch (CmdPtr->Command) {
    case HostCmd_CMD_MAC_REG_ACCESS:
        {
            HostCmd_DS_MAC_REG_ACCESS *macreg;

            CmdPtr->Size =
                wlan_cpu_to_le16(sizeof(HostCmd_DS_MAC_REG_ACCESS) +
                                 S_DS_GEN);
            macreg = (PHostCmd_DS_MAC_REG_ACCESS) & CmdPtr->params.macreg;

            macreg->Action = wlan_cpu_to_le16(cmd_action);
            macreg->Offset = wlan_cpu_to_le16((cyg_uint16) offval->offset);
            macreg->Value = wlan_cpu_to_le32(offval->value);

            break;
        }

    case HostCmd_CMD_BBP_REG_ACCESS:
        {
            HostCmd_DS_BBP_REG_ACCESS *bbpreg;

            CmdPtr->Size =
                wlan_cpu_to_le16(sizeof(HostCmd_DS_BBP_REG_ACCESS) +
                                 S_DS_GEN);
            bbpreg = (PHostCmd_DS_BBP_REG_ACCESS) & CmdPtr->params.bbpreg;

            bbpreg->Action = wlan_cpu_to_le16(cmd_action);
            bbpreg->Offset = wlan_cpu_to_le16((cyg_uint16) offval->offset);
            bbpreg->Value = (cyg_uint8) offval->value;

            break;
        }

    case HostCmd_CMD_RF_REG_ACCESS:
        {
            HostCmd_DS_RF_REG_ACCESS *rfreg;

            CmdPtr->Size =
                wlan_cpu_to_le16(sizeof(HostCmd_DS_RF_REG_ACCESS) + S_DS_GEN);
            rfreg = (PHostCmd_DS_RF_REG_ACCESS) & CmdPtr->params.rfreg;

            rfreg->Action = wlan_cpu_to_le16(cmd_action);
            rfreg->Offset = wlan_cpu_to_le16((cyg_uint16) offval->offset);
            rfreg->Value = (cyg_uint8) offval->value;

            break;
        }

    default:
        break;
    }

    LEAVE();
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function prepares command of mac_address.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @param cmd_action   the action: GET or SET
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_mac_address(wlan_private * priv,
                            HostCmd_DS_COMMAND * cmd, cyg_uint16 cmd_action)
{
    wlan_adapter *Adapter = priv->adapter;

    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_MAC_ADDRESS);
    cmd->Size = wlan_cpu_to_le16(sizeof(HostCmd_DS_802_11_MAC_ADDRESS) +
                                 S_DS_GEN);
    cmd->Result = 0;

    cmd->params.macadd.Action = wlan_cpu_to_le16(cmd_action);

    if (cmd_action == HostCmd_ACT_SET) {
        memcpy((void *)cmd->params.macadd.MacAdd, Adapter->CurrentAddr, ETH_ALEN);
        HEXDUMP("SET_CMD: MAC ADDRESS-", Adapter->CurrentAddr, 6);
    }

    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function prepares command of cal_data_ext.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @param pdata_buf	A pointer to data buffer
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_cal_data_ext(wlan_private * priv,
                             HostCmd_DS_COMMAND * cmd, void *pdata_buf)
{
    HostCmd_DS_802_11_CAL_DATA_EXT *PCalDataext = pdata_buf;

    pHostCmd_DS_802_11_CAL_DATA_EXT pCmdCalData =
        (pHostCmd_DS_802_11_CAL_DATA_EXT) & cmd->params.caldataext;

    ENTER();
    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_CAL_DATA_EXT);

    WLAN_DEBUG_PRINTF("CalDataLen = %d(d)\n", PCalDataext->CalDataLen);

#define MAX_ALLOWED_LEN	1024
    if (PCalDataext->CalDataLen > MAX_ALLOWED_LEN) {
        WLAN_DEBUG_PRINTF("CAL_DATA_EXT: Cal data lenght too large!\n");
        return WLAN_STATUS_FAILURE;
    }
#define ACTION_REV_CALDATA_LEN_FIELDS_LEN 6
    memcpy((void *)pCmdCalData, (void *)PCalDataext,
           PCalDataext->CalDataLen +
           ACTION_REV_CALDATA_LEN_FIELDS_LEN + S_DS_GEN);

    cmd->Size = wlan_cpu_to_le16(PCalDataext->CalDataLen +
                                 ACTION_REV_CALDATA_LEN_FIELDS_LEN +
                                 S_DS_GEN);

    WLAN_DEBUG_PRINTF("CAL_DATA_EXT: cmd->Size = %d(d)\n", cmd->Size);

    cmd->Result = 0;

    LEAVE();
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function prepares command of eeprom_access.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param cmd	   	A pointer to HostCmd_DS_COMMAND structure
 *  @param cmd_action   the action: GET or SET
 *  @param pdata_buf	A pointer to data buffer
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_802_11_eeprom_access(wlan_private * priv,
                              HostCmd_DS_COMMAND * cmd,
                              int cmd_action, void *pdata_buf)
{
    wlan_ioctl_regrdwr *ea = pdata_buf;

    ENTER();

    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_EEPROM_ACCESS);
    cmd->Size =
        wlan_cpu_to_le16(sizeof(HostCmd_DS_802_11_EEPROM_ACCESS) + S_DS_GEN);
    cmd->Result = 0;

    cmd->params.rdeeprom.Action = wlan_cpu_to_le16(ea->Action);
    cmd->params.rdeeprom.Offset = wlan_cpu_to_le16(ea->Offset);
    cmd->params.rdeeprom.ByteCount = wlan_cpu_to_le16(ea->NOB);
    cmd->params.rdeeprom.Value = 0;

    return WLAN_STATUS_SUCCESS;
}

static int
wlan_cmd_802_11_IBSS_Coalesced_Status(wlan_private * priv,
                                      HostCmd_DS_COMMAND * cmd,
                                      int cmd_action, void *pdata_buf)
{
    HostCmd_DS_802_11_IBSS_Status *pIBSSReq = &(cmd->params.ibssCoalescing);
    cyg_uint16 *enable = pdata_buf;

    WLAN_DEBUG_PRINTF("HostCmd_CMD_802_11_BSSID_QUERY request");

    cmd->Command =
        wlan_cpu_to_le16(HostCmd_CMD_802_11_IBSS_COALESCING_STATUS);
    cmd->Size =
        wlan_cpu_to_le16(sizeof(HostCmd_DS_802_11_IBSS_Status) + S_DS_GEN);
    cmd->Result = 0;
    pIBSSReq->Action = wlan_cpu_to_le16(cmd_action);

    switch (cmd_action) {
    case HostCmd_ACT_SET:
        pIBSSReq->Enable = wlan_cpu_to_le16(*enable);
        break;

        /* In other case.. Noting to do */
    case HostCmd_ACT_GET:
    default:
        break;
    }
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function queues the command to cmd list.
 *  
 *  @param Adapter	A pointer to wlan_adapter structure
 *  @param CmdNode   	A pointer to CmdCtrlNode structure
 *  @param addtail	specify if the cmd needs to be queued in the header or tail
 *  @return 	   	n/a
 */
void
QueueCmd(wlan_adapter * Adapter, CmdCtrlNode * CmdNode, BOOLEAN addtail)
{
    ulong flags;
    HostCmd_DS_COMMAND *CmdPtr;

    ENTER();

    if (!CmdNode) {
        WLAN_DEBUG_PRINTF("QUEUE_CMD: CmdNode is NULL\n");
        goto done;
    }

    CmdPtr = (HostCmd_DS_COMMAND *) (CmdNode->BufVirtualAddr + SDIO_HEADER_LEN);//clyu SDIO_HEADER_LEN
    if (!CmdPtr) {
        WLAN_DEBUG_PRINTF("QUEUE_CMD: CmdPtr is NULL\n");
        goto done;
    }

    /* Exit_PS command needs to be queued in the header always. */
    if (CmdPtr->Command == HostCmd_CMD_802_11_PS_MODE) {
        HostCmd_DS_802_11_PS_MODE *psm = &CmdPtr->params.psmode;
        if (psm->Action == HostCmd_SubCmd_Exit_PS) {
            if (Adapter->PSState != PS_STATE_FULL_POWER)
                addtail = FALSE;
        }
    }

    if ((CmdPtr->Command == HostCmd_CMD_802_11_HOST_SLEEP_AWAKE_CONFIRM)
        || (CmdPtr->Command == HostCmd_CMD_802_11_HOST_SLEEP_ACTIVATE) //JONO
        || (CmdPtr->Command == HostCmd_CMD_802_11_HOST_SLEEP_CFG) //JONO
        ) {
        addtail = FALSE;
    }

    //spin_lock_irqsave(&Adapter->QueueSpinLock, flags);
    OS_INT_DISABLE(flags);

    if (addtail)
        list_add_tail((struct list_head *) CmdNode, &Adapter->CmdPendingQ);
    else
        list_add((struct list_head *) CmdNode, &Adapter->CmdPendingQ);
		
		OS_INT_RESTORE(flags);
    //spin_unlock_irqrestore(&Adapter->QueueSpinLock, flags);

    WLAN_DEBUG_PRINTF("QUEUE_CMD: Inserted node=0x%x, cmd=0x%x in CmdPendingQ\n",
           (cyg_uint32) CmdNode,
           ((HostCmd_DS_GEN *) (CmdNode->BufVirtualAddr + SDIO_HEADER_LEN))->Command);//clyu SDIO_HEADER_LEN

  done:
    LEAVE();
    return;
}

#ifdef MFG_CMD_SUPPORT
/** 
 *  @brief This function sends general command to firmware.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param CmdNode   	A pointer to CmdCtrlNode structure
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
SendMfgCommand(wlan_private * priv, CmdCtrlNode * cmdnode)
{
    HostCmd_DS_GEN *pCmdPtr;
    wlan_adapter *Adapter = priv->adapter;

    ENTER();

    pCmdPtr = (PHostCmd_DS_GEN) Adapter->mfg_cmd;

    pCmdPtr->Command = wlan_cpu_to_le16(HostCmd_CMD_MFG_COMMAND);

    SetCmdCtrlNode(priv, cmdnode, OID_MRVL_MFG_COMMAND,
                   HostCmd_OPTION_WAITFORRSP, pCmdPtr);

    /* Assign new sequence number */
    pCmdPtr->SeqNum = wlan_cpu_to_le16(priv->adapter->SeqNum);

    WLAN_DEBUG_PRINTF("Sizeof CmdPtr->size %d\n", (cyg_uint32) pCmdPtr->Size);

    /* copy the command from information buffer to command queue */
    memcpy((void *) (cmdnode->BufVirtualAddr + SDIO_HEADER_LEN), (void *) pCmdPtr, pCmdPtr->Size);//clyu

    Adapter->mfg_cmd_flag = 1;

    LEAVE();
    return WLAN_STATUS_SUCCESS;
}
#endif

/*
 * TODO: Fix the issue when DownloadCommandToStation is being called the
 * second time when the command timesout. All the CmdPtr->xxx are in little
 * endian and therefore all the comparissions will fail.
 * For now - we are not performing the endian conversion the second time - but
 * for PS and DEEP_SLEEP we need to worry
 */

/** 
 *  @brief This function downloads the command to firmware.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param CmdNode   	A pointer to CmdCtrlNode structure
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
cyg_uint32 _uSDIO_cmd_send;
static int
DownloadCommandToStation(wlan_private * priv, CmdCtrlNode * CmdNode)
{
    ulong flags;
    HostCmd_DS_COMMAND *CmdPtr;
    wlan_adapter *Adapter = priv->adapter;
    int ret = WLAN_STATUS_SUCCESS;
    cyg_uint16 CmdSize;
    cyg_uint16 Command;
    int volatile save_irq;

    //OS_INTERRUPT_SAVE_AREA;
    sbi_reenable_host_interrupt(priv, 0x00);//clyu

    ENTER();

    if (!Adapter || !CmdNode) {
        WLAN_DEBUG_PRINTF("DNLD_CMD: Adapter = %#x, CmdNode = %#x\n",
               (int) Adapter, (int) CmdNode);
        if (CmdNode)
            CleanupAndInsertCmd(priv, CmdNode);
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    CmdPtr = (HostCmd_DS_COMMAND *) (CmdNode->BufVirtualAddr + SDIO_HEADER_LEN);//clyu

    if (!CmdPtr || !CmdPtr->Size) {
        WLAN_DEBUG_PRINTF("DNLD_CMD: CmdPtr is Null or Cmd Size is Zero, "
               "Not sending\n");
        CleanupAndInsertCmd(priv, CmdNode);
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }
		
		
    //spin_lock_irqsave(&Adapter->QueueSpinLock, flags);
    OS_INT_DISABLE(flags);
    Adapter->CurCmd = CmdNode;
    OS_INT_RESTORE(flags);
    //spin_unlock_irqrestore(&Adapter->QueueSpinLock, flags);
    Adapter->CurCmdRetCode = 0;
    WLAN_DEBUG_PRINTF("DNLD_CMD:: Before download, Size of Cmd = %d\n",
           CmdPtr->Size);

    CmdSize = CmdPtr->Size;

    Command = wlan_cpu_to_le16(CmdPtr->Command);

    CmdNode->CmdWaitQWoken = FALSE;
    CmdSize = wlan_cpu_to_le16(CmdSize);
	
	_uSDIO_cmd_send = 1;
    ret = sbi_host_to_card(priv, MVMS_CMD, (cyg_uint8 *) CmdNode->BufVirtualAddr/*CmdPtr*/, CmdSize);//clyu CmdPtr->CmdNode->BufVirtualAddr

    /* clear TxDone interrupt bit */
    OS_INT_DISABLE(save_irq);
    Adapter->HisRegCpy &= ~HIS_TxDnLdRdy;
    OS_INT_RESTORE(save_irq);

    if (ret != 0) {
        WLAN_DEBUG_PRINTF("DNLD_CMD: Host to Card Failed\n");
        /* set error code that will be transferred back to PrepareAndSendCommand() */
        Adapter->CurCmdRetCode = WLAN_STATUS_FAILURE;
        CleanupAndInsertCmd(priv, Adapter->CurCmd);
        //spin_lock_irqsave(&Adapter->QueueSpinLock, flags);
        OS_INT_DISABLE(flags);
        Adapter->CurCmd = NULL;
        OS_INT_RESTORE(flags);
        //spin_unlock_irqrestore(&Adapter->QueueSpinLock, flags);
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    WLAN_DEBUG_PRINTF("DNLD_CMD: Sent command 0x%x @ %lu\n", Command,
           cyg_current_time());
    HEXDUMP("DNLD_CMD: Command", CmdNode->BufVirtualAddr, CmdSize);

    /* Setup the timer after transmit command */
    if (Command == HostCmd_CMD_802_11_SCAN
        || Command == HostCmd_CMD_802_11_AUTHENTICATE
        || Command == HostCmd_CMD_802_11_ASSOCIATE)
        //ModTimer(&Adapter->MrvDrvCommandTimer, MRVDRV_TIMER_10S);
        cyg_alarm_initialize(cmd_alarm_handle, cyg_current_time()+1000, 0);
    else
        //ModTimer(&Adapter->MrvDrvCommandTimer, MRVDRV_TIMER_5S);
        cyg_alarm_initialize(cmd_alarm_handle, cyg_current_time()+500, 0);
	
	cyg_alarm_enable(cmd_alarm_handle);
	
    Adapter->CommandTimerIsSet = TRUE;

    if (Command == HostCmd_CMD_802_11_DEEP_SLEEP) {
        if (Adapter->IntCounter /*|| Adapter->CurrentTxSkb*/)
            WLAN_DEBUG_PRINTF("DNLD_CMD: DS- IntCounter=%d\n",
                   Adapter->IntCounter/*, Adapter->CurrentTxSkb*/);

        if (Adapter->IntCounter) {
            OS_INT_DISABLE(save_irq);
            Adapter->IntCounterSaved = Adapter->IntCounter;
            Adapter->IntCounter = 0;
            OS_INT_RESTORE(save_irq);
        }
	#if 0
        if (Adapter->CurrentTxSkb) {
            free(Adapter->CurrentTxSkb);
            OS_INT_DISABLE(save_irq);
            Adapter->CurrentTxSkb = NULL;
            OS_INT_RESTORE(save_irq);
            priv->stats.tx_dropped++;
        }
    #endif
        /* 1. change the PS state to DEEP_SLEEP
         * 2. since there is no response for this command, so 
         *    delete the command timer and free the Node. */

        Adapter->IsDeepSleep = TRUE;

        CleanupAndInsertCmd(priv, CmdNode);
        OS_INT_DISABLE(flags);
        //spin_lock_irqsave(&Adapter->QueueSpinLock, flags);
        Adapter->CurCmd = NULL;
        OS_INT_RESTORE(flags);
        //spin_unlock_irqrestore(&Adapter->QueueSpinLock, flags);

        if (Adapter->CommandTimerIsSet) {
            //CancelTimer(&Adapter->MrvDrvCommandTimer);
            cyg_alarm_disable(cmd_alarm_handle);
            //cyg_alarm_delete(cmd_alarm_handle);
            Adapter->CommandTimerIsSet = FALSE;
        }

        /* stop clock to save more power */
        sbi_set_bus_clock(priv, FALSE);

        if (Adapter->bHostSleepConfigured) {
            wlan_host_sleep_activated_event(priv);
        }

    }

    ret = WLAN_STATUS_SUCCESS;

  done:
    LEAVE();
    return ret;
}

/** 
 *  @brief This function prepares command of mac_control.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param cmd		A pointer to HostCmd_DS_COMMAND structure
 *  @return 		WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_cmd_mac_control(wlan_private * priv, HostCmd_DS_COMMAND * cmd)
{
    HostCmd_DS_MAC_CONTROL *mac = &cmd->params.macctrl;

    ENTER();

    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_MAC_CONTROL);
    cmd->Size = wlan_cpu_to_le16(sizeof(HostCmd_DS_MAC_CONTROL) + S_DS_GEN);
    mac->Action = wlan_cpu_to_le16(priv->adapter->CurrentPacketFilter);

    WLAN_DEBUG_PRINTF("wlan_cmd_mac_control(): Action=0x%X Size=%d\n",
           mac->Action, cmd->Size);

    LEAVE();
    return WLAN_STATUS_SUCCESS;
}

/********************************************************
		Global Functions
********************************************************/

/** 
 *  @brief This function inserts command node to CmdFreeQ
 *  after cleans it.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param pTempCmd	A pointer to CmdCtrlNode structure
 *  @return 		n/a
 */
void
CleanupAndInsertCmd(wlan_private * priv, CmdCtrlNode * pTempCmd)
{
    ulong flags;
    wlan_adapter *Adapter = priv->adapter;

    ENTER();

    if (!pTempCmd)
        goto done;

    //spin_lock_irqsave(&Adapter->QueueSpinLock, flags);
    OS_INT_DISABLE(flags);
    CleanUpCmdCtrlNode(pTempCmd);
    list_add_tail((struct list_head *) pTempCmd, &Adapter->CmdFreeQ);
    //spin_unlock_irqrestore(&Adapter->QueueSpinLock, flags);
    OS_INT_RESTORE(flags);

  done:
    LEAVE();
}

/** 
 *  @brief This function sets radio control.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @return 		WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
SetRadioControl(wlan_private * priv)
{
    int ret = WLAN_STATUS_SUCCESS;

    ENTER();

    ret = PrepareAndSendCommand(priv,
                                HostCmd_CMD_802_11_RADIO_CONTROL,
                                HostCmd_ACT_GEN_SET,
                                HostCmd_OPTION_WAITFORRSP, 0, NULL);

    WLAN_DEBUG_PRINTF("RADIO_SET: on or off: 0x%X, Preamble = 0x%X\n",
           priv->adapter->RadioOn, priv->adapter->Preamble);

    LEAVE();
    return ret;
}

/** 
 *  @brief This function sets packet filter.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @return 		WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
SetMacPacketFilter(wlan_private * priv)
{
    int ret = WLAN_STATUS_SUCCESS;

    ENTER();

    WLAN_DEBUG_PRINTF("SetMacPacketFilter Value = %x\n",
           priv->adapter->CurrentPacketFilter);

    /* Send MAC control command to station */
    ret = PrepareAndSendCommand(priv, HostCmd_CMD_MAC_CONTROL, 0, 0, 0, NULL);

    LEAVE();
    return ret;
}

/** 
 *  @brief This function prepare the command before send to firmware.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param cmd_no	command number
 *  @param cmd_action	command action: GET or SET
 *  @param wait_option	wait option: wait response or not
 *  @param cmd_oid	cmd oid: treated as sub command
 *  @param pdata_buf	A pointer to informaion buffer
 *  @return 		WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
PrepareAndSendCommand(wlan_private * priv,
                      cyg_uint16 cmd_no,
                      cyg_uint16 cmd_action,
                      cyg_uint16 wait_option, WLAN_OID cmd_oid, void *pdata_buf)
{
    int ret = WLAN_STATUS_SUCCESS;
    wlan_adapter *Adapter = priv->adapter;
    CmdCtrlNode *CmdNode;
    HostCmd_DS_COMMAND *CmdPtr;
    int x;

    ENTER();

    if (!Adapter) {
        WLAN_DEBUG_PRINTF("PREP_CMD: Adapter is Null\n");
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    if (Adapter->IsDeepSleep == TRUE) {
        WLAN_DEBUG_PRINTF("PREP_CMD: Deep sleep enabled\n");
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    if (Adapter->SurpriseRemoved) {
        WLAN_DEBUG_PRINTF("PREP_CMD: Card is Removed\n");
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    CmdNode = GetFreeCmdCtrlNode(priv);

    if (CmdNode == NULL) {
        WLAN_DEBUG_PRINTF("PREP_CMD: No free CmdNode\n");

        /* Wake up main thread to execute next command */
//        wake_up_interruptible(&priv->MainThread.waitQ);

		cyg_flag_setbits( &priv->MainThread.waitQ_flag_q, 1 );
        
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    SetCmdCtrlNode(priv, CmdNode, cmd_oid, wait_option, pdata_buf);

    CmdPtr = (HostCmd_DS_COMMAND *) (CmdNode->BufVirtualAddr + SDIO_HEADER_LEN);//clyu

    WLAN_DEBUG_PRINTF("PREP_CMD: Val of Cmd ptr =0x%x, command=0x%X\n",
           (cyg_uint32) CmdPtr, cmd_no);

    if (!CmdPtr) {
        WLAN_DEBUG_PRINTF("PREP_CMD: BufVirtualAddr of CmdNode is NULL\n");
        CleanupAndInsertCmd(priv, CmdNode);
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    /* Set sequence number, command and INT option */
    Adapter->SeqNum++;
    CmdPtr->SeqNum = wlan_cpu_to_le16(Adapter->SeqNum);

    CmdPtr->Command = cmd_no;
    CmdPtr->Result = 0;

//    TX_EVENT_FLAGS_SET(&CmdNode->cmdwait_q, 0, TX_AND);
    switch (cmd_no) {
    case HostCmd_CMD_GET_HW_SPEC:
        ret = wlan_cmd_hw_spec(priv, CmdPtr);
        break;
    case HostCmd_CMD_802_11_PS_MODE:
        ret = wlan_cmd_802_11_ps_mode(priv, CmdPtr, cmd_action);
        break;

    case HostCmd_CMD_802_11_SCAN:
        ret = wlan_cmd_802_11_scan(priv, CmdPtr, pdata_buf);
        break;

    case HostCmd_CMD_MAC_CONTROL:
        ret = wlan_cmd_mac_control(priv, CmdPtr);
        break;

    case HostCmd_CMD_802_11_ASSOCIATE:
    case HostCmd_CMD_802_11_REASSOCIATE:
        ret = wlan_cmd_802_11_associate(priv, CmdPtr, pdata_buf);
        break;

    case HostCmd_CMD_802_11_DEAUTHENTICATE:
        ret = wlan_cmd_802_11_deauthenticate(priv, CmdPtr);
        break;

    case HostCmd_CMD_802_11_SET_WEP:
        ret = wlan_cmd_802_11_set_wep(priv, CmdPtr, cmd_oid);
        break;

    case HostCmd_CMD_802_11_AD_HOC_START:
        ret = wlan_cmd_802_11_ad_hoc_start(priv, CmdPtr, pdata_buf);
        break;
    case HostCmd_CMD_802_11_RESET:
        CmdPtr->Command = wlan_cpu_to_le16(cmd_no);
        CmdPtr->Size = wlan_cpu_to_le16(S_DS_GEN);
        break;

    case HostCmd_CMD_802_11_GET_LOG:
        ret = wlan_cmd_802_11_get_log(priv, CmdPtr);
        break;

    case HostCmd_CMD_802_11_AUTHENTICATE:
        ret = wlan_cmd_802_11_authenticate(priv, CmdPtr, pdata_buf);
        break;

    case HostCmd_CMD_802_11_GET_STAT:
        ret = wlan_cmd_802_11_get_stat(priv, CmdPtr);
        break;

    case HostCmd_CMD_802_11_SNMP_MIB:
        ret = wlan_cmd_802_11_snmp_mib(priv, CmdPtr,
                                       cmd_action, cmd_oid, pdata_buf);
        break;

    case HostCmd_CMD_MAC_REG_ACCESS:
    case HostCmd_CMD_BBP_REG_ACCESS:
    case HostCmd_CMD_RF_REG_ACCESS:
        ret = wlan_cmd_reg_access(priv, CmdPtr, cmd_action, pdata_buf);
        break;

    case HostCmd_CMD_802_11_RF_CHANNEL:
        ret = wlan_cmd_802_11_rf_channel(priv, CmdPtr, cmd_action, pdata_buf);
        break;

    case HostCmd_CMD_802_11_RF_TX_POWER:
        ret = wlan_cmd_802_11_rf_tx_power(priv, CmdPtr,
                                          cmd_action, pdata_buf);
        break;

    case HostCmd_CMD_802_11_RADIO_CONTROL:
        ret = wlan_cmd_802_11_radio_control(priv, CmdPtr, cmd_action);
        break;

    case HostCmd_CMD_802_11_RF_ANTENNA:
        ret = wlan_cmd_802_11_rf_antenna(priv, CmdPtr, cmd_action, pdata_buf);
        break;

    case HostCmd_CMD_802_11_RATE_ADAPT_RATESET:
        ret = wlan_cmd_802_11_rate_adapt_rateset(priv, CmdPtr, cmd_action);
        break;

    case HostCmd_CMD_MAC_MULTICAST_ADR:
        ret = wlan_cmd_mac_multicast_adr(priv, CmdPtr, cmd_action);
        break;

    case HostCmd_CMD_802_11_AD_HOC_JOIN:
        ret = wlan_cmd_802_11_ad_hoc_join(priv, CmdPtr, pdata_buf);
        break;

    case HostCmd_CMD_802_11_RSSI:
        ret = wlan_cmd_802_11_rssi(priv, CmdPtr);
        break;

    case HostCmd_CMD_802_11_AD_HOC_STOP:
        ret = wlan_cmd_802_11_ad_hoc_stop(priv, CmdPtr);
        break;

    case HostCmd_CMD_802_11_ENABLE_RSN:
        ret = wlan_cmd_802_11_enable_rsn(priv, CmdPtr, cmd_action);
        break;

    case HostCmd_CMD_802_11_KEY_MATERIAL:
        ret = wlan_cmd_802_11_key_material(priv, CmdPtr,
                                           cmd_action, cmd_oid, pdata_buf);
        break;

    case HostCmd_CMD_802_11_PAIRWISE_TSC:
        break;
    case HostCmd_CMD_802_11_GROUP_TSC:
        break;

    case HostCmd_CMD_802_11_MAC_ADDRESS:
        ret = wlan_cmd_802_11_mac_address(priv, CmdPtr, cmd_action);
        break;
    case HostCmd_CMD_802_11_CAL_DATA_EXT:
        ret = wlan_cmd_802_11_cal_data_ext(priv, CmdPtr, pdata_buf);
        break;

    case HostCmd_CMD_802_11_DEEP_SLEEP:
        CmdPtr->Command = wlan_cpu_to_le16(cmd_no);
        CmdPtr->Size = wlan_cpu_to_le16((cyg_uint16)
                                        (sizeof
                                         (HostCmd_DS_802_11_DEEP_SLEEP)));
        break;

    case HostCmd_CMD_802_11_HOST_SLEEP_CFG:
        ret = wlan_cmd_802_11_host_sleep_cfg(priv, CmdPtr, pdata_buf);
        break;
    case HostCmd_CMD_802_11_HOST_SLEEP_AWAKE_CONFIRM:
    case HostCmd_CMD_802_11_HOST_SLEEP_ACTIVATE: //JONO  
        CmdPtr->Command = wlan_cpu_to_le16(cmd_no);
        CmdPtr->Size = wlan_cpu_to_le16(S_DS_GEN);
        break;

    case HostCmd_CMD_802_11_EEPROM_ACCESS:
        ret = wlan_cmd_802_11_eeprom_access(priv, CmdPtr,
                                            cmd_action, pdata_buf);
        break;

#ifdef MFG_CMD_SUPPORT
    case HostCmd_CMD_MFG_COMMAND:
        ret = SendMfgCommand(priv, CmdNode);
        break;
#endif

    case HostCmd_CMD_802_11_SET_AFC:
    case HostCmd_CMD_802_11_GET_AFC:

        CmdPtr->Command = wlan_cpu_to_le16(cmd_no);
        CmdPtr->Size =
            wlan_cpu_to_le16(sizeof(HostCmd_DS_802_11_AFC) + S_DS_GEN);

        memmove((void *)&CmdPtr->params.afc,
                pdata_buf, sizeof(HostCmd_DS_802_11_AFC));

        ret = WLAN_STATUS_SUCCESS;
        goto done;

    case HostCmd_CMD_802_11D_DOMAIN_INFO:
        ret = wlan_cmd_802_11d_domain_info(priv, CmdPtr, cmd_no, cmd_action);
        break;

    case HostCmd_CMD_802_11_SLEEP_PARAMS:
        ret = wlan_cmd_802_11_sleep_params(priv, CmdPtr, cmd_action);
        break;
    case HostCmd_CMD_802_11_BCA_CONFIG_TIMESHARE:
        ret = wlan_cmd_802_11_bca_timeshare(priv, CmdPtr,
                                            cmd_action, pdata_buf);
        break;
    case HostCmd_CMD_802_11_INACTIVITY_TIMEOUT:
        ret = wlan_cmd_802_11_inactivity_timeout(priv, CmdPtr,
                                                 cmd_action, pdata_buf);
        break;
    case HostCmd_CMD_802_11_BG_SCAN_CONFIG:
        ret = wlan_cmd_802_11_bg_scan_config(priv, CmdPtr,
                                             cmd_action, pdata_buf);
        break;

    case HostCmd_CMD_802_11_BG_SCAN_QUERY:
        ret = wlan_cmd_802_11_bg_scan_query(priv, CmdPtr);
        break;

    case HostCmd_CMD_802_11_FW_WAKEUP_METHOD:
        ret = wlan_cmd_802_11_fw_wakeup_method(priv, CmdPtr,
                                               cmd_action, pdata_buf);
        break;

    case HostCmd_CMD_WMM_GET_STATUS:
        ret = wlan_cmd_wmm_get_status(priv, CmdPtr, pdata_buf);
        break;
    case HostCmd_CMD_WMM_ACK_POLICY:
        ret = wlan_cmd_wmm_ack_policy(priv, CmdPtr, pdata_buf);
        break;
#if 0
    case HostCmd_CMD_WMM_PRIO_PKT_AVAIL:
        break;
#endif

    case HostCmd_CMD_WMM_ADDTS_REQ:
        ret = wlan_cmd_wmm_addts_req(priv, CmdPtr, pdata_buf);
        break;
    case HostCmd_CMD_WMM_DELTS_REQ:
        ret = wlan_cmd_wmm_delts_req(priv, CmdPtr, pdata_buf);
        break;
    case HostCmd_CMD_WMM_QUEUE_CONFIG:
        ret = wlan_cmd_wmm_queue_config(priv, CmdPtr, pdata_buf);
        break;
    case HostCmd_CMD_WMM_QUEUE_STATS:
        ret = wlan_cmd_wmm_queue_stats(priv, CmdPtr, pdata_buf);
        break;
    case HostCmd_CMD_802_11_TPC_CFG:
        CmdPtr->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_TPC_CFG);
        CmdPtr->Size =
            wlan_cpu_to_le16(sizeof(HostCmd_DS_802_11_TPC_CFG) + S_DS_GEN);

        memmove((void *)&CmdPtr->params.tpccfg,
                pdata_buf, sizeof(HostCmd_DS_802_11_TPC_CFG));

        ret = WLAN_STATUS_SUCCESS;
        break;
    case HostCmd_CMD_802_11_LED_GPIO_CTRL:
        {
            MrvlIEtypes_LedGpio_t *gpio =
                (MrvlIEtypes_LedGpio_t *) CmdPtr->params.ledgpio.data;

            memmove((void *)&CmdPtr->params.ledgpio,
                    pdata_buf, sizeof(HostCmd_DS_802_11_LED_CTRL));

            CmdPtr->Command =
                wlan_cpu_to_le16(HostCmd_CMD_802_11_LED_GPIO_CTRL);

#define ACTION_NUMLED_TLVTYPE_LEN_FIELDS_LEN 8
            CmdPtr->Size = wlan_cpu_to_le16(gpio->Header.Len + S_DS_GEN
                                            +
                                            ACTION_NUMLED_TLVTYPE_LEN_FIELDS_LEN);
            gpio->Header.Len = wlan_cpu_to_le16(gpio->Header.Len);

            ret = WLAN_STATUS_SUCCESS;
            break;
        }
    case HostCmd_CMD_802_11_PWR_CFG:
        CmdPtr->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_PWR_CFG);
        CmdPtr->Size =
            wlan_cpu_to_le16(sizeof(HostCmd_DS_802_11_PWR_CFG) + S_DS_GEN);
        memmove((void *)&CmdPtr->params.pwrcfg, pdata_buf,
                sizeof(HostCmd_DS_802_11_PWR_CFG));

        ret = WLAN_STATUS_SUCCESS;
        break;
    case HostCmd_CMD_802_11_SLEEP_PERIOD:
        ret = wlan_cmd_802_11_sleep_period(priv, CmdPtr,
                                           cmd_action, pdata_buf);
        break;
    case HostCmd_CMD_GET_TSF:
        CmdPtr->Command = wlan_cpu_to_le16(HostCmd_CMD_GET_TSF);
        CmdPtr->Size = wlan_cpu_to_le16(sizeof(HostCmd_DS_GET_TSF)
                                        + S_DS_GEN);
        ret = WLAN_STATUS_SUCCESS;
        break;
    case HostCmd_CMD_802_11_TX_RATE_QUERY:
        CmdPtr->Command = wlan_cpu_to_le16(HostCmd_CMD_802_11_TX_RATE_QUERY);
        CmdPtr->Size =
            wlan_cpu_to_le16(sizeof(HostCmd_TX_RATE_QUERY) + S_DS_GEN);
        Adapter->TxRate = 0;
        ret = WLAN_STATUS_SUCCESS;
        break;
    case HostCmd_CMD_802_11_IBSS_COALESCING_STATUS:
        ret =
            wlan_cmd_802_11_IBSS_Coalesced_Status(priv, CmdPtr, cmd_action,
                                                  pdata_buf);
        break;

    default:
        WLAN_DEBUG_PRINTF("PREP_CMD: unknown command- %#x\n", cmd_no);
        ret = WLAN_STATUS_FAILURE;
        break;
    }

    /* return error, since the command preparation failed */
    if (ret != WLAN_STATUS_SUCCESS) {
        WLAN_DEBUG_PRINTF("PREP_CMD: Command preparation failed\n");
        CleanupAndInsertCmd(priv, CmdNode);
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    CmdNode->CmdWaitQWoken = FALSE;
	
	cyg_flag_maskbits(&CmdNode->cmdwait_flag_q, 0x0);//clyu clear all bits
	
    QueueCmd(Adapter, CmdNode, TRUE);
    //wake_up_interruptible(&priv->MainThread.waitQ);
    cyg_flag_setbits( &priv->MainThread.waitQ_flag_q, 1 ); 
    

    //sbi_reenable_host_interrupt(priv, 0x00);

    if (wait_option & HostCmd_OPTION_WAITFORRSP) {
        WLAN_DEBUG_PRINTF("PREP_CMD: Wait for CMD response\n");
        //wait_event_interruptible(CmdNode->cmdwait_q, CmdNode->CmdWaitQWoken);
 
        x = cyg_flag_wait(
            &CmdNode->cmdwait_flag_q,
            0x01,
            CYG_FLAG_WAITMODE_AND | CYG_FLAG_WAITMODE_CLR );

        if (Adapter->CurCmdRetCode) {
            WLAN_DEBUG_PRINTF("PREP_CMD: Command failed with return code=%d\n",
                   Adapter->CurCmdRetCode);
            Adapter->CurCmdRetCode = 0;
            ret = WLAN_STATUS_FAILURE;
        }
    }

  done:
    LEAVE();
    return ret;
}

/** 
 *  @brief This function allocates the command buffer and link
 *  it to command free queue.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @return 		WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
AllocateCmdBuffer(wlan_private * priv)
{
    int ret = WLAN_STATUS_SUCCESS;
    cyg_uint32 ulBufSize;
    cyg_uint32 i;
    CmdCtrlNode *TempCmdArray;
    cyg_uint8 *pTempVirtualAddr;
    wlan_adapter *Adapter = priv->adapter;

    ENTER();

    /* Allocate and initialize CmdCtrlNode */
    ulBufSize = sizeof(CmdCtrlNode) * MRVDRV_NUM_OF_CMD_BUFFER;

    MALLOC(TempCmdArray, CmdCtrlNode *, ulBufSize, 0, M_NOWAIT);
    if(!TempCmdArray)
    {
        WLAN_DEBUG_PRINTF("ALLOC_CMD_BUF: Failed to allocate TempCmdArray\n");
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    Adapter->CmdArray = TempCmdArray;
    memset(Adapter->CmdArray, 0, ulBufSize);

    /* Allocate and initialize command buffers */
    ulBufSize = MRVDRV_SIZE_OF_CMD_BUFFER;//2K
    for (i = 0; i < MRVDRV_NUM_OF_CMD_BUFFER; i++) {
        MALLOC(pTempVirtualAddr, cyg_uint8 *, ulBufSize, 0, M_NOWAIT);
        if(!pTempVirtualAddr)
        {
            WLAN_DEBUG_PRINTF("ALLOC_CMD_BUF: pTempVirtualAddr: out of memory\n");
            ret = WLAN_STATUS_FAILURE;
            goto done;
        }

        memset(pTempVirtualAddr, 0, ulBufSize);

        /* Update command buffer virtual */
        TempCmdArray[i].BufVirtualAddr = (cyg_uint8*)((cyg_uint32)pTempVirtualAddr | NON_CACHE_FLAG);
    }

    for (i = 0; i < MRVDRV_NUM_OF_CMD_BUFFER; i++) {
        //init_waitqueue_head(&TempCmdArray[i].cmdwait_q);
        cyg_flag_init(&TempCmdArray[i].cmdwait_flag_q);
        CleanupAndInsertCmd(priv, &TempCmdArray[i]);
    }

    ret = WLAN_STATUS_SUCCESS;
  done:
    LEAVE();
    return ret;
}

/** 
 *  @brief This function frees the command buffer.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @return 		WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
FreeCmdBuffer(wlan_private * priv)
{
    cyg_uint32 ulBufSize;
    cyg_uint32 i;
    CmdCtrlNode *TempCmdArray;
    wlan_adapter *Adapter = priv->adapter;

    ENTER();

    /* need to check if cmd array is allocated or not */
    if (Adapter->CmdArray == NULL) {
        WLAN_DEBUG_PRINTF("FREE_CMD_BUF: CmdArray is Null\n");
        goto done;
    }

    TempCmdArray = Adapter->CmdArray;

    /* Release shared memory buffers */
    ulBufSize = MRVDRV_SIZE_OF_CMD_BUFFER;
    for (i = 0; i < MRVDRV_NUM_OF_CMD_BUFFER; i++) {
        if (TempCmdArray[i].BufVirtualAddr) {
            WLAN_DEBUG_PRINTF("Free all the array\n");
            FREE(TempCmdArray[i].BufVirtualAddr, 0);
            TempCmdArray[i].BufVirtualAddr = NULL;
        }
    }

    /* Release CmdCtrlNode */
    if (Adapter->CmdArray) {
        WLAN_DEBUG_PRINTF("Free CmdArray\n");
        FREE(Adapter->CmdArray, 0);
        Adapter->CmdArray = NULL;
    }

  done:
    LEAVE();
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function gets a free command node if available in
 *  command free queue.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @return CmdCtrlNode A pointer to CmdCtrlNode structure or NULL
 */
CmdCtrlNode *
GetFreeCmdCtrlNode(wlan_private * priv)
{
    CmdCtrlNode *TempNode;
    wlan_adapter *Adapter = priv->adapter;
    ulong flags;

    ENTER();

    if (!Adapter)
        return NULL;

    //spin_lock_irqsave(&Adapter->QueueSpinLock, flags);
    OS_INT_DISABLE(flags);

    if (!list_empty(&Adapter->CmdFreeQ)) {
        TempNode = (CmdCtrlNode *) Adapter->CmdFreeQ.next;
        list_del((struct list_head *) TempNode);
    } else {
        WLAN_DEBUG_PRINTF("GET_CMD_NODE: CmdCtrlNode is not available\n");
        TempNode = NULL;
    }
		OS_INT_RESTORE(flags);
    //spin_unlock_irqrestore(&Adapter->QueueSpinLock, flags);

    if (TempNode) {
       // WLAN_DEBUG_PRINTF("GET_CMD_NODE: CmdCtrlNode available\n");
       // WLAN_DEBUG_PRINTF("GET_CMD_NODE: CmdCtrlNode Address = %p\n", TempNode);
        CleanUpCmdCtrlNode(TempNode);
    }

    LEAVE();
    return TempNode;
}

/** 
 *  @brief This function cleans command node.
 *  
 *  @param pTempNode	A pointer to CmdCtrlNode structure
 *  @return 		n/a
 */
void
CleanUpCmdCtrlNode(CmdCtrlNode * pTempNode)
{
    ENTER();

    if (!pTempNode)
        return;
    pTempNode->CmdWaitQWoken = TRUE;
    //wake_up_interruptible(&pTempNode->cmdwait_q);

    cyg_flag_setbits( &pTempNode->cmdwait_flag_q, 3 ); 
    pTempNode->Status = 0;
    pTempNode->cmd_oid = (WLAN_OID) 0;
    pTempNode->wait_option = 0;
    pTempNode->pdata_buf = NULL;

    if (pTempNode->BufVirtualAddr != NULL)
        memset(pTempNode->BufVirtualAddr, 0, MRVDRV_SIZE_OF_CMD_BUFFER);

    LEAVE();
    return;
}

/** 
 *  @brief This function initializes the command node.
 *  
 *  @param priv		A pointer to wlan_private structure
 *  @param pTempNode	A pointer to CmdCtrlNode structure
 *  @param cmd_oid	cmd oid: treated as sub command
 *  @param wait_option	wait option: wait response or not
 *  @param pdata_buf	A pointer to informaion buffer
 *  @return 		WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
void
SetCmdCtrlNode(wlan_private * priv,
               CmdCtrlNode * pTempNode,
               WLAN_OID cmd_oid, cyg_uint16 wait_option, void *pdata_buf)
{
    ENTER();

    if (!pTempNode)
        return;

    pTempNode->cmd_oid = cmd_oid;
    pTempNode->wait_option = wait_option;
    pTempNode->pdata_buf = pdata_buf;

    LEAVE();
}

/** 
 *  @brief This function executes next command in command
 *  pending queue. It will put fimware back to PS mode
 *  if applicable.
 *  
 *  @param priv     A pointer to wlan_private structure
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
ExecuteNextCommand(wlan_private * priv)
{
    wlan_adapter *Adapter = priv->adapter;
    CmdCtrlNode *CmdNode = NULL;
    HostCmd_DS_COMMAND *CmdPtr;
    ulong flags;
    int ret = WLAN_STATUS_SUCCESS;

    ENTER();

    if (!Adapter) {
        WLAN_DEBUG_PRINTF("EXEC_NEXT_CMD: Adapter is NULL\n");
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    if (Adapter->IsDeepSleep == TRUE) {
        WLAN_DEBUG_PRINTF("EXEC_NEXT_CMD: Device is in deep sleep mode.\n");
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    //spin_lock_irqsave(&Adapter->QueueSpinLock, flags);
    OS_INT_DISABLE(flags);

    if (Adapter->CurCmd) {
        WLAN_DEBUG_PRINTF("EXEC_NEXT_CMD: there is command in processing!\n");
        //spin_unlock_irqrestore(&Adapter->QueueSpinLock, flags);
        OS_INT_RESTORE(flags);
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    if (!list_empty(&Adapter->CmdPendingQ)) {
        CmdNode = (CmdCtrlNode *)
            Adapter->CmdPendingQ.next;
    }

    //spin_unlock_irqrestore(&Adapter->QueueSpinLock, flags);
    OS_INT_RESTORE(flags);

    if (CmdNode) {
        //WLAN_DEBUG_PRINTF("EXEC_NEXT_CMD: Got next command from CmdPendingQ\n");
        CmdPtr = (HostCmd_DS_COMMAND *) (CmdNode->BufVirtualAddr + SDIO_HEADER_LEN);//clyu SDIO_HEADER_LEN

        if (Is_Command_Allowed_In_PS(CmdPtr->Command)) {
            if ((Adapter->PSState == PS_STATE_SLEEP)
                || (Adapter->PSState == PS_STATE_PRE_SLEEP)
                ) {
                WLAN_DEBUG_PRINTF(
                       "EXEC_NEXT_CMD: Cannot send cmd 0x%x in PSState %d\n",
                       CmdPtr->Command, Adapter->PSState);
                ret = WLAN_STATUS_FAILURE;
                goto done;
            }
            WLAN_DEBUG_PRINTF("EXEC_NEXT_CMD: OK to send command "
                   "0x%x in PSState %d\n", CmdPtr->Command, Adapter->PSState);
        } else if (Adapter->PSState != PS_STATE_FULL_POWER) {
            /*
             * 1. Non-PS command: 
             * Queue it. set NeedToWakeup to TRUE if current state 
             * is SLEEP, otherwise call PSWakeup to send Exit_PS.
             * 2. PS command but not Exit_PS: 
             * Ignore it.
             * 3. PS command Exit_PS:
             * Set NeedToWakeup to TRUE if current state is SLEEP, 
             * otherwise send this command down to firmware
             * immediately.
             */
            if (CmdPtr->Command !=
                wlan_cpu_to_le16(HostCmd_CMD_802_11_PS_MODE)) {
                /*  Prepare to send Exit PS,
                 *  this non PS command will be sent later */
                if ((Adapter->PSState == PS_STATE_SLEEP)
                    || (Adapter->PSState == PS_STATE_PRE_SLEEP)
                    ) {
                    /* w/ new scheme, it will not reach here.
                       since it is blocked in main_thread. */
                    Adapter->NeedToWakeup = TRUE;
                } else
                    PSWakeup(priv, 0);

                ret = WLAN_STATUS_SUCCESS;
                goto done;
            } else {
                /*
                 * PS command. Ignore it if it is not Exit_PS. 
                 * otherwise send it down immediately.
                 */
                HostCmd_DS_802_11_PS_MODE *psm = &CmdPtr->params.psmode;

                WLAN_DEBUG_PRINTF("EXEC_NEXT_CMD: PS cmd- Action=0x%x\n",
                       psm->Action);
                if (psm->Action != wlan_cpu_to_le16(HostCmd_SubCmd_Exit_PS)) {
                    WLAN_DEBUG_PRINTF("EXEC_NEXT_CMD: Ignore Enter PS cmd\n");
                    list_del((struct list_head *) CmdNode);
                    CleanupAndInsertCmd(priv, CmdNode);

                    ret = WLAN_STATUS_SUCCESS;
                    goto done;
                }

                if ((Adapter->PSState == PS_STATE_SLEEP)
                    || (Adapter->PSState == PS_STATE_PRE_SLEEP)
                    ) {
                    WLAN_DEBUG_PRINTF(
                           "EXEC_NEXT_CMD: Ignore ExitPS cmd in sleep\n");
                    list_del((struct list_head *) CmdNode);
                    CleanupAndInsertCmd(priv, CmdNode);
                    Adapter->NeedToWakeup = TRUE;

                    ret = WLAN_STATUS_SUCCESS;
                    goto done;
                }

                WLAN_DEBUG_PRINTF("EXEC_NEXT_CMD: Sending Exit_PS down...\n");
            }
        }
        list_del((struct list_head *) CmdNode);
        WLAN_DEBUG_PRINTF("EXEC_NEXT_CMD: Sending 0x%04X Command\n",
               CmdPtr->Command);
        DownloadCommandToStation(priv, CmdNode);
        
    } else {
        /*
         * check if in power save mode, if yes, put the device back
         * to PS mode
         */
        if ((Adapter->PSMode != Wlan802_11PowerModeCAM) &&
            (Adapter->PSState == PS_STATE_FULL_POWER) &&
            (Adapter->MediaConnectStatus == WlanMediaStateConnected)) {
            if (Adapter->SecInfo.WPAEnabled || Adapter->SecInfo.WPA2Enabled) {
                if (Adapter->IsGTK_SET) {
                    WLAN_DEBUG_PRINTF("EXEC_NEXT_CMD: WPA enabled and GTK_SET"
                           " go back to PS_SLEEP");
                    PSSleep(priv, 0);
                }
            } else {
                  if ((Adapter->InfrastructureMode != Wlan802_11IBSS) || Adapter->CurBssParams.BSSDescriptor.ATIMWindow) { //JONO
                    WLAN_DEBUG_PRINTF("EXEC_NEXT_CMD: Command PendQ is empty,"
                           " go back to PS_SLEEP");
                    PSSleep(priv, 0);
                }
            }
        }
        
        //cyg_thread_delay(2);
    }

    ret = WLAN_STATUS_SUCCESS;
  done:
    LEAVE();
    return ret;
}

/** 
 *  @brief This function sends customized event to application.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @para str	   A pointer to event string
 *  @return 	   n/a
 */
void
send_iwevcustom_event(wlan_private * priv, cyg_int8 * str)
{
#if WIRELESS_EXT > 14
    union iwreq_data iwrq;
    cyg_uint8 buf[50];

    ENTER();

    memset(&iwrq, 0, sizeof(union iwreq_data));
    memset(buf, 0, sizeof(buf));

    snprintf(buf, sizeof(buf) - 1, "%s", str);

    iwrq.data.pointer = (char*)buf;
    iwrq.data.length = strlen((char*)buf) + 1 + IW_EV_LCP_LEN;

    /* Send Event to upper layer */
    WLAN_DEBUG_PRINTF("Event Indication string = %s\n",
           (char *) iwrq.data.pointer);
    WLAN_DEBUG_PRINTF("Event Indication String Length = %d\n", iwrq.data.length);

    WLAN_DEBUG_PRINTF("Sending wireless event IWEVCUSTOM for %s\n", str);
    wireless_send_event(priv->wlan_dev.netdev, IWEVCUSTOM, &iwrq, (char*)buf);

    LEAVE();
#endif
    return;
}


static volatile BOOL g_bHostSleepFlag = FALSE;

void wifiSetHostSleepFlag(BOOL bSleep)
{
	g_bHostSleepFlag = bSleep;
}

BOOL wifiIsHostSleep()
{
	return g_bHostSleepFlag;
}

/** 
 *  @brief This function sends sleep confirm command to firmware.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @param cmdptr  A pointer to the command
 *  @param size	   the size of command
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
SendConfirmSleep(wlan_private * priv, cyg_uint8 * CmdPtr, cyg_uint16 size)
{
    wlan_adapter *Adapter = priv->adapter;
    int ret = WLAN_STATUS_SUCCESS;
    int volatile save_irq;
    int volatile sdio_int;
 
    ENTER();
 
 sdio_int = inpw(REG_SDIER);
 //if((sdio_int & 0x10) == 0x10)
 // sdio_disable_SDIO_INT();
    WLAN_DEBUG_PRINTF("SEND_SLEEPC_CMD: Before download, Size of cmd = %d, IntC=%d\n",
           size, Adapter->IntCounter);
 
    HEXDUMP("SEND_SLEEPC_CMD: Sleep confirm Command", CmdPtr, size);
    Adapter->PSState = PS_STATE_SLEEP; //JONO
 
    ret = sbi_host_to_card(priv, MVMS_CMD, CmdPtr, size);
    priv->wlan_dev.dnld_sent = DNLD_RES_RECEIVED;
    Adapter->IntCounter = 0;//clyu
 
    if (ret) {
        WLAN_DEBUG_PRINTF(
               "SEND_SLEEPC_CMD: Host to Card Failed for Confirm Sleep\n");
        outpw(REG_SDIER, sdio_int);
    } else {
        sdio_clear_imask(((mmc_card_t) ((priv->wlan_dev).card))->ctrlr);
        //OS_INT_DISABLE(save_irq);
        if (!Adapter->IntCounter) {
            Adapter->PSState = PS_STATE_SLEEP;
            WLAN_DEBUG_PRINTF("*********psstate === sleep %d,%x**********\n", Adapter->PSState, &Adapter->PSState);
        } else {
            WLAN_DEBUG_PRINTF("SEND_SLEEPC_CMD: After sent,IntC=%d\n",
                   Adapter->IntCounter);
        }
        //OS_INT_RESTORE(save_irq);
        outpw(REG_SDIER, sdio_int);
 
        if (Adapter->PSState == PS_STATE_SLEEP &&
            Adapter->bHostSleepConfigured &&
            (Adapter->sleep_period.period == 0)) {
            Adapter->bWakeupDevRequired = TRUE;
            wlan_host_sleep_activated_event(priv);
            
            wifiSetHostSleepFlag(TRUE);
            diag_printf("-----------Suspend success------------\n");
        }
 
        WLAN_DEBUG_PRINTF("SEND_SLEEPC_CMD: Sent Confirm Sleep command\n");
        WLAN_DEBUG_PRINTF("+");
    }
 
    LEAVE();
    return ret;
}

/** 
 *  @brief This function sends Enter_PS command to firmware.
 *  
 *  @param priv    	A pointer to wlan_private structure
 *  @param wait_option	wait response or not
 *  @return 	   	n/a 
 */
void
PSSleep(wlan_private * priv, int wait_option)
{

    ENTER();

    PrepareAndSendCommand(priv, HostCmd_CMD_802_11_PS_MODE,
                          HostCmd_SubCmd_Enter_PS, wait_option, 0, NULL);

    LEAVE();
    return;
}

/** 
 *  @brief This function sends Eixt_PS command to firmware.
 *  
 *  @param priv    	A pointer to wlan_private structure
 *  @param wait_option	wait response or not
 *  @return 	   	n/a 
 */
void
PSWakeup(wlan_private * priv, int wait_option)
{
    WLAN_802_11_POWER_MODE LocalPSMode;

    ENTER();

    LocalPSMode = Wlan802_11PowerModeCAM;

    WLAN_DEBUG_PRINTF("Exit_PS: LocalPSMode = %d\n", LocalPSMode);

    PrepareAndSendCommand(priv, HostCmd_CMD_802_11_PS_MODE,
                          HostCmd_SubCmd_Exit_PS,
                          wait_option, 0, &LocalPSMode);

    LEAVE();
    return;
}

/** 
 *  @brief This function checks condition and prepares to
 *  send sleep confirm command to firmware if ok.
 *  
 *  @param priv    	A pointer to wlan_private structure
 *  @param PSMode  	Power Saving mode
 *  @return 	   	n/a 
 */
void
PSConfirmSleep(wlan_private * priv, cyg_uint16 PSMode)
{
    wlan_adapter *Adapter = priv->adapter;
    BOOLEAN allowed = TRUE;

    ENTER();

    if (priv->wlan_dev.dnld_sent) {
        allowed = FALSE;
        WLAN_DEBUG_PRINTF("D");
    } else if (Adapter->CurCmd) {
        allowed = FALSE;
        WLAN_DEBUG_PRINTF("C");
    } else if (Adapter->IntCounter > 0) {
        allowed = FALSE;
        WLAN_DEBUG_PRINTF("I%d", Adapter->IntCounter);
    }

    if (allowed) {
        WLAN_DEBUG_PRINTF("Sending PSConfirmSleep\n");
        SendConfirmSleep(priv, (cyg_uint8 *) ((cyg_uint32)&Adapter->PSConfirmSleep | NON_CACHE_FLAG),
                         sizeof(PS_CMD_ConfirmSleep));
		WLAN_DEBUG_PRINTF("Pause W99802 after SendConfirmSleep to WiFi\n");
		//cyg_interrupt_disable();while(1);//xhchen
    } else {
        WLAN_DEBUG_PRINTF("Sleep Confirm has been delayed\n");
    }

    LEAVE();
}

void WirelessDrvCmdAlarm(wlan_private * priv)
{
	cyg_handle_t h;
    cyg_clock_to_counter(cyg_real_time_clock(), &h);
	cyg_alarm_create(h, (cyg_alarm_t* )MrvDrvCommandTimerFunction, (cyg_addrword_t)priv, &cmd_alarm_handle, &cmd_alarm);
    //cyg_alarm_initialize(cmd_alarm_handle, cyg_current_time()+500, 500);
//    cyg_alarm_disable();
}

void WirelessDrvAlarm(wlan_private * priv)
{
	cyg_handle_t h;
    cyg_clock_to_counter(cyg_real_time_clock(), &h);
	cyg_alarm_create(h, (cyg_alarm_t* )MrvDrvTimerFunction, (cyg_addrword_t)priv, &reassociation_alarm_handle, &reassociation_alarm);
    //cyg_alarm_initialize(cmd_alarm_handle, cyg_current_time()+500, 500);
//    cyg_alarm_disable();
}


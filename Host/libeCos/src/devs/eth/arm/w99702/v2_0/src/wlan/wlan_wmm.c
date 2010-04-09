/** @file wlan_wmm.c
 * @brief This file contains functions for WMM.
 * 
 *  Copyright © Marvell International Ltd. and/or its affiliates, 2003-2006
 */
/********************************************************
Change log:
    10/04/05: Add Doxygen format comments
    11/11/05: Add support for WMM Status change event
    01/05/06: Add kernel 2.6.x support  
    01/11/06: Conditionalize new scan/join code modifications.
    04/06/06: Add TSPEC, queue metrics, and MSDU expiry support
********************************************************/
#include    "include.h"
#include 	"asm/uaccess.h"
#include    "wlan_wmm.h"

/********************************************************
        Local Variables
********************************************************/

#define IPTOS_OFFSET 5

static cyg_uint8 wmm_tos2ac[8] = {
    AC_PRIO_BE,
    AC_PRIO_BK,
    AC_PRIO_BK,
    AC_PRIO_BE,
    AC_PRIO_VI,
    AC_PRIO_VI,
    AC_PRIO_VO,
    AC_PRIO_VO
};

static cyg_uint8 wmm_ac_downgrade[MAX_AC_QUEUES] = {
    AC_PRIO_BK,
    AC_PRIO_BE,
    AC_PRIO_VI,
    AC_PRIO_VO
};

/* This mapping table will be useful if bit-flip is needed */
static cyg_uint8 wmm_tos2priority[8] = {
/*  Priority   DSCP   DSCP   DSCP   WMM
    P2     P1     P0    AC    */
    0x00,                       /*  0      0      0 AC_BE */
    0x01,                       /*  0      0      1 AC_BK */
    0x02,                       /*  0      1      0 AC_BK */
    0x03,                       /*  0      1      1 AC_BE */
    0x04,                       /*  1      0      0 AC_VI */
    0x05,                       /*  1      0      1 AC_VI */
    0x06,                       /*  1      1      0 AC_VO */
    0x07                        /*  1      1      1 AC_VO */
};

static cyg_uint8 wmm_ie[WMM_IE_LENGTH] = { WMM_IE, 0x07, 0x00,
    0x50, 0xf2, 0x02, 0x00, 0x01, 0x00
};

/********************************************************
        Global Variables
********************************************************/

/********************************************************
        Local Functions
********************************************************/
#ifdef DEBUG_LEVEL4
/** 
 *  @brief Debug print function to display the priority parameters for a WMM AC
 *
 *  @param acStr    String pointer giving the AC enumeration (BK, BE, VI, VO)
 *  @param pACParam Pointer to the AC paramters to display
 *
 *  @return         void
 */
static void
wmm_debugPrintAC(const char *acStr, const WMM_AC_PARAS * pACParam)
{
    diag_printf("WMM AC_%s: ACI=%d, ACM=%d, AIFSN=%d, "
           "ECWmin=%d, ECWmax=%d, Txop_Limit=%d\n",
           acStr, pACParam->ACI_AIFSN.ACI, pACParam->ACI_AIFSN.ACM,
           pACParam->ACI_AIFSN.AIFSN, pACParam->ECW.ECW_Min,
           pACParam->ECW.ECW_Max, wlan_le16_to_cpu(pACParam->Txop_Limit));
}

#define PRINTM_AC(acStr, pACParam) wmm_debugPrintAC(acStr, pACParam)
#else
#define PRINTM_AC(acStr, pACParam)
#endif

/** 
 *  @brief Initialize WMM priority queues
 *
 *  @param priv     Pointer to the wlan_private driver data struct
 *
 *  @return         void
 */
static void
wmm_setup_queue_priorities(wlan_private * priv)
{
    wlan_adapter *Adapter = priv->adapter;
    WMM_PARAMETER_IE *pIe;
    cyg_uint16 cwmax, cwmin, avg_back_off, tmp[4];
    int i, j, n;

    n = 0;

    pIe = &Adapter->CurBssParams.BSSDescriptor.wmmIE;

    HEXDUMP("WMM: setup_queue_priorities: param IE",
            (cyg_uint8 *) pIe, sizeof(WMM_PARAMETER_IE));

    diag_printf("WMM Parameter IE: version=%d, "
           "QoSInfo Parameter Set Count=%d, Reserved=%#x\n",
           pIe->Version, pIe->QoSInfo.ParaSetCount, pIe->Reserved);

    /*
     * AC_BE
     */
    cwmax = (1 << pIe->AC_Paras_BE.ECW.ECW_Max) - 1;
    cwmin = (1 << pIe->AC_Paras_BE.ECW.ECW_Min) - 1;
    avg_back_off = (cwmin >> 1) + pIe->AC_Paras_BE.ACI_AIFSN.AIFSN;
    Adapter->CurBssParams.wmm_queue_prio[n] = AC_PRIO_BE;
    tmp[n++] = avg_back_off;

    PRINTM_AC("BE", &pIe->AC_Paras_BE);
    diag_printf("WMM AC_BE: CWmax=%d CWmin=%d Avg Back-off=%d\n",
           cwmax, cwmin, avg_back_off);

    /*
     * AC_BK
     */
    cwmax = (1 << pIe->AC_Paras_BK.ECW.ECW_Max) - 1;
    cwmin = (1 << pIe->AC_Paras_BK.ECW.ECW_Min) - 1;
    avg_back_off = (cwmin >> 1) + pIe->AC_Paras_BK.ACI_AIFSN.AIFSN;
    Adapter->CurBssParams.wmm_queue_prio[n] = AC_PRIO_BK;
    tmp[n++] = avg_back_off;

    PRINTM_AC("BK", &pIe->AC_Paras_BK);
    diag_printf("WMM AC_BK: CWmax=%d CWmin=%d Avg Back-off=%d\n",
           cwmax, cwmin, avg_back_off);

    /*
     * AC_VI
     */
    cwmax = (1 << pIe->AC_Paras_VI.ECW.ECW_Max) - 1;
    cwmin = (1 << pIe->AC_Paras_VI.ECW.ECW_Min) - 1;
    avg_back_off = (cwmin >> 1) + pIe->AC_Paras_VI.ACI_AIFSN.AIFSN;
    Adapter->CurBssParams.wmm_queue_prio[n] = AC_PRIO_VI;
    tmp[n++] = avg_back_off;

    PRINTM_AC("VI", &pIe->AC_Paras_VI);
    diag_printf("WMM AC_VI: CWmax=%d CWmin=%d Avg Back-off=%d\n",
           cwmax, cwmin, avg_back_off);

    /*
     * AC_VO
     */
    cwmax = (1 << pIe->AC_Paras_VO.ECW.ECW_Max) - 1;
    cwmin = (1 << pIe->AC_Paras_VO.ECW.ECW_Min) - 1;
    avg_back_off = (cwmin >> 1) + pIe->AC_Paras_VO.ACI_AIFSN.AIFSN;
    Adapter->CurBssParams.wmm_queue_prio[n] = AC_PRIO_VO;
    tmp[n++] = avg_back_off;

    PRINTM_AC("VO", &pIe->AC_Paras_VO);
    diag_printf("WMM AC_VO: CWmax=%d CWmin=%d Avg Back-off=%d\n",
           cwmax, cwmin, avg_back_off);

    HEXDUMP("WMM avg_back_off  ", (cyg_uint8 *) tmp, sizeof(tmp));
    HEXDUMP("WMM wmm_queue_prio", Adapter->CurBssParams.wmm_queue_prio,
            sizeof(Adapter->CurBssParams.wmm_queue_prio));

    /* bubble sort */
    for (i = 0; i < n; i++) {
        for (j = 1; j < n - i; j++) {
            if (tmp[j - 1] > tmp[j]) {
                SWAP_U16(tmp[j - 1], tmp[j]);
                SWAP_U8(Adapter->CurBssParams.wmm_queue_prio[j - 1],
                        Adapter->CurBssParams.wmm_queue_prio[j]);
            } else if (tmp[j - 1] == tmp[j]) {
                if (Adapter->CurBssParams.wmm_queue_prio[j - 1] <
                    Adapter->CurBssParams.wmm_queue_prio[j]) {
                    SWAP_U8(Adapter->CurBssParams.wmm_queue_prio[j - 1],
                            Adapter->CurBssParams.wmm_queue_prio[j]);
                }
            }
        }
    }

    HEXDUMP("WMM avg_back_off ", (cyg_uint8 *) tmp, sizeof(tmp));
    HEXDUMP("WMM wmm_queue_prio", Adapter->CurBssParams.wmm_queue_prio,
            sizeof(Adapter->CurBssParams.wmm_queue_prio));

}

#if 0
/** 
 *  @brief pop up the highest skb from wmm queue
 *
 *  @param priv     Pointer to the wlan_private driver data struct
 *
 *  @return         void
 */
static void
wmm_pop_highest_prio_skb(wlan_private * priv)
{
    int i;
    wlan_adapter *Adapter = priv->adapter;
    cyg_uint8 ac;

    for (i = 0; i < MAX_AC_QUEUES; i++) {
        ac = Adapter->CurBssParams.wmm_queue_prio[i];
        if (!list_empty((struct list_head *) &Adapter->wmm.TxSkbQ[ac])) {
            Adapter->CurrentTxSkb = Adapter->wmm.TxSkbQ[ac].next;
            list_del((struct list_head *) Adapter->wmm.TxSkbQ[ac].next);
            break;
        }
    }
}
#endif

/** 
 *  @brief Evaluate whether or not an AC is to be downgraded
 *
 *  @param priv     Pointer to the wlan_private driver data struct
 *  @param evalAC   AC to evaluate for downgrading
 *
 *  @return WMM AC the evalAC traffic is to be sent on.  
 */
static wlan_wmm_ac_e
wmm_eval_downgrade_ac(wlan_private * priv, wlan_wmm_ac_e evalAC)
{
    wlan_wmm_ac_e downAC;
    wlan_wmm_ac_e retAC;
    WMM_AC_STATUS *pACStatus;

    pACStatus = &priv->adapter->wmm.acStatus[evalAC];

    if (pACStatus->Disabled == FALSE) {
        /* Okay to use this AC, its enabled */
        return evalAC;
    }

    /* Setup a default return value of the lowest priority */
    retAC = AC_PRIO_BK;

    /*
     *  Find the highest AC that is enabled and does not require admission
     *    control.  The spec disallows downgarding to an AC which is enabled
     *    due to a completed admission control.  Unadmitted traffic is not 
     *    to be sent on an AC with admitted traffic.
     */
    for (downAC = AC_PRIO_BK; downAC < evalAC; downAC++) {
        pACStatus = &priv->adapter->wmm.acStatus[downAC];

        if ((pACStatus->Disabled == FALSE)
            && (pACStatus->FlowRequired == FALSE)) {
            /* AC is enabled and does not require admission control */
            retAC = downAC;
        }
    }

    return retAC;
}

/** 
 *  @brief Downgrade WMM priority queue
 *
 *  @param priv     Pointer to the wlan_private driver data struct
 *
 *  @return         void
 */
static void
wmm_setup_ac_downgrade(wlan_private * priv)
{
    wlan_wmm_ac_e acVal;

    diag_printf("WMM: AC Priorities: BK(0), BE(1), VI(2), VO(3)\n");

    for (acVal = AC_PRIO_BK; acVal <= AC_PRIO_VO; acVal++) {
        wmm_ac_downgrade[acVal] = wmm_eval_downgrade_ac(priv, acVal);
        diag_printf("WMM: AC PRIO %d maps to %d\n",
               acVal, wmm_ac_downgrade[acVal]);
    }
}

/** 
 *  @brief Send cmd to FW to announce package available
 *
 *  @param priv     Pointer to the wlan_private driver data struct
 *
 *  @return         void
 */
static void
wmm_send_prio_pkt_avail(wlan_private * priv)
{
#if 0                           /* WMM_PRIO_PKT_AVAIL command not supported for now */
    int i;

    for (i = 0; i < MAX_AC_QUEUES; i++) {
        ac = Adapter->CurBssParams.wmm_queue_prio[i];
        if (!list_empty((struct list_head *)
                        &priv->adapter->wmm.TxSkbQ[ac]))
            break;

        if (i >= MAX_AC_QUEUES) /* No High prio packets available */
            return;

        priv->adapter->priopktavail.PacketAC = ac;

        PrepareAndSendCommand(priv, HostCmd_CMD_WMM_PRIO_PKT_AVAIL,
                              0, 0, 0, NULL);
    }
#endif
}

/** 
 *  @brief implement WMM enable command
 *
 *  @param priv     Pointer to the wlan_private driver data struct
 *  @param wrq      Pointer to user data
 *
 *  @return         WLAN_STATUS_SUCCESS if success; otherwise <0
 */
int
wlan_wmm_enable_ioctl(wlan_private * priv, struct iwreq *wrq)
{
    wlan_adapter *Adapter = priv->adapter;
    ulong flags;
    int data, data1;
    int *val;

    ENTER();

    data = *((int *) (wrq->u.name + SUBCMD_OFFSET));
    switch (data) {
    case CMD_DISABLED:         /* disable */
        if (Adapter->MediaConnectStatus == WlanMediaStateConnected) {
            return -EPERM;
        }

 //       spin_lock_irqsave(&Adapter->CurrentTxLock, flags);
 				OS_INT_DISABLE(flags);
        Adapter->wmm.required = 0;
        if (!Adapter->wmm.enabled) {
//            spin_unlock_irqrestore(&Adapter->CurrentTxLock, flags);
						OS_INT_RESTORE(flags);
            data1 = Adapter->wmm.required;
            val = (int *) wrq->u.name;
            *val = data;
            return WLAN_STATUS_SUCCESS;
        } else {
            Adapter->wmm.enabled = 0;
        }
	#if 0
        if (Adapter->CurrentTxSkb) {
            //kfree_skb(Adapter->CurrentTxSkb);
            OS_INT_DISABLE(flags);
            Adapter->CurrentTxSkb = NULL;
            OS_INT_RESTORE(flags);
            priv->stats.tx_dropped++;
        }
   #endif

        /* Release all skb's in all the queues */
        wmm_cleanup_queues(priv);

  //      spin_unlock_irqrestore(&Adapter->CurrentTxLock, flags);
  			OS_INT_RESTORE(flags);
        Adapter->CurrentPacketFilter &= ~HostCmd_ACT_MAC_WMM_ENABLE;
        SetMacPacketFilter(priv);
        break;

    case CMD_ENABLED:          /* enable */
        if (Adapter->MediaConnectStatus == WlanMediaStateConnected) {
            return -EPERM;
        }
     //   spin_lock_irqsave(&Adapter->CurrentTxLock, flags);
     		OS_INT_DISABLE(flags);
        Adapter->wmm.required = 1;
        OS_INT_RESTORE(flags);
     //   spin_unlock_irqrestore(&Adapter->CurrentTxLock, flags);
        break;

    case CMD_GET:
        break;
    default:
        diag_printf("Invalid option\n");
        return -EINVAL;
    }
    data = Adapter->wmm.required;
    val = (int *) wrq->u.name;
    *val = data;
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief Set WMM IE
 *
 *  @param priv     Pointer to the wlan_private driver data struct
 *  @param req      Pointer to user data containing WMM IE
 *
 *  @return         WLAN_STATUS_SUCCESS if success; otherwise <0
 */
int
wlan_wmm_para_ie_ioctl(wlan_private * priv, struct ifreq *req)
{
    cyg_uint16 Action;
    cyg_uint8 *para_ie = (void *)priv->adapter->wmm.Para_IE;

    Action =
        (cyg_uint16) * ((cyg_uint8 *) req->ifr_data +
                 1) | ((cyg_uint16) * ((cyg_uint8 *) req->ifr_data + 2) << 8);

    switch (Action) {
    case HostCmd_ACT_GEN_GET:
        if (copy_to_user(req->ifr_data + SKIP_TYPE_SIZE, para_ie,
                         WMM_PARA_IE_LENGTH)) {
            diag_printf("Copy to user failed\n");
            return -EFAULT;
        }

        HEXDUMP("Para IE Conf GET", (cyg_uint8 *) para_ie, WMM_PARA_IE_LENGTH);

        break;
    case HostCmd_ACT_GEN_SET:
        if (priv->adapter->MediaConnectStatus == WlanMediaStateConnected) {
            return -EPERM;
        }

        HEXDUMP("Para IE Conf SET", (cyg_uint8 *) para_ie, WMM_PARA_IE_LENGTH);

        if (copy_from_user(para_ie, req->ifr_data + SKIP_TYPE_SIZE,
                           WMM_PARA_IE_LENGTH)) {
            diag_printf("Copy from user failed\n");
            return -EFAULT;
        }
        break;
    default:
        diag_printf("Invalid Option\n");
        return -EINVAL;
    }

    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief Read/Set WMM ACK policy
 *
 *  @param priv     Pointer to the wlan_private driver data struct
 *  @param req      Pointer to user data
 *
 *  @return         WLAN_STATUS_SUCCESS if success; otherwise <0
 */
int
wlan_wmm_ack_policy_ioctl(wlan_private * priv, struct ifreq *req)
{
    int ret = WLAN_STATUS_SUCCESS, i, index;
    HostCmd_DS_WMM_ACK_POLICY ackPolicy;

    memset((void*)&ackPolicy, 0x00, sizeof(ackPolicy));

    if (copy_from_user((void *)&ackPolicy,
                       req->ifr_data + SKIP_TYPE,
                       sizeof(HostCmd_DS_WMM_ACK_POLICY))) {
        diag_printf("Copy from user failed\n");
        return -EFAULT;
    }

    HEXDUMP("Ack Policy Conf", (cyg_uint8 *) & ackPolicy,
            sizeof(HostCmd_DS_WMM_ACK_POLICY));

    switch (ackPolicy.Action) {
    case HostCmd_ACT_GET:
        for (i = 0; i < WMM_ACK_POLICY_PRIO; ++i) {
            ackPolicy.AC = i;

            if ((ret = PrepareAndSendCommand(priv,
                                             HostCmd_CMD_WMM_ACK_POLICY,
                                             0, HostCmd_OPTION_WAITFORRSP,
                                             0, (void *)&ackPolicy))) {
                LEAVE();
                diag_printf("PrepareAndSend Failed\n");
                return ret;
            }

            index = SKIP_TYPE_ACTION + (i * 2);
            if (copy_to_user(req->ifr_data + index, (cyg_uint8 *) & ackPolicy.AC, 2)) {
                diag_printf("Copy from user failed\n");
                return -EFAULT;
            }

            HEXDUMP("Ack Policy Conf", (cyg_uint8 *) & ackPolicy + SKIP_ACTION,
                    sizeof(HostCmd_DS_WMM_ACK_POLICY));
        }
        break;

    case HostCmd_ACT_SET:
        ackPolicy.AC = *((cyg_uint8 *) req->ifr_data + SKIP_TYPE_ACTION);
        ackPolicy.AckPolicy = *((cyg_uint8 *) req->ifr_data + SKIP_TYPE_ACTION + 1);

        if ((ret = PrepareAndSendCommand(priv,
                                         HostCmd_CMD_WMM_ACK_POLICY, 0,
                                         HostCmd_OPTION_WAITFORRSP,
                                         0, (void *)&ackPolicy))) {

            LEAVE();
            return ret;
        }

        if (copy_to_user(req->ifr_data + SKIP_TYPE,
                         (void *)&ackPolicy, sizeof(HostCmd_DS_WMM_ACK_POLICY))) {
            diag_printf("Copy from user failed\n");
            return -EFAULT;
        }

        HEXDUMP("Ack Policy Conf", (cyg_uint8 *) & ackPolicy,
                sizeof(HostCmd_DS_WMM_ACK_POLICY));

        break;
    default:
        diag_printf("Invalid Action\n");
        return -EINVAL;
    }
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief Implement cmd HostCmd_DS_WMM_ACK_POLICY
 *
 *  @param priv     Pointer to the wlan_private driver data struct
 *  @param cmd      Pointer to CMD buffer 
 *  @param InfoBuf  Pointer to cmd data
 *
 *  @return         WLAN_STATUS_SUCCESS
 */
int
wlan_cmd_wmm_ack_policy(wlan_private * priv,
                        HostCmd_DS_COMMAND * cmd, void *InfoBuf)
{
    HostCmd_DS_WMM_ACK_POLICY *pAckPolicy;

    pAckPolicy = (HostCmd_DS_WMM_ACK_POLICY *) InfoBuf;

    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_WMM_ACK_POLICY);
    cmd->Size =
        wlan_cpu_to_le16(sizeof(HostCmd_DS_WMM_ACK_POLICY) + S_DS_GEN);

    memcpy((void*)&cmd->params.ackpolicy, (void*)pAckPolicy, sizeof(cmd->params.ackpolicy));

    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief Implement cmd HostCmd_DS_WMM_ACK_POLICY
 *
 *  @param priv    Pointer to the wlan_private driver data struct
 *  @param resp    Pointer to the command response from firmware
 *
 *  @return        WLAN_STATUS_SUCCESS
 */
int
wlan_cmdresp_wmm_ack_policy(wlan_private * priv,
                            const HostCmd_DS_COMMAND * resp)
{
    wlan_adapter *Adapter = priv->adapter;
    HostCmd_DS_WMM_ACK_POLICY *pAckPolicy;

    pAckPolicy = (HostCmd_DS_WMM_ACK_POLICY *) (Adapter->CurCmd->pdata_buf);
    memcpy((void*)pAckPolicy,
           (void*)&resp->params.ackpolicy, sizeof(HostCmd_DS_WMM_ACK_POLICY));

    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief Implement cmd HostCmd_CMD_WMM_GET_STATUS
 *
 *  @param priv     Pointer to the wlan_private driver data struct
 *  @param cmd      Pointer to CMD buffer 
 *  @param InfoBuf  Pointer to cmd data
 *
 *  @return         WLAN_STATUS_SUCCESS
 */
int
wlan_cmd_wmm_get_status(wlan_private * priv,
                        HostCmd_DS_COMMAND * cmd, void *InfoBuf)
{
    diag_printf("WMM: WMM_GET_STATUS cmd sent\n");
    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_WMM_GET_STATUS);
    cmd->Size =
        wlan_cpu_to_le16(sizeof(HostCmd_DS_WMM_GET_STATUS) + S_DS_GEN);

    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief Implement cmd HostCmd_CMD_WMM_PRIO_PKT_AVAIL
 *
 *  @param priv     Pointer to the wlan_private driver data struct
 *  @param cmd      Pointer to CMD buffer 
 *  @param InfoBuf  Pointer to cmd data
 *
 *  @return         WLAN_STATUS_SUCCESS
 */
int
wlan_cmd_wmm_prio_pkt_avail(wlan_private * priv,
                            HostCmd_DS_COMMAND * cmd, void *InfoBuf)
{
#if 0
    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_WMM_PRIO_PKT_AVAIL);
    cmd->Size =
        wlan_cpu_to_le16(sizeof(HostCmd_DS_WMM_PRIO_PKT_AVAIL) + S_DS_GEN);

    cmd->params.priopktavail.PacketAC = priv->adapter->priopktavail.PacketAC;
#endif
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief  Send a command to firmware to retrieve the current WMM status
 *
 *  @param priv     Pointer to the wlan_private driver data struct
 *
 *  @return         WLAN_STATUS_SUCCESS; WLAN_STATUS_FAILURE
 */
int
sendWMMStatusChangeCmd(wlan_private * priv)
{
    return PrepareAndSendCommand(priv, HostCmd_CMD_WMM_GET_STATUS,
                                 0, 0, 0, NULL);
}

/** 
 *  @brief Check if wmm TX queue is empty
 *
 *  @param priv     Pointer to the wlan_private driver data struct
 *
 *  @return         FALSE if not empty; TRUE if empty
 */
int
wmm_lists_empty(wlan_private * wlan_priv)
{
    int i;
    w99702_priv_t * priv = wlan_priv->priv;
    TXBD *txbd = priv->tx_head;
    
    if(txbd->mode)
    	return FALSE;

#if 0
    for (i = 0; i < MAX_AC_QUEUES; i++) {
        if (!list_empty((struct list_head *) &priv->adapter->wmm.TxSkbQ[i])) {
            return FALSE;
        }
    }
#endif
    return TRUE;
}

/** 
 *  @brief Cleanup wmm TX queue
 *
 *  @param priv     Pointer to the wlan_private driver data struct
 *
 *  @return         void
 */
void
wmm_cleanup_queues(wlan_private * wlan_priv)
{
    int i;
    w99702_priv_t * priv = wlan_priv->priv;
    TXBD *txbd = priv->tx_head;
#if 0    
    struct sk_buff *delNode, *Q;

    for (i = 0; i < MAX_AC_QUEUES; i++) {
        Q = &priv->adapter->wmm.TxSkbQ[i];

        while (!list_empty((struct list_head *) Q)) {
            delNode = Q->next;
            list_del((struct list_head *) delNode);
            kfree_skb(delNode);
        }
    }
#else
	while(txbd->mode)
	{
		txbd->mode = 0;
		txbd->SL = 0;
		txbd = (TXBD*)txbd->next;
	}
#endif
}

/** 
 *  @brief Add skb to WMM queue
 *
 *  @param priv     Pointer to the wlan_private driver data struct
 *  @param skb      Pointer to sk_buff
 *
 *  @return         void
 */
void
wmm_map_and_add_skb(wlan_private * priv, struct eth_drv_sg *sg_list,
                               int sg_len,
                               int total_len)
{
#if 0
    wlan_adapter *Adapter = priv->adapter;
    cyg_uint8 tos, ac0, ac;
    struct ethhdr *eth = (struct ethhdr *) sg_list[0].data;
    struct timeval tstamp;
    struct ip *ip;

    switch (eth->h_proto) {
    case __constant_htons(ETH_P_IP):
        /*diag_printf("packet type ETH_P_IP: %04x, tos=%#x prio=%#x\n",
               eth->h_proto, skb->nh.iph->tos, skb->priority);*/
       // tos = IPTOS_PREC(skb->nh.iph->tos) >> IPTOS_OFFSET;
		ip = (struct ip *)sg_list[0].data;
		tos = ip->ip_tos >> IPTOS_OFFSET;
        break;
    case __constant_htons(ETH_P_ARP):
        diag_printf("ARP packet %04x\n", eth->h_proto);
    default:
        tos = 0;
        break;
    }

    ac0 = wmm_tos2ac[tos];
    ac = wmm_ac_downgrade[ac0];

    //skb->priority = wmm_tos2priority[tos];
    /*diag_printf("wmm_map: tos=%#x, ac0=%#x ac=%#x, prio=%#x\n",
           tos, ac0, ac, skb->priority);*/

    /* Access control of the current packet not the Lowest */
    if (ac > AC_PRIO_BE)
        Adapter->wmm.fw_notify = 1;

    list_add_tail((struct list_head *) skb,
                  (struct list_head *) &Adapter->wmm.TxSkbQ[ac]);
#else

	wlan_process_tx(priv, sg_list, sg_len, total_len);
#endif
    /* Record the current time the packet was queued; used to determine
     *   the amount of time the packet was queued in the driver before it 
     *   was sent to the firmware.  The delay is then sent along with the 
     *   packet to the firmware for aggregate delay calculation for stats
     *   and MSDU lifetime expiry.
     */
    //do_gettimeofday(&tstamp);

    //memcpy(&skb->stamp, &tstamp, sizeof(skb->stamp));

}

/**
 *  @brief Process the GET_WMM_STATUS command response from firmware
 *
 *  The GET_WMM_STATUS command returns multiple TLVs for:
 *      - Each AC Queue status
 *      - Current WMM Paramter IE
 *
 *  This function parses the TLVs and then calls further functions
 *   to process any changes in the queue prioritization or state.
 *
 *  @param priv    Pointer to the wlan_private driver data struct
 *  @param resp    Pointer to the command response buffer including TLVs
 *                 TLVs for each queue and the WMM Parameter IE.
 * 
 *  @return WLAN_STATUS_SUCCESS
 */
int
wlan_cmdresp_wmm_get_status(wlan_private * priv,
                            const HostCmd_DS_COMMAND * resp)
{
    wlan_adapter *Adapter = priv->adapter;
    cyg_uint8 *pCurrent = (cyg_uint8 *) & resp->params.getWmmStatus;
    cyg_uint32 respLen = resp->Size;
    int valid = TRUE;

    MrvlIEtypes_Data_t *pTlvHdr;
    MrvlIEtypes_WmmQueueStatus_t *pTlvWmmQStatus;
    WMM_PARAMETER_IE *pWmmParamIe;
    WMM_AC_STATUS *pACStatus;

    diag_printf("WMM: WMM_GET_STATUS cmdresp received: %d\n", respLen);
    HEXDUMP("CMD_RESP: WMM_GET_STATUS", pCurrent, respLen);

    while ((respLen >= sizeof(pTlvHdr->Header)) && valid) {
        pTlvHdr = (MrvlIEtypes_Data_t *) pCurrent;

        switch (pTlvHdr->Header.Type) {
        case TLV_TYPE_WMMQSTATUS:
            pTlvWmmQStatus = (MrvlIEtypes_WmmQueueStatus_t *) pTlvHdr;
            diag_printf(
                   "CMD_RESP: WMM_GET_STATUS: QSTATUS TLV: %d, %d, %d\n",
                   pTlvWmmQStatus->QueueIndex, pTlvWmmQStatus->FlowRequired,
                   pTlvWmmQStatus->Disabled);

            pACStatus = &Adapter->wmm.acStatus[pTlvWmmQStatus->QueueIndex];
            pACStatus->Disabled = pTlvWmmQStatus->Disabled;
            pACStatus->FlowRequired = pTlvWmmQStatus->FlowRequired;
            pACStatus->FlowCreated = pTlvWmmQStatus->FlowCreated;
            break;

        case WMM_IE:
            /*
             * Point the regular IEEE IE 2 bytes into the Marvell IE
             *   and setup the IEEE IE type and length byte fields
             */

            HEXDUMP("WMM: WMM TLV:", (cyg_uint8 *) pTlvHdr, pTlvHdr->Header.Len + 4);

            pWmmParamIe = (WMM_PARAMETER_IE *) (pCurrent + 2);
            pWmmParamIe->Length = pTlvHdr->Header.Len;
            pWmmParamIe->ElementId = WMM_IE;

            diag_printf("CMD_RESP: WMM_GET_STATUS: WMM Parameter Set: %d\n",
                   pWmmParamIe->QoSInfo.ParaSetCount);

            memcpy((cyg_uint8 *) & Adapter->CurBssParams.BSSDescriptor.wmmIE,
                   (void *)pWmmParamIe, pWmmParamIe->Length + 2);

            break;

        default:
            valid = FALSE;
            break;
        }

        pCurrent += (pTlvHdr->Header.Len + sizeof(pTlvHdr->Header));
        respLen -= (pTlvHdr->Header.Len + sizeof(pTlvHdr->Header));
    }

    wmm_setup_queue_priorities(priv);
    wmm_setup_ac_downgrade(priv);

#if WIRELESS_EXT > 14
    send_iwevcustom_event(priv, (cyg_int8 *)WMM_CONFIG_CHANGE_INDICATION);
#endif
   // os_carrier_on(priv);
   // os_start_queue(priv);

    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief Call back from the command module to allow insertion of a WMM TLV
 *   
 *  If the BSS we are associating to supports WMM, add the required WMM
 *    Information IE to the association request command buffer in the form
 *    of a Marvell extended IEEE IE.
 *     
 *  @param priv        Pointer to the wlan_private driver data struct
 *  @param ppAssocBuf  Output parameter: Pointer to the TLV output buffer,
 *                     modified on return to point after the appended WMM TLV
 *  @param pWmmIE      Pointer to the WMM IE for the BSS we are joining
 * 
 *  @return Length of data appended to the association tlv buffer
 */
cyg_uint32
wlan_wmm_process_association_req(wlan_private * priv,
                                 cyg_uint8 ** ppAssocBuf, WMM_PARAMETER_IE * pWmmIE)
{
    wlan_adapter *Adapter = priv->adapter;
    MrvlIEtypes_WmmParamSet_t *wmm;
    cyg_uint32 retLen = 0;

    /* Null checks */
    if (ppAssocBuf == 0)
        return 0;
    if (*ppAssocBuf == 0)
        return 0;
    if (pWmmIE == 0)
        return 0;

    diag_printf("WMM: process assoc req: bss->wmmIe=%x\n",
           pWmmIE->ElementId);

    if (Adapter->wmm.required && pWmmIE->ElementId == WMM_IE) {
        Adapter->CurrentPacketFilter |= HostCmd_ACT_MAC_WMM_ENABLE;
        SetMacPacketFilter(priv);

        wmm = (MrvlIEtypes_WmmParamSet_t *) * ppAssocBuf;
        wmm->Header.Type = (cyg_uint16) wmm_ie[0];
        wmm->Header.Type = wlan_cpu_to_le16(wmm->Header.Type);
        wmm->Header.Len = (cyg_uint16) wmm_ie[1];
        wmm->Header.Len = wlan_cpu_to_le16(wmm->Header.Len);

        memcpy((void*)wmm->WmmIE, (void*)&wmm_ie[2], wmm->Header.Len);
#define QOS_INFO_PARA_MASK 0x0f
        if (pWmmIE->QoSInfo.QosUAPSD
            && ((Adapter->wmm.qosinfo & QOS_INFO_PARA_MASK) != 0)) {
            memcpy((cyg_uint8 *) (wmm->WmmIE + wmm->Header.Len
                           - sizeof(Adapter->wmm.qosinfo)),
                   (void*)&Adapter->wmm.qosinfo, sizeof(Adapter->wmm.qosinfo));
        }
        retLen = sizeof(wmm->Header) + wmm->Header.Len;

        HEXDUMP("ASSOC_CMD: WMM IE", (cyg_uint8 *) wmm, retLen);
        *ppAssocBuf += retLen;

    } else {
        Adapter->CurrentPacketFilter &= ~HostCmd_ACT_MAC_WMM_ENABLE;
        SetMacPacketFilter(priv);
    }

    return retLen;
}

#if 0
/** 
 *  @brief handle TX data
 *
 *  @param priv    Pointer to the wlan_private driver data struct
 *
 *  @return        void
 */
void
wmm_process_tx(wlan_private * priv)
{
    wlan_adapter *Adapter = priv->adapter;
    ulong flags;

    OS_INTERRUPT_SAVE_AREA;

    ENTER();

    if ((Adapter->PSState == PS_STATE_SLEEP)
        || (Adapter->PSState == PS_STATE_PRE_SLEEP)
        ) {
        diag_printf("In PS State %d"
               " - Not sending the packet\n", Adapter->PSState);
        goto done;
    }

    //spin_lock_irqsave(&Adapter->CurrentTxLock, flags);
		OS_INT_DISABLE(flags);
		
    if (priv->wlan_dev.dnld_sent) {

        if (priv->adapter->wmm.fw_notify) {
            wmm_send_prio_pkt_avail(priv);
            priv->adapter->wmm.fw_notify = 0;
        }
				
				OS_INT_RESTORE(flags);
        //spin_unlock_irqrestore(&Adapter->CurrentTxLock, flags);

        goto done;
    }

    wmm_pop_highest_prio_skb(priv);
    
    OS_INT_RESTORE(flags);
    //spin_unlock_irqrestore(&Adapter->CurrentTxLock, flags);

    if (Adapter->CurrentTxSkb) {
        wlan_process_tx(priv);
    }

  done:
    LEAVE();
}
#endif
/**
 *  @brief Private IOCTL entry to get the status of the WMM queues
 *
 *  Return the following information for each WMM AC:
 *        - WMM IE ACM Required
 *        - Firmware Flow Required 
 *        - Firmware Flow Established
 *        - Firmware Queue Enabled
 *
 *  @param priv    Pointer to the wlan_private driver data struct
 *  @param wrq     A pointer to iwreq structure containing the
 *                 wlan_ioctl_wmm_queue_status_t struct for request
 *
 *  @return        0 if successful; IOCTL error code otherwise
 */
int
wlan_wmm_queue_status_ioctl(wlan_private * priv, struct iwreq *wrq)
{
    wlan_adapter *Adapter = priv->adapter;
    wlan_ioctl_wmm_queue_status_t qstatus;
    wlan_wmm_ac_e acVal;
    WMM_AC_STATUS *pACStatus;
    WMM_AC_PARAS *pWmmIeAC;

    for (acVal = AC_PRIO_BK; acVal <= AC_PRIO_VO; acVal++) {
        pACStatus = &Adapter->wmm.acStatus[acVal];

        /* Get a pointer to the WMM IE parameters for this AC */
        switch (acVal) {
        default:
        case AC_PRIO_BK:
            pWmmIeAC = &Adapter->CurBssParams.BSSDescriptor.wmmIE.AC_Paras_BK;
            break;
        case AC_PRIO_BE:
            pWmmIeAC = &Adapter->CurBssParams.BSSDescriptor.wmmIE.AC_Paras_BE;
            break;
        case AC_PRIO_VI:
            pWmmIeAC = &Adapter->CurBssParams.BSSDescriptor.wmmIE.AC_Paras_VI;
            break;
        case AC_PRIO_VO:
            pWmmIeAC = &Adapter->CurBssParams.BSSDescriptor.wmmIE.AC_Paras_VO;
            break;
        }

        /* ACM bit */
        qstatus.acStatus[acVal].wmmACM = pWmmIeAC->ACI_AIFSN.ACM;

        /* Firmware status */
        qstatus.acStatus[acVal].flowRequired = pACStatus->FlowRequired;
        qstatus.acStatus[acVal].flowCreated = pACStatus->FlowCreated;
        qstatus.acStatus[acVal].disabled = pACStatus->Disabled;
    }

    if (copy_to_user(wrq->u.data.pointer, (void *)&qstatus, sizeof(qstatus))) {
        diag_printf("Copy to user failed\n");
        return -EFAULT;
    }

    return WLAN_STATUS_SUCCESS;
}

/**
 *  @brief Private IOCTL entry to send an ADDTS TSPEC
 *
 *  Receive a ADDTS command from the application.  The command structure
 *    contains a TSPEC and timeout in milliseconds.  The timeout is performed
 *    in the firmware after the ADDTS command frame is sent.  
 *
 *  The TSPEC is received in the API as an opaque block whose length is 
 *    calculated from the IOCTL data length.  The firmware will send the 
 *    entire data block, including the bytes after the TSPEC.  This is done
 *    to allow extra IEs to be packaged with the TSPEC in the ADDTS action
 *    frame.
 *
 *  The IOCTL structure contains two return fields: 
 *    - The firmware command result which indicates failure and timeouts
 *    - The IEEE Status code which contains the corresponding value from
 *      any ADDTS response frame received.
 *
 *  In addition, the opaque TSPEC data block passed in is replaced with the 
 *    TSPEC recieved in the ADDTS response frame.  In case of failure, the
 *    AP may modify the TSPEC on return and in the case of success, the 
 *    medium time is returned as calculated by the AP.  Along with the TSPEC,
 *    any IEs that are sent in the ADDTS response are also returned and can be
 *    parsed using the IOCTL length as an indicator of extra elements.
 *
 *  The return value to the application layer indicates a driver execution
 *    success or failure.  A successful return could still indicate a firmware
 *    failure or AP negotiation failure via the commandResult field copied
 *    back to the application.
 *
 *  @param priv    Pointer to the wlan_private driver data struct
 *  @param wrq     A pointer to iwreq structure containing the
 *                 wlan_ioctl_wmm_addts_req_t struct for this ADDTS request
 *
 *  @return        0 if successful; IOCTL error code otherwise
 */
int
wlan_wmm_addts_req_ioctl(wlan_private * priv, struct iwreq *wrq)
{
    static int dialogTok = 0;
    wlan_ioctl_wmm_addts_req_t addtsIoctl;
    wlan_cmd_wmm_addts_req_t addtsCmd;
    int retcode;

    if (copy_from_user((void *)&addtsIoctl,
                       wrq->u.data.pointer,
                       MIN(wrq->u.data.length, sizeof(addtsIoctl))) != 0) {
        /* copy_from_user failed  */
        diag_printf("TSPEC: ADDTS copy from user failed\n");
        retcode = -EFAULT;

    } else {
        memset(&addtsCmd, 0x00, sizeof(addtsCmd));
        addtsCmd.dialogToken = ++dialogTok;
        addtsCmd.timeout_ms = addtsIoctl.timeout_ms;
        addtsCmd.tspecDataLen = (wrq->u.data.length
                                 - sizeof(addtsCmd.timeout_ms)
                                 - sizeof(addtsCmd.commandResult)
                                 - sizeof(addtsCmd.ieeeStatusCode));
        memcpy(addtsCmd.tspecData,
               (void *)addtsIoctl.tspecData, addtsCmd.tspecDataLen);

        retcode = PrepareAndSendCommand(priv,
                                        HostCmd_CMD_WMM_ADDTS_REQ, 0,
                                        HostCmd_OPTION_WAITFORRSP, 0,
                                        &addtsCmd);

        wrq->u.data.length = (sizeof(addtsIoctl.timeout_ms)
                              + sizeof(addtsIoctl.commandResult)
                              + sizeof(addtsIoctl.ieeeStatusCode)
                              + addtsCmd.tspecDataLen);

        addtsIoctl.commandResult = addtsCmd.commandResult;
        addtsIoctl.ieeeStatusCode = addtsCmd.ieeeStatusCode;
        memcpy((void *)addtsIoctl.tspecData,
               addtsCmd.tspecData, addtsCmd.tspecDataLen);

        if (copy_to_user(wrq->u.data.pointer,
                         (void *)&addtsIoctl, sizeof(addtsIoctl))) {
            diag_printf("Copy to user failed\n");
            return -EFAULT;
        }

        if (retcode) {
            return -EFAULT;
        }
    }

    return retcode;
}

/**
 *  @brief Private IOCTL entry to send a DELTS TSPEC
 *
 *  Receive a DELTS command from the application.  The command structure
 *    contains a TSPEC and reason code along with space for a command result
 *    to be returned.  The information is packaged is sent to the wlan_cmd.c
 *    firmware command prep and send routines for execution in the firmware.
 *
 *  The reason code is not used for WMM implementations but is indicated in
 *    the 802.11e specification.
 *
 *  The return value to the application layer indicates a driver execution
 *    success or failure.  A successful return could still indicate a firmware
 *    failure via the commandResult field copied back to the application.
 *  
 *  @param priv    Pointer to the wlan_private driver data struct
 *  @param wrq     A pointer to iwreq structure containing the
 *                 wlan_ioctl_wmm_delts_req_t struct for this DELTS request
 *
 *  @return        0 if successful; IOCTL error code otherwise
 */
int
wlan_wmm_delts_req_ioctl(wlan_private * priv, struct iwreq *wrq)
{
    wlan_ioctl_wmm_delts_req_t deltsIoctl;
    wlan_cmd_wmm_delts_req_t deltsCmd;
    int retcode;

    if (copy_from_user((void *)&deltsIoctl,
                       wrq->u.data.pointer,
                       MIN(wrq->u.data.length, sizeof(deltsIoctl))) != 0) {
        /* copy_from_user failed  */
        diag_printf("TSPEC: DELTS copy from user failed\n");
        retcode = -EFAULT;

    } else {
        memset(&deltsCmd, 0x00, sizeof(deltsCmd));

        /* Dialog token unused for WMM implementations */
        deltsCmd.dialogToken = 0;

        deltsCmd.ieeeReasonCode = deltsIoctl.ieeeReasonCode;

        /* Calculate the length of the TSPEC and any other IEs */
        deltsCmd.tspecDataLen = (wrq->u.data.length
                                 - sizeof(deltsCmd.commandResult)
                                 - sizeof(deltsCmd.ieeeReasonCode));
        memcpy(deltsCmd.tspecData,
               (void *)deltsIoctl.tspecData, deltsCmd.tspecDataLen);

        /* Send the DELTS request to firmware, wait for a response */
        retcode = PrepareAndSendCommand(priv,
                                        HostCmd_CMD_WMM_DELTS_REQ, 0,
                                        HostCmd_OPTION_WAITFORRSP, 0,
                                        &deltsCmd);

        /* Return the firmware command result back to the application layer */
        deltsIoctl.commandResult = deltsCmd.commandResult;

        if (copy_to_user(wrq->u.data.pointer,
                         &deltsCmd,
                         MIN(wrq->u.data.length, sizeof(deltsIoctl)))) {
            diag_printf("Copy to user failed\n");
            return -EFAULT;
        }

        if (retcode) {
            retcode = -EFAULT;
        }
    }

    return retcode;
}

/**
 *  @brief Process the ADDTS_REQ command response from firmware
 *
 *  Return the ADDTS firmware response to the calling thread that sent 
 *    the command.  The result is then relayed back the app layer.
 *
 *  @param priv    Pointer to the wlan_private driver data struct
 *  @param resp    Pointer to the command response buffer including the 
 *                 command result and any returned ADDTS response TSPEC
 *                 elements 
 * 
 *  @return WLAN_STATUS_SUCCESS
 *
 *  @sa wlan_wmm_addts_req_ioctl
 */
int
wlan_cmdresp_wmm_addts_req(wlan_private * priv,
                           const HostCmd_DS_COMMAND * resp)
{
    wlan_cmd_wmm_addts_req_t *pAddTsCmd;
    const HostCmd_DS_WMM_ADDTS_REQ *pCmdResp;

    /* Cast the NULL pointer of the buffer the IOCTL sent in the command req */
    pAddTsCmd = (wlan_cmd_wmm_addts_req_t *) priv->adapter->CurCmd->pdata_buf;

    /* Convenience variable for the ADDTS response from the firmware */
    pCmdResp = &resp->params.addTsReq;

    /* Assign return data */
    pAddTsCmd->dialogToken = pCmdResp->dialogToken;
    pAddTsCmd->commandResult = pCmdResp->commandResult;
    pAddTsCmd->ieeeStatusCode = pCmdResp->ieeeStatusCode;

    /* The tspecData field is potentially variable in size due to extra IEs
     *   that may have been in the ADDTS response action frame.  Calculate
     *   the data length from the firmware command response.
     */
    pAddTsCmd->tspecDataLen = (resp->Size - sizeof(pCmdResp->commandResult)
                               - sizeof(pCmdResp->timeout_ms)
                               - sizeof(pCmdResp->dialogToken)
                               - sizeof(pCmdResp->ieeeStatusCode)
                               - S_DS_GEN);

    /* Copy back the TSPEC data including any extra IEs after the TSPEC */
    memcpy(pAddTsCmd->tspecData,
           (void *)pCmdResp->tspecData, pAddTsCmd->tspecDataLen);

    diag_printf("TSPEC: ADDTS ret = %d,%d sz=%d\n",
           pAddTsCmd->commandResult, pAddTsCmd->ieeeStatusCode,
           pAddTsCmd->tspecDataLen);

    HEXDUMP("TSPEC: ADDTS data",
            pAddTsCmd->tspecData, pAddTsCmd->tspecDataLen);

    return WLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process the DELTS_REQ command response from firmware
 *
 *  Return the DELTS firmware response to the calling thread that sent 
 *    the command.  The result is then relayed back the app layer.
 *
 *  @param priv    Pointer to the wlan_private driver data struct
 *  @param resp    Pointer to the command response buffer with the command
 *                 result.  No other response information is passed back 
 *                 to the driver.
 * 
 *  @return WLAN_STATUS_SUCCESS
 *
 *  @sa wlan_wmm_delts_req_ioctl
 */
int
wlan_cmdresp_wmm_delts_req(wlan_private * priv,
                           const HostCmd_DS_COMMAND * resp)
{
    wlan_cmd_wmm_delts_req_t *pDelTsCmd;

    /* Cast the NULL pointer of the buffer the IOCTL sent in the command req */
    pDelTsCmd = (wlan_cmd_wmm_delts_req_t *) priv->adapter->CurCmd->pdata_buf;

    pDelTsCmd->commandResult = resp->params.delTsReq.commandResult;

    diag_printf("TSPEC: DELTS result = %d\n", pDelTsCmd->commandResult);

    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief Implement cmd HostCmd_DS_WMM_ADDTS_REQ
 *
 *  @param priv    Pointer to the wlan_private driver data struct
 *  @param cmd     Pointer to CMD buffer 
 *  @param InfoBuf Pointer to cmd data
 *
 *  @return        WLAN_STATUS_SUCCESS
 *
 *  @sa wlan_wmm_addts_req_ioctl
 */
int
wlan_cmd_wmm_addts_req(wlan_private * priv,
                       HostCmd_DS_COMMAND * cmd, void *InfoBuf)
{
    wlan_cmd_wmm_addts_req_t *pAddTsCmd;
    int tspecCopySize;

    pAddTsCmd = (wlan_cmd_wmm_addts_req_t *) InfoBuf;

    cmd->params.addTsReq.timeout_ms = pAddTsCmd->timeout_ms;
    cmd->params.addTsReq.dialogToken = pAddTsCmd->dialogToken;

    tspecCopySize = MIN(pAddTsCmd->tspecDataLen,
                        sizeof(cmd->params.addTsReq.tspecData));
    memcpy((void *)&cmd->params.addTsReq.tspecData,
           pAddTsCmd->tspecData, tspecCopySize);

    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_WMM_ADDTS_REQ);
    cmd->Size = wlan_cpu_to_le16(sizeof(cmd->params.addTsReq.dialogToken)
                                 + sizeof(cmd->params.addTsReq.timeout_ms)
                                 + sizeof(cmd->params.addTsReq.commandResult)
                                 + sizeof(cmd->params.addTsReq.ieeeStatusCode)
                                 + tspecCopySize + S_DS_GEN);

    diag_printf("WMM: ADDTS Cmd: Data Len = %d\n", pAddTsCmd->tspecDataLen);

    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief Implement cmd HostCmd_DS_WMM_DELTS_REQ
 *
 *  @param priv     Pointer to the wlan_private driver data struct
 *  @param cmd      Pointer to CMD buffer 
 *  @param InfoBuf  Void pointer cast of a wlan_cmd_wmm_delts_req_t struct
 *
 *  @return         WLAN_STATUS_SUCCESS
 *
 *  @sa wlan_wmm_delts_req_ioctl
 */
int
wlan_cmd_wmm_delts_req(wlan_private * priv,
                       HostCmd_DS_COMMAND * cmd, void *InfoBuf)
{
    wlan_cmd_wmm_delts_req_t *pDelTsCmd;
    int tspecCopySize;

    pDelTsCmd = (wlan_cmd_wmm_delts_req_t *) InfoBuf;

    cmd->params.delTsReq.dialogToken = pDelTsCmd->dialogToken;
    cmd->params.delTsReq.ieeeReasonCode = pDelTsCmd->ieeeReasonCode;

    tspecCopySize = MIN(pDelTsCmd->tspecDataLen,
                        sizeof(cmd->params.delTsReq.tspecData));
    memcpy((void *)&cmd->params.delTsReq.tspecData,
           pDelTsCmd->tspecData, tspecCopySize);

    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_WMM_DELTS_REQ);
    cmd->Size = wlan_cpu_to_le16(sizeof(cmd->params.delTsReq.dialogToken)
                                 + sizeof(cmd->params.delTsReq.commandResult)
                                 + sizeof(cmd->params.delTsReq.ieeeReasonCode)
                                 + tspecCopySize + S_DS_GEN);

    diag_printf("WMM: DELTS Cmd prepared\n");

    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief Prepare the firmware command buffer for the WMM_QUEUE_CONFIG command
 *
 *  @param priv     Pointer to the wlan_private driver data struct
 *  @param cmd      Pointer to CMD buffer 
 *  @param InfoBuf  Void pointer cast of a wlan_cmd_wmm_queue_config_t struct
 *
 *  @return         WLAN_STATUS_SUCCESS
 */
int
wlan_cmd_wmm_queue_config(wlan_private * priv,
                          HostCmd_DS_COMMAND * cmd, void *InfoBuf)
{
    wlan_cmd_wmm_queue_config_t *pQConfigCmd;
    int tlvCopySize;

    pQConfigCmd = (wlan_cmd_wmm_queue_config_t *) InfoBuf;

    cmd->params.queueConfig.action = pQConfigCmd->action;
    cmd->params.queueConfig.accessCategory = pQConfigCmd->accessCategory;
    cmd->params.queueConfig.msduLifetimeExpiry
        = pQConfigCmd->msduLifetimeExpiry;

    tlvCopySize = MIN(pQConfigCmd->tlvBufLen,
                      sizeof(cmd->params.queueConfig.tlvBuffer));
    memcpy((void *)&cmd->params.queueConfig.tlvBuffer,
           pQConfigCmd->tlvBuffer, tlvCopySize);

    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_WMM_QUEUE_CONFIG);
    cmd->Size = wlan_cpu_to_le16(sizeof(cmd->params.queueConfig.action)
                                 +
                                 sizeof(cmd->params.queueConfig.
                                        accessCategory)
                                 +
                                 sizeof(cmd->params.queueConfig.
                                        msduLifetimeExpiry)
                                 + tlvCopySize + S_DS_GEN);

    diag_printf("WMM: QUEUE CONFIG Cmd prepared\n");

    return WLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process the WMM_QUEUE_CONFIG command response from firmware
 *
 *  Return the firmware command response to the blocked IOCTL caller function.
 *
 *  @param priv    Pointer to the wlan_private driver data struct
 *  @param resp    Pointer to the command response buffer with:
 *                      - action code
 *                      - access category
 *                      - collected statistics if requested 
 *
 *  @return WLAN_STATUS_SUCCESS
 *
 *  @sa wlan_wmm_queue_config_ioctl
 */
int
wlan_cmdresp_wmm_queue_config(wlan_private * priv,
                              const HostCmd_DS_COMMAND * resp)
{
    wlan_cmd_wmm_queue_config_t *pQConfigCmd;
    const HostCmd_DS_WMM_QUEUE_CONFIG *pCmdResp;

    pQConfigCmd =
        (wlan_cmd_wmm_queue_config_t *) (priv->adapter->CurCmd->pdata_buf);
    pCmdResp = &resp->params.queueConfig;

    pQConfigCmd->action = pCmdResp->action;
    pQConfigCmd->accessCategory = pCmdResp->accessCategory;
    pQConfigCmd->msduLifetimeExpiry = pCmdResp->msduLifetimeExpiry;

    pQConfigCmd->tlvBufLen = (resp->Size - sizeof(pCmdResp->action)
                              - sizeof(pCmdResp->accessCategory)
                              - sizeof(pCmdResp->msduLifetimeExpiry)
                              - S_DS_GEN);

    memcpy(pQConfigCmd->tlvBuffer,
           (void *)pCmdResp->tlvBuffer, pQConfigCmd->tlvBufLen);

    return WLAN_STATUS_SUCCESS;
}

/**
 *  @brief Private IOCTL entry to get/set a specified AC Queue's parameters
 *
 *  Receive a AC Queue configuration command which is used to get, set, or 
 *    default the parameters associated with a specific WMM AC Queue.
 *
 *  @param priv    Pointer to the wlan_private driver data struct
 *  @param wrq     A pointer to iwreq structure containing the
 *                 wlan_ioctl_wmm_queue_config_t struct
 *
 *  @return        0 if successful; IOCTL error code otherwise
 */
int
wlan_wmm_queue_config_ioctl(wlan_private * priv, struct iwreq *wrq)
{
    wlan_ioctl_wmm_queue_config_t queueConfigIoctl;
    wlan_cmd_wmm_queue_config_t queueConfigCmd;
    int retcode;

    diag_printf("WMM: Queue Config IOCTL Enter\n");

    if (copy_from_user((void *)&queueConfigIoctl,
                       wrq->u.data.pointer,
                       MIN(wrq->u.data.length,
                           sizeof(queueConfigIoctl))) != 0) {
        /* copy_from_user failed  */
        diag_printf("WMM: Queue Config: copy from user failed\n");
        retcode = -EFAULT;

    } else {
        memset(&queueConfigCmd, 0x00, sizeof(queueConfigCmd));

        queueConfigCmd.action = queueConfigIoctl.action;
        queueConfigCmd.accessCategory = queueConfigIoctl.accessCategory;
        queueConfigCmd.msduLifetimeExpiry
            = queueConfigIoctl.msduLifetimeExpiry;

        /* Create a rates TLV from the supportedRates[] ioctl field */
        queueConfigCmd.tlvBufLen = 0;

        retcode = PrepareAndSendCommand(priv,
                                        HostCmd_CMD_WMM_QUEUE_CONFIG, 0,
                                        HostCmd_OPTION_WAITFORRSP, 0,
                                        &queueConfigCmd);

        memset((void *)&queueConfigIoctl, 0x00, sizeof(queueConfigIoctl));

        queueConfigIoctl.action = queueConfigCmd.action;
        queueConfigIoctl.accessCategory = queueConfigCmd.accessCategory;
        queueConfigIoctl.msduLifetimeExpiry
            = queueConfigCmd.msduLifetimeExpiry;

        wrq->u.data.length = sizeof(queueConfigIoctl);

        /* Convert the rates TLV in the response (if any) to a rates[] ioctl */

        if (copy_to_user(wrq->u.data.pointer,
                         (void *)&queueConfigIoctl, sizeof(queueConfigIoctl))) {
            diag_printf("Copy to user failed\n");
            return -EFAULT;
        }

        if (retcode) {
            return -EFAULT;
        }
    }

    return retcode;
}

/** 
 *  @brief Prepare the firmware command buffer for the WMM_QUEUE_STATS command
 *
 *  @param priv     Pointer to the wlan_private driver data struct
 *  @param cmd      pointer to CMD buffer 
 *  @param InfoBuf  void pointer cast of a HostCmd_CMD_WMM_QUEUE_STATS struct
 *
 *  @return         WLAN_STATUS_SUCCESS
 */
int
wlan_cmd_wmm_queue_stats(wlan_private * priv,
                         HostCmd_DS_COMMAND * cmd, void *InfoBuf)
{
    memcpy((void*)&cmd->params.queueStats, InfoBuf, sizeof(cmd->params.queueStats));

    cmd->Command = wlan_cpu_to_le16(HostCmd_CMD_WMM_QUEUE_STATS);
    cmd->Size = wlan_cpu_to_le16(sizeof(HostCmd_DS_WMM_QUEUE_STATS)
                                 + S_DS_GEN);

    diag_printf("WMM: QUEUE STATS Cmd prepared\n");

    return WLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process the WMM_QUEUE_STATS command response from firmware
 *
 *  Return the firmware command response to the blocked IOCTL caller function.
 *
 *  @param priv    Pointer to the wlan_private driver data struct
 *  @param resp    Pointer to the command response buffer with:
 *                      - action code
 *                      - access category
 *                      - collected statistics if requested 
 *
 *  @return WLAN_STATUS_SUCCESS
 *
 *  @sa wlan_wmm_queue_stats_ioctl
 */
int
wlan_cmdresp_wmm_queue_stats(wlan_private * priv,
                             const HostCmd_DS_COMMAND * resp)
{
    memcpy(priv->adapter->CurCmd->pdata_buf,
           (void*)&resp->params.queueStats, (resp->Size - S_DS_GEN));

    diag_printf("WMM: Queue Stats response: %d\n", resp->Size - S_DS_GEN);

    return WLAN_STATUS_SUCCESS;
}

/**
 *  @brief Private IOCTL entry to get and start/stop queue stats on a WMM AC
 *
 *  Receive a AC Queue statistics command from the application for a specific
 *    WMM AC.  The command can:
 *         - Turn stats on
 *         - Turn stats off
 *         - Collect and clear the stats 
 *
 *  @param priv    Pointer to the wlan_private driver data struct
 *  @param wrq     A pointer to iwreq structure containing the
 *                 wlan_ioctl_wmm_queue_stats_t struct
 *
 *  @return        0 if successful; IOCTL error code otherwise
 */
int
wlan_wmm_queue_stats_ioctl(wlan_private * priv, struct iwreq *wrq)
{
    wlan_ioctl_wmm_queue_stats_t queueStatsIoctl;
    HostCmd_DS_WMM_QUEUE_STATS queueStatsCmd;
    int retcode;

    if (copy_from_user((void *)&queueStatsIoctl,
                       wrq->u.data.pointer,
                       MIN(wrq->u.data.length,
                           sizeof(queueStatsIoctl))) != 0) {
        /* copy_from_user failed  */
        diag_printf("WMM: Queue Stats: copy from user failed\n");
        retcode = -EFAULT;

    } else {
        memcpy((void*)&queueStatsCmd, (void*)&queueStatsIoctl, sizeof(queueStatsCmd));

        diag_printf("WMM: QUEUE STATS Ioctl: %d, %d\n",
               queueStatsCmd.action, queueStatsCmd.accessCategory);

        retcode = PrepareAndSendCommand(priv,
                                        HostCmd_CMD_WMM_QUEUE_STATS, 0,
                                        HostCmd_OPTION_WAITFORRSP, 0,
                                        (void*)&queueStatsCmd);
        if (copy_to_user(wrq->u.data.pointer,
                         (void *)&queueStatsCmd,
                         MIN(wrq->u.data.length, sizeof(queueStatsCmd)))) {
            diag_printf("Copy to user failed\n");
            return -EFAULT;
        }

        if (retcode) {
            retcode = -EFAULT;
        }
    }

    if (retcode != WLAN_STATUS_SUCCESS) {
        diag_printf("WMM: QUEUE STATS Ioctl FAILED: %d, %d\n",
               queueStatsIoctl.action, queueStatsIoctl.accessCategory);
    }

    return retcode;
}

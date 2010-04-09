/** @file wlan_tx.c
  * @brief This file contains the handling of TX in wlan
  * driver.
  * 
  *  Copyright ?Marvell International Ltd. and/or its affiliates, 2003-2006
  */
/********************************************************
Change log:
	09/28/05: Add Doxygen format comments
	12/13/05: Add Proprietary periodic sleep support
	01/05/06: Add kernel 2.6.x support	
	04/06/06: Add TSPEC, queue metrics, and MSDU expiry support
********************************************************/

#include	"include.h"

/********************************************************
		Local Variables
********************************************************/

/********************************************************
		Global Variables
********************************************************/

/********************************************************
		Local Functions
********************************************************/
static int
timeval_diff_in_ms(const struct timeval *pTv1, const struct timeval *pTv2)
{
    int diff_ms;

    diff_ms = (pTv1->tv_sec - pTv2->tv_sec) * 1000;
    diff_ms += (pTv1->tv_usec - pTv2->tv_usec) / 1000;

    return diff_ms;
}


int 
gettimeofday(struct timeval *tv,
             struct timezone *tz)
{
    tv->tv_usec = 0;
    tv->tv_sec = time(NULL);
    return(0);
}


/** 
 *  @brief This function processes a single packet and sends
 *  to IF layer
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @param skb     A pointer to skb which includes TX packet
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
SendSinglePacket(wlan_private * wlan_priv, struct eth_drv_sg *sg_list,
                               int sg_len,
                               int total_len)
{
    wlan_adapter *Adapter = wlan_priv->adapter;
    w99702_priv_t *priv = wlan_priv->priv;
    int ret = WLAN_STATUS_SUCCESS;
    //TxPD LocalTxPD;
    TxPD *pLocalTxPD;// = &LocalTxPD;
    TXBD *txbd =( TXBD *)priv->tx_tail;
    cyg_uint8 *ptr = (unsigned char *)(txbd->buffer);//priv->adapter->TmpTxBuf;
    struct timeval in_tv;
    struct timeval out_tv;
    int queue_delay;
    int i, j;
 

    ENTER();

    if (!total_len || (total_len > MRVDRV_ETH_TX_PACKET_BUFFER_SIZE)) {
        diag_printf("Tx Error: Bad skb length %d : %d\n",
               total_len, MRVDRV_ETH_TX_PACKET_BUFFER_SIZE);
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }
	
	ptr += SDIO_HEADER_LEN;
	
	pLocalTxPD = (TxPD *)ptr;
    memset((void*)pLocalTxPD, 0, sizeof(TxPD));

    pLocalTxPD->TxPacketLength = total_len;

    if (Adapter->wmm.enabled) {
        /* 
         * original skb->priority has been overwritten 
         * by wmm_map_and_add_skb()
         */
        //pLocalTxPD->Priority = (cyg_uint8) skb->priority;
		//do_gettimeofday(&in_tv);
		gettimeofday(&in_tv, NULL);
        //memcpy(&in_tv, &skb->stamp, sizeof(in_tv));

        //do_gettimeofday(&out_tv);
        gettimeofday(&out_tv, NULL);

        /* Queue delay is passed as a uint8 in units of 2ms (ms shifted by 1).
         *   Min value (other than 0) is therefore 2ms, max is 510ms.
         */
        queue_delay = timeval_diff_in_ms(&out_tv, &in_tv) >> 1;

        /* Pass max value if queue_delay is beyond the uint8 range */
        pLocalTxPD->PktDelay_2ms = MIN(queue_delay, 0xFF);

        diag_printf("WMM: Pkt Delay: %d ms\n",
               pLocalTxPD->PktDelay_2ms << 1);
    }
    if (Adapter->PSState != PS_STATE_FULL_POWER) {
        if (TRUE == CheckLastPacketIndication(wlan_priv)) {
            Adapter->TxLockFlag = TRUE;
            pLocalTxPD->PowerMgmt = MRVDRV_TxPD_POWER_MGMT_LAST_PACKET;
        }
    }
    /* offset of actual data */
    pLocalTxPD->TxPacketLocation = sizeof(TxPD);

    /* TxCtrl set by user or default */
    pLocalTxPD->TxControl = Adapter->PktTxCtrl;

    memcpy((void*)pLocalTxPD->TxDestAddrHigh, (void*)sg_list[0].buf/*skb->data*/, MRVDRV_ETH_ADDR_LEN);

    HEXDUMP("TxPD", (cyg_uint8 *) pLocalTxPD, sizeof(TxPD));

    
    //memcpy(ptr, (void*)pLocalTxPD, sizeof(TxPD));

    ptr += sizeof(TxPD);

    //HEXDUMP("Tx Data", (cyg_uint8 *) skb->data, skb->len);
    for (i = 0;  i < sg_len;  i++) {
		memcpy((void *)ptr, (void *)sg_list[i].buf, sg_list[i].len);
		ptr += sg_list[i].len;
		//diag_printf("sg_list[i].buf = %x %x\n",sg_list[i].buf,dest,sg_list[i].len);
	}

 //   memcpy(ptr, skb->data, skb->len);
 	priv->tx_tail = (TXBD *)txbd->next;
 	txbd->SL = total_len + sizeof(TxPD); 
	txbd->mode |= TXfOwnership_DMA;
 	

#if 0
    ret = sbi_host_to_card(wlan_priv, MVMS_DAT,
                           txbd->buffer, total_len + sizeof(TxPD));

    if (ret) {
        diag_printf("Tx Error: sbi_host_to_card failed: 0x%X\n", ret);
        goto done;
    }

    diag_printf("SendSinglePacket succeeds\n");

  done:
    if (!ret) {
        wlan_priv->stats.tx_packets++;
        wlan_priv->stats.tx_bytes += skb->len;
    } else {
        wlan_priv->stats.tx_dropped++;
        wlan_priv->stats.tx_errors++;
    }
#else
done:
#endif
    /* need to be freed in all cases */
    //os_free_tx_packet(priv);
    

    LEAVE();
    return ret;
}

#define eth_drv_tx_done(sc,key,retval) (sc)->funs->eth_drv->tx_done(sc,key,retval)
int handle_send(struct eth_drv_sc *sc)
{
	wlan_private * wlan_priv = (wlan_private *)sc->driver_private;
	w99702_priv_t *priv = wlan_priv->priv;
	wlan_adapter *Adapter = wlan_priv->adapter;
    int ret = WLAN_STATUS_SUCCESS;
    TXBD *txbd =( TXBD *)priv->tx_head;
	
	if (wlan_priv->wlan_dev.dnld_sent) {
        diag_printf("TX Error: dnld_sent = %d, not sending\n",
               wlan_priv->wlan_dev.dnld_sent);
        goto done;
    }
	

	if(txbd->mode & TXfOwnership_DMA)
	{
	    ret = sbi_host_to_card(wlan_priv, MVMS_DAT,
                           (cyg_uint8 *)txbd->buffer, txbd->SL/*total_len + sizeof(TxPD)*/);
    
    	if (ret) {
        	diag_printf("Tx Error: sbi_host_to_card failed: 0x%X\n", ret);
	        goto done;
    	}
    
	   // diag_printf("SendSinglePacket succeeds\n");
	    
	    Adapter->TxSkbNum--;
	    
	    txbd->SL=0; 
    	txbd->mode = 0;
		priv->tx_head = (TXBD *)txbd->next;
		//if(priv->tx_full && (priv->tx_head == priv->tx_tail))//clyu
    	//	eth_drv_tx_done(sc,0,0);
    	cyg_flag_setbits( &priv->send_flag, 1 );
done:
    	if (!ret) {
       		wlan_priv->stats.tx_complete++;
	   		wlan_priv->stats.tx_count += txbd->SL;
    	} else {
       		wlan_priv->stats.tx_dropped++;
	   		// wlan_priv->stats.tx_errors++;
    	}
    	//txbd->SL=0; 
       	//txbd->mode=0;
    }
    	
}
/********************************************************
		Global functions
********************************************************/

/** 
 *  @brief This function checks the conditions and sends packet to IF
 *  layer if everything is ok.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   n/a
 */
void
wlan_process_tx(wlan_private * priv, struct eth_drv_sg *sg_list,
                               int sg_len,
                               int total_len)
{
    wlan_adapter *Adapter = priv->adapter;
    int volatile save_irq;

//    OS_INTERRUPT_SAVE_AREA;

    ENTER();
#if 0
    if (priv->wlan_dev.dnld_sent) {
        diag_printf("TX Error: dnld_sent = %d, not sending\n",
               priv->wlan_dev.dnld_sent);
        goto done;
    }
#endif
	
	//OS_INT_DISABLE(save_irq);
    priv->adapter->HisRegCpy &= ~HIS_TxDnLdRdy;
    //OS_INT_RESTORE(save_irq);

    SendSinglePacket(priv, sg_list, sg_len, total_len);

    
  done:
    LEAVE();
}

/** 
 *  @brief This function queues the packet received from
 *  kernel/upper layer and wake up the main thread to handle it.
 *  
 *  @param priv    A pointer to wlan_private structure
  * @param skb     A pointer to skb which includes TX packet
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
typedef struct sdio_buffer{
	struct eth_drv_sg *sg_list;
	int sg_len;
	int total_len;
}sdio_buffer_t;
int
wlan_tx_packet(wlan_private * priv, struct eth_drv_sg *sg_list,
                               int sg_len,
                               int total_len)
{
    ulong flags;
    wlan_adapter *Adapter = priv->adapter;
    int ret = WLAN_STATUS_SUCCESS;

    ENTER();

    //HEXDUMP("TX Data", skb->data, MIN(total_len, 100));

    //spin_lock_irqsave(&Adapter->CurrentTxLock, flags);
    OS_INT_DISABLE(flags);

    if (Adapter->wmm.enabled) {
        wmm_map_and_add_skb(priv, sg_list, sg_len, total_len);
        //wake_up_interruptible(&priv->MainThread.waitQ);
        
        OS_INT_RESTORE(flags);
        
        cyg_flag_setbits( &priv->MainThread.waitQ_flag_q, 1 ); 

    } else {

		#if 0
        list_add_tail((struct list_head *) skb,
                      (struct list_head *) &priv->adapter->TxSkbQ);
        //wake_up_interruptible(&priv->MainThread.waitQ);
       	#endif
   		wlan_process_tx(priv, sg_list, sg_len,  total_len);
   		Adapter->TxSkbNum++;
   		
   		OS_INT_RESTORE(flags);
   		
        cyg_flag_setbits( &priv->MainThread.waitQ_flag_q, 1 ); 
       
    }
    
    //spin_unlock_irqrestore(&Adapter->CurrentTxLock, flags);

    LEAVE();

    return ret;
}

/** 
 *  @brief This function tells firmware to send a NULL data packet.
 *  
 *  @param priv     A pointer to wlan_private structure
 *  @param pwr_mgmt indicate if power management bit should be 0 or 1
 *  @return 	    n/a
 */
int
SendNullPacket(wlan_private * priv, cyg_uint8 pwr_mgmt)
{
    wlan_adapter *Adapter = priv->adapter;
    TxPD txpd;
    int ret = WLAN_STATUS_SUCCESS;
    cyg_uint8 *ptr = priv->adapter->TmpTxBuf;

    ENTER();

    if (priv->adapter->SurpriseRemoved == TRUE) {
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    if (priv->adapter->MediaConnectStatus == WlanMediaStateDisconnected) {
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    memset((void*)&txpd, 0, sizeof(TxPD));

    txpd.TxControl = Adapter->PktTxCtrl;
    txpd.PowerMgmt = pwr_mgmt;
    txpd.TxPacketLocation = sizeof(TxPD);

    ptr += SDIO_HEADER_LEN;
    memcpy(ptr, (void*)&txpd, sizeof(TxPD));

    ret = sbi_host_to_card(priv, MVMS_DAT,
                           priv->adapter->TmpTxBuf, sizeof(TxPD));

    if (ret != 0) {
        diag_printf("TX Error: SendNullPacket failed!\n");
        goto done;
    }

  done:
    LEAVE();
    return ret;
}

/** 
 *  @brief This function check if we need send last packet indication.
 *  
 *  @param priv     A pointer to wlan_private structure
 *
 *  @return 	   TRUE or FALSE
 */
BOOLEAN
CheckLastPacketIndication(wlan_private * priv)
{
    wlan_adapter *Adapter = priv->adapter;
    BOOLEAN ret = FALSE;
    BOOLEAN prop_ps = TRUE;
    if (Adapter->sleep_period.period == 0)
        goto done;
    if (Adapter->wmm.enabled) {
        if (wmm_lists_empty(priv)) {
            if (((Adapter->CurBssParams.wmm_uapsd_enabled == TRUE) &&
                 (Adapter->wmm.qosinfo != 0)) || prop_ps)
                ret = TRUE;
        }
        goto done;
    }
    if (!Adapter->TxSkbNum)
        ret = TRUE;
  done:
    return ret;
}

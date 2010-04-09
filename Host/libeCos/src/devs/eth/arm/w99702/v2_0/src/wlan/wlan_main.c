/** @file wlan_main.c
  *  
  * @brief This file contains the major functions in WLAN
  * driver. It includes init, exit, open, close and main
  * thread etc..
  * 
  */
/**
  * @mainpage M-WLAN Linux Driver
  *
  * @section overview_sec Overview
  *
  * The M-WLAN is a Linux reference driver for Marvell
  * 802.11 (a/b/g) WLAN chipset.
  * 
  * @section copyright_sec Copyright
  *
  * Copyright ?Marvell International Ltd. and/or its affiliates, 2003-2006
  *
  */
/********************************************************
Change log:
	09/30/05: Add Doxygen format comments
	12/09/05: Add TX_QUEUE support	
	01/05/06: Add kernel 2.6.x support	
	01/11/06: Change compile flag BULVERDE_SDIO to SD to support
	          Monahans/Zylonite
	01/11/06: Conditionalize new scan/join functions.
	01/12/06: Add TxLockFlag for UAPSD power save mode 
	          and Proprietary Periodic sleep support
	02/13/06: Add a patch for USB interoperability issue
********************************************************/

#include	"include.h"
#include	"sys/malloc.h"



#define WIRELESS_THREAD_PRIORITY	10	//CYGPKG_NET_FAST_THREAD_PRIORITY

/********************************************************
		Local Variables
********************************************************/
extern cyg_netdevtab_entry_t w99702_netdev;
extern struct eth_drv_sc w99702_sc;

#ifdef ENABLE_PM
static struct pm_dev *wlan_pm_dev = NULL;
#endif

//spinlock_t driver_lock = SPIN_LOCK_UNLOCKED;
ulong driver_flags;

#define WLAN_TX_PWR_DEFAULT		20      /*100mW */
#define WLAN_TX_PWR_US_DEFAULT		20      /*100mW */
#define WLAN_TX_PWR_JP_DEFAULT		16      /*50mW */
#define WLAN_TX_PWR_FR_100MW		20      /*100mW */
#define WLAN_TX_PWR_EMEA_DEFAULT	20      /*100mW */

/* Format { Channel, Frequency (MHz), MaxTxPower } */
/* Band: 'B/G', Region: USA FCC/Canada IC */
static CHANNEL_FREQ_POWER channel_freq_power_US_BG[] = {
    {1, 2412, WLAN_TX_PWR_US_DEFAULT},
    {2, 2417, WLAN_TX_PWR_US_DEFAULT},
    {3, 2422, WLAN_TX_PWR_US_DEFAULT},
    {4, 2427, WLAN_TX_PWR_US_DEFAULT},
    {5, 2432, WLAN_TX_PWR_US_DEFAULT},
    {6, 2437, WLAN_TX_PWR_US_DEFAULT},
    {7, 2442, WLAN_TX_PWR_US_DEFAULT},
    {8, 2447, WLAN_TX_PWR_US_DEFAULT},
    {9, 2452, WLAN_TX_PWR_US_DEFAULT},
    {10, 2457, WLAN_TX_PWR_US_DEFAULT},
    {11, 2462, WLAN_TX_PWR_US_DEFAULT}
};

/* Band: 'B/G', Region: Europe ETSI */
static CHANNEL_FREQ_POWER channel_freq_power_EU_BG[] = {
    {1, 2412, WLAN_TX_PWR_EMEA_DEFAULT},
    {2, 2417, WLAN_TX_PWR_EMEA_DEFAULT},
    {3, 2422, WLAN_TX_PWR_EMEA_DEFAULT},
    {4, 2427, WLAN_TX_PWR_EMEA_DEFAULT},
    {5, 2432, WLAN_TX_PWR_EMEA_DEFAULT},
    {6, 2437, WLAN_TX_PWR_EMEA_DEFAULT},
    {7, 2442, WLAN_TX_PWR_EMEA_DEFAULT},
    {8, 2447, WLAN_TX_PWR_EMEA_DEFAULT},
    {9, 2452, WLAN_TX_PWR_EMEA_DEFAULT},
    {10, 2457, WLAN_TX_PWR_EMEA_DEFAULT},
    {11, 2462, WLAN_TX_PWR_EMEA_DEFAULT},
    {12, 2467, WLAN_TX_PWR_EMEA_DEFAULT},
    {13, 2472, WLAN_TX_PWR_EMEA_DEFAULT}
};

/* Band: 'B/G', Region: Spain */
static CHANNEL_FREQ_POWER channel_freq_power_SPN_BG[] = {
    {10, 2457, WLAN_TX_PWR_DEFAULT},
    {11, 2462, WLAN_TX_PWR_DEFAULT}
};

/* Band: 'B/G', Region: France */
static CHANNEL_FREQ_POWER channel_freq_power_FR_BG[] = {
    {10, 2457, WLAN_TX_PWR_FR_100MW},
    {11, 2462, WLAN_TX_PWR_FR_100MW},
    {12, 2467, WLAN_TX_PWR_FR_100MW},
    {13, 2472, WLAN_TX_PWR_FR_100MW}
};

/* Band: 'B/G', Region: Japan */
static CHANNEL_FREQ_POWER channel_freq_power_JPN_BG[] = {
    {1, 2412, WLAN_TX_PWR_JP_DEFAULT},
    {2, 2417, WLAN_TX_PWR_JP_DEFAULT},
    {3, 2422, WLAN_TX_PWR_JP_DEFAULT},
    {4, 2427, WLAN_TX_PWR_JP_DEFAULT},
    {5, 2432, WLAN_TX_PWR_JP_DEFAULT},
    {6, 2437, WLAN_TX_PWR_JP_DEFAULT},
    {7, 2442, WLAN_TX_PWR_JP_DEFAULT},
    {8, 2447, WLAN_TX_PWR_JP_DEFAULT},
    {9, 2452, WLAN_TX_PWR_JP_DEFAULT},
    {10, 2457, WLAN_TX_PWR_JP_DEFAULT},
    {11, 2462, WLAN_TX_PWR_JP_DEFAULT},
    {12, 2467, WLAN_TX_PWR_JP_DEFAULT},
    {13, 2472, WLAN_TX_PWR_JP_DEFAULT},
    {14, 2484, WLAN_TX_PWR_JP_DEFAULT}
};

/********************************************************
		Global Variables
********************************************************/

/**
 * the structure for channel, frequency and power
 */
typedef struct _region_cfp_table
{
    cyg_uint8 region;
    CHANNEL_FREQ_POWER *cfp_BG;
    int cfp_no_BG;
} region_cfp_table_t;

/**
 * the structure for the mapping between region and CFP
 */
region_cfp_table_t region_cfp_table[] = {
    {0x10,                      /*US FCC */
     channel_freq_power_US_BG,
     sizeof(channel_freq_power_US_BG) / sizeof(CHANNEL_FREQ_POWER),
     }
    ,
    {0x20,                      /*CANADA IC */
     channel_freq_power_US_BG,
     sizeof(channel_freq_power_US_BG) / sizeof(CHANNEL_FREQ_POWER),
     }
    ,
    {0x30, /*EU*/ channel_freq_power_EU_BG,
     sizeof(channel_freq_power_EU_BG) / sizeof(CHANNEL_FREQ_POWER),
     }
    ,
    {0x31, /*SPAIN*/ channel_freq_power_SPN_BG,
     sizeof(channel_freq_power_SPN_BG) / sizeof(CHANNEL_FREQ_POWER),
     }
    ,
    {0x32, /*FRANCE*/ channel_freq_power_FR_BG,
     sizeof(channel_freq_power_FR_BG) / sizeof(CHANNEL_FREQ_POWER),
     }
    ,
    {0x40, /*JAPAN*/ channel_freq_power_JPN_BG,
     sizeof(channel_freq_power_JPN_BG) / sizeof(CHANNEL_FREQ_POWER),
     }
    ,
/*Add new region here */
};

/**
 * the rates supported by the card
 */
cyg_uint8 WlanDataRates[WLAN_SUPPORTED_RATES] =
    { 0x02, 0x04, 0x0B, 0x16, 0x00, 0x0C, 0x12,
    0x18, 0x24, 0x30, 0x48, 0x60, 0x6C, 0x00
};

/**
 * the rates supported
 */
cyg_uint8 SupportedRates[G_SUPPORTED_RATES] =
    { 0x82, 0x84, 0x8b, 0x96, 0x0c, 0x12, 0x18, 0x24, 0x30, 0x48, 0x60, 0x6c,
0 };

/**
 * the rates supported for ad-hoc G mode
 */
cyg_uint8 AdhocRates_G[G_SUPPORTED_RATES] =
    { 0x82, 0x84, 0x8b, 0x96, 0x0c, 0x12, 0x18, 0x24, 0x30, 0x48, 0x60, 0x6c,
0 };

/**
 * the rates supported for ad-hoc B mode
 */
cyg_uint8 AdhocRates_B[4] = { 0x82, 0x84, 0x8b, 0x96 };

/**
 * the global variable of a pointer to wlan_private
 * structure variable
 */
//wlan_private wlanpriv;
extern wlan_private w99702_priv_data0;

cyg_uint32 DSFreqList[15] = {
    0, 2412000, 2417000, 2422000, 2427000, 2432000, 2437000, 2442000,
    2447000, 2452000, 2457000, 2462000, 2467000, 2472000, 2484000
};

/**
 * the table to keep region code
 */
cyg_uint16 RegionCodeToIndex[MRVDRV_MAX_REGION_CODE] =
    { 0x10, 0x20, 0x30, 0x31, 0x32, 0x40 };

/*
* eCos thread
*/

#pragma arm section zidata = "nozero"
static char thread_stack[8192];
static cyg_handle_t thread_handle;
static cyg_thread thread_data;
static char reassoc_stack[8192];
static cyg_handle_t reassoc_handle;
static cyg_thread reassoc_data;
#pragma arm section zidata

cyg_uint8 g_bMainThreadResume = FALSE;
cyg_handle_t thread_stop_ptr[2];

/********************************************************
		Local Functions
********************************************************/

/** 
 *  @brief This function opens the network device
 *  
 *  @param dev     A pointer to net_device structure
 *  @return 	   WLAN_STATUS_SUCCESS
 */
int
wlan_open(struct eth_drv_sc *sc)
{
    wlan_private *priv = (wlan_private *) sc->driver_private;
    wlan_adapter *adapter = priv->adapter;

    ENTER();

//    MODULE_GET;

    priv->open = TRUE;
#if 0
    if (adapter->MediaConnectStatus == WlanMediaStateConnected)
        os_carrier_on(priv);
    else
        os_carrier_off(priv);

    os_start_queue(priv);
#endif
    LEAVE();
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function closes the network device
 *  
 *  @param dev     A pointer to net_device structure
 *  @return 	   WLAN_STATUS_SUCCESS
 */
int
wlan_close(struct eth_drv_sc *sc)
{
    wlan_private *priv = sc->driver_private;
    w99702_priv_t * priv702 = (w99702_priv_t *)priv->priv;

    ENTER();

    /* Flush all the packets upto the OS before stopping */
   // wlan_send_rxskbQ(priv);
   if((priv702->rx_head != priv702->rx_tail) || !priv702->rx_head->mode)
       	eth_drv_dsr(0, 0, (cyg_addrword_t)sc);
   // os_stop_queue(priv);
   // os_carrier_off(priv);

//    MODULE_PUT;

    priv->open = FALSE;

    LEAVE();
    return WLAN_STATUS_SUCCESS;
}

#ifdef ENABLE_PM


struct pm_dev *pm_register(pm_dev_t type,
                           unsigned long id,
                           pm_callback callback)
{
        struct pm_dev *dev = malloc(sizeof(struct pm_dev));
        if (dev) {
                memset(dev, 0, sizeof(*dev));
                dev->type = type;
                dev->id = id;
                dev->callback = callback;

                down(&pm_devs_lock);
                list_add(&dev->entry, &pm_devs);
                up(&pm_devs_lock);
        }
        return dev;
}


/** 
 *  @brief This function is a callback function. it is called by
 *  kernel to enter or exit power saving mode.
 *  
 *  @param pmdev   A pointer to pm_dev
 *  @param pmreq   pm_request_t
 *  @param pmdata  A pointer to pmdata
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_pm_callback(struct pm_dev *pmdev, pm_request_t pmreq, void *pmdata)
{
    wlan_private *priv = &w99702_priv_data0;
    wlan_adapter *Adapter = priv->adapter;
	//struct net_device *dev = priv->wlan_dev.netdev;
	struct cyg_netdevtab_entry *dev = priv->wlan_dev.netdev;

    static BOOLEAN OS_Enable_DS = FALSE;

    diag_printf("WPRM_PM_CALLBACK: pmreq = %d.\n", pmreq);

    switch (pmreq) {
    case PM_SUSPEND:
        diag_printf("WPRM_PM_CALLBACK: enter PM_SUSPEND.\n");

#ifdef WPRM_DRV
        /* check WLAN_HOST_WAKEB */
        if (wprm_wlan_host_wakeb_is_triggered()) {
            diag_printf("exit on GPIO-1 triggered.\n");
            return WLAN_STATUS_FAILURE;
        }
#endif

        /* in associated mode */
        if (Adapter->MediaConnectStatus == WlanMediaStateConnected) {
            if ((Adapter->PSState != PS_STATE_SLEEP)
                || !Adapter->bWakeupDevRequired || (Adapter->WakeupTries != 0)
                ) {
                diag_printf("wlan_pm_callback: can't enter sleep mode\n");
                return WLAN_STATUS_FAILURE;
            } else {

                /*
                 * Detach the network interface
                 * if the network is running
                 */
                if (netif_running(dev)) {
                    netif_device_detach(dev);
                    diag_printf("netif_device_detach().\n");
                }
                /* Stop SDIO bus clock */
                stop_bus_clock_2(((mmc_card_t) ((priv->wlan_dev).card))->
                                 ctrlr);
                sbi_suspend(priv);
            }
            break;
        }

        /* in non associated mode */

        /*
         * Detach the network interface 
         * if the network is running
         */
        if (netif_running(dev))
            netif_device_detach(dev);

        /* 
         * Storing and restoring of the regs be taken care 
         * at the driver rest will be done at wlan driver
         * this makes driver independent of the card
         */
        if (Adapter->IsDeepSleep == FALSE) {
            SetDeepSleep(priv, TRUE);
            OS_Enable_DS = TRUE;
        }

        sbi_suspend(priv);

        break;

    case PM_RESUME:
        /* in associated mode */
        if (Adapter->MediaConnectStatus == WlanMediaStateConnected) {
            if (Adapter->bWakeupDevRequired == FALSE) {
                /* could never happen */
                diag_printf("wlan_pm_callback: serious error.\n");
            } else {
                /*
                 * Bring the inteface up first
                 * This case should not happen still ...
                 */
                sbi_resume(priv);

                /*
                 * Start SDIO bus clock
                 */
                sbi_set_bus_clock(priv, TRUE);
                /*
                 * Attach the network interface
                 * if the network is running
                 */
                if (netif_running(dev)) {
                    netif_device_attach(dev);
                    diag_printf("after netif_device_attach().\n");
                }
                diag_printf("After netif attach, in associated mode.\n");
            }
            break;
        }

        /* in non associated mode */

#ifdef WPRM_DRV
        /* Background scan support */
        WPRM_DRV_TRACING_PRINT();
        /* check if WLAN_HOST_WAKEB triggered, turn on SDIO_CLK */
        if (wprm_wlan_host_wakeb_is_triggered()) {      /* WLAN_HSOT_WAKEB is triggered */
            if (sbi_set_bus_clock(priv, TRUE)) {
                diag_printf(
                       "wlan_pm_callback: in PM_RESUME, wlan sdio clock turn on fail\n");
            }
            WPRM_DRV_TRACING_PRINT();
        }
#endif
        /*
         * Bring the inteface up first 
         * This case should not happen still ...
         */

        if (OS_Enable_DS == TRUE) {
#ifdef WPRM_DRV
            /* if need to wakeup FW, then trigger HOST_WLAN_WAKEB first */
            wprm_trigger_host_wlan_wakeb(1);
#endif
        }

        sbi_resume(priv);

        if (OS_Enable_DS == TRUE) {
            SetDeepSleep(priv, FALSE);
            OS_Enable_DS = FALSE;
        }

        if (netif_running(dev))
            netif_device_attach(dev);

        diag_printf("after netif attach, in NON associated mode.\n");
        break;
    }

    return WLAN_STATUS_SUCCESS;
}
#endif /* ENABLE_PM */

#if 0
/** 
 *  @brief This function handles packet transmission
 *  
 *  @param skb     A pointer to sk_buff structure
 *  @param dev     A pointer to net_device structure
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
wlan_hard_start_xmit(struct sk_buff *skb, struct eth_drv_sc *sc)
{
    int ret;
    wlan_private *priv = sc->driver_private;

    ENTER();

    if (wlan_tx_packet(priv, skb)) {
        /* Transmit failed */
        ret = WLAN_STATUS_FAILURE;
        goto done;
    } else {
        /* Transmit succeeded */
	#if 0
        if (!priv->adapter->wmm.enabled) {
            if (priv->adapter->TxSkbNum >= MAX_NUM_IN_TX_Q) {
                UpdateTransStart(dev);
                os_stop_queue(priv);
            }
        }
	#endif
    }

    ret = WLAN_STATUS_SUCCESS;
  done:

    LEAVE();
    return ret;
}


/** 
 *  @brief This function handles the timeout of packet
 *  transmission
 *  
 *  @param dev     A pointer to net_device structure
 *  @return 	   n/a
 */
static void
wlan_tx_timeout(struct net_device *dev)
{
    wlan_private *priv = (wlan_private *) dev->priv;

    ENTER();

    diag_printf("tx watch dog timeout!\n");

    priv->wlan_dev.dnld_sent = DNLD_RES_RECEIVED;
    UpdateTransStart(dev);

    if (priv->adapter->CurrentTxSkb) {
        //wake_up_interruptible(&priv->MainThread.waitQ);

        cyg_flag_setbits( &priv->MainThread.waitQ_flag_q, 1 ); 
		
    } else {
        //os_start_queue(priv);
    }

    LEAVE();
}
#endif
/** 
 *  @brief This function returns the network statistics
 *  
 *  @param dev     A pointer to wlan_private structure
 *  @return 	   A pointer to net_device_stats structure
 */
static struct ether_drv_stats *
wlan_get_stats(struct eth_drv_sc *sc)
{
    wlan_private * wlan_priv = (wlan_private *)sc->driver_private;

    return &wlan_priv->stats;
}

/** 
 *  @brief This function sets the MAC address to firmware.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @param pRxPD   A pointer to RxPD structure of received packet
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
wlan_set_mac_address(struct eth_drv_sc *sc, void *addr)
{
    int ret = WLAN_STATUS_SUCCESS;
    wlan_private *priv = (wlan_private *) sc->driver_private;
    wlan_adapter *Adapter = priv->adapter;
//    struct sockaddr *pHwAddr = (struct sockaddr *) addr;

    ENTER();

    memset(Adapter->CurrentAddr, 0, MRVDRV_ETH_ADDR_LEN);

    /* dev->dev_addr is 8 bytes */
//    HEXDUMP("dev->dev_addr:", dev->dev_addr, ETH_ALEN);

    //HEXDUMP("addr:", pHwAddr->sa_data, ETH_ALEN);
    memcpy(Adapter->CurrentAddr, addr, ETH_ALEN);

    ret = PrepareAndSendCommand(priv, HostCmd_CMD_802_11_MAC_ADDRESS,
                                HostCmd_ACT_SET,
                                HostCmd_OPTION_WAITFORRSP, 0, NULL);

    if (ret) {
        diag_printf("set mac address failed.\n");
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    HEXDUMP("Adapter->MacAddr:", Adapter->CurrentAddr, ETH_ALEN);
   // memcpy(dev->dev_addr, Adapter->CurrentAddr, ETH_ALEN);

  done:
    LEAVE();
    return ret;
}

/** 
 *  @brief This function sets multicast addresses to firmware
 *  
 *  @param dev     A pointer to net_device structure
 *  @return 	   n/a
 */
static void
wlan_set_multicast_list(struct eth_drv_sc *sc/*struct net_device *dev*/)
{
    wlan_private *priv = (wlan_private *)sc->driver_private;
    wlan_adapter *Adapter = priv->adapter;
    int OldPacketFilter;
#if 0
    ENTER();

    OldPacketFilter = Adapter->CurrentPacketFilter;

    if (dev->flags & IFF_PROMISC) {
        diag_printf("Enable Promiscuous mode\n");
        Adapter->CurrentPacketFilter |= HostCmd_ACT_MAC_PROMISCUOUS_ENABLE;
        Adapter->CurrentPacketFilter &=
            ~(HostCmd_ACT_MAC_ALL_MULTICAST_ENABLE |
              HostCmd_ACT_MAC_MULTICAST_ENABLE);
    } else {
        /* Multicast */
        Adapter->CurrentPacketFilter &= ~HostCmd_ACT_MAC_PROMISCUOUS_ENABLE;

        if (dev->flags & IFF_ALLMULTI || dev->mc_count >
            MRVDRV_MAX_MULTICAST_LIST_SIZE) {
            diag_printf("Enabling All Multicast!\n");
            Adapter->CurrentPacketFilter |=
                HostCmd_ACT_MAC_ALL_MULTICAST_ENABLE;
            Adapter->CurrentPacketFilter &= ~HostCmd_ACT_MAC_MULTICAST_ENABLE;
        } else {
            Adapter->CurrentPacketFilter &=
                ~HostCmd_ACT_MAC_ALL_MULTICAST_ENABLE;

            if (!dev->mc_count) {
                diag_printf("No multicast addresses - "
                       "disabling multicast!\n");
                Adapter->CurrentPacketFilter &=
                    ~HostCmd_ACT_MAC_MULTICAST_ENABLE;
            } else {
                int i;

                Adapter->CurrentPacketFilter |=
                    HostCmd_ACT_MAC_MULTICAST_ENABLE;

                Adapter->NumOfMulticastMACAddr =
                    CopyMulticastAddrs(Adapter, dev);

                diag_printf("Multicast addresses: %d\n", dev->mc_count);

                for (i = 0; i < dev->mc_count; i++) {
                    diag_printf("Multicast address %d:"
                           "%x %x %x %x %x %x\n", i,
                           Adapter->MulticastList[i][0],
                           Adapter->MulticastList[i][1],
                           Adapter->MulticastList[i][2],
                           Adapter->MulticastList[i][3],
                           Adapter->MulticastList[i][4],
                           Adapter->MulticastList[i][5]);
                }
                /* set multicast addresses to firmware */
                PrepareAndSendCommand(priv, HostCmd_CMD_MAC_MULTICAST_ADR,
                                      HostCmd_ACT_GEN_SET, 0, 0, NULL);
            }
        }
    }

    if (Adapter->CurrentPacketFilter != OldPacketFilter) {
        SetMacPacketFilter(priv);
    }

    LEAVE();
#endif
}

#if 0
/** 
 *  @brief This function pops rx_skb from the rx queue.
 *  
 *  @param RxSkbQ  A pointer to rx_skb queue
 *  @return 	   A pointer to skb
 */
static struct sk_buff *
wlan_pop_rx_skb(struct list_head *RxSkbQ)
{
    struct sk_buff *skb_data = NULL;

    if (!list_empty((struct list_head *) RxSkbQ)) {
        skb_data = RxSkbQ->next;
        list_del((struct list_head *) RxSkbQ->next);
    }

    return skb_data;
}
#endif

int WakeUpWaitQ(wlan_thread *thread)
{

	cyg_flag_setbits( &thread->waitQ_flag_q, 1 ); 

	return 0;

}

int  udelay(int volatile n)
{
	int volatile nLoop;
	nLoop = n;
	while(nLoop--);
}

int thread_stop(cyg_handle_t k)
{
	//return kthread_stop_sem(k, NULL);
	if(k == thread_handle)
		thread_stop_ptr[0] = k;
	else
		thread_stop_ptr[1] = k;
}

int thread_should_stop(void)
{
	if(cyg_thread_self() == thread_handle)
		return (thread_stop_ptr[0] == cyg_thread_self());
	else
		return (thread_stop_ptr[1] == cyg_thread_self());
}

/** 
 *  @brief This function hanldes the major job in WLAN driver.
 *  it handles the event generated by firmware, rx data received
 *  from firmware and tx data sent from kernel.
 *  
 *  @param data    A pointer to wlan_thread structure
 *  @return 	   WLAN_STATUS_SUCCESS
 */
static void wlan_service_main_thread(cyg_addrword_t data)
{
	struct eth_drv_sc *sc = (struct eth_drv_sc *)data;
	wlan_private *priv = (wlan_private *)sc->driver_private;
	w99702_priv_t * priv702 = priv->priv;
    wlan_thread *thread = &priv->MainThread;
    wlan_adapter *Adapter = priv->adapter;
    //wait_queue_t wait;
    cyg_uint8 ireg = 0;
    int volatile save_irq;
    int x;
    int ret;

    //OS_INTERRUPT_SAVE_AREA;

    ENTER();

    wlan_activate_thread(thread);
    thread_stop_ptr[0] = NULL;

    //init_waitqueue_entry(&wait, current);

    for (;;) {
    
    
        #if 0
        diag_printf("main-thread 111: IntCounter=%d "
               "CurCmd=%p CmdPending=%s Connect=%d "
               "dnld_sent=%d\n",
               Adapter->IntCounter,
               Adapter->CurCmd,
               list_empty(&Adapter->CmdPendingQ) ? "N" : "Y",
               Adapter->MediaConnectStatus,
               /*Adapter->CurrentTxSkb,*/ priv->wlan_dev.dnld_sent);
		#endif
        //add_wait_queue(&thread->waitQ, &wait);
        //OS_SET_THREAD_STATE(TASK_INTERRUPTIBLE);

        TX_DISABLE;
				
        if ((Adapter->WakeupTries) ||
            (Adapter->PSState == PS_STATE_SLEEP
             && !Adapter->bWakeupDevRequired) ||
            (!Adapter->IntCounter
             && (priv->wlan_dev.dnld_sent || !Adapter->wmm.enabled ||
                 Adapter->TxLockFlag /*|| !os_queue_is_active(priv)*/ ||
                 wmm_lists_empty(priv))
             && (priv->wlan_dev.dnld_sent || Adapter->TxLockFlag ||
                 !Adapter->TxSkbNum)
             && (priv->wlan_dev.dnld_sent || Adapter->CurCmd ||
                 list_empty(&Adapter->CmdPendingQ))
            )
            ) {
            #if 0
            diag_printf(
                   "main-thread sleeping... Conn=%d IntC=%d PS_Mode=%d PS_State=%d\n",
                   Adapter->MediaConnectStatus, Adapter->IntCounter,
                   Adapter->PSMode, Adapter->PSState);
			#endif


#ifdef _MAINSTONE
            MST_LEDDAT1 = get_utimeofday();
#endif
            TX_RESTORE;
            //schedule();
            /* Lock control thread to avoid re-entry when waiting event. ??? */
			//cyg_mutex_lock(&thread->waitQ_mutex);
			
			//diag_printf("main_thread wait\n");
			/* Waitint until any event input */
			x = cyg_flag_wait(
            &thread->waitQ_flag_q,
            -1,
            CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR );
            
            //diag_printf("main_thread wakeup %d\n", Adapter->IntCounter);
/* ----------if keypad pressed then print message-----------------  */    
    	//if ( 2 & x )

		
        } else {
            TX_RESTORE;
        }
		
		#if 0
        diag_printf(
               "main-thread 222 (waking up): IntCounter=%d  "
               "dnld_sent=%d\n", Adapter->IntCounter, /*Adapter->CurrentTxSkb,*/
               priv->wlan_dev.dnld_sent);
		
        //OS_SET_THREAD_STATE(TASK_RUNNING);
        //remove_wait_queue(&thread->waitQ, &wait);
		
        diag_printf("main-thread 333: IntCounter=%d "//CurrentTxSkb=%p
               "dnld_sent=%d g_NeedWakeupMainThread = %d\n",
               Adapter->IntCounter,
               /*Adapter->CurrentTxSkb,*/ priv->wlan_dev.dnld_sent, g_WakeupMainThreadStatus);
		#endif
		
        if (thread_should_stop()
            || Adapter->SurpriseRemoved) {
            diag_printf(
                   "main-thread: break from main thread: SurpriseRemoved=0x%x\n",
                   Adapter->SurpriseRemoved);
            break;
        }
		
		//OS_INT_DISABLE(save_irq);
        if(Adapter->IntCounter)
        {
            Adapter->IntCounter = 0;
            
            if ((ret = sbi_get_int_status(sc, &ireg)) < 0) {
                diag_printf("main-thread: reading HOST_INT_STATUS_REG failed\n");
                
                //OS_INT_RESTORE(save_irq);
                continue;
            }
            
            OS_INT_DISABLE(save_irq);
            Adapter->HisRegCpy |= ireg;
            OS_INT_RESTORE(save_irq);
        }
        //else if (Adapter->bWakeupDevRequired && ((Adapter->PSState == PS_STATE_SLEEP)))
		else if (Adapter->bWakeupDevRequired && Adapter->HS_Activated)  //JONO
        {
        	//OS_INT_RESTORE(save_irq);
            Adapter->WakeupTries++;
            /* we borrow deep_sleep wakeup code for time being */
            if (sbi_exit_deep_sleep(priv))
                diag_printf("main-thread: wakeup dev failed\n");
            continue;
        }
        //else OS_INT_RESTORE(save_irq);
		
		#if 0
        diag_printf("main-thread 444: IntCounter=%d "// CurrentTxSkb=%p
               "dnld_sent=%d TxSkbNum=%d\n",
               Adapter->IntCounter,
               /*Adapter->CurrentTxSkb,*/ priv->wlan_dev.dnld_sent, Adapter->TxSkbNum);
		 diag_printf("Adapter->HisRegCpy = %x, %d, %d, %d\n",	
		 	Adapter->HisRegCpy, priv->wlan_dev.dnld_sent, Adapter->TxLockFlag, priv->open);
		#endif
        /* Command response? */

        if (Adapter->HisRegCpy & HIS_CmdUpLdRdy) {
            //diag_printf("main-thread: Cmd response ready.\n");

            OS_INT_DISABLE(save_irq);
            Adapter->HisRegCpy &= ~HIS_CmdUpLdRdy;
            OS_INT_RESTORE(save_irq);

            wlan_process_rx_command(priv);
        }
       

        /* Any received data? */
#if 1
        if (Adapter->HisRegCpy & HIS_RxUpLdRdy) {
            //diag_printf("main-thread: Rx Packet ready.\n");

            OS_INT_DISABLE(save_irq);
            Adapter->HisRegCpy &= ~HIS_RxUpLdRdy;
            OS_INT_RESTORE(save_irq);

            //wlan_send_rxskbQ(priv);
            if((priv702->rx_head != priv702->rx_tail) || !priv702->rx_head->mode)
            	eth_drv_dsr(0, 0, (cyg_addrword_t)sc);
        }
        else
        	OS_INT_RESTORE(save_irq);
#endif
        /* Any Card Event */
        if (Adapter->HisRegCpy & HIS_CardEvent) {
            //diag_printf("main-thread: Card Event Activity.\n");

            OS_INT_DISABLE(save_irq);
            Adapter->HisRegCpy &= ~HIS_CardEvent;
            OS_INT_RESTORE(save_irq);

            if (sbi_read_event_cause(priv)) {
                diag_printf("main-thread: sbi_read_event_cause failed.\n");
                continue;
            }
            wlan_process_event(priv);
        }

        /* Check if we need to confirm Sleep Request received previously */
        if (Adapter->PSState == PS_STATE_PRE_SLEEP) {
            if (!priv->wlan_dev.dnld_sent && !Adapter->CurCmd) {
                if (Adapter->MediaConnectStatus == WlanMediaStateConnected) {
                    diag_printf(
                           "main_thread: PRE_SLEEP--IntCounter=%d "// CurrentTxSkb=%p
                           "dnld_sent=%d CurCmd=%p, confirm now\n",
                           Adapter->IntCounter, /*Adapter->CurrentTxSkb,*/
                           priv->wlan_dev.dnld_sent, Adapter->CurCmd);

                    PSConfirmSleep(priv, (cyg_uint16) Adapter->PSMode);
                } else {
                    /* workaround for firmware sending deauth/linkloss event
                       immediately after sleep request, remove this after
                       firmware fixes it */
                    Adapter->PSState = PS_STATE_AWAKE;
                    diag_printf(
                           "main-thread: ignore PS_SleepConfirm in non-connected state\n");
                }
            }
        }

        /* The PS state is changed during processing of Sleep Request event above */
        if ((priv->adapter->PSState == PS_STATE_SLEEP)
            || (priv->adapter->PSState == PS_STATE_PRE_SLEEP)
            ) {
                    diag_printf("999 The PS state is changed during processing ... %d\n", priv->adapter->PSState);
             continue; //JONO
        }
        if (Adapter->HS_Activated && Adapter->bWakeupDevRequired) { //JONO
            diag_printf(
                   "main-thread: cannot send command or date, HS_Activated=%d\n",
                   Adapter->HS_Activated); //JONO
            continue;
        }

        /* Execute the next command */
        if (!priv->wlan_dev.dnld_sent && !Adapter->CurCmd) {
            ExecuteNextCommand(priv);
        }

        if (Adapter->wmm.enabled) {
            if (!wmm_lists_empty(priv) && (priv->open == TRUE)/* && os_queue_is_active(priv)*/) {
                if ((Adapter->PSState == PS_STATE_FULL_POWER) ||
                    (Adapter->sleep_period.period == 0)
                    || (Adapter->TxLockFlag == FALSE))
                    //wmm_process_tx(priv);
                    handle_send(sc);
            }
        } else {
            if (!priv->wlan_dev.dnld_sent && (Adapter->TxLockFlag == false)
            &&(priv->open == TRUE)
                /*&& !list_empty((struct list_head *) &priv->adapter->TxSkbQ)*/) {
                //wlan_process_txqueue(priv);
                //diag_printf("send01\n");
                handle_send(sc);
                //diag_printf("send02\n");
            }
        }
    }

    wlan_deactivate_thread(thread);

    LEAVE();
    return ;//WLAN_STATUS_SUCCESS;
}


/**
 * @brief This function adds the card. it will probe the
 * card, allocate the wlan_priv and initialize the device. 
 *  
 *  @param card    A pointer to card
 *  @return 	   A pointer to wlan_private structure
 */
//static 
wlan_private *
wlan_add_card(void *card)
{
    //struct net_device *dev = NULL;
    wlan_private *priv = NULL;
	//struct eth_drv_sc *dev;

    ENTER();

    /* probe the card */
    if (sbi_probe_card(card) < 0) {
        diag_printf("NO card found!\n");
        return NULL;
    }
#if 0
    /* Allocate an Ethernet device and register it */
    if (!(dev = alloc_etherdev(sizeof(wlan_private)))) {
        diag_printf("Init ethernet device failed!\n");
        return NULL;
    }
#endif

    priv = (wlan_private *)((cyg_uint32)&w99702_priv_data0 | NON_CACHE_FLAG);

    /* allocate buffer for wlan_adapter */
    MALLOC(priv->adapter, wlan_adapter *, sizeof(wlan_adapter), 0, M_NOWAIT);
    if(!priv->adapter)
    {
        diag_printf("Allocate buffer for wlan_adapter failed!\n");
        goto err_kmalloc;
    }
	priv->adapter = (wlan_adapter *)((unsigned int)priv->adapter | NON_CACHE_FLAG);
    /* init wlan_adapter */
    memset(priv->adapter, 0, sizeof(wlan_adapter));

    priv->wlan_dev.netdev = &w99702_netdev;
    priv->wlan_dev.card = card;
   // wlanpriv = priv;

   // SET_MODULE_OWNER(dev);

 #if 0
    /* Setup the OS Interface to our functions */
    dev->open = wlan_open;
    dev->hard_start_xmit = wlan_hard_start_xmit;
    dev->stop = wlan_close;
    dev->do_ioctl = wlan_do_ioctl;
    dev->set_mac_address = wlan_set_mac_address;

#define	WLAN_WATCHDOG_TIMEOUT	(2 * HZ)

    dev->tx_timeout = wlan_tx_timeout;
    dev->get_stats = wlan_get_stats;
    dev->watchdog_timeo = WLAN_WATCHDOG_TIMEOUT;

#ifdef	WIRELESS_EXT
    dev->get_wireless_stats = wlan_get_wireless_stats;
    dev->wireless_handlers = (struct iw_handler_def *) &wlan_handler_def;
#endif
#define NETIF_F_DYNALLOC 16
    dev->features |= NETIF_F_DYNALLOC;
    dev->flags |= IFF_BROADCAST | IFF_MULTICAST;
    dev->set_multicast_list = wlan_set_multicast_list;

#endif//clyu



#ifdef MFG_CMD_SUPPORT
    /* Required for the mfg command */
    //init_waitqueue_head(&priv->adapter->mfg_cmd_q);
    cyg_cond_init(&priv->adapter->mfg_cond_q, &priv->adapter->mfg_mutex);
#endif

    //init_waitqueue_head(&priv->adapter->ds_awake_q);
    cyg_cond_init(&priv->adapter->ds_cond_q, &priv->adapter->ds_mutex);

#ifdef ENABLE_PM
    if (!(wlan_pm_dev = pm_register(PM_UNKNOWN_DEV, 0, wlan_pm_callback)))
        diag_printf("Failed to register PM callback\n");
#endif

    INIT_LIST_HEAD(&priv->adapter->CmdFreeQ);
    INIT_LIST_HEAD(&priv->adapter->CmdPendingQ);

    diag_printf("Starting kthread...\n");
    priv->MainThread.priv = priv;
	
	cyg_thread_create(WIRELESS_THREAD_PRIORITY,          // Priority
                      wlan_service_main_thread,                   // entry
                      (cyg_addrword_t)&w99702_sc,                    // entry parameter
                      "pxa270m card support",                // Name
                      &thread_stack[0],         // Stack
                      sizeof(thread_stack),                            // Size
                      &thread_handle,    // Handle
                      &thread_data       // Thread data structure
            );
    priv->MainThread.task = thread_handle;
    cyg_thread_resume(thread_handle);    // Start it
	
//    ConfigureThreadPriority();

#ifdef REASSOCIATION
    priv->ReassocThread.priv = priv;
   /*
    wlan_create_thread(wlan_reassociation_thread,
                       &priv->ReassocThread, "wlan_reassoc_service");*/
	
	cyg_thread_create(WIRELESS_THREAD_PRIORITY,          // Priority
                      wlan_reassociation_thread,                   // entry
                      (cyg_addrword_t)&w99702_sc,                    // entry parameter
                      "wlan_reassoc_service",                // Name
                      &reassoc_stack[0],         // Stack
                      sizeof(reassoc_stack),                            // Size
                      &reassoc_handle,    // Handle
                      &reassoc_data       // Thread data structure
            );
	priv->ReassocThread.task = reassoc_handle;
    cyg_thread_resume(reassoc_handle);    // Start it
	
#endif /* REASSOCIATION */

    /*
     * Register the device. Fillup the private data structure with
     * relevant information from the card and request for the required
     * IRQ. 
     */

    if (sbi_register_dev(priv) < 0) {
        diag_printf("Failed to register wlan device!\n");
        goto err_registerdev;
    }
#if 0
    diag_printf("%s: Marvell Wlan 802.11 Adapter "
           "revision 0x%02X at IRQ %i\n", dev->name,
           priv->adapter->chip_rev, dev->irq);
#endif
    //wlan_proc_entry(priv, dev);
#ifdef PROC_DEBUG
    wlan_debug_entry(priv, dev);
#endif

#if 0
    /* Get the CIS Table */
    sbi_get_cis_info(priv);
#endif

    /* init FW and HW */
    if (wlan_init_fw(priv)) {
        diag_printf("Firmware Init Failed\n");
        goto err_init_fw;
    }
#if 0
    if (register_netdev(dev)) {
        printk(KERN_ERR "Cannot register network device!\n");
        goto err_init_fw;
    }
#endif

    LEAVE();
    return priv;

  err_init_fw:
    sbi_unregister_dev(priv);

  err_registerdev:
    /* Stop the thread servicing the interrupts */
    //wake_up_interruptible(&priv->MainThread.waitQ);

    cyg_flag_setbits( &priv->MainThread.waitQ_flag_q, 1 ); 
    wlan_terminate_thread(&priv->MainThread);

#ifdef REASSOCIATION
    //wake_up_interruptible(&priv->ReassocThread.waitQ);

    cyg_flag_setbits( &priv->ReassocThread.waitQ_flag_q, 1 ); 
    wlan_terminate_thread(&priv->ReassocThread);
#endif /* REASSOCIATION */
  err_kmalloc:
   // unregister_netdev(dev);
    FREE(priv->adapter, 0);
    //wlanpriv = NULL;

    LEAVE();
    return NULL;
}

/** 
 *  @brief This function removes the card.
 *  
 *  @param priv    A pointer to card
 *  @return 	   WLAN_STATUS_SUCCESS
 */
static int
wlan_remove_card(void *card)
{
    wlan_private *priv = &w99702_priv_data0;
    wlan_adapter *Adapter;
    //struct net_device *dev;
    struct cyg_netdevtab_entry *dev;
	struct eth_drv_sc *sc;
    union iwreq_data wrqu;

    ENTER();

    if (!priv) {
        LEAVE();
        return WLAN_STATUS_SUCCESS;
    }

    Adapter = priv->adapter;

    if (!Adapter) {
        LEAVE();
        return WLAN_STATUS_SUCCESS;
    }

    dev = priv->wlan_dev.netdev;
	sc = (struct eth_drv_sc *)dev->device_instance;
	
   // wake_up_interruptible(&Adapter->ds_awake_q);
   cyg_cond_broadcast(&Adapter->ds_cond_q);

    if (Adapter->CurCmd) {
        diag_printf("Wake up current cmdwait_q\n");
//        wake_up_interruptible(&Adapter->CurCmd->cmdwait_q);
		cyg_flag_setbits( &Adapter->CurCmd->cmdwait_flag_q, 3 );
    }

    Adapter->CurCmd = NULL;

    if (Adapter->PSMode == Wlan802_11PowerModeMAX_PSP) {
        Adapter->PSMode = Wlan802_11PowerModeCAM;
        PSWakeup(priv, HostCmd_OPTION_WAITFORRSP);
    }
    if (Adapter->IsDeepSleep == TRUE) {
        Adapter->IsDeepSleep = FALSE;
        sbi_exit_deep_sleep(priv);
    }

    memset(wrqu.ap_addr.sa_data, 0xaa, ETH_ALEN);
    wrqu.ap_addr.sa_family = ARPHRD_ETHER;
    wireless_send_event(priv->wlan_dev.netdev, SIOCGIWAP, &wrqu, NULL);

    /* Disable interrupts on the card as we cannot handle them after RESET */
    sbi_disable_host_int(priv);

    PrepareAndSendCommand(priv, HostCmd_CMD_802_11_RESET, 0, 0, 0, NULL);

    cyg_thread_delay(20);
    //udelay(200*1000);

#ifdef ENABLE_PM
    pm_unregister(wlan_pm_dev);
#endif

    /* Flush all the packets upto the OS before stopping */
   // wlan_send_rxskbQ(priv);
   eth_drv_dsr(0, 0, (cyg_addrword_t)sc);
    cleanup_txqueues(priv);
   // os_stop_queue(priv);
   // os_carrier_off(priv);

    Adapter->SurpriseRemoved = TRUE;

    /* Stop the thread servicing the interrupts */
    //wake_up_interruptible(&priv->MainThread.waitQ);
    cyg_flag_setbits( &priv->MainThread.waitQ_flag_q, 1 );

#ifdef REASSOCIATION
    //wake_up_interruptible(&priv->ReassocThread.waitQ);
    cyg_flag_setbits( &priv->ReassocThread.waitQ_flag_q, 1 );
#endif /* REASSOCIATION */

#ifdef PROC_DEBUG
    wlan_debug_remove(priv);
#endif
   // wlan_proc_remove(priv);

    diag_printf("unregester dev\n");
    sbi_unregister_dev(priv);

    diag_printf("Free Adapter\n");
    wlan_free_adapter(priv);

    /* Last reference is our one */
//    diag_printf("refcnt = %d\n", atomic_read(&dev->refcnt));

 //   diag_printf("netdev_finish_unregister: %s%s.\n", dev->name,
 //          (dev->features & NETIF_F_DYNALLOC) ? "" : ", old style");

 //   unregister_netdev(dev);

    diag_printf("Unregister finish\n");

    priv->wlan_dev.netdev = NULL;
    //free_netdev(dev);
    //wlanpriv = NULL;

    LEAVE();
    return WLAN_STATUS_SUCCESS;
}

/********************************************************
		Global Functions
********************************************************/
/** 
 *  @brief Cleanup TX queue
 *  @param priv       pointer to wlan_private
 *  @return 	      N/A
*/
void
cleanup_txqueues(wlan_private * wlan_priv)
{
#if 0
    struct sk_buff *delNode, *Q;

    Q = &priv->adapter->TxSkbQ;
    while (!list_empty((struct list_head *) Q)) {
        delNode = Q->next;
        list_del((struct list_head *) delNode);
        kfree_skb(delNode);
    }
    priv->adapter->TxSkbNum = 0;
#else
	w99702_priv_t * priv = wlan_priv->priv;
	wlan_adapter *Adapter = wlan_priv->adapter;
    TXBD *txbd = priv->tx_head;

	while(txbd != priv->tx_tail)
	{
		txbd->mode = 0;
		txbd->SL = 0;
		priv->tx_head = (TXBD *)txbd->next;
		txbd = priv->tx_head;
	}
	Adapter->TxSkbNum = 0;
#endif
}

#if 0
/** 
 *  @brief handle TX Queue
 *  @param priv       pointer to wlan_private
 *  @return 	      N/A
*/
void
wlan_process_txqueue(wlan_private * priv)
{
    wlan_adapter *Adapter = priv->adapter;
    ulong flags;
    struct sk_buff *Q;
    OS_INTERRUPT_SAVE_AREA;
    ENTER();
    //spin_lock_irqsave(&Adapter->CurrentTxLock, flags);
    OS_INT_DISABLE(flags);
    if (Adapter->TxSkbNum > 0) {
        Q = &priv->adapter->TxSkbQ;
        Adapter->CurrentTxSkb = Q->next;
        list_del((struct list_head *) Adapter->CurrentTxSkb);
        Adapter->TxSkbNum--;
    }
    //spin_unlock_irqrestore(&Adapter->CurrentTxLock, flags);
    OS_INT_RESTORE(flags);
    if (Adapter->CurrentTxSkb) {
        wlan_process_tx(priv);
    }

    LEAVE();
}
#endif

#if 0
/**
 * @brief This function sends the rx packets to the os from the skb queue
 *
 * @param priv	A pointer to wlan_private structure
 * @return	n/a
 */
void
wlan_send_rxskbQ(wlan_private * priv)
{
    struct sk_buff *skb;

    ENTER();
    if (priv->adapter) {
        while ((skb = wlan_pop_rx_skb(&priv->adapter->RxSkbQ))) {
            if (ProcessRxedPacket(priv, skb) == -ENOMEM)
                break;
        }
    }
    LEAVE();
}
#endif
/** 
 *  @brief This function finds the CFP in 
 *  region_cfp_table based on region and band parameter.
 *  
 *  @param region  The region code
 *  @param band	   The band
 *  @param cfp_no  A pointer to CFP number
 *  @return 	   A pointer to CFP
 */
CHANNEL_FREQ_POWER *
wlan_get_region_cfp_table(cyg_uint8 region, cyg_uint8 band, int *cfp_no)
{
    int i;

    ENTER();

    for (i = 0; i < sizeof(region_cfp_table) / sizeof(region_cfp_table_t);
         i++) {
        diag_printf("region_cfp_table[i].region=%d\n",
               region_cfp_table[i].region);
        if (region_cfp_table[i].region == region) {
            {
                *cfp_no = region_cfp_table[i].cfp_no_BG;
                LEAVE();
                return region_cfp_table[i].cfp_BG;
            }
        }
    }

    LEAVE();
    return NULL;
}

/** 
 *  @brief This function sets region table. 
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @param region  The region code
 *  @param band	   The band
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
wlan_set_regiontable(wlan_private * priv, cyg_uint8 region, cyg_uint8 band)
{
    wlan_adapter *Adapter = priv->adapter;
    int i = 0;

    CHANNEL_FREQ_POWER *cfp;
    int cfp_no;

    ENTER();

    memset(Adapter->region_channel, 0, sizeof(Adapter->region_channel));

    {
        cfp = wlan_get_region_cfp_table(region, band, &cfp_no);
        if (cfp != NULL) {
            Adapter->region_channel[i].NrCFP = cfp_no;
            Adapter->region_channel[i].CFP = cfp;
        } else {
            diag_printf("wrong region code %#x in Band B-G\n", region);
            return WLAN_STATUS_FAILURE;
        }
        Adapter->region_channel[i].Valid = TRUE;
        Adapter->region_channel[i].Region = region;
        Adapter->region_channel[i].Band = band;
        i++;
    }
    LEAVE();
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function handles the interrupt. it will change PS
 *  state if applicable. it will wake up main_thread to handle
 *  the interrupt event as well.
 *  
 *  @param dev     A pointer to net_device structure
 *  @return 	   n/a
 */
void
wlan_interrupt(struct eth_drv_sc *sc)
{
    wlan_private *priv = (wlan_private *)sc->driver_private;

    ENTER();

    //diag_printf("wlan_interrupt: IntCounter=%d\n", priv->adapter->IntCounter);

    priv->adapter->IntCounter++;

     if (priv->adapter->HS_Activated && !priv->adapter->WakeupTries && //JONO
        priv->adapter->fwWakeupMethod == WAKEUP_FW_THRU_INTERFACE) {
        diag_printf("SDIO interrupt received while HS_Activated\n");
        wlan_host_sleep_gpio_int_event(priv);
    } //JONO
    priv->adapter->WakeupTries = 0;

    if (priv->adapter->PSState == PS_STATE_SLEEP) {
        priv->adapter->PSState = PS_STATE_AWAKE;
    }

    if (priv->adapter->IsDeepSleep == TRUE) {
        priv->adapter->IsDeepSleep = FALSE;
        priv->wlan_dev.dnld_sent = DNLD_RES_RECEIVED;
        priv->adapter->HisRegCpy |= HIS_TxDnLdRdy;
        diag_printf("Interrupt received in DEEP SLEEP mode!\n");
        //os_carrier_on(priv);
        //os_start_queue(priv);
    }
	
    //wake_up_interruptible(&priv->MainThread.waitQ);
  
    cyg_flag_setbits( &priv->MainThread.waitQ_flag_q, 1 ); 

    LEAVE();
}

/** 
 *  @brief This function initializes module.
 *  
 *  @param	   n/a    A pointer to wlan_private structure
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
wlan_init_module(void)
{
    int ret = WLAN_STATUS_SUCCESS;

    ENTER();

    if (sbi_register(wlan_add_card, wlan_remove_card, NULL) == NULL) {
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

  done:
    LEAVE();
    return ret;
}

/** 
 *  @brief This function cleans module
 *  
 *  @param priv    n/a
 *  @return 	   n/a
 */
void
wlan_cleanup_module(void)
{
    ENTER();

    sbi_unregister();

    LEAVE();
}

//module_init(wlan_init_module);
//module_exit(wlan_cleanup_module);

//MODULE_DESCRIPTION("M-WLAN Driver");
//MODULE_AUTHOR("Marvell International Ltd.");

/* add by HPChen */
//sdio_ctrller fmi_sdio_ctr, *fmi_ctrller;
//wlan_private fmi_wlan_priv, *fmi_priv;
int sdio_init_module(cyg_uint8 io_func)
{
#if 0
	wlan_adapter *Adapter = fmi_priv->adapter;

	fmi_ctrller = &fmi_sdio_ctr;
	memset(fmi_ctrller, 0, sizeof(sdio_ctrller));
	
	fmi_priv = &fmi_wlan_priv;
	if (!((int)fmi_priv->adapter = malloc(sizeof(wlan_adapter)))) {
		diag_printf("Allocate Adapter memory Fail!!\n");
		return -1;
	}
	memset(fmi_priv->adapter, 0, sizeof(wlan_adapter));

	fmi_ctrller->card_capability.num_of_io_funcs = io_func;

	fmi_priv->wlan_dev.card = fmi_ctrller->card;
	fmi_ctrller->card->ctrlr = fmi_ctrller;

#if 1
	fmi_priv = (wlan_private *)wlan_add_card(fmi_ctrller->card);

#else
	get_cisptr_address(fmi_ctrller);
	read_manfid(fmi_ctrller, FN0);

	sbi_probe_card(fmi_ctrller);

	INIT_LIST_HEAD(&fmi_priv->adapter->CmdFreeQ);
	INIT_LIST_HEAD(&fmi_priv->adapter->CmdPendingQ);

	sbi_register_dev(fmi_priv);

	wlan_init_fw(fmi_priv);
#endif

#endif
	return 0;
}



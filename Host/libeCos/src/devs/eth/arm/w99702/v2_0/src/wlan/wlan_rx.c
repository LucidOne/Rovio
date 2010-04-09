/** @file wlan_rx.c
  * @brief This file contains the handling of RX in wlan
  * driver.
  * 
  *  Copyright ?Marvell International Ltd. and/or its affiliates, 2003-2006
  */
/********************************************************
Change log:
	09/28/05: Add Doxygen format comments
	12/09/05: ADD Sliding window SNR/NF Average Calculation support
	
********************************************************/

#include	"include.h"

/********************************************************
		Local Variables
********************************************************/

typedef __packed struct
{
    cyg_uint8 dest_addr[6];
    cyg_uint8 src_addr[6];
    cyg_uint16 h803_len;

} Eth803Hdr_t;

typedef __packed struct
{
    cyg_uint8 llc_dsap;
    cyg_uint8 llc_ssap;
    cyg_uint8 llc_ctrl;
    cyg_uint8 snap_oui[3];
    cyg_uint16 snap_type;

} Rfc1042Hdr_t;

typedef __packed struct
{
    RxPD rx_pd;
    Eth803Hdr_t eth803_hdr;
    Rfc1042Hdr_t rfc1042_hdr;

} RxPacketHdr_t;

typedef __packed struct
{
    cyg_uint8 dest_addr[6];
    cyg_uint8 src_addr[6];
    cyg_uint16 ethertype;

} EthII_Hdr_t;

/********************************************************
		Global Variables
********************************************************/

/********************************************************
		Local Functions
********************************************************/

/** 
 *  @brief This function computes the AvgSNR .
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   AvgSNR
 */
static cyg_uint8
wlan_getAvgSNR(wlan_private * priv)
{
    cyg_uint8 i;
    cyg_uint16 temp = 0;
    wlan_adapter *Adapter = priv->adapter;
    if (Adapter->numSNRNF == 0)
        return 0;
    for (i = 0; i < Adapter->numSNRNF; i++)
        temp += Adapter->rawSNR[i];
    return (cyg_uint8) (temp / Adapter->numSNRNF);

}

/** 
 *  @brief This function computes the AvgNF
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   AvgNF
 */
static cyg_uint8
wlan_getAvgNF(wlan_private * priv)
{
    cyg_uint8 i;
    cyg_uint16 temp = 0;
    wlan_adapter *Adapter = priv->adapter;
    if (Adapter->numSNRNF == 0)
        return 0;
    for (i = 0; i < Adapter->numSNRNF; i++)
        temp += Adapter->rawNF[i];
    return (cyg_uint8) (temp / Adapter->numSNRNF);

}

/** 
 *  @brief This function save the raw SNR/NF to our internel buffer
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @param pRxPD   A pointer to RxPD structure of received packet
 *  @return 	   n/a
 */
static void
wlan_save_rawSNRNF(wlan_private * priv, RxPD * pRxPD)
{
    wlan_adapter *Adapter = priv->adapter;
    if (Adapter->numSNRNF < Adapter->data_avg_factor)
        Adapter->numSNRNF++;
    Adapter->rawSNR[Adapter->nextSNRNF] = pRxPD->SNR;
    Adapter->rawNF[Adapter->nextSNRNF] = pRxPD->NF;
    Adapter->nextSNRNF++;
    if (Adapter->nextSNRNF >= Adapter->data_avg_factor)
        Adapter->nextSNRNF = 0;
    return;
}

/** 
 *  @brief This function computes the RSSI in received packet.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @param pRxPD   A pointer to RxPD structure of received packet
 *  @return 	   n/a
 */
static void
wlan_compute_rssi(wlan_private * priv, RxPD * pRxPD)
{
    wlan_adapter *Adapter = priv->adapter;

    ENTER();

#if 0
    diag_printf("RxPD: SNR = %d, NF = %d\n", pRxPD->SNR, pRxPD->NF);
    diag_printf("Before computing SNR: SNR- avg = %d, NF-avg = %d\n",
           Adapter->SNR[TYPE_RXPD][TYPE_AVG] / AVG_SCALE,
           Adapter->NF[TYPE_RXPD][TYPE_AVG] / AVG_SCALE);
#endif
    Adapter->SNR[TYPE_RXPD][TYPE_NOAVG] = pRxPD->SNR;
    Adapter->NF[TYPE_RXPD][TYPE_NOAVG] = pRxPD->NF;
    wlan_save_rawSNRNF(priv, pRxPD);

    Adapter->RxPDAge = cyg_current_time();//os_time_get();
    Adapter->RxPDRate = pRxPD->RxRate;

    Adapter->SNR[TYPE_RXPD][TYPE_AVG] = wlan_getAvgSNR(priv) * AVG_SCALE;
    Adapter->NF[TYPE_RXPD][TYPE_AVG] = wlan_getAvgNF(priv) * AVG_SCALE;
#if 0
    diag_printf("After computing SNR: SNR-avg = %d, NF-avg = %d\n",
           Adapter->SNR[TYPE_RXPD][TYPE_AVG] / AVG_SCALE,
           Adapter->NF[TYPE_RXPD][TYPE_AVG] / AVG_SCALE);
#endif
    Adapter->RSSI[TYPE_RXPD][TYPE_NOAVG] =
        CAL_RSSI(Adapter->SNR[TYPE_RXPD][TYPE_NOAVG],
                 Adapter->NF[TYPE_RXPD][TYPE_NOAVG]);

    Adapter->RSSI[TYPE_RXPD][TYPE_AVG] =
        CAL_RSSI(Adapter->SNR[TYPE_RXPD][TYPE_AVG] / AVG_SCALE,
                 Adapter->NF[TYPE_RXPD][TYPE_AVG] / AVG_SCALE);

    LEAVE();
}

/********************************************************
		Global functions
********************************************************/

/**
 *  @brief This function processes received packet and forwards it
 *  to kernel/upper layer
 *  
 *  @param priv    A pointer to wlan_private
 *  @param skb     A pointer to skb which includes the received packet
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
ProcessRxedPacket(wlan_private * priv, char *buffer, int *length)
{
    int ret = WLAN_STATUS_SUCCESS;

    RxPacketHdr_t *pRxPkt;
    RxPD *pRxPD;

    int hdrChop;
    EthII_Hdr_t *pEthHdr;

    const cyg_uint8 rfc1042_eth_hdr[] = { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00 };//SNAP header

    ENTER();

    pRxPkt = (RxPacketHdr_t *) buffer;
    pRxPD = &pRxPkt->rx_pd;

    HEXDUMP("RX Data: Before chop RxPD", buffer, MIN(*length, 100));

    if (*length < (ETH_HLEN + 8 + sizeof(RxPD))) {
        diag_printf("RX Error: FRAME RECEIVED WITH BAD LENGTH\n");
        priv->stats.rx_short_frames++;
        ret = WLAN_STATUS_SUCCESS;
        goto done;
    }

//    diag_printf("RX Data: skb->len - sizeof(RxPd) = %d - %d = %d\n",
//           *length, sizeof(RxPD), *length - sizeof(RxPD));

    HEXDUMP("RX Data: Dest", pRxPkt->eth803_hdr.dest_addr,
            sizeof(pRxPkt->eth803_hdr.dest_addr));
    HEXDUMP("RX Data: Src", pRxPkt->eth803_hdr.src_addr,
            sizeof(pRxPkt->eth803_hdr.src_addr));

    if (memcmp((void*)&pRxPkt->rfc1042_hdr,
               rfc1042_eth_hdr, sizeof(rfc1042_eth_hdr)) == 0) {
        /* 
         *  Replace the 803 header and rfc1042 header (llc/snap) with an 
         *    EthernetII header, keep the src/dst and snap_type (ethertype)
         *
         *  The firmware only passes up SNAP frames converting
         *    all RX Data from 802.11 to 802.2/LLC/SNAP frames.
         *
         *  To create the Ethernet II, just move the src, dst address right
         *    before the snap_type.
         */
        pEthHdr = (EthII_Hdr_t *)
            ((cyg_uint8 *) & pRxPkt->eth803_hdr
             + sizeof(pRxPkt->eth803_hdr) + sizeof(pRxPkt->rfc1042_hdr)
             - sizeof(pRxPkt->eth803_hdr.dest_addr)
             - sizeof(pRxPkt->eth803_hdr.src_addr)
             - sizeof(pRxPkt->rfc1042_hdr.snap_type));

        memcpy((void*)pEthHdr->src_addr, (void*)pRxPkt->eth803_hdr.src_addr,
               sizeof(pEthHdr->src_addr));
        memcpy((void*)pEthHdr->dest_addr, (void*)pRxPkt->eth803_hdr.dest_addr,
               sizeof(pEthHdr->dest_addr));

        /* Chop off the RxPD + the excess memory from the 802.2/llc/snap header
         *   that was removed 
         */
        hdrChop = (cyg_uint8 *) pEthHdr - (cyg_uint8 *) pRxPkt;
    } else {
        HEXDUMP("RX Data: LLC/SNAP",
                (cyg_uint8 *) & pRxPkt->rfc1042_hdr, sizeof(pRxPkt->rfc1042_hdr));

        /* Chop off the RxPD */
        hdrChop = (cyg_uint8 *) & pRxPkt->eth803_hdr - (cyg_uint8 *) pRxPkt;
    }

    /* Chop off the leading header bytes so the skb points to the start of 
     *   either the reconstructed EthII frame or the 802.2/llc/snap frame
     */
    //skb_pull(skb, hdrChop);
    *length -= hdrChop;
//    buffer += hdrChop;

    wlan_compute_rssi(priv, pRxPD);

#if 0
//    diag_printf("RX Data: Size of actual packet = %d\n", skb->len);
    if (os_upload_rx_packet(priv, skb)) {
        diag_printf("RX Error: os_upload_rx_packet" " returns failure\n");
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }
 #endif
    priv->stats.rx_count += *length;
    priv->stats.rx_resource++;

    ret = WLAN_STATUS_SUCCESS;
  done:
    LEAVE();

    return (ret);
}

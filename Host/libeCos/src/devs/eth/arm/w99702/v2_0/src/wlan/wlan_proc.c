/** @file wlan_proc.c
  * @brief This file contains functions for proc fin proc file.
  * 
  *  Copyright © Marvell International Ltd. and/or its affiliates, 2003-2006
  */
/********************************************************
Change log:
	10/04/05: Add Doxygen format comments
	01/05/06: Add kernel 2.6.x support	
	
********************************************************/
#include 	"include.h"

/********************************************************
		Local Variables
********************************************************/

static char *szModes[] = {
    "Ad-hoc",
    "Managed",
    "Auto",
    "Unknown"
};

static char *szStates[] = {
    "Connected",
    "Disconnected"
};

/********************************************************
		Global Variables
********************************************************/

/********************************************************
		Local Functions
********************************************************/

/** 
 *  @brief proc read function
 *
 *  @param page	   pointer to buffer
 *  @param start   read data starting position
 *  @param offset  offset
 *  @param count   counter 
 *  @param eof     end of file flag
 *  @param data    data to output
 *  @return 	   number of output data
 */
static int
wlan_proc_read(char *page, char **start, off_t offset,
               int count, int *eof, void *data)
{
#ifdef CONFIG_PROC_FS
    int i;
    char *p = page;
    struct net_device *netdev = data;
    struct dev_mc_list *mcptr = netdev->mc_list;
    char fmt[64];
    wlan_private *priv = netdev->priv;
    wlan_adapter *Adapter = priv->adapter;
#endif

    if (offset != 0) {
        *eof = 1;
        goto exit;
    }

    get_version(Adapter, fmt, sizeof(fmt) - 1);

    p += sprintf(p, "driver_name = " "\"wlan\"\n");
    p += sprintf(p, "driver_version = %s", fmt);
    p += sprintf(p, "\nInterfaceName=\"%s\"\n", netdev->name);
    p += sprintf(p, "Mode=\"%s\"\n", szModes[Adapter->InfrastructureMode]);
    p += sprintf(p, "State=\"%s\"\n", szStates[Adapter->MediaConnectStatus]);
    p += sprintf(p, "MACAddress=\"%02x:%02x:%02x:%02x:%02x:%02x\"\n",
                 netdev->dev_addr[0], netdev->dev_addr[1],
                 netdev->dev_addr[2], netdev->dev_addr[3],
                 netdev->dev_addr[4], netdev->dev_addr[5]);

    p += sprintf(p, "MCCount=\"%d\"\n", netdev->mc_count);
    p += sprintf(p, "ESSID=\"%s\"\n", (cyg_uint8 *) Adapter->CurBssParams.ssid.Ssid);
    p += sprintf(p, "Channel=\"%d\"\n", Adapter->CurBssParams.channel);
    p += sprintf(p, "region_code = \"%02x\"\n", (cyg_uint32) Adapter->RegionCode);

    /*
     * Put out the multicast list 
     */
    for (i = 0; i < netdev->mc_count; i++) {
        p += sprintf(p,
                     "MCAddr[%d]=\"%02x:%02x:%02x:%02x:%02x:%02x\"\n",
                     i,
                     mcptr->dmi_addr[0], mcptr->dmi_addr[1],
                     mcptr->dmi_addr[2], mcptr->dmi_addr[3],
                     mcptr->dmi_addr[4], mcptr->dmi_addr[5]);

        mcptr = mcptr->next;
    }
    p += sprintf(p, "num_tx_bytes_transmited = \"%lu\"\n",
                 priv->stats.tx_bytes);
    p += sprintf(p, "num_rx_bytes_recieved = \"%lu\"\n",
                 priv->stats.rx_bytes);
    p += sprintf(p, "num_tx_packets_transmited = \"%lu\"\n",
                 priv->stats.tx_packets);
    p += sprintf(p, "num_rx_packets_received = \"%lu\"\n",
                 priv->stats.rx_packets);
    p += sprintf(p, "num_tx_packets_dropped = \"%lu\"\n",
                 priv->stats.tx_dropped);
    p += sprintf(p, "num_rx_packets_dropped = \"%lu\"\n",
                 priv->stats.rx_dropped);
    p += sprintf(p, "num_tx_errors= \"%lu\"\n", priv->stats.tx_errors);
    p += sprintf(p, "num_rx_errors= \"%lu\"\n", priv->stats.rx_errors);

  exit:
    return (p - page);
}

/********************************************************
		Global Functions
********************************************************/

/** 
 *  @brief create wlan proc file
 *
 *  @param priv	   pointer wlan_private
 *  @param dev     pointer net_device
 *  @return 	   N/A
 */
void
wlan_proc_entry(wlan_private * priv, struct net_device *dev)
{

#ifdef	CONFIG_PROC_FS
    diag_printf("Creating Proc Interface\n");

    if (!priv->proc_entry) {
        priv->proc_entry = proc_mkdir("wlan", proc_net);

        if (priv->proc_entry) {
            priv->proc_dev = create_proc_read_entry
                ("info", 0, priv->proc_entry, wlan_proc_read, dev);
        }
    }
#endif
}

/** 
 *  @brief remove proc file
 *
 *  @param priv	   pointer wlan_private
 *  @return 	   N/A
 */
void
wlan_proc_remove(wlan_private * priv)
{
#ifdef CONFIG_PROC_FS
    if (priv->proc_entry) {
        remove_proc_entry("info", priv->proc_entry);
    }
    remove_proc_entry("wlan", proc_net);
#endif
}

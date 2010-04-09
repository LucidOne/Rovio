/** @file wlan_decl.h
 *  @brief This file contains declaration referring to
 *  functions defined in other source files
 *
 *  Copyright © Marvell International Ltd. and/or its affiliates, 2003-2006
 */
/******************************************************
Change log:
	09/29/05: add Doxygen format comments
	01/05/06: Add kernel 2.6.x support	
	01/11/06: Change compile flag BULVERDE_SDIO to SD to support
	          Monahans/Zylonite
	01/11/06: Conditionalize new scan/join structures.
	          Move wlan_wext statics to their source file.
******************************************************/

#ifndef _WLAN_DECL_H_
#define _WLAN_DECL_H_

#include "include.h"

/** Function Prototype Declaration */
//int wlan_tx_packet(wlan_private * priv, struct sk_buff *skb);
int wlan_tx_packet(wlan_private * priv, struct eth_drv_sg *sg_list,
                               int sg_len,
                               int total_len);
void wlan_free_adapter(wlan_private * priv);
int SetMacPacketFilter(wlan_private * priv);

int SendNullPacket(wlan_private * priv, cyg_uint8 pwr_mgmt);
BOOLEAN CheckLastPacketIndication(wlan_private * priv);

void Wep_encrypt(wlan_private * priv, cyg_uint8 * Buf, cyg_uint32 Len);
int FreeCmdBuffer(wlan_private * priv);
void CleanUpCmdCtrlNode(CmdCtrlNode * pTempNode);
CmdCtrlNode *GetFreeCmdCtrlNode(wlan_private * priv);

void SetCmdCtrlNode(wlan_private * priv,
                    CmdCtrlNode * pTempNode,
                    WLAN_OID cmd_oid, cyg_uint16 wait_option, void *pdata_buf);

BOOLEAN Is_Command_Allowed(wlan_private * priv);

int PrepareAndSendCommand(wlan_private * priv,
                          cyg_uint16 cmd_no,
                          cyg_uint16 cmd_action,
                          cyg_uint16 wait_option, WLAN_OID cmd_oid, void *pdata_buf);

void QueueCmd(wlan_adapter * Adapter, CmdCtrlNode * CmdNode, BOOLEAN addtail);

int SetDeepSleep(wlan_private * priv, BOOLEAN bDeepSleep);
int AllocateCmdBuffer(wlan_private * priv);
int ExecuteNextCommand(wlan_private * priv);
int wlan_process_event(wlan_private * priv);
//void wlan_interrupt(struct net_device *);
void wlan_interrupt(struct eth_drv_sc *sc);
int SetRadioControl(wlan_private * priv);
cyg_uint32 index_to_data_rate(cyg_uint8 index);
cyg_uint8 data_rate_to_index(cyg_uint32 rate);
void HexDump(char *prompt, cyg_uint8 * data, int len);
void get_version(wlan_adapter * adapter, char *version, int maxlen);
void wlan_read_write_rfreg(wlan_private * priv);

/** The proc fs interface */
void wlan_proc_entry(wlan_private * priv, struct net_device *dev);
void wlan_proc_remove(wlan_private * priv);
void wlan_debug_entry(wlan_private * priv, struct net_device *dev);
void wlan_debug_remove(wlan_private * priv);
int wlan_process_rx_command(wlan_private * priv);
//void wlan_process_tx(wlan_private * priv);
void wlan_process_tx(wlan_private * priv, struct eth_drv_sg *sg_list,
                               int sg_len,
                               int total_len);
void CleanupAndInsertCmd(wlan_private * priv, CmdCtrlNode * pTempCmd);
void MrvDrvCommandTimerFunction(cyg_handle_t alarm, cyg_addrword_t data);

#ifdef REASSOCIATION
void MrvDrvTimerFunction(cyg_handle_t alarm, cyg_addrword_t data);
#endif /* REASSOCIATION */

#if 0
int wlan_set_essid(struct net_device *dev, struct iw_request_info *info,
                   struct iw_point *dwrq, char *extra);
#else
int
wlan_set_essid(struct eth_drv_sc *sc, struct iw_request_info *info,
               struct iw_point *dwrq, char *extra);
#endif

int wlan_set_regiontable(wlan_private * priv, cyg_uint8 region, cyg_uint8 band);

int wlan_host_sleep_activated_event(wlan_private * priv);
int wlan_deep_sleep_ioctl(wlan_private * priv, struct ifreq *rq);

//int ProcessRxedPacket(wlan_private * priv, struct sk_buff *);
int ProcessRxedPacket(wlan_private * priv, char *buffer, int *length);

void PSSleep(wlan_private * priv, int wait_option);
void PSConfirmSleep(wlan_private * priv, cyg_uint16 PSMode);
void PSWakeup(wlan_private * priv, int wait_option);
//void sdio_clear_imask(mmc_controller_t ctrller);
//int sdio_check_idle_state(mmc_controller_t ctrller);
//void sdio_print_imask(mmc_controller_t ctrller);
//void sdio_clear_imask(mmc_controller_t ctrller);

void wlan_send_rxskbQ(wlan_private * priv);

extern CHANNEL_FREQ_POWER *find_cfp_by_band_and_channel(wlan_adapter *
                                                        adapter, cyg_uint8 band,
                                                        cyg_uint16 channel);

extern void MacEventDisconnected(wlan_private *priv);

#if WIRELESS_EXT > 14
void send_iwevcustom_event(wlan_private * priv, cyg_int8 * str);
#endif

void cleanup_txqueues(wlan_private * priv);
void wlan_process_txqueue(wlan_private * priv);

int fw_read(char *name, cyg_uint8 ** addr, cyg_uint32 * len);
void fw_buffer_free(cyg_uint8 * addr);

#endif /* _WLAN_DECL_H_ */

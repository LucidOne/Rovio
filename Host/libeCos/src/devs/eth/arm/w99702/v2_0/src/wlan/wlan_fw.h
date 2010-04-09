/** @file wlan_fw.h
 *  @brief This header file contains FW interface related definitions.
 *
 *  Copyright © Marvell International Ltd. and/or its affiliates, 2003-2006
 */
/*************************************************************
Change log:
	09/26/05: add Doxygen format comments 
 ************************************************************/

#ifndef _WLAN_FW_H_
#define _WLAN_FW_H_

#ifndef DEV_NAME_LEN
#define DEV_NAME_LEN            32
#endif

#define MAXKEYLEN           13

/* The number of times to try when waiting for downloaded firmware to 
 become active. (polling the scratch register). */

#define MAX_FIRMWARE_POLL_TRIES     100

#define FIRMWARE_TRANSFER_BLOCK_SIZE    1536

/** function prototypes */
int wlan_init_fw(wlan_private * priv);
int wlan_disable_host_int(wlan_private * priv, cyg_uint8 reg);
int wlan_enable_host_int(wlan_private * priv, cyg_uint8 mask);
int wlan_free_cmd_buffers(wlan_private * priv);

#endif /* _WLAN_FW_H_ */

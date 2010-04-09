/** @file include.h
 * 
 * @brief This file contains all the necessary include file.
 *  
 *  Copyright © Marvell International Ltd. and/or its affiliates, 2003-2006
 */
/********************************************************
Change log:
	10/11/05: Add Doxygen format comments
	01/11/06: Change compile flag BULVERDE_SDIO to SD to support
	          Monahans/Zylonite
	01/11/06: Conditional include file removal/addition
	01/30/06: Add kernel 2.6 support on GSPI8xxx/Bulverde
	
********************************************************/

#ifndef _INCLUDE_H_
#define _INCLUDE_H_

#include "pkgconf/system.h"
#include "cyg/error/codes.h"
#include "cyg/infra/cyg_type.h"  // Common type definitions and support
                                 // including endian-ness
#include "cyg/infra/diag.h"
#include "cyg/io/eth/netdev.h"
//#include "cyg/io/eth/eth_drv.h"
//#include "cyg/io/eth/eth_drv_stats.h"
#include "cyg/hal/hal_intr.h"
#include "net/wireless.h"
#include "net/iw_handler.h"
//#include    "os_headers.h"
#include    "wlan_defs.h"
#include    "wlan_thread.h"
#include    "wlan_types.h"

//#include    "wlan_wmm.h"
#include    "wlan_11d.h"

//#include    "os_timers.h"

#include    "host.h"
#include    "hostcmd.h"

#include    "wlan_scan.h"
#include    "wlan_join.h"

#include    "wlan_dev.h"
//#include    "os_macros.h"
#include    "sbi.h"

#include    "sdio.h"

#include    "wlan_fw.h"
#include    "wlan_wext.h"
#include    "wlan_decl.h"
#include	"w702sdio.h"


extern cyg_handle_t cmd_alarm_handle;
extern cyg_handle_t reassociation_alarm_handle;
extern cyg_handle_t thread_stop_ptr[2];

#endif /* _INCLUDE_H_ */

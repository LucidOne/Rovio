/** @file wlan_version.h
  * @brief This file contains wlan driver version number.
  * 
  *  Copyright © Marvell International Ltd. and/or its affiliates, 2003-2006
  */
/********************************************************
Change log:
	10/04/05: Add Doxygen format comments
	
********************************************************/
#include "../release_version.h"

#define FPNUM	"4"

const char driver_version[] =
    "sd8686-%s-" DRIVER_RELEASE_VERSION "-(" "FP" FPNUM ")"
#ifdef	DEBUG_LEVEL2
    "-dbg"
#endif
    " ";

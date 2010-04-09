/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      SETROP.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This module is used to set the ROP code of BitBlt.
 * 
 *  HISTORY
 *      2004/09/13  Created by PC50 Walter Tseng
 *
 *  REMARK
 *      None
 *
 ******************************************************************************/
#include "gfxlib.h"
#include "global.h"

#include "w99702_reg.h"


///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION
//      gfxSetROP()
//
//  DESCRIPTION
//      This function is used set the ROP code of BitBlt.
//
//  INPUTS
//      rop_code:   ROP code
//
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error
// 
///////////////////////////////////////////////////////////////////////////////
INT gfxSetROP(UINT8 rop_code)
{
    _gfx_ucROP = rop_code;
    
    return 0;   
} 




/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      SETCKEY.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This module is used to set the color key of BitBlt.
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
//      gfxSetColorKey()
//
//  DESCRIPTION
//      This function is used set the color key for BitBlt.
//
//  INPUTS
//      enable:     enable/disable color key
//      key_color:  color key data
//      key_mask:   color key mask
//
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error
// 
///////////////////////////////////////////////////////////////////////////////
INT gfxSetColorKey(BOOL enable, GFX_ENTRY key_color, GFX_ENTRY key_mask)
{
    if (! enable)
    {
        _gfx_bColorKeyEnabled = FALSE;
        
        return 0;
    }
    
    _gfx_uColorKey      = (UINT32)key_color;
    _gfx_uColorKeyMask  = (UINT32)key_mask;
    
    _gfx_bColorKeyEnabled = TRUE;
    
    return 0;   
} 




/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      SETALPHA.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This module is used to set the alpha control of BitBlt.
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


///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION
//      gfxSetAlpha()
//
//  DESCRIPTION
//      This function is used set the alpha control of BitBlt. 
//      The alpha only allows in SRCCOPY in current implementation.
//
//  INPUTS
//      enable:     enable/disable alpha control
//      alpha:      alpha [1..255]
//                  0:      keep destination result (nothing to do)
//                  256:    keep source result (same as disable)
//
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error
// 
///////////////////////////////////////////////////////////////////////////////
INT gfxSetAlpha(BOOL enable, INT alpha)
{
    if (! enable)
    {
        _gfx_bAlphaEnabled = FALSE;
        
        return 0;
    }
    
    if ((alpha < 0) || (alpha > 256)) return ERR_GFX_INVALID_ALPHA;
    
    if (alpha == 256) _gfx_bAlphaEnabled = FALSE;
    
    _gfx_uAlphaKs = (UINT32)alpha;
    
    _gfx_bAlphaEnabled = TRUE;
    
    return 0;   
} 




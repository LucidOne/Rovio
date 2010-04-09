/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      SETWMASK.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This module is used to set write mask control of drawing functions.
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

#include "wbio.h"
#include "w99702_reg.h"


///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION
//      gfxSetWriteMask()
//
//  DESCRIPTION
//      This function is used set the write mask control of drawing functions.
//
//  INPUTS
//      color_mask:     output color mask
//
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error
// 
///////////////////////////////////////////////////////////////////////////////
INT gfxSetWriteMask(GFX_MASK color_mask)
{
    gfxWaitEngineReady();

    _gfx_uWriteMask = (UINT32)color_mask;

    outpw(REG_GE_WRITE_MASK, _gfx_uWriteMask);
        
    return 0;   
} 




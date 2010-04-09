/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      SETDMODE.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This module is used to set the drawing mode.
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
//      gfxSetDrawMode()
//
//  DESCRIPTION
//      This function is used set the drawing mode. 
//
//  INPUTS
//      draw_mode:      drawing mode
//
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error
// 
///////////////////////////////////////////////////////////////////////////////
INT gfxSetDrawMode(GFX_DRAW_MODE_E draw_mode)
{
    _gfx_nDrawMode = draw_mode;
    
    return 0;   
} 




/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      SETPEN.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This module is used to set the pen (line) style for drawing lines.
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
//      gfxSetPen()
//
//  DESCRIPTION
//      This function is used set the pen (line) style for drawing lines.
//
//  INPUTS
//      style:      pen style represented by a 16-bit data
//      fore_color: foreground color of the pen
//      back_color: background color of the pen
//
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error
// 
///////////////////////////////////////////////////////////////////////////////
INT gfxSetPen(UINT16 style, GFX_ENTRY fore_color, GFX_ENTRY back_color)
{
    _gfx_usPenStyle = style;
    _gfx_uPenForeColor = fore_color;
    _gfx_uPenBackColor = back_color;
    
    _gfx_bPenInitialized = TRUE;
    
    return 0;   
} 




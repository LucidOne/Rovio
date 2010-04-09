/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      MAKECOLOR.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This is module is used to convert a logical color to physical color
 *      for painting.       
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
//      gfxMakeColor()
//
//  DESCRIPTION
//      This function is used to convert a logical color to physical 
//      color according to the color format of the drawing environment. 
//
//  INPUTS
//
//  OUTPUTS
//
//  RETURN
//      0:      Success
//      others: Error  
// 
///////////////////////////////////////////////////////////////////////////////
GFX_ENTRY gfxMakeColor(GFX_COLOR color)
{
    GFX_ENTRY r, g, b;
    
    if (! _gfx_bInitialized) return color; 
    
    switch (_gfx_nColorFormat)
    {
        case GFX_BPP_332:
            r = (color & 0x00e00000) >> 16; // 3 bits
            g = (color & 0x0000e000) >> 11; // 3 bits
            b = (color & 0x000000c0) >> 6;  // 2 bits
            break;
        case GFX_BPP_444H:
            break;    
        case GFX_BPP_444L:
            break;
        case GFX_BPP_565:
            r = (color & 0x00f80000) >> 8;  // 5 bits
            g = (color & 0x0000fc00) >> 5;  // 6 bits
            b = (color & 0x000000f8) >> 3;  // 5 bits
            break;
        case GFX_BPP_666:
            break;
        case GFX_BPP_888:
            r = (color & 0x00ff0000);   // 8 bits
            g = (color & 0x0000ff00);   // 8 bits
            b = (color & 0x000000ff);   // 8 bits
            break;               
    }

    return (r | g | b);
}

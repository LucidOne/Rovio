/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      CLIP.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This module is the clipper of the drawing functions. 
 * 
 *  HISTORY
 *      2004/09/13  Created by PC50 Walter Tseng
 *
 *  REMARK
 *      Current library only handles the inside clipping. 
 *                                                             
 ******************************************************************************/
#include "gfxlib.h"
#include "global.h"

#include "w99702_reg.h"


///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION
//      gfxSetClip()
//
//  DESCRIPTION
//      This function is used set the clipping rectangle of the 
//      drawing functions. 
//
//  INPUTS
//      enable:     enable/disable clipper
//      rect:       clipping rectangle
//
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error
// 
///////////////////////////////////////////////////////////////////////////////
INT gfxSetClip(BOOL enable, GFX_RECT_T rect)
{
    INT x1, x2, y1, y2;
    
    if (! enable)
    {
        _gfx_bClipEnabled = FALSE;
        
        return 0;
    }
    
    x1 = rect.fC.nLeft;
    y1 = rect.fC.nTop;
    x2 = rect.fC.nRight;
    y2 = rect.fC.nBottom;
    
    if ((x1 < 0) || (y1 < 0) || (x2 < 0) || (y2 < 0)) return ERR_GFX_INVALID_RECT;
    
    if ((x1 > x2) || (y1 > y2)) return ERR_GFX_INVALID_RECT;
    
    if (x2 >= _gfx_nDestWidth) return ERR_GFX_INVALID_RECT;
    
    if (y2 >= _gfx_nDestHeight) return ERR_GFX_INVALID_RECT; 

    _gfx_ClipRect.fC.nLeft = rect.fC.nLeft;
    _gfx_ClipRect.fC.nTop = rect.fC.nTop;
    _gfx_ClipRect.fC.nRight = rect.fC.nRight;
    _gfx_ClipRect.fC.nBottom = rect.fC.nBottom;
    
    /* hardware clipper not includes last pixel */
    
    x2++;
    y2++;
    _gfx_uClipTL = (UINT32)((y1 << 16) | x1);
    _gfx_uClipBR = (UINT32)((y2 << 16) | x2);
    
    _gfx_bClipEnabled = TRUE;
    
    return 0;   
} 




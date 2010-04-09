/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      TEXTBLT.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This module is used to handle the color expansion BLT operation.  
 * 
 *  HISTORY
 *      2004/09/13  Created by PC50 Walter Tseng
 *      2004/09/20  Support clipping rectangle   
 *
 *  REMARK
 *      None
 *                                                             
 ******************************************************************************/
#ifdef ECOS
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "drv_api.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "wbio.h"
#include "gfxlib.h"
#include "global.h"

#include "w99702_reg.h"


///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION
//      gfxColorExpansionBlt()
//
//  DESCRIPTION
//      This function is used to do the color expansion BLT.
//
//  INPUTS
//      rect:       destination rectangle
//      fore_color: foreground color
//      back_color: backround color
//      bit_buf:    pointer to the mono source for color expansion
//      
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error 
//
///////////////////////////////////////////////////////////////////////////////
INT gfxColorExpansionBlt(GFX_RECT_T rect, GFX_ENTRY fore_color, GFX_ENTRY back_color, PVOID bit_buf)
{
    UINT32 x, y, width, height, width_in_32;
    UINT32 cmd32, dest_pitch, src_pitch, pitch, dest_start, dest_dimension;
    INT x1, x2, y1, y2;
    UINT32 clipTL, clipBR;
    
    if (! _gfx_bInitialized) return ERR_GFX_ENV_NOT_INITIALIZED;
    
    if (! gfxIsValidRect(rect)) return ERR_GFX_INVALID_RECT;

    gfxWaitEngineReady();

    x = (UINT32)rect.fC.nLeft;
    y = (UINT32)rect.fC.nTop;
    width = (UINT32)(rect.fC.nRight - rect.fC.nLeft + 1);
    height = (UINT32)(rect.fC.nBottom - rect.fC.nTop + 1);

    width_in_32 = ((width + 31) >> 5) << 5; // must be 32 pixels multiple
    
    cmd32 = 0xcc430080; 
    if (_gfx_nDrawMode == GFX_SRC_TRANSPARENT)
        cmd32 |= 0x00004000; 
  
    outpw(REG_GE_CONTROL, cmd32);

    outpw(REG_GE_FORE_COLOR, fore_color);
    outpw(REG_GE_BACK_COLOR, back_color);
  
    dest_pitch = (UINT32)(_gfx_nDestPitch / _gfx_nByte); // pitch in pixels
    src_pitch = width_in_32; // pitch in pixels (must be in 32 pixels unit)
  
    pitch = (dest_pitch << 16) | src_pitch;
    outpw(REG_GE_PITCH, pitch);
  
    outpw(REG_GE_SRC_START00_ADDR, (UINT32)bit_buf);
    outpw(REG_GE_SRC_START_ADDR, 0); // always starts from (0,0)

    dest_start = y << 16 | x;
    outpw(REG_GE_DEST_START_ADDR, dest_start);
  
    dest_dimension = height << 16 | width_in_32;
    outpw(REG_GE_DIMENSION, dest_dimension);
  
    x1 = rect.fC.nLeft;
    y1 = rect.fC.nTop;
    x2 = rect.fC.nRight;
    y2 = rect.fC.nBottom;
    
    if (_gfx_bClipEnabled)
    {
        if (x1 < _gfx_ClipRect.fC.nLeft) 
            x1 = _gfx_ClipRect.fC.nLeft;
        
        if (y1 < _gfx_ClipRect.fC.nTop) 
            y1 = _gfx_ClipRect.fC.nTop;
            
        if (x2 > _gfx_ClipRect.fC.nRight)     
            x2 = _gfx_ClipRect.fC.nRight;
            
        if (y2 > _gfx_ClipRect.fC.nBottom)
            y2 = _gfx_ClipRect.fC.nBottom;    
    }  
  
    /* always use clipper */

    /* hardware clipper not includes last pixel */
    
    x2++;
    y2++;
    
    cmd32 |= 0x00000200;

    clipTL = (UINT32)((y1 << 16) | x1);
    clipBR = (UINT32)((y2 << 16) | x2);
    
    outpw(REG_GE_CONTROL, cmd32);
    outpw(REG_GE_CLIP_TL, clipTL);
    outpw(REG_GE_CLIP_BR, clipBR);
    
    outpw(REG_GE_TRIGGER, 1); 

    return 0;
}



/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      FRAME.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This is module is used draw a rectangle border.  
 * 
 *  HISTORY
 *      2004/09/14  Created by PC50 Walter Tseng
 *
 *  REMARK
 *      None
 *                                                             
 ******************************************************************************/
#include "wbio.h"
#include "gfxlib.h"
#include "global.h"

#include "w99702_reg.h"


///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION
//      gfxDrawFrame()
//
//  DESCRIPTION
//      This function is used to draw a rectangle border. 
//      Only the solid border is supported in current implementation.
//
//  INPUTS
//      rect:   rectangle to be filled
//      color:  color will be used to fill the rectangle
//
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error  
// 
///////////////////////////////////////////////////////////////////////////////
INT gfxDrawFrame(GFX_PNT_T p1, GFX_PNT_T p2, INT thick, GFX_ENTRY color)
{
    UINT32 cmd32, dest_start, dest_pitch, dest_dimension;
    UINT32 sx, sy, width, height;
    UINT32 x1, x2, y1, y2;
    
    if (! _gfx_bInitialized) return ERR_GFX_ENV_NOT_INITIALIZED;

    if (p1.nX > p2.nX) return ERR_GFX_INVALID_RECT;
    if (p1.nY > p2.nY) return ERR_GFX_INVALID_RECT;
    
    if (thick < 1) return ERR_GFX_INVALID_THICK;
    
    gfxWaitEngineReady();
        
    outpw(REG_GE_FORE_COLOR, (UINT32)color);

    dest_pitch = (UINT32)(_gfx_nDestPitch / _gfx_nByte) << 16; // pitch in pixels
    outpw(REG_GE_PITCH, dest_pitch);

    sx = (UINT32)p1.nX;
    sy = (UINT32)p1.nY;
    width = (UINT32)(p2.nX - p1.nX +1);
    height = (UINT32)(p2.nY - p1.nY +1);
    
    dest_start = sy << 16 | sx;
    outpw(REG_GE_DEST_START_ADDR, dest_start);
  
    dest_dimension = height << 16 | width;
    outpw(REG_GE_DIMENSION, dest_dimension);

    x1 = sx;
    x2 = x1 + width;    // not include last pixel
    y1 = sy;
    y2 = y1 + height;   // not include last pixel
    
    x1 += thick;
    x2 -= thick;
    y1 += thick;
    y2 -= thick;
    
    outpw(REG_GE_CLIP_TL, (y1 << 16) | x1);
    outpw(REG_GE_CLIP_BR, (y2 << 16) | x2);
  
    cmd32 = 0xcc430060;     // solid fill
    cmd32 |= 0x00000300;    // outside clip enable
    outpw(REG_GE_CONTROL, cmd32);
    
    outpw(REG_GE_TRIGGER, 1); 
    
    return 0;
}

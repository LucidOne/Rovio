/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      SOLIDFILL.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This is module is used fill a rectangle with a specified pattern.
 * 
 *  HISTORY
 *      2004/09/14  Created by PC50 Walter Tseng
 *      2004/09/20  Bug fix
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
//      gfxSolidRect()
//
//  DESCRIPTION
//      This function is used to fill a rectangle with a specified pattern.
//      The pattern is specified by gfxSetPattern().
//
//  INPUTS
//      rect:   rectangle to be filled
//
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error  
// 
///////////////////////////////////////////////////////////////////////////////
INT gfxFillRect(GFX_RECT_T rect)
{
    UINT32 cmd32, dest_start, dest_pitch, dest_dimension;
    UINT32 sx, sy, width, height;
    
    if (! _gfx_bInitialized) return ERR_GFX_ENV_NOT_INITIALIZED;

    if (! _gfx_bPatternInitialized) return ERR_GFX_PATTERN_NOT_INITIALIZED;
    
    if (rect.fC.nLeft > rect.fC.nRight) return ERR_GFX_INVALID_RECT;
    if (rect.fC.nTop > rect.fC.nBottom) return ERR_GFX_INVALID_RECT;
    
    gfxWaitEngineReady();
   
    if (_gfx_nPatternType == GFX_MONO_PATTERN)
    {
        /* 8x8 mono pattern already in pattern registers */
        cmd32 = 0xf0430010;
        
        outpw(REG_GE_FORE_COLOR, _gfx_uPatternForeColor);
        outpw(REG_GE_BACK_COLOR, _gfx_uPatternBackColor);         
    }
    else /* GFX_COLOR_PATTERN */
    {
        /* 8x8 color pattern already in pattern buffer */
        cmd32 = 0xf0430000; 
    }
    
    dest_pitch = (UINT32)(_gfx_nDestPitch / _gfx_nByte) << 16; // pitch in pixels
    outpw(REG_GE_PITCH, dest_pitch);

    sx = (UINT32)rect.fC.nLeft;
    sy = (UINT32)rect.fC.nTop;
    width = (UINT32)(rect.fC.nRight - rect.fC.nLeft + 1);
    height = (UINT32)(rect.fC.nBottom - rect.fC.nTop + 1);
    
    dest_start = sy << 16 | sx;
    outpw(REG_GE_DEST_START_ADDR, dest_start);
  
    dest_dimension = height << 16 | width;
    outpw(REG_GE_DIMENSION, dest_dimension);

    if (_gfx_bClipEnabled)
    {
        cmd32 |= 0x00000200;

        outpw(REG_GE_CONTROL, cmd32);
        outpw(REG_GE_CLIP_TL, _gfx_uClipTL);
        outpw(REG_GE_CLIP_BR, _gfx_uClipBR);
    }  
  
    outpw(REG_GE_CONTROL, cmd32);
  
    outpw(REG_GE_TRIGGER, 1); 

    return 0;
}



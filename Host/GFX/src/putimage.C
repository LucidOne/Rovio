/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      PUTIMAGE.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This module is used to draw a bitmap from source surface 
 *      to destination surface.        
 * 
 *  HISTORY
 *      2004/09/13  Created by PC50 Walter Tseng
 *      2004/09/20  Supported tile option
 *      2004/11/06  Add ROP support
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
//      gfxPutImage()
//
//  DESCRIPTION
//      This function is used to draw a selected bitmap rectangle
//      from source surface to a specified address of destination surface. 
//
//  INPUTS
//      src_rect:   source rectangle
//      dest_pnt:   destination point
//      
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error 
//
///////////////////////////////////////////////////////////////////////////////
INT gfxPutImage(GFX_RECT_T src_rect, GFX_PNT_T dest_pnt)
{
    UINT32 sx, sy, dx, dy, width, height;
    UINT32 cmd32, src_pitch, dest_pitch, pitch, dest_start, src_start, dimension;
    UINT32 data32, alpha;
    UINT32 tile_ctl;
    GFX_RECT_T dest_rect;

    if ((_gfx_ucROP == 0x00) || (_gfx_ucROP == 0xff))
    {
        dest_rect.fC.nLeft = dest_pnt.nX;
        dest_rect.fC.nRight = dest_rect.fC.nLeft + (src_rect.fC.nRight - src_rect.fC.nLeft);
        dest_rect.fC.nTop = dest_pnt.nY;
        dest_rect.fC.nBottom = dest_rect.fC.nTop + (src_rect.fC.nBottom - src_rect.fC.nTop);
    
        if (_gfx_ucROP == 0x00)
            return gfxFillSolidRect(dest_rect, 0x000000);
        
        if (_gfx_ucROP == 0xff)
            return gfxFillSolidRect(dest_rect, 0xffffff);    
    }
        
    if (! _gfx_bInitialized) return ERR_GFX_ENV_NOT_INITIALIZED;

#if 0 // can't call this because it is memory buffer    
    if (! gfxIsValidRect(src_rect)) return ERR_GFX_INVALID_RECT;
#endif
    
    gfxWaitEngineReady();

    sx = (UINT32)src_rect.fC.nLeft;
    sy = (UINT32)src_rect.fC.nTop;
    width = (UINT32)(src_rect.fC.nRight - src_rect.fC.nLeft + 1);
    height = (UINT32)(src_rect.fC.nBottom - src_rect.fC.nTop + 1);
    
    dx = (UINT32)dest_pnt.nX;
    dy = (UINT32)dest_pnt.nY;

#if 1
    cmd32 = 0x00430000 | ((UINT32)_gfx_ucROP << 24);
    
    if (_gfx_nPatternType == GFX_MONO_PATTERN)
    {
        cmd32 |= 0x00000010;
        outpw(REG_GE_FORE_COLOR, _gfx_uPatternForeColor);
        outpw(REG_GE_BACK_COLOR, _gfx_uPatternBackColor); 
    }
#else
    cmd32 = 0xcc430000;
#endif
        
    outpw(REG_GE_CONTROL, cmd32);

    src_pitch = (UINT32)(_gfx_nSrcPitch / _gfx_nByte); // pitch in pixels
    dest_pitch = (UINT32)(_gfx_nDestPitch / _gfx_nByte); // pitch in pixels

    pitch = dest_pitch << 16 | src_pitch; // pitch in pixel
    outpw(REG_GE_PITCH, pitch);

    outpw(REG_GE_SRC_START00_ADDR, _gfx_uSrcStartAddr);
  
    src_start = sy << 16 | sx;
    outpw(REG_GE_SRC_START_ADDR, src_start);
  
    dest_start = dy << 16 | dx;
    outpw(REG_GE_DEST_START_ADDR, dest_start);
  
    dimension = height << 16 | width;
    outpw(REG_GE_DIMENSION, dimension);  
  
    if (_gfx_bClipEnabled)
    {
        cmd32 |= 0x00000200;
        outpw(REG_GE_CONTROL, cmd32);
        outpw(REG_GE_CLIP_TL, _gfx_uClipTL);
        outpw(REG_GE_CLIP_BR, _gfx_uClipBR);
    }  

    if (_gfx_bAlphaEnabled)
    {
        //
        // NOTE:
        //   Hardware doesn't allow pattern with alpha blending.
        //
        cmd32 = 0xcc430000; // only support SRCCOPY
        cmd32 |= 0x00200000;
        outpw(REG_GE_CONTROL, cmd32);
    
        data32 = inpw(REG_GE_MISC) & 0x0000ffff;
        alpha = _gfx_uAlphaKs << 8 | (256 - _gfx_uAlphaKs);
        data32 |= (alpha << 16);
    
        outpw(REG_GE_MISC, data32);
    }
        
    if (_gfx_bColorKeyEnabled)
    {
        cmd32 |= 0x00008000; // color transparency 
        outpw(REG_GE_CONTROL, cmd32);
        outpw(REG_GE_TRANSPARENCY_COLOR, _gfx_uColorKey);
        outpw(REG_GE_TRANSPARENCY_MASK, _gfx_uColorKeyMask);
    }
  
    /* 
    ** The tile height and width are changed to CR0000 bit 31 to b16.
    */
    tile_ctl = ((_gfx_uTileY-1) << 24) | ((_gfx_uTileX-1) << 16);
    
    if (tile_ctl != 0) // not 1x1
    {
        cmd32 |= 0x0000400; // b10 controls the tile option
        outpw(REG_GE_CONTROL, cmd32);
    }
     
    tile_ctl |= 1; // GO bit
    outpw(REG_GE_TRIGGER, tile_ctl);     

    return GFX_OK;
}



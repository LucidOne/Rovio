/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      GETIMAGE.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This module is used to get a bitmap from destination surface 
 *      to source surface. .       
 * 
 *  HISTORY
 *      2004/09/13  Created by PC50 Walter Tseng
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
//      gfxGetImage()
//
//  DESCRIPTION
//      This function is used to get a selected bitmap rectangle
//      from destination surface to a specified address of source surface. 
//
//  INPUTS
//      dest_rect:  destination rectangle
//      src_pnt:    source point (in X/Y addressing)
//      
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error 
//
///////////////////////////////////////////////////////////////////////////////
INT gfxGetImage(GFX_RECT_T dest_rect, GFX_PNT_T src_pnt)
{
    UINT32 sx, sy, dx, dy, width, height;
    UINT32 cmd32, src_pitch, dest_pitch, pitch, dest_start, src_start, dimension;
    UINT32 data32, alpha;
    
    if (! _gfx_bInitialized) return ERR_GFX_ENV_NOT_INITIALIZED;

    if (! gfxIsValidRect(dest_rect)) return ERR_GFX_INVALID_RECT;
    
    gfxWaitEngineReady();

    sx = (UINT32)dest_rect.fC.nLeft;
    sy = (UINT32)dest_rect.fC.nTop;
    width = (UINT32)(dest_rect.fC.nRight - dest_rect.fC.nLeft +1);
    height = (UINT32)(dest_rect.fC.nBottom - dest_rect.fC.nTop +1);
    
    dx = (UINT32)src_pnt.nX;
    dy = (UINT32)src_pnt.nY;

    cmd32 = 0xcc430000;
    outpw(REG_GE_CONTROL, cmd32);

    src_pitch = (UINT32)(_gfx_nSrcPitch / _gfx_nByte) << 16; // pitch in pixels
    dest_pitch = (UINT32)(_gfx_nDestPitch / _gfx_nByte) << 16; // pitch in pixels

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
    
    if (_gfx_bColorKeyEnabled)
    {
        cmd32 |= 0x00008000; // color transparency 
        outpw(REG_GE_CONTROL, cmd32);
        outpw(REG_GE_TRANSPARENCY_COLOR, _gfx_uColorKey);
        outpw(REG_GE_TRANSPARENCY_MASK, _gfx_uColorKeyMask);
    }
    
    if (_gfx_bAlphaEnabled)
    {
        cmd32 |= 0x00200000;
        outpw(REG_GE_CONTROL, cmd32);
    
        data32 = inpw(REG_GE_MISC) & 0x0000ffff;
        alpha = _gfx_uAlphaKs << 8 | (256 - _gfx_uAlphaKs);
        data32 |= (alpha << 16);
    
        outpw(REG_GE_MISC, data32);
    }
  
    outpw(REG_GE_TRIGGER, 1); 

    return 0;
}



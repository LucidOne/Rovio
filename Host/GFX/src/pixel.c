/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      PIXEL.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This is module is used to draw a single pixel on to screen.      
 * 
 *  HISTORY
 *      2004/09/13  Created by PC50 Walter Tseng
 *      2004/09/20  Added drawing ROP
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
//      gfxDrawPixel()
//
//  DESCRIPTION
//      This function is used to draw a pixel onto the destination 
//      surface with a specified address and raster operation.
//
//      This is the basis of all drawing functions. The application can use
//      gfxDrawPixel() to implement high level drawing functions like arc or
//      ellipse, etc. 
//
//  INPUTS
//      pnt:    pixel address in X/Y addressing
//      color:  pixel source color
//      rop:    rop for pixel drawing
//
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error  
// 
///////////////////////////////////////////////////////////////////////////////
INT gfxDrawPixel(GFX_PNT_T pnt, GFX_ENTRY color, GFX_DRAW_ROP_E rop)
{
    UINT32 addr; 
    UINT8 *p8;
    UINT16 *p16;
    UINT32 *p32;
    
    if (! _gfx_bInitialized) return ERR_GFX_ENV_NOT_INITIALIZED;

    gfxWaitEngineReady();

    addr = _gfx_uDestStartAddr + (_gfx_nDestPitch * pnt.nY) + (_gfx_nByte * pnt.nX);
    addr |= 0x10000000;
    
    if (_gfx_nByte == 1)
    {
        p8 = (UINT8 *)addr;
        switch (rop)
        {
            case GFX_DRAW_ROP_COPY:
                *p8 = (UINT8)color;
                break;
            case GFX_DRAW_ROP_XOR:
                *p8 = (UINT8)color ^ *p8;
                break;
            case GFX_DRAW_ROP_OR:
                *p8 = (UINT8)color | *p8;
                break;
            case GFX_DRAW_ROP_AND:
                *p8 = (UINT8)color & *p8;
                break;                    
        }
    }    
    if (_gfx_nByte == 2)
    {
        p16 = (UINT16 *)addr;
        switch (rop)
        {
            case GFX_DRAW_ROP_COPY:
                *p16 = (UINT16)color;
                break;
            case GFX_DRAW_ROP_XOR:
                *p16 = (UINT16)color ^ *p16;
                break;
            case GFX_DRAW_ROP_OR:
                *p16 = (UINT16)color | *p16;
                break;
            case GFX_DRAW_ROP_AND:
                *p16 = (UINT16)color & *p16;
                break;                    
        }
    }   
    else /* _gfx_nByte == 4 */
    {
        p32 = (UINT32 *)addr;
        switch (rop)
        {
            case GFX_DRAW_ROP_COPY:
                *p32 = (UINT32)color;
                break;
            case GFX_DRAW_ROP_XOR:
                *p32 = (UINT32)color ^ *p16;
                break;
            case GFX_DRAW_ROP_OR:
                *p32 = (UINT32)color | *p16;
                break;
            case GFX_DRAW_ROP_AND:
                *p32 = (UINT32)color & *p16;
                break;                    
        }
    }
       
    return 0;
}

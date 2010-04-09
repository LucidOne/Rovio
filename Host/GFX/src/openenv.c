/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      OPENENV.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This module is used to open the drawing environment.      
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
#include "wblib.h"
#include "gfxlib.h"
#include "global.h"

#include "w99702_reg.h"


#define GE_MISC_BPP_8   0x00000000
#define GE_MISC_BPP_16  0x00000010
#define GE_MISC_BPP_32  0x00000020


///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION
//      gfxCheckColorFormat()
//
//  DESCRIPTION
//      This function is used to check if the color format is valid
//      or not.
//
//  INPUTS
//      color_format:   color format of the drawing environment
//      
//  OUTPUTS
//      None
//
//  RETURN
//      0:      the color format is invalid
//      others: the BPP of the drawing environment
//
///////////////////////////////////////////////////////////////////////////////
static INT gfxCheckColorFormat(GFX_COLOR_FORMAT_E color_format)
{
    if (color_format == GFX_BPP_332) 
        return 8;
    
    if ((color_format == GFX_BPP_444H) || (color_format == GFX_BPP_444L))
        return 16;
    
    if (color_format == GFX_BPP_565) return 16;
    
    if ((color_format == GFX_BPP_666) || (color_format == GFX_BPP_888))
        return 32;
    
    return 0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION
//      gfxResetGE()
//
//  DESCRIPTION
//      This function is used to reset graphics engine hardware.
//
//  INPUTS
//      None
//      
//  OUTPUTS
//      None
//
//  RETURN
//      None
//
//  REMARK:
//      This register is undocumented in the design specification.
//      REG_GE_MISC b6 is for FIFO reset, b7 is for engine reset.
//      If the FIFO reset and engine reset are enabled at the same time,
//      the FIFO reset may be ignored. Therefore, firmware has to do
//      FIFO reset before engine reset. 
//
///////////////////////////////////////////////////////////////////////////////
static VOID gfxResetGE()
{
    outpw(REG_GE_MISC, 0x00000040); // FIFO reset
    outpw(REG_GE_MISC, 0x00000000);
    
    outpw(REG_GE_MISC, 0x00000080); // Engine reset
    outpw(REG_GE_MISC, 0x00000000);
}


///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION
//      gfxInitGE()
//
//  DESCRIPTION
//      This function is used to initialize the graphics engine hardware
//      according to the specified drawing environment parameters.
//
//  INPUTS
//      None
//      
//  OUTPUTS
//      None
//
//  RETURN
//      None
//
///////////////////////////////////////////////////////////////////////////////
static VOID gfxInitGE()
{
    gfxResetGE();
    
    outpw(REG_GE_CONTROL, 0);   // disable interrupt
    outpw(REG_GE_INTS, 0);      // clear interrupt
    
    outpw(REG_GE_PAT_START_ADDR, _gfx_uColorPatternStartAddr);
    outpw(REG_GE_DEST_START00_ADDR, _gfx_uDestStartAddr);
    outpw(REG_GE_SRC_START00_ADDR, _gfx_uSrcStartAddr);

    outpw(REG_GE_WRITE_MASK, 0x00ffffff);
    
    switch (_gfx_nBpp)
    {
        case 8:
            outpw(REG_GE_MISC, GE_MISC_BPP_8);
            break;
        case 16:
            outpw(REG_GE_MISC, GE_MISC_BPP_16);
            break;
        case 32:
            outpw(REG_GE_MISC, GE_MISC_BPP_32);
            break;          
    }

#if 0    
    gfxClearScreen(0);  // clear destination graphics buffer to black
#endif
}


///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION
//      gfxOpenEnv()
//
//  DESCRIPTION
//      This function is used to open a drawing environment. The drawing 
//      environment is specified a GFX_INFO_T structure. This function 
//      surface with a specified address and raster operation.
//
//  INPUTS
//      in_param:   pointer to a drawing environment structure 
//
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      othesr: error
// 
///////////////////////////////////////////////////////////////////////////////
INT gfxOpenEnv(GFX_INFO_T *in_param)
{
    if (_gfx_bInitialized) // check if the drawing environment is open
        return ERR_GFX_ENV_OPENED;
    
    _gfx_nBpp = gfxCheckColorFormat(in_param->nColorFormat);

#if (DBG_LEVEL > 0)    
    gfx_debug("_gfx_nBpp = %d\n", _gfx_nBpp);
#endif
    
    if (_gfx_nBpp == 0) // color format valid?
        return ERR_GFX_INVALID_COLOR_FORMAT;
    
    _gfx_nColorFormat = in_param->nColorFormat;
    
    _gfx_nByte = _gfx_nBpp >> 3;   
    
    _gfx_nDestWidth     = in_param->nDestWidth;
    _gfx_nDestHeight    = in_param->nDestHeight;
    _gfx_nSrcWidth      = in_param->nSrcWidth;
    _gfx_nSrcHeight     = in_param->nSrcHeight;

#if (DBG_LEVEL > 0)    
    gfx_debug("_gfx_nDestWidth = %d\n", _gfx_nDestWidth);
    gfx_debug("_gfx_nDestHeight = %d\n", _gfx_nDestHeight);
    gfx_debug("_gfx_nSrcWidth = %d\n", _gfx_nSrcWidth);
    gfx_debug("_gfx_nSrcHeight = %d\n", _gfx_nSrcHeight);
#endif
    
    _gfx_nDestPitch     = in_param->nDestPitch;
    _gfx_nSrcPitch      = in_param->nSrcPitch;
    
    _gfx_nScreenSize    = _gfx_nDestHeight * _gfx_nDestPitch;
    
    _gfx_uDestStartAddr         = in_param->uDestStartAddr;
    _gfx_uColorPatternStartAddr = in_param->uColorPatternStartAddr;
    _gfx_uSrcStartAddr          = in_param->uSrcStartAddr;
    
    gfxInitGE();
    
    _gfx_bInitialized = TRUE;
    
    return 0; 
}


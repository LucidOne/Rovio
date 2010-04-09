/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      SETDEST.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This module is used to change the destination surface for BitBlt. 
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

#include "wbio.h"
#include "w99702_reg.h"


///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION
//      gfxSetDestSurface()
//
//  DESCRIPTION
//      This function is used change the destination surface for BitBlt.
//
//  INPUTS
//      surface:    pointer to a surface structure
//
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error
// 
//  REMARK
//      The bpp can't be changed.
//
///////////////////////////////////////////////////////////////////////////////
INT gfxSetDestSurface(GFX_SURFACE_T *surface)
{
    if (! _gfx_bInitialized) return ERR_GFX_ENV_NOT_INITIALIZED;

    gfxWaitEngineReady();
    
    _gfx_nDestWidth     = surface->nWidth;
    _gfx_nDestHeight    = surface->nHeight;
    _gfx_nDestPitch     = surface->nPitch;
    _gfx_uDestStartAddr = surface->uStartAddr;

    outpw(REG_GE_DEST_START00_ADDR, _gfx_uDestStartAddr);
    
    return 0;   
} 




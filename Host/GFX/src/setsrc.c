/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      SETSRC.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This module is used to change the source surface for BitBlt. 
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
//      gfxSetSrcSurface()
//
//  DESCRIPTION
//      This function is used change the source surface for BitBlt.
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
INT gfxSetSrcSurface(GFX_SURFACE_T *surface)
{
    if (! _gfx_bInitialized) return ERR_GFX_ENV_NOT_INITIALIZED;

    gfxWaitEngineReady();

    _gfx_nSrcWidth      = surface->nWidth;
    _gfx_nSrcHeight     = surface->nHeight;
    _gfx_nSrcPitch      = surface->nPitch;
    _gfx_uSrcStartAddr  = surface->uStartAddr;

    outpw(REG_GE_SRC_START00_ADDR, _gfx_uSrcStartAddr);
    
    return 0;   
} 




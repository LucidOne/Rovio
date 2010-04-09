/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      CLOSEENV.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This module is used to close the drawing environment.      
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


///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION
//      gfxCloseEnv()
//
//  DESCRIPTION
//      This function is used to close current drawing environment. 
//
//  INPUTS
//      None
//
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error
//
//  REMARK:
//      This function must be called before the application want to 
//      change the drawing environment. 
//
//      All of the drawing objects like pen will be reset if application
//      calls gfxCloseEnv().
//
//      The application can call gfxOpenEnv() to open a new drawing 
//      environment again.
// 
///////////////////////////////////////////////////////////////////////////////
INT gfxCloseEnv(VOID)
{
    _gfx_bInitialized = FALSE;
    
    _gfx_bClipEnabled       = FALSE;
    _gfx_bColorKeyEnabled   = FALSE;
    _gfx_bAlphaEnabled      = FALSE;
    
    _gfx_bPenInitialized        = FALSE;
    _gfx_bPatternInitialized    = FALSE;
    
    _gfx_ucROP      = SRCCOPY;
    _gfx_nDrawMode  = GFX_OPAQUE;
    
    _gfx_uTileX = 1;
    _gfx_uTileY = 1;
        
    return 0; 
}


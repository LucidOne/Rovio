/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      CLRSCR.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This is module is used to clear graphics buffer.      
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
//      gfxClearScreen()
//
//  DESCRIPTION
//      This function is used to clear the destination graphics buffer
//      with a specified color. 
//
//  INPUTS
//      color:  color will be used to fill the graphics buffer
//
//  OUTPUTS
//      None
//
//  RETURN
//      0:      Success
//      others: Error  
// 
///////////////////////////////////////////////////////////////////////////////
INT gfxClearScreen(GFX_ENTRY color)
{
    UINT32 cmd32, dest_pitch, dest_dimension;
    
    if (! _gfx_bInitialized) return ERR_GFX_ENV_NOT_INITIALIZED;

    gfxWaitEngineReady();
    
    cmd32 = 0xcc430040;
    outpw(REG_GE_CONTROL, cmd32);
    outpw(REG_GE_BACK_COLOR, (UINT32)color);
    
    dest_pitch = (UINT32)(_gfx_nDestPitch / _gfx_nByte) << 16; // pitch in pixels
    outpw(REG_GE_PITCH, dest_pitch);
    
    outpw(REG_GE_DEST_START_ADDR, 0);   // starts from (0,0)
    
    dest_dimension = (UINT32)_gfx_nDestHeight << 16 | (UINT32)_gfx_nDestWidth;
    outpw(REG_GE_DIMENSION, dest_dimension);
    
    outpw(REG_GE_TRIGGER, 1); 
    
    return 0;
}

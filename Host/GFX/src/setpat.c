/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      SETPAT.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This module is used to set the pattern (brush) for BitBlt.
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
//      gfxSetPattern()
//
//  DESCRIPTION
//      This function is used set the pattern (brush) for drawing lines.
//      The pattern size is fixed at 8x8x. It can be either color or mono.
//      The mono pattern data is from the pattern registers.
//      The color pattern data is from memory buffer. 
//
//  INPUTS
//      type:       8x8 pattern type (could be mono or color)
//      fore_color: foreground color of mono pattern
//      back_color: background color of mono pattern
//      pat_buf:    pointer to the pattern data buffer
//
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error
// 
///////////////////////////////////////////////////////////////////////////////
INT gfxSetPattern(GFX_PATTERN_TYPE_E type, GFX_ENTRY fore_color, GFX_ENTRY back_color, PVOID pat_buf)
{
    UINT32 *pat32;
    UINT8 *ptr8, *pat8;
    INT i;

    gfxWaitEngineReady();

    if (type == GFX_MONO_PATTERN)
    {
        /* mono pattern has to keep the foreground color and background color */
 
        pat32 = (UINT32 *)pat_buf;
       
        _gfx_uPatternForeColor = fore_color;
        _gfx_uPatternBackColor = back_color;
        
        outpw(REG_GE_PATA, *pat32++);
        outpw(REG_GE_PATB, *pat32);
    }
    else if (type == GFX_COLOR_PATTERN)
    {
        /* color pattern has to fill the pattern buffer */
      
        pat8 = (UINT8 *)pat_buf;
        
        ptr8 = (UINT8 *)_gfx_uColorPatternStartAddr;
        for (i=0; i<(64*_gfx_nByte); i++) *ptr8++ = *pat8++;
    }    
    else return ERR_GFX_INVALID_PATTERN_TYPE; 
    
    _gfx_nPatternType = type;
    _gfx_bPatternInitialized = TRUE;
    
    return 0;   
} 




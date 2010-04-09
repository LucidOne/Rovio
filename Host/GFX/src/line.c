/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      LINE.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This is module is used to draw lines on to screen.      
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


///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION
//      gfxDrawLine()
//
//  DESCRIPTION
//      This function is used to draw lines onto the destination 
//      surface.
//
//      The applicaiton must call gfxSetPen() to specify the line before
//      this function is called. 
//
//  INPUTS
//      p1:     start point in X/Y addressing
//      p2:     end point in X/Y addressing
//
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error  
// 
///////////////////////////////////////////////////////////////////////////////
INT gfxDrawLine(GFX_PNT_T p1, GFX_PNT_T p2)
{
    INT abs_X, abs_Y, min, max;
    INT x1, y1, x2, y2;
    UINT32 step_constant, initial_error, direction_code;
    UINT32 cmd32, dest_pitch, dest_start;
    UINT32 temp32, line_control_code;
    
    if (! _gfx_bInitialized) return ERR_GFX_ENV_NOT_INITIALIZED;
    
    if (! _gfx_bPenInitialized) return ERR_GFX_PEN_NOT_INITIALIZED;

    gfxWaitEngineReady();
  
    x1 = p1.nX;
    y1 = p1.nY;
    x2 = p2.nX;
    y2 = p2.nY;
    
    abs_X = ABS(x2-x1);
    abs_Y = ABS(y2-y1);
    
    if (abs_X > abs_Y) // X major
    {	
        max = abs_X;
        min = abs_Y;
    
        step_constant = (((UINT32)(2*(min-max))) << 16) | (UINT32)(2*min);
        initial_error = (((UINT32)(2*min-max)) << 16) | (UINT32)(max);
        
        if (x2 > x1) // +X direction
        {
            if (y2 > y1) // +Y direction
                direction_code = XpYpXl;
            else // -Y direction   	
                direction_code = XpYmXl;
        }
        else // -X direction
        {
            if (y2 > y1) // +Y direction
                direction_code = XmYpXl;
            else // -Y direction   	
                direction_code = XmYmXl;    	
        }
    }		
    else // Y major
    {
        max = abs_Y;
        min = abs_X;
    
        step_constant = (((UINT32)(2*(min-max))) << 16) | (UINT32)(2*min);
        initial_error = (((UINT32)(2*min-max)) << 16) | (UINT32)(max);
  
        if (x2 > x1) // +X direction
        {
            if (y2 > y1) // +Y direction
                direction_code = XpYpYl;
            else // -Y direction   	
                direction_code = XpYmYl;
        }
        else // -X direction
        {
            if (y2 > y1) // +Y direction
                direction_code = XmYpYl;
            else // -Y direction   	
                direction_code = XmYmYl;    	
        }
    }
  
    outpw(REG_GE_ERR_TERM_STEP_CONST, step_constant);
    outpw(REG_GE_INIT_ERR_COUNT, initial_error);
  
    cmd32 = 0x009b0000 | direction_code; // styled line
  
    if (_gfx_nDrawMode == GFX_SRC_TRANSPARENT)
    {
        cmd32 |= 0x00004000; 
    }
    outpw(REG_GE_CONTROL, cmd32);

    outpw(REG_GE_BACK_COLOR, _gfx_uPenForeColor); 
    outpw(REG_GE_FORE_COLOR, _gfx_uPenBackColor);

    dest_pitch = (UINT32)(_gfx_nDestPitch / _gfx_nByte) << 16; // pitch in pixels
    outpw(REG_GE_PITCH, dest_pitch);

    dest_start = y1 << 16 | x1;
    outpw(REG_GE_DEST_START_ADDR, dest_start);
  
    if (_gfx_bClipEnabled)
    {
        cmd32 |= 0x00000200;
        outpw(REG_GE_CONTROL, cmd32);
        outpw(REG_GE_CLIP_TL, _gfx_uClipTL);
        outpw(REG_GE_CLIP_BR, _gfx_uClipBR);
    }  
  
    line_control_code = (UINT32)_gfx_usPenStyle; 
    temp32 = inpw(REG_GE_MISC) & 0x0000ffff;
    temp32 = (line_control_code << 16) | temp32;
  
    outpw(REG_GE_MISC, temp32); 
    
    outpw(REG_GE_TRIGGER, 1); 
    
    return 0;
}

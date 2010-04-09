/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      TEXTOUT.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This module is used to handle the text output.
 * 
 *  HISTORY
 *      2004/09/22  Created by PC50 Walter Tseng
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
//      gfxTextOutput()
//
//  DESCRIPTION
//      This function is used to do the text output. It will draw the 
//      characters of the input string pointed by str according to the
//      drawing mode set by gfxSetDrawMode() on the destination surface.
//
//      This function must allocate string bitmap buffer for the 
//      gfxRealizeText() function.
//
//      This function doesn't support negative coordinaiton. This requirement
//      must be done by software.
//
//      Some functions are supported in the font library:
//          - gfxGetTextWidth()
//          - gfxGetTextHeight()
//          - gfxRealizeText()
//
//  INPUTS
//      p:          starting coordination
//      str:        pointer to the string
//      len:        length in the string buffer to be output
//      fore_color: foreground color
//      back_color: backround color
//      
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error 
//
///////////////////////////////////////////////////////////////////////////////
INT gfxTextOutput(GFX_PNT_T p, PSTR str, INT len, GFX_ENTRY fore_color, GFX_ENTRY back_color)
{
    INT str_width, str_height, str_stride, buf_size;
    PVOID bit_buf;
    GFX_RECT_T rect;
    
    if (! _gfx_bInitialized) return ERR_GFX_ENV_NOT_INITIALIZED;
 
    str_width = gfxGetTextWidth(str, len);
    if (str_width == 0) return 0;
    
    str_height = gfxGetTextHeight(str, len);
    if (str_height == 0) return 0;
    
    str_stride = ((str_width + 31) >> 5) << 2; // 4-byte multiple
    
    buf_size = str_stride * str_height;

#ifdef ECOS
#else
    bit_buf = (PVOID)malloc(buf_size);    
#endif
    
    if (bit_buf == NULL) return 0; // memory not enough

    bit_buf = (PVOID)((UINT32)bit_buf | 0x10000000);
        
    if (gfxRealizeText(bit_buf, str, len) != 0) return 0; // something wrong
        
    rect.fC.nLeft = p.nX;
    rect.fC.nTop = p.nY;
    rect.fC.nRight = rect.fC.nLeft + str_width - 1;
    rect.fC.nBottom = rect.fC.nTop + str_height - 1;

    gfxColorExpansionBlt(rect, fore_color, back_color, bit_buf);
    
    return 0;
}



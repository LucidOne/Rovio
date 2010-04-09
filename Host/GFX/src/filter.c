/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      FILTER.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This is module is used to do 3x3 image filter.
 * 
 *  HISTORY
 *      2004/09/27  Created by PC50 Walter Tseng
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
//      gfxImageFilter()
//
//  DESCRIPTION
//      This function is used to do 3x3 filter of an image block.
//
//  INPUTS
//      dest:   pointer to output image block buffer
//      src:    pointer to input image block buffer
//      size:   block size information
//      ctl:    pointer to the filter control parameters
//
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error  
// 
///////////////////////////////////////////////////////////////////////////////
INT gfxImageFilter(GFX_FILTER_BLOCK_T *dest, GFX_FILTER_BLOCK_T *src, GFX_SIZE_T size, GFX_FILTER_CONTROL_T *ctl)
{
    UINT32 temp_misc, temp_mask, cmd32, pitch, dimension;
    UINT32 offset, threshold, trigger_clamp, filter_ctl0, filter_ctl1;
    
    gfxWaitEngineReady();
   
    // backup BPP information
    temp_misc = inpw(REG_GE_MISC);
    
    // must use 8-BPP  
    outpw(REG_GE_MISC, temp_misc & 0xffffffcf); // filter always uses 8-BPP

    // backup write mask
    temp_mask = inpw(REG_GE_WRITE_MASK); 
    
    outpw(REG_GE_WRITE_MASK, 0x0000ff);
       
    cmd32 = 0x00010000; // 3x3 filter command (must be linear addressing mode !!)
    outpw(REG_GE_CONTROL, cmd32);  

    offset = ctl->uOffset;
    threshold = ctl->uThreshold;
    trigger_clamp = ctl->uMaxClamp << 8;
    
    filter_ctl0 = (offset << 24) | ctl->uFilter0;
    filter_ctl1 = ctl->uFilter1;
    
    outpw(REG_GE_FILTER0, filter_ctl0);
    outpw(REG_GE_FILTER1, filter_ctl1);
    outpw(REG_GE_FILTER_THRESHOLD, threshold);
    outpw(REG_GE_TRIGGER, trigger_clamp);
  
    pitch = (dest->uPitch << 16) | src->uPitch;
    outpw(REG_GE_PITCH, pitch);  

    outpw(REG_GE_SRC_START00_ADDR, src->uStartAddr);
    outpw(REG_GE_SRC_START_ADDR, 0);
  
    outpw(REG_GE_DEST_START00_ADDR, dest->uStartAddr);
    outpw(REG_GE_DEST_START_ADDR, 0);

    dimension = (UINT32)(size.nHeight << 16 | size.nWidth);
    outpw(REG_GE_DIMENSION, dimension);  

    outpw(REG_GE_MISC, inpw(REG_GE_MISC)); // address calculation
     
    trigger_clamp |= 0x00000001; 
    outpw(REG_GE_TRIGGER, trigger_clamp);        
    
    while ((inpw(REG_GE_INTS) & 0x01) == 0); // wait for command complete

    outpw(REG_GE_INTS, 1); // clear interrupt status       
    
    outpw(REG_GE_MISC, temp_misc); // restore BPP
    outpw(REG_GE_WRITE_MASK, temp_mask); // restore mask
      
    return 0;
}



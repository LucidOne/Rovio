/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      WAITREADY.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This is module is used to wait for the previous engine command
 *      complete..      
 * 
 *  HISTORY
 *      2004/09/14  Created by PC50 Walter Tseng
 *
 *  REMARK
 *      None
 *                                                             
 ******************************************************************************/
#include "wbio.h"
#include "w99702_reg.h"


///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION
//      gfxWaitEngineReady()
//
//  DESCRIPTION
//      This function is used to wait for the previous command complete in 
//      order to process a new graphics engine command.
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
VOID gfxWaitEngineReady(VOID)
{
    if ((inpw(REG_GE_TRIGGER) & 0x01) != 0) // engine busy?
    {
        while ((inpw(REG_GE_INTS) & 0x01) == 0); // wait for command complete
    }         
           
    outpw(REG_GE_INTS, 1); // clear interrupt status
}

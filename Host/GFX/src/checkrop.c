/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      CHECKROP.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This is module is used to check the ROP.
 * 
 *  HISTORY
 *      2004/11/06  Created by PC50 Walter Tseng
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
//      gfxIsPInROP()
//      
//  DESCRIPTION
//      This function is used to check if the input ROP has P or not.
//      Rule for checking ROP: b0=b4, b1=b5, b2=b6, b3=b7 => No P in a ROP         
//  INPUTS
//      rop:    rop code
//
//  OUTPUTS
//      None
//
//  RETURN
//      TRUE:   ROP contains P
//      FALSE:  ROP not contains P
// 
///////////////////////////////////////////////////////////////////////////////
BOOL gfxIsPInROP(UINT8 rop)
{
    if ((rop & 0xf0)==(rop << 4)) return FALSE;
     
    return TRUE;
}

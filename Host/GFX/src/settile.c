/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      SETROP.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This module is used to set tile control of BitBlt.
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
//      gfxSetTile()
//
//  DESCRIPTION
//      This function is used set the tile control of BitBlt.
//
//  INPUTS
//      tile_x:     tile counter in X direction
//      tile_y:     tile counter in Y direction
//
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error
// 
///////////////////////////////////////////////////////////////////////////////
INT gfxSetTile(INT tile_x, INT tile_y)
{
    _gfx_uTileX = (UINT32)tile_x;
    _gfx_uTileY = (UINT32)tile_y;
    
    return 0;   
} 




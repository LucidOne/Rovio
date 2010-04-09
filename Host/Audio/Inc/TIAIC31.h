/**************************************************************************************************
 *                                                                          
 * Copyright (c) 2004 - 2007 Winbond Electronics Corp. All rights reserved.      
 *                                                                         
 * FILENAME
 *     TIAIC31.h
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     This file contains the register map of TIAIC31
 *
 * HISTORY
 *     08/08/2004		 Ver 1.0 Created by PC31 SJLu
 *
 * REMARK
 *     None
 *     
 *************************************************************************************************/
#ifndef _TIAIC31_H_
#define _TIAIC31_H_

//#define _I2C
#define READSTATUS() ((inpw(REG_SerialBusCR)&0x38)>>3)

#ifdef _I2C
#define TI_SDA (0x1<<1)
#define TI_SCLK (0x1)
#else
#define TI_SDA (1<<5)
#define TI_SCLK (1<<1)
#endif







                
#endif	/* _TIAIC31_H_ */
                


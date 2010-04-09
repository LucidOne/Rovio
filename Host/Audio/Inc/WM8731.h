/**************************************************************************************************
 *                                                                          
 * Copyright (c) 2004 - 2007 Winbond Electronics Corp. All rights reserved.      
 *                                                                         
 * FILENAME
 *     WM8731.h
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     This file contains the register map of IIS audio interface
 *
 * HISTORY
 *     11/01/2005		 Ver 1.0 Created by PC31 SJLu
 *
 * REMARK
 *     None
 *     
 *************************************************************************************************/
#ifndef _WM8731_H_
#define _WM8731_H_

#define _2WIRE



#ifdef _3WIRE
/*for veriation board*/
#define WM_SCLK	(1<<12)
#define WM_DO	(1<<13)
#define WM_SDIN	(1<<14)
#define WM_CSB	(1<<15)
/* end of for veriation board */
#endif


/* for demo board */
#define READSTATUS() ((inpw(REG_SerialBusCR)&0x38)>>3)
/*--------*/







                
#endif	/* _WM8731_H_ */
                


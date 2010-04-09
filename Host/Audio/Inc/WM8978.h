/**************************************************************************************************
 *                                                                          
 * Copyright (c) 2004 - 2007 Winbond Electronics Corp. All rights reserved.      
 *                                                                         
 * FILENAME
 *     WM8978.h
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     This file contains the register map of IIS audio interface
 *
 * HISTORY
 *     02/09/2004		 Ver 1.0 Created by PC31 SJLu
 *
 * REMARK
 *     None
 *     
 *************************************************************************************************/
#ifndef _WM8978_H_
#define _WM8978_H_

#define _2WIRE

#ifdef _3WIRE
/*for veriation board*/
#define WM_SCLK	(1<<12)
#define WM_DO	(1<<13)
#define WM_SDIN	(1<<14)
#define WM_CSB	(1<<15)
/* end of for veriation board */
#endif




#define READSTATUS() ((inpw(REG_SerialBusCR)&0x38)>>3)
#define HEADPHONE_PATH_1 (1<<15)
#define HEADPHONE_PATH_2 (1<<16)
/*--------*/

#define	WM_8000		(5<<1)
#define	WM_12000	(4<<1)
#define	WM_16000	(3<<1)
#define	WM_24000	(2<<1)	
#define	WM_32000	(1<<1)
#define	WM_48000	0
           
#define	WM_11025	(4<<1)
#define	WM_22050	(2<<1)
#define	WM_44100	0



/*
#define	WM_8000		0xC<<1
#define	WM_16000	0xC<<1
#define	WM_24000	0x0	
#define	WM_32000	0xC<<1
#define	WM_48000	0x0
           
#define	WM_11025	0x11<<1
#define	WM_22050	0x10<<1
#define	WM_44100	0x11<<1
*/
/*
#define	WM_8000		0x6<<1
#define	WM_16000	0xA<<1	
#define	WM_24000	0x1C<<1	
#define	WM_32000	0xC<<1
#define	WM_48000	0x0
           
#define	WM_11025	0x19<<1
#define	WM_22050	0x1B<<1
#define	WM_44100	0x11<<1
*/
#define WM_384fs	(0x1<<7)
#define WM_256fs	0x0



                
#endif	/* _WM8978_H_ */
                


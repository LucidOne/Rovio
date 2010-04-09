/**************************************************************************************************
 *                                                                          
 * Copyright (c) 2004 - 2007 Winbond Electronics Corp. All rights reserved.      
 *                                                                         
 * FILENAME
 *     WM8751.h
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
#ifndef _WM8751_H_
#define _WM8751_H_

#define _2WIRE

/*----- register definition of wolfson -----*/
#define R1 	(0x1<<9 )
#define R2 	(0x2<<9 )
#define R3 	(0x3<<9 )
#define R4 	(0x4<<9 )
#define R5 	(0x5<<9 )
#define R6 	(0x6<<9 )
#define R7 	(0x7<<9 )
#define R8 	(0x8<<9 )
#define R9 	(0x9<<9 )
#define R10 (0xA<<9 )
#define R11	(0xB<<9 )
#define R12 (0xC<<9 )
#define R13	(0xD<<9 )
#define R14 (0xE<<9 )
#define R15 (0xF<<9 )
#define R16 (0x10<<9)
#define R17 (0x11<<9)
#define R18 (0x12<<9)
#define R19 (0x13<<9)
#define R20 (0x14<<9)
#define R21 (0x15<<9)
#define R22 (0x16<<9)
#define R23 (0x17<<9)
#define R24 (0x18<<9)
#define R25 (0x19<<9)
#define R26 (0x1A<<9)
#define R27 (0x1B<<9)
#define R28 (0x1C<<9)
#define R29 (0x1D<<9)
#define R30 (0x1E<<9)
#define R31 (0x1F<<9)
#define R32 (0x20<<9)
#define R33 (0x21<<9)
#define R34 (0x22<<9)
#define R35 (0x23<<9)
#define R36 (0x24<<9)
#define R37 (0x25<<9)
#define R38 (0x26<<9)
#define R39 (0x27<<9)
#define R40 (0x28<<9)
#define R41 (0x29<<9)
#define R42 (0x2A<<9)
#define R43 (0x2B<<9)
#define R44 (0x2C<<9)
#define R45 (0x2D<<9)
#define R46 (0x2E<<9)
#define R47 (0x2F<<9)
#define R48 (0x30<<9)
#define R49 (0x31<<9)
#define R50 (0x32<<9)
#define R51 (0x33<<9)
#define R52 (0x34<<9)
#define R53 (0x35<<9)
#define R54 (0x36<<9)
#define R55 (0x37<<9)
#define R56 (0x38<<9)
#define R57 (0x39<<9)
#define R58 (0x3A<<9)
#define R59 (0x3B<<9)
#define R60 (0x3C<<9)
#define R61 (0x3D<<9)
#define R62 (0x3E<<9)

#ifdef _3WIRE
/*for veriation board*/
#define WM_SCLK	(1<<12)
#define WM_DO	(1<<13)
#define WM_SDIN	(1<<14)
#define WM_CSB	(1<<15)
/* end of for veriation board */
#endif

#ifdef _2WIRE
/* for demo board */
#define WM_SDIN	(0x1<<1)
#define WM_SCLK	(0x1)
#define READSTATUS() ((inpw(REG_SerialBusCR)&0x38)>>3)
#define HEADPHONE_PATH_1 (1<<15)
#define HEADPHONE_PATH_2 (1<<16)
/*--------*/
#endif

#define	WM_8000		(0xC<<1)
#define	WM_16000	(0xC<<1)
#define	WM_24000	0x0	
#define	WM_32000	(0xC<<1)
#define	WM_48000	0x0
           
#define	WM_11025	(0x11<<1)
#define	WM_22050	(0x10<<1)
#define	WM_44100	(0x11<<1)



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



                
#endif	/* _WM8751_H_ */
                


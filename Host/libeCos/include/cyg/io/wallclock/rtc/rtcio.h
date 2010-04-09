/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2003 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *     WBIO.h
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     This file contains I/O macros, and basic macros and inline functions.
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *     None
 *
 * HISTORY
 *     03/28/02		 Ver 1.0 Created by PC30 YCHuang
 *
 * REMARK
 *     None
 **************************************************************************/

#ifndef _WBIO_H
#define _WBIO_H

#define outpb(port,value)     (*((cyg_uint8 volatile *) (port))=value)
#define inpb(port)            (*((cyg_uint8 volatile *) (port)))
#define outphw(port,value)    (*((cyg_uint16 volatile *) (port))=value)
#define inphw(port)           (*((cyg_uint16 volatile *) (port)))
#define outpw(port,value)     (*((cyg_uint32 volatile *) (port))=value)
#define inpw(port)            (*((cyg_uint32 volatile *) (port)))

#define readb(addr)           (*(cyg_uint8 volatile *)(addr))
#define writeb(addr,x)        ((*(cyg_uint8 volatile *)(addr)) = (cyg_uint8 volatile)x)
#define readhw(addr)          (*(cyg_uint16 volatile *)(addr))
#define writehw(addr,x)       ((*(cyg_uint16 volatile *)(addr)) = (cyg_uint16 volatile)x)
#define readw(addr)           (*(cyg_uint32 volatile *)(addr))
#define writew(addr,x)        ((*(cyg_uint32 volatile *)(addr)) = (cyg_uint32 volatile)x)

#endif /* _WBIO_H */


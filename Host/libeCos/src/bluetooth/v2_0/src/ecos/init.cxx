//==========================================================================
//
//      ecos/init.cxx
//
//      Networking package initializer class
//
//==========================================================================
//####ECOSPDCOPYRIGHTBEGIN####
//
// Copyright (C) 2000, 2001, 2002 Red Hat, Inc.
// All Rights Reserved.
//
// Permission is granted to use, copy, modify and redistribute this
// file.
//
//####ECOSPDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-01-10
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#ifdef CYGPKG_NET_BLUEZ_STACK
// Network initialization

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

externC int bluez_init(void);
//#define NET_INIT CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_LIBC)
#define NET_INIT CYGBLD_ATTRIB_INIT_AFTER(CYG_INIT_LIBC)

// This is a dummy class just so we can execute the network package 
// initialization at it's proper priority

class bluez_init_class{
public:
    bluez_init_class(void){ 
        bluez_init();
    }
};

// And here's an instance of the class just to make the code run
static bluez_init_class _bluez_init NET_INIT;

#endif

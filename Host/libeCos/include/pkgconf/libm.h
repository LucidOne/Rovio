#ifndef CYGONCE_PKGCONF_LIBM_H
#define CYGONCE_PKGCONF_LIBM_H
/*
 * File <pkgconf/libm.h>
 *
 * This file is generated automatically by the configuration
 * system. It should not be edited. Any changes to this file
 * may be overwritten.
 */

/***** proc output start *****/
#include "pkgconf/system.h"
typedef enum {
    CYGNUM_LIBM_COMPAT_UNINIT= 0,  // Default state. DO NOT set it to this
    CYGNUM_LIBM_COMPAT_POSIX = 1,  // ANSI/POSIX 1003.1
    CYGNUM_LIBM_COMPAT_IEEE  = 2,  // IEEE-754
    CYGNUM_LIBM_COMPAT_XOPEN = 3,  // X/OPEN Portability guide issue 3
                                   // (XPG3)
    CYGNUM_LIBM_COMPAT_SVID  = 4   // System V Interface Definition 3rd
                                   // edition
} Cyg_libm_compat_t;

/****** proc output end ******/
#define CYGPKG_LIBM_COMPATIBILITY 1
#define CYGINT_LIBM_COMPAT 1
#define CYGINT_LIBM_COMPAT_1
#define CYGNUM_LIBM_COMPATIBILITY POSIX
#define CYGNUM_LIBM_COMPATIBILITY_POSIX
#define CYGPKG_LIBM_COMPATIBILITY_DEFAULT POSIX
#define CYGPKG_LIBM_COMPATIBILITY_DEFAULT_POSIX
#define CYGNUM_LIBM_COMPAT_DEFAULT CYGNUM_LIBM_COMPAT_POSIX
#define CYGNUM_LIBM_COMPAT_DEFAULT_CYGNUM_LIBM_COMPAT_POSIX
#define CYGFUN_LIBM_SVID3_scalb 1
#define CYGPKG_LIBM_THREAD_SAFETY 1
#define X_TLOSS 1.41484755040569E+16
#define CYGPKG_LIBM_OPTIONS 1

#endif

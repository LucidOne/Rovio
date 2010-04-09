/*
 * standalone.c - minimal bootstrap for C library
 * Copyright (C) 2000 ARM Limited.
 * All rights reserved.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 2006/01/23 11:21:16 $ 0
 * Revising $Author: xhchen $
 */

/*
 * This code defines a run-time environment for the C library.
 * Without this, the C startup code will attempt to use semi-hosting
 * calls to get environment information.
 */

#include <rt_misc.h>
#define MAX_STACK_SIZE (128*1024)

void _sys_exit(int return_code)
{
label:  goto label; /* endless loop */
}

void _ttywrch(int ch)
{
    char tempch = (char)ch;
    (void)tempch;
}


__value_in_regs struct __initial_stackheap 
__user_initial_stackheap(unsigned int R0, unsigned int SP, unsigned int R2, unsigned int SL)
{
	struct __initial_stackheap config;
    extern unsigned int Image$$ZI$$Limit;

	/*
		To place heap_base directly above the ZI area, use:
			extern unsigned int Image$$ZI$$Limit;
			config.heap_base = (unsigned int)&Image$$ZI$$Limit;
		(or &Image$$region_name$$ZI$$Limit for scatterloaded images)
	 */
	config.heap_base = (unsigned int)&Image$$ZI$$Limit;
	config.heap_limit = SP - MAX_STACK_SIZE;

	config.stack_base = SP;
	config.stack_limit = SP - MAX_STACK_SIZE;
    return config;
}


/* end of file standalone.c */

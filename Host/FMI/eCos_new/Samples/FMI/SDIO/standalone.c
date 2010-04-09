/*
 * standalone.c - minimal bootstrap for C library
 * Copyright (C) 2000 ARM Limited.
 * All rights reserved.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 2006/02/16 11:51:08 $ 0
 * Revising $Author: ypxin $
 */

/*
 * This code defines a run-time environment for the C library.
 * Without this, the C startup code will attempt to use semi-hosting
 * calls to get environment information.
 */
 extern unsigned int Image$$ZI$$Limit;

void _sys_exit(int return_code)
{
label:  goto label; /* endless loop */
}

void _ttywrch(int ch)
{
    char tempch = (char)ch;
    (void)tempch;
}


#if 1
__value_in_regs struct R0_R3 {unsigned heap_base, stack_base, heap_limit, stack_limit;} 
    __user_initial_stackheap(unsigned int R0, unsigned int SP, unsigned int R2, unsigned int SL)
{
    struct R0_R3 config;

    //config.heap_base = 0x00060000;
    config.heap_base = (unsigned int)&Image$$ZI$$Limit;
    config.stack_base = SP;

/*
To place heap_base directly above the ZI area, use:
    extern unsigned int Image$$ZI$$Limit;
    config.heap_base = (unsigned int)&Image$$ZI$$Limit;
(or &Image$$region_name$$ZI$$Limit for scatterloaded images)

To specify the limits for the heap & stack, use e.g:
    config.heap_limit = SL;
    config.stack_limit = SL;
*/

    return config;
}
#endif

/* end of file standalone.c */

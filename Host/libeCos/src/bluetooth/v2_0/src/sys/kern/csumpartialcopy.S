;/*
; *  linux/arch/arm/lib/csumpartialcopy.S
; *
; *  Copyright (C) 1995-1998 Russell King
; *
; * This program is free software; you can redistribute it and/or modify
; * it under the terms of the GNU General Public License version 2 as
; * published by the Free Software Foundation.
; */
;#include <linux/linkage.h>
;#include <asm/assembler.h>

		AREA |.text|, CODE, READONLY
		CODE32

;/* Function: __u32 csum_partial_copy_nocheck(const char *src, char *dst, int len, __u32 sum)
; * Params  : r0 = src, r1 = dst, r2 = len, r3 = checksum
; * Returns : r0 = new checksum
; */

		MACRO	
$__save_regs save_regs	
		stmfd	sp!, {r1, r4 - r8, fp, ip, lr, pc}
		MEND

		MACRO	
$__load_regs load_regs	$flags
		;LOADREGS($flags,fp,{r1, r4 - r8, fp, sp, pc})
		ldm$flags	fp,{r1, r4 - r8, fp, sp, pc}
		MEND

		MACRO	
$__load1b load1b $reg1
		ldrb	$reg1, [r0], #1
		MEND

		MACRO	
$__load2b load2b $reg1, $reg2
		ldrb	$reg1, [r0], #1
		ldrb	$reg2, [r0], #1
		MEND

		MACRO	
$__load1l load1l $reg1
		ldr	$reg1, [r0], #4
		MEND

		MACRO	
$__load2l load2l $reg1, $reg2
		ldr	$reg1, [r0], #4
		ldr	$reg2, [r0], #4
		MEND

		MACRO	
$__load4l	load4l $reg1, $reg2, $reg3, $reg4
		ldmia	r0!, {$reg1, $reg2, $reg3, $reg4}
		MEND

FN_ENTRY EQU	csum_partial_copy_nocheck

	include csumpartialcopygeneric.S

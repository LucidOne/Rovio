;/*
; *  linux/arch/arm/lib/memcpy.S
; *
; *  Copyright (C) 1995-1999 Russell King
; *
; * This program is free software; you can redistribute it and/or modify
; * it under the terms of the GNU General Public License version 2 as
; * published by the Free Software Foundation.
; *
; *  ASM optimised string functions
; */


	AREA CODE, READONLY

	MACRO
	LOADREGS  $cond, $base, $reglist

	ldm##cond	base,reglist
	MEND

	MACRO
	ENTER
	mov	ip,sp
	stmfd	sp!,{r4-r9,fp,ip,lr,pc}
	sub	fp,ip,#4
	MEND

	MACRO
	EXIT
		ldmea	fp, {r4 - r9, fp, sp, pc}
	MEND
	
	MACRO
	EXITEQ
	ldmeqea	fp, {r4 - r9, fp, sp, pc}
	MEND
	
	AREA memcopy, CODE, READONLY
	EXPORT  kmemcpy
;/*
; * Prototype: void memcpy(void *to,const void *from,unsigned long n);
; * ARM3: cant use memcopy here!!!
; */
;memmove
kmemcpy
		ENTER
		cmp	r1, r0
		bcc	%F19
		subs	r2, r2, #4
		blt	%F6
		ands	ip, r0, #3
		bne	%F7
		ands	ip, r1, #3
		bne	%F8

1		subs	r2, r2, #8
		blt	%F5
		subs	r2, r2, #0x14
		blt	%F3
2		ldmia	r1!,{r3 - r9, ip}
		stmia	r0!,{r3 - r9, ip}
		subs	r2, r2, #32
		bge	%B2
		cmn	r2, #16
		ldmgeia	r1!, {r3 - r6}
		stmgeia	r0!, {r3 - r6}
		subge	r2, r2, #0x10
3		adds	r2, r2, #0x14
4		ldmgeia	r1!, {r3 - r5}
		stmgeia	r0!, {r3 - r5}
		subges	r2, r2, #12
		bge	%B4
5		adds	r2, r2, #8
		blt	%F6
		subs	r2, r2, #4
		ldrlt	r3, [r1], #4
		ldmgeia	r1!, {r4, r5}
		strlt	r3, [r0], #4
		stmgeia	r0!, {r4, r5}
		subge	r2, r2, #4

6		adds	r2, r2, #4
		EXITEQ
		cmp	r2, #2
		ldrb	r3, [r1], #1
		ldrgeb	r4, [r1], #1
		ldrgtb	r5, [r1], #1
		strb	r3, [r0], #1
		strgeb	r4, [r0], #1
		strgtb	r5, [r0], #1
		EXIT

7		rsb	ip, ip, #4
		cmp	ip, #2
		ldrb	r3, [r1], #1
		ldrgeb	r4, [r1], #1
		ldrgtb	r5, [r1], #1
		strb	r3, [r0], #1
		strgeb	r4, [r0], #1
		strgtb	r5, [r0], #1
		subs	r2, r2, ip
		blt	%B6
		ands	ip, r1, #3
		beq	%B1

8		bic	r1, r1, #3
		ldr	r7, [r1], #4
		cmp	ip, #2
		bgt	%F15
		beq	%F11
		cmp	r2, #12
		blt	%F10
		sub	r2, r2, #12
9		mov	r3, r7, lsr #8
		ldmia	r1!, {r4 - r7}
		orr	r3, r3, r4, lsl #24
		mov	r4, r4, lsr #8
		orr	r4, r4, r5, lsl #24
		mov	r5, r5, lsr #8
		orr	r5, r5, r6, lsl #24
		mov	r6, r6, lsr #8
		orr	r6, r6, r7, lsl #24
		stmia	r0!, {r3 - r6}
		subs	r2, r2, #16
		bge	%B9
		adds	r2, r2, #12
		blt	%F100
10		mov	r3, r7, lsr #8
		ldr	r7, [r1], #4
		subs	r2, r2, #4
		orr	r3, r3, r7, lsl #24
		str	r3, [r0], #4
		bge	%B10
100		sub	r1, r1, #3
		b	%B6

11		cmp	r2, #12
		blt	%F13
		sub	r2, r2, #12
12		mov	r3, r7, lsr #16
		ldmia	r1!, {r4 - r7}
		orr	r3, r3, r4, lsl #16
		mov	r4, r4, lsr #16
		orr	r4, r4, r5, lsl #16
		mov	r5, r5, lsr #16
		orr	r5, r5, r6, lsl #16
		mov	r6, r6, lsr #16
		orr	r6, r6, r7, lsl #16
		stmia	r0!, {r3 - r6}
		subs	r2, r2, #16
		bge	%B12
		adds	r2, r2, #12
		blt	%F14
13		mov	r3, r7, lsr #16
		ldr	r7, [r1], #4
		subs	r2, r2, #4
		orr	r3, r3, r7, lsl #16
		str	r3, [r0], #4
		bge	%B13
14		sub	r1, r1, #2
		b	%B6

15		cmp	r2, #12
		blt	%F17
		sub	r2, r2, #12
16		mov	r3, r7, lsr #24
		ldmia	r1!, {r4 - r7}
		orr	r3, r3, r4, lsl #8
		mov	r4, r4, lsr #24
		orr	r4, r4, r5, lsl #8
		mov	r5, r5, lsr #24
		orr	r5, r5, r6, lsl #8
		mov	r6, r6, lsr #24
		orr	r6, r6, r7, lsl #8
		stmia	r0!, {r3 - r6}
		subs	r2, r2, #16
		bge	%B16
		adds	r2, r2, #12
		blt	%F18
17		mov	r3, r7, lsr #24
		ldr	r7, [r1], #4
		subs	r2, r2, #4
		orr	r3, r3, r7, lsl #8
		str	r3, [r0], #4
		bge	%B17
18		sub	r1, r1, #1
		b	%B6


19		add	r1, r1, r2
		add	r0, r0, r2
		subs	r2, r2, #4
		blt	%F24
		ands	ip, r0, #3
		bne	%F25
		ands	ip, r1, #3
		bne	%F26

20		subs	r2, r2, #8
		blt	%F23
		subs	r2, r2, #0x14
		blt	%F22
21		ldmdb	r1!, {r3 - r9, ip}
		stmdb	r0!, {r3 - r9, ip}
		subs	r2, r2, #32
		bge	%B21
22		cmn	r2, #16
		ldmgedb	r1!, {r3 - r6}
		stmgedb	r0!, {r3 - r6}
		subge	r2, r2, #16
		adds	r2, r2, #20
		ldmgedb	r1!, {r3 - r5}
		stmgedb	r0!, {r3 - r5}
		subge	r2, r2, #12
23		adds	r2, r2, #8
		blt	%F24
		subs	r2, r2, #4
		ldrlt	r3, [r1, #-4]!
		ldmgedb	r1!, {r4, r5}
		strlt	r3, [r0, #-4]!
		stmgedb	r0!, {r4, r5}
		subge	r2, r2, #4

24		adds	r2, r2, #4
		EXITEQ
		cmp	r2, #2
		ldrb	r3, [r1, #-1]!
		ldrgeb	r4, [r1, #-1]!
		ldrgtb	r5, [r1, #-1]!
		strb	r3, [r0, #-1]!
		strgeb	r4, [r0, #-1]!
		strgtb	r5, [r0, #-1]!
		EXIT

25		cmp	ip, #2
		ldrb	r3, [r1, #-1]!
		ldrgeb	r4, [r1, #-1]!
		ldrgtb	r5, [r1, #-1]!
		strb	r3, [r0, #-1]!
		strgeb	r4, [r0, #-1]!
		strgtb	r5, [r0, #-1]!
		subs	r2, r2, ip
		blt	%B24
		ands	ip, r1, #3
		beq	%B20

26		bic	r1, r1, #3
		ldr	r3, [r1], #0
		cmp	ip, #2
		blt	%F34
		beq	%F30
		cmp	r2, #12
		blt	%F28
		sub	r2, r2, #12
27		mov	r7, r3, lsl #8
		ldmdb	r1!, {r3, r4, r5, r6}
		orr	r7, r7, r6, lsr #24
		mov	r6, r6, lsl #8
		orr	r6, r6, r5, lsr #24
		mov	r5, r5, lsl #8
		orr	r5, r5, r4, lsr #24
		mov	r4, r4, lsl #8
		orr	r4, r4, r3, lsr #24
		stmdb	r0!, {r4, r5, r6, r7}
		subs	r2, r2, #16
		bge	%B27
		adds	r2, r2, #12
		blt	%F29
28		mov	ip, r3, lsl #8
		ldr	r3, [r1, #-4]!
		subs	r2, r2, #4
		orr	ip, ip, r3, lsr #24
		str	ip, [r0, #-4]!
		bge	%B28
29		add	r1, r1, #3
		b	%B24

30		cmp	r2, #12
		blt	%F32
		sub	r2, r2, #12
31		mov	r7, r3, lsl #16
		ldmdb	r1!, {r3, r4, r5, r6}
		orr	r7, r7, r6, lsr #16
		mov	r6, r6, lsl #16
		orr	r6, r6, r5, lsr #16
		mov	r5, r5, lsl #16
		orr	r5, r5, r4, lsr #16
		mov	r4, r4, lsl #16
		orr	r4, r4, r3, lsr #16
		stmdb	r0!, {r4, r5, r6, r7}
		subs	r2, r2, #16
		bge	%B31
		adds	r2, r2, #12
		blt	%F33
32		mov	ip, r3, lsl #16
		ldr	r3, [r1, #-4]!
		subs	r2, r2, #4
		orr	ip, ip, r3, lsr #16
		str	ip, [r0, #-4]!
		bge	%B32
33		add	r1, r1, #2
		b	%B24

34		cmp	r2, #12
		blt	%F36
		sub	r2, r2, #12
35		mov	r7, r3, lsl #24
		ldmdb	r1!, {r3, r4, r5, r6}
		orr	r7, r7, r6, lsr #8
		mov	r6, r6, lsl #24
		orr	r6, r6, r5, lsr #8
		mov	r5, r5, lsl #24
		orr	r5, r5, r4, lsr #8
		mov	r4, r4, lsl #24
		orr	r4, r4, r3, lsr #8
		stmdb	r0!, {r4, r5, r6, r7}
		subs	r2, r2, #16
		bge	%B35
		adds	r2, r2, #12
		blt	%F37
36		mov	ip, r3, lsl #24
		ldr	r3, [r1, #-4]!
		subs	r2, r2, #4
		orr	ip, ip, r3, lsr #8
		str	ip, [r0, #-4]!
		bge	%B36
37		add	r1, r1, #1
		b	%B24

		align

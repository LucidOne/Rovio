;/*
; *  linux/arch/arm/lib/csumpartial.S
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

;/*
; * Function: __u32 csum_partial(const char *src, int len, __u32 sum)
; * Params  : r0 = buffer, r1 = len, r2 = checksum
; * Returns : r0 = new checksum
; */

buf	EQU	r0
len	EQU	r1
sum	EQU	r2
td0	EQU	r3
td1	EQU	r4	; save before use
td2	EQU	r5	; save before use
td3	EQU	lr

zero
		mov	r0, r2	;r2==sum
		add	sp, sp, #4
		ldr	pc, [sp], #4

		;/*
		; * Handle 0 to 7 bytes, with any alignment of source and
		; * destination pointers.  Note that when we get here, C = 0
		; */
		;r1==len
less8	
		teq	r1, #0			; check for zero count
		beq	zero

		;/* we must have at least one byte. */
		;r0==buf
		tst	r0, #1			; odd address?
		ldrneb	r3, [r0], #1	;r0==buf r3==td0
		subne	r1, r1, #1	;two r1==len
		adcnes	r2, r2, r3, lsl #8	;two r2==sum r3==td0

less4		
		tst	r1, #6	;r1==len
		beq	less8_byte

		;/* we are now half-word aligned */

less8_wordlp
 IF :DEF: __ARM_ARCH_4__
		ldrh	r3, [r0], #2	;r0==buf r3==td0
		sub	r1, r1, #2	;two r1==len
 ELSE
		ldrb	r3, [r0], #1	;r0==buf,r3==td0
		ldrb	lr, [r0], #1	;r0==buf,lr==td3
		sub	r1, r1, #2	;two r1==len
		orr	r3, r3, lr, lsl #8	;two r3==td0,lr==td3
 ENDIF
		adcs	r2, r2, r3	;two r2==sum r3==td0
		tst	r1, #6	;r1==len
		bne	less8_wordlp
		
		;r1==len
less8_byte	
		tst	r1, #1			; odd number of bytes
		;r0==buf,r3==td0
		ldrneb	r3, [r0], #1		; include last byte
		;two r2==sum,r3==td0
		adcnes	r2, r2, r3		; update checksum
		
		;r2==sum
done	
		adc	r0, r2, #0		; collect up the last carry
		ldr	r3, [sp], #4	;r3==td0
		;r3==td0
		tst	r3, #1			; check buffer alignment
		;r3==td0
		movne	r3, r0, lsl #8		; rotate checksum by 8 bits
		orrne	r0, r3, r0, lsr #24	;r3==td0
		ldr	pc, [sp], #4		; return
not_aligned	
		;r0==buf
		tst	r0, #1			;odd address
		;r0==buf,r3==td0
		ldrneb	r3, [r0], #1		; make even
		subne	r1, r1, #1	;two r1==len
		;two r2==sum,r3==td0
		adcnes	r2, r2, r3, lsl #8	; update checksum
		
		;r0==buf
		tst	r0, #2			; 32-bit aligned?
 IF :DEF:  __ARM_ARCH_4__
		;r0==buf,r3==td0
		ldrneh	r3, [r0], #2		; make 32-bit aligned
		subne	r1, r1, #2	;two r1==len
 ELSE
		ldrneb	r3, [r0], #1	;r0==buf,r3==td0
		ldrneb	ip, [r0], #1	;r0==buf
		subne	r1, r1, #2	;two r1==len
		orrne	r3, r3, ip, lsl #8	;two r3==td0
 ENDIF
		;two r2==sum,r3==td0
		adcnes	r2, r2, r3		; update checksum
		mov	pc, lr
	
	EXPORT	csum_partial
	
csum_partial		;ENTRY	
		stmfd	sp!, {r0, lr}	;r0==buf
		;r1==len
		cmp	r1, #8			; Ensure that we have at least
		blo	less8			; 8 bytes to copy.
		
		;two r2==sum
		adds	r2, r2, #0		; C = 0
		;r0==buf
		tst	r0, #3			; Test destination alignment
		blne	not_aligned		; aligh destination, return here

1		
		bics	ip, r1, #31	;r1==len
		beq	%F3

		stmfd	sp!, {r4 - r5}
2		
		;r0==buf,r3==td0,r4==td1,r5==td2,lr==td3
		ldmia	r0!, {r3, r4, r5, lr}
		adcs	r2, r2, r3	;two r2==sum,r3==td0
		adcs	r2, r2, r4	;two r2==sum,r4==td1
		adcs	r2, r2, r5	;two r2==sum,r5==td2
		adcs	r2, r2, lr	;two r2==sum,lr==td3
		;r0==buf,r3==td0,r4==td1,r5==td2,lr==td3
		ldmia	r0!, {r3, r4, r5, lr}
		adcs	r2, r2, r3	;two r2==sum,r3==td0
		adcs	r2, r2, r4	;two r2==sum,r4==td1
		adcs	r2, r2, r5	;two r2==sum,r5==td2
		adcs	r2, r2, lr	;two r2==sum,lr==td3
		sub	ip, ip, #32
		teq	ip, #0
		bne	%B2
		ldmfd	sp!, {r4 - r5}

3		
		;r1==len
		tst	r1, #0x1c		;should not change C
		beq	less4

4		;r0==buf,r3==td0
		ldr	r3, [r0], #4
		sub	r1, r1, #4	;two r1==len
		adcs	r2, r2, r3	;two r2==sum,r3==td0
		tst	r1, #0x1c	;r1==len
		bne	%B4
		b	less4

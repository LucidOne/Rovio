;/*
; *  linux/arch/arm/lib/csumpartialcopygeneric.S
; *
; *  Copyright (C) 1995-2001 Russell King
; *
; * This program is free software; you can redistribute it and/or modify
; * it under the terms of the GNU General Public License version 2 as
; * published by the Free Software Foundation.
; */

;/*
; * unsigned int
; * csum_partial_copy_xxx(const char *src, char *dst, int len, int sum, )
; *  r0 = src, r1 = dst, r2 = len, r3 = sum
; *  Returns : r0 = checksum
; *
; * Note that 'tst' and 'teq' preserve the carry flag.
; */

src	EQU	r0
dst	EQU	r1
len	EQU	r2
sum	EQU	r3

zero		
		mov	r0, r3	;r3==sum
		load_regs	ea

		;/*
		; * Align an unaligned destination pointer.  We know that
		; * we have >= 8 bytes here, so we don't need to check
		; * the length.  Note that the source pointer hasn't been
		; * aligned yet.
		; */
dst_unaligned
		tst	r1, #1	;r1==dst
		beq	dst_16bit

		load1b	ip
		sub	r2, r2, #1	;two r2==len
		;two r3 == sum
		adcs	r3, r3, ip, lsl #8	; update checksum
		strb	ip, [r1], #1	;r1==dst
		tst	r1, #2	;r1==dst
		moveq	pc, lr			; dst is now 32bit aligned

dst_16bit	
		load2b	r8, ip
		sub	r2, r2, #2	;two r2==len
		;two r3==sum
		adcs	r3, r3, r8
		strb	r8, [r1], #1	;r1==dst
		;two r3==sum
		adcs	r3, r3, ip, lsl #8
		strb	ip, [r1], #1	;r1==dst
		mov	pc, lr			; dst is now 32bit aligned

		;/*
		; * Handle 0 to 7 bytes, with any alignment of source and
		; * destination pointers.  Note that when we get here, C = 0
		; */
		;r2==len
less8		teq	r2, #0			; check for zero count
		beq	zero

		;/* we must have at least one byte. */
		;r1==dst
		tst	r1, #1			; dst 16-bit aligned
		beq	less8_aligned

		;/* Align dst */
		load1b	ip
		sub	r2, r2, #1	;two r2==len
		;two r3==sum
		adcs	r3, r3, ip, lsl #8	; update checksum
		strb	ip, [r1], #1	;r1==dst
		tst	r2, #6	;r2==len
		beq	less8_byteonly

1		
		load2b	r8, ip
		sub	r2, r2, #2	;two r2==len
		
		;two r3==sum
		adcs	r3, r3, r8
		strb	r8, [r1], #1	;r1==dst
		
		;two r3==sum
		adcs	r3, r3, ip, lsl #8
		strb	ip, [r1], #1	;r1==dst
less8_aligned	
		tst	r2, #6	;r2==len
		bne	%B1
less8_byteonly
		tst	r2, #1	;r2==len
		beq	done
		load1b	r8
		
		;two r3==sum
		adcs	r3, r3, r8		; update checksum
		strb	r8, [r1], #1	;r1==dst
		b	done
		
		EXPORT	csum_partial_copy_nocheck
		EXPORT	csum_partial_copy_from_user
csum_partial_copy_nocheck
csum_partial_copy_from_user
		mov	ip, sp
		save_regs
		sub	fp, ip, #4
		
		;r2==len
		cmp	r2, #8			; Ensure that we have at least
		blo	less8			; 8 bytes to copy.
		
		;two r3==sum
		adds	r3, r3, #0		; C = 0
		
		;r1==dst
		tst	r1, #3			; Test destination alignment
		blne	dst_unaligned		; align destination, return here

		;/*
		; * Ok, the dst pointer is now 32bit aligned, and we know
		; * that we must have more than 4 bytes to copy.  Note
		; * that C contains the carry from the dst alignment above.
		; */
		
		;r0==src
		tst	r0, #3			; Test source alignment
		bne	src_not_aligned

		;/* Routine for src & dst aligned */

		bics	ip, r2, #15	;r2==len
		beq	%F2

1		load4l	r4, r5, r6, r7
		stmia	r1!, {r4, r5, r6, r7}	;r1==dst
		
		;two r3==sum
		adcs	r3, r3, r4
		;two r3==sum
		adcs	r3, r3, r5
		;two r3==sum
		adcs	r3, r3, r6
		;two r3==sum
		adcs	r3, r3, r7
		sub	ip, ip, #16
		teq	ip, #0
		bne	%B1

2		
		ands	ip, r2, #12	;r2==len
		beq	%F4
		tst	ip, #8
		beq	%F3
		load2l	r4, r5
		stmia	r1!, {r4, r5}	;r1 == dst
		
		;two r3==sum
		adcs	r3, r3, r4
		;two r3==sum
		adcs	r3, r3, r5
		tst	ip, #4
		beq	%F4

3		
		load1l	r4
		str	r4, [r1], #4	;r1==dst
		adcs	r3, r3, r4	;two r3==sum

4		
		ands	r2, r2, #3	;two r2==len
		beq	done
		load1l	r4
		tst	r2, #2	;r2==len
		beq	exit
		
		;two r3==sum
		adcs	r3, r3, r4, lsl #16
		strb	r4, [r1], #1	;r1==dst
		mov	r4, r4, lsr #8
		strb	r4, [r1], #1	;r1==dst
		mov	r4, r4, lsr #8
exit	
		tst	r2, #1	;r2==len
		strneb	r4, [r1], #1	;r1==dst
		andne	r4, r4, #255
		adcnes	r3, r3, r4	;two r3==sum

		;/*
		; * If the dst pointer was not 16-bit aligned, we
		; * need to rotate the checksum here to get around
		; * the inefficient byte manipulations in the
		; * architecture independent code.
		; */
done		
		adc	r0, r3, #0	;r3 == sum
		ldr	r3, [sp, #0]		; dst	;r3 == sum
		tst	r3, #1	;r3 == sum
		movne	r3, r0, lsl #8	;r3 == sum
		orrne	r0, r3, r0, lsr #24	;r3 == sum
		load_regs	ea

src_not_aligned
		;two r3 == sum
		adc	r3, r3, #0		; include C from dst alignment
		and	ip, r0, #3		;r0==src
		bic	r0, r0, #3	;two r0==src
		load1l	r4
		cmp	ip, #2
		beq	src2_aligned
		bhi	src3_aligned
		mov	r4, r4, lsr #8		; C = 0
		bics	ip, r2, #15	;r2==len
		beq	%F2
1		
		load4l	r5, r6, r7, r8
		orr	r4, r4, r5, lsl #24
		mov	r5, r5, lsr #8
		orr	r5, r5, r6, lsl #24
		mov	r6, r6, lsr #8
		orr	r6, r6, r7, lsl #24
		mov	r7, r7, lsr #8
		orr	r7, r7, r8, lsl #24
		stmia	r1!, {r4, r5, r6, r7}	;r1==dst
		
		;two r3 == sum
		adcs	r3, r3, r4
		;two r3 == sum
		adcs	r3, r3, r5
		;two r3 == sum
		adcs	r3, r3, r6
		;two r3 == sum
		adcs	r3, r3, r7
		mov	r4, r8, lsr #8
		sub	ip, ip, #16
		teq	ip, #0
		bne	%B1
2		
		ands	ip, r2, #12	;r2==len
		beq	%F4
		tst	ip, #8
		beq	%F3
		load2l	r5, r6
		orr	r4, r4, r5, lsl #24
		mov	r5, r5, lsr #8
		orr	r5, r5, r6, lsl #24
		stmia	r1!, {r4, r5}	;r1==dst
			
		;two r3 == sum
		adcs	r3, r3, r4
		
		;two r3 == sum
		adcs	r3, r3, r5
		mov	r4, r6, lsr #8
		tst	ip, #4
		beq	%F4
3		
		load1l	r5
		orr	r4, r4, r5, lsl #24
		str	r4, [r1], #4	;r1==dst
		
		;two r3 == sum
		adcs	r3, r3, r4
		mov	r4, r5, lsr #8
4		
		ands	r2, r2, #3	;two r2==len
		beq	done
		tst	r2, #2	;r2==len
		beq	exit
		;two r3 == sum
		adcs	r3, r3, r4, lsl #16
		strb	r4, [r1], #1	;r1==dst
		mov	r4, r4, lsr #8
		strb	r4, [r1], #1	;r1==dst
		mov	r4, r4, lsr #8
		b	exit

src2_aligned
		mov	r4, r4, lsr #16
		;two r3 == sum
		adds	r3, r3, #0
		bics	ip, r2, #15	;r2==len
		beq	%F2
1		
		load4l	r5, r6, r7, r8
		orr	r4, r4, r5, lsl #16
		mov	r5, r5, lsr #16
		orr	r5, r5, r6, lsl #16
		mov	r6, r6, lsr #16
		orr	r6, r6, r7, lsl #16
		mov	r7, r7, lsr #16
		orr	r7, r7, r8, lsl #16
		stmia	r1!, {r4, r5, r6, r7}	;r1==dst
		adcs	r3, r3, r4	;two r3 == sum
		adcs	r3, r3, r5	;two r3 == sum
		adcs	r3, r3, r6	;two r3 == sum
		adcs	r3, r3, r7	;two r3 == sum
		mov	r4, r8, lsr #16
		sub	ip, ip, #16
		teq	ip, #0
		bne	%B1
2		
		ands	ip, r2, #12	;r2==len
		beq	%F4
		tst	ip, #8
		beq	%F3
		load2l	r5, r6
		orr	r4, r4, r5, lsl #16
		mov	r5, r5, lsr #16
		orr	r5, r5, r6, lsl #16
		stmia	r1!, {r4, r5}	;r1==dst
		adcs	r3, r3, r4	;two r3 == sum
		adcs	r3, r3, r5	;two r3 == sum
		mov	r4, r6, lsr #16
		tst	ip, #4
		beq	%F4
3		
		load1l	r5
		orr	r4, r4, r5, lsl #16
		str	r4, [r1], #4	;r1==dst
		adcs	r3, r3, r4	;two r3 == sum
		mov	r4, r5, lsr #16
4		
		ands	r2, r2, #3	;two r2==len
		beq	done
		tst	r2, #2	;r2==len
		beq	exit
		adcs	r3, r3, r4, lsl #16	;two r3 == sum
		strb	r4, [r1], #1	;r1==dst
		mov	r4, r4, lsr #8
		strb	r4, [r1], #1	;r1==dst
		tst	r2, #1	;r2==len
		beq	done
		load1b	r4
		b	exit

src3_aligned	
		mov	r4, r4, lsr #24
		adds	r3, r3, #0	;two r3 == sum
		bics	ip, r2, #15	;r2==len
		beq	%F2
1		
		load4l	r5, r6, r7, r8
		orr	r4, r4, r5, lsl #8
		mov	r5, r5, lsr #24
		orr	r5, r5, r6, lsl #8
		mov	r6, r6, lsr #24
		orr	r6, r6, r7, lsl #8
		mov	r7, r7, lsr #24
		orr	r7, r7, r8, lsl #8
		stmia	r1!, {r4, r5, r6, r7}	;r1==dst
		adcs	r3, r3, r4	;two r3 == sum
		adcs	r3, r3, r5	;two r3 == sum
		adcs	r3, r3, r6	;two r3 == sum
		adcs	r3, r3, r7	;two r3 == sum
		mov	r4, r8, lsr #24
		sub	ip, ip, #16
		teq	ip, #0
		bne	%B1
2		
		ands	ip, r2, #12	;r2==len
		beq	%F4
		tst	ip, #8
		beq	%F3
		load2l	r5, r6
		orr	r4, r4, r5, lsl #8
		mov	r5, r5, lsr #24
		orr	r5, r5, r6, lsl #8
		stmia	r1!, {r4, r5}	;r1==dst
		adcs	r3, r3, r4	;two r3 == sum
		adcs	r3, r3, r5	;two r3 == sum
		mov	r4, r6, lsr #24
		tst	ip, #4
		beq	%F4
3		
		load1l	r5
		orr	r4, r4, r5, lsl #8
		str	r4, [r1], #4	;r1==dst
		adcs	r3, r3, r4	;two r3 == sum
		mov	r4, r5, lsr #24
4		
		ands	r2, r2, #3	;two r2==len
		beq	done
		tst	r2, #2	;r2==len
		beq	exit
		adcs	r3, r3, r4, lsl #16	;two r3 == sum
		strb	r4, [r1], #1	;r1==dst
		load1l	r4
		strb	r4, [r1], #1	;r1==dst
		adcs	r3, r3, r4, lsl #24	;two r3 == sum
		mov	r4, r4, lsr #8
		b	exit

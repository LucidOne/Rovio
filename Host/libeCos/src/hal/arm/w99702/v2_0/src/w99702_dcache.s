	AREA MEM_INIT, CODE, READONLY
	
	EXPORT	sys_flush_and_clean_dcache

sys_flush_and_clean_dcache

	MOV	r1, #0x0				; initialize segment counter
outer_loop
	MOV	r0, #0x0				; initialize line counter
inner_loop
	ORR	r2, r1, r0				; generate segment and line address
	MCR	p15, 0, r2, c7, c14, 2	; clean and flush the line
	ADD	r0, r0, #0x20			; increment to next line
	CMP	r0, #0x800				; complete all entries in one segment for 4KB dcache
	BNE	inner_loop				; if NOT branch back to inner_loop
	ADD	r1, r1, #0x40000000		; increment segment counter
	CMP	r1, #0x0				; complete all segments
	BNE	outer_loop				; if NOT branch back to  outer_loop	
	
 	BX  r14

		
	END
	
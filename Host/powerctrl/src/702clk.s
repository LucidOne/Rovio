CLKCON	EQU	0x7FF00204
SDCON	EQU	0x3FFF0004
SDTIME0	EQU	0x3FFF0010

	EXPORT	asm_set_clk	; r0 = &clk_config 

	AREA w99702clk_config, CODE, READONLY
asm_set_clk
	STMDB	r13!, {r3-r11, r14}
	
;	MRS		r11, CPSR
;	MOV		r2, r11
;	LDR		r1, =0x80
;	ORR		r11, r2, r1
;	MSR		CPSR_c, r11

;	LDR		r11, =0x1000
;0	
;	SUBS	r11, r11, #1
;	BNE		%B0
	
		
	LDR		r11, =CLKCON
	LDR		r12, =SDCON
	LDR		r14, =SDTIME0
	LDMIA	r0, {r3-r10}
	STR		r5, [r11, #8] ; set pll first to make sure it is stable

	LDR		r0, =0x50
1
	SUBS	r0, r0, #1
	BNE		%B1
	
	ADD		r11, r11, #4	
	STMIA	r11, {r4-r8}
	STR		r9 , [r12]
	STR		r10, [r14]
	
	LDR		r0, =0x400
2	
	SUBS	r0, r0, #1
	BNE		%B2


	
	STR		r3, [r11, #-4]	
	
	
	

;	MSR		CPSR_c, r2


	LDMIA	r13!, {r3-r11, pc}
	END
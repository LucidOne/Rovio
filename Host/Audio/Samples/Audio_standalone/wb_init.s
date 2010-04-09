

	AREA WB_INIT, CODE, READONLY

	ENTRY

	LDR	PC, INT_Reset_Addr
	LDR	PC, Undef_Instruction_Addr
	LDR	PC, Software_INT_Addr
	LDR	PC, Prefetch_Abort_Addr
	LDR	PC, Data_Abort_Addr
	LDR	PC, Reserved_Addr
	LDR	PC, INT_IRQ_Addr
	LDR	PC, INT_FIQ_Addr

;	IMPORT	IRQ_Handler
;	IMPORT	FIQ_Handler

INT_Reset_Addr				DCD		Reset_Handler
Undef_Instruction_Addr		DCD		Undef_Handler
Software_INT_Addr			DCD		SWI_Handler
Prefetch_Abort_Addr			DCD		IAbort_Handler
Data_Abort_Addr				DCD		DABort_Handler
Reserved_Addr				DCD		Reserved_Handler
INT_IRQ_Addr				DCD		IRQ_Handler
INT_FIQ_Addr				DCD		FIQ_Handler


;-------------------
; Exception handler
;-------------------
Undef_Handler
	B	Undef_Handler

SWI_Handler
	;B	SWI_Handler
	MOV		r0, #0		; Pretend it was all okay
	MOVS	pc, lr		; just ignore SWI

IAbort_Handler
	B	IAbort_Handler

DABort_Handler
	B	DABort_Handler

Reserved_Handler
	B	Reserved_Handler

IRQ_Handler
	B	IRQ_Handler

FIQ_Handler
	B	FIQ_Handler


;--------------------------------------------
; Mode bits and interrupt flag (I&F) defines
;--------------------------------------------
USR_MODE	EQU		0x10
FIQ_MODE	EQU		0x11
IRQ_MODE	EQU		0x12
SVC_MODE	EQU		0x13
ABT_MODE	EQU		0x17
UDF_MODE	EQU		0x1B
SYS_MODE	EQU		0x1F

I_BIT		EQU		0x80
F_BIT		EQU		0x40

;----------------------------
; System / User Stack Memory
;----------------------------
FIQ_Stack_Size		EQU		256
IRQ_Stack_Size		EQU		256
SVC_Stack_Size		EQU		256
ABT_Stack_Size		EQU		64
UDF_Stack_Size		EQU		64
USR_Stack_Size		EQU		1024
SYS_Stack_Size		EQU		256


Stack_Base			EQU		0x100000
FIQ_Stack_Offset	EQU		Stack_Base
IRQ_Stack_Offset	EQU		FIQ_Stack_Offset - FIQ_Stack_Size
SVC_Stack_Offset	EQU		IRQ_Stack_Offset - IRQ_Stack_Size
ABT_Stack_Offset	EQU		SVC_Stack_Offset - SVC_Stack_Size
UDF_Stack_Offset	EQU		ABT_Stack_Offset - ABT_Stack_Size
USR_Stack_Offset	EQU		UDF_Stack_Offset - UDF_Stack_Size
SYS_Stack_Offset	EQU		USR_Stack_Offset - USR_Stack_Size

;--------------------------------
; AIC Control Registers
;--------------------------------
AIC_MDCR		EQU		0x7FFC3124	; Mask Disable Control Register
AIC_MASKALL		EQU		0x3FFFFE

	EXPORT	Reset_Handler	
Reset_Handler

	LDR r0, =0x0
	LDR r1, =0x0
	LDR r2, =0x0
	LDR r3, =0x0
	LDR r4, =0x0
	LDR r5, =0x0
	LDR r6, =0x0
	LDR r7, =0x0
	LDR r8, =0x0
	LDR r9, =0x0
	LDR r10, =0x0
	LDR r11, =0x0
	LDR r12, =0x0

;-----------------------------------------
; Disable Interrupt, This is for safe ...
;-----------------------------------------
	LDR	r0, =AIC_MDCR
	LDR	r1, =AIC_MASKALL
	STR	r1, [r0]
	MRS	r0, CPSR
	ORR	r0, r0, #0xC0
	MSR	CPSR_c, r0

;------------------------------------------------------
; Set mode to SVC, interrupts disabled (just paranoid)
;------------------------------------------------------
	MRS   r0, cpsr
	BIC   r0, r0, #0x1F
	ORR   r0, r0, #0xD3
	MSR   cpsr_fc, r0

;------------------------------------------------------
; Set the high exception vector of CP15 control bit    
;------------------------------------------------------        
	MRC	p15, 0, r0, c1, c0, 0  ; read control register         
    BIC r0, r0, #0x2000        ; clear bit 13                  
    MCR	p15, 0, r0, c1, c0, 0  ; write control register,  

;--------------------------------
; Initial Stack Pointer register
;--------------------------------
INIT_STACK
	MSR	CPSR_c, #FIQ_MODE | I_BIT | F_BIT
	LDR	sp, =FIQ_Stack_Offset

	MSR	CPSR_c, #IRQ_MODE | I_BIT | F_BIT
	LDR	sp, =IRQ_Stack_Offset

	;MSR	CPSR_c, #SVC_MODE | I_BIT | F_BIT
	;LDR	sp, =SVC_Stack_Offset

	MSR	CPSR_c, #ABT_MODE | I_BIT | F_BIT
	LDR	sp, =ABT_Stack_Offset

	MSR	CPSR_c, #UDF_MODE | I_BIT | F_BIT
	LDR	sp, =UDF_Stack_Offset

	MSR	CPSR_c, #SYS_MODE | I_BIT | F_BIT
	LDR	sp, =SYS_Stack_Offset

	MSR	CPSR_c, #SVC_MODE | I_BIT | F_BIT
	LDR	sp, =SVC_Stack_Offset

	;MSR	CPSR_c, #USR_MODE | F_BIT
	;LDR	sp, =USR_Stack_Offset

;-------------------------------------------------------------------
; Initial memory system (Mask these lines if use 1Mx16
;-------------------------------------------------------------------
	;LDR		r9, =0x30000000			; SDRAM control registers
	;LDR		r10, =SDI_PARAMETERS	; Parameter table
	;LDMIA	r10!, {r0-r5}			; Load parameter table into registers
	;STMIA	r9!, {r0-r5}			; Store parameter table value at correct address

;----------------------------------------------------------------------
; Initial Critical I/O devices
;----------------------------------------------------------------------
	LDR r0, =0x7ff00200   ; CLK_BA
	LDR r1, =0x7F0F7CD6   ; Enable all engine clock
    STR r1, [r0, #0x04]

	LDR r2, =0x7ff00000	  ; GCR_BA
	LDR r3, =0x00040000
	STR r3, [r2, #0x20]   ; enable the UART0 interfcae pins

	IMPORT	__main
;----------------------------
; enter the C code
;----------------------------
	B	__main

	EXPORT	SDI_PARAMETERS
SDI_PARAMETERS
SDICEN_Value	DCD		0x00000002		; SDRAM controller enable
SDICON_Value	DCD		0x110080FF		; auto pre-charge, clock enable
SDCONF0_Value	DCD		0x000C0000		; DRAM: base=0x0, 2M*32
SDCONF1_Value	DCD		0x00840000		; DRAm: BASE=0X800000, 1M*16
;SDTIME0_Value	DCD		0xC0008B6E		; default value
;SDTIME1_Value	DCD		0xC0008B6E		; default value
SDTIME0_Value	DCD		0x80005829		; default value
SDTIME1_Value	DCD		0x80005829		; default value


	END





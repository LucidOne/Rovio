/*
 * linux/asm/assembler.h
 *
 * This file contains arm architecture specific defines
 * for the different processors.
 *
 * Do not include any C declarations in this file - it is included by
 * assembler source.
 */

/*
 * Endian independent macros for shifting bytes within registers.
 */
#ifndef __ARMEB__
#define pull            lsr
#define push            lsl
#define byte(x)         (x*8)
#else
#define pull            lsl
#define push            lsr
#define byte(x)         ((3-x)*8)
#endif

#ifdef __STDC__
#define LOADREGS(cond, base, reglist...)\
	ldm##cond	base,reglist
#else
#define LOADREGS(cond, base, reglist...)\
	ldm/**/cond	base,reglist
#endif

/*
 * Build a return instruction for this processor type.
 */
#define RETINSTR(instr, regs...)\
	instr	regs

/*
 * Save the current IRQ state and disable IRQs.  Note that this macro
 * assumes FIQs are enabled, and that the processor is in SVC mode.
 */
	.macro	save_and_disable_irqs, oldcpsr, temp
	mrs	\oldcpsr, cpsr
	mov	\temp, #I_BIT | MODE_SVC
	msr	cpsr_c, \temp
	.endm

/*
 * Restore interrupt state previously stored in a register.  We don't
 * guarantee that this will preserve the flags.
 */
	.macro	restore_irqs, oldcpsr
	msr	cpsr_c, \oldcpsr
	.endm

/*
 * These two are used to save LR/restore PC over a user-based access.
 * The old 26-bit architecture requires that we do.  On 32-bit
 * architecture, we can safely ignore this requirement.
 */
	.macro	save_lr
	.endm

	.macro	restore_pc
	mov	pc, lr
	.endm

#define USER(x...)				\
9999:	x;




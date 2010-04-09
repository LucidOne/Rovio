	AREA ECOS_POWER_IDLE, CODE, READONLY

	export pwr_halt_cpu
pwr_halt_cpu
	STMFD	sp!, {r0, lr}
	MCR		p15, 0, r0, c7, c0, 4
	LDMFD	sp!, {r0, pc}
	
	END
	
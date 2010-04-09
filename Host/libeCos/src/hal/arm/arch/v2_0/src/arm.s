armreg_r0 EQU 0
armreg_r4 EQU 16
armreg_r8 EQU 32
armreg_r9 EQU 36
armreg_r10 EQU 40
armreg_sp EQU 52
armreg_fp EQU 44
armreg_ip EQU 48
armreg_lr EQU 56
armreg_pc EQU 60
armreg_cpsr EQU 64
armreg_vector EQU 68
armreg_svclr EQU 72
armreg_svcsp EQU 76
ARMREG_SIZE EQU 80
CYGNUM_HAL_ISR_COUNT EQU 27
CYGNUM_HAL_VSR_COUNT EQU 8
CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION EQU 1
CYGNUM_HAL_EXCEPTION_INTERRUPT EQU 2
CYGNUM_HAL_EXCEPTION_CODE_ACCESS EQU 3
CYGNUM_HAL_EXCEPTION_DATA_ACCESS EQU 4
CYGNUM_HAL_VECTOR_IRQ EQU 6
RAISE_INTR EQU 769
CPSR_IRQ_DISABLE EQU 128
CPSR_FIQ_DISABLE EQU 64
CPSR_THUMB_ENABLE EQU 32
CPSR_IRQ_MODE EQU 18
CPSR_FIQ_MODE EQU 17
CPSR_SUPERVISOR_MODE EQU 19
CPSR_UNDEF_MODE EQU 27
CPSR_MODE_BITS EQU 31
CPSR_INITIAL EQU 211
CPSR_THREAD_INITIAL EQU 19
CYGNUM_CALL_IF_TABLE_SIZE EQU 64
CYGNUM_HAL_INTERRUPT_NONE EQU -1

	END
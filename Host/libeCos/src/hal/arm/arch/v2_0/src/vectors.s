; #========================================================================
; #
; #    vectors.S
; #
; #    ARM exception vectors
; #
; #========================================================================
;####ECOSGPLCOPYRIGHTBEGIN####
; -------------------------------------------
; This file is part of eCos, the Embedded Configurable Operating System.
; Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
;
; eCos is free software; you can redistribute it and/or modify it under
; the terms of the GNU General Public License as published by the Free
; Software Foundation; either version 2 or (at your option) any later version.
;
; eCos is distributed in the hope that it will be useful, but WITHOUT ANY
; WARRANTY; without even the implied warranty of MERCHANTABILITY or
; FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
; for more details.
;
; You should have received a copy of the GNU General Public License along
; with eCos; if not, write to the Free Software Foundation, Inc.,
; 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
;
; As a special exception, if other files instantiate templates or use macros
; or inline functions from this file, or you compile this file and link it
; with other works to produce a work based on this file, this file does not
; by itself cause the resulting work to be covered by the GNU General Public
; License. However the source code for this file must still be made available
; in accordance with section (3) of the GNU General Public License.
;
; This exception does not invalidate any other reasons why a work based on
; this file might be covered by the GNU General Public License.
;
; Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
; at http://sources.redhat.com/ecos/ecos-license/
; -------------------------------------------
;####ECOSGPLCOPYRIGHTEND####
; #========================================================================
; ######DESCRIPTIONBEGIN####
; #
; # Author(s):     nickg, gthomas
; # Contributors:  nickg, gthomas
; # Date:          1999-02-20
; # Purpose:       ARM exception vectors
; # Description:   This file defines the code placed into the exception
; #                vectors. It also contains the first level default VSRs
; #                that save and restore state for both exceptions and
; #                interrupts.
; #
; #####DESCRIPTIONEND####
; #
; #========================================================================
	AREA Init, CODE, READONLY
; CWS: for ADS  environment
	IMPORT tty_io_diag
	IMPORT haldiag_io0
	IMPORT cyg_iso_c_start
	IMPORT dev_fste
	IMPORT dev_mte
	IMPORT romfs_fste
 IF :LNOT: :DEF: RW99702
	IMPORT w90n740_99685_video0
	IMPORT w90n740_serial_io0
	IMPORT fat16_fste
	IMPORT fat12_fste
	IMPORT cyg_io_sdram
 ELSE	
	IMPORT w99702_serial_io0
 ENDIF
	IMPORT ramfs_fste
	IMPORT termios_io0
 IF :DEF: CYGPKG_NET_BLUEZ_STACK
	IMPORT _hci_init
	IMPORT _bluez_init
 ENDIF
	
	IMPORT hal_hardware_init
	IMPORT cyg_hal_invoke_constructors
	IMPORT cyg_start
	IMPORT exception_handler
	IMPORT hal_IRQ_handler
	IMPORT hal_spurious_IRQ
	IMPORT interrupt_end
	IMPORT cyg_interrupt_call_pending_DSRs
	IMPORT |Image$$RAM_B$$ZI$$Base|
	IMPORT |Image$$RAM_B$$ZI$$Limit|
	IMPORT |Image$$RAM_D$$Base|
	IMPORT |Image$$RAM_D$$Limit|
	IMPORT |Image$$ECOS_MTAB$$Limit|
	IMPORT |Image$$ECOS_MTAB$$ZI$$Limit|
	
	
	include pkgconf/hal.s
	include pkgconf/hal_arm.s
 IF :DEF: CYGPKG_KERNEL
	include pkgconf/kernel.s
 ELSE
# undef CYGFUN_HAL_COMMON_KERNEL_SUPPORT
# undef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
 ENDIF

 IF :DEF: CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
; The CDL should enforce this
#undef  CYGHWR_HAL_ARM_DUMP_EXCEPTIONS
 ENDIF

	include arm.s

; Switch to thumb mode
;#define THUMB_MODE(_r_, _l_)

; Switch to ARM mode
;#define ARM_MODE(_r_, _l_)

; Function definition, start executing body in ARM mode
	MACRO 
	FUNC_START_ARM $_name_, $_r_
    EXPORT $_name_
$_name_
	MEND

        
	MACRO
	PTR $name
l_$name DCD  $name
	MEND

; CYGHWR_HAL_ROM_VADDR is used when compiling for a different location
; from the base of ROM.  hal_platform_setup.h might define it.  For
; example, if flash is from 0x50000000 upwards (as on SA11x0), and we are
; to execute at 0x50040000, then we want the reset vector to point to
; 0x0004pqrs - the unmapped ROM address of the code - rather than
; 0x0000pqrs, which is the offset into our flash block.
; 
; But usually it's not defined, so the behaviour is the obvious.

 IF :LNOT: :DEF: UNMAPPED        

;# define UNMAPPED(x) (x)

 ENDIF        
                                
	MACRO
	UNMAPPED_PTR $name
.$name DCB  ##$name
	MEND

;        .file   "vectors.S"


; CYGHWR_LED_MACRO can be defined in hal_platform_setup.h. It's free to
; use r0+r1. Argument is in "\x" - cannot use macro arguments since the
; macro may contain #-chars and use of arguments cause these to be 
; interpreted as CPP stringify operators.
; See example in PID hal_platform_setup.h.
 IF :LNOT: :DEF: CYGHWR_LED_MACRO
;CYGHWR_LED_MACRO EQU 1
 ENDIF
        
	MACRO
	LED $x
 IF :DEF: CYGHWR_LED_MACRO
    CYGHWR_LED_MACRO
 ENDIF
	MEND


;==========================================================================
; Hardware exception vectors.
;   This entire section will be copied to location 0x0000 at startup time.
;
        CODE32

; This macro allows platforms to add their own code at the very start of
; the image.  This may be required in some circumstances where eCos ROM 
; based code does not run immediately upon reset and/or when some sort of
; special header is required at the start of the image.        
 IF :DEF: PLATFORM_PREAMBLE
        PLATFORM_PREAMBLE
 ENDIF
		KEEP
    	AREA Vect, CODE, READONLY
    	
        EXPORT __exception_handlers
        ENTRY
__exception_handlers
 IF :DEF: CYGSEM_HAL_ROM_RESET_USES_JUMP
; Assumption:  ROM code has these vectors at the hardware reset address.
; A simple jump removes any address-space dependencies [i.e. safer]
        b       reset_vector                    ; 0x00
 ELSE        
 		; 0x00
        ldr     pc,_reset_vector
 ENDIF        
        ldr     pc,_undefined_instruction       ; 0x04
        ldr     pc,_software_interrupt          ; 0x08 start && software int
        ldr     pc,_abort_prefetch              ; 0x0C
        ldr     pc,_abort_data                  ; 0x10
        NOP	                               ; unused
        ldr     pc,_IRQ                         ; 0x18
        ldr     pc,_FIQ                         ; 0x1C
		LTORG
; The layout of these pointers should match the vector table above since
; they are copied in pairs.
        ;EXPORT vectors
;vectors
_reset_vector	DCD reset_vector                      ; 0x20
_undefined_instruction	DCD undefined_instruction                      ; 0x24
_software_interrupt	DCD software_interrupt                         ; 0x28
_abort_prefetch	DCD abort_prefetch                             ; 0x2C
_abort_data	DCD abort_data                                 ; 0x30
        DCD   0                               ; 0x34
_IRQ	DCD IRQ                                        ; 0x38
_FIQ	DCD FIQ                                        ; 0x3c
 IF :DEF: CYGSEM_HAL_ARM_PID_ANGEL_BOOT         
_start	DCD start ; This is copied to 0x28 for bootup ; 0x40
 ENDIF        
           ; location 0x40 is used for storing DRAM size if known
           ; for some platforms.
        
;
; "Vectors" - fixed location data items
;    This section contains any data which might be shared between
; an eCos application and any other environment, e.g. the debug
; ROM.                        
;
        ;AREA fixed_vectors, COMMON, READWRITE
        ; Interrupt/exception VSR pointers
        EXPORT  hal_vsr_table
hal_vsr_table 
        DCD   0
        DCD   0
        DCD   0
        DCD   0
        DCD   0
        DCD   0
        DCD   0
        DCD   0

        EXPORT  hal_dram_size
hal_dram_size
        DCD   0
	; what, if anything, hal_dram_type means is up to the platform
        EXPORT  hal_dram_type
hal_dram_type  
        DCD   0

        ALIGN   16
 IF :DEF: CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
	; Vectors used to communicate between eCos and ROM environments
        EXPORT  hal_virtual_vector_table
hal_virtual_vector_table
	SPACE CYGNUM_CALL_IF_TABLE_SIZE*4

 ENDIF
        
 IF :DEF: CYGHWR_HAL_ARM_ICE_THREAD_SUPPORT
        balign 16      ; Should be at 0x50
ice_thread_vector
        DCD   0       ; Must be 'MICE'             
        DCD   0       ; Pointer to thread support vector
        DCD   0       ; eCos executing flag
        DCD   0       ; Must be 'GDB '
 ENDIF ; CYGHWR_HAL_ARM_ICE_THREAD_SUPPORT
        ALIGN   32
        
; Other vectors - this may include "fixed" locations
 IF :DEF: PLATFORM_VECTORS
        PLATFORM_VECTORS
 ENDIF

        ;KEEP
    	;AREA Vect, CODE, READONLY
; Startup code which will get the machine into supervisor mode
        ;EXPORT reset_vector
        ;ENTRY
reset_vector
 IF :DEF: PLATFORM_SETUP1
        PLATFORM_SETUP1         ; Early stage platform initialization
                                ; which can set DRAM size at 0x40
                                ; see <cyg/hal/hal_platform_setup.h>
 ENDIF
        ; Come here to reset board
warm_reset          

 IF :DEF: CYG_HAL_STARTUP_RAM
	IF :LNOT: :DEF: CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        mrs     r7,cpsr                 ; move back to IRQ mode
        and     r7,r7,#CPSR_MODE_BITS
        cmp     r7,#CPSR_SUPERVISOR_MODE
        beq     start
    ENDIF
 ENDIF

        ; We cannot access any LED registers until after PLATFORM_SETUP1
        LED 7

        mov     r0,#0           ; move vectors
        ldr     r1,l___exception_handlers	;=
 IF :LNOT: :DEF: CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        ; Wait with this if stubs are included (see further down).
        ldr     r2,[r1,#0x04]   ; undefined instruction
        str     r2,[r0,#0x04]
        ldr     r2,[r1,#0x24]   
        str     r2,[r0,#0x24]
 ENDIF
        ldr     r2,[r1,#0x08]   ; software interrupt
        str     r2,[r0,#0x08]

 IF :DEF: CYGHWR_HAL_ARM_ICE_THREAD_SUPPORT        
        ldr     r2,=ice_thread_vector
        sub     r2,r2,r1        ; compute fixed (low memory) address
        ldr     r3,=0x4D494345  ; 'MICE'
        str     r3,[r2],#4
        ldr     r3,=hal_arm_ice_thread_handler
        str     r3,[r2],#4
        mov     r3,#1
        str     r3,[r2],#4
        ldr     r3,=0x47444220  ; 'GDB '
        str     r3,[r2],#4
 ENDIF ; CYGHWR_HAL_ARM_ICE_THREAD_SUPPORT

 IF :DEF: CYGSEM_HAL_ARM_PID_ANGEL_BOOT
; Ugly hack to get into supervisor mode
        ldr     r2,[r1,#0x40]
        str     r2,[r0,#0x28]

        LED 6
                
        swi                     ; switch to supervisor mode
 ENDIF        

; =========================================================================
; Real startup code. We jump here from the reset vector to set up the world.
        EXPORT  start
start

        LED 5

 IF :DEF: CYG_HAL_STARTUP_RAM
    IF :LNOT: :DEF: CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
; If we get restarted, hang here to avoid corrupting memory
        ldr     r0,l_init_flag
        ldr     r1,[r0]
1       cmp     r1,#0
        bne     %B1
        ldr     r1,init_done
        str     r1,[r0]
     ENDIF
 ENDIF

        ; Reset software interrupt pointer
        mov     r0,#0           ; move vectors
        ldr     r1,l___exception_handlers
		
		ldr     r2,[r1,#0x00]   ;reset instruction
        str     r2,[r0,#0x00]
        ldr     r2,[r1,#0x04]   ;undefined instruction
        str     r2,[r0,#0x04]
        ldr     r2,[r1,#0x24]   
        str     r2,[r0,#0x24]

        ldr     r2,[r1,#0x08]   ;software interrupt
        str     r2,[r0,#0x08]

 IF :DEF: CYG_HAL_STARTUP_RAM
    IF    :LNOT: :DEF: CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        cmp     r7,#CPSR_SUPERVISOR_MODE
        beq     %F10
    ENDIF
        
 ENDIF

        ldr     r2,[r1,#0x28]   ; software interrupt
        str     r2,[r0,#0x28]

10
        ldr     r2,[r1,#0x18]   ; IRQ
        str     r2,[r0,#0x18]
        ldr     r2,[r1,#0x38]
        str     r2,[r0,#0x38]
        ldr     r2,[r1,#0x1C]   ; FIQ
        str     r2,[r0,#0x1C]
        ldr     r2,[r1,#0x3C]
        str     r2,[r0,#0x3C]
        ldr     r2,[r1,#0x0C]   ; abort (prefetch)
        str     r2,[r0,#0x0C]
        ldr     r2,[r1,#0x2C]   
        str     r2,[r0,#0x2C]
        ldr     r2,[r1,#0x10]   ; abort (data)
        str     r2,[r0,#0x10]
        ldr     r2,[r1,#0x30]
        str     r2,[r0,#0x30]

        LED 4

 IF :DEF: CYG_HAL_STARTUP_ROM
        ; Set up reset vector
        mov     r0,#0
        ldr     r1,.__exception_handlers
        ldr     r2,[r1,#0x00]    ; reset vector intstruction
        str     r2,[r0,#0x00]
        ldr     r2,=warm_reset
        str     r2,[r0,#0x20]
        ; Relocate [copy] data from ROM to RAM
        ldr     r3,.__rom_data_start
        ldr     r4,.__ram_data_start
        ldr     r5,.__ram_data_end
        cmp     r4,r5           ; jump if no data to move
        beq     %FT2
        sub     r3,r3,#4        ; loop adjustments
        sub     r4,r4,#4
1       ldr     r0,[r3,#4]!     ; copy info
        str     r0,[r4,#4]!
        cmp     r4,r5
        bne     %B1
2
 ELSE
 	IF :DEF: CYG_HAL_STARTUP_ROMRAM
		mov     r0,#0
        ldr     r1,l___exception_handlers
        ldr     r2,[r1,#0x00]    ; reset vector intstruction
        str     r2,[r0,#0x00]
        ldr     r2,=warm_reset
        str     r2,[r0,#0x20]
        ; Relocate [copy] data from ROM to RAM
        ldr     r3,l___rom_data_start
        ldr     r4,l___ram_data_start
        ldr     r5,l___ram_data_end
        cmp     r4,r5           ; jump if no data to move
        beq     %FT2
        sub     r3,r3,#4        ; loop adjustments
        sub     r4,r4,#4
1       ldr     r0,[r3,#4]!     ; copy info
        str     r0,[r4,#4]!
        cmp     r4,r5
        bne     %B1
2
 	ENDIF
 ENDIF
 
  IF :DEF: RW99702
	;------------------------------------------------------
	; Set the high exception vector of CP15 control bit    
	;------------------------------------------------------        
	MRC	p15, 0, r0, c1, c0, 0  ; read control register         
    BIC r0, r0, #0x2000        ; clear bit 13                  
    MCR	p15, 0, r0, c1, c0, 0  ; write control register,          
 ENDIF
 
        ; initialize interrupt/exception environments
        ldr     sp,l___startup_stack	;=
        mov     r0,#(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_IRQ_MODE)
        MSR     cpsr_cf,r0
        ldr     sp,l___exception_stack
        mov     r0,#(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_UNDEF_MODE)
        msr     cpsr_cf,r0
        ldr     sp,l___exception_stack

        ; initialize CPSR (machine state register)
        mov     r0,#(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SUPERVISOR_MODE)
        msr     cpsr_cf,r0

        ; Note: some functions in LIBGCC1 will cause a "restore from SPSR"!!
        msr     spsr_cf,r0

        ; initialize stack
 IF :DEF: CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
        ; use interrupt stack for system initialization since it's bigger 
        ; than the "startup" stack in this configuration                                
        ldr     sp,l___interrupt_stack
 ELSE        
        ldr     sp,l___startup_stack
 ENDIF        

        ; clear BSS
        ldr     r1,l___bss_start
        ldr     r2,l___bss_end
        mov     r0,#0
        cmp     r1,r2
        beq     %FT2
1       str     r0,[r1],#4
        cmp     r1,r2
        bls     %B1
		
		;clear mtab_extra
2       ldr     r1,l___mtabbss_start
        ldr     r2,l___mtabbss_end
        mov     r0,#0
        cmp     r1,r2
        beq     %FT2
1       str     r0,[r1],#4
        cmp     r1,r2
        bls     %B1

2
 IF :DEF: THUMB_MODE
        ; Run kernel + application in THUMB mode
        THUMB_MODE(r1,10)
 ENDIF
        LED 3
        
        ; Call platform specific hardware initialization
        bl      hal_hardware_init

 IF :DEF: CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        bl      initialize_stub

        ; Now that stub is initialized, change vector. It is possible
        ; to single-step through most of the init code, except the below.
        ; Put a breakpoint at the call to cyg_hal_invoke_constructors to
        ; pass over this bit (s-s depends on internal state in the stub).
 ENDIF

 IF :DEF: CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
    
        mov     r0,#0           ; move vectors
        ldr     r1,=l___exception_handlers
        ldr     r2,[r1,#0x04]   ; undefined instruction
        str     r2,[r0,#0x04]
        ldr     r2,[r1,#0x24]   
        str     r2,[r0,#0x24]
 ELIF :DEF: CYGIMP_HAL_PROCESS_ALL_EXCEPTIONS
 		 mov     r0,#0           ; move vectors
        ldr     r1,=l___exception_handlers
        ldr     r2,[r1,#0x04]   ; undefined instruction
        str     r2,[r0,#0x04]
        ldr     r2,[r1,#0x24]   
        str     r2,[r0,#0x24]
 ENDIF

 IF :DEF: CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT 
        IMPORT hal_ctrlc_isr_init
        bl      hal_ctrlc_isr_init
 ELIF :DEF:CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
 	IMPORT hal_ctrlc_isr_init
    bl      hal_ctrlc_isr_init
 ENDIF
        LED 2
        
        ; Run through static constructors
        bl      cyg_hal_invoke_constructors

        LED 1
        
        ; This starts up the eCos kernel
 IF :DEF: CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
        ldr     r1,l___startup_stack	;=
        mov     sp,r1
 ENDIF        
        bl      cyg_start
_start_hang
        b       _start_hang
        LTORG
        
        KEEP

		CODE32
		EXPORT	reset_platform
        
reset_platform      
 IF :DEF: CYGSEM_HAL_ROM_MONITOR
        ; initialize CPSR (machine state register)
        mov     r0,#(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SUPERVISOR_MODE)
        msr     cpsr,r0
        b       warm_reset
 ELSE
        mov     r0,#0
        mov     pc,r0           ; Jump to reset vector        
 ENDIF                   

init_done
        DCD   0xDEADB00B

;
; Exception handlers
; Assumption: get here from a non-user context [mode]
;
        CODE32
undefined_instruction
        ldr     sp,l___undef_exception_stack     ; get good stack
        stmfd   sp!,{r0-r5}                     ; save some supervisor regs
        mrs     r1,spsr
        tst     r1,#CPSR_THUMB_ENABLE
        subeq   r0,lr,#4                ; PC at time of interrupt (ARM)
        subne   r0,lr,#2                ; PC at time of interrupt (thumb)
        mov     r2,#CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION
        mov     r3,sp
        b       call_exception_handler
		CODE32
software_interrupt
        stmfd   sp!,{r8}
        ldr     r8,l___undef_exception_stack     ; get good stack
        stmfd   r8!,{r0-r5}                     ; save some supervisor regs
        mov     r3,r8
        ldmfd   sp!,{r8}
        mrs     r1,spsr
        tst     r1,#CPSR_THUMB_ENABLE
        subeq   r0,lr,#4                ; PC at time of SWI (ARM)
        subne   r0,lr,#2                ; PC at time of SWI (thumb)
        mov     r2,#CYGNUM_HAL_EXCEPTION_INTERRUPT
        b       call_exception_handler
		CODE32
abort_prefetch
        ldr     sp,l___undef_exception_stack     ; get good stack
        stmfd   sp!,{r0-r5}                     ; save some supervisor regs
        sub     r0,lr,#4                        ; PC at time of interrupt
        mrs     r1,spsr
        mov     r2,#CYGNUM_HAL_EXCEPTION_CODE_ACCESS
        mov     r3,sp
        b       call_exception_handler
		CODE32
abort_data
        ldr     sp,l___undef_exception_stack     ; get good stack
        stmfd   sp!,{r0-r5}                     ; save some supervisor regs
        sub     r0,lr,#4                        ; PC at time of interrupt
        mrs     r1,spsr
        mov     r2,#CYGNUM_HAL_EXCEPTION_DATA_ACCESS
        mov     r3,sp
        b       call_exception_handler
        
;
; Dispatch an exception handler.
		CODE32
call_exception_handler
        ;
        ; On Entry:
        ;
        ; r4,r5 = scratch
        ; r3 = pointer to temp save area
        ; r2 = vector number
        ; r1 = exception psr
        ; r0 = exception pc
        ; 
        ; [r3+20]: exception r5
        ; [r3+16]: exception r4
        ; [r3+12]: exception r3
        ; [r3+8] : exception r2
        ; [r3+4] : exception r1
        ; [r3]   : exception r0
        
        mrs     r4,cpsr                 ; switch to Supervisor Mode
        bic     r4,r4,#CPSR_MODE_BITS
        orr     r4,r4,#CPSR_SUPERVISOR_MODE
        msr     cpsr_cf,r4

        mov     r5,sp                   ; save original svc sp
        mov	r4,lr                   ; and original svc lr
 IF :DEF: CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        ; Make sure we use the GDB stack.
        ldr     sp,.__GDB_stack
        cmp     r5,sp                   ; already on GDB stack?
        bhi     %FT10
        ldr     r4,.__GDB_stack_base            
        cmp     r5,r4
        movhi   sp,r5
10
 ENDIF
        ;
        ; r5 holds original svc sp, current sp is stack to use
        ; r4 holds original svc lr, which must also be preserved
        ;

        stmfd   sp!,{r0-r2,r4,r5}       ; push svc_sp, svc_lr, vector, psr, pc
        
        ; switch to pre-exception mode to get banked regs
        mov     r0,sp                   ; r0 survives mode switch
        mrs     r2,cpsr                 ; Save current psr for return
        orr     r1,r1,#CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE
        bic     r1,r1,#CPSR_THUMB_ENABLE
        msr     cpsr_cf,r1
        stmfd   r0!,{r8-r12,sp,lr}
        msr     cpsr_cf,r2                 ; back to svc mode
        mov     sp,r0                   ; update stack pointer

        ; now save pre-exception r0-r7 on current stack
        ldmfd   r3,{r0-r5}
        stmfd   sp!,{r0-r7}

        ; SP needs fixing if exception occured in SVC mode.
        ; The original SVC LR is still in place so that 
        ; does not need to be fixed here.
        ldr     r1,[sp,#armreg_cpsr]
        and     r1,r1,#CPSR_MODE_BITS
        cmp     r1,#CPSR_SUPERVISOR_MODE
        ldreq   r1,[sp,#armreg_svcsp]
        streq   r1,[sp,#armreg_sp]

 IF :DEF:  CYGHWR_HAL_ARM_DUMP_EXCEPTIONS
        mov     r0,sp
        ldr     r1,.__dump_procs
        ldr     r2,[sp,#armreg_vector]
        mov     lr,pc
        ldr     pc,[r1,r2,lsl #2]
 ENDIF

        ; call exception handler
        mov     r0,sp
 IF :DEF:  THUMB_MODE
        THUMB_MODE(r9,10)
 ENDIF 
        bl      exception_handler

 IF :DEF:  CYGHWR_HAL_ARM_DUMP_EXCEPTIONS
        mov     r0,sp
        bl      cyg_hal_report_exception_handler_returned
 ENDIF
 IF :DEF: ARM_MODE
        ARM_MODE(r1,10)
 ENDIF
        ;
        ; Return from exception
        ;
return_from_exception

        ldr     r0,[sp,#armreg_cpsr]
        msr     spsr_cf,r0

        ; return to supervisor mode is simple
        and     r1,r0,#CPSR_MODE_BITS
        cmp     r1,#CPSR_SUPERVISOR_MODE
        ldmeqfd sp,{r0-r14,pc}^

        ;
        ; return to other non-user modes is a little trickier
        ;

        ; switch to pre-exception mode and restore r8-r14
        add     r2,sp,#armreg_r8
        mrs     r1,cpsr
        orr     r0,r0,#CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE
        bic     r0,r0,#CPSR_THUMB_ENABLE
        msr     cpsr_cf,r0
        ldmfd   r2,{r8-r14}
        msr     cpsr_cf, r1        ; back to svc mode

        ; move sp,lr and pc for final load
        ldr     r0,[sp,#armreg_svcsp]
        str     r0,[sp,#armreg_r8]
        ldr     r0,[sp,#armreg_svclr]	
        str     r0,[sp,#armreg_r9]
        ldr     r0,[sp,#armreg_pc]
        str     r0,[sp,#armreg_r10]

        ; restore r0-r7,sp,lr and return from exception
        ldmfd   sp,{r0-r7,sp,lr,pc}^

 IF :DEF:  CYGHWR_HAL_ARM_DUMP_EXCEPTIONS
__dump_procs
        DCD  0    ; placeholder for reset
        DCD  cyg_hal_report_undefined_instruction
        DCD  cyg_hal_report_software_interrupt
        DCD  cyg_hal_report_abort_prefetch
        DCD  cyg_hal_report_abort_data
        DCD  0    ; reserved
 ENDIF


; Handle device interrupts
; This is slightly more complicated than the other exception handlers because
; it needs to interface with the kernel (if present).
		CODE32
FIQ
        ; We can get here from any non-user mode.
        mrs     r8,spsr                 ; CPSR at time of interrupt
        and     r9,r8,#CPSR_MODE_BITS   ; isolate pre-interrupt mode
        cmp	r9,#CPSR_IRQ_MODE
        bne	%FT1
        ; If FIQ interrupted IRQ mode, just return with FIQ disabled.
        ; The common interrupt handling takes care of the rest.
        orr	r8,r8,#CPSR_FIQ_DISABLE
        msr	spsr_cf,r8
        subs	pc,lr,#4
1
        ; If FIQ interrupted other non-user mode, switch to IRQ mode and
        ; fall through to IRQ handler.
        ldr     sp,l___exception_stack   ; get good stack to save lr and spsr
        stmdb   sp,{r8,lr}
        mov     r8,#CPSR_IRQ_MODE|CPSR_FIQ_DISABLE|CPSR_IRQ_DISABLE
        msr     cpsr_cf,r8			; switch to IRQ mode
        ldr     sp,l___exception_stack   ; get regs saved in FIQ mode
        ldmdb	sp,{sp,lr}
        msr     spsr_cf,sp

        ; now it looks like we got an IRQ instead of an FIQ except that
        ; FIQ is disabled so we don't recurse.
IRQ
        ; Note: I use this exception stack while saving the context because
        ; the current SP does not seem to be always valid in this CPU mode.
        ldr     sp,l___exception_stack   ; get good stack
        stmfd   sp!,{r0-r5}             ; save some supervisor regs
        sub     r0,lr,#4                ; PC at time of interrupt
        mrs     r1,spsr
        mov     r2,#CYGNUM_HAL_VECTOR_IRQ
        mov     r3,sp

handle_IRQ_or_FIQ

        mrs     r4,cpsr                 ; switch to Supervisor Mode
        bic     r4,r4,#CPSR_MODE_BITS
        orr     r4,r4,#CPSR_SUPERVISOR_MODE
        msr     cpsr_cf,r4

        mov     r5,sp                   ; save original svc sp
	mov	r4,lr			; save original svc lr
        stmfd   sp!,{r0-r2,r4,r5}       ; push svc_sp, svc_lr, vector, psr, pc
		
        ; switch to pre-exception mode to get banked regs
        mov     r0,sp                   ; r0 survives mode switch
        mrs     r2,cpsr                 ; Save current psr for return
        orr     r1,r1,#CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE
        bic     r1,r1,#CPSR_THUMB_ENABLE
        msr     cpsr_cf,r1
        stmfd   r0!,{r8-r12,sp,lr}
        msr     cpsr_cf,r2                 ; back to svc mode
        mov     sp,r0                   ; update stack pointer
	
        ; now save pre-exception r0-r7 on current stack
        ldmfd   r3,{r0-r5}
        stmfd   sp!,{r0-r7}

        ; sp needs fixing if exception occured in SVC mode.
        ldr     r1,[sp,#armreg_cpsr]
        and     r1,r1,#CPSR_MODE_BITS
        cmp     r1,#CPSR_SUPERVISOR_MODE
        ldreq   r1,[sp,#armreg_svcsp]
        streq   r1,[sp,#armreg_sp]

        mov     v6,sp                   ; Save pointer to register frame

;      mov     r0,sp
;      bl      _show_frame_in

 IF :DEF: CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
        ; Switch to interrupt stack
        ldr     r2,l_irq_level           ; current number of nested interrupts
        ldr     r0,[r2]
        add     r1,r0,#1
        str     r1,[r2]                 ; if was zero, switch stacks
        cmp     r0,#0
        moveq   r1,sp                   ; save old stack pointer
        ldreq   sp,l___interrupt_stack
        stmeqfd sp!,{r1}
10
 ENDIF
 
        ; The entire CPU state is now stashed on the stack,
        ; increment the scheduler lock and handle the interrupt

 IF :DEF: CYGFUN_HAL_COMMON_KERNEL_SUPPORT                 
        IMPORT cyg_scheduler_sched_lock
        ldr     r3,l_cyg_scheduler_sched_lock
        ldr		r3,[r3]
        ldr     r4,[r3]
        add     r4,r4,#1
        str     r4,[r3]
 ENDIF
 IF :DEF THUMB_MODE
        THUMB_MODE(r3,10)
 ENDIF
        mov     r0,v6
        bl      hal_IRQ_handler         ; determine interrupt source
        mov     v1,r0                   ; returned vector #

 IF :DEF: CYGPKG_KERNEL_INSTRUMENT
    IF :DEF: CYGDBG_KERNEL_INSTRUMENT_INTR
        ldr     r0,=RAISE_INTR          ; arg0 = type = INTR,RAISE
        mov     r1,v1                   ; arg1 = vector
        mov     r2,#0                   ; arg2 = 0
        bl      cyg_instrument          ; call instrument function
    ENDIF
 ENDIF
 IF :DEF: ARM_MODE
        ARM_MODE(r0,10)
 ENDIF
        mov     r0,v1                   ; vector #

 IF :DEF: CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT
        ; If we are supporting Ctrl-C interrupts from GDB, we must squirrel
        ; away a pointer to the save interrupt state here so that we can
        ; plant a breakpoint at some later time.

       .extern  hal_saved_interrupt_state
        ldr     r2,=hal_saved_interrupt_state
        str     v6,[r2]
 ELIF :DEF:CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
		.extern  hal_saved_interrupt_state
        ldr     r2,=hal_saved_interrupt_state
        str     v6,[r2]
 ENDIF

        cmp     r0,#CYGNUM_HAL_INTERRUPT_NONE   ; spurious interrupt
        bne     %FT10
 IF :LNOT: :DEF: CYGIMP_HAL_COMMON_INTERRUPTS_IGNORE_SPURIOUS
        mov     r0,v6                   ; register frame
        bl      hal_spurious_IRQ
 ENDIF ; CYGIMP_HAL_COMMON_INTERRUPTS_IGNORE_SPURIOUS
        b       spurious_IRQ
        
10     ldr     r1,l_hal_interrupt_data
        ldr     r1,[r1,v1,lsl #2]       ; handler data
        ldr     r2,l_hal_interrupt_handlers
        ldr     v3,[r2,v1,lsl #2]       ; handler (indexed by vector #)
        mov     r2,v6                   ; register frame (this is necessary
                                        ; for the ISR too, for ^C detection)

 IF :DEF: __thumb__
        ldr     lr,=10f
        bx      v3                      ; invoke handler (thumb mode)
        pool
        CODE16
        thumb_func
IRQ_10T
10     ldr     r2,=15f
        bx      r2                      ; switch back to ARM mode
        pool
        CODE32
15
IRQ_15A
 ELSE
        mov     lr,pc                   ; invoke handler (call indirect
        mov     pc,v3                   ; thru v3)
 ENDIF

spurious_IRQ

 IF :DEF: CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
        ; If we are returning from the last nested interrupt, move back
        ; to the thread stack. interrupt_end() must be called on the
        ; thread stack since it potentially causes a context switch.
        ldr     r2,l_irq_level
        ldr     r3,[r2]
        subs    r1,r3,#1
        str     r1,[r2]
        ldreq   sp,[sp]         ; This should be the saved stack pointer
 ENDIF
 IF :DEF: CYGFUN_HAL_COMMON_KERNEL_SUPPORT
        ; The return value from the handler (in r0) will indicate whether a 
        ; DSR is to be posted. Pass this together with a pointer to the
        ; interrupt object we have just used to the interrupt tidy up routine.

                              ; don't run this for spurious interrupts!
        cmp     v1,#CYGNUM_HAL_INTERRUPT_NONE
        beq     %F17
        ldr     r1,l_hal_interrupt_objects
        ldr     r1,[r1,v1,lsl #2]
        mov     r2,v6           ; register frame
 IF :DEF: THUMB_MODE
        THUMB_MODE(r3,10)
 ENDIF
        bl      interrupt_end   ; post any bottom layer handler
                                ; threads and call scheduler
 IF :DEF: ARM_MODE
        ARM_MODE(r1,10)
 ENDIF
17
 ENDIF

;      mov     r0,sp
;      bl      show_frame_out

	; return from IRQ is same as return from exception
	b	return_from_exception

 IF :DEF: CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
; Execute pending DSRs the interrupt stack
; Note: this can only be called from code running on a thread stack
 FUNC_START_ARM hal_interrupt_stack_call_pending_DSRs, r1
        stmfd   sp!,{r4,r5,lr}
        ; Disable interrupts
        mrs     r4,cpsr                 ; disable IRQ's
        orr     r2,r4,#CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE
        bic     r5,r4,#CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE
        msr     cpsr_cf,r2
        ; Switch to interrupt stack
        mov     r3,sp                   ; save old stack pointer
        ldr     sp,l___interrupt_stack
        stmfd   sp!,{r3}                ; stored at top of interrupt stack
        ldr     r2,l_irq_level           ; current number of nested interrupts
        ldr     r3,[r2]
        add     r3,r3,#1                ; bump nesting level
        str     r3,[r2]
        msr     cpsr_cf,r5                 ; enable interrupts
 IF :DEF: THUMB_MODE
        THUMB_MODE(r1,20)
 ENDIF
        bl      cyg_interrupt_call_pending_DSRs

 IF :DEF: ARM_MODE
        ARM_MODE(r1,22)
 ENDIF
        ; Disable interrupts
        mrs     r1,cpsr                 ; disable IRQ's
        orr     r2,r1,#CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE
        msr     cpsr_cf,r2

        ; Move back to the thread stack.
        ldr     r2,l_irq_level
        ldr     r3,[r2]
        sub     r3,r3,#1                ; decrement nesting level
        str     r3,[r2]
        ldr     sp,[sp]                 ; This should be the saved stack pointer
        msr     cpsr_cf,r4                 ; restore interrupts to original state

 IF :DEF: __thumb__
        ldmfd   sp!,{r4,r5,lr}          ; return
        bx      lr
 ELSE
        ldmfd   sp!,{r4,r5,pc}          ; return
 ENDIF ; __thumb__
 ENDIF ; CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
        
; Thumb-only support functions
 IF :DEF: __thumb__

 FUNC_START_ARM hal_disable_interrupts, r1
        mrs     r0,cpsr                 ; current state
        orr     r1,r0,#0xC0             ; mask both FIQ and IRQ
        msr     cpsr_cf,r1
        bx      lr                      ; exit, _old_ in r0        

 FUNC_START_ARM hal_enable_interrupts, r1
        mrs     r0,cpsr                 ; current state
        bic     r1,r0,#0xC0             ; mask both FIQ and IRQ
        msr     cpsr_cf,r1
        bx      lr                      ; exit
        
 FUNC_START_ARM hal_restore_interrupts, r1)
        mrs     r1,cpsr                 ; current state
        bic     r1,r1,#0xC0             ; mask out FIQ/IRQ bits
        and     r0,r0,#0xC0             ; keep only FIQ/IRQ
        orr     r1,r1,r0                ; mask both FIQ and IRQ
        msr     cpsr,r1
        bx      lr                      ; exit        

FUNC_START_ARM(hal_query_interrupts, r1)
        mrs     r0,cpsr                 ; current state
        bx      lr                      ; exit, state in r0

 ENDIF ; __thumb__

; Dummy/support functions

        EXPORT __gccmain
        EXPORT _psr
        EXPORT _sp

 IF :DEF: __thumb__
        CODE16
        thumb_func
__gccmain
        bx      lr

        CODE16
        thumb_func
_psr
        ARM_MODE(r1,10)
        mrs     r0,cpsr
        bx      lr

        CODE16
        thumb_func
_sp
        mov     r0,sp
        bx      lr
 ELSE
__gccmain
        mov     pc,lr   

_psr
        mrs     r0,cpsr
        mov     pc,lr

_sp
        mov     r0,sp
        mov     pc,lr
 ENDIF               

	;AREA DATA,READONLY
	EXPORT __heap1
	;
; Pointers to various objects.
;
 IF :DEF: CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
 PTR(__GDB_stack_base)
 PTR(__GDB_stack)
 ENDIF
 PTR __startup_stack
 PTR __exception_stack
 PTR __undef_exception_stack
l___bss_start	DCD |Image$$RAM_B$$ZI$$Base|
l___bss_end	DCD	|Image$$RAM_B$$ZI$$Limit|
l__end	DCD	|Image$$RAM_B$$ZI$$Limit|
l___rom_data_start	DCD	|Image$$RAM_D$$Base|
l___ram_data_start	DCD	|Image$$RAM_D$$Base|
l___ram_data_end	DCD	|Image$$RAM_D$$Limit|
__heap1	DCD	|Image$$RAM_B$$ZI$$Limit|
l___mtabbss_start	DCD	|Image$$ECOS_MTAB$$Limit|
l___mtabbss_end	DCD	|Image$$ECOS_MTAB$$ZI$$Limit|

 PTR hal_interrupt_handlers
 PTR hal_interrupt_data
 PTR hal_interrupt_objects
 PTR __exception_handlers
 PTR init_flag
 IF :DEF: CYGFUN_HAL_COMMON_KERNEL_SUPPORT
 PTR cyg_scheduler_sched_lock
 ENDIF
 IF :DEF: CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
 PTR irq_level
 PTR __interrupt_stack
 ENDIF
 IF :DEF:  CYGHWR_HAL_ARM_DUMP_EXCEPTIONS
 PTR(__dump_procs)
 ENDIF

;
; Identification - useful to find out when a system was configured
_eCos_id
        DCB  "eCos : "


; -------------------------------------------------------------------------
; Interrupt vector tables.
; These tables contain the isr, data and object pointers used to deliver
; interrupts to user code.

; Despite appearances, their sizes are not #defines, but .equ symbols
; generated by magic without proper dependencies in arm.inc
; Recompiling will not DTRT without manual intervention.
	
		;AREA DATA, READWRITE
		ALIGN	4
init_flag
        DCD   0

	MACRO
	REPT $total,$lab
	LCLA count
count SETA $total
		WHILE count > 0
count SETA count-1
        	DCD  $lab
        WEND
	MEND
        IMPORT hal_default_isr

        EXPORT hal_interrupt_handlers
hal_interrupt_handlers
	REPT CYGNUM_HAL_ISR_COUNT,hal_default_isr 


        EXPORT  hal_interrupt_data
hal_interrupt_data
        SPACE   CYGNUM_HAL_ISR_COUNT*4
        ;DCD   0

        EXPORT  hal_interrupt_objects
hal_interrupt_objects
        SPACE   CYGNUM_HAL_ISR_COUNT*4
        ;DCD   0

; -------------------------------------------------------------------------
; Temporary interrupt stack
        
        ;.section ".bss"
        ;AREA DATA, READWRITE

; Small stacks, only used for saving information between CPU modes
__exception_stack_base
        SPACE 32*4
        ;DCD   0
__exception_stack
        SPACE   32*4
        ;DCD   0

__undef_exception_stack

; Runtime stack used during all interrupt processing
 IF :LNOT: :DEF: CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE EQU 4096
 ENDIF
 IF :DEF: CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
        align 16
        EXPORT cyg_interrupt_stack_base
cyg_interrupt_stack_base
__interrupt_stack_base
        SPACE CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
        ;DCB 0
        align 16
        EXPORT cyg_interrupt_stack
cyg_interrupt_stack
__interrupt_stack
irq_level
        DCD   0
 ENDIF

 IF :DEF: CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        align 16
__GDB_stack_base
        SPACE CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE ; rather than 1k
        ;DCB 0
__GDB_stack
 ENDIF
        align 16
__startup_stack_base
 IF :DEF: CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
        SPACE  512
 ELSE
        SPACE CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
 ENDIF

        align 16
__startup_stack
		DCD 0

 IF :DEF: PLATFORM_EXTRAS
	include PLATFORM_EXTRAS
 ENDIF                                

	END
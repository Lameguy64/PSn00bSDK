;
; The debug monitor consists of two parts; the installer binary (assembled as
; patchinst.bin) and the monitor binary (assembled as patchcode.bin). The
; purpose of the installer binary is to patch the debug monitor into the kernel
; space starting at address C000h. The monitor's entrypoints are then hooked to
; BIOS functions A(40h) (SystemErrorUnresolvedException) and B(17h)
; (ReturnFromException), the latter of which is required so the debug monitor
; can poll for any commands during any IRQ exception as opposed to polling the
; debug monitor manually with a special break instruction (break 1024, defined
; in pollhost()). This allows the debug monitor to stop program execution at any
; point in the program, provided interrupts are not disabled.
;

.psx

.include "cmdefs.inc"

; ****************************
; Register constants
; ****************************

BPC				equ		$3				; cop0 register definitions
BDA				equ		$5
JUMPDEST		equ		$6
DCIC			equ		$7
BADVADDR		equ		$8
BDAM			equ		$9
BPCM			equ		$11
SR				equ		$12
CAUSE			equ		$13
EPC				equ		$14
PRID			equ		$15

; ****************************
; Constants
; ****************************

PCB_addr		equ		0x108(r0)		; Pointer to main/current TCB

TCB_status		equ		0				; TCB block
TCB_reserve		equ		4
TCB_r0			equ		8
TCB_at			equ		12
TCB_v0			equ		16
TCB_v1			equ		20
TCB_a0			equ		24
TCB_a1			equ		28
TCB_a2			equ		32
TCB_a3			equ		36
TCB_t0			equ		40
TCB_t1			equ		44
TCB_t2			equ		48
TCB_t3			equ		52
TCB_t4			equ		56
TCB_t5			equ		60
TCB_t6			equ		64
TCB_t7			equ		68
TCB_s0			equ		72
TCB_s1			equ		76
TCB_s2			equ		80
TCB_s3			equ		84
TCB_s4			equ		88
TCB_s5			equ		92
TCB_s6			equ		96
TCB_s7			equ		100
TCB_t8			equ		104
TCB_t9			equ		108
TCB_k0			equ		112
TCB_k1			equ		116
TCB_gp			equ		120
TCB_sp			equ		124
TCB_fp			equ		128
TCB_ra			equ		132
TCB_epc			equ		136
TCB_hi			equ		140
TCB_lo			equ		144
TCB_sr			equ		148
TCB_cause		equ		152

MONADDR			equ		0xC000				; Monitor install address

SAVE_mode		equ		0x30(r0)			; Saved variables
SAVE_tmode		equ		0x31(r0)			; These are stored from address 0
SAVE_k0			equ		0x34(r0)
SAVE_k1			equ		0x38(r0)
SAVE_dcic		equ		0x3C(r0)

; ****************************
; Debug monitor status values
; ****************************

DB_STAT_STOP	equ		0
DB_STAT_BREAK	equ		1
DB_STAT_EXCEPT	equ		2
DB_STAT_RUN		equ		3

; ****************************
; Program Breakpoint entry (16 bytes)
; ****************************

MAX_BREAK		equ		32

DB_BRK_FLAG		equ		0					; Breakpoint flags
DB_BRK_ADDR		equ		4					; Breakpoint address
DB_BRK_INST		equ		8					; Breakpoint original opcode
DB_BRK_LCNT		equ		12					; Breakpoint counters
DB_BRK_HCNT		equ		14
DB_BRK_LEN		equ		16					; Breakpoint entry length

; ****************************
; Macros
; ****************************

.macro EnterCriticalSection					
		addiu	a0, r0, 0x1
		syscall	0
.endmacro

.macro ExitCriticalSection
		addiu	a0, r0, 0x2
		syscall	0
.endmacro

.macro bios_rfe								; Jumps to real ReturnFromException()
		la		v0, rfe_ptr
		lw		v0, 0(v0)
		nop
		jr		v0
		nop
.endmacro

.macro rfe									; RFE opcode workaround for armips
		.word	0x42000010					; (for the version I'm stuck with)
.endmacro

; ****************************
; Monitor installer binary
; ****************************

.create "patchinst.bin", 0x80010000

; workaround notes for getting trace to operate properly...
;
; Modify exception handler jump at address 0x80 from:
;
; lui	k0, 0
; addiu	k0, 0xC80
; jr	k0
; nop
;
; To the following routine:
;
; mfc0	k0, DCIC			; backup DCIC value to a known, unused address
; nop
; sw	k0, SAVE_dcic
; mtc0	r0, DCIC			; deactivate debug flags by clearing DCIC
; lui	k0, 0
; addiu	k0, 0xC80
; jr	k0
; nop
;
; This debug monitor is not really meant to debug commercial games that don't
; play well with kernel registers (k0/k1) being modified by exception handlers.
; Though games are not supposed to touch the kernel registers in the first
; place albeit FlushCache() uses the two registers.

;
; = Patch installer routine
;
;
patchinst:
		addiu	sp, -4
		sw		ra, 0(sp)
		la		a0, breakhook				; Install breakpoint vector hook
		la		a1, (breakhook_end-breakhook)+4
		li		a2, 0xA0000040
		jal		copymem
		nop
		li		v0, 0x200					; Hook monitor entrypoint to
		li		v1, 0x40					; SysErrUnresolvedException() slot
		sll		v1, 2
		addu	v0, v1
		la		v1, monitor_entry
		sw		v1, 0(v0)
		addiu	t2, r0, 0xB0				; GetB0Table()
		jalr	t2
		addiu	t1, r0, 0x57
		li		a0, 0x17					; Get pointer of ReturnFromException()
		sll		a0, 2
		addu	a0, v0
		lw		v0, 0(a0)					; Save original address for later
		la		v1, monitor_entry
		beq		v0, v1, @@noinstall			; Don't install if hooked already
		nop
		addiu	sp, -4
		sw		a0, 0(sp)
		la		a0, payload					; Load monitor code into target
		la		a1, (monitor_end-monitor)+4	; address
		lui		a2, 0xA000					; write via uncached segment
		jal		copymem
		ori		a2, MONADDR
		lw		a0, 0(sp)
		addiu	sp, 4
		lw		v0, 0(a0)
		la		v1, rfe_ptr
		sw		v0, 0(v1)
		la		v0, monitor_entry			; Set new address to table
		sw		v0, 0(a0)
		li		a0, 0x80					; Move existing exception vector
		li		a2, 0x90                    ; jump to prepend patch
		jal		copymem
		li		a1, 16
		la		a0, exceptpatch				; Patch the exception vector for
		li		a2, 0x80					; trace to work properly
		jal		copymem
		li		a1, 16
		addiu	t2, r0, 0xA0				; FlushCache() just to make sure
		jalr	t2
		addiu	t1, r0, 0x44
		jal		init_breakpoints
		nop
	@@noinstall:
		lw		ra, 0(sp)					; Return to caller, debugger
		addiu	sp, 4                       ; already installed
		jr		ra
		nop
		
	; patchinst

;
; = installer's copy routine
;
;
;
copymem:
		subiu	a1, 4
		lw		v0, 0(a0)
		addiu	a0, 4
		sw		v0, 0(a2)
		bgtz	a1, copymem
		addiu	a2, 4
		jr		ra
		nop
		
		; copymem

;
; = Initializes breakpoint list
;
init_breakpoints:
		la		a2, break_entries
		li		a3, MAX_BREAK
		addiu	v0, r0, -1
	@@clear_loop:
		sw		v0, DB_BRK_INST(a2)
		sw		v0, DB_BRK_ADDR(a2)
		sw		r0, DB_BRK_FLAG(a2)
		addiu	a3, -1
		bnez	a3, @@clear_loop
		addiu	a2, DB_BRK_LEN
		jr		ra
		nop
		
		; init_breakpoints
		
;
; = Break vector hook code
;
; This is copied to the breakpoint exception vector at address 40h
;
breakhook:
		sw		k0, SAVE_k0					; Save K0 and K1 registers
		mfc0	k0, DCIC					; Save DCIC
		sw		k1, SAVE_k1
		mtc0	r0, DCIC					; Clear DCIC in case trace is still
		sw		k0, SAVE_dcic				; effective
		la		k0, breakhandler			; Jump to break vector handler
		jr		k0
		nop
breakhook_end:

;
; = Exception vector patch
;
; This is prepended to the exception vector jump at address 80h
;
exceptpatch:
		mfc0	k0, DCIC					; backup DCIC value
		nop
		mtc0	r0, DCIC					; clear DCIC
		sw		k0, SAVE_dcic
payload:									; patchinst.bin must be appended
											; with patchcode.bin after assembly
.close ; patchinst.bin


;
; = The actual debug monitor itself
;
;
.create "patchcode.bin", MONADDR			; The debug monitor program itself

monitor:

		dw	monitor_entry					; Pointers used by stub installer
		dw	rfe_ptr
		dw	break_entries
		nop

str_dbinfo:	.asciiz "PSnDBmk2"				; Debug monitor identifier (at
			.align	4						; start so it can be seen easily)

;
; = Include comms routines
;
;.include "comms.inc"
.include "sio.inc"

;
; = Main exception handler
;
; When BIOS function ReturnFromException() or SysErrorUnresolvedException()
; is called, this routine checks for unresolved exceptions and any pending
; monitor commands to process before actually returning from exception.
;
; In other words, this routine is executed on every interrupt. Thus, allowing
; the debug monitor to operate like a background task.
;
monitor_entry:
		lw		k0, PCB_addr				; Retrieve address of current TCB
		nop
		lw		k0, 0(k0)					; Load the address
		nop
		lw		v0, TCB_cause(k0)			; Get cause register value from TCB
		nop
		srl		v0, 2
		andi	v0, 0x1F
		beq		v0, 0x8, @@return			; Ignore syscall exceptions
		nop
		beq		v0, 0x9, @@checkbrk			; Check opcode if break instruction
		nop
		bnez	v0, dbexception				; Any other cause other than INTs
		nop									; is an unhandled exception
		addiu	sp, -4
		sw		ra, 0(sp)
		jal		comm_EnableListen			; Enable listening of the comms
		nop									; interface
		jal		comm_GetStatus				; Get comms status
		nop
		lw		ra, 0(sp)
		addiu	sp, 4
		bnez	v0, commcmd					; If comm_GetStatus returns pending
		nop									; data (non-zero) then query command
	@@return:
		lw		v0, SAVE_dcic				; Resume program execution
		nop
		mtc0	v0, DCIC
		bios_rfe							; Call actual ReturnFromException()
	@@checkbrk:								; Breakpoint check routine
		lw		v0, TCB_epc(k0)				; Get PC address
		nop
		lw		v1, 0(v0)					; Get opcode
		nop
		srl		v1, 6						; Check if break 1024 opcode
		andi	v1, 0xFFFF					; (pollhost)
		beq		v1, 1024, @@is_pollhost
		nop
		beq		v1, 512, breakexception		; Check if break 512 opcode
		nop									; (soft breakpoint)
		b		dbexception
		nop
	@@is_pollhost:
		addiu	v0, 4
		sw		v0, TCB_epc(k0)				; Set adjusted PC address to skip
		b		@@return					; break instruction
		nop
		
		; monitor_entry

;
; = Breakpoint exception handler
;
; This routine is only executed when a COP0 debug exception occurs and the
; processor jumps to the debug exception vector on address 040h. The breakpoints
; here are not to be confused with breakpoint instructions, which still vectors
; to the standard exception vector at address 080h.
;
breakhandler:
		lw		k0, PCB_addr				; Obtain TCB address
		nop
		lw		k0, 0(k0)
		nop
		sw		ra, TCB_ra(k0)				; Save some register values to TCB
		sw		v0, TCB_v0(k0)
		lw		v0, SAVE_dcic				; Obtain DCIC register value
		sw		v1, TCB_v1(k0)
		andi	v0, 0x20					; Mask DCIC for trace condition
		bnez	v0, @@istrace				; Check if trace condition
		nop
		mfc0	v0, CAUSE					; Check if break on branch delay
		lui		v1, 0x8000
		and		v0, v1
		beqz	v0, @@nobd					; If not then take it as a runto
		nop									; or trace completion
		mfc0	v0, CAUSE					; Check if branch was taken
		lui		v1, 0x4000                  ; (matched condition)
		and		v0, v1
		beqz	v0, @@notake
		nop
		mfc0	v0, JUMPDEST				; If taken, set JUMPDEST as program
		nop									; breakpoint address
		mtc0	v0, BPC
		b		@@destbreak					; Resume execution with break
		nop
	@@notake:								; If branch is not taken...
		mfc0	v0, EPC						; (note: EPC still points to the
		nop									; branch instruction even if
		addiu	v0, 8						; breakpoint was in the delay slot,
		mtc0	v0, BPC						; so must increment address by 8)
		b		@@destbreak					; Resume execution with break
		nop
	@@nobd:
		la		v0, db_step
		lbu		v1, 0(v0)
		nop
		bnez	v1, @@stepbreak
		nop
		la		v1, db_brkpass				; Obtain breakpoint pass status byte
		lbu		v1, 0(v1)
		nop
		bnez	v1, @@is_brkpass			; Branch to pass routine if set
		nop
	@@stepbreak:
		sb		r0, 0(v0)					; Clear step flag
		sw		at, TCB_at(k0)				; Simulate kernel saving 
		sw		a0, TCB_a0(k0)				; registers to current TCB
		sw		a1, TCB_a1(k0)
		sw		a2, TCB_a2(k0)
		sw		a3, TCB_a3(k0)
		sw		t0, TCB_t0(k0)
		sw		t1, TCB_t1(k0)
		sw		t2, TCB_t2(k0)
		sw		t3, TCB_t3(k0)
		sw		t4, TCB_t4(k0)
		sw		t5, TCB_t5(k0)
		sw		t6, TCB_t6(k0)
		sw		t7, TCB_t7(k0)
		sw		s0, TCB_s0(k0)
		sw		s1, TCB_s1(k0)
		sw		s2, TCB_s2(k0)
		sw		s3, TCB_s3(k0)
		sw		s4, TCB_s4(k0)
		sw		s5, TCB_s5(k0)
		sw		s6, TCB_s6(k0)
		sw		s7, TCB_s7(k0)
		sw		t8, TCB_t8(k0)
		sw		t9, TCB_t9(k0)
		lw		v0, SAVE_k0
		lw		v1, SAVE_k1
		sw		v0, TCB_k0(k0)
		sw		v1, TCB_k1(k0)
		sw		gp, TCB_gp(k0)
		sw		sp, TCB_sp(k0)
		sw		fp, TCB_fp(k0)
		mfc0	v0, EPC
		mfc0	v1, SR
		sw		v0, TCB_epc(k0)
		mfc0	v0, CAUSE
		sw		v1, TCB_sr(k0)
		sw		v0, TCB_cause(k0)
		mfhi	v0
		mflo	v1
		sw		v0, TCB_hi(k0)
		sw		v1, TCB_lo(k0)
		la		v1, db_state				; Update execution status
		li		v0, DB_STAT_BREAK
		sb		v0, 0(v1)
		jal		comm_DisableListen			; Adjust comms before debug mode
		nop
		jal		unset_breakpoints			; Undo breakpoint patches if any
		nop
		la		v0, db_runto				; Clear runto status
		sb		r0, 0(v0)
		b		cmdstart					; Branch to command query loop
		nop
	@@istrace:								; For break on jump/branch conditions
		mfc0	v1, EPC						; Get exception address
		lui		v0, 0xA180					; Enable software breakpoint
		addiu	v1, 4						; Adjust to breakpoint to delay slot
		mtc0	v1, BPC						; Set as program break address
		b		breakhandler_ret			; Resume execution
		sw		v0, SAVE_dcic
	@@destbreak:							; For jump/branch destination break
		lui		v0, 0xA180					; Enable software breakpoint
		b		breakhandler_ret			; Resume execution
		sw		v0, SAVE_dcic
	@@is_brkpass:							; For breakpoint pass condition
		jal		set_breakpoints				; Apply breakpoints again
		nop
		addiu	sp, -4
		sw		k0, 0(sp)
		sw		at, TCB_at(k0)				; Save a bunch of registers as
		sw		t0, TCB_t0(k0)				; FlushCache() uses them
		sw		t1, TCB_t1(k0)
		sw		t2, TCB_t2(k0)
		sw		t3, TCB_t3(k0)
		sw		t4, TCB_t4(k0)
		addiu	t2, r0, 0xA0				; FlushCache()
		jalr	t2
		addiu	t1, r0, 0x44
		lw		k0, 0(sp)
		addiu	sp, 4
		lw		at, TCB_at(k0)
		lw		t0, TCB_t0(k0)
		lw		t1, TCB_t1(k0)
		lw		t2, TCB_t2(k0)
		lw		t3, TCB_t3(k0)
		lw		t4, TCB_t4(k0)
		la		v0, db_runto				; Get runto state
		lbu		v0, 0(v0)
		sw		r0, SAVE_dcic				; Clear DCIC if no runto condition
		beqz	v0, @@no_runto				; is active
		nop
		la		v0, runto_addr				; Set runto parameters
		lw		v0, 0(v0)
		la		v1, runto_dcic
		lw		v1, 0(v1)
		nop		; arsemips weirdness, complains about load delay here
		mtc0	v0, BPC
		sw		v1, SAVE_dcic
	@@no_runto:
		la		v0, db_brkpass				; Clear breakpass flag
		sb		r0, 0(v0)
		b		breakhandler_ret			; Resume execution
		nop
		
	; breakhandler

;
; = Break handler return routine
;
; Used to resume program execution from the breakpoint exception vector.
;
breakhandler_ret:
		lw		v0, TCB_v0(k0)
		lw		v1, TCB_v1(k0)
		lw		ra, TCB_ra(k0)				; Restore return address
		lw		k0, SAVE_dcic				; Restore DCIC
		lw		k1, SAVE_k1					; Restore k1 (just in case)
		mtc0	k0, DCIC
		nop
		mfc0	k0, EPC						; Return from exception
		nop
		jr		k0
		rfe
		nop
		
		; breakhandler_ret
	
;
; = On command receive routine
;
; This routine is intended to be jumped to from monitor_entry.
;
commcmd:
		jal		unset_breakpoints			; Undo patched breakpoints if any
		nop
		jal		comm_ReadByte
		nop
		move	a0, v0
		jal		comm_DisableListen			; Adjust comms before debug mode
		nop
		lui		sp,0x8001					; Put stack to kernel memory space
		addiu	sp, -4
		b		querycmd
		move	v0, a0
		; commcmd

;
; = Breakpoint exception routine
;
breakexception:
		lw		a0, TCB_epc(k0)				; Get breakpoint flags
		jal		getbreakaddr
		nop
		bltz	v0, @@unknown_break
		nop
		lhu		v1, DB_BRK_LCNT(v0)			; Increment counter of breakpoint
		nop
		addiu	v1, 1
		sh		v1, DB_BRK_LCNT(v0)			; Store updated value
		lw		v1, 0(v0)
		nop
		andi	v1, 0x3						; Mask break mode bits
		beqz	v1, @@do_resume				; Resume execution if disabled
		nop
		beq		v1, 2, @@counter_break		; continue until counter reached
		nop
	@@unknown_break:				
		la		v1, db_state				; Set monitor state that a
		li		v0, DB_STAT_BREAK			; program breakpoint occurred
		sb		v0, 0(v1)
		b		dbbreak
		nop
	@@counter_break:						; Counter break
		addiu	sp, -4
		sw		v0, 0(sp)
		lhu		v1, DB_BRK_LCNT(v0)
		lhu		v0, DB_BRK_FLAG+2(v0)		; Retrieve counter target
		nop
		blt		v1, v0, @@do_resume			; Resume if counter not met
		nop
		lw		v0, 0(sp)					; Clear the counter
		addiu	sp, 4
		sh		r0, DB_BRK_LCNT(v0)
		b		@@unknown_break				; Branch to trigger breakpoint
		nop
	@@do_resume:
		jal		unset_breakpoints
		addiu	sp, 4
		b		resume
		nop
		
		; breakexception

;
; = Enter debug exception routine
;
dbexception:				
		la		v1, db_state				; Set monitor state that a
		li		v0, DB_STAT_EXCEPT			; program exception occurred
		sb		v0, 0(v1)

dbbreak:
		jal		comm_DisableListen			; Disable comms listen mode
		nop
		jal		unset_breakpoints			; Undo breakpoints
		nop
		
		; dbexception

;
; = Monitor command loop
;
cmdstart:
		lui		sp,0x8001					; Put stack to kernel memory space
		addiu	sp, -4
cmdloop:
		jal		comm_ReadByte
		nop
		bltz	v0, cmdloop					; Keep repeating if connection
		nop									; is in a dropped state
		
;
; = Query monitor commands (see commands.txt)
;
querycmd:
		bne		v0, CMD_REBOOT, @@no_reboot	; Check if reset command
		nop
		j		reboot
		nop
	@@no_reboot:
		blt		v0, 0xD0, @@cmdend			; Ignore command bytes that are
		nop									; out of range
		bgt		v0, 0xDB, @@cmdend
		nop
		subiu	v0, 0xD0					; Adjust command byte
		sll		v0, 2						; Multiply by four
		la		v1, cmd_table				; Apply command value offset
		addu	v0, v1
		lw		a0, 0(v0)					; Load function address
		nop
		jalr	a0							; Jump to it
		nop
	@@cmdend:
		la		v1, db_state				; if status is 4 (running),
		lbu		v0, 0(v1)					; return from exception
		nop
		beq		v0, DB_STAT_RUN, resume
		nop
		b		cmdloop
		nop
		
cmd_table:
		dw		cmd_getstatus	; D0
		dw		cmd_info		; D1
		dw		cmd_setexec		; D2
		dw		cmd_runto		; D3
		dw		cmd_setbrk		; D4
		dw		cmd_clrbrk		; D5
		dw		cmd_getregs		; D6
		dw		cmd_mem			; D7
		dw		cmd_word		; D8
		dw		cmd_getbrk		; D9
		dw		cmd_setreg		; DA
		dw		cmd_setdbrk		; DB
		
resume:										; Resume execution
		addiu	v1, r0, -1
		mtc0	v1, BPCM
		jal		set_breakpoints				; Apply the breakpoints
		nop
		bnez	v0, @@place_bpc				; If EPC is on a breakpoint,
		nop									; place a break address to step
		la		v0, db_runto				; If EPC is not on breakpoint
		lbu		v0, 0(v0)					; check if a runto operation is
		nop									; in progress
		beqz	v0, @@cont_resume			; skip if a runto is not set
		nop
		la		a0, runto_dcic				; set runto as software break
		lw		v0, 0(a0)
		la		a0, runto_addr
		lw		v1, 0(a0)
		sw		v0, SAVE_dcic
		mtc0	v1, BPC
	@@cont_resume:
		addiu	t2, r0, 0xA0				; FlushCache()
		jalr	t2
		addiu	t1, r0, 0x44
		addiu	sp, -4
		sw		ra, 0(sp)
		jal		comm_EnableListen			; Enable listening of the comms
		nop									; interface
		lw		ra, 0(sp)
		addiu	sp, 4
		lw		v0, SAVE_dcic				; Apply DCIC settings
		nop
		mtc0	v0, DCIC
		bios_rfe							; Return from exception
	@@place_bpc:
		mfc0	v0, EPC						; Set break address to the
		lui		v1, 0xA180					; opcode following EPC
		addiu	v0, 4
		mtc0	v0, BPC
		sw		v1, SAVE_dcic
		li		v0, 1						; Set that a breakpoint pass is
		la		v1, db_brkpass				; in progress
		sb		v0, 0(v1)
		b		@@cont_resume				; Continue resume sequence
		nop

;	
; = Performs a soft reboot
;
reboot:
		li		v1, 0x1F801814				; Blank video output
		li		v0, 0x03000001
		sw		v0, 0(v1)
		;la		v1, SIO_CTRL_REG_A			; Reset serial before reboot otherwise
		;li		v0, 0x40					; serial will stop listening properly
		;sh		v0, 0(v1)
		lui		v0, 0xBFC0					; Jump to BIOS ROM
		jr		v0
		nop
		
		; reboot

;
; = Queries info command
;
cmd_info:
		addiu	sp, -4
		sw		ra, 0(sp)
		la		a0, str_dbinfo				; Send info string
		jal		comm_SendStr
		nop
		lw		ra, 0(sp)
		addiu	sp, 4
		jr		ra
		nop
		
		; cmd_info

;
; = Queries get status
;
cmd_getstatus:
		addiu	sp, -4
		sw		ra, 0(sp)
		la		a0, db_state				; Send status byte
		jal		comm_SendByte
		lbu		a0, 0(a0)
		lw		ra, 0(sp)
		addiu	sp, 4
		jr		ra
		nop
		
		; cmd_getstatus

;
; = Queries set exec command
;
cmd_setexec:
		addiu	sp, -4
		sw		ra, 0(sp)
		jal		comm_ReadByte
		nop
		beq		v0, 0, @@setstate			; Program stop
		li		v1, DB_STAT_STOP
		beq		v0, 1, @@setstep			; Program step
		nop
		sw		r0, SAVE_dcic
		beq		v0, 2, @@setstate			; Program resume
		li		v1, DB_STAT_RUN
		b		@@return					; ignore any unknown value
		nop
	@@setstep:
		lw		v0, TCB_epc(k0)				; Set breakpoint for single step
		nop
		addiu	v0, 4
		mtc0	v0, BPC
		lui		v0, 0xA180					; Enable program break
		sw		v0, SAVE_dcic
		la		a0, db_step					; Set step flag
		li		v0, 1						; when starting from a breakpoint
		sb		v0, 0(a0)
		b		@@setstate					; Proceed to run state
		li		v1, DB_STAT_RUN
	@@setstate:
		move	v0, v1
		la		v1, db_state				; Set new state
		sb		v0, 0(v1)
	@@return:
		la		a0, db_state				; Send acknowledgement byte
		jal		comm_SendByte
		lbu		a0, 0(a0)
		lw		ra, 0(sp)
		addiu	sp, 4
		jr		ra
		nop
		
	; cmd_setexec
	

;
; = Run to specified address
;
cmd_runto:
		addiu	sp, -4
		sw		ra, 0(sp)
		jal		comm_ReadByte				; Get flags byte
		nop
		jal		comm_ReadReg				; Get target address
		move	a0, v0
		andi	a0, 0x1						; Check if trace bit is set
		bnez	a0, @@dotrace
		lui		v1, 0xB180					; Enable program break with trace
		lui		v1, 0xA180					; Enable program break
	@@dotrace:
		la		a0, runto_addr				; Store run-to address in case of
		sw		v0, 0(a0)					; breakpoint resume
		la		a0, runto_dcic				; Store DCIC settings in case of
		sw		v1, 0(a0)					; breakpoint resume
		li		v0, 1						; Set flag that a runto operation
		la		a0, db_runto				; is in progress
		sb		v0, 0(a0)
		li		v0, DB_STAT_RUN				; Set to run mode
		la		v1, db_state
		sb		v0, 0(v1)
		jal		comm_SendByte				; Send response byte
		move	a0, v0
		lw		ra, 0(sp)
		addiu	sp, 4
		jr		ra
		nop
		
	; cmd_runto

;
; = Set program breakpoint
;
cmd_setbrk:
		addiu	sp, -4
		sw		ra, 0(sp)
		jal		comm_ReadReg				; Get breakpoint address
		nop
		jal		comm_ReadReg				; Get flags value
		move	a0, v0
		move	a1, v0
		move	v0, a0
		andi	v0, 0x3						; Reject address if not aligned						
		bnez	v0, @@brk_badaddr
		nop
		la		a2, break_entries			; Get address to breakpoint list
		li		a3, MAX_BREAK
		addiu	v1, r0, -1
	@@brk_scan:
		lw		v0, DB_BRK_ADDR(a2)
		addiu	a3, -1
		beq		v0, a0, @@brk_update		; Branch to entry update if
		nop									; breakpoint already set
		bne		v0, -1, @@brk_next			; Advance if entry not unset
		nop
		sw		a0, DB_BRK_ADDR(a2)			; Complete the breakpoint entry
		sw		a1, DB_BRK_FLAG(a2)
		sh		r0, DB_BRK_LCNT(a2)			; Reset counter
		b		@@brk_set
		nop
	@@brk_next:
		bnez	a3, @@brk_scan
		addiu	a2, DB_BRK_LEN				; Advance to next breakpoint entry
		jal		comm_SendByte				; Send a zero when no breakpoint
		move	a0, r0						; slots available
		b		@@brk_exit
		nop
	@@brk_set:								; Breakpoint was set/updated,
		jal		comm_SendByte				; respond with 1
		li		a0, 1
	@@brk_exit:
		lw		ra, 0(sp)
		addiu	sp, 4
		jr		ra
		nop
	@@brk_badaddr:							; Send a bad address byte
		jal		comm_SendByte
		move	a0, r0
		b		@@brk_exit
		nop
	@@brk_update:
		sw		a1, DB_BRK_FLAG(a2)
		sh		r0, DB_BRK_LCNT(a2)			; Reset counter
		jal		comm_SendByte				; respond with 1
		li		a0, 2
		b		@@brk_exit
		nop
		
	; cmd_setbrk
	
;
; = Clear program breakpoints
;
cmd_clrbrk:
		addiu	sp, -4
		sw		ra, 0(sp)
		la		a2, break_entries
		li		a3, MAX_BREAK
		addiu	v0, r0, -1
	@@clear_loop:
		sw		v0, DB_BRK_INST(a2)
		sw		v0, DB_BRK_ADDR(a2)
		sw		r0, DB_BRK_FLAG(a2)
		addiu	a3, -1
		bnez	a3, @@clear_loop
		addiu	a2, DB_BRK_LEN
		jal		comm_SendByte
		li		a0, 1
		lw		ra, 0(sp)
		addiu	sp, 4
		jr		ra
		nop
		
	; cmd_clrbrk
	
;
; = Do memory I/O operation
;
cmd_word:
		addiu	sp, -4
		sw		ra, 0(sp)
		jal		comm_ReadByte				; Get type of operation
		nop
		beq		v0, 1, @@dowrite
		nop
		beq		v0, -1, @@cancel
		nop
		jal		comm_ReadByte				; Get word type
		nop
		beq		v0, -1, @@cancel
		nop
		jal		comm_ReadReg				; Get read address
		move	a0, v0
		beq		v0, -1, @@cancel
		nop
		beq		a0, 2, @@rw32
		nop
		beq		a0, 1, @@rw16
		nop
		jal		comm_SendReg				; Do byte read
		lbu		a0, 0(v0)
		b		@@cancel
		nop
	@@rw16:
		jal		comm_SendReg				; Do halfword read
		lhu		a0, 0(v0)
		b		@@cancel
		nop
	@@rw32:
		jal		comm_SendReg				; Do word read
		lw		a0, 0(v0)
	@@cancel:
		lw		ra, 0(sp)
		addiu	sp, 4
		jr		ra
		nop
	@@dowrite:
		jal		comm_ReadByte				; Get word type
		nop
		beq		v0, -1, @@cancel
		nop
		jal		comm_ReadReg				; Get write address
		move	a1, v0
		beq		v0, -1, @@cancel
		nop
		jal		comm_ReadReg				; Get write value
		move	a0, v0
		beq		v0, -1, @@cancel
		nop
		beq		a1, 2, @@ww32
		nop
		beq		a1, 1, @@ww16
		nop
		b		@@done						; Write byte value
		sb		v0, 0(a0)
	@@ww16:
		b		@@done						; Write halfword value
		sh		v0, 0(a0)
	@@ww32:
		sw		v0, 0(a0)					; Write word value
	@@done:
		la		a0, db_state				; Send acknowledge byte
		jal		comm_SendByte
		lbu		a0, 0(a0)
		b		@@cancel
		nop
		
	; cmd_word
		
cmd_getregs:
		addiu	sp, -4
		sw		ra, 0(sp)
		addiu	a1, k0, TCB_r0				; Send registers 0-34 from TCB
		addi	a2, r0, 36					; registers 32-34 are EPC,hi,lo
	@@regloop:								
		lw		a0, 0(a1)
		jal		comm_SendReg
		addi	a2, -1
		bltz	v0, @@cancel
		nop
		bgtz	a2, @@regloop
		addiu	a1, 4
		lw		a0, TCB_cause(k0)			; Send CAUSE register value
		jal		comm_SendReg
		nop
		bltz	v0, @@cancel
		nop
		mfc0	a0, BADVADDR				; Send bad address
		jal		comm_SendReg
		nop
		bltz	v0, @@cancel
		nop
		mfc0	a0, JUMPDEST				; Send JUMPDEST
		jal		comm_SendReg
		nop
		bltz	v0, @@cancel
		nop
		lw		a0, SAVE_dcic				; Send last DCIC
		jal		comm_SendReg
		nop
		bltz	v0, @@cancel
		nop
		lw		a0, TCB_epc(k0)				; Send opcode
		nop
		move	v0, a0						; Don't try to read opcode
		andi	v0, 0x3						; if address is not aligned or
		bnez	v0, @@skipload				; the debug monitor will crash
		nop
		lw		a0, 0(a0)
	@@skipload:
		jal		comm_SendReg
		nop
		lw		a0, TCB_epc(k0)				; Send opcode following it
		nop
		addiu	a0, 4
		move	v0, a0						; Don't try to read opcode
		andi	v0, 0x3						; if address is not aligned or
		bnez	v0, @@skipload2				; the debug monitor will crash
		nop
		lw		a0, 0(a0)
	@@skipload2:
		jal		comm_SendReg
		nop
	@@cancel:
		lw		ra, 0(sp)
		addiu	sp, 4
		jr		ra
		nop
		
	; cmd_getregs


cmd_mem:									; = Queries download/upload memory
		addiu	sp, -4
		sw		ra, 0(sp)
		jal		comm_ReadByte				; Get type of operation
		nop
		beq		v0, 1, @@dowrite
		nop
		jal		comm_ReadReg				; Get source address value
		nop
		beq		v0, -1, @@cancel
		move	a0, v0
		jal		comm_ReadReg				; Get number of bytes to get
		nop
		beq		v0, -1, @@cancel
		move	a1, v0
		jal		comm_BlockSend
		nop
		b		@@cancel
		nop
	@@dowrite:
		jal		comm_ReadReg				; Get target address value
		nop
		beq		v0, -1, @@cancel
		move	a0, v0
		jal		comm_ReadReg				; Get number of bytes to upload
		nop
		beq		v0, -1, @@cancel
		move	a1, v0
		jal		comm_BlockRead				; Download block of data
		nop
	@@cancel:
		lw		ra, 0(sp)
		addiu	sp, 4
		jr		ra
		nop
		; cmd_mem


cmd_getbrk:
		addiu	sp, -4
		sw		ra, 0(sp)
		la		a1, break_entries
		li		a2, MAX_BREAK
	@@get_bloop:
		lw		a0, DB_BRK_ADDR(a1)
		addiu	v0, r0, -1
		beq		a0, v0, @@end_bloop
		nop
		lw		a0, DB_BRK_ADDR(a1)
		jal		comm_SendReg
		nop
		lw		a0, DB_BRK_FLAG(a1)
		jal		comm_SendReg
		nop
		lw		a0, DB_BRK_LCNT(a1)
		jal		comm_SendReg
		nop
		addiu	a2, -1
		bnez	a2, @@get_bloop
		addiu	a1, DB_BRK_LEN
	@@end_bloop:
		jal		comm_SendReg				; Send terminator word
		addiu	a0, r0, -1
		lw		ra, 0(sp)
		addiu	sp, 4
		jr		ra
		nop
		; cmd_getbrk
		

cmd_setreg:
		addiu	sp, -4
		sw		ra, 0(sp)
		jal		comm_ReadByte				; Get register number
		nop
		beq		v0, -1, @@cancel
		nop
		move	a1, v0
		jal		comm_ReadReg				; Get new value
		nop
		beq		v0, -1, @@cancel
		nop
		beqz	a1, @@skip					; Ignore register 0 changes
		nop
		lw		k0, PCB_addr				; Retrieve address of current TCB
		sll		a1, 2						; Multiply reg number by 4
		lw		k0, 0(k0)					; Load the address
		move	a0, v0
		addiu	a1, TCB_r0
		addu	v0, k0, a1					; k0 must point to TCB
		sw		a0, 0(v0)					; Set new register value
	@@skip:
		jal		comm_SendByte				; Send acknowledge byte
		move	a0, r0
	@@cancel:
		lw		ra, 0(sp)
		addiu	sp, 4
		jr		ra
		nop
		; cmd_setreg
		
		
cmd_setdbrk:
		addiu	sp,-4
		sw		ra,0(sp)
		jal		comm_ReadReg				; Get memory break address
		nop
		mtc0	v0,BDA						; Set as data break address
		jal		comm_ReadReg				; Get memory compare mask
		nop
		mtc0	v0,BDAM						; Set as data break mask
		jal		comm_ReadByte				; Get memory break flags
		sb		v0,db_dbrkflags
		jal		comm_SendByte				; Send acknowledge byte
		move	a0, r0
		lw		ra,0(sp)
		addiu	sp,4
		jr		ra
		nop
		; cmd_setmembrk
		
		
getbreakaddr:
		la		a1, break_entries
		li		a2, MAX_BREAK
	@@flag_loop:
		lw		v0, DB_BRK_ADDR(a1)
		addiu	a2, -1
		beq		v0, a0, @@flag_found
		nop
		bnez	a2, @@flag_loop
		addiu	a1, DB_BRK_LEN
		jr		ra
		addiu	v0, r0, -1
	@@flag_found:
		jr		ra
		move	v0, a1
		
		; getbreakflags
		
;
; = Backs up original opcodes and places break opcodes
;
; Destroys: v1
; Returns: Non-zero if EPC was on a breakpoint (step required)
;
set_breakpoints:
		addiu	sp, -16
		sw		a0, 0(sp)
		sw		a1, 4(sp)
		sw		a2, 8(sp)
		sw		a3, 12(sp)
		la		a2, break_entries
		li		a3, MAX_BREAK
		move	a1, r0
	@@set_loop:
		lw		v1, DB_BRK_ADDR(a2)			; Test if breakpoint is active
		addiu	v0, r0, -1
		beq		v0, v1, @@set_cont			; Skip if not set
		nop
		mfc0	v1, EPC						
		lw		a0, DB_BRK_ADDR(a2)			; Don't patch break if on EPC
		nop
		beq		v1, a0, @@on_epc
		nop
		lw		v1, DB_BRK_INST(a2)			; Check if address already patched
		addiu	v0, r0, -1
		bne		v0, v1, @@set_cont			; Skip if already set
		nop
		la		v1, break_inst				; Insert the break instruction
		lw		v1, 0(v1)
		lw		v0, 0(a0)
		sw		v1, 0(a0)
		sw		v0, DB_BRK_INST(a2)
		b		@@set_cont					; Continue to next iteration
		nop
	@@on_epc:
		addiu	a1, 1						; Set as flag that EPC points to a
	@@set_cont:								; breakpoint
		addiu	a3, -1
		bnez	a3, @@set_loop
		addiu	a2, DB_BRK_LEN
		move	v0, a1
		lw		a0, 0(sp)
		lw		a1, 4(sp)
		lw		a2, 8(sp)
		lw		a3, 12(sp)
		jr		ra
		addiu	sp, 16
		
		; set_breakpoints

;
; = Undoes any patched break opcodes
;
; Destroys: v0, v1
; Returns: none
;
unset_breakpoints:
		addiu	sp, -12
		sw		a0, 0(sp)
		sw		a2, 4(sp)
		sw		a3, 8(sp)
		la		a2, break_entries
		li		a3, MAX_BREAK
	@@set_loop:
		lw		v1, DB_BRK_ADDR(a2)			; Test if breakpoint enabled
		addiu	v0, r0, -1
		beq		v0, v1, @@set_cont
		nop
		lw		v1, DB_BRK_INST(a2)			; Check if address already unpatched
		lw		a0, DB_BRK_ADDR(a2)
		beq		v0, v1, @@set_cont
		nop
		sw		v1, 0(a0)
		sw		v0, DB_BRK_INST(a2)
	@@set_cont:
		addiu	a3, -1
		bnez	a3, @@set_loop
		addiu	a2, 16
		lw		a0, 0(sp)
		lw		a2, 4(sp)
		lw		a3, 8(sp)
		jr		ra
		addiu	sp, 12
		
		; unset_breakpoints
		
;
; = Variables area
;

break_inst:		break 512					; Break opcode to use with soft
											; program breakpoints

rfe_ptr:		dw 0						; Original ReturnFromException()
											; function address
											
runto_addr:		dw 0						; Target address of run-to
runto_dcic:		dw 0						; DCIC settings for run-to

db_state:		db DB_STAT_RUN				; Monitor execution state
db_runto:		db 0						; Monitor run-to state
db_step:		db 0						; Program step on-going
db_brkpass:		db 0						; Break pass flag

db_dbrkflags:	db 0						; Memory breakpoint flags

.align 4
break_entries:

monitor_end:
.close

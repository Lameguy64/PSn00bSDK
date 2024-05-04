CMD_REBOOT		equ		0xA0

; ****************************
; Debug monitor commands
; (see commands.txt)
; ****************************

CMD_DB_GETSTAT	equ		0xD0	; Get status about the debug monitor
CMD_DB_GETINFO	equ		0xD1	; Get info about debug monitor
CMD_DB_SETEXEC	equ		0xD2	; Set program execution mode
CMD_DB_RUNTO	equ		0xD3	; Run or trace to specified address
CMD_DB_SETBRK	equ		0xD4	; Set a program breakpoint
CMD_DB_CLRBRK	equ		0xD5	; Clear program breakpoints
CMD_DB_GETREGS	equ		0xD6	; Get processor registers (including debug regs)
CMD_DB_MEM		equ		0xD7	; Get region of memory
CMD_DB_WORD		equ		0xD8	; Get word of memory
CMD_DB_GETBRK	equ		0xD9	; Get currently set breakpoints
CMD_DB_MEMBRK	equ		0xDB	; Set memory access breakpoint registers

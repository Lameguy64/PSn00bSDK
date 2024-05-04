; Experimental PSn00bDEBUG Mk2 debug monitor by John Wilbert Villamor
; Copyright 2020 Meido-Tek Productions (Lameguy64)
;
; These are the communication routines for the monitor. This file can be
; swapped out with a different set of routines to adapt it for other
; interfaces.

SIO_TXRX_REG_A		equ		0x1F801050	; Serial I/O port absolute addresses
SIO_MODE_REG_A		equ		0x1F801058
SIO_CTRL_REG_A		equ		0x1F80105A
SIO_BAUD_REG_A		equ		0x1F80105E
SIO_STAT_REG_A		equ		0x1F801054

SIO_BAUD			equ		0x12		; Baud rate constant

; = Function definitions of communication routines
;
comm_ReadByte		equ sioReadByte
comm_ReadReg		equ sioReadReg
comm_BlockRead		equ sioBlockRead
comm_SendByte		equ sioSendByte
comm_SendReg		equ sioSendReg
comm_BlockSend		equ sioBlockSend
comm_SendStr		equ sioSendStr
comm_DisableListen	equ sioDisableListen
comm_EnableListen	equ sioEnableListen
comm_GetStatus		equ sioGetStatus

; sioReadByte - Read a byte from the serial port.
;
; Arguments: none
;
; Destroys: v0, v1
;
; Returns: v0 - byte read
;
sioReadByte:
		la		v1, SIO_CTRL_REG_A		; Turn on RTS
		lhu		v0, 0(v1)
		nop
		ori		v0, 0x22
		sh		v0, 0(v1)
	@@rx_wait:
		la		v0, SIO_STAT_REG_A		; Read status register
		lw		v0, 0(v0)
		nop
		andi	v0, 0x80				; Check DSR status
		beqz	v0, @@cancel
		addi	v0, r0, -1
		la		v0, SIO_STAT_REG_A		; Read value register
		lw		v0, 0(v0)
		nop
		andi	v0, 0x2
		beqz	v0, @@rx_wait
		nop
		la		v0, SIO_TXRX_REG_A
		lbu		v0, 0(v0)
	@@cancel:
		addiu	sp, -4
		sw		v0, 0(sp)
		la		v1, SIO_CTRL_REG_A		; Turn off RTS
		lhu		v0, 0(v1)
		nop
		xori	v0, 0x20
		sh		v0, 0(v1)
		lw		v0, 0(sp)
		jr		ra
		addiu	sp, 4
		; sioReadByte


; sioSendByte - Send a byte to the serial port
;
; Input: a0 - byte to send to serial.
;
; Destroys: v0
;
; Returns: 0 on success, -1 on drop
;
sioSendByte:							
	@@cts_wait:							; Wait for Clear-to-Send signal
		la		v0, SIO_STAT_REG_A		; Check DSR line
		lhu		v0, 0(v0)
		nop
		andi	v0, 0x80
		beqz	v0, @@cancel
		addi	v0, r0, -1
		la		v0, SIO_STAT_REG_A		; Wait for CTS
		lhu		v0, 0(v0)
		nop
		andi	v0, 0x100
		beqz	v0, @@cts_wait
		nop
	@@tx_ready:							; Wait for TX to become ready
		la		v0, SIO_STAT_REG_A		; Check DSR line
		lhu		v0, 0(v0)
		nop
		andi	v0, 0x80
		beqz	v0, @@cancel
		addi	v0, r0, -1
		la		v0, SIO_STAT_REG_A
		lhu		v0, 0(v0)
		nop
		andi	v0, 0x1
		beqz	v0, @@tx_ready
		nop
		la		v0, SIO_TXRX_REG_A		; Send byte
		sb		a0, 0(v0)
	@@tx_done:							; Wait for TX to finish
		la		v0, SIO_STAT_REG_A		; Check DSR line
		lhu		v0, 0(v0)
		nop
		andi	v0, 0x80
		beqz	v0, @@cancel
		addi	v0, r0, -1
		la		v0, SIO_STAT_REG_A
		lhu		v0, 0(v0)
		nop
		andi	v0, 0x4
		beqz	v0, @@tx_done
		nop
		move	v0, r0
	@@cancel:
		jr		ra						; Return
		nop
		; sioSendByte


; sioBlockSend - Send a block of data to the serial port
;
; Arguments:
;	a0 - Pointer to start sending data from
;	a1 - Number of bytes to send
;
sioBlockSend:
		addiu	sp, -4
		sw		ra, 0(sp)
		move	a2, a0
	@@sendloop:
		lbu		a0, 0(a2)					; Read and send a byte
		jal		sioSendByte
		addiu	a2, 1
		beq		v0, -1, @@cancel			; Cancel if connection dropped
		addiu	a1, -1
		bgtz	a1, @@sendloop
		nop
	@@cancel:
		lw		ra, 0(sp)
		addiu	sp, 4
		jr		ra
		nop
		; sioBlockSend
		
		
; sioBlockRead - Read a block of data from the serial port
;
; Arguments:
;	a0 - Pointer to store read data to
;	a1 - Number of bytes to read
;
sioBlockRead:
		addiu	sp, -4
		sw		ra, 0(sp)
	@@readloop:
		jal		sioReadByte
		nop
		beq		v0, -1, @@cancel			; Check if comms got terminated
		addiu	a1, -1
		sb		v0, 0(a0)
		bgtz	a1, @@readloop
		addiu	a0, 1
	@@cancel:
		lw		ra, 0(sp)
		addiu	sp, 4
		jr		ra
		nop
		; sioBlockRead


; sioSendStr - Send a string to serial port
;
; Arguments: a0 - pointer to string
;
; Destroys: a0 and a1
;
; Returns: none
;
sioSendStr:
		addiu	sp, -4
		sw		ra, 0(sp)
		move	a1, a0
	@@send_loop:
		lbu		a0, 0(a1)			; Get byte
		nop
		jal		sioSendByte			; Send byte
		addiu	a1, 1
		bltz	v0, @@send_end		; End transmission if DSR was dropped
		nop
		bnez	a0,	@@send_loop		; Loop until terminator is sent
		nop
	@@send_end:
		lw		ra, 0(sp)
		addiu	sp, 4
		jr		ra
		nop
		; sioSendStr
		

; sioSendStr - Send 4 bytes of a 32-bit register
;
; Input: a0 - Value to send.
;
; Return: none.
;
sioSendReg:
		addiu	sp, -24
		sw		ra, 0(sp)
		sw		v0, 4(sp)
		sw		v1, 8(sp)
		sw		a0, 12(sp)
		sw		a1, 16(sp)
		sw		a2, 20(sp)
		move	a1, zero
		move	a2, a0
	@@write_loop:
		andi	a0, a2, 0xFF
		jal		sioSendByte
		srl		a2, 8
		bltz	v0, @@cancel
		addiu	v0, r0, -1
		li		at, 3
		bne		a1, at, @@write_loop
		addiu	a1, 1
	@@cancel:
		lw		ra, 0(sp)			; Restore stack and return
		lw		v0, 4(sp)
		lw		v1, 8(sp)
		lw		a0, 12(sp)
		lw		a1, 16(sp)
		lw		a2, 20(sp)
		jr		ra
		addiu	sp, 24
		; sioSendReg


; sioReadReg - Receives 4 bytes to a register
;
; Arguments: none.
;
; Return: v0 - Received word.
;
sioReadReg:
		addiu	sp, -16
		sw		ra, 0(sp)			; Save stuff to stack
		sw		a0, 4(sp)
		sw		a1, 8(sp)
		sw		a2, 12(sp)
		move	a0, r0				; Clear registers
		move	a1, r0
		move	a2, r0
	@@loop:
		jal		sioReadByte			; Receive a byte
		nop
		bltz	v0, @@cancel		; Cancel if return value is negative
		sllv	v0, a0				; Shift return value to its respective
		addiu	a0, 8				; bit location in the register
		slti	at, a0, 32
		bnez	at, @@loop
		or		a2, v0				; Merge it to the register
		b		@@done
		move	v0, a2
	@@cancel:
		addiu	v0, r0, -1			; Set return value to -1 if canceled
	@@done:
		lw		ra, 0(sp)
		lw		a0, 4(sp)
		lw		a1, 8(sp)
		lw		a2, 12(sp)
		jr		ra
		addiu	sp, 16
		; sioReadReg


; sioEnableListen - Enable asynchronous listening of the serial port
;
;   This function simply enables SIO interrupts and sets RTS and DTR high to
;   allow debug commands to be issued to the target console. This is executed
;   on every interrupt to make sure the SIO interface can receive comamnds at
;   any time, and when the debug monitor leaves query mode.
;
;   The monitor leaves query mode with the following commands:
;	- When the monitor completes a command and resumes execution (if exec 
;	  mode is set to run).
;	- When the monitor is instructed to continue execution from a paused state.
;	- When a trace or run-to operation is issued while the target is stopped.
;
; Arguments: none
;
; Destroys: v0 and v1
;
; Returns: none
;
sioEnableListen:
		la		v1, SIO_CTRL_REG_A		; Enable RTS and DTR so PC can send
		lhu		v0, 0(v1)
		nop
		ori		v0, 0x22
		sh		v0, 0(v1)
		jr		ra
		nop
		; sioEnableListen
		

; sioDisableListen - Disable asynchronous listening of the serial port
;
;   This function simply disables SIO interrupts and sets RTS and DTR low
;   to allow handshaking. This is executed when the debug monitor enters query
;	mode, when it has received a command, told to stop the target or when a
;   break/trace/unhandled exception occurs.
;
; Arguments: none
;
; Destroys: v0 and v1
;
; Returns: none
;
sioDisableListen:
		la		v1, SIO_CTRL_REG_A			
		lhu		v0, 0(v1)
		nop
		andi	v0, 0xF					; Disable interrupts and RTS
		ori		v0, 0x10				; Acknowledge serial just in case
		sh		v0, 0(v1)
		jr		ra
		nop
		; sioDisableListen


; sioGetStatus - Get status if there's any pending data in the serial port
;
;   This function checks if the SIO interface has connection with the host
;   system (when CTS is set) and if there's data pending in the RX buffer of
;   the SIO interface. This function is executed on every interrupt, to check
;   for any commands pending in the SIO interface.
;
; Arguments: none
; Destroys: v0 and v1
; Returns: Zero if no data pending, non-zero if there's data pending.
;
sioGetStatus:
		la		v0, SIO_STAT_REG_A		; Check serial status
		lhu		v0, 0(v0)
		nop
		andi	v1, v0, 0x80			; Check if CTS is set
		beqz	v1, @@nodata
		andi	v0, 0x2
		beqz	v0, @@nodata			; Check if rx buffer is filled
		nop
		jr		ra
		li		v0, 1
	@@nodata:
		jr		ra
		move	v0, r0

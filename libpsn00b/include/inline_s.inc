# Inline GTE macros for GNU assembler (as).
#
# Part of the PSn00bSDK Project by Lameguy64.
# 2019 Meido-Tek Productions
#
# Similar to inline_c.h, it does not require running your object file
# through some silly tool.

.macro nRTPS
	nop
	nop
	cop2 0x0180001
.endm

.macro nRTPT
	nop
	nop
	cop2 0x0280030
.endm

.macro nNCLIP
	nop
	nop
	cop2 0x1400006
.endm
	
.macro nAVSZ3
	nop
	nop
	cop2 0x158002D
.endm
	
.macro nAVSZ4
	nop
	nop
	cop2 0x168002E
.endm

.macro nMVMVA sf mx v cv lm
	nop
	nop
	cop2	0x0400012|(\sf<<19)|(\mx<<17)|(\v<<15)|(\cv<<13)|(\lm<<10)
.endm
	
.macro nSQR sf
	nop
	nop
	cop2	0x0A00428|(\sf<<19)
.endm
	
.macro nnOP sf lm	# extra n to prevent conflict with the nop opcode
	nop
	nop
	cop2	0x170000C|(\sf<<19)|(\lm<<10)
.endm
	
.macro nNCS
	nop
	nop
	cop2	0x0C8041E
.endm
	
.macro nNCT
	nop
	nop
	cop2	0x0D80420
.endm
	
.macro nNCCS
	nop
	nop
	cop2	0x108041B
.endm
	
.macro nNCCT
	nop
	nop
	cop2	0x118043F
.endm
	
.macro nNCDS
	nop
	nop
	cop2	0x0E80413
.endm
	
.macro nNCDT
	nop
	nop
	cop2	0x0F80416
.endm
	
.macro nCC
	nop
	nop
	cop2	0x138041C
.endm
	
.macro nCDP
	nop
	nop
	cop2	0x1280414
.endm
	
.macro nDCPL
	nop
	nop
	cop2	0x0680029
.endm
	
.macro nDPCS
	nop
	nop
	cop2	0x0780010
.endm
	
.macro nDPCT
	nop
	nop
	cop2	0x0180001
.endm

.macro nINTPL
	nop
	nop
	cop2	0x0980011
.endm

.macro nGPF sf
	nop
	nop
	cop2	0x190003D|(\sf<<19)
.endm
	
.macro nGPL sf
	nop
	nop
	cop2	0x1A0003E|(\sf<<19)
.endm
	
#
# Macros without leading nops (for optimized usage)
#
.macro RTPS
	cop2 0x0180001
.endm

.macro RTPT
	cop2 0x0280030
.endm

.macro NCLIP
	cop2 0x1400006
.endm
	
.macro AVSZ3
	cop2 0x158002D
.endm
	
.macro AVSZ4
	cop2 0x168002E
.endm

.macro MVMVA sf mx v cv lm
	cop2	0x0400012|(\sf<<19)|(\mx<<17)|(\v<<15)|(\cv<<13)|(\lm<<10)
.endm
	
.macro SQR sf
	cop2	0x0A00428|(\sf<<19)
.endm
	
.macro OP sf lm
	cop2	0x170000C|(\sf<<19)|(\lm<<10)
.endm
	
.macro NCS
	cop2	0x0C8041E
.endm
	
.macro NCT
	cop2	0x0D80420
.endm
	
.macro NCCS
	cop2	0x108041B
.endm
	
.macro NCCT
	cop2	0x118043F
.endm
	
.macro NCDS
	cop2	0x0E80413
.endm
	
.macro NCDT
	cop2	0x0F80416
.endm
	
.macro CC
	cop2	0x138041C
.endm
	
.macro CDP
	cop2	0x1280414
.endm
	
.macro DCPL
	cop2	0x0680029
.endm
	
.macro DPCS
	cop2	0x0780010
.endm
	
.macro DPCT
	cop2	0x0180001
.endm

.macro INTPL
	cop2	0x0980011
.endm

.macro GPF sf
	cop2	0x190003D|(\sf<<19)
.endm

.macro GPL sf
	cop2	0x1A0003E|(\sf<<19)
.endm

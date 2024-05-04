#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "mips_disassembler.h"
#include "pstypes.h"

#define OPCODE1(p)	( (p>>26)&0x3F )
#define OPCODE2(p)	( p&0x3F )
#define OPREGS(p)	( (p>>21)&0x1f )
#define OPREGT(p)	( (p>>16)&0x1f )
#define OPREGD(p)	( (p>>11)&0x1f )

#define OPIMM5(p)	( (p>>6)&0x1f )
#define OPIMM16(p)	( p&0xffff )
#define OPIMM20(p)	( (p>>6)&0x1FFFFF )
#define OPIMM25(p)	( p&0x1FFFFFF )
#define OPIMM26(p)	( p&0x3FFFFFF )
#define EXTEND26(p)	(((int)(p<<6))>>6)
#define EXTEND16(p) (((int)(p<<16))>>16)

static const char* regs[] = {
	"r0","at","v0","v1",
	"a0","a1","a2","a3",
	"t0","t1","t2","t3",
	"t4","t5","t6","t7",
	"s0","s1","s2","s3",
	"s4","s5","s6","s7",
	"t8","t9","k0","k1",
	"gp","sp","fp","ra"
};

static void Decode_cop(unsigned int opcode, char* output) {
	
	int copnum = OPCODE1(opcode)&0x3;
	
	const char* cop0_nick[32] = {
		"N/A","N/A","N/A","BPC",		// r0-r3
		"N/A","BDA","JUMPDEST","DCIC",	// r4-r7
		"BadVaddr","BDAM","N/A","BPCM",	// r8-r11
		"SR","CAUSE","EPC","PRID",		// r12-r15
		"N/A","N/A","N/A","N/A",
		"N/A","N/A","N/A","N/A",
		"N/A","N/A","N/A","N/A",
		"N/A","N/A","N/A","N/A",
	};
	
	const char* cop2_data_nick[32] = {
		"VXY0","VZ0","VXY1","VZ1",		// r0-r3
		"VXY2","VZ2","RGBC","OTZ",		// r4-r7
		"IR0","IR1","IR2","IR3",		// r8-r11
		"SXY0","SXY1","SXY2","SXYP",	// r12-r15
		"SZ0","SZ1","SZ2","SZ3",
		"RGB0","RGB1","RGB2","RES1",
		"MAC0","MAC1","MAC2","MAC3",
		"IRGB","ORGB","LZCS","LZCR",
	};
	
	const char* cop2_ctrl_nick[32] = {
		"RT11","RT12","RT13","RT21",	// r0-r3
		"RT22","RT23","RT31","RT32",	// r4-r7
		"RT33","TRX","TRY","TRZ",		// r8-r11
		"LR1","LR2","LR3","LG1",		// r12-r15
		"LG2","LG3","LB1","LB2",
		"LB3","RFC","GFC","BFC",
		"OFX","OFY","H","DQA",
		"DQB","ZSF3","ZSF4","FLAG",
	};
	
	char temp[16];
	
	if( (OPCODE1(opcode)&0x20) == 0 ) {
		
		switch(OPREGS(opcode)) {
			case 0x0:	// mfc?
				sprintf( output, "mfc%d    %s, r%d", copnum, 
					regs[OPREGT(opcode)],
					OPREGD(opcode) );
				
				if ( copnum == 0 ) {
				
					sprintf( temp, "  (%s)", cop0_nick[OPREGD(opcode)] );
					strcat( output, temp );
				
				} else if ( copnum == 2 ) {

					sprintf( temp, "  (%s)", cop2_data_nick[OPREGD(opcode)] );
					strcat( output, temp );

				}
				
				break;
			case 0x2:	// cfc?
				
				sprintf( output, "cfc%d    %s, r%d", copnum, 
					regs[OPREGT(opcode)],
					OPREGD(opcode) );
				
				if ( copnum == 2 ) {

					sprintf( temp, "  (%s)", cop2_ctrl_nick[OPREGD(opcode)] );
					strcat( output, temp );

				}
				
				break;
			case 0x4:	// mtc?
				
				sprintf( output, "mtc%d    %s, r%d", copnum, 
					regs[OPREGT(opcode)],
					OPREGD(opcode) );
				
				if ( copnum == 0 ) {
				
					sprintf( temp, "  (%s)", cop0_nick[OPREGD(opcode)] );
					strcat( output, temp );
				
				}  else if ( copnum == 2 ) {

					sprintf( temp, "  (%s)", cop2_data_nick[OPREGD(opcode)] );
					strcat( output, temp );

				}
				
				break;
			case 0x6:	// ctc?
				
				sprintf( output, "ctc%d    %s, r%d", copnum, 
					regs[OPREGT(opcode)], OPREGT(opcode) );
				
				if ( copnum == 2 )
				{
					sprintf( temp, "  (%s)", cop2_ctrl_nick[OPREGT(opcode)] );
					strcat( output, temp );
				}
				
				break;
				
			case 0x8:	// bc?f bc?t
				if( OPREGT(opcode) == 1 )
				{
					sprintf( output, "bc%dt    $%X", copnum, 
						OPIMM16(opcode) );
				}
				else
				{
					sprintf( output, "bc%df    $%X", copnum, 
						OPIMM16(opcode) );
				}
				break;
				
			default:	// cop?
				
				//strcpy( output, "cp" );
				//sprintf( output, "%x", OPREGS(opcode) );
				
				if( OPREGS(opcode)&0x10 )
				{
					sprintf( output, "cop%d    $%X", copnum, OPIMM25(opcode) );
				}
				break;
		}
		
	} else {
		
		if( (OPCODE1(opcode)&0x8) == 0 ) {
			
			sprintf( output, "lwc%d    r%d,%d(%s)", copnum, 
				OPREGT(opcode), EXTEND16(OPIMM16(opcode)),
				regs[OPREGS(opcode)] );
			
			if ( copnum == 0 ) {
				
				sprintf( temp, "  (%s)", cop0_nick[OPREGT(opcode)] );
				strcat( output, temp );
				
			} else if ( copnum == 2 ) {
				
				sprintf( temp, "  (%s)", cop2_data_nick[OPREGT(opcode)] );
				strcat( output, temp );
				
			}
			
		} else {
			
			sprintf( output, "swc%d    r%d,%d(%s)", copnum, 
				OPREGT(opcode), EXTEND16(OPIMM16(opcode)),
				regs[OPREGS(opcode)] );
			
			if ( copnum == 0 ) {
			
				sprintf( temp, "  (%s)", cop0_nick[OPREGT(opcode)] );
				strcat( output, temp );
			
			} else if ( copnum == 2 ) {
				
				sprintf( temp, "  (%s)", cop2_data_nick[OPREGT(opcode)] );
				strcat( output, temp );
				
			}
			
		}
		
	}
	
}

void mips_Decode(unsigned int opcode, unsigned int addr, char* output, int arrows) {

	char temp[32];
	int immval;
	
	const char* pri_op[] = {
		NULL		,NULL		,"j       "	,"jal     ",//4
		"beq     "	,"bne     "	,"blez    "	,"bgtz    ",//8
		"addi    "	,"addiu   "	,"slti    "	,"sltiu   ",//12
		"andi    "	,"ori     "	,"xori    "	,"lui     ",//16
		"cop0    "	,"cop1<!> "	,"cop2    "	,"cop3<!> ",//24
		NULL		, NULL		,NULL		, NULL,		//32
		NULL		, NULL		,NULL		, NULL,		//36
		NULL		, NULL		,NULL		, NULL,		//40
		"lb      "	,"lh      "	,"lwl     "	,"lw      ",//44
		"lbu     "	,"lhu     "	,"lwr     "	,NULL,		//48
		"sb      "	,"sh      "	,"swl     "	,"sw      ",
		NULL		,NULL		,"swr     "	,NULL,
	};
	
	const char* sc_op[] = {
		"sll     "	,NULL		,"srl     "	,"sra     "	,	//0
		"sllv    "	,NULL		,"srlv    "	,"srav    "	,	//4
		"jr      "	,"jalr    "	,NULL		,NULL		,	//8
		"syscall "	,"break   "	,NULL		,NULL		,	//12
		"mfhi    "	,"mthi    "	,"mflo    "	,"mtlo    "	,	//16
		NULL		,NULL		,NULL		,NULL		,	//20
		"mult    "	,"multu   "	,"div     "	,"divu    "	,	//24
		NULL		,NULL		,NULL		,NULL		,	//28
		"add     "	,"addu    "	,"sub     "	,"subu    "	,	//32
		"and     "	,"or      "	,"xor     "	,"nor     "	,	//36
		NULL		,NULL		,"slt     "	,"sltu    "	,	//38
	};
	
	strcpy( output, "???" );
	
	if( opcode ) {
		
		// Secondary codes
		if( OPCODE1(opcode) == 0 ) {

			switch( OPCODE2(opcode) ) {
				case 0x0:	// sll, srl, sra
				case 0x2:
				case 0x3:
					strcpy( output, sc_op[OPCODE2(opcode)] );
					sprintf( temp, "%s, %s, %d",
						regs[OPREGD(opcode)], regs[OPREGT(opcode)], OPIMM5(opcode) );
					strcat( output, temp );
					break;
				case 0x4:	// sllv, srlv, srav
				case 0x6:
				case 0x7:
					strcpy( output, sc_op[OPCODE2(opcode)] );
					sprintf( temp, "%s, %s, %s", regs[OPREGT(opcode)],
						regs[OPREGD(opcode)], regs[OPREGS(opcode)] );
					strcat( output, temp );
					break;
				case 0x8:	// jr
					strcpy( output, sc_op[OPCODE2(opcode)] );
					strcat( output, regs[OPREGS(opcode)] );
					break;
				case 0x9:	// jalr
					strcpy( output, sc_op[OPCODE2(opcode)] );
					sprintf( temp, "%s, %s", regs[OPREGD(opcode)],
						regs[OPREGS(opcode)] );
					strcat( output, temp );
					break;
				case 0xc:	// syscall, break
				case 0xd:
					strcpy( output, sc_op[OPCODE2(opcode)] );
					sprintf( temp, "$%x", OPIMM20(opcode) );
					strcat( output, temp );
					break;
				case 0x10:	// mfhi, mflo
				case 0x12:
					strcpy( output, sc_op[OPCODE2(opcode)] );
					strcat( output, regs[OPREGD(opcode)] );
					break;
				case 0x11:	// mthi, mtlo
				case 0x13:
					strcpy( output, sc_op[OPCODE2(opcode)] );
					strcat( output, regs[OPREGS(opcode)] );
					break;
				case 0x18:	// mult, multu, div, divu
				case 0x19:
				case 0x1a:
				case 0x1b:
					strcpy( output, sc_op[OPCODE2(opcode)] );
					sprintf( temp, "%s, %s", regs[OPREGS(opcode)],
						regs[OPREGT(opcode)] );
					strcat( output, temp );
					break;
				case 0x2a:	// slt, sltu
				case 0x2b:
					strcpy( output, sc_op[OPCODE2(opcode)] );
					sprintf( temp, "%s, %s, %s", regs[OPREGD(opcode)],
						regs[OPREGS(opcode)], regs[OPREGT(opcode)] );
					strcat( output, temp );
					break;
				default:
					
					if( ( OPCODE2(opcode) >= 0x20 ) && ( OPCODE2(opcode) <= 0x27 ) ) {
						strcpy( output, sc_op[OPCODE2(opcode)] );
						sprintf( temp, "%s, %s, %s", regs[OPREGD(opcode)],
							regs[OPREGT(opcode)], regs[OPREGS(opcode)] );
						strcat( output, temp );
					}
					
					break;
			}
			
		// Primary
		} else {
			
			if( OPCODE1(opcode) == 0x1 ) {	// Branches
				
				switch((opcode>>16)&0x1f) {
					case 0x0:
						strcpy(output, "bltz    ");
						break;
					case 0x1:
						strcpy(output, "bgez    ");
						break;
					case 0x10:
						strcpy(output, "bltzal  ");
						break;
					case 0x11:
						strcpy(output, "bgtzal  ");
						break;
				}
				
				sprintf( temp, "%s,$%08X ", regs[OPREGS(opcode)], 
					addr+4+(EXTEND16(OPIMM16(opcode))*4) );
				strcat( output, temp );
				
				if( arrows ) {
					if ( addr+4+(EXTEND16(OPIMM16(opcode))*4) > addr ) {
						strcat( output, "▼" );
					} else {
						strcat( output, "▲" );
					}
				}
				
			} else if( ( OPCODE1(opcode) == 0x2 ) || ( OPCODE1(opcode) == 0x3 ) ) {	// j, jal
				
				unsigned int dest = (OPIMM26(opcode)*4)|(addr&0xf0000000);
				
				strcpy( output, pri_op[OPCODE1(opcode)] );
				sprintf( temp, "$%08X ", dest );
				strcat( output, temp );
				
				if( arrows ) {
					if ( dest > addr ) {
						strcat( output, "▼" );
					} else {
						strcat( output, "▲" );
					}
				}
			} else if( ( OPCODE1(opcode) >= 0x8 ) && 
				( OPCODE1(opcode) <= 0x9 ) ) {
				
				strcpy( output, pri_op[OPCODE1(opcode)] );
				
				sprintf( temp, "%s, %s, %d", regs[OPREGT(opcode)], 
					regs[OPREGS(opcode)], EXTEND16(OPIMM16(opcode)) );
				strcat( output, temp );
				
			} else if( ( OPCODE1(opcode) >= 0xa ) && 
				( OPCODE1(opcode) <= 0xe ) ) {
				
				strcpy( output, pri_op[OPCODE1(opcode)] );
				
				sprintf( temp, "%s, %s, $%lX", regs[OPREGT(opcode)], 
					regs[OPREGS(opcode)], labs( OPIMM16(opcode) ) );
				strcat( output, temp );
				
			} else if( OPCODE1(opcode) == 0xf ) {
				
				strcpy( output, pri_op[OPCODE1(opcode)] );
				sprintf( temp, "%s, $%X", 
					regs[OPREGT(opcode)], OPIMM16(opcode) );
				strcat( output, temp );
			
			} else if( ( OPCODE1(opcode) >= 0x20 ) && 
				( OPCODE1(opcode) <= 0x2e ) ) {
				
				switch( OPCODE1(opcode) ) {
					case 0x27:
					case 0x2c:
					case 0x2d:
						return;
				}
				
				strcpy( output, pri_op[OPCODE1(opcode)] );
				sprintf( temp, "%s,", 
					regs[OPREGT(opcode)] );
				strcat( output, temp );
				
				immval = EXTEND16(OPIMM16(opcode));
				sprintf( temp, "$%lX", labs( immval ) );
				if( immval < 0 ) {
					strcat( output, "-" );
				} else {
					strcat( output, " " );
				}
				strcat( output, temp );
				
				sprintf( temp, "(%s)", regs[OPREGS(opcode)] );
				strcat( output, temp );
						
			} else if ( OPCODE1(opcode)&0x10 ) {
				
				Decode_cop( opcode, output );
				
			} else {
				
				switch( OPCODE1(opcode) ) {
					case 0x4:
					case 0x5:
						strcpy( output, pri_op[OPCODE1(opcode)] );
						sprintf( temp, "%s, %s, $%08X ",
							regs[OPREGS(opcode)],
							regs[OPREGT(opcode)], 
							addr+4+(EXTEND16(OPIMM16(opcode))*4) );
						strcat( output, temp );
						
						if( arrows ) {
							if ( addr+4+(EXTEND16(OPIMM16(opcode))*4) > addr ) {
								strcat( output, "▼" );
							} else {
								strcat( output, "▲" );
							}
						}
						
						break;
					case 0x6:
					case 0x7:
						strcpy( output, pri_op[OPCODE1(opcode)] );
						sprintf( temp, "%s, $%08X ",
							regs[OPREGS(opcode)],
							addr+4+(EXTEND16(OPIMM16(opcode))*4) );
						strcat( output, temp );
						
						if( arrows ) {
							if ( addr+4+(EXTEND16(OPIMM16(opcode))*4) > addr ) {
								strcat( output, "▼" );
							} else {
								strcat( output, "▲" );
							}
						}
						
						break;
				}
				
			}
			
		}
		
	} else {
		
		strcpy( output, "nop" );
		
	}
	
}

unsigned int mips_GetNextPc(unsigned int *regs, int stepover) {
	
	unsigned int dest = regs[PS_REG_epc]+4;
	unsigned int opcode = regs[PS_REG_opcode];
	unsigned int rv,rt;
	
	if( OPCODE1(opcode) == 0 ) {
		
		if( ( OPCODE2(opcode) == 8 )||( OPCODE2(opcode) == 9 ) ) { // jr, jalr
			
			if( ( OPCODE2(opcode) == 9 ) && ( stepover ) ) {
				
				dest += 4;
				
			} else {
				
				if( OPREGS(opcode) == 0 )
					dest = 0;
				else
					dest = regs[OPREGS(opcode)-1];
				
			}
			
		}
		
	} else if( OPCODE1(opcode) == 1 ) {
		
		if( OPREGS(opcode) == 0 )
			rv = 0;
		else
			rv = regs[OPREGS(opcode)-1];
				
		switch(OPREGT(opcode)) {
			case 0x0:	// bltz
				if ( (((int)0)|rv) < 0 ) {
					dest = regs[PS_REG_epc]+4+(EXTEND16(OPIMM16(opcode))*4);
				} else {
					dest += 4;
				}
				break;
			case 0x1:	// bgez
				if ( (((int)0)|rv) >= 0 ) {
					dest = regs[PS_REG_epc]+4+(EXTEND16(OPIMM16(opcode))*4);
				} else {
					dest += 4;
				}
				break;
			case 0x8:	// bltzal
				if ( (((int)0)|rv) < 0 ) {
					dest = regs[PS_REG_epc]+4+(EXTEND16(OPIMM16(opcode))*4);
				} else {
					dest += 4;
				}
				break;
			case 0x9:	// bgezal
				if ( (((int)0)|rv) >= 0 ) {
					dest = regs[PS_REG_epc]+4+(EXTEND16(OPIMM16(opcode))*4);
				} else {
					dest += 4;
				}
				break;
		}
		
	} else if( ( OPCODE1(opcode) == 2 ) || ( OPCODE1(opcode) == 3 ) ) {	// j,jal
		
		if( ( OPCODE1(opcode) == 3 ) && ( stepover ) ) {
			
			dest += 4;
			
		} else {
			
			dest = (regs[PS_REG_epc]&0xf0000000)|(OPIMM26(opcode)*4);
			
		}
		
	} else if( OPCODE1(opcode) == 4 ) {	// beq
		
		if( OPREGS(opcode) == 0 ) {
			rv = 0;
		} else {
			rv = regs[OPREGS(opcode)-1];
		}
		
		if( OPREGT(opcode) == 0 ) {
			rt = 0;
		} else {
			rt = regs[OPREGT(opcode)-1];
		}
		
		if( rv == rt ) {

			dest = regs[PS_REG_epc]+4+(EXTEND16(OPIMM16(opcode))*4);
		
		} else {
			
			dest += 4;
			
		}
		
	} else if( OPCODE1(opcode) == 5 ) {	// bne
		
		if( OPREGS(opcode) == 0 ) {
			rv = 0;
		} else {
			rv = regs[OPREGS(opcode)-1];
		}
		
		if( OPREGT(opcode) == 0 ) {
			rt = 0;
		} else {
			rt = regs[OPREGT(opcode)-1];
		}
		
		if( rv != rt ) {

			dest = regs[PS_REG_epc]+4+(EXTEND16(OPIMM16(opcode))*4);

		} else {

			dest += 4;

		}
		
	} else if( OPCODE1(opcode) == 6 ) {	// blez
		
		if( OPREGS(opcode) == 0 ) {
			rv = 0;
		} else {
			rv = regs[OPREGS(opcode)-1];
		}
		
		if( (((int)0)|rv) <= 0 ) {

			dest = regs[PS_REG_epc]+4+(EXTEND16(OPIMM16(opcode))*4);

		} else {

			dest += 4;

		}
		
	} else if( OPCODE1(opcode) == 7 ) {	// bgtz
		
		if( OPREGS(opcode) == 0 ) {
			rv = 0;
		} else {
			rv = regs[OPREGS(opcode)-1];
		}
		
		if( (((int)0)|rv) > 0 ) {

			dest = regs[PS_REG_epc]+4+(EXTEND16(OPIMM16(opcode))*4);

		} else {

			dest += 4;

		}
		
	}
	
	return dest;
}

unsigned int mips_GetJumpAddr(unsigned int addr, unsigned int opcode) {
	
	if( OPCODE1(opcode) == 1 ) {
		
		switch(OPREGT(opcode)) {
			case 0x0:	// bltz
			case 0x1:	// bgez
			case 0x8:	// bltzal
			case 0x9:	// bgezal
				return addr+4+(EXTEND16(OPIMM16(opcode))*4);			
		}
		
	} else if( ( OPCODE1(opcode) == 2 ) || ( OPCODE1(opcode) == 3 ) ) {	// j,jal
		
		return (addr&0xf0000000)|(OPIMM26(opcode)*4);
		
	} else if( ( OPCODE1(opcode) >= 4 ) && ( OPCODE1(opcode) <= 7 ) ) {	// beq
		
		return addr+4+(EXTEND16(OPIMM16(opcode))*4);
		
	}
	
	return 0;
	
}

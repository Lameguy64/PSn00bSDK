#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

void mips_Decode(unsigned int opcode, unsigned int addr, char* output, int arrows);

unsigned int mips_GetNextPc(unsigned int *regs, int stepover);
unsigned int mips_GetJumpAddr(unsigned int addr, unsigned int opcode);

#endif /* DISASSEMBLER_H */


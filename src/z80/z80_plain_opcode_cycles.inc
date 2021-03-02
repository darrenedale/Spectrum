/* NOTE this is not a header file; it is a source
	file fragment. it contains code that belongs in z80.cpp
	to define the opcode cycle costs and has been extracted
	to this file for easier maintenance. */

/* cycle costs of plain opcodes. 32-bit unsigned values.
	conditional jumps contain the cost if jump is taken in
	0xNNNN0000 and cost if jump is not taken in 0x0000NNNN.
	there are macros defined in z80.cpp to extract the
	values */
m_plain_opcode_cycles[Z80__PLAIN__NOP] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__BC__NN] = 10;
m_plain_opcode_cycles[Z80__PLAIN__LD__INDIRECT_BC__A] = 7;
m_plain_opcode_cycles[Z80__PLAIN__INC__BC] = 6;
m_plain_opcode_cycles[Z80__PLAIN__INC__B] = 4;
m_plain_opcode_cycles[Z80__PLAIN__DEC__B] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__B__N] = 7;
m_plain_opcode_cycles[Z80__PLAIN__RLCA] = 4;

m_plain_opcode_cycles[Z80__PLAIN__EX__AF__AF_SHADOW] = 4;
m_plain_opcode_cycles[Z80__PLAIN__ADD__HL__BC] = 11;
m_plain_opcode_cycles[Z80__PLAIN__LD__A__INDIRECT_BC] = 7;
m_plain_opcode_cycles[Z80__PLAIN__DEC__BC] = 6;
m_plain_opcode_cycles[Z80__PLAIN__INC__C] = 4;
m_plain_opcode_cycles[Z80__PLAIN__DEC__C] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__C__N] = 7;
m_plain_opcode_cycles[Z80__PLAIN__RRCA] = 4;

m_plain_opcode_cycles[Z80__PLAIN__DJNZ__d] = 8 | (13 << 16);
m_plain_opcode_cycles[Z80__PLAIN__LD__DE__NN] = 10;
m_plain_opcode_cycles[Z80__PLAIN__LD__INDIRECT_DE__A] = 7;
m_plain_opcode_cycles[Z80__PLAIN__INC__DE] = 6;
m_plain_opcode_cycles[Z80__PLAIN__INC__D] = 4;
m_plain_opcode_cycles[Z80__PLAIN__DEC__D] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__D__N] = 7;
m_plain_opcode_cycles[Z80__PLAIN__RLA] = 4;

m_plain_opcode_cycles[Z80__PLAIN__JR__d] = 7 | (12 << 16);
m_plain_opcode_cycles[Z80__PLAIN__ADD__HL__DE] = 11;
m_plain_opcode_cycles[Z80__PLAIN__LD__A__INDIRECT_DE] = 7;
m_plain_opcode_cycles[Z80__PLAIN__DEC__DE] = 6;
m_plain_opcode_cycles[Z80__PLAIN__INC__E] = 4;
m_plain_opcode_cycles[Z80__PLAIN__DEC__E] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__E__N] = 7;
m_plain_opcode_cycles[Z80__PLAIN__RRA] = 4;

m_plain_opcode_cycles[Z80__PLAIN__JR__NZ__d] = 7 | (12 << 16);
m_plain_opcode_cycles[Z80__PLAIN__LD__HL__NN] = 10;
m_plain_opcode_cycles[Z80__PLAIN__LD__INDIRECT_NN__HL] = 16;
m_plain_opcode_cycles[Z80__PLAIN__INC__HL] = 6;
m_plain_opcode_cycles[Z80__PLAIN__INC__H] = 4;
m_plain_opcode_cycles[Z80__PLAIN__DEC__H] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__H__N] = 7;
m_plain_opcode_cycles[Z80__PLAIN__DAA] = 4;

m_plain_opcode_cycles[Z80__PLAIN__JR__Z__d] = 7 | (12 << 16);
m_plain_opcode_cycles[Z80__PLAIN__ADD__HL__HL] = 11;
m_plain_opcode_cycles[Z80__PLAIN__LD__HL__INDIRECT_NN] = 16;
m_plain_opcode_cycles[Z80__PLAIN__DEC__HL] = 6;
m_plain_opcode_cycles[Z80__PLAIN__INC__L] = 4;
m_plain_opcode_cycles[Z80__PLAIN__DEC__L] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__L__N] = 7;
m_plain_opcode_cycles[Z80__PLAIN__CPL] = 4;

m_plain_opcode_cycles[Z80__PLAIN__JR__NC__d] = 7 | (12 << 16);
m_plain_opcode_cycles[Z80__PLAIN__LD__SP__NN] = 10;
m_plain_opcode_cycles[Z80__PLAIN__LD__INDIRECT_NN__A] = 13;
m_plain_opcode_cycles[Z80__PLAIN__INC__SP] = 6;
m_plain_opcode_cycles[Z80__PLAIN__INC__INDIRECT_HL] = 11;
m_plain_opcode_cycles[Z80__PLAIN__DEC__INDIRECT_HL] = 11;
m_plain_opcode_cycles[Z80__PLAIN__LD__INDIRECT_HL__N] = 10;
m_plain_opcode_cycles[Z80__PLAIN__SCF] = 4;

m_plain_opcode_cycles[Z80__PLAIN__JR__C__d] = 7 | (12 << 16);
m_plain_opcode_cycles[Z80__PLAIN__ADD__HL__SP] = 11;
m_plain_opcode_cycles[Z80__PLAIN__LD__A__INDIRECT_NN] = 13;
m_plain_opcode_cycles[Z80__PLAIN__DEC__SP] = 6;
m_plain_opcode_cycles[Z80__PLAIN__INC__A] = 4;
m_plain_opcode_cycles[Z80__PLAIN__DEC__A] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__A__N] = 7;
m_plain_opcode_cycles[Z80__PLAIN__CCF] = 4;

m_plain_opcode_cycles[Z80__PLAIN__LD__B__B] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__B__C] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__B__D] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__B__E] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__B__H] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__B__L] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__B__INDIRECT_HL] = 7;
m_plain_opcode_cycles[Z80__PLAIN__LD__B__A] = 4;

m_plain_opcode_cycles[Z80__PLAIN__LD__C__B] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__C__C] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__C__D] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__C__E] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__C__H] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__C__L] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__C__INDIRECT_HL] = 7;
m_plain_opcode_cycles[Z80__PLAIN__LD__C__A] = 4;

m_plain_opcode_cycles[Z80__PLAIN__LD__D__B] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__D__C] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__D__D] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__D__E] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__D__H] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__D__L] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__D__INDIRECT_HL] = 7;
m_plain_opcode_cycles[Z80__PLAIN__LD__D__A] = 4;

m_plain_opcode_cycles[Z80__PLAIN__LD__E__B] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__E__C] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__E__D] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__E__E] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__E__H] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__E__L] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__E__INDIRECT_HL] = 7;
m_plain_opcode_cycles[Z80__PLAIN__LD__E__A] = 4;

m_plain_opcode_cycles[Z80__PLAIN__LD__H__B] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__H__C] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__H__D] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__H__E] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__H__H] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__H__L] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__H__INDIRECT_HL] = 7;
m_plain_opcode_cycles[Z80__PLAIN__LD__H__A] = 4;

m_plain_opcode_cycles[Z80__PLAIN__LD__L__B] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__L__C] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__L__D] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__L__E] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__L__H] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__L__L] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__L__INDIRECT_HL] = 7;
m_plain_opcode_cycles[Z80__PLAIN__LD__L__A] = 4;

m_plain_opcode_cycles[Z80__PLAIN__LD__INDIRECT_HL__B] = 7;
m_plain_opcode_cycles[Z80__PLAIN__LD__INDIRECT_HL__C] = 7;
m_plain_opcode_cycles[Z80__PLAIN__LD__INDIRECT_HL__D] = 7;
m_plain_opcode_cycles[Z80__PLAIN__LD__INDIRECT_HL__E] = 7;
m_plain_opcode_cycles[Z80__PLAIN__LD__INDIRECT_HL__H] = 7;
m_plain_opcode_cycles[Z80__PLAIN__LD__INDIRECT_HL__L] = 7;
m_plain_opcode_cycles[Z80__PLAIN__HALT] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__INDIRECT_HL__A] = 7;

m_plain_opcode_cycles[Z80__PLAIN__LD__A__B] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__A__C] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__A__D] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__A__E] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__A__H] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__A__L] = 4;
m_plain_opcode_cycles[Z80__PLAIN__LD__A__INDIRECT_HL] = 7;
m_plain_opcode_cycles[Z80__PLAIN__LD__A__A] = 4;

m_plain_opcode_cycles[Z80__PLAIN__ADD__A__B] = 4;
m_plain_opcode_cycles[Z80__PLAIN__ADD__A__C] = 4;
m_plain_opcode_cycles[Z80__PLAIN__ADD__A__D] = 4;
m_plain_opcode_cycles[Z80__PLAIN__ADD__A__E] = 4;
m_plain_opcode_cycles[Z80__PLAIN__ADD__A__H] = 4;
m_plain_opcode_cycles[Z80__PLAIN__ADD__A__L] = 4;
m_plain_opcode_cycles[Z80__PLAIN__ADD__A__INDIRECT_HL] = 7;
m_plain_opcode_cycles[Z80__PLAIN__ADD__A__A] = 4;

m_plain_opcode_cycles[Z80__PLAIN__ADC__A__B] = 4;
m_plain_opcode_cycles[Z80__PLAIN__ADC__A__C] = 4;
m_plain_opcode_cycles[Z80__PLAIN__ADC__A__D] = 4;
m_plain_opcode_cycles[Z80__PLAIN__ADC__A__E] = 4;
m_plain_opcode_cycles[Z80__PLAIN__ADC__A__H] = 4;
m_plain_opcode_cycles[Z80__PLAIN__ADC__A__L] = 4;
m_plain_opcode_cycles[Z80__PLAIN__ADC__A__INDIRECT_HL] = 7;
m_plain_opcode_cycles[Z80__PLAIN__ADC__A__A] = 4;

m_plain_opcode_cycles[Z80__PLAIN__SUB__B] = 4;
m_plain_opcode_cycles[Z80__PLAIN__SUB__C] = 4;
m_plain_opcode_cycles[Z80__PLAIN__SUB__D] = 4;
m_plain_opcode_cycles[Z80__PLAIN__SUB__E] = 4;
m_plain_opcode_cycles[Z80__PLAIN__SUB__H] = 4;
m_plain_opcode_cycles[Z80__PLAIN__SUB__L] = 4;
m_plain_opcode_cycles[Z80__PLAIN__SUB__INDIRECT_HL] = 7;
m_plain_opcode_cycles[Z80__PLAIN__SUB__A] = 4;

m_plain_opcode_cycles[Z80__PLAIN__SBC__A__B] = 4;
m_plain_opcode_cycles[Z80__PLAIN__SBC__A__C] = 4;
m_plain_opcode_cycles[Z80__PLAIN__SBC__A__D] = 4;
m_plain_opcode_cycles[Z80__PLAIN__SBC__A__E] = 4;
m_plain_opcode_cycles[Z80__PLAIN__SBC__A__H] = 4;
m_plain_opcode_cycles[Z80__PLAIN__SBC__A__L] = 4;
m_plain_opcode_cycles[Z80__PLAIN__SBC__A__INDIRECT_HL] = 7;
m_plain_opcode_cycles[Z80__PLAIN__SBC__A__A] = 4;

m_plain_opcode_cycles[Z80__PLAIN__AND__B] = 4;
m_plain_opcode_cycles[Z80__PLAIN__AND__C] = 4;
m_plain_opcode_cycles[Z80__PLAIN__AND__D] = 4;
m_plain_opcode_cycles[Z80__PLAIN__AND__E] = 4;
m_plain_opcode_cycles[Z80__PLAIN__AND__H] = 4;
m_plain_opcode_cycles[Z80__PLAIN__AND__L] = 4;
m_plain_opcode_cycles[Z80__PLAIN__AND__INDIRECT_HL] = 7;
m_plain_opcode_cycles[Z80__PLAIN__AND__A] = 4;

m_plain_opcode_cycles[Z80__PLAIN__XOR__B] = 4;
m_plain_opcode_cycles[Z80__PLAIN__XOR__C] = 4;
m_plain_opcode_cycles[Z80__PLAIN__XOR__D] = 4;
m_plain_opcode_cycles[Z80__PLAIN__XOR__E] = 4;
m_plain_opcode_cycles[Z80__PLAIN__XOR__H] = 4;
m_plain_opcode_cycles[Z80__PLAIN__XOR__L] = 4;
m_plain_opcode_cycles[Z80__PLAIN__XOR__INDIRECT_HL] = 7;
m_plain_opcode_cycles[Z80__PLAIN__XOR__A] = 4;

m_plain_opcode_cycles[Z80__PLAIN__OR__B] = 4;
m_plain_opcode_cycles[Z80__PLAIN__OR__C] = 4;
m_plain_opcode_cycles[Z80__PLAIN__OR__D] = 4;
m_plain_opcode_cycles[Z80__PLAIN__OR__E] = 4;
m_plain_opcode_cycles[Z80__PLAIN__OR__H] = 4;
m_plain_opcode_cycles[Z80__PLAIN__OR__L] = 4;
m_plain_opcode_cycles[Z80__PLAIN__OR__INDIRECT_HL] = 7;
m_plain_opcode_cycles[Z80__PLAIN__OR__A] = 4;

m_plain_opcode_cycles[Z80__PLAIN__CP__B] = 4;
m_plain_opcode_cycles[Z80__PLAIN__CP__C] = 4;
m_plain_opcode_cycles[Z80__PLAIN__CP__D] = 4;
m_plain_opcode_cycles[Z80__PLAIN__CP__E] = 4;
m_plain_opcode_cycles[Z80__PLAIN__CP__H] = 4;
m_plain_opcode_cycles[Z80__PLAIN__CP__L] = 4;
m_plain_opcode_cycles[Z80__PLAIN__CP__INDIRECT_HL] = 7;
m_plain_opcode_cycles[Z80__PLAIN__CP__A] = 4;

m_plain_opcode_cycles[Z80__PLAIN__RET__NZ] = 5 | (11 << 16);
m_plain_opcode_cycles[Z80__PLAIN__POP__BC] = 10;
m_plain_opcode_cycles[Z80__PLAIN__JP__NZ__NN] = 1 | (10 << 16);
m_plain_opcode_cycles[Z80__PLAIN__JP__NN] = 10;
m_plain_opcode_cycles[Z80__PLAIN__CALL__NZ__NN] = 10 | (17 << 16);
m_plain_opcode_cycles[Z80__PLAIN__PUSH__BC] = 11;
m_plain_opcode_cycles[Z80__PLAIN__ADD__A__N] = 7;
m_plain_opcode_cycles[Z80__PLAIN__RST__00] = 11;	/* this cost needs to be verified */

m_plain_opcode_cycles[Z80__PLAIN__RET__Z] = 5 | (11 << 16);
m_plain_opcode_cycles[Z80__PLAIN__RET] = 10;
m_plain_opcode_cycles[Z80__PLAIN__JP__Z__NN] = 1 | (10 << 16);
m_plain_opcode_cycles[Z80__PLAIN__PREFIX__CB] = 0;	/* 0xcb prefix */
m_plain_opcode_cycles[Z80__PLAIN__CALL__Z__NN] = 10 | (17 << 16);
m_plain_opcode_cycles[Z80__PLAIN__CALL__NN] = 17;
m_plain_opcode_cycles[Z80__PLAIN__ADC__A__N] = 7;
m_plain_opcode_cycles[Z80__PLAIN__RST__08] = 11;

m_plain_opcode_cycles[Z80__PLAIN__RET__NC] = 5 | (11 << 16);
m_plain_opcode_cycles[Z80__PLAIN__POP__DE] = 10;
m_plain_opcode_cycles[Z80__PLAIN__JP__NC__NN] = 1 | (10 << 16);
m_plain_opcode_cycles[Z80__PLAIN__OUT__INDIRECT_N__A] = 11;
m_plain_opcode_cycles[Z80__PLAIN__CALL__NC__NN] = 10 | (17 << 16);
m_plain_opcode_cycles[Z80__PLAIN__PUSH__DE] = 11;
m_plain_opcode_cycles[Z80__PLAIN__SUB__N] = 7;
m_plain_opcode_cycles[Z80__PLAIN__RST__10] = 11;

m_plain_opcode_cycles[Z80__PLAIN__RET__C] = 5 | (11 << 16);
m_plain_opcode_cycles[Z80__PLAIN__EXX] = 4;
m_plain_opcode_cycles[Z80__PLAIN__JP__C__NN] = 1 | (10 << 16);
m_plain_opcode_cycles[Z80__PLAIN__IN__A__INDIRECT_N] = 11;
m_plain_opcode_cycles[Z80__PLAIN__CALL__C__NN] = 10 | (17 << 16);
m_plain_opcode_cycles[Z80__PLAIN__PREFIX__DD] = 0;	/* 0xdd prefix */
m_plain_opcode_cycles[Z80__PLAIN__SBC__A__N] = 7;
m_plain_opcode_cycles[Z80__PLAIN__RST__18] = 11;

m_plain_opcode_cycles[Z80__PLAIN__RET__PO] = 5 | (11 << 16);
m_plain_opcode_cycles[Z80__PLAIN__POP__HL] = 10;
m_plain_opcode_cycles[Z80__PLAIN__JP__PO__NN] = 1 | (10 << 16);
m_plain_opcode_cycles[Z80__PLAIN__EX__INDIRECT_SP__HL] = 19;
m_plain_opcode_cycles[Z80__PLAIN__CALL__PO__NN] = 10 | (17 << 16);
m_plain_opcode_cycles[Z80__PLAIN__PUSH__HL] = 11;
m_plain_opcode_cycles[Z80__PLAIN__AND__N] = 7;
m_plain_opcode_cycles[Z80__PLAIN__RST__20] = 11;

m_plain_opcode_cycles[Z80__PLAIN__RET__PE] = 5 | (11 << 16);
m_plain_opcode_cycles[Z80__PLAIN__JP__INDIRECT_HL] = 4;
m_plain_opcode_cycles[Z80__PLAIN__JP__PE__NN] = 1 | (10 << 16);
m_plain_opcode_cycles[Z80__PLAIN__EX__DE__HL] = 4;
m_plain_opcode_cycles[Z80__PLAIN__CALL__PE__NN] = 10 | (17 << 16);
m_plain_opcode_cycles[Z80__PLAIN__PREFIX__ED] = 0;	/* 0xed prefix */
m_plain_opcode_cycles[Z80__PLAIN__XOR__N] = 7;
m_plain_opcode_cycles[Z80__PLAIN__RST__28] = 11;

m_plain_opcode_cycles[Z80__PLAIN__RET__P] = 5 | (11 << 16);
m_plain_opcode_cycles[Z80__PLAIN__POP__AF] = 10;
m_plain_opcode_cycles[Z80__PLAIN__JP__P__NN] = 1 | (10 << 16);
m_plain_opcode_cycles[Z80__PLAIN__DI] = 4;
m_plain_opcode_cycles[Z80__PLAIN__CALL__P__NN] = 10 | (17 << 16);
m_plain_opcode_cycles[Z80__PLAIN__PUSH__AF] = 11;
m_plain_opcode_cycles[Z80__PLAIN__OR__N] = 7;
m_plain_opcode_cycles[Z80__PLAIN__RST__30] = 11;

m_plain_opcode_cycles[Z80__PLAIN__RET__M] = 5 | (11 << 16);
m_plain_opcode_cycles[Z80__PLAIN__LD__SP__HL] = 6;
m_plain_opcode_cycles[Z80__PLAIN__JP__M__NN] = 1 | (10 << 16);
m_plain_opcode_cycles[Z80__PLAIN__EI] = 4;
m_plain_opcode_cycles[Z80__PLAIN__CALL__M__NN] = 10 | (17 << 16);
m_plain_opcode_cycles[Z80__PLAIN__PREFIX__FD] = 0;	/* 0xfd prefix */
m_plain_opcode_cycles[Z80__PLAIN__CP__N] = 7;
m_plain_opcode_cycles[Z80__PLAIN__RST__38] = 11;

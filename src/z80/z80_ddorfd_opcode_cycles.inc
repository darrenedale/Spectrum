// NOTE this is not a header file; it is a source file fragment. it contains code that belongs in z80.cpp to define the
//  opcode cycle costs and has been extracted to this file for easier maintenance.

// cycle costs of 0xdd and 0xfd (IX and IY register) opcodes. 32-bit unsigned values.
// conditional jumps contain the cost if jump is taken in 0xNNNN0000 and cost if jump is not taken in 0x0000NNNN.
// there are macros defined in z80.cpp to extract the values

// TODO not all of the costs in this file have been verified
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__NOP] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__BC__NN] = 10;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__INDIRECT_BC__A] = 7;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__INC__BC] = 6;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__INC__B] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__DEC__B] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__B__N] = 7;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RLCA] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__EX__AF__AF_SHADOW] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADD__IX_OR_IY__BC] = 15;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__A__INDIRECT_BC] = 7;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__DEC__BC] = 6;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__INC__C] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__DEC__C] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__C__N] = 7;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RRCA] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__DJNZ__d] = 8 | (13 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__DE__NN] = 10;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__INDIRECT_DE__A] = 7;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__INC__DE] = 6;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__INC__D] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__DEC__D] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__D__N] = 7;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RLA] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__JR__d] = 7 | (12 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADD__IX_OR_IY__DE] = 15;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__A__INDIRECT_DE] = 7;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__DEC__DE] = 6;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__INC__E] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__DEC__E] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__E__N] = 7;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RRA] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__JR__NZ__d] = 7 | (12 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__IX_OR_IY__NN] = 14;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__INDIRECT_NN__IX_OR_IY] = 20;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__INC__IX_OR_IY] = 10;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__INC__IXH_OR_IYH] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__DEC__IXH_OR_IYH] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__IXH_OR_IYH__N] = 11;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__DAA] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__JR__Z__d] = 7 | (12 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADD__IX_OR_IY__IX_OR_IY] = 15;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__IX_OR_IY__INDIRECT_NN] = 20;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__DEC__IX_OR_IY] = 10;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__INC__IXL_OR_IYL] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__DEC__IXL_OR_IYL] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__IXL_OR_IYL__N] = 11;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__CPL] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__JR__NC__d] = 7 | (12 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__SP__NN] = 10;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__INDIRECT_NN__A] = 13;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__INC__SP] = 6;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__INC__INDIRECT_IX_d_OR_IY_d] = 23;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__DEC__INDIRECT_IX_d_OR_IY_d] = 23;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__N] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__SCF] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__JR__C__d] = 7 | (12 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADD__IX_OR_IY__SP] = 15;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__A__INDIRECT_NN] = 13;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__DEC__SP] = 6;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__INC__A] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__DEC__A] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__A__N] = 7;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__CCF] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__B__B] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__B__C] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__B__D] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__B__E] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__B__IXH_OR_IYH] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__B__IXL_OR_IYL] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__B__INDIRECT_IX_d_OR_IY_d] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__B__A] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__C__B] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__C__C] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__C__D] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__C__E] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__C__IXH_OR_IYH] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__C__IXL_OR_IYL] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__C__INDIRECT_IX_d_OR_IY_d] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__C__A] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__D__B] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__D__C] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__D__D] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__D__E] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__D__IXH_OR_IYH] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__D__IXL_OR_IYL] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__D__INDIRECT_IX_d_OR_IY_d] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__D__A] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__E__B] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__E__C] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__E__D] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__E__E] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__E__IXH_OR_IYH] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__E__IXL_OR_IYL] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__E__INDIRECT_IX_d_OR_IY_d] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__E__A] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__IXH_OR_IYH__B] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__IXH_OR_IYH__C] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__IXH_OR_IYH__D] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__IXH_OR_IYH__E] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__IXH_OR_IYH__IXH_OR_IYH] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__IXH_OR_IYH__IXL_OR_IYL] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__H__INDIRECT_IX_d_OR_IY_d] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__IXH_OR_IYH__A] = 8;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__IXL_OR_IYL__B] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__IXL_OR_IYL__C] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__IXL_OR_IYL__D] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__IXL_OR_IYL__E] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__IXL_OR_IYL__IXH_OR_IYH] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__IXL_OR_IYL__IXL_OR_IYL] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__L__INDIRECT_IX_d_OR_IY_d] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__IXL_OR_IYL__A] = 8;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__B] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__C] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__D] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__E] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__H] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__L] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__HALT] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__A] = 19;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__A__B] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__A__C] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__A__D] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__A__E] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__A__IXH_OR_IYH] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__A__IXL_OR_IYL] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__A__INDIRECT_IX_d_OR_IY_d] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__A__A] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADD__A__B] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADD__A__C] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADD__A__D] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADD__A__E] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADD__A__IXH_OR_IYH] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADD__A__IXL_OR_IYL] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADD__A__INDIRECT_IX_d_OR_IY_d] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADD__A__A] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADC__A__B] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADC__A__C] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADC__A__D] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADC__A__E] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADC__A__IXH_OR_IYH] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADC__A__IXL_OR_IYL] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADC__A__INDIRECT_IX_d_OR_IY_d] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADC__A__A] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__SUB__B] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__SUB__C] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__SUB__D] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__SUB__E] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__SUB__IXH_OR_IYH] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__SUB__IXL_OR_IYL] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__SUB__INDIRECT_IX_d_OR_IY_d] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__SUB__A] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__SBC__A__B] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__SBC__A__C] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__SBC__A__D] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__SBC__A__E] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__SBC__A__IXH_OR_IYH] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__SBC__A__IXL_OR_IYL] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__SBC__A__INDIRECT_IX_d_OR_IY_d] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__SBC__A__A] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__AND__B] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__AND__C] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__AND__D] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__AND__E] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__AND__IXH_OR_IYH] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__AND__IXL_OR_IYL] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__AND__INDIRECT_IX_d_OR_IY_d] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__AND__A] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__XOR__B] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__XOR__C] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__XOR__D] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__XOR__E] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__XOR__IXH_OR_IYH] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__XOR__IXL_OR_IYL] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__XOR__INDIRECT_IX_d_OR_IY_d] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__XOR__A] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__OR__B] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__OR__C] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__OR__D] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__OR__E] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__OR__IXH_OR_IYH] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__OR__IXL_OR_IYL] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__OR__INDIRECT_IX_d_OR_IY_d] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__OR__A] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__CP__B] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__CP__C] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__CP__D] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__CP__E] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__CP__IXH_OR_IYH] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__CP__IXL_OR_IYL] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__CP__INDIRECT_IX_d_OR_IY_d] = 19;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__CP__A] = 4;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RET__NZ] = 5 | (11 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__POP__BC] = 10;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__JP__NZ__NN] = 1 | (10 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__JP__NN] = 10;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__CALL__NZ__NN] = 10 | (17 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__PUSH__BC] = 11;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADD__A__N] = 7;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RST__00] = 11;	/* this cost needs to be verified */

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RET__Z] = 5 | (11 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RET] = 10;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__JP__Z__NN] = 1 | (10 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__PREFIX__CB] = 0;	/* 0xcb prefix */
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__CALL__Z__NN] = 10 | (17 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__CALL__NN] = 17;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__ADC__A__N] = 7;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RST__08] = 11;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RET__NC] = 5 | (11 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__POP__DE] = 10;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__JP__NC__NN] = 1 | (10 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__OUT__INDIRECT_N__A] = 11;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__CALL__NC__NN] = 10 | (17 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__PUSH__DE] = 11;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__SUB__N] = 7;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RST__10] = 11;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RET__C] = 5 | (11 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__EXX] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__JP__C__NN] = 1 | (10 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__IN__A__INDIRECT_N] = 11;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__CALL__C__NN] = 10 | (17 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__PREFIX__DD] = 0;	/* 0xdd prefix */
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__SBC__A__N] = 7;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RST__18] = 11;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RET__PO] = 5 | (11 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__POP__IX_OR_IY] = 14;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__JP__PO__NN] = 1 | (10 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__EX__INDIRECT_SP__IX_OR_IY] = 23;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__CALL__PO__NN] = 10 | (17 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__PUSH__IX_OR_IY] = 15;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__AND__N] = 7;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RST__20] = 11;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RET__PE] = 5 | (11 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__JP__IX_OR_IY] = 8;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__JP__PE__NN] = 1 | (10 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__EX__DE__HL] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__CALL__PE__NN] = 10 | (17 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__PREFIX__ED] = 0;	/* 0xed prefix */
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__XOR__N] = 7;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RST__28] = 11;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RET__P] = 5 | (11 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__POP__AF] = 10;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__JP__P__NN] = 1 | (10 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__DI] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__CALL__P__NN] = 10 | (17 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__PUSH__AF] = 11;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__OR__N] = 7;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RST__30] = 11;

m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RET__M] = 5 | (11 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__LD__SP__IX_OR_IY] = 10;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__JP__M__NN] = 1 | (10 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__EI] = 4;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__CALL__M__NN] = 10 | (17 << 16);
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__PREFIX__FD] = 0;	/* 0xfd prefix */
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__CP__N] = 7;
m_ddorfd_opcode_cycles[Z80__DD_OR_FD__RST__38] = 11;

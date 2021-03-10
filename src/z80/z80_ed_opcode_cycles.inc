/* NOTE this is not a header file; it is a source
	file fragment. it contains code that belongs in z80.cpp
	to define the opcode cycle costs and has been extracted
	to this file for easier maintenance. */

/* cycle costs of extended opcodes. 32-bit unsigned values.
	conditional jumps contain the cost if jump is taken in
	0xNNNN0000 and cost if jump is not taken in 0x0000NNNN.
	there are macros defined in z80.cpp to extract the
	values */

/* it is not clear from the documents I have whether the
	alternate costs specified are for jumps or for each
	iteration of the increment/decrement loop. the latter
	seems more likely for the instructions for which alt
	costs are given (see LDIR, LDDR etc.) */
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X00] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X01] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X02] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X03] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X04] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X05] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X06] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X07] = 4;

m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X08] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X09] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X0A] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X0B] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X0C] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X0D] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X0E] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X0F] = 4;

m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X10] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X11] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X12] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X13] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X14] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X15] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X16] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X17] = 4;

m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X18] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X19] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X1A] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X1B] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X1C] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X1D] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X1E] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X1F] = 4;

m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X20] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X21] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X22] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X23] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X24] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X25] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X26] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X27] = 4;

m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X28] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X29] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X2A] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X2B] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X2C] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X2D] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X2E] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X2F] = 4;

m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X30] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X31] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X32] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X33] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X34] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X35] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X36] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X37] = 4;

m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X38] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X39] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X3A] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X3B] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X3C] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X3D] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X3E] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X3F] = 4;

m_ed_opcode_cycles[Z80__ED__IN__B__INDIRECT_C] = 12;
m_ed_opcode_cycles[Z80__ED__OUT__INDIRECT_C__B] = 12;
m_ed_opcode_cycles[Z80__ED__SBC__HL__BC] = 15;
m_ed_opcode_cycles[Z80__ED__LD__INDIRECT_NN__BC] = 20;
m_ed_opcode_cycles[Z80__ED__NEG] = 8;
m_ed_opcode_cycles[Z80__ED__RETN] = 14;
m_ed_opcode_cycles[Z80__ED__IM__0] = 8;
m_ed_opcode_cycles[Z80__ED__LD__I__A] = 9;

m_ed_opcode_cycles[Z80__ED__IN__C__INDIRECT_C] = 12;
m_ed_opcode_cycles[Z80__ED__OUT__INDIRECT_C__C] = 12;
m_ed_opcode_cycles[Z80__ED__ADC__HL__BC] = 15;
m_ed_opcode_cycles[Z80__ED__LD__BC__INDIRECT_NN] = 20;
m_ed_opcode_cycles[Z80__ED__NEG__0XED__0X4C] = 8;
m_ed_opcode_cycles[Z80__ED__RETI] = 14;
m_ed_opcode_cycles[Z80__ED__IM__0__0XED__0X4E] = 8;
m_ed_opcode_cycles[Z80__ED__LD__R__A] = 9;

m_ed_opcode_cycles[Z80__ED__IN__D__INDIRECT_C] = 12;
m_ed_opcode_cycles[Z80__ED__OUT__INDIRECT_C__D] = 12;
m_ed_opcode_cycles[Z80__ED__SBC__HL__DE] = 15;
m_ed_opcode_cycles[Z80__ED__LD__INDIRECT_NN__DE] = 20;
m_ed_opcode_cycles[Z80__ED__NEG__0XED__0X54] = 8;
m_ed_opcode_cycles[Z80__ED__RETN__0XED__0X55] = 14;
m_ed_opcode_cycles[Z80__ED__IM__1] = 8;
m_ed_opcode_cycles[Z80__ED__LD__A__I] = 9;

m_ed_opcode_cycles[Z80__ED__IN__E__INDIRECT_C] = 12;
m_ed_opcode_cycles[Z80__ED__OUT__INDIRECT_C__E] = 12;
m_ed_opcode_cycles[Z80__ED__ADC__HL__DE] = 15;
m_ed_opcode_cycles[Z80__ED__LD__DE__INDIRECT_NN] = 20;
m_ed_opcode_cycles[Z80__ED__NEG__0XED__0X5C] = 8;
m_ed_opcode_cycles[Z80__ED__RETI__0XED__0X5D] = 14;
m_ed_opcode_cycles[Z80__ED__IM__2] = 8;
m_ed_opcode_cycles[Z80__ED__LD__A__R] = 9;

m_ed_opcode_cycles[Z80__ED__IN__H__INDIRECT_C] = 12;
m_ed_opcode_cycles[Z80__ED__OUT__INDIRECT_C__H] = 12;
m_ed_opcode_cycles[Z80__ED__SBC__HL__HL] = 15;
m_ed_opcode_cycles[Z80__ED__LD__INDIRECT_NN__HL] = 20;
m_ed_opcode_cycles[Z80__ED__NEG__0XED__0X64] = 8;
m_ed_opcode_cycles[Z80__ED__RETN__0XED__0X65] = 14;
m_ed_opcode_cycles[Z80__ED__IM__0__0XED__0X66] = 8;
m_ed_opcode_cycles[Z80__ED__RRD] = 18;

m_ed_opcode_cycles[Z80__ED__IN__L__INDIRECT_C] = 12;
m_ed_opcode_cycles[Z80__ED__OUT__INDIRECT_C__L] = 12;
m_ed_opcode_cycles[Z80__ED__ADC__HL__HL] = 15;
m_ed_opcode_cycles[Z80__ED__LD__HL__INDIRECT_NN] = 20;
m_ed_opcode_cycles[Z80__ED__NEG__0XED__0X6C] = 8;
m_ed_opcode_cycles[Z80__ED__RETI__0XED__0X6D] = 14;
m_ed_opcode_cycles[Z80__ED__IM__0__0XED__0X6E] = 8;
m_ed_opcode_cycles[Z80__ED__RLD] = 18;

m_ed_opcode_cycles[Z80__ED__IN__INDIRECT_C] = 12;
m_ed_opcode_cycles[Z80__ED__OUT__INDIRECT_C__0] = 12;
m_ed_opcode_cycles[Z80__ED__SBC__HL__SP] = 15;
m_ed_opcode_cycles[Z80__ED__LD__INDIRECT_NN__SP] = 20;
m_ed_opcode_cycles[Z80__ED__NEG__0XED__0X74] = 8;
m_ed_opcode_cycles[Z80__ED__RETN__0XED__0X75] = 14;
m_ed_opcode_cycles[Z80__ED__IM__1__0XED__0X76] = 8;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0x77] = 4;

m_ed_opcode_cycles[Z80__ED__IN__A__INDIRECT_C] = 12;
m_ed_opcode_cycles[Z80__ED__OUT__INDIRECT_C__A] = 12;
m_ed_opcode_cycles[Z80__ED__ADC__HL__SP] = 15;
m_ed_opcode_cycles[Z80__ED__LD__SP__INDIRECT_NN] = 20;
m_ed_opcode_cycles[Z80__ED__NEG__0XED__0X7C] = 8;
m_ed_opcode_cycles[Z80__ED__RETI__0XED__0X7D] = 14;
m_ed_opcode_cycles[Z80__ED__IM__2__0XED__0X7E] = 8;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X7F] = 4;

m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X80] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X81] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X82] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X83] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X84] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X85] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X86] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X87] = 4;

m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X88] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X89] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X8A] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X8B] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X8C] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X8D] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X8E] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X8F] = 4;

m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X90] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X91] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X92] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X93] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X94] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X95] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X96] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X97] = 4;

m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X98] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X99] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X9A] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X9B] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X9C] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X9D] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X9E] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0X9F] = 4;

m_ed_opcode_cycles[Z80__ED__LDI] = 16;
m_ed_opcode_cycles[Z80__ED__CPI] = 16;
m_ed_opcode_cycles[Z80__ED__INI] = 16;
m_ed_opcode_cycles[Z80__ED__OUTI] = 16;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XA4] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XA5] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XA6] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XA7] = 4;

m_ed_opcode_cycles[Z80__ED__LDD] = 16;
m_ed_opcode_cycles[Z80__ED__CPD] = 16;
m_ed_opcode_cycles[Z80__ED__IND] = 16;
m_ed_opcode_cycles[Z80__ED__OUTD] = 16;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XAC] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XAD] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XAE] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XAF] = 4;

m_ed_opcode_cycles[Z80__ED__LDIR]= 0 | (21 << 16);
m_ed_opcode_cycles[Z80__ED__CPIR]= 0 | (21 << 16);
m_ed_opcode_cycles[Z80__ED__INIR]= 0 | (21 << 16);
m_ed_opcode_cycles[Z80__ED__OTIR]= 0 | (21 << 16);
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XB4] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XB5] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XB6] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XB7] = 4;

m_ed_opcode_cycles[Z80__ED__LDDR]= 0 | (21 << 16);
m_ed_opcode_cycles[Z80__ED__CPDR]= 0 | (21 << 16);
m_ed_opcode_cycles[Z80__ED__INDR]= 0 | (21 << 16);
m_ed_opcode_cycles[Z80__ED__OTDR]= 0 | (21 << 16);
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XBC] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XBD] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XBE] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XBF] = 4;

m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XC0] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XC1] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XC2] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XC3] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XC4] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XC5] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XC6] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XC7] = 4;

m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XC8] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XC9] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XCA] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XCB] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XCC] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XCD] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XCE] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XCF] = 4;

m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XD0] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XD1] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XD2] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XD3] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XD4] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XD5] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XD6] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XD7] = 4;

m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XD8] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XD9] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XDA] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XDB] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XDC] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XDD] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XDE] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XDF] = 4;

m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XE0] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XE1] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XE2] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XE3] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XE4] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XE5] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XE6] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XE7] = 4;

m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XE8] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XE9] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XEA] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XEB] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XEC] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XED] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XEE] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XEF] = 4;

m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XF0] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XF1] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XF2] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XF3] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XF4] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XF5] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XF6] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XF7] = 4;

m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XF8] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XF9] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XFA] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XFB] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XFC] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XFD] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XFE] = 4;
m_ed_opcode_cycles[Z80__ED__NOP__0XED__0XFF] = 4;

// size in bytes of plain opcodes, including operand

1,        // Z80__PLAIN__NOP
3,        // Z80__PLAIN__LD__BC__NN
1,        // Z80__PLAIN__LD__INDIRECT_BC__A
1,        // Z80__PLAIN__INC__BC
1,        // Z80__PLAIN__INC__B
1,        // Z80__PLAIN__DEC__B
2,        // Z80__PLAIN__LD__B__N
1,        // Z80__PLAIN__RLCA
1,        // Z80__PLAIN__EX__AF__AF_SHADOW
1,        // Z80__PLAIN__ADD__HL__BC
1,        // Z80__PLAIN__LD__A__INDIRECT_BC
1,        // Z80__PLAIN__DEC__BC
1,        // Z80__PLAIN__INC__C
1,        // Z80__PLAIN__DEC__C
2,        // Z80__PLAIN__LD__C__N
1,        // Z80__PLAIN__RRCA
 
2,        // Z80__PLAIN__DJNZ__d
3,        // Z80__PLAIN__LD__DE__NN
1,        // Z80__PLAIN__LD__INDIRECT_DE__A
1,        // Z80__PLAIN__INC__DE
1,        // Z80__PLAIN__INC__D
1,        // Z80__PLAIN__DEC__D
2,        // Z80__PLAIN__LD__D__N
1,        // Z80__PLAIN__RLA
2,        // Z80__PLAIN__JR__d
1,        // Z80__PLAIN__ADD__HL__DE
1,        // Z80__PLAIN__LD__A__INDIRECT_DE
1,        // Z80__PLAIN__DEC__DE
1,        // Z80__PLAIN__INC__E
1,        // Z80__PLAIN__DEC__E
2,        // Z80__PLAIN__LD__E__N
1,        // Z80__PLAIN__RRA
 
2,        // Z80__PLAIN__JR__NZ__d
3,        // Z80__PLAIN__LD__HL__NN
3,        // Z80__PLAIN__LD__INDIRECT_NN__HL
1,        // Z80__PLAIN__INC__HL
1,        // Z80__PLAIN__INC__H
1,        // Z80__PLAIN__DEC__H
2,        // Z80__PLAIN__LD__H__N
1,        // Z80__PLAIN__DAA
2,        // Z80__PLAIN__JR__Z__d
1,        // Z80__PLAIN__ADD__HL__HL
3,        // Z80__PLAIN__LD__HL__INDIRECT_NN
1,        // Z80__PLAIN__DEC__HL
1,        // Z80__PLAIN__INC__L
1,        // Z80__PLAIN__DEC__L
2,        // Z80__PLAIN__LD__L__N
1,        // Z80__PLAIN__CPL
 
2,        // Z80__PLAIN__JR__NC__d
3,        // Z80__PLAIN__LD__SP__NN
3,        // Z80__PLAIN__LD__INDIRECT_NN__A
1,        // Z80__PLAIN__INC__SP
1,        // Z80__PLAIN__INC__INDIRECT_HL
1,        // Z80__PLAIN__DEC__INDIRECT_HL
2,        // Z80__PLAIN__LD__INDIRECT_HL__N
1,        // Z80__PLAIN__SCF
2,        // Z80__PLAIN__JR__C__d
1,        // Z80__PLAIN__ADD__HL__SP
3,        // Z80__PLAIN__LD__A__INDIRECT_NN
1,        // Z80__PLAIN__DEC__SP
1,        // Z80__PLAIN__INC__A
1,        // Z80__PLAIN__DEC__A
2,        // Z80__PLAIN__LD__A__N
1,        // Z80__PLAIN__CCF
 
1,        // Z80__PLAIN__LD__B__B
1,        // Z80__PLAIN__LD__B__C
1,        // Z80__PLAIN__LD__B__D
1,        // Z80__PLAIN__LD__B__E
1,        // Z80__PLAIN__LD__B__H
1,        // Z80__PLAIN__LD__B__L
1,        // Z80__PLAIN__LD__B__INDIRECT_HL
1,        // Z80__PLAIN__LD__B__A
1,        // Z80__PLAIN__LD__C__B
1,        // Z80__PLAIN__LD__C__C
1,        // Z80__PLAIN__LD__C__D
1,        // Z80__PLAIN__LD__C__E
1,        // Z80__PLAIN__LD__C__H
1,        // Z80__PLAIN__LD__C__L
1,        // Z80__PLAIN__LD__C__INDIRECT_HL
1,        // Z80__PLAIN__LD__C__A
 
1,        // Z80__PLAIN__LD__D__B
1,        // Z80__PLAIN__LD__D__C
1,        // Z80__PLAIN__LD__D__D
1,        // Z80__PLAIN__LD__D__E
1,        // Z80__PLAIN__LD__D__H
1,        // Z80__PLAIN__LD__D__L
1,        // Z80__PLAIN__LD__D__INDIRECT_HL
1,        // Z80__PLAIN__LD__D__A
1,        // Z80__PLAIN__LD__E__B
1,        // Z80__PLAIN__LD__E__C
1,        // Z80__PLAIN__LD__E__D
1,        // Z80__PLAIN__LD__E__E
1,        // Z80__PLAIN__LD__E__H
1,        // Z80__PLAIN__LD__E__L
1,        // Z80__PLAIN__LD__E__INDIRECT_HL
1,        // Z80__PLAIN__LD__E__A
 
1,        // Z80__PLAIN__LD__H__B
1,        // Z80__PLAIN__LD__H__C
1,        // Z80__PLAIN__LD__H__D
1,        // Z80__PLAIN__LD__H__E
1,        // Z80__PLAIN__LD__H__H
1,        // Z80__PLAIN__LD__H__L
1,        // Z80__PLAIN__LD__H__INDIRECT_HL
1,        // Z80__PLAIN__LD__H__A
1,        // Z80__PLAIN__LD__L__B
1,        // Z80__PLAIN__LD__L__C
1,        // Z80__PLAIN__LD__L__D
1,        // Z80__PLAIN__LD__L__E
1,        // Z80__PLAIN__LD__L__H
1,        // Z80__PLAIN__LD__L__L
1,        // Z80__PLAIN__LD__L__INDIRECT_HL
1,        // Z80__PLAIN__LD__L__A
 
1,        // Z80__PLAIN__LD__INDIRECT_HL__B
1,        // Z80__PLAIN__LD__INDIRECT_HL__C
1,        // Z80__PLAIN__LD__INDIRECT_HL__D
1,        // Z80__PLAIN__LD__INDIRECT_HL__E
1,        // Z80__PLAIN__LD__INDIRECT_HL__H
1,        // Z80__PLAIN__LD__INDIRECT_HL__L
1,        // Z80__PLAIN__HALT
1,        // Z80__PLAIN__LD__INDIRECT_HL__A
1,        // Z80__PLAIN__LD__A__B
1,        // Z80__PLAIN__LD__A__C
1,        // Z80__PLAIN__LD__A__D
1,        // Z80__PLAIN__LD__A__E
1,        // Z80__PLAIN__LD__A__H
1,        // Z80__PLAIN__LD__A__L
1,        // Z80__PLAIN__LD__A__INDIRECT_HL
1,        // Z80__PLAIN__LD__A__A
 
1,        // Z80__PLAIN__ADD__A__B
1,        // Z80__PLAIN__ADD__A__C
1,        // Z80__PLAIN__ADD__A__D
1,        // Z80__PLAIN__ADD__A__E
1,        // Z80__PLAIN__ADD__A__H
1,        // Z80__PLAIN__ADD__A__L
1,        // Z80__PLAIN__ADD__A__INDIRECT_HL
1,        // Z80__PLAIN__ADD__A__A
1,        // Z80__PLAIN__ADC__A__B
1,        // Z80__PLAIN__ADC__A__C
1,        // Z80__PLAIN__ADC__A__D
1,        // Z80__PLAIN__ADC__A__E
1,        // Z80__PLAIN__ADC__A__H
1,        // Z80__PLAIN__ADC__A__L
1,        // Z80__PLAIN__ADC__A__INDIRECT_HL
1,        // Z80__PLAIN__ADC__A__A
 
1,        // Z80__PLAIN__SUB__B
1,        // Z80__PLAIN__SUB__C
1,        // Z80__PLAIN__SUB__D
1,        // Z80__PLAIN__SUB__E
1,        // Z80__PLAIN__SUB__H
1,        // Z80__PLAIN__SUB__L
1,        // Z80__PLAIN__SUB__INDIRECT_HL
1,        // Z80__PLAIN__SUB__A
1,        // Z80__PLAIN__SBC__A__B
1,        // Z80__PLAIN__SBC__A__C
1,        // Z80__PLAIN__SBC__A__D
1,        // Z80__PLAIN__SBC__A__E
1,        // Z80__PLAIN__SBC__A__H
1,        // Z80__PLAIN__SBC__A__L
1,        // Z80__PLAIN__SBC__A__INDIRECT_HL
1,        // Z80__PLAIN__SBC__A__A
 
1,        // Z80__PLAIN__AND__B
1,        // Z80__PLAIN__AND__C
1,        // Z80__PLAIN__AND__D
1,        // Z80__PLAIN__AND__E
1,        // Z80__PLAIN__AND__H
1,        // Z80__PLAIN__AND__L
1,        // Z80__PLAIN__AND__INDIRECT_HL
1,        // Z80__PLAIN__AND__A
1,        // Z80__PLAIN__XOR__B
1,        // Z80__PLAIN__XOR__C
1,        // Z80__PLAIN__XOR__D
1,        // Z80__PLAIN__XOR__E
1,        // Z80__PLAIN__XOR__H
1,        // Z80__PLAIN__XOR__L
1,        // Z80__PLAIN__XOR__INDIRECT_HL
1,        // Z80__PLAIN__XOR__A
 
1,        // Z80__PLAIN__OR__B
1,        // Z80__PLAIN__OR__C
1,        // Z80__PLAIN__OR__D
1,        // Z80__PLAIN__OR__E
1,        // Z80__PLAIN__OR__H
1,        // Z80__PLAIN__OR__L
1,        // Z80__PLAIN__OR__INDIRECT_HL
1,        // Z80__PLAIN__OR__A
1,        // Z80__PLAIN__CP__B
1,        // Z80__PLAIN__CP__C
1,        // Z80__PLAIN__CP__D
1,        // Z80__PLAIN__CP__E
1,        // Z80__PLAIN__CP__H
1,        // Z80__PLAIN__CP__L
1,        // Z80__PLAIN__CP__INDIRECT_HL
1,        // Z80__PLAIN__CP__A
 
1,        // Z80__PLAIN__RET__NZ
1,        // Z80__PLAIN__POP__BC
3,        // Z80__PLAIN__JP__NZ__NN
3,        // Z80__PLAIN__JP__NN
3,        // Z80__PLAIN__CALL__NZ__NN
1,        // Z80__PLAIN__PUSH__BC
2,        // Z80__PLAIN__ADD__A__N
1,        // Z80__PLAIN__RST__00
1,        // Z80__PLAIN__RET__Z
1,        // Z80__PLAIN__RET
3,        // Z80__PLAIN__JP__Z__NN
0,        // Z80__PLAIN__PREFIX__CB /* 0xcb */ 
3,        // Z80__PLAIN__CALL__Z__NN
3,        // Z80__PLAIN__CALL__NN
2,        // Z80__PLAIN__ADC__A__N
1,        // Z80__PLAIN__RST__08
 
1,        // Z80__PLAIN__RET__NC
1,        // Z80__PLAIN__POP__DE
3,        // Z80__PLAIN__JP__NC__NN
2,        // Z80__PLAIN__OUT__INDIRECT_N__A
3,        // Z80__PLAIN__CALL__NC__NN
1,        // Z80__PLAIN__PUSH__DE
2,        // Z80__PLAIN__SUB__N
1,        // Z80__PLAIN__RST__10
1,        // Z80__PLAIN__RET__C
1,        // Z80__PLAIN__EXX
3,        // Z80__PLAIN__JP__C__NN
2,        // Z80__PLAIN__IN__A__INDIRECT_N
3,        // Z80__PLAIN__CALL__C__NN
0,        // Z80__PLAIN__PREFIX__DD	/* oxdd */ 
2,        // Z80__PLAIN__SBC__A__N
1,        // Z80__PLAIN__RST__18
 
1,        // Z80__PLAIN__RET__PO
1,        // Z80__PLAIN__POP__HL
3,        // Z80__PLAIN__JP__PO__NN
1,        // Z80__PLAIN__EX__INDIRECT_SP__HL
3,        // Z80__PLAIN__CALL__PO__NN
1,        // Z80__PLAIN__PUSH__HL
2,        // Z80__PLAIN__AND__N
1,        // Z80__PLAIN__RST__20
1,        // Z80__PLAIN__RET__PE
1,        // Z80__PLAIN__JP__INDIRECT_HL
3,        // Z80__PLAIN__JP__PE__NN
1,        // Z80__PLAIN__EX__DE__HL
3,        // Z80__PLAIN__CALL__PE__NN
0,        // Z80__PLAIN__PREFIX__ED
2,        // Z80__PLAIN__XOR__N
1,        // Z80__PLAIN__RST__28
 
1,        // Z80__PLAIN__RET__P
1,        // Z80__PLAIN__POP__AF
3,        // Z80__PLAIN__JP__P__NN
1,        // Z80__PLAIN__DI
3,        // Z80__PLAIN__CALL__P__NN
1,        // Z80__PLAIN__PUSH__AF
2,        // Z80__PLAIN__OR__N
1,        // Z80__PLAIN__RST__30
1,        // Z80__PLAIN__RET__M
1,        // Z80__PLAIN__LD__SP__HL
3,        // Z80__PLAIN__JP__M__NN
1,        // Z80__PLAIN__EI
3,        // Z80__PLAIN__CALL__M__NN
0,        // Z80__PLAIN__PREFIX__FD
2,        // Z80__PLAIN__CP__N
1,        // Z80__PLAIN__RST__38

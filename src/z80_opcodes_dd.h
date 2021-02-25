#ifndef Z80_OPCODES_DD_H
#define Z80_OPCODES_DD_H

/* a large number of these opcodes replicate precisely plain opcodes. in these
 * cases, the executor for 0xdd opcodes should call the plain opcode executor
 * and add to the returned byte size. this saves code duplication, but care will
 * need to be taken that an infinite recursion does not take place where the
 * two execution methods continually call one another */
#define Z80__DD__NOP				0x00
#define Z80__DD__LD__BC__NN				0x01
#define Z80__DD__LD__INDIRECT_BC__A				0x02
#define Z80__DD__INC__BC				0x03
#define Z80__DD__INC__B				0x04
#define Z80__DD__DEC__B				0x05
#define Z80__DD__LD__B__N				0x06
#define Z80__DD__RLCA				0x07

#define Z80__DD__EX__AF__AF_SHADOW				0x08
#define Z80__DD__ADD__IX__BC				0x09
#define Z80__DD__LD__A__INDIRECT_BC				0x0a
#define Z80__DD__DEC__BC				0x0b
#define Z80__DD__INC__C				0x0c
#define Z80__DD__DEC__C				0x0d
#define Z80__DD__LD__C__N				0x0e
#define Z80__DD__RRCA				0x0f

#define Z80__DD__DJNZ__d				0x10
#define Z80__DD__LD__DE__NN				0x11
#define Z80__DD__LD__INDIRECT_DE__A				0x12
#define Z80__DD__INC__DE				0x13
#define Z80__DD__INC__D				0x14
#define Z80__DD__DEC__D				0x15
#define Z80__DD__LD__D__N				0x16
#define Z80__DD__RLA				0x17

#define Z80__DD__JR__d				0x18
#define Z80__DD__ADD__IX__DE				0x19
#define Z80__DD__LD__A__INDIRECT_DE				0x1a
#define Z80__DD__DEC__DE				0x1b
#define Z80__DD__INC__E				0x1c
#define Z80__DD__DEC__E				0x1d
#define Z80__DD__LD__E__N				0x1e
#define Z80__DD__RRA				0x1f

#define Z80__DD__JR__NZ__d				0x20
#define Z80__DD__LD__IX__NN				0x21
#define Z80__DD__LD__INDIRECT_NN__IX				0x22
#define Z80__DD__INC__IX				0x23
#define Z80__DD__INC__IXH				0x24
#define Z80__DD__DEC__IXH				0x25
#define Z80__DD__LD__IXH__N				0x26
#define Z80__DD__DAA				0x27

#define Z80__DD__JR__Z__d				0x28
#define Z80__DD__ADD__IX__IX				0x29
#define Z80__DD__LD__IX__INDIRECT_NN				0x2a
#define Z80__DD__DEC__IX				0x2b
#define Z80__DD__INC__IXL				0x2c
#define Z80__DD__DEC__IXL				0x2d
#define Z80__DD__LD__IXL__N				0x2e
#define Z80__DD__CPL				0x2f

#define Z80__DD__JR__NC__d				0x30
#define Z80__DD__LD__SP__NN				0x31
#define Z80__DD__LD__INDIRECT_NN__A				0x32
#define Z80__DD__INC__SP				0x33
#define Z80__DD__INC__(IX+d)				0x34
#define Z80__DD__DEC__(IX+d)				0x35
#define Z80__DD__LD__(IX+d)__N				0x36
#define Z80__DD__SCF				0x37

#define Z80__DD__JR__C__d				0x38
#define Z80__DD__ADD__IX__SP				0x39
#define Z80__DD__LD__A__INDIRECT_NN				0x3a
#define Z80__DD__DEC__SP				0x3b
#define Z80__DD__INC__A				0x3c
#define Z80__DD__DEC__A				0x3d
#define Z80__DD__LD__A__N				0x3e
#define Z80__DD__CCF				0x3f

#define Z80__DD__LD__B__B				0x40
#define Z80__DD__LD__B__C				0x41
#define Z80__DD__LD__B__D				0x42
#define Z80__DD__LD__B__E				0x43
#define Z80__DD__LD__B__IXH				0x44
#define Z80__DD__LD__B__IXL				0x45
#define Z80__DD__LD__B__(IX+d)				0x46
#define Z80__DD__LD__B__A				0x47

#define Z80__DD__LD__C__B				0x48
#define Z80__DD__LD__C__C				0x49
#define Z80__DD__LD__C__D				0x4a
#define Z80__DD__LD__C__E				0x4b
#define Z80__DD__LD__C__IXH				0x4c
#define Z80__DD__LD__C__IXL				0x4d
#define Z80__DD__LD__C__(IX+d)				0x4e
#define Z80__DD__LD__C__A				0x4f

#define Z80__DD__LD__D__B				0x50
#define Z80__DD__LD__D__C				0x51
#define Z80__DD__LD__D__D				0x52
#define Z80__DD__LD__D__E				0x53
#define Z80__DD__LD__D__IXH				0x54
#define Z80__DD__LD__D__IXL				0x55
#define Z80__DD__LD__D__(IX+d)				0x56
#define Z80__DD__LD__D__A				0x57

#define Z80__DD__LD__E__B				0x58
#define Z80__DD__LD__E__C				0x59
#define Z80__DD__LD__E__D				0x5a
#define Z80__DD__LD__E__E				0x5b
#define Z80__DD__LD__E__IXH				0x5c
#define Z80__DD__LD__E__IXL				0x5d
#define Z80__DD__LD__E__(IX+d)				0x5e
#define Z80__DD__LD__E__A				0x5f

#define Z80__DD__LD__IXH__B				0x60
#define Z80__DD__LD__IXH__C				0x61
#define Z80__DD__LD__IXH__D				0x62
#define Z80__DD__LD__IXH__E				0x63
#define Z80__DD__LD__IXH__IXH				0x64
#define Z80__DD__LD__IXH__IXL				0x65
#define Z80__DD__LD__H__(IX+d)				0x66
#define Z80__DD__LD__IXH__A				0x67

#define Z80__DD__LD__IXL__B				0x68
#define Z80__DD__LD__IXL__C				0x69
#define Z80__DD__LD__IXL__D				0x6a
#define Z80__DD__LD__IXL__E				0x6b
#define Z80__DD__LD__IXL__H				0x6c
#define Z80__DD__LD__IXL__IXL				0x6d
#define Z80__DD__LD__L__(IX+d)				0x6e
#define Z80__DD__LD__IXL__A				0x6f

#define Z80__DD__LD__(IX+d)__B				0x70
#define Z80__DD__LD__(IX+d)__C				0x71
#define Z80__DD__LD__(IX+d)__D				0x72
#define Z80__DD__LD__(IX+d)__E				0x73
#define Z80__DD__LD__(IX+d)__H				0x74
#define Z80__DD__LD__(IX+d)__L				0x75
#define Z80__DD__HALT				0x76
#define Z80__DD__LD__(IX+d)__A				0x77

#define Z80__DD__LD__A__B				0x78
#define Z80__DD__LD__A__C				0x79
#define Z80__DD__LD__A__D				0x7a
#define Z80__DD__LD__A__E				0x7b
#define Z80__DD__LD__A__IXH				0x7c
#define Z80__DD__LD__A__IXL				0x7d
#define Z80__DD__LD__A__(IX+d)				0x7e
#define Z80__DD__LD__A__A				0x7f

#define Z80__DD__ADD__A__B				0x80
#define Z80__DD__ADD__A__C				0x81
#define Z80__DD__ADD__A__D				0x82
#define Z80__DD__ADD__A__E				0x83
#define Z80__DD__ADD__A__IXH				0x84
#define Z80__DD__ADD__A__IXL				0x85
#define Z80__DD__ADD__A__(IX+d)				0x86
#define Z80__DD__ADD__A__A				0x87

#define Z80__DD__ADC__A__B				0x88
#define Z80__DD__ADC__A__C				0x89
#define Z80__DD__ADC__A__D				0x8a
#define Z80__DD__ADC__A__E				0x8b
#define Z80__DD__ADC__A__IXH				0x8c
#define Z80__DD__ADC__A__IXL				0x8d
#define Z80__DD__ADC__A__(IX+d)				0x8e
#define Z80__DD__ADC__A__A				0x8f

#define Z80__DD__SUB__B				0x90
#define Z80__DD__SUB__C				0x91
#define Z80__DD__SUB__D				0x92
#define Z80__DD__SUB__E				0x93
#define Z80__DD__SUB__IXH				0x94
#define Z80__DD__SUB__IXL				0x95
#define Z80__DD__SUB__(IX+d)				0x96
#define Z80__DD__SUB__A				0x97

#define Z80__DD__SBC__A__B				0x98
#define Z80__DD__SBC__A__C				0x99
#define Z80__DD__SBC__A__D				0x9a
#define Z80__DD__SBC__A__E				0x9b
#define Z80__DD__SBC__A__IXH				0x9c
#define Z80__DD__SBC__A__IXL				0x9d
#define Z80__DD__SBC__A__(IX+d)				0x9e
#define Z80__DD__SBC__A__A				0x9f

#define Z80__DD__AND__B				0xa0
#define Z80__DD__AND__C				0xa1
#define Z80__DD__AND__D				0xa2
#define Z80__DD__AND__E				0xa3
#define Z80__DD__AND__IXH				0xa4
#define Z80__DD__AND__IXL				0xa5
#define Z80__DD__AND__(IX+d)				0xa6
#define Z80__DD__AND__A				0xa7

#define Z80__DD__XOR__B				0xa8
#define Z80__DD__XOR__C				0xa9
#define Z80__DD__XOR__D				0xaa
#define Z80__DD__XOR__E				0xab
#define Z80__DD__XOR__IXH				0xac
#define Z80__DD__XOR__IXL				0xad
#define Z80__DD__XOR__(IX+d)				0xae
#define Z80__DD__XOR__A				0xaf

#define Z80__DD__OR__B				0xb0
#define Z80__DD__OR__C				0xb1
#define Z80__DD__OR__D				0xb2
#define Z80__DD__OR__E				0xb3
#define Z80__DD__OR__IXH				0xb4
#define Z80__DD__OR__IXL				0xb5
#define Z80__DD__OR__(IX+d)				0xb6
#define Z80__DD__OR__A				0xb7

#define Z80__DD__CP__B				0xb8
#define Z80__DD__CP__C				0xb9
#define Z80__DD__CP__D				0xba
#define Z80__DD__CP__E				0xbb
#define Z80__DD__CP__IXH				0xbc
#define Z80__DD__CP__IXL				0xbd
#define Z80__DD__CP__(IX+d)				0xbe
#define Z80__DD__CP__A				0xbf

#define Z80__DD__RET__NZ				0xc0
#define Z80__DD__POP__BC				0xc1
#define Z80__DD__JP__NZ__NN				0xc2
#define Z80__DD__JP__NN				0xc3
#define Z80__DD__CALL__NZ__NN				0xc4
#define Z80__DD__PUSH__BC				0xc5
#define Z80__DD__ADD__A__N				0xc6
#define Z80__DD__RST__00				0xc7

#define Z80__DD__RET__Z				0xc8
#define Z80__DD__RET				0xc9
#define Z80__DD__JP__Z__NN				0xca
#define Z80__DD__PREFIX__[...]				0xcb
#define Z80__DD__CALL__Z__NN				0xcc
#define Z80__DD__CALL__NN				0xcd
#define Z80__DD__ADC__A__N				0xce
#define Z80__DD__RST__08				0xcf

#define Z80__DD__RET__NC				0xd0
#define Z80__DD__POP__DE				0xd1
#define Z80__DD__JP__NC__NN				0xd2
#define Z80__DD__OUT__(N)__A				0xd3
#define Z80__DD__CALL__NC__NN				0xd4
#define Z80__DD__PUSH__DE				0xd5
#define Z80__DD__SUB__N				0xd6
#define Z80__DD__RST__10				0xd7

#define Z80__DD__RET__C				0xd8
#define Z80__DD__EXX				0xd9
#define Z80__DD__JP__C__NN				0xda
#define Z80__DD__IN__A__(N)				0xdb
#define Z80__DD__CALL__C__NN				0xdc
#define Z80__DD__PREFIX__INDIRECT_IX				0xdd
#define Z80__DD__SBC__A__N				0xde
#define Z80__DD__RST__18				0xdf

#define Z80__DD__RET__PO				0xe0
#define Z80__DD__POP__IX				0xe1
#define Z80__DD__JP__PO__NN				0xe2
#define Z80__DD__EX__(SP)__IX				0xe3
#define Z80__DD__CALL__PO__NN				0xe4
#define Z80__DD__PUSH__IX				0xe5
#define Z80__DD__AND__N				0xe6
#define Z80__DD__RST__20				0xe7

#define Z80__DD__RET__PE				0xe8
#define Z80__DD__JP__INDIRECT_IX				0xe9
#define Z80__DD__JP__PE__NN				0xea
#define Z80__DD__EX__DE__HL				0xeb
#define Z80__DD__CALL__PE__NN				0xec
#define Z80__DD__PREFIX				0xed
#define Z80__DD__XOR__N				0xee
#define Z80__DD__RST__28				0xef

#define Z80__DD__RET__P				0xf0
#define Z80__DD__POP__AF				0xf1
#define Z80__DD__JP__P__NN				0xf2
#define Z80__DD__DI				0xf3
#define Z80__DD__CALL__P__NN				0xf4
#define Z80__DD__PUSH__AF				0xf5
#define Z80__DD__OR__N				0xf6
#define Z80__DD__RST__30				0xf7

#define Z80__DD__RET__M				0xf8
#define Z80__DD__LD__SP__IX				0xf9
#define Z80__DD__JP__M__NN				0xfa
#define Z80__DD__EI				0xfb
#define Z80__DD__CALL__M__NN				0xfc
#define Z80__DD__PREFIX__FD				0xfd
#define Z80__DD__CP__N				0xfe
#define Z80__DD__RST__38				0xff

#endif // Z80_OPCODES_DD_H

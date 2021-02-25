#ifndef Z80_ED_OPCODES_H
#define Z80_ED_OPCODES_H

#define Z80__ED__NOP__0XED__0X00				0x00
#define Z80__ED__NOP__0XED__0X01				0x01
#define Z80__ED__NOP__0XED__0X02				0x02
#define Z80__ED__NOP__0XED__0X03				0x03
#define Z80__ED__NOP__0XED__0X04				0x04
#define Z80__ED__NOP__0XED__0X05				0x05
#define Z80__ED__NOP__0XED__0X06				0x06
#define Z80__ED__NOP__0XED__0X07				0x07

#define Z80__ED__NOP__0XED__0X08				0x08
#define Z80__ED__NOP__0XED__0X09				0x09
#define Z80__ED__NOP__0XED__0X0A				0x0a
#define Z80__ED__NOP__0XED__0X0B				0x0b
#define Z80__ED__NOP__0XED__0X0C				0x0c
#define Z80__ED__NOP__0XED__0X0D				0x0d
#define Z80__ED__NOP__0XED__0X0E				0x0e
#define Z80__ED__NOP__0XED__0X0F				0x0f

#define Z80__ED__NOP__0XED__0X10				0x10
#define Z80__ED__NOP__0XED__0X11				0x11
#define Z80__ED__NOP__0XED__0X12				0x12
#define Z80__ED__NOP__0XED__0X13				0x13
#define Z80__ED__NOP__0XED__0X14				0x14
#define Z80__ED__NOP__0XED__0X15				0x15
#define Z80__ED__NOP__0XED__0X16				0x16
#define Z80__ED__NOP__0XED__0X17				0x17

#define Z80__ED__NOP__0XED__0X18				0x18
#define Z80__ED__NOP__0XED__0X19				0x19
#define Z80__ED__NOP__0XED__0X1A				0x1a
#define Z80__ED__NOP__0XED__0X1B				0x1b
#define Z80__ED__NOP__0XED__0X1C				0x1c
#define Z80__ED__NOP__0XED__0X1D				0x1d
#define Z80__ED__NOP__0XED__0X1E				0x1e
#define Z80__ED__NOP__0XED__0X1F				0x1f

#define Z80__ED__NOP__0XED__0X20				0x20
#define Z80__ED__NOP__0XED__0X21				0x21
#define Z80__ED__NOP__0XED__0X22				0x22
#define Z80__ED__NOP__0XED__0X23				0x23
#define Z80__ED__NOP__0XED__0X24				0x24
#define Z80__ED__NOP__0XED__0X25				0x25
#define Z80__ED__NOP__0XED__0X26				0x26
#define Z80__ED__NOP__0XED__0X27				0x27

#define Z80__ED__NOP__0XED__0X28				0x28
#define Z80__ED__NOP__0XED__0X29				0x29
#define Z80__ED__NOP__0XED__0X2A				0x2a
#define Z80__ED__NOP__0XED__0X2B				0x2b
#define Z80__ED__NOP__0XED__0X2C				0x2c
#define Z80__ED__NOP__0XED__0X2D				0x2d
#define Z80__ED__NOP__0XED__0X2E				0x2e
#define Z80__ED__NOP__0XED__0X2F				0x2f

#define Z80__ED__NOP__0XED__0X30				0x30
#define Z80__ED__NOP__0XED__0X31				0x31
#define Z80__ED__NOP__0XED__0X32				0x32
#define Z80__ED__NOP__0XED__0X33				0x33
#define Z80__ED__NOP__0XED__0X34				0x34
#define Z80__ED__NOP__0XED__0X35				0x35
#define Z80__ED__NOP__0XED__0X36				0x36
#define Z80__ED__NOP__0XED__0X37				0x37

#define Z80__ED__NOP__0XED__0X38				0x38
#define Z80__ED__NOP__0XED__0X39				0x39
#define Z80__ED__NOP__0XED__0X3A				0x3a
#define Z80__ED__NOP__0XED__0X3B				0x3b
#define Z80__ED__NOP__0XED__0X3C				0x3c
#define Z80__ED__NOP__0XED__0X3D				0x3d
#define Z80__ED__NOP__0XED__0X3E				0x3e
#define Z80__ED__NOP__0XED__0X3F				0x3f

#define Z80__ED__IN__B__INDIRECT_C			0x40
#define Z80__ED__OUT__INDIRECT_C__B			0x41
#define Z80__ED__SBC__HL__BC					0x42
#define Z80__ED__LD__INDIRECT_NN__BC		0x43
#define Z80__ED__NEG								0x44
#define Z80__ED__RETN							0x45
#define Z80__ED__IM__0							0x46
#define Z80__ED__LD__I__A						0x47

#define Z80__ED__IN__C__INDIRECT_C			0x48
#define Z80__ED__OUT__INDIRECT_C__C			0x49
#define Z80__ED__ADC__HL__BC					0x4a
#define Z80__ED__LD__BC__INDIRECT_NN		0x4b
#define Z80__ED__NEG__0XED__0X4C				0x4c
#define Z80__ED__RETI							0x4d
#define Z80__ED__IM__0__0XED__0X4E			0x4e
#define Z80__ED__LD__R__A						0x4f

#define Z80__ED__IN__D__INDIRECT_C			0x50
#define Z80__ED__OUT__INDIRECT_C__D			0x51
#define Z80__ED__SBC__HL__DE					0x52
#define Z80__ED__LD__INDIRECT_NN__DE		0x53
#define Z80__ED__NEG__0XED__0X54				0x54
#define Z80__ED__RETN__0XED__0X55			0x55
#define Z80__ED__IM__1							0x56
#define Z80__ED__LD__A__I						0x57

#define Z80__ED__IN__E__INDIRECT_C			0x58
#define Z80__ED__OUT__INDIRECT_C__E			0x59
#define Z80__ED__ADC__HL__DE					0x5a
#define Z80__ED__LD__DE__INDIRECT_NN		0x5b
#define Z80__ED__NEG__0XED__0X5C				0x5c
#define Z80__ED__RETI__0XED__0X5D			0x5d
#define Z80__ED__IM__2							0x5e
#define Z80__ED__LD__A__R						0x5f

#define Z80__ED__IN__H__INDIRECT_C			0x60
#define Z80__ED__OUT__INDIRECT_C__H			0x61
#define Z80__ED__SBC__HL__HL					0x62
#define Z80__ED__LD__INDIRECT_NN__HL		0x63
#define Z80__ED__NEG__0XED__0X64				0x64
#define Z80__ED__RETN__0XED__0X65			0x65
#define Z80__ED__IM__0__0XED__0X66			0x66
#define Z80__ED__RRD								0x67

#define Z80__ED__IN__L__INDIRECT_C			0x68
#define Z80__ED__OUT__INDIRECT_C__L			0x69
#define Z80__ED__ADC__HL__HL					0x6a
#define Z80__ED__LD__HL__INDIRECT_NN		0x6b
#define Z80__ED__NEG__0XED__0X6C				0x6c
#define Z80__ED__RETI__0XED__0X6D			0x6d
#define Z80__ED__IM__0__0XED__0X6E			0x6e
#define Z80__ED__RLD								0x6f

#define Z80__ED__IN__INDIRECT_C				0x70
#define Z80__ED__OUT__INDIRECT_C__0			0x71
#define Z80__ED__SBC__HL__SP					0x72
#define Z80__ED__LD__INDIRECT_NN__SP		0x73
#define Z80__ED__NEG__0XED__0X74				0x74
#define Z80__ED__RETN__0XED__0X75			0x75
#define Z80__ED__IM__1__0XED__0X76			0x76
#define Z80__ED__NOP__0XED__0x77				0x77

#define Z80__ED__IN__A__INDIRECT_C			0x78
#define Z80__ED__OUT__INDIRECT_C__A			0x79
#define Z80__ED__ADC__HL__SP					0x7a
#define Z80__ED__LD__SP__INDIRECT_NN		0x7b
#define Z80__ED__NEG__0XED__0X7C				0x7c
#define Z80__ED__RETI__0XED__0X7D			0x7d
#define Z80__ED__IM__2__0XED__0X7E			0x7e
#define Z80__ED__NOP__0XED__0X7F				0x7f

#define Z80__ED__NOP__0XED__0X80				0x80
#define Z80__ED__NOP__0XED__0X81				0x81
#define Z80__ED__NOP__0XED__0X82				0x82
#define Z80__ED__NOP__0XED__0X83				0x83
#define Z80__ED__NOP__0XED__0X84				0x84
#define Z80__ED__NOP__0XED__0X85				0x85
#define Z80__ED__NOP__0XED__0X86				0x86
#define Z80__ED__NOP__0XED__0X87				0x87

#define Z80__ED__NOP__0XED__0X88				0x88
#define Z80__ED__NOP__0XED__0X89				0x89
#define Z80__ED__NOP__0XED__0X8A				0x8a
#define Z80__ED__NOP__0XED__0X8B				0x8b
#define Z80__ED__NOP__0XED__0X8C				0x8c
#define Z80__ED__NOP__0XED__0X8D				0x8d
#define Z80__ED__NOP__0XED__0X8E				0x8e
#define Z80__ED__NOP__0XED__0X8F				0x8f

#define Z80__ED__NOP__0XED__0X90				0x90
#define Z80__ED__NOP__0XED__0X91				0x91
#define Z80__ED__NOP__0XED__0X92				0x92
#define Z80__ED__NOP__0XED__0X93				0x93
#define Z80__ED__NOP__0XED__0X94				0x94
#define Z80__ED__NOP__0XED__0X95				0x95
#define Z80__ED__NOP__0XED__0X96				0x96
#define Z80__ED__NOP__0XED__0X97				0x97

#define Z80__ED__NOP__0XED__0X98				0x98
#define Z80__ED__NOP__0XED__0X99				0x99
#define Z80__ED__NOP__0XED__0X9A				0x9a
#define Z80__ED__NOP__0XED__0X9B				0x9b
#define Z80__ED__NOP__0XED__0X9C				0x9c
#define Z80__ED__NOP__0XED__0X9D				0x9d
#define Z80__ED__NOP__0XED__0X9E				0x9e
#define Z80__ED__NOP__0XED__0X9F				0x9f


#define Z80__ED__LDI								0xa0
#define Z80__ED__CPI								0xa1
#define Z80__ED__INI								0xa2
#define Z80__ED__OUTI							0xa3
#define Z80__ED__NOP__0XED__0XA4				0xa4
#define Z80__ED__NOP__0XED__0XA5				0xa5
#define Z80__ED__NOP__0XED__0XA6				0xa6
#define Z80__ED__NOP__0XED__0XA7				0xa7

#define Z80__ED__LDD								0xa8
#define Z80__ED__CPD								0xa9
#define Z80__ED__IND								0xaa
#define Z80__ED__OUTD							0xab
#define Z80__ED__NOP__0XED__0XAC				0xac
#define Z80__ED__NOP__0XED__0XAD				0xad
#define Z80__ED__NOP__0XED__0XAE				0xae
#define Z80__ED__NOP__0XED__0XAF				0xaf

#define Z80__ED__LDIR							0xb0
#define Z80__ED__CPIR							0xb1
#define Z80__ED__INIR							0xb2
#define Z80__ED__OTIR							0xb3
#define Z80__ED__NOP__0XED__0XB4				0xb4
#define Z80__ED__NOP__0XED__0XB5				0xb5
#define Z80__ED__NOP__0XED__0XB6				0xb6
#define Z80__ED__NOP__0XED__0XB7				0xb7

#define Z80__ED__LDDR							0xb8
#define Z80__ED__CPDR							0xb9
#define Z80__ED__INDR							0xba
#define Z80__ED__OTDR							0xbb
#define Z80__ED__NOP__0XED__0XBC				0xbc
#define Z80__ED__NOP__0XED__0XBD				0xbd
#define Z80__ED__NOP__0XED__0XBE				0xbe
#define Z80__ED__NOP__0XED__0XBF				0xbf

#define Z80__ED__NOP__0XED__0XC0				0xc0
#define Z80__ED__NOP__0XED__0XC1				0xc1
#define Z80__ED__NOP__0XED__0XC2				0xc2
#define Z80__ED__NOP__0XED__0XC3				0xc3
#define Z80__ED__NOP__0XED__0XC4				0xc4
#define Z80__ED__NOP__0XED__0XC5				0xc5
#define Z80__ED__NOP__0XED__0XC6				0xc6
#define Z80__ED__NOP__0XED__0XC7				0xc7

#define Z80__ED__NOP__0XED__0XC8				0xc8
#define Z80__ED__NOP__0XED__0XC9				0xc9
#define Z80__ED__NOP__0XED__0XCA				0xca
#define Z80__ED__NOP__0XED__0XCB				0xcb
#define Z80__ED__NOP__0XED__0XCC				0xcc
#define Z80__ED__NOP__0XED__0XCD				0xcd
#define Z80__ED__NOP__0XED__0XCE				0xce
#define Z80__ED__NOP__0XED__0XCF				0xcf

#define Z80__ED__NOP__0XED__0XD0				0xd0
#define Z80__ED__NOP__0XED__0XD1				0xd1
#define Z80__ED__NOP__0XED__0XD2				0xd2
#define Z80__ED__NOP__0XED__0XD3				0xd3
#define Z80__ED__NOP__0XED__0XD4				0xd4
#define Z80__ED__NOP__0XED__0XD5				0xd5
#define Z80__ED__NOP__0XED__0XD6				0xd6
#define Z80__ED__NOP__0XED__0XD7				0xd7

#define Z80__ED__NOP__0XED__0XD8				0xd8
#define Z80__ED__NOP__0XED__0XD9				0xd9
#define Z80__ED__NOP__0XED__0XDA				0xda
#define Z80__ED__NOP__0XED__0XDB				0xdb
#define Z80__ED__NOP__0XED__0XDC				0xdc
#define Z80__ED__NOP__0XED__0XDD				0xdd
#define Z80__ED__NOP__0XED__0XDE				0xde
#define Z80__ED__NOP__0XED__0XDF				0xdf

#define Z80__ED__NOP__0XED__0XE0				0xe0
#define Z80__ED__NOP__0XED__0XE1				0xe1
#define Z80__ED__NOP__0XED__0XE2				0xe2
#define Z80__ED__NOP__0XED__0XE3				0xe3
#define Z80__ED__NOP__0XED__0XE4				0xe4
#define Z80__ED__NOP__0XED__0XE5				0xe5
#define Z80__ED__NOP__0XED__0XE6				0xe6
#define Z80__ED__NOP__0XED__0XE7				0xe7

#define Z80__ED__NOP__0XED__0XE8				0xe8
#define Z80__ED__NOP__0XED__0XE9				0xe9
#define Z80__ED__NOP__0XED__0XEA				0xea
#define Z80__ED__NOP__0XED__0XEB				0xeb
#define Z80__ED__NOP__0XED__0XEC				0xec
#define Z80__ED__NOP__0XED__0XED				0xed
#define Z80__ED__NOP__0XED__0XEE				0xee
#define Z80__ED__NOP__0XED__0XEF				0xef

#define Z80__ED__NOP__0XED__0XF0				0xf0
#define Z80__ED__NOP__0XED__0XF1				0xf1
#define Z80__ED__NOP__0XED__0XF2				0xf2
#define Z80__ED__NOP__0XED__0XF3				0xf3
#define Z80__ED__NOP__0XED__0XF4				0xf4
#define Z80__ED__NOP__0XED__0XF5				0xf5
#define Z80__ED__NOP__0XED__0XF6				0xf6
#define Z80__ED__NOP__0XED__0XF7				0xf7

#define Z80__ED__NOP__0XED__0XF8				0xf8
#define Z80__ED__NOP__0XED__0XF9				0xf9
#define Z80__ED__NOP__0XED__0XFA				0xfa
#define Z80__ED__NOP__0XED__0XFB				0xfb
#define Z80__ED__NOP__0XED__0XFC				0xfc
#define Z80__ED__NOP__0XED__0XFD				0xfd
#define Z80__ED__NOP__0XED__0XFE				0xfe
#define Z80__ED__NOP__0XED__0XFF				0xff

#endif		// Z80_ED_OPCODES_H

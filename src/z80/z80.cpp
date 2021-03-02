/**
 * \file z80.cpp
 * \author Darren Edale
 * \version 0.5
 *
 * \brief Implementation of the Z80 CPU emulation.
 *
 * This emulation is based on interpretation. There is no dynamic compilation and no cross-assembly.
 *
 * \todo
 * - IX and IY register instructions (0xdd and 0xfd)
 * - interrupt modes 0 and 2
 * - all IN and OUT instructions
 * - architecture to connect "devices" to IN and OUT ports.
 */
#include "z80.h"

#include "z80iodevice.h"
//#include "../types.h"
#include "z80_opcodes.h"
#include <cstdlib>
#include <iostream>
#include <cassert>

#define Z80_UNUSED(x) ((void) (x));

#define Z80_TICK_DURATION 50 /* 1/n seconds */
#define Z80_TICK_CYCLE_THRESHOLD (clockSpeed() / Z80_TICK_DURATION)

// downcast values to Z80 data types
#define Z80_CUBYTE(v) Z80::Z80::UnsignedByte((v) & 0x00ff)
#define Z80_CBYTE(v) Z80::Z80::SignedByte((v) & 0x00ff)
#define Z80_CUWORD(v) Z80::Z80::UnsignedWord((v) & 0x0000ffff)
#define Z80_CWORD(v) Z80::Z80::SignedWord((v) & 0x0000ffff)

#define Z80_FLAG_C_SET (m_registers.f |= Z80_FLAG_C_MASK)
#define Z80_FLAG_Z_SET (m_registers.f |= Z80_FLAG_Z_MASK)
#define Z80_FLAG_P_SET (m_registers.f |= Z80_FLAG_P_MASK)
#define Z80_FLAG_S_SET (m_registers.f |= Z80_FLAG_S_MASK)
#define Z80_FLAG_N_SET (m_registers.f |= Z80_FLAG_N_MASK)
#define Z80_FLAG_H_SET (m_registers.f |= Z80_FLAG_H_MASK)
#define Z80_FLAG_F3_SET (m_registers.f |= Z80_FLAG_F3_MASK)
#define Z80_FLAG_F5_SET (m_registers.f |= Z80_FLAG_F5_MASK)

#define Z80_FLAG_C_CLEAR (m_registers.f &= ~Z80_FLAG_C_MASK)
#define Z80_FLAG_Z_CLEAR (m_registers.f &= ~Z80_FLAG_Z_MASK)
#define Z80_FLAG_P_CLEAR (m_registers.f &= ~Z80_FLAG_P_MASK)
#define Z80_FLAG_S_CLEAR (m_registers.f &= ~Z80_FLAG_S_MASK)
#define Z80_FLAG_N_CLEAR (m_registers.f &= ~Z80_FLAG_N_MASK)
#define Z80_FLAG_H_CLEAR (m_registers.f &= ~Z80_FLAG_H_MASK)
#define Z80_FLAG_F3_CLEAR (m_registers.f &= ~Z80_FLAG_F3_MASK)
#define Z80_FLAG_F5_CLEAR (m_registers.f &= ~Z80_FLAG_F5_MASK)

#define Z80_FLAG_C_UPDATE(cond) if (cond) Z80_FLAG_C_SET; else Z80_FLAG_C_CLEAR
#define Z80_FLAG_Z_UPDATE(cond) if (cond) Z80_FLAG_Z_SET; else Z80_FLAG_Z_CLEAR
#define Z80_FLAG_P_UPDATE(cond) if (cond) Z80_FLAG_P_SET; else Z80_FLAG_P_CLEAR
#define Z80_FLAG_S_UPDATE(cond) if (cond) Z80_FLAG_S_SET; else Z80_FLAG_S_CLEAR
#define Z80_FLAG_N_UPDATE(cond) if (cond) Z80_FLAG_N_SET; else Z80_FLAG_N_CLEAR
#define Z80_FLAG_H_UPDATE(cond) if (cond) Z80_FLAG_H_SET; else Z80_FLAG_H_CLEAR
#define Z80_FLAG_F3_UPDATE(cond) if (cond) Z80_FLAG_F3_SET; else Z80_FLAG_F3_CLEAR
#define Z80_FLAG_F5_UPDATE(cond) if (cond) Z80_FLAG_F5_SET; else Z80_FLAG_F5_CLEAR

// TODO very often S, 5 and 3 flags are simply set to the same bits as the result (usually reg A)
#define Z80_FLAGS_S53_UPDATE(byte) (m_registers.f = (m_registers.f & 0b01010111) | ((byte) & 0b10101000))

#define Z80_FLAG_C_ISSET (0 != (m_registers.f & Z80_FLAG_C_MASK))
#define Z80_FLAG_Z_ISSET (0 != (m_registers.f & Z80_FLAG_Z_MASK))
#define Z80_FLAG_P_ISSET (0 != (m_registers.f & Z80_FLAG_P_MASK))
#define Z80_FLAG_S_ISSET (0 != (m_registers.f & Z80_FLAG_S_MASK))
#define Z80_FLAG_N_ISSET (0 != (m_registers.f & Z80_FLAG_N_MASK))
#define Z80_FLAG_H_ISSET (0 != (m_registers.f & Z80_FLAG_H_MASK))
#define Z80_FLAG_F3_ISSET (0 != (m_registers.f & Z80_FLAG_F3_MASK))
#define Z80_FLAG_F5_ISSET (0 != (m_registers.f & Z80_FLAG_F5_MASK))

/* default behaviour of flags after an operation on an 8-bit register */
#define Z80_FLAG_Z_DEFAULTBEHAVIOUR Z80_FLAG_Z_UPDATE(0 == m_registers.a);
#define Z80_FLAG_S_DEFAULTBEHAVIOUR Z80_FLAG_S_UPDATE(0x80 & m_registers.a);
#define Z80_FLAG_F5_DEFAULTBEHAVIOUR
#define Z80_FLAG_P_DEFAULTBEHAVIOUR
#define Z80_FLAG_F3_DEFAULTBEHAVIOUR
#define Z80_FLAG_N_DEFAULTBEHAVIOUR
#define Z80_FLAG_H_DEFAULTBEHAVIOUR
#define Z80_FLAG_C_DEFAULTBEHAVIOUR

/* default behaviour of flags after an operation on a 16-bit register */
#define Z80_FLAG_Z_DEFAULTBEHAVIOUR16 Z80_FLAG_Z_UPDATE(0 == m_registers.hl);
#define Z80_FLAG_S_DEFAULTBEHAVIOUR16 Z80_FLAG_S_UPDATE(0x80 & m_registers.hl);

#define Z80_FLAG_P_OVERFLOW Z80_FLAG_P_UPDATE()

/* checks for carry from bit 3 to it 4 */
#define Z80_CHECK_8BIT_HALFCARRY(before,after) ((before) <= 0x0f && (after) > 0x0f)
/* checks for carry from bit 10 to bit 11 */
#define Z80_CHECK_16BIT_HALFCARRY_10_TO_11(before,after) ((before) <= 0x07ff && (after) > 0x07ff)
#define Z80_CHECK_16BIT_HALFCARRY_11_TO_12(before,after) ((before) <= 0x0fff && (after) > 0x0fff)

/* update P/V flag for 8-bit overflow during addition */
#define Z80_FLAG_H_UPDATE_ADD(orig,delta,result) \
{                                               \
    UnsignedByte tmpHalfCarry = (               \
        (((result) & 0b00001000) >> 1)          \
        | (((delta) & 0b00001000) >> 2)          \
        | (((orig) & 0b00001000) >> 3)          \
    );                                          \
                                                \
    Z80_FLAG_H_UPDATE(tmpHalfCarry == 1 || tmpHalfCarry == 2 || tmpHalfCarry == 3 || tmpHalfCarry == 7);\
}

/* update P/V flag for 8-bit overflow during subtraction */
#define Z80_FLAG_H_UPDATE_SUB(orig,delta,result) \
{                                               \
    UnsignedByte tmpHalfCarry = (               \
        (((result) & 0b00001000) >> 1)          \
        | (((delta) & 0b00001000) >> 2)          \
        | (((orig) & 0b00001000) >> 3)          \
    );                                          \
                                                \
    Z80_FLAG_H_UPDATE(tmpHalfCarry == 2 || tmpHalfCarry == 4 || tmpHalfCarry == 6 || tmpHalfCarry == 7);\
}

/* update P/V flag for 8-bit overflow during addition */
#define Z80_FLAG_P_UPDATE_OVERFLOW_ADD(orig,delta,result) \
{                                                        \
    UnsignedByte tmpOverflow = (                         \
        (((result) & 0b10000000) >> 5)                   \
        | (((delta) & 0b10000000) >> 6)                   \
        | (((orig) & 0b10000000) >> 7)                   \
    );                                                   \
                                                         \
    Z80_FLAG_P_UPDATE(tmpOverflow == 3 || tmpOverflow == 4);\
}

/* update P/V flag for 8-bit overflow during subtraction */
#define Z80_FLAG_P_UPDATE_OVERFLOW_SUB(orig,delta,result) \
{                                                        \
    UnsignedByte tmpOverflow = (                         \
        (((result) & 0b10000000) >> 5)                   \
        | (((delta) & 0b10000000) >> 6)                   \
        | (((orig) & 0b10000000) >> 7)                   \
    );                                                   \
                                                         \
    Z80_FLAG_P_UPDATE(tmpOverflow == 1 || tmpOverflow == 6);\
}

/* used in instruction execution methods to force the PC NOT to be updated with
 * the size of the instruction in execute() in cases where the instruction
 * directly changes the PC - e.g. JP, JR, DJNZ, RET, CALL etc. */
#define Z80_DONT_UPDATE_PC if (doPc) *doPc = false;
#define Z80_USE_JUMP_CYCLE_COST useJumpCycleCost = true;
#define Z80_INVALID_INSTRUCTION if (cycles) *cycles = 0; if (size) *size = 0; return false;

/* macros to fetch opcode cycle costs. most non-jump opcodes
	don't actually need to use these. for conditional jump opcodes,
	the cost if the jump is taken is stored in the rightmost 16
	bits; the cost if the jump is not taken is stored in the
	leftmost 16 bits */
#define Z80_CYCLES_JUMP(cycles) (((cycles) & 0xffff0000) >> 16)
#define Z80_CYCLES_NOJUMP(cycles) ((cycles) & 0x0000ffff)

/*
 * macros implementing common instruction semantics using different combinations
 * of like operands
 *
 * TODO to check results and flags, make an interpreter based on the HOB java
 * code and use that as comparison base.
 */

/* data loading instructions
 *
 * FLAGS: no flags are modified, except when I (0xed 0x47) or R (oxed 0x4f) is
 * loaded from A, in which case C is preserved, H and N are reset, Z and S are
 * flipped(?) and P is set if interrupts are enabled. these cases are handled in
 * the code in the fetch-execute cycle.
 *
 * TODO: check result against known working implementation.
 */
#define Z80__LD__REG8__N(dest, n) ((dest) = (n))
#define Z80__LD__REG8__REG8(dest, src) ((dest) = (src))

/*
 * nn (the memory address to retrieve) MUST be in HOST byte order
 * TODO: check result against known working implementation.
 */
#define Z80__LD__REG8__INDIRECT_NN(dest, nn) ((dest) = peekUnsigned(nn))
#define Z80__LD__REG8__INDIRECT_REG16(dest, src) ((dest) = peekUnsigned((src)))

/*
 * nn MUST be in HOST byte order
 * TODO: check result against known working implementation.
 */
#define Z80__LD__REG16__NN(dest, nn) ((dest) = (nn))
#define Z80__LD__REG16__REG16(dest, src) ((dest) = (src))

/*
 * TODO: check result against known working implementation.
 */
#define Z80__LD__INDIRECT_REG16__REG8(dest, src) pokeUnsigned((dest), (src))

/*
 * nn (the memory address to load) MUST be in HOST byte order
 * TODO: check result against known working implementation.
 */
#define Z80__LD__INDIRECT_NN__REG16(nn, src) { auto __tmpAddr = (nn); pokeHostWord(__tmpAddr, (src)); m_registers.memptr = hostToZ80ByteOrder(__tmpAddr); }

/*
 * nn (the memory address to retrieve) MUST be in HOST byte order
 */
#define Z80__LD__REG16__INDIRECT_NN(dest, nn) ((dest) = z80ToHostByteOrder(peekUnsignedWord(nn)))
#define Z80__LD__INDIRECT_REG16__N(dest, n) (pokeUnsigned((dest), n))

/*
 * nn (the memory address to load) MUST be in HOST byte order
 * TODO: check result against known working implementation.
 */
#define Z80__LD__INDIRECT_NN__REG8(nn, src) (pokeUnsigned(nn, (src)))
#define Z80__LD__INDIRECT_REG16_D__N(reg, d, n) Z80__LD__INDIRECT_REG16__N(((reg) + (d)), (n));
#define Z80__LD__INDIRECT_REG16_D__REG8(reg, d, src) Z80__LD__INDIRECT_NN__REG8((reg) + (d), (src));
#define Z80__LD__REG8__INDIRECT_REG16_D(dest, reg, d) Z80__LD__REG8__INDIRECT_NN((dest), (reg) + (d));

/* reset instructions
 *
 * addr can be 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30 or 0x38
 *
 * FLAGS: no flags are modified.
 * TODO: check result against known working implementation.
 */
#define Z80__RST__N(addr) \
Z80__PUSH__REG16(m_registers.pc + 1); \
m_registers.pc = (addr); \
m_registers.memptr = m_registers.pc

/* context switching instructions
 *
 * FLAGS: no flags are modified.
 * TODO: check result against known working implementation.
 */
#define Z80__EX__REG16__REG16(src, dest) {Z80::Z80::UnsignedWord __mytmp = (dest); (dest) = (src); (src) = __mytmp; }
#define Z80__EX__INDIRECT_REG16__REG16(src, dest) {\
    UnsignedWord tmpWord = peekUnsignedWord((src));\
    pokeHostWord((src), (dest));                   \
    (dest) = tmpWord;                              \
}

/* stack instructions
 *
 * FLAGS: no flags are modified.
 * TODO: check result against known working implementation.
 */
#define Z80__POP__REG16(reg) {(reg) = peekUnsignedWord(m_registers.sp); m_registers.sp += 2;}
#define Z80__PUSH__REG16(reg) {m_registers.sp -= 2; pokeHostWord(m_registers.sp, (reg));}

/* addition instructions
 *
 * FLAGS (8-bit): N is cleared, P is overflow, others by definition.
 * FLAGS (16-bit): S, Z and P preserved, H indicates carry from bit11 to bit12,
 * N is cleared, C by definition.
 *
 * NOTE: flags checked against known working implementation.
 * TODO: check result against known working implementation.
 */
#define Z80__ADD__REG8__N(dest,n) { UnsignedWord res = Z80_CUBYTE(dest) + Z80_CUBYTE(n); Z80_FLAG_P_UPDATE((((dest) ^ ~(n)) & ((dest) ^ Z80_CUBYTE(res)) & 0x80) != 0); Z80_FLAG_N_CLEAR; Z80_FLAG_Z_UPDATE(0 == (res & 0x00ff)); Z80_FLAG_S_UPDATE((res) & 0x0080); Z80_FLAG_C_UPDATE(res & 0x0100); Z80_FLAG_H_UPDATE(((dest) ^ (n) ^ Z80_CUBYTE(res)) & 0x10); Z80_FLAG_F3_UPDATE(Z80_CUBYTE(res) & 0x20); Z80_FLAG_F5_UPDATE(Z80_CUBYTE(res) & 0x08); (dest) = Z80_CUBYTE(res); Z80_FLAG_F3_UPDATE(m_registers.a & Z80_FLAG_F3_MASK); Z80_FLAG_F5_UPDATE(m_registers.a & Z80_FLAG_F5_MASK); }
#define Z80__ADD__REG8__REG8(dest,src) Z80__ADD__REG8__N(dest,src)
#define Z80__ADD__REG8__INDIRECT_REG16(dest,src) Z80__ADD__REG8__N(dest,(*(m_ram + (src))))
#define Z80__ADD__REG16__REG16(dest,src) { unsigned long res = (dest) + (src); Z80_FLAG_H_UPDATE((Z80_CUWORD(res) ^ (dest) ^ (src)) & 0x1000); Z80_FLAG_N_CLEAR; Z80_FLAG_C_UPDATE(res & 0x10000); (dest) = Z80_CUWORD(res); Z80_FLAG_F5_UPDATE((dest) & 0x2000); Z80_FLAG_F3_UPDATE((dest) & 0x0800); }
#define Z80__ADD__REG8__INDIRECT_REG16_D(dest, reg, d) Z80__ADD__REG8__N((dest),(*(m_ram + (reg) + (d))))

/* addition with carry instructions
 * dest = dest + src + carry
 *
 * FLAGS (8-bit): P is overflow, N is cleared, others by definition.
 * FLAGS (16-bit): P is overflow, N is cleared, H indicates carry from bit10 to
 * bit11, others by definition.
 *
 * TODO: 16-bit flags
 * TODO: check flags against known working implementation.
 * TODO: check result against known working implementation.
 */
#define Z80__ADC__REG8__N(dest,n) {      \
    UnsignedByte tmpOldValue = (dest);   \
    UnsignedByte tmpAdd = (n);           \
    UnsignedWord tmpResult = (dest) + tmpAdd + (Z80_FLAG_C_ISSET ? 1 : 0);\
    (dest) = tmpResult & 0xff;           \
    Z80_FLAG_C_UPDATE(tmpResult & 0x100);\
    Z80_FLAG_N_CLEAR;                    \
    Z80_FLAG_P_UPDATE_OVERFLOW_ADD(tmpOldValue, tmpAdd, (dest));\
    Z80_FLAG_H_UPDATE_ADD(tmpOldValue, tmpAdd, (dest));\
    Z80_FLAGS_S53_UPDATE((dest));        \
    Z80_FLAG_Z_UPDATE(0 == (dest));      \
}
#define Z80__ADC__REG8__REG8(dest,src) Z80__ADC__REG8__N((dest), (src));
#define Z80__ADC__REG8__INDIRECT_REG16(dest,src) Z80__ADC__REG8__N((dest), peekUnsigned(src))
#define Z80__ADC__REG16__REG16(dest,src) { UnsignedWord oldValue = (dest); (dest) += (src) + (Z80_FLAG_C_ISSET ? 1 : 0); Z80_FLAG_N_CLEAR; Z80_FLAG_Z_UPDATE(0 == (dest)); Z80_FLAG_S_UPDATE((dest) & 0x8000); Z80_FLAG_P_UPDATE((dest) < oldValue); Z80_FLAG_C_UPDATE((dest) < oldValue); Z80_FLAG_H_UPDATE(Z80_CHECK_16BIT_HALFCARRY_10_TO_11(oldValue,(dest))); }
#define Z80__ADC__REG8__INDRIECT_REG16_D(dest, reg, d) { UnsignedByte oldValue = (dest); UnsignedWord addr = (reg) + (d); (dest) += (peekUnsigned(addr)) + (Z80_FLAG_C_ISSET ? 1 : 0); Z80_FLAG_N_CLEAR; Z80_FLAG_Z_UPDATE(0 == (dest)); Z80_FLAG_S_UPDATE((dest) & 0x80); Z80_FLAG_P_UPDATE((dest) < oldValue); Z80_FLAG_C_UPDATE((dest) < oldValue); Z80_FLAG_H_UPDATE(Z80_CHECK_8BIT_HALFCARRY(oldValue,(dest))); }
#define Z80__ADC__REG8__INDIRECT_REG16_D(dest, reg, d) Z80__ADC__REG8__N((dest), peekUnsigned((reg) + (d)))

/* subtraction instructions
 *
 * FLAGS: N is set, P is overflow, others by definition
 * TODO: check flags against known working implementation.
 * TODO: check result against known working implementation.
 */
#define Z80__SUB__N(n) { \
    UnsignedByte tmpOldValue = m_registers.a;   \
    UnsignedByte tmpSub = (n);           \
    UnsignedWord tmpResult = m_registers.a - tmpSub; \
    m_registers.a = tmpResult & 0xff;           \
    Z80_FLAG_C_UPDATE(tmpResult & 0x0100);                       \
    Z80_FLAG_N_SET;      \
    Z80_FLAG_P_UPDATE_OVERFLOW_SUB(tmpOldValue, tmpSub, m_registers.a); \
    Z80_FLAG_H_UPDATE_SUB(tmpOldValue, tmpSub, m_registers.a); \
    Z80_FLAGS_S53_UPDATE(m_registers.a);\
    Z80_FLAG_Z_UPDATE(0 == m_registers.a);      \
}
#define Z80__SUB__REG8(reg) Z80__SUB__N(reg)
#define Z80__SUB__INDIRECT_REG16(reg) { UnsignedByte v = peekUnsigned(reg); Z80__SUB__N(v) }
#define Z80__SUB__INDIRECT_REG16_D(reg,d) Z80__SUB__N(peekUnsigned((reg) + (d)))

/* subtraction with carry instructions
 * dest = dest - src - carry
 *
 * FLAGS (8-bit): P is overflow, N is set, others by definition.
 * FLAGS (16-bit): P is overflow, N is set, H indicates carry from bit10 to
 * bit11, others by definition.
 *
 * TODO: 16-bit flags
 * TODO: check flags against known working implementation.
 * TODO: check result against known working implementation.
 */
#define Z80__SBC__REG8__N(dest,n) { \
    UnsignedByte tmpOldValue = (dest);\
    UnsignedByte tmpSub = (n);            \
    UnsignedWord tmpResult = (dest) - tmpSub - (Z80_FLAG_C_ISSET ? 1 : 0);\
    (dest) = tmpResult & 0xff;   \
    Z80_FLAG_C_UPDATE(tmpResult & 0x0100);\
    Z80_FLAG_N_SET;                       \
    Z80_FLAG_P_UPDATE_OVERFLOW_SUB(tmpOldValue, tmpSub, (dest));\
    Z80_FLAG_H_UPDATE_SUB(tmpOldValue, tmpSub, (dest));\
    Z80_FLAGS_S53_UPDATE((dest));  \
    Z80_FLAG_Z_UPDATE(0 == (dest));\
}
#define Z80__SBC__REG8__REG8(dest,src) Z80__SBC__REG8__N((dest), (src))
#define Z80__SBC__REG8__INDIRECT_REG16(dest,src) Z80__SBC__REG8__N((dest),peekUnsigned(src))

#define Z80__SBC__REG16__REG16(dest, src) { \
    UnsignedWord oldValue = (dest);         \
    (dest) -= ((src) + (Z80_FLAG_C_ISSET ? 1 : 0)); \
    Z80_FLAG_N_SET;                         \
    Z80_FLAG_Z_UPDATE(0 == (dest));         \
    Z80_FLAG_S_DEFAULTBEHAVIOUR16;          \
    Z80_FLAG_P_UPDATE((dest) > oldValue);   \
    Z80_FLAG_C_UPDATE((dest) > oldValue);   \
    Z80_FLAG_H_UPDATE(Z80_CHECK_16BIT_HALFCARRY_10_TO_11(oldValue, (dest))); \
}

#define Z80__SBC__REG8__INDIRECT_REG16_D(dest,reg,d) Z80__SBC__REG8__N((dest), peekUnsigned((reg) + (d)))

/* increment instructions
 *
 * FLAGS (8-bit): C preserved, P is overflow, others by definition
 * FLAGS (16-bit): no flags are modified
 *
 * halfcarry flag update checks whether the incremented value has bits 3, 2, 1
 * and 0 set (i.e. all four least significant bits), and bit 7 cleared (the sign
 * bit) which are the only situations in which an increment of an 8-bit signed
 * twos-complement integer carries a bit over from bit3 to bit4.
 *
 * TODO: check flags against known working implementation.
 * TODO: check result against known working implementation.
 */
#define Z80__INC__REG8(reg) \
(reg)++;                    \
Z80_FLAG_H_UPDATE(0 == (0x0f & (reg))); \
Z80_FLAG_P_UPDATE(0x80 == (reg)); \
Z80_FLAG_N_CLEAR;           \
Z80_FLAG_Z_UPDATE(0 == (reg));   \
Z80_FLAG_S_UPDATE((reg) & 0x80); \
Z80_FLAG_F3_UPDATE((reg) & Z80_FLAG_F3_MASK); \
Z80_FLAG_F5_UPDATE((reg) & Z80_FLAG_F5_MASK);

#define Z80__INC__INDIRECT_REG16(reg) Z80__INC__REG8(*(m_ram + (reg))) /*{ UnsignedByte oldValue = peekUnsigned(reg); pokeUnsigned((reg), peekUnsigned(reg) + 1); Z80_FLAG_P_UPDATE((reg) < oldValue); Z80_FLAG_C_UPDATE((reg) < oldValue); Z80_FLAG_H_UPDATE(0x0f == (oldValue & 0x8f)); Z80_FLAG_N_CLEAR; Z80_FLAG_Z_UPDATE(0 == (reg)); Z80_FLAG_S_UPDATE((reg) & 0x80); } */
#define Z80__INC__REG16(reg) (reg)++;
#define Z80__INC__INDIRECT_REG16_D(reg, d) (*(m_ram + (reg) + (d)))++;/*{ UnsignedWord addr = (reg) + (d); pokeUnsigned(addr, peekUnsigned(addr) + 1); }*/

/* decrement instructions
 *
 * FLAGS (8-bit): C preserved, P is overflow, others by definition
 * FLAGS (16-bit): no flags are modified
 *
 * halfcarry flag update checks whether the decremented value has bits 7, 3, 2
 * and 1 set (i.e. all four least significant bits and the sign bit), which are
 * the only situations in which a decrement of an 8-bit signed twos-complement
 * integer carries a bit over from bit3 to bit4.
 */
#define Z80__DEC__REG8(reg) \
Z80_FLAG_H_UPDATE(0 == (0x0f & (reg))); \
(reg)--;                    \
Z80_FLAG_P_UPDATE(0x7f == (reg)); \
Z80_FLAG_N_SET;             \
Z80_FLAG_Z_UPDATE(0 == (reg));   \
Z80_FLAG_S_UPDATE((reg) & 0x80); \
Z80_FLAG_F3_UPDATE((reg) & Z80_FLAG_F3_MASK); \
Z80_FLAG_F5_UPDATE((reg) & Z80_FLAG_F5_MASK);

#define Z80__DEC__INDIRECT_REG16(reg) Z80__DEC__REG8(*(m_ram + (reg)))
#define Z80__DEC__REG16(reg) (reg)--;
#define Z80__DEC__INDIRECT_REG16_D(reg, d) (*(m_ram + (reg) + (d)))--; /*{ UnsignedWord addr = (reg) + (d); pokeUnsigned(addr, peekUnsigned(addr) - 1); }*/

/* negation instruction
 *
 * there is only one negation instruction, but it has several opcodes (most of
 * which are unofficial), so a macro is provided for a common implementation.
 *
 * FLAGS: P is overflow, N is set, others by definition
 *
 * the H and C flag updates work on the basis that only a value of -128 can
 * result in an overflow: 128 is the only value that can be represented -ve
 * but not +ve (i.e. the range of an 8-bit twos-complement integer is -128 to
 * 127).
 *
 * TODO: check flags against known working implementation.
 * TODO: check result against known working implementation.
 */
#define Z80_NEG { UnsignedByte oldValue = m_registers.a; m_registers.a = 0 - (m_registers.a); Z80_FLAG_S_UPDATE((m_registers.a) & 0x80); Z80_FLAG_Z_DEFAULTBEHAVIOUR; Z80_FLAG_H_UPDATE(Z80_CHECK_8BIT_HALFCARRY(oldValue, m_registers.a)); Z80_FLAG_C_UPDATE(oldValue == 0); Z80_FLAG_P_UPDATE(SignedByte(oldValue) == -128); Z80_FLAG_N_SET; }

/* compare instructions
 *
 * These instructions are mostly identical to SUB instruction, except that the result
 * is discarded rather than loaded into A and the handling of flags F5 and F3 differs.
 *
 * FLAGS: V is overflow, N is set, others by definition
 * NOTE Flags F3 and F5 are set according to the bits in the OPERAND (i.e. what A is being compared to) not A or the
 *  result of subtracting from A
 */
#define Z80__CP__N(n) \
{ \
    UnsignedByte tmpSub = (n);           \
    UnsignedWord tmpResult = m_registers.a - tmpSub; \
    Z80_FLAG_C_UPDATE(tmpResult & 0x0100);                       \
    Z80_FLAG_N_SET;      \
    Z80_FLAG_P_UPDATE_OVERFLOW_SUB(m_registers.a, tmpSub, tmpResult); \
    Z80_FLAG_H_UPDATE_SUB(m_registers.a, tmpSub, tmpResult); \
    Z80_FLAG_S_UPDATE(tmpResult & Z80_FLAG_S_MASK);\
    Z80_FLAG_F5_UPDATE(tmpSub & Z80_FLAG_F5_MASK);\
    Z80_FLAG_F3_UPDATE(tmpSub & Z80_FLAG_F3_MASK);\
    Z80_FLAG_Z_UPDATE(0 == tmpResult);      \
}
#define Z80__CP__REG8(reg) Z80__CP__N(reg)
#define Z80__CP__INDIRECT_REG16(reg) Z80__CP__N(peekUnsigned((reg)))
#define Z80__CP__INDIRECT_REG16_D(reg,d) Z80__CP__N(peekUnsigned((reg) + (d)))

/* bitwise operations
 *
 * FLAGS: C cleared, N cleared, P is parity, others by definition
 */
#define Z80_BITWISE_FLAGS Z80_FLAG_C_CLEAR; Z80_FLAG_N_CLEAR; Z80_FLAG_P_UPDATE(isEvenParity(m_registers.a)); Z80_FLAGS_S53_UPDATE(m_registers.a); Z80_FLAG_Z_DEFAULTBEHAVIOUR;
#define Z80__AND__N(n) m_registers.a &= (n); Z80_BITWISE_FLAGS; Z80_FLAG_H_SET;
#define Z80__AND__REG8(reg) Z80__AND__N((reg))
#define Z80__AND__INDIRECT_REG16(reg) Z80__AND__N(peekUnsigned(reg))
#define Z80__AND__INDIRECT_REG16_D(reg,d) Z80__AND__N(peekUnsigned((reg) + (d)))

#define Z80__OR__N(n) m_registers.a |= (n); Z80_BITWISE_FLAGS; Z80_FLAG_H_CLEAR;
#define Z80__OR__REG8(reg) Z80__OR__N((reg))
#define Z80__OR__INDIRECT_REG16(reg) Z80__OR__N(peekUnsigned(reg))
#define Z80__OR__INDIRECT_REG16_D(reg,d) Z80__OR__N(peekUnsigned((reg) + (d)))

#define Z80__XOR__N(n) m_registers.a ^= (n); Z80_BITWISE_FLAGS; Z80_FLAG_H_CLEAR;
#define Z80__XOR__REG8(reg) Z80__XOR__N((reg))
#define Z80__XOR__INDIRECT_REG16(reg) Z80__XOR__N((peekUnsigned(reg)))
#define Z80__XOR__INDIRECT_REG16_D(reg,d) Z80__XOR__N(peekUnsigned((reg) + (d)))

/* FLAGS: all preserved */
#define Z80__SET__N__REG8(n,reg) (reg) |= (1 << (n))
#define Z80__SET__N__INDIRECT_REG16(n,reg) pokeUnsigned((reg), peekUnsigned((reg)) | (1 << (n)))
#define Z80__SET__N__INDIRECT_REG16_D(n,reg,d) Z80__SET__N__INDIRECT_REG16(n,(reg) + (d))
#define Z80__SET__N__INDIRECT_REG16_D__REG8(n,reg16,d,reg8) Z80__SET__N__INDIRECT_REG16(n, (reg16) + (d)); /* anything to do with (reg8)? */

/*
 * FLAGS: all preserved
 * TODO: check result against known working implementation.
 */
#define Z80__RES__N__REG8(n,reg) (reg) &= ~(1 << (n))
#define Z80__RES__N__INDIRECT_REG16(n,reg) pokeUnsigned((reg), peekUnsigned((reg)) & ~(1 << (n)))
#define Z80__RES__N__INDIRECT_REG16_D(n,reg,d) Z80__RES__N__INDIRECT_REG16(n,(reg) + (d))
#define Z80__RES__N__INDIRECT_REG16_D__REG8(n,reg16,d,reg8) Z80__RES__N__INDIRECT_REG16(n,(reg16) + (d)); /* anything to do with (reg8)? */

/* bit shift and rotation instructions */
/* rotate left with carry instructons
 *
 * FLAGS: H cleared, N cleared, P is parity, S as defined, Z as defined, C is
 * manipulated by instruction
 *
 * TODO: check flags against known working implementation.
 * TODO: check result against known working implementation.
 */
#define Z80__RLC__REG8(reg) { bool bit = (reg) & 0x80; (reg) <<= 1; if (bit) { (reg) |= 0x01; Z80_FLAG_C_SET; } else { (reg) &= 0xfe; Z80_FLAG_C_CLEAR; } Z80_FLAG_H_CLEAR;  Z80_FLAG_N_CLEAR; Z80_FLAG_P_UPDATE(isEvenParity(reg)); Z80_FLAG_S_DEFAULTBEHAVIOUR; Z80_FLAG_Z_DEFAULTBEHAVIOUR; }
#define Z80__RLC__INDIRECT_REG16(reg) { UnsignedByte v = peekUnsigned(reg); bool bit = v & 0x80; v <<= 1; if (bit) { v |= 0x01; Z80_FLAG_C_SET; } else { v &= 0xfe; Z80_FLAG_C_CLEAR; }; pokeUnsigned((reg), v); Z80_FLAG_H_CLEAR; Z80_FLAG_N_CLEAR; Z80_FLAG_P_UPDATE(isEvenParity(v)); Z80_FLAG_S_DEFAULTBEHAVIOUR; Z80_FLAG_Z_DEFAULTBEHAVIOUR; }
#define Z80__RLC__INDIRECT_REG16_D(reg,d) Z80__RLC__INDIRECT_REG16((reg) + (d))
#define Z80__RLC__INDIRECT_REG16_D__REG8(reg16,d,reg8) Z80__RLC__INDIRECT_REG16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

/* rotate right with carry instructons
 *
 * FLAGS: H cleared, N cleared, P is parity, S as defined, Z as defined,
 * C is manipulated by instruction
 *
 * TODO: check flags against known working implementation.
 * TODO: check result against known working implementation.
 */
#define Z80__RRC__REG8(reg) { bool bit = (reg) & 0x01; (reg) >>= 1; if (bit) { (reg) |= 0x80; Z80_FLAG_C_SET; } else { (reg) &= 0x7f; Z80_FLAG_C_CLEAR; } Z80_FLAG_H_CLEAR;  Z80_FLAG_N_CLEAR; Z80_FLAG_P_UPDATE(isEvenParity(reg)); Z80_FLAG_S_DEFAULTBEHAVIOUR; Z80_FLAG_Z_DEFAULTBEHAVIOUR; }
#define Z80__RRC__INDIRECT_REG16(reg) { UnsignedByte v = peekUnsigned(reg); bool bit = v & 0x01; v >>= 1; if (bit) { v |= 0x80; Z80_FLAG_C_SET; } else { v &= 0x7f; Z80_FLAG_C_CLEAR; }; pokeUnsigned((reg), v); Z80_FLAG_H_CLEAR; Z80_FLAG_N_CLEAR; Z80_FLAG_P_UPDATE(isEvenParity(v)); Z80_FLAG_S_DEFAULTBEHAVIOUR; Z80_FLAG_Z_DEFAULTBEHAVIOUR; }
#define Z80__RRC__INDIRECT_REG16_D(reg,d) Z80__RRC__INDIRECT_REG16((reg) + (d))
#define Z80__RRC__INDIRECT_REG16_D__REG8(reg16,d,reg8) Z80__RRC__INDIRECT_REG16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

/* rotate left instruction
 *
 * value is rotated left. bit 7 moves into carry flag and carry flag
 * moves into bit 0. In other words, it's as if the value was 9 bits in
 * size with the carry flag as bit 8.
 *
 * FLAGS: H and N cleared, P is parity, C modified directly by
 * instruction, S and Z as defined
 * TODO: check flags against known working implementation.
 * TODO: check result against known working implementation.
 */
 #define Z80__RL__REG8(reg) {\
	bool bit = (reg) & 0x80;\
	(reg) <<= 1; \
\
	/* copy carry flag into bit 1 */\
	if (Z80_FLAG_C_ISSET) (reg) |= 0x01;\
	else (reg) &= 0xfe;\
\
	/* copy old bit 7 into carry flag */\
	Z80_FLAG_C_UPDATE(bit);\
\
	Z80_FLAG_H_CLEAR;\
	Z80_FLAG_N_CLEAR;\
	Z80_FLAG_P_UPDATE(isEvenParity(reg));\
	Z80_FLAG_S_UPDATE((reg) & 0x80);\
	Z80_FLAG_Z_UPDATE(0 == (reg));\
}

/*
 * re-use RL instruction for 8-bit reg to do the actual work
 * TODO: check result against known working implementation.
 */
#define Z80__RL__INDIRECT_REG16(reg) { UnsignedByte v = peekUnsigned(reg); Z80__RL__REG8(v); pokeUnsigned(reg, v); }
#define Z80__RL__INDIRECT_REG16_D(reg,d) Z80__RL__INDIRECT_REG16((reg) + (d))
#define Z80__RL__INDIRECT_REG16_D__REG8(reg16,d,reg8) Z80__RL__INDIRECT_REG16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

/* rotate right instruction
 *
 * value is rotated right. bit 0 moves into carry flag and carry flag
 * moves into bit 7. In other words, it's as if the value was 9 bits in
 * size with the carry flag as bit 0.
 *
 * FLAGS: H and N cleared, P is parity, C modified directly by
 * instruction, S and Z as defined
 *
 * TODO: check flags against known working implementation.
 * TODO: check result against known working implementation.
 */
 #define Z80__RR__REG8(reg) \
{\
	bool bit = (reg) & 0x01;\
	(reg) >>= 1;            \
                            \
	/* copy carry flag into bit 7 */\
	if (Z80_FLAG_C_ISSET) { \
        (reg) |= 0x80;      \
    } else {                \
        (reg) &= 0x7f;      \
    }\
\
	/* copy old bit 0 into carry flag */\
	Z80_FLAG_C_UPDATE(bit);\
\
	Z80_FLAG_H_CLEAR;\
	Z80_FLAG_N_CLEAR;\
	Z80_FLAG_P_UPDATE(isEvenParity(reg));\
	Z80_FLAG_S_UPDATE((reg) & 0x80);\
	Z80_FLAG_Z_UPDATE(0 == (reg));\
}

/*
 * re-use RR instruction for 8-bit reg to do the actual work
 * TODO: check result against known working implementation.
 */
#define Z80__RR__INDIRECT_REG16(reg) { UnsignedByte v = peekUnsigned(reg); Z80__RR__REG8(v); pokeUnsigned((reg), v); }
#define Z80__RR__INDIRECT_REG16_D(reg,d) Z80__RR__INDIRECT_REG16((reg) + (d))
#define Z80__RR__INDIRECT_REG16_D__REG8(reg16,d,reg8) Z80__RR__INDIRECT_REG16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

/* arithmetic left shift instruction
 *
 * FLAGS: H and N are cleared, P is parity, others as defined.
 * TODO: check flags against known working implementation.
 * TODO: check result against known working implementation.
 */
#define Z80__SLA__REG8(reg) {\
	Z80_FLAG_C_UPDATE((reg) & 0x80);\
\
	(reg) <<= 1;\
\
	Z80_FLAG_H_CLEAR;\
	Z80_FLAG_N_CLEAR;\
	Z80_FLAG_P_UPDATE(isEvenParity(reg));\
	Z80_FLAG_S_UPDATE((reg) & 0x80);\
	Z80_FLAG_Z_UPDATE(0 == (reg));\
}

/*
 * re-use SLA instruction for 8-bit reg to do the actual work
 * TODO: check result against known working implementation.
 */
#define Z80__SLA__INDIRECT_REG16(reg) { UnsignedByte v = peekUnsigned(reg); Z80__SLA__REG8(v); pokeUnsigned((reg), v); }
#define Z80__SLA__INDIRECT_REG16_D(reg,d) Z80__SLA__INDIRECT_REG16((reg) + (d))
#define Z80__SLA__INDIRECT_REG16_D__REG8(reg16,d,reg8) Z80__SLA__INDIRECT_REG16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

/* arithmetic right shift instruction
 *
 * FLAGS: H and N are cleared, P is parity, others as defined.
 * TODO: check flags against known working implementation.
 * TODO: check result against known working implementation.
 */
#define Z80__SRA__REG8(reg) {\
	Z80_FLAG_C_UPDATE((reg) & 0x01);\
\
	(reg) >>= 1;\
\
	Z80_FLAG_H_CLEAR;\
	Z80_FLAG_N_CLEAR;\
	Z80_FLAG_P_UPDATE(isEvenParity(reg));\
	Z80_FLAG_S_UPDATE((reg) & 0x80);\
	Z80_FLAG_Z_UPDATE(0 == (reg));\
}

/*
 * re-use SRA instruction for 8-bit reg to do the actual work
 * TODO: check result against known working implementation.
 */
#define Z80__SRA__INDIRECT_REG16(reg) { UnsignedByte v = peekUnsigned(reg); Z80__SRA__REG8(v); pokeUnsigned((reg), v); }
#define Z80__SRA__INDIRECT_REG16_D(reg,d) Z80__SRA__INDIRECT_REG16((reg) + (d))
#define Z80__SRA__INDIRECT_REG16_D__REG8(reg16,d,reg8) Z80__SRA__INDIRECT_REG16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

/* logical left shift instruction
 *
 * bits are left shifted one place. bit 7 goes into carry flag and 1 goes into
 * bit 0.
 *
 * FLAGS: H and N are cleared, P is parity, others as defined.
 * TODO: check flags against known working implementation.
 * TODO: check result against known working implementation.
 */
#define Z80__SLL__REG8(reg) {\
	Z80_FLAG_C_UPDATE((reg) & 0x80);\
\
	(reg) <<= 1;\
	(reg) |= 0x01;\
\
	Z80_FLAG_H_CLEAR;\
	Z80_FLAG_N_CLEAR;\
	Z80_FLAG_P_UPDATE(isEvenParity(reg));\
	Z80_FLAG_S_UPDATE((reg) & 0x80);\
	Z80_FLAG_Z_UPDATE(0 == (reg));\
}

/* reuse Z80__SLL__REG8() to do the actual work
 * TODO: check result against known working implementation.
 */
#define Z80__SLL__INDIRECT_REG16(reg) { UnsignedByte v = peekUnsigned(reg); Z80__SLL__REG8(v); pokeUnsigned((reg), v); }
#define Z80__SLL__INDIRECT_REG16_D(reg,d) Z80__SLL__INDIRECT_REG16((reg) + (d))
#define Z80__SLL__INDIRECT_REG16_D__REG8(reg16,d,reg8) Z80__SLL__INDIRECT_REG16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

/* logical right shift instruction
 *
 * bits are right shifted one place. bit 0 goes into carry flag and 1 goes into
 * bit 7.
 *
 * FLAGS: H and N are cleared, P is parity, others as defined.
 * TODO: check flags against known working implementation.
 * TODO: check result against known working implementation.
 */
#define Z80__SRL__REG8(reg) {\
	Z80_FLAG_C_UPDATE((reg) & 0x01);\
\
	(reg) >>= 1;\
	(reg) |= 0x80;\
\
	Z80_FLAG_H_CLEAR;\
	Z80_FLAG_N_CLEAR;\
	Z80_FLAG_P_UPDATE(isEvenParity(reg));\
	Z80_FLAG_S_UPDATE((reg) & 0x80);\
	Z80_FLAG_Z_UPDATE(0 == (reg));\
}

/* reuse Z80__SRL__REG8 to do the actual work */
#define Z80__SRL__INDIRECT_REG16(reg) { UnsignedByte v = peekUnsigned(reg); Z80__SRL__REG8(v); pokeUnsigned((reg), v); }
#define Z80__SRL__INDIRECT_REG16_D(reg,d) Z80__SRL__INDIRECT_REG16((reg) + (d))
#define Z80__SRL__INDIRECT_REG16_D__REG8(reg16,d,reg8) Z80__SRL__INDIRECT_REG16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

/* bit testing instructions
 *
 * FLAGS: C is preserved, N is cleared, P is unknown, H is set, Z is as defined, S is unknown
 * TODO: check flags against known working implementation.
 * TODO: check result against known working implementation.
 */
#define Z80__BIT__N__REG8(n,reg) Z80_FLAG_Z_UPDATE(0 != ((reg) & (1 << n))); Z80_FLAG_P_UPDATE(Z80_FLAG_Z_ISSET); Z80_FLAG_N_CLEAR; Z80_FLAG_H_SET; Z80_FLAG_S_UPDATE((n) != 7 ? false : !Z80_FLAG_Z_ISSET);
#define Z80__BIT__N__INDIRECT_REG16(n,reg) Z80__BIT__N__REG8(n,peekUnsigned(reg));
#define Z80__BIT__N__INDIRECT_REG16_D(n,reg,d) Z80__BIT__N__INDIRECT_REG16(n,(reg) + (d))
#define Z80__BIT__N__INDIRECT_REG16_D__REG8(n,reg16,d,reg8) Z80__BIT__N__INDIRECT_REG16(n,(reg16) + (d)); /* anything to do with (reg8)? */

/* nmi handler return instruction
 *
 * there is only one nmi return instruction, but it has several opcodes (most of
 * which are unofficial), so a macro is provided for a common implementation.
 *
 * FLAGS: all preserved.
 * TODO: check result against known working implementation.
 */
#define Z80__RETN Z80__POP__REG16(m_registers.pc); m_iff1 = m_iff2;

/* interrupt handler return instruction
 *
 * there is only one interrupt return instruction, but it has several opcodes
 * (most of which are unofficial), so a macro is provided for a common
 * implementation.
 *
 * FLAGS: all preserved.
 * TODO: check result against known working implementation.
 */
#define Z80__RETI Z80__POP__REG16(m_registers.pc); /* TODO signal IO device that interrupt has finished */

/* TODO: */
//OUT (REG8),REG8
//OUT (n),REG8
//IN REG8,(REG8)
//IN REG8,(n)

namespace
{
    constexpr const int DefaultClockSpeed = 3500000;
}

constexpr const unsigned int Z80::Z80::DdOrFdOpcodeSize[256] = {
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 4, 4, 2, 2, 2, 3, 2, 2, 2, 4, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 3, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
    3, 3, 3, 3, 3, 3, 2, 3, 2, 2, 2, 2, 2, 2, 3, 2,

    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2	/* 0xcd */, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};


Z80::Z80::Z80(unsigned char * mem, int memSize)
: Cpu(mem, memSize),
	// the 16-bit registers are all 16-bit scalar variables. I and R are 8-bit and are 8-bit scalar variables
//	m_af(0),
//  m_bc(0),
//  m_de(0),
//  m_hl(0),
//  m_sp(0),
//  m_pc(0),
//  m_ix(0),
//  m_iy(0),
//  m_i(0),
//  m_r(0),
//  m_afshadow(0),
//  m_bcshadow(0),
//  m_deshadow(0),
//  m_hlshadow(0),
//	// the 8-bit registers are all pointers to the appropriate 8 bits of the 16-bit register - these are initialised,
//	// taking into account host endianness, in init()
//	m_a(nullptr),
//  m_f(nullptr),
//  m_b(nullptr),
//  m_c(nullptr),
//  m_d(nullptr),
//  m_e(nullptr),
//  m_h(nullptr),
//  m_l(nullptr),
//  m_ixh(nullptr),
//  m_ixl(nullptr),
//  m_iyh(nullptr),
//  m_iyl(nullptr),
//  m_ashadow(nullptr),
//  m_fshadow(nullptr),
//  m_bshadow(nullptr),
//  m_cshadow(nullptr),
//  m_dshadow(nullptr),
//  m_eshadow(nullptr),
//  m_hshadow(nullptr),
//  m_lshadow(nullptr),
  m_ram(mem),
  m_ramSize(memSize),
  m_clockSpeed(DefaultClockSpeed),
  m_nmiPending(false),
  m_interruptRequested(false),
  m_iff1(false),
  m_iff2(false),
  m_interruptMode(0),
  m_ioDeviceConnections(nullptr),
  m_ioDeviceCount(0),
  m_ioDeviceCapacity(0),
  m_interruptMode0Instruction{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
{
	init();
}

Z80::Z80::~Z80() = default;

bool Z80::Z80::isEvenParity(Z80::Z80::UnsignedByte v)
{
	static bool parityLut[256] = {
#include "evenparitytable8.inc"
	};

	return parityLut[v];

//	int c = 0;
//	UnsignedByte mask = 1;

//	/* count the 1s in the binary representation */
//	for(int i = 0; i < 8; ++i) {
//		if (v & mask) c++;
//		mask <<= 1;
//	}

//	/* return true if there's an even number of 1s */
//	return !(c % 2);
}


bool Z80::Z80::isEvenParity(Z80::Z80::UnsignedWord v)
{
	static bool parityLut[65536] = {
#include "evenparitytable16.inc"
	};

	return parityLut[v];

//	int c = 0;
//	UnsignedByte mask = 1;

//	/* count the 1s in the binary representation */
//	for(int i = 0; i < 16; ++i) {
//		if (v & mask) c++;
//		mask <<= 1;
//	}

//	/* return true if there's an even number of 1s */
//	return !(c % 2);
}


void Z80::Z80::init()
{
//	if (HostByteOrder == Z80ByteOrder) {
//        m_a = (Z80::Z80::UnsignedByte *) &m_af;
//        m_f = m_a + 1;
//        m_b = (Z80::Z80::UnsignedByte *) &m_bc;
//        m_c = m_b + 1;
//        m_d = (Z80::Z80::UnsignedByte *) &m_de;
//        m_e = m_d + 1;
//        m_h = (Z80::Z80::UnsignedByte *) &m_hl;
//        m_l = m_h + 1;
//        m_ixh = (Z80::Z80::UnsignedByte *) &m_ix;
//        m_ixl = m_ixl + 1;
//        m_iyh = (Z80::Z80::UnsignedByte *) &m_iy;
//        m_iyl = m_iyl + 1;
//        m_ashadow = (Z80::Z80::UnsignedByte *) &m_afshadow;
//        m_fshadow = m_ashadow + 1;
//        m_bshadow = (Z80::Z80::UnsignedByte *) &m_bcshadow;
//        m_cshadow = m_bshadow + 1;
//        m_dshadow = (Z80::Z80::UnsignedByte *) &m_deshadow;
//        m_eshadow = m_dshadow + 1;
//        m_hshadow = (Z80::Z80::UnsignedByte *) &m_hlshadow;
//        m_lshadow = m_hshadow + 1;
//	} else {
//        m_f = (Z80::Z80::UnsignedByte *) &m_af;
//        m_a = m_f + 1;
//        m_c = (Z80::Z80::UnsignedByte *) &m_bc;
//        m_b = m_c + 1;
//        m_e = (Z80::Z80::UnsignedByte *) &m_de;
//        m_d = m_e + 1;
//        m_l = (Z80::Z80::UnsignedByte *) &m_hl;
//        m_h = m_l + 1;
//        m_ixl = (Z80::Z80::UnsignedByte *) &m_ix;
//        m_ixh = m_ixl + 1;
//        m_iyl = (Z80::Z80::UnsignedByte *) &m_iy;
//        m_iyh = m_iyl + 1;
//        m_fshadow = (Z80::Z80::UnsignedByte *) &m_afshadow;
//        m_ashadow = m_fshadow + 1;
//        m_cshadow = (Z80::Z80::UnsignedByte *) &m_bcshadow;
//        m_bshadow = m_cshadow + 1;
//        m_eshadow = (Z80::Z80::UnsignedByte *) &m_deshadow;
//        m_dshadow = m_eshadow + 1;
//        m_lshadow = (Z80::Z80::UnsignedByte *) &m_hlshadow;
//        m_hshadow = m_lshadow + 1;
//	}

	/* set IO up device connections */
	m_ioDeviceCount = 0;
	m_ioDeviceCapacity = 100;
	m_ioDeviceConnections = (IODeviceConnection **) malloc(sizeof(IODeviceConnection *) * m_ioDeviceCapacity);
	*m_interruptMode0Instruction = 0;

	/* set up opcode cycle costs */
#include "z80_plain_opcode_cycles.inc"
#include "z80_cb_opcode_cycles.inc"
#include "z80_ed_opcode_cycles.inc"
//#include "z80_dd_opcode_cycles.inc"
//#include "z80_fd_opcode_cycles.inc"

	/* set up instruction byte sizes */
#include "z80_plain_opcode_sizes.inc"
	/* all 0xcb opcodes are 2 bytes in size */
#include "z80_ed_opcode_sizes.inc"
//#include "z80_dd_opcode_sizes.inc"
//#include "z80_fd_opcode_sizes.inc"
	/* all 0xdd 0xcb opcodes are 2 bytes in size */
	/* all 0xfd 0xcb opcodes are 2 bytes in size */
}


bool Z80::Z80::connectIODevice(Z80::Z80::UnsignedWord port, Z80IODevice * device)
{
	if (m_ioDeviceCount >= m_ioDeviceCapacity) {
		m_ioDeviceCapacity += 100;
		m_ioDeviceConnections = (IODeviceConnection **) std::realloc(m_ioDeviceConnections, sizeof(IODeviceConnection *) * m_ioDeviceCapacity);
	}

	IODeviceConnection * myDevice = m_ioDeviceConnections[m_ioDeviceCount++] = new IODeviceConnection();
	myDevice->device = device;
	myDevice->port = port;
	device->setCpu(this);
	return true;
}

void Z80::Z80::disconnectIODevice(Z80::Z80::UnsignedWord port, Z80IODevice * device) {
	for(int i = 0; i < m_ioDeviceCount; ++i) {
		if (m_ioDeviceConnections[i]->device == device && m_ioDeviceConnections[i]->port == port) {
			m_ioDeviceConnections[i]->device->setCpu(0);
			delete m_ioDeviceConnections[i];
			--m_ioDeviceCount;

			for(int j = i; j < m_ioDeviceCount; ++j)
				m_ioDeviceConnections[j] = m_ioDeviceConnections[j + 1];
		}
	}
}

void Z80::Z80::setInterruptMode0Instruction(Z80::Z80::UnsignedByte * instructions, int bytes) {
	for(int i = 0; i < bytes; ++i) {
        m_interruptMode0Instruction[i] = instructions[i];
    }
}

void Z80::Z80::interrupt() {
	m_interruptRequested = true;
}

void Z80::Z80::nmi()
{
	m_nmiPending = true;
}

void Z80::Z80::reset()
{
	m_iff1 = m_iff2 = false;
	m_nmiPending = false;
	m_registers.reset();
}

/**
 * Store a 16-bit value in the Z80's memory.
 *
 * The value is provided in HOST byte order and converted to Z80 byte order if necessary. The address to write is
 * provided in HOST byte order and has no conversion applied - it's just an offset into the Z80 memory. The memory
 * locations actually written are addr and addr + 1. For example, if you poke the value 65280 (0xff00) into address
 * 20000 (0x4e20) the result in the Z80 memory will be:
 *
 * 0x4e20: 0x00
 * 0x4e21: 0xff
 *
 * @param addr
 * @param value
 */
void Z80::Z80::pokeHostWord(int addr, Z80::Z80::UnsignedWord value)
{
	if (0 > addr || m_ramSize <= addr) {
	    return;
	}

	value = hostToZ80ByteOrder(value);
	auto * bytes = reinterpret_cast<UnsignedByte *>(&value);
	*(m_ram + addr) = bytes[0];
	*(m_ram + addr + 1) = bytes[1];
}

/**
 * Store a 16-bit value in the Z80's memory.
 *
 * The value is provided in Z80 byte order - it is the caller's responsibility to ensure that the value has already been
 * converted to Z80 byte order if this differs from the host byte order. The address to write is provided in HOST byte
 * order and has no conversion applied - it's just an offset into the Z80 memory. The memory locations actually written
 * are addr and addr + 1. For example, if you poke the value 65280 (0xff00) in Z80 byte order (0x00 0xff) into address
 * 20000 (0x4e20) the result in the Z80 memory will be:
 *
 * 0x4e20: 0x00
 * 0x4e21: 0xff
 *
 * @param addr
 * @param value
 */
void Z80::Z80::pokeZ80Word(int addr, UnsignedWord value)
{
	if (0 > addr || m_ramSize <= addr) {
	    return;
	}

    auto * bytes = reinterpret_cast<UnsignedByte *>(&value);
    *(m_ram + addr) = bytes[0];
    *(m_ram + addr + 1) = bytes[1];
}

bool Z80::Z80::execute(const Z80::Z80::UnsignedByte * instruction, bool doPc, int * cycles, int * size)
{
	static int dummySize;

	assert(instruction);
	bool ret;

	if (!size) {
	    size = &dummySize;
	}

	switch (*instruction) {
		case Z80__PLAIN__PREFIX__CB:
			/* no 0xcb instructions modify PC directly so this method never needs
			 * to override this request */
			ret = executeCbInstruction(instruction + 1, cycles, size);

            if (ret) {
                ++m_registers.r;
            }
			break;

		case Z80__PLAIN__PREFIX__ED:
			ret = executeEdInstruction(instruction + 1, &doPc, cycles, size);

            if (ret) {
                ++m_registers.r;
            }
			break;

		case Z80__PLAIN__PREFIX__DD:
			ret = executeDdOrFdInstruction(m_registers.ix, instruction + 1, &doPc, cycles, size);

            if (ret) {
                ++m_registers.r;
            }
			break;

		case Z80__PLAIN__PREFIX__FD:
			ret = executeDdOrFdInstruction(m_registers.iy, instruction + 1, &doPc, cycles, size);

            if (ret) {
                ++m_registers.r;
            }
			break;

		default:
			ret = executePlainInstruction(instruction, &doPc, cycles, size);
			break;
	}

	/* doPc is altered by the instruction execution method to be false if a jump
	 * was taken or the PC was otherwise directly affected by the instruction */
	if (ret) {
	    ++m_registers.r;

	    if (doPc) {
            m_registers.pc += *size;
        }
	}

	return ret;
}


int Z80::Z80::fetchExecuteCycle()
{
	int cycles = 0;
	int size = 0;

	if (m_nmiPending) {
		/* do the interrupt */
		m_iff2 = m_iff1;
		m_iff1 = false;
		Z80__PUSH__REG16(m_registers.pcZ80);
		m_registers.pc = 0x0066;
		m_nmiPending = false;
	}

	if (m_iff1 && m_interruptRequested) {
		/* process maskable interrupts */
		switch(m_interruptMode) {
			case 0:
				m_iff1 = m_iff2 = false;
				// TODO if the instruction is a call or RST, push PC onto stack
				if (false/* is_call_or_rst */) {
				    Z80__PUSH__REG16(m_registers.pcZ80);
				}

				/* execute the instruction */
				execute(m_interruptMode0Instruction, false);
				// clear the instruction cache - actually just turns it into a NOP
				*m_interruptMode0Instruction = 0;
				break;

			case 1:
				m_iff1 = m_iff2 = false;
				Z80__PUSH__REG16(m_registers.pcZ80);
                m_registers.pc = 0x0038;
				break;

			case 2:
				break;
		}

		m_interruptRequested = false;
	}

	execute(m_ram + m_registers.pc, true, &cycles, &size);
	return cycles;
}


bool Z80::Z80::executePlainInstruction(const Z80::Z80::UnsignedByte * instruction, bool * doPc, int * cycles, int * size)
{
	bool useJumpCycleCost = false;

	switch(*instruction) {
		case Z80__PLAIN__NOP:							// 0x00
			/* nothing to do, just consume some cycles */
			break;

		case Z80__PLAIN__LD__BC__NN:					// 0x01
			Z80__LD__REG16__NN(m_registers.bcZ80, *((UnsignedWord *)(instruction + 1)));
			break;

		case Z80__PLAIN__LD__INDIRECT_BC__A:		// 0x02
			Z80__LD__INDIRECT_REG16__REG8(m_registers.bc, m_registers.a);
			break;

		case Z80__PLAIN__INC__BC:						// 0x03
			Z80__INC__REG16(m_registers.bc);
			break;

		case Z80__PLAIN__INC__B:						// 0x04
			Z80__INC__REG8(m_registers.b);
			break;

		case Z80__PLAIN__DEC__B:						// 0x05
			Z80__DEC__REG8(m_registers.b);
			break;

		case Z80__PLAIN__LD__B__N:					// 0x06
			Z80__LD__REG8__N(m_registers.b, *(instruction + 1));
			break;

		case Z80__PLAIN__RLCA:						// 0x07
			/* FLAGS: S, Z and P preserved, H and N cleared, C modified directly by
				instruction */
			{
				bool bit = (m_registers.a) & 0x80;
				(m_registers.a) <<= 1;

				if (bit) {
					(m_registers.a) |= 0x01;
					Z80_FLAG_C_SET;
				}
				else {
					(m_registers.a) &= 0xfe;
					Z80_FLAG_C_CLEAR;
				}

				Z80_FLAG_H_CLEAR;
				Z80_FLAG_N_CLEAR;
			}
			break;

		case Z80__PLAIN__EX__AF__AF_SHADOW:			// 0x08
			Z80__EX__REG16__REG16(m_registers.af, m_registers.afShadow);
			break;

		case Z80__PLAIN__ADD__HL__BC:				// 0x09
			Z80__ADD__REG16__REG16(m_registers.hl, m_registers.bc);
			break;

		case Z80__PLAIN__LD__A__INDIRECT_BC:		// 0x0a
			Z80__LD__REG8__INDIRECT_REG16(m_registers.a, m_registers.bc);
			break;

		case Z80__PLAIN__DEC__BC:						// 0x0b
			Z80__DEC__REG16(m_registers.bc);
			break;

		case Z80__PLAIN__INC__C:						// 0x0c
			Z80__INC__REG8(m_registers.c);
			break;

		case Z80__PLAIN__DEC__C:						// 0x0d
			Z80__DEC__REG8(m_registers.c);
			break;

		case Z80__PLAIN__LD__C__N:					// 0x0e
			Z80__LD__REG8__N(m_registers.c, *(instruction + 1));
			break;

		case Z80__PLAIN__RRCA:						// 0x0f
			/* FLAGS: S, Z and P preserved, H and N cleared, C modified directly by
				instruction */
			{
				bool bit = (m_registers.a) & 0x01;
				(m_registers.a) >>= 1;

				if (bit) {
					(m_registers.a) |= 0x80;
					Z80_FLAG_C_SET;
				}
				else {
					(m_registers.a) &= 0x7f;
					Z80_FLAG_C_CLEAR;
				}

				Z80_FLAG_H_CLEAR;
				Z80_FLAG_N_CLEAR;
				Z80_FLAG_F3_UPDATE((m_registers.a & Z80_FLAG_F3_MASK));
				Z80_FLAG_F5_UPDATE((m_registers.a & Z80_FLAG_F5_MASK));
			}
			break;

		case Z80__PLAIN__DJNZ__d:						// 0x10
			if (0 != --(m_registers.b)) {
				m_registers.pc += (SignedByte)(*(instruction + 1));
				m_registers.memptr = m_registers.pc;
				Z80_USE_JUMP_CYCLE_COST;
			}

			break;

		case Z80__PLAIN__LD__DE__NN:				// 0x11
			Z80__LD__REG16__NN(m_registers.de, *((UnsignedWord *)(instruction + 1)));
			break;

		case Z80__PLAIN__LD__INDIRECT_DE__A:		// 0x12
			Z80__LD__INDIRECT_REG16__REG8(m_registers.de, m_registers.a);
			break;

		case Z80__PLAIN__INC__DE:					// 0x13
			Z80__INC__REG16(m_registers.de);
			break;

		case Z80__PLAIN__INC__D:						// 0x14
			Z80__INC__REG8(m_registers.d);
			break;

		case Z80__PLAIN__DEC__D:						// 0x15
			Z80__DEC__REG8(m_registers.d);
			break;

		case Z80__PLAIN__LD__D__N:					// 0x16
			Z80__LD__REG8__N(m_registers.d, *(instruction + 1));
			break;

		case Z80__PLAIN__RLA:							// 0x17
			{
				/* re-use RL instruction, but cache all flags except C (which is
				 * modified by the instruction) and re-edit as they are different
				 * for RL and RLA
				 *
				 * FLAGS: S, Z and P preserved, C modified directly by instruction,
				 * H and N cleared */
				UnsignedByte flags = (m_registers.f & ~Z80_FLAG_C_MASK);
				Z80__RL__REG8(m_registers.a);
				m_registers.f = flags | (m_registers.f & Z80_FLAG_C_MASK);
				Z80_FLAG_H_CLEAR;
				Z80_FLAG_N_CLEAR;
			}
			break;

		case Z80__PLAIN__JR__d:						// 0x18
			m_registers.pc += SignedByte(*(instruction + 1));
//			Z80_DONT_UPDATE_PC;
			Z80_USE_JUMP_CYCLE_COST;
			break;

		case Z80__PLAIN__ADD__HL__DE:				// 0x19
			Z80__ADD__REG16__REG16(m_registers.hl, m_registers.de);
			break;

		case Z80__PLAIN__LD__A__INDIRECT_DE:		// 0x1a
			Z80__LD__REG8__INDIRECT_REG16(m_registers.a, m_registers.de);
			break;

		case Z80__PLAIN__DEC__DE:					// 0x1b
			Z80__DEC__REG16(m_registers.de);
			break;

		case Z80__PLAIN__INC__E:						// 0x1c
			Z80__INC__REG8(m_registers.e);
			break;

		case Z80__PLAIN__DEC__E:						// 0x1d
			Z80__DEC__REG8(m_registers.e);
			break;

		case Z80__PLAIN__LD__E__N:					// 0x1e
			Z80__LD__REG8__N(m_registers.e, *(instruction + 1));
			break;

		case Z80__PLAIN__RRA:							// 0x1f
			{
				/* re-use RR instruction, but cache all flags except C (which is
				 * modified by the instruction) and re-edit as they are different
				 * for RL and RLA
				 *
				 * FLAGS: S, Z and P preserved, C modified directly by instruction,
				 * H and N cleared */
				UnsignedByte flags = (m_registers.f & ~Z80_FLAG_C_MASK);
				Z80__RR__REG8(m_registers.a);
				m_registers.f = flags | (m_registers.f & Z80_FLAG_C_MASK);
				Z80_FLAG_H_CLEAR;
				Z80_FLAG_N_CLEAR;
			}
			break;

		case Z80__PLAIN__JR__NZ__d:					// 0x20
			if (!Z80_FLAG_Z_ISSET) {
				m_registers.pc += static_cast<SignedByte>(*(instruction + 1));
//				Z80_DONT_UPDATE_PC;
				Z80_USE_JUMP_CYCLE_COST;
			}
			break;

		case Z80__PLAIN__LD__HL__NN:				// 0x21
			Z80__LD__REG16__NN(m_registers.hl, *((UnsignedWord *)(instruction + 1)));
			break;

		case Z80__PLAIN__LD__INDIRECT_NN__HL:	// 0x22
			Z80__LD__INDIRECT_NN__REG16(z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))), m_registers.hl);
			break;

		case Z80__PLAIN__INC__HL:					// 0x23
			Z80__INC__REG16(m_registers.hl);
			break;

		case Z80__PLAIN__INC__H:						// 0x24
			Z80__INC__REG8(m_registers.h);
			break;

		case Z80__PLAIN__DEC__H:						// 0x25
			Z80__DEC__REG8(m_registers.h);
			break;

		case Z80__PLAIN__LD__H__N:					// 0x26
			Z80__LD__REG8__N(m_registers.h, *(instruction + 1));
			break;

		case Z80__PLAIN__DAA:							// 0x27
			/* decimal accumulator adjust instruction.
			 *
			 * makes sure the A register contains a valid BCD value after a BCD arithmetic operation.
			 *
			 * FLAGS: N is preserved, P is parity, C is set as above, others as
			 * defined.
			 */
            {
                UnsignedByte correction = 0;
                bool carry = Z80_FLAG_C_ISSET;

                // if the BCD calculation carried from the LS 4 bits or the LS BCD digit carries (i.e. > 9), correct
                // the LS digit
                if (Z80_FLAG_H_ISSET || ((m_registers.a & 0x0f) > 9)) {
                    correction = 0x06;
                }

                // if the BCD calculation carried from the MS 4 bits or the MS BCD digit carries (i.e. > 9), correct
                // the LS digit and ensure the carry flag is set
                if (carry || m_registers.a > 0x99) {
                    correction |= 0x60;
                    carry = true;
                }

                // if the BCD calculation was a subtraction, correct by subtracting; otherwise correct by adding
                // NOTE we rely on the ADD/SUB instruction to set the flags, so we must use it even if there is no
                //  correction
                if( Z80_FLAG_N_ISSET ) {
                    Z80__SUB__N(correction);
                } else {
                    Z80__ADD__REG8__N(m_registers.a, correction);
                }

                // finally, set the flags that weren't take care of in the ADD/SUB instruction
                Z80_FLAG_C_UPDATE(carry);
                Z80_FLAG_P_UPDATE(isEvenParity(m_registers.a));
			}
			break;

		case Z80__PLAIN__JR__Z__d:					// 0x28
			if (Z80_FLAG_Z_ISSET) {
				m_registers.pc += static_cast<SignedByte>(*(instruction + 1));
//				Z80_DONT_UPDATE_PC;
				Z80_USE_JUMP_CYCLE_COST;
			}
			break;

		case Z80__PLAIN__ADD__HL__HL:				// 0x29
			Z80__ADD__REG16__REG16(m_registers.hl, m_registers.hl);
			break;

		case Z80__PLAIN__LD__HL__INDIRECT_NN:	// 0x2a
		    // NOTE the interface of the Z80 class expects addresses in host byte order
			Z80__LD__REG16__INDIRECT_NN(m_registers.hl, z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))));
			break;

		case Z80__PLAIN__DEC__HL:					// 0x2b
			Z80__DEC__REG16(m_registers.hl);
			break;

		case Z80__PLAIN__INC__L:						// 0x2c
			Z80__INC__REG8(m_registers.l);
			break;

		case Z80__PLAIN__DEC__L:						// 0x2d
			Z80__DEC__REG8(m_registers.l);
			break;

		case Z80__PLAIN__LD__L__N:					// 0x2e
			Z80__LD__REG8__N(m_registers.l, *(instruction + 1));
			break;

		case Z80__PLAIN__CPL:							// 0x2f
			/* complement A
			 *
			 * FLAGS: S, Z, P and C preserved, N and H set */
			(m_registers.a) = ~(m_registers.a);
			Z80_FLAG_N_SET;
			Z80_FLAG_H_SET;
            Z80_FLAG_F3_UPDATE(m_registers.a & Z80_FLAG_F3_MASK);
            Z80_FLAG_F5_UPDATE(m_registers.a & Z80_FLAG_F5_MASK);
			break;

		case Z80__PLAIN__JR__NC__d:					// 0x30
			if (!Z80_FLAG_C_ISSET) {
				m_registers.pc += static_cast<SignedByte>(*(instruction + 1));
//				Z80_DONT_UPDATE_PC;
				Z80_USE_JUMP_CYCLE_COST;
			}
			break;

		case Z80__PLAIN__LD__SP__NN:				// 0x31
			Z80__LD__REG16__NN(m_registers.sp, Z80::Z80::z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))));
			break;

		case Z80__PLAIN__LD__INDIRECT_NN__A:		// 0x32
			Z80__LD__INDIRECT_NN__REG8(Z80::Z80::z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))), m_registers.a);
			break;

		case Z80__PLAIN__INC__SP:					// 0x33
			Z80__INC__REG16(m_registers.sp);
			break;

		case Z80__PLAIN__INC__INDIRECT_HL:		// 0x34
			Z80__INC__INDIRECT_REG16(m_registers.hl);
			break;

		case Z80__PLAIN__DEC__INDIRECT_HL:		// 0x35
			Z80__DEC__INDIRECT_REG16(m_registers.hl);
			break;

		case Z80__PLAIN__LD__INDIRECT_HL__N:		// 0x36
			Z80__LD__INDIRECT_REG16__N(m_registers.hl, *(instruction + 1));
			break;

		case Z80__PLAIN__SCF:							// 0x37
			Z80_FLAG_H_CLEAR;
			Z80_FLAG_N_CLEAR;
			Z80_FLAG_F3_UPDATE(m_registers.a & Z80_FLAG_F3_MASK);
			Z80_FLAG_F5_UPDATE(m_registers.a & Z80_FLAG_F5_MASK);
			Z80_FLAG_C_SET;
			break;

		case Z80__PLAIN__JR__C__d:					// 0x38
			if (Z80_FLAG_C_ISSET) {
				m_registers.pc += static_cast<SignedByte>(*(instruction + 1));
				Z80_USE_JUMP_CYCLE_COST;
			}
			break;

		case Z80__PLAIN__ADD__HL__SP:				// 0x39
			Z80__ADD__REG16__REG16(m_registers.hl, m_registers.sp);
			break;

		case Z80__PLAIN__LD__A__INDIRECT_NN:		// 0x3a
			Z80__LD__REG8__INDIRECT_NN(m_registers.a, Z80::Z80::z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))));
			break;

		case Z80__PLAIN__DEC__SP:					// 0x3b
			Z80__DEC__REG16(m_registers.sp);
			break;

		case Z80__PLAIN__INC__A:						// 0x3c
			Z80__INC__REG8(m_registers.a);
			break;

		case Z80__PLAIN__DEC__A:						// 0x3d
			Z80__DEC__REG8(m_registers.a);
			break;

		case Z80__PLAIN__LD__A__N:					// 0x3e
			Z80__LD__REG8__N(m_registers.a, *(instruction + 1));
			break;

		case Z80__PLAIN__CCF:							// 0x3f
			Z80_FLAG_H_UPDATE(Z80_FLAG_C_ISSET);
			Z80_FLAG_C_UPDATE(!Z80_FLAG_C_ISSET);
			Z80_FLAG_N_CLEAR;
            Z80_FLAG_F3_UPDATE(m_registers.a & Z80_FLAG_F3_MASK);
            Z80_FLAG_F5_UPDATE(m_registers.a & Z80_FLAG_F5_MASK);
			break;

		case Z80__PLAIN__LD__B__B:					// 0x40
//			Z80__LD__REG8__REG8(m_registers.b, m_registers.b);
			break;

		case Z80__PLAIN__LD__B__C:					// 0x41
			Z80__LD__REG8__REG8(m_registers.b, m_registers.c);
			break;

		case Z80__PLAIN__LD__B__D:					// 0x42
			Z80__LD__REG8__REG8(m_registers.b, m_registers.d);
			break;

		case Z80__PLAIN__LD__B__E:					// 0x43
			Z80__LD__REG8__REG8(m_registers.b, m_registers.e);
			break;

		case Z80__PLAIN__LD__B__H:					// 0x44
			Z80__LD__REG8__REG8(m_registers.b, m_registers.h);
			break;

		case Z80__PLAIN__LD__B__L:					// 0x45
			Z80__LD__REG8__REG8(m_registers.b, m_registers.l);
			break;

		case Z80__PLAIN__LD__B__INDIRECT_HL:		// 0x46
			Z80__LD__REG8__INDIRECT_REG16(m_registers.b, m_registers.hl);
			break;

		case Z80__PLAIN__LD__B__A:					// 0x47
			Z80__LD__REG8__REG8(m_registers.b, m_registers.a);
			break;

		case Z80__PLAIN__LD__C__B:					// 0x48
			Z80__LD__REG8__REG8(m_registers.c, m_registers.b);
			break;

		case Z80__PLAIN__LD__C__C:					// 0x49
//			Z80__LD__REG8__REG8(m_registers.c, m_registers.c);
			break;

		case Z80__PLAIN__LD__C__D:					// 0x4a
			Z80__LD__REG8__REG8(m_registers.c, m_registers.d);
			break;

		case Z80__PLAIN__LD__C__E:					// 0x4b
			Z80__LD__REG8__REG8(m_registers.c, m_registers.e);
			break;

		case Z80__PLAIN__LD__C__H:					// 0x4c
			Z80__LD__REG8__REG8(m_registers.c, m_registers.h);
			break;

		case Z80__PLAIN__LD__C__L:					// 0x4d
			Z80__LD__REG8__REG8(m_registers.c, m_registers.l);
			break;

		case Z80__PLAIN__LD__C__INDIRECT_HL:		// 0x4e
			Z80__LD__REG8__INDIRECT_REG16(m_registers.c, m_registers.hl);
			break;

		case Z80__PLAIN__LD__C__A:					// 0x4f
			Z80__LD__REG8__REG8(m_registers.c, m_registers.a);
			break;

		case Z80__PLAIN__LD__D__B:					// 0x50
			Z80__LD__REG8__REG8(m_registers.d, m_registers.b);
			break;

		case Z80__PLAIN__LD__D__C:					// 0x51
			Z80__LD__REG8__REG8(m_registers.d, m_registers.c);
			break;

		case Z80__PLAIN__LD__D__D:					// 0x52
//			Z80__LD__REG8__REG8(m_registers.d, m_registers.d);
			break;

		case Z80__PLAIN__LD__D__E:					// 0x53
			Z80__LD__REG8__REG8(m_registers.d, m_registers.e);
			break;

		case Z80__PLAIN__LD__D__H:					// 0x54
			Z80__LD__REG8__REG8(m_registers.d, m_registers.h);
			break;

		case Z80__PLAIN__LD__D__L:					// 0x55
			Z80__LD__REG8__REG8(m_registers.d, m_registers.l);
			break;

		case Z80__PLAIN__LD__D__INDIRECT_HL:		// 0x56
			Z80__LD__REG8__INDIRECT_REG16(m_registers.d, m_registers.hl);
			break;

		case Z80__PLAIN__LD__D__A:					// 0x57
			Z80__LD__REG8__REG8(m_registers.d, m_registers.a);
			break;

		case Z80__PLAIN__LD__E__B:					// 0x58
			Z80__LD__REG8__REG8(m_registers.e, m_registers.b);
			break;

		case Z80__PLAIN__LD__E__C:					// 0x59
			Z80__LD__REG8__REG8(m_registers.e, m_registers.c);
			break;

		case Z80__PLAIN__LD__E__D:					// 0x5a
			Z80__LD__REG8__REG8(m_registers.e, m_registers.d);
			break;

		case Z80__PLAIN__LD__E__E:					// 0x5b
//			Z80__LD__REG8__REG8(m_registers.e, m_registers.e);
			break;

		case Z80__PLAIN__LD__E__H:					// 0x5c
			Z80__LD__REG8__REG8(m_registers.e, m_registers.h);
			break;

		case Z80__PLAIN__LD__E__L:					// 0x5d
			Z80__LD__REG8__REG8(m_registers.e, m_registers.l);
			break;

		case Z80__PLAIN__LD__E__INDIRECT_HL:		// 0x5e
			Z80__LD__REG8__INDIRECT_REG16(m_registers.e, m_registers.hl);
			break;

		case Z80__PLAIN__LD__E__A:					// 0x5f
			Z80__LD__REG8__REG8(m_registers.e, m_registers.a);
			break;

		case Z80__PLAIN__LD__H__B:					// 0x60
			Z80__LD__REG8__REG8(m_registers.h, m_registers.b);
			break;

		case Z80__PLAIN__LD__H__C:					// 0x61
			Z80__LD__REG8__REG8(m_registers.h, m_registers.c);
			break;

		case Z80__PLAIN__LD__H__D:					// 0x62
			Z80__LD__REG8__REG8(m_registers.h, m_registers.d);
			break;

		case Z80__PLAIN__LD__H__E:					// 0x63
			Z80__LD__REG8__REG8(m_registers.h, m_registers.e);
			break;

		case Z80__PLAIN__LD__H__H:					// 0x64
//			Z80__LD__REG8__REG8(m_registers.h, m_registers.h);
			break;

		case Z80__PLAIN__LD__H__L:					// 0x65
			Z80__LD__REG8__REG8(m_registers.h, m_registers.l);
			break;

		case Z80__PLAIN__LD__H__INDIRECT_HL:		// 0x66
			Z80__LD__REG8__INDIRECT_REG16(m_registers.h, m_registers.hl);
			break;

		case Z80__PLAIN__LD__H__A:					// 0x67
			Z80__LD__REG8__REG8(m_registers.h, m_registers.a);
			break;

		case Z80__PLAIN__LD__L__B:					// 0x68
			Z80__LD__REG8__REG8(m_registers.l, m_registers.b);
			break;

		case Z80__PLAIN__LD__L__C:					// 0x69
			Z80__LD__REG8__REG8(m_registers.l, m_registers.c);
			break;

		case Z80__PLAIN__LD__L__D:					// 0x6a
			Z80__LD__REG8__REG8(m_registers.l, m_registers.d);
			break;

		case Z80__PLAIN__LD__L__E:					// 0x6b
			Z80__LD__REG8__REG8(m_registers.l, m_registers.e);
			break;

		case Z80__PLAIN__LD__L__H:					// 0x6c
			Z80__LD__REG8__REG8(m_registers.l, m_registers.h);
			break;

		case Z80__PLAIN__LD__L__L:					// 0x6d
//			Z80__LD__REG8__REG8(m_registers.l, m_registers.l);
			break;

		case Z80__PLAIN__LD__L__INDIRECT_HL:		// 0x6e
			Z80__LD__REG8__INDIRECT_REG16(m_registers.l, m_registers.hl);
			break;

		case Z80__PLAIN__LD__L__A:					// 0x6f
			Z80__LD__REG8__REG8(m_registers.l, m_registers.a);
			break;

		case Z80__PLAIN__LD__INDIRECT_HL__B:		// 0x70
			Z80__LD__INDIRECT_REG16__REG8(m_registers.hl, m_registers.b);
			break;

		case Z80__PLAIN__LD__INDIRECT_HL__C:		// 0x71
			Z80__LD__INDIRECT_REG16__REG8(m_registers.hl, m_registers.c);
			break;

		case Z80__PLAIN__LD__INDIRECT_HL__D:		// 0x72
			Z80__LD__INDIRECT_REG16__REG8(m_registers.hl, m_registers.d);
			break;

		case Z80__PLAIN__LD__INDIRECT_HL__E:		// 0x73
			Z80__LD__INDIRECT_REG16__REG8(m_registers.hl, m_registers.e);
			break;

		case Z80__PLAIN__LD__INDIRECT_HL__H:		// 0x74
			Z80__LD__INDIRECT_REG16__REG8(m_registers.hl, m_registers.h);
			break;

		case Z80__PLAIN__LD__INDIRECT_HL__L:		// 0x75
			Z80__LD__INDIRECT_REG16__REG8(m_registers.hl, m_registers.l);
			break;

		case Z80__PLAIN__HALT:						// 0x76
			/* HALT doesn't actually halt the computer, it halts the CPU and waits
			 * for an interrupt. */
			/* TODO */
			break;

		case Z80__PLAIN__LD__INDIRECT_HL__A:		// 0x77
			Z80__LD__INDIRECT_REG16__REG8(m_registers.hl, m_registers.a);
			break;

		case Z80__PLAIN__LD__A__B:					// 0x78
			Z80__LD__REG8__REG8(m_registers.a, m_registers.b);
			break;

		case Z80__PLAIN__LD__A__C:					// 0x79
			Z80__LD__REG8__REG8(m_registers.a, m_registers.c);
			break;

		case Z80__PLAIN__LD__A__D:					// 0x7a
			Z80__LD__REG8__REG8(m_registers.a, m_registers.d);
			break;

		case Z80__PLAIN__LD__A__E:					// 0x7b
			Z80__LD__REG8__REG8(m_registers.a, m_registers.e);
			break;

		case Z80__PLAIN__LD__A__H:					// 0x7c
			Z80__LD__REG8__REG8(m_registers.a, m_registers.h);
			break;

		case Z80__PLAIN__LD__A__L:					// 0x7d
			Z80__LD__REG8__REG8(m_registers.a, m_registers.l);
			break;

		case Z80__PLAIN__LD__A__INDIRECT_HL:		// 0x7e
			Z80__LD__REG8__INDIRECT_REG16(m_registers.a, m_registers.hl);
			break;

		case Z80__PLAIN__LD__A__A:					// 0x7f
//			Z80__LD__REG8__REG8(m_registers.a, m_registers.a);
			break;

		case Z80__PLAIN__ADD__A__B:					// 0x80
			Z80__ADD__REG8__REG8(m_registers.a, m_registers.b);
			break;

		case Z80__PLAIN__ADD__A__C:					// 0x81
			Z80__ADD__REG8__REG8(m_registers.a, m_registers.c);
			break;

		case Z80__PLAIN__ADD__A__D:					// 0x82
			Z80__ADD__REG8__REG8(m_registers.a, m_registers.d);
			break;

		case Z80__PLAIN__ADD__A__E:					// 0x83
			Z80__ADD__REG8__REG8(m_registers.a, m_registers.e);
			break;

		case Z80__PLAIN__ADD__A__H:					// 0x84
			Z80__ADD__REG8__REG8(m_registers.a, m_registers.h);
			break;

		case Z80__PLAIN__ADD__A__L:					// 0x85
			Z80__ADD__REG8__REG8(m_registers.a, m_registers.l);
			break;

		case Z80__PLAIN__ADD__A__INDIRECT_HL:	// 0x86
			Z80__ADD__REG8__INDIRECT_REG16(m_registers.a, m_registers.hl);
			break;

		case Z80__PLAIN__ADD__A__A:					// 0x87
			Z80__ADD__REG8__REG8(m_registers.a, m_registers.a);
			break;

		case Z80__PLAIN__ADC__A__B:					// 0x88
			Z80__ADC__REG8__REG8(m_registers.a, m_registers.b);
			break;

		case Z80__PLAIN__ADC__A__C:					// 0x89
			Z80__ADC__REG8__REG8(m_registers.a, m_registers.c);
			break;

		case Z80__PLAIN__ADC__A__D:					// 0x8a
			Z80__ADC__REG8__REG8(m_registers.a, m_registers.d);
			break;

		case Z80__PLAIN__ADC__A__E:					// 0x8b
			Z80__ADC__REG8__REG8(m_registers.a, m_registers.e);
			break;

		case Z80__PLAIN__ADC__A__H:					// 0x8c
			Z80__ADC__REG8__REG8(m_registers.a, m_registers.h);
			break;

		case Z80__PLAIN__ADC__A__L:					// 0x8d
			Z80__ADC__REG8__REG8(m_registers.a, m_registers.l);
			break;

		case Z80__PLAIN__ADC__A__INDIRECT_HL:	// 0x8e
			Z80__ADC__REG8__INDIRECT_REG16(m_registers.a, m_registers.hl);
			break;

		case Z80__PLAIN__ADC__A__A:					// 0x8f
			Z80__ADC__REG8__REG8(m_registers.a, m_registers.a);
			break;

		case Z80__PLAIN__SUB__B:						// 0x90
			Z80__SUB__REG8(m_registers.b);
			break;

		case Z80__PLAIN__SUB__C:						// 0x91
			Z80__SUB__REG8(m_registers.c);
			break;

		case Z80__PLAIN__SUB__D:						// 0x92
			Z80__SUB__REG8(m_registers.d);
			break;

		case Z80__PLAIN__SUB__E:						// 0x93
			Z80__SUB__REG8(m_registers.e);
			break;

		case Z80__PLAIN__SUB__H:						// 0x94
			Z80__SUB__REG8(m_registers.h);
			break;

		case Z80__PLAIN__SUB__L:						// 0x95
			Z80__SUB__REG8(m_registers.l);
			break;

		case Z80__PLAIN__SUB__INDIRECT_HL:		// 0x96
			Z80__SUB__INDIRECT_REG16(m_registers.hl);
			break;

		case Z80__PLAIN__SUB__A:						// 0x97
			Z80__SUB__REG8(m_registers.a);
			break;

		case Z80__PLAIN__SBC__A__B:					// 0x98
			Z80__SBC__REG8__REG8(m_registers.a, m_registers.b);
			break;

		case Z80__PLAIN__SBC__A__C:					// 0x99
			Z80__SBC__REG8__REG8(m_registers.a, m_registers.c);
			break;

		case Z80__PLAIN__SBC__A__D:					// 0x9a
			Z80__SBC__REG8__REG8(m_registers.a, m_registers.d);
			break;

		case Z80__PLAIN__SBC__A__E:					// 0x9b
			Z80__SBC__REG8__REG8(m_registers.a, m_registers.e);
			break;

		case Z80__PLAIN__SBC__A__H:					// 0x9c
			Z80__SBC__REG8__REG8(m_registers.a, m_registers.h);
			break;

		case Z80__PLAIN__SBC__A__L:					// 0x9d
			Z80__SBC__REG8__REG8(m_registers.a, m_registers.l);
			break;

		case Z80__PLAIN__SBC__A__INDIRECT_HL:	// 0x9e
			Z80__SBC__REG8__INDIRECT_REG16(m_registers.a, m_registers.hl);
			break;

		case Z80__PLAIN__SBC__A__A:					// 0x9f
			Z80__SBC__REG8__REG8(m_registers.a, m_registers.a);
			break;

		case Z80__PLAIN__AND__B:						// 0xa0
			Z80__AND__REG8(m_registers.b);
			break;

		case Z80__PLAIN__AND__C:						// 0xa1
			Z80__AND__REG8(m_registers.c);
			break;

		case Z80__PLAIN__AND__D:						// 0xa2
			Z80__AND__REG8(m_registers.d);
			break;

		case Z80__PLAIN__AND__E:						// 0xa3
			Z80__AND__REG8(m_registers.e);
			break;

		case Z80__PLAIN__AND__H:						// 0xa4
			Z80__AND__REG8(m_registers.h);
			break;

		case Z80__PLAIN__AND__L:						// 0xa5
			Z80__AND__REG8(m_registers.l);
			break;

		case Z80__PLAIN__AND__INDIRECT_HL:		// 0xa6
			Z80__AND__INDIRECT_REG16(m_registers.hl);
			break;

		case Z80__PLAIN__AND__A:						// 0xa7
			Z80__AND__REG8(m_registers.a);
			break;

		case Z80__PLAIN__XOR__B:						// 0xa8
			Z80__XOR__REG8(m_registers.b);
			break;

		case Z80__PLAIN__XOR__C:						// 0xa9
			Z80__XOR__REG8(m_registers.c);
			break;

		case Z80__PLAIN__XOR__D:						// 0xaa
			Z80__XOR__REG8(m_registers.d);
			break;

		case Z80__PLAIN__XOR__E:						// 0xab
			Z80__XOR__REG8(m_registers.e);
			break;

		case Z80__PLAIN__XOR__H:						// 0xac
			Z80__XOR__REG8(m_registers.h);
			break;

		case Z80__PLAIN__XOR__L:						// 0xad
			Z80__XOR__REG8(m_registers.l);
			break;

		case Z80__PLAIN__XOR__INDIRECT_HL:		// 0xae
			Z80__XOR__INDIRECT_REG16(m_registers.hl);
			break;

		case Z80__PLAIN__XOR__A:						// 0xaf
			Z80__XOR__REG8(m_registers.a);
			break;

		case Z80__PLAIN__OR__B:						// 0xb0
			Z80__OR__REG8(m_registers.b);
			break;

		case Z80__PLAIN__OR__C:						// 0xb1
			Z80__OR__REG8(m_registers.c);
			break;

		case Z80__PLAIN__OR__D:						// 0xb2
			Z80__OR__REG8(m_registers.d);
			break;

		case Z80__PLAIN__OR__E:						// 0xb3
			Z80__OR__REG8(m_registers.e);
			break;

		case Z80__PLAIN__OR__H:						// 0xb4
			Z80__OR__REG8(m_registers.h);
			break;

		case Z80__PLAIN__OR__L:						// 0xb5
			Z80__OR__REG8(m_registers.l);
			break;

		case Z80__PLAIN__OR__INDIRECT_HL:			// 0xb6
			Z80__OR__INDIRECT_REG16(m_registers.hl);
			break;

		case Z80__PLAIN__OR__A:						// 0xb7
			Z80__OR__REG8(m_registers.a);
			break;

		case Z80__PLAIN__CP__B:						// 0xb8
			Z80__CP__REG8(m_registers.b);
			break;

		case Z80__PLAIN__CP__C:						// 0xb9
			Z80__CP__REG8(m_registers.c);
			break;

		case Z80__PLAIN__CP__D:						// 0xba
			Z80__CP__REG8(m_registers.d);
			break;

		case Z80__PLAIN__CP__E:						// 0xbb
			Z80__CP__REG8(m_registers.e);
			break;

		case Z80__PLAIN__CP__H:						// 0xbc
			Z80__CP__REG8(m_registers.h);
			break;

		case Z80__PLAIN__CP__L:						// 0xbd
			Z80__CP__REG8(m_registers.l);
			break;

		case Z80__PLAIN__CP__INDIRECT_HL:			// 0xbe
			Z80__CP__INDIRECT_REG16(m_registers.hl);
			break;

		case Z80__PLAIN__CP__A:						// 0xbf
			Z80__CP__REG8(m_registers.a);
			break;

		case Z80__PLAIN__RET__NZ:					// 0xc0
			if (!Z80_FLAG_Z_ISSET) {
				Z80__POP__REG16(m_registers.pc);
				Z80_USE_JUMP_CYCLE_COST;
				Z80_DONT_UPDATE_PC;
			}
			break;

		case Z80__PLAIN__POP__BC:					// 0xc1
			Z80__POP__REG16(m_registers.bc);
			break;

		case Z80__PLAIN__JP__NZ__NN:				// 0xc2
            m_registers.memptr = z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1)));

            // NOTE docs I've found say cost is 10 if jump taken, 1 if not; however Z80 test suite expects cost to
            //  always bew 10
            Z80_USE_JUMP_CYCLE_COST;

			if (!Z80_FLAG_Z_ISSET) {
				m_registers.pc = m_registers.memptr;
				Z80_DONT_UPDATE_PC;
			}
			break;

		case Z80__PLAIN__JP__NN:						// 0xc3
            m_registers.memptr = z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1)));
			m_registers.pc = m_registers.memptr;
			Z80_DONT_UPDATE_PC;
			// NOTE don't set the jumped indicator because there's no different cycle cost - the jump always takes
			//  place, so the base cost in cycles is all that's used
			break;

		case Z80__PLAIN__CALL__NZ__NN:				// 0xc4
            m_registers.memptr = z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1)));

			if (!Z80_FLAG_Z_ISSET) {
                Z80_USE_JUMP_CYCLE_COST;
				Z80__PUSH__REG16(m_registers.pc + 3);
				m_registers.pc = m_registers.memptr;
				Z80_DONT_UPDATE_PC;
			}
			break;

		case Z80__PLAIN__PUSH__BC:					// 0xc5
			Z80__PUSH__REG16(m_registers.bc);
			break;

		case Z80__PLAIN__ADD__A__N:					// 0xc6
			Z80__ADD__REG8__N(m_registers.a, *(instruction + 1));
			break;

		case Z80__PLAIN__RST__00:					// 0xc7
			/* restart at 0x0000 */
			Z80__RST__N(0x00);
			Z80_DONT_UPDATE_PC;
			break;

		case Z80__PLAIN__RET__Z:						// 0xc8
			if (Z80_FLAG_Z_ISSET) {
				Z80__POP__REG16(m_registers.pc);
				Z80_USE_JUMP_CYCLE_COST;
				Z80_DONT_UPDATE_PC;
			}
			break;

		case Z80__PLAIN__RET:							// 0xc9
			Z80__POP__REG16(m_registers.pc);
			Z80_DONT_UPDATE_PC;
            // NOTE don't set the jumped indicator because there's no different cycle cost - the jump always takes
            //  place, so the base cost in cycles is all that's used
			break;

		case Z80__PLAIN__JP__Z__NN:					// 0xca
            m_registers.memptr = z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1)));

            // NOTE docs I've found say cost is 10 if jump taken, 1 if not; however Z80 test suite expects cost to
            //  always bew 10
            Z80_USE_JUMP_CYCLE_COST;

            if (Z80_FLAG_Z_ISSET) {
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Z80__PLAIN__PREFIX__CB:				// 0xcb
			std::cerr << "executePlainInstruction() called with opcode 0xcb. such an opcode should be handled by executeCbInstruction()" << std::endl;
			break;

		case Z80__PLAIN__CALL__Z__NN:				// 0xcc
            m_registers.memptr = z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1)));

            if (Z80_FLAG_Z_ISSET) {
                Z80_USE_JUMP_CYCLE_COST;
                Z80__PUSH__REG16(m_registers.pc + 3);
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Z80__PLAIN__CALL__NN:					// 0xcd
            m_registers.memptr = z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1)));
			Z80__PUSH__REG16(m_registers.pc + 3);
			m_registers.pc = m_registers.memptr;
			Z80_DONT_UPDATE_PC;
			break;

		case Z80__PLAIN__ADC__A__N:					// 0xce
			Z80__ADC__REG8__N(m_registers.a, *(instruction + 1));
			break;

		case Z80__PLAIN__RST__08:					// 0xcf
			Z80__RST__N(0x08);
			Z80_DONT_UPDATE_PC;
			break;

		case Z80__PLAIN__RET__NC:					// 0xd0
			if (!Z80_FLAG_C_ISSET) {
				Z80__POP__REG16(m_registers.pc);
				Z80_USE_JUMP_CYCLE_COST;
				Z80_DONT_UPDATE_PC;
			}
			break;

		case Z80__PLAIN__POP__DE:					// 0xd1
			Z80__POP__REG16(m_registers.de);
			break;

		case Z80__PLAIN__JP__NC__NN:				// 0xd2
            m_registers.memptr = z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1)));

            // NOTE docs I've found say cost is 10 if jump taken, 1 if not; however Z80 test suite expects cost to
            //  always bew 10
            Z80_USE_JUMP_CYCLE_COST;

            if (!Z80_FLAG_C_ISSET) {
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Z80__PLAIN__OUT__INDIRECT_N__A:		// 0xd3
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__PLAIN__CALL__NC__NN:				// 0xd4
            m_registers.memptr = z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1)));

            if (!Z80_FLAG_C_ISSET) {
                Z80_USE_JUMP_CYCLE_COST;
                Z80__PUSH__REG16(m_registers.pc + 3);
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Z80__PLAIN__PUSH__DE:					// 0xd5
			Z80__PUSH__REG16(m_registers.de);
			break;

		case Z80__PLAIN__SUB__N:						// 0xd6
			Z80__SUB__N(*(instruction + 1));
			break;

		case Z80__PLAIN__RST__10:					// 0xd7
			Z80__RST__N(0x10);
			Z80_DONT_UPDATE_PC;
			break;

		case Z80__PLAIN__RET__C:						// 0xd8
			if (Z80_FLAG_C_ISSET) {
				Z80__POP__REG16(m_registers.pc);
				Z80_USE_JUMP_CYCLE_COST;
				Z80_DONT_UPDATE_PC;
			}
			break;

		case Z80__PLAIN__EXX:							// 0xd9
			Z80__EX__REG16__REG16(m_registers.bc, m_registers.bcShadow);
			Z80__EX__REG16__REG16(m_registers.de, m_registers.deShadow);
			Z80__EX__REG16__REG16(m_registers.hl, m_registers.hlShadow);
			break;

		case Z80__PLAIN__JP__C__NN:					// 0xda
            m_registers.memptr = z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1)));

            // NOTE docs I've found say cost is 10 if jump taken, 1 if not; however Z80 test suite expects cost to
            //  always bew 10
            Z80_USE_JUMP_CYCLE_COST;

            if (Z80_FLAG_C_ISSET) {
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Z80__PLAIN__IN__A__INDIRECT_N:		// 0xdb
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__PLAIN__CALL__C__NN:				// 0xdc
            m_registers.memptr = z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1)));

            if (Z80_FLAG_C_ISSET) {
                Z80_USE_JUMP_CYCLE_COST;
                Z80__PUSH__REG16(m_registers.pc + 3);
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Z80__PLAIN__PREFIX__DD:				// 0xdd
			std::cerr << "executePlainInstruction() called with opcode 0xdd. such an opcode should be handled by executeDdInstruction()" << std::endl;
			break;

		case Z80__PLAIN__SBC__A__N:					// 0xde
			Z80__SBC__REG8__N(m_registers.a, *(instruction + 1));
			break;

		case Z80__PLAIN__RST__18:					// 0xdf
			Z80__RST__N(0x18);
			Z80_DONT_UPDATE_PC;
			break;

		case Z80__PLAIN__RET__PO:					// 0xe0
			/* the operand PO stands for "parity odd" */
			if (!Z80_FLAG_P_ISSET) {
				Z80__POP__REG16(m_registers.pc);
				Z80_USE_JUMP_CYCLE_COST;
				Z80_DONT_UPDATE_PC;
			}
			break;

		case Z80__PLAIN__POP__HL:					// 0xe1
			Z80__POP__REG16(m_registers.hl);
			break;

		case Z80__PLAIN__JP__PO__NN:				// 0xe2
			// the operand PO stands for "parity odd"
            m_registers.memptr = z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1)));

            // NOTE docs I've found say cost is 10 if jump taken, 1 if not; however Z80 test suite expects cost to
            //  always bew 10
            Z80_USE_JUMP_CYCLE_COST;

            if (!Z80_FLAG_P_ISSET) {
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Z80__PLAIN__EX__INDIRECT_SP__HL:	// 0xe3
			Z80__EX__INDIRECT_REG16__REG16(m_registers.sp, m_registers.hl);
			break;

		case Z80__PLAIN__CALL__PO__NN:				// 0xe4
			// the operand PO stands for "parity odd"
            m_registers.memptr = z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1)));

            if (!Z80_FLAG_P_ISSET) {
                Z80_USE_JUMP_CYCLE_COST;
                Z80__PUSH__REG16(m_registers.pc + 3);
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Z80__PLAIN__PUSH__HL:					// 0xe5
			Z80__PUSH__REG16(m_registers.hl);
			break;

		case Z80__PLAIN__AND__N:						// 0xe6
			Z80__AND__N(*(instruction + 1));
			break;

		case Z80__PLAIN__RST__20:					// 0xe7
			Z80__RST__N(0x20);
			Z80_DONT_UPDATE_PC;
			break;

		case Z80__PLAIN__RET__PE:					// 0xe8
			/* the operand PE stands for "parity even" */
			if (Z80_FLAG_P_ISSET) {
				Z80__POP__REG16(m_registers.pc);
				Z80_USE_JUMP_CYCLE_COST;
				Z80_DONT_UPDATE_PC;
			}
			break;

		case Z80__PLAIN__JP__INDIRECT_HL:			// 0xe9
			m_registers.pc = m_registers.hl;
			Z80_DONT_UPDATE_PC;
            // NOTE don't set the jumped indicator because there's no different cycle cost - the jump always takes
            //  place, so the base cost in cycles is all that's used
			break;

		case Z80__PLAIN__JP__PE__NN:				// 0xea
            // the operand PO stands for "parity even"
            m_registers.memptr = z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1)));

            // NOTE docs I've found say cost is 10 if jump taken, 1 if not; however Z80 test suite expects cost to
            //  always bew 10
            Z80_USE_JUMP_CYCLE_COST;

            if (Z80_FLAG_P_ISSET) {
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Z80__PLAIN__EX__DE__HL:				// 0xeb
			Z80__EX__REG16__REG16(m_registers.de, m_registers.hl);
			break;

		case Z80__PLAIN__CALL__PE__NN:				// 0xec
			/* the operand PE stands for "parity even" */
            m_registers.memptr = z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1)));

            if (Z80_FLAG_P_ISSET) {
                Z80_USE_JUMP_CYCLE_COST;
                Z80__PUSH__REG16(m_registers.pc + 3);
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Z80__PLAIN__PREFIX__ED:				// 0xed
			/* should never happen */
			std::cerr << "executePlainInstruction() called with opcode 0xed. such an opcode should be handled by executeEdInstruction()" << std::endl;
			break;

		case Z80__PLAIN__XOR__N:						// 0xee
			Z80__XOR__N(*(instruction + 1));
			break;

		case Z80__PLAIN__RST__28:					// 0xef
			Z80__RST__N(0x28);
			Z80_DONT_UPDATE_PC;
			break;

		case Z80__PLAIN__RET__P:						// 0xf0
			/* the operand P means "plus" and therefore uses the sign flag; not
				to be confused with the parity flag */
			if (!Z80_FLAG_S_ISSET) {
				Z80__POP__REG16(m_registers.pc);
				Z80_USE_JUMP_CYCLE_COST;
				Z80_DONT_UPDATE_PC;
			}
			break;

		case Z80__PLAIN__POP__AF:					// 0xf1
			Z80__POP__REG16(m_registers.af);
			break;

		case Z80__PLAIN__JP__P__NN:					// 0xf2
			// the P operand in this instruction stands for "plus", not to be confused for the parity flag. it properly
            //  operates using the sign flag
            m_registers.memptr = z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1)));

            // NOTE docs I've found say cost is 10 if jump taken, 1 if not; however Z80 test suite expects cost to
            //  always bew 10
            Z80_USE_JUMP_CYCLE_COST;

            if (!Z80_FLAG_S_ISSET) {
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Z80__PLAIN__DI:							// 0xf3
			m_iff1 = m_iff2 = false;
			break;

		case Z80__PLAIN__CALL__P__NN:				// 0xf4
			// the operand P stands for "plus" and thus uses the sign flag, not to be confused with the parity flag
            m_registers.memptr = z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1)));

            if (!Z80_FLAG_S_ISSET) {
                Z80_USE_JUMP_CYCLE_COST;
                Z80__PUSH__REG16(m_registers.pc + 3);
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Z80__PLAIN__PUSH__AF:					// 0xf5
			Z80__PUSH__REG16(m_registers.af);
			break;

		case Z80__PLAIN__OR__N:						// 0xf6
			Z80__OR__N(*(instruction + 1));
			break;

		case Z80__PLAIN__RST__30:					// 0xf7
			Z80__RST__N(0x30);
			Z80_DONT_UPDATE_PC;
			break;

		case Z80__PLAIN__RET__M:						// 0xf8
			/* the operand M stands for "minus" and therefore uses the sign flag */
			if (Z80_FLAG_S_ISSET) {
				Z80__POP__REG16(m_registers.pc);
				Z80_USE_JUMP_CYCLE_COST;
				Z80_DONT_UPDATE_PC;
			}
			break;

		case Z80__PLAIN__LD__SP__HL:				// 0xf9
			Z80__LD__REG16__REG16(m_registers.sp, m_registers.hl);
			break;

		case Z80__PLAIN__JP__M__NN:					// 0xfa
			// the operand M stands for "minus" and therefore uses the sign flag
            m_registers.memptr = z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1)));

            // NOTE docs I've found say cost is 10 if jump taken, 1 if not; however Z80 test suite expects cost to
            //  always bew 10
            Z80_USE_JUMP_CYCLE_COST;

            if (Z80_FLAG_S_ISSET) {
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Z80__PLAIN__EI:							// 0xfb
			m_iff1 = m_iff2 = true;
			break;

		case Z80__PLAIN__CALL__M__NN:				// 0xfc
			// the operand M stands for "minus" and therefore uses the sign flag
            m_registers.memptr = z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1)));

            if (Z80_FLAG_S_ISSET) {
                Z80_USE_JUMP_CYCLE_COST;
                Z80__PUSH__REG16(m_registers.pc + 3);
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Z80__PLAIN__PREFIX__FD:				// 0xfd
			std::cerr << "executePlainInstruction() called with opcode 0xfd. such an opcode should be handled by executeFdInstruction()" << std::endl;
			break;

		case Z80__PLAIN__CP__N:						// 0xfe
			Z80__CP__N(*(instruction + 1));
			break;

		case Z80__PLAIN__RST__38:					// 0xff
			Z80__RST__N(0x38);
			Z80_DONT_UPDATE_PC;
			break;

		default:
			std::cerr << "unhandled opcode: 0x" << std::hex << (*instruction) << std::endl;
			Z80_INVALID_INSTRUCTION;
			break;
	}

	if (cycles) {
	    *cycles = static_cast<int>(useJumpCycleCost ? Z80_CYCLES_JUMP(m_plain_opcode_cycles[*instruction]) : Z80_CYCLES_NOJUMP(m_plain_opcode_cycles[*instruction]));
	}

	if (size) {
	    *size = static_cast<int>(m_plain_opcode_size[*instruction]);
	}

	return true;
}


// no 0xcb instructions directly modify the PC so we don't need to receive the (bool *) doPc parameter to indicate this
bool Z80::Z80::executeCbInstruction(const Z80::Z80::UnsignedByte * instruction, int * cycles, int * size)
{
	bool useJumpCycleCost = false;

	switch(*instruction) {
		case Z80__CB__RLC__B:		/* 0xcb 0x00 */
			Z80__RLC__REG8(m_registers.b);
			break;

		case Z80__CB__RLC__C:		/* 0xcb 0x01 */
			Z80__RLC__REG8(m_registers.c);
			break;

		case Z80__CB__RLC__D:		/* 0xcb 0x02 */
			Z80__RLC__REG8(m_registers.d);
			break;

		case Z80__CB__RLC__E:		/* 0xcb 0x03 */
			Z80__RLC__REG8(m_registers.e);
			break;

		case Z80__CB__RLC__H:		/* 0xcb 0x04 */
			Z80__RLC__REG8(m_registers.h);
			break;

		case Z80__CB__RLC__L:		/* 0xcb 0x05 */
			Z80__RLC__REG8(m_registers.l);
			break;

		case Z80__CB__RLC__INDIRECT_HL:		/* 0xcb 0x06 */
			Z80__RLC__INDIRECT_REG16(m_registers.hl);
			break;

		case Z80__CB__RLC__A:		/* 0xcb 0x07 */
			Z80__RLC__REG8(m_registers.a);
			break;

		case Z80__CB__RRC__B:		/* 0xcb 0x08 */
			Z80__RRC__REG8(m_registers.b);
			break;

		case Z80__CB__RRC__C:		/* 0xcb 0x09 */
			Z80__RRC__REG8(m_registers.c);
			break;

		case Z80__CB__RRC__D:		/* 0xcb 0x0a */
			Z80__RRC__REG8(m_registers.d);
			break;

		case Z80__CB__RRC__E:		/* 0xcb 0x0b */
			Z80__RRC__REG8(m_registers.e);
			break;

		case Z80__CB__RRC__H:		/* 0xcb 0x0c */
			Z80__RRC__REG8(m_registers.h);
			break;

		case Z80__CB__RRC__L:		/* 0xcb 0x0d */
			Z80__RRC__REG8(m_registers.l);
			break;

		case Z80__CB__RRC__INDIRECT_HL:		/* 0xcb 0x0e */
			Z80__RRC__INDIRECT_REG16(m_registers.hl);
			break;

		case Z80__CB__RRC__A:		/* 0xcb 0x0f */
			Z80__RRC__REG8(m_registers.a);
			break;

		case Z80__CB__RL__B:		/* 0xcb 0x10 */
			Z80__RL__REG8(m_registers.b);
			break;

		case Z80__CB__RL__C:		/* 0xcb 0x11 */
			Z80__RL__REG8(m_registers.c);
			break;

		case Z80__CB__RL__D:		/* 0xcb 0x12 */
			Z80__RL__REG8(m_registers.d);
			break;

		case Z80__CB__RL__E:		/* 0xcb 0x13 */
			Z80__RL__REG8(m_registers.e);
			break;

		case Z80__CB__RL__H:		/* 0xcb 0x14 */
			Z80__RL__REG8(m_registers.h);
			break;

		case Z80__CB__RL__L:		/* 0xcb 0x15 */
			Z80__RL__REG8(m_registers.l);
			break;

		case Z80__CB__RL__INDIRECT_HL:		/* 0xcb 0x16 */
			Z80__RL__INDIRECT_REG16(m_registers.hl);
			break;

		case Z80__CB__RL__A:		/* 0xcb 0x17 */
			Z80__RL__REG8(m_registers.a);
			break;

		case Z80__CB__RR__B:		/* 0xcb 0x18 */
			Z80__RR__REG8(m_registers.b);
			break;

		case Z80__CB__RR__C:		/* 0xcb 0x19 */
			Z80__RR__REG8(m_registers.c);
			break;

		case Z80__CB__RR__D:		/* 0xcb 0x1a */
			Z80__RR__REG8(m_registers.d);
			break;

		case Z80__CB__RR__E:		/* 0xcb 0x1b */
			Z80__RR__REG8(m_registers.e);
			break;

		case Z80__CB__RR__H:		/* 0xcb 0x1c */
			Z80__RR__REG8(m_registers.h);
			break;

		case Z80__CB__RR__L:		/* 0xcb 0x1d */
			Z80__RR__REG8(m_registers.l);
			break;

		case Z80__CB__RR__INDIRECT_HL:		/* 0xcb 0x1e */
			Z80__RR__INDIRECT_REG16(m_registers.hl);
			break;

		case Z80__CB__RR__A:		/* 0xcb 0x1f */
			Z80__RR__REG8(m_registers.a);
			break;

		case Z80__CB__SLA__B:		/* 0xcb 0x21 */
			Z80__SLA__REG8(m_registers.b);
			break;

		case Z80__CB__SLA__C:		/* 0xcb 0x22 */
			Z80__SLA__REG8(m_registers.c);
			break;

		case Z80__CB__SLA__D:		/* 0xcb 0x23 */
			Z80__SLA__REG8(m_registers.d);
			break;

		case Z80__CB__SLA__E:		/* 0xcb 0x24 */
			Z80__SLA__REG8(m_registers.e);
			break;

		case Z80__CB__SLA__H:		/* 0xcb 0x25 */
			Z80__SLA__REG8(m_registers.h);
			break;

		case Z80__CB__SLA__L:		/* 0xcb 0x26 */
			Z80__SLA__REG8(m_registers.l);
			break;

		case Z80__CB__SLA__INDIRECT_HL:		/* 0xcb 0x26 */
			Z80__SLA__INDIRECT_REG16(m_registers.hl);
			break;

		case Z80__CB__SLA__A:		/* 0xcb 0x27 */
			Z80__SLA__REG8(m_registers.a);
			break;

		case Z80__CB__SRA__B:		/* 0xcb 0x28 */
			Z80__SRA__REG8(m_registers.b);
			break;

		case Z80__CB__SRA__C:		/* 0xcb 0x29 */
			Z80__SRA__REG8(m_registers.c);
			break;

		case Z80__CB__SRA__D:		/* 0xcb 0x2a */
			Z80__SRA__REG8(m_registers.d);
			break;

		case Z80__CB__SRA__E:		/* 0xcb 0x2b */
			Z80__SRA__REG8(m_registers.e);
			break;

		case Z80__CB__SRA__H:		/* 0xcb 0x2c */
			Z80__SRA__REG8(m_registers.h);
			break;

		case Z80__CB__SRA__L:		/* 0xcb 0x2d */
			Z80__SRA__REG8(m_registers.l);
			break;

		case Z80__CB__SRA__INDIRECT_HL:		/* 0xcb 0x2e */
			Z80__SRA__INDIRECT_REG16(m_registers.hl);
			break;

		case Z80__CB__SRA__A:		/* 0xcb 0x2f */
			Z80__SRA__REG8(m_registers.a);
			break;

		case Z80__CB__SLL__B:		/* 0xcb 0x30 */
			Z80__SLL__REG8(m_registers.b);
			break;

		case Z80__CB__SLL__C:		/* 0xcb 0x31 */
			Z80__SLL__REG8(m_registers.c);
			break;

		case Z80__CB__SLL__D:		/* 0xcb 0x32 */
			Z80__SLL__REG8(m_registers.d);
			break;

		case Z80__CB__SLL__E:		/* 0xcb 0x33 */
			Z80__SLL__REG8(m_registers.e);
			break;

		case Z80__CB__SLL__H:		/* 0xcb 0x34 */
			Z80__SLL__REG8(m_registers.h);
			break;

		case Z80__CB__SLL__L:		/* 0xcb 0x35 */
			Z80__SLL__REG8(m_registers.l);
			break;

		case Z80__CB__SLL__INDIRECT_HL:		/* 0xcb 0x36 */
			Z80__SLL__INDIRECT_REG16(m_registers.hl);
			break;

		case Z80__CB__SLL__A:		/* 0xcb 0x37 */
			Z80__SLL__REG8(m_registers.a);
			break;

		case Z80__CB__SRL__B:		/* 0xcb 0x38 */
			Z80__SRL__REG8(m_registers.b);
			break;

		case Z80__CB__SRL__C:		/* 0xcb 0x39 */
			Z80__SRL__REG8(m_registers.c);
			break;

		case Z80__CB__SRL__D:		/* 0xcb 0x3a */
			Z80__SRL__REG8(m_registers.d);
			break;

		case Z80__CB__SRL__E:		/* 0xcb 0x3b */
			Z80__SRL__REG8(m_registers.e);
			break;

		case Z80__CB__SRL__H:		/* 0xcb 0x3c */
			Z80__SRL__REG8(m_registers.h);
			break;

		case Z80__CB__SRL__L:		/* 0xcb 0x3d */
			Z80__SRL__REG8(m_registers.l);
			break;

		case Z80__CB__SRL__INDIRECT_HL:		/* 0xcb 0x3e */
			Z80__SRL__INDIRECT_REG16(m_registers.hl);
			break;

		case Z80__CB__SRL__A:		/* 0xcb 0x3f */
			Z80__SRL__REG8(m_registers.a);
			break;

		/* BIT opcodes */
		case Z80__CB__BIT__0__B:		/* 0xcb 0x40 */
			Z80__BIT__N__REG8(0, m_registers.b);
			break;

		case Z80__CB__BIT__0__C:		/* 0xcb 0x41 */
			Z80__BIT__N__REG8(0, m_registers.c);
			break;

		case Z80__CB__BIT__0__D:		/* 0xcb 0x42 */
			Z80__BIT__N__REG8(0, m_registers.d);
			break;

		case Z80__CB__BIT__0__E:		/* 0xcb 0x43 */
			Z80__BIT__N__REG8(0, m_registers.e);
			break;

		case Z80__CB__BIT__0__H:		/* 0xcb 0x44 */
			Z80__BIT__N__REG8(0, m_registers.h);
			break;

		case Z80__CB__BIT__0__L:		/* 0xcb 0x45 */
			Z80__BIT__N__REG8(0, m_registers.l);
			break;

		case Z80__CB__BIT__0__INDIRECT_HL:		/* 0xcb 0x46 */
			Z80__BIT__N__INDIRECT_REG16(0, m_registers.hl);
			break;

		case Z80__CB__BIT__0__A:		/* 0xcb 0x47 */
			Z80__BIT__N__REG8(0, m_registers.a);
			break;

		case Z80__CB__BIT__1__B:		/* 0xcb 0x48 */
			Z80__BIT__N__REG8(1, m_registers.b);
			break;

		case Z80__CB__BIT__1__C:		/* 0xcb 0x49 */
			Z80__BIT__N__REG8(1, m_registers.c);
			break;

		case Z80__CB__BIT__1__D:		/* 0xcb 0x4a */
			Z80__BIT__N__REG8(1, m_registers.d);
			break;

		case Z80__CB__BIT__1__E:		/* 0xcb 0x4b */
			Z80__BIT__N__REG8(1, m_registers.e);
			break;

		case Z80__CB__BIT__1__H:		/* 0xcb 0x4c */
			Z80__BIT__N__REG8(1, m_registers.h);
			break;

		case Z80__CB__BIT__1__L:		/* 0xcb 0x4d */
			Z80__BIT__N__REG8(1, m_registers.l);
			break;

		case Z80__CB__BIT__1__INDIRECT_HL:		/* 0xcb 0x4e */
			Z80__BIT__N__INDIRECT_REG16(1, m_registers.hl);
			break;

		case Z80__CB__BIT__1__A:		/* 0xcb 0x4f */
			Z80__BIT__N__REG8(1, m_registers.a);
			break;

		case Z80__CB__BIT__2__B:		/* 0xcb 0x50 */
			Z80__BIT__N__REG8(2, m_registers.b);
			break;

		case Z80__CB__BIT__2__C:		/* 0xcb 0x51 */
			Z80__BIT__N__REG8(2, m_registers.c);
			break;

		case Z80__CB__BIT__2__D:		/* 0xcb 0x52 */
			Z80__BIT__N__REG8(2, m_registers.d);
			break;

		case Z80__CB__BIT__2__E:		/* 0xcb 0x53 */
			Z80__BIT__N__REG8(2, m_registers.e);
			break;

		case Z80__CB__BIT__2__H:		/* 0xcb 0x54 */
			Z80__BIT__N__REG8(2, m_registers.h);
			break;

		case Z80__CB__BIT__2__L:		/* 0xcb 0x55 */
			Z80__BIT__N__REG8(2, m_registers.l);
			break;

		case Z80__CB__BIT__2__INDIRECT_HL:		/* 0xcb 0x56 */
			Z80__BIT__N__INDIRECT_REG16(2, m_registers.b);
			break;

		case Z80__CB__BIT__2__A:		/* 0xcb 0x57 */
			Z80__BIT__N__REG8(2, m_registers.a);
			break;

		case Z80__CB__BIT__3__B:		/* 0xcb 0x58 */
			Z80__BIT__N__REG8(3, m_registers.b);
			break;

		case Z80__CB__BIT__3__C:		/* 0xcb 0x59 */
			Z80__BIT__N__REG8(3, m_registers.c);
			break;

		case Z80__CB__BIT__3__D:		/* 0xcb 0x5a */
			Z80__BIT__N__REG8(3, m_registers.d);
			break;

		case Z80__CB__BIT__3__E:		/* 0xcb 0x5b */
			Z80__BIT__N__REG8(3, m_registers.e);
			break;

		case Z80__CB__BIT__3__H:		/* 0xcb 0x5c */
			Z80__BIT__N__REG8(3, m_registers.h);
			break;

		case Z80__CB__BIT__3__L:		/* 0xcb 0x5d */
			Z80__BIT__N__REG8(3, m_registers.l);
			break;

		case Z80__CB__BIT__3__INDIRECT_HL:		/* 0xcb 0x5e */
			Z80__BIT__N__INDIRECT_REG16(3, m_registers.a);
			break;

		case Z80__CB__BIT__3__A:		/* 0xcb 0x5f */
			Z80__BIT__N__REG8(3, m_registers.a);
			break;

		case Z80__CB__BIT__4__B:		/* 0xcb 0x60 */
			Z80__BIT__N__REG8(4, m_registers.b);
			break;

		case Z80__CB__BIT__4__C:		/* 0xcb 0x61 */
			Z80__BIT__N__REG8(4, m_registers.c);
			break;

		case Z80__CB__BIT__4__D:		/* 0xcb 0x62 */
			Z80__BIT__N__REG8(4, m_registers.d);
			break;

		case Z80__CB__BIT__4__E:		/* 0xcb 0x63 */
			Z80__BIT__N__REG8(4, m_registers.e);
			break;

		case Z80__CB__BIT__4__H:		/* 0xcb 0x64 */
			Z80__BIT__N__REG8(4, m_registers.h);
			break;

		case Z80__CB__BIT__4__L:		/* 0xcb 0x65 */
			Z80__BIT__N__REG8(4, m_registers.l);
			break;

		case Z80__CB__BIT__4__INDIRECT_HL:		/* 0xcb 0x66 */
			Z80__BIT__N__INDIRECT_REG16(4, m_registers.hl);
			break;

		case Z80__CB__BIT__4__A:		/* 0xcb 0x67 */
			Z80__BIT__N__REG8(4, m_registers.a);
			break;

		case Z80__CB__BIT__5__B:		/* 0xcb 0x68 */
			Z80__BIT__N__REG8(5, m_registers.b);
			break;

		case Z80__CB__BIT__5__C:		/* 0xcb 0x69 */
			Z80__BIT__N__REG8(5, m_registers.c);
			break;

		case Z80__CB__BIT__5__D:		/* 0xcb 0x6a */
			Z80__BIT__N__REG8(5, m_registers.d);
			break;

		case Z80__CB__BIT__5__E:		/* 0xcb 0x6b */
			Z80__BIT__N__REG8(5, m_registers.e);
			break;

		case Z80__CB__BIT__5__H:		/* 0xcb 0x6c */
			Z80__BIT__N__REG8(5, m_registers.h);
			break;

		case Z80__CB__BIT__5__L:		/* 0xcb 0x6d */
			Z80__BIT__N__REG8(5, m_registers.l);
			break;

		case Z80__CB__BIT__5__INDIRECT_HL:		/* 0xcb 0x6e */
			Z80__BIT__N__INDIRECT_REG16(5, m_registers.hl);
			break;

		case Z80__CB__BIT__5__A:		/* 0xcb 0x6f */
			Z80__BIT__N__REG8(5, m_registers.a);
			break;

		case Z80__CB__BIT__6__B:		/* 0xcb 0x70 */
			Z80__BIT__N__REG8(6, m_registers.b);
			break;

		case Z80__CB__BIT__6__C:		/* 0xcb 0x71 */
			Z80__BIT__N__REG8(6, m_registers.c);
			break;

		case Z80__CB__BIT__6__D:		/* 0xcb 0x72 */
			Z80__BIT__N__REG8(6, m_registers.d);
			break;

		case Z80__CB__BIT__6__E:		/* 0xcb 0x73 */
			Z80__BIT__N__REG8(6, m_registers.e);
			break;

		case Z80__CB__BIT__6__H:		/* 0xcb 0x74 */
			Z80__BIT__N__REG8(6, m_registers.h);
			break;

		case Z80__CB__BIT__6__L:		/* 0xcb 0x75 */
			Z80__BIT__N__REG8(6, m_registers.l);
			break;

		case Z80__CB__BIT__6__INDIRECT_HL:		/* 0xcb 0x76 */
			Z80__BIT__N__INDIRECT_REG16(6, m_registers.hl);
			break;

		case Z80__CB__BIT__6__A:		/* 0xcb 0x77 */
			Z80__BIT__N__REG8(6, m_registers.a);
			break;

		case Z80__CB__BIT__7__B:		/* 0xcb 0x78 */
			Z80__BIT__N__REG8(7, m_registers.b);
			break;

		case Z80__CB__BIT__7__C:		/* 0xcb 0x79 */
			Z80__BIT__N__REG8(7, m_registers.c);
			break;

		case Z80__CB__BIT__7__D:		/* 0xcb 0x7a */
			Z80__BIT__N__REG8(7, m_registers.d);
			break;

		case Z80__CB__BIT__7__E:		/* 0xcb 0x7b */
			Z80__BIT__N__REG8(7, m_registers.e);
			break;

		case Z80__CB__BIT__7__H:		/* 0xcb 0x7c */
			Z80__BIT__N__REG8(7, m_registers.h);
			break;

		case Z80__CB__BIT__7__L:		/* 0xcb 0x7d */
			Z80__BIT__N__REG8(7, m_registers.l);
			break;

		case Z80__CB__BIT__7__INDIRECT_HL:		/* 0xcb 0x7e */
			Z80__BIT__N__INDIRECT_REG16(7, m_registers.hl);
			break;

		case Z80__CB__BIT__7__A:		/* 0xcb 0x7f */
			Z80__BIT__N__REG8(7, m_registers.a);
			break;

		/* RES opcodes */
		case Z80__CB__RES__0__B:		/* 0xcb 0x80 */
			Z80__RES__N__REG8(0, m_registers.b);
			break;

		case Z80__CB__RES__0__C:		/* 0xcb 0x81 */
			Z80__RES__N__REG8(0, m_registers.c);
			break;

		case Z80__CB__RES__0__D:		/* 0xcb 0x82 */
			Z80__RES__N__REG8(0, m_registers.d);
			break;

		case Z80__CB__RES__0__E:		/* 0xcb 0x83 */
			Z80__RES__N__REG8(0, m_registers.e);
			break;

		case Z80__CB__RES__0__H:		/* 0xcb 0x84 */
			Z80__RES__N__REG8(0, m_registers.h);
			break;

		case Z80__CB__RES__0__L:		/* 0xcb 0x85 */
			Z80__RES__N__REG8(0, m_registers.l);
			break;

		case Z80__CB__RES__0__INDIRECT_HL:		/* 0xcb 0x86 */
			Z80__RES__N__INDIRECT_REG16(0, m_registers.hl);
			break;

		case Z80__CB__RES__0__A:		/* 0xcb 0x87 */
			Z80__RES__N__REG8(0, m_registers.a);
			break;

		case Z80__CB__RES__1__B:		/* 0xcb 0x88 */
			Z80__RES__N__REG8(1, m_registers.b);
			break;

		case Z80__CB__RES__1__C:		/* 0xcb 0x89 */
			Z80__RES__N__REG8(1, m_registers.c);
			break;

		case Z80__CB__RES__1__D:		/* 0xcb 0x8a */
			Z80__RES__N__REG8(1, m_registers.d);
			break;

		case Z80__CB__RES__1__E:		/* 0xcb 0x8b */
			Z80__RES__N__REG8(1, m_registers.e);
			break;

		case Z80__CB__RES__1__H:		/* 0xcb 0x8c */
			Z80__RES__N__REG8(1, m_registers.h);
			break;

		case Z80__CB__RES__1__L:		/* 0xcb 0x8d */
			Z80__RES__N__REG8(1, m_registers.l);
			break;

		case Z80__CB__RES__1__INDIRECT_HL:		/* 0xcb 0x8e */
			Z80__RES__N__INDIRECT_REG16(1, m_registers.hl);
			break;

		case Z80__CB__RES__1__A:		/* 0xcb 0x8f */
			Z80__RES__N__REG8(1, m_registers.a);
			break;

		case Z80__CB__RES__2__B:		/* 0xcb 0x90 */
			Z80__RES__N__REG8(2, m_registers.b);
			break;

		case Z80__CB__RES__2__C:		/* 0xcb 0x91 */
			Z80__RES__N__REG8(2, m_registers.c);
			break;

		case Z80__CB__RES__2__D:		/* 0xcb 0x92 */
			Z80__RES__N__REG8(2, m_registers.d);
			break;

		case Z80__CB__RES__2__E:		/* 0xcb 0x93 */
			Z80__RES__N__REG8(2, m_registers.e);
			break;

		case Z80__CB__RES__2__H:		/* 0xcb 0x94 */
			Z80__RES__N__REG8(2, m_registers.h);
			break;

		case Z80__CB__RES__2__L:		/* 0xcb 0x95 */
			Z80__RES__N__REG8(2, m_registers.l);
			break;

		case Z80__CB__RES__2__INDIRECT_HL:		/* 0xcb 0x96 */
			Z80__RES__N__INDIRECT_REG16(2, m_registers.hl);
			break;

		case Z80__CB__RES__2__A:		/* 0xcb 0x97 */
			Z80__RES__N__REG8(2, m_registers.a);
			break;

		case Z80__CB__RES__3__B:		/* 0xcb 0x98 */
			Z80__RES__N__REG8(3, m_registers.b);
			break;

		case Z80__CB__RES__3__C:		/* 0xcb 0x99 */
			Z80__RES__N__REG8(3, m_registers.c);
			break;

		case Z80__CB__RES__3__D:		/* 0xcb 0x9a */
			Z80__RES__N__REG8(3, m_registers.d);
			break;

		case Z80__CB__RES__3__E:		/* 0xcb 0x9b */
			Z80__RES__N__REG8(3, m_registers.e);
			break;

		case Z80__CB__RES__3__H:		/* 0xcb 0x9c */
			Z80__RES__N__REG8(3, m_registers.h);
			break;

		case Z80__CB__RES__3__L:		/* 0xcb 0x9d */
			Z80__RES__N__REG8(3, m_registers.l);
			break;

		case Z80__CB__RES__3__INDIRECT_HL:		/* 0xcb 0x9e */
			Z80__RES__N__INDIRECT_REG16(3, m_registers.hl);
			break;

		case Z80__CB__RES__3__A:		/* 0xcb 0x9f */
			Z80__RES__N__REG8(3, m_registers.a);
			break;

		case Z80__CB__RES__4__B:		/* 0xcb 0xa0 */
			Z80__RES__N__REG8(4, m_registers.b);
			break;

		case Z80__CB__RES__4__C:		/* 0xcb 0xa1 */
			Z80__RES__N__REG8(4, m_registers.c);
			break;

		case Z80__CB__RES__4__D:		/* 0xcb 0xa2 */
			Z80__RES__N__REG8(4, m_registers.d);
			break;

		case Z80__CB__RES__4__E:		/* 0xcb 0xa3 */
			Z80__RES__N__REG8(4, m_registers.e);
			break;

		case Z80__CB__RES__4__H:		/* 0xcb 0xa4 */
			Z80__RES__N__REG8(4, m_registers.h);
			break;

		case Z80__CB__RES__4__L:		/* 0xcb 0xa5 */
			Z80__RES__N__REG8(4, m_registers.l);
			break;

		case Z80__CB__RES__4__INDIRECT_HL:		/* 0xcb 0xa6 */
			Z80__RES__N__INDIRECT_REG16(4, m_registers.hl);
			break;

		case Z80__CB__RES__4__A:		/* 0xcb 0xa7 */
			Z80__RES__N__REG8(4, m_registers.a);
			break;

		case Z80__CB__RES__5__B:		/* 0xcb 0xa8 */
			Z80__RES__N__REG8(5, m_registers.b);
			break;

		case Z80__CB__RES__5__C:		/* 0xcb 0xa9 */
			Z80__RES__N__REG8(5, m_registers.c);
			break;

		case Z80__CB__RES__5__D:		/* 0xcb 0xaa */
			Z80__RES__N__REG8(5, m_registers.d);
			break;

		case Z80__CB__RES__5__E:		/* 0xcb 0xab */
			Z80__RES__N__REG8(5, m_registers.e);
			break;

		case Z80__CB__RES__5__H:		/* 0xcb 0xac */
			Z80__RES__N__REG8(5, m_registers.h);
			break;

		case Z80__CB__RES__5__L:		/* 0xcb 0xad */
			Z80__RES__N__REG8(5, m_registers.l);
			break;

		case Z80__CB__RES__5__INDIRECT_HL:		/* 0xcb 0xae */
			Z80__RES__N__INDIRECT_REG16(5, m_registers.hl);
			break;

		case Z80__CB__RES__5__A:		/* 0xcb 0xaf */
			Z80__RES__N__REG8(5, m_registers.a);
			break;

		case Z80__CB__RES__6__B:		/* 0xcb 0xb0 */
			Z80__RES__N__REG8(6, m_registers.b);
			break;

		case Z80__CB__RES__6__C:		/* 0xcb 0xb1 */
			Z80__RES__N__REG8(6, m_registers.c);
			break;

		case Z80__CB__RES__6__D:		/* 0xcb 0xb2 */
			Z80__RES__N__REG8(6, m_registers.d);
			break;

		case Z80__CB__RES__6__E:		/* 0xcb 0xb3 */
			Z80__RES__N__REG8(6, m_registers.e);
			break;

		case Z80__CB__RES__6__H:		/* 0xcb 0xb4 */
			Z80__RES__N__REG8(6, m_registers.h);
			break;

		case Z80__CB__RES__6__L:		/* 0xcb 0xb5 */
			Z80__RES__N__REG8(6, m_registers.l);
			break;

		case Z80__CB__RES__6__INDIRECT_HL:		/* 0xcb 0xb6 */
			Z80__RES__N__INDIRECT_REG16(6, m_registers.hl);
			break;

		case Z80__CB__RES__6__A:		/* 0xcb 0xb7 */
			Z80__RES__N__REG8(6, m_registers.a);
			break;

		case Z80__CB__RES__7__B:		/* 0xcb 0xb8 */
			Z80__RES__N__REG8(7, m_registers.b);
			break;

		case Z80__CB__RES__7__C:		/* 0xcb 0xb9 */
			Z80__RES__N__REG8(7, m_registers.c);
			break;

		case Z80__CB__RES__7__D:		/* 0xcb 0xba */
			Z80__RES__N__REG8(7, m_registers.d);
			break;

		case Z80__CB__RES__7__E:		/* 0xcb 0xbb */
			Z80__RES__N__REG8(7, m_registers.e);
			break;

		case Z80__CB__RES__7__H:		/* 0xcb 0xbc */
			Z80__RES__N__REG8(7, m_registers.h);
			break;

		case Z80__CB__RES__7__L:		/* 0xcb 0xbd */
			Z80__RES__N__REG8(7, m_registers.l);
			break;

		case Z80__CB__RES__7__INDIRECT_HL:		/* 0xcb 0xbe */
			Z80__RES__N__INDIRECT_REG16(7, m_registers.hl);
			break;

		case Z80__CB__RES__7__A:		/* 0xcb 0xbf */
			Z80__RES__N__REG8(7, m_registers.a);
			break;

		/* SET opcodes */
		case Z80__CB__SET__0__B:		/* 0xcb 0xc0 */
			Z80__SET__N__REG8(0, m_registers.b);
			break;

		case Z80__CB__SET__0__C:		/* 0xcb 0xc1 */
			Z80__SET__N__REG8(0, m_registers.c);
			break;

		case Z80__CB__SET__0__D:		/* 0xcb 0xc2 */
			Z80__SET__N__REG8(0, m_registers.d);
			break;

		case Z80__CB__SET__0__E:		/* 0xcb 0xc3 */
			Z80__SET__N__REG8(0, m_registers.e);
			break;

		case Z80__CB__SET__0__H:		/* 0xcb 0xc4 */
			Z80__SET__N__REG8(0, m_registers.h);
			break;

		case Z80__CB__SET__0__L:		/* 0xcb 0xc5 */
			Z80__SET__N__REG8(0, m_registers.l);
			break;

		case Z80__CB__SET__0__INDIRECT_HL:		/* 0xcb 0xc6 */
			Z80__SET__N__INDIRECT_REG16(0, m_registers.hl);
			break;

		case Z80__CB__SET__0__A:		/* 0xcb 0xc7 */
			Z80__SET__N__REG8(0, m_registers.a);
			break;

		case Z80__CB__SET__1__B:		/* 0xcb 0xc8 */
			Z80__SET__N__REG8(1, m_registers.b);
			break;

		case Z80__CB__SET__1__C:		/* 0xcb 0xc9 */
			Z80__SET__N__REG8(1, m_registers.c);
			break;

		case Z80__CB__SET__1__D:		/* 0xcb 0xca */
			Z80__SET__N__REG8(1, m_registers.d);
			break;

		case Z80__CB__SET__1__E:		/* 0xcb 0xcb */
			Z80__SET__N__REG8(1, m_registers.e);
			break;

		case Z80__CB__SET__1__H:		/* 0xcb 0xcc */
			Z80__SET__N__REG8(1, m_registers.h);
			break;

		case Z80__CB__SET__1__L:		/* 0xcb 0xcd */
			Z80__SET__N__REG8(1, m_registers.l);
			break;

		case Z80__CB__SET__1__INDIRECT_HL:		/* 0xcb 0xce */
			Z80__SET__N__INDIRECT_REG16(1, m_registers.hl);
			break;

		case Z80__CB__SET__1__A:		/* 0xcb 0xcf */
			Z80__SET__N__REG8(1, m_registers.a);
			break;

		case Z80__CB__SET__2__B:		/* 0xcb 0xd0 */
			Z80__SET__N__REG8(2, m_registers.b);
			break;

		case Z80__CB__SET__2__C:		/* 0xcb 0xd1 */
			Z80__SET__N__REG8(2, m_registers.c);
			break;

		case Z80__CB__SET__2__D:		/* 0xcb 0xd2 */
			Z80__SET__N__REG8(2, m_registers.d);
			break;

		case Z80__CB__SET__2__E:		/* 0xcb 0xd3 */
			Z80__SET__N__REG8(2, m_registers.e);
			break;

		case Z80__CB__SET__2__H:		/* 0xcb 0xd4 */
			Z80__SET__N__REG8(2, m_registers.h);
			break;

		case Z80__CB__SET__2__L:		/* 0xcb 0xd5 */
			Z80__SET__N__REG8(2, m_registers.l);
			break;

		case Z80__CB__SET__2__INDIRECT_HL:		/* 0xcb 0xd6 */
			Z80__SET__N__INDIRECT_REG16(2, m_registers.hl);
			break;

		case Z80__CB__SET__2__A:		/* 0xcb 0xd7 */
			Z80__SET__N__REG8(2, m_registers.a);
			break;

		case Z80__CB__SET__3__B:		/* 0xcb 0xd8 */
			Z80__SET__N__REG8(3, m_registers.b);
			break;

		case Z80__CB__SET__3__C:		/* 0xcb 0xd9 */
			Z80__SET__N__REG8(3, m_registers.c);
			break;

		case Z80__CB__SET__3__D:		/* 0xcb 0xda */
			Z80__SET__N__REG8(3, m_registers.d);
			break;

		case Z80__CB__SET__3__E:		/* 0xcb 0xdb */
			Z80__SET__N__REG8(3, m_registers.e);
			break;

		case Z80__CB__SET__3__H:		/* 0xcb 0xdc */
			Z80__SET__N__REG8(3, m_registers.h);
			break;

		case Z80__CB__SET__3__L:		/* 0xcb 0xdd */
			Z80__SET__N__REG8(3, m_registers.l);
			break;

		case Z80__CB__SET__3__INDIRECT_HL:		/* 0xcb 0xde */
			Z80__SET__N__INDIRECT_REG16(3, m_registers.hl);
			break;

		case Z80__CB__SET__3__A:		/* 0xcb 0xdf */
			Z80__SET__N__REG8(3, m_registers.a);
			break;

		case Z80__CB__SET__4__B:		/* 0xcb 0xe0 */
			Z80__SET__N__REG8(4, m_registers.b);
			break;

		case Z80__CB__SET__4__C:		/* 0xcb 0xe1 */
			Z80__SET__N__REG8(4, m_registers.c);
			break;

		case Z80__CB__SET__4__D:		/* 0xcb 0xe2 */
			Z80__SET__N__REG8(4, m_registers.d);
			break;

		case Z80__CB__SET__4__E:		/* 0xcb 0xe3 */
			Z80__SET__N__REG8(4, m_registers.e);
			break;

		case Z80__CB__SET__4__H:		/* 0xcb 0xe4 */
			Z80__SET__N__REG8(4, m_registers.h);
			break;

		case Z80__CB__SET__4__L:		/* 0xcb 0xe5 */
			Z80__SET__N__REG8(4, m_registers.l);
			break;

		case Z80__CB__SET__4__INDIRECT_HL:		/* 0xcb 0xe6 */
			Z80__SET__N__INDIRECT_REG16(4, m_registers.hl);
			break;

		case Z80__CB__SET__4__A:		/* 0xcb 0xe7 */
			Z80__SET__N__REG8(4, m_registers.a);
			break;

		case Z80__CB__SET__5__B:		/* 0xcb 0xe8 */
			Z80__SET__N__REG8(5, m_registers.b);
			break;

		case Z80__CB__SET__5__C:		/* 0xcb 0xe9 */
			Z80__SET__N__REG8(5, m_registers.c);
			break;

		case Z80__CB__SET__5__D:		/* 0xcb 0xea */
			Z80__SET__N__REG8(5, m_registers.d);
			break;

		case Z80__CB__SET__5__E:		/* 0xcb 0xeb */
			Z80__SET__N__REG8(5, m_registers.e);
			break;

		case Z80__CB__SET__5__H:		/* 0xcb 0xec */
			Z80__SET__N__REG8(5, m_registers.h);
			break;

		case Z80__CB__SET__5__L:		/* 0xcb 0xed */
			Z80__SET__N__REG8(5, m_registers.l);
			break;

		case Z80__CB__SET__5__INDIRECT_HL:		/* 0xcb 0xee */
			Z80__SET__N__INDIRECT_REG16(5, m_registers.hl);
			break;

		case Z80__CB__SET__5__A:		/* 0xcb 0xef */
			Z80__SET__N__REG8(5, m_registers.a);
			break;

		case Z80__CB__SET__6__B:		/* 0xcb 0xf0 */
			Z80__SET__N__REG8(6, m_registers.b);
			break;

		case Z80__CB__SET__6__C:		/* 0xcb 0xf1 */
			Z80__SET__N__REG8(6, m_registers.c);
			break;

		case Z80__CB__SET__6__D:		/* 0xcb 0xf2 */
			Z80__SET__N__REG8(6, m_registers.d);
			break;

		case Z80__CB__SET__6__E:		/* 0xcb 0xf3 */
			Z80__SET__N__REG8(6, m_registers.e);
			break;

		case Z80__CB__SET__6__H:		/* 0xcb 0xf4 */
			Z80__SET__N__REG8(6, m_registers.h);
			break;

		case Z80__CB__SET__6__L:		/* 0xcb 0xf5 */
			Z80__SET__N__REG8(6, m_registers.l);
			break;

		case Z80__CB__SET__6__INDIRECT_HL:		/* 0xcb 0xf6 */
			Z80__SET__N__INDIRECT_REG16(6, m_registers.hl);
			break;

		case Z80__CB__SET__6__A:		/* 0xcb 0xf7 */
			Z80__SET__N__REG8(6, m_registers.a);
			break;

		case Z80__CB__SET__7__B:		/* 0xcb 0xf8 */
			Z80__SET__N__REG8(7, m_registers.b);
			break;

		case Z80__CB__SET__7__C:		/* 0xcb 0xf9 */
			Z80__SET__N__REG8(7, m_registers.c);
			break;

		case Z80__CB__SET__7__D:		/* 0xcb 0xfa */
			Z80__SET__N__REG8(7, m_registers.d);
			break;

		case Z80__CB__SET__7__E:		/* 0xcb 0xfb */
			Z80__SET__N__REG8(7, m_registers.e);
			break;

		case Z80__CB__SET__7__H:		/* 0xcb 0xfc */
			Z80__SET__N__REG8(7, m_registers.h);
			break;

		case Z80__CB__SET__7__L:		/* 0xcb 0xfd */
			Z80__SET__N__REG8(7, m_registers.l);
			break;

		case Z80__CB__SET__7__INDIRECT_HL:		/* 0xcb 0xfe */
			Z80__SET__N__INDIRECT_REG16(7, m_registers.hl);
			break;

		case Z80__CB__SET__7__A:		/* 0xcb 0xff */
			Z80__SET__N__REG8(7, m_registers.a);
			break;

		default:
			std::cerr << "unhandled opcode: 0xcb 0x" << std::hex << (*instruction) << std::endl;
			Z80_INVALID_INSTRUCTION;
			break;
	}

	if (cycles) {
	    *cycles = (useJumpCycleCost ? Z80_CYCLES_JUMP(m_cb_opcode_cycles[*instruction]) : Z80_CYCLES_NOJUMP(m_cb_opcode_cycles[*instruction]));
	}

	if (size) {
	    *size = 2;
	}

	return true;
}


bool Z80::Z80::executeEdInstruction(const Z80::Z80::UnsignedByte * instruction, bool * doPc, int * cycles, int * size)
{
	bool useJumpCycleCost = false;

	switch (*instruction) {
		case Z80__ED__NOP__0XED__0X00:
			break;

		case Z80__ED__NOP__0XED__0X01:
			break;

		case Z80__ED__NOP__0XED__0X02:
			break;

		case Z80__ED__NOP__0XED__0X03:
			break;

		case Z80__ED__NOP__0XED__0X04:
			break;

		case Z80__ED__NOP__0XED__0X05:
			break;

		case Z80__ED__NOP__0XED__0X06:
			break;

		case Z80__ED__NOP__0XED__0X07:
			break;

		case Z80__ED__NOP__0XED__0X08:
			break;

		case Z80__ED__NOP__0XED__0X09:
			break;

		case Z80__ED__NOP__0XED__0X0A:
			break;

		case Z80__ED__NOP__0XED__0X0B:
			break;

		case Z80__ED__NOP__0XED__0X0C:
			break;

		case Z80__ED__NOP__0XED__0X0D:
			break;

		case Z80__ED__NOP__0XED__0X0E:
			break;

		case Z80__ED__NOP__0XED__0X0F:
			break;

		case Z80__ED__NOP__0XED__0X10:
			break;

		case Z80__ED__NOP__0XED__0X11:
			break;

		case Z80__ED__NOP__0XED__0X12:
			break;

		case Z80__ED__NOP__0XED__0X13:
			break;

		case Z80__ED__NOP__0XED__0X14:
			break;

		case Z80__ED__NOP__0XED__0X15:
			break;

		case Z80__ED__NOP__0XED__0X16:
			break;

		case Z80__ED__NOP__0XED__0X17:
			break;

		case Z80__ED__NOP__0XED__0X18:
			break;

		case Z80__ED__NOP__0XED__0X19:
			break;

		case Z80__ED__NOP__0XED__0X1A:
			break;

		case Z80__ED__NOP__0XED__0X1B:
			break;

		case Z80__ED__NOP__0XED__0X1C:
			break;

		case Z80__ED__NOP__0XED__0X1D:
			break;

		case Z80__ED__NOP__0XED__0X1E:
			break;

		case Z80__ED__NOP__0XED__0X1F:
			break;

		case Z80__ED__NOP__0XED__0X20:
			break;

		case Z80__ED__NOP__0XED__0X21:
			break;

		case Z80__ED__NOP__0XED__0X22:
			break;

		case Z80__ED__NOP__0XED__0X23:
			break;

		case Z80__ED__NOP__0XED__0X24:
			break;

		case Z80__ED__NOP__0XED__0X25:
			break;

		case Z80__ED__NOP__0XED__0X26:
			break;

		case Z80__ED__NOP__0XED__0X27:
			break;

		case Z80__ED__NOP__0XED__0X28:
			break;

		case Z80__ED__NOP__0XED__0X29:
			break;

		case Z80__ED__NOP__0XED__0X2A:
			break;

		case Z80__ED__NOP__0XED__0X2B:
			break;

		case Z80__ED__NOP__0XED__0X2C:
			break;

		case Z80__ED__NOP__0XED__0X2D:
			break;

		case Z80__ED__NOP__0XED__0X2E:
			break;

		case Z80__ED__NOP__0XED__0X2F:
			break;

		case Z80__ED__NOP__0XED__0X30:
			break;

		case Z80__ED__NOP__0XED__0X31:
			break;

		case Z80__ED__NOP__0XED__0X32:
			break;

		case Z80__ED__NOP__0XED__0X33:
			break;

		case Z80__ED__NOP__0XED__0X34:
			break;

		case Z80__ED__NOP__0XED__0X35:
			break;

		case Z80__ED__NOP__0XED__0X36:
			break;

		case Z80__ED__NOP__0XED__0X37:
			break;

		case Z80__ED__NOP__0XED__0X38:
			break;

		case Z80__ED__NOP__0XED__0X39:
			break;

		case Z80__ED__NOP__0XED__0X3A:
			break;

		case Z80__ED__NOP__0XED__0X3B:
			break;

		case Z80__ED__NOP__0XED__0X3C:
			break;

		case Z80__ED__NOP__0XED__0X3D:
			break;

		case Z80__ED__NOP__0XED__0X3E:
			break;

		case Z80__ED__NOP__0XED__0X3F:
			break;

		case Z80__ED__IN__B__INDIRECT_C:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__OUT__INDIRECT_C__B:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__SBC__HL__BC:					/* 0xed 0x42 */
//        {
//            UnsignedWord oldValue = (m_registers.hl);
//            (m_registers.hl) -= ((m_registers.bc) + (Z80_FLAG_C_ISSET ? 1 : 0));
//            Z80_FLAG_N_SET;
//            Z80_FLAG_Z_UPDATE(0 == (m_registers.hl));
//            Z80_FLAG_S_DEFAULTBEHAVIOUR16;
//            Z80_FLAG_P_UPDATE((m_registers.hl) > oldValue);
//            Z80_FLAG_C_UPDATE((m_registers.hl) > oldValue);
//            Z80_FLAG_H_UPDATE(Z80_CHECK_16BIT_HALFCARRY_10_TO_11(oldValue, (m_registers.hl)));
//        }
            Z80__SBC__REG16__REG16(m_registers.hl, m_registers.bc);
			break;

		case Z80__ED__LD__INDIRECT_NN__BC:		/* 0xed 0x43 */
			Z80__LD__INDIRECT_NN__REG16(Z80::Z80::z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))), m_registers.bc);
			break;

		case Z80__ED__NEG:							/* 0xed 0x44 */
			Z80_NEG;
			break;

		case Z80__ED__RETN:							/* 0xed 0c45 */
			Z80__RETN;
			Z80_DONT_UPDATE_PC;
			break;

		case Z80__ED__IM__0:
			m_interruptMode = 0;
			break;

		case Z80__ED__IN__C__INDIRECT_C:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__OUT__INDIRECT_C__C:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__ADC__HL__BC:
			Z80__ADC__REG16__REG16(m_registers.hl, m_registers.bc);
			break;

		case Z80__ED__LD__BC__INDIRECT_NN:
			Z80__LD__REG16__INDIRECT_NN(m_registers.bc, Z80::Z80::z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))));
			break;

		case Z80__ED__NEG__0XED__0X4C:		/* 0xed 0x4c */
			Z80_NEG;
			break;

		case Z80__ED__RETI:					/* 0xed 0x4d */
			Z80__RETI;
			Z80_DONT_UPDATE_PC;
			break;

		case Z80__ED__IM__0__0XED__0X4E:	/* 0xed 0x4e */
			/* non-standard instruction; not guaranteed that this is the instruction
			 * in all versions of the Z80 */
			m_interruptMode = 0;
			break;

		case Z80__ED__IN__D__INDIRECT_C:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__OUT__INDIRECT_C__D:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__SBC__HL__DE:
			Z80__SBC__REG16__REG16(m_registers.hl, m_registers.de);
			break;

		case Z80__ED__LD__INDIRECT_NN__DE:	/* 0xed 0x53 */
			Z80__LD__INDIRECT_NN__REG16(Z80::Z80::z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))), m_registers.de);
			break;

		case Z80__ED__NEG__0XED__0X54:		/* 0xed 0x54 */
			Z80_NEG;
			break;

		case Z80__ED__RETN__0XED__0X55:		/* 0xed 0x55 */
			Z80__RETN;
			Z80_DONT_UPDATE_PC;
			break;

		case Z80__ED__IM__1:					/* 0xed 0x56 */
			m_interruptMode = 1;
			break;

		case Z80__ED__LD__A__I:				/* 0xed 0x57 */
			Z80__LD__REG8__REG8(m_registers.a, m_registers.i);
			Z80_FLAG_S_DEFAULTBEHAVIOUR;
			Z80_FLAG_Z_DEFAULTBEHAVIOUR;
			Z80_FLAG_H_CLEAR;
			Z80_FLAG_P_UPDATE(isEvenParity(m_registers.a));
			Z80_FLAG_N_CLEAR;
			break;

		case Z80__ED__IN__E__INDIRECT_C:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__OUT__INDIRECT_C__E:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__ADC__HL__DE:
			Z80__ADC__REG16__REG16(m_registers.hl, m_registers.de);
			break;

		case Z80__ED__LD__DE__INDIRECT_NN:
			Z80__LD__REG16__INDIRECT_NN(m_registers.de, Z80::Z80::z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))));
			break;

		case Z80__ED__NEG__0XED__0X5C:		/* 0xed 0x5c */
			Z80_NEG;
			break;

		case Z80__ED__RETI__0XED__0X5D:		/* 0xed 0x5d */
			Z80__RETI;
			Z80_DONT_UPDATE_PC;
			break;

		case Z80__ED__IM__2:					/* 0xed 0x5e */
			m_interruptMode = 2;
			break;

		case Z80__ED__LD__A__R:
			Z80__LD__REG8__REG8(m_registers.a, m_registers.r);
			Z80_FLAG_S_DEFAULTBEHAVIOUR;
			Z80_FLAG_Z_DEFAULTBEHAVIOUR;
			Z80_FLAG_H_CLEAR;
			Z80_FLAG_P_UPDATE(isEvenParity(m_registers.a));
			Z80_FLAG_N_CLEAR;
			break;

		case Z80__ED__IN__H__INDIRECT_C:	/* 0xed 0x60 */
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__OUT__INDIRECT_C__H:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__SBC__HL__HL:			/* 0xed 0x62 */
			Z80__SBC__REG16__REG16(m_registers.hl, m_registers.hl);
			break;

		case Z80__ED__LD__INDIRECT_NN__HL:
			Z80__LD__INDIRECT_NN__REG16(Z80::Z80::z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))), m_registers.hl);
			break;

		case Z80__ED__NEG__0XED__0X64:	/* 0xed 0x64 */
			Z80_NEG;
			break;

		case Z80__ED__RETN__0XED__0X65:	/* 0xed 0x65 */
			Z80__RETN;
			Z80_DONT_UPDATE_PC;
			break;

		case Z80__ED__IM__0__0XED__0X66:		/* 0xed 0x66 */
			/* non-standard instruction; not guaranteed that this is the instruction
			 * in all versions of the Z80 */
			m_interruptMode = 0;
			break;

		case Z80__ED__RRD:
			/* FLAGS: H and N cleared, C preserved, P is parity, S and Z as defined */
			{
				UnsignedByte v = peekUnsigned(m_registers.hl);

				/* cache least sig. nybble of A */
				UnsignedByte tmp = (m_registers.a) & 0x0f;

				/* copy least sig. nybble of (HL) into least sig. nybble of A */
				(m_registers.a) &= 0xf0;
				(m_registers.a) |= (v & 0x0f);

				/* copy cached least sig. nybble of A into most sig. nybble of (HL)
				 * and most sig. nybble of (HL) into least sig. nybble of (HL) */
				v >>= 4;
				v |= (tmp << 4);
				pokeUnsigned(m_registers.hl, v);

				Z80_FLAG_H_CLEAR;
				Z80_FLAG_N_CLEAR;
				/* should this be set according to A or both A and (HL) ? */
				Z80_FLAG_P_UPDATE(isEvenParity(m_registers.a));
				Z80_FLAG_Z_DEFAULTBEHAVIOUR;
				Z80_FLAG_S_DEFAULTBEHAVIOUR;
			}
			break;

		case Z80__ED__IN__L__INDIRECT_C:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__OUT__INDIRECT_C__L:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__ADC__HL__HL:
			Z80__ADC__REG16__REG16(m_registers.hl, m_registers.hl);
			break;

		case Z80__ED__LD__HL__INDIRECT_NN:
			Z80__LD__REG16__INDIRECT_NN(m_registers.hl, Z80::Z80::z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))));
			break;

		case Z80__ED__NEG__0XED__0X6C:	/* 0xed 0x6c */
			Z80_NEG;
			break;

		case Z80__ED__RETI__0XED__0X6D:	/* 0xed 0x6e */
			Z80__RETI;
			Z80_DONT_UPDATE_PC;
			break;

		case Z80__ED__IM__0__0XED__0X6E:		/* 0xed 0x6e */
			/* non-standard instruction; not guaranteed that this is the instruction
			 * in all versions of the Z80 */
			m_interruptMode = 0;
			break;

		case Z80__ED__RLD:
			/* FLAGS: H and N cleared, C preserved, P is parity, S and Z as defined */
			{
				UnsignedByte v = peekUnsigned(m_registers.hl);

				/* cache least sig. nybble of (HL) */
				UnsignedByte tmp = v & 0x0f;

				/* copy least sig. nybble of A into least sig. nybble of (HL) */
				v &= 0xf0;
				v |= ((m_registers.a) & 0x0f);

				/* copy cached least sig. nybble of (HL) into most sig. nybble of (HL)
				 * and most sig. nybble of (HL) into least sig. nybble of A */
				(m_registers.a) &= 0xf0;
				(m_registers.a) |= ((v & 0xf0) >> 4);
				v = (v & 0x0f) | (tmp << 4);

				pokeUnsigned(m_registers.hl, v);

				Z80_FLAG_H_CLEAR;
				Z80_FLAG_N_CLEAR;
				/* should this be set according to A or both A and (HL) ? */
				Z80_FLAG_P_UPDATE(isEvenParity(m_registers.a));
				Z80_FLAG_Z_DEFAULTBEHAVIOUR;
				Z80_FLAG_S_DEFAULTBEHAVIOUR;
			}
			break;

		case Z80__ED__IN__INDIRECT_C:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__OUT__INDIRECT_C__0:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__SBC__HL__SP:
			Z80__SBC__REG16__REG16(m_registers.hl, m_registers.sp);
			break;

		case Z80__ED__LD__INDIRECT_NN__SP:
			Z80__LD__INDIRECT_NN__REG16(Z80::Z80::z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))), m_registers.sp);
			break;

		case Z80__ED__NEG__0XED__0X74:	/* oxed 0x74 */
			Z80_NEG;
			break;

		case Z80__ED__RETN__0XED__0X75:	/* 0xed 0x75 */
			Z80__RETN;
			Z80_DONT_UPDATE_PC;
			break;

		case Z80__ED__IM__1__0XED__0X76:		/* 0xed 0x76 */
			/* non-standard instruction; not guaranteed that this is the instruction
			 * in all versions of the Z80 */
			m_interruptMode = 1;
			break;

		case Z80__ED__NOP__0XED__0x77:	/* 0xed 0x77 */
			break;

		case Z80__ED__IN__A__INDIRECT_C:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__OUT__INDIRECT_C__A:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__ADC__HL__SP:
			Z80__ADC__REG16__REG16(m_registers.hl, m_registers.sp);
			break;

		case Z80__ED__LD__SP__INDIRECT_NN:
			Z80__LD__REG16__INDIRECT_NN(m_registers.sp, Z80::Z80::z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))));
			break;

		case Z80__ED__NEG__0XED__0X7C:		/* 0xed 0x7c */
			Z80_NEG;
			break;

		case Z80__ED__RETI__0XED__0X7D:		/* 0xed 0x7d */
			Z80__RETI;
			Z80_DONT_UPDATE_PC;
			break;

		case Z80__ED__IM__2__0XED__0X7E:		/* 0xed 0x7e */
			/* non-standard instruction; not guaranteed that this is the instruction
			 * in all versions of the Z80 */
			m_interruptMode = 2;
			break;

		case Z80__ED__NOP__0XED__0X7F:
			break;

		case Z80__ED__NOP__0XED__0X80:
			break;

		case Z80__ED__NOP__0XED__0X81:
			break;

		case Z80__ED__NOP__0XED__0X82:
			break;

		case Z80__ED__NOP__0XED__0X83:
			break;

		case Z80__ED__NOP__0XED__0X84:
			break;

		case Z80__ED__NOP__0XED__0X85:
			break;

		case Z80__ED__NOP__0XED__0X86:
			break;

		case Z80__ED__NOP__0XED__0X87:
			break;

		case Z80__ED__NOP__0XED__0X88:
			break;

		case Z80__ED__NOP__0XED__0X89:
			break;

		case Z80__ED__NOP__0XED__0X8A:
			break;

		case Z80__ED__NOP__0XED__0X8B:
			break;

		case Z80__ED__NOP__0XED__0X8C:
			break;

		case Z80__ED__NOP__0XED__0X8D:
			break;

		case Z80__ED__NOP__0XED__0X8E:
			break;

		case Z80__ED__NOP__0XED__0X8F:
			break;

		case Z80__ED__NOP__0XED__0X90:
			break;

		case Z80__ED__NOP__0XED__0X91:
			break;

		case Z80__ED__NOP__0XED__0X92:
			break;

		case Z80__ED__NOP__0XED__0X93:
			break;

		case Z80__ED__NOP__0XED__0X94:
			break;

		case Z80__ED__NOP__0XED__0X95:
			break;

		case Z80__ED__NOP__0XED__0X96:
			break;

		case Z80__ED__NOP__0XED__0X97:
			break;

		case Z80__ED__NOP__0XED__0X98:
			break;

		case Z80__ED__NOP__0XED__0X99:
			break;

		case Z80__ED__NOP__0XED__0X9A:
			break;

		case Z80__ED__NOP__0XED__0X9B:
			break;

		case Z80__ED__NOP__0XED__0X9C:
			break;

		case Z80__ED__NOP__0XED__0X9D:
			break;

		case Z80__ED__NOP__0XED__0X9E:
			break;

		case Z80__ED__NOP__0XED__0X9F:
			break;

		case Z80__ED__LDI:
			/* FLAGS: H and N are cleared, S, Z and C are preserved, P is unknown */
			pokeUnsigned(m_registers.de, peekUnsigned(m_registers.hl));
			m_registers.de++;
			m_registers.hl++;
			m_registers.bc--;
			Z80_FLAG_H_CLEAR;
			Z80_FLAG_N_CLEAR;
			/* TODO flag P */
			break;

		case Z80__ED__CPI:
			{
				/* FLAGS: N is set, C is unmodified, others by definition
				 *
				 * this is identical to CP instruction, except C is unmodified and
				 * P is currently unknown */
				bool flagC = Z80_FLAG_C_ISSET;
				Z80__CP__INDIRECT_REG16(m_registers.hl);

				m_registers.hl++;
				m_registers.bc--;

				/* TODO not sure if this is the correct P flag behaviour */
				Z80_FLAG_C_UPDATE(flagC);
				Z80_FLAG_P_UPDATE(isEvenParity(m_registers.a));
			}
			break;

		case Z80__ED__INI:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__OUTI:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__NOP__0XED__0XA4:
			break;

		case Z80__ED__NOP__0XED__0XA5:
			break;

		case Z80__ED__NOP__0XED__0XA6:
			break;

		case Z80__ED__NOP__0XED__0XA7:
			break;

		case Z80__ED__LDD:
			/* FLAGS: H and N are cleared, S, Z and C are preserved, P is unknown */
			pokeUnsigned(m_registers.de, peekUnsigned(m_registers.hl));
			m_registers.de--;
			m_registers.hl--;
			m_registers.bc--;
			Z80_FLAG_H_CLEAR;
			Z80_FLAG_N_CLEAR;
			/* TODO flag P */
			break;

		case Z80__ED__CPD:
			{
				/* FLAGS: N is set, C is unmodified, others by definition
				 *
				 * this is identical to CP instruction, except C is unmodified and
				 * P is currently unknown */
				bool flagC = Z80_FLAG_C_ISSET;
				Z80__CP__INDIRECT_REG16(m_registers.hl);

				m_registers.hl--;
				m_registers.bc--;

				/* TODO not sure if this is the correct P flag behaviour */
				Z80_FLAG_C_UPDATE(flagC);
				Z80_FLAG_P_UPDATE(isEvenParity(m_registers.a));
			}
			break;

		case Z80__ED__IND:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__OUTD:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__NOP__0XED__0XAC:
			break;

		case Z80__ED__NOP__0XED__0XAD:
			break;

		case Z80__ED__NOP__0XED__0XAE:
			break;

		case Z80__ED__NOP__0XED__0XAF:
			break;

		case Z80__ED__LDIR:
			/* TODO cycle cost is dependent on loop iterations ? */
			/* TODO interrupts can occur while this instruction is processing */
			/* FLAGS: H, P and N are cleared, S, Z and C are preserved */
			do {
				pokeUnsigned(m_registers.de, peekUnsigned(m_registers.hl));
				m_registers.de++;
				m_registers.hl++;
			} while(--m_registers.bc);

			Z80_FLAG_H_CLEAR;
			Z80_FLAG_N_CLEAR;
			Z80_FLAG_P_CLEAR;
			break;

		case Z80__ED__CPIR:
			/* TODO cycle cost is dependent on loop iterations ? */
			/* TODO interrupts can occur while this instruction is processing */
			/* FLAGS: N is set, C is unmodified, others by definition
			 *
			 * this is identical to CP instruction, except C is unmodified and
			 * P is currently unknown */
			{
				bool flagC = Z80_FLAG_C_ISSET;

				do {
					Z80__CP__INDIRECT_REG16(m_registers.hl);
					m_registers.hl++;
				} while(--m_registers.bc);

				/* TODO not sure if this is the correct P flag behaviour */
				Z80_FLAG_C_UPDATE(flagC);
				Z80_FLAG_P_UPDATE(isEvenParity(m_registers.a));
			}
			break;

		case Z80__ED__INIR:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__OTIR:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__NOP__0XED__0XB4:
			break;

		case Z80__ED__NOP__0XED__0XB5:
			break;

		case Z80__ED__NOP__0XED__0XB6:
			break;

		case Z80__ED__NOP__0XED__0XB7:
			break;

		case Z80__ED__LDDR:
			/* TODO cycle cost is dependent on loop iterations ? */
			/* TODO interrupts can occur while this instruction is processing */
			/* FLAGS: H, P and N are cleared, S, Z and C are preserved */
			do {
				pokeUnsigned(m_registers.de, peekUnsigned(m_registers.hl));
				m_registers.de--;
				m_registers.hl--;
			} while(--m_registers.bc);

			Z80_FLAG_H_CLEAR;
			Z80_FLAG_N_CLEAR;
			Z80_FLAG_P_CLEAR;
			break;

		case Z80__ED__CPDR:
			/* TODO cycle cost is dependent on loop iterations ? */
			/* TODO interrupts can occur while this instruction is processing */
			/* FLAGS: N is set, C is unmodified, others by definition
			 *
			 * this is identical to CP instruction, except C is unmodified and
			 * P is currently unknown */
			{
				bool flagC = Z80_FLAG_C_ISSET;

				do {
					Z80__CP__INDIRECT_REG16(m_registers.hl);
					m_registers.hl--;
				} while(--m_registers.bc);

				/* TODO not sure if this is the correct P flag behaviour */
				Z80_FLAG_C_UPDATE(flagC);
				Z80_FLAG_P_UPDATE(isEvenParity(m_registers.a));
			}
			break;

		case Z80__ED__INDR:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__OTDR:
			/* FLAGS: all preserved */
			/* TODO */
			break;

		case Z80__ED__NOP__0XED__0XBC:
			break;

		case Z80__ED__NOP__0XED__0XBD:
			break;

		case Z80__ED__NOP__0XED__0XBE:
			break;

		case Z80__ED__NOP__0XED__0XBF:
			break;

		case Z80__ED__NOP__0XED__0XC0:
			break;

		case Z80__ED__NOP__0XED__0XC1:
			break;

		case Z80__ED__NOP__0XED__0XC2:
			break;

		case Z80__ED__NOP__0XED__0XC3:
			break;

		case Z80__ED__NOP__0XED__0XC4:
			break;

		case Z80__ED__NOP__0XED__0XC5:
			break;

		case Z80__ED__NOP__0XED__0XC6:
			break;

		case Z80__ED__NOP__0XED__0XC7:
			break;

		case Z80__ED__NOP__0XED__0XC8:
			break;

		case Z80__ED__NOP__0XED__0XC9:
			break;

		case Z80__ED__NOP__0XED__0XCA:
			break;

		case Z80__ED__NOP__0XED__0XCB:
			break;

		case Z80__ED__NOP__0XED__0XCC:
			break;

		case Z80__ED__NOP__0XED__0XCD:
			break;

		case Z80__ED__NOP__0XED__0XCE:
			break;

		case Z80__ED__NOP__0XED__0XCF:
			break;

		case Z80__ED__NOP__0XED__0XD0:
			break;

		case Z80__ED__NOP__0XED__0XD1:
			break;

		case Z80__ED__NOP__0XED__0XD2:
			break;

		case Z80__ED__NOP__0XED__0XD3:
			break;

		case Z80__ED__NOP__0XED__0XD4:
			break;

		case Z80__ED__NOP__0XED__0XD5:
			break;

		case Z80__ED__NOP__0XED__0XD6:
			break;

		case Z80__ED__NOP__0XED__0XD7:
			break;

		case Z80__ED__NOP__0XED__0XD8:
			break;

		case Z80__ED__NOP__0XED__0XD9:
			break;

		case Z80__ED__NOP__0XED__0XDA:
			break;

		case Z80__ED__NOP__0XED__0XDB:
			break;

		case Z80__ED__NOP__0XED__0XDC:
			break;

		case Z80__ED__NOP__0XED__0XDD:
			break;

		case Z80__ED__NOP__0XED__0XDE:
			break;

		case Z80__ED__NOP__0XED__0XDF:
			break;

		case Z80__ED__NOP__0XED__0XE0:
			break;

		case Z80__ED__NOP__0XED__0XE1:
			break;

		case Z80__ED__NOP__0XED__0XE2:
			break;

		case Z80__ED__NOP__0XED__0XE3:
			break;

		case Z80__ED__NOP__0XED__0XE4:
			break;

		case Z80__ED__NOP__0XED__0XE5:
			break;

		case Z80__ED__NOP__0XED__0XE6:
			break;

		case Z80__ED__NOP__0XED__0XE7:
			break;

		case Z80__ED__NOP__0XED__0XE8:
			break;

		case Z80__ED__NOP__0XED__0XE9:
			break;

		case Z80__ED__NOP__0XED__0XEA:
			break;

		case Z80__ED__NOP__0XED__0XEB:
			break;

		case Z80__ED__NOP__0XED__0XEC:
			break;

		case Z80__ED__NOP__0XED__0XED:
			break;

		case Z80__ED__NOP__0XED__0XEE:
			break;

		case Z80__ED__NOP__0XED__0XEF:
			break;

		case Z80__ED__NOP__0XED__0XF0:
			break;

		case Z80__ED__NOP__0XED__0XF1:
			break;

		case Z80__ED__NOP__0XED__0XF2:
			break;

		case Z80__ED__NOP__0XED__0XF3:
			break;

		case Z80__ED__NOP__0XED__0XF4:
			break;

		case Z80__ED__NOP__0XED__0XF5:
			break;

		case Z80__ED__NOP__0XED__0XF6:
			break;

		case Z80__ED__NOP__0XED__0XF7:
			break;

		case Z80__ED__NOP__0XED__0XF8:
			break;

		case Z80__ED__NOP__0XED__0XF9:
			break;

		case Z80__ED__NOP__0XED__0XFA:
			break;

		case Z80__ED__NOP__0XED__0XFB:
			break;

		case Z80__ED__NOP__0XED__0XFC:
			break;

		case Z80__ED__NOP__0XED__0XFD:
			break;

		case Z80__ED__NOP__0XED__0XFE:
			break;

		case Z80__ED__NOP__0XED__0XFF:
			break;

		case Z80__ED__LD__I__A:				/* 0xed 0x47 */
			Z80__LD__REG8__REG8(m_registers.i, m_registers.a);
			/* Z80__LD__REG8__REG8() preserves all flags because all but 2 8-bit
			 * LD instructions don't alter any flags. LD I,A and LD R,A both alter
			 * flags thus:
			 * C is preserved, H and N are reset, Z and S are flipped(?) and P is
			 * set if interrupts are enabled
			 *
			 * This means we do the flags manually here.
			 */
			Z80_FLAG_N_CLEAR;
			Z80_FLAG_H_CLEAR;
			Z80_FLAG_Z_UPDATE(!Z80_FLAG_Z_ISSET);
			Z80_FLAG_S_UPDATE(!Z80_FLAG_S_ISSET);
			Z80_FLAG_P_UPDATE(m_iff1);
			break;

		case Z80__ED__LD__R__A:				/* 0xed 0x4f */
			Z80__LD__REG8__REG8(m_registers.r, m_registers.a);
			/* Z80__LD__REG8__REG8() preserves all flags because all but 2 8-bit
			 * LD instructions don't alter any flags. LD I,A and LD R,A both alter
			 * flags thus:
			 * C is preserved, H and N are reset, Z and S are flipped(?) and P is
			 * set if interrupts are enabled
			 *
			 * This means we do the flags manually here.
			 */
			Z80_FLAG_N_CLEAR;
			Z80_FLAG_H_CLEAR;
			Z80_FLAG_Z_UPDATE(!Z80_FLAG_Z_ISSET);
			Z80_FLAG_S_UPDATE(!Z80_FLAG_S_ISSET);
			Z80_FLAG_P_UPDATE(m_iff1);
			break;

		default:
			std::cerr << "unhandled opcode: 0xed 0x" << std::hex << (*instruction) << std::endl;
			Z80_INVALID_INSTRUCTION;
			break;
	}

	if (cycles) {
	    *cycles = (useJumpCycleCost ? Z80_CYCLES_JUMP(m_ed_opcode_cycles[*instruction]) : Z80_CYCLES_NOJUMP(m_ed_opcode_cycles[*instruction]));
	}

	if (size) {
	    *size = m_ed_opcode_size[*instruction];
	}

	return true;
}


bool Z80::Z80::executeDdOrFdInstruction(Z80::Z80::UnsignedWord & reg, const Z80::Z80::UnsignedByte * instruction, bool * doPc, int * cycles, int * size)
{
	Z80_UNUSED(doPc);
	bool useJumpCycleCost = false;

	// work out which high and low reg pointers to use
	Z80::Z80::UnsignedByte * regHigh;
	Z80::Z80::UnsignedByte * regLow;

	if (&reg == &m_registers.ix) {
		regHigh = &(m_registers.ixh);
		regLow = &(m_registers.ixl);
	} else {
		regHigh = &(m_registers.iyh);
		regLow = &(m_registers.iyl);
	}

	switch (*instruction) {
		case Z80__DD_OR_FD__ADD__IX_OR_IY__BC: /*  0x09 */
			Z80__ADD__REG16__REG16(reg, m_registers.bc);
			break;

		case Z80__DD_OR_FD__ADD__IX_OR_IY__DE: /*  0x19 */
			Z80__ADD__REG16__REG16(reg, m_registers.de);
			break;

		case Z80__DD_OR_FD__LD__IX_OR_IY__NN: /*  0x21 */
			Z80__LD__REG16__NN(reg, Z80::Z80::z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))));
			break;

		case Z80__DD_OR_FD__LD__INDIRECT_NN__IX_OR_IY: /*  0x22 */
			Z80__LD__INDIRECT_NN__REG16(Z80::Z80::z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))), reg);
			break;

		case Z80__DD_OR_FD__INC__IX_OR_IY: /*  0x23 */
			Z80__INC__REG16(reg);
			break;

		case Z80__DD_OR_FD__INC__IXH_OR_IYH: /*  0x24 */
			Z80__INC__REG8(*regHigh);
			break;

		case Z80__DD_OR_FD__DEC__IXH_OR_IYH: /*  0x25 */
			Z80__DEC__REG8(*regHigh);
			break;

		case Z80__DD_OR_FD__LD__IXH_OR_IYH__N: /*  0x26 */
			Z80__LD__REG8__N(*regHigh, *(instruction + 1));
			break;

		case Z80__DD_OR_FD__ADD__IX_OR_IY__IX_OR_IY: /*  0x29 */
			Z80__ADD__REG16__REG16(reg, reg);
			break;

		case Z80__DD_OR_FD__LD__IX_OR_IY__INDIRECT_NN: /*  0x2a */
			Z80__LD__REG16__INDIRECT_NN(reg, Z80::Z80::z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))));
			break;

		case Z80__DD_OR_FD__DEC__IX_OR_IY: /*  0x2b */
			Z80__DEC__REG16(reg);
			break;

		case Z80__DD_OR_FD__INC__IXL_OR_IYL: /*  0x2c */
			Z80__INC__REG8(*regLow);
			break;

		case Z80__DD_OR_FD__DEC__IXL_OR_IYL: /*  0x2d */
			Z80__DEC__REG8(*regLow);
			break;

		case Z80__DD_OR_FD__LD__IXL_OR_IYL__N: /*  0x2e */
			Z80__LD__REG8__N(*regLow, *(instruction + 1));
			break;

		case Z80__DD_OR_FD__INC__INDIRECT_IX_d_OR_IY_d: /*  0x34 */
			Z80__INC__INDIRECT_REG16_D(reg, SignedByte(*(instruction + 1)));
			break;

		case Z80__DD_OR_FD__DEC__INDIRECT_IX_d_OR_IY_d: /*  0x35 */
			Z80__DEC__INDIRECT_REG16_D(reg, SignedByte(*(instruction + 1)));
			break;

		case Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__N: /*  0x36 */
			Z80__LD__INDIRECT_REG16_D__N(reg, SignedByte(*(instruction + 1)), *(instruction + 2));
			break;

		case Z80__DD_OR_FD__ADD__IX_OR_IY__SP: /*  0x39 */
			Z80__ADD__REG16__REG16(reg, m_registers.sp);
			break;

		case Z80__DD_OR_FD__LD__B__IXH_OR_IYH: /*  0x44 */
			Z80__LD__REG8__REG8(m_registers.b, *regHigh);
			break;

		case Z80__DD_OR_FD__LD__B__IXL_OR_IYL: /*  0x45 */
			Z80__LD__REG8__REG8(m_registers.b, *regLow);
			break;

		case Z80__DD_OR_FD__LD__B__INDIRECT_IX_d_OR_IY_d: /*  0x46 */
			Z80__LD__REG8__INDIRECT_REG16_D(m_registers.b, reg, SignedByte(*(instruction + 1)));
			break;

		case Z80__DD_OR_FD__LD__C__IXH_OR_IYH: /*  0x4c */
			Z80__LD__REG8__REG8(m_registers.c, *regHigh);
			break;


		case Z80__DD_OR_FD__LD__C__IXL_OR_IYL: /*  0x4d */
			Z80__LD__REG8__REG8(m_registers.c, *regLow);
			break;


		case Z80__DD_OR_FD__LD__C__INDIRECT_IX_d_OR_IY_d: /*  0x4e */
			Z80__LD__REG8__INDIRECT_REG16_D(m_registers.c, reg, SignedByte(*(instruction + 1)));
			break;

		case Z80__DD_OR_FD__LD__D__IXH_OR_IYH: /*  0x54 */
			Z80__LD__REG8__REG8(m_registers.d, *regHigh);
			break;


		case Z80__DD_OR_FD__LD__D__IXL_OR_IYL: /*  0x55 */
			Z80__LD__REG8__REG8(m_registers.d, *regLow);
			break;


		case Z80__DD_OR_FD__LD__D__INDIRECT_IX_d_OR_IY_d: /*  0x56 */
			Z80__LD__REG8__INDIRECT_REG16_D(m_registers.d, reg, SignedByte(*(instruction + 1)));
			break;

		case Z80__DD_OR_FD__LD__E__IXH_OR_IYH: /*  0x5c */
			Z80__LD__REG8__REG8(m_registers.e, *regHigh);
			break;


		case Z80__DD_OR_FD__LD__E__IXL_OR_IYL: /*  0x5d */
			Z80__LD__REG8__REG8(m_registers.e, *regLow);
			break;

		case Z80__DD_OR_FD__LD__E__INDIRECT_IX_d_OR_IY_d: /*  0x5e */
			Z80__LD__REG8__INDIRECT_REG16_D(m_registers.e, reg, SignedByte(*(instruction + 1)));
			break;

		case Z80__DD_OR_FD__LD__IXH_OR_IYH__B: /*  0x60 */
			Z80__LD__REG8__REG8(*regHigh, m_registers.b);
			break;

		case Z80__DD_OR_FD__LD__IXH_OR_IYH__C: /*  0x61 */
			Z80__LD__REG8__REG8(*regHigh, m_registers.c);
			break;

		case Z80__DD_OR_FD__LD__IXH_OR_IYH__D: /*  0x62 */
			Z80__LD__REG8__REG8(*regHigh, m_registers.d);
			break;

		case Z80__DD_OR_FD__LD__IXH_OR_IYH__E: /*  0x63 */
			Z80__LD__REG8__REG8(*regHigh, m_registers.e);
			break;

		case Z80__DD_OR_FD__LD__IXH_OR_IYH__IXH_OR_IYH: /*  0x64 */
			/* TODO if there are no implications for flags, can skip this */
			Z80__LD__REG8__REG8(*regHigh, *regHigh);
			break;

		case Z80__DD_OR_FD__LD__IXH_OR_IYH__IXL_OR_IYL: /*  0x65 */
			Z80__LD__REG8__REG8(*regHigh, *regLow);
			break;

		case Z80__DD_OR_FD__LD__H__INDIRECT_IX_d_OR_IY_d: /*  0x66 */
			Z80__LD__REG8__INDIRECT_REG16_D(m_registers.h, reg, SignedByte(*(instruction + 1)));
			break;

		case Z80__DD_OR_FD__LD__IXH_OR_IYH__A: /*  0x67 */
			Z80__LD__REG8__REG8(*regHigh, m_registers.a);
			break;

		case Z80__DD_OR_FD__LD__IXL_OR_IYL__B: /*  0x68 */
			Z80__LD__REG8__REG8(*regLow, m_registers.b);
			break;

		case Z80__DD_OR_FD__LD__IXL_OR_IYL__C: /*  0x69 */
			Z80__LD__REG8__REG8(*regLow, m_registers.c);
			break;

		case Z80__DD_OR_FD__LD__IXL_OR_IYL__D: /*  0x6a */
			Z80__LD__REG8__REG8(*regLow, m_registers.d);
			break;

		case Z80__DD_OR_FD__LD__IXL_OR_IYL__E: /*  0x6b */
			Z80__LD__REG8__REG8(*regLow, m_registers.e);
			break;

		case Z80__DD_OR_FD__LD__IXL_OR_IYL__IXH_OR_IYH: /*  0x6c */
			Z80__LD__REG8__REG8(*regLow, *regHigh);
			break;

		case Z80__DD_OR_FD__LD__IXL_OR_IYL__IXL_OR_IYL: /*  0x6d */
			/* TODO if there are no implications for flags, can skip this */
			Z80__LD__REG8__REG8(*regLow, *regLow);
			break;

		case Z80__DD_OR_FD__LD__L__INDIRECT_IX_d_OR_IY_d: /*  0x6e */
			Z80__LD__REG8__INDIRECT_REG16_D(m_registers.l, reg, SignedByte(*(instruction + 1)));
			break;

		case Z80__DD_OR_FD__LD__IXL_OR_IYL__A: /*  0x6f */
			Z80__LD__REG8__REG8(*regLow, m_registers.a);
			break;

		case Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__B: /*  0x70 */
			Z80__LD__INDIRECT_REG16_D__REG8(reg, SignedByte(*(instruction + 1)), m_registers.b);
			break;

		case Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__C: /*  0x71 */
			Z80__LD__INDIRECT_REG16_D__REG8(reg, SignedByte(*(instruction + 1)), m_registers.c);
			break;

		case Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__D: /*  0x72 */
			Z80__LD__INDIRECT_REG16_D__REG8(reg, SignedByte(*(instruction + 1)), m_registers.d);
			break;

		case Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__E: /*  0x73 */
			Z80__LD__INDIRECT_REG16_D__REG8(reg, SignedByte(*(instruction + 1)), m_registers.e);
			break;

		case Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__H: /*  0x74 */
			Z80__LD__INDIRECT_REG16_D__REG8(reg, SignedByte(*(instruction + 1)), m_registers.h);
			break;

		case Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__L: /*  0x75 */
			Z80__LD__INDIRECT_REG16_D__REG8(reg, SignedByte(*(instruction + 1)), m_registers.l);
			break;

		case Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__A: /*  0x77 */
			Z80__LD__INDIRECT_REG16_D__REG8(reg, SignedByte(*(instruction + 1)), m_registers.a);
			break;

		case Z80__DD_OR_FD__LD__A__IXH_OR_IYH: /*  0x7c */
			Z80__LD__REG8__REG8(m_registers.a, *regHigh);
			break;

		case Z80__DD_OR_FD__LD__A__IXL_OR_IYL: /*  0x7d */
			Z80__LD__REG8__REG8(m_registers.a, *regLow);
			break;

		case Z80__DD_OR_FD__LD__A__INDIRECT_IX_d_OR_IY_d: /*  0x7e */
			Z80__LD__REG8__INDIRECT_REG16_D(m_registers.a, reg, SignedByte(*(instruction + 1)));
			break;

		case Z80__DD_OR_FD__ADD__A__IXH_OR_IYH: /*  0x84 */
			Z80__ADD__REG8__REG8(m_registers.a, *regHigh);
			break;

		case Z80__DD_OR_FD__ADD__A__IXL_OR_IYL: /*  0x85 */
			Z80__ADD__REG8__REG8(m_registers.a, *regLow);
			break;

		case Z80__DD_OR_FD__ADD__A__INDIRECT_IX_d_OR_IY_d: /*  0x86 */
			Z80__ADD__REG8__INDIRECT_REG16_D(m_registers.a, reg, SignedByte(*(instruction + 1)));
			break;

		case Z80__DD_OR_FD__ADC__A__IXH_OR_IYH: /*  0x8c */
			Z80__ADC__REG8__REG8(m_registers.a, *regHigh);
			break;

		case Z80__DD_OR_FD__ADC__A__IXL_OR_IYL: /*  0x8d */
			Z80__ADC__REG8__REG8(m_registers.a, *regLow);
			break;

		case Z80__DD_OR_FD__ADC__A__INDIRECT_IX_d_OR_IY_d: /*  0x8e */
			Z80__ADC__REG8__INDRIECT_REG16_D(m_registers.a, reg, SignedByte(*(instruction + 1)));
			break;

		case Z80__DD_OR_FD__SUB__IXH_OR_IYH: /*  0x94 */
			Z80__SUB__REG8(*regHigh);
			break;

		case Z80__DD_OR_FD__SUB__IXL_OR_IYL: /*  0x95 */
			Z80__SUB__REG8(*regLow);
			break;

		case Z80__DD_OR_FD__SUB__INDIRECT_IX_d_OR_IY_d: /*  0x96 */
			Z80__SUB__INDIRECT_REG16_D(reg, SignedByte(*(instruction + 1)));
			break;

		case Z80__DD_OR_FD__SBC__A__IXH_OR_IYH: /*  0x9c */
			Z80__SBC__REG8__REG8(m_registers.a, *regHigh);
			break;

		case Z80__DD_OR_FD__SBC__A__IXL_OR_IYL: /*  0x9d */
			Z80__SBC__REG8__REG8(m_registers.a, *regLow);
			break;

		case Z80__DD_OR_FD__SBC__A__INDIRECT_IX_d_OR_IY_d: /*  0x9e */
			Z80__SBC__REG8__INDIRECT_REG16_D(m_registers.a, reg, SignedByte(*(instruction + 1)));
			break;

		case Z80__DD_OR_FD__AND__IXH_OR_IYH: /*  0xa4 */
			Z80__AND__REG8(*regHigh);
			break;

		case Z80__DD_OR_FD__AND__IXL_OR_IYL: /*  0xa5 */
			Z80__AND__REG8(*regLow);
			break;

		case Z80__DD_OR_FD__AND__INDIRECT_IX_d_OR_IY_d: /*  0xa6 */
			Z80__AND__INDIRECT_REG16_D(reg, SignedByte(*(instruction + 1)));
			break;

		case Z80__DD_OR_FD__XOR__IXH_OR_IYH: /*  0xac */
			Z80__XOR__REG8(*regHigh);
			break;

		case Z80__DD_OR_FD__XOR__IXL_OR_IYL: /*  0xad */
			Z80__XOR__REG8(*regLow);
			break;

		case Z80__DD_OR_FD__XOR__INDIRECT_IX_d_OR_IY_d: /*  0xae */
			Z80__XOR__INDIRECT_REG16_D(reg, SignedByte(*(instruction + 1)));
			break;

		case Z80__DD_OR_FD__OR__IXH_OR_IYH: /*  0xb4 */
			Z80__OR__REG8(*regHigh);
			break;

		case Z80__DD_OR_FD__OR__IXL_OR_IYL: /*  0xb5 */
			Z80__OR__REG8(*regLow);
			break;

		case Z80__DD_OR_FD__OR__INDIRECT_IX_d_OR_IY_d: /*  0xb6 */
			Z80__OR__INDIRECT_REG16_D(reg, SignedByte(*(instruction + 1)));
			break;

		case Z80__DD_OR_FD__CP__IXH_OR_IYH: /*  0xbc */
			Z80__CP__REG8(*regHigh);
			break;

		case Z80__DD_OR_FD__CP__IXL_OR_IYL: /*  0xbd */
			Z80__CP__REG8(*regLow);
			break;

		case Z80__DD_OR_FD__CP__INDIRECT_IX_d_OR_IY_d: /*  0xbe */
			Z80__CP__INDIRECT_REG16_D(reg, SignedByte(*(instruction + 1)));
			break;

		case Z80__DD_OR_FD__POP__IX_OR_IY: /*  0xe1 */
			Z80__POP__REG16(reg);
			break;

		case Z80__DD_OR_FD__EX__INDIRECT_SP__IX_OR_IY: /*  0xe3 */
			Z80__EX__REG16__REG16(m_registers.sp, reg);
			break;

		case Z80__DD_OR_FD__JP__INDIRECT_IX_OR_IY: /*  0xe9 */
			m_registers.pc = Z80::Z80::z80ToHostByteOrder(peekUnsignedWord(reg));
			Z80_USE_JUMP_CYCLE_COST;
			Z80_DONT_UPDATE_PC;
			break;

		case Z80__DD_OR_FD__LD__SP__IX_OR_IY: /*  0xf9 */
			Z80__LD__REG16__REG16(m_registers.sp, reg);
			break;

		case Z80__DD_OR_FD__PREFIX__CB: /*  0xcb */
			return executeDdcbOrFdcbInstruction(reg, instruction + 1, cycles, size);

		/* the following are all (expensive) replicas of plain instructions, so
		 * defer to the plain opcode executor method */
		case Z80__DD_OR_FD__NOP: /*  0x00 */
		case Z80__DD_OR_FD__LD__BC__NN: /*  0x01 */
		case Z80__DD_OR_FD__LD__INDIRECT_BC__A: /*  0x02 */
		case Z80__DD_OR_FD__INC__BC: /*  0x03 */
		case Z80__DD_OR_FD__INC__B: /*  0x04 */
		case Z80__DD_OR_FD__DEC__B: /*  0x05 */
		case Z80__DD_OR_FD__LD__B__N: /*  0x06 */
		case Z80__DD_OR_FD__RLCA: /*  0x07 */

		case Z80__DD_OR_FD__EX__AF__AF_SHADOW: /*  0x08 */
		case Z80__DD_OR_FD__LD__A__INDIRECT_BC: /*  0x0a */
		case Z80__DD_OR_FD__DEC__BC: /*  0x0b */
		case Z80__DD_OR_FD__INC__C: /*  0x0c */
		case Z80__DD_OR_FD__DEC__C: /*  0x0d */
		case Z80__DD_OR_FD__LD__C__N: /*  0x0e */
		case Z80__DD_OR_FD__RRCA: /*  0x0f */

		case Z80__DD_OR_FD__DJNZ__d: /*  0x10 */
		case Z80__DD_OR_FD__LD__DE__NN: /*  0x11#define  */
		case Z80__DD_OR_FD__LD__INDIRECT_DE__A: /*  0x12 */
		case Z80__DD_OR_FD__INC__DE: /*  0x13 */
		case Z80__DD_OR_FD__INC__D: /*  0x14 */
		case Z80__DD_OR_FD__DEC__D: /*  0x15 */
		case Z80__DD_OR_FD__LD__D__N: /*  0x16 */
		case Z80__DD_OR_FD__RLA: /*  0x17 */

		case Z80__DD_OR_FD__JR__d: /*  0x18 */
		case Z80__DD_OR_FD__LD__A__INDIRECT_DE: /*  0x1a */
		case Z80__DD_OR_FD__DEC__DE: /*  0x1b */
		case Z80__DD_OR_FD__INC__E: /*  0x1c */
		case Z80__DD_OR_FD__DEC__E: /*  0x1d */
		case Z80__DD_OR_FD__LD__E__N: /*  0x1e */
		case Z80__DD_OR_FD__RRA: /*  0x1f */

		case Z80__DD_OR_FD__JR__NZ__d: /*  0x20 */
		case Z80__DD_OR_FD__DAA: /*  0x27 */

		case Z80__DD_OR_FD__JR__Z__d: /*  0x28 */
		case Z80__DD_OR_FD__CPL: /*  0x2f */

		case Z80__DD_OR_FD__JR__NC__d: /*  0x30 */
		case Z80__DD_OR_FD__LD__SP__NN: /*  0x31 */
		case Z80__DD_OR_FD__LD__INDIRECT_NN__A: /*  0x32 */
		case Z80__DD_OR_FD__INC__SP: /*  0x33 */
		case Z80__DD_OR_FD__SCF: /*  0x37 */

		case Z80__DD_OR_FD__JR__C__d: /*  0x38 */
		case Z80__DD_OR_FD__LD__A__INDIRECT_NN: /*  0x3a */
		case Z80__DD_OR_FD__DEC__SP: /*  0x3b */
		case Z80__DD_OR_FD__INC__A: /*  0x3c */
		case Z80__DD_OR_FD__DEC__A: /*  0x3d */
		case Z80__DD_OR_FD__LD__A__N: /*  0x3e */
		case Z80__DD_OR_FD__CCF: /*  0x3f */

		case Z80__DD_OR_FD__LD__B__B: /*  0x40 */
		case Z80__DD_OR_FD__LD__B__C: /*  0x41 */
		case Z80__DD_OR_FD__LD__B__D: /*  0x42 */
		case Z80__DD_OR_FD__LD__B__E: /*  0x43 */
		case Z80__DD_OR_FD__LD__B__A: /*  0x47 */

		case Z80__DD_OR_FD__LD__C__B: /*  0x48 */
		case Z80__DD_OR_FD__LD__C__C: /*  0x49 */
		case Z80__DD_OR_FD__LD__C__D: /*  0x4a */
		case Z80__DD_OR_FD__LD__C__E: /*  0x4b */
		case Z80__DD_OR_FD__LD__C__A: /*  0x4f */

		case Z80__DD_OR_FD__LD__D__B: /*  0x50 */
		case Z80__DD_OR_FD__LD__D__C: /*  0x51 */
		case Z80__DD_OR_FD__LD__D__D: /*  0x52 */
		case Z80__DD_OR_FD__LD__D__E: /*  0x53 */
		case Z80__DD_OR_FD__LD__D__A: /*  0x57 */

		case Z80__DD_OR_FD__LD__E__B: /*  0x58 */
		case Z80__DD_OR_FD__LD__E__C: /*  0x59 */
		case Z80__DD_OR_FD__LD__E__D: /*  0x5a */
		case Z80__DD_OR_FD__LD__E__E: /*  0x5b */
		case Z80__DD_OR_FD__LD__E__A: /*  0x5f */

		case Z80__DD_OR_FD__HALT: /*  0x76 */

		case Z80__DD_OR_FD__LD__A__B: /*  0x78 */
		case Z80__DD_OR_FD__LD__A__C: /*  0x79 */
		case Z80__DD_OR_FD__LD__A__D: /*  0x7a */
		case Z80__DD_OR_FD__LD__A__E: /*  0x7b */
		case Z80__DD_OR_FD__LD__A__A: /*  0x7f */

		case Z80__DD_OR_FD__ADD__A__B: /*  0x80 */
		case Z80__DD_OR_FD__ADD__A__C: /*  0x81 */
		case Z80__DD_OR_FD__ADD__A__D: /*  0x82 */
		case Z80__DD_OR_FD__ADD__A__E: /*  0x83 */
		case Z80__DD_OR_FD__ADD__A__A: /*  0x87 */

		case Z80__DD_OR_FD__ADC__A__B: /*  0x88 */
		case Z80__DD_OR_FD__ADC__A__C: /*  0x89 */
		case Z80__DD_OR_FD__ADC__A__D: /*  0x8a */
		case Z80__DD_OR_FD__ADC__A__E: /*  0x8b */
		case Z80__DD_OR_FD__ADC__A__A: /*  0x8f */

		case Z80__DD_OR_FD__SUB__B: /*  0x90 */
		case Z80__DD_OR_FD__SUB__C: /*  0x91 */
		case Z80__DD_OR_FD__SUB__D: /*  0x92 */
		case Z80__DD_OR_FD__SUB__E: /*  0x93 */
		case Z80__DD_OR_FD__SUB__A: /*  0x97 */

		case Z80__DD_OR_FD__SBC__A__B: /*  0x98 */
		case Z80__DD_OR_FD__SBC__A__C: /*  0x99 */
		case Z80__DD_OR_FD__SBC__A__D: /*  0x9a */
		case Z80__DD_OR_FD__SBC__A__E: /*  0x9b */
		case Z80__DD_OR_FD__SBC__A__A: /*  0x9f */

		case Z80__DD_OR_FD__AND__B: /*  0xa0 */
		case Z80__DD_OR_FD__AND__C: /*  0xa1 */
		case Z80__DD_OR_FD__AND__D: /*  0xa2 */
		case Z80__DD_OR_FD__AND__E: /*  0xa3 */
		case Z80__DD_OR_FD__AND__A: /*  0xa7 */

		case Z80__DD_OR_FD__XOR__B: /*  0xa8 */
		case Z80__DD_OR_FD__XOR__C: /*  0xa9 */
		case Z80__DD_OR_FD__XOR__D: /*  0xaa */
		case Z80__DD_OR_FD__XOR__E: /*  0xab */
		case Z80__DD_OR_FD__XOR__A: /*  0xaf */

		case Z80__DD_OR_FD__OR__B: /*  0xb0 */
		case Z80__DD_OR_FD__OR__C: /*  0xb1 */
		case Z80__DD_OR_FD__OR__D: /*  0xb2 */
		case Z80__DD_OR_FD__OR__E: /*  0xb3 */
		case Z80__DD_OR_FD__OR__A: /*  0xb7 */

		case Z80__DD_OR_FD__CP__B: /*  0xb8 */
		case Z80__DD_OR_FD__CP__C: /*  0xb9 */
		case Z80__DD_OR_FD__CP__D: /*  0xba */
		case Z80__DD_OR_FD__CP__E: /*  0xbb */
		case Z80__DD_OR_FD__CP__A: /*  0xbf */

		case Z80__DD_OR_FD__RET__NZ: /*  0xc0 */
		case Z80__DD_OR_FD__POP__BC: /*  0xc1 */
		case Z80__DD_OR_FD__JP__NZ__NN: /*  0xc2 */
		case Z80__DD_OR_FD__JP__NN: /*  0xc3 */
		case Z80__DD_OR_FD__CALL__NZ__NN: /*  0xc4 */
		case Z80__DD_OR_FD__PUSH__BC: /*  0xc5 */
		case Z80__DD_OR_FD__ADD__A__N: /*  0xc6 */
		case Z80__DD_OR_FD__RST__00: /*  0xc7 */

		case Z80__DD_OR_FD__RET__Z: /*  0xc8 */
		case Z80__DD_OR_FD__RET: /*  0xc9 */
		case Z80__DD_OR_FD__JP__Z__NN: /*  0xca */
		case Z80__DD_OR_FD__CALL__Z__NN: /*  0xcc */
		case Z80__DD_OR_FD__CALL__NN: /*  0xcd */
		case Z80__DD_OR_FD__ADC__A__N: /*  0xce */
		case Z80__DD_OR_FD__RST__08: /*  0xcf */

		case Z80__DD_OR_FD__RET__NC: /*  0xd0 */
		case Z80__DD_OR_FD__POP__DE: /*  0xd1 */
		case Z80__DD_OR_FD__JP__NC__NN: /*  0xd2 */
		case Z80__DD_OR_FD__OUT__INDIRECT_N__A: /*  0xd3 */
		case Z80__DD_OR_FD__CALL__NC__NN: /*  0xd4 */
		case Z80__DD_OR_FD__PUSH__DE: /*  0xd5 */
		case Z80__DD_OR_FD__SUB__N: /*  0xd6 */
		case Z80__DD_OR_FD__RST__10: /*  0xd7 */

		case Z80__DD_OR_FD__RET__C: /*  0xd8 */
		case Z80__DD_OR_FD__EXX: /*  0xd9 */
		case Z80__DD_OR_FD__JP__C__NN: /*  0xda */
		case Z80__DD_OR_FD__IN__A__INDIRECT_N: /*  0xdb */
		case Z80__DD_OR_FD__CALL__C__NN: /*  0xdc */
		case Z80__DD_OR_FD__PREFIX__DD: /*  0xdd */
		case Z80__DD_OR_FD__SBC__A__N: /*  0xde */
		case Z80__DD_OR_FD__RST__18: /*  0xdf */

		case Z80__DD_OR_FD__RET__PO: /*  0xe0 */
		case Z80__DD_OR_FD__JP__PO__NN: /*  0xe2 */
		case Z80__DD_OR_FD__CALL__PO__NN: /*  0xe4 */
		case Z80__DD_OR_FD__PUSH__IX_OR_IY: /*  0xe5 */
		case Z80__DD_OR_FD__AND__N: /*  0xe6 */
		case Z80__DD_OR_FD__RST__20: /*  0xe7 */

		case Z80__DD_OR_FD__RET__PE: /*  0xe8 */
		case Z80__DD_OR_FD__JP__PE__NN: /*  0xea */
		case Z80__DD_OR_FD__EX__DE__HL: /*  0xeb */
		case Z80__DD_OR_FD__CALL__PE__NN: /*  0xec */
		case Z80__DD_OR_FD__PREFIX__ED: /*  0xed */
		case Z80__DD_OR_FD__XOR__N: /*  0xee */
		case Z80__DD_OR_FD__RST__28: /*  0xef */

		case Z80__DD_OR_FD__RET__P: /*  0xf0 */
		case Z80__DD_OR_FD__POP__AF: /*  0xf1 */
		case Z80__DD_OR_FD__JP__P__NN: /*  0xf2 */
		case Z80__DD_OR_FD__DI: /*  0xf3 */
		case Z80__DD_OR_FD__CALL__P__NN: /*  0xf4 */
		case Z80__DD_OR_FD__PUSH__AF: /*  0xf5 */
		case Z80__DD_OR_FD__OR__N: /*  0xf6 */
		case Z80__DD_OR_FD__RST__30: /*  0xf7 */

		case Z80__DD_OR_FD__RET__M: /*  0xf8 */
		case Z80__DD_OR_FD__JP__M__NN: /*  0xfa */
		case Z80__DD_OR_FD__EI: /*  0xfb */
		case Z80__DD_OR_FD__CALL__M__NN: /*  0xfc */
		case Z80__DD_OR_FD__PREFIX__FD: /*  0xfd */
		case Z80__DD_OR_FD__CP__N: /*  0xfe */
		case Z80__DD_OR_FD__RST__38: /*  0xff */
		{
			/* these are all (expensive) replicas of plain instructions, so defer
			 * to the plain opcode executor method */
			bool ret = executePlainInstruction(instruction + 1, doPc, cycles, size);
			if (size) *size += 1;
			return ret;
		}
	}

	if (cycles) {
	    *cycles = (useJumpCycleCost ? Z80_CYCLES_JUMP(m_ddorfd_opcode_cycles[*instruction]) : Z80_CYCLES_NOJUMP(m_ddorfd_opcode_cycles[*instruction]));
	}

	if (size) {
	    *size = DdOrFdOpcodeSize[*instruction];
	}

	return true;
}


bool Z80::Z80::executeDdcbOrFdcbInstruction(Z80::Z80::UnsignedWord & reg, const Z80::Z80::UnsignedByte * instruction, int * cycles, int * size)
{
	/* NOTE these opcodes are of the form 0xdd 0xcb DD II or 0xfd 0xcb DD II
	 * where II is the opcode and DD is the offset to use with IX or IY */
	SignedByte d(*(instruction));

	switch(*(instruction + 1)) {
		case Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x00 */
			Z80__RLC__INDIRECT_REG16_D__REG8(reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x01 */
			Z80__RLC__INDIRECT_REG16_D__REG8(reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x02 */
			Z80__RLC__INDIRECT_REG16_D__REG8(reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x03 */
			Z80__RLC__INDIRECT_REG16_D__REG8(reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x04 */
			Z80__RLC__INDIRECT_REG16_D__REG8(reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x05 */
			Z80__RLC__INDIRECT_REG16_D__REG8(reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x06 */
			Z80__RLC__INDIRECT_REG16_D(reg, d);
			break;

		case Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x07 */
			Z80__RLC__INDIRECT_REG16_D__REG8(reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x08 */
			Z80__RRC__INDIRECT_REG16_D__REG8(reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x09 */
			Z80__RRC__INDIRECT_REG16_D__REG8(reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x0a */
			Z80__RRC__INDIRECT_REG16_D__REG8(reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x0b */
			Z80__RRC__INDIRECT_REG16_D__REG8(reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x0c */
			Z80__RRC__INDIRECT_REG16_D__REG8(reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x0d */
			Z80__RRC__INDIRECT_REG16_D__REG8(reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x0e */
			Z80__RRC__INDIRECT_REG16_D(reg, d);
			break;

		case Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x0f */
			Z80__RRC__INDIRECT_REG16_D__REG8(reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x10 */
			Z80__RL__INDIRECT_REG16_D__REG8(reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x11 */
			Z80__RL__INDIRECT_REG16_D__REG8(reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x12 */
			Z80__RL__INDIRECT_REG16_D__REG8(reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x13 */
			Z80__RL__INDIRECT_REG16_D__REG8(reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x14 */
			Z80__RL__INDIRECT_REG16_D__REG8(reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x15 */
			Z80__RL__INDIRECT_REG16_D__REG8(reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x16 */
			Z80__RL__INDIRECT_REG16_D(reg, d);
			break;

		case Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x17 */
			Z80__RL__INDIRECT_REG16_D__REG8(reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x18 */
			Z80__RR__INDIRECT_REG16_D__REG8(reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x19 */
			Z80__RR__INDIRECT_REG16_D__REG8(reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x1a */
			Z80__RR__INDIRECT_REG16_D__REG8(reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x1b */
			Z80__RR__INDIRECT_REG16_D__REG8(reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x1c */
			Z80__RR__INDIRECT_REG16_D__REG8(reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x1d */
			Z80__RR__INDIRECT_REG16_D__REG8(reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x1e */
			Z80__RR__INDIRECT_REG16_D(reg, d);
			break;

		case Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x1f */
			Z80__RR__INDIRECT_REG16_D__REG8(reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x21 */
			Z80__SLA__INDIRECT_REG16_D__REG8(reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x22 */
			Z80__SLA__INDIRECT_REG16_D__REG8(reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x23 */
			Z80__SLA__INDIRECT_REG16_D__REG8(reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x24 */
			Z80__SLA__INDIRECT_REG16_D__REG8(reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x25 */
			Z80__SLA__INDIRECT_REG16_D__REG8(reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x26 */
			Z80__SLA__INDIRECT_REG16_D__REG8(reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x26 */
			Z80__SLA__INDIRECT_REG16_D(reg, d);
			break;

		case Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x27 */
			Z80__SLA__INDIRECT_REG16_D__REG8(reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x28 */
			Z80__SRA__INDIRECT_REG16_D__REG8(reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x29 */
			Z80__SRA__INDIRECT_REG16_D__REG8(reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x2a */
			Z80__SRA__INDIRECT_REG16_D__REG8(reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x2b */
			Z80__SRA__INDIRECT_REG16_D__REG8(reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x2c */
			Z80__SRA__INDIRECT_REG16_D__REG8(reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x2d */
			Z80__SRA__INDIRECT_REG16_D__REG8(reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x2e */
			Z80__SRA__INDIRECT_REG16_D(reg, d);
			break;

		case Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x2f */
			Z80__SRA__INDIRECT_REG16_D__REG8(reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x30 */
			Z80__SLL__INDIRECT_REG16_D__REG8(reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x31 */
			Z80__SLL__INDIRECT_REG16_D__REG8(reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x32 */
			Z80__SLL__INDIRECT_REG16_D__REG8(reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x33 */
			Z80__SLL__INDIRECT_REG16_D__REG8(reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x34 */
			Z80__SLL__INDIRECT_REG16_D__REG8(reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x35 */
			Z80__SLL__INDIRECT_REG16_D__REG8(reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x36 */
			Z80__SLL__INDIRECT_REG16_D(reg, d);
			break;

		case Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x37 */
			Z80__SLL__INDIRECT_REG16_D__REG8(reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x38 */
			Z80__SRL__INDIRECT_REG16_D__REG8(reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x39 */
			Z80__SRL__INDIRECT_REG16_D__REG8(reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x3a */
			Z80__SRL__INDIRECT_REG16_D__REG8(reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x3b */
			Z80__SRL__INDIRECT_REG16_D__REG8(reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x3c */
			Z80__SRL__INDIRECT_REG16_D__REG8(reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x3d */
			Z80__SRL__INDIRECT_REG16_D__REG8(reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x3e */
			Z80__SRL__INDIRECT_REG16_D(reg, d);
			break;

		case Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x3f */
			Z80__SRL__INDIRECT_REG16_D__REG8(reg, d, m_registers.a);
			break;

		/* BIT opcodes */
		case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x40 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x41 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x42 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x43 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x44 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x45 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x46 */
			Z80__BIT__N__INDIRECT_REG16_D(0, reg, d);
			break;

		case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x47 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x48 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x49 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x4a */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x4b */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x4c */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x4d */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x4e */
			Z80__BIT__N__INDIRECT_REG16_D(1, reg, d);
			break;

		case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x4f */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x50 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x51 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x52 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x53 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x54 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x55 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x56 */
			Z80__BIT__N__INDIRECT_REG16_D(2, reg, d);
			break;

		case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x57 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x58 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x59 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x5a */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x5b */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x5c */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x5d */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x5e */
			Z80__BIT__N__INDIRECT_REG16_D(3, reg, d);
			break;

		case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x5f */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x60 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x61 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x62 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x63 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x64 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x65 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x66 */
			Z80__BIT__N__INDIRECT_REG16_D(4, reg, d);
			break;

		case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x67 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x68 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x69 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x6a */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x6b */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x6c */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x6d */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x6e */
			Z80__BIT__N__INDIRECT_REG16_D(5, reg, d);
			break;

		case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x6f */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x70 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x71 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x72 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x73 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x74 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x75 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x76 */
			Z80__BIT__N__INDIRECT_REG16_D(6, reg, d);
			break;

		case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x77 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x78 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x79 */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x7a */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x7b */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x7c */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x7d */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x7e */
			Z80__BIT__N__INDIRECT_REG16_D(7, reg, d);
			break;

		case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x7f */
			Z80__BIT__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.a);
			break;

		/* RES opcodes */
		case Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x80 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x81 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x82 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x83 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x84 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x85 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x86 */
			Z80__RES__N__INDIRECT_REG16_D(0, reg, d);
			break;

		case Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x87 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x88 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x89 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x8a */
			Z80__RES__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x8b */
			Z80__RES__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x8c */
			Z80__RES__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x8d */
			Z80__RES__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x8e */
			Z80__RES__N__INDIRECT_REG16_D(1, reg, d);
			break;

		case Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x8f */
			Z80__RES__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x90 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x91 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x92 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x93 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x94 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x95 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x96 */
			Z80__RES__N__INDIRECT_REG16_D(2, reg, d);
			break;

		case Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x97 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x98 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x99 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x9a */
			Z80__RES__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x9b */
			Z80__RES__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x9c */
			Z80__RES__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x9d */
			Z80__RES__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x9e */
			Z80__RES__N__INDIRECT_REG16_D(3, reg, d);
			break;

		case Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x9f */
			Z80__RES__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0xa0 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0xa1 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0xa2 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0xa3 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0xa4 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0xa5 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0xa6 */
			Z80__RES__N__INDIRECT_REG16_D(4, reg, d);
			break;

		case Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0xa7 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0xa8 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0xa9 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0xaa */
			Z80__RES__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0xab */
			Z80__RES__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0xac */
			Z80__RES__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0xad */
			Z80__RES__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0xae */
			Z80__RES__N__INDIRECT_REG16_D(5, reg, d);
			break;

		case Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0xaf */
			Z80__RES__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0xb0 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0xb1 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0xb2 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0xb3 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0xb4 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0xb5 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0xb6 */
			Z80__RES__N__INDIRECT_REG16_D(6, reg, d);
			break;

		case Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0xb7 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0xb8 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0xb9 */
			Z80__RES__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0xba */
			Z80__RES__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0xbb */
			Z80__RES__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0xbc */
			Z80__RES__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0xbd */
			Z80__RES__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0xbe */
			Z80__RES__N__INDIRECT_REG16_D(7, reg, d);
			break;

		case Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0xbf */
			Z80__RES__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.a);
			break;

		/* SET opcodes */
		case Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0xc0 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0xc1 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0xc2 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0xc3 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0xc4 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0xc5 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0xc6 */
			Z80__SET__N__INDIRECT_REG16_D(0, reg, d);
			break;

		case Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0xc7 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(0, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0xc8 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0xc9 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0xca */
			Z80__SET__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0xcb */
			Z80__SET__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0xcc */
			Z80__SET__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0xcd */
			Z80__SET__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0xce */
			Z80__SET__N__INDIRECT_REG16_D(1, reg, d);
			break;

		case Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0xcf */
			Z80__SET__N__INDIRECT_REG16_D__REG8(1, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0xd0 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0xd1 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0xd2 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0xd3 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0xd4 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0xd5 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0xd6 */
			Z80__SET__N__INDIRECT_REG16_D(2, reg, d);
			break;

		case Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0xd7 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(2, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0xd8 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0xd9 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0xda */
			Z80__SET__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0xdb */
			Z80__SET__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0xdc */
			Z80__SET__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0xdd */
			Z80__SET__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0xde */
			Z80__SET__N__INDIRECT_REG16_D(3, reg, d);
			break;

		case Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0xdf */
			Z80__SET__N__INDIRECT_REG16_D__REG8(3, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0xe0 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0xe1 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0xe2 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0xe3 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0xe4 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0xe5 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0xe6 */
			Z80__SET__N__INDIRECT_REG16_D(4, reg, d);
			break;

		case Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0xe7 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(4, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0xe8 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0xe9 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0xea */
			Z80__SET__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0xeb */
			Z80__SET__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0xec */
			Z80__SET__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0xed */
			Z80__SET__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0xee */
			Z80__SET__N__INDIRECT_REG16_D(5, reg, d);
			break;

		case Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0xef */
			Z80__SET__N__INDIRECT_REG16_D__REG8(5, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0xf0 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0xf1 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0xf2 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0xf3 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0xf4 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.h);
			break;

		case Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0xf5 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.l);
			break;

		case Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0xf6 */
			Z80__SET__N__INDIRECT_REG16_D(6, reg, d);
			break;

		case Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0xf7 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(6, reg, d, m_registers.a);
			break;

		case Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0xf8 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.b);
			break;

		case Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0xf9 */
			Z80__SET__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.c);
			break;

		case Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0xfa */
			Z80__SET__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.d);
			break;

		case Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0xfb */
			Z80__SET__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.e);
			break;

		case Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0xfc */
			Z80__SET__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.h);
			break;
	}

	if (cycles) {
	    *cycles = 23;
	}

	if (size) {
	    *size = 4;
	}

	return true;
}

Z80::Z80::UnsignedWord Z80::Z80::registerValue(Register16 reg) const
{
	switch (reg) {
        case Register16::AF: return m_registers.af;
        case Register16::BC: return m_registers.bc;
        case Register16::DE: return m_registers.de;
        case Register16::HL: return m_registers.hl;
        case Register16::AFShadow: return m_registers.afShadow;
        case Register16::BCShadow: return m_registers.bcShadow;
        case Register16::DEShadow: return m_registers.deShadow;
        case Register16::HLShadow: return m_registers.hlShadow;
        case Register16::IX: return m_registers.ix;
        case Register16::IY: return m_registers.iy;
        case Register16::SP: return m_registers.sp;
        case Register16::PC: return m_registers.pc;
	}

	return 0;
}

Z80::Z80::UnsignedWord Z80::Z80::registerValueZ80(Register16 reg) const
{
	switch (reg) {
        case Register16::AF: return hostToZ80ByteOrder(m_registers.af);
        case Register16::BC: return hostToZ80ByteOrder(m_registers.bc);
        case Register16::DE: return hostToZ80ByteOrder(m_registers.de);
        case Register16::HL: return hostToZ80ByteOrder(m_registers.hl);
        case Register16::AFShadow: return hostToZ80ByteOrder(m_registers.afShadow);
        case Register16::BCShadow: return hostToZ80ByteOrder(m_registers.bcShadow);
        case Register16::DEShadow: return hostToZ80ByteOrder(m_registers.deShadow);
        case Register16::HLShadow: return hostToZ80ByteOrder(m_registers.hlShadow);
        case Register16::IX: return hostToZ80ByteOrder(m_registers.ix);
        case Register16::IY: return hostToZ80ByteOrder(m_registers.iy);
        case Register16::SP: return hostToZ80ByteOrder(m_registers.sp);
        case Register16::PC: return hostToZ80ByteOrder(m_registers.pc);
	}

	return 0;
}

Z80::Z80::UnsignedByte Z80::Z80::registerValue(Register8 reg) const
{
	switch (reg) {
		case Register8::A: return m_registers.a;
		case Register8::F: return m_registers.f;
		case Register8::B: return m_registers.b;
		case Register8::C: return m_registers.c;
		case Register8::D: return m_registers.d;
		case Register8::E: return m_registers.e;
		case Register8::H: return m_registers.h;
		case Register8::L: return m_registers.l;
		case Register8::IXH: return m_registers.ixh;
		case Register8::IXL: return m_registers.ixl;
		case Register8::IYH: return m_registers.iyh;
		case Register8::IYL: return m_registers.iyl;

		case Register8::AShadow: return m_registers.aShadow;
		case Register8::FShadow: return m_registers.fShadow;
		case Register8::BShadow: return m_registers.bShadow;
		case Register8::CShadow: return m_registers.cShadow;
		case Register8::DShadow: return m_registers.dShadow;
		case Register8::EShadow: return m_registers.eShadow;
		case Register8::HShadow: return m_registers.hShadow;
		case Register8::LShadow: return m_registers.lShadow;

		case Register8::I: return m_registers.i;
		case Register8::R: return m_registers.r;
	}

	return 0;
}

void Z80::Z80::setRegisterValue(Register16 reg, Z80::Z80::UnsignedWord value)
{
	switch (reg) {
        case Register16::AF: m_registers.af = value; break;
        case Register16::BC: m_registers.bc = value; break;
        case Register16::DE: m_registers.de = value; break;
        case Register16::HL: m_registers.hl = value; break;
        case Register16::AFShadow: m_registers.afShadow = value; break;
        case Register16::BCShadow: m_registers.bcShadow = value; break;
        case Register16::DEShadow: m_registers.deShadow = value; break;
        case Register16::HLShadow: m_registers.hlShadow = value; break;
        case Register16::IX: m_registers.ix = value; break;
        case Register16::IY: m_registers.iy = value; break;
        case Register16::SP: m_registers.sp = value; break;
        case Register16::PC: m_registers.pc = value; break;
	}
}

void Z80::Z80::setRegisterValueZ80(Register16 reg, Z80::Z80::UnsignedWord value)
{
	switch (reg) {
        case Register16::AF: m_registers.af = z80ToHostByteOrder(value); break;
        case Register16::BC: m_registers.bc = z80ToHostByteOrder(value); break;
        case Register16::DE: m_registers.de = z80ToHostByteOrder(value); break;
        case Register16::HL: m_registers.hl = z80ToHostByteOrder(value); break;
        case Register16::AFShadow: m_registers.afShadow = z80ToHostByteOrder(value); break;
        case Register16::BCShadow: m_registers.bcShadow = z80ToHostByteOrder(value); break;
        case Register16::DEShadow: m_registers.deShadow = z80ToHostByteOrder(value); break;
        case Register16::HLShadow: m_registers.hlShadow = z80ToHostByteOrder(value); break;
        case Register16::IX: m_registers.ix = z80ToHostByteOrder(value); break;
        case Register16::IY: m_registers.iy = z80ToHostByteOrder(value); break;
        case Register16::SP: m_registers.sp = z80ToHostByteOrder(value); break;
        case Register16::PC: m_registers.pc = z80ToHostByteOrder(value); break;
	}
}

void Z80::Z80::setRegisterValue(Register8 reg, Z80::Z80::UnsignedByte value)
{
	switch (reg) {
		case Register8::A: m_registers.a = value; break;
		case Register8::F: m_registers.f = value; break;
		case Register8::B: m_registers.b = value; break;
		case Register8::C: m_registers.c = value; break;
		case Register8::D: m_registers.d = value; break;
		case Register8::E: m_registers.e = value; break;
		case Register8::H: m_registers.h = value; break;
		case Register8::L: m_registers.l = value; break;
		case Register8::IXH: m_registers.ixh = value; break;
		case Register8::IXL: m_registers.ixl = value; break;
		case Register8::IYH: m_registers.iyh = value; break;
		case Register8::IYL: m_registers.iyl = value; break;
		case Register8::AShadow: m_registers.aShadow = value; break;
		case Register8::FShadow: m_registers.fShadow = value; break;
		case Register8::BShadow: m_registers.bShadow = value; break;
		case Register8::CShadow: m_registers.cShadow = value; break;
		case Register8::DShadow: m_registers.dShadow = value; break;
		case Register8::EShadow: m_registers.eShadow = value; break;
		case Register8::HShadow: m_registers.hShadow = value; break;
		case Register8::LShadow: m_registers.lShadow = value; break;
		case Register8::I: m_registers.i = value; break;
		case Register8::R: m_registers.r = value; break;
	}
}

Z80::Z80::UnsignedWord Z80::Z80::peekUnsignedWord(int addr) const
{
    // TODO make an assertion instead
	if (addr < 0 || addr >= (m_ramSize - 1)) {
	    return 0;
	}

	if (HostByteOrder == Z80ByteOrder) {
        return m_ram[addr + 1] << 8 | m_ram[addr];
    }

    return (m_ram[addr] << 8) | m_ram[addr + 1];
}

Z80::Z80::UnsignedWord Z80::Z80::peekUnsignedWordZ80(int addr) const
{
    // TODO make an assertion instead
	if (addr < 0 || addr >= (m_ramSize - 1)) {
	    return 0;
	}

    return m_ram[addr + 1] << 8 | m_ram[addr];
}

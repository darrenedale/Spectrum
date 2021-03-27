/**
 * \file z80.cpp
 * \author Darren Edale
 * \version 0.5
 *
 * \brief Implementation of the Z80 CPU emulation.
 *
 * This emulation is based on interpretation. There is no dynamic compilation and no cross-assembly.
 *
 * TODO interrupt mode 0 multi-byte instructions
 * TODO review the memory access code now that we're using the Memory abstraction rather than a raw array of bytes
 * TODO in debug mode keep a trace of the last N PCs and instructions, including operand values
 * TODO there is definitely an issue with stack handling, this is what is causing chuckie egg to fail, and is a likely
 *  candidate for any snapshot that fails with a reset back to the ROM
 */
#include <iostream>
#include <iomanip>
#include <cassert>

#include "z80.h"
#include "iodevice.h"
#include "invalidopcode.h"
#include "invalidinterruptmode.h"
#include "opcodes.h"

#if !(defined(NDEBUG))
#include "assembly/disassembler.h"
#endif

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

// very often S, 5 and 3 flags are simply set to the same bits as the result (usually reg A)
#define Z80_FLAGS_S53_UPDATE(byte) (m_registers.f = (m_registers.f & 0b01010111) | ((byte) & 0b10101000))

#define Z80_FLAG_C_ISSET (0 != (m_registers.f & Z80_FLAG_C_MASK))
#define Z80_FLAG_Z_ISSET (0 != (m_registers.f & Z80_FLAG_Z_MASK))
#define Z80_FLAG_P_ISSET (0 != (m_registers.f & Z80_FLAG_P_MASK))
#define Z80_FLAG_S_ISSET (0 != (m_registers.f & Z80_FLAG_S_MASK))
#define Z80_FLAG_N_ISSET (0 != (m_registers.f & Z80_FLAG_N_MASK))
#define Z80_FLAG_H_ISSET (0 != (m_registers.f & Z80_FLAG_H_MASK))

/* used in instruction execution methods to force the PC NOT to be updated with
 * the size of the instruction in execute() in cases where the instruction
 * directly changes the PC - e.g. JP, JR, DJNZ, RET, CALL etc. */
#define Z80_DONT_UPDATE_PC if (doPc) *doPc = false;
#define Z80_USE_JUMP_CYCLE_COST useJumpCycleCost = true;

/* macros to fetch opcode t-state costs. most non-jump opcodes
	don't actually need to use these. for conditional jump opcodes,
	the cost if the jump is taken is stored in the rightmost 16
	bits; the cost if the jump is not taken is stored in the
	leftmost 16 bits */
#define Z80_TSTATES_JUMP(tStates) (((tStates) & 0xffff0000) >> 16)
#define Z80_TSTATES_NOJUMP(tStates) ((tStates) & 0x0000ffff)

// update H flag for carry into bit 4 during addition
#define Z80_FLAG_H_UPDATE_ADD(orig,delta,result) \
{                                                \
    UnsignedByte tmpHalfCarry = (                \
        (((result) & 0b00001000) >> 1)           \
        | (((delta) & 0b00001000) >> 2)          \
        | (((orig) & 0b00001000) >> 3)           \
    );                                           \
                                                 \
    Z80_FLAG_H_UPDATE(tmpHalfCarry == 1 || tmpHalfCarry == 2 || tmpHalfCarry == 3 || tmpHalfCarry == 7);\
}

// update H flag for carry into bit 4 during subtraction
#define Z80_FLAG_H_UPDATE_SUB(orig,delta,result) \
{                                                \
    UnsignedByte tmpHalfCarry = (                \
        (((result) & 0b00001000) >> 1)           \
        | (((delta) & 0b00001000) >> 2)          \
        | (((orig) & 0b00001000) >> 3)           \
    );                                           \
                                                 \
    Z80_FLAG_H_UPDATE(tmpHalfCarry == 2 || tmpHalfCarry == 4 || tmpHalfCarry == 6 || tmpHalfCarry == 7);\
}

// update H flag for carry into bit-12 during addition
#define Z80_FLAG_H_UPDATE_16_ADD(orig,delta,result) \
{                                                   \
    UnsignedByte tmpHalfCarry = (                   \
        (((result) & 0x0800) >> 9)                  \
        | (((delta) & 0x0800) >> 10)                \
        | (((orig) & 0x0800) >> 11)                 \
    );                                              \
                                                    \
    Z80_FLAG_H_UPDATE(tmpHalfCarry == 1 || tmpHalfCarry == 2 || tmpHalfCarry == 3 || tmpHalfCarry == 7);\
}

// NOTE there are no 16-bit subtraction instructions so no macro for managing the H flag in this scenario

// update P/V flag for 8-bit overflow during addition 
#define Z80_FLAG_P_UPDATE_OVERFLOW_ADD(orig,delta,result)   \
{                                                           \
    UnsignedByte tmpOverflow = (                            \
        (((result) & 0b10000000) >> 5)                      \
        | (((delta) & 0b10000000) >> 6)                     \
        | (((orig) & 0b10000000) >> 7)                      \
    );                                                      \
                                                            \
    Z80_FLAG_P_UPDATE(tmpOverflow == 3 || tmpOverflow == 4);\
}

// update P/V flag for 16-bit overflow during addition
#define Z80_FLAG_P_UPDATE_OVERFLOW16_ADD(orig,delta,result) \
{                                                           \
    UnsignedByte tmpOverflow = (                            \
        (((result) & 0x8000) >> 13)                         \
        | (((delta) & 0x8000) >> 14)                        \
        | (((orig) & 0x8000) >> 15)                         \
    );                                                      \
                                                            \
    Z80_FLAG_P_UPDATE(tmpOverflow == 3 || tmpOverflow == 4);\
}

// update P/V flag for 8-bit overflow during subtraction
#define Z80_FLAG_P_UPDATE_OVERFLOW_SUB(orig,delta,result)   \
{                                                           \
    UnsignedByte tmpOverflow = (                            \
        (((result) & 0x80) >> 5)                            \
        | (((delta) & 0x80) >> 6)                           \
        | (((orig) & 0x80) >> 7)                            \
    );                                                      \
                                                            \
    Z80_FLAG_P_UPDATE(tmpOverflow == 1 || tmpOverflow == 6);\
}

// update P/V flag for 16-bit overflow during subtraction
#define Z80_FLAG_P_UPDATE_OVERFLOW16_SUB(orig,delta,result) \
{                                                           \
    UnsignedByte tmpOverflow = (                            \
        (((result) & 0x8000) >> 13)                         \
        | (((delta) & 0x8000) >> 14)                        \
        | (((orig) & 0x8000) >> 15)                         \
    );                                                      \
                                                            \
    Z80_FLAG_P_UPDATE(tmpOverflow == 1 || tmpOverflow == 6);\
}

//
//macros implementing common instruction semantics using different combinations of like operands
//

// data loading instructions
//
// FLAGS: no flags are modified
#define Z80__LD__REG8__N(dest, n) ((dest) = (n))
#define Z80__LD__REG8__REG8(dest, src) ((dest) = (src))

// nn (the memory address to retrieve) MUST be in HOST byte order
#define Z80__LD__REG8__INDIRECT_NN(dest, nn) ((dest) = peekUnsigned(nn))
#define Z80__LD__REG8__INDIRECT_REG16(dest, src) ((dest) = peekUnsigned((src)))

// nn (the value to write to the register) MUST be in HOST byte order
#define Z80__LD__REG16__NN(dest, nn) ((dest) = (nn))
#define Z80__LD__REG16__REG16(dest, src) ((dest) = (src))

#define Z80__LD__INDIRECT_REG16__REG8(dest, src) pokeUnsigned((dest), (src))

// nn (the memory address to load) MUST be in HOST byte order
// TODO check how we're handling memptr - this code seems to imply that unlike the other registers it's being stored
//  in Z80 byte order?
#define Z80__LD__INDIRECT_NN__REG16(nn, src) \
{                                            \
    auto tmpAddr = (nn);                     \
    pokeHostWord(tmpAddr, (src));            \
    m_registers.memptr = hostToZ80ByteOrder(tmpAddr); \
}

// nn (the memory address to retrieve) MUST be in HOST byte order
#define Z80__LD__REG16__INDIRECT_NN(dest, nn) ((dest) = peekUnsignedWord(nn))
#define Z80__LD__INDIRECT_REG16__N(dest, n) (pokeUnsigned((dest), n))

// nn (the memory address to load) MUST be in HOST byte order
#define Z80__LD__INDIRECT_NN__REG8(nn, src) (pokeUnsigned(nn, (src)))
#define Z80__LD__INDIRECT_REG16_D__N(reg, d, n) Z80__LD__INDIRECT_REG16__N(((reg) + (d)), (n));
#define Z80__LD__INDIRECT_REG16_D__REG8(reg, d, src) Z80__LD__INDIRECT_NN__REG8((reg) + (d), (src));
#define Z80__LD__REG8__INDIRECT_REG16_D(dest, reg, d) Z80__LD__REG8__INDIRECT_NN((dest), (reg) + (d));

// reset instructions
//
// addr can be 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30 or 0x38, and must be in HOST byte order
//
// FLAGS: no flags are modified.
#define Z80__RST__N(addr)             \
Z80__PUSH__REG16(m_registers.pc + 1); \
m_registers.pc = (addr);              \
m_registers.memptr = m_registers.pc

/* context switching instructions
 *
 * FLAGS: no flags are modified.
 */
#define Z80__EX__REG16__REG16(src, dest)    \
{                                           \
    UnsignedWord tmpWord = (dest);          \
    (dest) = (src);                         \
    (src) = tmpWord;                        \
}

#define Z80__EX__INDIRECT_REG16__REG16(src, dest) \
{\
    UnsignedWord tmpWord = peekUnsignedWord((src));\
    pokeHostWord((src), (dest));                   \
    (dest) = tmpWord;                              \
}

//
// stack instructions
//
#define Z80__POP__REG16(reg)              \
(reg) = peekUnsignedWord(m_registers.sp); \
m_registers.sp += 2;

#define Z80__PUSH__REG16(reg) \
m_registers.sp -= 2;          \
pokeHostWord(m_registers.sp, (reg));

//
// addition instructions
//
#define Z80__ADD__REG8__N(dest,n) \
{    \
    UnsignedByte tmpOldValue = (dest); \
    UnsignedByte tmpDelta = (n);         \
    UnsignedWord tmpResult = tmpOldValue + tmpDelta; \
    (dest) = tmpResult & 0xff;                 \
    Z80_FLAG_P_UPDATE_OVERFLOW_ADD(tmpOldValue, tmpDelta, tmpResult); \
    Z80_FLAG_N_CLEAR;                   \
    Z80_FLAG_Z_UPDATE(0 == ((dest) & 0xff));              \
    Z80_FLAG_S_UPDATE((dest) & 0x80);  \
    Z80_FLAG_C_UPDATE(tmpResult & 0x100);    \
    Z80_FLAG_H_UPDATE_ADD(tmpOldValue, tmpDelta, tmpResult);                    \
    Z80_FLAG_F3_UPDATE((dest) & Z80_FLAG_F3_MASK);\
    Z80_FLAG_F5_UPDATE((dest) & Z80_FLAG_F5_MASK);\
}

#define Z80__ADD__REG8__REG8(dest,src) Z80__ADD__REG8__N(dest,src)
#define Z80__ADD__REG8__INDIRECT_REG16(dest,src) Z80__ADD__REG8__N(dest,(*(m_memory->pointerTo(src))))

#define Z80__ADD__REG16__REG16(dest,src) \
{       \
    UnsignedWord tmpOldValue = (dest);           \
    UnsignedWord tmpDelta = (src);                 \
    std::uint32_t tmpResult = tmpOldValue + tmpDelta;  \
    (dest) = tmpResult & 0xffff;                 \
    Z80_FLAG_H_UPDATE_16_ADD(tmpOldValue, tmpDelta, tmpResult); \
    Z80_FLAG_N_CLEAR;                            \
    Z80_FLAG_C_UPDATE(tmpResult & 0x10000);\
    Z80_FLAG_F5_UPDATE((dest) & (Z80_FLAG_F5_MASK << 8)); \
    Z80_FLAG_F3_UPDATE((dest) & (Z80_FLAG_F3_MASK << 8)); \
}

#define Z80__ADD__REG8__INDIRECT_REG16_D(dest, reg, d) Z80__ADD__REG8__N((dest),(*(m_memory->pointerTo((reg) + (d)))))

//
// addition with carry instructions
// dest = dest + src + carry
//
#define Z80__ADC__REG8__N(dest,n) \
{      \
    UnsignedByte tmpOldValue = (dest);   \
    UnsignedByte tmpDelta = (n);           \
    UnsignedWord tmpResult = (dest) + tmpDelta + (Z80_FLAG_C_ISSET ? 1 : 0);\
    (dest) = tmpResult & 0xff;           \
    Z80_FLAG_C_UPDATE(tmpResult & 0x100);\
    Z80_FLAG_N_CLEAR;                    \
    Z80_FLAG_P_UPDATE_OVERFLOW_ADD(tmpOldValue, tmpDelta, (dest));\
    Z80_FLAG_H_UPDATE_ADD(tmpOldValue, tmpDelta, (dest));\
    Z80_FLAGS_S53_UPDATE((dest));        \
    Z80_FLAG_Z_UPDATE(0 == (dest));      \
}

#define Z80__ADC__REG8__REG8(dest,src) Z80__ADC__REG8__N((dest), (src));
#define Z80__ADC__REG8__INDIRECT_REG16(dest,src) Z80__ADC__REG8__N((dest), peekUnsigned(src))

#define Z80__ADC__REG16__REG16(dest,src) \
{ \
    UnsignedWord tmpOldValue = (dest);            \
    UnsignedWord tmpDelta = (src);                    \
    std::uint32_t tmpResult = (dest) + tmpDelta + (Z80_FLAG_C_ISSET ? 1 : 0);\
    (dest) = tmpResult & 0xffff;                  \
    Z80_FLAG_N_CLEAR;                             \
    Z80_FLAG_Z_UPDATE(0 == (dest));               \
    Z80_FLAGS_S53_UPDATE(((dest) & 0xff00) >> 8); \
    Z80_FLAG_P_UPDATE_OVERFLOW16_ADD(tmpOldValue, tmpDelta, (dest));      \
    Z80_FLAG_C_UPDATE(tmpResult & 0x00010000);    \
    /* check for carry between bits 11 and 12 - exactly the same as half-carry flag for 8-bit ADC, except we're
     * working with the high byte */              \
    Z80_FLAG_H_UPDATE_ADD(static_cast<UnsignedByte>((tmpOldValue & 0xff00) >> 8), static_cast<UnsignedByte>((tmpDelta & 0xff00) >> 8), static_cast<UnsignedByte>((tmpResult & 0xff00) >> 8));\
}

#define Z80__ADC__REG8__INDIRECT_REG16_D(dest, reg, d) Z80__ADC__REG8__N((dest), peekUnsigned((reg) + (d)))

//
// subtraction instructions
//
#define Z80__SUB__N(n) \
{ \
    UnsignedByte tmpOldValue = m_registers.a;   \
    UnsignedByte tmpDelta = (n);           \
    UnsignedWord tmpResult = m_registers.a - tmpDelta; \
    m_registers.a = tmpResult & 0xff;           \
    Z80_FLAG_C_UPDATE(tmpResult & 0x0100);                       \
    Z80_FLAG_N_SET;      \
    Z80_FLAG_P_UPDATE_OVERFLOW_SUB(tmpOldValue, tmpDelta, m_registers.a); \
    Z80_FLAG_H_UPDATE_SUB(tmpOldValue, tmpDelta, m_registers.a); \
    Z80_FLAGS_S53_UPDATE(m_registers.a);\
    Z80_FLAG_Z_UPDATE(0 == m_registers.a);      \
}

#define Z80__SUB__REG8(reg) Z80__SUB__N(reg)
#define Z80__SUB__INDIRECT_REG16(reg) { UnsignedByte v = peekUnsigned(reg); Z80__SUB__N(v) }
#define Z80__SUB__INDIRECT_REG16_D(reg,d) Z80__SUB__N(peekUnsigned((reg) + (d)))

//
// subtraction with carry instructions
// dest = dest - src - carry
//
#define Z80__SBC__REG8__N(dest,n) \
{ \
    UnsignedByte tmpOldValue = (dest);\
    UnsignedByte tmpDelta = (n);            \
    UnsignedWord tmpResult = (dest) - tmpDelta - (Z80_FLAG_C_ISSET ? 1 : 0);\
    (dest) = tmpResult & 0xff;   \
    Z80_FLAG_C_UPDATE(tmpResult & 0x0100);\
    Z80_FLAG_N_SET;                       \
    Z80_FLAG_P_UPDATE_OVERFLOW_SUB(tmpOldValue, tmpDelta, (dest));\
    Z80_FLAG_H_UPDATE_SUB(tmpOldValue, tmpDelta, (dest));\
    Z80_FLAGS_S53_UPDATE((dest));  \
    Z80_FLAG_Z_UPDATE(0 == (dest));\
}

#define Z80__SBC__REG8__REG8(dest,src) Z80__SBC__REG8__N((dest), (src))
#define Z80__SBC__REG8__INDIRECT_REG16(dest,src) Z80__SBC__REG8__N((dest),peekUnsigned(src))

#define Z80__SBC__REG16__REG16(dest, src) \
{     \
    UnsignedWord tmpOldValue = (dest);          \
    UnsignedWord tmpDelta = (src);                \
    std::uint32_t tmpResult = (dest) - tmpDelta - (Z80_FLAG_C_ISSET ? 1 : 0); \
    (dest) = tmpResult & 0xffff;                \
    Z80_FLAG_N_SET;                             \
    Z80_FLAG_Z_UPDATE(0 == (dest));             \
    Z80_FLAGS_S53_UPDATE(((dest) & 0xff00) >> 8); \
    Z80_FLAG_P_UPDATE_OVERFLOW16_SUB(tmpOldValue, tmpDelta, (dest));    \
    Z80_FLAG_C_UPDATE((dest) > tmpOldValue);    \
    /* check for carry between bits 11 and 12 - exactly the same as half-carry flag for 8-bit SBC, except we're
     * working with the high byte */ \
   Z80_FLAG_H_UPDATE_SUB(static_cast<UnsignedByte>((tmpOldValue & 0xff00) >> 8), static_cast<UnsignedByte>((tmpDelta & 0xff00) >> 8), static_cast<UnsignedByte>((tmpResult & 0xff00) >> 8));\
}

#define Z80__SBC__REG8__INDIRECT_REG16_D(dest,reg,d) Z80__SBC__REG8__N((dest), peekUnsigned((reg) + (d)))

//
// increment instructions
//
#define Z80__INC__REG8(reg)             \
(reg)++;                                \
Z80_FLAG_H_UPDATE(0 == (0x0f & (reg))); \
Z80_FLAG_P_UPDATE(0x80 == (reg));       \
Z80_FLAG_N_CLEAR;                       \
Z80_FLAG_Z_UPDATE(0 == (reg));          \
Z80_FLAG_S_UPDATE((reg) & 0x80);        \
Z80_FLAG_F3_UPDATE((reg) & Z80_FLAG_F3_MASK); \
Z80_FLAG_F5_UPDATE((reg) & Z80_FLAG_F5_MASK);

#define Z80__INC__INDIRECT_REG16(reg) Z80__INC__REG8(*(m_memory->pointerTo(reg)))
#define Z80__INC__REG16(reg) (reg)++;
// TODO memptr
#define Z80__INC__INDIRECT_REG16_D(reg, d) Z80__INC__REG8(*(m_memory->pointerTo((reg) + (d))))

//
// decrement instructions
//
#define Z80__DEC__REG8(reg) \
Z80_FLAG_H_UPDATE(0 == (0x0f & (reg))); \
(reg)--;                    \
Z80_FLAG_P_UPDATE(0x7f == (reg)); \
Z80_FLAG_N_SET;             \
Z80_FLAG_Z_UPDATE(0 == (reg));   \
Z80_FLAG_S_UPDATE((reg) & 0x80); \
Z80_FLAG_F3_UPDATE((reg) & Z80_FLAG_F3_MASK); \
Z80_FLAG_F5_UPDATE((reg) & Z80_FLAG_F5_MASK);

#define Z80__DEC__INDIRECT_REG16(reg) Z80__DEC__REG8(*(m_memory->pointerTo(reg)))
#define Z80__DEC__REG16(reg) (reg)--;
// TODO memptr
#define Z80__DEC__INDIRECT_REG16_D(reg, d) Z80__DEC__REG8(*(m_memory->pointerTo((reg) + (d))));

//
// negation instruction
//
// there is only one negation instruction, but it has several opcodes (most of
// which are unofficial), so a macro is provided for a common implementation.
//
#define Z80_NEG                                       \
{                                                     \
    UnsignedByte tmpOldValue = m_registers.a;         \
    m_registers.a = 0 - (m_registers.a);              \
    Z80_FLAG_Z_UPDATE(0 == m_registers.a);            \
    Z80_FLAG_H_UPDATE_SUB(0, tmpOldValue, m_registers.a); \
    Z80_FLAG_C_UPDATE(0x00 != tmpOldValue);           \
    Z80_FLAG_P_UPDATE(0x80 == tmpOldValue);           \
    Z80_FLAGS_S53_UPDATE(m_registers.a);              \
    Z80_FLAG_N_SET;                                   \
}

//
// compare instructions
//
// These instructions are mostly identical to SUB instruction, except that the result
// is discarded rather than loaded into A and the handling of flags F5 and F3 differs.
//
#define Z80__CP__N(n) \
{ \
    UnsignedByte tmpDelta = (n);           \
    UnsignedWord tmpResult = m_registers.a - tmpDelta; \
    Z80_FLAG_C_UPDATE(tmpResult & 0x0100);                       \
    Z80_FLAG_N_SET;      \
    Z80_FLAG_P_UPDATE_OVERFLOW_SUB(m_registers.a, tmpDelta, tmpResult); \
    Z80_FLAG_H_UPDATE_SUB(m_registers.a, tmpDelta, tmpResult); \
    Z80_FLAG_S_UPDATE(tmpResult & Z80_FLAG_S_MASK);\
    Z80_FLAG_F5_UPDATE(tmpDelta & Z80_FLAG_F5_MASK);\
    Z80_FLAG_F3_UPDATE(tmpDelta & Z80_FLAG_F3_MASK);\
    Z80_FLAG_Z_UPDATE(0 == tmpResult);      \
}

#define Z80__CP__REG8(reg) Z80__CP__N(reg)
#define Z80__CP__INDIRECT_REG16(reg) Z80__CP__N(peekUnsigned((reg)))
#define Z80__CP__INDIRECT_REG16_D(reg,d) Z80__CP__N(peekUnsigned((reg) + (d)))

//
// bitwise operations
//
// FLAGS: C cleared, N cleared, P is parity, others by definition
//
#define Z80_BITWISE_FLAGS \
Z80_FLAG_C_CLEAR;         \
Z80_FLAG_N_CLEAR;         \
Z80_FLAG_P_UPDATE(isEvenParity(m_registers.a)); \
Z80_FLAGS_S53_UPDATE(m_registers.a);            \
Z80_FLAG_Z_UPDATE(0 == m_registers.a);

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

//
// bit set instructions
//
/* FLAGS: all preserved */
#define Z80__SET__N__REG8(n,reg) (reg) |= (1 << (n))
#define Z80__SET__N__INDIRECT_REG16(n,reg) pokeUnsigned((reg), peekUnsigned((reg)) | (1 << (n)))
#define Z80__SET__N__INDIRECT_REG16_D(n,reg,d) Z80__SET__N__INDIRECT_REG16(n,(reg) + (d))
#define Z80__SET__N__INDIRECT_REG16_D__REG8(n,reg16,d,reg8) \
Z80__SET__N__INDIRECT_REG16_D(n, (reg16), (d)); \
(reg8) = peekUnsigned((reg16) + (d));

//
// bit reset instructions
//
#define Z80__RES__N__REG8(n,reg) (reg) &= ~(1 << (n))
#define Z80__RES__N__INDIRECT_REG16(n,reg) pokeUnsigned((reg), peekUnsigned((reg)) & ~(1 << (n)))
#define Z80__RES__N__INDIRECT_REG16_D(n,reg,d) Z80__RES__N__INDIRECT_REG16(n,(reg) + (d))
#define Z80__RES__N__INDIRECT_REG16_D__REG8(n,reg16,d,reg8) \
Z80__RES__N__INDIRECT_REG16_D(n,(reg16),(d));               \
(reg8) = peekUnsigned((reg16) + (d))

//
// bit shift and rotation instructions
//

//
// rotate left with carry instructions
//
#define Z80__RLC__REG8(reg)              \
{                                        \
    bool tmpBit = (reg) & 0x80;          \
    (reg) <<= 1;                         \
                                         \
    if (tmpBit) {                        \
        (reg) |= 0x01;                   \
        Z80_FLAG_C_SET;                  \
    } else {                             \
        (reg) &= 0xfe;                   \
        Z80_FLAG_C_CLEAR;                \
    }                                    \
    Z80_FLAG_H_CLEAR;                    \
    Z80_FLAG_N_CLEAR;                    \
    Z80_FLAG_P_UPDATE(isEvenParity(reg));\
    Z80_FLAGS_S53_UPDATE((reg));         \
    Z80_FLAG_Z_UPDATE(0 == (reg));       \
}

#define Z80__RLC__INDIRECT_REG16(reg) {     \
    UnsignedByte * tmpValue = memory()->pointerTo(reg);\
    Z80__RLC__REG8(*tmpValue);               \
}

#define Z80__RLC__INDIRECT_REG16_D(reg,d) Z80__RLC__INDIRECT_REG16((reg) + (d))
#define Z80__RLC__INDIRECT_REG16_D__REG8(reg16,d,reg8) Z80__RLC__INDIRECT_REG16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

//
// rotate right with carry instructions
//
#define Z80__RRC__REG8(reg) \
{ \
    bool bit = (reg) & 0x01;      \
    (reg) >>= 1;                  \
                                  \
    if (bit) {                    \
        (reg) |= 0x80;            \
        Z80_FLAG_C_SET;           \
    } else {                      \
        (reg) &= 0x7f;            \
        Z80_FLAG_C_CLEAR;         \
    }                             \
                                  \
    Z80_FLAG_H_CLEAR;             \
    Z80_FLAG_N_CLEAR;             \
    Z80_FLAG_P_UPDATE(isEvenParity(reg));\
    Z80_FLAGS_S53_UPDATE((reg));         \
    Z80_FLAG_Z_UPDATE(0 == (reg));       \
}
#define Z80__RRC__INDIRECT_REG16(reg) \
{\
    UnsignedByte * tmpValue = memory()->pointerTo(reg); \
    Z80__RRC__REG8(*tmpValue);          \
}
#define Z80__RRC__INDIRECT_REG16_D(reg,d) Z80__RRC__INDIRECT_REG16((reg) + (d))
#define Z80__RRC__INDIRECT_REG16_D__REG8(reg16,d,reg8) Z80__RRC__INDIRECT_REG16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

//
// rotate left instructions
//
// value is rotated left. bit 7 moves into carry flag and carry flag
// moves into bit 0. In other words, it's as if the value was 9 bits in
// size with the carry flag as bit 8.
//
#define Z80__RL__REG8(reg)        \
{                                 \
	bool bit = (reg) & 0x80;      \
	(reg) <<= 1;                  \
	                              \
	if (Z80_FLAG_C_ISSET) {       \
        (reg) |= 0x01;            \
    } else {                      \
        (reg) &= 0xfe;            \
    }                             \
                                  \
	Z80_FLAG_C_UPDATE(bit);       \
	Z80_FLAG_H_CLEAR;             \
	Z80_FLAG_N_CLEAR;             \
	Z80_FLAG_P_UPDATE(isEvenParity(reg));\
	Z80_FLAGS_S53_UPDATE((reg));  \
	Z80_FLAG_Z_UPDATE(0 == (reg));\
}

/*
 * re-use RL instruction for 8-bit reg to do the actual work
 */
#define Z80__RL__INDIRECT_REG16(reg) \
{      \
    UnsignedByte * tmpValue = memory()->pointerTo(reg);\
    Z80__RL__REG8(*tmpValue);                \
}

#define Z80__RL__INDIRECT_REG16_D(reg,d) Z80__RL__INDIRECT_REG16((reg) + (d))
#define Z80__RL__INDIRECT_REG16_D__REG8(reg16,d,reg8) Z80__RL__INDIRECT_REG16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

//
// rotate right instructions
//
// value is rotated right. bit 0 moves into carry flag and carry flag
// moves into bit 7. In other words, it's as if the value was 9 bits in
// size with the carry flag as bit 0.
//
#define Z80__RR__REG8(reg) \
{     \
	bool tmpBit = (reg) & 0x01;   \
	(reg) >>= 1;                  \
                                  \
	if (Z80_FLAG_C_ISSET) {       \
        (reg) |= 0x80;            \
    } else {                      \
        (reg) &= 0x7f;            \
    }                             \
                                  \
	Z80_FLAG_C_UPDATE(tmpBit);    \
	Z80_FLAG_H_CLEAR;             \
	Z80_FLAG_N_CLEAR;             \
	Z80_FLAG_P_UPDATE(isEvenParity(reg));\
	Z80_FLAGS_S53_UPDATE((reg));  \
	Z80_FLAG_Z_UPDATE(0 == (reg));\
}
#define Z80__RR__INDIRECT_REG16(reg) {      \
    UnsignedByte * tmpValue = memory()->pointerTo(reg);\
    Z80__RR__REG8(*tmpValue);                \
}
#define Z80__RR__INDIRECT_REG16_D(reg,d) Z80__RR__INDIRECT_REG16((reg) + (d))
#define Z80__RR__INDIRECT_REG16_D__REG8(reg16,d,reg8) Z80__RR__INDIRECT_REG16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

//
// arithmetic left shift instructions
//
#define Z80__SLA__REG8(reg) {            \
	Z80_FLAG_C_UPDATE((reg) & 0x80);     \
	(reg) <<= 1;                         \
	Z80_FLAG_H_CLEAR;                    \
	Z80_FLAG_N_CLEAR;                    \
	Z80_FLAG_P_UPDATE(isEvenParity(reg));\
	Z80_FLAGS_S53_UPDATE((reg));         \
	Z80_FLAG_Z_UPDATE(0 == (reg));       \
}
#define Z80__SLA__INDIRECT_REG16(reg) {     \
    UnsignedByte * tmpValue = memory()->pointerTo(reg);\
    Z80__SLA__REG8(*tmpValue);               \
}
#define Z80__SLA__INDIRECT_REG16_D(reg,d) Z80__SLA__INDIRECT_REG16((reg) + (d))
#define Z80__SLA__INDIRECT_REG16_D__REG8(reg16,d,reg8) Z80__SLA__INDIRECT_REG16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

//
// arithmetic right shift instructions
//
#define Z80__SRA__REG8(reg)                \
{                                          \
	Z80_FLAG_C_UPDATE((reg) & 0x01);       \
    (reg) = ((reg) & 0x80) | ((reg) >> 1); \
	Z80_FLAG_H_CLEAR;                      \
	Z80_FLAG_N_CLEAR;                      \
	Z80_FLAG_P_UPDATE(isEvenParity(reg));  \
	Z80_FLAGS_S53_UPDATE((reg));           \
	Z80_FLAG_Z_UPDATE(0 == (reg));         \
}

#define Z80__SRA__INDIRECT_REG16(reg)       \
{                                           \
    UnsignedByte * tmpValue = memory()->pointerTo(reg);\
    Z80__SRA__REG8(*tmpValue);               \
}

#define Z80__SRA__INDIRECT_REG16_D(reg,d) Z80__SRA__INDIRECT_REG16((reg) + (d))
#define Z80__SRA__INDIRECT_REG16_D__REG8(reg16,d,reg8) Z80__SRA__INDIRECT_REG16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

//
// logical left shift instruction
//
// bits are left shifted one place. bit 7 goes into carry flag and 1 goes into bit 0.
//
#define Z80__SLL__REG8(reg) {            \
	Z80_FLAG_C_UPDATE((reg) & 0x80);     \
    (reg) = 0x01 | ((reg) << 1);         \
	Z80_FLAG_H_CLEAR;                    \
	Z80_FLAG_N_CLEAR;                    \
	Z80_FLAG_P_UPDATE(isEvenParity(reg));\
	Z80_FLAGS_S53_UPDATE((reg));  \
	Z80_FLAG_Z_UPDATE(0 == (reg));       \
}
#define Z80__SLL__INDIRECT_REG16(reg) {     \
    UnsignedByte * tmpValue = memory()->pointerTo(reg);\
    Z80__SLL__REG8(*tmpValue);               \
}
#define Z80__SLL__INDIRECT_REG16_D(reg,d) Z80__SLL__INDIRECT_REG16((reg) + (d))
#define Z80__SLL__INDIRECT_REG16_D__REG8(reg16,d,reg8) Z80__SLL__INDIRECT_REG16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

//
// logical right shift instructions
//
// bits are right shifted one place. bit 0 goes into carry flag and 1 goes into bit 7.
//
#define Z80__SRL__REG8(reg) {       \
	Z80_FLAG_C_UPDATE((reg) & 0x01);\
	(reg) >>= 1;                    \
	Z80_FLAG_H_CLEAR;               \
	Z80_FLAG_N_CLEAR;               \
	Z80_FLAG_P_UPDATE(isEvenParity(reg));\
	Z80_FLAGS_S53_UPDATE((reg));    \
	Z80_FLAG_Z_UPDATE(0 == (reg));  \
}
#define Z80__SRL__INDIRECT_REG16(reg) { \
    UnsignedByte * tmpValue = memory()->pointerTo(reg);\
    Z80__SRL__REG8(*tmpValue);               \
}
#define Z80__SRL__INDIRECT_REG16_D(reg,d) Z80__SRL__INDIRECT_REG16((reg) + (d))
#define Z80__SRL__INDIRECT_REG16_D__REG8(reg16,d,reg8) Z80__SRL__INDIRECT_REG16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

//
// bit testing instructions
//
#define Z80__BIT__N__REG8(n,reg) \
Z80_FLAG_Z_UPDATE(0 == ((reg) & (0x01 << n))); \
Z80_FLAG_P_UPDATE(Z80_FLAG_Z_ISSET);        \
Z80_FLAG_N_CLEAR;                \
Z80_FLAG_H_SET;                  \
Z80_FLAG_F5_UPDATE((reg) & Z80_FLAG_F5_MASK);\
Z80_FLAG_F3_UPDATE((reg) & Z80_FLAG_F3_MASK);\
Z80_FLAG_S_UPDATE((n) == 7 && (reg) & Z80_FLAG_S_MASK);

#define Z80__BIT__N__INDIRECT_REG16(n,reg) \
Z80__BIT__N__REG8(n,peekUnsigned(reg));    \
Z80_FLAG_F5_UPDATE(m_registers.memptrH & Z80_FLAG_F5_MASK);\
Z80_FLAG_F3_UPDATE(m_registers.memptrH & Z80_FLAG_F3_MASK);

#define Z80__BIT__N__INDIRECT_REG16_D(n,reg,d) \
m_registers.memptr = ((reg) + (d)); \
Z80__BIT__N__INDIRECT_REG16(n,m_registers.memptr);

//
// nmi handler return instruction
//
// there is only one nmi return instruction, but it has several opcodes (most of
// which are unofficial), so a macro is provided for a common implementation.
//
#define Z80__RETN                \
Z80__POP__REG16(m_registers.pc); \
m_iff1 = m_iff2;

//
// interrupt handler return instruction
//
// there is only one interrupt return instruction, but it has several opcodes
// (most of which are unofficial), so a macro is provided for a common implementation.
//
#define Z80__RETI                \
Z80__POP__REG16(m_registers.pc); \
m_iff1 = m_iff2;                 \
/* TODO signal IO device that interrupt has finished */

//
// port IO instructions
//

// helper to perform byte write to connected IO devices
#define Z80__WRITE_IO_DEVICES(value, port) { \
    for (auto * device : m_ioDevices) {      \
        if (!device->checkWritePort(port)) { \
            continue;                        \
        }                                    \
                                             \
        device->writeByte(port, value);      \
    }                                        \
}

// port is 8-bit and is the LSB for the 16-bit port.
#define Z80__OUT__INDIRECT_REG8__REG8(port,value) {    \
    UnsignedWord tmpPort = ((port) & 0xff | (m_registers.b << 8)); \
    Z80__WRITE_IO_DEVICES((value), tmpPort)            \
    m_registers.memptr = tmpPort + 1;                  \
}

#define Z80__OUT__INDIRECT_N__REG8(port,value) {       \
    UnsignedWord tmpPort = ((port) & 0xff | (m_registers.a << 8)); \
    Z80__WRITE_IO_DEVICES((value), tmpPort)            \
    m_registers.memptr = tmpPort + 1;                  \
}

// TODO if multiple devices are reading from a given port, what happens to the result?
#define Z80__READ_IO_DEVICES(result, port) { \
    (result) = 0xff;                         \
                                             \
    for (auto * device : m_ioDevices) {      \
        if (!device->checkReadPort(port)) {  \
            continue;                        \
        }                                    \
                                             \
        (result) &= device->readByte(port);  \
    }                                        \
/*                                             \
    if ((port) == 0xeffe || (port) == 0xf7fe) { \
    std::cout << "byte from IN " << std::hex << std::setfill('0') << std::setw(4) << (port) << ": 0x" << std::setw(2) << static_cast<std::uint16_t>(result) << '\n';                                         \
    }*/ \
}

#define Z80__IN__REG8__INDIRECT_REG8(dest,port) {    \
    UnsignedWord tmpPort = ((port) & 0xff) | (m_registers.b << 8); \
    Z80__READ_IO_DEVICES((dest), tmpPort);           \
    m_registers.memptr = tmpPort + 1;                \
    Z80_FLAG_H_CLEAR;                                \
    Z80_FLAG_N_CLEAR;                                \
    Z80_FLAG_Z_UPDATE(0 == (dest));                  \
    Z80_FLAGS_S53_UPDATE((dest));                    \
    Z80_FLAG_P_UPDATE(isEvenParity((dest)));         \
}

#define Z80__IN__REG8__INDIRECT_REG16(dest,port) {   \
/*    UnsignedWord tmpPort = ((port) & 0xff) | (m_registers.b << 8);*/ \
    Z80__READ_IO_DEVICES((dest), (port));           \
/*    m_registers.memptr = tmpPort + 1; */               \
    Z80_FLAG_H_CLEAR;                                \
    Z80_FLAG_N_CLEAR;                                \
    Z80_FLAG_Z_UPDATE(0 == (dest));                  \
    Z80_FLAGS_S53_UPDATE((dest));                    \
    Z80_FLAG_P_UPDATE(isEvenParity((dest)));         \
}

#define Z80__IN__REG8__INDIRECT_N(dest,port) {       \
    UnsignedWord tmpPort = ((port) & 0xff) | (m_registers.a << 8); \
    Z80__READ_IO_DEVICES((dest), tmpPort);           \
}

using UnsignedByte = ::Z80::UnsignedByte;
using UnsignedWord = ::Z80::UnsignedWord;
using SignedByte = ::Z80::SignedByte;

namespace
{
    constexpr const int DefaultClockSpeed = 3500000;

    inline bool isEvenParity(UnsignedByte value)
    {
        static bool parity[256] = {
#include "includes/8bit_parity.inc"
        };

        return parity[value];
    }
}

constexpr const int Z80::Z80::PlainOpcodeSizes[256] = {
#include "includes/z80_plain_opcode_sizes.inc"
};

// NOTE all 0xcb opcodes are 2 bytes in size

constexpr const int Z80::Z80::EdOpcodeSizes[256] = {
#include "includes/z80_ed_opcode_sizes.inc"
};

constexpr const int Z80::Z80::DdOrFdOpcodeSizes[256] = {
#include "includes/z80_ddorfd_opcode_sizes.inc"
};

constexpr const int Z80::Z80::PlainOpcodeTStates[256] = {
#include "includes/z80_plain_opcode_tstates.inc"
};

constexpr const int Z80::Z80::CbOpcodeTStates[256] = {
#include "includes/z80_cb_opcode_tstates.inc"
};

constexpr const int Z80::Z80::EdOpcodeTStates[256] = {
#include "includes/z80_ed_opcode_tstates.inc"
};

constexpr const int Z80::Z80::DdOrFdOpcodeTStates[256] = {
#include "includes/z80_ddorfd_opcode_tstates.inc"
};

constexpr const int Z80::Z80::DdCbOrFdCbOpcodeTStates[256] = {
#include "includes/z80_ddorfd_cb_opcode_tstates.inc"
};

Z80::Z80::Z80(Memory * memory)
: Cpu(memory),
  m_memory(memory),
  m_clockSpeed(DefaultClockSpeed),
  m_nmiPending(false),
  m_interruptRequested(false),
  m_iff1(false),
  m_iff2(false),
  m_interruptMode(InterruptMode::IM0),
  m_halted(false)
{
    m_interruptData = 0x00;
}

Z80::Z80::~Z80() = default;

bool Z80::Z80::connectIODevice(IODevice * device)
{
    m_ioDevices.insert(device);
	device->setCpu(this);
	return true;
}

void Z80::Z80::disconnectIODevice(IODevice * device)
{
    const auto deviceIterator = m_ioDevices.find(device);

    if (m_ioDevices.cend() == deviceIterator) {
        return;
    }

    if (device->cpu() == this) {
        device->setCpu(nullptr);
    }

    m_ioDevices.erase(deviceIterator);
}

void Z80::Z80::interrupt(UnsignedByte data)
{
    m_interruptData = data;
	m_interruptRequested = true;
}

void Z80::Z80::nmi()
{
	m_nmiPending = true;
}

void Z80::Z80::reset()
{
	m_nmiPending = false;
    m_interruptMode = InterruptMode::IM0;
    m_interruptData = 0x00;
	m_iff1 = false;
	m_iff2 = false;
    m_interruptRequested = false;
    m_halted = false;
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
void Z80::Z80::pokeHostWord(MemoryType::Address addr, UnsignedWord value)
{
	if (0 > addr || memorySize() <= addr + 1) {
	    std::cerr << "Attempt to write outside addressable range (word 0x"
	        << std::hex << std::setfill('0') << std::setw(4) << value << " to 0x"
	        << std::setw(8) << addr << ")\n";

#if (!defined(NDEBUG))
        dumpState(std::cerr);
        dumpExecutionHistory(10, std::cerr);
#endif
	    return;
	}

	value = hostToZ80ByteOrder(value);
	auto * bytes = reinterpret_cast<UnsignedByte *>(&value);
	m_memory->writeByte(addr, bytes[0]);
	m_memory->writeByte(addr + 1, bytes[1]);
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
void Z80::Z80::pokeZ80Word(MemoryType::Address addr, UnsignedWord value)
{
	if (0 > addr || memorySize() <= addr + 1) {
        std::cerr << "Attempt to write outside addressable range (Z80 word 0x"
                  << std::hex << std::setfill('0') << std::setw(4) << value << " to 0x"
                  << std::setw(8) << addr << ")\n";
#if (!defined(NDEBUG))
        dumpState(std::cerr);
        dumpExecutionHistory(10, std::cerr);
#endif
        return;
	}

    auto * bytes = reinterpret_cast<UnsignedByte *>(&value);
    m_memory->writeByte(addr, bytes[0]);
    m_memory->writeByte(addr + 1, bytes[1]);
}

void Z80::Z80::pokeUnsigned(MemoryType::Address addr, UnsignedByte value)
{
    if (addr < 0 || addr > memorySize()) {
        std::cerr << "Attempt to write outside addressable range (byte 0x"
                  << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint16_t>(value) << " to 0x"
                  << std::setw(8) << addr << ")\n";
#if (!defined(NDEBUG))
        dumpState(std::cerr);
        dumpExecutionHistory(10, std::cerr);
#endif
        return;
    }

    m_memory->writeByte(addr, value);
}

UnsignedWord Z80::Z80::peekUnsignedWord(MemoryType::Address addr) const
{
    // TODO make an assertion instead
    if (0 > addr || memorySize() <= addr + 1) {
        std::cerr << "Attempt to read outside addressable range (word from 0x"
                  << std::hex << std::setfill('0') << std::setw(8) << addr << ")\n";
#if (!defined(NDEBUG))
        dumpState(std::cerr);
        dumpExecutionHistory(10, std::cerr);
#endif
        return 0;
    }

    if (HostByteOrder == Z80ByteOrder) {
        return (*m_memory)[addr + 1] << 8 | (*m_memory)[addr];
    }

    return ((*m_memory)[addr] << 8) | (*m_memory)[addr + 1];
}

UnsignedWord Z80::Z80::peekUnsignedWordZ80(MemoryType::Address addr) const
{
    // TODO make an assertion instead
    if (0 > addr || memorySize() <= addr + 1) {
        std::cerr << "Attempt to read outside addressable range (Z80 word from 0x"
                  << std::hex << std::setfill('0') << std::setw(8) << addr << ")\n";
#if (!defined(NDEBUG))
        dumpState(std::cerr);
        dumpExecutionHistory(10, std::cerr);
#endif
    }

    return (*m_memory)[addr + 1] << 8 | (*m_memory)[addr];
}

void Z80::Z80::execute(const UnsignedByte * instruction, bool doPc, int * tStates, int * size)
{
	static int dummySize;

	assert(instruction);

	if (!size) {
	    size = &dummySize;
	}

    ++m_registers.r;

#if (!defined(NDEBUG))
	ExecutedInstruction historyEntry;
	historyEntry.mnemonic = Assembly::Disassembler::disassembleOne(instruction);
	std::memcpy(historyEntry.machineCode, instruction, historyEntry.mnemonic.size);
	historyEntry.registersBefore = registers();

	for (int operandIndex = 0; operandIndex < historyEntry.mnemonic.operands.size(); ++operandIndex) {
	    historyEntry.operandValues.emplace_back(historyEntry.mnemonic.operands[operandIndex].evaluate(this, 0 == operandIndex));
	}
#endif
	switch (*instruction) {
		case Z80__PLAIN__PREFIX__CB:
            ++m_registers.r;
			// no 0xcb instructions modify PC directly so this method never needs to forcibly suppress update of the PC
			executeCbInstruction(instruction + 1, tStates, size);
			break;

		case Z80__PLAIN__PREFIX__ED:
            ++m_registers.r;
		    // some jumps and rets need to directly modify the PC
			executeEdInstruction(instruction + 1, &doPc, tStates, size);
			break;

		case Z80__PLAIN__PREFIX__DD:
            ++m_registers.r;
		    // instructions that work with IX
            // some jumps and rets need to directly modify the PC
			executeDdOrFdInstruction(m_registers.ix, instruction + 1, &doPc, tStates, size);
			break;

		case Z80__PLAIN__PREFIX__FD:
            ++m_registers.r;
            // instructions that work with IY
            // some jumps and rets need to directly modify the PC
			executeDdOrFdInstruction(m_registers.iy, instruction + 1, &doPc, tStates, size);
			break;

		default:
            // some jumps and rets need to directly modify the PC
			executePlainInstruction(instruction, &doPc, tStates, size);
			break;
	}

	// doPc is set to false by the instruction execution method if a jump was taken or the PC was otherwise directly
	// affected by the instruction
	if (doPc) {
        m_registers.pc += *size;
    }

#if (!defined(NDEBUG))
	historyEntry.registersAfter = registers();
    m_executionHistory.add(std::move(historyEntry));
#endif
}

void Z80::Z80::handleNmi()
{
    m_iff2 = m_iff1;
    m_iff1 = false;
    Z80__PUSH__REG16(m_registers.pc);
    m_registers.pc = 0x0066;
    m_halted = false;
    // TODO R register?
}

int Z80::Z80::handleInterrupt()
{
    int tStates = 0;
    m_iff1 = m_iff2 = false;
    // TODO R register?
    m_halted = false;

    switch (m_interruptMode) {
        case InterruptMode::IM0:
            std::cerr << "IM0 is not currently handled correctly.\n";
            // TODO if the instruction is a call or RST, push PC onto stack
            if (false/* is_call_or_rst */) {
                Z80__PUSH__REG16(m_registers.pc);
            }

            // TODO fetch the instruction from the device, up to 4 bytes
            // execute the instruction
//				execute(reinterpret_cast<UnsignedByte *>(&m_interruptData), false);
            // clear the instruction cache - actually just turns it into a NOP
            m_interruptData = 0x00;
            return 0;

        case InterruptMode::IM1:
            Z80__PUSH__REG16(m_registers.pc);
            m_registers.pc = 0x0038;
            return 13;

        case InterruptMode::IM2:
            Z80__PUSH__REG16(m_registers.pc);
            // interrupt service routine is pointed to by interrupt vector table starting at 0x{regI}00; byte on
            // data bus from interrupting device is offset into table (e.g. if device provides 0x20, the service
            // routine starts at the memory address stored at 0x{regI}20). For example, if I contains 0xfe and the
            // device puts 0x20 on the data bus, then the 16-bit word at 0xfe20 will be fetched. If the two bytes at
            // 0xfe20 and 0xfe21 are 0x38, 0x04 respectively, then the interrupt routine is located at address
            // 0x0438 (because Z80 words are little-endian) and the PC will jump to 0x0438 for the interrupt service
            // routine
            m_registers.pc = peekUnsignedWord(static_cast<UnsignedWord>(m_registers.i) << 8 | (m_interruptData & 0xfe));
            return 19;
            break;
    }

    // should never happen
    throw InvalidInterruptMode(static_cast<UnsignedByte>(m_interruptMode));
}

int Z80::Z80::fetchExecuteCycle()
{
    // a buffer in which to store the machine code instruction and operand data - we need this because in Spectrum
    // models from the 128k onwards the memory can be paged in, so we can't assume that we can just read the memory
    // pointer for the PC and keep adding offsets to it to retrieve bytes - when memory banks are paged in, the actual
    // next byte might not be the next byte in host memory if it crosses 0xc000, depending on which memory bank is
    // currently paged in
	static UnsignedByte machineCode[4];

	int tStates = 0;
	int size = 0;

	// TODO technically interrupts occur at the end of instruction processing
	if (m_nmiPending) {
	    handleNmi();
	    tStates += 11;
	    m_nmiPending = false;
	}

	// TODO defer interrupt by a single instruction if the next instruction is a return
	if (m_iff1 && m_interruptRequested) {
		// process maskable interrupt
        tStates += handleInterrupt();
        m_interruptRequested = false;
	}

	if (m_halted) {
        // execute NOPs while halted
        // TODO R register?
        return PlainOpcodeSizes[Z80__PLAIN__NOP];
    }

    auto bytesAvailable = memory()->size() - m_registers.pc;

    if (bytesAvailable < 4) {
        memory()->readBytes(m_registers.pc, bytesAvailable, machineCode);
        memory()->readBytes(0, 4 - bytesAvailable, machineCode + bytesAvailable);
    } else {
        memory()->readBytes(m_registers.pc, 4, machineCode);
    }

	execute(machineCode, true, &tStates, &size);
	return tStates;
}

void Z80::Z80::executePlainInstruction(const UnsignedByte * instruction, bool * doPc, int * tStates, int * size)
{
	bool useJumpCycleCost = false;

	switch(*instruction) {
		case Z80__PLAIN__NOP:							// 0x00
			/* nothing to do, just consume some tStates */
			break;

		case Z80__PLAIN__LD__BC__NN:					// 0x01
			Z80__LD__REG16__NN(m_registers.bc, z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))));
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
			m_registers.pc += static_cast<SignedByte>(*(instruction + 1));
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
				Z80_USE_JUMP_CYCLE_COST;
			}
			break;

		case Z80__PLAIN__LD__SP__NN:				// 0x31
			Z80__LD__REG16__NN(m_registers.sp, z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))));
			break;

		case Z80__PLAIN__LD__INDIRECT_NN__A:		// 0x32
			Z80__LD__INDIRECT_NN__REG8(z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))), m_registers.a);
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
            m_halted = true;
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
			// NOTE don't set the jumped indicator because there's no different t-state cost - the jump always takes
			//  place, so the base cost in t-states is all that's used
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
            // NOTE don't set the jumped indicator because there's no different t-state cost - the jump always takes
            //  place, so the base cost in t-states is all that's used
			break;

		case Z80__PLAIN__JP__Z__NN:					// 0xca
            m_registers.memptr = z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1)));

            if (Z80_FLAG_Z_ISSET) {
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Z80__PLAIN__PREFIX__CB:				// 0xcb
			std::cerr << "executePlainInstruction() called with opcode 0xcb. such an opcode should be handled by executeCbInstruction()" << "\n";
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

            if (!Z80_FLAG_C_ISSET) {
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Z80__PLAIN__OUT__INDIRECT_N__A:		// 0xd3
            Z80__OUT__INDIRECT_N__REG8(*(instruction + 1), m_registers.a);
            m_registers.memptr = m_registers.bc + 1;
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

            if (Z80_FLAG_C_ISSET) {
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Z80__PLAIN__IN__A__INDIRECT_N:		// 0xdb
            Z80__IN__REG8__INDIRECT_N(m_registers.a, *(instruction + 1));
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
			std::cerr << "executePlainInstruction() called with opcode 0xdd. such an opcode should be handled by executeDdInstruction()" << "\n";
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
            // NOTE don't set the jumped indicator because there's no different t-state cost - the jump always takes
            //  place, so the base cost in t-states is all that's used
			break;

		case Z80__PLAIN__JP__PE__NN:				// 0xea
            // the operand PO stands for "parity even"
            m_registers.memptr = z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1)));

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
			std::cerr << "executePlainInstruction() called with opcode 0xed. such an opcode should be handled by executeEdInstruction()" << "\n";
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
			std::cerr << "executePlainInstruction() called with opcode 0xfd. such an opcode should be handled by executeFdInstruction()" << "\n";
			break;

		case Z80__PLAIN__CP__N:						// 0xfe
			Z80__CP__N(*(instruction + 1));
			break;

		case Z80__PLAIN__RST__38:					// 0xff
			Z80__RST__N(0x38);
			Z80_DONT_UPDATE_PC;
			break;

		default:
			std::cerr << "unexpected opcode: 0x" << std::hex << (*instruction) << "\n";
            throw InvalidOpcode({*instruction}, m_registers.pc);
	}

	if (tStates) {
	    *tStates = static_cast<int>(useJumpCycleCost ? Z80_TSTATES_JUMP(PlainOpcodeTStates[*instruction]) : Z80_TSTATES_NOJUMP(PlainOpcodeTStates[*instruction]));
	}

	if (size) {
	    *size = static_cast<int>(PlainOpcodeSizes[*instruction]);
	}
}

// no 0xcb instructions directly modify the PC so we don't need to receive the (bool *) doPc parameter to indicate this
void Z80::Z80::executeCbInstruction(const UnsignedByte * instruction, int * tStates, int * size)
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
			Z80__BIT__N__INDIRECT_REG16(2, m_registers.hl);
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
			Z80__BIT__N__INDIRECT_REG16(3, m_registers.hl);
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
            std::cerr << "unexpected opcode: 0xcb 0x" << std::hex << (*instruction) << "\n";
            throw InvalidOpcode({0xcb, *instruction}, m_registers.pc);
	}

	if (tStates) {
	    *tStates = static_cast<int>(useJumpCycleCost ? Z80_TSTATES_JUMP(CbOpcodeTStates[*instruction]) : Z80_TSTATES_NOJUMP(CbOpcodeTStates[*instruction]));
	}

	if (size) {
	    *size = 2;
	}
}

void Z80::Z80::executeEdInstruction(const UnsignedByte * instruction, bool * doPc, int * tStates, int * size)
{
	bool useJumpCycleCost = false;

	switch (*instruction) {
		case Z80__ED__NOP__0XED__0X00:
        case Z80__ED__NOP__0XED__0X01:
        case Z80__ED__NOP__0XED__0X02:
        case Z80__ED__NOP__0XED__0X03:
        case Z80__ED__NOP__0XED__0X04:
        case Z80__ED__NOP__0XED__0X05:
        case Z80__ED__NOP__0XED__0X06:
        case Z80__ED__NOP__0XED__0X07:
        case Z80__ED__NOP__0XED__0X08:
        case Z80__ED__NOP__0XED__0X09:
        case Z80__ED__NOP__0XED__0X0A:
        case Z80__ED__NOP__0XED__0X0B:
        case Z80__ED__NOP__0XED__0X0C:
        case Z80__ED__NOP__0XED__0X0D:
        case Z80__ED__NOP__0XED__0X0E:
        case Z80__ED__NOP__0XED__0X0F:
        case Z80__ED__NOP__0XED__0X10:
        case Z80__ED__NOP__0XED__0X11:
        case Z80__ED__NOP__0XED__0X12:
        case Z80__ED__NOP__0XED__0X13:
        case Z80__ED__NOP__0XED__0X14:
        case Z80__ED__NOP__0XED__0X15:
        case Z80__ED__NOP__0XED__0X16:
        case Z80__ED__NOP__0XED__0X17:
        case Z80__ED__NOP__0XED__0X18:
        case Z80__ED__NOP__0XED__0X19:
        case Z80__ED__NOP__0XED__0X1A:
        case Z80__ED__NOP__0XED__0X1B:
        case Z80__ED__NOP__0XED__0X1C:
        case Z80__ED__NOP__0XED__0X1D:
        case Z80__ED__NOP__0XED__0X1E:
        case Z80__ED__NOP__0XED__0X1F:
        case Z80__ED__NOP__0XED__0X20:
        case Z80__ED__NOP__0XED__0X21:
        case Z80__ED__NOP__0XED__0X22:
        case Z80__ED__NOP__0XED__0X23:
        case Z80__ED__NOP__0XED__0X24:
        case Z80__ED__NOP__0XED__0X25:
        case Z80__ED__NOP__0XED__0X26:
        case Z80__ED__NOP__0XED__0X27:
        case Z80__ED__NOP__0XED__0X28:
        case Z80__ED__NOP__0XED__0X29:
        case Z80__ED__NOP__0XED__0X2A:
        case Z80__ED__NOP__0XED__0X2B:
        case Z80__ED__NOP__0XED__0X2C:
        case Z80__ED__NOP__0XED__0X2D:
        case Z80__ED__NOP__0XED__0X2E:
        case Z80__ED__NOP__0XED__0X2F:
        case Z80__ED__NOP__0XED__0X30:
        case Z80__ED__NOP__0XED__0X31:
        case Z80__ED__NOP__0XED__0X32:
        case Z80__ED__NOP__0XED__0X33:
        case Z80__ED__NOP__0XED__0X34:
        case Z80__ED__NOP__0XED__0X35:
        case Z80__ED__NOP__0XED__0X36:
        case Z80__ED__NOP__0XED__0X37:
        case Z80__ED__NOP__0XED__0X38:
        case Z80__ED__NOP__0XED__0X39:
        case Z80__ED__NOP__0XED__0X3A:
        case Z80__ED__NOP__0XED__0X3B:
        case Z80__ED__NOP__0XED__0X3C:
        case Z80__ED__NOP__0XED__0X3D:
        case Z80__ED__NOP__0XED__0X3E:
        case Z80__ED__NOP__0XED__0X3F:
			break;

		case Z80__ED__IN__B__INDIRECT_C:
            Z80__IN__REG8__INDIRECT_REG8(m_registers.b, m_registers.c);
			break;

		case Z80__ED__OUT__INDIRECT_C__B:
            Z80__OUT__INDIRECT_REG8__REG8(m_registers.c, m_registers.b);
			break;

		case Z80__ED__SBC__HL__BC:					/* 0xed 0x42 */
            Z80__SBC__REG16__REG16(m_registers.hl, m_registers.bc);
			break;

		case Z80__ED__LD__INDIRECT_NN__BC:		/* 0xed 0x43 */
			Z80__LD__INDIRECT_NN__REG16(z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))), m_registers.bc);
			break;

		case Z80__ED__NEG:							/* 0xed 0x44 */
			Z80_NEG;
			break;

		case Z80__ED__RETN:							/* 0xed 0c45 */
			Z80__RETN;
			Z80_DONT_UPDATE_PC;
			break;

		case Z80__ED__IM__0:			/* 0xed 0x46 */
			m_interruptMode = InterruptMode::IM0;
			break;

        case Z80__ED__LD__I__A:				/* 0xed 0x47 */
            Z80__LD__REG8__REG8(m_registers.i, m_registers.a);
            break;

		case Z80__ED__IN__C__INDIRECT_C:        // 0xed 0x48
            Z80__IN__REG8__INDIRECT_REG16(m_registers.c, m_registers.bc);
			break;

		case Z80__ED__OUT__INDIRECT_C__C:        /* 0xed 0x49 */
            Z80__OUT__INDIRECT_REG8__REG8(m_registers.c, m_registers.c);
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
			m_interruptMode = InterruptMode::IM0;
			break;

        case Z80__ED__LD__R__A:				/* 0xed 0x4f */
            Z80__LD__REG8__REG8(m_registers.r, m_registers.a);
            break;

		case Z80__ED__IN__D__INDIRECT_C:
            Z80__IN__REG8__INDIRECT_REG8(m_registers.d, m_registers.c);
			break;

		case Z80__ED__OUT__INDIRECT_C__D:
            Z80__OUT__INDIRECT_REG8__REG8(m_registers.c, m_registers.d);
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
			m_interruptMode = InterruptMode::IM1;
			break;

		case Z80__ED__LD__A__I:				/* 0xed 0x57 */
			Z80__LD__REG8__REG8(m_registers.a, m_registers.i);
			Z80_FLAGS_S53_UPDATE(m_registers.a);
			Z80_FLAG_Z_UPDATE(0 == m_registers.a);
			Z80_FLAG_H_CLEAR;
			Z80_FLAG_P_UPDATE(m_iff2);
			Z80_FLAG_N_CLEAR;
			break;

		case Z80__ED__IN__E__INDIRECT_C:
            Z80__IN__REG8__INDIRECT_REG8(m_registers.e, m_registers.c);
			break;

		case Z80__ED__OUT__INDIRECT_C__E:
            Z80__OUT__INDIRECT_REG8__REG8(m_registers.c, m_registers.e);
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
			m_interruptMode = InterruptMode::IM2;
			break;

		case Z80__ED__LD__A__R:                  /* 0xed 0x5f */
			Z80__LD__REG8__REG8(m_registers.a, m_registers.r);
			// NOTE carry flag is unmodified
			Z80_FLAGS_S53_UPDATE(m_registers.a);
			Z80_FLAG_Z_UPDATE(0 == m_registers.a);
			Z80_FLAG_H_CLEAR;
			Z80_FLAG_P_UPDATE(m_iff2);
			Z80_FLAG_N_CLEAR;
			break;

		case Z80__ED__IN__H__INDIRECT_C:	/* 0xed 0x60 */
            Z80__IN__REG8__INDIRECT_REG8(m_registers.h, m_registers.c);
			break;

		case Z80__ED__OUT__INDIRECT_C__H:
            Z80__OUT__INDIRECT_REG8__REG8(m_registers.c, m_registers.h);
			break;

		case Z80__ED__SBC__HL__HL:			/* 0xed 0x62 */
			Z80__SBC__REG16__REG16(m_registers.hl, m_registers.hl);
			break;

		case Z80__ED__LD__INDIRECT_NN__HL:
			Z80__LD__INDIRECT_NN__REG16(z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))), m_registers.hl);
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
			m_interruptMode = InterruptMode::IM0;
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
				Z80_FLAG_Z_UPDATE(0 == m_registers.a);
				Z80_FLAG_S_UPDATE(0x80 & m_registers.a);;
			}
			break;

		case Z80__ED__IN__L__INDIRECT_C:
            Z80__IN__REG8__INDIRECT_REG8(m_registers.l, m_registers.c);
			break;

		case Z80__ED__OUT__INDIRECT_C__L:
            Z80__OUT__INDIRECT_REG8__REG8(m_registers.c, m_registers.l);
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
			m_interruptMode = InterruptMode::IM0;
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
				Z80_FLAG_P_UPDATE(isEvenParity(m_registers.a));
				Z80_FLAG_Z_UPDATE(0 == m_registers.a);
				Z80_FLAGS_S53_UPDATE(m_registers.a);
			}
			break;

		case Z80__ED__IN__INDIRECT_C:           // 0xed 0x70
            {
                std::cout << "opcode 0xed 0x70 IN F,(C) - just setting flags\n";
                UnsignedByte tmpInByte;
                Z80__IN__REG8__INDIRECT_REG16(tmpInByte, m_registers.bc);
            }
			break;

		case Z80__ED__OUT__INDIRECT_C__0:   	/* 0xed 0x71 */
            Z80__OUT__INDIRECT_REG8__REG8(m_registers.c, 0);
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
			m_interruptMode = InterruptMode::IM1;
			break;

		case Z80__ED__NOP__0XED__0x77:	        /* 0xed 0x77 */
			break;

		case Z80__ED__IN__A__INDIRECT_C:   	    /* 0xed 0x78 */
            Z80__IN__REG8__INDIRECT_REG8(m_registers.a, m_registers.c);
			break;

		case Z80__ED__OUT__INDIRECT_C__A:   	/* 0xed 0x79 */
            Z80__OUT__INDIRECT_REG8__REG8(m_registers.c, m_registers.a);
			break;

		case Z80__ED__ADC__HL__SP:   	        /* 0xed 0x7a */
			Z80__ADC__REG16__REG16(m_registers.hl, m_registers.sp);
			break;

		case Z80__ED__LD__SP__INDIRECT_NN:   	/* 0xed 0x7b */
			Z80__LD__REG16__INDIRECT_NN(m_registers.sp, Z80::Z80::z80ToHostByteOrder(*((UnsignedWord *)(instruction + 1))));
			break;

		case Z80__ED__NEG__0XED__0X7C:	    	/* 0xed 0x7c */
			Z80_NEG;
			break;

		case Z80__ED__RETI__0XED__0X7D:	    	/* 0xed 0x7d */
			Z80__RETI;
			Z80_DONT_UPDATE_PC;
			break;

		case Z80__ED__IM__2__0XED__0X7E:		/* 0xed 0x7e */
			/* non-standard instruction; not guaranteed that this is the instruction
			 * in all versions of the Z80 */
			m_interruptMode = InterruptMode::IM2;
			break;

		case Z80__ED__NOP__0XED__0X7F:
        case Z80__ED__NOP__0XED__0X80:
        case Z80__ED__NOP__0XED__0X81:
        case Z80__ED__NOP__0XED__0X82:
        case Z80__ED__NOP__0XED__0X83:
        case Z80__ED__NOP__0XED__0X84:
        case Z80__ED__NOP__0XED__0X85:
        case Z80__ED__NOP__0XED__0X86:
        case Z80__ED__NOP__0XED__0X87:
        case Z80__ED__NOP__0XED__0X88:
        case Z80__ED__NOP__0XED__0X89:
        case Z80__ED__NOP__0XED__0X8A:
        case Z80__ED__NOP__0XED__0X8B:
        case Z80__ED__NOP__0XED__0X8C:
        case Z80__ED__NOP__0XED__0X8D:
        case Z80__ED__NOP__0XED__0X8E:
        case Z80__ED__NOP__0XED__0X8F:
        case Z80__ED__NOP__0XED__0X90:
        case Z80__ED__NOP__0XED__0X91:
        case Z80__ED__NOP__0XED__0X92:
        case Z80__ED__NOP__0XED__0X93:
        case Z80__ED__NOP__0XED__0X94:
        case Z80__ED__NOP__0XED__0X95:
        case Z80__ED__NOP__0XED__0X96:
        case Z80__ED__NOP__0XED__0X97:
        case Z80__ED__NOP__0XED__0X98:
        case Z80__ED__NOP__0XED__0X99:
        case Z80__ED__NOP__0XED__0X9A:
        case Z80__ED__NOP__0XED__0X9B:
        case Z80__ED__NOP__0XED__0X9C:
        case Z80__ED__NOP__0XED__0X9D:
        case Z80__ED__NOP__0XED__0X9E:
        case Z80__ED__NOP__0XED__0X9F:
			break;

		case Z80__ED__LDI:      // 0xed 0xa0
		    {
		        auto tmpByte = peekUnsigned(m_registers.hl);
                pokeUnsigned(m_registers.de, tmpByte);
                m_registers.de++;
                m_registers.hl++;
                m_registers.bc--;
                Z80_FLAG_H_CLEAR;
                Z80_FLAG_N_CLEAR;
                Z80_FLAG_P_UPDATE(0 != m_registers.bc);
                Z80_FLAG_F5_UPDATE(tmpByte & (Z80_FLAG_F5_MASK >> 4));
                Z80_FLAG_F3_UPDATE(tmpByte & Z80_FLAG_F3_MASK);
            }
			break;

		case Z80__ED__CPI:      // 0xed 0xa1
			{
				bool flagC = Z80_FLAG_C_ISSET;
				Z80__CP__INDIRECT_REG16(m_registers.hl);

				m_registers.hl++;
				m_registers.bc--;

				Z80_FLAG_C_UPDATE(flagC);
				Z80_FLAG_P_UPDATE(0 != m_registers.bc);
				Z80_FLAG_F5_UPDATE(m_registers.a & (Z80_FLAG_F5_MASK >> 4));
				Z80_FLAG_F3_UPDATE(m_registers.a & Z80_FLAG_F3_MASK);
			}
			break;

		case Z80__ED__INI:      // 0xed 0xa2
            {
                UnsignedByte result;
                Z80__READ_IO_DEVICES(result, m_registers.bc);
                pokeUnsigned(m_registers.hl, result);
                UnsignedByte carryCheck = result + m_registers.c + 1;
                --m_registers.b;
                ++m_registers.hl;

                Z80_FLAG_H_UPDATE(carryCheck < result);
                Z80_FLAG_C_UPDATE(carryCheck < result);
                Z80_FLAG_P_UPDATE(isEvenParity(static_cast<UnsignedByte>((carryCheck & 0x07) ^ m_registers.b)));
                Z80_FLAG_Z_UPDATE(0 == m_registers.b);
                Z80_FLAGS_S53_UPDATE(m_registers.b);
                Z80_FLAG_N_UPDATE(result & 0x80);
            }
			break;

		case Z80__ED__OUTI:      // 0xed 0xa3
            {
                UnsignedByte value = peekUnsigned(m_registers.hl);
                Z80__WRITE_IO_DEVICES(value, m_registers.bc);
                --m_registers.b;
                ++m_registers.hl;
                UnsignedByte carryCheck = value + m_registers.l;

                Z80_FLAG_H_UPDATE(carryCheck < value);
                Z80_FLAG_C_UPDATE(carryCheck < value);
                Z80_FLAG_P_UPDATE(isEvenParity(static_cast<UnsignedByte>((carryCheck & 0x07) ^ m_registers.b)));
                Z80_FLAG_Z_UPDATE(0 == m_registers.b);
                Z80_FLAGS_S53_UPDATE(m_registers.b);
                Z80_FLAG_N_UPDATE(value & 0x80);
            }
			break;

		case Z80__ED__NOP__0XED__0XA4:
		case Z80__ED__NOP__0XED__0XA5:
		case Z80__ED__NOP__0XED__0XA6:
		case Z80__ED__NOP__0XED__0XA7:
			break;

		case Z80__ED__LDD:      // 0xed 0xa8
		    {
		        auto value = peekUnsigned(m_registers.hl);
                pokeUnsigned(m_registers.de, value);
                m_registers.de--;
                m_registers.hl--;
                m_registers.bc--;
                Z80_FLAG_H_CLEAR;
                Z80_FLAG_N_CLEAR;
                Z80_FLAG_P_UPDATE(0 != m_registers.bc);
                value += m_registers.a;
                Z80_FLAG_F5_UPDATE(value & (Z80_FLAG_F5_MASK >> 4));
                Z80_FLAG_F3_UPDATE(value & Z80_FLAG_F3_MASK);
            }
			break;

		case Z80__ED__CPD:      // 0xed 0xa9
			{
                auto sub = peekUnsigned(m_registers.hl);
                auto result = m_registers.a - sub;
				m_registers.hl--;
				m_registers.bc--;

                Z80_FLAG_N_SET;
				Z80_FLAG_P_UPDATE(0 != m_registers.bc);
				Z80_FLAG_Z_UPDATE(0 == result);
                Z80_FLAG_H_UPDATE_SUB(m_registers.a, sub, result);
                Z80_FLAG_S_UPDATE(result & Z80_FLAG_S_MASK);

				if (Z80_FLAG_H_ISSET) {
				    --result;
				}

				Z80_FLAG_F5_UPDATE(result & (Z80_FLAG_F5_MASK >> 4));
				Z80_FLAG_F3_UPDATE(result & Z80_FLAG_F3_MASK);
			}
			break;

		case Z80__ED__IND:      // 0xed 0xaa
            {
                UnsignedByte result;
                Z80__READ_IO_DEVICES(result, m_registers.bc);
                pokeUnsigned(m_registers.hl, result);
                UnsignedByte carryCheck = result + m_registers.c - 1;
                --m_registers.b;
                --m_registers.hl;

                Z80_FLAG_H_UPDATE(carryCheck < result);
                Z80_FLAG_C_UPDATE(carryCheck < result);
                Z80_FLAG_P_UPDATE(isEvenParity(static_cast<UnsignedByte>((carryCheck & 0x07) ^ m_registers.b)));
                Z80_FLAG_Z_UPDATE(0 == m_registers.b);
                Z80_FLAGS_S53_UPDATE(m_registers.b);
                Z80_FLAG_N_UPDATE(result & 0x80);
            }
			break;

		case Z80__ED__OUTD:      // 0xed 0xab
            {
                UnsignedByte value = peekUnsigned(m_registers.hl);
                Z80__WRITE_IO_DEVICES(value, m_registers.bc);
                --m_registers.b;
                --m_registers.hl;
                UnsignedByte carryCheck = value + m_registers.l;

                Z80_FLAG_H_UPDATE(carryCheck < value);
                Z80_FLAG_C_UPDATE(carryCheck < value);
                Z80_FLAG_P_UPDATE(isEvenParity(static_cast<UnsignedByte>((carryCheck & 0x07) ^ m_registers.b)));
                Z80_FLAG_Z_UPDATE(0 == m_registers.b);
                Z80_FLAGS_S53_UPDATE(m_registers.b);
                Z80_FLAG_N_UPDATE(value & 0x80);
            }
			break;

		case Z80__ED__NOP__0XED__0XAC:
		case Z80__ED__NOP__0XED__0XAD:
		case Z80__ED__NOP__0XED__0XAE:
		case Z80__ED__NOP__0XED__0XAF:
			break;

		// TODO R is always off by 4
		case Z80__ED__LDIR:     // 0xed 0xb0
            {
                // interrupts can occur while this instruction is processing so we can't just implement it as a loop
                auto value = peekUnsigned(m_registers.hl);
                pokeUnsigned(m_registers.de, value);
                value += m_registers.a;
                --m_registers.bc;
                ++m_registers.de;
                ++m_registers.hl;

                if (m_registers.bc) {
                    // repeat this instruction
                    m_registers.pc -= 2;
                    Z80_USE_JUMP_CYCLE_COST;
                }

                Z80_FLAG_H_CLEAR;
                Z80_FLAG_N_CLEAR;
                Z80_FLAG_P_UPDATE(0 != m_registers.bc);
                Z80_FLAG_F5_UPDATE(value & (Z80_FLAG_F5_MASK >> 4));
                Z80_FLAG_F3_UPDATE(value & Z80_FLAG_F3_MASK);
            }
			break;

		case Z80__ED__CPIR:     // 0xed 0xb1
            {
                // interrupts can occur while this instruction is processing so we can't just implement it as a loop
                auto value = peekUnsigned(m_registers.hl);
                UnsignedWord result = m_registers.a - value;
                --m_registers.bc;
                ++m_registers.hl;

                if (0 != result && m_registers.bc) {
                    // repeat this instruction
                    m_registers.pc -= 2;
                    Z80_USE_JUMP_CYCLE_COST;
                }

                Z80_FLAG_S_UPDATE(result & 0x80);
                Z80_FLAG_Z_UPDATE(0 == result);
                Z80_FLAG_H_UPDATE_SUB(m_registers.a, value, result);
                Z80_FLAG_N_SET;
                Z80_FLAG_P_UPDATE(0 != m_registers.bc);
                Z80_FLAG_F5_UPDATE(result & (Z80_FLAG_F5_MASK >> 4));
                Z80_FLAG_F3_UPDATE(result & Z80_FLAG_F3_MASK);
            }
			break;

		case Z80__ED__INIR:     // 0xed 0xb2
            {
                UnsignedByte result;
                Z80__READ_IO_DEVICES(result, m_registers.bc);
                pokeUnsigned(m_registers.hl, result);
                UnsignedByte carryCheck = result + m_registers.c + 1;
                --m_registers.b;
                ++m_registers.hl;

                if (0 != m_registers.b) {
                    // repeat this instruction
                    m_registers.pc -= 2;
                    Z80_USE_JUMP_CYCLE_COST;
                }

                Z80_FLAG_H_UPDATE(carryCheck < result);
                Z80_FLAG_C_UPDATE(carryCheck < result);
                Z80_FLAG_P_UPDATE(isEvenParity(static_cast<UnsignedByte>((carryCheck & 0x07) ^ m_registers.b)));
                Z80_FLAG_Z_UPDATE(0 == m_registers.b);
                Z80_FLAGS_S53_UPDATE(m_registers.b);
                Z80_FLAG_N_UPDATE(result & 0x80);
            }
			break;

		case Z80__ED__OTIR:     // 0xed 0xb3
            {
                UnsignedByte value = peekUnsigned(m_registers.hl);
                Z80__WRITE_IO_DEVICES(value, m_registers.bc);
                --m_registers.b;
                ++m_registers.hl;
                UnsignedByte carryCheck = value + m_registers.l;

                if (0 != m_registers.b) {
                    // repeat this instruction
                    m_registers.pc -= 2;
                    Z80_USE_JUMP_CYCLE_COST;
                }

                Z80_FLAG_H_UPDATE(carryCheck < value);
                Z80_FLAG_C_UPDATE(carryCheck < value);
                Z80_FLAG_P_UPDATE(isEvenParity(static_cast<UnsignedByte>((carryCheck & 0x07) ^ m_registers.b)));
                Z80_FLAG_Z_UPDATE(0 == m_registers.b);
                Z80_FLAGS_S53_UPDATE(m_registers.b);
                Z80_FLAG_N_UPDATE(value & 0x80);
            }
            break;

		case Z80__ED__NOP__0XED__0XB4:
		case Z80__ED__NOP__0XED__0XB5:
		case Z80__ED__NOP__0XED__0XB6:
		case Z80__ED__NOP__0XED__0XB7:
			break;

        // TODO R is always off by 4
		case Z80__ED__LDDR:
            {
                // interrupts can occur while this instruction is processing so we can't just implement it as a loop
                auto value = peekUnsigned(m_registers.hl);
                pokeUnsigned(m_registers.de, value);
                value += m_registers.a;
                --m_registers.bc;
                --m_registers.de;
                --m_registers.hl;

                if (m_registers.bc) {
                    // repeat this instruction
                    m_registers.pc -= 2;
                    Z80_USE_JUMP_CYCLE_COST;
                }

                Z80_FLAG_H_CLEAR;
                Z80_FLAG_N_CLEAR;
                Z80_FLAG_P_UPDATE(0 != m_registers.bc);
                Z80_FLAG_F5_UPDATE(value & (Z80_FLAG_F5_MASK >> 4));
                Z80_FLAG_F3_UPDATE(value & Z80_FLAG_F3_MASK);
            }
			break;

		case Z80__ED__CPDR:     //0xed 0xb9
            {
                // interrupts can occur while this instruction is processing so we can't just implement it as a loop
                auto value = peekUnsigned(m_registers.hl);
                UnsignedWord result = m_registers.a - value;
                --m_registers.bc;
                --m_registers.hl;

                if (0 != result && m_registers.bc) {
                    // repeat this instruction
                    m_registers.pc -= 2;
                    Z80_USE_JUMP_CYCLE_COST;
                }

                Z80_FLAG_S_UPDATE(result & 0x80);
                Z80_FLAG_Z_UPDATE(0 == result);
                Z80_FLAG_H_UPDATE_SUB(m_registers.a, value, result);
                Z80_FLAG_N_SET;
                Z80_FLAG_P_UPDATE(0 != m_registers.bc);
                Z80_FLAG_F5_UPDATE(result & (Z80_FLAG_F5_MASK >> 4));
                Z80_FLAG_F3_UPDATE(result & Z80_FLAG_F3_MASK);
            }
			break;

		case Z80__ED__INDR:     //0xed 0xba
        {
            UnsignedByte result;
            Z80__READ_IO_DEVICES(result, m_registers.bc);
            pokeUnsigned(m_registers.hl, result);
            UnsignedByte carryCheck = result + m_registers.c - 1;
            --m_registers.b;
            --m_registers.hl;

            if (0 != m_registers.b) {
                // repeat this instruction
                m_registers.pc -= 2;
                Z80_USE_JUMP_CYCLE_COST;
            }

            Z80_FLAG_H_UPDATE(carryCheck < result);
            Z80_FLAG_C_UPDATE(carryCheck < result);
            Z80_FLAG_P_UPDATE(isEvenParity(static_cast<UnsignedByte>((carryCheck & 0x07) ^ m_registers.b)));
            Z80_FLAG_Z_UPDATE(0 == m_registers.b);
            Z80_FLAGS_S53_UPDATE(m_registers.b);
            Z80_FLAG_N_UPDATE(result & 0x80);
        }
        break;

		case Z80__ED__OTDR:     // 0xed 0xbb
        {
            UnsignedByte value = peekUnsigned(m_registers.hl);
            --m_registers.b;
            Z80__WRITE_IO_DEVICES(value, m_registers.bc);
            --m_registers.hl;
            UnsignedByte carryCheck = value + m_registers.l;

            if (0 != m_registers.b) {
                // repeat this instruction
                m_registers.pc -= 2;
                Z80_USE_JUMP_CYCLE_COST;
            }

            Z80_FLAG_H_UPDATE(carryCheck < value);
            Z80_FLAG_C_UPDATE(carryCheck < value);
            Z80_FLAG_P_UPDATE(isEvenParity(static_cast<UnsignedByte>((carryCheck & 0x07) ^ m_registers.b)));
            Z80_FLAG_Z_UPDATE(0 == m_registers.b);
            Z80_FLAGS_S53_UPDATE(m_registers.b);
            Z80_FLAG_N_UPDATE(value & 0x80);
        }
        break;

		case Z80__ED__NOP__0XED__0XBC:
		case Z80__ED__NOP__0XED__0XBD:
		case Z80__ED__NOP__0XED__0XBE:
		case Z80__ED__NOP__0XED__0XBF:
		case Z80__ED__NOP__0XED__0XC0:
        case Z80__ED__NOP__0XED__0XC1:
        case Z80__ED__NOP__0XED__0XC2:
        case Z80__ED__NOP__0XED__0XC3:
        case Z80__ED__NOP__0XED__0XC4:
        case Z80__ED__NOP__0XED__0XC5:
        case Z80__ED__NOP__0XED__0XC6:
        case Z80__ED__NOP__0XED__0XC7:
        case Z80__ED__NOP__0XED__0XC8:
        case Z80__ED__NOP__0XED__0XC9:
        case Z80__ED__NOP__0XED__0XCA:
        case Z80__ED__NOP__0XED__0XCB:
        case Z80__ED__NOP__0XED__0XCC:
        case Z80__ED__NOP__0XED__0XCD:
        case Z80__ED__NOP__0XED__0XCE:
        case Z80__ED__NOP__0XED__0XCF:
        case Z80__ED__NOP__0XED__0XD0:
        case Z80__ED__NOP__0XED__0XD1:
        case Z80__ED__NOP__0XED__0XD2:
        case Z80__ED__NOP__0XED__0XD3:
        case Z80__ED__NOP__0XED__0XD4:
        case Z80__ED__NOP__0XED__0XD5:
        case Z80__ED__NOP__0XED__0XD6:
        case Z80__ED__NOP__0XED__0XD7:
        case Z80__ED__NOP__0XED__0XD8:
        case Z80__ED__NOP__0XED__0XD9:
        case Z80__ED__NOP__0XED__0XDA:
        case Z80__ED__NOP__0XED__0XDB:
        case Z80__ED__NOP__0XED__0XDC:
        case Z80__ED__NOP__0XED__0XDD:
        case Z80__ED__NOP__0XED__0XDE:
        case Z80__ED__NOP__0XED__0XDF:
        case Z80__ED__NOP__0XED__0XE0:
        case Z80__ED__NOP__0XED__0XE1:
        case Z80__ED__NOP__0XED__0XE2:
        case Z80__ED__NOP__0XED__0XE3:
        case Z80__ED__NOP__0XED__0XE4:
        case Z80__ED__NOP__0XED__0XE5:
        case Z80__ED__NOP__0XED__0XE6:
        case Z80__ED__NOP__0XED__0XE7:
        case Z80__ED__NOP__0XED__0XE8:
        case Z80__ED__NOP__0XED__0XE9:
        case Z80__ED__NOP__0XED__0XEA:
        case Z80__ED__NOP__0XED__0XEB:
        case Z80__ED__NOP__0XED__0XEC:
        case Z80__ED__NOP__0XED__0XED:
        case Z80__ED__NOP__0XED__0XEE:
        case Z80__ED__NOP__0XED__0XEF:
        case Z80__ED__NOP__0XED__0XF0:
        case Z80__ED__NOP__0XED__0XF1:
        case Z80__ED__NOP__0XED__0XF2:
        case Z80__ED__NOP__0XED__0XF3:
        case Z80__ED__NOP__0XED__0XF4:
        case Z80__ED__NOP__0XED__0XF5:
        case Z80__ED__NOP__0XED__0XF6:
        case Z80__ED__NOP__0XED__0XF7:
        case Z80__ED__NOP__0XED__0XF8:
        case Z80__ED__NOP__0XED__0XF9:
        case Z80__ED__NOP__0XED__0XFA:
        case Z80__ED__NOP__0XED__0XFB:
        case Z80__ED__NOP__0XED__0XFC:
        case Z80__ED__NOP__0XED__0XFD:
        case Z80__ED__NOP__0XED__0XFE:
        case Z80__ED__NOP__0XED__0XFF:
			break;

		default:
            std::cerr << "unexpected opcode: 0xed 0x" << std::hex << (*instruction) << "\n";
            throw InvalidOpcode({0xed, *instruction}, m_registers.pc);
	}

	if (tStates) {
	    *tStates = static_cast<int>(useJumpCycleCost ? Z80_TSTATES_JUMP(EdOpcodeTStates[*instruction]) : Z80_TSTATES_NOJUMP(EdOpcodeTStates[*instruction]));
	}

	if (size) {
	    *size = EdOpcodeSizes[*instruction];
	}
}

void Z80::Z80::executeDdOrFdInstruction(UnsignedWord & reg, const UnsignedByte * instruction, bool * doPc, int * tStates, int * size)
{
	bool useJumpCycleCost = false;

	// work out which high and low reg pointers to use
	UnsignedByte * regHigh;
	UnsignedByte * regLow;

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
			Z80__ADC__REG8__INDIRECT_REG16_D(m_registers.a, reg, SignedByte(*(instruction + 1)));
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
			Z80__EX__INDIRECT_REG16__REG16(m_registers.sp, reg);
			break;

        case Z80__DD_OR_FD__PUSH__IX_OR_IY:     //  0xe5
            Z80__PUSH__REG16(reg);
            break;

		case Z80__DD_OR_FD__JP__IX_OR_IY: /*  0xe9 */
		    // NOTE unlike the plain E9 that uses (HL) this IS NOT INDIRECT and is just JP IX or JP IY
			m_registers.pc = reg;
			Z80_DONT_UPDATE_PC;
			break;

		case Z80__DD_OR_FD__LD__SP__IX_OR_IY: /*  0xf9 */
			Z80__LD__REG16__REG16(m_registers.sp, reg);
			break;

		case Z80__DD_OR_FD__PREFIX__CB: /*  0xcb */
			return executeDdcbOrFdcbInstruction(reg, instruction + 1, tStates, size);

		/* the following are all (expensive) replicas of plain instructions, so
		 * defer to the plain opcode executor method */
		case Z80__DD_OR_FD__NOP:                // 0x00
		case Z80__DD_OR_FD__LD__BC__NN:         // 0x01
		case Z80__DD_OR_FD__LD__INDIRECT_BC__A: // 0x02
		case Z80__DD_OR_FD__INC__BC:            // 0x03
		case Z80__DD_OR_FD__INC__B:             // 0x04
		case Z80__DD_OR_FD__DEC__B:             // 0x05
		case Z80__DD_OR_FD__LD__B__N:           // 0x06
		case Z80__DD_OR_FD__RLCA:               // 0x07

		case Z80__DD_OR_FD__EX__AF__AF_SHADOW:  // 0x08
		case Z80__DD_OR_FD__LD__A__INDIRECT_BC: // 0x0a
		case Z80__DD_OR_FD__DEC__BC:            // 0x0b
		case Z80__DD_OR_FD__INC__C:             // 0x0c
		case Z80__DD_OR_FD__DEC__C:             // 0x0d
		case Z80__DD_OR_FD__LD__C__N:           // 0x0e
		case Z80__DD_OR_FD__RRCA:               // 0x0f

		case Z80__DD_OR_FD__DJNZ__d:            // 0x10
		case Z80__DD_OR_FD__LD__DE__NN:         // 0x11
		case Z80__DD_OR_FD__LD__INDIRECT_DE__A: // 0x12
		case Z80__DD_OR_FD__INC__DE:            // 0x13
		case Z80__DD_OR_FD__INC__D:             // 0x14
		case Z80__DD_OR_FD__DEC__D:             // 0x15
		case Z80__DD_OR_FD__LD__D__N:           // 0x16
		case Z80__DD_OR_FD__RLA:                // 0x17

		case Z80__DD_OR_FD__JR__d:              // 0x18
		case Z80__DD_OR_FD__LD__A__INDIRECT_DE: // 0x1a
		case Z80__DD_OR_FD__DEC__DE:            // 0x1b
		case Z80__DD_OR_FD__INC__E:             // 0x1c
		case Z80__DD_OR_FD__DEC__E:             // 0x1d
		case Z80__DD_OR_FD__LD__E__N:           // 0x1e
		case Z80__DD_OR_FD__RRA:                // 0x1f

		case Z80__DD_OR_FD__JR__NZ__d:          // 0x20
		case Z80__DD_OR_FD__DAA:                // 0x27

		case Z80__DD_OR_FD__JR__Z__d:           // 0x28
		case Z80__DD_OR_FD__CPL:                // 0x2f

		case Z80__DD_OR_FD__JR__NC__d:          // 0x30
		case Z80__DD_OR_FD__LD__SP__NN:         // 0x31
		case Z80__DD_OR_FD__LD__INDIRECT_NN__A: // 0x32
		case Z80__DD_OR_FD__INC__SP:            // 0x33
		case Z80__DD_OR_FD__SCF:                // 0x37

		case Z80__DD_OR_FD__JR__C__d:           // 0x38
		case Z80__DD_OR_FD__LD__A__INDIRECT_NN: // 0x3a
		case Z80__DD_OR_FD__DEC__SP:            // 0x3b
		case Z80__DD_OR_FD__INC__A:             // 0x3c
		case Z80__DD_OR_FD__DEC__A:             // 0x3d
		case Z80__DD_OR_FD__LD__A__N:           // 0x3e
		case Z80__DD_OR_FD__CCF:                // 0x3f

		case Z80__DD_OR_FD__LD__B__B:           // 0x40
		case Z80__DD_OR_FD__LD__B__C:           // 0x41
		case Z80__DD_OR_FD__LD__B__D:           // 0x42
		case Z80__DD_OR_FD__LD__B__E:           // 0x43
		case Z80__DD_OR_FD__LD__B__A:           // 0x47

		case Z80__DD_OR_FD__LD__C__B:           // 0x48
		case Z80__DD_OR_FD__LD__C__C:           // 0x49
		case Z80__DD_OR_FD__LD__C__D:           // 0x4a
		case Z80__DD_OR_FD__LD__C__E:           // 0x4b
		case Z80__DD_OR_FD__LD__C__A:           // 0x4f

		case Z80__DD_OR_FD__LD__D__B:           // 0x50
		case Z80__DD_OR_FD__LD__D__C:           // 0x51
		case Z80__DD_OR_FD__LD__D__D:           // 0x52
		case Z80__DD_OR_FD__LD__D__E:           // 0x53
		case Z80__DD_OR_FD__LD__D__A:           // 0x57

		case Z80__DD_OR_FD__LD__E__B:           // 0x58
		case Z80__DD_OR_FD__LD__E__C:           // 0x59
		case Z80__DD_OR_FD__LD__E__D:           // 0x5a
		case Z80__DD_OR_FD__LD__E__E:           // 0x5b
		case Z80__DD_OR_FD__LD__E__A:           // 0x5f

		case Z80__DD_OR_FD__HALT:               // 0x76

		case Z80__DD_OR_FD__LD__A__B:           // 0x78
		case Z80__DD_OR_FD__LD__A__C:           // 0x79
		case Z80__DD_OR_FD__LD__A__D:           // 0x7a
		case Z80__DD_OR_FD__LD__A__E:           // 0x7b
		case Z80__DD_OR_FD__LD__A__A:           // 0x7f

		case Z80__DD_OR_FD__ADD__A__B:          // 0x80
		case Z80__DD_OR_FD__ADD__A__C:          // 0x81
		case Z80__DD_OR_FD__ADD__A__D:          // 0x82
		case Z80__DD_OR_FD__ADD__A__E:          // 0x83
		case Z80__DD_OR_FD__ADD__A__A:          // 0x87

		case Z80__DD_OR_FD__ADC__A__B:          // 0x88
		case Z80__DD_OR_FD__ADC__A__C:          // 0x89
		case Z80__DD_OR_FD__ADC__A__D:          // 0x8a
		case Z80__DD_OR_FD__ADC__A__E:          // 0x8b
		case Z80__DD_OR_FD__ADC__A__A:          // 0x8f

		case Z80__DD_OR_FD__SUB__B:             // 0x90
		case Z80__DD_OR_FD__SUB__C:             // 0x91
		case Z80__DD_OR_FD__SUB__D:             // 0x92
		case Z80__DD_OR_FD__SUB__E:             // 0x93
		case Z80__DD_OR_FD__SUB__A:             // 0x97

		case Z80__DD_OR_FD__SBC__A__B:          // 0x98
		case Z80__DD_OR_FD__SBC__A__C:          // 0x99
		case Z80__DD_OR_FD__SBC__A__D:          // 0x9a
		case Z80__DD_OR_FD__SBC__A__E:          // 0x9b
		case Z80__DD_OR_FD__SBC__A__A:          // 0x9f

		case Z80__DD_OR_FD__AND__B:             // 0xa0
		case Z80__DD_OR_FD__AND__C:             // 0xa1
		case Z80__DD_OR_FD__AND__D:             // 0xa2
		case Z80__DD_OR_FD__AND__E:             // 0xa3
		case Z80__DD_OR_FD__AND__A:             // 0xa7

		case Z80__DD_OR_FD__XOR__B:             // 0xa8
		case Z80__DD_OR_FD__XOR__C:             // 0xa9
		case Z80__DD_OR_FD__XOR__D:             // 0xaa
		case Z80__DD_OR_FD__XOR__E:             // 0xab
		case Z80__DD_OR_FD__XOR__A:             // 0xaf

		case Z80__DD_OR_FD__OR__B:              // 0xb0
		case Z80__DD_OR_FD__OR__C:              // 0xb1
		case Z80__DD_OR_FD__OR__D:              // 0xb2
		case Z80__DD_OR_FD__OR__E:              // 0xb3
		case Z80__DD_OR_FD__OR__A:              // 0xb7

		case Z80__DD_OR_FD__CP__B:              // 0xb8
		case Z80__DD_OR_FD__CP__C:              // 0xb9
		case Z80__DD_OR_FD__CP__D:              // 0xba
		case Z80__DD_OR_FD__CP__E:              // 0xbb
		case Z80__DD_OR_FD__CP__A:              // 0xbf

		case Z80__DD_OR_FD__RET__NZ:            // 0xc0
		case Z80__DD_OR_FD__POP__BC:            // 0xc1
		case Z80__DD_OR_FD__JP__NZ__NN:         // 0xc2
		case Z80__DD_OR_FD__JP__NN:             // 0xc3
		case Z80__DD_OR_FD__CALL__NZ__NN:       // 0xc4
		case Z80__DD_OR_FD__PUSH__BC:           // 0xc5
		case Z80__DD_OR_FD__ADD__A__N:          // 0xc6
		case Z80__DD_OR_FD__RST__00:            // 0xc7

		case Z80__DD_OR_FD__RET__Z:             // 0xc8
		case Z80__DD_OR_FD__RET:                // 0xc9
		case Z80__DD_OR_FD__JP__Z__NN:          // 0xca
		case Z80__DD_OR_FD__CALL__Z__NN:        // 0xcc
		case Z80__DD_OR_FD__CALL__NN:           // 0xcd
		case Z80__DD_OR_FD__ADC__A__N:          // 0xce
		case Z80__DD_OR_FD__RST__08:            // 0xcf

		case Z80__DD_OR_FD__RET__NC:            // 0xd0
		case Z80__DD_OR_FD__POP__DE:            // 0xd1
		case Z80__DD_OR_FD__JP__NC__NN:         // 0xd2
		case Z80__DD_OR_FD__OUT__INDIRECT_N__A: // 0xd3
		case Z80__DD_OR_FD__CALL__NC__NN:       // 0xd4
		case Z80__DD_OR_FD__PUSH__DE:           // 0xd5
		case Z80__DD_OR_FD__SUB__N:             // 0xd6
		case Z80__DD_OR_FD__RST__10:            // 0xd7

		case Z80__DD_OR_FD__RET__C:             // 0xd8
		case Z80__DD_OR_FD__EXX:                // 0xd9
		case Z80__DD_OR_FD__JP__C__NN:          // 0xda
		case Z80__DD_OR_FD__IN__A__INDIRECT_N:  // 0xdb
		case Z80__DD_OR_FD__CALL__C__NN:        // 0xdc
		case Z80__DD_OR_FD__SBC__A__N:          // 0xde
		case Z80__DD_OR_FD__RST__18:            // 0xdf

		case Z80__DD_OR_FD__RET__PO:            // 0xe0
		case Z80__DD_OR_FD__JP__PO__NN:         // 0xe2
		case Z80__DD_OR_FD__CALL__PO__NN:       // 0xe4
		case Z80__DD_OR_FD__AND__N:             // 0xe6
		case Z80__DD_OR_FD__RST__20:            // 0xe7

		case Z80__DD_OR_FD__RET__PE:            // 0xe8
		case Z80__DD_OR_FD__JP__PE__NN:         // 0xea
		case Z80__DD_OR_FD__EX__DE__HL:         // 0xeb
		case Z80__DD_OR_FD__CALL__PE__NN:       // 0xec
		case Z80__DD_OR_FD__PREFIX__ED:         // 0xed
		case Z80__DD_OR_FD__XOR__N:             // 0xee
		case Z80__DD_OR_FD__RST__28:            // 0xef

		case Z80__DD_OR_FD__RET__P:             // 0xf0
		case Z80__DD_OR_FD__POP__AF:            // 0xf1
		case Z80__DD_OR_FD__JP__P__NN:          // 0xf2
		case Z80__DD_OR_FD__DI:                 // 0xf3
		case Z80__DD_OR_FD__CALL__P__NN:        // 0xf4
		case Z80__DD_OR_FD__PUSH__AF:           // 0xf5
		case Z80__DD_OR_FD__OR__N:              // 0xf6
		case Z80__DD_OR_FD__RST__30:            // 0xf7

		case Z80__DD_OR_FD__RET__M:             // 0xf8
		case Z80__DD_OR_FD__JP__M__NN:          // 0xfa
		case Z80__DD_OR_FD__EI:                 // 0xfb
		case Z80__DD_OR_FD__CALL__M__NN:        // 0xfc
		case Z80__DD_OR_FD__CP__N:              // 0xfe
		case Z80__DD_OR_FD__RST__38:            // 0xff
            {
                // these are (expensive) replicas of plain instructions, so defer to the plain opcode executor method
                executePlainInstruction(instruction + 1, doPc, tStates, size);
    
                if (size) {
                    *size += 1;
                }
            }
            break;

        case Z80__DD_OR_FD__PREFIX__DD:         // 0xdd
        case Z80__DD_OR_FD__PREFIX__FD:         // 0xfd
#if (!defined(NDEBUG))
            // this is like a NOP - the second 0xdd or 0xfd supersedes the first and consumes 1 byte and 4 t-states. The
            // PC is subsequently incremented and the second 0xdd or 0xfd becomes the first byte of the next instruction
            std::cout << "Encountered redundant double-extended 0x"
                << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint16_t>(*instruction) << '\n'
                << std::dec << std::setfill(' ');
#endif
            if (tStates) {
                *tStates = 4;
            }

            if (size) {
                *size = 1;
            }
            return;
//            {
//                // TODO this assumes that instruction is a pointer into the spectrum memory and that the memory is a
//                //  linear array of bytes which it isn't. probably isn't any problem in simply returning as a NOP - the
//                //  PC wil be updated and the CPU will call execute() with the PC pointing to the repeated 0xdd or 0xfd
//                // this is an (expensive) replica of the extension instruction
//                --m_registers.r;
//                executeDdOrFdInstruction(reg, instruction + 1, doPc, tStates, size);
//
//                if (size) {
//                    *size += 1;
//                }
//            }
//            break;
	    
	    default:
            {
                UnsignedByte prefix = (&reg == &m_registers.ix ? 0xdd : 0xfd); 
                std::cerr << "unexpected opcode: 0x" << std::hex << prefix << " 0x" << (*instruction) << "\n";
                throw InvalidOpcode({prefix, *instruction}, m_registers.pc);
            }
    }

	if (tStates) {
	    *tStates = static_cast<int>(useJumpCycleCost ? Z80_TSTATES_JUMP(DdOrFdOpcodeTStates[*instruction]) : Z80_TSTATES_NOJUMP(DdOrFdOpcodeTStates[*instruction]));
	}

	if (size) {
	    *size = DdOrFdOpcodeSizes[*instruction];
	}
}

void Z80::Z80::executeDdcbOrFdcbInstruction(UnsignedWord & reg, const UnsignedByte * instruction, int * tStates, int * size)
{
	// NOTE these opcodes are of the form 0xdd 0xcb DD II or 0xfd 0xcb DD II where II is the 8-bit opcode and DD is the
	// 8-bit 2s-complement offset to use with IX or IY
	auto d = static_cast<SignedByte>(*(instruction));
	auto opcodeByte = *(instruction + 1);

	switch(opcodeByte) {
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

		// BIT opcodes
		// These are all the (IX/IY) + d equivalents of the 0xcb BIT opcodes that work with specific reg8s, except that
		// these versions don't use the reg8, so they're all just the BIT opcode on the memory offset - i.e. 8 identical
		// opcodes for each bit position
		case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x40 */
		case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x41 */
		case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x42 */
		case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x43 */
		case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x44 */
		case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x45 */
		case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x46 */
		case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x47 */
			Z80__BIT__N__INDIRECT_REG16_D(0, reg, d);
			break;

		case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x48 */
		case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x49 */
		case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x4a */
		case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x4b */
		case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x4c */
		case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x4d */
		case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x4e */
		case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x4f */
			Z80__BIT__N__INDIRECT_REG16_D(1, reg, d);
			break;

		case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x50 */
		case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x51 */
		case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x52 */
		case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x53 */
		case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x54 */
		case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x55 */
		case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x56 */
		case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x57 */
			Z80__BIT__N__INDIRECT_REG16_D(2, reg, d);
			break;

		case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x58 */
		case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x59 */
		case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x5a */
		case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x5b */
		case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x5c */
		case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x5d */
		case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x5e */
		case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x5f */
			Z80__BIT__N__INDIRECT_REG16_D(3, reg, d);
			break;

		case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x60 */
		case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x61 */
		case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x62 */
		case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x63 */
		case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x64 */
		case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x65 */
		case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x66 */
		case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x67 */
			Z80__BIT__N__INDIRECT_REG16_D(4, reg, d);
			break;

		case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x68 */
		case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x69 */
		case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x6a */
		case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x6b */
		case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x6c */
		case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x6d */
		case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x6e */
		case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x6f */
			Z80__BIT__N__INDIRECT_REG16_D(5, reg, d);
			break;

		case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x70 */
		case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x71 */
		case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x72 */
		case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x73 */
		case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x74 */
		case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x75 */
		case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x76 */
		case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x77 */
			Z80__BIT__N__INDIRECT_REG16_D(6, reg, d);
			break;

		case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__B:		/* 0xdd/0xfd 0xcb 0x78 */
		case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__C:		/* 0xdd/0xfd 0xcb 0x79 */
		case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__D:		/* 0xdd/0xfd 0xcb 0x7a */
		case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__E:		/* 0xdd/0xfd 0xcb 0x7b */
		case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__H:		/* 0xdd/0xfd 0xcb 0x7c */
		case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0x7d */
		case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d:		/* 0xdd/0xfd 0xcb 0x7e */
		case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0x7f */
			Z80__BIT__N__INDIRECT_REG16_D(7, reg, d);
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

	    case Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__L:		/* 0xdd/0xfd 0xcb 0xfd */
            Z80__SET__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.l);
            break;

	    case Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d:			/* 0xdd/0xfd 0xcb 0xfe */
            Z80__SET__N__INDIRECT_REG16_D(7, reg, d);
            break;

	    case Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__A:		/* 0xdd/0xfd 0xcb 0xff */
            Z80__SET__N__INDIRECT_REG16_D__REG8(7, reg, d, m_registers.a);
            break;
	}

	if (tStates) {
	    *tStates = DdCbOrFdCbOpcodeTStates[opcodeByte];
	}

	if (size) {
	    *size = 4;
	}
}

UnsignedWord Z80::Z80::registerValue(Register16 reg) const
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

UnsignedWord Z80::Z80::registerValueZ80(Register16 reg) const
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

UnsignedByte Z80::Z80::registerValue(Register8 reg) const
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

void Z80::Z80::setRegisterValue(Register16 reg, UnsignedWord value)
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

void Z80::Z80::setRegisterValueZ80(Register16 reg, UnsignedWord value)
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

void Z80::Z80::setRegisterValue(Register8 reg, UnsignedByte value)
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

#if (!defined(NDEBUG))
namespace
{
    void dumpRegisters(std::ostream & out, const ::Z80::Registers & registers)
    {
        out << std::hex << std::setfill('0')
            << "   AF     BC     DE     HL     IX     IY    AF'    BC'    DE'    HL'\n"
            << " $" << std::setw(4) << registers.af << ' '
            << " $" << std::setw(4) << registers.bc << ' '
            << " $" << std::setw(4) << registers.de << ' '
            << " $" << std::setw(4) << registers.hl << ' '
            << " $" << std::setw(4) << registers.ix << ' '
            << " $" << std::setw(4) << registers.iy << ' '
            << " $" << std::setw(4) << registers.afShadow << ' '
            << " $" << std::setw(4) << registers.bcShadow << ' '
            << " $" << std::setw(4) << registers.deShadow << ' '
            << " $" << std::setw(4) << registers.hlShadow << "\n\n"
            << "   PC     SP      I      R\n"
            << " $" << std::setw(4) << registers.pc
            << "  $" << std::setw(4) << registers.sp
            << "    $" << std::setw(2) << static_cast<std::uint16_t>(registers.i)
            << "    $" << std::setw(2) << static_cast<std::uint16_t>(registers.r) << "\n"
            << std::dec << std::setfill(' ');
    }
}

void Z80::Z80::dumpState(std::ostream & out) const
{
    out << "Z80 state:\n";
    dumpRegisters(out, registers());
    out << std::hex << std::setfill('0')
        << "\n  IM   IFF1  IFF2\n"
        << "   " << static_cast<std::uint16_t>(interruptMode())
        << "     " << (iff1() ? '1' : '0')
        << "     " << (iff2() ? '1' : '0') << "\n"
        << std::dec << std::setfill(' ');
}

void Z80::Z80::dumpExecutionHistory(int entries, std::ostream & out) const
{
    auto entry = m_executionHistory.newest();
    out << "Instruction History\n"
        << "===================\n";

    for (int instructionIndex = 0; instructionIndex < entries; ++instructionIndex) {
        out << "\n----------------------------------------------------------------------\n";
        out << "#" << std::dec << std::setw(0) << instructionIndex << " (@ 0x"
            << std::hex << std::setfill('0') << std::setw(4) << entry->registersBefore.pc << ")\n"
            << std::to_string(entry->mnemonic) << "          [" << std::hex << std::setfill('0');
        
        for (auto byteIndex = 0; byteIndex < entry->mnemonic.size; ++byteIndex) {
            if (0 < byteIndex) {
                out << ", ";
            }
            
            out << "0x" << std::setw(2) << static_cast<std::uint16_t>(entry->machineCode[byteIndex]);
        }
        
        out << "]\n";

        // TODO output operand values when captured
        out << std::to_string(entry->mnemonic.instruction) << ' ';

        for (auto operandIndex = 0; operandIndex < entry->operandValues.size(); ++operandIndex) {
            if (0 < operandIndex) {
                out << ", ";
            }

            switch (entry->mnemonic.operands[operandIndex].mode) {
                case Assembly::AddressingMode::Immediate:
                case Assembly::AddressingMode::Register8:
                    out << "0x" << std::setw(2) << static_cast<std::uint16_t>(entry->operandValues[operandIndex].unsignedByte);
                    break;

                case Assembly::AddressingMode::ImmediateExtended:
                case Assembly::AddressingMode::ModifiedPageZero:
                case Assembly::AddressingMode::Relative:
                case Assembly::AddressingMode::Extended:
                case Assembly::AddressingMode::Indexed:
                case Assembly::AddressingMode::Register16:
                case Assembly::AddressingMode::Register8Indirect:
                case Assembly::AddressingMode::Register16Indirect:
                    out << "0x" << std::setw(4) << static_cast<std::uint16_t>(entry->operandValues[operandIndex].unsignedWord);
                    break;

                case Assembly::AddressingMode::Bit:
                    out << std::setw(1) << static_cast<std::uint16_t>(entry->operandValues[operandIndex].bit);
                    break;
            }
        }

        out << "\nRegisters before:\n";
        dumpRegisters(out, entry->registersBefore);
        out << "\nRegisters after:\n";
        dumpRegisters(out, entry->registersAfter);

        // loop around in the ring buffer from the first entry to the last if necessary
        if (entry == m_executionHistory.begin()) {
            entry = m_executionHistory.end() - 1;
        } else {
            --entry;
        }
    }

    out << "\n----------------------------------------------------------------------\n";
}
#endif

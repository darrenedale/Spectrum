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
 * TODO there is definitely an issue with stack handling, this is what is causing chuckie egg to fail, and is a likely
 *  candidate for any snapshot that fails with a reset back to the ROM
 */
#include <iomanip>
#include <iostream>
#include <print>

#include "iodevice.h"
#include "invalidopcode.h"
#include "invalidinterruptmode.h"
#include "opcodes/opcodes.h"
#include "z80.h"
#include "../util/assert.h"
#include "../util/debug.h"

#if !defined(NDEBUG)
#include "assembly/disassembler.h"
#endif

#define Z80_FLAG_C_SET (m_registers.f |= Z80FlagCMask)
#define Z80_FLAG_Z_SET (m_registers.f |= Z80FlagZMask)
#define Z80_FLAG_P_SET (m_registers.f |= Z80FlagPMask)
#define Z80_FLAG_S_SET (m_registers.f |= Z80FlagSMask)
#define Z80_FLAG_N_SET (m_registers.f |= Z80FlagNMask)
#define Z80_FLAG_H_SET (m_registers.f |= Z80FlagHMask)
#define Z80_FLAG_F3_SET (m_registers.f |= Z80FlagF3Mask)
#define Z80_FLAG_F5_SET (m_registers.f |= Z80FlagF5Mask)

#define Z80_FLAG_C_CLEAR (m_registers.f &= ~Z80FlagCMask)
#define Z80_FLAG_Z_CLEAR (m_registers.f &= ~Z80FlagZMask)
#define Z80_FLAG_P_CLEAR (m_registers.f &= ~Z80FlagPMask)
#define Z80_FLAG_S_CLEAR (m_registers.f &= ~Z80FlagSMask)
#define Z80_FLAG_N_CLEAR (m_registers.f &= ~Z80FlagNMask)
#define Z80_FLAG_H_CLEAR (m_registers.f &= ~Z80FlagHMask)
#define Z80_FLAG_F3_CLEAR (m_registers.f &= ~Z80FlagF3Mask)
#define Z80_FLAG_F5_CLEAR (m_registers.f &= ~Z80FlagF5Mask)

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

#define Z80_FLAG_C_ISSET (0 != (m_registers.f & Z80FlagCMask))
#define Z80_FLAG_Z_ISSET (0 != (m_registers.f & Z80FlagZMask))
#define Z80_FLAG_P_ISSET (0 != (m_registers.f & Z80FlagPMask))
#define Z80_FLAG_S_ISSET (0 != (m_registers.f & Z80FlagSMask))
#define Z80_FLAG_N_ISSET (0 != (m_registers.f & Z80FlagNMask))
#define Z80_FLAG_H_ISSET (0 != (m_registers.f & Z80FlagHMask))

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
#define Z80_LD_Reg8_N(dest, n) ((dest) = (n))
#define Z80_LD_Reg8_Reg8(dest, src) ((dest) = (src))

// nn (the memory address to retrieve) MUST be in HOST byte order
#define Z80_LD_Reg8_IndirectNn(dest, nn) ((dest) = peekUnsigned(nn))
#define Z80_LD_Reg8_IndirectReg16(dest, src) ((dest) = peekUnsigned((src)))

// nn (the value to write to the register) MUST be in HOST byte order
#define Z80_LD_Reg16_NN(dest, nn) ((dest) = (nn))
#define Z80_LD_Reg16_Reg16(dest, src) ((dest) = (src))

#define Z80_LD_IndirectReg16_Reg8(dest, src) pokeUnsigned((dest), (src))

// nn (the memory address to load) MUST be in HOST byte order
// TODO check how we're handling memptr - this code seems to imply that unlike the other registers it's being stored
//  in Z80 byte order?
#define Z80_LD_IndirectNn_Reg16(nn, src) \
{                                            \
    auto tmpAddr = (nn);                     \
    pokeHostWord(tmpAddr, (src));            \
    m_registers.memptr = hostToZ80ByteOrder(tmpAddr); \
}

// nn (the memory address to retrieve) MUST be in HOST byte order
#define Z80_LD_Reg16_IndirectNn(dest, nn) ((dest) = peekUnsignedHostWord(nn))
#define Z80_LD_IndirectReg16_N(dest, n) (pokeUnsigned((dest), n))

// nn (the memory address to load) MUST be in HOST byte order
#define Z80_LD_IndirectNn_Reg8(nn, src) (pokeUnsigned(nn, (src)))
#define Z80_LD_IndirectReg16D_N(reg, d, n) Z80_LD_IndirectReg16_N(((reg) + (d)), (n));
#define Z80_LD_IndirectReg16D_Reg8(reg, d, src) Z80_LD_IndirectNn_Reg8((reg) + (d), (src));
#define Z80_LD_Reg8_IndirectReg16D(dest, reg, d) Z80_LD_Reg8_IndirectNn((dest), (reg) + (d));

// reset instructions
//
// addr can be 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30 or 0x38, and must be in HOST byte order
//
// FLAGS: no flags are modified.
#define Z80_RST_N(addr)             \
Z80_PUSH_Reg16(m_registers.pc + 1); \
m_registers.pc = (addr);              \
m_registers.memptr = m_registers.pc

/* context switching instructions
 *
 * FLAGS: no flags are modified.
 */
#define Z80_EX_Reg16_Reg16(src, dest)    \
{                                           \
    UnsignedWord tmpWord = (dest);          \
    (dest) = (src);                         \
    (src) = tmpWord;                        \
}

#define Z80_EX_IndirectReg16_Reg16(src, dest) \
{\
    UnsignedWord tmpWord = peekUnsignedHostWord((src));\
    pokeHostWord((src), (dest));                   \
    (dest) = tmpWord;                              \
}

//
// stack instructions
//
#define Z80_POP_Reg16(reg)              \
(reg) = peekUnsignedHostWord(m_registers.sp); \
m_registers.sp += 2;

#define Z80_PUSH_Reg16(reg) \
m_registers.sp -= 2;          \
pokeHostWord(m_registers.sp, (reg));

//
// addition instructions
//
#define Z80_ADD_Reg8_N(dest,n) \
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
    Z80_FLAG_F3_UPDATE((dest) & Z80FlagF3Mask);\
    Z80_FLAG_F5_UPDATE((dest) & Z80FlagF5Mask);\
}

#define Z80_ADD_Reg8_Reg8(dest,src) Z80_ADD_Reg8_N(dest,src)
#define Z80_ADD_Reg8_IndirectReg16(dest,src) Z80_ADD_Reg8_N(dest,(*(memory()->pointerTo(src))))

#define Z80_ADD_Reg16_Reg16(dest,src) \
{       \
    UnsignedWord tmpOldValue = (dest);           \
    UnsignedWord tmpDelta = (src);                 \
    std::uint32_t tmpResult = tmpOldValue + tmpDelta;  \
    (dest) = tmpResult & 0xffff;                 \
    Z80_FLAG_H_UPDATE_16_ADD(tmpOldValue, tmpDelta, tmpResult); \
    Z80_FLAG_N_CLEAR;                            \
    Z80_FLAG_C_UPDATE(tmpResult & 0x10000);\
    Z80_FLAG_F5_UPDATE((dest) & (Z80FlagF5Mask << 8)); \
    Z80_FLAG_F3_UPDATE((dest) & (Z80FlagF3Mask << 8)); \
}

#define Z80_ADD_Reg8_IndirectReg16D(dest, reg, d) Z80_ADD_Reg8_N((dest),(*(memory()->pointerTo((reg) + (d)))))

//
// addition with carry instructions
// dest = dest + src + carry
//
#define Z80_ADC_Reg8_N(dest,n) \
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

#define Z80_ADC_Reg8_Reg8(dest,src) Z80_ADC_Reg8_N((dest), (src));
#define Z80_ADC_Reg8_IndirectReg16(dest,src) Z80_ADC_Reg8_N((dest), peekUnsigned(src))

#define Z80_ADC_Reg16_Reg16(dest,src) \
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

#define Z80_ADC_Reg8_IndirectReg16D(dest, reg, d) Z80_ADC_Reg8_N((dest), peekUnsigned((reg) + (d)))

//
// subtraction instructions
//
#define Z80_SUB_N(n) \
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

#define Z80_SUB_Reg8(reg) Z80_SUB_N(reg)
#define Z80_SUB_IndirectReg16(reg) { UnsignedByte v = peekUnsigned(reg); Z80_SUB_N(v) }
#define Z80_SUB_IndirectReg16D(reg,d) Z80_SUB_N(peekUnsigned((reg) + (d)))

//
// subtraction with carry instructions
// dest = dest - src - carry
//
#define Z80_SBC_Reg8_N(dest,n) \
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

#define Z80_SBC_Reg8_Reg8(dest,src) Z80_SBC_Reg8_N((dest), (src))
#define Z80_SBC_Reg8_IndirectReg16(dest,src) Z80_SBC_Reg8_N((dest),peekUnsigned(src))

#define Z80_SBC_Reg16_Reg16(dest, src) \
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

#define Z80_SBC_Reg8_IndirectReg16D(dest,reg,d) Z80_SBC_Reg8_N((dest), peekUnsigned((reg) + (d)))

//
// increment instructions
//
#define Z80_INC_Reg8(reg)             \
(reg)++;                                \
Z80_FLAG_H_UPDATE(0 == (0x0f & (reg))); \
Z80_FLAG_P_UPDATE(0x80 == (reg));       \
Z80_FLAG_N_CLEAR;                       \
Z80_FLAG_Z_UPDATE(0 == (reg));          \
Z80_FLAG_S_UPDATE((reg) & 0x80);        \
Z80_FLAG_F3_UPDATE((reg) & Z80FlagF3Mask); \
Z80_FLAG_F5_UPDATE((reg) & Z80FlagF5Mask);

#define Z80_INC_IndirectReg16(reg) Z80_INC_Reg8(*(memory()->pointerTo(reg)))
#define Z80_INC_Reg16(reg) (reg)++;
// TODO memptr
#define Z80_INC_IndirectReg16D(reg, d) Z80_INC_Reg8(*(memory()->pointerTo((reg) + (d))))

//
// decrement instructions
//
#define Z80_DEC_Reg8(reg) \
Z80_FLAG_H_UPDATE(0 == (0x0f & (reg))); \
(reg)--;                    \
Z80_FLAG_P_UPDATE(0x7f == (reg)); \
Z80_FLAG_N_SET;             \
Z80_FLAG_Z_UPDATE(0 == (reg));   \
Z80_FLAG_S_UPDATE((reg) & 0x80); \
Z80_FLAG_F3_UPDATE((reg) & Z80FlagF3Mask); \
Z80_FLAG_F5_UPDATE((reg) & Z80FlagF5Mask);

#define Z80_DEC_IndirectReg16(reg) Z80_DEC_Reg8(*(memory()->pointerTo(reg)))
#define Z80_DEC_Reg16(reg) (reg)--;
// TODO memptr
#define Z80_DEC_IndirectReg16D(reg, d) Z80_DEC_Reg8(*(memory()->pointerTo((reg) + (d))));

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
#define Z80_CP_N(n) \
{ \
    UnsignedByte tmpDelta = (n);           \
    UnsignedWord tmpResult = m_registers.a - tmpDelta; \
    Z80_FLAG_C_UPDATE(tmpResult & 0x0100);                       \
    Z80_FLAG_N_SET;      \
    Z80_FLAG_P_UPDATE_OVERFLOW_SUB(m_registers.a, tmpDelta, tmpResult); \
    Z80_FLAG_H_UPDATE_SUB(m_registers.a, tmpDelta, tmpResult); \
    Z80_FLAG_S_UPDATE(tmpResult & Z80FlagSMask);\
    Z80_FLAG_F5_UPDATE(tmpDelta & Z80FlagF5Mask);\
    Z80_FLAG_F3_UPDATE(tmpDelta & Z80FlagF3Mask);\
    Z80_FLAG_Z_UPDATE(0 == tmpResult);      \
}

#define Z80_CP_Reg8(reg) Z80_CP_N(reg)
#define Z80_CP_IndirectReg16(reg) Z80_CP_N(peekUnsigned((reg)))
#define Z80_CP_IndirectReg16D(reg,d) Z80_CP_N(peekUnsigned((reg) + (d)))

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

#define Z80_AND_N(n) m_registers.a &= (n); Z80_BITWISE_FLAGS; Z80_FLAG_H_SET;
#define Z80_AND_Reg8(reg) Z80_AND_N((reg))
#define Z80_AND_IndirectReg16(reg) Z80_AND_N(peekUnsigned(reg))
#define Z80_AND_IndirectReg16D(reg,d) Z80_AND_N(peekUnsigned((reg) + (d)))

#define Z80_OR_N(n) m_registers.a |= (n); Z80_BITWISE_FLAGS; Z80_FLAG_H_CLEAR;
#define Z80_OR_Reg8(reg) Z80_OR_N((reg))
#define Z80_OR_IndirectReg16(reg) Z80_OR_N(peekUnsigned(reg))
#define Z80_OR_IndirectReg16D(reg,d) Z80_OR_N(peekUnsigned((reg) + (d)))

#define Z80_XOR_N(n) m_registers.a ^= (n); Z80_BITWISE_FLAGS; Z80_FLAG_H_CLEAR;
#define Z80_XOR_Reg8(reg) Z80_XOR_N((reg))
#define Z80_XOR_IndirectReg16(reg) Z80_XOR_N((peekUnsigned(reg)))
#define Z80_XOR_IndirectReg16D(reg,d) Z80_XOR_N(peekUnsigned((reg) + (d)))

//
// bit set instructions
//
/* FLAGS: all preserved */
#define Z80_SET_N_Reg8(n,reg) (reg) |= (1 << (n))
#define Z80_SET_N_IndirectReg16(n,reg) pokeUnsigned((reg), peekUnsigned((reg)) | (1 << (n)))
#define Z80_SET_N_IndirectReg16D(n,reg,d) Z80_SET_N_IndirectReg16(n,(reg) + (d))
#define Z80_SET_N_IndirectReg16D_Reg8(n,reg16,d,reg8) \
Z80_SET_N_IndirectReg16D(n, (reg16), (d)); \
(reg8) = peekUnsigned((reg16) + (d));

//
// bit reset instructions
//
#define Z80_RES_N_Reg8(n,reg) (reg) &= ~(1 << (n))
#define Z80_RES_N_IndirectReg16(n,reg) pokeUnsigned((reg), peekUnsigned((reg)) & ~(1 << (n)))
#define Z80_RES_N_IndirectReg16D(n,reg,d) Z80_RES_N_IndirectReg16(n,(reg) + (d))
#define Z80_RES_N_IndirectReg16D_Reg8(n,reg16,d,reg8) \
Z80_RES_N_IndirectReg16D(n,(reg16),(d));               \
(reg8) = peekUnsigned((reg16) + (d))

//
// bit shift and rotation instructions
//

//
// rotate left with carry instructions
//
#define Z80_RLC_Reg8(reg)              \
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

#define Z80_RLC_IndirectReg16(reg) {     \
    UnsignedByte * tmpValue = memory()->pointerTo(reg);\
    Z80_RLC_Reg8(*tmpValue);               \
}

#define Z80_RLC_IndirectReg16D(reg,d) Z80_RLC_IndirectReg16((reg) + (d))
#define Z80_RLC_IndirectReg16D_Reg8(reg16,d,reg8) Z80_RLC_IndirectReg16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

//
// rotate right with carry instructions
//
#define Z80_RRC_Reg8(reg) \
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
#define Z80_RRC_IndirectReg16(reg) \
{\
    UnsignedByte * tmpValue = memory()->pointerTo(reg); \
    Z80_RRC_Reg8(*tmpValue);          \
}
#define Z80_RRC_IndirectReg16D(reg,d) Z80_RRC_IndirectReg16((reg) + (d))
#define Z80_RRC_IndirectReg16D_Reg8(reg16,d,reg8) Z80_RRC_IndirectReg16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

//
// rotate left instructions
//
// value is rotated left. bit 7 moves into carry flag and carry flag
// moves into bit 0. In other words, it's as if the value was 9 bits in
// size with the carry flag as bit 8.
//
#define Z80_RL_Reg8(reg)        \
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
#define Z80_RL_IndirectReg16(reg) \
{      \
    UnsignedByte * tmpValue = memory()->pointerTo(reg);\
    Z80_RL_Reg8(*tmpValue);                \
}

#define Z80_RL_IndirectReg16D(reg,d) Z80_RL_IndirectReg16((reg) + (d))
#define Z80_RL_IndirectReg16D_Reg8(reg16,d,reg8) Z80_RL_IndirectReg16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

//
// rotate right instructions
//
// value is rotated right. bit 0 moves into carry flag and carry flag
// moves into bit 7. In other words, it's as if the value was 9 bits in
// size with the carry flag as bit 0.
//
#define Z80_RR_Reg8(reg) \
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
#define Z80_RR_IndirectReg16(reg) {      \
    UnsignedByte * tmpValue = memory()->pointerTo(reg);\
    Z80_RR_Reg8(*tmpValue);                \
}
#define Z80_RR_IndirectReg16D(reg,d) Z80_RR_IndirectReg16((reg) + (d))
#define Z80_RR_IndirectReg16D_Reg8(reg16,d,reg8) Z80_RR_IndirectReg16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

//
// arithmetic left shift instructions
//
#define Z80_SLA_Reg8(reg) {            \
	Z80_FLAG_C_UPDATE((reg) & 0x80);     \
	(reg) <<= 1;                         \
	Z80_FLAG_H_CLEAR;                    \
	Z80_FLAG_N_CLEAR;                    \
	Z80_FLAG_P_UPDATE(isEvenParity(reg));\
	Z80_FLAGS_S53_UPDATE((reg));         \
	Z80_FLAG_Z_UPDATE(0 == (reg));       \
}
#define Z80_SLA_IndirectReg16(reg) {     \
    UnsignedByte * tmpValue = memory()->pointerTo(reg);\
    Z80_SLA_Reg8(*tmpValue);               \
}
#define Z80_SLA_IndirectReg16D(reg,d) Z80_SLA_IndirectReg16((reg) + (d))
#define Z80_SLA_IndirectReg16D_Reg8(reg16,d,reg8) Z80_SLA_IndirectReg16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

//
// arithmetic right shift instructions
//
#define Z80_SRA_Reg8(reg)                \
{                                          \
	Z80_FLAG_C_UPDATE((reg) & 0x01);       \
    (reg) = ((reg) & 0x80) | ((reg) >> 1); \
	Z80_FLAG_H_CLEAR;                      \
	Z80_FLAG_N_CLEAR;                      \
	Z80_FLAG_P_UPDATE(isEvenParity(reg));  \
	Z80_FLAGS_S53_UPDATE((reg));           \
	Z80_FLAG_Z_UPDATE(0 == (reg));         \
}

#define Z80_SRA_IndirectReg16(reg)       \
{                                           \
    UnsignedByte * tmpValue = memory()->pointerTo(reg);\
    Z80_SRA_Reg8(*tmpValue);               \
}

#define Z80_SRA_IndirectReg16D(reg,d) Z80_SRA_IndirectReg16((reg) + (d))
#define Z80_SRA_IndirectReg16D_Reg8(reg16,d,reg8) Z80_SRA_IndirectReg16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

//
// logical left shift instruction
//
// bits are left shifted one place. bit 7 goes into carry flag and 1 goes into bit 0.
//
#define Z80_SLL_Reg8(reg) {            \
	Z80_FLAG_C_UPDATE((reg) & 0x80);     \
    (reg) = 0x01 | ((reg) << 1);         \
	Z80_FLAG_H_CLEAR;                    \
	Z80_FLAG_N_CLEAR;                    \
	Z80_FLAG_P_UPDATE(isEvenParity(reg));\
	Z80_FLAGS_S53_UPDATE((reg));  \
	Z80_FLAG_Z_UPDATE(0 == (reg));       \
}
#define Z80_SLL_IndirectReg16(reg) {     \
    UnsignedByte * tmpValue = memory()->pointerTo(reg);\
    Z80_SLL_Reg8(*tmpValue);               \
}
#define Z80_SLL_IndirectReg16D(reg,d) Z80_SLL_IndirectReg16((reg) + (d))
#define Z80_SLL_IndirectReg16D_Reg8(reg16,d,reg8) Z80_SLL_IndirectReg16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

//
// logical right shift instructions
//
// bits are right shifted one place. bit 0 goes into carry flag and 1 goes into bit 7.
//
#define Z80_SRL_Reg8(reg) {       \
	Z80_FLAG_C_UPDATE((reg) & 0x01);\
	(reg) >>= 1;                    \
	Z80_FLAG_H_CLEAR;               \
	Z80_FLAG_N_CLEAR;               \
	Z80_FLAG_P_UPDATE(isEvenParity(reg));\
	Z80_FLAGS_S53_UPDATE((reg));    \
	Z80_FLAG_Z_UPDATE(0 == (reg));  \
}
#define Z80_SRL_IndirectReg16(reg) { \
    UnsignedByte * tmpValue = memory()->pointerTo(reg);\
    Z80_SRL_Reg8(*tmpValue);               \
}
#define Z80_SRL_IndirectReg16D(reg,d) Z80_SRL_IndirectReg16((reg) + (d))
#define Z80_SRL_IndirectReg16D_Reg8(reg16,d,reg8) Z80_SRL_IndirectReg16((reg16) + (d)); (reg8) = peekUnsigned((reg16) + (d));

//
// bit testing instructions
//
#define Z80_BIT_N_Reg8(n,reg) \
Z80_FLAG_Z_UPDATE(0 == ((reg) & (0x01 << n))); \
Z80_FLAG_P_UPDATE(Z80_FLAG_Z_ISSET);        \
Z80_FLAG_N_CLEAR;                \
Z80_FLAG_H_SET;                  \
Z80_FLAG_F5_UPDATE((reg) & Z80FlagF5Mask);\
Z80_FLAG_F3_UPDATE((reg) & Z80FlagF3Mask);\
Z80_FLAG_S_UPDATE((n) == 7 && (reg) & Z80FlagSMask);

#define Z80_BIT_N_IndirectReg16(n,reg) \
Z80_BIT_N_Reg8(n,peekUnsigned(reg));    \
Z80_FLAG_F5_UPDATE(m_registers.memptrH & Z80FlagF5Mask);\
Z80_FLAG_F3_UPDATE(m_registers.memptrH & Z80FlagF3Mask);

#define Z80_BIT_N_IndirectReg16D(n,reg,d) \
m_registers.memptr = ((reg) + (d)); \
Z80_BIT_N_IndirectReg16(n,m_registers.memptr);

//
// nmi handler return instruction
//
// there is only one nmi return instruction, but it has several opcodes (most of
// which are unofficial), so a macro is provided for a common implementation.
//
#define Z80_RETN                \
Z80_POP_Reg16(m_registers.pc); \
m_iff1 = m_iff2;

//
// interrupt handler return instruction
//
// there is only one interrupt return instruction, but it has several opcodes
// (most of which are unofficial), so a macro is provided for a common implementation.
//
#define Z80_RETI                \
Z80_POP_Reg16(m_registers.pc); \
m_iff1 = m_iff2;                 \
/* TODO signal IO device that interrupt has finished */

//
// port IO instructions
//

// helper to perform byte write to connected IO devices
#define Z80_WRITE_IO_DEVICES(value, port) { \
    for (auto * device : m_ioDevices) {      \
        if (!device->checkWritePort(port)) { \
            continue;                        \
        }                                    \
                                             \
        device->writeByte(port, value);      \
    }                                        \
}

// port is 8-bit and is the LSB for the 16-bit port.
#define Z80_OUT_IndirectReg8_Reg8(port,value) {    \
    UnsignedWord tmpPort = ((port) & 0xff | (m_registers.b << 8)); \
    Z80_WRITE_IO_DEVICES((value), tmpPort)            \
    m_registers.memptr = tmpPort + 1;                  \
}

#define Z80_OUT_Indirect_N_Reg8(port,value) {       \
    UnsignedWord tmpPort = ((port) & 0xff | (m_registers.a << 8)); \
    Z80_WRITE_IO_DEVICES((value), tmpPort)            \
    m_registers.memptr = tmpPort + 1;                  \
}

// TODO if multiple devices are reading from a given port, what happens to the result?
#define Z80_READ_IO_DEVICES(result, port) { \
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

#define Z80_IN_Reg8_IndirectReg8(dest,port) {    \
    UnsignedWord tmpPort = ((port) & 0xff) | (m_registers.b << 8); \
    Z80_READ_IO_DEVICES((dest), tmpPort);           \
    m_registers.memptr = tmpPort + 1;                \
    Z80_FLAG_H_CLEAR;                                \
    Z80_FLAG_N_CLEAR;                                \
    Z80_FLAG_Z_UPDATE(0 == (dest));                  \
    Z80_FLAGS_S53_UPDATE((dest));                    \
    Z80_FLAG_P_UPDATE(isEvenParity((dest)));         \
}

#define Z80_IN_Reg8_IndirectReg16(dest,port) {   \
/*    UnsignedWord tmpPort = ((port) & 0xff) | (m_registers.b << 8);*/ \
    Z80_READ_IO_DEVICES((dest), (port));           \
/*    m_registers.memptr = tmpPort + 1; */               \
    Z80_FLAG_H_CLEAR;                                \
    Z80_FLAG_N_CLEAR;                                \
    Z80_FLAG_Z_UPDATE(0 == (dest));                  \
    Z80_FLAGS_S53_UPDATE((dest));                    \
    Z80_FLAG_P_UPDATE(isEvenParity((dest)));         \
}

#define Z80_IN_Reg8_Indirect_N(dest,port) {       \
    UnsignedWord tmpPort = ((port) & 0xff) | (m_registers.a << 8); \
    Z80_READ_IO_DEVICES((dest), tmpPort);           \
}

using UnsignedByte = ::Z80::UnsignedByte;
using UnsignedWord = ::Z80::UnsignedWord;
using SignedByte = ::Z80::SignedByte;

namespace
{
    bool isEvenParity(const UnsignedByte value)
    {
        static bool parity[256] = {
#include "includes/8bit_parity.inc"
        };

        return parity[value];
    }
}

constexpr const std::uint8_t Z80::Z80::PlainOpcodeSizes[256] = {
#include "includes/z80_plain_opcode_sizes.inc"
};

// NOTE all 0xcb opcodes are 2 bytes in size

constexpr const std::uint8_t Z80::Z80::EdOpcodeSizes[256] = {
#include "includes/z80_ed_opcode_sizes.inc"
};

constexpr const std::uint8_t Z80::Z80::DdOrFdOpcodeSizes[256] = {
#include "includes/z80_ddorfd_opcode_sizes.inc"
};

constexpr const int Z80::Z80::PlainOpcodeTStates[256] = {
#include "includes/z80_plain_opcode_tstates.inc"
};

constexpr const std::uint8_t Z80::Z80::CbOpcodeTStates[256] = {
#include "includes/z80_cb_opcode_tstates.inc"
};

constexpr const std::uint8_t Z80::Z80::EdOpcodeTStates[256] = {
#include "includes/z80_ed_opcode_tstates.inc"
};

constexpr const int Z80::Z80::DdOrFdOpcodeTStates[256] = {
#include "includes/z80_ddorfd_opcode_tstates.inc"
};

constexpr const std::uint8_t Z80::Z80::DdCbOrFdCbOpcodeTStates[256] = {
#include "includes/z80_ddorfd_cb_opcode_tstates.inc"
};

Z80::Z80::Z80(Memory * memory)
: Cpu(memory),
  m_registers(),
  m_tStates(0),
  m_iff1(false),
  m_iff2(false),
  m_interruptMode(InterruptMode::IM0),
  m_nmiPending(false),
  m_interruptRequested(false),
  m_delayInterruptOneInstruction(false),
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
    m_tStates = 0;
	m_nmiPending = false;
    m_interruptMode = InterruptMode::IM0;
    m_interruptData = 0x00;
	m_iff1 = false;
	m_iff2 = false;
    m_interruptRequested = false;
    m_halted = false;
	m_registers.reset();
}

void Z80::Z80::pokeHostWord(const MemoryType::Address addr, UnsignedWord value)
{
    sp_assert(memory() && 0 <= addr && memory()->addressableSize() > (addr + 1), "null memory or invalid address {:#04x} poking host word", addr);

	value = hostToZ80ByteOrder(value);
    memory()->writeBytes(addr, 2, reinterpret_cast<const UnsignedByte *>(&value));
}

void Z80::Z80::pokeZ80Word(const MemoryType::Address addr, const UnsignedWord value)
{
    sp_assert(memory() && 0 <= addr && memory()->addressableSize() > (addr + 1), "null memory or invalid address {:#04x} poking Z80 word", addr);
    memory()->writeBytes(addr, 2, reinterpret_cast<const UnsignedByte *>(&value));
}

void Z80::Z80::pokeUnsigned(const MemoryType::Address addr, const UnsignedByte value)
{
    sp_assert(memory() && 0 <= addr && memory()->addressableSize() > addr, "null memory or invalid address {:#04x} poking unsigned byte", addr);
    memory()->writeByte(addr, value);
}

UnsignedWord Z80::Z80::peekUnsignedHostWord(const MemoryType::Address addr) const
{
    sp_assert(memory() && 0 <= addr && memory()->addressableSize() > (addr + 1), "null memory or invalid address {:#04x} peeking host word", addr);

    if constexpr (HostByteOrder == Z80ByteOrder) {
        return (*memory())[addr + 1] << 8 | (*memory())[addr];
    }

    return ((*memory())[addr] << 8) | (*memory())[addr + 1];
}

UnsignedWord Z80::Z80::peekUnsignedZ80Word(const MemoryType::Address addr) const
{
    sp_assert(memory() && 0 <= addr && memory()->addressableSize() > (addr + 1), "null memory or invalid address {:#04x} peeking Z80 word", addr);

    return (*memory())[addr + 1] << 8 | (*memory())[addr];
}

Z80::InstructionCost Z80::Z80::execute(const UnsignedByte * instruction, bool doPc)
{
	sp_assert(instruction, "null instruction passed to Z80::Z80::execute");
    ++m_registers.r;

    // cost is always assigned in the switch() so we don't need to initialise it here
    InstructionCost cost;

#if !defined(NDEBUG) && defined(DEBUG_EXECUTION_HISTORY)
	ExecutedInstruction historyEntry(instruction, this);
#endif

    switch (*instruction) {
		case Opcodes::Z80_Plain_Prefix_Cb:
            ++m_registers.r;
			// no 0xcb instructions modify PC directly so this method never needs to forcibly suppress update of the PC
			cost = executeCbInstruction(instruction + 1);
			break;

		case Opcodes::Z80_Plain_Prefix_Ed:
            ++m_registers.r;
		    // some jumps and rets need to directly modify the PC
            cost = executeEdInstruction(instruction + 1, &doPc);
			break;

		case Opcodes::Z80_Plain_Prefix_Dd:
            ++m_registers.r;
		    // instructions that work with IX
            // some jumps and rets need to directly modify the PC
            cost = executeDdOrFdInstruction(m_registers.ix, instruction + 1, &doPc);
			break;

		case Opcodes::Z80_Plain_Prefix_Fd:
            ++m_registers.r;
            // instructions that work with IY
            // some jumps and rets need to directly modify the PC
            cost = executeDdOrFdInstruction(m_registers.iy, instruction + 1, &doPc);
			break;

		default:
            // some jumps and rets need to directly modify the PC
            cost = executePlainInstruction(instruction, &doPc);
			break;
	}

    // doPc is set to false by the instruction execution method if a jump was taken or the PC was otherwise directly
	// affected by the instruction
	if (doPc) {
        m_registers.pc += cost.size;
    }

#if !defined(NDEBUG) && defined(DEBUG_EXECUTION_HISTORY)
	historyEntry.registersAfter = registers();
    m_executionHistory.add(std::move(historyEntry));

    // expensive debug code to monitor changes to the stack pointer. only enable when specifically building to examine
    // stack changes
//    if (historyEntry.registersBefore.sp != historyEntry.registersAfter.sp) {
//        Util::debug << std::hex << std::setfill('0');
//        Util::debug << "\nStack changed by instruction at 0x" << std::setw(4) << historyEntry.registersBefore.pc << "\n";
//        auto mnemonic = Assembly::Disassembler::disassembleOne(instruction);
//        Util::debug << "  Instruction: " << to_string(mnemonic) << "\n";
//        Util::debug << "  Machine code:";
//
//        for (int idx = 0; idx < mnemonic.size; ++idx) {
//            Util::debug << " 0x" << std::setw(2) << static_cast<std::uint16_t>(instruction[idx]);
//        }
//
//        Util::debug << "\n  SP before: 0x" << std::setw(4) << historyEntry.registersBefore.sp << '\n';
//        Util::debug << "\n  SP after : 0x" << std::setw(4) << historyEntry.registersAfter.sp << '\n';
//
//        if (0xfffe < historyEntry.registersAfter.sp) {
//            Util::debug("  Stack is now empty");
//        } else {
//            Util::debug << "\n  Top of stack: 0x" << std::setw(4) << peekUnsignedHostWord(historyEntry.registersAfter.sp);
//            Util::debug << "   [0x" << std::setw(2) << static_cast<std::uint16_t>(peekUnsigned(historyEntry.registersAfter.sp));
//            Util::debug << " 0x" << std::setw(2) << static_cast<std::uint16_t>(peekUnsigned(historyEntry.registersAfter.sp + 1)) << "]\n";
//        }
//
//        Util::debug << '\n';
//    }
#endif

    return cost;
}

void Z80::Z80::handleNmi()
{
    m_iff2 = m_iff1;
    m_iff1 = false;

    if (m_halted) {
        m_halted = false;
        ++m_registers.pc;
    }

    Z80_PUSH_Reg16(m_registers.pc);
    m_registers.pc = 0x0066;
    ++m_registers.r;
    m_tStates += 5;
}

int Z80::Z80::handleInterrupt()
{
    m_iff1 = m_iff2 = false;
    ++m_registers.r;

    if (m_halted) {
        // HALTing the CPU (either via HALT instruction or by some device signalling the HALT pin) freezes the PC. In
        // either case, once we resume we need the PC to move on to the next instruction (otherwise it would simply
        // re-execute the HALT)
        m_halted = false;
        ++m_registers.pc;
    }

    // Z80__PUSH__REG16(m_registers.pc);

    // TODO when we support IRequest, reset if needed

    switch (m_interruptMode) {
        case InterruptMode::IM0:
            // Util::debug("IM0 is not currently handled correctly");;
            // TODO if the instruction is a call or RST, push PC onto stack
            // if (false/* is_call_or_rst */) {
                Z80_PUSH_Reg16(m_registers.pc);
            // }

            // TODO fetch the instruction from the device, up to 4 bytes
            // execute the instruction
            //				execute(reinterpret_cast<UnsignedByte *>(&m_interruptData), false);
            // clear the instruction cache - actually just turns it into a NOP

            // TODO this is only suitable for Spectrums, which only use RST $0038
            switch (m_interruptData) {
                case InterruptRst00:
                    m_registers.pc = 0x0000;
                    break;

                case InterruptRst08:
                    m_registers.pc = 0x0008;
                    break;

                case InterruptRst10:
                    m_registers.pc = 0x0010;
                    break;

                case InterruptRst18:
                    m_registers.pc = 0x0018;
                    break;

                case InterruptRst20:
                    m_registers.pc = 0x0020;
                    break;

                case InterruptRst28:
                    m_registers.pc = 0x0028;
                    break;

                case InterruptRst30:
                    m_registers.pc = 0x0030;
                    break;

                case InterruptRst38:
                    m_registers.pc = 0x0038;
                    break;

                [[unlikely]]
                default:
                    sp_assert(false, "invalid interrupt vector {} in interrupt mode IM0", m_interruptData);
                    break;
            }

            m_interruptData = 0x00;
            return 13;

        case InterruptMode::IM1:
            Z80_PUSH_Reg16(m_registers.pc);
            m_registers.pc = 0x0038;
            return 13;

        case InterruptMode::IM2:
            Z80_PUSH_Reg16(m_registers.pc);
            // interrupt service routine is pointed to by interrupt vector table starting at 0x{regI}00; byte on
            // data bus from interrupting device is offset into table (e.g. if device provides 0x20, the service
            // routine starts at the memory address stored at 0x{regI}20). For example, if I contains 0xfe and the
            // device puts 0x20 on the data bus, then the 16-bit word at 0xfe20 will be fetched. If the two bytes at
            // 0xfe20 and 0xfe21 are 0x38, 0x04 respectively, then the interrupt routine is located at address
            // 0x0438 (because Z80 words are little-endian) and the PC will jump to 0x0438 for the interrupt service
            // routine
            m_registers.pc = peekUnsignedHostWord(static_cast<UnsignedWord>(m_registers.i) << 8 | (m_interruptData & 0xfe));
            return 19;
    }

    // should never happen
    [[unlikely]]
    throw InvalidInterruptMode(static_cast<UnsignedByte>(m_interruptMode));
}

int Z80::Z80::fetchExecuteCycle()
{
#if (!defined(NDEBUG))
    static bool haltAndDiWarningShown = false;
#endif
    // a buffer in which to store the machine code instruction and operand data - we need this because in Spectrum
    // models from the 128k onwards the memory can be paged in, so we can't assume that we can just read the memory
    // pointer for the PC and keep adding offsets to it to retrieve bytes - when memory banks are paged in, the actual
    // next byte might not be the next byte in host memory if it crosses 0xc000, depending on which memory bank is
    // currently paged in
	static UnsignedByte machineCode[4];

	int tStates = 0;
    // the Z80 defers a pending interrupt by one instruction after EI to allow for a RET to be executed - EI instruction
    // handling code sets this to ensure the interrupt is delayed
    m_delayInterruptOneInstruction = false;

	if (m_halted) {
        // execute NOPs while halted
        // TODO R register?
        tStates = PlainOpcodeTStates[Opcodes::Z80_Plain_Nop];
    } else {
        if (const auto bytesAvailable = memory()->addressableSize() - m_registers.pc; bytesAvailable < 4) {
            memory()->readBytes(m_registers.pc, bytesAvailable, machineCode);
            memory()->readBytes(0, 4 - bytesAvailable, machineCode + bytesAvailable);
        } else {
            memory()->readBytes(m_registers.pc, 4, machineCode);
        }

        tStates = execute(machineCode, true).tStates;
    }

    if (m_nmiPending) {
        handleNmi();
        tStates += 11;
        m_nmiPending = false;
    }

    if (m_iff1 && m_interruptRequested) {
        if (!m_delayInterruptOneInstruction) {
            // process maskable interrupt
            tStates += handleInterrupt();
            m_interruptRequested = false;
        }
    }

#if (!defined(NDEBUG))
    if (m_halted && !m_iff1) {
        if (!haltAndDiWarningShown) {
            Util::debugln("CPU is halted and interrupts are disabled - the CPU cannot be resumed");
        }
    } else {
        haltAndDiWarningShown = false;
    }
#endif

    m_tStates += tStates;
	return tStates;
}

Z80::InstructionCost Z80::Z80::executePlainInstruction(const UnsignedByte * instruction, bool * doPc)
{
	bool useJumpCycleCost = false;

	switch(*instruction) {
		case Opcodes::Z80_Plain_Nop:							// 0x00
			/* nothing to do, just consume some tStates */
			break;

		case Opcodes::Z80_Plain_Ld_Bc_Nn:					// 0x01
            Z80_LD_Reg16_NN(m_registers.bc, z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1)));
			break;

		case Opcodes::Z80_Plain_Ld_IndirectBc_A:		// 0x02
			Z80_LD_IndirectReg16_Reg8(m_registers.bc, m_registers.a);
			break;

		case Opcodes::Z80_Plain_Inc_Bc:						// 0x03
			Z80_INC_Reg16(m_registers.bc);
			break;

		case Opcodes::Z80_Plain_Inc_B:						// 0x04
			Z80_INC_Reg8(m_registers.b);
			break;

		case Opcodes::Z80_Plain_Dec_B:						// 0x05
			Z80_DEC_Reg8(m_registers.b);
			break;

		case Opcodes::Z80_Plain_Ld_B_N:					// 0x06
			Z80_LD_Reg8_N(m_registers.b, *(instruction + 1));
			break;

		case Opcodes::Z80_Plain_Rlca:						// 0x07
			{
				bool bit = m_registers.a & 0x80;
				m_registers.a <<= 1;

				if (bit) {
					m_registers.a |= 0x01;
					Z80_FLAG_C_SET;
				}
				else {
					m_registers.a &= 0xfe;
					Z80_FLAG_C_CLEAR;
				}

				Z80_FLAG_F5_UPDATE(m_registers.a & Z80FlagF5Mask);
				Z80_FLAG_F3_UPDATE(m_registers.a & Z80FlagF3Mask);
				Z80_FLAG_H_CLEAR;
				Z80_FLAG_N_CLEAR;
			}
			break;

		case Opcodes::Z80_Plain_Ex_Af_AfShadow:			// 0x08
			Z80_EX_Reg16_Reg16(m_registers.af, m_registers.afShadow);
			break;

		case Opcodes::Z80_Plain_Add_Hl_Bc:				// 0x09
			Z80_ADD_Reg16_Reg16(m_registers.hl, m_registers.bc);
			break;

		case Opcodes::Z80_Plain_Ld_A_IndirectBc:		// 0x0a
			Z80_LD_Reg8_IndirectReg16(m_registers.a, m_registers.bc);
			break;

		case Opcodes::Z80_Plain_Dec_Bc:						// 0x0b
			Z80_DEC_Reg16(m_registers.bc);
			break;

		case Opcodes::Z80_Plain_Inc_C:						// 0x0c
			Z80_INC_Reg8(m_registers.c);
			break;

		case Opcodes::Z80_Plain_Dec_C:						// 0x0d
			Z80_DEC_Reg8(m_registers.c);
			break;

		case Opcodes::Z80_Plain_Ld_C_N:					// 0x0e
			Z80_LD_Reg8_N(m_registers.c, *(instruction + 1));
			break;

		case Opcodes::Z80_Plain_Rrca:						// 0x0f
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
				Z80_FLAG_F3_UPDATE((m_registers.a & Z80FlagF3Mask));
				Z80_FLAG_F5_UPDATE((m_registers.a & Z80FlagF5Mask));
			}
			break;

		case Opcodes::Z80_Plain_Djnz_d:						// 0x10
			if (0 != --(m_registers.b)) {
				m_registers.pc += static_cast<SignedByte>(*(instruction + 1));
				m_registers.memptr = m_registers.pc;
				Z80_USE_JUMP_CYCLE_COST;
			}

			break;

		case Opcodes::Z80_Plain_Ld_De_Nn:				// 0x11
			Z80_LD_Reg16_NN(m_registers.de, *reinterpret_cast<const UnsignedWord *>(instruction + 1));
			break;

		case Opcodes::Z80_Plain_Ld_IndirectDe_A:		// 0x12
			Z80_LD_IndirectReg16_Reg8(m_registers.de, m_registers.a);
			break;

		case Opcodes::Z80_Plain_Inc_De:					// 0x13
			Z80_INC_Reg16(m_registers.de);
			break;

		case Opcodes::Z80_Plain_Inc_D:						// 0x14
			Z80_INC_Reg8(m_registers.d);
			break;

		case Opcodes::Z80_Plain_Dec_D:						// 0x15
			Z80_DEC_Reg8(m_registers.d);
			break;

		case Opcodes::Z80_Plain_Ld_D_N:					// 0x16
			Z80_LD_Reg8_N(m_registers.d, *(instruction + 1));
			break;

		case Opcodes::Z80_Plain_Rla:							// 0x17
			{
				/* re-use RL instruction, but cache all flags except C (which is
				 * modified by the instruction) and re-edit as they are different
				 * for RL and RLA
				 *
				 * FLAGS: S, Z and P preserved, C modified directly by instruction,
				 * H and N cleared */
				UnsignedByte flags = (m_registers.f & ~Z80FlagCMask);
				Z80_RL_Reg8(m_registers.a);
				m_registers.f = flags | (m_registers.f & Z80FlagCMask);
				Z80_FLAG_H_CLEAR;
				Z80_FLAG_N_CLEAR;
			}
			break;

		case Opcodes::Z80_Plain_Jr_d:						// 0x18
			m_registers.pc += static_cast<SignedByte>(*(instruction + 1));
			Z80_USE_JUMP_CYCLE_COST;
			break;

		case Opcodes::Z80_Plain_Add_Hl_De:				// 0x19
			Z80_ADD_Reg16_Reg16(m_registers.hl, m_registers.de);
			break;

		case Opcodes::Z80_Plain_Ld_A_IndirectDe:		// 0x1a
			Z80_LD_Reg8_IndirectReg16(m_registers.a, m_registers.de);
			break;

		case Opcodes::Z80_Plain_Dec_De:					// 0x1b
			Z80_DEC_Reg16(m_registers.de);
			break;

		case Opcodes::Z80_Plain_Inc_E:						// 0x1c
			Z80_INC_Reg8(m_registers.e);
			break;

		case Opcodes::Z80_Plain_Dec_E:						// 0x1d
			Z80_DEC_Reg8(m_registers.e);
			break;

		case Opcodes::Z80_Plain_Ld_E_N:					// 0x1e
			Z80_LD_Reg8_N(m_registers.e, *(instruction + 1));
			break;

		case Opcodes::Z80_Plain_Rra:							// 0x1f
			{
				/* re-use RR instruction, but cache all flags except C (which is
				 * modified by the instruction) and re-edit as they are different
				 * for RL and RLA
				 *
				 * FLAGS: S, Z and P preserved, C modified directly by instruction,
				 * H and N cleared */
				UnsignedByte flags = (m_registers.f & ~Z80FlagCMask);
				Z80_RR_Reg8(m_registers.a);
				m_registers.f = flags | (m_registers.f & Z80FlagCMask);
				Z80_FLAG_H_CLEAR;
				Z80_FLAG_N_CLEAR;
			}
			break;

		case Opcodes::Z80_Plain_Jr_Nz_d:					// 0x20
			if (!Z80_FLAG_Z_ISSET) {
				m_registers.pc += static_cast<SignedByte>(*(instruction + 1));
				Z80_USE_JUMP_CYCLE_COST;
			}
			break;

		case Opcodes::Z80_Plain_Ld_Hl_Nn:				// 0x21
			Z80_LD_Reg16_NN(m_registers.hl, *reinterpret_cast<const UnsignedWord *>(instruction + 1));
			break;

		case Opcodes::Z80_Plain_Ld_IndirectNn_Hl:	// 0x22
			Z80_LD_IndirectNn_Reg16(z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1)), m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Inc_Hl:					// 0x23
			Z80_INC_Reg16(m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Inc_H:						// 0x24
			Z80_INC_Reg8(m_registers.h);
			break;

		case Opcodes::Z80_Plain_Dec_H:						// 0x25
			Z80_DEC_Reg8(m_registers.h);
			break;

		case Opcodes::Z80_Plain_Ld_H_N:					// 0x26
			Z80_LD_Reg8_N(m_registers.h, *(instruction + 1));
			break;

		case Opcodes::Z80_Plain_Daa:							// 0x27
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
                    Z80_SUB_N(correction);
                } else {
                    Z80_ADD_Reg8_N(m_registers.a, correction);
                }

                // finally, set the flags that weren't take care of in the ADD/SUB instruction
                Z80_FLAG_C_UPDATE(carry);
                Z80_FLAG_P_UPDATE(isEvenParity(m_registers.a));
			}
			break;

		case Opcodes::Z80_Plain_Jr_Z_d:					// 0x28
			if (Z80_FLAG_Z_ISSET) {
				m_registers.pc += static_cast<SignedByte>(*(instruction + 1));
				Z80_USE_JUMP_CYCLE_COST;
			}
			break;

		case Opcodes::Z80_Plain_Add_Hl_Hl:				// 0x29
			Z80_ADD_Reg16_Reg16(m_registers.hl, m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Ld_Hl_IndirectNn:	// 0x2a
		    // NOTE the interface of the Z80 class expects addresses in host byte order
			Z80_LD_Reg16_IndirectNn(m_registers.hl, z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1)));
			break;

		case Opcodes::Z80_Plain_Dec_Hl:					// 0x2b
			Z80_DEC_Reg16(m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Inc_L:						// 0x2c
			Z80_INC_Reg8(m_registers.l);
			break;

		case Opcodes::Z80_Plain_Dec_L:						// 0x2d
			Z80_DEC_Reg8(m_registers.l);
			break;

		case Opcodes::Z80_Plain_Ld_L_N:					// 0x2e
			Z80_LD_Reg8_N(m_registers.l, *(instruction + 1));
			break;

		case Opcodes::Z80_Plain_Cpl:							// 0x2f
			/* complement A
			 *
			 * FLAGS: S, Z, P and C preserved, N and H set */
			(m_registers.a) = ~(m_registers.a);
			Z80_FLAG_N_SET;
			Z80_FLAG_H_SET;
            Z80_FLAG_F3_UPDATE(m_registers.a & Z80FlagF3Mask);
            Z80_FLAG_F5_UPDATE(m_registers.a & Z80FlagF5Mask);
			break;

		case Opcodes::Z80_Plain_Jr_Nc_d:					// 0x30
			if (!Z80_FLAG_C_ISSET) {
				m_registers.pc += static_cast<SignedByte>(*(instruction + 1));
				Z80_USE_JUMP_CYCLE_COST;
			}
			break;

		case Opcodes::Z80_Plain_Ld_Sp_Nn:				// 0x31
			Z80_LD_Reg16_NN(m_registers.sp, z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1)));
			break;

		case Opcodes::Z80_Plain_Ld_IndirectNn_A:		// 0x32
			Z80_LD_IndirectNn_Reg8(z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1)), m_registers.a);
			break;

		case Opcodes::Z80_Plain_Inc_Sp:					// 0x33
			Z80_INC_Reg16(m_registers.sp);
			break;

		case Opcodes::Z80_Plain_Inc_IndirectHl:		// 0x34
			Z80_INC_IndirectReg16(m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Dec_IndirectHl:		// 0x35
			Z80_DEC_IndirectReg16(m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Ld_IndirectHl_N:		// 0x36
			Z80_LD_IndirectReg16_N(m_registers.hl, *(instruction + 1));
			break;

		case Opcodes::Z80_Plain_Scf:							// 0x37
			Z80_FLAG_H_CLEAR;
			Z80_FLAG_N_CLEAR;
			Z80_FLAG_F3_UPDATE(m_registers.a & Z80FlagF3Mask);
			Z80_FLAG_F5_UPDATE(m_registers.a & Z80FlagF5Mask);
			Z80_FLAG_C_SET;
			break;

		case Opcodes::Z80_Plain_Jr_C_d:					// 0x38
			if (Z80_FLAG_C_ISSET) {
				m_registers.pc += static_cast<SignedByte>(*(instruction + 1));
				Z80_USE_JUMP_CYCLE_COST;
			}
			break;

		case Opcodes::Z80_Plain_Add_Hl_Sp:				// 0x39
			Z80_ADD_Reg16_Reg16(m_registers.hl, m_registers.sp);
			break;

		case Opcodes::Z80_Plain_Ld_A_IndirectNn:		// 0x3a
			Z80_LD_Reg8_IndirectNn(m_registers.a, z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1)));
			break;

		case Opcodes::Z80_Plain_Dec_Sp:					// 0x3b
			Z80_DEC_Reg16(m_registers.sp);
			break;

		case Opcodes::Z80_Plain_Inc_A:						// 0x3c
			Z80_INC_Reg8(m_registers.a);
			break;

		case Opcodes::Z80_Plain_Dec_A:						// 0x3d
			Z80_DEC_Reg8(m_registers.a);
			break;

		case Opcodes::Z80_Plain_Ld_A_N:					// 0x3e
			Z80_LD_Reg8_N(m_registers.a, *(instruction + 1));
			break;

		case Opcodes::Z80_Plain_Ccf:							// 0x3f
			Z80_FLAG_H_UPDATE(Z80_FLAG_C_ISSET);
			Z80_FLAG_C_UPDATE(!Z80_FLAG_C_ISSET);
			Z80_FLAG_N_CLEAR;
            Z80_FLAG_F3_UPDATE(m_registers.a & Z80FlagF3Mask);
            Z80_FLAG_F5_UPDATE(m_registers.a & Z80FlagF5Mask);
			break;

		case Opcodes::Z80_Plain_Ld_B_B:					// 0x40
//			Z80__LD__REG8__REG8(m_registers.b, m_registers.b);
			break;

		case Opcodes::Z80_Plain_Ld_B_C:					// 0x41
			Z80_LD_Reg8_Reg8(m_registers.b, m_registers.c);
			break;

		case Opcodes::Z80_Plain_Ld_B_D:					// 0x42
			Z80_LD_Reg8_Reg8(m_registers.b, m_registers.d);
			break;

		case Opcodes::Z80_Plain_Ld_B_E:					// 0x43
			Z80_LD_Reg8_Reg8(m_registers.b, m_registers.e);
			break;

		case Opcodes::Z80_Plain_Ld_B_H:					// 0x44
			Z80_LD_Reg8_Reg8(m_registers.b, m_registers.h);
			break;

		case Opcodes::Z80_Plain_Ld_B_L:					// 0x45
			Z80_LD_Reg8_Reg8(m_registers.b, m_registers.l);
			break;

		case Opcodes::Z80_Plain_Ld_B_IndirectHl:		// 0x46
			Z80_LD_Reg8_IndirectReg16(m_registers.b, m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Ld_B_A:					// 0x47
			Z80_LD_Reg8_Reg8(m_registers.b, m_registers.a);
			break;

		case Opcodes::Z80_Plain_Ld_C_B:					// 0x48
			Z80_LD_Reg8_Reg8(m_registers.c, m_registers.b);
			break;

		case Opcodes::Z80_Plain_Ld_C_C:					// 0x49
//			Z80__LD__REG8__REG8(m_registers.c, m_registers.c);
			break;

		case Opcodes::Z80_Plain_Ld_C_D:					// 0x4a
			Z80_LD_Reg8_Reg8(m_registers.c, m_registers.d);
			break;

		case Opcodes::Z80_Plain_Ld_C_E:					// 0x4b
			Z80_LD_Reg8_Reg8(m_registers.c, m_registers.e);
			break;

		case Opcodes::Z80_Plain_Ld_C_H:					// 0x4c
			Z80_LD_Reg8_Reg8(m_registers.c, m_registers.h);
			break;

		case Opcodes::Z80_Plain_Ld_C_L:					// 0x4d
			Z80_LD_Reg8_Reg8(m_registers.c, m_registers.l);
			break;

		case Opcodes::Z80_Plain_Ld_C_IndirectHl:		// 0x4e
			Z80_LD_Reg8_IndirectReg16(m_registers.c, m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Ld_C_A:					// 0x4f
			Z80_LD_Reg8_Reg8(m_registers.c, m_registers.a);
			break;

		case Opcodes::Z80_Plain_Ld_D_B:					// 0x50
			Z80_LD_Reg8_Reg8(m_registers.d, m_registers.b);
			break;

		case Opcodes::Z80_Plain_Ld_D_C:					// 0x51
			Z80_LD_Reg8_Reg8(m_registers.d, m_registers.c);
			break;

		case Opcodes::Z80_Plain_Ld_D_D:					// 0x52
//			Z80__LD__REG8__REG8(m_registers.d, m_registers.d);
			break;

		case Opcodes::Z80_Plain_Ld_D_E:					// 0x53
			Z80_LD_Reg8_Reg8(m_registers.d, m_registers.e);
			break;

		case Opcodes::Z80_Plain_Ld_D_H:					// 0x54
			Z80_LD_Reg8_Reg8(m_registers.d, m_registers.h);
			break;

		case Opcodes::Z80_Plain_Ld_D_L:					// 0x55
			Z80_LD_Reg8_Reg8(m_registers.d, m_registers.l);
			break;

		case Opcodes::Z80_Plain_Ld_D_IndirectHl:		// 0x56
			Z80_LD_Reg8_IndirectReg16(m_registers.d, m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Ld_D_A:					// 0x57
			Z80_LD_Reg8_Reg8(m_registers.d, m_registers.a);
			break;

		case Opcodes::Z80_Plain_Ld_E_B:					// 0x58
			Z80_LD_Reg8_Reg8(m_registers.e, m_registers.b);
			break;

		case Opcodes::Z80_Plain_Ld_E_C:					// 0x59
			Z80_LD_Reg8_Reg8(m_registers.e, m_registers.c);
			break;

		case Opcodes::Z80_Plain_Ld_E_D:					// 0x5a
			Z80_LD_Reg8_Reg8(m_registers.e, m_registers.d);
			break;

		case Opcodes::Z80_Plain_Ld_E_E:					// 0x5b
//			Z80__LD__REG8__REG8(m_registers.e, m_registers.e);
			break;

		case Opcodes::Z80_Plain_Ld_E_H:					// 0x5c
			Z80_LD_Reg8_Reg8(m_registers.e, m_registers.h);
			break;

		case Opcodes::Z80_Plain_Ld_E_L:					// 0x5d
			Z80_LD_Reg8_Reg8(m_registers.e, m_registers.l);
			break;

		case Opcodes::Z80_Plain_Ld_E_IndirectHl:		// 0x5e
			Z80_LD_Reg8_IndirectReg16(m_registers.e, m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Ld_E_A:					// 0x5f
			Z80_LD_Reg8_Reg8(m_registers.e, m_registers.a);
			break;

		case Opcodes::Z80_Plain_Ld_H_B:					// 0x60
			Z80_LD_Reg8_Reg8(m_registers.h, m_registers.b);
			break;

		case Opcodes::Z80_Plain_Ld_H_C:					// 0x61
			Z80_LD_Reg8_Reg8(m_registers.h, m_registers.c);
			break;

		case Opcodes::Z80_Plain_Ld_H_D:					// 0x62
			Z80_LD_Reg8_Reg8(m_registers.h, m_registers.d);
			break;

		case Opcodes::Z80_Plain_Ld_H_E:					// 0x63
			Z80_LD_Reg8_Reg8(m_registers.h, m_registers.e);
			break;

		case Opcodes::Z80_Plain_Ld_H_H:					// 0x64
//			Z80__LD__REG8__REG8(m_registers.h, m_registers.h);
			break;

		case Opcodes::Z80_Plain_Ld_H_L:					// 0x65
			Z80_LD_Reg8_Reg8(m_registers.h, m_registers.l);
			break;

		case Opcodes::Z80_Plain_Ld_H_IndirectHl:		// 0x66
			Z80_LD_Reg8_IndirectReg16(m_registers.h, m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Ld_H_A:					// 0x67
			Z80_LD_Reg8_Reg8(m_registers.h, m_registers.a);
			break;

		case Opcodes::Z80_Plain_Ld_L_B:					// 0x68
			Z80_LD_Reg8_Reg8(m_registers.l, m_registers.b);
			break;

		case Opcodes::Z80_Plain_Ld_L_C:					// 0x69
			Z80_LD_Reg8_Reg8(m_registers.l, m_registers.c);
			break;

		case Opcodes::Z80_Plain_Ld_L_D:					// 0x6a
			Z80_LD_Reg8_Reg8(m_registers.l, m_registers.d);
			break;

		case Opcodes::Z80_Plain_Ld_L_E:					// 0x6b
			Z80_LD_Reg8_Reg8(m_registers.l, m_registers.e);
			break;

		case Opcodes::Z80_Plain_Ld_L_H:					// 0x6c
			Z80_LD_Reg8_Reg8(m_registers.l, m_registers.h);
			break;

		case Opcodes::Z80_Plain_Ld_L_L:					// 0x6d
//			Z80__LD__REG8__REG8(m_registers.l, m_registers.l);
			break;

		case Opcodes::Z80_Plain_Ld_L_IndirectHl:		// 0x6e
			Z80_LD_Reg8_IndirectReg16(m_registers.l, m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Ld_L_A:					// 0x6f
			Z80_LD_Reg8_Reg8(m_registers.l, m_registers.a);
			break;

		case Opcodes::Z80_Plain_Ld_IndirectHl_B:		// 0x70
			Z80_LD_IndirectReg16_Reg8(m_registers.hl, m_registers.b);
			break;

		case Opcodes::Z80_Plain_Ld_IndirectHl_C:		// 0x71
			Z80_LD_IndirectReg16_Reg8(m_registers.hl, m_registers.c);
			break;

		case Opcodes::Z80_Plain_Ld_IndirectHl_D:		// 0x72
			Z80_LD_IndirectReg16_Reg8(m_registers.hl, m_registers.d);
			break;

		case Opcodes::Z80_Plain_Ld_IndirectHl_E:		// 0x73
			Z80_LD_IndirectReg16_Reg8(m_registers.hl, m_registers.e);
			break;

		case Opcodes::Z80_Plain_Ld_IndirectHl_H:		// 0x74
			Z80_LD_IndirectReg16_Reg8(m_registers.hl, m_registers.h);
			break;

		case Opcodes::Z80_Plain_Ld_IndirectHl_L:		// 0x75
			Z80_LD_IndirectReg16_Reg8(m_registers.hl, m_registers.l);
			break;

		case Opcodes::Z80_Plain_Halt:						// 0x76
            m_halted = true;
            Z80_DONT_UPDATE_PC;
			break;

		case Opcodes::Z80_Plain_Ld_IndirectHl_A:		// 0x77
			Z80_LD_IndirectReg16_Reg8(m_registers.hl, m_registers.a);
			break;

		case Opcodes::Z80_Plain_Ld_A_B:					// 0x78
			Z80_LD_Reg8_Reg8(m_registers.a, m_registers.b);
			break;

		case Opcodes::Z80_Plain_Ld_A_C:					// 0x79
			Z80_LD_Reg8_Reg8(m_registers.a, m_registers.c);
			break;

		case Opcodes::Z80_Plain_Ld_A_D:					// 0x7a
			Z80_LD_Reg8_Reg8(m_registers.a, m_registers.d);
			break;

		case Opcodes::Z80_Plain_Ld_A_E:					// 0x7b
			Z80_LD_Reg8_Reg8(m_registers.a, m_registers.e);
			break;

		case Opcodes::Z80_Plain_Ld_A_H:					// 0x7c
			Z80_LD_Reg8_Reg8(m_registers.a, m_registers.h);
			break;

		case Opcodes::Z80_Plain_Ld_A_L:					// 0x7d
			Z80_LD_Reg8_Reg8(m_registers.a, m_registers.l);
			break;

		case Opcodes::Z80_Plain_Ld_A_IndirectHl:		// 0x7e
			Z80_LD_Reg8_IndirectReg16(m_registers.a, m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Ld_A_A:					// 0x7f
//			Z80__LD__REG8__REG8(m_registers.a, m_registers.a);
			break;

		case Opcodes::Z80_Plain_Add_A_B:					// 0x80
			Z80_ADD_Reg8_Reg8(m_registers.a, m_registers.b);
			break;

		case Opcodes::Z80_Plain_Add_A_C:					// 0x81
			Z80_ADD_Reg8_Reg8(m_registers.a, m_registers.c);
			break;

		case Opcodes::Z80_Plain_Add_A_D:					// 0x82
			Z80_ADD_Reg8_Reg8(m_registers.a, m_registers.d);
			break;

		case Opcodes::Z80_Plain_Add_A_E:					// 0x83
			Z80_ADD_Reg8_Reg8(m_registers.a, m_registers.e);
			break;

		case Opcodes::Z80_Plain_Add_A_H:					// 0x84
			Z80_ADD_Reg8_Reg8(m_registers.a, m_registers.h);
			break;

		case Opcodes::Z80_Plain_Add_A_L:					// 0x85
			Z80_ADD_Reg8_Reg8(m_registers.a, m_registers.l);
			break;

		case Opcodes::Z80_Plain_Add_A_IndirectHl:	// 0x86
			Z80_ADD_Reg8_IndirectReg16(m_registers.a, m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Add_A_A:					// 0x87
			Z80_ADD_Reg8_Reg8(m_registers.a, m_registers.a);
			break;

		case Opcodes::Z80_Plain_Adc_A_B:					// 0x88
			Z80_ADC_Reg8_Reg8(m_registers.a, m_registers.b);
			break;

		case Opcodes::Z80_Plain_Adc_A_C:					// 0x89
			Z80_ADC_Reg8_Reg8(m_registers.a, m_registers.c);
			break;

		case Opcodes::Z80_Plain_Adc_A_D:					// 0x8a
			Z80_ADC_Reg8_Reg8(m_registers.a, m_registers.d);
			break;

		case Opcodes::Z80_Plain_Adc_A_E:					// 0x8b
			Z80_ADC_Reg8_Reg8(m_registers.a, m_registers.e);
			break;

		case Opcodes::Z80_Plain_Adc_A_H:					// 0x8c
			Z80_ADC_Reg8_Reg8(m_registers.a, m_registers.h);
			break;

		case Opcodes::Z80_Plain_Adc_A_L:					// 0x8d
			Z80_ADC_Reg8_Reg8(m_registers.a, m_registers.l);
			break;

		case Opcodes::Z80_Plain_Adc_A_IndirectHl:	// 0x8e
			Z80_ADC_Reg8_IndirectReg16(m_registers.a, m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Adc_A_A:					// 0x8f
			Z80_ADC_Reg8_Reg8(m_registers.a, m_registers.a);
			break;

		case Opcodes::Z80_Plain_Sub_B:						// 0x90
			Z80_SUB_Reg8(m_registers.b);
			break;

		case Opcodes::Z80_Plain_Sub_C:						// 0x91
			Z80_SUB_Reg8(m_registers.c);
			break;

		case Opcodes::Z80_Plain_Sub_D:						// 0x92
			Z80_SUB_Reg8(m_registers.d);
			break;

		case Opcodes::Z80_Plain_Sub_E:						// 0x93
			Z80_SUB_Reg8(m_registers.e);
			break;

		case Opcodes::Z80_Plain_Sub_H:						// 0x94
			Z80_SUB_Reg8(m_registers.h);
			break;

		case Opcodes::Z80_Plain_Sub_L:						// 0x95
			Z80_SUB_Reg8(m_registers.l);
			break;

		case Opcodes::Z80_Plain_Sub_IndirectHl:		// 0x96
			Z80_SUB_IndirectReg16(m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Sub_A:						// 0x97
			Z80_SUB_Reg8(m_registers.a);
			break;

		case Opcodes::Z80_Plain_Sbc_A_B:					// 0x98
			Z80_SBC_Reg8_Reg8(m_registers.a, m_registers.b);
			break;

		case Opcodes::Z80_Plain_Sbc_A_C:					// 0x99
			Z80_SBC_Reg8_Reg8(m_registers.a, m_registers.c);
			break;

		case Opcodes::Z80_Plain_Sbc_A_D:					// 0x9a
			Z80_SBC_Reg8_Reg8(m_registers.a, m_registers.d);
			break;

		case Opcodes::Z80_Plain_Sbc_A_E:					// 0x9b
			Z80_SBC_Reg8_Reg8(m_registers.a, m_registers.e);
			break;

		case Opcodes::Z80_Plain_Sbc_A_H:					// 0x9c
			Z80_SBC_Reg8_Reg8(m_registers.a, m_registers.h);
			break;

		case Opcodes::Z80_Plain_Sbc_A_L:					// 0x9d
			Z80_SBC_Reg8_Reg8(m_registers.a, m_registers.l);
			break;

		case Opcodes::Z80_Plain_Sbc_A_IndirectHl:	// 0x9e
			Z80_SBC_Reg8_IndirectReg16(m_registers.a, m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Sbc_A_A:					// 0x9f
			Z80_SBC_Reg8_Reg8(m_registers.a, m_registers.a);
			break;

		case Opcodes::Z80_Plain_And_B:						// 0xa0
			Z80_AND_Reg8(m_registers.b);
			break;

		case Opcodes::Z80_Plain_And_C:						// 0xa1
			Z80_AND_Reg8(m_registers.c);
			break;

		case Opcodes::Z80_Plain_And_D:						// 0xa2
			Z80_AND_Reg8(m_registers.d);
			break;

		case Opcodes::Z80_Plain_And_E:						// 0xa3
			Z80_AND_Reg8(m_registers.e);
			break;

		case Opcodes::Z80_Plain_And_H:						// 0xa4
			Z80_AND_Reg8(m_registers.h);
			break;

		case Opcodes::Z80_Plain_And_L:						// 0xa5
			Z80_AND_Reg8(m_registers.l);
			break;

		case Opcodes::Z80_Plain_And_IndirectHl:		// 0xa6
			Z80_AND_IndirectReg16(m_registers.hl);
			break;

		case Opcodes::Z80_Plain_And_A:						// 0xa7
			Z80_AND_Reg8(m_registers.a);
			break;

		case Opcodes::Z80_Plain_Xor_B:						// 0xa8
			Z80_XOR_Reg8(m_registers.b);
			break;

		case Opcodes::Z80_Plain_Xor_C:						// 0xa9
			Z80_XOR_Reg8(m_registers.c);
			break;

		case Opcodes::Z80_Plain_Xor_D:						// 0xaa
			Z80_XOR_Reg8(m_registers.d);
			break;

		case Opcodes::Z80_Plain_Xor_E:						// 0xab
			Z80_XOR_Reg8(m_registers.e);
			break;

		case Opcodes::Z80_Plain_Xor_H:						// 0xac
			Z80_XOR_Reg8(m_registers.h);
			break;

		case Opcodes::Z80_Plain_Xor_L:						// 0xad
			Z80_XOR_Reg8(m_registers.l);
			break;

		case Opcodes::Z80_Plain_Xor_IndirectHl:		// 0xae
			Z80_XOR_IndirectReg16(m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Xor_A:						// 0xaf
			Z80_XOR_Reg8(m_registers.a);
			break;

		case Opcodes::Z80_Plain_Or_B:						// 0xb0
			Z80_OR_Reg8(m_registers.b);
			break;

		case Opcodes::Z80_Plain_Or_C:						// 0xb1
			Z80_OR_Reg8(m_registers.c);
			break;

		case Opcodes::Z80_Plain_Or_D:						// 0xb2
			Z80_OR_Reg8(m_registers.d);
			break;

		case Opcodes::Z80_Plain_Or_E:						// 0xb3
			Z80_OR_Reg8(m_registers.e);
			break;

		case Opcodes::Z80_Plain_Or_H:						// 0xb4
			Z80_OR_Reg8(m_registers.h);
			break;

		case Opcodes::Z80_Plain_Or_L:						// 0xb5
			Z80_OR_Reg8(m_registers.l);
			break;

		case Opcodes::Z80_Plain_Or_IndirectHl:			// 0xb6
			Z80_OR_IndirectReg16(m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Or_A:						// 0xb7
			Z80_OR_Reg8(m_registers.a);
			break;

		case Opcodes::Z80_Plain_Cp_B:						// 0xb8
			Z80_CP_Reg8(m_registers.b);
			break;

		case Opcodes::Z80_Plain_Cp_C:						// 0xb9
			Z80_CP_Reg8(m_registers.c);
			break;

		case Opcodes::Z80_Plain_Cp_D:						// 0xba
			Z80_CP_Reg8(m_registers.d);
			break;

		case Opcodes::Z80_Plain_Cp_E:						// 0xbb
			Z80_CP_Reg8(m_registers.e);
			break;

		case Opcodes::Z80_Plain_Cp_H:						// 0xbc
			Z80_CP_Reg8(m_registers.h);
			break;

		case Opcodes::Z80_Plain_Cp_L:						// 0xbd
			Z80_CP_Reg8(m_registers.l);
			break;

		case Opcodes::Z80_Plain_Cp_IndirectHl:			// 0xbe
			Z80_CP_IndirectReg16(m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Cp_A:						// 0xbf
			Z80_CP_Reg8(m_registers.a);
			break;

		case Opcodes::Z80_Plain_Ret_Nz:					// 0xc0
			if (!Z80_FLAG_Z_ISSET) {
				Z80_POP_Reg16(m_registers.pc);
				Z80_USE_JUMP_CYCLE_COST;
				Z80_DONT_UPDATE_PC;
			}
			break;

		case Opcodes::Z80_Plain_Pop_Bc:					// 0xc1
			Z80_POP_Reg16(m_registers.bc);
			break;

		case Opcodes::Z80_Plain_Jp_Nz_Nn:				// 0xc2
            m_registers.memptr = z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1));

            // NOTE docs I've found say cost is 10 if jump taken, 1 if not; however Z80 test suite expects cost to
            //  always bew 10
            Z80_USE_JUMP_CYCLE_COST;

			if (!Z80_FLAG_Z_ISSET) {
				m_registers.pc = m_registers.memptr;
				Z80_DONT_UPDATE_PC;
			}
			break;

		case Opcodes::Z80_Plain_Jp_Nn:						// 0xc3
            m_registers.memptr = z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1));
			m_registers.pc = m_registers.memptr;
			Z80_DONT_UPDATE_PC;
			// NOTE don't set the jumped indicator because there's no different t-state cost - the jump always takes
			//  place, so the base cost in t-states is all that's used
			break;

		case Opcodes::Z80_Plain_Call_Nz_Nn:				// 0xc4
            m_registers.memptr = z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1));

			if (!Z80_FLAG_Z_ISSET) {
                Z80_USE_JUMP_CYCLE_COST;
				Z80_PUSH_Reg16(m_registers.pc + 3);
				m_registers.pc = m_registers.memptr;
				Z80_DONT_UPDATE_PC;
			}
			break;

		case Opcodes::Z80_Plain_Push_Bc:					// 0xc5
			Z80_PUSH_Reg16(m_registers.bc);
			break;

		case Opcodes::Z80_Plain_Add_A_N:					// 0xc6
			Z80_ADD_Reg8_N(m_registers.a, *(instruction + 1));
			break;

		case Opcodes::Z80_Plain_Rst_00:					// 0xc7
			/* restart at 0x0000 */
			Z80_RST_N(0x00);
			Z80_DONT_UPDATE_PC;
			break;

		case Opcodes::Z80_Plain_Ret_Z:						// 0xc8
			if (Z80_FLAG_Z_ISSET) {
				Z80_POP_Reg16(m_registers.pc);
				Z80_USE_JUMP_CYCLE_COST;
				Z80_DONT_UPDATE_PC;
			}
			break;

		case Opcodes::Z80_Plain_Ret:							// 0xc9
			Z80_POP_Reg16(m_registers.pc);
			Z80_DONT_UPDATE_PC;
            // NOTE don't set the jumped indicator because there's no different t-state cost - the jump always takes
            //  place, so the base cost in t-states is all that's used
			break;

		case Opcodes::Z80_Plain_Jp_Z_Nn:					// 0xca
            m_registers.memptr = z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1));

            if (Z80_FLAG_Z_ISSET) {
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Opcodes::Z80_Plain_Prefix_Cb:				// 0xcb
            Util::debugln("executePlainInstruction() called with opcode 0xcb - such an opcode should be handled by executeCbInstruction()");
			break;

		case Opcodes::Z80_Plain_Call_Z_Nn:				// 0xcc
            m_registers.memptr = z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1));

            if (Z80_FLAG_Z_ISSET) {
                Z80_USE_JUMP_CYCLE_COST;
                Z80_PUSH_Reg16(m_registers.pc + 3);
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Opcodes::Z80_Plain_Call_Nn:					// 0xcd
            m_registers.memptr = z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1));
			Z80_PUSH_Reg16(m_registers.pc + 3);
			m_registers.pc = m_registers.memptr;
			Z80_DONT_UPDATE_PC;
			break;

		case Opcodes::Z80_Plain_Adc_A_N:					// 0xce
			Z80_ADC_Reg8_N(m_registers.a, *(instruction + 1));
			break;

		case Opcodes::Z80_Plain_Rst_08:					// 0xcf
			Z80_RST_N(0x08);
			Z80_DONT_UPDATE_PC;
			break;

		case Opcodes::Z80_Plain_Ret_Nc:					// 0xd0
			if (!Z80_FLAG_C_ISSET) {
				Z80_POP_Reg16(m_registers.pc);
				Z80_USE_JUMP_CYCLE_COST;
				Z80_DONT_UPDATE_PC;
			}
			break;

		case Opcodes::Z80_Plain_Pop_De:					// 0xd1
			Z80_POP_Reg16(m_registers.de);
			break;

		case Opcodes::Z80_Plain_Jp_Nc_Nn:				// 0xd2
            m_registers.memptr = z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1));

            if (!Z80_FLAG_C_ISSET) {
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Opcodes::Z80_Plain_Out_IndirectN_A:		// 0xd3
            Z80_OUT_Indirect_N_Reg8(*(instruction + 1), m_registers.a);
            m_registers.memptr = m_registers.bc + 1;
			break;

		case Opcodes::Z80_Plain_Call_Nc_Nn:				// 0xd4
            m_registers.memptr = z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1));

            if (!Z80_FLAG_C_ISSET) {
                Z80_USE_JUMP_CYCLE_COST;
                Z80_PUSH_Reg16(m_registers.pc + 3);
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Opcodes::Z80_Plain_Push_De:					// 0xd5
			Z80_PUSH_Reg16(m_registers.de);
			break;

		case Opcodes::Z80_Plain_Sub_N:						// 0xd6
			Z80_SUB_N(*(instruction + 1));
			break;

		case Opcodes::Z80_Plain_Rst_10:					// 0xd7
			Z80_RST_N(0x10);
			Z80_DONT_UPDATE_PC;
			break;

		case Opcodes::Z80_Plain_Ret_C:						// 0xd8
			if (Z80_FLAG_C_ISSET) {
				Z80_POP_Reg16(m_registers.pc);
				Z80_USE_JUMP_CYCLE_COST;
				Z80_DONT_UPDATE_PC;
			}
			break;

		case Opcodes::Z80_Plain_Exx:							// 0xd9
			Z80_EX_Reg16_Reg16(m_registers.bc, m_registers.bcShadow);
			Z80_EX_Reg16_Reg16(m_registers.de, m_registers.deShadow);
			Z80_EX_Reg16_Reg16(m_registers.hl, m_registers.hlShadow);
			break;

		case Opcodes::Z80_Plain_Jp_C_Nn:					// 0xda
            m_registers.memptr = z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1));

            if (Z80_FLAG_C_ISSET) {
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Opcodes::Z80_Plain_In_A_IndirectN:		// 0xdb
            Z80_IN_Reg8_Indirect_N(m_registers.a, *(instruction + 1));
			break;

		case Opcodes::Z80_Plain_Call_C_Nn:				// 0xdc
            m_registers.memptr = z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1));

            if (Z80_FLAG_C_ISSET) {
                Z80_USE_JUMP_CYCLE_COST;
                Z80_PUSH_Reg16(m_registers.pc + 3);
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Opcodes::Z80_Plain_Prefix_Dd:				// 0xdd
            Util::debugln("executePlainInstruction() called with opcode 0xdd. such an opcode should be handled by executeDdInstruction()");
			break;

		case Opcodes::Z80_Plain_Sbc_A_N:					// 0xde
			Z80_SBC_Reg8_N(m_registers.a, *(instruction + 1));
			break;

		case Opcodes::Z80_Plain_Rst_18:					// 0xdf
			Z80_RST_N(0x18);
			Z80_DONT_UPDATE_PC;
			break;

		case Opcodes::Z80_Plain_Ret_Po:					// 0xe0
			/* the operand PO stands for "parity odd" */
			if (!Z80_FLAG_P_ISSET) {
				Z80_POP_Reg16(m_registers.pc);
				Z80_USE_JUMP_CYCLE_COST;
				Z80_DONT_UPDATE_PC;
			}
			break;

		case Opcodes::Z80_Plain_Pop_Hl:					// 0xe1
			Z80_POP_Reg16(m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Jp_Po_Nn:				// 0xe2
			// the operand PO stands for "parity odd"
            m_registers.memptr = z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1));

            if (!Z80_FLAG_P_ISSET) {
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Opcodes::Z80_Plain_Ex_IndirectSp_Hl:	// 0xe3
			Z80_EX_IndirectReg16_Reg16(m_registers.sp, m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Call_Po_Nn:				// 0xe4
			// the operand PO stands for "parity odd"
            m_registers.memptr = z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1));

            if (!Z80_FLAG_P_ISSET) {
                Z80_USE_JUMP_CYCLE_COST;
                Z80_PUSH_Reg16(m_registers.pc + 3);
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Opcodes::Z80_Plain_Push_Hl:					// 0xe5
			Z80_PUSH_Reg16(m_registers.hl);
			break;

		case Opcodes::Z80_Plain_And_N:						// 0xe6
			Z80_AND_N(*(instruction + 1));
			break;

		case Opcodes::Z80_Plain_Rst_20:					// 0xe7
			Z80_RST_N(0x20);
			Z80_DONT_UPDATE_PC;
			break;

		case Opcodes::Z80_Plain_Ret_Pe:					// 0xe8
			/* the operand PE stands for "parity even" */
			if (Z80_FLAG_P_ISSET) {
				Z80_POP_Reg16(m_registers.pc);
				Z80_USE_JUMP_CYCLE_COST;
				Z80_DONT_UPDATE_PC;
			}
			break;

		case Opcodes::Z80_Plain_Jp_IndirectHl:			// 0xe9
			m_registers.pc = m_registers.hl;
			Z80_DONT_UPDATE_PC;
            // NOTE don't set the jumped indicator because there's no different t-state cost - the jump always takes
            //  place, so the base cost in t-states is all that's used
			break;

		case Opcodes::Z80_Plain_Jp_Pe_Nn:				// 0xea
            // the operand PO stands for "parity even"
            m_registers.memptr = z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1));

            if (Z80_FLAG_P_ISSET) {
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Opcodes::Z80_Plain_Ex_De_Hl:				// 0xeb
			Z80_EX_Reg16_Reg16(m_registers.de, m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Call_Pe_Nn:				// 0xec
			/* the operand PE stands for "parity even" */
            m_registers.memptr = z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1));

            if (Z80_FLAG_P_ISSET) {
                Z80_USE_JUMP_CYCLE_COST;
                Z80_PUSH_Reg16(m_registers.pc + 3);
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Opcodes::Z80_Plain_Prefix_Ed:				// 0xed
			/* should never happen */
            Util::debugln("executePlainInstruction() called with opcode 0xed. such an opcode should be handled by executeEdInstruction()");
			break;

		case Opcodes::Z80_Plain_Xor_N:						// 0xee
			Z80_XOR_N(*(instruction + 1));
			break;

		case Opcodes::Z80_Plain_Rst_28:					// 0xef
			Z80_RST_N(0x28);
			Z80_DONT_UPDATE_PC;
			break;

		case Opcodes::Z80_Plain_Ret_P:						// 0xf0
			/* the operand P means "plus" and therefore uses the sign flag; not
				to be confused with the parity flag */
			if (!Z80_FLAG_S_ISSET) {
				Z80_POP_Reg16(m_registers.pc);
				Z80_USE_JUMP_CYCLE_COST;
				Z80_DONT_UPDATE_PC;
			}
			break;

		case Opcodes::Z80_Plain_Pop_Af:					// 0xf1
			Z80_POP_Reg16(m_registers.af);
			break;

		case Opcodes::Z80_Plain_Jp_P_Nn:					// 0xf2
			// the P operand in this instruction stands for "plus", not to be confused for the parity flag. it properly
            //  operates using the sign flag
            m_registers.memptr = z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1));

            if (!Z80_FLAG_S_ISSET) {
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Opcodes::Z80_Plain_Di:							// 0xf3
			m_iff1 = m_iff2 = false;
			break;

		case Opcodes::Z80_Plain_Call_P_Nn:				// 0xf4
			// the operand P stands for "plus" and thus uses the sign flag, not to be confused with the parity flag
            m_registers.memptr = z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1));

            if (!Z80_FLAG_S_ISSET) {
                Z80_USE_JUMP_CYCLE_COST;
                Z80_PUSH_Reg16(m_registers.pc + 3);
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Opcodes::Z80_Plain_Push_Af:					// 0xf5
			Z80_PUSH_Reg16(m_registers.af);
			break;

		case Opcodes::Z80_Plain_Or_N:						// 0xf6
			Z80_OR_N(*(instruction + 1));
			break;

		case Opcodes::Z80_Plain_Rst_30:					// 0xf7
			Z80_RST_N(0x30);
			Z80_DONT_UPDATE_PC;
			break;

		case Opcodes::Z80_Plain_Ret_M:						// 0xf8
			/* the operand M stands for "minus" and therefore uses the sign flag */
			if (Z80_FLAG_S_ISSET) {
				Z80_POP_Reg16(m_registers.pc);
				Z80_USE_JUMP_CYCLE_COST;
				Z80_DONT_UPDATE_PC;
			}
			break;

		case Opcodes::Z80_Plain_Ld_Sp_Hl:				// 0xf9
			Z80_LD_Reg16_Reg16(m_registers.sp, m_registers.hl);
			break;

		case Opcodes::Z80_Plain_Jp_M_Nn:					// 0xfa
			// the operand M stands for "minus" and therefore uses the sign flag
            m_registers.memptr = z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1));

            if (Z80_FLAG_S_ISSET) {
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Opcodes::Z80_Plain_Ei:							// 0xfb
			m_iff1 = m_iff2 = true;
			// after an EI, any pending interrupt is delayed until after the next instruction has been executed to allow
			// for a return
            m_delayInterruptOneInstruction = true;
			break;

		case Opcodes::Z80_Plain_Call_M_Nn:				// 0xfc
			// the operand M stands for "minus" and therefore uses the sign flag
            m_registers.memptr = z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1));

            if (Z80_FLAG_S_ISSET) {
                Z80_USE_JUMP_CYCLE_COST;
                Z80_PUSH_Reg16(m_registers.pc + 3);
                m_registers.pc = m_registers.memptr;
                Z80_DONT_UPDATE_PC;
            }
			break;

		case Opcodes::Z80_Plain_Prefix_Fd:				// 0xfd
            Util::debugln("executePlainInstruction() called with opcode 0xfd. such an opcode should be handled by executeFdInstruction()");
			break;

		case Opcodes::Z80_Plain_Cp_N:						// 0xfe
			Z80_CP_N(*(instruction + 1));
			break;

		case Opcodes::Z80_Plain_Rst_38:					// 0xff
			Z80_RST_N(0x38);
			Z80_DONT_UPDATE_PC;
			break;

		default:
            Util::debugln("unexpected opcode: {:#02x}", *instruction);
            throw InvalidOpcode({*instruction}, m_registers.pc);
	}

	return {
	    .tStates = static_cast<std::uint8_t>(useJumpCycleCost ? Z80_TSTATES_JUMP(PlainOpcodeTStates[*instruction]) : Z80_TSTATES_NOJUMP(PlainOpcodeTStates[*instruction])),
	    .size = PlainOpcodeSizes[*instruction],
	};
}

// no 0xcb instructions directly modify the PC so we don't need to receive the bool * doPc parameter to indicate this
Z80::InstructionCost Z80::Z80::executeCbInstruction(const UnsignedByte * instruction)
{
	switch(*instruction) {
		case Opcodes::Z80_Cb_Rlc_B:		// 0xcb 0x00
			Z80_RLC_Reg8(m_registers.b);
			break;

		case Opcodes::Z80_Cb_Rlc_C:		// 0xcb 0x01
			Z80_RLC_Reg8(m_registers.c);
			break;

		case Opcodes::Z80_Cb_Rlc_D:		// 0xcb 0x02
			Z80_RLC_Reg8(m_registers.d);
			break;

		case Opcodes::Z80_Cb_Rlc_E:		// 0xcb 0x03
			Z80_RLC_Reg8(m_registers.e);
			break;

		case Opcodes::Z80_Cb_Rlc_H:		// 0xcb 0x04
			Z80_RLC_Reg8(m_registers.h);
			break;

		case Opcodes::Z80_Cb_Rlc_L:		// 0xcb 0x05
			Z80_RLC_Reg8(m_registers.l);
			break;

		case Opcodes::Z80_Cb_Rlc_IndirectHl:		// 0xcb 0x06
			Z80_RLC_IndirectReg16(m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Rlc_A:		// 0xcb 0x07
			Z80_RLC_Reg8(m_registers.a);
			break;

		case Opcodes::Z80_Cb_Rrc_B:		// 0xcb 0x08
			Z80_RRC_Reg8(m_registers.b);
			break;

		case Opcodes::Z80_Cb_Rrc_C:		// 0xcb 0x09
			Z80_RRC_Reg8(m_registers.c);
			break;

		case Opcodes::Z80_Cb_Rrc_D:		// 0xcb 0x0a
			Z80_RRC_Reg8(m_registers.d);
			break;

		case Opcodes::Z80_Cb_Rrc_E:		// 0xcb 0x0b
			Z80_RRC_Reg8(m_registers.e);
			break;

		case Opcodes::Z80_Cb_Rrc_H:		// 0xcb 0x0c
			Z80_RRC_Reg8(m_registers.h);
			break;

		case Opcodes::Z80_Cb_Rrc_L:		// 0xcb 0x0d
			Z80_RRC_Reg8(m_registers.l);
			break;

		case Opcodes::Z80_Cb_Rrc_IndirectHl:		// 0xcb 0x0e
			Z80_RRC_IndirectReg16(m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Rrc_A:		// 0xcb 0x0f
			Z80_RRC_Reg8(m_registers.a);
			break;

		case Opcodes::Z80_Cb_Rl_B:		// 0xcb 0x10
			Z80_RL_Reg8(m_registers.b);
			break;

		case Opcodes::Z80_Cb_Rl_C:		// 0xcb 0x11
			Z80_RL_Reg8(m_registers.c);
			break;

		case Opcodes::Z80_Cb_Rl_D:		// 0xcb 0x12
			Z80_RL_Reg8(m_registers.d);
			break;

		case Opcodes::Z80_Cb_Rl_E:		// 0xcb 0x13
			Z80_RL_Reg8(m_registers.e);
			break;

		case Opcodes::Z80_Cb_Rl_H:		// 0xcb 0x14
			Z80_RL_Reg8(m_registers.h);
			break;

		case Opcodes::Z80_Cb_Rl_L:		// 0xcb 0x15
			Z80_RL_Reg8(m_registers.l);
			break;

		case Opcodes::Z80_Cb_Rl_IndirectHl:		// 0xcb 0x16
			Z80_RL_IndirectReg16(m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Rl_A:		// 0xcb 0x17
			Z80_RL_Reg8(m_registers.a);
			break;

		case Opcodes::Z80_Cb_Rr_B:		// 0xcb 0x18
			Z80_RR_Reg8(m_registers.b);
			break;

		case Opcodes::Z80_Cb_Rr_C:		// 0xcb 0x19
			Z80_RR_Reg8(m_registers.c);
			break;

		case Opcodes::Z80_Cb_Rr_D:		// 0xcb 0x1a
			Z80_RR_Reg8(m_registers.d);
			break;

		case Opcodes::Z80_Cb_Rr_E:		// 0xcb 0x1b
			Z80_RR_Reg8(m_registers.e);
			break;

		case Opcodes::Z80_Cb_Rr_H:		// 0xcb 0x1c
			Z80_RR_Reg8(m_registers.h);
			break;

		case Opcodes::Z80_Cb_Rr_L:		// 0xcb 0x1d
			Z80_RR_Reg8(m_registers.l);
			break;

		case Opcodes::Z80_Cb_Rr_IndirectHl:		// 0xcb 0x1e
			Z80_RR_IndirectReg16(m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Rr_A:		// 0xcb 0x1f
			Z80_RR_Reg8(m_registers.a);
			break;

		case Opcodes::Z80_Cb_Sla_B:		// 0xcb 0x21
			Z80_SLA_Reg8(m_registers.b);
			break;

		case Opcodes::Z80_Cb_Sla_C:		// 0xcb 0x22
			Z80_SLA_Reg8(m_registers.c);
			break;

		case Opcodes::Z80_Cb_Sla_D:		// 0xcb 0x23
			Z80_SLA_Reg8(m_registers.d);
			break;

		case Opcodes::Z80_Cb_Sla_E:		// 0xcb 0x24
			Z80_SLA_Reg8(m_registers.e);
			break;

		case Opcodes::Z80_Cb_Sla_H:		// 0xcb 0x25
			Z80_SLA_Reg8(m_registers.h);
			break;

		case Opcodes::Z80_Cb_Sla_L:		// 0xcb 0x26
			Z80_SLA_Reg8(m_registers.l);
			break;

		case Opcodes::Z80_Cb_Sla_IndirectHl:		// 0xcb 0x26
			Z80_SLA_IndirectReg16(m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Sla_A:		// 0xcb 0x27
			Z80_SLA_Reg8(m_registers.a);
			break;

		case Opcodes::Z80_Cb_Sra_B:		// 0xcb 0x28
			Z80_SRA_Reg8(m_registers.b);
			break;

		case Opcodes::Z80_Cb_Sra_C:		// 0xcb 0x29
			Z80_SRA_Reg8(m_registers.c);
			break;

		case Opcodes::Z80_Cb_Sra_D:		// 0xcb 0x2a
			Z80_SRA_Reg8(m_registers.d);
			break;

		case Opcodes::Z80_Cb_Sra_E:		// 0xcb 0x2b
			Z80_SRA_Reg8(m_registers.e);
			break;

		case Opcodes::Z80_Cb_Sra_H:		// 0xcb 0x2c
			Z80_SRA_Reg8(m_registers.h);
			break;

		case Opcodes::Z80_Cb_Sra_L:		// 0xcb 0x2d
			Z80_SRA_Reg8(m_registers.l);
			break;

		case Opcodes::Z80_Cb_Sra_IndirectHl:		// 0xcb 0x2e
			Z80_SRA_IndirectReg16(m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Sra_A:		// 0xcb 0x2f
			Z80_SRA_Reg8(m_registers.a);
			break;

		case Opcodes::Z80_Cb_Sll_B:		// 0xcb 0x30
			Z80_SLL_Reg8(m_registers.b);
			break;

		case Opcodes::Z80_Cb_Sll_C:		// 0xcb 0x31
			Z80_SLL_Reg8(m_registers.c);
			break;

		case Opcodes::Z80_Cb_Sll_D:		// 0xcb 0x32
			Z80_SLL_Reg8(m_registers.d);
			break;

		case Opcodes::Z80_Cb_Sll_E:		// 0xcb 0x33
			Z80_SLL_Reg8(m_registers.e);
			break;

		case Opcodes::Z80_Cb_Sll_H:		// 0xcb 0x34
			Z80_SLL_Reg8(m_registers.h);
			break;

		case Opcodes::Z80_Cb_Sll_L:		// 0xcb 0x35
			Z80_SLL_Reg8(m_registers.l);
			break;

		case Opcodes::Z80_Cb_Sll_IndirectHl:		// 0xcb 0x36
			Z80_SLL_IndirectReg16(m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Sll_A:		// 0xcb 0x37
			Z80_SLL_Reg8(m_registers.a);
			break;

		case Opcodes::Z80_Cb_Srl_B:		// 0xcb 0x38
			Z80_SRL_Reg8(m_registers.b);
			break;

		case Opcodes::Z80_Cb_Srl_C:		// 0xcb 0x39
			Z80_SRL_Reg8(m_registers.c);
			break;

		case Opcodes::Z80_Cb_Srl_D:		// 0xcb 0x3a
			Z80_SRL_Reg8(m_registers.d);
			break;

		case Opcodes::Z80_Cb_Srl_E:		// 0xcb 0x3b
			Z80_SRL_Reg8(m_registers.e);
			break;

		case Opcodes::Z80_Cb_Srl_H:		// 0xcb 0x3c
			Z80_SRL_Reg8(m_registers.h);
			break;

		case Opcodes::Z80_Cb_Srl_L:		// 0xcb 0x3d
			Z80_SRL_Reg8(m_registers.l);
			break;

		case Opcodes::Z80_Cb_Srl_IndirectHl:		// 0xcb 0x3e
			Z80_SRL_IndirectReg16(m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Srl_A:		// 0xcb 0x3f
			Z80_SRL_Reg8(m_registers.a);
			break;

		/* BIT opcodes */
		case Opcodes::Z80_Cb_Bit_0_B:		// 0xcb 0x40
			Z80_BIT_N_Reg8(0, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Bit_0_C:		// 0xcb 0x41
			Z80_BIT_N_Reg8(0, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Bit_0_D:		// 0xcb 0x42
			Z80_BIT_N_Reg8(0, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Bit_0_E:		// 0xcb 0x43
			Z80_BIT_N_Reg8(0, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Bit_0_H:		// 0xcb 0x44
			Z80_BIT_N_Reg8(0, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Bit_0_L:		// 0xcb 0x45
			Z80_BIT_N_Reg8(0, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Bit_0_IndirectHl:		// 0xcb 0x46
            Z80_BIT_N_IndirectReg16(0, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Bit_0_A:		// 0xcb 0x47
			Z80_BIT_N_Reg8(0, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Bit_1_B:		// 0xcb 0x48
			Z80_BIT_N_Reg8(1, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Bit_1_C:		// 0xcb 0x49
			Z80_BIT_N_Reg8(1, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Bit_1_D:		// 0xcb 0x4a
			Z80_BIT_N_Reg8(1, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Bit_1_E:		// 0xcb 0x4b
			Z80_BIT_N_Reg8(1, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Bit_1_H:		// 0xcb 0x4c
			Z80_BIT_N_Reg8(1, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Bit_1_L:		// 0xcb 0x4d
			Z80_BIT_N_Reg8(1, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Bit_1_IndirectHl:		// 0xcb 0x4e
			Z80_BIT_N_IndirectReg16(1, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Bit_1_A:		// 0xcb 0x4f
			Z80_BIT_N_Reg8(1, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Bit_2_B:		// 0xcb 0x50
			Z80_BIT_N_Reg8(2, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Bit_2_C:		// 0xcb 0x51
			Z80_BIT_N_Reg8(2, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Bit_2_D:		// 0xcb 0x52
			Z80_BIT_N_Reg8(2, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Bit_2_E:		// 0xcb 0x53
			Z80_BIT_N_Reg8(2, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Bit_2_H:		// 0xcb 0x54
			Z80_BIT_N_Reg8(2, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Bit_2_L:		// 0xcb 0x55
			Z80_BIT_N_Reg8(2, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Bit_2_IndirectHl:		// 0xcb 0x56
			Z80_BIT_N_IndirectReg16(2, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Bit_2_A:		// 0xcb 0x57
			Z80_BIT_N_Reg8(2, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Bit_3_B:		// 0xcb 0x58
			Z80_BIT_N_Reg8(3, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Bit_3_C:		// 0xcb 0x59
			Z80_BIT_N_Reg8(3, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Bit_3_D:		// 0xcb 0x5a
			Z80_BIT_N_Reg8(3, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Bit_3_E:		// 0xcb 0x5b
			Z80_BIT_N_Reg8(3, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Bit_3_H:		// 0xcb 0x5c
			Z80_BIT_N_Reg8(3, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Bit_3_L:		// 0xcb 0x5d
			Z80_BIT_N_Reg8(3, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Bit_3_IndirectHl:		// 0xcb 0x5e
			Z80_BIT_N_IndirectReg16(3, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Bit_3_A:		// 0xcb 0x5f
			Z80_BIT_N_Reg8(3, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Bit_4_B:		// 0xcb 0x60
			Z80_BIT_N_Reg8(4, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Bit_4_C:		// 0xcb 0x61
			Z80_BIT_N_Reg8(4, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Bit_4_D:		// 0xcb 0x62
			Z80_BIT_N_Reg8(4, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Bit_4_E:		// 0xcb 0x63
			Z80_BIT_N_Reg8(4, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Bit_4_H:		// 0xcb 0x64
			Z80_BIT_N_Reg8(4, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Bit_4_L:		// 0xcb 0x65
			Z80_BIT_N_Reg8(4, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Bit_4_IndirectHl:		// 0xcb 0x66
			Z80_BIT_N_IndirectReg16(4, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Bit_4_A:		// 0xcb 0x67
			Z80_BIT_N_Reg8(4, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Bit_5_B:		// 0xcb 0x68
			Z80_BIT_N_Reg8(5, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Bit_5_C:		// 0xcb 0x69
			Z80_BIT_N_Reg8(5, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Bit_5_D:		// 0xcb 0x6a
			Z80_BIT_N_Reg8(5, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Bit_5_E:		// 0xcb 0x6b
			Z80_BIT_N_Reg8(5, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Bit_5_H:		// 0xcb 0x6c
			Z80_BIT_N_Reg8(5, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Bit_5_L:		// 0xcb 0x6d
			Z80_BIT_N_Reg8(5, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Bit_5_IndirectHl:		// 0xcb 0x6e
			Z80_BIT_N_IndirectReg16(5, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Bit_5_A:		// 0xcb 0x6f
			Z80_BIT_N_Reg8(5, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Bit_6_B:		// 0xcb 0x70
			Z80_BIT_N_Reg8(6, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Bit_6_C:		// 0xcb 0x71
			Z80_BIT_N_Reg8(6, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Bit_6_D:		// 0xcb 0x72
			Z80_BIT_N_Reg8(6, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Bit_6_E:		// 0xcb 0x73
			Z80_BIT_N_Reg8(6, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Bit_6_H:		// 0xcb 0x74
			Z80_BIT_N_Reg8(6, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Bit_6_L:		// 0xcb 0x75
			Z80_BIT_N_Reg8(6, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Bit_6_IndirectHl:		// 0xcb 0x76
			Z80_BIT_N_IndirectReg16(6, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Bit_6_A:		// 0xcb 0x77
			Z80_BIT_N_Reg8(6, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Bit_7_B:		// 0xcb 0x78
			Z80_BIT_N_Reg8(7, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Bit_7_C:		// 0xcb 0x79
			Z80_BIT_N_Reg8(7, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Bit_7_D:		// 0xcb 0x7a
			Z80_BIT_N_Reg8(7, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Bit_7_E:		// 0xcb 0x7b
			Z80_BIT_N_Reg8(7, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Bit_7_H:		// 0xcb 0x7c
			Z80_BIT_N_Reg8(7, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Bit_7_L:		// 0xcb 0x7d
			Z80_BIT_N_Reg8(7, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Bit_7_IndirectHl:		// 0xcb 0x7e
			Z80_BIT_N_IndirectReg16(7, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Bit_7_A:		// 0xcb 0x7f
			Z80_BIT_N_Reg8(7, m_registers.a);
			break;

		/* RES opcodes */
		case Opcodes::Z80_Cb_Res_0_B:		// 0xcb 0x80
			Z80_RES_N_Reg8(0, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Res_0_C:		// 0xcb 0x81
			Z80_RES_N_Reg8(0, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Res_0_D:		// 0xcb 0x82
			Z80_RES_N_Reg8(0, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Res_0_E:		// 0xcb 0x83
			Z80_RES_N_Reg8(0, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Res_0_H:		// 0xcb 0x84
			Z80_RES_N_Reg8(0, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Res_0_L:		// 0xcb 0x85
			Z80_RES_N_Reg8(0, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Res_0_IndirectHl:		// 0xcb 0x86
			Z80_RES_N_IndirectReg16(0, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Res_0_A:		// 0xcb 0x87
			Z80_RES_N_Reg8(0, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Res_1_B:		// 0xcb 0x88
			Z80_RES_N_Reg8(1, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Res_1_C:		// 0xcb 0x89
			Z80_RES_N_Reg8(1, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Res_1_D:		// 0xcb 0x8a
			Z80_RES_N_Reg8(1, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Res_1_E:		// 0xcb 0x8b
			Z80_RES_N_Reg8(1, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Res_1_H:		// 0xcb 0x8c
			Z80_RES_N_Reg8(1, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Res_1_L:		// 0xcb 0x8d
			Z80_RES_N_Reg8(1, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Res_1_IndirectHl:		// 0xcb 0x8e
			Z80_RES_N_IndirectReg16(1, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Res_1_A:		// 0xcb 0x8f
			Z80_RES_N_Reg8(1, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Res_2_B:		// 0xcb 0x90
			Z80_RES_N_Reg8(2, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Res_2_C:		// 0xcb 0x91
			Z80_RES_N_Reg8(2, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Res_2_D:		// 0xcb 0x92
			Z80_RES_N_Reg8(2, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Res_2_E:		// 0xcb 0x93
			Z80_RES_N_Reg8(2, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Res_2_H:		// 0xcb 0x94
			Z80_RES_N_Reg8(2, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Res_2_L:		// 0xcb 0x95
			Z80_RES_N_Reg8(2, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Res_2_IndirectHl:		// 0xcb 0x96
			Z80_RES_N_IndirectReg16(2, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Res_2_A:		// 0xcb 0x97
			Z80_RES_N_Reg8(2, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Res_3_B:		// 0xcb 0x98
			Z80_RES_N_Reg8(3, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Res_3_C:		// 0xcb 0x99
			Z80_RES_N_Reg8(3, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Res_3_D:		// 0xcb 0x9a
			Z80_RES_N_Reg8(3, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Res_3_E:		// 0xcb 0x9b
			Z80_RES_N_Reg8(3, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Res_3_H:		// 0xcb 0x9c
			Z80_RES_N_Reg8(3, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Res_3_L:		// 0xcb 0x9d
			Z80_RES_N_Reg8(3, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Res_3_IndirectHl:		// 0xcb 0x9e
			Z80_RES_N_IndirectReg16(3, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Res_3_A:		// 0xcb 0x9f
			Z80_RES_N_Reg8(3, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Res_4_B:		// 0xcb 0xa0
			Z80_RES_N_Reg8(4, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Res_4_C:		// 0xcb 0xa1
			Z80_RES_N_Reg8(4, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Res_4_D:		// 0xcb 0xa2
			Z80_RES_N_Reg8(4, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Res_4_E:		// 0xcb 0xa3
			Z80_RES_N_Reg8(4, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Res_4_H:		// 0xcb 0xa4
			Z80_RES_N_Reg8(4, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Res_4_L:		// 0xcb 0xa5
			Z80_RES_N_Reg8(4, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Res_4_IndirectHl:		// 0xcb 0xa6
			Z80_RES_N_IndirectReg16(4, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Res_4_A:		// 0xcb 0xa7
			Z80_RES_N_Reg8(4, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Res_5_B:		// 0xcb 0xa8
			Z80_RES_N_Reg8(5, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Res_5_C:		// 0xcb 0xa9
			Z80_RES_N_Reg8(5, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Res_5_D:		// 0xcb 0xaa
			Z80_RES_N_Reg8(5, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Res_5_E:		// 0xcb 0xab
			Z80_RES_N_Reg8(5, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Res_5_H:		// 0xcb 0xac
			Z80_RES_N_Reg8(5, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Res_5_L:		// 0xcb 0xad
			Z80_RES_N_Reg8(5, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Res_5_IndirectHl:		// 0xcb 0xae
			Z80_RES_N_IndirectReg16(5, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Res_5_A:		// 0xcb 0xaf
			Z80_RES_N_Reg8(5, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Res_6_B:		// 0xcb 0xb0
			Z80_RES_N_Reg8(6, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Res_6_C:		// 0xcb 0xb1
			Z80_RES_N_Reg8(6, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Res_6_D:		// 0xcb 0xb2
			Z80_RES_N_Reg8(6, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Res_6_E:		// 0xcb 0xb3
			Z80_RES_N_Reg8(6, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Res_6_H:		// 0xcb 0xb4
			Z80_RES_N_Reg8(6, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Res_6_L:		// 0xcb 0xb5
			Z80_RES_N_Reg8(6, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Res_6_IndirectHl:		// 0xcb 0xb6
			Z80_RES_N_IndirectReg16(6, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Res_6_A:		// 0xcb 0xb7
			Z80_RES_N_Reg8(6, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Res_7_B:		// 0xcb 0xb8
			Z80_RES_N_Reg8(7, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Res_7_C:		// 0xcb 0xb9
			Z80_RES_N_Reg8(7, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Res_7_D:		// 0xcb 0xba
			Z80_RES_N_Reg8(7, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Res_7_E:		// 0xcb 0xbb
			Z80_RES_N_Reg8(7, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Res_7_H:		// 0xcb 0xbc
			Z80_RES_N_Reg8(7, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Res_7_L:		// 0xcb 0xbd
			Z80_RES_N_Reg8(7, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Res_7_IndirectHl:		// 0xcb 0xbe
			Z80_RES_N_IndirectReg16(7, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Res_7_A:		// 0xcb 0xbf
			Z80_RES_N_Reg8(7, m_registers.a);
			break;

		/* SET opcodes */
		case Opcodes::Z80_Cb_Set_0_B:		// 0xcb 0xc0
			Z80_SET_N_Reg8(0, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Set_0_C:		// 0xcb 0xc1
			Z80_SET_N_Reg8(0, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Set_0_D:		// 0xcb 0xc2
			Z80_SET_N_Reg8(0, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Set_0_E:		// 0xcb 0xc3
			Z80_SET_N_Reg8(0, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Set_0_H:		// 0xcb 0xc4
			Z80_SET_N_Reg8(0, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Set_0_L:		// 0xcb 0xc5
			Z80_SET_N_Reg8(0, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Set_0_IndirectHl:		// 0xcb 0xc6
			Z80_SET_N_IndirectReg16(0, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Set_0_A:		// 0xcb 0xc7
			Z80_SET_N_Reg8(0, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Set_1_B:		// 0xcb 0xc8
			Z80_SET_N_Reg8(1, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Set_1_C:		// 0xcb 0xc9
			Z80_SET_N_Reg8(1, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Set_1_D:		// 0xcb 0xca
			Z80_SET_N_Reg8(1, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Set_1_E:		// 0xcb 0xcb
			Z80_SET_N_Reg8(1, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Set_1_H:		// 0xcb 0xcc
			Z80_SET_N_Reg8(1, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Set_1_L:		// 0xcb 0xcd
			Z80_SET_N_Reg8(1, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Set_1_IndirectHl:		// 0xcb 0xce
			Z80_SET_N_IndirectReg16(1, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Set_1_A:		// 0xcb 0xcf
			Z80_SET_N_Reg8(1, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Set_2_B:		// 0xcb 0xd0
			Z80_SET_N_Reg8(2, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Set_2_C:		// 0xcb 0xd1
			Z80_SET_N_Reg8(2, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Set_2_D:		// 0xcb 0xd2
			Z80_SET_N_Reg8(2, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Set_2_E:		// 0xcb 0xd3
			Z80_SET_N_Reg8(2, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Set_2_H:		// 0xcb 0xd4
			Z80_SET_N_Reg8(2, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Set_2_L:		// 0xcb 0xd5
			Z80_SET_N_Reg8(2, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Set_2_IndirectHl:		// 0xcb 0xd6
			Z80_SET_N_IndirectReg16(2, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Set_2_A:		// 0xcb 0xd7
			Z80_SET_N_Reg8(2, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Set_3_B:		// 0xcb 0xd8
			Z80_SET_N_Reg8(3, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Set_3_C:		// 0xcb 0xd9
			Z80_SET_N_Reg8(3, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Set_3_D:		// 0xcb 0xda
			Z80_SET_N_Reg8(3, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Set_3_E:		// 0xcb 0xdb
			Z80_SET_N_Reg8(3, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Set_3_H:		// 0xcb 0xdc
			Z80_SET_N_Reg8(3, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Set_3_L:		// 0xcb 0xdd
			Z80_SET_N_Reg8(3, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Set_3_IndirectHl:		// 0xcb 0xde
			Z80_SET_N_IndirectReg16(3, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Set_3_A:		// 0xcb 0xdf
			Z80_SET_N_Reg8(3, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Set_4_B:		// 0xcb 0xe0
			Z80_SET_N_Reg8(4, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Set_4_C:		// 0xcb 0xe1
			Z80_SET_N_Reg8(4, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Set_4_D:		// 0xcb 0xe2
			Z80_SET_N_Reg8(4, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Set_4_E:		// 0xcb 0xe3
			Z80_SET_N_Reg8(4, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Set_4_H:		// 0xcb 0xe4
			Z80_SET_N_Reg8(4, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Set_4_L:		// 0xcb 0xe5
			Z80_SET_N_Reg8(4, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Set_4_IndirectHl:		// 0xcb 0xe6
			Z80_SET_N_IndirectReg16(4, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Set_4_A:		// 0xcb 0xe7
			Z80_SET_N_Reg8(4, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Set_5_B:		// 0xcb 0xe8
			Z80_SET_N_Reg8(5, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Set_5_C:		// 0xcb 0xe9
			Z80_SET_N_Reg8(5, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Set_5_D:		// 0xcb 0xea
			Z80_SET_N_Reg8(5, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Set_5_E:		// 0xcb 0xeb
			Z80_SET_N_Reg8(5, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Set_5_H:		// 0xcb 0xec
			Z80_SET_N_Reg8(5, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Set_5_L:		// 0xcb 0xed
			Z80_SET_N_Reg8(5, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Set_5_IndirectHl:		// 0xcb 0xee
			Z80_SET_N_IndirectReg16(5, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Set_5_A:		// 0xcb 0xef
			Z80_SET_N_Reg8(5, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Set_6_B:		// 0xcb 0xf0
			Z80_SET_N_Reg8(6, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Set_6_C:		// 0xcb 0xf1
			Z80_SET_N_Reg8(6, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Set_6_D:		// 0xcb 0xf2
			Z80_SET_N_Reg8(6, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Set_6_E:		// 0xcb 0xf3
			Z80_SET_N_Reg8(6, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Set_6_H:		// 0xcb 0xf4
			Z80_SET_N_Reg8(6, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Set_6_L:		// 0xcb 0xf5
			Z80_SET_N_Reg8(6, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Set_6_IndirectHl:		// 0xcb 0xf6
			Z80_SET_N_IndirectReg16(6, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Set_6_A:		// 0xcb 0xf7
			Z80_SET_N_Reg8(6, m_registers.a);
			break;

		case Opcodes::Z80_Cb_Set_7_B:		// 0xcb 0xf8
			Z80_SET_N_Reg8(7, m_registers.b);
			break;

		case Opcodes::Z80_Cb_Set_7_C:		// 0xcb 0xf9
			Z80_SET_N_Reg8(7, m_registers.c);
			break;

		case Opcodes::Z80_Cb_Set_7_D:		// 0xcb 0xfa
			Z80_SET_N_Reg8(7, m_registers.d);
			break;

		case Opcodes::Z80_Cb_Set_7_E:		// 0xcb 0xfb
			Z80_SET_N_Reg8(7, m_registers.e);
			break;

		case Opcodes::Z80_Cb_Set_7_H:		// 0xcb 0xfc
			Z80_SET_N_Reg8(7, m_registers.h);
			break;

		case Opcodes::Z80_Cb_Set_7_L:		// 0xcb 0xfd
			Z80_SET_N_Reg8(7, m_registers.l);
			break;

		case Opcodes::Z80_Cb_Set_7_IndirectHl:		// 0xcb 0xfe
			Z80_SET_N_IndirectReg16(7, m_registers.hl);
			break;

		case Opcodes::Z80_Cb_Set_7_A:		// 0xcb 0xff
			Z80_SET_N_Reg8(7, m_registers.a);
			break;

		default:
            Util::debugln("unexpected opcode: 0xcb {:#02x}", *instruction);
            throw InvalidOpcode({0xcb, *instruction}, m_registers.pc);
	}

	return {
	    .tStates = CbOpcodeTStates[*instruction],
	    .size = 2,
	};
}

Z80::InstructionCost Z80::Z80::executeEdInstruction(const UnsignedByte * instruction, bool * doPc)
{
	bool useJumpCycleCost = false;

	switch (*instruction) {
		case Opcodes::Z80_Ed_Nop_0xEd_0x00:
        case Opcodes::Z80_Ed_Nop_0xEd_0x01:
        case Opcodes::Z80_Ed_Nop_0xEd_0x02:
        case Opcodes::Z80_Ed_Nop_0xEd_0x03:
        case Opcodes::Z80_Ed_Nop_0xEd_0x04:
        case Opcodes::Z80_Ed_Nop_0xEd_0x05:
        case Opcodes::Z80_Ed_Nop_0xEd_0x06:
        case Opcodes::Z80_Ed_Nop_0xEd_0x07:
        case Opcodes::Z80_Ed_Nop_0xEd_0x08:
        case Opcodes::Z80_Ed_Nop_0xEd_0x09:
        case Opcodes::Z80_Ed_Nop_0xEd_0x0a:
        case Opcodes::Z80_Ed_Nop_0xEd_0x0b:
        case Opcodes::Z80_Ed_Nop_0xEd_0x0c:
        case Opcodes::Z80_Ed_Nop_0xEd_0x0d:
        case Opcodes::Z80_Ed_Nop_0xEd_0x0e:
        case Opcodes::Z80_Ed_Nop_0xEd_0x0f:
        case Opcodes::Z80_Ed_Nop_0xEd_0x10:
        case Opcodes::Z80_Ed_Nop_0xEd_0x11:
        case Opcodes::Z80_Ed_Nop_0xEd_0x12:
        case Opcodes::Z80_Ed_Nop_0xEd_0x13:
        case Opcodes::Z80_Ed_Nop_0xEd_0x14:
        case Opcodes::Z80_Ed_Nop_0xEd_0x15:
        case Opcodes::Z80_Ed_Nop_0xEd_0x16:
        case Opcodes::Z80_Ed_Nop_0xEd_0x17:
        case Opcodes::Z80_Ed_Nop_0xEd_0x18:
        case Opcodes::Z80_Ed_Nop_0xEd_0x19:
        case Opcodes::Z80_Ed_Nop_0xEd_0x1a:
        case Opcodes::Z80_Ed_Nop_0xEd_0x1b:
        case Opcodes::Z80_Ed_Nop_0xEd_0x1c:
        case Opcodes::Z80_Ed_Nop_0xEd_0x1d:
        case Opcodes::Z80_Ed_Nop_0xEd_0x1e:
        case Opcodes::Z80_Ed_Nop_0xEd_0x1f:
        case Opcodes::Z80_Ed_Nop_0xEd_0x20:
        case Opcodes::Z80_Ed_Nop_0xEd_0x21:
        case Opcodes::Z80_Ed_Nop_0xEd_0x22:
        case Opcodes::Z80_Ed_Nop_0xEd_0x23:
        case Opcodes::Z80_Ed_Nop_0xEd_0x24:
        case Opcodes::Z80_Ed_Nop_0xEd_0x25:
        case Opcodes::Z80_Ed_Nop_0xEd_0x26:
        case Opcodes::Z80_Ed_Nop_0xEd_0x27:
        case Opcodes::Z80_Ed_Nop_0xEd_0x28:
        case Opcodes::Z80_Ed_Nop_0xEd_0x29:
        case Opcodes::Z80_Ed_Nop_0xEd_0x2a:
        case Opcodes::Z80_Ed_Nop_0xEd_0x2b:
        case Opcodes::Z80_Ed_Nop_0xEd_0x2c:
        case Opcodes::Z80_Ed_Nop_0xEd_0x2d:
        case Opcodes::Z80_Ed_Nop_0xEd_0x2e:
        case Opcodes::Z80_Ed_Nop_0xEd_0x2f:
        case Opcodes::Z80_Ed_Nop_0xEd_0x30:
        case Opcodes::Z80_Ed_Nop_0xEd_0x31:
        case Opcodes::Z80_Ed_Nop_0xEd_0x32:
        case Opcodes::Z80_Ed_Nop_0xEd_0x33:
        case Opcodes::Z80_Ed_Nop_0xEd_0x34:
        case Opcodes::Z80_Ed_Nop_0xEd_0x35:
        case Opcodes::Z80_Ed_Nop_0xEd_0x36:
        case Opcodes::Z80_Ed_Nop_0xEd_0x37:
        case Opcodes::Z80_Ed_Nop_0xEd_0x38:
        case Opcodes::Z80_Ed_Nop_0xEd_0x39:
        case Opcodes::Z80_Ed_Nop_0xEd_0x3a:
        case Opcodes::Z80_Ed_Nop_0xEd_0x3b:
        case Opcodes::Z80_Ed_Nop_0xEd_0x3c:
        case Opcodes::Z80_Ed_Nop_0xEd_0x3d:
        case Opcodes::Z80_Ed_Nop_0xEd_0x3e:
        case Opcodes::Z80_Ed_Nop_0xEd_0x3f:
			break;

		case Opcodes::Z80_Ed_In_B_IndirectC:					// 0xed 0x40
            Z80_IN_Reg8_IndirectReg8(m_registers.b, m_registers.c);
			break;

		case Opcodes::Z80_Ed_Out_IndirectC_B:					// 0xed 0x41
            Z80_OUT_IndirectReg8_Reg8(m_registers.c, m_registers.b);
			break;

		case Opcodes::Z80_Ed_Sbc_Hl_Bc:					// 0xed 0x42
            Z80_SBC_Reg16_Reg16(m_registers.hl, m_registers.bc);
			break;

		case Opcodes::Z80_Ed_Ld_IndirectNn_Bc:		// 0xed 0x43
			Z80_LD_IndirectNn_Reg16(z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1)), m_registers.bc);
			break;

		case Opcodes::Z80_Ed_Neg:							// 0xed 0x44
			Z80_NEG;
			break;

		case Opcodes::Z80_Ed_Retn:							// 0xed 0x45
			Z80_RETN;
			Z80_DONT_UPDATE_PC;
			break;

		case Opcodes::Z80_Ed_Im_0:			// 0xed 0x46
			m_interruptMode = InterruptMode::IM0;
			break;

        case Opcodes::Z80_Ed_Ld_I_A:				// 0xed 0x47
            Z80_LD_Reg8_Reg8(m_registers.i, m_registers.a);
            break;

		case Opcodes::Z80_Ed_In_C_IndirectC:        // 0xed 0x48
            Z80_IN_Reg8_IndirectReg16(m_registers.c, m_registers.bc);
			break;

		case Opcodes::Z80_Ed_Out_IndirectC_C:        // 0xed 0x49
            Z80_OUT_IndirectReg8_Reg8(m_registers.c, m_registers.c);
			break;

		case Opcodes::Z80_Ed_Adc_Hl_Bc:       // 0xed 0x4a
			Z80_ADC_Reg16_Reg16(m_registers.hl, m_registers.bc);
			break;

		case Opcodes::Z80_Ed_Ld_Bc_IndirectNn:       // 0xed 0x4b
			Z80_LD_Reg16_IndirectNn(m_registers.bc, z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1)));
			break;

		case Opcodes::Z80_Ed_Neg_0xEd_0x4c:		// 0xed 0x4c
			Z80_NEG;
			break;

		case Opcodes::Z80_Ed_Reti:					// 0xed 0x4d
			Z80_RETI;
			Z80_DONT_UPDATE_PC;
			break;

		case Opcodes::Z80_Ed_Im_0_0xEd_0x4e:	// 0xed 0x4e
			// non-standard instruction; not guaranteed that this is the instruction
			// in all versions of the Z80
			m_interruptMode = InterruptMode::IM0;
			break;

        case Opcodes::Z80_Ed_Ld_R_A:				// 0xed 0x4f
            Z80_LD_Reg8_Reg8(m_registers.r, m_registers.a);
            break;

		case Opcodes::Z80_Ed_In_D_IndirectC:					// 0xed 0x50
            Z80_IN_Reg8_IndirectReg8(m_registers.d, m_registers.c);
			break;

		case Opcodes::Z80_Ed_Out_IndirectC_D:					// 0xed 0x52
            Z80_OUT_IndirectReg8_Reg8(m_registers.c, m_registers.d);
			break;

		case Opcodes::Z80_Ed_Sbc_Hl_De:					// 0xed 0x52
			Z80_SBC_Reg16_Reg16(m_registers.hl, m_registers.de);
			break;

		case Opcodes::Z80_Ed_Ld_IndirectNn_De:	// 0xed 0x53
			Z80_LD_IndirectNn_Reg16(z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1)), m_registers.de);
			break;

		case Opcodes::Z80_Ed_Neg_0xEd_0x54:		// 0xed 0x54
			Z80_NEG;
			break;

		case Opcodes::Z80_Ed_Retn_0xEd_0x55:		// 0xed 0x55
			Z80_RETN;
			Z80_DONT_UPDATE_PC;
			break;

		case Opcodes::Z80_Ed_Im_1:					// 0xed 0x56
			m_interruptMode = InterruptMode::IM1;
			break;

		case Opcodes::Z80_Ed_Ld_A_I:				// 0xed 0x57
			Z80_LD_Reg8_Reg8(m_registers.a, m_registers.i);
			Z80_FLAGS_S53_UPDATE(m_registers.a);
			Z80_FLAG_Z_UPDATE(0 == m_registers.a);
			Z80_FLAG_H_CLEAR;
			Z80_FLAG_P_UPDATE(m_iff2);
			Z80_FLAG_N_CLEAR;
			break;

		case Opcodes::Z80_Ed_In_E_IndirectC:					// 0xed 0x58
            Z80_IN_Reg8_IndirectReg8(m_registers.e, m_registers.c);
			break;

		case Opcodes::Z80_Ed_Out_IndirectC_E:					// 0xed 0x59
            Z80_OUT_IndirectReg8_Reg8(m_registers.c, m_registers.e);
			break;

		case Opcodes::Z80_Ed_Adc_Hl_De:					// 0xed 0x5a
			Z80_ADC_Reg16_Reg16(m_registers.hl, m_registers.de);
			break;

		case Opcodes::Z80_Ed_Ld_De_IndirectNn:
			Z80_LD_Reg16_IndirectNn(m_registers.de, z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1)));
			break;

		case Opcodes::Z80_Ed_Neg_0xEd_0x5c:		// 0xed 0x5c
			Z80_NEG;
			break;

		case Opcodes::Z80_Ed_Reti_0xEd_0x5d:		// 0xed 0x5d
			Z80_RETI;
			Z80_DONT_UPDATE_PC;
			break;

		case Opcodes::Z80_Ed_Im_2:					// 0xed 0x5e
			m_interruptMode = InterruptMode::IM2;
			break;

		case Opcodes::Z80_Ed_Ld_A_R:                  // 0xed 0x5f
			Z80_LD_Reg8_Reg8(m_registers.a, m_registers.r);
			// NOTE carry flag is unmodified
			Z80_FLAGS_S53_UPDATE(m_registers.a);
			Z80_FLAG_Z_UPDATE(0 == m_registers.a);
			Z80_FLAG_H_CLEAR;
			Z80_FLAG_P_UPDATE(m_iff2);
			Z80_FLAG_N_CLEAR;
			break;

		case Opcodes::Z80_Ed_In_H_IndirectC:	// 0xed 0x60
            Z80_IN_Reg8_IndirectReg8(m_registers.h, m_registers.c);
			break;

		case Opcodes::Z80_Ed_Out_IndirectC_H:
            Z80_OUT_IndirectReg8_Reg8(m_registers.c, m_registers.h);
			break;

		case Opcodes::Z80_Ed_Sbc_Hl_Hl:			// 0xed 0x62
			Z80_SBC_Reg16_Reg16(m_registers.hl, m_registers.hl);
			break;

		case Opcodes::Z80_Ed_Ld_IndirectNn_Hl:
			Z80_LD_IndirectNn_Reg16(z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1)), m_registers.hl);
			break;

		case Opcodes::Z80_Ed_Neg_0xEd_0x64:	// 0xed 0x64
			Z80_NEG;
			break;

		case Opcodes::Z80_Ed_Retn_0xEd_0x65:	// 0xed 0x65
			Z80_RETN;
			Z80_DONT_UPDATE_PC;
			break;

		case Opcodes::Z80_Ed_Im_0_0xEd_0x66:		// 0xed 0x66
			/* non-standard instruction; not guaranteed that this is the instruction
			 * in all versions of the Z80 */
			m_interruptMode = InterruptMode::IM0;
			break;

		case Opcodes::Z80_Ed_Rrd:
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

		case Opcodes::Z80_Ed_In_L_IndirectC:
            Z80_IN_Reg8_IndirectReg8(m_registers.l, m_registers.c);
			break;

		case Opcodes::Z80_Ed_Out_IndirectC_L:
            Z80_OUT_IndirectReg8_Reg8(m_registers.c, m_registers.l);
			break;

		case Opcodes::Z80_Ed_Adc_Hl_Hl:
			Z80_ADC_Reg16_Reg16(m_registers.hl, m_registers.hl);
			break;

		case Opcodes::Z80_Ed_Ld_Hl_IndirectNn:
			Z80_LD_Reg16_IndirectNn(m_registers.hl, z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1)));
			break;

		case Opcodes::Z80_Ed_Neg_0xEd_0x6c:	// 0xed 0x6c
			Z80_NEG;
			break;

		case Opcodes::Z80_Ed_Reti_0xEd_0x6d:	// 0xed 0x6e
			Z80_RETI;
			Z80_DONT_UPDATE_PC;
			break;

		case Opcodes::Z80_Ed_Im_0_0xEd_0x6e:		// 0xed 0x6e
			/* non-standard instruction; not guaranteed that this is the instruction
			 * in all versions of the Z80 */
			m_interruptMode = InterruptMode::IM0;
			break;

		case Opcodes::Z80_Ed_Rld:
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

		case Opcodes::Z80_Ed_In_IndirectC:           // 0xed 0x70
            {
                Util::debugln("opcode 0xed 0x70 IN F,(C) - just setting flags");
                UnsignedByte tmpInByte;
                Z80_IN_Reg8_IndirectReg16(tmpInByte, m_registers.bc);
            }
			break;

		case Opcodes::Z80_Ed_Out_IndirectC_0:   	// 0xed 0x71
            Z80_OUT_IndirectReg8_Reg8(m_registers.c, 0);
			break;

		case Opcodes::Z80_Ed_Sbc_Hl_Sp:
			Z80_SBC_Reg16_Reg16(m_registers.hl, m_registers.sp);
			break;

		case Opcodes::Z80_Ed_Ld_IndirectNn_Sp:
			Z80_LD_IndirectNn_Reg16(z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1)), m_registers.sp);
			break;

		case Opcodes::Z80_Ed_Neg_0xEd_0x74:	/* oxed 0x74 */
			Z80_NEG;
			break;

		case Opcodes::Z80_Ed_Retn_0xEd_0x75:	// 0xed 0x75
			Z80_RETN;
			Z80_DONT_UPDATE_PC;
			break;

		case Opcodes::Z80_Ed_Im_1_0xEd_0x76:		// 0xed 0x76
			/* non-standard instruction; not guaranteed that this is the instruction
			 * in all versions of the Z80 */
			m_interruptMode = InterruptMode::IM1;
			break;

	    case Opcodes::Z80_Ed_Nop_0xEd_0x77:
			break;

		case Opcodes::Z80_Ed_In_A_IndirectC:   	    // 0xed 0x78
            Z80_IN_Reg8_IndirectReg8(m_registers.a, m_registers.c);
			break;

		case Opcodes::Z80_Ed_Out_IndirectC_A:   	// 0xed 0x79
            Z80_OUT_IndirectReg8_Reg8(m_registers.c, m_registers.a);
			break;

		case Opcodes::Z80_Ed_Adc_Hl_Sp:   	        // 0xed 0x7a
			Z80_ADC_Reg16_Reg16(m_registers.hl, m_registers.sp);
			break;

		case Opcodes::Z80_Ed_Ld_Sp_IndirectNn:   	// 0xed 0x7b
			Z80_LD_Reg16_IndirectNn(m_registers.sp, z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1)));
			break;

		case Opcodes::Z80_Ed_Neg_0xEd_0x7c:	    	// 0xed 0x7c
			Z80_NEG;
			break;

		case Opcodes::Z80_Ed_Reti_0xEd_0x7d:	    	// 0xed 0x7d
			Z80_RETI;
			Z80_DONT_UPDATE_PC;
			break;

		case Opcodes::Z80_Ed_Im_2_0xEd_0x7e:		// 0xed 0x7e
			/* non-standard instruction; not guaranteed that this is the instruction
			 * in all versions of the Z80 */
			m_interruptMode = InterruptMode::IM2;
			break;

		case Opcodes::Z80_Ed_Nop_0xEd_0x7f:
        case Opcodes::Z80_Ed_Nop_0xEd_0x80:
        case Opcodes::Z80_Ed_Nop_0xEd_0x81:
        case Opcodes::Z80_Ed_Nop_0xEd_0x82:
        case Opcodes::Z80_Ed_Nop_0xEd_0x83:
        case Opcodes::Z80_Ed_Nop_0xEd_0x84:
        case Opcodes::Z80_Ed_Nop_0xEd_0x85:
        case Opcodes::Z80_Ed_Nop_0xEd_0x86:
        case Opcodes::Z80_Ed_Nop_0xEd_0x87:
        case Opcodes::Z80_Ed_Nop_0xEd_0x88:
        case Opcodes::Z80_Ed_Nop_0xEd_0x89:
        case Opcodes::Z80_Ed_Nop_0xEd_0x8a:
        case Opcodes::Z80_Ed_Nop_0xEd_0x8b:
        case Opcodes::Z80_Ed_Nop_0xEd_0x8c:
        case Opcodes::Z80_Ed_Nop_0xEd_0x8d:
        case Opcodes::Z80_Ed_Nop_0xEd_0x8e:
        case Opcodes::Z80_Ed_Nop_0xEd_0x8f:
        case Opcodes::Z80_Ed_Nop_0xEd_0x90:
        case Opcodes::Z80_Ed_Nop_0xEd_0x91:
        case Opcodes::Z80_Ed_Nop_0xEd_0x92:
        case Opcodes::Z80_Ed_Nop_0xEd_0x93:
        case Opcodes::Z80_Ed_Nop_0xEd_0x94:
        case Opcodes::Z80_Ed_Nop_0xEd_0x95:
        case Opcodes::Z80_Ed_Nop_0xEd_0x96:
        case Opcodes::Z80_Ed_Nop_0xEd_0x97:
        case Opcodes::Z80_Ed_Nop_0xEd_0x98:
        case Opcodes::Z80_Ed_Nop_0xEd_0x99:
        case Opcodes::Z80_Ed_Nop_0xEd_0x9a:
        case Opcodes::Z80_Ed_Nop_0xEd_0x9b:
        case Opcodes::Z80_Ed_Nop_0xEd_0x9c:
        case Opcodes::Z80_Ed_Nop_0xEd_0x9d:
        case Opcodes::Z80_Ed_Nop_0xEd_0x9e:
        case Opcodes::Z80_Ed_Nop_0xEd_0x9f:
			break;

		case Opcodes::Z80_Ed_Ldi:      // 0xed 0xa0
		    {
		        auto tmpByte = peekUnsigned(m_registers.hl);
                pokeUnsigned(m_registers.de, tmpByte);
                m_registers.de++;
                m_registers.hl++;
                m_registers.bc--;
                Z80_FLAG_H_CLEAR;
                Z80_FLAG_N_CLEAR;
                Z80_FLAG_P_UPDATE(0 != m_registers.bc);
                Z80_FLAG_F5_UPDATE(tmpByte & (Z80FlagF5Mask >> 4));
                Z80_FLAG_F3_UPDATE(tmpByte & Z80FlagF3Mask);
            }
			break;

		case Opcodes::Z80_Ed_Cpi:      // 0xed 0xa1
			{
				bool flagC = Z80_FLAG_C_ISSET;
				Z80_CP_IndirectReg16(m_registers.hl);

				m_registers.hl++;
				m_registers.bc--;

				Z80_FLAG_C_UPDATE(flagC);
				Z80_FLAG_P_UPDATE(0 != m_registers.bc);
				Z80_FLAG_F5_UPDATE(m_registers.a & (Z80FlagF5Mask >> 4));
				Z80_FLAG_F3_UPDATE(m_registers.a & Z80FlagF3Mask);
			}
			break;

		case Opcodes::Z80_Ed_Ini:      // 0xed 0xa2
            {
                UnsignedByte result;
                Z80_READ_IO_DEVICES(result, m_registers.bc);
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

		case Opcodes::Z80_Ed_Outi:      // 0xed 0xa3
            {
                UnsignedByte value = peekUnsigned(m_registers.hl);
                Z80_WRITE_IO_DEVICES(value, m_registers.bc);
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

		case Opcodes::Z80_Ed_Nop_0xEd_0xA4:
		case Opcodes::Z80_Ed_Nop_0xEd_0xA5:
		case Opcodes::Z80_Ed_Nop_0xEd_0xA6:
		case Opcodes::Z80_Ed_Nop_0xEd_0xA7:
			break;

		case Opcodes::Z80_Ed_Ldd:      // 0xed 0xa8
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
                Z80_FLAG_F5_UPDATE(value & (Z80FlagF5Mask >> 4));
                Z80_FLAG_F3_UPDATE(value & Z80FlagF3Mask);
            }
			break;

		case Opcodes::Z80_Ed_Cpd:      // 0xed 0xa9
			{
                auto sub = peekUnsigned(m_registers.hl);
                auto result = m_registers.a - sub;
				m_registers.hl--;
				m_registers.bc--;

                Z80_FLAG_N_SET;
				Z80_FLAG_P_UPDATE(0 != m_registers.bc);
				Z80_FLAG_Z_UPDATE(0 == result);
                Z80_FLAG_H_UPDATE_SUB(m_registers.a, sub, result);
                Z80_FLAG_S_UPDATE(result & Z80FlagSMask);

				if (Z80_FLAG_H_ISSET) {
				    --result;
				}

				Z80_FLAG_F5_UPDATE(result & (Z80FlagF5Mask >> 4));
				Z80_FLAG_F3_UPDATE(result & Z80FlagF3Mask);
			}
			break;

		case Opcodes::Z80_Ed_Ind:      // 0xed 0xaa
            {
                UnsignedByte result;
                Z80_READ_IO_DEVICES(result, m_registers.bc);
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

		case Opcodes::Z80_Ed_Outd:      // 0xed 0xab
            {
                UnsignedByte value = peekUnsigned(m_registers.hl);
                Z80_WRITE_IO_DEVICES(value, m_registers.bc);
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

		case Opcodes::Z80_Ed_Nop_0xEd_0xAc:
		case Opcodes::Z80_Ed_Nop_0xEd_0xAd:
		case Opcodes::Z80_Ed_Nop_0xEd_0xAe:
		case Opcodes::Z80_Ed_Nop_0xEd_0xAf:
			break;

		// TODO R is always off by 4
		case Opcodes::Z80_Ed_Ldir:     // 0xed 0xb0
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
                Z80_FLAG_F5_UPDATE(value & (Z80FlagF5Mask >> 4));
                Z80_FLAG_F3_UPDATE(value & Z80FlagF3Mask);
            }
			break;

		case Opcodes::Z80_Ed_Cpir:     // 0xed 0xb1
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
                Z80_FLAG_F5_UPDATE(result & (Z80FlagF5Mask >> 4));
                Z80_FLAG_F3_UPDATE(result & Z80FlagF3Mask);
            }
			break;

		case Opcodes::Z80_Ed_Inir:     // 0xed 0xb2
            {
                UnsignedByte result;
                Z80_READ_IO_DEVICES(result, m_registers.bc);
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

		case Opcodes::Z80_Ed_Otir:     // 0xed 0xb3
            {
                UnsignedByte value = peekUnsigned(m_registers.hl);
                Z80_WRITE_IO_DEVICES(value, m_registers.bc);
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

		case Opcodes::Z80_Ed_Nop_0xEd_0xB4:
		case Opcodes::Z80_Ed_Nop_0xEd_0xB5:
		case Opcodes::Z80_Ed_Nop_0xEd_0xB6:
		case Opcodes::Z80_Ed_Nop_0xEd_0xB7:
			break;

        // TODO R is always off by 4
		case Opcodes::Z80_Ed_Lddr:
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
                Z80_FLAG_F5_UPDATE(value & (Z80FlagF5Mask >> 4));
                Z80_FLAG_F3_UPDATE(value & Z80FlagF3Mask);
            }
			break;

		case Opcodes::Z80_Ed_Cpdr:     //0xed 0xb9
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
                Z80_FLAG_F5_UPDATE(result & (Z80FlagF5Mask >> 4));
                Z80_FLAG_F3_UPDATE(result & Z80FlagF3Mask);
            }
			break;

		case Opcodes::Z80_Ed_Indr:     //0xed 0xba
        {
            UnsignedByte result;
            Z80_READ_IO_DEVICES(result, m_registers.bc);
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

		case Opcodes::Z80_Ed_Otdr:     // 0xed 0xbb
        {
            UnsignedByte value = peekUnsigned(m_registers.hl);
            --m_registers.b;
            Z80_WRITE_IO_DEVICES(value, m_registers.bc);
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

		case Opcodes::Z80_Ed_Nop_0xEd_0xBc:
		case Opcodes::Z80_Ed_Nop_0xEd_0xBd:
		case Opcodes::Z80_Ed_Nop_0xEd_0xBe:
		case Opcodes::Z80_Ed_Nop_0xEd_0xBf:
		case Opcodes::Z80_Ed_Nop_0xEd_0xC0:
        case Opcodes::Z80_Ed_Nop_0xEd_0xC1:
        case Opcodes::Z80_Ed_Nop_0xEd_0xC2:
        case Opcodes::Z80_Ed_Nop_0xEd_0xC3:
        case Opcodes::Z80_Ed_Nop_0xEd_0xC4:
        case Opcodes::Z80_Ed_Nop_0xEd_0xC5:
        case Opcodes::Z80_Ed_Nop_0xEd_0xC6:
        case Opcodes::Z80_Ed_Nop_0xEd_0xC7:
        case Opcodes::Z80_Ed_Nop_0xEd_0xC8:
        case Opcodes::Z80_Ed_Nop_0xEd_0xC9:
        case Opcodes::Z80_Ed_Nop_0xEd_0xCa:
        case Opcodes::Z80_Ed_Nop_0xEd_0xCb:
        case Opcodes::Z80_Ed_Nop_0xEd_0xCc:
        case Opcodes::Z80_Ed_Nop_0xEd_0xCd:
        case Opcodes::Z80_Ed_Nop_0xEd_0xCe:
        case Opcodes::Z80_Ed_Nop_0xEd_0xCf:
        case Opcodes::Z80_Ed_Nop_0xEd_0xD0:
        case Opcodes::Z80_Ed_Nop_0xEd_0xD1:
        case Opcodes::Z80_Ed_Nop_0xEd_0xD2:
        case Opcodes::Z80_Ed_Nop_0xEd_0xD3:
        case Opcodes::Z80_Ed_Nop_0xEd_0xD4:
        case Opcodes::Z80_Ed_Nop_0xEd_0xD5:
        case Opcodes::Z80_Ed_Nop_0xEd_0xD6:
        case Opcodes::Z80_Ed_Nop_0xEd_0xD7:
        case Opcodes::Z80_Ed_Nop_0xEd_0xD8:
        case Opcodes::Z80_Ed_Nop_0xEd_0xD9:
        case Opcodes::Z80_Ed_Nop_0xEd_0xDa:
        case Opcodes::Z80_Ed_Nop_0xEd_0xDb:
        case Opcodes::Z80_Ed_Nop_0xEd_0xDc:
        case Opcodes::Z80_Ed_Nop_0xEd_0xDd:
        case Opcodes::Z80_Ed_Nop_0xEd_0xDe:
        case Opcodes::Z80_Ed_Nop_0xEd_0xDf:
        case Opcodes::Z80_Ed_Nop_0xEd_0xE0:
        case Opcodes::Z80_Ed_Nop_0xEd_0xE1:
        case Opcodes::Z80_Ed_Nop_0xEd_0xE2:
        case Opcodes::Z80_Ed_Nop_0xEd_0xE3:
        case Opcodes::Z80_Ed_Nop_0xEd_0xE4:
        case Opcodes::Z80_Ed_Nop_0xEd_0xE5:
        case Opcodes::Z80_Ed_Nop_0xEd_0xE6:
        case Opcodes::Z80_Ed_Nop_0xEd_0xE7:
        case Opcodes::Z80_Ed_Nop_0xEd_0xE8:
        case Opcodes::Z80_Ed_Nop_0xEd_0xE9:
        case Opcodes::Z80_Ed_Nop_0xEd_0xEa:
        case Opcodes::Z80_Ed_Nop_0xEd_0xEb:
        case Opcodes::Z80_Ed_Nop_0xEd_0xEc:
        case Opcodes::Z80_Ed_Nop_0xEd_0xEd:
        case Opcodes::Z80_Ed_Nop_0xEd_0xEe:
        case Opcodes::Z80_Ed_Nop_0xEd_0xEf:
        case Opcodes::Z80_Ed_Nop_0xEd_0xF0:
        case Opcodes::Z80_Ed_Nop_0xEd_0xF1:
        case Opcodes::Z80_Ed_Nop_0xEd_0xF2:
        case Opcodes::Z80_Ed_Nop_0xEd_0xF3:
        case Opcodes::Z80_Ed_Nop_0xEd_0xF4:
        case Opcodes::Z80_Ed_Nop_0xEd_0xF5:
        case Opcodes::Z80_Ed_Nop_0xEd_0xF6:
        case Opcodes::Z80_Ed_Nop_0xEd_0xF7:
        case Opcodes::Z80_Ed_Nop_0xEd_0xF8:
        case Opcodes::Z80_Ed_Nop_0xEd_0xF9:
        case Opcodes::Z80_Ed_Nop_0xEd_0xFa:
        case Opcodes::Z80_Ed_Nop_0xEd_0xFb:
        case Opcodes::Z80_Ed_Nop_0xEd_0xFc:
        case Opcodes::Z80_Ed_Nop_0xEd_0xFd:
        case Opcodes::Z80_Ed_Nop_0xEd_0xFe:
        case Opcodes::Z80_Ed_Nop_0xEd_0xFf:
			break;

		default:
            Util::debugln("unexpected opcode: 0xed {:#02x}", *instruction);
            throw InvalidOpcode({0xed, *instruction}, m_registers.pc);
	}

	return {
	    .tStates = (useJumpCycleCost ? 21_z80ub : EdOpcodeTStates[*instruction]),
	    .size = EdOpcodeSizes[*instruction],
	};
}

Z80::InstructionCost Z80::Z80::executeDdOrFdInstruction(UnsignedWord & reg, const UnsignedByte * instruction, bool * doPc)
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
		case Opcodes::Z80_DdOrFd_Add_IxOrIy_Bc: /*  0x09 */
			Z80_ADD_Reg16_Reg16(reg, m_registers.bc);
			break;

		case Opcodes::Z80_DdOrFd_Add_IxOrIy_De: /*  0x19 */
			Z80_ADD_Reg16_Reg16(reg, m_registers.de);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IxOrIy_Nn: /*  0x21 */
			Z80_LD_Reg16_NN(reg, z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1)));
			break;

		case Opcodes::Z80_DdOrFd_Ld_IndirectNn_IxOrIy: /*  0x22 */
			Z80_LD_IndirectNn_Reg16(z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1)), reg);
			break;

		case Opcodes::Z80_DdOrFd_Inc_IxOrIy: /*  0x23 */
			Z80_INC_Reg16(reg);
			break;

		case Opcodes::Z80_DdOrFd_Inc_IxhOrIyh: /*  0x24 */
			Z80_INC_Reg8(*regHigh);
			break;

		case Opcodes::Z80_DdOrFd_Dec_IxhOrIyh: /*  0x25 */
			Z80_DEC_Reg8(*regHigh);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IxhOrIyh_N: /*  0x26 */
			Z80_LD_Reg8_N(*regHigh, *(instruction + 1));
			break;

		case Opcodes::Z80_DdOrFd_Add_IxOrIy_IxOrIy: /*  0x29 */
			Z80_ADD_Reg16_Reg16(reg, reg);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IxOrIy_IndirectNn: /*  0x2a */
			Z80_LD_Reg16_IndirectNn(reg, z80ToHostByteOrder(*reinterpret_cast<const UnsignedWord *>(instruction + 1)));
			break;

		case Opcodes::Z80_DdOrFd_Dec_IxOrIy: /*  0x2b */
			Z80_DEC_Reg16(reg);
			break;

		case Opcodes::Z80_DdOrFd_Inc_IxlOrIyl: /*  0x2c */
			Z80_INC_Reg8(*regLow);
			break;

		case Opcodes::Z80_DdOrFd_Dec_IxlOrIyl: /*  0x2d */
			Z80_DEC_Reg8(*regLow);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IxlOrIyl_N: /*  0x2e */
			Z80_LD_Reg8_N(*regLow, *(instruction + 1));
			break;

		case Opcodes::Z80_DdOrFd_Inc_IndirectIxdOrIyd: /*  0x34 */
			Z80_INC_IndirectReg16D(reg, static_cast<SignedByte>(*(instruction + 1)));
			break;

		case Opcodes::Z80_DdOrFd_Dec_IndirectIxdOrIyd: /*  0x35 */
			Z80_DEC_IndirectReg16D(reg, static_cast<SignedByte>(*(instruction + 1)));
			break;

		case Opcodes::Z80_DdOrFd_Ld_IndirectIxdOrIyd_N: /*  0x36 */
			Z80_LD_IndirectReg16D_N(reg, static_cast<SignedByte>(*(instruction + 1)), *(instruction + 2));
			break;

		case Opcodes::Z80_DdOrFd_Add_IxOrIy_Sp: /*  0x39 */
			Z80_ADD_Reg16_Reg16(reg, m_registers.sp);
			break;

		case Opcodes::Z80_DdOrFd_Ld_B_IxhOrIyh: /*  0x44 */
			Z80_LD_Reg8_Reg8(m_registers.b, *regHigh);
			break;

		case Opcodes::Z80_DdOrFd_Ld_B_IxlOrIyl: /*  0x45 */
			Z80_LD_Reg8_Reg8(m_registers.b, *regLow);
			break;

		case Opcodes::Z80_DdOrFd_Ld_B_IndirectIxdOrIyd: /*  0x46 */
			Z80_LD_Reg8_IndirectReg16D(m_registers.b, reg, static_cast<SignedByte>(*(instruction + 1)));
			break;

		case Opcodes::Z80_DdOrFd_Ld_C_IxhOrIyh: /*  0x4c */
			Z80_LD_Reg8_Reg8(m_registers.c, *regHigh);
			break;

		case Opcodes::Z80_DdOrFd_Ld_C_IxlOrIyl: /*  0x4d */
			Z80_LD_Reg8_Reg8(m_registers.c, *regLow);
			break;

		case Opcodes::Z80_DdOrFd_Ld_C_IndirectIxdOrIyd: /*  0x4e */
			Z80_LD_Reg8_IndirectReg16D(m_registers.c, reg, static_cast<SignedByte>(*(instruction + 1)));
			break;

		case Opcodes::Z80_DdOrFd_Ld_D_IxhOrIyh: /*  0x54 */
			Z80_LD_Reg8_Reg8(m_registers.d, *regHigh);
			break;

		case Opcodes::Z80_DdOrFd_Ld_D_IxlOrIyl: /*  0x55 */
			Z80_LD_Reg8_Reg8(m_registers.d, *regLow);
			break;

		case Opcodes::Z80_DdOrFd_Ld_D_IndirectIxdOrIyd: /*  0x56 */
			Z80_LD_Reg8_IndirectReg16D(m_registers.d, reg, static_cast<SignedByte>(*(instruction + 1)));
			break;

		case Opcodes::Z80_DdOrFd_Ld_E_IxhOrIyh: /*  0x5c */
			Z80_LD_Reg8_Reg8(m_registers.e, *regHigh);
			break;

		case Opcodes::Z80_DdOrFd_Ld_E_IxlOrIyl: /*  0x5d */
			Z80_LD_Reg8_Reg8(m_registers.e, *regLow);
			break;

		case Opcodes::Z80_DdOrFd_Ld_E_IndirectIxdOrIyd: /*  0x5e */
			Z80_LD_Reg8_IndirectReg16D(m_registers.e, reg, static_cast<SignedByte>(*(instruction + 1)));
			break;

		case Opcodes::Z80_DdOrFd_Ld_IxhOrIyh_B: /*  0x60 */
			Z80_LD_Reg8_Reg8(*regHigh, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IxhOrIyh_C: /*  0x61 */
			Z80_LD_Reg8_Reg8(*regHigh, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IxhOrIyh_D: /*  0x62 */
			Z80_LD_Reg8_Reg8(*regHigh, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IxhOrIyh_E: /*  0x63 */
			Z80_LD_Reg8_Reg8(*regHigh, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IxhOrIyh_IxhOrIyh: /*  0x64 */
		    // NOOP - loading the register with itself, no flag changes
			break;

		case Opcodes::Z80_DdOrFd_Ld_IxhOrIyh_IxlOrIyl: /*  0x65 */
			Z80_LD_Reg8_Reg8(*regHigh, *regLow);
			break;

		case Opcodes::Z80_DdOrFd_Ld_H_IndirectIxdOrIyd: /*  0x66 */
			Z80_LD_Reg8_IndirectReg16D(m_registers.h, reg, static_cast<SignedByte>(*(instruction + 1)));
			break;

		case Opcodes::Z80_DdOrFd_Ld_IxhOrIyh_A: /*  0x67 */
			Z80_LD_Reg8_Reg8(*regHigh, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IxlOrIyl_B: /*  0x68 */
			Z80_LD_Reg8_Reg8(*regLow, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IxlOrIyl_C: /*  0x69 */
			Z80_LD_Reg8_Reg8(*regLow, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IxlOrIyl_D: /*  0x6a */
			Z80_LD_Reg8_Reg8(*regLow, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IxlOrIyl_E: /*  0x6b */
			Z80_LD_Reg8_Reg8(*regLow, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IxlOrIyl_IxhOrIyh: /*  0x6c */
			Z80_LD_Reg8_Reg8(*regLow, *regHigh);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IxlOrIyl_IxlOrIyl: /*  0x6d */
            // NOOP - loading the register with itself, no flag changes
			break;

		case Opcodes::Z80_DdOrFd_Ld_L_IndirectIxdOrIyd: /*  0x6e */
			Z80_LD_Reg8_IndirectReg16D(m_registers.l, reg, static_cast<SignedByte>(*(instruction + 1)));
			break;

		case Opcodes::Z80_DdOrFd_Ld_IxlOrIyl_A: /*  0x6f */
			Z80_LD_Reg8_Reg8(*regLow, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IndirectIxdOrIyd_B: /*  0x70 */
			Z80_LD_IndirectReg16D_Reg8(reg, static_cast<SignedByte>(*(instruction + 1)), m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IndirectIxdOrIyd_C: /*  0x71 */
			Z80_LD_IndirectReg16D_Reg8(reg, static_cast<SignedByte>(*(instruction + 1)), m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IndirectIxdOrIyd_D: /*  0x72 */
			Z80_LD_IndirectReg16D_Reg8(reg, static_cast<SignedByte>(*(instruction + 1)), m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IndirectIxdOrIyd_E: /*  0x73 */
			Z80_LD_IndirectReg16D_Reg8(reg, static_cast<SignedByte>(*(instruction + 1)), m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IndirectIxdOrIyd_H: /*  0x74 */
			Z80_LD_IndirectReg16D_Reg8(reg, static_cast<SignedByte>(*(instruction + 1)), m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IndirectIxdOrIyd_L: /*  0x75 */
			Z80_LD_IndirectReg16D_Reg8(reg, static_cast<SignedByte>(*(instruction + 1)), m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Ld_IndirectIxdOrIyd_A: /*  0x77 */
			Z80_LD_IndirectReg16D_Reg8(reg, static_cast<SignedByte>(*(instruction + 1)), m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Ld_A_IxhOrIyh: /*  0x7c */
			Z80_LD_Reg8_Reg8(m_registers.a, *regHigh);
			break;

		case Opcodes::Z80_DdOrFd_Ld_A_IxlOrIyl: /*  0x7d */
			Z80_LD_Reg8_Reg8(m_registers.a, *regLow);
			break;

		case Opcodes::Z80_DdOrFd_Ld_A_IndirectIxdOrIyd: /*  0x7e */
			Z80_LD_Reg8_IndirectReg16D(m_registers.a, reg, static_cast<SignedByte>(*(instruction + 1)));
			break;

		case Opcodes::Z80_DdOrFd_Add_A_IxhOrIyh: /*  0x84 */
			Z80_ADD_Reg8_Reg8(m_registers.a, *regHigh);
			break;

		case Opcodes::Z80_DdOrFd_Add_A_IxlOrIyl: /*  0x85 */
			Z80_ADD_Reg8_Reg8(m_registers.a, *regLow);
			break;

		case Opcodes::Z80_DdOrFd_Add_A_IndirectIxdOrIyd: /*  0x86 */
			Z80_ADD_Reg8_IndirectReg16D(m_registers.a, reg, static_cast<SignedByte>(*(instruction + 1)));
			break;

		case Opcodes::Z80_DdOrFd_Adc_A_IxhOrIyh: /*  0x8c */
			Z80_ADC_Reg8_Reg8(m_registers.a, *regHigh);
			break;

		case Opcodes::Z80_DdOrFd_Adc_A_IxlOrIyl: /*  0x8d */
			Z80_ADC_Reg8_Reg8(m_registers.a, *regLow);
			break;

		case Opcodes::Z80_DdOrFd_Adc_A_IndirectIxdOrIyd: /*  0x8e */
			Z80_ADC_Reg8_IndirectReg16D(m_registers.a, reg, static_cast<SignedByte>(*(instruction + 1)));
			break;

		case Opcodes::Z80_DdOrFd_Sub_IxhOrIyh: /*  0x94 */
			Z80_SUB_Reg8(*regHigh);
			break;

		case Opcodes::Z80_DdOrFd_Sub_IxlOrIyl: /*  0x95 */
			Z80_SUB_Reg8(*regLow);
			break;

		case Opcodes::Z80_DdOrFd_Sub_IndirectIxdOrIyd: /*  0x96 */
			Z80_SUB_IndirectReg16D(reg, static_cast<SignedByte>(*(instruction + 1)));
			break;

		case Opcodes::Z80_DdOrFd_Sbc_A_IxhOrIyh: /*  0x9c */
			Z80_SBC_Reg8_Reg8(m_registers.a, *regHigh);
			break;

		case Opcodes::Z80_DdOrFd_Sbc_A_IxlOrIyl: /*  0x9d */
			Z80_SBC_Reg8_Reg8(m_registers.a, *regLow);
			break;

		case Opcodes::Z80_DdOrFd_Sbc_A_IndirectIxdOrIyd: /*  0x9e */
			Z80_SBC_Reg8_IndirectReg16D(m_registers.a, reg, static_cast<SignedByte>(*(instruction + 1)));
			break;

		case Opcodes::Z80_DdOrFd_And_IxhOrIyh: /*  0xa4 */
			Z80_AND_Reg8(*regHigh);
			break;

		case Opcodes::Z80_DdOrFd_And_IxlOrIyl: /*  0xa5 */
			Z80_AND_Reg8(*regLow);
			break;

		case Opcodes::Z80_DdOrFd_And_IndirectIxdOrIyd: /*  0xa6 */
			Z80_AND_IndirectReg16D(reg, static_cast<SignedByte>(*(instruction + 1)));
			break;

		case Opcodes::Z80_DdOrFd_Xor_IxhOrIyh: /*  0xac */
			Z80_XOR_Reg8(*regHigh);
			break;

		case Opcodes::Z80_DdOrFd_Xor_IxlOrIyl: /*  0xad */
			Z80_XOR_Reg8(*regLow);
			break;

		case Opcodes::Z80_DdOrFd_Xor_IndirectIxdOrIyd: /*  0xae */
			Z80_XOR_IndirectReg16D(reg, static_cast<SignedByte>(*(instruction + 1)));
			break;

		case Opcodes::Z80_DdOrFd_Or_IxhOrIyh: /*  0xb4 */
			Z80_OR_Reg8(*regHigh);
			break;

		case Opcodes::Z80_DdOrFd_Or_IxlOrIyl: /*  0xb5 */
			Z80_OR_Reg8(*regLow);
			break;

		case Opcodes::Z80_DdOrFd_Or_IndirectIxdOrIyd: /*  0xb6 */
			Z80_OR_IndirectReg16D(reg, static_cast<SignedByte>(*(instruction + 1)));
			break;

		case Opcodes::Z80_DdOrFd_Cp_IxhOrIyh: /*  0xbc */
			Z80_CP_Reg8(*regHigh);
			break;

		case Opcodes::Z80_DdOrFd_Cp_IxlOrIyl: /*  0xbd */
			Z80_CP_Reg8(*regLow);
			break;

		case Opcodes::Z80_DdOrFd_Cp_IndirectIxdOrIyd: /*  0xbe */
			Z80_CP_IndirectReg16D(reg, static_cast<SignedByte>(*(instruction + 1)));
			break;

		case Opcodes::Z80_DdOrFd_Pop_IxOrIy: /*  0xe1 */
			Z80_POP_Reg16(reg);
			break;

		case Opcodes::Z80_DdOrFd_Ex_IndirectSp_IxOrIy: /*  0xe3 */
            Z80_EX_IndirectReg16_Reg16(m_registers.sp, reg);
			break;

        case Opcodes::Z80_DdOrFd_Push_IxOrIy:     //  0xe5
            Z80_PUSH_Reg16(reg);
            break;

		case Opcodes::Z80_DdOrFd_Jp_IxOrIy: /*  0xe9 */
		    // NOTE unlike the plain E9 that uses (HL) this IS NOT INDIRECT and is just JP IX or JP IY
			m_registers.pc = reg;
			Z80_DONT_UPDATE_PC;
			break;

		case Opcodes::Z80_DdOrFd_Ld_Sp_IxOrIy: /*  0xf9 */
			Z80_LD_Reg16_Reg16(m_registers.sp, reg);
			break;

		case Opcodes::Z80_DdOrFd_Prefix_Cb: /*  0xcb */
			return executeDdcbOrFdcbInstruction(reg, instruction + 1);

		/* the following are all (expensive) replicas of plain instructions, so
		 * defer to the plain opcode executor method */
		case Opcodes::Z80_DdOrFd_Nop:                // 0x00
		case Opcodes::Z80_DdOrFd_Ld_Bc_Nn:         // 0x01
		case Opcodes::Z80_DdOrFd_Ld_IndirectBc_A: // 0x02
		case Opcodes::Z80_DdOrFd_Inc_Bc:            // 0x03
		case Opcodes::Z80_DdOrFd_Inc_B:             // 0x04
		case Opcodes::Z80_DdOrFd_Dec_B:             // 0x05
		case Opcodes::Z80_DdOrFd_Ld_B_N:           // 0x06
		case Opcodes::Z80_DdOrFd_Rlca:               // 0x07

		case Opcodes::Z80_DdOrFd_Ex_Af_AfShadow:  // 0x08
		case Opcodes::Z80_DdOrFd_Ld_A_IndirectBc: // 0x0a
		case Opcodes::Z80_DdOrFd_Dec_Bc:            // 0x0b
		case Opcodes::Z80_DdOrFd_Inc_C:             // 0x0c
		case Opcodes::Z80_DdOrFd_Dec_C:             // 0x0d
		case Opcodes::Z80_DdOrFd_Ld_C_N:           // 0x0e
		case Opcodes::Z80_DdOrFd_Rrca:               // 0x0f

		case Opcodes::Z80_DdOrFd_Djnz_d:            // 0x10
		case Opcodes::Z80_DdOrFd_Ld_De_Nn:         // 0x11
		case Opcodes::Z80_DdOrFd_Ld_IndirectDe_A: // 0x12
		case Opcodes::Z80_DdOrFd_Inc_De:            // 0x13
		case Opcodes::Z80_DdOrFd_Inc_D:             // 0x14
		case Opcodes::Z80_DdOrFd_Dec_D:             // 0x15
		case Opcodes::Z80_DdOrFd_Ld_D_N:           // 0x16
		case Opcodes::Z80_DdOrFd_Rla:                // 0x17

		case Opcodes::Z80_DdOrFd_Jr_d:              // 0x18
		case Opcodes::Z80_DdOrFd_Ld_A_IndirectDe: // 0x1a
		case Opcodes::Z80_DdOrFd_Dec_De:            // 0x1b
		case Opcodes::Z80_DdOrFd_Inc_E:             // 0x1c
		case Opcodes::Z80_DdOrFd_Dec_E:             // 0x1d
		case Opcodes::Z80_DdOrFd_Ld_E_N:           // 0x1e
		case Opcodes::Z80_DdOrFd_Rra:                // 0x1f

		case Opcodes::Z80_DdOrFd_Jr_Nz_d:          // 0x20
		case Opcodes::Z80_DdOrFd_Daa:                // 0x27

		case Opcodes::Z80_DdOrFd_Jr_Z_d:           // 0x28
		case Opcodes::Z80_DdOrFd_Cpl:                // 0x2f

		case Opcodes::Z80_DdOrFd_Jr_Nc_d:          // 0x30
		case Opcodes::Z80_DdOrFd_Ld_Sp_Nn:         // 0x31
		case Opcodes::Z80_DdOrFd_Ld_IndirectNn_A: // 0x32
		case Opcodes::Z80_DdOrFd_Inc_Sp:            // 0x33
		case Opcodes::Z80_DdOrFd_Scf:                // 0x37

		case Opcodes::Z80_DdOrFd_Jr_C_d:           // 0x38
		case Opcodes::Z80_DdOrFd_Ld_A_IndirectNn: // 0x3a
		case Opcodes::Z80_DdOrFd_Dec_Sp:            // 0x3b
		case Opcodes::Z80_DdOrFd_Inc_A:             // 0x3c
		case Opcodes::Z80_DdOrFd_Dec_A:             // 0x3d
		case Opcodes::Z80_DdOrFd_Ld_A_N:           // 0x3e
		case Opcodes::Z80_DdOrFd_Ccf:                // 0x3f

		case Opcodes::Z80_DdOrFd_Ld_B_B:           // 0x40
		case Opcodes::Z80_DdOrFd_Ld_B_C:           // 0x41
		case Opcodes::Z80_DdOrFd_Ld_B_D:           // 0x42
		case Opcodes::Z80_DdOrFd_Ld_B_E:           // 0x43
		case Opcodes::Z80_DdOrFd_Ld_B_A:           // 0x47

		case Opcodes::Z80_DdOrFd_Ld_C_B:           // 0x48
		case Opcodes::Z80_DdOrFd_Ld_C_C:           // 0x49
		case Opcodes::Z80_DdOrFd_Ld_C_D:           // 0x4a
		case Opcodes::Z80_DdOrFd_Ld_C_E:           // 0x4b
		case Opcodes::Z80_DdOrFd_Ld_C_A:           // 0x4f

		case Opcodes::Z80_DdOrFd_Ld_D_B:           // 0x50
		case Opcodes::Z80_DdOrFd_Ld_D_C:           // 0x51
		case Opcodes::Z80_DdOrFd_Ld_D_D:           // 0x52
		case Opcodes::Z80_DdOrFd_Ld_D_E:           // 0x53
		case Opcodes::Z80_DdOrFd_Ld_D_A:           // 0x57

		case Opcodes::Z80_DdOrFd_Ld_E_B:           // 0x58
		case Opcodes::Z80_DdOrFd_Ld_E_C:           // 0x59
		case Opcodes::Z80_DdOrFd_Ld_E_D:           // 0x5a
		case Opcodes::Z80_DdOrFd_Ld_E_E:           // 0x5b
		case Opcodes::Z80_DdOrFd_Ld_E_A:           // 0x5f

		case Opcodes::Z80_DdOrFd_Halt:               // 0x76

		case Opcodes::Z80_DdOrFd_Ld_A_B:           // 0x78
		case Opcodes::Z80_DdOrFd_Ld_A_C:           // 0x79
		case Opcodes::Z80_DdOrFd_Ld_A_D:           // 0x7a
		case Opcodes::Z80_DdOrFd_Ld_A_E:           // 0x7b
		case Opcodes::Z80_DdOrFd_Ld_A_A:           // 0x7f

		case Opcodes::Z80_DdOrFd_Add_A_B:          // 0x80
		case Opcodes::Z80_DdOrFd_Add_A_C:          // 0x81
		case Opcodes::Z80_DdOrFd_Add_A_D:          // 0x82
		case Opcodes::Z80_DdOrFd_Add_A_E:          // 0x83
		case Opcodes::Z80_DdOrFd_Add_A_A:          // 0x87

		case Opcodes::Z80_DdOrFd_Adc_A_B:          // 0x88
		case Opcodes::Z80_DdOrFd_Adc_A_C:          // 0x89
		case Opcodes::Z80_DdOrFd_Adc_A_D:          // 0x8a
		case Opcodes::Z80_DdOrFd_Adc_A_E:          // 0x8b
		case Opcodes::Z80_DdOrFd_Adc_A_A:          // 0x8f

		case Opcodes::Z80_DdOrFd_Sub_B:             // 0x90
		case Opcodes::Z80_DdOrFd_Sub_C:             // 0x91
		case Opcodes::Z80_DdOrFd_Sub_D:             // 0x92
		case Opcodes::Z80_DdOrFd_Sub_E:             // 0x93
		case Opcodes::Z80_DdOrFd_Sub_A:             // 0x97

		case Opcodes::Z80_DdOrFd_Sbc_A_B:          // 0x98
		case Opcodes::Z80_DdOrFd_Sbc_A_C:          // 0x99
		case Opcodes::Z80_DdOrFd_Sbc_A_D:          // 0x9a
		case Opcodes::Z80_DdOrFd_Sbc_A_E:          // 0x9b
		case Opcodes::Z80_DdOrFd_Sbc_A_A:          // 0x9f

		case Opcodes::Z80_DdOrFd_And_B:             // 0xa0
		case Opcodes::Z80_DdOrFd_And_C:             // 0xa1
		case Opcodes::Z80_DdOrFd_And_D:             // 0xa2
		case Opcodes::Z80_DdOrFd_And_E:             // 0xa3
		case Opcodes::Z80_DdOrFd_And_A:             // 0xa7

		case Opcodes::Z80_DdOrFd_Xor_B:             // 0xa8
		case Opcodes::Z80_DdOrFd_Xor_C:             // 0xa9
		case Opcodes::Z80_DdOrFd_Xor_D:             // 0xaa
		case Opcodes::Z80_DdOrFd_Xor_E:             // 0xab
		case Opcodes::Z80_DdOrFd_Xor_A:             // 0xaf

		case Opcodes::Z80_DdOrFd_Or_B:              // 0xb0
		case Opcodes::Z80_DdOrFd_Or_C:              // 0xb1
		case Opcodes::Z80_DdOrFd_Or_D:              // 0xb2
		case Opcodes::Z80_DdOrFd_Or_E:              // 0xb3
		case Opcodes::Z80_DdOrFd_Or_A:              // 0xb7

		case Opcodes::Z80_DdOrFd_Cp_B:              // 0xb8
		case Opcodes::Z80_DdOrFd_Cp_C:              // 0xb9
		case Opcodes::Z80_DdOrFd_Cp_D:              // 0xba
		case Opcodes::Z80_DdOrFd_Cp_E:              // 0xbb
		case Opcodes::Z80_DdOrFd_Cp_A:              // 0xbf

		case Opcodes::Z80_DdOrFd_Ret_Nz:            // 0xc0
		case Opcodes::Z80_DdOrFd_Pop_Bc:            // 0xc1
		case Opcodes::Z80_DdOrFd_Jp_Nz_Nn:         // 0xc2
		case Opcodes::Z80_DdOrFd_Jp_Nn:             // 0xc3
		case Opcodes::Z80_DdOrFd_Call_Nz_Nn:       // 0xc4
		case Opcodes::Z80_DdOrFd_Push_Bc:           // 0xc5
		case Opcodes::Z80_DdOrFd_Add_A_N:          // 0xc6
		case Opcodes::Z80_DdOrFd_Rst_00:            // 0xc7

		case Opcodes::Z80_DdOrFd_Ret_Z:             // 0xc8
		case Opcodes::Z80_DdOrFd_Ret:                // 0xc9
		case Opcodes::Z80_DdOrFd_Jp_Z_Nn:          // 0xca
		case Opcodes::Z80_DdOrFd_Call_Z_Nn:        // 0xcc
		case Opcodes::Z80_DdOrFd_Call_Nn:           // 0xcd
		case Opcodes::Z80_DdOrFd_Adc_A_N:          // 0xce
		case Opcodes::Z80_DdOrFd_Rst_08:            // 0xcf

		case Opcodes::Z80_DdOrFd_Ret_Nc:            // 0xd0
		case Opcodes::Z80_DdOrFd_Pop_De:            // 0xd1
		case Opcodes::Z80_DdOrFd_Jp_Nc_Nn:         // 0xd2
		case Opcodes::Z80_DdOrFd_Out_IndirectN_A: // 0xd3
		case Opcodes::Z80_DdOrFd_Call_Nc_Nn:       // 0xd4
		case Opcodes::Z80_DdOrFd_Push_De:           // 0xd5
		case Opcodes::Z80_DdOrFd_Sub_N:             // 0xd6
		case Opcodes::Z80_DdOrFd_Rst_10:            // 0xd7

		case Opcodes::Z80_DdOrFd_Ret_C:             // 0xd8
		case Opcodes::Z80_DdOrFd_Exx:                // 0xd9
		case Opcodes::Z80_DdOrFd_Jp_C_Nn:          // 0xda
		case Opcodes::Z80_DdOrFd_In_A_IndirectN:  // 0xdb
		case Opcodes::Z80_DdOrFd_Call_C_Nn:        // 0xdc
		case Opcodes::Z80_DdOrFd_Sbc_A_N:          // 0xde
		case Opcodes::Z80_DdOrFd_Rst_18:            // 0xdf

		case Opcodes::Z80_DdOrFd_Ret_Po:            // 0xe0
		case Opcodes::Z80_DdOrFd_Jp_Po_Nn:         // 0xe2
		case Opcodes::Z80_DdOrFd_Call_Po_Nn:       // 0xe4
		case Opcodes::Z80_DdOrFd_And_N:             // 0xe6
		case Opcodes::Z80_DdOrFd_Rst_20:            // 0xe7

		case Opcodes::Z80_DdOrFd_Ret_Pe:            // 0xe8
		case Opcodes::Z80_DdOrFd_Jp_Pe_Nn:         // 0xea
		case Opcodes::Z80_DdOrFd_Ex_De_Hl:         // 0xeb
		case Opcodes::Z80_DdOrFd_Call_Pe_Nn:       // 0xec
		case Opcodes::Z80_DdOrFd_Prefix_Ed:         // 0xed
		case Opcodes::Z80_DdOrFd_Xor_N:             // 0xee
		case Opcodes::Z80_DdOrFd_Rst_28:            // 0xef

		case Opcodes::Z80_DdOrFd_Ret_P:             // 0xf0
		case Opcodes::Z80_DdOrFd_Pop_Af:            // 0xf1
		case Opcodes::Z80_DdOrFd_Jp_P_Nn:          // 0xf2
		case Opcodes::Z80_DdOrFd_Di:                 // 0xf3
		case Opcodes::Z80_DdOrFd_Call_P_Nn:        // 0xf4
		case Opcodes::Z80_DdOrFd_Push_Af:           // 0xf5
		case Opcodes::Z80_DdOrFd_Or_N:              // 0xf6
		case Opcodes::Z80_DdOrFd_Rst_30:            // 0xf7

		case Opcodes::Z80_DdOrFd_Ret_M:             // 0xf8
		case Opcodes::Z80_DdOrFd_Jp_M_Nn:          // 0xfa
		case Opcodes::Z80_DdOrFd_Ei:                 // 0xfb
		case Opcodes::Z80_DdOrFd_Call_M_Nn:        // 0xfc
		case Opcodes::Z80_DdOrFd_Cp_N:              // 0xfe
		case Opcodes::Z80_DdOrFd_Rst_38:            // 0xff
            {
                // these are (expensive) replicas of plain instructions, so defer to the plain opcode executor method
                auto cost = executePlainInstruction(instruction + 1, doPc);
                ++cost.size;
                return cost;
            }
            break;

        case Opcodes::Z80_DdOrFd_Prefix_Dd:         // 0xdd
        case Opcodes::Z80_DdOrFd_Prefix_Fd:         // 0xfd
#if (!defined(NDEBUG))
            // this is like a NOP - the second 0xdd or 0xfd supersedes the first and consumes 1 byte and 4 t-states. The
            // PC is subsequently incremented and the second 0xdd or 0xfd becomes the first byte of the next instruction
            Util::debugln("Encountered redundant double-extended {:#02x}", static_cast<std::uint16_t>(*instruction));
#endif
            return {4, 1};

	    default:
            {
                UnsignedByte prefix = (&reg == &m_registers.ix ? 0xdd : 0xfd);
                Util::debugln("unexpected opcode: {:#02x} {:#02x}", prefix, *instruction);
                throw InvalidOpcode({prefix, *instruction}, m_registers.pc);
            }
    }

	return {
	    .tStates = static_cast<std::uint8_t>(useJumpCycleCost ? Z80_TSTATES_JUMP(DdOrFdOpcodeTStates[*instruction]) : Z80_TSTATES_NOJUMP(DdOrFdOpcodeTStates[*instruction])),
	    .size = DdOrFdOpcodeSizes[*instruction],
	};
}

Z80::InstructionCost Z80::Z80::executeDdcbOrFdcbInstruction(UnsignedWord & reg, const UnsignedByte * instruction)
{
	// NOTE these opcodes are of the form 0xdd 0xcb DD II or 0xfd 0xcb DD II where II is the 8-bit opcode and DD is the
	// 8-bit 2s-complement offset to use with IX or IY
	auto d = static_cast<SignedByte>(*(instruction));
	auto opcodeByte = *(instruction + 1);

	switch(opcodeByte) {
		case Opcodes::Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0x00 */
			Z80_RLC_IndirectReg16D_Reg8(reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0x01 */
			Z80_RLC_IndirectReg16D_Reg8(reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0x02 */
			Z80_RLC_IndirectReg16D_Reg8(reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0x03 */
			Z80_RLC_IndirectReg16D_Reg8(reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0x04 */
			Z80_RLC_IndirectReg16D_Reg8(reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0x05 */
			Z80_RLC_IndirectReg16D_Reg8(reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0x06 */
			Z80_RLC_IndirectReg16D(reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0x07 */
			Z80_RLC_IndirectReg16D_Reg8(reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0x08 */
			Z80_RRC_IndirectReg16D_Reg8(reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0x09 */
			Z80_RRC_IndirectReg16D_Reg8(reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0x0a */
			Z80_RRC_IndirectReg16D_Reg8(reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0x0b */
			Z80_RRC_IndirectReg16D_Reg8(reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0x0c */
			Z80_RRC_IndirectReg16D_Reg8(reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0x0d */
			Z80_RRC_IndirectReg16D_Reg8(reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0x0e */
			Z80_RRC_IndirectReg16D(reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0x0f */
			Z80_RRC_IndirectReg16D_Reg8(reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0x10 */
			Z80_RL_IndirectReg16D_Reg8(reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0x11 */
			Z80_RL_IndirectReg16D_Reg8(reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0x12 */
			Z80_RL_IndirectReg16D_Reg8(reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0x13 */
			Z80_RL_IndirectReg16D_Reg8(reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0x14 */
			Z80_RL_IndirectReg16D_Reg8(reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0x15 */
			Z80_RL_IndirectReg16D_Reg8(reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0x16 */
			Z80_RL_IndirectReg16D(reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0x17 */
			Z80_RL_IndirectReg16D_Reg8(reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0x18 */
			Z80_RR_IndirectReg16D_Reg8(reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0x19 */
			Z80_RR_IndirectReg16D_Reg8(reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0x1a */
			Z80_RR_IndirectReg16D_Reg8(reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0x1b */
			Z80_RR_IndirectReg16D_Reg8(reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0x1c */
			Z80_RR_IndirectReg16D_Reg8(reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0x1d */
			Z80_RR_IndirectReg16D_Reg8(reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0x1e */
			Z80_RR_IndirectReg16D(reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0x1f */
			Z80_RR_IndirectReg16D_Reg8(reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0x21 */
			Z80_SLA_IndirectReg16D_Reg8(reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0x22 */
			Z80_SLA_IndirectReg16D_Reg8(reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0x23 */
			Z80_SLA_IndirectReg16D_Reg8(reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0x24 */
			Z80_SLA_IndirectReg16D_Reg8(reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0x25 */
			Z80_SLA_IndirectReg16D_Reg8(reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0x26 */
			Z80_SLA_IndirectReg16D_Reg8(reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0x26 */
			Z80_SLA_IndirectReg16D(reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0x27 */
			Z80_SLA_IndirectReg16D_Reg8(reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0x28 */
			Z80_SRA_IndirectReg16D_Reg8(reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0x29 */
			Z80_SRA_IndirectReg16D_Reg8(reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0x2a */
			Z80_SRA_IndirectReg16D_Reg8(reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0x2b */
			Z80_SRA_IndirectReg16D_Reg8(reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0x2c */
			Z80_SRA_IndirectReg16D_Reg8(reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0x2d */
			Z80_SRA_IndirectReg16D_Reg8(reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0x2e */
			Z80_SRA_IndirectReg16D(reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0x2f */
			Z80_SRA_IndirectReg16D_Reg8(reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0x30 */
			Z80_SLL_IndirectReg16D_Reg8(reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0x31 */
			Z80_SLL_IndirectReg16D_Reg8(reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0x32 */
			Z80_SLL_IndirectReg16D_Reg8(reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0x33 */
			Z80_SLL_IndirectReg16D_Reg8(reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0x34 */
			Z80_SLL_IndirectReg16D_Reg8(reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0x35 */
			Z80_SLL_IndirectReg16D_Reg8(reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0x36 */
			Z80_SLL_IndirectReg16D(reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0x37 */
			Z80_SLL_IndirectReg16D_Reg8(reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0x38 */
			Z80_SRL_IndirectReg16D_Reg8(reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0x39 */
			Z80_SRL_IndirectReg16D_Reg8(reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0x3a */
			Z80_SRL_IndirectReg16D_Reg8(reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0x3b */
			Z80_SRL_IndirectReg16D_Reg8(reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0x3c */
			Z80_SRL_IndirectReg16D_Reg8(reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0x3d */
			Z80_SRL_IndirectReg16D_Reg8(reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0x3e */
			Z80_SRL_IndirectReg16D(reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0x3f */
			Z80_SRL_IndirectReg16D_Reg8(reg, d, m_registers.a);
			break;

		// BIT opcodes
		// These are all the (IX/IY) + d equivalents of the 0xcb BIT opcodes that work with specific reg8s, except that
		// these versions don't use the reg8, so they're all just the BIT opcode on the memory offset - i.e. 8 identical
		// opcodes for each bit position
		case Opcodes::Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0x40 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0x41 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0x42 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0x43 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0x44 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0x45 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0x46 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0x47 */
			Z80_BIT_N_IndirectReg16D(0, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0x48 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0x49 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0x4a */
		case Opcodes::Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0x4b */
		case Opcodes::Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0x4c */
		case Opcodes::Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0x4d */
		case Opcodes::Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0x4e */
		case Opcodes::Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0x4f */
			Z80_BIT_N_IndirectReg16D(1, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0x50 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0x51 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0x52 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0x53 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0x54 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0x55 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0x56 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0x57 */
			Z80_BIT_N_IndirectReg16D(2, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0x58 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0x59 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0x5a */
		case Opcodes::Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0x5b */
		case Opcodes::Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0x5c */
		case Opcodes::Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0x5d */
		case Opcodes::Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0x5e */
		case Opcodes::Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0x5f */
			Z80_BIT_N_IndirectReg16D(3, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0x60 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0x61 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0x62 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0x63 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0x64 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0x65 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0x66 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0x67 */
			Z80_BIT_N_IndirectReg16D(4, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0x68 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0x69 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0x6a */
		case Opcodes::Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0x6b */
		case Opcodes::Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0x6c */
		case Opcodes::Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0x6d */
		case Opcodes::Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0x6e */
		case Opcodes::Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0x6f */
			Z80_BIT_N_IndirectReg16D(5, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0x70 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0x71 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0x72 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0x73 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0x74 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0x75 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0x76 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0x77 */
			Z80_BIT_N_IndirectReg16D(6, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0x78 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0x79 */
		case Opcodes::Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0x7a */
		case Opcodes::Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0x7b */
		case Opcodes::Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0x7c */
		case Opcodes::Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0x7d */
		case Opcodes::Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0x7e */
		case Opcodes::Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0x7f */
			Z80_BIT_N_IndirectReg16D(7, reg, d);
			break;

		/* RES opcodes */
		case Opcodes::Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0x80 */
			Z80_RES_N_IndirectReg16D_Reg8(0, reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0x81 */
			Z80_RES_N_IndirectReg16D_Reg8(0, reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0x82 */
			Z80_RES_N_IndirectReg16D_Reg8(0, reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0x83 */
			Z80_RES_N_IndirectReg16D_Reg8(0, reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0x84 */
			Z80_RES_N_IndirectReg16D_Reg8(0, reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0x85 */
			Z80_RES_N_IndirectReg16D_Reg8(0, reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0x86 */
			Z80_RES_N_IndirectReg16D(0, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0x87 */
			Z80_RES_N_IndirectReg16D_Reg8(0, reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0x88 */
			Z80_RES_N_IndirectReg16D_Reg8(1, reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0x89 */
			Z80_RES_N_IndirectReg16D_Reg8(1, reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0x8a */
			Z80_RES_N_IndirectReg16D_Reg8(1, reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0x8b */
			Z80_RES_N_IndirectReg16D_Reg8(1, reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0x8c */
			Z80_RES_N_IndirectReg16D_Reg8(1, reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0x8d */
			Z80_RES_N_IndirectReg16D_Reg8(1, reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0x8e */
			Z80_RES_N_IndirectReg16D(1, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0x8f */
			Z80_RES_N_IndirectReg16D_Reg8(1, reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0x90 */
			Z80_RES_N_IndirectReg16D_Reg8(2, reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0x91 */
			Z80_RES_N_IndirectReg16D_Reg8(2, reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0x92 */
			Z80_RES_N_IndirectReg16D_Reg8(2, reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0x93 */
			Z80_RES_N_IndirectReg16D_Reg8(2, reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0x94 */
			Z80_RES_N_IndirectReg16D_Reg8(2, reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0x95 */
			Z80_RES_N_IndirectReg16D_Reg8(2, reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0x96 */
			Z80_RES_N_IndirectReg16D(2, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0x97 */
			Z80_RES_N_IndirectReg16D_Reg8(2, reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0x98 */
			Z80_RES_N_IndirectReg16D_Reg8(3, reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0x99 */
			Z80_RES_N_IndirectReg16D_Reg8(3, reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0x9a */
			Z80_RES_N_IndirectReg16D_Reg8(3, reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0x9b */
			Z80_RES_N_IndirectReg16D_Reg8(3, reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0x9c */
			Z80_RES_N_IndirectReg16D_Reg8(3, reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0x9d */
			Z80_RES_N_IndirectReg16D_Reg8(3, reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0x9e */
			Z80_RES_N_IndirectReg16D(3, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0x9f */
			Z80_RES_N_IndirectReg16D_Reg8(3, reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0xa0 */
			Z80_RES_N_IndirectReg16D_Reg8(4, reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0xa1 */
			Z80_RES_N_IndirectReg16D_Reg8(4, reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0xa2 */
			Z80_RES_N_IndirectReg16D_Reg8(4, reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0xa3 */
			Z80_RES_N_IndirectReg16D_Reg8(4, reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0xa4 */
			Z80_RES_N_IndirectReg16D_Reg8(4, reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0xa5 */
			Z80_RES_N_IndirectReg16D_Reg8(4, reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0xa6 */
			Z80_RES_N_IndirectReg16D(4, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0xa7 */
			Z80_RES_N_IndirectReg16D_Reg8(4, reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0xa8 */
			Z80_RES_N_IndirectReg16D_Reg8(5, reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0xa9 */
			Z80_RES_N_IndirectReg16D_Reg8(5, reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0xaa */
			Z80_RES_N_IndirectReg16D_Reg8(5, reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0xab */
			Z80_RES_N_IndirectReg16D_Reg8(5, reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0xac */
			Z80_RES_N_IndirectReg16D_Reg8(5, reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0xad */
			Z80_RES_N_IndirectReg16D_Reg8(5, reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0xae */
			Z80_RES_N_IndirectReg16D(5, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0xaf */
			Z80_RES_N_IndirectReg16D_Reg8(5, reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0xb0 */
			Z80_RES_N_IndirectReg16D_Reg8(6, reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0xb1 */
			Z80_RES_N_IndirectReg16D_Reg8(6, reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0xb2 */
			Z80_RES_N_IndirectReg16D_Reg8(6, reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0xb3 */
			Z80_RES_N_IndirectReg16D_Reg8(6, reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0xb4 */
			Z80_RES_N_IndirectReg16D_Reg8(6, reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0xb5 */
			Z80_RES_N_IndirectReg16D_Reg8(6, reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0xb6 */
			Z80_RES_N_IndirectReg16D(6, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0xb7 */
			Z80_RES_N_IndirectReg16D_Reg8(6, reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0xb8 */
			Z80_RES_N_IndirectReg16D_Reg8(7, reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0xb9 */
			Z80_RES_N_IndirectReg16D_Reg8(7, reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0xba */
			Z80_RES_N_IndirectReg16D_Reg8(7, reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0xbb */
			Z80_RES_N_IndirectReg16D_Reg8(7, reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0xbc */
			Z80_RES_N_IndirectReg16D_Reg8(7, reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0xbd */
			Z80_RES_N_IndirectReg16D_Reg8(7, reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0xbe */
			Z80_RES_N_IndirectReg16D(7, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0xbf */
			Z80_RES_N_IndirectReg16D_Reg8(7, reg, d, m_registers.a);
			break;

		/* SET opcodes */
		case Opcodes::Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0xc0 */
			Z80_SET_N_IndirectReg16D_Reg8(0, reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0xc1 */
			Z80_SET_N_IndirectReg16D_Reg8(0, reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0xc2 */
			Z80_SET_N_IndirectReg16D_Reg8(0, reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0xc3 */
			Z80_SET_N_IndirectReg16D_Reg8(0, reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0xc4 */
			Z80_SET_N_IndirectReg16D_Reg8(0, reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0xc5 */
			Z80_SET_N_IndirectReg16D_Reg8(0, reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0xc6 */
			Z80_SET_N_IndirectReg16D(0, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0xc7 */
			Z80_SET_N_IndirectReg16D_Reg8(0, reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0xc8 */
			Z80_SET_N_IndirectReg16D_Reg8(1, reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0xc9 */
			Z80_SET_N_IndirectReg16D_Reg8(1, reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0xca */
			Z80_SET_N_IndirectReg16D_Reg8(1, reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0xcb */
			Z80_SET_N_IndirectReg16D_Reg8(1, reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0xcc */
			Z80_SET_N_IndirectReg16D_Reg8(1, reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0xcd */
			Z80_SET_N_IndirectReg16D_Reg8(1, reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0xce */
			Z80_SET_N_IndirectReg16D(1, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0xcf */
			Z80_SET_N_IndirectReg16D_Reg8(1, reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0xd0 */
			Z80_SET_N_IndirectReg16D_Reg8(2, reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0xd1 */
			Z80_SET_N_IndirectReg16D_Reg8(2, reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0xd2 */
			Z80_SET_N_IndirectReg16D_Reg8(2, reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0xd3 */
			Z80_SET_N_IndirectReg16D_Reg8(2, reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0xd4 */
			Z80_SET_N_IndirectReg16D_Reg8(2, reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0xd5 */
			Z80_SET_N_IndirectReg16D_Reg8(2, reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0xd6 */
			Z80_SET_N_IndirectReg16D(2, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0xd7 */
			Z80_SET_N_IndirectReg16D_Reg8(2, reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0xd8 */
			Z80_SET_N_IndirectReg16D_Reg8(3, reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0xd9 */
			Z80_SET_N_IndirectReg16D_Reg8(3, reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0xda */
			Z80_SET_N_IndirectReg16D_Reg8(3, reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0xdb */
			Z80_SET_N_IndirectReg16D_Reg8(3, reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0xdc */
			Z80_SET_N_IndirectReg16D_Reg8(3, reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0xdd */
			Z80_SET_N_IndirectReg16D_Reg8(3, reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0xde */
			Z80_SET_N_IndirectReg16D(3, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0xdf */
			Z80_SET_N_IndirectReg16D_Reg8(3, reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0xe0 */
			Z80_SET_N_IndirectReg16D_Reg8(4, reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0xe1 */
			Z80_SET_N_IndirectReg16D_Reg8(4, reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0xe2 */
			Z80_SET_N_IndirectReg16D_Reg8(4, reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0xe3 */
			Z80_SET_N_IndirectReg16D_Reg8(4, reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0xe4 */
			Z80_SET_N_IndirectReg16D_Reg8(4, reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0xe5 */
			Z80_SET_N_IndirectReg16D_Reg8(4, reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0xe6 */
			Z80_SET_N_IndirectReg16D(4, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0xe7 */
			Z80_SET_N_IndirectReg16D_Reg8(4, reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0xe8 */
			Z80_SET_N_IndirectReg16D_Reg8(5, reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0xe9 */
			Z80_SET_N_IndirectReg16D_Reg8(5, reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0xea */
			Z80_SET_N_IndirectReg16D_Reg8(5, reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0xeb */
			Z80_SET_N_IndirectReg16D_Reg8(5, reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0xec */
			Z80_SET_N_IndirectReg16D_Reg8(5, reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0xed */
			Z80_SET_N_IndirectReg16D_Reg8(5, reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0xee */
			Z80_SET_N_IndirectReg16D(5, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0xef */
			Z80_SET_N_IndirectReg16D_Reg8(5, reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0xf0 */
			Z80_SET_N_IndirectReg16D_Reg8(6, reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0xf1 */
			Z80_SET_N_IndirectReg16D_Reg8(6, reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0xf2 */
			Z80_SET_N_IndirectReg16D_Reg8(6, reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0xf3 */
			Z80_SET_N_IndirectReg16D_Reg8(6, reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0xf4 */
			Z80_SET_N_IndirectReg16D_Reg8(6, reg, d, m_registers.h);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0xf5 */
			Z80_SET_N_IndirectReg16D_Reg8(6, reg, d, m_registers.l);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd:		/* 0xdd/0xfd 0xcb 0xf6 */
			Z80_SET_N_IndirectReg16D(6, reg, d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0xf7 */
			Z80_SET_N_IndirectReg16D_Reg8(6, reg, d, m_registers.a);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_B:		/* 0xdd/0xfd 0xcb 0xf8 */
			Z80_SET_N_IndirectReg16D_Reg8(7, reg, d, m_registers.b);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_C:		/* 0xdd/0xfd 0xcb 0xf9 */
			Z80_SET_N_IndirectReg16D_Reg8(7, reg, d, m_registers.c);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_D:		/* 0xdd/0xfd 0xcb 0xfa */
			Z80_SET_N_IndirectReg16D_Reg8(7, reg, d, m_registers.d);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_E:		/* 0xdd/0xfd 0xcb 0xfb */
			Z80_SET_N_IndirectReg16D_Reg8(7, reg, d, m_registers.e);
			break;

		case Opcodes::Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_H:		/* 0xdd/0xfd 0xcb 0xfc */
			Z80_SET_N_IndirectReg16D_Reg8(7, reg, d, m_registers.h);
			break;

	    case Opcodes::Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_L:		/* 0xdd/0xfd 0xcb 0xfd */
            Z80_SET_N_IndirectReg16D_Reg8(7, reg, d, m_registers.l);
            break;

	    case Opcodes::Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd:			/* 0xdd/0xfd 0xcb 0xfe */
            Z80_SET_N_IndirectReg16D(7, reg, d);
            break;

	    case Opcodes::Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_A:		/* 0xdd/0xfd 0xcb 0xff */
            Z80_SET_N_IndirectReg16D_Reg8(7, reg, d, m_registers.a);
            break;
	}

	return {
	    .tStates = DdCbOrFdCbOpcodeTStates[opcodeByte],
	    .size = 4,
	};
}

UnsignedWord Z80::Z80::registerValue(const Register16 reg) const noexcept
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

UnsignedWord Z80::Z80::registerValueZ80(const Register16 reg) const noexcept
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

UnsignedByte Z80::Z80::registerValue(const Register8 reg) const noexcept
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

void Z80::Z80::setRegisterValue(const Register16 reg, const UnsignedWord value) noexcept
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

void Z80::Z80::setRegisterValueZ80(const Register16 reg, const UnsignedWord value) noexcept
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

void Z80::Z80::setRegisterValue(const Register8 reg, const UnsignedByte value) noexcept
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

#if !defined(NDEBUG)
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

#if defined(DEBUG_EXECUTION_HISTORY)
void Z80::Z80::dumpExecutionHistory(const int entries, std::ostream & out) const
{
    auto entry = m_executionHistory.newest();
    out << "\n======================================================================\n";
    out << "Instruction History\n";

    for (int instructionIndex = 0; instructionIndex < entries; ++instructionIndex) {
        auto mnemonic = Assembly::Disassembler::disassembleOne(entry->machineCode);
        out << "\n----------------------------------------------------------------------\n";
        out << "#" << std::dec << std::setw(0) << instructionIndex << " (@ 0x"
            << std::hex << std::setfill('0') << std::setw(4) << entry->registersBefore.pc << ")\n"
            << to_string(mnemonic) << "          [" << std::hex << std::setfill('0');

        for (auto byteIndex = 0; byteIndex < mnemonic.size; ++byteIndex) {
            if (0 < byteIndex) {
                out << ", ";
            }

            out << "0x" << std::setw(2) << static_cast<std::uint16_t>(entry->machineCode[byteIndex]);
        }

        out << "]\n";

        out << to_string(mnemonic.instruction) << ' ';

        for (auto operandIndex = 0; operandIndex < mnemonic.operands.size(); ++operandIndex) {
            auto operandValue = entry->evaluateOperand(mnemonic.operands[operandIndex], 0 == operandIndex);

            if (0 < operandIndex) {
                out << ", ";
            }

            switch (mnemonic.operands[operandIndex].mode) {
                case Assembly::AddressingMode::Immediate:
                case Assembly::AddressingMode::Register8:
                    out << "0x" << std::setw(2) << static_cast<std::uint16_t>(operandValue.unsignedByte);
                    break;

                case Assembly::AddressingMode::ImmediateExtended:
                case Assembly::AddressingMode::ModifiedPageZero:
                case Assembly::AddressingMode::Relative:
                case Assembly::AddressingMode::Extended:
                case Assembly::AddressingMode::Indexed:
                case Assembly::AddressingMode::Register16:
                case Assembly::AddressingMode::Register8Indirect:
                case Assembly::AddressingMode::Register16Indirect:
                    out << "0x" << std::setw(4) << static_cast<std::uint16_t>(operandValue.unsignedWord);
                    break;

                case Assembly::AddressingMode::Bit:
                    out << std::setw(1) << static_cast<std::uint16_t>(operandValue.bit);
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

    out << "\n======================================================================\n";
}
#endif
#endif

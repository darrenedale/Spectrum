//
// Created by darren on 15/03/2021.
//

#include "registers.h"

using namespace Z80;

Registers::Registers(const Registers & other)
: af(other.af),
  bc(other.bc),
  de(other.de),
  hl(other.hl),
  ix(other.ix),
  iy(other.iy),
  pc(other.pc),
  sp(other.sp),
  afShadow(other.afShadow),
  bcShadow(other.bcShadow),
  deShadow(other.deShadow),
  hlShadow(other.hlShadow),
  memptr(other.memptr),
  i(other.i),
  r(other.r)
{}

void Registers::reset() noexcept
{
    af = 0xffff;
    bc = 0x0000;
    de = 0x0000;
    hl = 0x0000;
    ix = 0x0000;
    iy = 0x0000;
    pc = 0x0000;
    sp = 0xffff;
    afShadow = 0xffff;
    bcShadow = 0x0000;
    deShadow = 0x0000;
    hlShadow = 0x0000;
    memptr = 0x0000;
    i = 0;
    r = 0;
}

Registers & Registers::operator=(const Registers & other)
{
    af = other.af;
    bc = other.bc;
    de = other.de;
    hl = other.hl;
    ix = other.ix;
    iy = other.iy;
    pc = other.pc;
    sp = other.sp;

    afShadow = other.afShadow;
    bcShadow = other.bcShadow;
    deShadow = other.deShadow;
    hlShadow = other.hlShadow;

    memptr = other.memptr;

    i = other.i;
    r = other.r;
    return *this;
}

std::ostream & operator<<(std::ostream & out, const RegisterZ80Endian & reg)
{
    out << static_cast<UnsignedWord>(reg);
    return out;
}

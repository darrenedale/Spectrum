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
  r(other.r),
  a((*(reinterpret_cast<UnsignedByte *>(&af) + (ByteOrderMatch ? 1 : 0)))),
  f((*(reinterpret_cast<UnsignedByte *>(&af) + (ByteOrderMatch ? 0 : 1)))),
  b((*(reinterpret_cast<UnsignedByte *>(&bc) + (ByteOrderMatch ? 1 : 0)))),
  c((*(reinterpret_cast<UnsignedByte *>(&bc) + (ByteOrderMatch ? 0 : 1)))),
  d((*(reinterpret_cast<UnsignedByte *>(&de) + (ByteOrderMatch ? 1 : 0)))),
  e((*(reinterpret_cast<UnsignedByte *>(&de) + (ByteOrderMatch ? 0 : 1)))),
  h((*(reinterpret_cast<UnsignedByte *>(&hl) + (ByteOrderMatch ? 1 : 0)))),
  l((*(reinterpret_cast<UnsignedByte *>(&hl) + (ByteOrderMatch ? 0 : 1)))),
  ixh((*(reinterpret_cast<UnsignedByte *>(&ix) + (ByteOrderMatch ? 1 : 0)))),
  ixl((*(reinterpret_cast<UnsignedByte *>(&ix) + (ByteOrderMatch ? 0 : 1)))),
  iyh((*(reinterpret_cast<UnsignedByte *>(&iy) + (ByteOrderMatch ? 1 : 0)))),
  iyl((*(reinterpret_cast<UnsignedByte *>(&iy) + (ByteOrderMatch ? 0 : 1)))),
  aShadow((*(reinterpret_cast<UnsignedByte *>(&afShadow) + (ByteOrderMatch ? 1 : 0)))),
  fShadow((*(reinterpret_cast<UnsignedByte *>(&afShadow) + (ByteOrderMatch ? 0 : 1)))),
  bShadow((*(reinterpret_cast<UnsignedByte *>(&bcShadow) + (ByteOrderMatch ? 1 : 0)))),
  cShadow((*(reinterpret_cast<UnsignedByte *>(&bcShadow) + (ByteOrderMatch ? 0 : 1)))),
  dShadow((*(reinterpret_cast<UnsignedByte *>(&deShadow) + (ByteOrderMatch ? 1 : 0)))),
  eShadow((*(reinterpret_cast<UnsignedByte *>(&deShadow) + (ByteOrderMatch ? 0 : 1)))),
  hShadow((*(reinterpret_cast<UnsignedByte *>(&hlShadow) + (ByteOrderMatch ? 1 : 0)))),
  lShadow((*(reinterpret_cast<UnsignedByte *>(&hlShadow) + (ByteOrderMatch ? 0 : 1)))),
  pcH((*(reinterpret_cast<UnsignedByte *>(&pc) + (ByteOrderMatch ? 1 : 0)))),
  pcL((*(reinterpret_cast<UnsignedByte *>(&pc) + (ByteOrderMatch ? 0 : 1)))),
  spH((*(reinterpret_cast<UnsignedByte *>(&sp) + (ByteOrderMatch ? 1 : 0)))),
  spL((*(reinterpret_cast<UnsignedByte *>(&sp) + (ByteOrderMatch ? 0 : 1)))),
  memptrH((*(reinterpret_cast<UnsignedByte *>(&memptr) + (ByteOrderMatch ? 1 : 0)))),
  memptrL((*(reinterpret_cast<UnsignedByte *>(&memptr) + (ByteOrderMatch ? 0 : 1)))),
  afZ80{af},
  bcZ80{bc},
  deZ80{de},
  hlZ80{hl},
  ixZ80{ix},
  iyZ80{iy},
  pcZ80{pc},
  spZ80{sp},
  afShadowZ80{afShadow},
  bcShadowZ80{bcShadow},
  deShadowZ80{deShadow},
  hlShadowZ80{hlShadow},
  memptrZ80{memptr}
{}

void Registers::reset()
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

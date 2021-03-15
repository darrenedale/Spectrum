//
// Created by darren on 14/03/2021.
//

#include <iostream>

#include "z80snapshotwriter.h"

using namespace Spectrum;


bool Z80SnapshotWriter::writeTo(std::ostream & out) const
{
    // TODO this only writes a v1 file; support v2 and v3 file output
    auto & snap = snapshot();
    auto & registers = snap.registers();
    auto & memory = snap.memory();

    out.put(static_cast<std::ostream::char_type>(registers.a));
    out.put(static_cast<std::ostream::char_type>(registers.f));
    writeHostWord(out, registers.bc);
    writeHostWord(out, registers.hl);
    writeHostWord(out, registers.pc);
    writeHostWord(out, registers.sp);
    out.put(static_cast<std::ostream::char_type>(registers.i));
    out.put(static_cast<std::ostream::char_type>(registers.r));
    out.put(static_cast<std::ostream::char_type>(((registers.r & 0x80) >> 7) | ((static_cast<Z80::UnsignedByte>(snap.border) & 0x07) << 1) | /* TODO support compression (bit 5) */ 0x00));
    writeHostWord(out, registers.de);
    writeHostWord(out, registers.bcShadow);
    writeHostWord(out, registers.deShadow);
    writeHostWord(out, registers.hlShadow);
    out.put(static_cast<std::ostream::char_type>(registers.aShadow));
    out.put(static_cast<std::ostream::char_type>(registers.fShadow));
    writeHostWord(out, registers.ix);
    writeHostWord(out, registers.iy);
    out.put(static_cast<std::ostream::char_type>(snap.iff1 ? 0x01 : 0x00));
    out.put(static_cast<std::ostream::char_type>(snap.iff2 ? 0x01 : 0x00));
    out.put(static_cast<std::ostream::char_type>(snap.im) & 0x03);

    if (memory.image) {
        // only the RAM is written to .z80 files
        out.write(reinterpret_cast<std::ostream::char_type *>(memory.image + 0x4000), memory.size - 0x4000);
    }

    return !out.bad() && !out.fail();
}

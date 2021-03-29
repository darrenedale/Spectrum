//
// Created by darren on 15/03/2021.
//

#include "snasnapshotwriter.h"

using namespace Spectrum::Io;

bool SnaSnapshotWriter::writeTo(std::ostream & out) const
{
    auto & snap = snapshot();
    auto & registers = snap.registers();
    auto & memory = snap.memory();

    out.put(static_cast<std::ostream::char_type>(registers.i));
    writeHostWord(out, registers.hlShadow);
    writeHostWord(out, registers.deShadow);
    writeHostWord(out, registers.bcShadow);
    writeHostWord(out, registers.afShadow);
    writeHostWord(out, registers.hl);
    writeHostWord(out, registers.de);
    writeHostWord(out, registers.bc);
    writeHostWord(out, registers.iy);
    writeHostWord(out, registers.ix);
    out.put(static_cast<std::ostream::char_type>(snap.iff2 ? 0x02 : 0x00));
    out.put(static_cast<std::ostream::char_type>(registers.r));
    writeHostWord(out, registers.af);
    writeHostWord(out, registers.sp);
    out.put(static_cast<std::ostream::char_type>(snap.im));
    out.put(static_cast<std::ostream::char_type>(snap.border));

    if (memory.image) {
        // only the RAM is written to .sna files
        out.write(reinterpret_cast<std::ostream::char_type *>(memory.image + 0x4000), memory.size - 0x4000);
    }

    return !out.bad() && !out.fail();
}

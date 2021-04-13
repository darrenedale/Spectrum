//
// Created by darren on 15/03/2021.
//

#include "snasnapshotwriter.h"

using namespace Spectrum::Io;

bool SnaSnapshotWriter::writeTo(std::ostream & out) const
{
    auto & snap = snapshot();

    if (Model::Spectrum48k != snap.model()) {
        std::cerr << "Only Spectrum 48k snapshots are currently supported by the SNA file writer\n";
        return false;
    }

    auto & registers = snap.registers();
    auto * memory = snap.memory();

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

    if (memory) {
        // only the RAM is written to .sna files
        out.write(reinterpret_cast<const std::ostream::char_type *>(memory->pointerTo(0x4000)), static_cast<std::streamsize>(memory->availableSize() - 0x4000));
    }

    return !out.bad() && !out.fail();
}

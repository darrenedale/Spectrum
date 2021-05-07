//
// Created by darren on 15/03/2021.
//

#include "snasnapshotwriter.h"
#include "../../util/debug.h"

using namespace Spectrum::Io;

using ::Z80::UnsignedByte;
using ::Z80::UnsignedWord;
using ::Z80::hostToZ80ByteOrder;

namespace
{
    // all words are in Z80 byte order
    struct Header
    {
        UnsignedByte i;
        UnsignedWord hlShadow;
        UnsignedWord deShadow;
        UnsignedWord bcShadow;
        UnsignedWord afShadow;
        UnsignedWord hl;
        UnsignedWord de;
        UnsignedWord bc;
        UnsignedWord iy;
        UnsignedWord ix;
        UnsignedByte iff2;
        UnsignedByte r;
        UnsignedWord af;
        UnsignedWord sp;
        UnsignedByte im;
        UnsignedByte border;
    };
}

bool SnaSnapshotWriter::writeTo(std::ostream & out) const
{
    auto & snap = snapshot();
    auto * memory = snap.memory();

    if (!memory) {
        Util::debug << "Snapshot is incomplete (no memory)\n";
        return false;
    }

    if (Model::Spectrum48k != snap.model()) {
        Util::debug << "Only Spectrum 48k snapshots are currently supported by the SNA file writer\n";
        return false;
    }

    auto & registers = snap.registers();
    Header header {
        .i = registers.i,
        .hlShadow = hostToZ80ByteOrder(registers.hlShadow),
        .deShadow = hostToZ80ByteOrder(registers.deShadow),
        .bcShadow = hostToZ80ByteOrder(registers.bcShadow),
        .afShadow = hostToZ80ByteOrder(registers.afShadow),
        .hl = hostToZ80ByteOrder(registers.hl),
        .de = hostToZ80ByteOrder(registers.de),
        .bc = hostToZ80ByteOrder(registers.bc),
        .iy = hostToZ80ByteOrder(registers.iy),
        .ix = hostToZ80ByteOrder(registers.ix),
        .iff2 = static_cast<UnsignedByte>(snap.iff2 ? 0x02 : 0x00),
        .r = registers.r,
        .af = hostToZ80ByteOrder(registers.af),
        .sp = hostToZ80ByteOrder(registers.sp),
        .im = static_cast<UnsignedByte>(snap.im),
        .border = static_cast<UnsignedByte>(snap.border),
    };
    
    out.write(reinterpret_cast<std::ostream::char_type *>(&header), sizeof(Header));
    
    if (out.bad()) {
        Util::debug << "error writing .sna header\n";
        return false;
    }

    // NOTE we assume a standard Spectrum 16K/48K memory object with a single contiguous block of bytes
    out.write(reinterpret_cast<const std::ostream::char_type *>(memory->pointerTo(0x4000)), static_cast<std::streamsize>(memory->availableSize() - 0x4000));
    return !out.bad() && !out.fail();
}

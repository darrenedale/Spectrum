//
// Created by darren on 22/03/2021.
//

#include <iostream>
#include <cassert>

#include "snasnapshotreader.h"
#include "../types.h"
#include "../../z80/types.h"

using namespace Spectrum::Io;

using ::Z80::UnsignedByte;
using ::Z80::UnsignedWord;
using ::Z80::InterruptMode;

namespace
{
    #pragma pack(1) // the compiler must not pad this struct otherwise header won't be read from the stream correctly
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
        UnsignedByte iff;       // bit 2: 0 = DI, 1 = EI
        UnsignedByte r;
        UnsignedWord af;
        UnsignedWord sp;
        UnsignedByte im;        // only bits 0-2 are significant
        UnsignedByte border;    // only bits 0-3 are significant
    };

    constexpr const int MemoryImageOffset = 0x4000;
}

/**
 * The snapshot MUST have at least a 64kb memory image.
 *
 * @param snapshot
 * @return
 */
bool SnaSnapshotReader::readInto(Snapshot & snapshot) const
{
    assert(0xffff <= snapshot.memory().size);

    if (!isOpen()) {
        return false;
    }
    
    auto & in = inputStream();
    Header header;  // NOLINT(cppcoreguidelines-pro-type-member-init) used as memory buffer for stream read, no need
                    // to initialise
    
    in.read(reinterpret_cast<std::istream::char_type *>(&header), sizeof(Header));
    
    if (in.fail()) {
        std::cerr << "Failed to read SNA header from input stream.\n";
        return false;
    }

    auto & registers = snapshot.registers();
    registers.af = Z80::z80ToHostByteOrder(header.af);
    registers.bc = Z80::z80ToHostByteOrder(header.bc);
    registers.de = Z80::z80ToHostByteOrder(header.de);
    registers.hl = Z80::z80ToHostByteOrder(header.hl);
    registers.ix = Z80::z80ToHostByteOrder(header.ix);
    registers.iy = Z80::z80ToHostByteOrder(header.iy);

    registers.sp = Z80::z80ToHostByteOrder(header.sp);
    registers.pc = 0x0000;      // .sna snapshots have pc set by calling RETN (i.e. PC in on the stack)

    registers.afShadow = Z80::z80ToHostByteOrder(header.afShadow);
    registers.bcShadow = Z80::z80ToHostByteOrder(header.bcShadow);
    registers.deShadow = Z80::z80ToHostByteOrder(header.deShadow);
    registers.hlShadow = Z80::z80ToHostByteOrder(header.hlShadow);

    registers.i = header.i;
    registers.r = header.r;

    snapshot.iff1 = header.iff & 0x04;
    snapshot.iff2 = snapshot.iff1;
    snapshot.im = static_cast<InterruptMode>(header.im & 0x07);
    snapshot.border = static_cast<Colour>(header.border & 0x03);
    in.read(reinterpret_cast<std::istream::char_type *>(snapshot.memory().image + MemoryImageOffset), 0xc000);
    return true;
}

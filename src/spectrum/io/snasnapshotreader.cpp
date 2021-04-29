//
// Created by darren on 22/03/2021.
//

#include <filesystem>
#include "snasnapshotreader.h"
#include "../spectrum48k.h"
#include "../../util/debug.h"

using namespace Spectrum::Io;
using ::Z80::UnsignedByte;
using ::Z80::UnsignedWord;
using ::Z80::InterruptMode;
using ::Z80::z80ToHostByteOrder;
using ::Z80::hostToZ80ByteOrder;

namespace
{
#pragma pack(push, 1)
    // the compiler must not pad this struct otherwise header won't be read from the stream correctly
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
#pragma pack(pop)

    /**
     * First byte of the Spectrum memory image that the snapshot stores.
     */
    constexpr const int MemoryImageOffset = 0x4000;
}

/**
 * The snapshot MUST have at least a 64kb memory image.
 *
 * @param snapshot
 * @return
 */
const Spectrum::Snapshot * SnaSnapshotReader::read() const
{
    if (!isOpen()) {
        return nullptr;
    }
    
    auto & in = inputStream();
    Header header;  // NOLINT(cppcoreguidelines-pro-type-member-init) used as memory buffer for stream read, no need
                    // to initialise
    
    in.read(reinterpret_cast<std::istream::char_type *>(&header), sizeof(Header));
    
    if (in.fail()) {
        Util::debug << "Failed to read SNA header from input stream.\n";
        return nullptr;
    }

    auto snapshot = std::make_unique<Snapshot>(Model::Spectrum48k);
    auto & registers = snapshot->registers();
    registers.af = z80ToHostByteOrder(header.af);
    registers.bc = z80ToHostByteOrder(header.bc);
    registers.de = z80ToHostByteOrder(header.de);
    registers.hl = z80ToHostByteOrder(header.hl);
    registers.ix = z80ToHostByteOrder(header.ix);
    registers.iy = z80ToHostByteOrder(header.iy);

    registers.sp = z80ToHostByteOrder(header.sp);
    registers.pc = 0x0000;      // .sna snapshots have pc set by calling RETN (i.e. PC in on the stack)

    registers.afShadow = z80ToHostByteOrder(header.afShadow);
    registers.bcShadow = z80ToHostByteOrder(header.bcShadow);
    registers.deShadow = z80ToHostByteOrder(header.deShadow);
    registers.hlShadow = z80ToHostByteOrder(header.hlShadow);

    registers.i = header.i;
    registers.r = header.r;

    snapshot->iff1 = header.iff & 0x04;
    snapshot->iff2 = snapshot->iff1;
    snapshot->im = static_cast<InterruptMode>(header.im & 0x07);
    snapshot->border = static_cast<Colour>(header.border & 0x03);

    auto memory = std::make_unique<Spectrum48k::MemoryType>();
    in.read(reinterpret_cast<std::istream::char_type *>(memory->pointerTo(0) + MemoryImageOffset), 0xc000);
    snapshot->setMemory(std::move(memory));
    setSnapshot(std::move(snapshot));
    return this->snapshot();
}

bool SnaSnapshotReader::couldBeSnapshot(std::istream & in)
{
    if (!in) {
        Util::debug << "stream is not open.\n";
        return false;
    }

    // byte 25 in file is interrupt mode, must be 1, 2 or 3
    in.seekg(25, std::ios_base::beg);
    auto byteValue = in.get();

    if (in.fail() || 1 > byteValue || 3 < byteValue) {
        return false;
    }

    // byte 26 in file is border colour, must be 0 .. 7
    in.seekg(26, std::ios_base::beg);
    byteValue = in.get();

    if (in.fail() || 0 > byteValue || 7 < byteValue) {
        return false;
    }

    return true;
}

bool SnaSnapshotReader::couldBeSnapshot(const std::string & fileName)
{
    if (49179 != std::filesystem::file_size(fileName)) {
        Util::debug << ".zx snapshots are always 49179 bytes in size.\n";
        return false;
    }

    auto in = std::ifstream(fileName);
    return couldBeSnapshot(in);
}

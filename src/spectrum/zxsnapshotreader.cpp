//
// Created by darren on 22/03/2021.
//

#include <iostream>
#include <cstring>
#include <bit>

#include "zxsnapshotreader.h"
#include "../z80/types.h"

using namespace Spectrum;

using UnsignedByte = ::Z80::UnsignedByte;
using InterruptMode = ::Z80::InterruptMode;

namespace
{
    struct Settings
    {
        std::uint16_t setting1;     // apparently always 0x000a
        std::uint16_t setting2;     // apparently always 0x000a
        std::uint16_t setting3;     // apparently always 0x0004
        std::uint16_t setting4;     // apparently always 0x0001
        std::uint16_t setting5;     // apparently always 0x0001
    };

    // there's a fair amount of padding here. it's likely just an unmodified dump of the internal data structure used in
    // the originating emulator to store the registers, etc. (e.g. PC and SP were represented using 32-bit ints)
    //
    // words are typed as uint16_t rather than UnsignedWord to make it clear that they're not in Z80 byte order
    struct ZXFileContent
    {
        UnsignedByte memory[49284];     // starting from 16252 (0x3f7c - part way through character data in ROM...)
        UnsignedByte unused1[132];      // apparently always all 0x00
        Settings settings;
        UnsignedByte interruptStatus;   // 0 = DI, 1 = EI
        UnsignedByte unused2[2];        // apparently always 0x00 0x03
        UnsignedByte colourMode;        // 0 = b&w, 1 = colour
        std::uint32_t unused3;          // apparently always 0x00000000
        std::uint16_t bc;               // in BE format
        std::uint16_t bcShadow;         // in BE format
        std::uint16_t de;               // in BE format
        std::uint16_t deShadow;         // in BE format
        std::uint16_t hl;               // in BE format
        std::uint16_t hlShadow;         // in BE format
        std::uint16_t ix;               // in BE format
        std::uint16_t iy;               // in BE format
        UnsignedByte i;
        UnsignedByte r;
        std::uint16_t unused4;          // apparently always 0x0000
        UnsignedByte unused5;           // apparently always 0x00
        UnsignedByte a;
        UnsignedByte unused6;           // apparently always 0x00
        UnsignedByte f;
        UnsignedByte unused7;           // apparently always 0x00
        UnsignedByte aShadow;
        UnsignedByte unused8;           // apparently always 0x00
        UnsignedByte fShadow;
        std::uint16_t unused9;          // apparently always 0x0000
        std::uint16_t pc;               // apparently always 0x0000
        std::uint16_t unused10;         // apparently always 0x0000
        std::uint16_t sp;               // apparently always 0x0000
        std::uint16_t soundMode;        // 0 = simple, 1 = pitch, 2 = ROMOnly ...
        std::uint16_t halt;             // 0 = running, 1 = halted ...
        std::uint16_t interruptMode;    // 0xffff = IM0, 0 = IM1, 1 = IM2
        UnsignedByte unused11[10];      // apparently always all 0x00
    };

    constexpr const std::uint16_t MemoryImageOffset = 16252;
}

bool ZXSnapshotReader::readInto(Snapshot & snapshot) const
{
    if (!isOpen()) {
        std::cerr << "Input stream is not open.\n";
        return false;
    }

    auto & in = inputStream();
    ZXFileContent content; // NOLINT(cppcoreguidelines-pro-type-member-init) no need to init because it's effectively
                           // used as a memory buffer and is not utilised unless confirmed fully-populated
    in.read(reinterpret_cast<std::istream::char_type *>(&content), sizeof(ZXFileContent));

    if (in.fail() || sizeof(ZXFileContent) != in.gcount()) {
        std::cerr << "Failed to read from input stream.\n";
        return false;
    }

    auto & registers = snapshot.registers();
    registers.a = content.a;
    registers.f = content.f;
    registers.aShadow = content.aShadow;
    registers.fShadow = content.fShadow;
    registers.i = content.i;
    registers.r = content.r;
    snapshot.iff1 = (1 == content.interruptStatus);
    snapshot.iff2 = snapshot.iff1;

    if (std::endian::native == std::endian::big) {
        registers.bc = content.bc;
        registers.de = content.de;
        registers.hl = content.hl;
        registers.ix = content.ix;
        registers.iy = content.iy;
        registers.bcShadow = content.bcShadow;
        registers.deShadow = content.deShadow;
        registers.hlShadow = content.hlShadow;

        registers.pc = content.pc;
        registers.sp = content.sp;
    } else {
        registers.bc = Z80::swapByteOrder(content.bc);
        registers.de = Z80::swapByteOrder(content.de);
        registers.hl = Z80::swapByteOrder(content.hl);
        registers.ix = Z80::swapByteOrder(content.ix);
        registers.iy = Z80::swapByteOrder(content.iy);
        registers.bcShadow = Z80::swapByteOrder(content.bcShadow);
        registers.deShadow = Z80::swapByteOrder(content.deShadow);
        registers.hlShadow = Z80::swapByteOrder(content.hlShadow);

        registers.pc = Z80::swapByteOrder(content.pc);
        registers.sp = Z80::swapByteOrder(content.sp);
    }
    
    switch (content.interruptMode) {
        case 0xffff:
            snapshot.im = InterruptMode::IM0;
            break;

        case 0:
            snapshot.im = InterruptMode::IM1;
            break;

        case 1:
            snapshot.im = InterruptMode::IM2;
            break;

        default:
            std::cerr << "Invalid interrupt mode in .ZX input stream.\n";
            return false;
    }

    std::memcpy(snapshot.memory().image + MemoryImageOffset, content.memory, sizeof(content.memory));
    return true;
}

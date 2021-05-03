//
// Created by darren on 22/03/2021.
//

#include <cstring>
#include <bit>
#include <filesystem>
#include "zxsnapshotreader.h"
#include "../spectrum48k.h"
#include "../../util/debug.h"
#include "../../util/endian.h"

using namespace Spectrum::Io;

using UnsignedByte = ::Z80::UnsignedByte;
using InterruptMode = ::Z80::InterruptMode;
using Spectrum::Snapshot;

namespace
{
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
    // required for format compatibility but unused
    struct Settings
    {
        std::uint16_t setting1;     // apparently always 0x000a
        std::uint16_t setting2;     // apparently always 0x000a
        std::uint16_t setting3;     // apparently always 0x0004
        std::uint16_t setting4;     // apparently always 0x0001
        std::uint16_t setting5;     // apparently always 0x0001
    };
#pragma clang diagnostic pop

    // Updated from http://spectrum-zx.chat.ru/faq/fileform.html
    //    Offset   Size   Description
    //    ------------------------------------------------------------------------
    //    0        132    bytes  132 bytes of 0 padding
    //    132      49152  bytes  RAM dump 0x4000..0xffff
    //    49284    132    bytes  132 bytes of 0 padding
    //    49416    10     word   10,10,4,1,1 (different settings)
    //    49426    1      byte   InterruptStatus (0=DI/1=EI)
    //    49427    2      byte   0,3
    //    49429    1      byte   ColorMode (0=BW/1=Color)
    //    49430    4      long   0
    //    49434    16     word   BC,BC',DE,DE',HL,HL',IX,IY
    //    49450    2      byte   I,R
    //    49452    2      word   0
    //    49454    8      byte   0,A',0,A,0,F',0,F
    //    49462    8      word   0,PC,0,SP
    //    49470    2      word   SoundMode (0=Simple/1=Pitch/2=RomOnly)
    //    49472    2      word   HaltMode  (0=NoHalt/1=Halt)
    //    49474    2      word   IntMode   (-1=IM0/0=IM1/1=IM2)
    //    49476    10     bytes  unused, make 0
    //    ------------------------------------------------------------------------
    //
    // there's a fair amount of padding here. it's likely just an unmodified dump of the internal data structure used in
    // the originating emulator to store the registers, etc. (e.g. PC and SP were represented using 32-bit ints)
    //
    // words are typed as uint16_t rather than UnsignedWord to make it clear that they're not in Z80 byte order
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
    // several members are required for format compatibility but are unused
    struct ZxFileContent
    {
        UnsignedByte unused0[132];      // apparently always all 0x00
        UnsignedByte memory[0xc000];
        UnsignedByte unused1[132];      // apparently always all 0x00
        Settings settings;
        UnsignedByte interruptStatus;   // 0 = DI, 1 = EI
        UnsignedByte unused2[2];        // apparently always 0x00 0x03 - could this be border colour? last OUT to ULA?
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
        std::uint16_t pc;               // in BE format
        std::uint16_t unused10;         // apparently always 0x0000
        std::uint16_t sp;               // in BE format
        std::uint16_t soundMode;        // 0 = simple, 1 = pitch, 2 = ROMOnly ...
        std::uint16_t halt;             // 0 = running, 1 = halted ...
        std::uint16_t interruptMode;    // 0xffff = IM0, 0 = IM1, 1 = IM2
        UnsignedByte unused11[10];      // apparently always all 0x00
    };
#pragma clang diagnostic pop

    constexpr const std::uint16_t MemoryImageOffset = 0x4000;

    using Util::swapByteOrder;
}

const Snapshot * ZxSnapshotReader::read() const
{
    if (!isOpen()) {
        Util::debug << "Input stream is not open.\n";
        return nullptr;
    }

    auto & in = inputStream();
    ZxFileContent content; // NOLINT(cppcoreguidelines-pro-type-member-init) no need to init because it's effectively
                           // used as a memory buffer and is not utilised unless confirmed fully-populated
    in.read(reinterpret_cast<std::istream::char_type *>(&content), sizeof(ZxFileContent));

    if (in.fail() || sizeof(ZxFileContent) != in.gcount()) {
        Util::debug << "Failed to read from input stream.\n";
        return nullptr;
    }

    auto snapshot = std::make_unique<Snapshot>(Model::Spectrum48k);
    auto & registers = snapshot->registers();
    registers.a = content.a;
    registers.f = content.f;
    registers.aShadow = content.aShadow;
    registers.fShadow = content.fShadow;
    registers.i = content.i;
    registers.r = content.r;
    snapshot->iff1 = (1 == content.interruptStatus);
    snapshot->iff2 = snapshot->iff1;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
#pragma ide diagnostic ignored "UnreachableCode"
    // NOTE one of these two branches will be diagnosed as unnecessary, depending on the byte order of the host, but the
    // code is required for cross-platform compatibility
    if constexpr (std::endian::native == std::endian::big) {
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
        registers.bc = swapByteOrder(content.bc);
        registers.de = swapByteOrder(content.de);
        registers.hl = swapByteOrder(content.hl);
        registers.ix = swapByteOrder(content.ix);
        registers.iy = swapByteOrder(content.iy);
        registers.bcShadow = swapByteOrder(content.bcShadow);
        registers.deShadow = swapByteOrder(content.deShadow);
        registers.hlShadow = swapByteOrder(content.hlShadow);

        registers.pc = swapByteOrder(content.pc);
        registers.sp = swapByteOrder(content.sp);
    }
#pragma clang diagnostic pop

    switch (content.interruptMode) {
        case 0xffff:
            snapshot->im = InterruptMode::IM0;
            break;

        case 0:
            snapshot->im = InterruptMode::IM1;
            break;

        case 1:
            snapshot->im = InterruptMode::IM2;
            break;

        default:
            Util::debug << "Invalid interrupt mode in .ZX input stream.\n";
            return nullptr;
    }

    auto memory = std::make_unique<Spectrum48k::MemoryType>(0x10000);
    // we know that 48K memory is a contiguous block of 64k bytes
    std::memcpy(memory->pointerTo(MemoryImageOffset), content.memory, sizeof(content.memory));
    snapshot->setMemory(std::move(memory));
    setSnapshot(std::move(snapshot));
    return this->snapshot();
}

bool ZxSnapshotReader::couldBeSnapshot(std::istream & in)
{
    if (!in) {
        Util::debug << "stream is not open.\n";
        return false;
    }

    // byte 49426 must be 0 for DI, 1 for EI
    in.seekg(49426, std::ios_base::beg);
    std::istream::char_type byteValue;
    in.get(byteValue);

    if (in.fail() || 0 != byteValue || 1 != byteValue) {
        return false;
    }

    // byte 49429 must be 0 for B/W, 1 for colour
    in.seekg(49429, std::ios_base::beg);

    if (in.fail() || 0 != byteValue || 1 != byteValue) {
        return false;
    }

    // BE word at 49474 must be -1 for IM0, 0 for IM1, 1 for IM2
    in.seekg(49474, std::ios_base::beg);
    std::int16_t wordValue;
    in.read(reinterpret_cast<std::istream::char_type *>(&wordValue), sizeof(wordValue));

    if (in.fail()) {
        return false;
    }

    if constexpr (std::endian::native != std::endian::big) {
        wordValue = swapByteOrder(wordValue);
    }

    if (-1 > wordValue || 1 < wordValue) {
        return false;
    }

    return true;
}

bool ZxSnapshotReader::couldBeSnapshot(const std::string & fileName)
{
    if (49486 != std::filesystem::file_size(fileName)) {
        Util::debug << ".zx snapshots are always 49486 bytes in size.\n";
        return false;
    }

    auto in = std::ifstream(fileName);
    return couldBeSnapshot(in);
}

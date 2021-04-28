//
// Created by darren on 27/04/2021.
//

#include "zxsnapshotwriter.h"
#include "../../util/debug.h"

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
    struct Header
    {
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
        std::uint16_t pc;               // apparently always 0x0000
        std::uint16_t unused10;         // apparently always 0x0000
        std::uint16_t sp;               // apparently always 0x0000
        std::uint16_t soundMode;        // 0 = simple, 1 = pitch, 2 = ROMOnly ...
        std::uint16_t halt;             // 0 = running, 1 = halted ...
        std::uint16_t interruptMode;    // 0xffff = IM0, 0 = IM1, 1 = IM2
        UnsignedByte unused11[10];      // apparently always all 0x00
    };
#pragma clang diagnostic pop

    constexpr const std::uint16_t MemoryImageOffset = 0x4000;

    using Util::swapByteOrder;
}

bool ZxSnapshotWriter::writeTo(std::ostream & out) const
{
    const auto & snap = snapshot();

    if (Model::Spectrum48k != snap.model()) {
        Util::debug << "Only Spectrum 48k snapshots are currently supported by the ZX82 file writer\n";
        return false;
    }

    const auto * memory = snap.memory();

    if (!memory) {
        Util::debug << "Snapshot is incomplete (no memory)\n";
        return false;
    }

    std::array<std::ostream::char_type, 132> padding132Bytes = {};
    padding132Bytes.fill(0x00);
    out.write(padding132Bytes.data(), padding132Bytes.size());

    if (out.bad()) {
        Util::debug << "Error writing 132 bytes of 0-padding to snapshot stream.\n";
        return false;
    }

    out.write(reinterpret_cast<const std::ostream::char_type *>(memory->pointerTo(MemoryImageOffset)), 0x10000 - MemoryImageOffset);
    
    if (out.bad()) {
        Util::debug << "Error writing memory image to snapshot stream.\n";
        return false;
    }

    out.write(padding132Bytes.data(), padding132Bytes.size());

    if (out.bad()) {
        Util::debug << "Error writing 132 bytes of 0-padding to snapshot stream.\n";
        return false;
    }

    const auto & registers = snap.registers();

    Header header = {
        .settings =  {
            .setting1 = 0x000a,
            .setting2 = 0x000a,
            .setting3 = 0x0004,
            .setting4 = 0x0001,
            .setting5 = 0x0001,
        },
        .interruptStatus = (snap.iff1 ? static_cast<UnsignedByte>(0x01) : static_cast<UnsignedByte>(0x00)),
        .unused2 = {0x00, 0x03},
        .colourMode = 0x01,
        .unused3 = 0,           // apparently always 0x00000000
        .bc = registers.bc,
        .bcShadow = registers.bcShadow,
        .de = registers.de,
        .deShadow = registers.deShadow,
        .hl = registers.hl,
        .hlShadow = registers.hlShadow,
        .ix = registers.ix,
        .iy = registers.iy,
        .i = registers.i,
        .r = registers.r,
        .unused4 = 0,
        .unused5 = 0,
        .a = registers.a,
        .unused6 = 0,
        .f = registers.f,
        .unused7 = 0,
        .aShadow = registers.aShadow,
        .unused8 = 0,
        .fShadow = registers.fShadow,
        .unused9 = 0,
        .pc = registers.pc,
        .unused10 = 0,
        .sp = registers.sp,
        .soundMode = 0,
        .halt = 0,
        // NOTE interrupt mode is set in switch below
        .unused11 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
    };

    //    49474    2      word   IntMode   (-1=IM0/0=IM1/1=IM2)
    switch (snap.im) {
        case InterruptMode::IM0:
            header.interruptMode = -1;
            break;

        case InterruptMode::IM1:
            header.interruptMode = 0;
            break;

        case InterruptMode::IM2:
            header.interruptMode = 1;
            break;
    }

    if (std::endian::native != std::endian::big) {
        header.bc = swapByteOrder(header.bc);
        header.de = swapByteOrder(header.de);
        header.hl = swapByteOrder(header.hl);
        header.ix = swapByteOrder(header.ix);
        header.iy = swapByteOrder(header.iy);
        header.pc = swapByteOrder(header.pc);
        header.sp = swapByteOrder(header.sp);
        header.bcShadow = swapByteOrder(header.bcShadow);
        header.deShadow = swapByteOrder(header.deShadow);
        header.hlShadow = swapByteOrder(header.hlShadow);

        header.settings.setting1 = swapByteOrder(header.settings.setting1);
        header.settings.setting2 = swapByteOrder(header.settings.setting2);
        header.settings.setting3 = swapByteOrder(header.settings.setting3);
        header.settings.setting4 = swapByteOrder(header.settings.setting4);
        header.settings.setting5 = swapByteOrder(header.settings.setting5);

        header.interruptMode = swapByteOrder(header.interruptMode);
    }

    out.write(reinterpret_cast<const std::ostream::char_type *>(&header), sizeof(Header));
    return !out.bad() && !out.fail();
}

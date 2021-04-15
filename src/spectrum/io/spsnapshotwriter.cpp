//
// Created by darren on 14/04/2021.
//

#include "spsnapshotwriter.h"

using namespace Spectrum::Io;

using ::Z80::UnsignedByte;
using ::Z80::UnsignedWord;
using ::Z80::InterruptMode ;
using ::Z80::hostToZ80ByteOrder;

namespace
{
    // from http://spectrum-zx.chat.ru/faq/fileform.html
    //    Offset   Size   Description
    //    ------------------------------------------------------------------------
    //    0        2      byte   "SP" (signature)
    //    2        2      word   Program length in bytes (49152 bytes)
    //    4        2      word   Program location (16384)
    //    6        8      word   BC,DE,HL,AF
    //    14       4      word   IX,IY
    //    18       8      word   BC',DE',HL',AF'
    //    26       2      byte   R,I
    //    28       4      word   SP,PC
    //    32       2      word   0 (reserved for future use)
    //    34       1      byte   Border color
    //    35       1      byte   0 (reserved for future use)
    //    36       2      word   Status word
    //    ------------------------------------------------------------------------
    //
    // NOTE every (used) member starts on a 16-bit aligned byte so no need to pack the struct
    //
    // all words are in Z80 byte order
    struct Header
    {
        char signature[2];
        UnsignedWord length;        // # of bytes of program data (always 48k)
        UnsignedWord baseAddress;   // base address for program data

        struct
        {
            UnsignedWord bc;
            UnsignedWord de;
            UnsignedWord hl;
            UnsignedWord af;
            UnsignedWord ix;
            UnsignedWord iy;
        } registers;                // the registers, Z80 byte order

        struct
        {
            UnsignedWord bc;
            UnsignedWord de;
            UnsignedWord hl;
            UnsignedWord af;
        } shadowRegisters;          // the shadow registers, Z80 byte order

        struct {
            UnsignedByte r;
            UnsignedByte i;
        } interruptRegisters;       // the I and R registers

        UnsignedWord sp;            // the stack pointer, Z80 byte order
        UnsignedWord pc;            // the program counter, Z80 byte order
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
        // reserved by the format for future use and currently unused by this code
        UnsignedWord reserved1;
#pragma clang diagnostic pop
        UnsignedByte border;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
        // reserved by the format for future use and currently unused by this code
        UnsignedByte reserved2;
#pragma clang diagnostic pop

        //   Bit     Description
        //   ------------------------------------------------------------------------
        //   15-8    Reserved for future use
        //    7-6    Reserved for internal use (0)
        //      5    Flash: 0=INK/1=PAPER
        //      4    Interrupt pending for execution
        //      3    Reserved for future use
        //      2    IFF2 (internal use)
        //      1    Interrupt Mode: 0=IM1/1=IM2
        //      0    IFF1: 0=DI/1=EI
        UnsignedWord status;
    };
}

bool SpSnapshotWriter::writeTo(std::ostream & out) const
{
    auto & snap = snapshot();
    auto * memory = snap.memory();

    if (!memory) {
        std::cerr << "Snapshot is incomplete (no memory)\n";
        return false;
    }

    if (Model::Spectrum48k != snap.model()) {
        std::cerr << "Only Spectrum 48k snapshots are currently supported by the SP file writer\n";
        return false;
    }

    auto & registers = snap.registers();
    Header header {
        .signature = {'S', 'P'},
        .length = hostToZ80ByteOrder(0xc000),
        .baseAddress = hostToZ80ByteOrder(0x4000),
        .registers = {
            .bc = hostToZ80ByteOrder(registers.bc),
            .de = hostToZ80ByteOrder(registers.de),
            .hl = hostToZ80ByteOrder(registers.hl),
            .af = hostToZ80ByteOrder(registers.af),
            .ix = hostToZ80ByteOrder(registers.ix),
            .iy = hostToZ80ByteOrder(registers.iy),
        },
        .shadowRegisters = {
            .bc = hostToZ80ByteOrder(registers.bcShadow),
            .de = hostToZ80ByteOrder(registers.deShadow),
            .hl = hostToZ80ByteOrder(registers.hlShadow),
            .af = hostToZ80ByteOrder(registers.afShadow),
        },
        .interruptRegisters = {
            .r = registers.r,
            .i = registers.i,
        },
        .sp = hostToZ80ByteOrder(registers.sp),
        .pc = hostToZ80ByteOrder(registers.pc),
        .reserved1 = 0,
        .border = static_cast<UnsignedByte>(snap.border),
        .reserved2 = 0,
        .status = hostToZ80ByteOrder(0x0000
                | (snap.iff1 ? 0x01 : 0x00)
                | (InterruptMode::IM2 == snap.im ? 0x02 : 0x00)
                | (snap.iff2 ? 0x04 : 0x00)),
    };

    out.write(reinterpret_cast<std::ostream::char_type *>(&header), sizeof(Header));

    if (out.bad() || out.fail()) {
        std::cerr << "error writing .sp header\n";
        return false;
    }

    out.write(reinterpret_cast<const std::ostream::char_type *>(memory->pointerTo(0x4000)), static_cast<std::streamsize>(memory->availableSize() - 0x4000));
    return !out.bad() && !out.fail();
}

//
// Created by darren on 22/03/2021.
//

#include <iostream>
#include <iomanip>
#include <iterator>
#include <cstring>

#include "spsnapshotreader.h"
#include "../types.h"
#include "../../z80/types.h"

using namespace Spectrum::Io;

using ::Z80::UnsignedByte;
using ::Z80::UnsignedWord;
using ::Z80::InterruptMode;
using ::Z80::z80ToHostByteOrder;

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
    // NOTE every (used) member starts on a 16-bit aligned byte so no need to pack the struct, I think
    struct Header
    {
        char signature[2];          // always "SP"
        UnsignedWord length;        // # of bytes of program data
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
        UnsignedWord reserved1;
        UnsignedByte border;
        UnsignedByte reserved2;

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

bool SpSnapshotReader::readInto(Snapshot & snapshot) const
{
    if (!isOpen()) {
        std::cerr << "Input stream is not open.\n";
        return false;
    }

    auto & in = inputStream();

    // read file
    Header header = {};
    std::ifstream::char_type * programBuffer;

    // read header
    in.read(reinterpret_cast<std::ifstream::char_type *>(&header), sizeof(header));

    if (in.fail()) {
        std::cerr << "Error reading SP header from stream\n";
        return false;
    }

    // validate header
    if (*reinterpret_cast<const std::uint16_t *>("SP") !=
        *reinterpret_cast<const std::uint16_t *>(header.signature)) {
        std::cerr << "Not an SP file.";
        return false;
    }

    // NOTE from here on, the length and baseAddress members of the header are in host byte order
    header.length = z80ToHostByteOrder(header.length);
    header.baseAddress = z80ToHostByteOrder(header.baseAddress);

    if (0x0000ffff < static_cast<int>(header.baseAddress) + header.length - 1) {
#if (!defined(ndebug))
        std::cerr << std::hex << std::setfill('0');
        std::cerr << "Program extends beyond upper bounds of RAM (0x" << std::setw(4) << header.baseAddress << " + "
                  << std::dec << header.length << ") > 0xffff\n";
        std::cerr << "Base address: 0x" << std::hex << std::setw(4) << header.baseAddress << "\n";
        std::cerr << "Length      : " << std::dec << header.length << " bytes\n";
        std::cerr << "End address : 0x" << std::hex << std::setw(5)
                  << (static_cast<std::uint32_t>(header.baseAddress) + header.length) << "\n";
        std::cerr << "Program extends beyond upper bounds of RAM (0x" << std::setw(4) << header.baseAddress << " + "
                  << std::dec << header.length << ") > 0xffff\n";
        std::cerr << std::setfill(' ');
#endif
        return false;
    }

    header.border = header.border & 0x07;

    // set state
    auto & registers = snapshot.registers();

    registers.bc = z80ToHostByteOrder(header.registers.bc);
    registers.de = z80ToHostByteOrder(header.registers.de);
    registers.hl = z80ToHostByteOrder(header.registers.hl);
    registers.af = z80ToHostByteOrder(header.registers.af);
    registers.ix = z80ToHostByteOrder(header.registers.ix);
    registers.iy = z80ToHostByteOrder(header.registers.iy);

    registers.bcShadow = z80ToHostByteOrder(header.shadowRegisters.bc);
    registers.deShadow = z80ToHostByteOrder(header.shadowRegisters.de);
    registers.hlShadow = z80ToHostByteOrder(header.shadowRegisters.hl);
    registers.afShadow = z80ToHostByteOrder(header.shadowRegisters.af);

    registers.r = header.interruptRegisters.r;
    registers.i = header.interruptRegisters.i;

    registers.sp = z80ToHostByteOrder(header.sp);
    registers.pc = z80ToHostByteOrder(header.pc);

    snapshot.border = static_cast<Colour>(header.border);

    // NOTE from here on, the status member of the header is in host byte order
    header.status = z80ToHostByteOrder(header.status);
    // bit 0 = IFF1
    snapshot.iff1 = header.status & 0x0001;
    // bit 2 = IFF2
    snapshot.iff2 = header.status & 0x0004;

    // bit 1 = IM
    snapshot.im = (header.status & 0x0002 ? ::InterruptMode::IM2 : ::InterruptMode::IM1);

    // bit 4 = interrupt pending
//    if (header.status & 0x0010) {
//        cpu.interrupt();
//    }

    // NOTE bit 5 of status indicates flash state but we don't use this

    in.read(reinterpret_cast<std::istream::char_type *>(snapshot.memory().image + header.baseAddress), header.length);

    if (in.fail()) {
        std::cerr << "Error reading program from stream\n";
        return false;
    }

    if (!in.eof()) {
        std::cerr << "Warning: ignored extraneous content at end of stream (from byte " << in.tellg() << " onward).\n";
    }

    return true;
}
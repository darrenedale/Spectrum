//
// Created by darren on 22/03/2021.
//

#include <iostream>
#include <iomanip>
#include <iterator>
#include <cstring>

#include "spsnapshotreader.h"
#include "types.h"
#include "../z80/types.h"

using namespace Spectrum;

using UnsignedByte = ::Z80::UnsignedByte;
using UnsignedWord = ::Z80::UnsignedWord;
using InterruptMode = ::Z80::InterruptMode;

namespace
{
    // NOTE every member starts on a 16-bit aligned byte so no need to pack the struct, I think
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
    header.length = Z80::z80ToHostByteOrder(header.length);
    header.baseAddress = Z80::z80ToHostByteOrder(header.baseAddress);

    if (0x0000ffff < static_cast<int>(header.baseAddress) + header.length - 1) {
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
        return false;
    }

    header.border = header.border & 0x07;

    // set state
    auto & registers = snapshot.registers();

    registers.bc = Z80::z80ToHostByteOrder(header.registers.bc);
    registers.de = Z80::z80ToHostByteOrder(header.registers.de);
    registers.hl = Z80::z80ToHostByteOrder(header.registers.hl);
    registers.af = Z80::z80ToHostByteOrder(header.registers.af);
    registers.ix = Z80::z80ToHostByteOrder(header.registers.ix);
    registers.iy = Z80::z80ToHostByteOrder(header.registers.iy);

    registers.bcShadow = Z80::z80ToHostByteOrder(header.shadowRegisters.bc);
    registers.deShadow = Z80::z80ToHostByteOrder(header.shadowRegisters.de);
    registers.hlShadow = Z80::z80ToHostByteOrder(header.shadowRegisters.hl);
    registers.afShadow = Z80::z80ToHostByteOrder(header.shadowRegisters.af);

    registers.r = header.interruptRegisters.r;
    registers.i = header.interruptRegisters.i;

    registers.sp = Z80::z80ToHostByteOrder(header.sp);
    registers.pc = Z80::z80ToHostByteOrder(header.pc);

    snapshot.border = static_cast<Colour>(header.border);

    // NOTE from here on, the status member of the header is in host byte order
    header.status = Z80::z80ToHostByteOrder(header.status);
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

//
// Created by darren on 08/04/2021.
//

#include <cstdint>
#include <iostream>
#include <iomanip>
#include "zx82snapshotreader.h"

using namespace Spectrum::Io;
using ::Z80::UnsignedByte;
using ::Z80::InterruptMode;

namespace
{
    // where the memory image read from the stream gets stored in the snapshot's memory image
    constexpr const std::uint16_t MemoryImageOffset = 16384;

    // the identifier required at the start of the header
    const std::uint32_t Identifier = *reinterpret_cast<const std::uint32_t *>("ZX82");

    enum class Type : std::uint8_t
    {
        BasicProgram = 0x00,
        NumericArray = 0x01,
        StringArray = 0x02,
        Code = 0x03,
        Snapshot = 0x04,
    };

    enum class CompressionType : std::uint8_t
    {
        None = 0x00,
        RunLength = 0xff,
    };

    using RegisterPair = union {
        std::uint16_t word;
        struct {
            std::uint8_t highByte;
            std::uint8_t lowByte;
        };
    };

    // NOTE all 16-bit values are big endian (MC68000 byte order)
#pragma pack(push, 1)
    struct Header
    {
        std::uint32_t identifier;
        Type type;
        CompressionType compressionType;
        std::uint16_t fileLength;
        std::uint16_t startAddress;
        std::uint16_t arrayName;
    };
#pragma pack(pop)

    // NOTE all register paris are big endian (MC68000 byte order)
#pragma pack(push, 1)
    struct SnapshotHeader
    : public Header
    {
        std::uint8_t borderColour;
        InterruptMode interruptMode;
        RegisterPair iy;
        RegisterPair ix;
        RegisterPair de;
        RegisterPair bc;
        RegisterPair hl;
        RegisterPair af;
        RegisterPair deShadow;
        RegisterPair bcShadow;
        RegisterPair hlShadow;
        RegisterPair afShadow;
        RegisterPair sp;
        std::uint8_t i;
        std::uint8_t unused;
        std::uint8_t r;
        std::uint8_t iff1;
        RegisterPair pc;
    };
#pragma pack(pop)

    // used to convert 16-bit values from MC68000 byte order to host-native byte order if required
    inline std::uint16_t swapByteOrder(std::uint16_t value)
    {
        return ((value & 0x00ff) << 8) | ((value & 0xff00) >> 8);
    }
}

bool ZX82SnapshotReader::readInto(Snapshot & snapshot) const
{
    if (!isOpen()) {
        std::cerr << "Input stream is not open.\n";
        return false;
    }

    auto & in = inputStream();
    SnapshotHeader header; // NOLINT(cppcoreguidelines-pro-type-member-init)
    in.read(reinterpret_cast<std::istream::char_type *>(&header), sizeof(SnapshotHeader));

    if (in.gcount() != sizeof(SnapshotHeader)) {
        std::cerr << "The stream is not a valid ZX82 stream: failed to read complete header.\n";
        return false;
    }

    if (header.identifier != Identifier) {
        std::cerr << "The stream is not a valid ZX82 stream: incorrect file signature.\n";
        return false;
    }
    
    if (header.type != Type::Snapshot) {
        std::cerr << "The stream is not a ZX82 snapshot: type is 0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint16_t>(header.type)
          << std::dec << std::setfill(' ') << ".\n";
        return false;
    }

    if (header.compressionType != CompressionType::None && header.compressionType != CompressionType::RunLength) {
        std::cerr << "The stream contains an invalid compression type identifier (0x"
            << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint16_t>(header.compressionType)
            << std::dec << std::setfill(' ') << ").\n";
        return false;
    }

    snapshot.border = static_cast<Colour>(header.borderColour);
    snapshot.im = header.interruptMode;
    snapshot.iff1 = 0 != header.iff1;
    auto & registers = snapshot.registers();
    registers.i = header.i;
    registers.r = header.r;

    if constexpr (std::endian::native == std::endian::big) {
        registers.iy = header.iy.word;
        registers.ix = header.ix.word;
        registers.de = header.de.word;
        registers.bc = header.bc.word;
        registers.hl = header.hl.word;
        registers.af = header.af.word;
        registers.deShadow = header.deShadow.word;
        registers.bcShadow = header.bcShadow.word;
        registers.hlShadow = header.hlShadow.word;
        registers.afShadow = header.afShadow.word;
        registers.sp = header.sp.word;
        registers.pc = header.pc.word;
    } else {
        registers.iy = swapByteOrder(header.iy.word);
        registers.ix = swapByteOrder(header.ix.word);
        registers.de = swapByteOrder(header.de.word);
        registers.bc = swapByteOrder(header.bc.word);
        registers.hl = swapByteOrder(header.hl.word);
        registers.af = swapByteOrder(header.af.word);
        registers.deShadow = swapByteOrder(header.deShadow.word);
        registers.bcShadow = swapByteOrder(header.bcShadow.word);
        registers.hlShadow = swapByteOrder(header.hlShadow.word);
        registers.afShadow = swapByteOrder(header.afShadow.word);
        registers.sp = swapByteOrder(header.sp.word);
        registers.pc = swapByteOrder(header.pc.word);
    }

    if (CompressionType::RunLength == header.compressionType) {
        // Speculator docs say data can be 65496 bytes in length...
        UnsignedByte buffer[65496];
        in.read(reinterpret_cast<std::istream::char_type *>(buffer), 65496);

        if (!decompress(snapshot.memory().image + MemoryImageOffset, buffer, in.gcount())) {
            std::cerr << "Error decompressing RAM image\n";
            return false;
        }
    } else {
        in.read(reinterpret_cast<std::istream::char_type *>(snapshot.memory().image + MemoryImageOffset), 49152);
    }

    return true;
}

bool ZX82SnapshotReader::decompress(UnsignedByte * dest, UnsignedByte * source, std::size_t size)
{
    // see Amiga ROM Kernel Reference Manual: Devices Third Edition, Appendix A - IFF Specification for RLE algorithm
    auto * sourceEnd = source + size;
    auto * destEnd = dest + 49152;

    while (source < sourceEnd) {
        auto len = *source++;

        if (0x80 == len) {
            continue;
        }

        if (len & 0x80) {
            // replicate the next byte len + 1 times
            len = (~len) + 2;

            if (source >= sourceEnd) {
                std::cerr << "invalid compressed image: reading the source byte to replicate " << static_cast<std::uint16_t>(len) << " times would overflow buffer\n";
                return false;
            }

            if (dest + len > destEnd) {
                std::cerr << "invalid compressed data: decompressing " << static_cast<std::uint16_t>(len) << " bytes would overflow 48K RAM\n";
                return false;
            }

            auto replicatedByte = *source++;

            while (len) {
                *dest++ = replicatedByte;
                --len;
            }
        } else {
            // copy the next len + 1 bytes literally
            len += 1;

            if (source + len > sourceEnd) {
                std::cerr << "invalid compressed image: reading " << static_cast<std::uint16_t>(len) << " source bytes would overflow buffer\n";
                return false;
            }

            if (dest + len > destEnd) {
                std::cerr << "invalid compressed data: decompressing " << static_cast<std::uint16_t>(len) << " bytes would overflow 48K RAM\n";
                return false;
            }

            while (len) {
                *dest++ = *source++;
                --len;
            }
        }
    }

    return true;
}
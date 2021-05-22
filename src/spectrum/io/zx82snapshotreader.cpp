//
// Created by darren on 08/04/2021.
//

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <array>
#include "zx82snapshotreader.h"
#include "../spectrum48k.h"
#include "../../util/endian.h"
#include "../../util/debug.h"
#include "../../util/compiler.h"

using namespace Spectrum::Io;
using ::Z80::UnsignedByte;
using ::Z80::InterruptMode;

namespace
{
    // where the memory image read from the stream gets stored in the snapshot's memory image
    constexpr const std::uint16_t MemoryImageOffset = 16384;

    // the identifier required at the start of the header
    const std::uint32_t Identifier = *reinterpret_cast<const std::uint32_t *>("ZX82");

DISABLE_WARNING_PUSH
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
    // these are all the types of data that can be stored in .zx82 files, but we only use Snapshot
    enum class Type : std::uint8_t
    {
        BasicProgram = 0x00,
        NumericArray = 0x01,
        StringArray = 0x02,
        Code = 0x03,
        Snapshot = 0x04,
    };
DISABLE_WARNING_POP

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

#pragma pack(push, 1)
DISABLE_WARNING_PUSH
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
    // fileLength, startAddress and arrayName are not used for Snapshot type stream content
    // NOTE all 16-bit values are big endian (MC68000 byte order)
    struct Header
    {
        std::uint32_t identifier;
        Type type;
        CompressionType compressionType;
        std::uint16_t fileLength;
        std::uint16_t startAddress;
        std::uint16_t arrayName;
    };
DISABLE_WARNING_POP
#pragma pack(pop)

#pragma pack(push, 1)
    // NOTE all register paris are big endian (MC68000 byte order)
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
DISABLE_WARNING_PUSH
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
        std::uint8_t unused;
DISABLE_WARNING_POP
        std::uint8_t r;
        std::uint8_t iff1;
        RegisterPair pc;
    };
#pragma pack(pop)

    // used to convert 16-bit values from MC68000 byte order to host-native byte order if required
    using Util::swapByteOrder;
}

const Spectrum::Snapshot * Zx82SnapshotReader::read() const
{
    if (!isOpen()) {
        Util::debug << "Input stream is not open.\n";
        return nullptr;
    }

    auto & in = inputStream();
    SnapshotHeader header; // NOLINT(cppcoreguidelines-pro-type-member-init)
    in.read(reinterpret_cast<std::istream::char_type *>(&header), sizeof(SnapshotHeader));

    if (in.gcount() != sizeof(SnapshotHeader)) {
        Util::debug << "The stream is not a valid ZX82 stream: failed to read complete header.\n";
        return nullptr;
    }

    if (header.identifier != Identifier) {
        Util::debug << "The stream is not a valid ZX82 stream: incorrect file signature.\n";
        return nullptr;
    }
    
    if (header.type != Type::Snapshot) {
        Util::debug << "The stream is not a ZX82 snapshot: type is 0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint16_t>(header.type)
          << std::dec << std::setfill(' ') << ".\n";
        return nullptr;
    }

    if (header.compressionType != CompressionType::None && header.compressionType != CompressionType::RunLength) {
        Util::debug << "The stream contains an invalid compression type identifier (0x"
            << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint16_t>(header.compressionType)
            << std::dec << std::setfill(' ') << ").\n";
        return nullptr;
    }

    auto snapshot = std::make_unique<Snapshot>(Model::Spectrum48k);
    snapshot->border = static_cast<Colour>(header.borderColour);
    snapshot->im = header.interruptMode;
    snapshot->iff1 = 0 != header.iff1;
    auto & registers = snapshot->registers();
    registers.i = header.i;
    registers.r = header.r;

DISABLE_WARNING_PUSH
#pragma ide diagnostic ignored "Simplify"
#pragma ide diagnostic ignored "UnreachableCode"
    // NOTE one of these two branches will be diagnosed as unnecessary, depending on the byte order of the host, but the
    // code is required for cross-platform compatibility
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
DISABLE_WARNING_POP

    auto memory = std::make_unique<Spectrum48k::MemoryType>(0x10000);

    if (CompressionType::RunLength == header.compressionType) {
        // Speculator docs say data can be 65496 bytes in length...
        std::array<UnsignedByte, 65496> buffer; // NOLINT(cppcoreguidelines-pro-type-member-init) we're going to fill it right away with content from the stream
        in.read(reinterpret_cast<std::istream::char_type *>(buffer.data()), buffer.size());

        if (in.fail() && !in.eof()) {
            Util::debug << "Error reading RAM image from stream\n";
            return nullptr;
        }

        if (!decompress(memory->pointerTo(0) + MemoryImageOffset, buffer.data(), in.gcount())) {
            Util::debug << "Error decompressing RAM image\n";
            return nullptr;
        }
    } else {
        in.read(reinterpret_cast<std::istream::char_type *>(memory->pointerTo(0) + MemoryImageOffset), 49152);

        if (in.fail() && !in.eof()) {
            Util::debug << "Error reading RAM image from stream\n";
            return nullptr;
        }
    }

    snapshot->setMemory(std::move(memory));
    setSnapshot(std::move(snapshot));
    return this->snapshot();
}

bool Zx82SnapshotReader::decompress(UnsignedByte * dest, UnsignedByte * source, std::size_t size)
{
    // see Amiga ROM Kernel Reference Manual: Devices Third Edition, Appendix A - IFF Specification for RLE algorithm
    auto * sourceStart = source;
    auto * sourceEnd = source + size;
    auto * destStart = dest;
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
                Util::debug << "invalid compressed image: reading the source byte to replicate " << static_cast<std::uint16_t>(len) << " times would overflow read buffer\n";
                return false;
            }

            if (dest + len > destEnd) {
                Util::debug << "invalid compressed data: decompressing "
                << static_cast<std::uint16_t>(len) << " bytes into memory starting at 0x"
                << std::hex << std::setfill('0') << std::setw(4) << (dest - destStart)
                << " from offset " << std::dec << std::setfill(' ') << (source - sourceStart) << " in snapshot file would overflow 48K RAM\n";
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
                Util::debug << "invalid compressed image: reading "
                << static_cast<std::uint16_t>(len) << " literal bytes into memory starting at 0x"
                << std::hex << std::setfill('0') << std::setw(4) << (dest - destStart)
                << " from offset " << std::dec << std::setfill(' ') << (source - sourceStart) << " in snapshot file would overflow read buffer\n";
                return false;
            }

            if (dest + len > destEnd) {
                Util::debug << "invalid compressed data: reading "
                << static_cast<std::uint16_t>(len) << " literal bytes into memory starting at 0x"
                << std::hex << std::setfill('0') << std::setw(4) << (dest - destStart)
                << " from offset " << std::dec << std::setfill(' ') << (source - sourceStart) << " in snapshot file would overflow 48K RAM\n";
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

bool Zx82SnapshotReader::couldBeSnapshot(std::istream & in)
{
    static auto signature = *reinterpret_cast<const std::uint32_t *>("ZX82");

    if (!in) {
        Util::debug << "stream is not open.\n";
        return false;
    }

    std::uint32_t streamSignature;
    in.read(reinterpret_cast<std::istream::char_type *>(&streamSignature), sizeof(streamSignature));
    return !in.fail() && streamSignature == signature;
}

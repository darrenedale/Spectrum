//
// Created by darren on 22/03/2021.
//

#include <iostream>
#include <iomanip>
#include <iterator>
#include <cstring>

#include "z80snapshotreader.h"
#include "../types.h"
#include "../../z80/types.h"

using namespace Spectrum::Io;

using UnsignedByte = ::Z80::UnsignedByte;
using UnsignedWord = ::Z80::UnsignedWord;
using InterruptMode = ::Z80::InterruptMode;

namespace
{
    // see https://worldofspectrum.org/faq/reference/z80format.htm

    enum class Format : std::uint8_t
    {
        Version1 = 0,
        Version2 = 23,
        Version3 = 54,
        Version3_1 = 55,
    };

    enum class V1MachineType : uint8_t
    {
        Spectrum48k = 0,
    };

    enum class V2MachineType : uint8_t
    {
        Spectrum48k = 0,
        Spectrum48kInterface1,
        SamRam,
        Spectrum128k,
        Spectrum128kInterface1,
        SpectrumPlus3 = 7,
        SpectrumPlus3Mistaken,
        Pentagon128k,
        Scorpion256k,
        DidaktikKompakt,
        SpectrumPlus2,
        SpectrumPlus2A,
        TC2048,
        TC2068,
        TS2068,
    };

    enum class V3MachineType : uint8_t
    {
        Spectrum48k = 0,
        Spectrum48kInterface1,
        SamRam,
        Spectrum48kMgt,
        Spectrum128k,
        Spectrum128kInterface1,
        Spectrum128kMgt,
        SpectrumPlus3,
        SpectrumPlus3Mistaken,
        Pentagon128k,
        Scorpion256k,
        DidaktikKompakt,
        SpectrumPlus2,
        SpectrumPlus2A,
        TC2048,
        TC2068,
        TS2068 = 128,
    };

    struct SoundChipRegisters
    {
        UnsignedByte register1;
        UnsignedByte register2;
        UnsignedByte register3;
        UnsignedByte register4;
        UnsignedByte register5;
        UnsignedByte register6;
        UnsignedByte register7;
        UnsignedByte register8;
        UnsignedByte register9;
        UnsignedByte register10;
        UnsignedByte register11;
        UnsignedByte register12;
        UnsignedByte register13;
        UnsignedByte register14;
        UnsignedByte register15;
        UnsignedByte register16;
    };

    struct JoystickMapping
    {
        UnsignedWord mappings[5];
        UnsignedWord keys[5];
    };

    #pragma pack(1) // the compiler must not pad this struct otherwise header won't be read from the stream correctly
    struct HeaderV1
    {
        UnsignedByte a;
        UnsignedByte f;
        UnsignedWord bc;
        UnsignedWord hl;
        union
        {
            UnsignedWord pc;
            UnsignedWord extendedHeaderIndicator;        // if 0x0000 then it's an extended header format file
        };
        UnsignedWord sp;
        UnsignedByte i;
        UnsignedByte r;
        UnsignedByte fileFlags1;
        UnsignedWord de;
        UnsignedWord bcShadow;
        UnsignedWord deShadow;
        UnsignedWord hlShadow;
        UnsignedByte aShadow;
        UnsignedByte fShadow;
        UnsignedWord ix;
        UnsignedWord iy;
        UnsignedByte iff1;          // 0 = DI, 1 = EI
        UnsignedByte iff2;
        UnsignedByte fileFlags2;
    };

    #pragma pack(1) // the compiler must not pad this struct otherwise header won't be read from the stream correctly
    struct HeaderV2 : public HeaderV1
    {
        UnsignedWord extendedHeaderLength;
        UnsignedWord pcV2;
        union
        {
            V1MachineType v1MachineType;
            V2MachineType v2MachineType;
            V3MachineType v3MachineType;
        };
        UnsignedByte samRamState;
        union
        {
            UnsignedByte interface1RomPaged;   // 0xff if IF1 ROM is paged
            UnsignedByte lastTimexOut0xff;     // if in Timex mode, the last byte output to 0xff
        };
        UnsignedByte fileFlags3;
        UnsignedByte lastOut0xfffd;
        SoundChipRegisters soundChipRegisters;
    };

    #pragma pack(1) // the compiler must not pad this struct otherwise header won't be read from the stream correctly
    struct HeaderV3 : public HeaderV2
    {
        UnsignedWord lowTStateCounter;
        UnsignedByte highTStateCounter;
        UnsignedByte spectatorFlags;
        UnsignedByte mgtRomPaged;               // 0xff if MGT ROM is paged
        UnsignedByte multifaceRomPaged;         // 0xff if multiface ROM is paged
        UnsignedByte page1IsRam;                // 0xff if 0x0000 to 0x1fff in address space (8k) is RAM, otherwise it's ROM
        UnsignedByte page2IsRam;                // 0xff if 0x2000 to 0x3fff in address space (8k - 16k) is RAM, otherwise it's ROM
        JoystickMapping joystickMapping;
        UnsignedByte mgtType;
        UnsignedByte discipleInhibitButtonState;
        UnsignedByte discipleInhibitFlag;
    };

    #pragma pack(1) // the compiler must not pad this struct otherwise header won't be read from the stream correctly
    struct HeaderV3_1 : public HeaderV3
    {
        UnsignedByte lastOut0x1ffd;
    };

    constexpr const std::uint8_t CompressedMemoryFlag = 0b00100000;
    constexpr const std::uint16_t MemoryImageOffset = 16384;

    std::uint16_t readHostWord(std::istream & in)
    {
        static UnsignedWord word;
        in.read(reinterpret_cast<std::istream::char_type *>(&word), 2);
        return ::Z80::Z80::z80ToHostByteOrder(word);
    }
}

bool Z80SnapshotReader::readInto(Snapshot & snapshot) const
{
    if (!isOpen()) {
        std::cerr << "Input stream is not open.\n";
        return false;
    }

    auto & in = inputStream();

    HeaderV3_1 header; // NOLINT(cppcoreguidelines-pro-type-member-init) used as memory buffer for stream read, no need
                       // to initialise
    in.read(reinterpret_cast<std::istream::char_type *>(&header), sizeof(HeaderV1));
    auto format = Format::Version1;

    if (0x0000 == header.extendedHeaderIndicator) {
        in.read(reinterpret_cast<std::istream::char_type *>(&header) + sizeof(HeaderV1), sizeof(HeaderV2) - sizeof(HeaderV1));
        format = static_cast<Format>(Z80::z80ToHostByteOrder(header.extendedHeaderLength));

        switch (format) {
            case Format::Version2:
                if (header.v2MachineType != V2MachineType::Spectrum48k && header.v2MachineType != V2MachineType::Spectrum48kInterface1) {
                    std::cerr << "The stream is for a Spectrum model not currently supported.\n";
                    // TODO position stream read pointer at end of V2 content
                    return false;
                }
                break;

            case Format::Version3:
            case Format::Version3_1:
                if (header.v3MachineType != V3MachineType::Spectrum48k && header.v3MachineType != V3MachineType::Spectrum48kInterface1 && header.v3MachineType != V3MachineType::Spectrum48kMgt) {
                    std::cerr << "The stream is for a Spectrum model not currently supported.\n";
                    // TODO position stream read pointer at end of V3 content
                    return false;
                }

                // NOTE the format enumerator is the number of bytes in the extended header for that version, so
                // subtracting Version2 from the extendedHeaderLength tells us how many more bytes of header to read
                // since we've already read the full V2 header content
                in.read(reinterpret_cast<std::istream::char_type *>(&header) + sizeof(HeaderV2), header.extendedHeaderLength - static_cast<std::streamsize>(Format::Version2));
                break;

            default:
                std::cerr << "The version (" << static_cast<uint32_t>(format) << ") of the stream content is not recognised.\n";
                return false;
        }
    } else {
        std::cout << "Reading Z80 v1 format snapshot.\n";
    }

    auto & registers = snapshot.registers();

    registers.af = static_cast<UnsignedWord>(header.a) << 8 | header.f;
    registers.bc = Z80::z80ToHostByteOrder(header.bc);
    registers.de = Z80::z80ToHostByteOrder(header.de);
    registers.hl = Z80::z80ToHostByteOrder(header.hl);
    registers.ix = Z80::z80ToHostByteOrder(header.ix);
    registers.iy = Z80::z80ToHostByteOrder(header.iy);
    registers.sp = Z80::z80ToHostByteOrder(header.sp);

    registers.afShadow = static_cast<UnsignedWord>(header.aShadow) << 8 | header.fShadow;
    registers.bcShadow = Z80::z80ToHostByteOrder(header.bcShadow);
    registers.deShadow = Z80::z80ToHostByteOrder(header.deShadow);
    registers.hlShadow = Z80::z80ToHostByteOrder(header.hlShadow);

    registers.i = header.i;
    registers.r = ((header.fileFlags1 & 0x01) << 7) | (header.r & 0x7f);

    snapshot.iff1 = header.iff1;
    snapshot.iff2 = header.iff2;
    snapshot.border = static_cast<Colour>((header.fileFlags1 & 0b00001110) >> 1);
    snapshot.im = static_cast<InterruptMode>(header.fileFlags2 & 0x03);

    if (format == Format::Version1) {
        registers.pc = Z80::z80ToHostByteOrder(header.pc);

        if (header.fileFlags1 & CompressedMemoryFlag) {
            // read the remaining content of the stream as the compressed memory image
            UnsignedByte buffer[0xffff];
            in.read(reinterpret_cast<std::istream::char_type *>(buffer), 0xffff);

            if (in.fail() && !in.eof()) {
                std::cerr << "Failed reading compressed memory image\n";
                return false;
            }

            // work out the actual size of the compressed data (i.e. look for the end marker)
            auto size = compressedSize(buffer, in.gcount());

            if (!size) {
                std::cerr << "failed to find end of memory image marker in stream\n";
                return false;
            }

            // decompress it into the snapshot's memory image
            decompress(snapshot.memory().image + MemoryImageOffset, buffer, *size);
        } else {
            in.read(reinterpret_cast<std::istream::char_type *>(snapshot.memory().image + MemoryImageOffset), (0x10000 - MemoryImageOffset));
        }
    } else {
        registers.pc = Z80::z80ToHostByteOrder(header.pcV2);

        while (!in.eof() && !in.fail()) {
            auto size = readHostWord(in);
            auto page = static_cast<std::uint8_t>(in.get());
            bool readPage = true;
            UnsignedByte * pageMemory;

            switch (page) {
                case 4:
                    pageMemory = snapshot.memory().image + 0x8000;
                    break;

                case 5:
                    pageMemory = snapshot.memory().image + 0xc000;
                    break;

                case 8:
                    pageMemory = snapshot.memory().image + 0x4000;
                    break;

                default:
                    in.seekg(in.tellg() + static_cast<std::streamsize>(size), std::ios_base::cur);
                    readPage = false;
                    break;
            }

            if (readPage) {
                auto * buffer = new UnsignedByte[size];
                in.read(reinterpret_cast<std::istream::char_type *>(buffer), size);
                decompress(pageMemory, buffer, size);
                delete[] buffer;
            }
        }
    }

    return true;
}

void Z80SnapshotReader::decompress(Z80::UnsignedByte * dest, Z80::UnsignedByte * source, std::size_t size)
{
    static const std::uint16_t rleMarker = 0xeded;
    auto * end = source + size;

    while (source < end) {
        if (rleMarker == *(reinterpret_cast<std::uint16_t *>(source))) {
            source += 2;
            auto count = *reinterpret_cast<std::uint8_t *>(source++);
            std::memset(dest, *(source++), count);
            dest += count;
        } else {
            *(dest++) = *(source++);
        }
    }

}

std::optional<std::size_t> Z80SnapshotReader::compressedSize(Z80::UnsignedByte * memory, std::size_t max)
{
    static auto endMarker = *reinterpret_cast<const std::uint32_t *>("\x00\xed\xed\x00");
    auto * end = memory;
    auto * maxEnd = end + max - 4;

    while (end <= maxEnd && *reinterpret_cast<std::uint32_t *>(end) != endMarker) {
        ++end;
    }

    if (end > maxEnd) {
        return {};
    }

    return (end - memory);
}

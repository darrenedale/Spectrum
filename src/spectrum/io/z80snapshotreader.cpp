//
// Created by darren on 22/03/2021.
//

#include <iostream>
#include <iterator>
#include <cstring>

#include "z80snapshotreader.h"
#include "../spectrum48k.h"
#include "../spectrum128k.h"
#include "../spectrumplus2.h"
#include "../spectrumplus2a.h"
#include "../spectrumplus3.h"

using namespace Spectrum::Io;

using ::Z80::UnsignedByte;
using ::Z80::UnsignedWord;
using ::Z80::InterruptMode;
using ::Z80::z80ToHostByteOrder;

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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
    // several enumerated values are required for the format but never actually used by the reader
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
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
    // sound is not currently emulated so this struct is never used but must be present in the format
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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
    // this is never used because it's emulator config not machine state
    struct JoystickMapping
    {
        UnsignedWord mappings[5];
        UnsignedWord keys[5];
    };
#pragma clang diagnostic pop

#pragma pack(push, 1)
    // the compiler must not pad this struct otherwise header won't be read from the stream correctly
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
#pragma pack(pop)

#pragma pack(push, 1)
    // the compiler must not pad this struct otherwise header won't be read from the stream correctly
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
        union {
            UnsignedByte samRamState;     // if machine type is SamRam
            UnsignedByte lastOut0x7ffd;   // if machine type is 128K Spectrum (128K, plus2/2a/3)
            UnsignedByte lastOut0xf4;     // if machine type is Times
        };
        union
        {
            UnsignedByte interface1RomPaged;   // 0xff if IF1 ROM is paged
            UnsignedByte lastTimexOut0xff;     // if in Timex mode, the last byte output to 0xff
        };
        UnsignedByte fileFlags3;
        UnsignedByte lastOut0xfffd;
        SoundChipRegisters soundChipRegisters;
    };
#pragma pack(pop)

#pragma pack(push, 1)
    // the compiler must not pad this struct otherwise header won't be read from the stream correctly
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
#pragma pack(pop)

#pragma pack(push, 1)
    // the compiler must not pad this struct otherwise header won't be read from the stream correctly
    struct HeaderV3_1 : public HeaderV3
    {
        UnsignedByte lastOut0x1ffd;
    };
#pragma pack(pop)

    // bits and masks to read values from flag bytes
    constexpr const std::uint8_t PagedRamMask128k = 0b00000111;      // applied to lastOut0x7ffd to retrieve the currently paged memory bank
    constexpr const std::uint8_t PagedRomBit128k = 4;                // the bit that indicates which ROM is paged in lastOut0x7ffd
    constexpr const std::uint8_t PagedRomMask128k = 1 << PagedRomBit128k; // applied to lastOut0x7ffd to retrieve the currently paged ROM
    constexpr const std::uint8_t ShadowDisplayFileFlag = 0b00010000; // applied to lastOut0x7ffd to determine whether the shadow display file is in use
    constexpr const std::uint8_t PagingDisabledFlag = 0b00100000;    // applied to lastOut0x7ffd to determine whether paging is disabled
    constexpr const std::uint8_t PagedRomBitPlus2a = 2;                // the high bit of the paged in ROM number in lastOut0x1ffd
    constexpr const std::uint8_t PagedRomMaskPlus2a = 1 << PagedRomBitPlus2a; // applied to lastOut0x1ffd to retrieve the high bit of the currently paged ROM

    constexpr const std::uint8_t CompressedMemoryFlag = 0b00100000;
    constexpr const std::uint16_t MemoryImageOffset = 16384;

    std::uint16_t readHostWord(std::istream & in)
    {
        static UnsignedWord word;
        in.read(reinterpret_cast<std::istream::char_type *>(&word), 2);
        return z80ToHostByteOrder(word);
    }
}

const Spectrum::Snapshot * Z80SnapshotReader::read() const
{
    if (!isOpen()) {
        std::cerr << "Input stream is not open.\n";
        return nullptr;
    }

    auto & in = inputStream();

    HeaderV3_1 header; // NOLINT(cppcoreguidelines-pro-type-member-init) used as memory buffer for stream read, no need
                       // to initialise
    in.read(reinterpret_cast<std::istream::char_type *>(&header), sizeof(HeaderV1));
    auto format = Format::Version1;
    std::unique_ptr<Snapshot> snapshot = nullptr;
    std::unique_ptr<BaseSpectrum::MemoryType> memory = nullptr;

    if (0x0000 == header.extendedHeaderIndicator) {
        in.read(reinterpret_cast<std::istream::char_type *>(&header) + sizeof(HeaderV1), sizeof(HeaderV2) - sizeof(HeaderV1));
        format = static_cast<Format>(z80ToHostByteOrder(header.extendedHeaderLength));

        switch (format) {
            case Format::Version2:
                switch (header.v2MachineType) {
                    case V2MachineType::Spectrum48k:
                    case V2MachineType::Spectrum48kInterface1:
                        snapshot = std::make_unique<Snapshot>(Model::Spectrum48k);
                        memory = std::make_unique<Spectrum48k::MemoryType>(0x10000);
                        break;

                    case V2MachineType::Spectrum128k:
                    case V2MachineType::Spectrum128kInterface1:
                        snapshot = std::make_unique<Snapshot>(Model::Spectrum128k);
                        memory = std::make_unique<Spectrum128k::MemoryType>();
                        break;

                    case V2MachineType::SpectrumPlus2:
                        snapshot = std::make_unique<Snapshot>(Model::SpectrumPlus2);
                        memory = std::make_unique<SpectrumPlus2::MemoryType>();
                        break;

                    case V2MachineType::SpectrumPlus2A:
                        snapshot = std::make_unique<Snapshot>(Model::SpectrumPlus2a);
                        memory = std::make_unique<SpectrumPlus2a::MemoryType>();
                        break;

                    case V2MachineType::SpectrumPlus3:
                        snapshot = std::make_unique<Snapshot>(Model::SpectrumPlus3);
                        memory = std::make_unique<SpectrumPlus3::MemoryType>();
                        break;

                    default:
                        return nullptr;
                }
                break;

            case Format::Version3:
            case Format::Version3_1:
                // NOTE the format enumerator is the number of bytes in the extended header for that version, so
                // subtracting Version2 from the extendedHeaderLength tells us how many more bytes of header to read
                // since we've already read the full V2 header content
                in.read(reinterpret_cast<std::istream::char_type *>(&header) + sizeof(HeaderV2), header.extendedHeaderLength - static_cast<std::streamsize>(Format::Version2));

                switch (header.v3MachineType) {
                    case V3MachineType::Spectrum48k:
                    case V3MachineType::Spectrum48kInterface1:
                    case V3MachineType::Spectrum48kMgt:
                        snapshot = std::make_unique<Snapshot>(Model::Spectrum48k);
                        memory = std::make_unique<Spectrum48k::MemoryType>(0x10000);
                        break;

                    case V3MachineType::Spectrum128k:
                    case V3MachineType::Spectrum128kInterface1:
                    case V3MachineType::Spectrum128kMgt:
                        snapshot = std::make_unique<Snapshot>(Model::Spectrum128k);
                        memory = std::make_unique<Spectrum128k::MemoryType>();
                        break;

                    case V3MachineType::SpectrumPlus2:
                        snapshot = std::make_unique<Snapshot>(Model::SpectrumPlus2);
                        memory = std::make_unique<SpectrumPlus2::MemoryType>();
                        break;

                    case V3MachineType::SpectrumPlus2A:
                        snapshot = std::make_unique<Snapshot>(Model::SpectrumPlus2a);
                        memory = std::make_unique<SpectrumPlus2a::MemoryType>();
                        break;

                    case V3MachineType::SpectrumPlus3:
                        snapshot = std::make_unique<Snapshot>(Model::SpectrumPlus3);
                        memory = std::make_unique<SpectrumPlus3::MemoryType>();
                        break;

                    default:
                        // no other spectrum models are currently supported
                        return nullptr;
                }
                break;

            default:
                std::cerr << "The version (" << static_cast<uint32_t>(format) << ") of the stream content is not recognised.\n";
                return nullptr;
        }
    } else {
        snapshot = std::make_unique<Snapshot>(Model::Spectrum48k);
        memory = std::make_unique<Spectrum48k::MemoryType>(0x10000);
    }

    if (!snapshot) {
        std::cerr << "The stream is for a Spectrum model not currently supported.\n";
        return nullptr;
    }

    assert(memory);
    auto & registers = snapshot->registers();

    registers.af = static_cast<UnsignedWord>(header.a) << 8 | header.f;
    registers.bc = z80ToHostByteOrder(header.bc);
    registers.de = z80ToHostByteOrder(header.de);
    registers.hl = z80ToHostByteOrder(header.hl);
    registers.ix = z80ToHostByteOrder(header.ix);
    registers.iy = z80ToHostByteOrder(header.iy);
    registers.sp = z80ToHostByteOrder(header.sp);

    registers.afShadow = static_cast<UnsignedWord>(header.aShadow) << 8 | header.fShadow;
    registers.bcShadow = z80ToHostByteOrder(header.bcShadow);
    registers.deShadow = z80ToHostByteOrder(header.deShadow);
    registers.hlShadow = z80ToHostByteOrder(header.hlShadow);

    registers.i = header.i;
    registers.r = ((header.fileFlags1 & 0x01) << 7) | (header.r & 0x7f);

    snapshot->iff1 = header.iff1;
    snapshot->iff2 = header.iff2;
    snapshot->border = static_cast<Colour>((header.fileFlags1 & 0b00001110) >> 1);
    snapshot->im = static_cast<InterruptMode>(header.fileFlags2 & 0x03);

    if (format == Format::Version1) {
        registers.pc = z80ToHostByteOrder(header.pc);

        if (header.fileFlags1 & CompressedMemoryFlag) {
            // read the remaining content of the stream as the compressed memory image
            UnsignedByte buffer[0xffff];
            in.read(reinterpret_cast<std::istream::char_type *>(buffer), 0xffff);

            if (in.fail() && !in.eof()) {
                std::cerr << "Failed reading compressed memory image\n";
                return nullptr;
            }

            // work out the actual size of the compressed data (i.e. look for the end marker)
            auto size = compressedSize(buffer, in.gcount());

            if (!size) {
                std::cerr << "failed to find end of memory image marker in stream\n";
                return nullptr;
            }

            // decompress it into the memory image
            decompress(memory->pointerTo(0) + MemoryImageOffset, buffer, *size);
        } else {
            in.read(reinterpret_cast<std::istream::char_type *>(memory->pointerTo(0) + MemoryImageOffset), (0x10000 - MemoryImageOffset));

            if (in.fail() && !in.eof()) {
                std::cerr << "Error reading memory image\n";
                return nullptr;
            }
        }
    } else {
        registers.pc = z80ToHostByteOrder(header.pcV2);

        // for 128k models, page in the appropriate ROM and RAM and set the display buffer and paging disabled flags
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
        switch (snapshot->model()) {
            case Model::Spectrum128k:
            case Model::SpectrumPlus2:
            case Model::SpectrumPlus2a:
            case Model::SpectrumPlus3:
                snapshot->screenBuffer = (header.lastOut0x7ffd & ShadowDisplayFileFlag ? ScreenBuffer128k::Shadow : ScreenBuffer128k::Normal);
                snapshot->pagedBankNumber = header.lastOut0x7ffd & PagedRamMask128k;

                // TODO if paging was disabled then a further OUT to port 0x7ffd was executed with bit 5 reset, paging
                //  will still be disabled but this byte will indicate that it's enabled - therefore our snapshot won't
                //  reflect the true state of the spectrum from which it was created. is there anywhere else in the file
                //  format that indicates paging status? while a genuine problem, it's unlikely to affect much as I
                //  believe disabling paging was very rare in the real world
                snapshot->pagingEnabled = !(header.lastOut0x7ffd & PagingDisabledFlag);

                if (Model::SpectrumPlus2a == snapshot->model() || Model::SpectrumPlus3 == snapshot->model()) {
                    if (header.lastOut0x1ffd & 0x01) {
                        snapshot->pagingMode = PagingMode::Special;
                        // paging config is in bits 1 and 2
                        snapshot->specialPagingConfig = static_cast<SpecialPagingConfiguration>((header.lastOut0x1ffd & 0b00000110) >> 1);
                    } else {
                        snapshot->pagingMode = PagingMode::Normal;
                        // ROM number is made up of bit 4 in lastOut0x7ffd and bit 2 in lastOut0x1ffd
                        snapshot->romNumber = ((header.lastOut0x1ffd & PagedRomMaskPlus2a) >> (PagedRomBitPlus2a - 1)) |  // high bit
                                ((header.lastOut0x7ffd & PagedRomMask128k) >> PagedRomBit128k);              // low bit
                    }
                } else {
                    // ROM number is simply value of bit 4 = Rom0 or Rom1
                    snapshot->romNumber = (header.lastOut0x7ffd & PagedRomMask128k) >> PagedRomBit128k;
                }
                break;
        }
#pragma clang diagnostic pop

        while (true) {
            in.peek();

            if (in.eof()) {
                break;
            }

            if (in.fail()) {
                std::cerr << "failed to read from stream at " << in.tellg() << " (expecting memory page)\n";
                return nullptr;
            }

            auto size = readHostWord(in);
            auto page = static_cast<std::uint8_t>(in.get());
            UnsignedByte * pageMemory = nullptr;

            switch (snapshot->model()) {
                case Model::Spectrum48k:
                    switch (page) {
                        case 4:
                            pageMemory = memory->pointerTo(0x8000);
                            break;

                        case 5:
                            pageMemory = memory->pointerTo(0xc000);
                            break;

                        case 8:
                            pageMemory = memory->pointerTo(0x4000);
                            break;

                        default:
                            in.seekg(in.tellg() + static_cast<std::streamsize>(size), std::ios_base::cur);
                            break;
                    }
                    break;

                case Model::Spectrum128k:
                case Model::SpectrumPlus2:
                case Model::SpectrumPlus2a:
                case Model::SpectrumPlus3:
                    // in Z80 files page #0 is represented by 0x03, page #1 by 0x04, etc. up to page#7 by 0x0a
                    page -= 3;
                    pageMemory = dynamic_cast<PagedMemoryInterface *>(memory.get())->pagePointer(page);
                    break;

                default:
                    // should never get here - if it's an unsupported model we should already have exited
                    break;
            }

            // pageMemory will be nullptr if it's a page that the model doesn't support
            if (pageMemory) {
                std::vector<UnsignedByte> buffer(size);
                in.read(reinterpret_cast<std::istream::char_type *>(buffer.data()), size);

                if (size != in.gcount()) {
                    std::cerr << "truncated read (expected " << size << " read " << in.gcount() << ") for page #" << static_cast<std::uint16_t>(page) << '\n';
                    return nullptr;
                }

                decompress(pageMemory, buffer.data(), size);
            }
        }
    }

    snapshot->setMemory(std::move(memory));
    setSnapshot(std::move(snapshot));
    return this->snapshot();
}

void Z80SnapshotReader::decompress(UnsignedByte * dest, UnsignedByte * source, std::size_t size)
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

std::optional<std::size_t> Z80SnapshotReader::compressedSize(UnsignedByte * memory, std::size_t max)
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

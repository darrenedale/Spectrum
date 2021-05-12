//
// Created by darren on 14/03/2021.
//

#include <iostream>
#include <array>
#include "z80snapshotwriter.h"
#include "../pagingmemoryinterface.h"
#include "../../util/debug.h"

using namespace Spectrum::Io;
using ::Z80::UnsignedWord;
using ::Z80::UnsignedByte;
using ::Z80::hostToZ80ByteOrder;

namespace
{
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
    // these are required by the format but many are unused because we don't support all models
    enum class MachineType : uint8_t
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

    // this is currently never populated as the sound chip is not currently emulated
    struct SoundChipRegisters
    {
        UnsignedByte registers[16];
    };

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
    // this is currently never populated as this is really emulator config rather than part of the machine state
    struct JoystickMapping
    {
        UnsignedWord mappings[5];
        UnsignedWord keys[5];
    };
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma pack(push, 1)
    // the compiler must not pad this struct otherwise header won't be read from the stream correctly
    // all words are in Z80 byte order
    struct Header
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
        UnsignedWord extendedHeaderLength;
        UnsignedWord pcV2;
        MachineType machineType;
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
        UnsignedByte lastOut0x1ffd;
    };
#pragma pack(pop)
#pragma clang diagnostic pop
}

bool Z80SnapshotWriter::writeTo(std::ostream & out) const
{
    switch (snapshot().model()) {
        case Model::Spectrum16k:
            return write16k(out);

        case Model::Spectrum48k:
            return write48k(out);

        case Model::Spectrum128k:
        case Model::SpectrumPlus2:
        case Model::SpectrumPlus2a:
        case Model::SpectrumPlus3:
            return write128kModel(out);
    }

    // unreachable code
    assert(false);
}

bool Z80SnapshotWriter::writeHeader(std::ostream & out) const
{
    auto & snap = snapshot();
    auto & registers = snap.registers();
    Header header{
        .a = registers.a,
        .f = registers.f,
        .bc = hostToZ80ByteOrder(registers.bc),
        .hl = hostToZ80ByteOrder(registers.hl),
        .extendedHeaderIndicator = 0x0000,
        .sp = hostToZ80ByteOrder(registers.sp),
        .i = registers.i,
        .r = registers.r,
        // bit 5 indicates compressed memory, bit 7 of r reg in bit 0, border colour in bits 1-3, compression flag in bit 5
        .fileFlags1 = static_cast<UnsignedByte>(0b00100000 | ((registers.r & 0x80) >> 7) | ((static_cast<UnsignedByte>(snap.border) & 0b00000111) << 1)),
        .de = hostToZ80ByteOrder(registers.de),
        .bcShadow = hostToZ80ByteOrder(registers.bcShadow),
        .deShadow = hostToZ80ByteOrder(registers.deShadow),
        .hlShadow = hostToZ80ByteOrder(registers.hlShadow),
        .aShadow = registers.aShadow,
        .fShadow = registers.fShadow,
        .ix = hostToZ80ByteOrder(registers.ix),
        .iy = hostToZ80ByteOrder(registers.iy),
        .iff1 = static_cast<std::uint8_t>(snap.iff1 ? 1 : 0),
        .iff2 = static_cast<std::uint8_t>(snap.iff2 ? 1 : 0),
        // IM in bits 0-1, nothing else is relevant to us
        .fileFlags2 = static_cast<UnsignedByte>(snap.im),
        .extendedHeaderLength = 55,
        .pcV2 = hostToZ80ByteOrder(snap.registers().pc),
        .lastOut0x7ffd = 0x00,      // if machine type is 128K Spectrum (128K, plus2/2a/3)
        .interface1RomPaged = 0x00, // 0xff if IF1 ROM is paged
        .fileFlags3 = 0b00000011,   // bit 0 = R reg emulation on, bit 1 = LDIR emulation on (other bits not relevant)
        .lastOut0xfffd = 0x00,
        .soundChipRegisters = {},
        .lowTStateCounter = 0x00,
        .highTStateCounter = 0x00,
        .spectatorFlags = 0x00,
        .mgtRomPaged = 0x00,        // 0xff if MGT ROM is paged
        .multifaceRomPaged = 0x00,  // 0xff if multiface ROM is paged
        .page1IsRam = 0x00,         // 0xff if 0x0000 to 0x1fff in address space (8k) is RAM, otherwise it's ROM
        .page2IsRam = 0x00,         // 0xff if 0x2000 to 0x3fff in address space (8k - 16k) is RAM, otherwise it's ROM
        .joystickMapping = {},
        .mgtType = 0x00,
        .discipleInhibitButtonState = 0x00,
        .discipleInhibitFlag = 0x00,
        .lastOut0x1ffd = 0x00,
    };

    switch (snap.model()) {
        case Model::Spectrum16k:
        case Model::Spectrum48k:
            header.machineType = MachineType::Spectrum48k;
            break;

        case Model::Spectrum128k:
            header.machineType = MachineType::Spectrum128k;
            header.lastOut0x7ffd = lastOut0x7ffd();
            break;

        case Model::SpectrumPlus2:
            header.machineType = MachineType::SpectrumPlus2;
            header.lastOut0x7ffd = lastOut0x7ffd();
            break;

        case Model::SpectrumPlus2a:
            header.machineType = MachineType::SpectrumPlus2A;
            header.lastOut0x7ffd = lastOut0x7ffd();
            header.lastOut0x1ffd = lastOut0x1ffd();
            break;

        case Model::SpectrumPlus3:
            header.machineType = MachineType::SpectrumPlus3;
            header.lastOut0x7ffd = lastOut0x7ffd();
            header.lastOut0x1ffd = lastOut0x1ffd();
            break;
    }

    out.write(reinterpret_cast<std::ostream::char_type *>(&header), sizeof(Header));
    return out.good();
}

bool Z80SnapshotWriter::write48k(std::ostream & out) const
{
    if (!writeHeader(out)) {
        Util::debug << "failed writing .z80 header\n";
        return false;
    }

    if (!writeMemoryPage(out, snapshot().memory()->pointerTo(0x8000), 1) ||
        !writeMemoryPage(out, snapshot().memory()->pointerTo(0xc000), 2) ||
        !writeMemoryPage(out, snapshot().memory()->pointerTo(0x4000), 5)
    ) {
        Util::debug << "failed writing memory to .z80 file\n";
        return false;
    }

    return true;
}

bool Z80SnapshotWriter::write16k(std::ostream & out) const
{
    if (!writeHeader(out)) {
        Util::debug << "failed writing .z80 header\n";
        return false;
    }

    std::array<::Z80::UnsignedByte, 0x4000> emptyPage = {};
    emptyPage.fill(0xff);

    if (!writeMemoryPage(out, snapshot().memory()->pointerTo(0x4000), 5) ||
        !writeMemoryPage(out, emptyPage.data(), 1) ||
        !writeMemoryPage(out, emptyPage.data(), 2)
    ) {
        Util::debug << "failed writing memory to .z80 file\n";
        return false;
    }

    return true;
}

bool Z80SnapshotWriter::write128kModel(std::ostream & out) const
{
    if (!writeHeader(out)) {
        Util::debug << "failed writing .z80 header\n";
        return false;
    }

    const auto * memory = dynamic_cast<const PagingMemoryInterface *>(snapshot().memory());
    assert(memory);
    auto pages = memory->pageCount();

    for (int page = 0; page < pages; ++page) {
        if (!writeMemoryPage(out, memory->pagePointer(page), page)) {
            Util::debug << "failed writing memory page #" << page << " to .z80 file\n";
            return false;
        }
    }

    return true;
}

std::uint8_t Z80SnapshotWriter::lastOut0x7ffd() const
{
    const auto & snap = snapshot();

    return (static_cast<std::uint8_t>(snap.pagedBankNumber) & 0b00000111) |     // paged bank in bits 0-2
        (snap.screenBuffer == ScreenBuffer128k::Shadow ? 0b00001000 : 0) |      // shadow buffer flag in bit 3
        ((snap.romNumber & 0x01) << 4) |                                        // rom number (or low bit thereof) in bit 4
        (!snap.pagingEnabled ? 0b00100000 : 0);                                 // paging disabled flag in bit 5
}

std::uint8_t Z80SnapshotWriter::lastOut0x1ffd() const
{
    const auto & snap = snapshot();

    if (PagingMode::Special == snap.pagingMode) {
        // bits 1 and 2 contain the special paging config
        return 0x01 | (static_cast<UnsignedByte>(snap.specialPagingConfig) << 1);
    }

    // for normal paging all we need to store in this flag is the bit that represents the high bit of the ROM number
    return (snap.romNumber & 0x02) << 1;
}

std::vector<::Z80::UnsignedByte> Z80SnapshotWriter::compressMemory(const ::Z80::UnsignedByte * memory, std::uint32_t size)
{
    auto * end = memory + size;
    std::vector<::Z80::UnsignedByte> ret;

    while (memory < end) {
        auto * sequenceStart = memory;
        auto sequenceByte = *memory;

        while (memory < end && memory < sequenceStart + 0xff &&  *memory == sequenceByte) {
            ++memory;
        }

        if (memory - sequenceStart < 5) {
            // insufficiently long sequence
            if (0xed == sequenceByte) {
                // 0xed bytes in the uncompressed memory are special
                if (memory - sequenceStart > 1) {
                    // two or more 0xed bytes are always encoded as a sequence
                    ret.push_back(0xed);        // 2-byte marker
                    ret.push_back(0xed);
                    ret.push_back(static_cast<std::uint8_t>(memory - sequenceStart));   // sequence length
                    ret.push_back(0xed);                                        // byte to repeat
                } else if (memory < end) {
                    // the byte following a single 0xed is never compressed as part of a sequence, it's always output as
                    // a single byte after the 0xed
                    ret.push_back(0xed);
                    ret.push_back(*memory++);
                } else {
                    // a single 0xed at the very end of the uncompressed memory
                    ret.push_back(0xed);
                }
            } else {
                while (sequenceStart < memory) {
                    ret.push_back(*sequenceStart++);
                }
            }
        } else {
            // sequence of >= 5
            ret.push_back(0xed);        // 2-byte marker
            ret.push_back(0xed);
            ret.push_back(static_cast<std::uint8_t>(memory - sequenceStart));   // sequence length
            ret.push_back(sequenceByte);                                        // byte to repeat
        }
    }

    return ret;
}

bool Z80SnapshotWriter::writeMemoryPage(std::ostream & out, const ::Z80::UnsignedByte * memory, PagingMemoryInterface::PageNumber pageNumber)
{
    // NOTE memory pages are ALWAYS 16kb in size
    auto compressed = compressMemory(memory, 0x4000);
    const std::ostream::char_type * data;
    std::streamsize size;

    if (compressed.size() > 0x3fff) {
        // "compressed" image is larger than uncompressed so write uncompressed
        writeHostWord(out, 0xffff);
        size = 0x4000;
        data = reinterpret_cast<const std::ostream::char_type *>(memory);
    } else {
        writeHostWord(out, compressed.size());
        data = reinterpret_cast<const std::ostream::char_type *>(compressed.data());
        size = static_cast<std::streamsize>(compressed.size());
    }

    // in .z80 file, pages are identified by page # + 3 (i.e. page 0 = 3, page 1 = 4, ... page 7 = 10)
    out.put(static_cast<std::ostream::char_type>(static_cast<std::uint8_t>(pageNumber) + 3));
    out.write(data, size);
    return out.good();
}

//
// Created by darren on 27/04/2021.
//

#include <cstdint>
#include <iostream>
#include <iomanip>
#include "zx82snapshotwriter.h"
#include "../spectrum48k.h"
#include "../../util/endian.h"
#include "../../util/debug.h"

using namespace Spectrum::Io;
using ::Z80::UnsignedByte;
using ::Z80::InterruptMode;

namespace
{
    // where the memory image read from the stream gets stored in the snapshot's memory image
    constexpr const std::uint16_t MemoryImageOffset = 0x4000;

    constexpr const int MaxRunLength = 128;

    // the identifier required at the start of the header
    const std::uint32_t Identifier = *reinterpret_cast<const std::uint32_t *>("ZX82");

#pragma clang diagnostic push
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
#pragma clang diagnostic pop

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
#pragma clang diagnostic push
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
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
        std::uint8_t unused;
#pragma clang diagnostic pop
        std::uint8_t r;
        std::uint8_t iff1;
        RegisterPair pc;
    };
#pragma pack(pop)

    // used to convert 16-bit values from MC68000 byte order to host-native byte order if required
    using Util::swapByteOrder;
}

bool Zx82SnapshotWriter::writeTo(std::ostream & out) const
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

    const auto & registers = snap.registers();

    Header header{
            .identifier = Identifier,
            .type = Type::Snapshot,
            .compressionType = compressionEnabled() ? CompressionType::RunLength : CompressionType::None,
            .fileLength = 0,            // not used for Snapshot type
            .startAddress = 0,          // not used for Snapshot type
            .arrayName = 0,             // not used for Snapshot type
            .borderColour = static_cast<std::uint8_t>(snap.border),
            .interruptMode = snap.im,
            .iy = {.word = registers.iy},
            .ix = {.word = registers.ix},
            .de = {.word = registers.de},
            .bc = {.word = registers.bc},
            .hl = {.word = registers.hl},
            .af = {.word = registers.af},
            .deShadow = {.word = registers.deShadow},
            .bcShadow = {.word = registers.bcShadow},
            .hlShadow = {.word = registers.hlShadow},
            .afShadow = {.word = registers.afShadow},
            .sp = {.word = registers.sp},
            .i = registers.i,
            .unused = 0,
            .r = registers.r,
            .iff1 = snap.iff1 ? static_cast<std::uint8_t>(0x01) : static_cast<std::uint8_t>(0x00),
            .pc = {.word = registers.pc},
    };

#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
    if constexpr (std::endian::native != std::endian::big) {
        header.iy.word = swapByteOrder(header.iy.word);
        header.ix.word = swapByteOrder(header.ix.word);
        header.de.word = swapByteOrder(header.de.word);
        header.bc.word = swapByteOrder(header.bc.word);
        header.hl.word = swapByteOrder(header.hl.word);
        header.af.word = swapByteOrder(header.af.word);
        header.deShadow.word = swapByteOrder(header.deShadow.word);
        header.bcShadow.word = swapByteOrder(header.bcShadow.word);
        header.hlShadow.word = swapByteOrder(header.hlShadow.word);
        header.afShadow.word = swapByteOrder(header.afShadow.word);
        header.sp.word = swapByteOrder(header.sp.word);
        header.pc.word = swapByteOrder(header.pc.word);
    }
#pragma clang diagnostic pop

    out.write(reinterpret_cast<const std::ostream::char_type *>(&header), sizeof(Header));

    if (!out.good()) {
        Util::debug << "failed writing .zx82 header\n";
        return false;
    }

    // NOTE we assume that the snapshot has a standard 48K Spectrum memory object
    if (compressionEnabled()) {
        auto compressed = compressMemory(snap.memory()->pointerTo(MemoryImageOffset), 0x10000 - MemoryImageOffset);
        out.write(reinterpret_cast<const std::ostream::char_type *>(compressed.data()), static_cast<std::streamsize>(compressed.size()));
    } else {
        out.write(reinterpret_cast<const std::ostream::char_type *>(snap.memory()->pointerTo(MemoryImageOffset)), 0x10000 - MemoryImageOffset);
    }

    return !out.bad() && !out.fail();
}

Zx82SnapshotWriter::CompressedMemory Zx82SnapshotWriter::compressMemory(const Z80::UnsignedByte * memory, std::uint32_t size)
{
    assert(size > 1);
    CompressedMemory ret;
    ret.reserve(size);
    auto * end = memory + size;

    enum class RunType : std::uint8_t
    {
        Literal = 0x00,
        Replicate = 0x80,
    };

    RunType currentRunType;

    // each time we return to the start of this loop, memory is guaranteed to be a pointer to at least two bytes at the start of a run to encode
    while (memory < end) {
        auto * runEnd = memory + 1;

        if (*runEnd == *memory) {
            // count up to 127 replica bytes
            currentRunType = RunType::Replicate;
            ++runEnd;   // we already know that the first two bytes are the same

            while (runEnd < end && MaxRunLength > (runEnd - memory) && *runEnd == *memory) {
                ++runEnd;
            }

            // if the replica run leaves a single dangling byte at the end, omit the last byte from the run so that it can be used with the dangling byte in a
            // literal run of two since we can't encode lone byte
            if (runEnd == (end - 1)) {
                --runEnd;

                // if this means the replicate run is now only a single byte, encode all three as a literal run
                if (runEnd == memory + 1) {
                    runEnd += 2;
                    currentRunType = RunType::Literal;
                }
            }
        } else {
            // encode a run of literal bytes
            currentRunType = RunType::Literal;
            ++runEnd;   // we already know that the first two bytes are different

            while (runEnd < end && MaxRunLength > (runEnd - memory) && *runEnd != *(runEnd - 1)) {
                ++runEnd;
            }

            if (runEnd != end) {
                if (MaxRunLength == (runEnd - memory)) {
                    // if we've maxed out and are leaving a dangling byte, shorten the run so that the last byte in the run is encoded with the dangling byte
                    // in the next pass, since we can't encode a lone byte
                    if (runEnd == end - 1) {
                        --runEnd;
                    }
                } else {
                    // runEnd points to the second of a pair of identical bytes, so adjust it to point to first, unless this would make the literal run just
                    // one byte, in which case the first byte of the identical pair is included in the verbatim set
                    if (runEnd > (memory + 2)) {
                        --runEnd;
                    } else if (runEnd == end - 1) {
                        // we've got a run of 2 literal bytes, the second of which is the first of a pair at the end of the buffer, so encode all three in the
                        // literal run to avoid leaving the second byte of the pair dangling (we can't encode a lone byte)
                        ++runEnd;
                    }
                }
            }
        }

        // at this point, memory points to the first byte in the sequence to be encoded and runEnd points to the first byte past the end of the sequence to be
        // encoded, so runEnd - memory tells us how long the run is. currentRunType tells us how to encode the run, and because of the values in the RunType
        // enum, it also contains bit 7 of the signal byte we write to the compressed data.
        //
        // note that when decompressing, 1 is added to the value represented by the signal byte, so we subtract 1 before writing

        switch (currentRunType) {
            case RunType::Literal:
                ret.push_back(runEnd - memory - 1);

                // dump all the source bytes to the compressed buffer
                while (memory < runEnd) {
                    ret.push_back(*memory);
                    ++memory;
                }
                break;

            case RunType::Replicate:
                ret.push_back(~static_cast<std::uint8_t>(runEnd - memory - 2));
                // dump just the replicated byte to the compressed buffer
                ret.push_back(*memory);
                memory = runEnd;
                break;
        }
    }

    return ret;
}

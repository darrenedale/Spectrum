//
// Created by darren on 22/03/2021.
//

#ifndef SPECTRUM_Z80SNAPSHOTREADER_H
#define SPECTRUM_Z80SNAPSHOTREADER_H

#include <optional>

#include "snapshotreader.h"
#include "../z80/types.h"

namespace Spectrum
{
    class Z80SnapshotReader
    : public SnapshotReader
    {
    public:
        using SnapshotReader::SnapshotReader;

    protected:
        bool readInto(Snapshot & snapshot) const override;
        static void decompress(Z80::UnsignedByte * dest, Z80::UnsignedByte * source, std::size_t size);

        /**
         * Attempt to determine the size of a block of compressed memory.
         *
         * In early .z80 files the block of compressed memory does not have a header indicating the number of
         * compressed bytes; rather, it has a byte sequence that signals the end of the compressed memory. This function
         * searches for the marker and returns the number of bytes before the marker if found, or an empty optional if
         * not.
         *
         * @param memory
         * @param max
         * @return
         */
        static std::optional<std::size_t> compressedSize(Z80::UnsignedByte * memory, std::size_t max);
    };
}

#endif //SPECTRUM_Z80SNAPSHOTREADER_H

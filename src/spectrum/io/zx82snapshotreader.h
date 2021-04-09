//
// Created by darren on 08/04/2021.
//

#ifndef SPECTRUM_IO_ZX82SNAPSHOTREADER_H
#define SPECTRUM_IO_ZX82SNAPSHOTREADER_H

#include "snapshotreader.h"
#include "../../z80/types.h"

namespace Spectrum::Io
{
    /**
     * Reader for the ZX82 snapshot format from the Speculator emulator (AmigaOS).
     *
     * References:
     * - https://hwiegman.home.xs4all.nl/fileformats/spectrum/Spectrum%20FAQ%20-%20File%20Formats.htm
     * - http://aminet.net/package/misc/emu/Speculator (See Docs/Spec97.guide)
     */
    class ZX82SnapshotReader
    : public SnapshotReader
    {
    public:
        using SnapshotReader::SnapshotReader;

    protected:
        /**
         * Read the snapshot represented by the file into the provided Snapshot object.
         *
         * @param snapshot
         *
         * @return True on success, false on failure.
         */
        bool readInto(Snapshot & snapshot) const override;

        /**
         * Decompress bytes read from the file/stream using the IFF/ILBM runLength1 scheme.
         *
         * The data is decompressed into a pre-allocated buffer. The buffer must be at least 48kb in size in order t
         * store a fully-decompressed Spectrum 48K RAM image.
         *
         * @param dest A buffer into which to store the decompressed bytes.
         * @param source The compressed image.
         * @param size The number of bytes in the compressed image.
         */
        static bool decompress(Z80::UnsignedByte * dest, Z80::UnsignedByte * source, std::size_t size);
    };
}

#endif //SPECTRUM_IO_ZX82SNAPSHOTREADER_H

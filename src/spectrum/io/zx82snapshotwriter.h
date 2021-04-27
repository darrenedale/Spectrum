//
// Created by darren on 27/04/2021.
//

#ifndef SPECTRUM_IO_ZX82SNAPSHOTWRITER_H
#define SPECTRUM_IO_ZX82SNAPSHOTWRITER_H

#include "snapshotwriter.h"
#include "../../z80/types.h"

namespace Spectrum::Io
{
    /**
     * Write a snapshot as a ZX82 file.
     *
     * Files of this format only support Spectrum 48K snapshots. It's a format that originated on the Amiga in the Speculator emulator. It can optionally
     * compress the memory image of the Spectrum 48K using the ByteRun1 scheme. The default behaviour for a writer is to use compression. Call
     * enableCompression(false) to turn off compression before writing snapshots.
     *
     * References:
     * - https://hwiegman.home.xs4all.nl/fileformats/spectrum/Spectrum%20FAQ%20-%20File%20Formats.htm
     * - http://aminet.net/package/misc/emu/Speculator (See Docs/Spec97.guide)
     */
    class Zx82SnapshotWriter
    : public SnapshotWriter
    {
    public:
        /**
         * Import the base class constructors.
         */
        using SnapshotWriter::SnapshotWriter;

        /**
         * Set whether to enable compression of the memory image in the written snapshot file.
         *
         * @param enable
         */
        void enableCompression(bool enable = true)
        {
            m_compression = enable;
        }

        /**
         * Check whether the writer will compress the memory image in the written snapshot file.
         *
         * @return true if compression is enabled, false otherwise.
         */
        [[nodiscard]] bool compressionEnabled() const
        {
            return m_compression;
        }

        /**
         * Write the snapshot to the provided stream.
         *
         * @param out The stream to write to.
         *
         * @return true if the write succeeded, false if not.
         */
        bool writeTo(std::ostream & out) const override;

        /**
         * Import remaining overloads of writeTo() from base class.
         */
        using SnapshotWriter::writeTo;

    protected:
        /**
         * Convenience alias for the type representing compressed memory.
         */
        using CompressedMemory = std::vector<::Z80::UnsignedByte>;

        /**
         * Helper to compress a memory image.
         *
         * The data is compressed using the ByteRun1 compression scheme used in Amiga IFF files. Note that in some rare circumstances the "compressed" memory
         * can be larger than the uncompressed.
         *
         * @param memory A pointer to the uncompressed memory to compress.
         * @param size The size of the uncompressed memory.
         *
         * @return A vector of bytes containing the compressed representation of the provided memory.
         */
        static CompressedMemory compressMemory(const Z80::UnsignedByte * memory, std::uint32_t size);

    private:
        /**
         * Whether or not compression is enabled.
         */
        bool m_compression = true;
    };
}

#endif //SPECTRUM_IO_ZX82SNAPSHOTWRITER_H

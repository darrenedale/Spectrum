//
// Created by darren on 14/03/2021.
//

#ifndef SPECTRUM_IO_Z80SNAPSHOTWRITER_H
#define SPECTRUM_IO_Z80SNAPSHOTWRITER_H

#include "snapshotwriter.h"

namespace Spectrum::Io
{
    /**
     * Writes a snapshot in .z80 format.
     *
     * All generated snapshots use the extended .z80 format with compressed memory images stored in pages. The format
     * doesn't support 16K models directly so snapshots of 16K models are output as 48K models (with the extra 32k of
     * memory as all 0xff bytes, as they would present when addressed in a 16K model).
     */
    class Z80SnapshotWriter
    : public SnapshotWriter
    {
    public:
        /**
         * Import base class constructors.
         */
        using SnapshotWriter::SnapshotWriter;

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
         * Helper to construct the byte representing the last output to port 0x7ffd.
         *
         * 0x7ffd is the port on which the memory paging for 128K models is controlled. The value of this byte represents the paging state of the snapshot. The
         * state of the current snapshot will be encoded into the returned byte.
         *
         * @return
         */
        [[nodiscard]] std::uint8_t lastOut0x7ffd() const;

        /**
         * Helper to construct the byte representing the last output to port 0x1ffd.
         *
         * 0x1ffd is the port on which the extended memory paging features of the +2a/+3 models are controlled. The value of this byte represents the extended
         * paging state of the snapshot. The state of the current snapshot will be encoded into the returned byte.
         *
         * @return
         */
        [[nodiscard]] std::uint8_t lastOut0x1ffd() const;

        /**
         * Write the header for the current snapshot.
         *
         * The header is always the extended header for the most recent (at the time of writing) .z80 format.
         *
         * @param out
         * @return
         */
        bool writeHeader(std::ostream & out) const;

        /**
         * Helper to write a snapshot taken from a 16K Spectrum.
         *
         * @param out The stream to write to.
         *
         * @return
         */
        bool write16k(std::ostream & out) const;

        /**
         * Helper to write a snapshot taken from a 48K Spectrum.
         *
         * @param out The stream to write to.
         *
         * @return
         */
        bool write48k(std::ostream & out) const;

        /**
         * Helper to write a snapshot taken from a 128K Spectrum model (128/+2/+2a/+3).
         *
         * @param out The stream to write to.
         *
         * @return
         */
        bool write128kModel(std::ostream & out) const;

        /**
         * Helper to write a memory page to an output stream.
         *
         * @param out The stream to write to.
         * @param memory A pointer to the memory in the memory page to write. This *must* be 16kb in size.
         * @param pageNumber The page number that the memory represents.
         *
         * @return
         */
        static bool writeMemoryPage(std::ostream & out, const ::Z80::UnsignedByte * memory, int pageNumber);

        /**
         * Helper to compress a memory image.
         *
         * Note that in some rare circumstances the "compressed" memory can be larger than the uncompressed.
         *
         * @param memory A pointer to the uncompressed memory to compress.
         * @param size The size of the uncompressed memory.
         *
         * @return A vector of bytes containing the compressed representation of the provided memory.
         */
        static CompressedMemory compressMemory(const ::Z80::UnsignedByte * memory, std::uint32_t size);
    };
}

#endif // SPECTRUM_IO_Z80SNAPSHOTWRITER_H

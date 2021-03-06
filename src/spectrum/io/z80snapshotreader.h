//
// Created by darren on 22/03/2021.
//

#ifndef SPECTRUM_IO_Z80SNAPSHOTREADER_H
#define SPECTRUM_IO_Z80SNAPSHOTREADER_H

#include <optional>
#include "snapshotreader.h"
#include "../../z80/types.h"

namespace Spectrum::Io
{
    /**
     * Read a snapshot in .z80 format.
     *
     * All versions of the format (at the time of writing) are supported. Only the models supported by the emulator will be read; other models will not produce
     * a Snapshot object (read() and snapshot() will return nullptr).
     */
    class Z80SnapshotReader
    : public SnapshotReader
    {
    public:
        /**
         * Import the base class constructors.
         */
        using SnapshotReader::SnapshotReader;

        /**
         * Read a Z80 snapshot from the stream.
         *
         * The returned pointer is owned by the reader. It must not be destroyed nor dereferenced after the reader has
         * been destroyed.
         *
         * @return The snapshot.
         */
        const Snapshot * read() const override;

        /**
         * Fetch the unique name for the format read by this class.
         *
         * This is a relatively arbitrary internal name that is distinct from the names of other formats.
         *
         * @return
         */
        static const std::string & formatName()
        {
            static const std::string name("z80");
            return name;
        }

        /**
         * Check whether an input stream looks like it could be a Z80 snapshot.
         *
         * This is not intended as a validation, it is just intended as a means of guessing the format of a potential snapshot input stream. It is assumed the
         * stream is positioned at the start of the potential snapshot.
         *
         * @param in The stream to check.
         *
         * @return true if the stream looks like it contains a Z80 snapshot, false otherwise.
         */
        static bool couldBeSnapshot(std::istream & in);

        /**
         * Check whether a file looks like it could be a Z80 snapshot.
         *
         * @param in The path to the file to check.
         *
         * @return true if the stream looks like it contains a Z80 snapshot, false otherwise.
         */
        static bool couldBeSnapshot(const std::string & fileName);

    protected:
        /**
         * Decompress a block of compressed memory.
         *
         * @param dest A buffer of at least XXX bytes in which to store the decompressed memory.
         * @param source The compressed memory.
         * @param size The number of bytes in the source.
         */
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

#endif //SPECTRUM_IO_Z80SNAPSHOTREADER_H

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
    class Zx82SnapshotReader
    : public SnapshotReader
    {
    public:
        /**
         * Import the base class constructors.
         */
        using SnapshotReader::SnapshotReader;

        /**
         * Attempt to read a snapshot from the current stream.
         *
         * The returned pointer is owned by the reader. It must not be destroyed nor dereferenced after the reader has
         * been destroyed.
         *
         * @return A pointer to a snapshot on success, nullptr on failure.
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
            static const std::string name("zx82");
            return name;
        }

        /**
         * Check whether an input stream looks like it contains a valid ZX82 snapshot.
         *
         * It is assumed the stream is positioned at the start of the potential snapshot.
         *
         * @param in The stream to check.
         *
         * @return true if the stream looks like it contains a ZX82 snapshot, false otherwise.
         */
        static bool couldBeSnapshot(std::istream & in);

        /**
         * Check whether a file looks like it contains a valid ZX82 snapshot.
         *
         * @param in The path to the file to check.
         *
         * @return true if the stream looks like it contains a ZX82 snapshot, false otherwise.
         */
        static bool couldBeSnapshot(const std::string & fileName)
        {
            auto in = std::ifstream(fileName);
            return couldBeSnapshot(in);
        }

    protected:
        /**
         * Decompress bytes read from the stream using the IFF/ILBM runLength1 scheme.
         *
         * The data is decompressed into a pre-allocated buffer. The buffer must be at least 48kb in size in order to store a fully-decompressed Spectrum 48K
         * RAM image.
         *
         * @param dest A buffer into which to store the decompressed bytes.
         * @param source The compressed image.
         * @param size The number of bytes in the compressed image.
         */
        static bool decompress(Z80::UnsignedByte * dest, Z80::UnsignedByte * source, std::size_t size);
    };
}

#endif //SPECTRUM_IO_ZX82SNAPSHOTREADER_H

//
// Created by darren on 22/03/2021.
//

#ifndef SPECTRUM_IO_SNASNAPSHOTREADER_H
#define SPECTRUM_IO_SNASNAPSHOTREADER_H

#include "snapshotreader.h"

namespace Spectrum::Io
{
    /**
     * Read a snapshot in .sna format.
     *
     * There are two variations of this format, one for 48K Spectrums, one for 128K Spectrums. This reader currently only supports the 48K format.
     */
    class SnaSnapshotReader
    : public SnapshotReader
    {
    public:
        /**
         * Import the base class constructors.
         */
        using SnapshotReader::SnapshotReader;

        /**
         * Read a .sna format snapshot from the current stream.
         *
         * The returned pointer is owned by the reader. It must not be destroyed nor dereferenced after the reader has
         * been destroyed.
         *
         * @return A pointer to a Snapshot on success, nullptr on failure.
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
            static const std::string name("sna");
            return name;
        }

        /**
         * Check whether an input stream looks like it contains a valid .sna snapshot.
         *
         * It is assumed the stream is positioned at the start of the potential snapshot.
         *
         * @param in The stream to check.
         *
         * @return true if the stream looks like it contains a .sna snapshot, false otherwise.
         */
        static bool couldBeSnapshot(std::istream & in);

        /**
         * Check whether an file looks like it contains a valid .sna snapshot.
         *
         * @param in The path to the file to check.
         *
         * @return true if the stream looks like it contains a .sna snapshot, false otherwise.
         */
        static bool couldBeSnapshot(const std::string & fileName);
    };
}

#endif //SPECTRUM_IO_SNASNAPSHOTREADER_H

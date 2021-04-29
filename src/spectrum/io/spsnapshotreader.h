//
// Created by darren on 22/03/2021.
//

#ifndef SPECTRUM_SPSNAPSHOTREADER_H
#define SPECTRUM_SPSNAPSHOTREADER_H

#include <optional>

#include "snapshotreader.h"
#include "../../z80/types.h"

namespace Spectrum::Io
{
    /**
     * Read a snapshot in .sp format.
     */
    class SpSnapshotReader
    : public SnapshotReader
    {
    public:
        /**
         * Import the base class constructors.
         */
        using SnapshotReader::SnapshotReader;

        /**
         * Read a snapshot from the current stream.
         *
         * The returned pointer is owned by the reader. It must not be destroyed nor dereferenced after the reader has
         * been destroyed.
         *
         * @return The snapshot, or nullptr if the read failed.
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
            static std::string name("sp");
            return name;
        }

        /**
         * Check whether an input stream looks like it contains a valid SP snapshot.
         *
         * It is assumed the stream is positioned at the start of the potential snapshot.
         *
         * @param in The stream to check.
         *
         * @return true if the stream looks like it contains a SP snapshot, false otherwise.
         */
        static bool couldBeSnapshot(std::istream & in);

        /**
         * Check whether an file looks like it contains a valid SP snapshot.
         *
         * @param in The path to the file to check.
         *
         * @return true if the stream looks like it contains a SP snapshot, false otherwise.
         */
        static bool couldBeSnapshot(const std::string & fileName)
        {
            auto in = std::ifstream(fileName);
            return couldBeSnapshot(in);
        }
    };
}

#endif //SPECTRUM_SPSNAPSHOTREADER_H

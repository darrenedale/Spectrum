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
    };
}

#endif //SPECTRUM_SPSNAPSHOTREADER_H

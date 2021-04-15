//
// Created by darren on 22/03/2021.
//

#ifndef SPECTRUM_SNASNAPSHOTREADER_H
#define SPECTRUM_SNASNAPSHOTREADER_H

#include "snapshotreader.h"

namespace Spectrum::Io
{
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
    };
}

#endif //SPECTRUM_SNASNAPSHOTREADER_H

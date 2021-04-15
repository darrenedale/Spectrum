//
// Created by darren on 14/04/2021.
//

#ifndef SPECTRUM_SPSNAPSHOTWRITER_H
#define SPECTRUM_SPSNAPSHOTWRITER_H

#include "snapshotwriter.h"

namespace Spectrum::Io
{
    /**
     * Write a Spectrum snapshot in .sp format.
     */
    class SpSnapshotWriter
    : public SnapshotWriter
    {
    public:
        /**
         * Import the base class constructors.
         */
        using SnapshotWriter::SnapshotWriter;

        /**
         * Write the current snapshot to the provided stream.
         *
         * @param out The stream to write the snapshot to.
         *
         * @return true if the snapshot was written successfully, false otherwise.
         */
        bool writeTo(std::ostream & out) const override;

        /**
         * Import the remaining overloads of writeTo() from the base class.
         */
        using SnapshotWriter::writeTo;
    };
}

#endif //SPECTRUM_SPSNAPSHOTWRITER_H

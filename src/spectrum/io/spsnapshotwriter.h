//
// Created by darren on 14/04/2021.
//

#ifndef SPECTRUM_IO_SPSNAPSHOTWRITER_H
#define SPECTRUM_IO_SPSNAPSHOTWRITER_H

#include "snapshotwriter.h"

namespace Spectrum::Io
{
    /**
     * Write a Spectrum snapshot in .sp format.
     *
     * Snapshots in .sp format only support 48K Spectrums.
     *
     * Satisfies the SnapshotWriterClass concept.
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
         * Fetch the name for the format written by this writer.
         *
         * Note that we could theoretically make this constexpr - it's in the C++20 standard but is not supported yet in most compilers.
         *
         * @return The string "sp".
         */
        static const std::string & formatName()
        {
            static const std::string name("sp");
            return name;
        }

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

#endif //SPECTRUM_IO_SPSNAPSHOTWRITER_H

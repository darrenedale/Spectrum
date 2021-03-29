//
// Created by darren on 14/03/2021.
//

#ifndef SPECTRUM_Z80SNAPSHOTWRITER_H
#define SPECTRUM_Z80SNAPSHOTWRITER_H

#include "snapshotwriter.h"

namespace Spectrum::Io
{
    /**
     * TODO support later versioned files with the extended header
     * TODO support writing compressed memory images
     */
    class Z80SnapshotWriter
    : public SnapshotWriter
    {
    public:
        using SnapshotWriter::SnapshotWriter;

        bool writeTo(std::ostream & out) const override;
        using SnapshotWriter::writeTo;
    };
}

#endif // SPECTRUM_Z80SNAPSHOTWRITER_H

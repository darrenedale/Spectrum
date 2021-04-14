//
// Created by darren on 14/04/2021.
//

#ifndef SPECTRUM_SPSNAPSHOTWRITER_H
#define SPECTRUM_SPSNAPSHOTWRITER_H

#include "snapshotwriter.h"

namespace Spectrum::Io
{
    class SpSnapshotWriter
    : public SnapshotWriter
    {
    public:
        using SnapshotWriter::SnapshotWriter;

        bool writeTo(std::ostream & out) const override;
        using SnapshotWriter::writeTo;
    };
}

#endif //SPECTRUM_SPSNAPSHOTWRITER_H

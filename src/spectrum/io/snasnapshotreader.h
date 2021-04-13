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
        using SnapshotReader::SnapshotReader;
        const Snapshot * read() const override;

    protected:
    };
}

#endif //SPECTRUM_SNASNAPSHOTREADER_H

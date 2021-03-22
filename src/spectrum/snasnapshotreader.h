//
// Created by darren on 22/03/2021.
//

#ifndef SPECTRUM_SNASNAPSHOTREADER_H
#define SPECTRUM_SNASNAPSHOTREADER_H

#include "snapshotreader.h"

namespace Spectrum
{
    class SnaSnapshotReader
    : public SnapshotReader
    {
    public:
        using SnapshotReader::SnapshotReader;

    protected:
        bool readInto(Snapshot & snapshot) const override;
    };
}

#endif //SPECTRUM_SNASNAPSHOTREADER_H

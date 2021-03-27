//
// Created by darren on 22/03/2021.
//

#ifndef SPECTRUM_SPSNAPSHOTREADER_H
#define SPECTRUM_SPSNAPSHOTREADER_H

#include <optional>

#include "snapshotreader.h"
#include "../z80/types.h"

namespace Spectrum
{
    class SpSnapshotReader
    : public SnapshotReader
    {
    public:
        using SnapshotReader::SnapshotReader;

    protected:
        bool readInto(Snapshot & snapshot) const override;
    };
}

#endif //SPECTRUM_SPSNAPSHOTREADER_H

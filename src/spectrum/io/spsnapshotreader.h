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
    class SpSnapshotReader
    : public SnapshotReader
    {
    public:
        using SnapshotReader::SnapshotReader;
        const Snapshot * read() const override;

    protected:
    };
}

#endif //SPECTRUM_SPSNAPSHOTREADER_H

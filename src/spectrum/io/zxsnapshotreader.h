//
// Created by darren on 22/03/2021.
//

#ifndef SPECTRUM_ZXSNAPSHOTREADER_H
#define SPECTRUM_ZXSNAPSHOTREADER_H

#include "snapshotreader.h"

namespace Spectrum::Io
{
    class ZXSnapshotReader
    : public SnapshotReader
    {
    public:
        using SnapshotReader::SnapshotReader;

        const Snapshot * read() const override;
    };
}

#endif //SPECTRUM_ZXSNAPSHOTREADER_H

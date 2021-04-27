//
// Created by darren on 22/03/2021.
//

#ifndef SPECTRUM_ZXSNAPSHOTREADER_H
#define SPECTRUM_ZXSNAPSHOTREADER_H

#include "snapshotreader.h"

namespace Spectrum::Io
{
    /**
     * Reader class for snapshots in .zx format.
     *
     * References:
     * - http://spectrum-zx.chat.ru/faq/fileform.html
     */
    class ZxSnapshotReader
    : public SnapshotReader
    {
    public:
        /**
         * Import the base class constructors.
         */
        using SnapshotReader::SnapshotReader;

        /**
         * Attempt to read a snapshot from the current stream.
         *
         * The returned pointer is owned by the reader. It must not be destroyed nor dereferenced after the reader has
         * been destroyed.
         *
         * @return A Snapshot, or nullptr if the read failed.
         */
        const Snapshot * read() const override;
    };
}

#endif //SPECTRUM_ZXSNAPSHOTREADER_H

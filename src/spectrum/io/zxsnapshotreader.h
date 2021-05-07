//
// Created by darren on 22/03/2021.
//

#ifndef SPECTRUM_IO_ZXSNAPSHOTREADER_H
#define SPECTRUM_IO_ZXSNAPSHOTREADER_H

#include "snapshotreader.h"

namespace Spectrum::Io
{
    /**
     * Reader class for snapshots in .zx format.
     *
     * References (valid April 2021):
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

        /**
         * Fetch the unique name for the format read by this class.
         *
         * This is a relatively arbitrary internal name that is distinct from the names of other formats.
         *
         * @return
         */
        static const std::string & formatName()
        {
            static const std::string name("zx");
            return name;
        }

        /**
         * Check whether an input stream looks like it contains a valid ZX snapshot.
         *
         * It is assumed the stream is positioned at the start of the potential snapshot.
         *
         * @param in The stream to check.
         *
         * @return true if the stream looks like it contains a ZX snapshot, false otherwise.
         */
        static bool couldBeSnapshot(std::istream & in);

        /**
         * Check whether a file looks like it contains a valid ZX snapshot.
         *
         * @param in The path to the file to check.
         *
         * @return true if the stream looks like it contains a ZX snapshot, false otherwise.
         */
        static bool couldBeSnapshot(const std::string & fileName);
    };
}

#endif //SPECTRUM_IO_ZXSNAPSHOTREADER_H

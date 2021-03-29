//
// Created by darren on 22/03/2021.
//

#ifndef SPECTRUM_SNAPSHOTREADER_H
#define SPECTRUM_SNAPSHOTREADER_H

#include <istream>
#include <fstream>

#include "../snapshot.h"

namespace Spectrum::Io
{
    class SnapshotReader
    {
    public:
        explicit SnapshotReader(std::istream * in)
        : m_borrowedStream(true),
          m_hasBeenRead(false),
          m_in(in),
          m_snapshot()
        {}

        explicit SnapshotReader(const std::string & inFile)
        : m_borrowedStream(false),
          m_hasBeenRead(false),
          m_in(new std::ifstream(inFile)),
          m_snapshot()
        {}

        SnapshotReader(const SnapshotReader & other) = delete;
        SnapshotReader(SnapshotReader && other) noexcept;
        SnapshotReader & operator=(const SnapshotReader & other) = delete;
        SnapshotReader & operator=(SnapshotReader && other) noexcept;

        virtual ~SnapshotReader();

        void setFileName(const std::string & fileName);
        void setStream(std::istream * in);

        /**
         * Force the reader to attempt to read (another) snapshot from the stream.
         *
         * @param ok This parameter will be filled with true/false to indicate success/failure
         *
         * @return The snapshot read. Any previously-retrieved snapshots will also be updated to reflect the newly-read
         * snapshot. Take copies of them first if you need to retain their content. If the read is not successful (see
         * the value of ok) the content of the snapshot is undefined.
         */
        inline const Snapshot & read(bool & ok) const
        {
            ok = readInto(m_snapshot);
            return m_snapshot;
        }

        /**
         * Force the reader to attempt to read (another) snapshot from the stream.
         *
         * @warning There is no way to determine whether the read was successful. Use read(bool) instead.
         *
         * @return The snapshot read. Any previously-retrieved snapshots will also be updated to reflect the newly-read
         * snapshot. Take copies of them first if you need to retain their content. If the read is not successful
         * the content of the snapshot is undefined.
         */
        inline const Snapshot & read() const
        {
            readInto(m_snapshot);
            return m_snapshot;
        }

        /**
         * Retrieve the Snapshot read from the stream.
         *
         * On the assumption that the overwhelming majority of use-cases for readers will be to read a single snapshot
         * from a stream (usually a file), this method will read the stream on the first call and will subsequently
         * return the cached Snapshot read on that first call. It is therefore very fast.
         *
         * See read() if you have a stream with multiple concatenated snapshots.
         *
         * @return
         */
        [[nodiscard]] const Snapshot & snapshot() const
        {
            if (!m_hasBeenRead) {
                read();
            }

            return m_snapshot;
        }

        [[nodiscard]] bool isOpen() const
        {
            return m_in && *m_in;
        }

    protected:
        void disposeStream();
        std::istream & inputStream() const;
        virtual bool readInto(Snapshot & snapshot) const = 0;

    private:
        bool m_borrowedStream;
        bool m_hasBeenRead = false;
        std::istream * m_in;
        mutable Snapshot m_snapshot;
    };
}

#endif //SPECTRUM_SNAPSHOTREADER_H

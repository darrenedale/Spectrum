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
    /**
     * Base class for readers that read Spectrum snapshots in various formats.
     */
    class SnapshotReader
    {
    public:
        /**
         * Initialise a reader with a stream to read.
         *
         * The stream is borrowed not owned. The caller is responsible for ensuring the provided stream outlives the
         * reader.
         *
         * @param in
         */
        explicit SnapshotReader(std::istream & in)
        : m_borrowedStream(true),
          m_in(&in),
          m_snapshot(nullptr)
        {}

        /**
         * Initialise a reader with the name of a file to read.
         *
         * @param inFile
         */
        explicit SnapshotReader(const std::string & inFile)
        : m_borrowedStream(false),
          m_in(new std::ifstream(inFile, std::ios::binary | std::ios::in)),
          m_snapshot(nullptr)
        {}

        // readers can be move constructed/assigned but not copied
        SnapshotReader(const SnapshotReader & other) = delete;
        SnapshotReader(SnapshotReader && other) noexcept;
        SnapshotReader & operator=(const SnapshotReader & other) = delete;
        SnapshotReader & operator=(SnapshotReader && other) noexcept;

        virtual ~SnapshotReader();

        /**
         * Set the reader to read the given file.
         *
         * Setting the stream invalidates any snapshot previously retrieved from the reader.
         *
         * @param fileName
         */
        void setFileName(const std::string & fileName);

        /**
         * Set the reader to use the provided stream for reading.
         *
         * Setting the stream invalidates any snapshot previously retrieved from the reader.
         *
         * The stream is borrowed, not owned. It is the caller's responsibility to ensure the provided stream outlives
         * the reader that is borrowing it.
         *
         * @param in
         */
        void setStream(std::istream & in);

        /**
         * Force the reader to attempt to read (another) snapshot from the stream.
         *
         * @return The snapshot read. The snapshot returned is valid only while the reader exists and only until one of
         * read(), setFileName() or setStream() is next called. On error nullptr will be returned. The returned snapshot
         * is owned by the reader and should not be disposed of externally.
         */
        [[nodiscard]] virtual const Snapshot * read() const = 0;

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
        [[nodiscard]] const Snapshot * snapshot() const
        {
            if (!m_snapshot) {
                return read();
            }

            return m_snapshot.get();
        }

        /**
         * Check whether the stream the reader is using is open.
         *
         * @return
         */
        [[nodiscard]] bool isOpen() const
        {
            return m_in && *m_in;
        }

    protected:
        /**
         * Fetch a reference to the stream from which to read the snapshot.
         *
         * Don't call this unless you are certain that there is a stream to read.
         *
         * @return
         */
        [[nodiscard]] std::istream & inputStream() const;

        /**
         * Implementing classes can call this to cache the snapshot in read() to make it available from snapshot()
         *
         * @param snapshot
         */
        void setSnapshot(std::unique_ptr<Snapshot> snapshot) const
        {
            m_snapshot = std::move(snapshot);
        }

    private:
        /**
         * Discard the stream.
         *
         * After a call to this method inputStream() will not be callable until a fresh stream is set. Any internal
         * method that uses it must ensure a new stream is set before inputStream() can be called.
         */
        void disposeStream();

        bool m_borrowedStream;
        std::istream * m_in;
        mutable std::unique_ptr<Snapshot> m_snapshot;
    };
}

#endif //SPECTRUM_SNAPSHOTREADER_H

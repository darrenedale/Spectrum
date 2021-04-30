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
         * Initialise a reader with a pointer to a stream to read.
         *
         * The stream is borrowed not owned. The caller is responsible for ensuring the provided stream outlives the reader.
         *
         * @param in Pointer to the stream to borrow.
         */
        explicit SnapshotReader(std::istream * in = nullptr)
        : m_borrowedStream(true),
          m_in(in),
          m_snapshot(nullptr)
        {}

        /**
         * Initialise a reader with a pointer to a stream to read.
         *
         * @param in Pointer to the stream. This may be nullptr.
         */
        explicit SnapshotReader(std::unique_ptr<std::istream> in)
        : m_borrowedStream(false),
          m_in(in.release()),
          m_snapshot(nullptr)
        {}

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

        /**
         * Destructor.
         */
        virtual ~SnapshotReader();

        /**
         * Check whether the writer is valid and can be used to read.
         *
         * The base implementation just checks that the reader has an input stream. Derived classes may implement additional logic.
         *
         * @return true if the reader is valid, false otherwise.
         */
        [[nodiscard]] virtual bool isValid() const
        {
            return m_in;
        }

        /**
         * Set the reader to read the given file.
         *
         * Setting the file invalidates any snapshot previously retrieved from the reader.
         *
         * @param fileName
         */
        void setFileName(const std::string & fileName);

        /**
         * Set the reader to use the provided stream for reading.
         *
         * Setting the stream invalidates any snapshot previously retrieved from the reader.
         *
         * The stream is borrowed, not owned. It is the caller's responsibility to ensure the provided stream outlives the reader that is borrowing it.
         *
         * @param in
         */
        void setStream(std::istream & in);

        /**
         * Set the reader to use the provided stream for reading.
         *
         * Setting the stream invalidates any snapshot previously retrieved from the reader.
         *
         * The stream is borrowed, not owned. It is the caller's responsibility to ensure the provided stream outlives the reader that is borrowing it.
         *
         * @param in
         */
        void setStream(std::istream * in);

        /**
         * Set the reader to use the provided stream for reading.
         *
         * Setting the stream invalidates any snapshot previously retrieved from the reader.
         *
         * Ownership of the stream is transferred to the reader. The reader will delete the stream when it is destroyed.
         *
         * @param in
         */
        void setStream(std::unique_ptr<std::istream> in);

        /**
         * Attempt to read (another) snapshot from the current stream.
         *
         * Subclasses implementing this method must return a pointer to a Snapshot on success or nullptr on failure. The
         * returned pointer is owned by the reader. It must not be destroyed nor dereferenced after the reader has been
         * destroyed. It is recommended that the return value is implemented by passing the created Snapshot to the
         * setSnapshot() method of this base class and returning snapshot() to ensure that the created Snapshot's
         * lifetime is managed correctly.
         *
         * @return A pointer to the snapshot read, or nullptr on error. The snapshot returned is valid only while the
         * reader exists and only until one of read(), setFileName() or setStream() is next called.
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

        /**
         * Flag to indicate whether the reader is borrowing the stream it's reading. If this is set, the destructor will
         * not destroy the stream, otherwise it will.
         */
        bool m_borrowedStream;

        /**
         * The stream to read from.
         */
        std::istream * m_in;

        /**
         * The last snapshot read from the current stream, if any.
         */
        mutable std::unique_ptr<Snapshot> m_snapshot;
    };
}

#endif //SPECTRUM_SNAPSHOTREADER_H

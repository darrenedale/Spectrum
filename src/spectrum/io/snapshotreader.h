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
          m_hasBeenRead(false),
          m_in(&in),
          m_snapshot()
        {}

        /**
         * Initialise a reader with the name of a file to read.
         *
         * @param inFile
         */
        explicit SnapshotReader(const std::string & inFile)
        : m_borrowedStream(false),
          m_hasBeenRead(false),
          m_in(new std::ifstream(inFile)),
          m_snapshot()
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
         * @param fileName
         */
        void setFileName(const std::string & fileName);

        /**
         * Set the reader to use the provided stream for reading.
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
        std::istream & inputStream() const;

        /**
         * Read the snapshot contained in the stream into the provided Snapshot object.
         *
         * @param snapshot
         *
         * @return True on success, false on failure.
         */
        virtual bool readInto(Snapshot & snapshot) const = 0;

    private:
        /**
         * Discard the stream.
         *
         * After a call to this method inputStream() will not be callable until a fresh stream is set. Any internal
         * method that uses it must ensure a new stream is set before inputStream() can be called.
         */
        void disposeStream();

        bool m_borrowedStream;
        bool m_hasBeenRead = false;
        std::istream * m_in;
        mutable Snapshot m_snapshot;
    };
}

#endif //SPECTRUM_SNAPSHOTREADER_H

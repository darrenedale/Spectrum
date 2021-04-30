//
// Created by darren on 15/03/2021.
//

#ifndef SPECTRUM_IO_SNAPSHOTWRITER_H
#define SPECTRUM_IO_SNAPSHOTWRITER_H

#include <ostream>
#include <fstream>
#include "../snapshot.h"

namespace Spectrum::Io
{
    /**
     * Base class for serialisation of Spectrum snapshots.
     *
     * In order to participate in resolution in SnapshotWriterFactory, derived classes must satisfy the SnapshotWriterClass concept also.
     */
    class SnapshotWriter
    {
    public:
        /**
         * Initialise a snapshot writer optionally with a snapshot to write.
         *
         * If no snapshot is provided, the writer won't be usable (see isValid()) until setSnapshot() is called to provide one.
         *
         * @param snapshot The snapshot to write. The default is nullptr.
         */
        explicit SnapshotWriter(std::unique_ptr<Snapshot> snapshot = nullptr) noexcept
        : m_snapshot(std::move(snapshot))
        {}

        /**
         * Initialise a snapshot writer with a copy of a snapshot to write.
         *
         * @param snapshot The snapshot to write. It will be copied.
         */
        explicit SnapshotWriter(const Snapshot & snapshot)
        : m_snapshot(std::make_unique<Snapshot>(snapshot))
        {}

        /**
         * Initialise a snapshot writer by moving the snapshot to write.
         *
         * @param snapshot The snapshot to write.
         */
        explicit SnapshotWriter(Snapshot && snapshot) noexcept
        : m_snapshot(std::make_unique<Snapshot>(std::move(snapshot)))
        {}

        /**
         * Copy constructor.
         *
         * @param other The writer to copy into this.
         */
        SnapshotWriter(const SnapshotWriter & other);

        /**
         * Move constructor.
         *
         * @param other The writer to move into this.
         */
        SnapshotWriter(SnapshotWriter && other) noexcept = default;

        /**
         * Copy assignment.
         *
         * @param other The writer to copy-assign to this.
         *
         * @return
         */
        SnapshotWriter & operator=(const SnapshotWriter & other);

        /**
         * Move assignment.
         *
         * @param other The writer to move-assign to this.
         *
         * @return
         */
        SnapshotWriter & operator=(SnapshotWriter && other) noexcept = default;

        /**
         * Destructor.
         */
        virtual ~SnapshotWriter() = default;

        /**
         * Check whether the writer is valid and can be used to write.
         *
         * The base implementation just checks that the writer has a snapshot. Derived classes may implement additional logic.
         *
         * @return
         */
        [[nodiscard]] virtual bool isValid() const
        {
            return static_cast<bool>(m_snapshot);
        }

        /**
         * Set the snapshot to write.
         *
         * The provided snapshot is moved into the writer.
         *
         * @param snapshot The snapshot to write.
         */
        void setSnapshot(std::unique_ptr<Snapshot> snapshot) noexcept
        {
            m_snapshot = std::move(snapshot);
        }

        /**
         * Set the snapshot to write.
         *
         * A copy of the provided snapshot is taken.
         *
         * @param snapshot The snapshot to write.
         */
        void setSnapshot(const Snapshot & snapshot)
        {
            m_snapshot = std::make_unique<Snapshot>(snapshot);
        }

        /**
         * Set the snapshot to write.
         *
         * The provided snapshot is moved into the writer.
         *
         * @param snapshot The snapshot to write.
         */
        void setSnapshot(Snapshot && snapshot) noexcept
        {
            m_snapshot = std::make_unique<Snapshot>(std::move(snapshot));
        }

        /**
         * Fetch a read-write reference to the snapshot to write.
         *
         * Do not call unless you are certain that the writer contains a snapshot. Call isValid() first if you are unsure.
         *
         * @return The snapshot.
         */
        Snapshot & snapshot() noexcept
        {
            assert(m_snapshot);
            return *m_snapshot;
        }

        /**
         * Fetch a read-only reference to the snapshot to write.
         *
         * Do not call unless you are certain that the writer contains a snapshot. Call isValid() first if you are unsure.
         *
         * @return The snapshot.
         */
        [[nodiscard]] const Snapshot & snapshot() const noexcept
        {
            assert(m_snapshot);
            return *m_snapshot;
        }

        /**
         * Write the snapshot to a stream.
         *
         * Subclasses must implement this method to actually write the snapshot in the desired format. If the write is
         * not successful, subclasses should leave the stream without any data having been written. The caveat to this
         * is if the actual attempt to write to the stream is the cause of the failure. Put another way, subclasses
         * should only start writing to the stream when they know they have a valid snapshot that it is at least
         * possible to write.
         *
         * Do not call unless you are certain that the writer is valid. Call isValid() first if you are unsure.
         *
         * @param out The stream to write to.
         *
         * @return true if the snapshot was written successfully, false otherwise.
         */
        virtual bool writeTo(std::ostream & out) const = 0;

        /**
         * Convenience method to write the snapshot to a file.
         *
         * The default implementation simply opens an output stream with the provided file name and passes the stream to
         * the std::ostream overload. This should suffice for most writers.
         *
         * Do not call unless you are certain that the writer is valid. Call isValid() first if you are unsure.
         *
         * @param fileName The full path to the file to write.
         *
         * @return
         */
        [[nodiscard]] virtual bool writeTo(const std::string & fileName) const
        {
            auto out = std::ofstream(fileName);
            return writeTo(out);
        }

    protected:
        /**
         * Helper to write a single native (i.e.. host) endian word to the output stream.
         *
         * The word will be written in Z80 byte order, and will be converted from host byte order if necessary - just
         * provide it the word in host byte order and it will do the right thing.
         *
         * If your word is already in Z80 byte order, use writeZ80Word().
         *
         * @param out
         * @param word
         */
        static void writeHostWord(std::ostream & out, ::Z80::UnsignedWord word);

        /**
         * Helper to write a single Z80-endian (i.e. little-endian) word to the output stream.
         *
         * If your word is in the native (host) byte order, use writeHostWord().
         *
         * @param out
         * @param word
         */
        static void writeZ80Word(std::ostream & out, ::Z80::UnsignedWord word);

    private:
        /**
         * The snapshot to write.
         */
        std::unique_ptr<Snapshot> m_snapshot;
    };
}

#endif //SPECTRUM_IO_SNAPSHOTWRITER_H

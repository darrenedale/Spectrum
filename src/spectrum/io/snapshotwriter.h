//
// Created by darren on 15/03/2021.
//

#ifndef SPECTRUM_SNAPSHOTWRITER_H
#define SPECTRUM_SNAPSHOTWRITER_H

#include <ostream>
#include <fstream>
#include "../snapshot.h"

namespace Spectrum::Io
{
    /**
     * Base class for serialisation of Spectrum snapshots.
     *
     * TODO consider keeping the snapshot as a std::unique_ptr for consistency in the interface and to enable default
     *  construction of writers.
     */
    class SnapshotWriter
    {
    public:
        /**
         * Initialise a snapshot writer with a copy of a snapshot to write.
         *
         * @param snapshot
         */
        explicit SnapshotWriter(const Snapshot & snapshot)
        : m_snapshot(snapshot)
        {}

        /**
         * Initialise a snapshot writer by moving the snapshot to write.
         *
         * @param snapshot
         */
        explicit SnapshotWriter(Snapshot && snapshot)
        : m_snapshot(std::move(snapshot))
        {}

        /**
         * Copy constructor.
         *
         * @param other
         */
        SnapshotWriter(const SnapshotWriter & other) = default;

        /**
         * Move constructor.
         *
         * @param other
         */
        SnapshotWriter(SnapshotWriter && other) = default;

        /**
         * Copy assignment.
         *
         * @param other The writer to copy into this.
         *
         * @return
         */
        SnapshotWriter & operator=(const SnapshotWriter & other) = default;

        /**
         * Move assignment.
         *
         * @param other The writer to move into this.
         *
         * @return
         */
        SnapshotWriter & operator=(SnapshotWriter && other) = default;

        /**
         * Destructor.
         */
        virtual ~SnapshotWriter() = default;

        /**
         * Set the snapshot to write.
         *
         * A copy of the provided snapshot is taken.
         *
         * @param snapshot The snapshot to write.
         */
        void setSnapshot(const Snapshot & snapshot)
        {
            m_snapshot = snapshot;
        }

        /**
         * Set the snapshot to write.
         *
         * The provided snapshot is moved into the writer.
         *
         * @param snapshot The snapshot to write.
         */
        void setSnapshot(Snapshot && snapshot)
        {
            m_snapshot = std::move(snapshot);
        }

        /**
         * Fetch a read-write reference to the snapshot to write.
         *
         * @return The snapshot.
         */
        Snapshot & snapshot()
        {
            return m_snapshot;
        }

        /**
         * Fetch a read-only reference to the snapshot to write.
         *
         * @return The snapshot.
         */
        [[nodiscard]] const Snapshot & snapshot() const
        {
            return m_snapshot;
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
        Snapshot m_snapshot;
    };
}

#endif //SPECTRUM_SNAPSHOTWRITER_H

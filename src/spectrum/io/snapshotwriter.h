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
    class SnapshotWriter
    {
    public:
        explicit SnapshotWriter(const Snapshot & snapshot)
        : m_snapshot(snapshot)
        {}

        explicit SnapshotWriter(Snapshot && snapshot)
        : m_snapshot(std::move(snapshot))
        {}

        SnapshotWriter(const SnapshotWriter & other) = default;
        SnapshotWriter(SnapshotWriter && other) = default;
        SnapshotWriter & operator=(const SnapshotWriter & other) = default;
        SnapshotWriter & operator=(SnapshotWriter && other) = default;
        virtual ~SnapshotWriter() = default;

        void setSnapshot(const Snapshot & snapshot)
        {
            m_snapshot = snapshot;
        }

        void setSnapshot(Snapshot && snapshot)
        {
            m_snapshot = std::move(snapshot);
        }

        Snapshot & snapshot()
        {
            return m_snapshot;
        }

        [[nodiscard]] const Snapshot & snapshot() const
        {
            return m_snapshot;
        }

        virtual bool writeTo(std::ostream & out) const = 0;

        [[nodiscard]] virtual bool writeTo(const std::string & fileName) const
        {
            auto out = std::ofstream(fileName);
            return writeTo(out);
        }

    protected:
        /**
         * Helper to write a single native (i.e.. host) endian word to the output stream.
         *
         * This will be used a lot, e.g. when writing registers. The word will be written in Z80 byte order, and will
         * be converted from host byte order if necessary - just provide it with the word in host byte order and it
         * will do the right thing.
         *
         * If your word is already in Z80 byte order, use writeZ80Word().
         *
         * These two helpers are provided on the assumption that most snapshot formats will expect values to be written
         * in Z80 byte order.
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
         * These two helpers are provided on the assumption that most snapshot formats will expect values to be written
         * in Z80 byte order.
         *
         * @param out
         * @param word
         */
        static void writeZ80Word(std::ostream & out, ::Z80::UnsignedWord word);

    private:
        Snapshot m_snapshot;
    };
}

#endif //SPECTRUM_SNAPSHOTWRITER_H

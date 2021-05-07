//
// Created by darren on 27/04/2021.
//

#ifndef SPECTRUM_IO_ZXSNAPSHOTWRITER_H
#define SPECTRUM_IO_ZXSNAPSHOTWRITER_H

#include "snapshotwriter.h"
#include "../../z80/types.h"

namespace Spectrum::Io
{
    /**
     * Write a snapshot in .zx (Amiga KGB emulator) format.
     *
     * This isn't a great format for interchange - as far as I can tell the screen border colour is not stored and there's plenty of wasted space in the
     * "header" that stores the registers etc. It also stores the memory image before the "header" so it can't be extended to support 128K Spectrums.
     *
     * Satisfies the SnapshotWriterClass concept.
     */
    class ZxSnapshotWriter
    : public SnapshotWriter
    {
    public:
        /**
         * Import the base class constructors.
         */
        using SnapshotWriter::SnapshotWriter;

        /**
         * Fetch the name for the format written by this writer.
         *
         * Note that we could theoretically make this constexpr - it's in the C++20 standard but is not supported yet in most compilers.
         *
         * @return The string "zx".
         */
        static const std::string & formatName()
        {
            static const std::string name("zx");
            return name;
        }

        /**
         * Write the snapshot to the provided stream.
         *
         * @param out The stream to write to.
         *
         * @return true if the write succeeded, false if not.
         */
        bool writeTo(std::ostream & out) const override;

        /**
         * Import remaining overloads of writeTo() from base class.
         */
        using SnapshotWriter::writeTo;

    };
}

#endif //SPECTRUM_IO_ZXSNAPSHOTWRITER_H
